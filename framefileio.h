#ifndef FRAMEFILEIO_H
#define FRAMEFILEIO_H

//Class full of static methods that load and save canbus frames to/from various file formats

#include "config.h"
#include <Qt>
#include <QApplication>
#include <QObject>
#include <QVector>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QFileDialog>
#include "can_structs.h"
#include "utility.h"

class FrameFileIO: public QObject
{
    Q_OBJECT

public:
    FrameFileIO();

    //these present a GUI to the user and allow them to pick the file to load/save
    //The QString returns the filename that was selected and so is really a sort of return value
    //The QVector is used as either the target for loading or the source for saving.
    //These routines call the below loading/saving functions so no need to use them directly if you don't want.
    static bool loadFrameFile(QString &, QVector<LINFrame>*);
    static bool saveFrameFile(QString &, const QVector<LINFrame>*);

    //These do the actual loading and saving and can be used directly if you'd prefer
    static bool autoDetectLoadFile(QString, QVector<LINFrame>*);
    static bool loadCRTDFile(QString, QVector<LINFrame>*);
    static bool loadNativeCSVFile(QString, QVector<LINFrame>*);
    static bool loadGenericCSVFile(QString, QVector<LINFrame>*);
    static bool loadLogFile(QString, QVector<LINFrame>*);
    static bool loadMicrochipFile(QString, QVector<LINFrame>*);
    static bool loadTraceFile(QString, QVector<LINFrame>*);
    static bool loadIXXATFile(QString, QVector<LINFrame>*);
    static bool loadCANDOFile(QString, QVector<LINFrame>*);
    static bool loadVehicleSpyFile(QString, QVector<LINFrame>*);
    static bool loadCanDumpFile(QString, QVector<LINFrame>*);
    static bool loadLawicelFile(QString, QVector<LINFrame>*);
    static bool loadPCANFile(QString, QVector<LINFrame>*);
    static bool loadKvaserFile(QString, QVector<LINFrame>*, bool);
    static bool loadCanalyzerASC(QString, QVector<LINFrame>*);
    static bool loadCanalyzerBLF(QString, QVector<LINFrame>*);
    static bool loadCARBUSAnalyzerFile(QString filename, QVector<LINFrame>* frames);
    static bool loadCANHackerFile(QString filename, QVector<LINFrame>* frames);
    static bool loadCabanaFile(QString filename, QVector<LINFrame>* frames);
    static bool loadCANOpenFile(QString filename, QVector<LINFrame>* frames);
    static bool loadTeslaAPFile(QString filename, QVector<LINFrame>* frames);
    static bool loadCLX000File(QString filename, QVector<LINFrame>* frames);
    static bool loadCANServerFile(QString filename, QVector<LINFrame>* frames);
    static bool loadWiresharkFile(QString filename, QVector<LINFrame>* frames);
    static bool loadWiresharkSocketCANFile(QString filename, QVector<LINFrame>* frames);

    //functions that pre-scan a file to try to figure out if they could read it. Used to automatically determine
    //file type and load it.
    static bool isCRTDFile(QString);
    static bool isNativeCSVFile(QString);
    static bool isGenericCSVFile(QString);
    static bool isLogFile(QString);
    static bool isMicrochipFile(QString);
    static bool isTraceFile(QString);
    static bool isIXXATFile(QString);
    static bool isCANDOFile(QString);
    static bool isVehicleSpyFile(QString);
    static bool isCanDumpFile(QString);
    static bool isLawicelFile(QString);
    static bool isPCANFile(QString);
    static bool isKvaserFile(QString);
    static bool isCanalyzerASC(QString);
    static bool isCanalyzerBLF(QString);
    static bool isCARBUSAnalyzerFile(QString filename);
    static bool isCANHackerFile(QString filename);
    static bool isCabanaFile(QString filename);
    static bool isCANOpenFile(QString filename);
    static bool isTeslaAPFile(QString filename);
    static bool isCLX000File(QString filename);
    static bool isCANServerFile(QString filename);
    static bool isWiresharkFile(QString filename);
    static bool isWiresharkSocketCANFile(QString filename);

    static bool saveCRTDFile(QString, const QVector<LINFrame>*);
    static bool saveNativeCSVFile(QString, const QVector<LINFrame>*);
    static bool saveGenericCSVFile(QString, const QVector<LINFrame>*);
    static bool saveLogFile(QString, const QVector<LINFrame>*);
    static bool saveMicrochipFile(QString, const QVector<LINFrame>*);
    static bool saveTraceFile(QString, const QVector<LINFrame>*);
    static bool saveIXXATFile(QString, const QVector<LINFrame>*);
    static bool saveCANDOFile(QString, const QVector<LINFrame>*);
    static bool saveVehicleSpyFile(QString, const QVector<LINFrame>*);
    static bool saveCanDumpFile(QString filename, const QVector<LINFrame> * frames);
    static bool saveCabanaFile(QString filename, const QVector<LINFrame>* frames);
    static bool saveCanalyzerASC(QString filename, const QVector<LINFrame>* frames);
    static bool saveCARBUSAnalzyer(QString filename, const QVector<LINFrame>* frames);

    static bool openContinuousNative();
    static bool closeContinuousNative();
    static bool writeContinuousNative(const QVector<LINFrame>*, int);
    static bool flushContinuousNative();

private:
    static QFile continuousFile;
};

#endif // FRAMEFILEIO_H
