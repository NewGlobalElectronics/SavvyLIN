#include "db_classes.h"
#include "dbhandler.h"
#include "utility.h"
#include <QtMath>


/*
 The way that the db file format works is kind of weird... For intel format signals you count up
from the start bit to the end bit which is (startbit + signallength - 1). At each point
bits are numbered in a sawtooth manner. What that means is that the very first bit is 0 and you count up
from there all of the way to 63 with each byte being 8 bits so bit 0 is the lowest bit in the first byte
and 8 is the lowest bit in the next byte up. The whole thing looks like this:
                 Bits
      7  6  5  4  3  2  1  0

  0   7  6  5  4  3  2  1  0
b 1   15 14 13 12 11 10 9  8
y 2   23 22 21 20 19 18 17 16
t 3   31 30 29 28 27 26 25 24
e 4   39 38 37 36 35 34 33 32
s 5   47 46 45 44 43 42 41 40
  6   55 54 53 52 51 50 49 48
  7   63 62 61 60 59 58 57 56

  For intel format you start at the start bit and keep counting up. If you have a signal size of 8
  and start at bit 12 then the bits are 12, 13, 14, 15, 16, 17, 18, 19 which spans across two bytes.
  In this format each bit is worth twice as much as the last and you just keep counting up.
  Bit 12 is worth 1, 13 is worth 2, 14 is worth 4, etc all of the way to bit 19 is worth 128.

  Motorola format turns most everything on its head. You count backward from the start bit but
  only within the current byte. If you are about to exit the current byte you go one higher and then keep
  going backward as before. Using the same example as for intel, start bit of 12 and a signal length of 8.
  So, the bits are 12, 11, 10, 9, 8, 23, 22, 21. Yes, that's confusing. They now go in reverse value order too.
  Bit 12 is worth 128, 11 is worth 64, etc until bit 21 is worth 1.
*/
db_NODE::db_NODE() :
    isMaster(false), timeBase(0), jitter(0),
    protocolVersion(2.1), NASTimeout(0), NCRTimeout(0), // ← à remplacer par 0,0
    P2Min(50), STMin(0), configuredNAD(0xFF), initialNAD(0xFF),
    supplierId(0), functionId(0), variant(0)
{}
bool DB_SIGNAL::getValueString(int64_t intVal, QString &outString)
{
    if (valList.count() > 0) //if this is a value list type then look it up and display the proper string
    {
        for (int x = 0; x < valList.count(); x++)
        {
            if (valList.at(x).value == intVal)
            {
                outString = valList.at(x).descript;
                return true;
            }
        }
    }
    return false;
}

QString DB_SIGNAL::makePrettyOutput(double floatVal, int64_t intVal, bool outputName, bool isInteger, bool outputUnit)
{
    QString outputString;

    if (outputName) outputString = name + ": ";

    if (valList.count() > 0) //if this is a value list type then look it up and display the proper string
    {
        bool foundVal = false;
        for (int x = 0; x < valList.count(); x++)
        {
            if (valList.at(x).value == intVal)
            {
                outputString += valList.at(x).descript;
                foundVal = true;
                break;
            }
        }
        if (!foundVal) outputString += QString::number(intVal);
        if (outputUnit) outputString += " " + unitName;
    }
    else //otherwise display the actual number and unit (if it exists)
    {
       outputString += (isInteger ? QString::number(intVal) : QString::number(floatVal));
       if (outputUnit) outputString += " " + unitName;
    }
    return outputString;
}

//Works quite a bit like the above version but this one is cut down and only will return int32_t which is perfect for
//uses like calculating a multiplexor value or if you know you are going to get an integer returned
//from a signal and you want to use it as-is and not have to convert back from a string. Use with caution though
//as this basically assumes the signal is an integer.
//The call syntax is different from the more generic processSignal. Instead of returning the value we return
//true or false to show whether the function succeeded. The variable to fill out is passed by reference.
bool DB_SIGNAL::processAsInt(const LINFrame &frame, int32_t &outValue)
{
    int32_t result = 0;
    bool isSigned = false;

    if (valType == STRING || valType == SP_FLOAT  || valType == DP_FLOAT)
    {
        return false;
    }

    //if (!isSignalInMessage(frame)) return false;

    if (valType == SIGNED_INT) isSigned = true;
    /*if ( static_cast<int>(frame.payload().length() * 8) <= (startBit + signalSize) )
    {
        result = 0;
        return false;
    }*/

    result = static_cast<int32_t>(Utility::processIntegerSignal(frame.payload(), startBit, signalSize, intelByteOrder, isSigned));

    double endResult = (result * factor) + bias;
    result = static_cast<int32_t>(endResult);
    cachedValue = result;
    outValue = result;
    return true;
}

//Another cut down version that will only return double precision data. This can be used on any of the types
//except STRING. Useful for when you know you'll need floating point data and don't want to incur a conversion
//back and forth to double or float. Such a use is the graphing window.
//Similar syntax to processSignalInt but with double instead.
bool DB_SIGNAL::processAsDouble(const LINFrame &frame, double &outValue) const
{
    int64_t result = 0;
    bool isSigned = false;
    double endResult;

    if (valType == STRING)
    {
        return false;
    }

    if (valType == SIGNED_INT) isSigned = true;
    if (valType == SIGNED_INT || valType == UNSIGNED_INT)
    {
        if ( frame.payload().length() * 8 < (startBit+signalSize) )
        {
            result = 0;
            return false;
        }
        result = Utility::processIntegerSignal(frame.payload(), startBit, signalSize, intelByteOrder, isSigned);
        endResult = ((double)result * factor) + bias;
        result = (int64_t)endResult;
    }
    else if (valType == SP_FLOAT)
    {
        if ( frame.payload().length() * 8 < (startBit + 32) )
        {
            result = 0;
            return false;
        }
        result = Utility::processIntegerSignal(frame.payload(), startBit, 32, false, false);
        float f;
        memcpy(&f, &result, sizeof(f));
        endResult = (f * factor) + bias;
    }
    else //double precision float
    {
        if ( frame.payload().length() < 8 )
        {
            result = 0;
            return false;
        }
        result = Utility::processIntegerSignal(frame.payload(), startBit, 64, false, false);
        double d;
        memcpy(&d, &result, sizeof(d));
        endResult = (d * factor) + bias;
    }
    cachedValue = endResult;
    outValue = endResult;
    return true;
}

bool DB_SIGNAL::processAsText(const LINFrame& frame, QString& outString,
                              bool outputName, bool outputUnit)
{
    bool isSigned  = false;
    bool isInteger = false;
    double endResult = 0.0;
    int64_t rawResult = 0;

    if (valType == STRING) {
        QString buildString;
        int startByte = startBit / 8;
        int bytes = signalSize / 8;
        for (int x = 0; x < bytes; x++)
            buildString.append(frame.payload().data()[startByte + x]);
        outString = buildString;
        cachedValue = outString;
        return true;
    }

    if (valType == SIGNED_INT) isSigned = true;

    if (valType == SIGNED_INT || valType == UNSIGNED_INT) {
        if (frame.payload().length() * 8 < (startBit + signalSize)) return false;
        rawResult = Utility::processIntegerSignal(frame.payload(),
                                                  startBit, signalSize, intelByteOrder, isSigned);
        endResult = ((double)rawResult * factor) + bias;
        isInteger = (factor == qFloor(factor) && bias == qFloor(bias));
    }
    else if (valType == SP_FLOAT) {
        if (frame.payload().length() * 8 < (startBit + 32)) return false;
        rawResult = Utility::processIntegerSignal(frame.payload(),
                                                  startBit, 32, intelByteOrder, false);
        float f;
        memcpy(&f, &rawResult, sizeof(f));
        endResult = (f * factor) + bias;
    }
    else { // DP_FLOAT
        if (frame.payload().length() < 8) return false;
        rawResult = Utility::processIntegerSignal(frame.payload(),
                                                  startBit, 64, intelByteOrder, false);
        double d;
        memcpy(&d, &rawResult, sizeof(d));
        endResult = (d * factor) + bias;
    }

    outString = makePrettyOutput(endResult, rawResult, outputName, isInteger, outputUnit);
    cachedValue = endResult;
    return true;
}
db_ATTRIBUTE_VALUE *DB_SIGNAL::findAttrValByName(QString name)
{
    if (attributes.length() == 0) return nullptr;
    for (int i = 0; i < attributes.length(); i++)
    {
        if (attributes[i].attrName.compare(name, Qt::CaseInsensitive) == 0)
        {
            return &attributes[i];
        }
    }
    return nullptr;
}

db_ATTRIBUTE_VALUE *DB_SIGNAL::findAttrValByIdx(int idx)
{
    if (idx < 0) return nullptr;
    if (idx >= attributes.count()) return nullptr;
    return &attributes[idx];
}

db_ATTRIBUTE_VALUE *db_MESSAGE::findAttrValByName(QString name)
{
    if (attributes.length() == 0) return nullptr;
    for (int i = 0; i < attributes.length(); i++)
    {
        if (attributes[i].attrName.compare(name, Qt::CaseInsensitive) == 0)
        {
            return &attributes[i];
        }
    }
    return nullptr;
}

db_ATTRIBUTE_VALUE *db_MESSAGE::findAttrValByIdx(int idx)
{
    if (idx < 0) return nullptr;
    if (idx >= attributes.count()) return nullptr;
    return &attributes[idx];
}

db_ATTRIBUTE_VALUE *db_NODE::findAttrValByName(QString name)
{
    if (attributes.length() == 0) return nullptr;
    for (int i = 0; i < attributes.length(); i++)
    {
        if (attributes[i].attrName.compare(name, Qt::CaseInsensitive) == 0)
        {
            return &attributes[i];
        }
    }
    return nullptr;
}

db_ATTRIBUTE_VALUE *db_NODE::findAttrValByIdx(int idx)
{
    if (idx < 0) return nullptr;
    if (idx >= attributes.count()) return nullptr;
    return &attributes[idx];
}
dbFile::~dbFile()
{
    delete messageHandler;
}