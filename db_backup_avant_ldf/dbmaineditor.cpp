#include "dbmaineditor.h"
#include "ui_dbmaineditor.h"

#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QColorDialog>
#include <QTableWidgetItem>
#include <QRandomGenerator>
#include <qevent.h>
#include "helpwindow.h"

dbMainEditor::dbMainEditor( const QVector<CANFrame> *frames, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dbMainEditor)
{
    ui->setupUi(this);

    readSettings();

    dbHandler = dbHandler::getReference();
    referenceFrames = frames;

    ui->treedb->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->btnSearch, &QAbstractButton::clicked, this, &dbMainEditor::handleSearch);
    connect(ui->lineSearch, &QLineEdit::returnPressed, this, &dbMainEditor::handleSearch);
    connect(ui->btnSearchNext, &QAbstractButton::clicked, this, &dbMainEditor::handleSearchForward);
    connect(ui->btnSearchPrev, &QAbstractButton::clicked, this, &dbMainEditor::handleSearchBackward);
    connect(ui->treedb, &QTreeWidget::doubleClicked, this, &dbMainEditor::onTreeDoubleClicked);
    connect(ui->treedb, &QTreeWidget::customContextMenuRequested, this, &dbMainEditor::onTreeContextMenu);
    connect(ui->treedb, &QTreeWidget::currentItemChanged, this, &dbMainEditor::currentItemChanged);
    connect(ui->btnDelete, &QAbstractButton::clicked, this, &dbMainEditor::deleteCurrentTreeItem);
    connect(ui->btnNewNode, &QAbstractButton::clicked, this, QOverload<>::of(&dbMainEditor::newNode));
    connect(ui->btnNewMessage, &QAbstractButton::clicked, this, &dbMainEditor::newMessage);
    connect(ui->btnNewSignal, &QAbstractButton::clicked, this, &dbMainEditor::newSignal);

    sigEditor = new dbSignalEditor(this);
    msgEditor = new dbMessageEditor(this);
    nodeEditor = new dbNodeEditor(this);
    nodeRebaseEditor = new dbNodeRebaseEditor(this);
    nodeDuplicateEditor = new dbNodeDuplicateEditor(this);

    //all three might potentially change the data stored and force the tree to be updated
    connect(sigEditor, &dbSignalEditor::updatedTreeInfo, this, &dbMainEditor::updatedSignal);
    connect(msgEditor, &dbMessageEditor::updatedTreeInfo, this, &dbMainEditor::updatedMessage);
    connect(nodeEditor, &dbNodeEditor::updatedTreeInfo, this, &dbMainEditor::updatedNode);
    connect(nodeRebaseEditor, &dbNodeRebaseEditor::updatedTreeInfo, this, &dbMainEditor::updatedMessage);
    connect(nodeDuplicateEditor, &dbNodeDuplicateEditor::updatedTreeInfo, this, &dbMainEditor::updatedMessage);
    connect(nodeDuplicateEditor, &dbNodeDuplicateEditor::createNode, this, QOverload<QString>::of(&dbMainEditor::newNode));
    connect(nodeDuplicateEditor, &dbNodeDuplicateEditor::cloneMessageToNode, this, &dbMainEditor::copyMessageToNode);
    connect(nodeDuplicateEditor, &dbNodeDuplicateEditor::nodeAdded, this, &dbMainEditor::refreshTree);

    nodeIcon = QIcon(":/icons/images/node.png");
    messageIcon = QIcon(":/icons/images/message.png");
    signalIcon = QIcon(":/icons/images/signal.png");
    multiplexedSignalIcon = QIcon(":/icons/images/multiplexed-signal.png");
    multiplexorSignalIcon = QIcon(":/icons/images/multiplexor-signal.png");

    //ui->btnDelete->setFixedSize(32,32);
    ui->btnDelete->setIconSize(QSize(32, 32));

    //ui->btnNewNode->setFixedSize(32,32);
    ui->btnNewNode->setIconSize(QSize(32, 32));

    //ui->btnNewMessage->setFixedSize(32,32);
    ui->btnNewMessage->setIconSize(QSize(32, 32));

    //ui->btnNewSignal->setFixedSize(32,32);
    ui->btnNewSignal->setIconSize(QSize(32, 32));

    installEventFilter(this);
}

void dbMainEditor::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);

    refreshTree();
}

dbMainEditor::~dbMainEditor()
{
    removeEventFilter(this);
    delete ui;
    delete sigEditor;
    delete msgEditor;
    delete nodeEditor;
}

bool dbMainEditor::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key())
        {
        case Qt::Key_F1:
            HelpWindow::getRef()->showHelp("db_editor.md");
            break;
        case Qt::Key_F3:
            handleSearchForward();
            break;
        case Qt::Key_F4:
            handleSearchBackward();
            break;
        case Qt::Key_F5:
            newNode();
            break;
        case Qt::Key_F6:
            newMessage();
            break;
        case Qt::Key_F7:
            newSignal();
            break;
        case Qt::Key_Delete:
            deleteCurrentTreeItem();
        }
        return true;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
    return false;
}

void dbMainEditor::setFileIdx(int idx)
{
    if (idx < 0 || idx > dbHandler->getFileCount() - 1) return;
    dbFile = dbHandler->getFileByIdx(idx);
    fileIdx = idx;
}

void dbMainEditor::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    writeSettings();
    sigEditor->close();
}

void dbMainEditor::readSettings()
{
    QSettings settings;
    if (settings.value("Main/SaveRestorePositions", false).toBool())
    {
        resize(settings.value("dbMainEditor/WindowSize", QSize(1103, 571)).toSize());
        move(Utility::constrainedWindowPos(settings.value("dbMainEditor/WindowPos", QPoint(50, 50)).toPoint()));
    }
}

void dbMainEditor::writeSettings()
{
    QSettings settings;

    if (settings.value("Main/SaveRestorePositions", false).toBool())
    {
        settings.setValue("dbMainEditor/WindowSize", size());
        settings.setValue("dbMainEditor/WindowPos", pos());
    }
}

void dbMainEditor::onCustomMenuTree(QPoint point)
{
    Q_UNUSED(point);
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    menu->addAction(tr("Delete currently selected message"), this, SLOT(deleteCurrentMessage()));

    //menu->popup(ui->MessagesTable->mapToGlobal(point));

}

void dbMainEditor::handleSearch()
{
    searchItems = ui->treedb->findItems(ui->lineSearch->text(),Qt::MatchContains | Qt::MatchRecursive);
    qDebug() << "Search returned " << searchItems.count() << "items.";
    if (searchItems.count() > 0)
    {
        ui->treedb->setCurrentItem(searchItems[0]);
        searchItemPos = 0;
        ui->lblSearchPos->setText("Search Results: " + QString::number(searchItemPos + 1) + " of " + QString::number(searchItems.count()));
    }
    else
    {
        ui->lblSearchPos->setText("Search Results: 0 of 0");
    }
}

void dbMainEditor::handleSearchForward()
{
    if (searchItems.count() == 0) return;
    if (searchItemPos < searchItems.count() - 1) searchItemPos++;
    else searchItemPos = 0;
    ui->treedb->setCurrentItem(searchItems[searchItemPos]);
    ui->lblSearchPos->setText("Search Results: " + QString::number(searchItemPos + 1) + " of " + QString::number(searchItems.count()));
}

void dbMainEditor::handleSearchBackward()
{
    if (searchItems.count() == 0) return;
    if (searchItemPos > 0) searchItemPos--;
    else searchItemPos = searchItems.count() - 1;
    ui->treedb->setCurrentItem(searchItems[searchItemPos]);
    ui->lblSearchPos->setText("Search Results: " + QString::number(searchItemPos + 1) + " of " + QString::number(searchItems.count()));
}

void dbMainEditor::currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *prev)
{
    Q_UNUSED(prev)
    if (!current)
    {
        ui->btnNewMessage->setEnabled(false);
        ui->btnNewSignal->setEnabled(false);
        ui->btnDelete->setEnabled(false);
        return;
    }
    switch (current->data(0, Qt::UserRole).toInt())
    {
    case dbItemTypes::NODE: //node
        ui->btnNewNode->setEnabled(true);
        ui->btnNewMessage->setEnabled(true);
        ui->btnNewSignal->setEnabled(false);
        ui->btnDelete->setEnabled(true);
        break;
    case dbItemTypes::MESG: //message
        ui->btnNewNode->setEnabled(true);
        ui->btnNewMessage->setEnabled(true);
        ui->btnNewSignal->setEnabled(true);
        ui->btnDelete->setEnabled(true);
        break;
    case dbItemTypes::SIG: //signal
        ui->btnNewNode->setEnabled(true);
        ui->btnNewMessage->setEnabled(true);
        ui->btnNewSignal->setEnabled(true);
        ui->btnDelete->setEnabled(true);
        break;
    default:
        ui->btnNewMessage->setEnabled(false);
        ui->btnNewSignal->setEnabled(false);
        ui->btnDelete->setEnabled(false);
    }
}

uint32_t dbMainEditor::getParentMessageID(QTreeWidgetItem *cell)
{
    if (cell->data(0, Qt::UserRole) == dbItemTypes::MESG)
    {
        return static_cast<uint32_t>(Utility::ParseStringToNum(cell->text(0).split(" ")[0]));
    }
    else
    {
        if (cell->parent()) return getParentMessageID(cell->parent());
        else return 0;
    }
}

//Double clicking is interpreted as a desire to edit the given item.
void dbMainEditor::onTreeDoubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index)

    QTreeWidgetItem* firstCol = ui->treedb->currentItem();
    //bool ret = false;
    db_MESSAGE *msg;
    DB_SIGNAL *sig;
    db_NODE *node;
    uint32_t msgID;
    QString idString;

    qDebug() << firstCol->data(0, Qt::UserRole) << " - " << firstCol->text(0);

    switch (firstCol->data(0, Qt::UserRole).toInt())
    {
    case dbItemTypes::NODE: //a node
        idString = firstCol->text(0).split(" ")[0];
        node = dbFile->findNodeByName(idString);
        nodeEditor->setFileIdx(fileIdx);
        nodeEditor->setNodeRef(node);
        nodeEditor->refreshView();
        nodeEditor->show();
        break;
    case dbItemTypes::MESG: //a message
        idString = firstCol->text(0).split(" ")[0];
        msgID = static_cast<uint32_t>(Utility::ParseStringToNum(idString));
        msg = dbFile->messageHandler->findMsgByID(msgID);
        msgEditor->setMessageRef(msg);
        msgEditor->setFileIdx(fileIdx);
        //msgEditor->setWindowModality(Qt::WindowModal);
        msgEditor->refreshView();
        msgEditor->show(); //show allows the rest of the forms to keep going
        break;
    case dbItemTypes::SIG: //a signal
        msgID = getParentMessageID(firstCol);
        msg = dbFile->messageHandler->findMsgByID(msgID);
        QString nameString = firstCol->text(0);
        if (nameString.contains("("))
        {
            nameString = nameString.split(")")[1].trimmed(); //remove (1-2) type stuff from beginning of string
        }
        nameString = nameString.split(" ")[0]; //get rid of [32m 8] type stuff after the name

        sig = msg->sigHandler->findSignalByName(nameString);
        if (sig)
        {
            sigEditor->setSignalRef(sig);
            sigEditor->setMessageRef(msg);
            sigEditor->setFileIdx(fileIdx);
            //sigEditor->setWindowModality(Qt::WindowModal);
            sigEditor->refreshView();
            sigEditor->show();
        }
        break;
    }
}

void dbMainEditor::onTreeContextMenu(const QPoint & pos)
{
    QTreeWidgetItem* firstCol = ui->treedb->currentItem();
    QString idString;

    qDebug() << firstCol->data(0, Qt::UserRole) << " - " << firstCol->text(0);

    switch (firstCol->data(0, Qt::UserRole).toInt())
    {
    case dbItemTypes::NODE: //a node
        idString = firstCol->text(0).split(" ")[0];
        //node = dbFile->findNodeByName(idString);

        QAction *actionRebase = new QAction(QIcon(":/Resource/warning32.ico"), tr("Rebase all messages"), this);
        actionRebase->setStatusTip(tr("Rebase all messages in node"));
        connect(actionRebase, SIGNAL(triggered()), this, SLOT(onRebaseMessages()));

        QAction *actionDupe = new QAction(QIcon(":/Resource/warning32.ico"), tr("Duplicate node"), this);
        actionDupe->setStatusTip(tr("Duplicate node and messages"));
        connect(actionDupe, SIGNAL(triggered()), this, SLOT(onDuplicateNode()));

        QMenu menu(this);
        menu.addAction(actionRebase);
        menu.addAction(actionDupe);

        //QPoint pt(pos);
        menu.exec( ui->treedb->mapToGlobal(pos) );
        break;
    }
}

void dbMainEditor::onRebaseMessages()
{
    QTreeWidgetItem* firstCol = ui->treedb->currentItem();
    db_NODE *node;
    QString idString;

    idString = firstCol->text(0).split(" ")[0];
    node = dbFile->findNodeByName(idString);
    nodeRebaseEditor->setFileIdx(fileIdx);
    nodeRebaseEditor->setNodeRef(node);
    if(nodeRebaseEditor->refreshView())
    {
        nodeRebaseEditor->setModal(true);
        nodeRebaseEditor->show();
    }
}

void dbMainEditor::onDuplicateNode()
{
    QTreeWidgetItem* firstCol = ui->treedb->currentItem();
    //bool ret = false;
    //db_MESSAGE *msg;
    //DB_SIGNAL *sig;
    //uint32_t msgID;
    db_NODE *node;
    QString idString;

    idString = firstCol->text(0).split(" ")[0];
    node = dbFile->findNodeByName(idString);
    nodeDuplicateEditor->setFileIdx(fileIdx);
    nodeDuplicateEditor->setNodeRef(node);    
    if(nodeDuplicateEditor->refreshView())
    {
        nodeDuplicateEditor->setModal(true);
        nodeDuplicateEditor->show();
    }
}

/*
 * Recreate the whole tree with pretty icons and custom user roles that give the rest of code an easy way to figure out whether a given tree node
 * is a node, message, or signal.
*/
void dbMainEditor::refreshTree()
{
    ui->treedb->clear();
    nodeToItem.clear();
    messageToItem.clear();
    signalToItem.clear();
    itemToNode.clear();
    itemToMessage.clear();
    itemToSignal.clear();

    if (dbFile->findNodeByName("Vector__XXX") == nullptr)
    {
        db_NODE newNode;
        newNode.name = "Vector__XXX";
        newNode.comment = "Default node if no other node is specified";
        dbFile->db_nodes.append(newNode);
    }

    for (int n = 0; n < dbFile->db_nodes.count(); n++)
    {
        db_NODE *node = &dbFile->db_nodes[n];
        QTreeWidgetItem *nodeItem = new QTreeWidgetItem();
        QString nodeInfo = node->name;
        if (node->comment.count() > 0) nodeInfo.append(" - ").append(node->comment);
        nodeItem->setText(0, nodeInfo);
        nodeItem->setIcon(0, nodeIcon);
        nodeItem->setData(0, Qt::UserRole, dbItemTypes::NODE);
        nodeToItem.insert(node, nodeItem);
        itemToNode.insert(nodeItem, node);
        for (int x = 0; x < dbFile->messageHandler->getCount(); x++)
        {
            db_MESSAGE *msg = dbFile->messageHandler->findMsgByIdx(x);
            if (msg->sender->name == node->name)
            {
                QTreeWidgetItem *msgItem = new QTreeWidgetItem(nodeItem);
                QString msgInfo = Utility::formatCANID(msg->ID) + " " + msg->name;
                if (msg->comment.count() > 0) msgInfo.append(" - ").append(msg->comment);
                msgItem->setText(0, msgInfo);
                msgItem->setIcon(0, messageIcon);
                msgItem->setData(0, Qt::UserRole, dbItemTypes::MESG);
                messageToItem.insert(msg, msgItem);
                itemToMessage.insert(msgItem, msg);
                for (int i = 0; i < msg->sigHandler->getCount(); i++)
                {
                    DB_SIGNAL *sig = msg->sigHandler->findSignalByIdx(i);
                    //only process signals here which are "top" level
                    if (sig->multiplexParent == nullptr) processSignalToTree(msgItem, sig);
                }
            }
        }
        ui->treedb->addTopLevelItem(nodeItem);
    }
    ui->treedb->sortItems(0, Qt::SortOrder::AscendingOrder); //sort the display list for ease in viewing by mere mortals, helps me a lot.
}

QString dbMainEditor::createSignalText(DB_SIGNAL *sig)
{
    QString sigInfo;
    if (sig->isMultiplexed)
    {
        sigInfo = "(" + sig->multiplexdbString() + ") ";
    }
    sigInfo.append(sig->name);

    if (sig->intelByteOrder)
        sigInfo.append(" [" + QString::number(sig->startBit) + "i " + QString::number(sig->signalSize) + "]");
    else
        sigInfo.append(" [" + QString::number(sig->startBit) + "m " + QString::number(sig->signalSize) + "]");

    if (sig->comment.count() > 0) sigInfo.append(" - ").append(sig->comment);
    return sigInfo;
}

//Signals can have a hierarchial relationship with other signals so this function is separate and calls itself recursively to build the tree
void dbMainEditor::processSignalToTree(QTreeWidgetItem *parent, DB_SIGNAL *sig)
{
    QTreeWidgetItem *sigItem = new QTreeWidgetItem(parent);
    QString sigInfo = createSignalText(sig);
    sigItem->setText(0, sigInfo);
    if (sig->isMultiplexor) sigItem->setIcon(0, multiplexorSignalIcon);
    else if (sig->isMultiplexed) sigItem->setIcon(0, multiplexedSignalIcon);
    else sigItem->setIcon(0, signalIcon);
    sigItem->setData(0, Qt::UserRole, dbItemTypes::SIG);
    signalToItem.insert(sig, sigItem);
    itemToSignal.insert(sigItem, sig);
    if (sig->multiplexedChildren.count() > 0)
    {
        for (int i = 0; i < sig->multiplexedChildren.count(); i++)
        {
            processSignalToTree(sigItem, sig->multiplexedChildren[i]);
        }
    }
}

void dbMainEditor::updatedNode(db_NODE *node)
{
    if (nodeToItem.contains(node))
    {
        QTreeWidgetItem *item = nodeToItem[node];
        QString nodeInfo = node->name;
        if (node->comment.count() > 0) nodeInfo.append(" - ").append(node->comment);
        item->setText(0, nodeInfo);
    }
    else qDebug() << "That node doesn't exist. That's a bug dude.";
}

void dbMainEditor::updatedMessage(db_MESSAGE *msg)
{
    if (messageToItem.contains(msg))
    {
        QTreeWidgetItem *item = messageToItem.value(msg);
        QString msgInfo = Utility::formatCANID(msg->ID) + " " + msg->name;
        if (msg->comment.count() > 0) msgInfo.append(" - ").append(msg->comment);
        item->setText(0, msgInfo);
        //editor could have changed the parent Node too. Have to figure out which node
        //is parent in the GUI and compare that to parent in the data.
        db_NODE *oldParent = dbFile->findNodeByName(item->parent()->text(0).split(" - ")[0]);
        if (oldParent != msg->sender && oldParent)
        {
            qDebug() << "Changed parent of message. Trying to rehome it.";
            QTreeWidgetItem *newParent = nullptr;
            if (nodeToItem.contains(msg->sender)) newParent = nodeToItem.value(msg->sender);
            else //new node created within message editor. Create the item and then use it
            {
                QTreeWidgetItem *newNodeItem = new QTreeWidgetItem();
                newNodeItem->setText(0, msg->sender->name);
                newNodeItem->setIcon(0, nodeIcon);
                newNodeItem->setData(0, Qt::UserRole, dbItemTypes::NODE);
                ui->treedb->addTopLevelItem(newNodeItem);
                nodeToItem.insert(msg->sender, newNodeItem);
                itemToNode.insert(newNodeItem, msg->sender);
                newParent = newNodeItem;
            }

            QTreeWidgetItem *prevParent = nodeToItem.value(oldParent);
            prevParent->removeChild(item);
            newParent->addChild(item);
            ui->treedb->setCurrentItem(item);
            ui->treedb->sortItems(0, Qt::AscendingOrder); //resort because we just moved an item
        }
    }
    else qDebug() << "That mesage doesn't exist. That's a bug dude.";
}

void dbMainEditor::updatedSignal(DB_SIGNAL *sig)
{
    if (signalToItem.contains(sig))
    {
        QTreeWidgetItem *item = signalToItem.value(sig);
        QString sigInfo = createSignalText(sig);
        item->setText(0, sigInfo);
        if (sig->isMultiplexed)
        {
            if (item->parent()->data(0, Qt::UserRole).toInt() == dbItemTypes::SIG) //if our parent is another signal
            {
                QString nameString = item->parent()->text(0);
                if (nameString.contains("(")) nameString = nameString.split(" ")[1];
                else nameString = nameString.split(" ")[0];
                DB_SIGNAL *oldParent = sig->parentMessage->sigHandler->findSignalByName(nameString);
                if (oldParent && (oldParent != sig->multiplexParent))
                {
                    qDebug() << "You changed the signal's parent";
                    QTreeWidgetItem *newParent = nullptr;
                    newParent = signalToItem.value(sig->multiplexParent);
                    QTreeWidgetItem *prevParent = signalToItem.value(oldParent);
                    prevParent->removeChild(item);
                    newParent->addChild(item);
                    ui->treedb->setCurrentItem(item);
                    ui->treedb->sortItems(0, Qt::AscendingOrder); //resort because we just moved an item
                }
            }
        }
    }
    else qDebug() << "That signal doesn't exist. That's a bug dude.";
}

void dbMainEditor::newNode(QString nodeName)
{
    db_NODE node;
    db_NODE *nodePtr;
    if(nodeName.isEmpty())
    {
        node.name = "Unnamed" + QString::number(randGen.bounded(50000));
    }
    else
    {
        node.name = nodeName;
    }
    dbFile->db_nodes.append(node);
    nodePtr = dbFile->findNodeByName(node.name);
    QTreeWidgetItem *nodeItem = new QTreeWidgetItem();
    nodeItem->setText(0, node.name);
    nodeItem->setIcon(0, nodeIcon);
    nodeItem->setData(0, Qt::UserRole, dbItemTypes::NODE);
    nodeToItem.insert(nodePtr, nodeItem);
    itemToNode.insert(nodeItem, nodePtr);
    ui->treedb->addTopLevelItem(nodeItem);
    ui->treedb->setCurrentItem(nodeItem);
    dbFile->setDirtyFlag();
}

void dbMainEditor::newNode()
{
    newNode(QString());
}

void dbMainEditor::copyMessageToNode(db_NODE *parentNode, db_MESSAGE *source, uint newMsgId)
{
    db_NODE *node = parentNode;
    if (!node) node = dbFile->findNodeByIdx(0);
    QTreeWidgetItem *nodeItem = nullptr;
    db_MESSAGE msg;
    db_MESSAGE *msgPtr;

    nodeItem = ui->treedb->currentItem();

    msg.name = source->name;
    msg.ID = newMsgId;
    msg.len = source->len;
    msg.bgColor = source->bgColor;
    msg.fgColor = source->fgColor;
    msg.comment = source->comment;
    msg.sender = node;

    DB_SIGNAL *sigSource;
    int sigCount = source->sigHandler->getCount();

    for(int i=0; i<sigCount; i++)
    {
        sigSource = source->sigHandler->findSignalByIdx(i);

        DB_SIGNAL sig;

        //Does not properly handle multiplexed signals, for now
        sig.name = sigSource->name;
        sig.bias = sigSource->bias;
        sig.isMultiplexed = false; //sigSource->isMultiplexed;
        sig.isMultiplexor = false; //sigSource->isMultiplexor;
        sig.max = sigSource->max;
        sig.min = sigSource->min;
        sig.copyMultiplexValuesFromSignal(*sigSource);
        sig.factor = sigSource->factor;
        sig.intelByteOrder = sigSource->intelByteOrder;
        sig.parentMessage = &msg;
        sig.multiplexParent = nullptr;  //need to learn about multiplexed signals and track them when copying
        sig.receiver = node;
        sig.signalSize = sigSource->signalSize;
        sig.startBit = sigSource->startBit;
        sig.valType = sigSource->valType;


        sig.parentMessage = &msg;
        msg.sigHandler->addSignal(sig);
    }

    msg.sigHandler->sort();

    dbFile->messageHandler->addMessage(msg);
    msgPtr = dbFile->messageHandler->findMsgByIdx(dbFile->messageHandler->getCount() - 1);
    QTreeWidgetItem *newMsgItem = new QTreeWidgetItem();
    QString msgInfo = Utility::formatCANID(msg.ID) + " " + msg.name;
    if (msg.comment.count() > 0) msgInfo.append(" - ").append(msg.comment);
    newMsgItem->setText(0, msgInfo);
    newMsgItem->setIcon(0, messageIcon);
    newMsgItem->setData(0, Qt::UserRole, dbItemTypes::MESG);
    messageToItem.insert(msgPtr, newMsgItem);
    itemToMessage.insert(newMsgItem, msgPtr);
    nodeItem->addChild(newMsgItem);
    //ui->treedb->setCurrentItem(newMsgItem);
    dbFile->setDirtyFlag();
}

//create a new message with it's parent being the node we're currently within
void dbMainEditor::newMessage()
{
    QTreeWidgetItem *nodeItem = nullptr;
    QTreeWidgetItem *msgItem = nullptr;
    nodeItem = ui->treedb->currentItem();
    int typ = nodeItem->data(0, Qt::UserRole).toInt();
    if (!nodeItem) return; //nothing selected!
    if (typ == dbItemTypes::MESG)
    {
        msgItem = nodeItem;
        nodeItem = nodeItem->parent();
    }
    if (typ == dbItemTypes::SIG)
    {
        msgItem = nodeItem->parent();
        nodeItem = msgItem->parent();
    }
    if (typ == dbItemTypes::NODE){
        msgItem = nodeItem;
    }

    //if there was a comment this will find the location of the comment and snip it out.
    QString nodeName = nodeItem->data(0, Qt::DisplayRole).toString().split(" - ")[0];

    db_NODE *node = dbFile->findNodeByName(nodeName);
    if (!node) node = dbFile->findNodeByIdx(0);
    db_MESSAGE msg;
    db_MESSAGE *msgPtr;
    if (msgItem)
    {
        QString idString = msgItem->text(0).split(" ")[0];
        int msgID = static_cast<uint32_t>(Utility::ParseStringToNum(idString));
        db_MESSAGE *oldMsg = dbFile->messageHandler->findMsgByID(msgID);
        if (oldMsg)
        {
            msg.name = "Msg" + QString::number(randGen.bounded(500));
            msg.ID = oldMsg->ID + 1;
            db_MESSAGE *overlappedMsg = dbFile->messageHandler->findMsgByID(msg.ID);
            while (overlappedMsg)
            {
                msg.ID++;
                overlappedMsg = dbFile->messageHandler->findMsgByID(msg.ID);
            }
            msg.len = oldMsg->len;
            msg.bgColor = oldMsg->bgColor;
            msg.fgColor = oldMsg->fgColor;
            msg.comment = oldMsg->comment;
        }
        else
        {
            msg.name = nodeName + "Msg" + QString::number(randGen.bounded(500));
            msg.ID = 0;
            msg.len = 8;
            msg.bgColor = QApplication::palette().color(QPalette::Base);
        }
    }
    else
    {
        msg.name = nodeName + "Msg" + QString::number(randGen.bounded(500));
        msg.ID = 0;
        msg.len = 8;
    }    
    msg.sender = node;

    dbFile->messageHandler->addMessage(msg);
    msgPtr = dbFile->messageHandler->findMsgByIdx(dbFile->messageHandler->getCount() - 1);
    QTreeWidgetItem *newMsgItem = new QTreeWidgetItem();
    QString msgInfo = Utility::formatCANID(msg.ID) + " " + msg.name;
    if (msg.comment.count() > 0) msgInfo.append(" - ").append(msg.comment);
    newMsgItem->setText(0, msgInfo);
    newMsgItem->setIcon(0, messageIcon);
    newMsgItem->setData(0, Qt::UserRole, dbItemTypes::MESG);
    messageToItem.insert(msgPtr, newMsgItem);
    itemToMessage.insert(newMsgItem, msgPtr);
    nodeItem->addChild(newMsgItem);
    ui->treedb->setCurrentItem(newMsgItem);
    dbFile->setDirtyFlag();
}

void dbMainEditor::newSignal()
{
    QTreeWidgetItem *msgItem = nullptr;
    QTreeWidgetItem *sigItem = nullptr;
    QTreeWidgetItem *parentItem = nullptr;
    msgItem = ui->treedb->currentItem();
    parentItem = msgItem;
    if (!msgItem) return; //nothing selected!
    int typ = msgItem->data(0, Qt::UserRole).toInt();
    if (typ == dbItemTypes::NODE) return; //can't add signals to a node!
    if (typ == dbItemTypes::SIG)
    {
        sigItem = msgItem;
        msgItem = msgItem->parent();
        parentItem = msgItem;
        //walk up the tree to find the parent msg
        while (msgItem && msgItem->data(0, Qt::UserRole).toInt() != dbItemTypes::MESG) msgItem = msgItem->parent();
        if (!msgItem) return; //something bad happened. abort.
    }

    QString idString = msgItem->text(0).split(" ")[0];
    int msgID = static_cast<uint32_t>(Utility::ParseStringToNum(idString));
    db_MESSAGE *msg = dbFile->messageHandler->findMsgByID(msgID);
    if (!msg) return; //null pointers are a bummer. Do not follow them.
    DB_SIGNAL sig;
    DB_SIGNAL *sigPtr;
    if (sigItem)
    {
        QString txt = sigItem->text(0);
        if (txt.startsWith('(')) txt = txt.split(" ")[1]; //if it was a multiplexed signal we need to ignore that part and still grab sig name
        else txt = txt.split(" ")[0];
        DB_SIGNAL *oldSig = msg->sigHandler->findSignalByName(txt);
        if (oldSig)
        {
            sig = *oldSig;
            sig.name = sig.name + QString::number(randGen.bounded(100));
        }
        else
        {
            sig.name = msgItem->text(0).split(" ")[1] + "Sig" + QString::number(randGen.bounded(500));
        }
    }
    else
    {
        sig.name = msgItem->text(0).split(" ")[1] + "Sig" + QString::number(randGen.bounded(500));
    }

    sig.parentMessage = msg;
    if (!sig.receiver) sig.receiver = &dbFile->db_nodes[0]; //if receiver not set then set it to... something.
    msg->sigHandler->addSignal(sig);
    sigPtr = msg->sigHandler->findSignalByIdx(msg->sigHandler->getCount() - 1);
    QTreeWidgetItem *newSigItem = new QTreeWidgetItem();
    QString sigInfo = createSignalText(&sig);
    newSigItem->setText(0, sigInfo);
    if (sig.isMultiplexed) newSigItem->setIcon(0, multiplexedSignalIcon);
    else if (sig.isMultiplexor) newSigItem->setIcon(0, multiplexorSignalIcon);
    else newSigItem->setIcon(0, signalIcon);
    newSigItem->setData(0, Qt::UserRole, dbItemTypes::SIG);
    signalToItem.insert(sigPtr, newSigItem);
    itemToSignal.insert(newSigItem, sigPtr);
    parentItem->addChild(newSigItem);
    ui->treedb->setCurrentItem(newSigItem);
    dbFile->setDirtyFlag();
}

//gets confirmation before calling the real routines that delete things
void dbMainEditor::deleteCurrentTreeItem()
{
    QTreeWidgetItem *currItem = ui->treedb->currentItem();
    int typ = currItem->data(0, Qt::UserRole).toInt();
    QString idString, columnText;
    int msgID;
    db_MESSAGE *msg;
    db_NODE *node;
    int numMsg = 0, numSig = 0;

    QMessageBox::StandardButton confirmDialog;

    switch (typ)
    {
    case dbItemTypes::NODE: //deleting a node cascades deletion down to messages and signals
        columnText = currItem->text(0);
        node = dbFile->findNodeByNameAndComment(columnText);
        if (!node) return;
        for (int x = 0; x < dbFile->messageHandler->getCount(); x++)
        {
            if (dbFile->messageHandler->findMsgByIdx(x)->sender == node)
            {
                numMsg++;
                numSig += dbFile->messageHandler->findMsgByIdx(x)->sigHandler->getCount();
            }
        }
        confirmDialog = QMessageBox::question(this, "Really?", "Are you sure you want to delete this node, its\n"
                                  + QString::number(numMsg) + " messages, and their " + QString::number(numSig) + " combined signals?", QMessageBox::Yes|QMessageBox::No);
        if (confirmDialog == QMessageBox::Yes)
        {
            if (itemToNode.contains(currItem))
            {
                db_NODE *node = itemToNode.value(currItem);
                deleteNode(node);
            }
            else
            {
                qDebug() << "Could not find the node in the map. That should not happen.";
            }
        }

        break;
    case dbItemTypes::MESG: //cascades to removing all signals too.
        idString = currItem->text(0).split(" ")[0];
        msgID = static_cast<uint32_t>(Utility::ParseStringToNum(idString));
        msg = dbFile->messageHandler->findMsgByID(msgID);

        confirmDialog = QMessageBox::question(this, "Really?", "Are you sure you want to delete\nthis message and its "
                                  + QString::number(msg->sigHandler->getCount()) + " signals?", QMessageBox::Yes|QMessageBox::No);
        if (confirmDialog == QMessageBox::Yes)
        {
            QTreeWidgetItem *currItem = ui->treedb->currentItem();
            if (itemToMessage.contains(currItem))
            {
                db_MESSAGE *msg = itemToMessage.value(currItem);
                deleteMessage(msg);
            }
            else
            {
                qDebug() << "Could not find the message in the map. That should not happen.";
            }
        }
        break;
    case dbItemTypes::SIG: //no cascade, just this one signal.
        confirmDialog = QMessageBox::question(this, "Really?", "Are you sure you want to delete this signal?",
                                  QMessageBox::Yes|QMessageBox::No);
        if (confirmDialog == QMessageBox::Yes)
        {
            QTreeWidgetItem *currItem = ui->treedb->currentItem();
            if (itemToSignal.contains(currItem))
            {
                DB_SIGNAL *sig = itemToSignal.value(currItem);
                deleteSignal(sig);
            }
            else
            {
                qDebug() << "Could not find the signal in the map. That should not happen.";
            }
        }
        break;
    }
}

//these don't ask for permission. You call it, it disappears forever.
void dbMainEditor::deleteNode(db_NODE *node)
{
    qDebug() << "Going through with it you mass deleter!";
    if (!nodeToItem.contains(node)) return;
    QTreeWidgetItem *currItem = nodeToItem[node];
    //don't actually store which messages are associated to which nodes so just iterate through the messages list and whack the ones
    //that claim to be associated to this node.
    int numItems = dbFile->messageHandler->getCount();
    for (int i = numItems - 1; i > -1; i--)
    {
        db_MESSAGE *msg = dbFile->messageHandler->findMsgByIdx(i);
        if (msg->sender == node) deleteMessage(dbFile->messageHandler->findMsgByIdx(i));
        //also, each signal has a receiver field that references the nodes. It was probably stupid to store
        //pointers to node structures in signals but that's how it is currently. Need to iterate over all
        //signals in all messages and set the receiver field to Vector__XXX if the old receiver node was this one.
        else //still check for signals with receiver set to this node
        {
            db_NODE *unset_node = dbFile->findNodeByName("Vector__XXX");
            int numSigs = msg->sigHandler->getCount();
            for (int j = numSigs - 1; j > -1; j--)
            {
                DB_SIGNAL *sig = msg->sigHandler->findSignalByIdx(j);
                if (sig->receiver == node) sig->receiver = unset_node;
            }
        }
    }

    nodeToItem.remove(node);
    itemToNode.remove(currItem);
    ui->treedb->removeItemWidget(currItem, 0);
    delete currItem;

    for (int j = 0; j < dbFile->db_nodes.count(); j++)
    {
        if (dbFile->db_nodes.at(j).name == node->name)
        {
            dbFile->db_nodes.removeAt(j);
            break;
        }
    }

    dbFile->setDirtyFlag();
}

void dbMainEditor::deleteMessage(db_MESSAGE *msg)
{
    qDebug() << "Deleting the message and all signals. Bye bye!";
    if (!messageToItem.contains(msg)) return;
    QTreeWidgetItem *currItem = messageToItem[msg];

    int numItems = msg->sigHandler->getCount();
    for (int i = numItems - 1; i > -1; i--)
    {
        deleteSignal(msg->sigHandler->findSignalByIdx(i));
    }

    dbFile->messageHandler->removeMessage(msg);

    itemToMessage.remove(currItem);
    messageToItem.remove(msg);
    ui->treedb->removeItemWidget(currItem, 0);
    delete currItem;
    dbFile->setDirtyFlag();
}

void dbMainEditor::deleteSignal(DB_SIGNAL *sig)
{
    qDebug() << "Signal about to vanish.";
    if (!signalToItem.contains(sig)) return;
    QTreeWidgetItem *currItem = signalToItem[sig];
    sig->parentMessage->sigHandler->removeSignal(sig);

    itemToSignal.remove(currItem);
    signalToItem.remove(sig);
    ui->treedb->removeItemWidget(currItem, 0);
    //delete currItem; //already removed by above remove call
    dbFile->setDirtyFlag();
}
