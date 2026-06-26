
#ifndef dbHANDLER_H
#define dbHANDLER_H

#include <QObject>
#include "db_classes.h"
#include "can_structs.h"
//#include <QLibrary>
#include "ICluster.h"
    /*typedef enum
    {
        EXACT,
        J1939,
        GMLAN
    } MatchingCriteria_t;*/

/*
 * TODO:
 * Finish coding up the decoupled design
 *
*/

    //typedef ERRORCODE (*PFN_ParseDBFile)(std::string, ETYPE_BUS, std::list<ClusterResult>&);
    //typedef ERRORCODE (*PFN_FreeCluster)(ICluster*);
    //typedef ERRORCODE (*PFN_CreateLDFCluster)(ICluster**);

    // en dehors de toute classe
    //extern PFN_FreeCluster gFreeCluster;
class dbHandler: public QObject
{
    Q_OBJECT
public:
    dbFile* loaddbFile(QString filename);
    dbFile* loaddbFile(int);
    void savedbFile(int);
    void removedbFile(int);
    void removeAllFiles();
    void swapFiles(int pos1, int pos2);
    db_MESSAGE* findMessage(const LINFrame &frame);
    db_MESSAGE* findMessage(const QString msgName);
    db_MESSAGE* findMessage(const QString msgName, const QString fullyQualifiedNodeName);
    db_MESSAGE* findMessage(const QString msgName, const QString nodeName, const QString fileNameNoExt);
    db_MESSAGE* findMessage(uint32_t id);
    db_MESSAGE* findMessageForFilter(uint32_t id, MatchingCriteria_t * matchingCriteria);
    int getFileCount();
    dbFile* getFileByIdx(int idx);
    dbFile* getFileByName(QString name);
    int createBlankFile();
    //dbFile* loadJSONFile(QString);
    //dbFile* loadSecretCSVFile(QString);
    static dbHandler *getReference();
   // bool populateFromCluster(dbFile* file, ICluster* cluster);
private:
    QList<dbFile> loadedFiles;

    dbHandler();
    static dbHandler *instance;
};

#endif // dbHANDLER_H
