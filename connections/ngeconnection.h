#ifndef NGECONNECTION_H
#define NGECONNECTION_H

#include "canconnection.h"
#include <QSerialPort>
#include <QTimer>
#include <QMap>
#include <QSet>  // ← AJOUT

// ── Types de trames protocole binaire (LIN seulement) ─────────────
static const uint8_t FRAME_START_BYTE = 0xAA;
static const uint8_t TYPE_LIN_CMD     = 0x02;  // host → device
static const uint8_t TYPE_LIN_RX      = 0x86;  // device → host (FULL)  ← CORRIGÉ
static const uint8_t TYPE_LIN_STATUS  = 0x87;  // device → host (status)  ← CORRIGÉ
static const uint8_t TYPE_LIN_RX_IDENT= 0x84;  // device → host (IDENT)
static const uint8_t TYPE_LIN_RX_DELTA= 0x85;  // device → host (DELTA)

class NGEConnectionLIN : public CANConnection
{
    Q_OBJECT

public:
    explicit NGEConnectionLIN(const QString& portName,
                              const QString& driver    = "",
                              CANCon::type type        = CANCon::NGE,
                              int serialSpeed          = 115200,
                              int busSpeed             = 500000,
                              bool canFd               = false,
                              int dataRate             = 2000000,
                              int numBuses             = 1,
                              int queueLen             = 200,
                              bool useThread           = true);
    ~NGEConnectionLIN();

    // Commandes LIN Master
    void sendLinReadMode(uint8_t id = 0x30, uint16_t period_ms = 1000);  // ← CORRIGÉ
    void sendLinWriteMode(uint8_t id, uint8_t dlc, const QByteArray &data, uint16_t period_ms = 1000);  // ← CORRIGÉ
    void sendLinReset();
    void sendLinFullReset();

    // Commandes LIN Slave
    void sendLinSlaveMode();
    void sendLinSlaveResponse(uint8_t id, const QByteArray &data);
    void removeLinSlaveResponse(uint8_t id);

    // Envoi d'une trame LIN (hérité de CANConnection mais utilisé pour LIN)
    bool piSendFrame(const LINFrame& frame) override;

public slots:
    void sendBinaryFrame(uint8_t type, const QByteArray &payload);
    void sendBinaryCommand(uint8_t type, const QByteArray &payload);
protected:
    void piStarted() override;
    void piStop() override;
    void piSuspend(bool suspend) override;
    bool piGetBusSettings(int busIdx, CANBus& bus) override;
    void piSetBusSettings(int busIdx, CANBus bus) override;

private slots:
    void readSerialData();
    void sendStartRxCommand();

private:
    QSerialPort *serial;
    QTimer      *readTimer;
    bool         openFlag;

    // Machine d'état du parser
    enum ParseState {
        PSTATE_WAIT_START,
        PSTATE_WAIT_TYPE,
        PSTATE_WAIT_LEN_L,
        PSTATE_WAIT_LEN_H,
        PSTATE_READ_PAYLOAD,
        PSTATE_WAIT_CRC
    };

    // Structure pour une trame LIN (avec compression)

    struct LinEntry {
        uint8_t    id;
        uint8_t    pid;
        uint8_t    dlc;
        uint8_t    checksum;
        QByteArray data;
        bool       valid;
        uint32_t   timestamp_us;
        bool       enhancedChecksum;   // ← AJOUT : déduit une seule fois à la réception FULL
        bool we_sent_header;
        bool we_sent_data;
    };

    ParseState  parseState;
    uint8_t     rxType;
    uint16_t    rxLen;
    uint16_t    rxPayloadIdx;
    uint8_t     rxCrcAccum;
    QByteArray  rxPayloadBuf;

    // Table de compression LIN (index = ID)
    QMap<uint8_t, LinEntry> linTable;

    // ← AJOUT : IDs que nous publions (WRITE ou SLAVE) → isReceived = false
    QSet<uint8_t> publishedIds;

    void parserFeed(uint8_t byte);
    void processRxFrame(uint8_t type, const QByteArray &payload);
    void enqueueLinFrame(const LinEntry &e);
};

#endif