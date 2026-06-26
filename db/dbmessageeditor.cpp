#include "dbmessageeditor.h"
#include "ui_dbmessageeditor.h"

#include <QSettings>
#include <QKeyEvent>
#include <QColorDialog>
#include "helpwindow.h"
#include "utility.h"

dbMessageEditor::dbMessageEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dbMessageEditor)
{
    ui->setupUi(this);

    readSettings();

    dbHandler = dbHandler::getReference();
    dbMessage = nullptr;
    suppressEditCallbacks = false;

    connect(ui->lineComment, &QLineEdit::editingFinished,
        [=]()
        {
            if (dbMessage == nullptr) return;
            if (suppressEditCallbacks) return;
            if (dbMessage->comment != ui->lineComment->text()) dbFile->setDirtyFlag();
            dbMessage->comment = ui->lineComment->text();
            emit updatedTreeInfo(dbMessage);
        });

    connect(ui->lineFrameID, &QLineEdit::editingFinished,
        [=]()
        {
            if (dbMessage == nullptr) return;
            if (suppressEditCallbacks) return;
            if ((dbMessage->ID & 0x1FFFFFFFul) != Utility::ParseStringToNum(ui->lineFrameID->text())) dbFile->setDirtyFlag();
            dbMessage->ID = Utility::ParseStringToNum(ui->lineFrameID->text());
            emit updatedTreeInfo(dbMessage);
        });

    connect(ui->lineMsgName, &QLineEdit::editingFinished,
        [=]()
        {
            if (dbMessage == nullptr) return;
            if (suppressEditCallbacks) return;
            if (dbMessage->name != ui->lineMsgName->text().simplified().replace(' ', '_')) dbFile->setDirtyFlag();
            dbMessage->name = ui->lineMsgName->text().simplified().replace(' ', '_');
            emit updatedTreeInfo(dbMessage);
        });

    connect(ui->lineFrameLen, &QLineEdit::editingFinished,
        [=]()
        {
            if (dbMessage == nullptr) return;
            if (suppressEditCallbacks) return;
            if (dbMessage->len != Utility::ParseStringToNum(ui->lineFrameLen->text())) dbFile->setDirtyFlag();
            dbMessage->len = Utility::ParseStringToNum(ui->lineFrameLen->text());
        });

    connect(ui->comboSender, &QComboBox::currentTextChanged,
            [=](const QString newText)
            {
                if (dbMessage == nullptr) return;
                if (suppressEditCallbacks) return;
                db_NODE *node = dbFile->findNodeByName(newText);
                if (!node) return;
                if (node != dbMessage->sender) dbFile->setDirtyFlag();
                dbMessage->sender = node;
                emit updatedTreeInfo(dbMessage);
            });

    connect(ui->comboSender->lineEdit(), &QLineEdit::editingFinished,
            [=]()
            {
                if (dbMessage == nullptr) return;
                if (suppressEditCallbacks) return;
                QString newText = ui->comboSender->currentText();
                db_NODE *node = dbFile->findNodeByName(newText);
                if (!node)
                {
                    db_NODE newNode;
                    newNode.name = newText;
                    dbFile->db_nodes.append(newNode);
                    node = dbFile->findNodeByName(newText);
                    ui->comboSender->addItem(newText);
                }
                if (node != dbMessage->sender) dbFile->setDirtyFlag();
                dbMessage->sender = node;
                emit updatedTreeInfo(dbMessage);
            });

    connect(ui->btnTextColor, &QAbstractButton::clicked,
        [=]()
        {
            if (suppressEditCallbacks) return;
            QColor newColor = QColorDialog::getColor(dbMessage->fgColor);
            if (dbMessage->fgColor != newColor) dbFile->setDirtyFlag();
            dbMessage->fgColor = newColor;
            db_ATTRIBUTE_VALUE *val = dbMessage->findAttrValByName("GenMsgForegroundColor");
            if (val)
            {
                val->value = newColor.name();
            }
            else
            {
                db_ATTRIBUTE_VALUE newVal;
                newVal.attrName = "GenMsgForegroundColor";
                newVal.value = newColor.name();
                dbMessage->attributes.append(newVal);
            }
            generateSampleText();
        });

    connect(ui->btnBackgroundColor, &QAbstractButton::clicked,
        [=]()
        {
            if (suppressEditCallbacks) return;
            QColor newColor = QColorDialog::getColor(dbMessage->bgColor);
            if (dbMessage->bgColor != newColor) dbFile->setDirtyFlag();
            dbMessage->bgColor = newColor;
            db_ATTRIBUTE_VALUE *val = dbMessage->findAttrValByName("GenMsgBackgroundColor");
            if (val)
            {
                val->value = newColor.name();
            }
            else
            {
                db_ATTRIBUTE_VALUE newVal;
                newVal.attrName = "GenMsgBackgroundColor";
                newVal.value = newColor.name();
                dbMessage->attributes.append(newVal);
            }
            generateSampleText();
        });

    installEventFilter(this);
}

dbMessageEditor::~dbMessageEditor()
{
    removeEventFilter(this);
    delete ui;
}

void dbMessageEditor::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    writeSettings();
}

bool dbMessageEditor::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key())
        {
        case Qt::Key_F1:
            HelpWindow::getRef()->showHelp("messageeditor.md");
            break;
        }
        return true;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
    return false;
}

void dbMessageEditor::setFileIdx(int idx)
{
    if (idx < 0 || idx > dbHandler->getFileCount() - 1) return;
    dbFile = dbHandler->getFileByIdx(idx);

    suppressEditCallbacks = true;
    ui->comboSender->clear();
    for (int x = 0; x < dbFile->db_nodes.count(); x++)
    {
        ui->comboSender->addItem(dbFile->db_nodes[x].name);
    }
    suppressEditCallbacks = false;
}

void dbMessageEditor::readSettings()
{
    QSettings settings;
    if (settings.value("Main/SaveRestorePositions", false).toBool())
    {
        resize(settings.value("dbMessageEditor/WindowSize", QSize(340, 400)).toSize());
        move(Utility::constrainedWindowPos(settings.value("dbMessageEditor/WindowPos", QPoint(100, 100)).toPoint()));
    }
}

void dbMessageEditor::writeSettings()
{
    QSettings settings;

    if (settings.value("Main/SaveRestorePositions", false).toBool())
    {
        settings.setValue("dbMessageEditor/WindowSize", size());
        settings.setValue("dbMessageEditor/WindowPos", pos());
    }
}


void dbMessageEditor::setMessageRef(db_MESSAGE *msg)
{
    dbMessage = msg;
}

void dbMessageEditor::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);

    refreshView();
}

void dbMessageEditor::refreshView()
{
    suppressEditCallbacks = true;

    ui->lineComment->setText(dbMessage->comment);
    ui->lineFrameID->setText(Utility::formatCANID(dbMessage->ID & 0x1FFFFFFFul));
    ui->lineMsgName->setText(dbMessage->name);
    ui->lineFrameLen->setText(QString::number(dbMessage->len));    
    for (int i = 0; i < ui->comboSender->count(); i++)
    {
        if (ui->comboSender->itemText(i) == dbMessage->sender->name)
        {
            ui->comboSender->setCurrentIndex(i);
            break;
        }
    }

    suppressEditCallbacks = false;

    generateSampleText();
}

void dbMessageEditor::generateSampleText()
{
    QBrush fg, bg;

    if (dbMessage->fgColor.isValid()) fg = QBrush(dbMessage->fgColor);
    else fg = QBrush(QColor(dbFile->findAttributeByName("GenMsgForegroundColor")->defaultValue.toString()));
    if (dbMessage->bgColor.isValid()) bg = QBrush(dbMessage->bgColor);
    else bg = QBrush(QColor(dbFile->findAttributeByName("GenMsgBackgroundColor")->defaultValue.toString()));

    ui->listSample->clear();
    QListWidgetItem *item = new QListWidgetItem("Test String");
    item->setForeground(fg);
    item->setBackground(bg);
    ui->listSample->addItem(item);
    item = new QListWidgetItem("0x20F TestMsg");
    item->setForeground(fg);
    item->setBackground(bg);
    ui->listSample->addItem(item);
    item = new QListWidgetItem("20 FF 10 A1 BB CC 4D");
    item->setForeground(fg);
    item->setBackground(bg);
    ui->listSample->addItem(item);
    item = new QListWidgetItem("1024.3434");
    item->setForeground(fg);
    item->setBackground(bg);
    ui->listSample->addItem(item);
}
