#include "framesenderwindow.h"
#include "ui_framesenderwindow.h"
#include "utility.h"

#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QThread>
#include <QComboBox>
#include <QPushButton>
#include <QBrush>
#include <QColor>

#include "mainwindow.h"
#include "helpwindow.h"
#include "connections/canconmanager.h"
#include "triggerdialog.h"
#include "connections/ngeconnection.h"

static const QColor COLOR_READ  (0xD0, 0xE8, 0xFF);
static const QColor COLOR_WRITE (0xD0, 0xFF, 0xD0);
static const QColor COLOR_SLAVE (0xFF, 0xF0, 0xCC);

// ─────────────────────────────────────────────────────────────────
// CONSTRUCTEUR
// ─────────────────────────────────────────────────────────────────
FrameSenderWindowLIN::FrameSenderWindowLIN(const QVector<LINFrame> *frames,
                                     QWidget *parent)
    : QDialog(parent), ui(new Ui::FrameSenderWindowLIN)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);

    modelFrames = frames;
    dbHandler   = dbHandler::getReference();

    intervalTimer = new QTimer();
    intervalTimer->setTimerType(Qt::PreciseTimer);
    intervalTimer->setInterval(1);

    setupGrid();
    appendBlankRow();

    connect(ui->tableSender, &QTableWidget::cellChanged,
            this, &FrameSenderWindowLIN::onCellChanged);
    connect(ui->tableSender, &QTableWidget::cellDoubleClicked,
            this, &FrameSenderWindowLIN::onCellDoubleTap);
    connect(intervalTimer, &QTimer::timeout,
            this, &FrameSenderWindowLIN::handleTick);

    connect(ui->btnClearGrid,     &QPushButton::clicked, this, &FrameSenderWindowLIN::clearGrid);
    connect(ui->btnDisableAll,    &QPushButton::clicked, this, &FrameSenderWindowLIN::disableAll);
    connect(ui->btnEnableAll,     &QPushButton::clicked, this, &FrameSenderWindowLIN::enableAll);
    connect(ui->btnLoadGrid,      &QPushButton::clicked, this, &FrameSenderWindowLIN::loadGrid);
    connect(ui->btnSaveGrid,      &QPushButton::clicked, this, &FrameSenderWindowLIN::saveGrid);
    connect(ui->btnAddFrame,      &QPushButton::clicked, this, &FrameSenderWindowLIN::onAddFrameClicked);
    connect(ui->btnActivateSlave, &QPushButton::clicked, this, &FrameSenderWindowLIN::sendSlaveModeCommand);

    connect(MainWindow::getReference(), SIGNAL(framesUpdated(int)),
            this, SLOT(updatedFrames(int)));

    buildFrameCache();
    intervalTimer->start();
    elapsedTimer.start();
    installEventFilter(this);
}
// ═══════════════════════════════════════════════════════════════════
// SYNCHRONISER une ligne UI vers lin_frame_table (via NGEConnectionLIN)
// ═════════════════════════════════════════════════════════════════==
void FrameSenderWindowLIN::pushRowToFirmware(int row)
{
    qDebug() << "[pushRowToFirmware] row=" << row;
    QTableWidget *t = ui->tableSender;
    QLabel *lbl = qobject_cast<QLabel*>(t->cellWidget(row, col(FSCol::Type)));
    if (!lbl) return;
    int rowType = lbl->property("typeIndex").toInt();

    QTableWidgetItem *idItem = t->item(row, col(FSCol::Id));
    if (!idItem || idItem->text().isEmpty()) return;
    bool ok;
    int idVal = idItem->text().toInt(&ok, 0);
    if (!ok || idVal < 0 || idVal > 0x3F) return;
    uint8_t id = (uint8_t)idVal;

    bool checked = t->item(row, col(FSCol::En))->checkState() == Qt::Checked;

    if (!checked) {
        removeFrameFromDevice(id, rowType);
        return;
    }

    uint16_t period_ms = 1000;
    QTableWidgetItem *trigItem = t->item(row, col(FSCol::Trigger));
    if (trigItem && !trigItem->text().isEmpty()) {
        QString trig = trigItem->text().toUpper().trimmed();
        // Extraire le nombre avant "MS"
        QRegularExpression re("(\\d+)\\s*MS");
        QRegularExpressionMatch match = re.match(trig);
        if (match.hasMatch()) {
            period_ms = (uint16_t)match.captured(1).toInt();
        }
    }

    QByteArray data;
    QTableWidgetItem *dataItem = t->item(row, col(FSCol::Data));
    if (dataItem && !dataItem->text().isEmpty()) {
        QStringList tokens = dataItem->text().split(' ', Qt::SkipEmptyParts);
        for (int i = 0; i < tokens.count() && i < 8; i++) {
            int v = tokens[i].toInt(&ok, 16);
            if (ok) data.append((uint8_t)v);
        }
    }

    switch (rowType) {
    case 0: {
        qDebug() << "[pushRowToFirmware] READ mode: sending 0x06 id=0x"
                 << QString::number(id, 16)
                 << "period_ms=" << period_ms;
        QByteArray p;
        p.append((char)0x06);
        p.append((char)(id & 0x3F));
        p.append((char)(period_ms & 0xFF));
        p.append((char)((period_ms >> 8) & 0xFF));
        CANConManager::getInstance()->sendBinaryCommand(TYPE_LIN_CMD, p);
        break;
    }
    case 1: {
        qDebug() << "[pushRowToFirmware] WRITE mode: sending 0x07 id=0x"
                 << QString::number(id, 16);
        uint8_t dlc = (uint8_t)qMin(data.size(), 8);
        QByteArray p;
        p.append((char)0x07);
        p.append((char)(id & 0x3F));
        p.append((char)dlc);
        p.append(data.left(dlc));
        p.append((char)(period_ms & 0xFF));
        p.append((char)((period_ms >> 8) & 0xFF));
        CANConManager::getInstance()->sendBinaryCommand(TYPE_LIN_CMD, p);
        break;
    }
    case 2:
        sendLinSlaveToDevice(id, data);
        break;
    }
}

// ── Envoyer commandes LIN au device ─────────────────────────────
void FrameSenderWindowLIN::sendLinReadToDevice(uint8_t id, uint16_t period_ms)
{
    QByteArray p;
    p.append((char)0x06);           // LIN_SUBCMD_READ_MODE
    p.append((char)(id & 0x3F));
    p.append((char)(period_ms & 0xFF));
    p.append((char)((period_ms >> 8) & 0xFF));
    CANConManager::getInstance()->sendBinaryCommand(TYPE_LIN_CMD, p);
}

void FrameSenderWindowLIN::sendLinWriteToDevice(uint8_t id, const QByteArray &data, uint16_t period_ms)
{
    uint8_t dlc = qMin((int)data.size(), 8);
    QByteArray p;
    p.append((char)0x07);           // LIN_SUBCMD_WRITE_MODE
    p.append((char)(id & 0x3F));
    p.append((char)dlc);
    p.append(data.left(dlc));
    p.append((char)(period_ms & 0xFF));
    p.append((char)((period_ms >> 8) & 0xFF));
    CANConManager::getInstance()->sendBinaryCommand(TYPE_LIN_CMD, p);
}
void FrameSenderWindowLIN::sendLinSlaveToDevice(uint8_t id, const QByteArray &data)
{
    // D'abord mode slave (0x04) si pas déjà fait
    sendSlaveModeCommand();

    // Puis subcommand 0x05 = SET_SLAVE_RESPONSE
    uint8_t dlc = qMin((int)data.size(), 8);
    QByteArray p;
    p.append((char)0x05);
    p.append((char)(id & 0x3F));
    p.append((char)dlc);
    p.append(data.left(dlc));
    CANConManager::getInstance()->sendBinaryCommand(TYPE_LIN_CMD, p);
}

void FrameSenderWindowLIN::removeFrameFromDevice(uint8_t id, int rowType)
{
    qDebug() << "[removeFrameFromDevice] id=0x"
             << QString::number(id, 16)
             << "rowType=" << rowType;
    if (rowType == 2) {
        QByteArray p;
        p.append((char)0xF6);
        p.append((char)(id & 0x3F));
        CANConManager::getInstance()->sendBinaryCommand(TYPE_LIN_CMD, p);
    } else if (rowType == 1) {
        // Write : période 0 via subcmd 0x07
        QByteArray p;
        p.append((char)0x07);
        p.append((char)(id & 0x3F));
        p.append((char)0x00);  // dlc = 0
        p.append((char)0x00);  // period LSB = 0
        p.append((char)0x00);  // period MSB = 0
        CANConManager::getInstance()->sendBinaryCommand(TYPE_LIN_CMD, p);
    } else {
        // Read : période 0 via subcmd 0x06
        QByteArray p;
        p.append((char)0x06);
        p.append((char)(id & 0x3F));
        p.append((char)0x00);
        p.append((char)0x00);
        CANConManager::getInstance()->sendBinaryCommand(TYPE_LIN_CMD, p);
    }
}
FrameSenderWindowLIN::~FrameSenderWindowLIN()
{
    removeEventFilter(this);
    intervalTimer->stop();

    // ── AJOUT : nettoyer le firmware avant fermeture ──
    QTableWidget *t = ui->tableSender;
    for (int i = 0; i < t->rowCount(); i++) {
        QTableWidgetItem *enIt = t->item(i, col(FSCol::En));
        if (!enIt || enIt->checkState() != Qt::Checked) continue;

        QTableWidgetItem *idIt = t->item(i, col(FSCol::Id));
        if (!idIt || idIt->text().isEmpty()) continue;

        bool ok;
        int idVal = idIt->text().toInt(&ok, 0);
        if (!ok || idVal < 0 || idVal > 0x3F) continue;

        QLabel *lbl = qobject_cast<QLabel*>(t->cellWidget(i, col(FSCol::Type)));
        int rowType = lbl ? lbl->property("typeIndex").toInt() : 0;

        removeFrameFromDevice((uint8_t)(idVal & 0x3F), rowType);
    }
    // ── FIN AJOUT ──

    delete intervalTimer;
    delete ui;
}
// ─────────────────────────────────────────────────────────────────
// SETUP TABLE
// ─────────────────────────────────────────────────────────────────
void FrameSenderWindowLIN::setupGrid()
{
    QTableWidget *t = ui->tableSender;
    t->setColumnCount(col(FSCol::NUM_COLS));
    t->setHorizontalHeaderLabels({
        "En","Type","Bus","ID","MsgName","Len","Data","Trigger","Mods","Count","Remove"
    });
    t->setColumnWidth(col(FSCol::En),      40);
    t->setColumnWidth(col(FSCol::Type),    220);
    t->setColumnWidth(col(FSCol::Bus),     45);
    t->setColumnWidth(col(FSCol::Id),      60);
    t->setColumnWidth(col(FSCol::MsgName),140);
    t->setColumnWidth(col(FSCol::Len),     40);
    t->setColumnWidth(col(FSCol::Data),   220);
    t->setColumnWidth(col(FSCol::Trigger),200);
    t->setColumnWidth(col(FSCol::Mods),   160);
    t->setColumnWidth(col(FSCol::Count),   55);
    t->setColumnWidth(col(FSCol::Remove),  70);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setAlternatingRowColors(true);
}

// ─────────────────────────────────────────────────────────────────
// APPEND ROW
// ─────────────────────────────────────────────────────────────────
void FrameSenderWindowLIN::appendBlankRow(int typeIndex)
{
    QTableWidget *t = ui->tableSender;
    int row = t->rowCount();
    t->insertRow(row);
    inhibitChanged = true;

    // Col En — checkbox
    QTableWidgetItem *chk = new QTableWidgetItem();
    chk->setFlags(chk->flags() | Qt::ItemIsUserCheckable);
    chk->setCheckState(Qt::Unchecked);
    t->setItem(row, col(FSCol::En), chk);

    // Col Type — QLabel (non modifiable)
    static const QStringList typeLabels = {
        "Request an in frame response",
        "Transmit message",
        "In frame response"
    };
    QLabel *lbl = new QLabel(typeLabels.value(typeIndex, "Unknown"));
    lbl->setProperty("typeIndex", typeIndex);
    lbl->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    lbl->setIndent(4);
    t->setCellWidget(row, col(FSCol::Type), lbl);

    // Colonnes texte
    auto addItem = [&](FSCol c, bool editable = true) {
        QTableWidgetItem *it = new QTableWidgetItem("");
        if (!editable) it->setFlags(it->flags() & ~Qt::ItemIsEditable);
        t->setItem(row, col(c), it);
    };
    addItem(FSCol::Bus);
    addItem(FSCol::Id);
    addItem(FSCol::MsgName, false);
    addItem(FSCol::Len);
    addItem(FSCol::Data);
    addItem(FSCol::Trigger);
    addItem(FSCol::Mods);
    addItem(FSCol::Count, false);

    // Col Remove — bouton ✕
    QPushButton *btnRm = new QPushButton("✕");
    btnRm->setFixedWidth(40);
    connect(btnRm, &QPushButton::clicked, this, &FrameSenderWindowLIN::removeRow);
    t->setCellWidget(row, col(FSCol::Remove), btnRm);

    // Style couleur
    refreshRowStyle(row);

    // Structure sendingData
    FrameSendData fsd;
    fsd.enabled = false; fsd.bus = 0;
    fsd.setFrameType(QCanBusFrame::DataFrame);
    fsd.setExtendedFrameFormat(false);
    fsd.setFrameId(0);
    sendingData.append(fsd);

    // SlaveData si besoin
    if (typeIndex == 2)
        while (slaveData.count() <= row) slaveData.append(SlaveResponse());

    inhibitChanged = false;
}
// ─────────────────────────────────────────────────────────────────
// MUTUAL EXCLUSION DES MODES
// ─────────────────────────────────────────────────────────────────
void FrameSenderWindowLIN::disableOtherModes(int activeRow, int activeType)
{
    if (m_disablingOtherModes) return;
    m_disablingOtherModes = true;

    QTableWidget *t = ui->tableSender;

    for (int i = 0; i < t->rowCount(); i++) {
        if (i == activeRow) continue;

        QLabel *lbl = qobject_cast<QLabel*>(t->cellWidget(i, col(FSCol::Type)));
        if (!lbl) continue;

        int otherType = lbl->property("typeIndex").toInt();

        if (otherType != activeType) {
            QTableWidgetItem *enItem = t->item(i, col(FSCol::En));
            if (enItem && enItem->checkState() == Qt::Checked) {
                inhibitChanged = true;
                enItem->setCheckState(Qt::Unchecked);
                inhibitChanged = false;

                if (otherType == 2) {
                    if (i < slaveData.count() && slaveData[i].enabled) {
                        removeSlaveResponse(slaveData[i].id);
                        slaveData[i].enabled = false;
                    }
                } else {
                    if (i < sendingData.count()) {
                        sendingData[i].enabled = false;
                    }
                }
            }
        }
    }

    m_disablingOtherModes = false;
}
// ─────────────────────────────────────────────────────────────────
// STYLE COULEUR PAR LIGNE
// ─────────────────────────────────────────────────────────────────
void FrameSenderWindowLIN::refreshRowStyle(int row)
{
    QTableWidget *t = ui->tableSender;
    QLabel *lbl = qobject_cast<QLabel*>(t->cellWidget(row, col(FSCol::Type)));
    int type = lbl ? lbl->property("typeIndex").toInt() : 0;

    QColor bg = (type == 0) ? COLOR_READ : (type == 1) ? COLOR_WRITE : COLOR_SLAVE;

    for (int c = 0; c < col(FSCol::NUM_COLS); c++) {
        QTableWidgetItem *it = t->item(row, c);
        if (it) it->setBackground(QBrush(bg));
    }

    auto grey = [&](FSCol c) {
        QTableWidgetItem *it = t->item(row, col(c));
        if (!it) return;
        it->setText("");
        it->setBackground(QBrush(Qt::lightGray));
        it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    };
    auto restore = [&](FSCol c) {
        QTableWidgetItem *it = t->item(row, col(c));
        if (!it) return;
        it->setBackground(QBrush(bg));
        it->setFlags(it->flags() | Qt::ItemIsEditable);
    };

    if (type == 0) {
        grey(FSCol::Len);
        grey(FSCol::Data);
        restore(FSCol::Bus); restore(FSCol::Trigger); restore(FSCol::Mods);
    } else if (type == 1) {
        restore(FSCol::Len); restore(FSCol::Data);
        restore(FSCol::Bus); restore(FSCol::Trigger); restore(FSCol::Mods);
    } else {
        grey(FSCol::Bus);
        grey(FSCol::MsgName);
        grey(FSCol::Trigger);
        grey(FSCol::Mods);
        grey(FSCol::Count);
        restore(FSCol::Len);
        restore(FSCol::Data);
    }
}

// ─────────────────────────────────────────────────────────────────
// ADD FRAME DIALOG
// ─────────────────────────────────────────────────────────────────
void FrameSenderWindowLIN::onAddFrameClicked()
{
    AddFrameDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    int type = dlg.frameType();
    int row  = ui->tableSender->rowCount();
    appendBlankRow(type);

    inhibitChanged = true;
    ui->tableSender->item(row, col(FSCol::Id))->setText(dlg.id());
    if (type >= 1)
        ui->tableSender->item(row, col(FSCol::Data))->setText(dlg.data());
    if (type < 2)
        ui->tableSender->item(row, col(FSCol::Trigger))
            ->setText(QString::number(dlg.period()) + "MS");
    inhibitChanged = false;

    // Forcer le parse des cellules avant d'activer
    processCellChange(row, col(FSCol::Id));
    if (type >= 1)
        processCellChange(row, col(FSCol::Data));
    if (type < 2)
        processCellChange(row, col(FSCol::Trigger));

    ui->tableSender->item(row, col(FSCol::En))->setCheckState(Qt::Checked);
}

// ─────────────────────────────────────────────────────────────────
// REMOVE ROW
// ─────────────────────────────────────────────────────────────────
void FrameSenderWindowLIN::removeRow()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    for (int i = 0; i < ui->tableSender->rowCount(); i++) {
        if (ui->tableSender->cellWidget(i, col(FSCol::Remove)) == btn) {
            QLabel *lbl = qobject_cast<QLabel*>(
                ui->tableSender->cellWidget(i, col(FSCol::Type)));

            QTableWidgetItem *idIt = ui->tableSender->item(i, col(FSCol::Id));
            if (idIt) {
                uint8_t id = (uint8_t)(Utility::ParseStringToNum(idIt->text()) & 0x3F);
                int rowType = lbl ? lbl->property("typeIndex").toInt() : 0;

                QTableWidgetItem *enIt = ui->tableSender->item(i, col(FSCol::En));
                if (enIt && enIt->checkState() == Qt::Checked) {
                    removeFrameFromDevice(id, rowType);
                }
            }

            if (lbl && lbl->property("typeIndex").toInt() == 2) {
                if (i < slaveData.count()) {
                    if (slaveData[i].enabled) {
                        removeSlaveResponse(slaveData[i].id);
                    }
                    slaveData.removeAt(i);
                }
            }

            ui->tableSender->removeRow(i);
            if (i < sendingData.count()) sendingData.removeAt(i);
            break;
        }
    }
}
// Retourne true si l'ID est publié par le master
bool FrameSenderWindowLIN::isPublishedByMaster(uint32_t id)
{
    db_MESSAGE *msg = dbHandler->findMessage(id);
    if (!msg) return false;

    for (int fi = 0; fi < dbHandler->getFileCount(); fi++) {
        dbFile *f = dbHandler->getFileByIdx(fi);
        if (!f) continue;
        for (int ni = 0; ni < f->db_nodes.count(); ni++) {
            db_NODE &node = f->db_nodes[ni];
            if (!node.isMaster) continue;
            if (msg->senderName == node.name) return true;
        }
    }
    return false;
}
// ─────────────────────────────────────────────────────────────────
// CELL CHANGED
// ─────────────────────────────────────────────────────────────────
void FrameSenderWindowLIN::onCellChanged(int row, int column)
{
    if (inhibitChanged) return;
    processCellChange(row, column);
}

void FrameSenderWindowLIN::onCellDoubleTap(int row, int column)
{
    if (column != col(FSCol::Trigger)) return;
    QLabel *lbl = qobject_cast<QLabel*>(
        ui->tableSender->cellWidget(row, col(FSCol::Type)));
    if (lbl && lbl->property("typeIndex").toInt() == 2) return;

    while (sendingData.count() <= row) {
        FrameSendData tmp; tmp.enabled = false;
        tmp.setFrameType(QCanBusFrame::DataFrame);
        tmp.setExtendedFrameFormat(false);
        sendingData.append(tmp);
    }
    sendData = &sendingData[row];
    td = new TriggerDialog(sendData->triggers);
    if (td->exec() == QDialog::Accepted) {
        sendData->triggers.clear();
        sendData->triggers.append(td->getUpdatedTriggers());
        QString out;
        foreach (Trigger t, sendData->triggers) out += td->buildEntry(t) + ",";
        out.chop(1);
        inhibitChanged = true;
        ui->tableSender->item(row, col(FSCol::Trigger))->setText(out);
        inhibitChanged = false;
    }
    delete td; td = nullptr;
}

void FrameSenderWindowLIN::processCellChange(int line, int c)
{
    QTableWidget *t = ui->tableSender;
    QLabel *typeLbl = qobject_cast<QLabel*>(t->cellWidget(line, col(FSCol::Type)));
    int rowType = typeLbl ? typeLbl->property("typeIndex").toInt() : 0;

    while (sendingData.count() <= line) {
        FrameSendData tmp; tmp.enabled = false; tmp.bus = 0;
        tmp.setFrameType(QCanBusFrame::DataFrame);
        tmp.setExtendedFrameFormat(false); tmp.setFrameId(0);
        sendingData.append(tmp);
    }
    sendingData[line].count = 0;

    int numBuses = CANConManager::getInstance()->getNumBuses();
    QByteArray arr;

    // ── Slave row ────────────────────────────────────────────────
    if (rowType == 2) {
        while (slaveData.count() <= line) slaveData.append(SlaveResponse());

        if (c == col(FSCol::En)) {
            bool checked = (t->item(line, col(FSCol::En))->checkState() == Qt::Checked);
            QTableWidgetItem *idIt   = t->item(line, col(FSCol::Id));
            QTableWidgetItem *dataIt = t->item(line, col(FSCol::Data));
            if (!idIt || !dataIt) return;
            bool ok;
            int idVal = idIt->text().toInt(&ok, 0);
            if (!ok || idVal < 0 || idVal > 0x3F) {
                if (checked) {
                    QMessageBox::warning(this, "Invalid ID",
                                         "LIN Slave ID must be 0x00–0x3F");
                    inhibitChanged = true;
                    t->item(line, col(FSCol::En))->setCheckState(Qt::Unchecked);
                    inhibitChanged = false;
                }
                return;
            }
            uint8_t id = (uint8_t)idVal;
            QStringList tokens = dataIt->text().split(' ', Qt::SkipEmptyParts);
            QByteArray data;
            for (int i = 0; i < tokens.count() && i < 8; i++) {
                int v = tokens[i].toInt(&ok, 16);
                if (ok && v >= 0 && v <= 0xFF) data.append((uint8_t)v);
            }
            if (checked) {
                //disableOtherModes(line, 2);
                if (slaveData[line].enabled && slaveData[line].id != id) {
                    removeSlaveResponse(slaveData[line].id);
                }
                sendSlaveModeCommand();
                slaveData[line] = { true, id, (uint8_t)data.size(), data };
                sendSlaveResponse(line);
            } else {
                if (slaveData[line].enabled) {
                    removeSlaveResponse(slaveData[line].id);
                }
                slaveData[line] = { false, id, (uint8_t)data.size(), data };
            }
        }
        else if (c == col(FSCol::Id)) {
            QTableWidgetItem *idIt = t->item(line, col(FSCol::Id));
            if (!idIt || idIt->text().isEmpty()) return;
            bool ok;
            int idVal = idIt->text().toInt(&ok, 0);
            if (!ok || idVal < 0 || idVal > 0x3F) return;

            db_MESSAGE *msg = dbHandler->findMessage((uint32_t)idVal);

            if (msg) {
                inhibitChanged = true;
                if (t->item(line, col(FSCol::MsgName)))
                    t->item(line, col(FSCol::MsgName))->setText(msg->name);
                if (t->item(line, col(FSCol::Len)))
                    t->item(line, col(FSCol::Len))->setText(QString::number(msg->len));

                QByteArray dataBytes(msg->len, 0x00);
                for (int j = 0; j < msg->sigHandler->getCount(); j++) {
                    DB_SIGNAL *sig = msg->sigHandler->findSignalByIdx(j);
                    if (!sig) continue;
                    if (!sig->initValueRaw.isEmpty() &&
                        sig->initValueRaw.trimmed().startsWith('{')) {
                        QString raw = sig->initValueRaw;
                        raw.remove('{').remove('}');
                        QStringList parts = raw.split(',', Qt::SkipEmptyParts);
                        int startByte = sig->startBit / 8;
                        for (int p = 0; p < parts.count() &&
                                        (startByte + p) < dataBytes.size(); p++) {
                            bool okB;
                            uint8_t byteVal = (uint8_t)parts[p].trimmed().toUInt(&okB, 0);
                            if (okB) dataBytes[startByte + p] = byteVal;
                        }
                        continue;
                    }
                    uint64_t initRaw = (uint64_t)sig->initValue;
                    if (sig->intelByteOrder) {
                        int bitPos = sig->startBit, bitsLeft = sig->signalSize;
                        uint64_t val = initRaw;
                        while (bitsLeft > 0 && bitPos < (int)(msg->len * 8)) {
                            int byteIdx = bitPos / 8, bitInByte = bitPos % 8;
                            int bitsInThis = qMin(8 - bitInByte, bitsLeft);
                            uint8_t mask = (uint8_t)((1 << bitsInThis) - 1);
                            if (byteIdx < dataBytes.size())
                                dataBytes[byteIdx] = (uint8_t)(
                                    ((uint8_t)dataBytes[byteIdx] & ~(mask << bitInByte)) |
                                    (((uint8_t)(val & mask)) << bitInByte));
                            val >>= bitsInThis; bitPos += bitsInThis; bitsLeft -= bitsInThis;
                        }
                    } else {
                        int bitPos = sig->startBit, bitsLeft = sig->signalSize;
                        uint64_t val = initRaw << (64 - sig->signalSize);
                        while (bitsLeft > 0) {
                            int byteIdx = bitPos / 8, bitInByte = 7 - (bitPos % 8);
                            if (byteIdx >= 0 && byteIdx < dataBytes.size()) {
                                if (val & (1ULL << 63))
                                    dataBytes[byteIdx] = (uint8_t)dataBytes[byteIdx] | (1 << bitInByte);
                                else
                                    dataBytes[byteIdx] = (uint8_t)dataBytes[byteIdx] & ~(1 << bitInByte);
                            }
                            val <<= 1; bitPos++; bitsLeft--;
                        }
                    }
                }
                QString dataStr;
                for (int b = 0; b < dataBytes.size(); b++) {
                    dataStr += QString::number((uint8_t)dataBytes[b], 16).toUpper().rightJustified(2, '0');
                    if (b < dataBytes.size() - 1) dataStr += " ";
                }
                if (t->item(line, col(FSCol::Data)))
                    t->item(line, col(FSCol::Data))->setText(dataStr);
                slaveData[line].id   = (uint8_t)idVal;
                slaveData[line].dlc  = (uint8_t)dataBytes.size();
                slaveData[line].data = dataBytes;
                inhibitChanged = false;
            }
            if (slaveData[line].enabled) {
                removeSlaveResponse(slaveData[line].id);
                slaveData[line].enabled = false;
                inhibitChanged = true;
                t->item(line, col(FSCol::En))->setCheckState(Qt::Unchecked);
                inhibitChanged = false;
            }
        }
        else if (c == col(FSCol::Data) && slaveData[line].enabled) {
            removeSlaveResponse(slaveData[line].id);
            QTableWidgetItem *idIt   = t->item(line, col(FSCol::Id));
            QTableWidgetItem *dataIt = t->item(line, col(FSCol::Data));
            bool ok;
            int idVal = idIt ? idIt->text().toInt(&ok, 0) : -1;
            uint8_t newId = (ok && idVal >= 0 && idVal <= 0x3F) ? (uint8_t)idVal : 0;
            QStringList tokens = dataIt ? dataIt->text().split(' ', Qt::SkipEmptyParts) : QStringList();
            QByteArray data;
            for (int i = 0; i < tokens.count() && i < 8; i++) {
                int v = tokens[i].toInt(&ok, 16);
                if (ok && v >= 0 && v <= 0xFF) data.append((uint8_t)v);
            }
            slaveData[line] = { false, newId, (uint8_t)data.size(), data };
            inhibitChanged = true;
            t->item(line, col(FSCol::En))->setCheckState(Qt::Unchecked);
            inhibitChanged = false;
        }
        return;
    }

    // ── Master rows (Read / Write) ───────────────────────────────
    FSCol fc = static_cast<FSCol>(c);
    switch (fc) {
    case FSCol::En: {
        bool checked = (t->item(line, col(FSCol::En))->checkState() == Qt::Checked);
        if (checked) {
            QTableWidgetItem *idIt   = t->item(line, col(FSCol::Id));
            QTableWidgetItem *dataIt = t->item(line, col(FSCol::Data));
            if (!idIt || !dataIt) return;
            bool ok;
            int idVal = idIt->text().toInt(&ok, 0);
            if (!ok || idVal < 0 || idVal > 0x3F) {
                if (checked) {
                    QMessageBox::warning(this, "Invalid ID",
                                         "LIN Slave ID must be 0x00–0x3F");
                    inhibitChanged = true;
                    t->item(line, col(FSCol::En))->setCheckState(Qt::Unchecked);
                    inhibitChanged = false;
                }
                return;
            }
            uint8_t id = (uint8_t)idVal;   // ← id disponible dès ici

            /* Conflit ID uniquement */
            for (int i = 0; i < t->rowCount(); i++) {
                if (i == line) continue;
                QTableWidgetItem *otherId = t->item(i, col(FSCol::Id));
                QTableWidgetItem *otherEn = t->item(i, col(FSCol::En));
                if (!otherId || !otherEn) continue;
                if (otherEn->checkState() != Qt::Checked) continue;
                bool otherOk;
                int otherIdVal = otherId->text().toInt(&otherOk, 0);
                if (!otherOk || otherIdVal != (int)id) continue;

                QLabel *otherLbl = qobject_cast<QLabel*>(
                    t->cellWidget(i, col(FSCol::Type)));
                int otherType = otherLbl ? otherLbl->property("typeIndex").toInt() : 0;

                inhibitChanged = true;
                otherEn->setCheckState(Qt::Unchecked);
                inhibitChanged = false;

                removeFrameFromDevice((uint8_t)(otherIdVal & 0x3F), otherType);
                if (i < sendingData.count()) sendingData[i].enabled = false;
                if (otherType == 2 && i < slaveData.count())
                    slaveData[i].enabled = false;
            }

            sendingData[line].enabled = true;

            if (sendingData[line].triggers.isEmpty()) {
                QTableWidgetItem *trigItem = t->item(line, col(FSCol::Trigger));
                if (trigItem && !trigItem->text().isEmpty()) {
                    processTriggerText(line);
                } else {
                    Trigger tr;
                    tr.bus = -1; tr.ID = 0; tr.maxCount = -1;
                    tr.milliseconds = 1000;
                    tr.currCount = 0; tr.msCounter = 0;
                    tr.triggerMask = TriggerMask::TRG_MS;
                    tr.readyCount = true;
                    sendingData[line].triggers.append(tr);
                }
            }
            pushRowToFirmware(line);

        } else {
            sendingData[line].enabled = false;
            QTableWidgetItem *idItem = t->item(line, col(FSCol::Id));
            if (idItem && !idItem->text().isEmpty()) {
                bool ok;
                int v = idItem->text().toInt(&ok, 0);
                if (ok) removeFrameFromDevice((uint8_t)(v & 0x3F), rowType);
            }
        }
        break;
    }

    case FSCol::Bus: {
        int v = Utility::ParseStringToNum(t->item(line, col(FSCol::Bus))->text());
        sendingData[line].bus = qBound(-1, v, numBuses - 1);
        break;
    }

    case FSCol::Id: {
        if (sendingData[line].enabled) {
            // Supprimer l'ancien frame avec l'ancien ID
            uint8_t oldId = (uint8_t)(sendingData[line].frameId() & 0x3F);
            removeFrameFromDevice(oldId, rowType);
            sendingData[line].enabled = false;
            inhibitChanged = true;
            t->item(line, col(FSCol::En))->setCheckState(Qt::Unchecked);
            inhibitChanged = false;
        }
        int v = Utility::ParseStringToNum(t->item(line, col(FSCol::Id))->text());
        sendingData[line].setFrameId((uint32_t)qBound(0, v, 0x3F));
        sendingData[line].setExtendedFrameFormat(false);

        QLabel *typeLbl2 = qobject_cast<QLabel*>(t->cellWidget(line, col(FSCol::Type)));
        int rowType2 = typeLbl2 ? typeLbl2->property("typeIndex").toInt() : 0;

        db_MESSAGE *msg = dbHandler->findMessage(sendingData[line].frameId());
        if (msg) {
            bool masterPublished = isPublishedByMaster(sendingData[line].frameId());

            if (rowType2 == 1 && !masterPublished) {
                QMessageBox::warning(this, "Wrong ID for Write mode",
                                     QString("ID 0x%1 (%2) is published by '%3' (Slave).\n"
                                             "Write mode requires a Master-published ID.\n"
                                             "Use Read mode to receive this frame.")
                                         .arg(v, 0, 16).arg(msg->name).arg(msg->senderName));
                inhibitChanged = true;
                t->item(line, col(FSCol::Id))->setText("");
                t->item(line, col(FSCol::MsgName))->setText("");
                t->item(line, col(FSCol::Len))->setText("");
                t->item(line, col(FSCol::Data))->setText("");
                t->item(line, col(FSCol::En))->setCheckState(Qt::Unchecked);
                inhibitChanged = false;
                sendingData[line].enabled = false;
                sendingData[line].setFrameId(0);
                return;
            }
            else if (rowType2 == 0 && masterPublished) {
                QMessageBox::warning(this, "Wrong ID for Read mode",
                                     QString("ID 0x%1 (%2) is published by '%3' (Master).\n"
                                             "Read mode requires a Slave-published ID.\n"
                                             "Use Write mode to send this frame.")
                                         .arg(v, 0, 16).arg(msg->name).arg(msg->senderName));
                inhibitChanged = true;
                t->item(line, col(FSCol::Id))->setText("");
                t->item(line, col(FSCol::MsgName))->setText("");
                t->item(line, col(FSCol::Len))->setText("");
                t->item(line, col(FSCol::Data))->setText("");
                t->item(line, col(FSCol::En))->setCheckState(Qt::Unchecked);
                inhibitChanged = false;
                sendingData[line].enabled = false;
                sendingData[line].setFrameId(0);
                return;
            }

            inhibitChanged = true;
            t->item(line, col(FSCol::MsgName))->setText(msg->name);
            t->item(line, col(FSCol::Len))->setText(QString::number(msg->len));

            if (rowType2 == 1 || rowType2 == 2) {
                QByteArray dataBytes(msg->len, 0x00);
                for (int j = 0; j < msg->sigHandler->getCount(); j++) {
                    DB_SIGNAL *sig = msg->sigHandler->findSignalByIdx(j);
                    if (!sig) continue;
                    if (!sig->initValueRaw.isEmpty() &&
                        sig->initValueRaw.trimmed().startsWith('{')) {
                        QString raw = sig->initValueRaw;
                        raw.remove('{').remove('}');
                        QStringList parts = raw.split(',', Qt::SkipEmptyParts);
                        int startByte = sig->startBit / 8;
                        for (int p = 0; p < parts.count() &&
                                        (startByte + p) < dataBytes.size(); p++) {
                            bool okB;
                            uint8_t byteVal = (uint8_t)parts[p].trimmed().toUInt(&okB, 0);
                            if (okB) dataBytes[startByte + p] = byteVal;
                        }
                        continue;
                    }
                    uint64_t initRaw = (uint64_t)sig->initValue;
                    if (sig->intelByteOrder) {
                        int bitPos = sig->startBit, bitsLeft = sig->signalSize;
                        uint64_t val = initRaw;
                        while (bitsLeft > 0 && bitPos < (int)(msg->len * 8)) {
                            int byteIdx = bitPos / 8, bitInByte = bitPos % 8;
                            int bitsInThis = qMin(8 - bitInByte, bitsLeft);
                            uint8_t mask = (uint8_t)((1 << bitsInThis) - 1);
                            if (byteIdx < dataBytes.size())
                                dataBytes[byteIdx] = (uint8_t)(
                                    ((uint8_t)dataBytes[byteIdx] & ~(mask << bitInByte)) |
                                    (((uint8_t)(val & mask)) << bitInByte));
                            val >>= bitsInThis; bitPos += bitsInThis; bitsLeft -= bitsInThis;
                        }
                    } else {
                        int bitPos = sig->startBit, bitsLeft = sig->signalSize;
                        uint64_t val = initRaw << (64 - sig->signalSize);
                        while (bitsLeft > 0) {
                            int byteIdx = bitPos / 8, bitInByte = 7 - (bitPos % 8);
                            if (byteIdx >= 0 && byteIdx < dataBytes.size()) {
                                if (val & (1ULL << 63))
                                    dataBytes[byteIdx] = (uint8_t)dataBytes[byteIdx] | (1 << bitInByte);
                                else
                                    dataBytes[byteIdx] = (uint8_t)dataBytes[byteIdx] & ~(1 << bitInByte);
                            }
                            val <<= 1; bitPos++; bitsLeft--;
                        }
                    }
                }
                QString dataStr;
                for (int b = 0; b < dataBytes.size(); b++) {
                    dataStr += QString::number((uint8_t)dataBytes[b], 16).toUpper().rightJustified(2, '0');
                    if (b < dataBytes.size() - 1) dataStr += " ";
                }
                if (t->item(line, col(FSCol::Data)))
                    t->item(line, col(FSCol::Data))->setText(dataStr);
                sendingData[line].setPayload(dataBytes);
            }
            inhibitChanged = false;
        }
        break;
    }

    case FSCol::Len: {
        int v = Utility::ParseStringToNum(t->item(line, col(FSCol::Len))->text());
        v = qBound(0, v, 8);
        if (sendingData[line].payload().size() != v) {
            arr = sendingData[line].payload();
            arr.resize(v);
            sendingData[line].setPayload(arr);
        }
        break;
    }

    case FSCol::Data: {
        if (sendingData[line].enabled) {
            uint8_t oldId = (uint8_t)(sendingData[line].frameId() & 0x3F);
            removeFrameFromDevice(oldId, rowType);
            sendingData[line].enabled = false;
            inhibitChanged = true;
            t->item(line, col(FSCol::En))->setCheckState(Qt::Unchecked);
            inhibitChanged = false;
        }
        QStringList toks = t->item(line, col(FSCol::Data))
                               ->text().split(' ', Qt::SkipEmptyParts);
        arr.clear();
        for (int j = 0; j < toks.count() && j < 8; j++)
            arr.append((uint8_t)Utility::ParseStringToNum(toks[j]));
        sendingData[line].setPayload(arr);
        // ── SUPPRIMER : if (sendingData[line].enabled) pushRowToFirmware(line);
        inhibitChanged = true;
        if (t->item(line, col(FSCol::Len)))
            t->item(line, col(FSCol::Len))->setText(QString::number(arr.size()));
        inhibitChanged = false;
        break;
    }

    case FSCol::Trigger: {
        // ── AJOUT : si la ligne est active, décocher et supprimer ──
        if (sendingData[line].enabled) {
            uint8_t oldId = (uint8_t)(sendingData[line].frameId() & 0x3F);
            removeFrameFromDevice(oldId, rowType);
            sendingData[line].enabled = false;
            inhibitChanged = true;
            t->item(line, col(FSCol::En))->setCheckState(Qt::Unchecked);
            inhibitChanged = false;
        }
        processTriggerText(line);
        // ── SUPPRIMER cette ligne qui re-push automatiquement ──
        // if (sendingData[line].enabled)   <-- plus jamais vrai ici
        //     pushRowToFirmware(line);
        break;}
    case FSCol::Mods: processModifierText(line); break;
    default: break;
    }
}

// ─────────────────────────────────────────────────────────────────
// GRID BUTTONS
// ─────────────────────────────────────────────────────────────────
void FrameSenderWindowLIN::enableAll()
{
    for (int i = 0; i < ui->tableSender->rowCount(); i++)
        ui->tableSender->item(i, col(FSCol::En))->setCheckState(Qt::Checked);
}

void FrameSenderWindowLIN::disableAll()
{
    for (int i = 0; i < ui->tableSender->rowCount(); i++)
        ui->tableSender->item(i, col(FSCol::En))->setCheckState(Qt::Unchecked);
}

void FrameSenderWindowLIN::clearGrid()
{
    // ── CORRECTION 5 : Désactiver toutes les réponses slave actives ──
    for (int i = 0; i < slaveData.count(); i++) {
        if (slaveData[i].enabled) {
            removeSlaveResponse(slaveData[i].id);
        }
    }

    for (int i = ui->tableSender->rowCount() - 1; i >= 0; i--) {
        if (i < sendingData.count()) {
            sendingData[i].enabled = false;
            sendingData.removeAt(i);
        }
        ui->tableSender->removeRow(i);
    }
    slaveData.clear();
}

// ─────────────────────────────────────────────────────────────────
// SAVE / LOAD
// ─────────────────────────────────────────────────────────────────
void FrameSenderWindowLIN::saveGrid()
{
    QFileDialog dlg(this); QSettings settings;
    dlg.setDirectory(settings.value("FrameSender/Dir", dlg.directory().path()).toString());
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setNameFilters({tr("Frame Sender Definition (*.fsd)")});
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    if (dlg.exec() == QDialog::Accepted) {
        QString fn = dlg.selectedFiles()[0];
        if (!fn.contains('.')) fn += ".fsd";
        saveSenderFile(fn);
        settings.setValue("FrameSender/Dir", dlg.directory().path());
    }
}

void FrameSenderWindowLIN::loadGrid()
{
    QFileDialog dlg(this); QSettings settings;
    dlg.setDirectory(settings.value("FrameSender/Dir", dlg.directory().path()).toString());
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setNameFilters({tr("Frame Sender Definition (*.fsd)")});
    if (dlg.exec() == QDialog::Accepted) {
        loadSenderFile(dlg.selectedFiles()[0]);
        settings.setValue("FrameSender/Dir", dlg.directory().path());
    }
}

void FrameSenderWindowLIN::saveSenderFile(QString filename)
{
    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTableWidget *t = ui->tableSender;
    for (int r = 0; r < t->rowCount(); r++) {
        QLabel *lbl = qobject_cast<QLabel*>(t->cellWidget(r, col(FSCol::Type)));
        int type = lbl ? lbl->property("typeIndex").toInt() : 0;
        QString line;
        line  = (t->item(r, col(FSCol::En))->checkState() == Qt::Checked) ? "T#" : "F#";
        line += QString::number(type)                           + "#";
        line += t->item(r, col(FSCol::Bus))->text()     + "#";
        line += t->item(r, col(FSCol::Id))->text()      + "#";
        line += t->item(r, col(FSCol::MsgName))->text() + "#";
        line += t->item(r, col(FSCol::Len))->text()     + "#";
        line += t->item(r, col(FSCol::Data))->text()    + "#";
        line += t->item(r, col(FSCol::Trigger))->text() + "#";
        line += t->item(r, col(FSCol::Mods))->text()    + "\n";
        f.write(line.toUtf8());
    }
    f.close();
}

void FrameSenderWindowLIN::loadSenderFile(QString filename)
{
    QFile *inFile = new QFile(filename);
    if (!inFile->open(QIODevice::ReadOnly | QIODevice::Text)) { delete inFile; return; }
    clearGrid();
    while (!inFile->atEnd()) {
        QByteArray line = inFile->readLine().simplified();
        if (line.length() < 3) continue;
        QList<QByteArray> tok = line.split('#');
        if (tok.length() < 9) continue;
        bool enabled = (tok[0] == "T");
        int  type    = QString(tok[1]).toInt();
        appendBlankRow(type);
        int row = ui->tableSender->rowCount() - 1;
        inhibitChanged = true;
        ui->tableSender->item(row, col(FSCol::Bus))->setText(QString(tok[2]));
        ui->tableSender->item(row, col(FSCol::Id))->setText(QString(tok[3]));
        ui->tableSender->item(row, col(FSCol::MsgName))->setText(QString(tok[4]));
        ui->tableSender->item(row, col(FSCol::Len))->setText(QString(tok[5]));
        ui->tableSender->item(row, col(FSCol::Data))->setText(QString(tok[6]));
        ui->tableSender->item(row, col(FSCol::Trigger))->setText(QString(tok[7]));
        ui->tableSender->item(row, col(FSCol::Mods))->setText(QString(tok[8]));
        inhibitChanged = false;
        ui->tableSender->item(row, col(FSCol::En))
            ->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
        for (int c = 0; c < col(FSCol::Remove); c++) processCellChange(row, c);
    }
    inFile->close();
    delete inFile;
}

// ─────────────────────────────────────────────────────────────────
// LIN SLAVE PROTOCOL
// ─────────────────────────────────────────────────────────────────
void FrameSenderWindowLIN::sendSlaveModeCommand()
{
    //static bool slaveModeActivated = false;  // ← AJOUT : flag local

    // Optionnel : n'envoyer qu'une fois par session, ou toujours envoyer
    // Si le device nécessite un envoi à chaque fois, retire la condition
    //if (slaveModeActivated) return;

    QByteArray p; p.append((char)0x04);
    CANConManager::getInstance()->sendBinaryCommand(TYPE_LIN_CMD, p);
    QThread::msleep(200);

    //slaveModeActivated = true;  // ← AJOUT

    // Ré-envoyer toutes les réponses slave actives
    for (int i = 0; i < slaveData.count(); i++)
        if (slaveData[i].enabled) sendSlaveResponse(i);
}

void FrameSenderWindowLIN::sendSlaveResponse(int row)
{
    if (row < 0 || row >= slaveData.count() || !slaveData[row].enabled) return;
    SlaveResponse &r = slaveData[row];
    QByteArray p;
    p.append((char)0x05);
    p.append((char)(r.id & 0x3F));
    p.append((char)r.dlc);
    p.append(r.data.left(8));
    CANConManager::getInstance()->sendBinaryCommand(TYPE_LIN_CMD, p);
}

void FrameSenderWindowLIN::removeSlaveResponse(uint8_t id)
{
    QByteArray p;
    p.append((char)0xF6);
    p.append((char)(id & 0x3F));
    CANConManager::getInstance()->sendBinaryCommand(TYPE_LIN_CMD, p);
}

// ─────────────────────────────────────────────────────────────────
// LIN MASTER SEND
// ─────────────────────────────────────────────────────────────────
void FrameSenderWindowLIN::onLinStartClickedForRow(int i)
{
    if (i < 0 || i >= sendingData.count() || !sendingData[i].enabled) return;

    QLabel *lbl = qobject_cast<QLabel*>(
        ui->tableSender->cellWidget(i, col(FSCol::Type)));
    int rowMode = lbl ? lbl->property("typeIndex").toInt() : 0;
    if (rowMode == 2) return;  // Slave géré ailleurs

    uint8_t id  = (uint8_t)(sendingData[i].frameId() & 0x3F);
    uint8_t dlc = (uint8_t)qMin((int)sendingData[i].payload().size(), 8);

    // Extraire la période depuis le trigger
    uint16_t period_ms = 1000;  // défaut
    QTableWidgetItem *trigItem = ui->tableSender->item(i, col(FSCol::Trigger));
    if (trigItem && !trigItem->text().isEmpty()) {
        QRegularExpression re("(\\d+)\\s*MS");
        QRegularExpressionMatch match = re.match(trigItem->text().toUpper());
        if (match.hasMatch()) period_ms = (uint16_t)match.captured(1).toInt();
    }

    QByteArray p;
    if (rowMode == 0) {
        // READ mode : [0x06][id][period_L][period_H]
        p.append((char)0x06);
        p.append((char)id);
        p.append((char)(period_ms & 0xFF));
        p.append((char)((period_ms >> 8) & 0xFF));
    } else {
        // WRITE mode : [0x07][id][dlc][data...][period_L][period_H]
        p.append((char)0x07);
        p.append((char)id);
        p.append((char)dlc);
        p.append(sendingData[i].payload().left(dlc));
        p.append((char)(period_ms & 0xFF));
        p.append((char)((period_ms >> 8) & 0xFF));
    }
    CANConManager::getInstance()->sendBinaryCommand(TYPE_LIN_CMD, p);
}

// ─────────────────────────────────────────────────────────────────
// TICK TIMER
// ─────────────────────────────────────────────────────────────────
void FrameSenderWindowLIN::handleTick()
{
    QList<LINFrame> sendList;
    QList<LINFrame> linTxList;
    if (!mutex.tryLock()) return;

    int elapsed = elapsedTimer.restart();
    if (elapsed == 0) elapsed = 1;

    for (int i = 0; i < sendingData.count(); i++) {
        FrameSendData *sd = &sendingData[i];
        if (!sd->enabled) { for (auto &tr : sd->triggers) tr.currCount = 0; continue; }
        if (sd->triggers.isEmpty()) continue;

        QLabel *lbl = qobject_cast<QLabel*>(
            ui->tableSender->cellWidget(i, col(FSCol::Type)));
        int rowMode = lbl ? lbl->property("typeIndex").toInt() : 0;
        bool isLin = (rowMode < 2);

        if (isLin) {
            if (rowMode == 1 && sd->payload().isEmpty()) continue;
            QTableWidgetItem *idItem = ui->tableSender->item(i, col(FSCol::Id));
            if (!idItem || idItem->text().isEmpty()) continue;
        }

        for (auto &tr : sd->triggers) {
            if (tr.maxCount > 0 && tr.currCount >= tr.maxCount) continue;
            if (!tr.readyCount) continue;
            tr.msCounter += elapsed;
            if (tr.msCounter >= tr.milliseconds) {
                tr.msCounter = 0; sd->count++; tr.currCount++;
                doModifiers(i); updateGridRow(i);

                if (!isLin) {
                    sendList.append(*sd);
                } else {
                    if (rowMode == 0) {
                        // ═══════════════════════════════════════════════
                        // CORRECTION : Ne RIEN envoyer en mode READ
                        // Le firmware gère la période, pas nous !
                        // On affiche juste un log pour debug
                        // ═══════════════════════════════════════════════
                        qDebug() << "[handleTick] LIN Read: ID=0x"
                                 << QString::number(sd->frameId() & 0x3F, 16)
                                 << "- firmware handles periodicity";
                    } else {
                        // Mode WRITE : on envoie la trame one-shot
                        // (ou on pourrait aussi laisser le firmware gérer)
                        LINFrame linTx = *sd;
                        linTx.setFrameId(sd->frameId() & 0x3F);
                        linTx.setExtendedFrameFormat(false);
                        linTx.setFrameType(QCanBusFrame::DataFrame);
                        linTx.bus = sd->bus;
                        linTx.isReceived = false;
                        linTxList.append(linTx);
                        qDebug() << "[handleTick] LIN Write TX: ID=0x"
                                 << QString::number(linTx.frameId(), 16);
                    }
                }

                if (tr.ID > 0) tr.readyCount = false;
            }
        }
    }

    if (!sendList.isEmpty())
        CANConManager::getInstance()->sendFrames(sendList);

   /* if (!linTxList.isEmpty()) {
        foreach (const LINFrame &f, linTxList)
        CANConManager::getInstance()->injectTxFrameForDisplay(f);
    }*/

    mutex.unlock();
}
// ─────────────────────────────────────────────────────────────────
// FRAME CACHE / INCOMING
// ─────────────────────────────────────────────────────────────────
void FrameSenderWindowLIN::buildFrameCache()
{
    frameCache.clear();
    for (const LINFrame &f : *modelFrames) frameCache[f.frameId()] = f;
}

void FrameSenderWindowLIN::updatedFrames(int numFrames)
{
    if (numFrames == -2) { buildFrameCache(); return; }
    if (numFrames > 0 && numFrames <= modelFrames->count()) {
        for (int i = modelFrames->count() - numFrames; i < modelFrames->count(); i++) {
            LINFrame f = modelFrames->at(i);
            frameCache[f.frameId()] = f;
            processIncomingFrame(&f);
        }
    }
}

void FrameSenderWindowLIN::processIncomingFrame(LINFrame *frame)
{
    for (int sd = 0; sd < sendingData.count(); sd++) {
        if (!sendingData[sd].enabled || sendingData[sd].triggers.isEmpty()) continue;
        for (auto &t : sendingData[sd].triggers) {
            if (!(t.triggerMask & (TriggerMask::TRG_BUS | TriggerMask::TRG_ID))) continue;
            bool ok = true;
            if ((t.triggerMask & TriggerMask::TRG_BUS) && t.bus != frame->bus) ok = false;
            if ((t.triggerMask & TriggerMask::TRG_ID)  && (uint32_t)t.ID != frame->frameId()) ok = false;
            if ((t.triggerMask & TriggerMask::TRG_COUNT) && t.currCount >= t.maxCount) ok = false;
            if (ok && (t.triggerMask & TriggerMask::TRG_SIGNAL)) {
                bool sigOk = false;
                db_MESSAGE *msg = dbHandler->findMessage(t.ID);
                if (msg) {
                    DB_SIGNAL *sig = msg->sigHandler->findSignalByName(t.sigName);
                    if (sig && sig->parentMessage && sig->parentMessage->ID == frame->frameId()) {
                        sigOk = true;
                        if (t.triggerMask & TriggerMask::TRG_SIGVAL) {
                            double v = 0;
                            if (sig->processAsDouble(*frame, v)) {
                                if (qAbs(v - t.sigValueDbl) > 0.001) sigOk = false;
                            } else sigOk = false;
                        }
                    }
                }
                ok &= sigOk;
            }
            if (ok) {
                if (t.milliseconds == 0) {
                    t.currCount++; sendingData[sd].count++;
                    doModifiers(sd); updateGridRow(sd);
                    CANConManager::getInstance()->sendFrame(sendingData[sd]);
                } else {
                    t.readyCount = true;
                }
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────
// MODIFIERS / TRIGGERS
// ─────────────────────────────────────────────────────────────────
void FrameSenderWindowLIN::processTriggerText(int line)
{
    QString trigger = ui->tableSender->item(line, col(FSCol::Trigger))->text().toUpper();
    sendingData[line].triggers.clear();
    if (trigger.isEmpty()) {
        Trigger t; t.bus=-1; t.ID=0; t.maxCount=1; t.milliseconds=10;
        t.currCount=0; t.msCounter=0; t.triggerMask=TriggerMask::TRG_MS; t.readyCount=true;
        sendingData[line].triggers.append(t);
        return;
    }
    for (const QString &entry : trigger.split(',')) {
        Trigger t; t.bus=-1; t.ID=-1; t.maxCount=-1; t.milliseconds=-1;
        t.currCount=0; t.msCounter=0; t.triggerMask=0; t.readyCount=true;
        for (const QString &tok : entry.split(' ')) {
            if (tok.left(2) == "ID") {
                t.ID = Utility::ParseStringToNum(tok.mid(2));
                if (t.milliseconds == -1) t.milliseconds = 0;
                t.readyCount = false; t.triggerMask |= TriggerMask::TRG_ID;
            } else if (tok.endsWith("MS")) {
                t.milliseconds = Utility::ParseStringToNum(tok.left(tok.length()-2));
                t.triggerMask |= TriggerMask::TRG_MS;
            } else if (tok.endsWith("X")) {
                t.maxCount = Utility::ParseStringToNum(tok.left(tok.length()-1));
                t.triggerMask |= TriggerMask::TRG_COUNT;
                if (t.milliseconds == -1) { t.milliseconds=10; t.triggerMask|=TriggerMask::TRG_ID; }
            } else if (tok.startsWith("BUS")) {
                t.bus = Utility::ParseStringToNum(tok.mid(3));
                t.triggerMask |= TriggerMask::TRG_BUS;
            }
        }
        sendingData[line].triggers.append(t);
    }
}

void FrameSenderWindowLIN::processModifierText(int line)
{
    QString m = ui->tableSender->item(line, col(FSCol::Mods))->text()
    .toUpper().trimmed()
        .replace("AND","&").replace("XOR","^").replace("OR","|").replace(" ","");
    sendingData[line].modifiers.clear();
    if (m.isEmpty()) return;

    for (const QString &modEntry : m.split(',')) {
        Modifier thisMod; thisMod.destByte = 0;
        QString me = modEntry;
        QRegularExpression rx("^\\[(\\w+)]=(\\w+)");
        QRegularExpressionMatch match = rx.match(me);
        if (match.hasMatch()) {
            thisMod.destByte = -1; thisMod.signalName = match.captured(1); me = match.captured(2);
        } else {
            QString left = Utility::grabAlphaNumeric(me);
            if (left.startsWith("D") && left.length() == 2) {
                thisMod.destByte = left.right(1).toInt();
                if (Utility::grabOperation(me) != "=") continue;
            }
        }
        ModifierOp op;
        QString token = Utility::grabAlphaNumeric(me);
        op.first.notOper = (!token.isEmpty() && token[0] == '~');
        if (op.first.notOper) token.remove(0, 1);
        parseOperandString(token.split(":"), op.first);
        if (me.length() < 2) {
            op.operation = ADDITION; op.second.ID = 0; op.second.databyte = 0; op.second.notOper = false;
            thisMod.operations.append(op);
        }
        while (me.length() >= 2) {
            QString oper = Utility::grabOperation(me);
            if (oper.isEmpty()) break;
            op.operation = parseOperation(oper);
            QString sec = Utility::grabAlphaNumeric(me);
            op.second.notOper = (!sec.isEmpty() && sec[0] == '~');
            if (op.second.notOper) sec.remove(0, 1);
            op.second.bus = sendingData[line].bus;
            op.second.ID  = sendingData[line].frameId();
            parseOperandString(sec.split(":"), op.second);
            thisMod.operations.append(op);
            op.first.ID = -1;
        }
        sendingData[line].modifiers.append(thisMod);
    }
}

void FrameSenderWindowLIN::parseOperandString(QStringList tokens, ModifierOperand &op)
{
    op.bus = -1; op.ID = -2; op.databyte = 0;
    for (int i = 0; i < tokens.length(); i++) {
        if (tokens[i] == "BUS")     op.bus  = Utility::ParseStringToNum(tokens[++i]);
        else if (tokens[i] == "ID") op.ID   = Utility::ParseStringToNum(tokens[++i]);
        else if (tokens[i].length() == 2 && tokens[i].startsWith("D"))
            op.databyte = Utility::ParseStringToNum(tokens[i].mid(1));
        else { op.databyte = Utility::ParseStringToNum(tokens[i]); op.ID = 0; }
    }
}

ModifierOperationType FrameSenderWindowLIN::parseOperation(QString op)
{
    if (op=="+") return ADDITION;    if (op=="-") return SUBTRACTION;
    if (op=="*") return MULTIPLICATION; if (op=="/") return DIVISION;
    if (op=="&") return AND;         if (op=="|") return OR;
    if (op=="^") return XOR;         if (op=="%") return MOD;
    return ADDITION;
}

void FrameSenderWindowLIN::doModifiers(int idx)
{
    FrameSendData *sd = &sendingData[idx];
    if (sd->modifiers.isEmpty()) return;
    int shadow = 0;
    for (auto &mod : sd->modifiers) {
        for (auto &op : mod.operations) {
            int first  = (op.first.ID == -1) ? shadow : fetchOperand(idx, op.first);
            int second = fetchOperand(idx, op.second);
            switch (op.operation) {
            case ADDITION:       shadow = first + second; break;
            case SUBTRACTION:    shadow = first - second; break;
            case MULTIPLICATION: shadow = first * second; break;
            case DIVISION:       shadow = first / second; break;
            case AND:            shadow = first & second; break;
            case OR:             shadow = first | second; break;
            case XOR:            shadow = first ^ second; break;
            case MOD:            shadow = first % second; break;
            }
        }
        QByteArray arr = sd->payload();
        arr[mod.destByte] = (char)shadow;
        sd->setPayload(arr);
    }
}

int FrameSenderWindowLIN::fetchOperand(int idx, ModifierOperand op)
{
    if (op.ID == 0)  return op.notOper ? ~op.databyte : op.databyte;
    if (op.ID == -2) return op.notOper
                   ? ~(uchar)sendingData[idx].payload()[op.databyte]
                   :  (uchar)sendingData[idx].payload()[op.databyte];
    LINFrame *f = lookupFrame(op.ID, op.bus);
    if (f) return op.notOper ? ~(uchar)f->payload()[op.databyte]
                          :  (uchar)f->payload()[op.databyte];
    return 0;
}

LINFrame *FrameSenderWindowLIN::lookupFrame(int ID, int bus)
{
    if (!frameCache.contains((uint32_t)ID)) return nullptr;
    if (bus == -1 || frameCache[(uint32_t)ID].bus == bus) return &frameCache[(uint32_t)ID];
    return nullptr;
}

void FrameSenderWindowLIN::updateGridRow(int idx)
{
    inhibitChanged = true;
    FrameSendData *tmp = &sendingData[idx];
    QTableWidgetItem *it = ui->tableSender->item(idx, col(FSCol::Count));
    if (!it) {
        it = new QTableWidgetItem();
        ui->tableSender->setItem(idx, col(FSCol::Count), it);
    }
    it->setText(QString::number(tmp->count));
    QString ds;
    const uchar *d = reinterpret_cast<const uchar*>(tmp->payload().constData());
    for (int i = 0; i < tmp->payload().length(); i++)
        ds += Utility::formatNumber(d[i]) + " ";
    QTableWidgetItem *di = ui->tableSender->item(idx, col(FSCol::Data));
    if (di) di->setText(ds.trimmed());
    inhibitChanged = false;
}

// ─────────────────────────────────────────────────────────────────
// EVENT FILTER
// ─────────────────────────────────────────────────────────────────
bool FrameSenderWindowLIN::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_F1)
            HelpWindow::getRef()->showHelp("customsender.md");
        return true;
    }
    return QObject::eventFilter(obj, event);
}