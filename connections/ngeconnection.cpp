#include "ngeconnection.h"
#include "canframemodel.h"
#include <QSerialPortInfo>
#include <QDebug>
#include <QByteArray>

/* ── Protocol constants ────────────────────────────────────────── */
static const uint8_t FRAME_START = 0xAA;

/* ── CRC8 XOR ─────────────────────────────────────────────────── */
static uint8_t crc8_xor(uint8_t type, uint16_t len, const uint8_t *payload, uint16_t plen)
{
    uint8_t crc = type ^ (uint8_t)(len & 0xFF) ^ (uint8_t)((len >> 8) & 0xFF);
    for (uint16_t i = 0; i < plen; i++)
        crc ^= payload[i];
    return crc;
}
/* ── LIN checksum (classic ou enhanced) ─────────────────────────── */
/* ── LIN checksum (classic ou enhanced) ─────────────────────────── */
static uint8_t linChecksum(uint8_t id, uint8_t pid, const QByteArray &data, bool enhanced)
{
    uint16_t s = (enhanced && id != 0x3C && id != 0x3D) ? pid : 0;
    for (int i = 0; i < data.size(); i++) {
        s += (uint8_t)data[i];
        if (s > 0xFF) s -= 0xFF;
    }
    return (uint8_t)(0xFF - s);
}
/* ── Build and send a binary frame ───────────────────────────── */
void NGEConnectionLIN::sendBinaryFrame(uint8_t type, const QByteArray &payload)
{
    if (!serial || !serial->isOpen()) return;

    uint16_t plen = (uint16_t)payload.size();
    QByteArray frame;
    frame.reserve(4 + plen + 1);

    frame.append((char)FRAME_START);
    frame.append((char)type);
    frame.append((char)(plen & 0xFF));
    frame.append((char)((plen >> 8) & 0xFF));
    frame.append(payload);

    uint8_t crc = crc8_xor(type, plen,
                           reinterpret_cast<const uint8_t*>(payload.constData()),
                           plen);
    frame.append((char)crc);

    qWarning() << "sendBinaryFrame type=0x"
               << QString::number(type, 16)
               << "plen=" << plen
               << "crc=0x" << QString::number(crc, 16)
               << "hex:" << frame.toHex();
    serial->write(frame);
}

/* ─────────────────────────────────────────────────────────────── */

NGEConnectionLIN::NGEConnectionLIN(const QString& portName,
                                   const QString& driver,
                                   CANCon::type type,
                                   int serialSpeed,
                                   int busSpeed,
                                   bool canFd,
                                   int dataRate,
                                   int numBuses,
                                   int queueLen,
                                   bool useThread)
    : CANConnection(portName, driver, type, serialSpeed, busSpeed,
                    canFd, dataRate, numBuses, queueLen, useThread),
    serial(nullptr),
    readTimer(nullptr),
    openFlag(false),
    parseState(PSTATE_WAIT_START),
    rxType(0),
    rxLen(0),
    rxPayloadIdx(0),
    rxCrcAccum(0)
{
    qWarning() << "NGEConnectionLIN constructor called";
}

NGEConnectionLIN::~NGEConnectionLIN()
{
    piStop();
}

void NGEConnectionLIN::piStarted()
{
    qWarning() << "piStarted() - opening port" << getPort();

    serial = new QSerialPort(QSerialPortInfo(getPort()));
    if (!serial) { qWarning() << "Failed to create serial port"; return; }

    connect(serial, &QSerialPort::readyRead,
            this, &NGEConnectionLIN::readSerialData);
    connect(serial, &QSerialPort::errorOccurred,
            this, [this](QSerialPort::SerialPortError err) {
                if (err != QSerialPort::NoError)
                    qWarning() << "Serial error:" << serial->errorString();
            });

    serial->setBaudRate(115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!serial->open(QIODevice::ReadWrite)) {
        qWarning() << "Cannot open serial port:" << serial->errorString();
        delete serial;
        serial = nullptr;
        setStatus(CANCon::NOT_CONNECTED);
        return;
    }

    serial->setDataTerminalReady(true);
    serial->setRequestToSend(true);

    qWarning() << "Serial port opened successfully";
    openFlag = true;
    setStatus(CANCon::CONNECTED);

    readTimer = new QTimer(this);
    connect(readTimer, &QTimer::timeout, this, &NGEConnectionLIN::readSerialData);
    readTimer->start(50);

    CANConStatus stats;
    stats.conStatus      = getStatus();
    stats.numHardwareBuses = 1;
    emit status(stats);

    /* Send "start RX" command after connection settles */
    QTimer::singleShot(500, this, &NGEConnectionLIN::sendStartRxCommand);
}

void NGEConnectionLIN::sendLinFullReset()
{
    /* QByteArray payload;
    payload.append((char)0xF1);   // FULL RP2040 RESET
    sendBinaryFrame(TYPE_LIN_CMD, payload);*/
}

void NGEConnectionLIN::sendStartRxCommand()
{
    qWarning() << "NGEConnectionLIN: sending LIN reset";

    // Reset de la table côté Qt
    linTable.clear();
    publishedIds.clear();  // ← AJOUT

    // Reset du firmware (efface aussi la table de compression LIN)
    QByteArray resetPayload;
    resetPayload.append((char)0xF0);
    sendBinaryFrame(TYPE_LIN_CMD, resetPayload);

    // Passage en mode slave (optionnel)
    QByteArray p;
    p.append((char)0x04);
    sendBinaryFrame(TYPE_LIN_CMD, p);
}

void NGEConnectionLIN::piStop()
{
    if (readTimer) {
        readTimer->stop();
        delete readTimer;
        readTimer = nullptr;
    }
    if (serial) {
        if (serial->isOpen()) serial->close();
        serial->deleteLater();
        serial = nullptr;
    }
    openFlag = false;
    setStatus(CANCon::NOT_CONNECTED);
}

void NGEConnectionLIN::piSuspend(bool suspend)
{
    setCapSuspended(suspend);
    if (isCapSuspended()) getQueue().flush();
}

void NGEConnectionLIN::sendLinReadMode(uint8_t id, uint16_t period_ms)
{
    QByteArray payload;
    payload.append((char)0x06);        // SET_READ_MODE  ← CORRIGÉ
    payload.append((char)(id & 0x3F)); // ID LIN cible
    payload.append((char)(period_ms & 0xFF));       // ← AJOUT période LSB
    payload.append((char)((period_ms >> 8) & 0xFF)); // ← AJOUT période MSB
    sendBinaryFrame(TYPE_LIN_CMD, payload);

    // ← AJOUT : READ mode = on écoute, donc pas "publié par nous"
    if (period_ms == 0)
        publishedIds.remove(id);
}


void NGEConnectionLIN::sendLinWriteMode(uint8_t id, uint8_t dlc, const QByteArray &data, uint16_t period_ms)
{
    // ← AJOUT : marquer AVANT d'envoyer (pour que le firmware réponde correctement)
    if (period_ms > 0)
        publishedIds.insert(id);
    else
        publishedIds.remove(id);

    QByteArray payload;
    payload.append((char)0x07);        // SET_WRITE_MODE
    payload.append((char)(id & 0x3F));
    payload.append((char)dlc);
    payload.append(data.left(dlc));
    payload.append((char)(period_ms & 0xFF));
    payload.append((char)((period_ms >> 8) & 0xFF));
    sendBinaryFrame(TYPE_LIN_CMD, payload);
}

void NGEConnectionLIN::sendLinSlaveResponse(uint8_t id, const QByteArray &data)
{
    // ← AJOUT : marquer AVANT d'envoyer
    publishedIds.insert(id);

    QByteArray payload;
    uint8_t dlc = (uint8_t)qMin(data.size(), 8);
    payload.append((char)0x05);           // SET_SLAVE_RESPONSE
    payload.append((char)(id & 0x3F));
    payload.append((char)dlc);
    payload.append(data.left(dlc));
    sendBinaryFrame(TYPE_LIN_CMD, payload);
}

void NGEConnectionLIN::sendLinReset()
{
    QByteArray payload;
    payload.append((char)0xF0);   // RESET
    sendBinaryFrame(TYPE_LIN_CMD, payload);

    // ← AJOUT
    publishedIds.clear();
}

void NGEConnectionLIN::sendLinSlaveMode()
{
    QByteArray payload;
    payload.append((char)0x04);   // SET_SLAVE_MODE
    sendBinaryFrame(TYPE_LIN_CMD, payload);
}



void NGEConnectionLIN::removeLinSlaveResponse(uint8_t id)
{
    QByteArray payload;
    payload.append((char)0xF6);   // REMOVE_SLAVE_RESPONSE  ← CORRIGÉ
    payload.append((char)(id & 0x3F));
    sendBinaryFrame(TYPE_LIN_CMD, payload);

    // ← AJOUT : retirer de la liste
    publishedIds.remove(id);
}

bool NGEConnectionLIN::piSendFrame(const LINFrame &frame)
{
    if (!openFlag || !serial) return false;

    QByteArray data = frame.payload();
    uint8_t id     = (uint8_t)(frame.frameId() & 0x3F);
    uint8_t dlc    = (uint8_t)data.size();
    if (dlc > 8) dlc = 8;

    QByteArray payload;
    payload.append((char)0x03);          // sub-command SEND_FRAME (one-shot)
    payload.append((char)id);
    payload.append((char)dlc);
    payload.append(data.left(dlc));

    sendBinaryFrame(TYPE_LIN_CMD, payload);
    return true;
}

bool NGEConnectionLIN::piGetBusSettings(int busIdx, CANBus &bus)
{
    return getBusConfig(busIdx, bus);
}

void NGEConnectionLIN::piSetBusSettings(int busIdx, CANBus bus)
{
    if (busIdx < 0 || busIdx >= getNumBuses()) return;
    setBusConfig(busIdx, bus);
}

void NGEConnectionLIN::readSerialData()
{
    if (!serial) return;
    QByteArray incoming = serial->readAll();
    if (incoming.isEmpty()) return;

    qWarning() << "Raw data (hex):" << incoming.toHex();

    for (int i = 0; i < incoming.size(); i++)
        parserFeed((uint8_t)incoming[i]);
}

/* ── Parser state machine — one byte at a time ───────────────── */
void NGEConnectionLIN::parserFeed(uint8_t byte)
{
    switch (parseState) {

    case PSTATE_WAIT_START:
        if (byte == FRAME_START)
            parseState = PSTATE_WAIT_TYPE;
        break;

    case PSTATE_WAIT_TYPE:
        rxType      = byte;
        rxCrcAccum  = byte;
        parseState  = PSTATE_WAIT_LEN_L;
        break;

    case PSTATE_WAIT_LEN_L:
        rxLen       = byte;
        rxCrcAccum ^= byte;
        parseState  = PSTATE_WAIT_LEN_H;
        break;

    case PSTATE_WAIT_LEN_H:
        rxLen      |= ((uint16_t)byte << 8);
        rxCrcAccum ^= byte;
        rxPayloadIdx = 0;
        rxPayloadBuf.clear();
        rxPayloadBuf.reserve(rxLen);
        if (rxLen == 0)
            parseState = PSTATE_WAIT_CRC;
        else if (rxLen > 512)
            parseState = PSTATE_WAIT_START; /* resync */
        else
            parseState = PSTATE_READ_PAYLOAD;
        break;

    case PSTATE_READ_PAYLOAD:
        rxPayloadBuf.append((char)byte);
        rxCrcAccum ^= byte;
        rxPayloadIdx++;
        if (rxPayloadIdx >= rxLen)
            parseState = PSTATE_WAIT_CRC;
        break;

    case PSTATE_WAIT_CRC:
        if (byte == rxCrcAccum)
            processRxFrame(rxType, rxPayloadBuf);
        else
            qWarning() << "CRC mismatch: got=0x"
                       << QString::number(byte, 16)
                       << "expected=0x"
                       << QString::number(rxCrcAccum, 16);
        qWarning() << "CRC check: got=0x" << QString::number(byte, 16)
                   << "expected=0x" << QString::number(rxCrcAccum, 16);
        parseState = PSTATE_WAIT_START;
        break;
    }
}

void NGEConnectionLIN::processRxFrame(uint8_t type, const QByteArray &payload)
{
    qWarning() << "processRxFrame type=0x" << QString::number(type, 16)
    << "size=" << payload.size();

    // --- LIN FULL FRAME (0x86) ← CORRIGÉ (était 0x82) ---
    if (type == TYPE_LIN_RX) {
        if (payload.size() < 9) return;  // 4(timestamp) + 5(flags,id,pid,dlc,checksum)

        uint32_t ts_us = (uint32_t)(uint8_t)payload[0]
                         | ((uint32_t)(uint8_t)payload[1] <<  8)
                         | ((uint32_t)(uint8_t)payload[2] << 16)
                         | ((uint32_t)(uint8_t)payload[3] << 24);

        uint8_t flags    = (uint8_t)payload[4];
        uint8_t id       = (uint8_t)payload[5];
        uint8_t pid      = (uint8_t)payload[6];
        uint8_t dlc      = (uint8_t)payload[7];
        uint8_t checksum = (uint8_t)payload[8];

        // ← MODIFIÉ : décoder les nouveaux flags
        bool valid          = (flags & 0x01) != 0;
        bool we_sent_header = (flags & 0x02) != 0;  // bit 1
        bool we_sent_data   = (flags & 0x04) != 0;  // bit 2

        if (!valid) {
            qWarning() << "LIN invalid frame id=0x" << QString::number(id, 16);
            return;
        }
        if (dlc > 8) dlc = 8;
        if (9 + dlc > payload.size()) return;

        QByteArray data = payload.mid(9, dlc);

        bool enhanced = (id != 0x3C && id != 0x3D)
                        && (linChecksum(id, pid, data, true) == checksum);

        LinEntry &e = linTable[id];
        e.id           = id;
        e.pid          = pid;
        e.dlc          = dlc;
        e.checksum     = checksum;
        e.data         = data;
        e.valid        = true;
        e.timestamp_us = ts_us;
        e.enhancedChecksum = enhanced;

        // ← NOUVEAU : stocker les flags de direction
        e.we_sent_header = we_sent_header;
        e.we_sent_data   = we_sent_data;

        enqueueLinFrame(e);
        return;
    }

    // --- LIN IDENT (0x84) : [timestamp_4B][id] ---
    if (type == TYPE_LIN_RX_IDENT) {
        if (payload.size() < 5) return;

        uint32_t ts_us = (uint32_t)(uint8_t)payload[0]
                         | ((uint32_t)(uint8_t)payload[1] <<  8)
                         | ((uint32_t)(uint8_t)payload[2] << 16)
                         | ((uint32_t)(uint8_t)payload[3] << 24);
        uint8_t id = (uint8_t)payload[4];

        if (!linTable.contains(id)) {
            qWarning() << "LIN IDENT unknown ID 0x" << QString::number(id, 16);
            return;
        }
        const LinEntry &src = linTable[id];
        if (!src.valid) {
            qWarning() << "LIN IDENT invalid ID 0x" << QString::number(id, 16);
            return;
        }
        LinEntry e = src;
        e.timestamp_us = ts_us;
        enqueueLinFrame(e);
        return;
    }

    // --- LIN DELTA (0x85) : [timestamp_4B][id][n][pos][val]... ---
    if (type == TYPE_LIN_RX_DELTA) {
        if (payload.size() < 6) return;

        uint32_t ts_us = (uint32_t)(uint8_t)payload[0]
                         | ((uint32_t)(uint8_t)payload[1] <<  8)
                         | ((uint32_t)(uint8_t)payload[2] << 16)
                         | ((uint32_t)(uint8_t)payload[3] << 24);
        uint8_t id = (uint8_t)payload[4];
        uint8_t n  = (uint8_t)payload[5];

        if (!linTable.contains(id)) {
            qWarning() << "LIN DELTA unknown ID 0x" << QString::number(id, 16);
            return;
        }
        if (6 + n * 2 > payload.size()) return;

        LinEntry e = linTable.value(id);
        for (int i = 0; i < n; ++i) {
            uint8_t pos = (uint8_t)payload[6 + i*2];
            uint8_t val = (uint8_t)payload[7 + i*2];
            if (pos < (uint8_t)e.data.size())
                e.data[pos] = (char)val;
        }
        e.timestamp_us = ts_us;
        linTable[id] = e;
        enqueueLinFrame(e);
        return;
    }

    // --- LIN STATUS (0x87) ← CORRIGÉ (était 0x83) ---
    if (type == TYPE_LIN_STATUS && payload.size() >= 1) {
        uint8_t status = (uint8_t)payload[0];
        qWarning() << (status == 0 ? "LIN: TX_OK" : "LIN: TX_ERROR");
        return;
    }

    qWarning() << "Unknown LIN frame type 0x" << QString::number(type, 16);
}

void NGEConnectionLIN::enqueueLinFrame(const LinEntry &e)
{
    LINFrame linFrame;
    linFrame.setFrameId((uint32_t)e.id);
    linFrame.setPayload(e.data);
    linFrame.setExtendedFrameFormat(false);
    linFrame.setFlexibleDataRateFormat(false);
    linFrame.setBitrateSwitch(false);
    linFrame.setFrameType(QCanBusFrame::DataFrame);
    linFrame.bus        = 0;

    // ← CORRECTION : isReceived = false si c'est un ID que NOUS publions (WRITE/SLAVE)
    //                isReceived = true si c'est un ID que nous écoutons (READ) ou inconnu
    // Mapper les flags du firmware vers LINFrame
    linFrame.weSentHeader = e.we_sent_header;  // ← nouveau champ dans LINFrame
    linFrame.weSentData   = e.we_sent_data;    // ← nouveau champ dans LINFrame
    linFrame.isReceived   = !e.we_sent_data;   // ← gardé pour compatibilité

    linFrame.enhancedChecksum = e.enhancedChecksum;   // ← AJOUT

    // timestamp firmware
    linFrame.setTimeStamp(QCanBusFrame::TimeStamp(
        e.timestamp_us / 1000000,
        e.timestamp_us % 1000000
        ));

    LINFrame *fp = getQueue().get();
    if (fp) {
        *fp = linFrame;
        checkTargettedFrame(*fp);
        getQueue().queue();
    } else {
        qWarning() << "LIN queue full, frame dropped id=0x"
                   << QString::number(e.id, 16).toUpper();
    }
}
void NGEConnectionLIN::sendBinaryCommand(uint8_t type, const QByteArray &payload)
{
    // ← AJOUT : mettre à jour publishedIds avant d'envoyer
    if (type == TYPE_LIN_CMD && payload.size() >= 2) {
        uint8_t subcmd = (uint8_t)payload[0];
        uint8_t id     = (uint8_t)payload[1] & 0x3F;

        if (subcmd == 0x05) {           // SLAVE_RESPONSE
            publishedIds.insert(id);
        }
        else if (subcmd == 0x07) {      // WRITE_MODE
            if (payload.size() >= 5) {
                uint8_t dlc = (uint8_t)payload[2];
                int period_off = 3 + dlc;
                if (period_off + 1 < payload.size()) {
                    uint16_t period = (uint8_t)payload[period_off]
                                      | ((uint16_t)(uint8_t)payload[period_off+1] << 8);
                    if (period > 0)
                        publishedIds.insert(id);
                    else
                        publishedIds.remove(id);
                }
            }
        }
        else if (subcmd == 0x06) {      // READ_MODE
            if (payload.size() >= 4) {
                uint16_t period = (uint8_t)payload[2]
                                  | ((uint16_t)(uint8_t)payload[3] << 8);
                if (period == 0)
                    publishedIds.remove(id);
            }
        }
        else if (subcmd == 0xF6) {      // REMOVE_SLAVE
            publishedIds.remove(id);
        }
        else if (subcmd == 0xF0) {      // RESET
            publishedIds.clear();
        }
    }

    sendBinaryFrame(type, payload);
}