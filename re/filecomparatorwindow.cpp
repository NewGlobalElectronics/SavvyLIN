#include "filecomparatorwindow.h"
#include "ui_filecomparatorwindow.h"
#include "helpwindow.h"
#include <QProgressDialog>
#include <QSettings>
#include <qevent.h>

FileComparatorWindowLIN::FileComparatorWindowLIN(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileComparatorWindowLIN)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);

    connect(ui->btnInterestedFile, SIGNAL(clicked(bool)), this, SLOT(loadInterestedFile()));
    connect(ui->btnLoadRefFile, SIGNAL(clicked(bool)), this, SLOT(loadReferenceFile()));
    connect(ui->btnSaveDetails, SIGNAL(clicked(bool)), this, SLOT(saveDetails()));
    connect(ui->btnClear, SIGNAL(clicked(bool)), this, SLOT(clearReference()));

    ui->lblFirstFile->setText("");
    ui->lblRefFrames->setText("Loaded frames: 0");

    dbHandler = dbHandler::getReference();

    installEventFilter(this);
}

FileComparatorWindowLIN::~FileComparatorWindowLIN()
{
    removeEventFilter(this);
    delete ui;
}

void FileComparatorWindowLIN::showEvent(QShowEvent *)
{
    readSettings();
}

void FileComparatorWindowLIN::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    writeSettings();
}

bool FileComparatorWindowLIN::eventFilter(QObject *obj, QEvent *event)
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
    }
    return QObject::eventFilter(obj, event);
}

void FileComparatorWindowLIN::readSettings()
{
    QSettings settings;
    if (settings.value("Main/SaveRestorePositions", false).toBool())
    {
        resize(settings.value("FileComparator/WindowSize", QSize(720, 631)).toSize());
        move(Utility::constrainedWindowPos(settings.value("FileComparator/WindowPos", QPoint(50, 50)).toPoint()));
    }
}

void FileComparatorWindowLIN::writeSettings()
{
    QSettings settings;
    if (settings.value("Main/SaveRestorePositions", false).toBool())
    {
        settings.setValue("FileComparator/WindowSize", size());
        settings.setValue("FileComparator/WindowPos", pos());
    }
}

void FileComparatorWindowLIN::loadInterestedFile()
{
    interestedFrames.clear();
    QString resultingFileName;

    qApp->processEvents();

    if (FrameFileIO::loadFrameFile(resultingFileName, &interestedFrames))
    {
        ui->lblFirstFile->setText(resultingFileName);
        interestedFilename = resultingFileName;
        if (interestedFrames.count() > 0 && referenceFrames.count() > 0) calculateDetails();
    }
}

void FileComparatorWindowLIN::loadReferenceFile()
{
    QString resultingFileName;

    qApp->processEvents();

    if (FrameFileIO::loadFrameFile(resultingFileName, &referenceFrames))
    {
        ui->lblRefFrames->setText("Loaded frames: " + QString::number(referenceFrames.length()));
        if (interestedFrames.count() > 0 && referenceFrames.count() > 0) calculateDetails();
    }
}

void FileComparatorWindowLIN::clearReference()
{
    referenceFrames.clear();
    ui->treeDetails->clear();
    ui->lblRefFrames->setText("Loaded frames: " + QString::number(referenceFrames.length()));
}

void FileComparatorWindowLIN::calculateDetails()
{
    QMap<uint32_t, FrameData> interestedIDs;
    QMap<uint32_t, FrameData> referenceIDs;
    QTreeWidgetItem *interestedOnlyBase, *referenceOnlyBase = nullptr, *sharedBase, *bitmapBaseInterested, *bitmapBaseReference = nullptr;
    QTreeWidgetItem *valuesBase, *detail, *sharedItem, *valuesInterested, *valuesReference = nullptr;
    uint64_t tmp;
    const unsigned char *data;
    int dataLen;

    bool uniqueInterested = ui->ckUniqueToInterested->isChecked();

    QProgressDialog progress(this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setLabelText("Calculating differences");
    progress.setCancelButton(nullptr);
    progress.setRange(0,0);
    progress.setMinimumDuration(0);
    progress.show();

    qApp->processEvents();

    ui->treeDetails->clear();

    interestedOnlyBase = new QTreeWidgetItem();
    interestedOnlyBase->setText(0,"IDs found only in " + interestedFilename);
    if (!uniqueInterested)
    {
        referenceOnlyBase = new QTreeWidgetItem();
        referenceOnlyBase->setText(0, "IDs found only in Side 2 - Reference frames");
    }
    sharedBase = new QTreeWidgetItem();
    sharedBase->setText(0,"IDs found on both sides");

    // Process interested frames
    for (int x = 0; x < interestedFrames.count(); x++)
    {
        LINFrame frame = interestedFrames.at(x);
        db_MESSAGE *msg = dbHandler->findMessage(frame.frameId());
        data = reinterpret_cast<const unsigned char *>(frame.payload().constData());
        dataLen = frame.payload().count();

        if (interestedIDs.contains(frame.frameId()))
        {
            for (int y = 0; y < dataLen; y++)
            {
                interestedIDs[frame.frameId()].values[y][data[y]]++;
                tmp = data[y];
                tmp = tmp << (8 * y);
                interestedIDs[frame.frameId()].bitmap |= tmp;
            }
            if (msg)
            {
                int numSignals = msg->sigHandler->getCount();
                for (int i = 0; i < numSignals; i++)
                {
                    DB_SIGNAL *sig = msg->sigHandler->findSignalByIdx(i);
                    if (sig && sig->isSignalInMessage(frame))
                    {
                        QString sigVal;
                        if (sig->processAsText(frame, sigVal, false))
                        {
                            QList<QString> tempList = interestedIDs[frame.frameId()].signalInstances[sig->name];
                            if (!tempList.contains(sigVal)) tempList.append(sigVal);
                            interestedIDs[frame.frameId()].signalInstances[sig->name] = tempList;
                        }
                    }
                }
                qApp->processEvents();
            }
        }
        else
        {
            FrameData newData;
            newData.ID = frame.frameId();
            newData.dataLen = dataLen;
            newData.bitmap = 0;
            memset(newData.values, 0, sizeof(newData.values));
            for (int y = 0; y < dataLen; y++)
            {
                newData.values[y][data[y]] = 1;
                tmp = data[y];
                tmp = tmp << (8 * y);
                newData.bitmap |= tmp;
            }
            if (msg)
            {
                int numSignals = msg->sigHandler->getCount();
                for (int i = 0; i < numSignals; i++)
                {
                    DB_SIGNAL *sig = msg->sigHandler->findSignalByIdx(i);
                    if (sig && sig->isSignalInMessage(frame))
                    {
                        QString sigVal;
                        if (sig->processAsText(frame, sigVal, false))
                        {
                            QList<QString> tempList;
                            tempList.append(sigVal);
                            newData.signalInstances[sig->name] = tempList;
                        }
                    }
                }
            }
            interestedIDs.insert(frame.frameId(), newData);
        }
    }

    qApp->processEvents();

    // Process reference frames
    for (int x = 0; x < referenceFrames.count(); x++)
    {
        LINFrame frame = referenceFrames.at(x);
        db_MESSAGE *msg = dbHandler->findMessage(frame.frameId());
        data = reinterpret_cast<const unsigned char *>(frame.payload().constData());
        dataLen = frame.payload().count();

        if (referenceIDs.contains(frame.frameId()))
        {
            for (int y = 0; y < dataLen; y++)
            {
                referenceIDs[frame.frameId()].values[y][data[y]]++;
                tmp = data[y];
                tmp = tmp << (8 * y);
                referenceIDs[frame.frameId()].bitmap |= tmp;
            }
            if (msg)
            {
                int numSignals = msg->sigHandler->getCount();
                for (int i = 0; i < numSignals; i++)
                {
                    DB_SIGNAL *sig = msg->sigHandler->findSignalByIdx(i);
                    if (sig && sig->isSignalInMessage(frame))
                    {
                        QString sigVal;
                        if (sig->processAsText(frame, sigVal, false))
                        {
                            QList<QString> tempList = referenceIDs[frame.frameId()].signalInstances[sig->name];
                            if (!tempList.contains(sigVal)) tempList.append(sigVal);
                            referenceIDs[frame.frameId()].signalInstances[sig->name] = tempList;
                        }
                    }
                }
            }
        }
        else
        {
            FrameData newData;
            newData.ID = frame.frameId();
            newData.dataLen = dataLen;
            newData.bitmap = 0;
            memset(newData.values, 0, sizeof(newData.values));
            for (int y = 0; y < dataLen; y++)
            {
                newData.values[y][data[y]] = 1;
                tmp = data[y];
                tmp = tmp << (8 * y);
                newData.bitmap |= tmp;
            }
            if (msg)
            {
                int numSignals = msg->sigHandler->getCount();
                for (int i = 0; i < numSignals; i++)
                {
                    DB_SIGNAL *sig = msg->sigHandler->findSignalByIdx(i);
                    if (sig && sig->isSignalInMessage(frame))
                    {
                        QString sigVal;
                        if (sig->processAsText(frame, sigVal, false))
                        {
                            QList<QString> tempList;
                            tempList.append(sigVal);
                            newData.signalInstances[sig->name] = tempList;
                        }
                    }
                }
            }
            referenceIDs.insert(frame.frameId(), newData);
        }
    }

    qApp->processEvents();

    // Compare IDs
    bool interestedHadUnique = false;
    QMap<uint32_t, FrameData>::iterator i;
    int framesCounter = 0;
    for (i = interestedIDs.begin(); i != interestedIDs.end(); ++i)
    {
        framesCounter++;
        if (framesCounter > 50)
        {
            framesCounter = 0;
            qApp->processEvents();
        }

        uint32_t keyone = i.key();
        if (!referenceIDs.contains(keyone))
        {
            valuesBase = new QTreeWidgetItem();
            db_MESSAGE *msg = dbHandler->findMessage(keyone);
            if (msg) valuesBase->setText(0, Utility::formatHexNum(keyone) + " (" + msg->name + ")");
            else valuesBase->setText(0, Utility::formatHexNum(keyone));
            interestedOnlyBase->addChild(valuesBase);
        }
        else
        {
            interestedHadUnique = false;
            sharedItem = new QTreeWidgetItem();
            db_MESSAGE *msg = dbHandler->findMessage(keyone);
            if (msg) sharedItem->setText(0, Utility::formatHexNum(keyone) + " (" + msg->name + ")");
            else sharedItem->setText(0, Utility::formatHexNum(keyone));

            FrameData interested = interestedIDs[keyone];
            FrameData reference = referenceIDs[keyone];

            bitmapBaseInterested = new QTreeWidgetItem();
            bitmapBaseInterested->setText(0, "Bits set only in " + interestedFilename);
            if (!uniqueInterested)
            {
                bitmapBaseReference = new QTreeWidgetItem();
                bitmapBaseReference->setText(0, "Bits set only in Side 2 - Reference frames");
            }
            sharedItem->addChild(bitmapBaseInterested);
            if (!uniqueInterested) sharedItem->addChild(bitmapBaseReference);

            uint64_t interestedBits = interested.bitmap;
            uint64_t referenceBits = reference.bitmap;

            for (int b = 0; b < (8 * interested.dataLen); b++)
            {
                detail = new QTreeWidgetItem();
                detail->setText(0, QString::number(b) + " (" + QString::number(b / 8) + ":" + QString::number(b % 8) + ")");
                if ((interestedBits & 1) && !(referenceBits & 1))
                {
                    bitmapBaseInterested->addChild(detail);
                    interestedHadUnique = true;
                }
                else if (!(interestedBits & 1) && (referenceBits & 1))
                {
                    if (!uniqueInterested) bitmapBaseReference->addChild(detail);
                }
                interestedBits >>= 1;
                referenceBits >>= 1;
            }

            for (int idx = 0; idx < qMax(interested.dataLen, reference.dataLen); idx++)
            {
                valuesBase = new QTreeWidgetItem();
                valuesBase->setText(0, "Byte " + QString::number(idx));
                sharedItem->addChild(valuesBase);
                valuesInterested = new QTreeWidgetItem();
                valuesInterested->setText(0, "Values found only in " + interestedFilename);
                if (!uniqueInterested)
                {
                    valuesReference = new QTreeWidgetItem();
                    valuesReference->setText(0, "Values found only in Side 2 - Reference frames");
                }
                valuesBase->addChild(valuesInterested);
                if (!uniqueInterested) valuesBase->addChild(valuesReference);
                for (int j = 0; j < 256; j++)
                {
                    detail = new QTreeWidgetItem();
                    detail->setText(0, Utility::formatHexNum(static_cast<unsigned int>(j)));
                    if ((interested.values[idx][j] > 0) && (reference.values[idx][j] == 0))
                    {
                        valuesInterested->addChild(detail);
                        interestedHadUnique = true;
                    }
                    if ((reference.values[idx][j] > 0) && (interested.values[idx][j] == 0))
                    {
                        if (!uniqueInterested) valuesReference->addChild(detail);
                    }
                }
            }

            // Compare signal values
            QHash<QString, QList<QString>>::const_iterator it = reference.signalInstances.constBegin();
            while (it != reference.signalInstances.constEnd())
            {
                valuesBase = new QTreeWidgetItem();
                valuesBase->setText(0, "Signal " + it.key());
                sharedItem->addChild(valuesBase);
                valuesInterested = new QTreeWidgetItem();
                valuesInterested->setText(0, "Values found only in " + interestedFilename);
                if (!uniqueInterested)
                {
                    valuesReference = new QTreeWidgetItem();
                    valuesReference->setText(0, "Values found only in Side 2 - Reference frames");
                }
                valuesBase->addChild(valuesInterested);
                if (!uniqueInterested) valuesBase->addChild(valuesReference);

                QList<QString> refVals = it.value();
                QList<QString> interestedVals = interested.signalInstances[it.key()];
                foreach (QString str, refVals)
                {
                    if (!interestedVals.contains(str))
                    {
                        detail = new QTreeWidgetItem();
                        detail->setText(0, str);
                        valuesReference->addChild(detail);
                    }
                }
                qApp->processEvents();
                foreach (QString str, interestedVals)
                {
                    if (!refVals.contains(str))
                    {
                        detail = new QTreeWidgetItem();
                        detail->setText(0, str);
                        valuesInterested->addChild(detail);
                    }
                }
                ++it;
            }

            if (interestedHadUnique || !uniqueInterested) sharedBase->addChild(sharedItem);
        }
    }

    qApp->processEvents();

    if (!uniqueInterested)
    {
        QMap<uint32_t, FrameData>::iterator itwo;
        for (itwo = referenceIDs.begin(); itwo != referenceIDs.end(); ++itwo)
        {
            uint32_t keytwo = itwo.key();
            if (!interestedIDs.contains(keytwo))
            {
                valuesBase = new QTreeWidgetItem();
                db_MESSAGE *msg = dbHandler->findMessage(keytwo);
                if (msg) valuesBase->setText(0, Utility::formatHexNum(keytwo) + " (" + msg->name + ")");
                else valuesBase->setText(0, Utility::formatHexNum(keytwo));
                referenceOnlyBase->addChild(valuesBase);
            }
        }
    }

    ui->treeDetails->addTopLevelItem(interestedOnlyBase);
    if (!uniqueInterested) ui->treeDetails->addTopLevelItem(referenceOnlyBase);
    ui->treeDetails->addTopLevelItem(sharedBase);

    QSettings settings;
    if (settings.value("InfoCompare/AutoExpand", false).toBool())
    {
        ui->treeDetails->expandAll();
    }

    progress.cancel();
}

void FileComparatorWindowLIN::saveDetails()
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
        QFile outFile(filename);
        if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        QTreeWidget *tree = ui->treeDetails;
        QTreeWidgetItemIterator it(tree);
        while (*it)
        {
            QTreeWidgetItem *item = *it;
            QString itemText = item->text(0);
            int depth = 0;
            QTreeWidgetItem *parent = item->parent();
            while (parent)
            {
                depth++;
                parent = parent->parent();
            }
            outFile.write(QString(depth * 3, ' ').toUtf8());
            outFile.write(itemText.toUtf8() + "\n");
            ++it;
        }
        outFile.close();
    }
}