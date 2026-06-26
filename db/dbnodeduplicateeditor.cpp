#include "dbnodeduplicateeditor.h"
#include "ui_dbnodeduplicateeditor.h"

#include <QSettings>
#include <QKeyEvent>
#include <QColorDialog>
#include <QMessageBox>
#include "helpwindow.h"
#include "utility.h"

dbNodeDuplicateEditor::dbNodeDuplicateEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dbNodeDuplicateEditor)
{
    ui->setupUi(this);

    readSettings();

    dbHandler = dbHandler::getReference();
    dbNode = nullptr;

    connect(ui->btnDuplicate, &QPushButton::pressed,
        [=]()
        {
            if (dbNode == nullptr) return;
            if (lowestMsgId > 0x1FFFFFFFul) return;

            uint newBase = Utility::ParseStringToNum(ui->lineNewBaseId->text());

            if(newBase <= 0 || newBase > 0x1FFFFFFFul)
            {
                QMessageBox::question(this, "Invalid Address", "The new address is outside of the valid range.",
                                                  QMessageBox::Ok);
                return;
            }

            if(newBase == lowestMsgId)
            {
                QMessageBox::question(this, "Invalid Address", "The new address is the same as the original.",
                                                  QMessageBox::Ok);
                return;
            }

            int32_t rebaseDiff = newBase - lowestMsgId;

            QList<db_MESSAGE*> messagesForNode = dbFile->messageHandler->findMsgsByNode(dbNode);
            if(messagesForNode.count() == 0)
            {
                QMessageBox::question(this, "No Messages", "The node has no messages to duplicate.",
                                                  QMessageBox::Ok);
                return;
            }

            if(ui->lineNodeName->text().isEmpty())
            {
                QMessageBox::question(this, "No Name", "The new node needs a name before it can be created.",
                                                  QMessageBox::Ok);
                return;
            }

            QString newNodeName = ui->lineNodeName->text();
            emit createNode(newNodeName);

            db_NODE *nodePtr = dbFile->findNodeByName(newNodeName);

            if(nodePtr == nullptr)
            {
                QMessageBox::question(this, "Node Invalid", "There was an problem identifying the selected node.",
                                                  QMessageBox::Ok);
                return;
            }

            for (int i = 0; i < messagesForNode.count(); i++)
            {
                int32_t newMsgId = messagesForNode[i]->ID + rebaseDiff;

                if(newMsgId < 0 || newMsgId > 0x1FFFFFFFl)
                {
                    QMessageBox::question(this, "Invalid Address Range", "The new starting address would cause a message to be outside of the valid address range.",
                                                      QMessageBox::Ok);
                    return;
                }
            }

            for (int i = 0; i < messagesForNode.count(); i++)
            {
                int32_t newMsgId = messagesForNode[i]->ID + rebaseDiff;
                emit cloneMessageToNode(nodePtr, messagesForNode[i], newMsgId);
            }

            dbFile->setDirtyFlag();
            emit nodeAdded();

            this->close();

        });

    connect(ui->btnCancel, &QPushButton::pressed,
        [=]()
        {
            this->close();
        });

    installEventFilter(this);
}

dbNodeDuplicateEditor::~dbNodeDuplicateEditor()
{
    removeEventFilter(this);
    delete ui;
}

void dbNodeDuplicateEditor::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    writeSettings();
}

bool dbNodeDuplicateEditor::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key())
        {
        case Qt::Key_F1:
            HelpWindow::getRef()->showHelp("nodeeditor.md");
            break;
        }
        return true;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
    return false;
}

void dbNodeDuplicateEditor::setFileIdx(int idx)
{
    if (idx < 0 || idx > dbHandler->getFileCount() - 1) return;
    dbFile = dbHandler->getFileByIdx(idx);
}

void dbNodeDuplicateEditor::readSettings()
{
    QSettings settings;
    if (settings.value("Main/SaveRestorePositions", false).toBool())
    {
        resize(settings.value("dbNodeDuplicateEditor/WindowSize", QSize(312, 128)).toSize());
        move(Utility::constrainedWindowPos(settings.value("dbNodeDuplicateEditor/WindowPos", QPoint(100, 100)).toPoint()));
    }
}

void dbNodeDuplicateEditor::writeSettings()
{
    QSettings settings;

    if (settings.value("Main/SaveRestorePositions", false).toBool())
    {
        settings.setValue("dbNodeDuplicateEditor/WindowSize", size());
        settings.setValue("dbNodeDuplicateEditor/WindowPos", pos());
    }
}


void dbNodeDuplicateEditor::setNodeRef(db_NODE *node)
{
    dbNode = node;
}

void dbNodeDuplicateEditor::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);

    refreshView();
}

bool dbNodeDuplicateEditor::refreshView()
{
    ui->lineNewBaseId->setText("");

    if(dbNode)
    {
        QList<db_MESSAGE*> messagesForNode = dbFile->messageHandler->findMsgsByNode(dbNode);
        lowestMsgId = 0xFFFFFFFF;

        if(messagesForNode.count() == 0)
        {
            return false;
        }

        for (int i=0; i<messagesForNode.count(); i++)
        {
            if(messagesForNode[i]->ID < lowestMsgId)
                lowestMsgId = messagesForNode[i]->ID;
        }

        ui->lineOriginalBaseId->setText(Utility::formatCANID(lowestMsgId & 0x1FFFFFFFul));
        ui->lineNodeName->setText(dbNode->name + QString("_Copy"));

        return true;
    }

    return false;
}
