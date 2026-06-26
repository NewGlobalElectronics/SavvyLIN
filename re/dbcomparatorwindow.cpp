#include "dbcomparatorwindow.h"
#include "ui_dbcomparatorwindow.h"
#include "helpwindow.h"
#include <QProgressDialog>
#include <QSettings>
#include <qevent.h>

dbComparatorWindow::dbComparatorWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dbComparatorWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);

    connect(ui->btndbFile1, SIGNAL(clicked(bool)), this, SLOT(loadFirstFile()));
    connect(ui->btndbFile2, SIGNAL(clicked(bool)), this, SLOT(loadSecondFile()));
    connect(ui->btnSaveDetails, SIGNAL(clicked(bool)), this, SLOT(saveDetails()));

    ui->lblFirstFile->setText("");
    ui->lblSecondFile->setText("");

    firstdb = nullptr;
    seconddb = nullptr;

    installEventFilter(this);
}

dbComparatorWindow::~dbComparatorWindow()
{
    removeEventFilter(this);
    delete ui;
}

void dbComparatorWindow::showEvent(QShowEvent *)
{
    readSettings();
}

void dbComparatorWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    writeSettings();
}

bool dbComparatorWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key())
        {
        case Qt::Key_F1:
            HelpWindow::getRef()->showHelp("filecomparison.md");
            break;
        }
        return true;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
    return false;
}

void dbComparatorWindow::readSettings()
{
    QSettings settings;
    if (settings.value("Main/SaveRestorePositions", false).toBool())
    {
        resize(settings.value("dbComparator/WindowSize", QSize(720, 631)).toSize());
        move(Utility::constrainedWindowPos(settings.value("dbComparator/WindowPos", QPoint(50, 50)).toPoint()));
    }
}

void dbComparatorWindow::writeSettings()
{
    QSettings settings;

    if (settings.value("Main/SaveRestorePositions", false).toBool())
    {
        settings.setValue("dbComparator/WindowSize", size());
        settings.setValue("dbComparator/WindowPos", pos());
    }
}

QString dbComparatorWindow::loaddb(dbFile **file)
{
    QString filename;
    QFileDialog dialog;
    QSettings settings;

    QStringList filters;
    filters.append(QString(tr("db Files (*.db *.db)")));

    dialog.setDirectory(settings.value("FileIO/LoadSaveDirectory", dialog.directory().path()).toString());
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilters(filters);
    dialog.setViewMode(QFileDialog::Detail);

    if (dialog.exec() == QDialog::Accepted)
    {
        filename = dialog.selectedFiles()[0];

        QProgressDialog progress(qApp->activeWindow());
        progress.setWindowModality(Qt::WindowModal);
        progress.setLabelText("Loading file...");
        progress.setCancelButton(nullptr);
        progress.setRange(0,0);
        progress.setMinimumDuration(0);
        progress.show();

        qApp->processEvents();

        if (dialog.selectedNameFilter() == filters[0])
        {
            if (file[0]) delete file[0];
            file[0] = new dbFile;
            file[0]->loadFile(filename);
            qDebug() << "Loaded the db file into first slot";
        }

        progress.cancel();
        return filename;
    }
    return QString();
}

void dbComparatorWindow::loadFirstFile()
{
    QString filename;
    filename = loaddb(&firstdb);
    ui->lblFirstFile->setText(filename);
    if (firstdb && seconddb) calculateDetails();
}

void dbComparatorWindow::loadSecondFile()
{
    QString filename;
    filename = loaddb(&seconddb);
    ui->lblSecondFile->setText(filename);
    if (firstdb && seconddb) calculateDetails();
}

void dbComparatorWindow::calculateDetails()
{
    QProgressDialog progress(this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setLabelText("Calculating differences");
    progress.setCancelButton(nullptr);
    progress.setRange(0,0);
    progress.setMinimumDuration(0);
    progress.show();

    qApp->processEvents();

    firstdb->sort();
    seconddb->sort();


    ui->treeDetails->clear();

    //Find nodes missing in one of the db files

    QTreeWidgetItem *nodeDiffRoot = new QTreeWidgetItem();
    nodeDiffRoot->setText(0, "Node Differences");
    QTreeWidgetItem *nodesMissingdbFirst = new QTreeWidgetItem();
    nodesMissingdbFirst->setText(0, "Nodes not found in first db");
    QTreeWidgetItem *nodesMissingdbSecond = new QTreeWidgetItem();
    nodesMissingdbSecond->setText(0, "Nodes not found in second db");
    nodeDiffRoot->addChild(nodesMissingdbFirst);
    nodeDiffRoot->addChild(nodesMissingdbSecond);
    ui->treeDetails->addTopLevelItem(nodeDiffRoot);

    for (int i = 0; i < firstdb->db_nodes.count(); i++)
    {
        QString nodeName = firstdb->db_nodes[i].name;
        db_NODE *node = seconddb->findNodeByName(nodeName);
        if (!node)
        {
            QTreeWidgetItem *missingNodeItem = new QTreeWidgetItem();
            missingNodeItem->setText(0, nodeName);
            nodesMissingdbSecond->addChild(missingNodeItem);
        }
    }

    for (int i = 0; i < seconddb->db_nodes.count(); i++)
    {
        QString nodeName = seconddb->db_nodes[i].name;
        db_NODE *node = firstdb->findNodeByName(nodeName);
        if (!node)
        {
            QTreeWidgetItem *missingNodeItem = new QTreeWidgetItem();
            missingNodeItem->setText(0, nodeName);
            nodesMissingdbFirst->addChild(missingNodeItem);
        }
    }



    //Find messages that are missing in one of the db files
    //If a message exists in both then check for missing signals

    QTreeWidgetItem *msgDiffRoot = new QTreeWidgetItem();
    msgDiffRoot->setText(0, "Message Differences");
    QTreeWidgetItem *msgMissingdbFirst = new QTreeWidgetItem();
    msgMissingdbFirst->setText(0, "Messages not found in first db");
    QTreeWidgetItem *msgMissingdbSecond = new QTreeWidgetItem();
    msgMissingdbSecond->setText(0, "Messages not found in second db");
    QTreeWidgetItem *msgSignalsDiff = new QTreeWidgetItem();
    msgSignalsDiff->setText(0, "Messages with missing signals");
    QTreeWidgetItem *sigDiffOne = new QTreeWidgetItem();
    sigDiffOne->setText(0, "Missing from first db");
    QTreeWidgetItem *sigDiffTwo = new QTreeWidgetItem();
    sigDiffTwo->setText(0, "Missing from second db");
    QTreeWidgetItem *sigModifiedRoot = new QTreeWidgetItem();
    sigModifiedRoot->setText(0, "Modified Signals");
    msgSignalsDiff->addChild(sigDiffOne);
    msgSignalsDiff->addChild(sigDiffTwo);

    msgDiffRoot->addChild(msgMissingdbFirst);
    msgDiffRoot->addChild(msgMissingdbSecond);
    msgDiffRoot->addChild(msgSignalsDiff);
    msgDiffRoot->addChild(sigModifiedRoot);

    ui->treeDetails->addTopLevelItem(msgDiffRoot);

    QTreeWidgetItem *msgItem {};
    QTreeWidgetItem *sigTemp {};

    for (int i = 0; i < firstdb->messageHandler->getCount(); i++)
    {
        db_MESSAGE *thisMsg = firstdb->messageHandler->findMsgByIdx(i);
        QString msgName = thisMsg->name;
        db_MESSAGE *otherMsg = seconddb->messageHandler->findMsgByName(msgName);
        if (!otherMsg)
        {
            QTreeWidgetItem *missingMsgItem = new QTreeWidgetItem();
            missingMsgItem->setText(0, msgName+ " (" + Utility::formatCANID(thisMsg->ID) + ")");
            msgMissingdbSecond->addChild(missingMsgItem);
        }
        else //both have Msg. Check sigs for missing
        {
            bool thisMsgHasMissing = false;
            bool thisMsgHasMods = false;

            for (int i = 0; i < thisMsg->sigHandler->getCount(); i++)
            {
                DB_SIGNAL *thisSig = thisMsg->sigHandler->findSignalByIdx(i);
                QString sigName = thisSig->name;
                DB_SIGNAL *otherSig = otherMsg->sigHandler->findSignalByName(sigName);
                if (!otherSig)
                {
                    QTreeWidgetItem *missingSigItem = new QTreeWidgetItem();
                    missingSigItem->setText(0, sigName);
                    if (!thisMsgHasMissing)
                    {
                        thisMsgHasMissing = true;
                        msgItem = new QTreeWidgetItem();
                        msgItem->setText(0, msgName + " (" + Utility::formatCANID(thisMsg->ID) + ")");
                        sigDiffTwo->addChild(msgItem);
                    }
                    if (msgItem)
                        msgItem->addChild(missingSigItem);
                }
                else //signal exists on both sides. See if it as changed position or length
                {
                    bool didChange = false;
                    QTreeWidgetItem *sigItem = new QTreeWidgetItem();
                    sigItem->setText(0, sigName);
                    if (thisSig->startBit != otherSig->startBit)
                    {
                        didChange = true;
                        QTreeWidgetItem *sigStartBit = new QTreeWidgetItem();
                        sigStartBit->setText(0, "Start Bit       First db: " + QString::number(thisSig->startBit) + "     Second: " + QString::number(otherSig->startBit));
                        sigItem->addChild(sigStartBit);
                    }
                    if (thisSig->signalSize != otherSig->signalSize)
                    {
                        didChange = true;
                        QTreeWidgetItem *sigSize = new QTreeWidgetItem();
                        sigSize->setText(0, "Size          First db: " + QString::number(thisSig->signalSize) + "      Second: " + QString::number(otherSig->signalSize));
                        sigItem->addChild(sigSize);
                    }
                    if (thisSig->bias != otherSig->bias)
                    {
                        didChange = true;
                        QTreeWidgetItem *sigBias = new QTreeWidgetItem();
                        sigBias->setText(0, "Bias          First db: " + QString::number(thisSig->bias) + "      Second: " + QString::number(otherSig->bias));
                        sigItem->addChild(sigBias);
                    }
                    if (thisSig->factor != otherSig->factor)
                    {
                        didChange = true;
                        QTreeWidgetItem *sigFactor = new QTreeWidgetItem();
                        sigFactor->setText(0, "Factor        First db: " + QString::number(thisSig->factor) + "      Second: " + QString::number(otherSig->factor));
                        sigItem->addChild(sigFactor);
                    }
                    if (didChange)
                    {
                        if (!thisMsgHasMods)
                        {
                            thisMsgHasMods = true;
                            sigTemp = new QTreeWidgetItem();
                            sigTemp->setText(0, msgName + " (" + Utility::formatCANID(thisMsg->ID) + ")");
                            sigModifiedRoot->addChild(sigTemp);
                        }
                        if (sigTemp)
                            sigTemp->addChild(sigItem);
                    }
                }
            }
            thisMsgHasMissing = false;
            for (int i = 0; i < otherMsg->sigHandler->getCount(); i++)
            {
                DB_SIGNAL *thisSig = otherMsg->sigHandler->findSignalByIdx(i);
                QString sigName = thisSig->name;
                DB_SIGNAL *otherSig = thisMsg->sigHandler->findSignalByName(sigName);
                if (!otherSig)
                {
                    QTreeWidgetItem *missingSigItem = new QTreeWidgetItem();
                    missingSigItem->setText(0, sigName);
                    if (!thisMsgHasMissing)
                    {
                        thisMsgHasMissing = true;
                        msgItem = new QTreeWidgetItem();
                        msgItem->setText(0, msgName + " (" + Utility::formatCANID(thisMsg->ID) + ")");
                        sigDiffOne->addChild(msgItem);
                    }
                    msgItem->addChild(missingSigItem);
                }
            }
        }
    }

    for (int i = 0; i < seconddb->messageHandler->getCount(); i++)
    {
        db_MESSAGE *origMsg = seconddb->messageHandler->findMsgByIdx(i);
        QString msgName = origMsg->name;
        db_MESSAGE *msg = firstdb->messageHandler->findMsgByName(msgName);
        if (!msg)
        {
            QTreeWidgetItem *missingMsgItem = new QTreeWidgetItem();
            missingMsgItem->setText(0, msgName + " (" + Utility::formatCANID(origMsg->ID) + ")");
            msgMissingdbFirst->addChild(missingMsgItem);
        }
    }

    QSettings settings;
    if (settings.value("InfoCompare/AutoExpand", false).toBool())
    {
        ui->treeDetails->expandAll();
    }

    progress.cancel();

    qApp->processEvents();
}

void dbComparatorWindow::saveDetails()
{
    QString filename;
    QFileDialog dialog(this);
    QSettings settings;

    QStringList filters;
    filters.append(QString(tr("Text File (*.txt)")));

    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilters(filters);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDirectory(settings.value("FileComparator/LoadSaveDirectory", dialog.directory().path()).toString());

    if (dialog.exec() == QDialog::Accepted)
    {
        filename = dialog.selectedFiles()[0];
        settings.setValue("FileComparator/LoadSaveDirectory", dialog.directory().path());
        if (!filename.contains('.')) filename += ".txt";
        QFile *outFile = new QFile(filename);

        if (!outFile->open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        QTreeWidget *tree = ui->treeDetails;


        QTreeWidgetItemIterator it(tree);
        while (*it) {
          QTreeWidgetItem *item = *it;
          QString itemText = item->text(0);
          while (item->parent())
          {
              outFile->write("   ");
              item = item->parent();
          }
          outFile->write(itemText.toUtf8() + "\n");
          ++it;
        }

        outFile->close();

    }
}

