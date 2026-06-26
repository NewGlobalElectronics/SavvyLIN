#include "dbsignaleditor.h"
#include "ui_dbsignaleditor.h"
#include <QDateTime>
#include <QDebug>
#include <QMenu>
#include <QSettings>
#include <QRandomGenerator>
#include <QMessageBox>
#include <qevent.h>
#include "helpwindow.h"

dbSignalEditor::dbSignalEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dbSignalEditor)
{
    ui->setupUi(this);

    readSettings();

    dbHandler = dbHandler::getReference();
    dbMessage = nullptr;
    currentSignal = nullptr;
    inhibitMsgProc = false;

    // Hide all multiplexing related UI elements (LIN does not support multiplexed signals)
    //ui->groupBoxMultiplex->setVisible(false);       // if the group box exists
   // ui->txtMultiplexValues->setVisible(false);
 //   ui->cbMultiplexParent->setVisible(false);
    //ui->rbMultiplexed->setVisible(false);
   // ui->rbMultiplexor->setVisible(false);
    //ui->rbNotMulti->setVisible(false);
    //ui->rbExtended->setVisible(false);

    QStringList headers2;
    headers2 << "Value" << "Text";
    ui->valuesTable->setColumnCount(2);
    ui->valuesTable->setColumnWidth(0, 200);
    ui->valuesTable->setColumnWidth(1, 440);
    ui->valuesTable->setHorizontalHeaderLabels(headers2);
    ui->valuesTable->horizontalHeader()->setStretchLastSection(true);

    ui->comboType->addItem("UNSIGNED INTEGER");
    ui->comboType->addItem("SIGNED INTEGER");
    ui->comboType->addItem("SINGLE PRECISION");
    ui->comboType->addItem("DOUBLE PRECISION");
    ui->comboType->addItem("STRING");

    ui->bitfield->setMode(GridMode::SIGNAL_VIEW);

    connect(ui->bitfield, SIGNAL(gridClicked(int)), this, SLOT(bitfieldLeftClicked(int)));
    connect(ui->bitfield, SIGNAL(gridRightClicked(int)), this, SLOT(bitfieldRightClicked(int)));

    connect(ui->valuesTable, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCustomMenuValues(QPoint)));
    ui->valuesTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->valuesTable, SIGNAL(cellChanged(int,int)), this, SLOT(onValuesCellChanged(int,int)));

    // Intel/Motorola byte order
    connect(ui->cbIntelFormat, &QCheckBox::toggled,
            [=]()
            {
                if (currentSignal == nullptr) return;
                if (currentSignal->intelByteOrder != ui->cbIntelFormat->isChecked())
                {
                    dbFile->setDirtyFlag();
                    pushToUndoBuffer();
                    currentSignal->intelByteOrder = ui->cbIntelFormat->isChecked();
                    refreshBitGrid();
                }
            });

    // Receiver node
    connect(ui->comboReceiver, &QComboBox::currentTextChanged,
            [=]()
            {
                if (currentSignal == nullptr) return;
                if (inhibitMsgProc) return;

                db_NODE *node = dbFile->findNodeByName(ui->comboReceiver->currentText());
                if (currentSignal->receiver != node)
                {
                    dbFile->setDirtyFlag();
                    pushToUndoBuffer();
                    currentSignal->receiver = node;
                }
            });

    // Signal type
    connect(ui->comboType, &QComboBox::currentTextChanged,
            [=]()
            {
                if (currentSignal == nullptr) return;
                switch (ui->comboType->currentIndex())
                {
                case 0:
                    if (currentSignal->valType != UNSIGNED_INT)
                    {
                        pushToUndoBuffer();
                        currentSignal->valType = UNSIGNED_INT;
                        dbFile->setDirtyFlag();
                        fillSignalForm(currentSignal);
                    }
                    break;
                case 1:
                    if (currentSignal->valType != SIGNED_INT)
                    {
                        pushToUndoBuffer();
                        currentSignal->valType = SIGNED_INT;
                        dbFile->setDirtyFlag();
                        fillSignalForm(currentSignal);
                    }
                    break;
                case 2:
                    if (currentSignal->valType != SP_FLOAT)
                    {
                        pushToUndoBuffer();
                        currentSignal->valType = SP_FLOAT;
                        dbFile->setDirtyFlag();
                        if (dbMessage) // use message length to constrain start bit
                        {
                            int maxBit = ((dbMessage->len * 8) - 32 + 7);
                            if (maxBit < 0) maxBit = 0;
                            if (currentSignal->startBit > maxBit) currentSignal->startBit = maxBit;
                        }
                        else if (currentSignal->startBit > 39) currentSignal->startBit = 39;
                        currentSignal->signalSize = 32;
                        fillSignalForm(currentSignal);
                    }
                    break;
                case 3:
                    if (currentSignal->valType != DP_FLOAT)
                    {
                        pushToUndoBuffer();
                        currentSignal->valType = DP_FLOAT;
                        dbFile->setDirtyFlag();
                        if (dbMessage)
                        {
                            int maxBit = ((dbMessage->len * 8) - 64 + 7);
                            if (currentSignal->startBit > maxBit) currentSignal->startBit = maxBit;
                        }
                        else currentSignal->startBit = 7; // has to be!
                        currentSignal->signalSize = 64;
                        fillSignalForm(currentSignal);
                    }
                    break;
                case 4:
                    if (currentSignal->valType != STRING)
                    {
                        pushToUndoBuffer();
                        currentSignal->valType = STRING;
                        dbFile->setDirtyFlag();
                        fillSignalForm(currentSignal);
                    }
                    break;
                }
            });

    // Bias
    connect(ui->txtBias, &QLineEdit::editingFinished,
            [=]()
            {
                if (currentSignal == nullptr) return;
                double temp;
                bool result;
                temp = ui->txtBias->text().toDouble(&result);
                if (result)
                {
                    if (currentSignal->bias != temp)
                    {
                        pushToUndoBuffer();
                        dbFile->setDirtyFlag();
                        currentSignal->bias = temp;
                    }
                }
            });

    // Max value
    connect(ui->txtMaxVal, &QLineEdit::editingFinished,
            [=]()
            {
                if (currentSignal == nullptr) return;
                double temp;
                bool result;
                temp = ui->txtMaxVal->text().toDouble(&result);
                if (result)
                {
                    if (currentSignal->max != temp)
                    {
                        pushToUndoBuffer();
                        dbFile->setDirtyFlag();
                        currentSignal->max = temp;
                    }
                }
            });

    // Min value
    connect(ui->txtMinVal, &QLineEdit::editingFinished,
            [=]()
            {
                if (currentSignal == nullptr) return;
                double temp;
                bool result;
                temp = ui->txtMinVal->text().toDouble(&result);
                if (result)
                {
                    if (currentSignal->min != temp)
                    {
                        pushToUndoBuffer();
                        dbFile->setDirtyFlag();
                        currentSignal->min = temp;
                    }
                }
            });

    // Scale factor
    connect(ui->txtScale, &QLineEdit::editingFinished,
            [=]()
            {
                if (currentSignal == nullptr) return;
                double temp;
                bool result;
                temp = ui->txtScale->text().toDouble(&result);
                if (result)
                {
                    if (currentSignal->factor != temp)
                    {
                        pushToUndoBuffer();
                        dbFile->setDirtyFlag();
                        currentSignal->factor = temp;
                    }
                }
            });

    // Comment
    connect(ui->txtComment, &QLineEdit::editingFinished,
            [=]()
            {
                if (currentSignal == nullptr) return;
                if (currentSignal->comment != ui->txtComment->text().simplified().replace(' ','_'))
                {
                    pushToUndoBuffer();
                    dbFile->setDirtyFlag();
                    currentSignal->comment = ui->txtComment->text().simplified().replace(' ', '_');
                    emit updatedTreeInfo(currentSignal);
                }
            });

    // Unit name
    connect(ui->txtUnitName, &QLineEdit::editingFinished,
            [=]()
            {
                if (currentSignal == nullptr) return;
                if (currentSignal->unitName != ui->txtUnitName->text().simplified().replace(' ','_'))
                {
                    pushToUndoBuffer();
                    dbFile->setDirtyFlag();
                    currentSignal->unitName = ui->txtUnitName->text().simplified().replace(' ', '_');
                }
            });

    // Bit length
    connect(ui->txtBitLength, &QLineEdit::textChanged,
            [=]()
            {
                if (currentSignal == nullptr) return;
                int temp = Utility::ParseStringToNum(ui->txtBitLength->text());
                if (temp < 1) return;
                if (dbMessage && temp > (int)(dbMessage->len * 8)) return;
                else if (temp > 64) return;

                // Force length for float/double
                if (currentSignal->valType == SP_FLOAT) temp = 32;
                if (currentSignal->valType == DP_FLOAT) temp = 64;

                if (currentSignal->signalSize != temp)
                {
                    pushToUndoBuffer();
                    dbFile->setDirtyFlag();
                    currentSignal->signalSize = temp;
                    refreshBitGrid();
                }
            });

    // Signal name
    connect(ui->txtName, &QLineEdit::editingFinished,
            [=]()
            {
                if (currentSignal == nullptr) return;
                QString tempNameStr = ui->txtName->text().simplified().replace(' ', '_');
                if (tempNameStr.length() == 0) return;
                if (currentSignal->name != tempNameStr)
                {
                    pushToUndoBuffer();
                    dbFile->setDirtyFlag();
                    currentSignal->name = tempNameStr;
                    refreshBitGrid();
                    emit updatedTreeInfo(currentSignal);
                }
            });

    installEventFilter(this);
}

dbSignalEditor::~dbSignalEditor()
{
    removeEventFilter(this);
    delete ui;
}

void dbSignalEditor::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    writeSettings();
}

bool dbSignalEditor::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key())
        {
        case Qt::Key_F1:
            HelpWindow::getRef()->showHelp("signaleditor.md");
            break;
        case Qt::Key_Z:
            if (keyEvent->modifiers() == Qt::ControlModifier)
            {
                popFromUndoBuffer();
            }
            break;
        }
        return true;
    }
    return QObject::eventFilter(obj, event);
}

void dbSignalEditor::setFileIdx(int idx)
{
    if (idx < 0 || idx >= dbHandler->getFileCount()) return;
    dbFile = dbHandler->getFileByIdx(idx);

    ui->comboReceiver->clear();
    for (int x = 0; x < dbFile->db_nodes.count(); x++)
    {
        ui->comboReceiver->addItem(dbFile->db_nodes[x].name);
    }
}

void dbSignalEditor::readSettings()
{
    QSettings settings;
    if (settings.value("Main/SaveRestorePositions", false).toBool())
    {
        resize(settings.value("dbSignalEditor/WindowSize", QSize(1000, 600)).toSize());
        move(Utility::constrainedWindowPos(settings.value("dbSignalEditor/WindowPos", QPoint(100, 100)).toPoint()));
    }
}

void dbSignalEditor::writeSettings()
{
    QSettings settings;
    if (settings.value("Main/SaveRestorePositions", false).toBool())
    {
        settings.setValue("dbSignalEditor/WindowSize", size());
        settings.setValue("dbSignalEditor/WindowPos", pos());
    }
}

void dbSignalEditor::setMessageRef(db_MESSAGE *msg)
{
    dbMessage = msg;
}

void dbSignalEditor::setSignalRef(DB_SIGNAL *sig)
{
    currentSignal = sig;
}

void dbSignalEditor::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    fillSignalForm(currentSignal);
    fillValueTable(currentSignal);
}

void dbSignalEditor::refreshView()
{
    fillSignalForm(currentSignal);
    fillValueTable(currentSignal);
}

void dbSignalEditor::onValuesCellChanged(int row, int col)
{
    if (inhibitCellChanged) return;

    if (row == ui->valuesTable->rowCount() - 1)
    {
        db_VAL_ENUM_ENTRY newVal;
        newVal.value = 0;
        newVal.descript = "No Description";
        currentSignal->valList.append(newVal);
        ui->valuesTable->insertRow(ui->valuesTable->rowCount());
    }

    if (col == 0)
    {
        currentSignal->valList[row].value = Utility::ParseStringToNum(ui->valuesTable->item(row, col)->text());
    }
    else if (col == 1)
    {
        currentSignal->valList[row].descript = ui->valuesTable->item(row, col)->text().simplified().replace(' ', '_');
    }
}

void dbSignalEditor::onCustomMenuValues(QPoint point)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addAction(tr("Delete currently selected value"), this, SLOT(deleteCurrentValue()));
    menu->popup(ui->valuesTable->mapToGlobal(point));
}

void dbSignalEditor::deleteCurrentValue()
{
    int currIdx = ui->valuesTable->currentRow();
    if (currIdx > -1)
    {
        ui->valuesTable->removeRow(currIdx);
        currentSignal->valList.removeAt(currIdx);
    }
}

void dbSignalEditor::fillSignalForm(DB_SIGNAL *sig)
{
    if (!dbMessage || !dbMessage->sigHandler) return;

    inhibitMsgProc = true;

    if (sig == nullptr)
    {
        ui->txtName->setText("");
        ui->txtBias->setText("");
        ui->txtBitLength->setText("");
        ui->txtComment->setText("");
        ui->txtMaxVal->setText("");
        ui->txtMinVal->setText("");
        ui->txtScale->setText("");
        ui->txtUnitName->setText("");
        ui->comboReceiver->setCurrentIndex(0);
        ui->comboType->setCurrentIndex(0);
        inhibitMsgProc = false;
        return;
    }

    ui->txtName->setText(sig->name);
    ui->txtBias->setText(QString::number(sig->bias));
    ui->txtBitLength->setText(QString::number(sig->signalSize));
    ui->txtComment->setText(sig->comment);
    ui->txtMaxVal->setText(QString::number(sig->max));
    ui->txtMinVal->setText(QString::number(sig->min));
    ui->txtScale->setText(QString::number(sig->factor));
    ui->txtUnitName->setText(sig->unitName);
    ui->cbIntelFormat->setChecked(sig->intelByteOrder);

    switch (sig->valType)
    {
    case UNSIGNED_INT: ui->comboType->setCurrentIndex(0); break;
    case SIGNED_INT:   ui->comboType->setCurrentIndex(1); break;
    case SP_FLOAT:     ui->comboType->setCurrentIndex(2); break;
    case DP_FLOAT:     ui->comboType->setCurrentIndex(3); break;
    case STRING:       ui->comboType->setCurrentIndex(4); break;
    }

    for (int i = 0; i < ui->comboReceiver->count(); i++)
    {
        if (ui->comboReceiver->itemText(i) == sig->receiver->name)
        {
            ui->comboReceiver->setCurrentIndex(i);
            break;
        }
    }

    refreshBitGrid();
    inhibitMsgProc = false;
}

void dbSignalEditor::refreshBitGrid()
{
    unsigned char bitpattern[64] = {0};
    ui->bitfield->setReference(bitpattern, false);
    ui->bitfield->updateData(bitpattern, true);
    ui->bitfield->clearSignalNames();

    // Assign names to all signals (no multiplex filtering)
    for (int x = 0; x < dbMessage->sigHandler->getCount(); x++)
    {
        DB_SIGNAL *sig = dbMessage->sigHandler->findSignalByIdx(x);
        if (sig)
            ui->bitfield->setSignalNames(x, sig->name);
    }

    generateUsedBits();

    memset(bitpattern, 0, 64);
    int startBit = currentSignal->startBit;
    bitpattern[startBit / 8] |= 1 << (startBit % 8);
    ui->bitfield->setReference(bitpattern, false);

    if (currentSignal->intelByteOrder)
    {
        int endBit = startBit + currentSignal->signalSize - 1;
        if (startBit < 0) startBit = 0;
        if (endBit > 511) endBit = 511;
        for (int y = startBit; y <= endBit; y++)
        {
            int byt = y / 8;
            bitpattern[byt] |= 1 << (y % 8);
        }
    }
    else // Motorola format
    {
        int size = currentSignal->signalSize;
        int bit = startBit;
        while (size > 0)
        {
            int byt = bit / 8;
            bitpattern[byt] |= 1 << (bit % 8);
            size--;
            if ((bit % 8) == 0)
                bit += 15;
            else
                bit--;
            if (bit > 511) bit = 511;
        }
    }

    ui->bitfield->updateData(bitpattern, true);
}

void dbSignalEditor::fillValueTable(DB_SIGNAL *sig)
{
    inhibitCellChanged = true;
    ui->valuesTable->clearContents();
    ui->valuesTable->setRowCount(0);

    if (sig == nullptr)
    {
        ui->valuesTable->setEnabled(false);
        inhibitCellChanged = false;
        return;
    }

    ui->valuesTable->setEnabled(true);

    for (int i = 0; i < sig->valList.count(); i++)
    {
        QTableWidgetItem *val = new QTableWidgetItem(Utility::formatNumber((uint64_t)sig->valList[i].value));
        QTableWidgetItem *desc = new QTableWidgetItem(sig->valList[i].descript);
        int rowIdx = ui->valuesTable->rowCount();
        ui->valuesTable->insertRow(rowIdx);
        ui->valuesTable->setItem(rowIdx, 0, val);
        ui->valuesTable->setItem(rowIdx, 1, desc);
    }
    ui->valuesTable->insertRow(ui->valuesTable->rowCount()); // blank row for adding new entries
    inhibitCellChanged = false;
}

void dbSignalEditor::bitfieldLeftClicked(int bit)
{
    if (currentSignal == nullptr) return;
    pushToUndoBuffer();
    currentSignal->startBit = bit;
    // Adjust for float/double if necessary
    if (currentSignal->valType == SP_FLOAT)
    {
        if (dbMessage)
        {
            int maxBit = ((dbMessage->len * 8) - 32 + 7);
            if (maxBit < 0) maxBit = 0;
            if (currentSignal->startBit > maxBit) currentSignal->startBit = maxBit;
        }
        else if (currentSignal->startBit > 31) currentSignal->startBit = 39;
    }
    if (currentSignal->valType == DP_FLOAT)
    {
        if (dbMessage)
        {
            int maxBit = ((dbMessage->len * 8) - 64 + 7);
            if (maxBit < 0) maxBit = 0;
            if (currentSignal->startBit > maxBit) currentSignal->startBit = maxBit;
        }
        else currentSignal->startBit = 7;
    }
    fillSignalForm(currentSignal);
}

void dbSignalEditor::bitfieldRightClicked(int bit)
{
    int sigNum = ui->bitfield->getUsedSignalNum(bit);
    if (sigNum < 0) return;

    pushToUndoBuffer();
    currentSignal = dbMessage->sigHandler->findSignalByIdx(sigNum);
    if (currentSignal)
    {
        fillSignalForm(currentSignal);
        fillValueTable(currentSignal);
    }
}

void dbSignalEditor::generateUsedBits()
{
    uint8_t usedBits[64] = {0};
    if (!dbMessage || !dbMessage->sigHandler) return;

    for (int x = 0; x < dbMessage->sigHandler->getCount(); x++)
    {
        DB_SIGNAL *sig = dbMessage->sigHandler->findSignalByIdx(x);
        if (!sig) continue;

        int startBit = sig->startBit;

        if (sig->intelByteOrder)
        {
            int endBit = startBit + sig->signalSize - 1;
            if (startBit < 0) startBit = 0;
            int maxBit = (dbMessage->len * 8) - 1;
            if (endBit > maxBit) endBit = maxBit;
            for (int y = startBit; y <= endBit; y++)
            {
                int byt = y / 8;
                usedBits[byt] |= 1 << (y % 8);
                ui->bitfield->setUsedSignalNum(y, x);
            }
        }
        else // Motorola
        {
            int size = sig->signalSize;
            int bit = startBit;
            while (size > 0)
            {
                int byt = bit / 8;
                usedBits[byt] |= 1 << (bit % 8);
                ui->bitfield->setUsedSignalNum(bit, x);
                size--;
                if ((bit % 8) == 0)
                    bit += 15;
                else
                    bit--;
                int maxBit = (dbMessage->len * 8) - 1;
                if (bit > maxBit) bit = maxBit;
            }
        }
    }
    ui->bitfield->setUsed(usedBits, false);
    ui->bitfield->setBytesToDraw(dbMessage->len);
}

void dbSignalEditor::pushToUndoBuffer()
{
    if (!currentSignal) return;
    currentSignal->self = currentSignal;
    undoBuffer.append(*currentSignal);
    qDebug() << "Pushing to undo buffer";
}

void dbSignalEditor::popFromUndoBuffer()
{
    if (undoBuffer.empty())
    {
        dbFile->clearDirtyFlag();
        qDebug() << "Undo buffer empty";
        return;
    }
    qDebug() << "Popping undo buffer";
    DB_SIGNAL sig = undoBuffer.back();
    undoBuffer.pop_back();
    currentSignal = sig.self;
    *currentSignal = sig;
    fillSignalForm(currentSignal);
    fillValueTable(currentSignal);
}