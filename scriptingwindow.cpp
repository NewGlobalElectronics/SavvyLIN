#include "scriptingwindow.h"
#include "ui_scriptingwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QSettings>
#if QT_VERSION >= QT_VERSION_CHECK( 5, 10, 0 )
#include <QtCore/QRandomGenerator>
#endif

#include "connections/canconmanager.h"
#include "helpwindow.h"

ScriptingWindowLIN::ScriptingWindowLIN(const QVector<LINFrame> *frames, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScriptingWindowLIN)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);

    editor = new JSEdit();
    editor->setFrameShape(JSEdit::NoFrame);
    editor->setWordWrapMode(QTextOption::NoWrap);
    editor->setEnabled(false);
    editor->setFont(QFont("Monospace", 12));
    editor->show();

    //Show whitespaces
    QTextOption option = editor->document()->defaultTextOption();
    option.setFlags(QTextOption::ShowTabsAndSpaces);
    editor->document()->setDefaultTextOption(option);

    ui->verticalLayout->insertWidget(2,editor, 10);

    readSettings();

    modelFrames = frames;

    connect(ui->btnLoadScript, &QAbstractButton::pressed, this, &ScriptingWindowLIN::loadNewScript);
    connect(ui->btnNewScript, &QAbstractButton::pressed, this, &ScriptingWindowLIN::createNewScript);
    connect(ui->btnRecompile, &QAbstractButton::pressed, this, &ScriptingWindowLIN::recompileScript);
    connect(ui->btnRemoveScript, &QAbstractButton::pressed, this, &ScriptingWindowLIN::deleteCurrentScript);
    connect(ui->btnRevertScript, &QAbstractButton::pressed, this, &ScriptingWindowLIN::revertScript);
    connect(ui->btnReloadScript, &QAbstractButton::pressed, this, &ScriptingWindowLIN::reloadScript);
    connect(ui->btnSaveScript, &QAbstractButton::pressed, this, &ScriptingWindowLIN::saveScript);
    connect(ui->btnSaveAsScript, &QAbstractButton::pressed, this, &ScriptingWindowLIN::saveAsScript);
    connect(ui->btnClearLog, &QAbstractButton::pressed, this, &ScriptingWindowLIN::clickedLogClear);
    connect(ui->btnSaveLog, &QAbstractButton::pressed, this, &ScriptingWindowLIN::saveLog);
    connect(ui->listLoadedScripts, &QListWidget::currentRowChanged, this, &ScriptingWindowLIN::changeCurrentScript);
    connect(ui->tableVariables, SIGNAL(cellChanged(int,int)), this, SLOT(updatedValue(int, int)));

    connect(CANConManager::getInstance(), &CANConManager::framesReceived, this, &ScriptingWindowLIN::newFrames);

    connect(&valuesTimer, SIGNAL(timeout()), this, SLOT(valuesTimerElapsed()));

    currentScript = nullptr;

    elapsedTime.start();
    valuesTimer.start(1000);

    ui->tableVariables->insertColumn(0);
    ui->tableVariables->insertColumn(1);
}

ScriptingWindowLIN::~ScriptingWindowLIN()
{
    delete editor;
    delete ui;
}


void ScriptingWindowLIN::newFrames(const CANConnection* pConn, const QVector<LINFrame>& pFrames)
{
    /*FIXME: name of the probe and bus should be checked */
    Q_UNUSED(pConn);
    Q_UNUSED(pFrames);

    /*for (int j = 0; j < scripts.length(); j++)
    {
        foreach(const LINFrame& frame, pFrames)
        {
            //scripts[j]->gotFrame(frame);
        }
    }*/
}

void ScriptingWindowLIN::updatedValue(int row, int col)
{
    QTableWidgetItem *nameItem = ui->tableVariables->item(row, 0);
    QTableWidgetItem *valItem = ui->tableVariables->item(row, 1);

    if (!valItem) return;
    if (col == 0) return;
    if (!valItem->isSelected()) return; // don't record updates not from a user edited cell

    if (nameItem && valItem)
    {
        QString name = nameItem->text();
        QString val = valItem->text();
        emit updatedParameter(name, val);
    }
}

void ScriptingWindowLIN::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    installEventFilter(this);
}

void ScriptingWindowLIN::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    removeEventFilter(this);
    writeSettings();
}

bool ScriptingWindowLIN::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key())
        {
        case Qt::Key_F1:
            HelpWindow::getRef()->showHelp("scriptingwindow.md");
            break;
        }
        return true;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
    return false;
}

void ScriptingWindowLIN::readSettings()
{
    QSettings settings;
    if (settings.value("Main/SaveRestorePositions", false).toBool())
    {
        resize(settings.value("ScriptingWindowLIN/WindowSize", QSize(860, 650)).toSize());
        move(Utility::constrainedWindowPos(settings.value("ScriptingWindowLIN/WindowPos", QPoint(100, 100)).toPoint()));
    }
}

void ScriptingWindowLIN::writeSettings()
{
    QSettings settings;

    if (settings.value("Main/SaveRestorePositions", false).toBool())
    {
        settings.setValue("ScriptingWindowLIN/WindowSize", size());
        settings.setValue("ScriptingWindowLIN/WindowPos", pos());
    }
}

void ScriptingWindowLIN::changeCurrentScript()
{
    ui->tableVariables->clear();
    for (int i = 0; i < ui->tableVariables->rowCount(); i++) ui->tableVariables->removeRow(0);
    QStringList labels;
    labels.append("Parameter");
    labels.append("Value");
    ui->tableVariables->setHorizontalHeaderLabels(labels);

    int sel = ui->listLoadedScripts->currentRow();
    if (sel < 0) return;

    if (currentScript) {
        currentScript->scriptText = editor->toPlainText();
        disconnect(this, SIGNAL(updateValueTable(QTableWidget*)), currentScript, SLOT(updateValuesTable(QTableWidget*)));
        disconnect(this, SIGNAL(updatedParameter(QString,QString)), currentScript, SLOT(updateParameter(QString,QString)));
    }

    ScriptContainer *container = scripts.at(sel);
    currentScript = container;
    editor->setPlainText(container->scriptText);
    editor->setEnabled(true);
    connect(this, SIGNAL(updateValueTable(QTableWidget*)), currentScript, SLOT(updateValuesTable(QTableWidget*)));
    connect(this, SIGNAL(updatedParameter(QString,QString)), currentScript, SLOT(updateParameter(QString,QString)));
}

void ScriptingWindowLIN::valuesTimerElapsed()
{
    if (currentScript)
    {
        emit updateValueTable(ui->tableVariables);
    }
}

void ScriptingWindowLIN::loadNewScript()
{
    QString filename;
    QFileDialog dialog;
    QSettings settings;
    ScriptContainer *container;

    QStringList filters;
    filters.append(QString(tr("Javascript File (*.js)")));

    dialog.setDirectory(settings.value("ScriptingWindowLIN/LoadSaveDirectory", dialog.directory().path()).toString());
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilters(filters);
    dialog.setViewMode(QFileDialog::Detail);

    if (dialog.exec() == QDialog::Accepted)
    {
        filename = dialog.selectedFiles()[0];

        if (dialog.selectedNameFilter() == filters[0])
        {
            QFile scriptFile(filename);

            if (scriptFile.open(QIODevice::ReadOnly))
            {
                QString contents = scriptFile.readAll();
                scriptFile.close();
                QStringList fileList = filename.split('/');
                QString justFileName = fileList[fileList.length() - 1];
                ui->listLoadedScripts->addItem(justFileName);

                container = new ScriptContainer();
                container->fileName = justFileName;
                container->filePath = filename;
                container->scriptText = contents;
                container->setScriptWindow(this);
                container->compileScript();
                scripts.append(container);

                ui->listLoadedScripts->setCurrentRow(ui->listLoadedScripts->count() - 1);
                changeCurrentScript();
                settings.setValue("ScriptingWindowLIN/LoadSaveDirectory", dialog.directory().path());
            }
        }
    }
}

void ScriptingWindowLIN::createNewScript()
{
    ScriptContainer *container;

    container = new ScriptContainer();

    QString randomPart;
#if QT_VERSION < QT_VERSION_CHECK( 5, 10, 0 )
    randomPart = QString::number((qrand() % 10000));
#else
    randomPart = QString::number((QRandomGenerator::global()->bounded(10000)));
#endif
    container->fileName = "UNNAMED_" + randomPart + ".js";
    container->filePath = QString();
    container->scriptText = QString();
    container->setScriptWindow(this);
    scripts.append(container);
    ui->listLoadedScripts->addItem(container->fileName);

    ui->listLoadedScripts->setCurrentRow(ui->listLoadedScripts->count() - 1);
    changeCurrentScript();
}

void ScriptingWindowLIN::deleteCurrentScript()
{
    ScriptContainer* thisScript;

    int sel = ui->listLoadedScripts->currentRow();
    if (sel < 0) return;

    QMessageBox msgBox;
    msgBox.setText("Really remove script?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    switch (ret)
    {
    case QMessageBox::Yes:
        ui->listLoadedScripts->takeItem(sel);
        thisScript = scripts.at(sel);
        scripts.removeAt(sel);
        delete thisScript;  //causes a seg fault. Seems to be due to currently running javascript code. No idea how to stop code from running
        thisScript = nullptr;
        currentScript = nullptr;

        if (ui->listLoadedScripts->count() > 0)
        {
            ui->listLoadedScripts->setCurrentRow(0);
            changeCurrentScript();
        }
        else
        {
            editor->setPlainText("");
            editor->setEnabled(false);
        }
        break;
    case QMessageBox::No:
        break;
    default:
        // should never be reached
        break;
    }
}

void ScriptingWindowLIN::refreshSourceWindow()
{
    editor->setPlainText(currentScript->scriptText);
}

void ScriptingWindowLIN::saveScript()
{
    QFile *outFile = new QFile(currentScript->filePath);

    if (!outFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        delete outFile;
        return;
    }
    outFile->write(editor->toPlainText().toUtf8());
    currentScript->scriptText = editor->toPlainText();
    outFile->close();
    delete outFile;
}

void ScriptingWindowLIN::saveAsScript()
{
    QString filename;
    QFileDialog dialog(this);
    QSettings settings;

    QStringList filters;
    filters.append(QString(tr("Javascript File (*.js)")));

    dialog.setDirectory(settings.value("ScriptingWindowLIN/LoadSaveDirectory", dialog.directory().path()).toString());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilters(filters);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setAcceptMode(QFileDialog::AcceptSave);

    if (dialog.exec() == QDialog::Accepted)
    {
        filename = dialog.selectedFiles()[0];
        if (!filename.contains('.')) filename += ".js";
        if (dialog.selectedNameFilter() == filters[0])
        {
            QFile *outFile = new QFile(filename);

            if (!outFile->open(QIODevice::WriteOnly | QIODevice::Text))
            {
                delete outFile;
                return;
            }
            outFile->write(editor->toPlainText().toUtf8());
            outFile->close();
            delete outFile;
            settings.setValue("ScriptingWindowLIN/LoadSaveDirectory", dialog.directory().path());
        }
    }
}

void ScriptingWindowLIN::revertScript()
{
    QMessageBox msgBox;
    msgBox.setText("Are you sure you'd like to revert?");
    msgBox.setInformativeText("Really do it?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    switch (ret)
    {
    case QMessageBox::Yes:
        //just grab the last stored copy of the script (last compiled version) and replace current text with that text
        editor->setPlainText(currentScript->scriptText);
        break;
    case QMessageBox::No:
        break;
    default:
        // should never be reached
        break;
    }
}

void ScriptingWindowLIN::reloadScript()
{
    QMessageBox msgBox;
    msgBox.setText("Are you sure you'd like to reload from disk?");
    msgBox.setInformativeText("Really do it?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    switch (ret)
    {
        case QMessageBox::Yes: 
        {
            // get the latest version from disk and set in the editor/state
            QFile scriptFile(currentScript->filePath);
            if (scriptFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QString contents = scriptFile.readAll();
                scriptFile.close();
                editor->setPlainText(contents);
                currentScript->scriptText = contents;
                currentScript->compileScript();
            }
            break;
        }
        case QMessageBox::No:
            break;
        default:
            // should never be reached
            break;
    }
}

void ScriptingWindowLIN::recompileScript()
{
    if (currentScript)
    {
        currentScript->scriptText = editor->toPlainText();
        currentScript->compileScript();
    }
}

void ScriptingWindowLIN::clickedLogClear()
{
    ui->listLog->clear();
    elapsedTime.start();
}

void ScriptingWindowLIN::saveLog()
{
    QString filename;
    QFileDialog dialog(this);
    QSettings settings;

    QStringList filters;
    filters.append(QString(tr("Log File (*.log)")));

    dialog.setDirectory(settings.value("ScriptingWindowLIN/LoadSaveDirectory", dialog.directory().path()).toString());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilters(filters);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setAcceptMode(QFileDialog::AcceptSave);

    if (dialog.exec() == QDialog::Accepted)
    {
        filename = dialog.selectedFiles()[0];
        if (!filename.contains('.')) filename += ".log";
        if (dialog.selectedNameFilter() == filters[0])
        {
            QFile *outFile = new QFile(filename);

            if (!outFile->open(QIODevice::WriteOnly | QIODevice::Text))
            {
                delete outFile;
                return;
            }
            int c = ui->listLog ->count();
            for (int row = 0; row < c; row++) {
                outFile->write(ui->listLog->item(row)->data(Qt::DisplayRole).toString().toUtf8() + "\n");
            }
            outFile->close();
            delete outFile;
            settings.setValue("ScriptingWindowLIN/LoadSaveDirectory", dialog.directory().path());
        }
    }
}

void ScriptingWindowLIN::log(QString text)
{
    ScriptContainer *cont = qobject_cast<ScriptContainer*>(sender());
    if (cont != nullptr)
       ui->listLog->addItem(QString::number(elapsedTime.elapsed()) + "(" + cont->fileName + "): " + text);
    else
       ui->listLog->addItem(QString::number(elapsedTime.elapsed()) + ": " + text);

    if (ui->cbAutoScroll->isChecked())
    {
        ui->listLog->scrollToBottom();
    }
}
