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

dbMainEditor::dbMainEditor( const QVector<LINFrame> *frames, QWidget *parent) :
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

    // connections
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
    // multiplex icons not used in LIN
    multiplexedSignalIcon = QIcon(":/icons/images/multiplexed-signal.png");
    multiplexorSignalIcon = QIcon(":/icons/images/multiplexor-signal.png");

    ui->btnDelete->setIconSize(QSize(32, 32));
    ui->btnNewNode->setIconSize(QSize(32, 32));
    ui->btnNewMessage->setIconSize(QSize(32, 32));
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
    delete nodeRebaseEditor;
    delete nodeDuplicateEditor;
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
    }
    return QObject::eventFilter(obj, event);
}

void dbMainEditor::setFileIdx(int idx)
{
    if (idx < 0 || idx >= dbHandler->getFileCount()) return;
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

void dbMainEditor::handleSearch()
{
    searchItems = ui->treedb->findItems(ui->lineSearch->text(), Qt::MatchContains | Qt::MatchRecursive);
    qDebug() << "Search returned" << searchItems.count() << "items.";
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
    case dbItemTypes::NODE:
        ui->btnNewNode->setEnabled(true);
        ui->btnNewMessage->setEnabled(true);
        ui->btnNewSignal->setEnabled(false);
        ui->btnDelete->setEnabled(true);
        break;
    case dbItemTypes::MESG:
        ui->btnNewNode->setEnabled(true);
        ui->btnNewMessage->setEnabled(true);
        ui->btnNewSignal->setEnabled(true);
        ui->btnDelete->setEnabled(true);
        break;
    case dbItemTypes::SIG:
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

void dbMainEditor::onTreeDoubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index)
    QTreeWidgetItem* firstCol = ui->treedb->currentItem();
    db_MESSAGE *msg;
    DB_SIGNAL *sig;
    db_NODE *node;
    uint32_t msgID;
    QString idString;

    qDebug() << firstCol->data(0, Qt::UserRole) << " - " << firstCol->text(0);

    switch (firstCol->data(0, Qt::UserRole).toInt())
    {
    case dbItemTypes::NODE:
        idString = firstCol->text(0).split(" - ")[0];  // node name may have comment after " - "
        node = dbFile->findNodeByName(idString);
        nodeEditor->setFileIdx(fileIdx);
        nodeEditor->setNodeRef(node);
        nodeEditor->refreshView();
        nodeEditor->show();
        break;
    case dbItemTypes::MESG:
        idString = firstCol->text(0).split(" ")[0];
        msgID = static_cast<uint32_t>(Utility::ParseStringToNum(idString));
        msg = dbFile->messageHandler->findMsgByID(msgID);
        msgEditor->setMessageRef(msg);
        msgEditor->setFileIdx(fileIdx);
        msgEditor->refreshView();
        msgEditor->show();
        break;
    case dbItemTypes::SIG:
        msgID = getParentMessageID(firstCol);
        msg = dbFile->messageHandler->findMsgByID(msgID);
        QString nameString = firstCol->text(0);
        // remove possible "[xxi yy]" suffix
        if (nameString.contains("["))
            nameString = nameString.split("[")[0].trimmed();
        // signal name is first word (before any space)
        nameString = nameString.split(" ")[0];
        sig = msg->sigHandler->findSignalByName(nameString);
        if (sig)
        {
            sigEditor->setSignalRef(sig);
            sigEditor->setMessageRef(msg);
            sigEditor->setFileIdx(fileIdx);
            sigEditor->refreshView();
            sigEditor->show();
        }
        break;
    }
}

void dbMainEditor::onTreeContextMenu(const QPoint & pos)
{
    QTreeWidgetItem* firstCol = ui->treedb->currentItem();
    if (!firstCol) return;

    switch (firstCol->data(0, Qt::UserRole).toInt())
    {
    case dbItemTypes::NODE:
    {
        QAction *actionRebase = new QAction(QIcon(":/Resource/warning32.ico"), tr("Rebase all messages"), this);
        actionRebase->setStatusTip(tr("Rebase all messages in node"));
        connect(actionRebase, SIGNAL(triggered()), this, SLOT(onRebaseMessages()));

        QAction *actionDupe = new QAction(QIcon(":/Resource/warning32.ico"), tr("Duplicate node"), this);
        actionDupe->setStatusTip(tr("Duplicate node and messages"));
        connect(actionDupe, SIGNAL(triggered()), this, SLOT(onDuplicateNode()));

        QMenu menu(this);
        menu.addAction(actionRebase);
        menu.addAction(actionDupe);
        menu.exec(ui->treedb->mapToGlobal(pos));
        break;
    }
    default:
        break;
    }
}

void dbMainEditor::onRebaseMessages()
{
    QTreeWidgetItem* firstCol = ui->treedb->currentItem();
    if (!firstCol) return;
    QString idString = firstCol->text(0).split(" - ")[0];
    db_NODE *node = dbFile->findNodeByName(idString);
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
    if (!firstCol) return;
    QString idString = firstCol->text(0).split(" - ")[0];
    db_NODE *node = dbFile->findNodeByName(idString);
    nodeDuplicateEditor->setFileIdx(fileIdx);
    nodeDuplicateEditor->setNodeRef(node);
    if(nodeDuplicateEditor->refreshView())
    {
        nodeDuplicateEditor->setModal(true);
        nodeDuplicateEditor->show();
    }
}

void dbMainEditor::refreshTree()
{
    ui->treedb->clear();
    nodeToItem.clear();
    messageToItem.clear();
    signalToItem.clear();
    itemToNode.clear();
    itemToMessage.clear();
    itemToSignal.clear();

    // Ensure a default node exists
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
            if (msg->sender == node)
            {
                QTreeWidgetItem *msgItem = new QTreeWidgetItem(nodeItem);
                QString msgInfo = Utility::formatCANID(msg->ID) + " " + msg->name;
                if (msg->comment.count() > 0) msgInfo.append(" - ").append(msg->comment);
                msgItem->setText(0, msgInfo);
                msgItem->setIcon(0, messageIcon);
                msgItem->setData(0, Qt::UserRole, dbItemTypes::MESG);
                messageToItem.insert(msg, msgItem);
                itemToMessage.insert(msgItem, msg);

                // Add all signals directly under the message (no multiplex hierarchy)
                for (int i = 0; i < msg->sigHandler->getCount(); i++)
                {
                    DB_SIGNAL *sig = msg->sigHandler->findSignalByIdx(i);
                    QTreeWidgetItem *sigItem = new QTreeWidgetItem(msgItem);
                    QString sigInfo = createSignalText(sig);
                    sigItem->setText(0, sigInfo);
                    sigItem->setIcon(0, signalIcon);
                    sigItem->setData(0, Qt::UserRole, dbItemTypes::SIG);
                    signalToItem.insert(sig, sigItem);
                    itemToSignal.insert(sigItem, sig);
                }
            }
        }
        ui->treedb->addTopLevelItem(nodeItem);
    }
    ui->treedb->sortItems(0, Qt::AscendingOrder);
}

QString dbMainEditor::createSignalText(DB_SIGNAL *sig)
{
    QString sigInfo = sig->name;
    if (sig->intelByteOrder)
        sigInfo.append(" [" + QString::number(sig->startBit) + "i " + QString::number(sig->signalSize) + "]");
    else
        sigInfo.append(" [" + QString::number(sig->startBit) + "m " + QString::number(sig->signalSize) + "]");

    if (sig->comment.count() > 0) sigInfo.append(" - ").append(sig->comment);
    return sigInfo;
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
        QTreeWidgetItem *item = messageToItem[msg];
        QString msgInfo = Utility::formatCANID(msg->ID) + " " + msg->name;
        if (msg->comment.count() > 0) msgInfo.append(" - ").append(msg->comment);
        item->setText(0, msgInfo);

        // Check if message's sender node changed
        db_NODE *oldParent = dbFile->findNodeByName(item->parent()->text(0).split(" - ")[0]);
        if (oldParent != msg->sender && oldParent)
        {
            qDebug() << "Changed parent of message. Rehoming it.";
            QTreeWidgetItem *newParent = nullptr;
            if (nodeToItem.contains(msg->sender))
                newParent = nodeToItem[msg->sender];
            else
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

            QTreeWidgetItem *prevParent = nodeToItem[oldParent];
            prevParent->removeChild(item);
            newParent->addChild(item);
            ui->treedb->setCurrentItem(item);
            ui->treedb->sortItems(0, Qt::AscendingOrder);
        }
    }
    else qDebug() << "That message doesn't exist. That's a bug dude.";
}

void dbMainEditor::updatedSignal(DB_SIGNAL *sig)
{
    if (signalToItem.contains(sig))
    {
        QTreeWidgetItem *item = signalToItem[sig];
        QString sigInfo = createSignalText(sig);
        item->setText(0, sigInfo);
    }
    else qDebug() << "That signal doesn't exist. That's a bug dude.";
}

void dbMainEditor::newNode(QString nodeName)
{
    db_NODE node;
    if(nodeName.isEmpty())
        node.name = "Unnamed" + QString::number(randGen.bounded(50000));
    else
        node.name = nodeName;

    dbFile->db_nodes.append(node);
    db_NODE *nodePtr = dbFile->findNodeByName(node.name);
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
    nodeItem = ui->treedb->currentItem();

    db_MESSAGE msg;
    msg.name = source->name;
    msg.ID = newMsgId;
    msg.len = source->len;
    msg.bgColor = source->bgColor;
    msg.fgColor = source->fgColor;
    msg.comment = source->comment;
    msg.sender = node;

    for (int i = 0; i < source->sigHandler->getCount(); i++)
    {
        DB_SIGNAL *sigSource = source->sigHandler->findSignalByIdx(i);
        DB_SIGNAL sig;
        sig.name = sigSource->name;
        sig.bias = sigSource->bias;
        sig.max = sigSource->max;
        sig.min = sigSource->min;
        sig.factor = sigSource->factor;
        sig.intelByteOrder = sigSource->intelByteOrder;
        sig.parentMessage = &msg;
        sig.receiver = node;
        sig.signalSize = sigSource->signalSize;
        sig.startBit = sigSource->startBit;
        sig.valType = sigSource->valType;
        // Copy value descriptions
        sig.valList = sigSource->valList;
        sig.unitName = sigSource->unitName;
        msg.sigHandler->addSignal(sig);
    }

    msg.sigHandler->sort();
    dbFile->messageHandler->addMessage(msg);
    db_MESSAGE *msgPtr = dbFile->messageHandler->findMsgByIdx(dbFile->messageHandler->getCount() - 1);

    QTreeWidgetItem *newMsgItem = new QTreeWidgetItem();
    QString msgInfo = Utility::formatCANID(msg.ID) + " " + msg.name;
    if (msg.comment.count() > 0) msgInfo.append(" - ").append(msg.comment);
    newMsgItem->setText(0, msgInfo);
    newMsgItem->setIcon(0, messageIcon);
    newMsgItem->setData(0, Qt::UserRole, dbItemTypes::MESG);
    messageToItem.insert(msgPtr, newMsgItem);
    itemToMessage.insert(newMsgItem, msgPtr);
    nodeItem->addChild(newMsgItem);
    dbFile->setDirtyFlag();
}

void dbMainEditor::newMessage()
{
    QTreeWidgetItem *nodeItem = ui->treedb->currentItem();
    if (!nodeItem) return;
    int typ = nodeItem->data(0, Qt::UserRole).toInt();

    // Determine the node item (parent for the new message)
    if (typ == dbItemTypes::MESG)
        nodeItem = nodeItem->parent();
    else if (typ == dbItemTypes::SIG)
        nodeItem = nodeItem->parent()->parent();

    QString nodeName = nodeItem->data(0, Qt::DisplayRole).toString().split(" - ")[0];
    db_NODE *node = dbFile->findNodeByName(nodeName);
    if (!node) node = dbFile->findNodeByIdx(0);

    db_MESSAGE msg;
    msg.name = nodeName + "Msg" + QString::number(randGen.bounded(500));
    msg.ID = 0;
    msg.len = 8;
    msg.sender = node;
    msg.bgColor = QApplication::palette().color(QPalette::Base);
    msg.fgColor = QApplication::palette().color(QPalette::WindowText);

    dbFile->messageHandler->addMessage(msg);
    db_MESSAGE *msgPtr = dbFile->messageHandler->findMsgByIdx(dbFile->messageHandler->getCount() - 1);

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
    QTreeWidgetItem *msgItem = ui->treedb->currentItem();
    if (!msgItem) return;
    int typ = msgItem->data(0, Qt::UserRole).toInt();

    // Find the message item
    if (typ == dbItemTypes::SIG)
        msgItem = msgItem->parent();
    if (typ == dbItemTypes::NODE) return; // cannot add signal to node

    QString idString = msgItem->text(0).split(" ")[0];
    uint32_t msgID = static_cast<uint32_t>(Utility::ParseStringToNum(idString));
    db_MESSAGE *msg = dbFile->messageHandler->findMsgByID(msgID);
    if (!msg) return;

    DB_SIGNAL sig;
    sig.name = msg->name + "Sig" + QString::number(randGen.bounded(500));
    sig.parentMessage = msg;
    if (!sig.receiver) sig.receiver = dbFile->findNodeByIdx(0);
    sig.startBit = 0;
    sig.signalSize = 1;
    sig.intelByteOrder = true;
    sig.valType = UNSIGNED_INT;
    sig.factor = 1.0;
    sig.bias = 0.0;
    msg->sigHandler->addSignal(sig);
    DB_SIGNAL *sigPtr = msg->sigHandler->findSignalByIdx(msg->sigHandler->getCount() - 1);

    QTreeWidgetItem *newSigItem = new QTreeWidgetItem();
    QString sigInfo = createSignalText(&sig);
    newSigItem->setText(0, sigInfo);
    newSigItem->setIcon(0, signalIcon);
    newSigItem->setData(0, Qt::UserRole, dbItemTypes::SIG);
    signalToItem.insert(sigPtr, newSigItem);
    itemToSignal.insert(newSigItem, sigPtr);
    msgItem->addChild(newSigItem);
    ui->treedb->setCurrentItem(newSigItem);
    dbFile->setDirtyFlag();
}

void dbMainEditor::deleteCurrentTreeItem()
{
    QTreeWidgetItem *currItem = ui->treedb->currentItem();
    if (!currItem) return;
    int typ = currItem->data(0, Qt::UserRole).toInt();
    QMessageBox::StandardButton confirmDialog;

    switch (typ)
    {
    case dbItemTypes::NODE:
    {
        db_NODE *node = itemToNode.value(currItem, nullptr);
        if (!node) return;
        int numMsg = 0, numSig = 0;
        for (int x = 0; x < dbFile->messageHandler->getCount(); x++)
        {
            if (dbFile->messageHandler->findMsgByIdx(x)->sender == node)
            {
                numMsg++;
                numSig += dbFile->messageHandler->findMsgByIdx(x)->sigHandler->getCount();
            }
        }
        confirmDialog = QMessageBox::question(this, "Really?",
                                              "Are you sure you want to delete this node, its\n" +
                                                  QString::number(numMsg) + " messages, and their " + QString::number(numSig) + " combined signals?",
                                              QMessageBox::Yes | QMessageBox::No);
        if (confirmDialog == QMessageBox::Yes)
            deleteNode(node);
        break;
    }
    case dbItemTypes::MESG:
    {
        db_MESSAGE *msg = itemToMessage.value(currItem, nullptr);
        if (!msg) return;
        confirmDialog = QMessageBox::question(this, "Really?",
                                              "Are you sure you want to delete this message and its " +
                                                  QString::number(msg->sigHandler->getCount()) + " signals?",
                                              QMessageBox::Yes | QMessageBox::No);
        if (confirmDialog == QMessageBox::Yes)
            deleteMessage(msg);
        break;
    }
    case dbItemTypes::SIG:
    {
        DB_SIGNAL *sig = itemToSignal.value(currItem, nullptr);
        if (!sig) return;
        confirmDialog = QMessageBox::question(this, "Really?",
                                              "Are you sure you want to delete this signal?",
                                              QMessageBox::Yes | QMessageBox::No);
        if (confirmDialog == QMessageBox::Yes)
            deleteSignal(sig);
        break;
    }
    }
}

void dbMainEditor::deleteNode(db_NODE *node)
{
    if (!nodeToItem.contains(node)) return;
    QTreeWidgetItem *currItem = nodeToItem[node];

    // Delete messages belonging to this node
    for (int i = dbFile->messageHandler->getCount() - 1; i >= 0; i--)
    {
        db_MESSAGE *msg = dbFile->messageHandler->findMsgByIdx(i);
        if (msg->sender == node)
            deleteMessage(msg);
        else
        {
            // For signals that have this node as receiver, reassign to default node
            db_NODE *defaultNode = dbFile->findNodeByName("Vector__XXX");
            for (int j = msg->sigHandler->getCount() - 1; j >= 0; j--)
            {
                DB_SIGNAL *sig = msg->sigHandler->findSignalByIdx(j);
                if (sig->receiver == node)
                    sig->receiver = defaultNode;
            }
        }
    }

    nodeToItem.remove(node);
    itemToNode.remove(currItem);
    delete currItem;

    for (int j = 0; j < dbFile->db_nodes.count(); j++)
    {
        if (dbFile->db_nodes[j].name == node->name)
        {
            dbFile->db_nodes.removeAt(j);
            break;
        }
    }
    dbFile->setDirtyFlag();
}

void dbMainEditor::deleteMessage(db_MESSAGE *msg)
{
    if (!messageToItem.contains(msg)) return;
    QTreeWidgetItem *currItem = messageToItem[msg];

    // Delete all signals of this message
    for (int i = msg->sigHandler->getCount() - 1; i >= 0; i--)
        deleteSignal(msg->sigHandler->findSignalByIdx(i));

    dbFile->messageHandler->removeMessage(msg);
    itemToMessage.remove(currItem);
    messageToItem.remove(msg);
    delete currItem;
    dbFile->setDirtyFlag();
}

void dbMainEditor::deleteSignal(DB_SIGNAL *sig)
{
    if (!signalToItem.contains(sig)) return;
    QTreeWidgetItem *currItem = signalToItem[sig];
    sig->parentMessage->sigHandler->removeSignal(sig);
    itemToSignal.remove(currItem);
    signalToItem.remove(sig);
    delete currItem;
    dbFile->setDirtyFlag();
}