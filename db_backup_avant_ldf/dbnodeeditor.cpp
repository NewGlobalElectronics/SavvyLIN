#include "dbnodeeditor.h"
#include "ui_dbnodeeditor.h"

#include <QSettings>
#include <QKeyEvent>
#include <QColorDialog>
#include "helpwindow.h"
#include "utility.h"

dbNodeEditor::dbNodeEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dbNodeEditor)
{
    ui->setupUi(this);

    readSettings();

    dbHandler = dbHandler::getReference();
    dbNode = nullptr;

    connect(ui->lineComment, &QLineEdit::editingFinished,
        [=]()
        {
            if (dbNode == nullptr) return;
            if (dbNode->comment != ui->lineComment->text()) dbFile->setDirtyFlag();
            dbNode->comment = ui->lineComment->text();
            emit updatedTreeInfo(dbNode);
        });

    connect(ui->lineMsgName, &QLineEdit::editingFinished,
        [=]()
        {
            if (dbNode == nullptr) return;
            if (dbNode->name != ui->lineMsgName->text()) dbFile->setDirtyFlag();
            dbNode->name = ui->lineMsgName->text();
            emit updatedTreeInfo(dbNode);
        });

    installEventFilter(this);
}

dbNodeEditor::~dbNodeEditor()
{
    removeEventFilter(this);
    delete ui;
}

void dbNodeEditor::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    writeSettings();
}

bool dbNodeEditor::eventFilter(QObject *obj, QEvent *event)
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

void dbNodeEditor::setFileIdx(int idx)
{
    if (idx < 0 || idx > dbHandler->getFileCount() - 1) return;
    dbFile = dbHandler->getFileByIdx(idx);
}

void dbNodeEditor::readSettings()
{
    QSettings settings;
    if (settings.value("Main/SaveRestorePositions", false).toBool())
    {
        resize(settings.value("dbNodeEditor/WindowSize", QSize(312, 128)).toSize());
        move(Utility::constrainedWindowPos(settings.value("dbNodeEditor/WindowPos", QPoint(100, 100)).toPoint()));
    }
}

void dbNodeEditor::writeSettings()
{
    QSettings settings;

    if (settings.value("Main/SaveRestorePositions", false).toBool())
    {
        settings.setValue("dbNodeEditor/WindowSize", size());
        settings.setValue("dbNodeEditor/WindowPos", pos());
    }
}


void dbNodeEditor::setNodeRef(db_NODE *node)
{
    dbNode = node;
}

void dbNodeEditor::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);

    refreshView();
}

void dbNodeEditor::refreshView()
{
    if(dbNode)
    {
        ui->lineComment->setText(dbNode->comment);
        ui->lineMsgName->setText(dbNode->name);
    }

    //generateSampleText();
}

void dbNodeEditor::generateSampleText()
{
    /*
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
    */
}
