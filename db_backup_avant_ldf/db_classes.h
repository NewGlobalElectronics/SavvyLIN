#ifndef db_CLASSES_H
#define db_CLASSES_H

#include <QColor>
#include <QString>
#include <QStringList>
#include <QVariant>
#include "can_structs.h"

/*classes to encapsulate data from a db file. Really, the stuff of interest
  are the nodes, messages, signals, attributes, and comments.

  These things sort of form a hierarchy. Nodes send and receive messages.
  Messages are comprised of signals. Nodes, signals, and messages potentially have attribute values.
  All of them can have comments.
*/

enum db_SIG_VAL_TYPE
{
    UNSIGNED_INT,
    SIGNED_INT,
    SP_FLOAT,
    DP_FLOAT,
    STRING
};

enum db_ATTRIBUTE_VAL_TYPE
{
    ATTR_INT,
    ATTR_FLOAT,
    ATTR_STRING,
    ATTR_ENUM,
};

enum db_ATTRIBUTE_TYPE
{
    ATTR_TYPE_GENERAL,
    ATTR_TYPE_NODE,
    ATTR_TYPE_MESSAGE,
    ATTR_TYPE_SIG,
    ATTR_TYPE_ANY
};

class db_ATTRIBUTE
{
public:
    QString name;
    db_ATTRIBUTE_VAL_TYPE valType;
    db_ATTRIBUTE_TYPE attrType;
    double upper, lower;
    QStringList enumVals;
    QVariant defaultValue;
};

class db_ATTRIBUTE_VALUE
{
public:
    QString attrName;
    QVariant value;
};

class db_VAL_ENUM_ENTRY
{
public:
    int value;
    QString descript;
};

class db_NODE
{
public:
    QString name;
    QString comment;
    QString sourceFileName;
    QList<db_ATTRIBUTE_VALUE> attributes;

    db_ATTRIBUTE_VALUE *findAttrValByName(QString name);
    db_ATTRIBUTE_VALUE *findAttrValByIdx(int idx);

    friend bool operator<(const db_NODE& l, const db_NODE& r)
    {
        return (l.name.toLower() < r.name.toLower());
    }
};

class db_MESSAGE; //forward reference so that DB_SIGNAL can compile before we get to real definition of db_MESSAGE
class DB_SIGNAL;

class DB_SIGNAL
{
public: //TODO: Clean up this class so that not everything is public. There is one private member which is a start...
    DB_SIGNAL() = default;

    enum dbMuxStringFormat {
        MuxStringFormat_dbFile,
        MuxStringFormat_UI
    };
    QString name;
    int startBit = 1;
    int signalSize = 1;
    bool intelByteOrder = false; //true is obviously little endian. False is big endian

    bool isMultiplexor = false;
    bool isMultiplexed = false;
    void addMultiplexRange(int min, int max);
    bool hasExtendedMultiplexing = false;
    QList<DB_SIGNAL *> multiplexedChildren;
    DB_SIGNAL *multiplexParent = nullptr;
    QString multiplexdbString(dbMuxStringFormat fmt = MuxStringFormat_dbFile) const;
    void copyMultiplexValuesFromSignal(const DB_SIGNAL &signal);
    bool parsedbMultiplexUiString(const QString &multiplexes, QString &errorString);
    bool multiplexesIdenticalToSignal(DB_SIGNAL *other) const;

    db_SIG_VAL_TYPE valType = db_SIG_VAL_TYPE::UNSIGNED_INT;
    double factor = 1.0;
    double bias = 0;
    double min = 0;
    double max = 1;
    db_NODE *receiver = nullptr; //it is fast to have a pointer but dangerous... Make sure to walk the whole tree and delete everything so nobody has stale references.
    db_MESSAGE *parentMessage = nullptr;
    QString unitName;
    QString comment;
    QVariant cachedValue;
    QList<db_ATTRIBUTE_VALUE> attributes;
    QList<db_VAL_ENUM_ENTRY> valList;
    DB_SIGNAL *self;

    bool processAsText(const CANFrame &frame, QString &outString, bool outputName = true, bool outputUnit = true);
    bool processAsInt(const CANFrame &frame, int32_t &outValue);
    bool processAsDouble(const CANFrame &frame, double &outValue);
    bool getValueString(int64_t intVal, QString &outString);
    QString makePrettyOutput(double floatVal, int64_t intVal, bool outputName = true, bool isInteger = false, bool outputUnit = true);
    QString processSignalTree(const CANFrame &frame);
    db_ATTRIBUTE_VALUE *findAttrValByName(QString name);
    db_ATTRIBUTE_VALUE *findAttrValByIdx(int idx);
    bool isSignalInMessage(const CANFrame &frame);
    bool isValueMatchingMultiplex(int val) const;
    int getSimpleMultiplexValue();


    friend bool operator<(const DB_SIGNAL& l, const DB_SIGNAL& r)
    {
        return (l.name.toLower() < r.name.toLower());
    }
private:
    QList<QPair<int, int>> multiplexLowAndHighValues;
};

class dbSignalHandler; //forward declaration to keep from having to include dbhandler.h in this file and thus create a loop

class db_MESSAGE
{
public:
    db_MESSAGE();

    uint32_t ID;
    bool extendedID;
    QString name;
    QString comment;
    unsigned int len;
    db_NODE *sender;
    QColor bgColor;
    QColor fgColor;
    QList<db_ATTRIBUTE_VALUE> attributes;
    dbSignalHandler *sigHandler;
    DB_SIGNAL* multiplexorSignal;

    db_ATTRIBUTE_VALUE *findAttrValByName(QString name);
    db_ATTRIBUTE_VALUE *findAttrValByIdx(int idx);

    friend bool operator<(const db_MESSAGE& l, const db_MESSAGE& r)
    {
        return (l.name.toLower() < r.name.toLower());
    }
};


#endif // db_CLASSES_H

