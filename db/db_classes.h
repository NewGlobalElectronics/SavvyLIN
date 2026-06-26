#ifndef DB_CLASSES_H
#define DB_CLASSES_H

#include <QString>
#include <QColor>
#include <QList>
#include <QVector>
#include <QVariant>
#include <QObject>
#include "can_structs.h"
#include "ldf/ProtocolDefinitions/LINDefines.h"
#include "ldf/ProtocolDefinitions/ICluster.h"
#include <cstring>
enum MatchingCriteria_t { EXACT_DB = 0 };

enum db_SIG_VAL_TYPE
{
    UNSIGNED_INT = 0,
    SIGNED_INT,
    SP_FLOAT,
    DP_FLOAT,
    STRING
};

enum db_ATTRIBUTE_TYPE
{
    ATTR_TYPE_NODE = 0,
    ATTR_TYPE_MESSAGE,
    ATTR_TYPE_SIG,
    ATTR_TYPE_ANY
};

enum db_ATTRIBUTE_VAL_TYPE
{
    ATTR_STRING = 0,
    ATTR_INT,
    ATTR_FLOAT,
    ATTR_ENUM
};

class db_ATTRIBUTE_VALUE
{
public:
    QString  attrName;
    QVariant value;
    db_ATTRIBUTE_VALUE* findAttrValByName(QString name);
};

struct db_SCHEDULE_ENTRY {
    QString frameName;
    double  delayMs = 0.0;
};

struct db_SCHEDULE_TABLE {
    QString name;
    QList<db_SCHEDULE_ENTRY> entries;
};

class db_ATTRIBUTE
{
public:
    QString               name;
    db_ATTRIBUTE_TYPE     attrType;
    db_ATTRIBUTE_VAL_TYPE valType;
    QVariant              defaultValue;
    QVariant              lower;
    QVariant              upper;
    QStringList           enumVals;
};

class db_NODE
{
public:
    db_NODE();

    QString name;
    QString comment;
    QList<db_ATTRIBUTE_VALUE> attributes;

    bool    isMaster = false;
    double  timeBase = 0.0;
    double  jitter   = 0.0;

    QString protocolVersionStr = "2.1";
    double  protocolVersion    = 2.1;

    double  NASTimeout    = 0.0;
    double  NCRTimeout    = 0.0;
    double  P2Min         = 50.0;
    double  STMin         = 0.0;
    int     configuredNAD = 0xFF;
    int     initialNAD    = 0xFF;
    int     supplierId    = 0;
    int     functionId    = 0;
    int     variant       = 0;
    QString responseErrorSignal;
    QList<QString> configurableFrames;

    bool operator<(const db_NODE& other) const { return name < other.name; }
    db_ATTRIBUTE_VALUE* findAttrValByName(QString name);
    db_ATTRIBUTE_VALUE* findAttrValByIdx(int idx);
};

class db_VAL_ENUM_ENTRY
{
public:
    unsigned long value;
    QString       descript;
};

class db_MESSAGE;

class DB_SIGNAL
{
public:
    // FIX 1 : une entrée par plage physical_value dans le LDF
    struct PhysRange {
        double  minVal  = 0.0;
        double  maxVal  = 0.0;
        double  factor  = 1.0;
        double  bias    = 0.0;
        QString unit;
    };

    DB_SIGNAL()
    {
        startBit       = 0;
        signalSize     = 1;
        intelByteOrder = true;
        valType        = UNSIGNED_INT;
        factor         = 1.0;
        bias           = 0.0;
        min            = 0.0;
        max            = 0.0;
        receiver       = nullptr;
        parentMessage  = nullptr;
        rawSignal      = nullptr;
        self           = nullptr;
    }

    QString               name;
    int                   startBit;
    int                   signalSize;
    bool                  intelByteOrder;
    db_SIG_VAL_TYPE       valType;
    double                factor;   // première plage (compatibilité)
    double                bias;
    double                min;
    double                max;
    QString               unitName;
    QString               comment;
    QList<db_VAL_ENUM_ENTRY>   valList;
    QList<db_ATTRIBUTE_VALUE>  attributes;
    mutable QVariant           cachedValue;
    db_NODE*              receiver;
    db_MESSAGE*           parentMessage;
    DB_SIGNAL*            self;
    QString               receiverName;
    ISignal*              rawSignal;
    bool                  hasPhysicalValue = false;
    double                initValue        = 0.0;
    QString               encodingName;
    QString               initValueRaw;

    // FIX 1 : toutes les plages physical_value (peut en avoir plusieurs)
    QList<PhysRange>      physRanges;

    bool operator<(const DB_SIGNAL& other) const { return name < other.name; }

    bool getValueString(int64_t intVal, QString &outString);
    QString makePrettyOutput(double floatVal, int64_t intVal, bool outputName = true,
                             bool isInteger = false, bool outputUnit = true);
    bool processAsInt(const LINFrame &frame, int32_t &outValue);
    bool processAsDouble(const LINFrame& frame, double& val) const;
    db_ATTRIBUTE_VALUE* findAttrValByIdx(int idx);
    db_ATTRIBUTE_VALUE* findAttrValByName(QString name);

    // FIX 1 : décodage physique avec la bonne plage selon la valeur brute
    double getPhysicalValue(double rawVal) const
    {
        if (physRanges.isEmpty())
            return rawVal * factor + bias;
        for (const PhysRange& r : physRanges) {
            if (rawVal >= r.minVal && rawVal <= r.maxVal)
                return rawVal * r.factor + r.bias;
        }
        // hors plage : utiliser la première par défaut
        return rawVal * physRanges[0].factor + physRanges[0].bias;
    }

    double getValue(const unsigned char* data, int dataLen) const
    {
        if (!rawSignal) return 0.0;
        unsigned __int64 rawVal = 0;
        rawSignal->GetRawValue(startBit, signalSize, dataLen, intelByteOrder, data, rawVal);
        double engVal = 0.0;
        rawSignal->GetEnggValueFromRaw(rawVal, engVal);
        return engVal;
    }

    bool isSignalInMessage(const LINFrame& frame) const
    {
        int maxBit = frame.payload().size() * 8;
        if (intelByteOrder)
            return (startBit >= 0 && (startBit + signalSize - 1) < maxBit);
        else
        {
            int bit = startBit;
            int size = signalSize;
            int maxBitUsed = startBit;
            while (size > 0) {
                if (bit > maxBitUsed) maxBitUsed = bit;
                size--;
                if ((bit % 8) == 0) bit += 15;
                else bit--;
            }
            return maxBitUsed < maxBit;
        }
    }

    bool processAsText(const LINFrame& frame, QString& outString,
                       bool outputName = true, bool outputUnit = true);
};

class dbSignalHandler
{
public:
    DB_SIGNAL* findSignalByIdx(int idx);
    DB_SIGNAL* findSignalByName(QString name);
    bool       addSignal(DB_SIGNAL& sig);
    bool       removeSignal(DB_SIGNAL* sig);
    bool       removeSignal(int idx);
    bool       removeSignal(QString name);
    void       removeAllSignals();
    int        getCount();
    void       sort();

private:
    QVector<DB_SIGNAL> sigs;
};

enum eLINFrameType {
    eLIN_INVALID_FRAME,
    eLIN_UNCONDITIONAL,
    eLIN_SPORADIC,
    eLIN_EVENT_TRIGGERED,
    eLIN_DIAGNOSTIC
};

class db_MESSAGE
{
public:
    db_MESSAGE()
    {
        ID           = 0;
        len          = 0;
        sender       = nullptr;
        sigHandler   = new dbSignalHandler();
        linFrameType = eLIN_UNCONDITIONAL;
        rawFrame     = nullptr;
        bgColor      = Qt::white;
        fgColor      = Qt::black;
    }

    ~db_MESSAGE() { delete sigHandler; }

    db_MESSAGE(const db_MESSAGE& other)
    {
        ID           = other.ID;
        len          = other.len;
        name         = other.name;
        comment      = other.comment;
        sender       = other.sender;
        linFrameType = other.linFrameType;
        rawFrame     = other.rawFrame;
        bgColor      = other.bgColor;
        fgColor      = other.fgColor;
        attributes   = other.attributes;
        associatedFrames       = other.associatedFrames;
        collisionResolveTable  = other.collisionResolveTable;
        sigHandler   = new dbSignalHandler(*other.sigHandler);
        senderName   = other.senderName;
    }

    db_MESSAGE& operator=(const db_MESSAGE& other)
    {
        if (this != &other) {
            ID           = other.ID;
            len          = other.len;
            name         = other.name;
            comment      = other.comment;
            sender       = other.sender;
            linFrameType = other.linFrameType;
            rawFrame     = other.rawFrame;
            bgColor      = other.bgColor;
            fgColor      = other.fgColor;
            attributes   = other.attributes;
            associatedFrames      = other.associatedFrames;
            collisionResolveTable = other.collisionResolveTable;
            delete sigHandler;
            sigHandler = new dbSignalHandler(*other.sigHandler);
            senderName = other.senderName;
        }
        return *this;
    }

    bool operator<(const db_MESSAGE& other) const { return ID < other.ID; }

    QStringList       associatedFrames;
    QString           collisionResolveTable;

    uint32_t          ID;
    unsigned int      len;
    QString           name;
    QString           comment;
    db_NODE*          sender;
    dbSignalHandler*  sigHandler;
    eLINFrameType     linFrameType;
    QColor            bgColor;
    QColor            fgColor;
    QList<db_ATTRIBUTE_VALUE> attributes;
    IFrame*           rawFrame;
    QString           senderName;

    db_ATTRIBUTE_VALUE* findAttrValByName(QString name);
    db_ATTRIBUTE_VALUE* findAttrValByIdx(int idx);

    DB_SIGNAL* findSignalByName(const QString& sigName) const
    {
        return sigHandler->findSignalByName(sigName);
    }
};

class dbMessageHandler
{
public:
    dbMessageHandler()
    {
        matchingCriteria      = EXACT_DB;
        filterLabelingEnabled = false;
    }

    db_MESSAGE* findMsgByID(uint32_t id);
    db_MESSAGE* findMsgByIdx(int idx);
    db_MESSAGE* findMsgByName(QString name);
    db_MESSAGE* findMsgByPartialName(QString name);
    QList<db_MESSAGE*> findMsgsByNode(db_NODE* node);

    bool addMessage(db_MESSAGE& msg);
    bool removeMessage(db_MESSAGE* msg);
    bool removeMessage(uint32_t ID);
    bool removeMessage(QString name);
    bool removeMessageByIndex(int idx);
    void removeAllMessages();

    int  getCount();
    void sort();

    bool               filterLabeling();
    void               setFilterLabeling(bool);
    MatchingCriteria_t getMatchingCriteria();
    void               setMatchingCriteria(MatchingCriteria_t);

private:
    QVector<db_MESSAGE> messages;
    MatchingCriteria_t  matchingCriteria;
    bool                filterLabelingEnabled;
};

class dbFile : public QObject
{
    Q_OBJECT
public:
    dbFile();
    dbFile(const dbFile& cpy);
    dbFile& operator=(const dbFile& cpy);
    ~dbFile();

    void sort();

    QString linProtocolVersionStr = "2.1";
    QString linLanguageVersionStr = "2.1";
    double  linProtocolVersion    = 2.1;
    double  linLanguageVersion    = 2.1;
    double  linBaudRate           = 19.2;
    QString linChannelName;
    QStringList sigDeclarationOrder;
    QList<db_SCHEDULE_TABLE> scheduleTables;
    QStringList encodingDeclarationOrder;
    QStringList signalReprOrder;

    // ── Signal_groups ──────────────────────────────────────
    struct db_SIGNAL_GROUP {
        QString name;
        QStringList signalNames;   // renamed
    };
    QList<db_SIGNAL_GROUP> signalGroups;

    // ── Frame_groups ───────────────────────────────────────
    struct db_FRAME_GROUP {
        QString     name;
        QStringList frames;
    };
    QList<db_FRAME_GROUP> frameGroups;
    // ───────────────────────────────────────────────────────

    struct OrphanSig {
        int     size             = 8;
        double  initValue        = 0.0;
        QString initValueRaw;
        QString publisher;
        QString subscriber;
        double  factor           = 1.0;
        double  bias             = 0.0;
        QString unit;
        QList<db_VAL_ENUM_ENTRY> valList;
        double  minVal           = 0.0;
        double  maxVal           = 0.0;
        QString encodingName;
        bool    hasPhysicalValue = false;
        QList<DB_SIGNAL::PhysRange> physRanges;
    };
    QMap<QString, OrphanSig> orphanSignals;

    struct RawEncoding {
        QList<DB_SIGNAL::PhysRange> physRanges;
        QList<db_VAL_ENUM_ENTRY>    valList;
        bool                        hasPhysical = false;
    };
    QMap<QString, RawEncoding>  unusedEncodings;
    QStringList                 unusedEncodingOrder;

    db_NODE* findNodeByIdx(int idx);
    db_NODE* findNodeByName(QString name);
    db_NODE* findNodeByNameAndComment(QString fullname);

    db_ATTRIBUTE* findAttributeByName(QString name, db_ATTRIBUTE_TYPE type = ATTR_TYPE_ANY);
    db_ATTRIBUTE* findAttributeByIdx(int idx);
    void findAttributesByType(db_ATTRIBUTE_TYPE typ, QList<db_ATTRIBUTE>* list);

    QString getFullFilename();
    QString getFilename();
    QString getFilenameNoExt();
    QString getPath();
    int     getAssocBus();
    void    setAssocBus(int bus);

    void setDirtyFlag();
    void clearDirtyFlag();
    bool getDirtyFlag();

    ICluster* m_pCluster;
    bool saveFile(QString fileName);

    QList<db_NODE>      db_nodes;
    QList<db_ATTRIBUTE> db_attributes;
    dbMessageHandler*   messageHandler;

    QString fileName;
    QString filePath;
    int     assocBuses;
    bool    isDirty;
};

#endif // DB_CLASSES_H