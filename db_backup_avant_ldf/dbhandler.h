#ifndef dbHANDLER_H
#define dbHANDLER_H

#include <QObject>
#include "db_classes.h"
#include "can_structs.h"

    /*typedef enum
    {
        EXACT,
        J1939,
        GMLAN
    } MatchingCriteria_t;*/
typedef enum
{
    EXACT_DB,
    J1939_DB,
    GMLAN_DB
} MatchingCriteria_t;
/*
 * TODO:
 * Finish coding up the decoupled design
 *
*/
class dbSignalHandler: public QObject
{
    Q_OBJECT
public:
    DB_SIGNAL *findSignalByName(QString name);
    DB_SIGNAL *findSignalByIdx(int idx);
    bool addSignal(DB_SIGNAL &sig);
    bool removeSignal(DB_SIGNAL *sig);
    bool removeSignal(int idx);
    bool removeSignal(QString name);
    void removeAllSignals();
    int getCount();
    void sort();

private:
    QList<DB_SIGNAL> sigs; //signals is a reserved word or I'd have used that
};

class dbMessageHandler: public QObject
{
    Q_OBJECT
public:
    db_MESSAGE *findMsgByID(uint32_t id);
    db_MESSAGE *findMsgByIdx(int idx);
    db_MESSAGE *findMsgByName(QString name);
    db_MESSAGE *findMsgByPartialName(QString name);
    QList<db_MESSAGE*> findMsgsByNode(db_NODE *node);
    bool addMessage(db_MESSAGE &msg);
    bool removeMessage(db_MESSAGE *msg);
    bool removeMessageByIndex(int idx);
    bool removeMessage(uint32_t ID);
    bool removeMessage(QString name);
    void removeAllMessages();
    int getCount();
    MatchingCriteria_t getMatchingCriteria();
    void setMatchingCriteria(MatchingCriteria_t mc);
    void setFilterLabeling( bool labelFiltering );
    bool filterLabeling();
    void sort();

private:
    QList<db_MESSAGE> messages;
    MatchingCriteria_t matchingCriteria;
    bool filterLabelingEnabled;
};

//technically there should be a node handler too but I'm sort of treating nodes as second class
//citizens since they aren't really all that important (to me anyway)
class dbFile: public QObject
{
    Q_OBJECT
public:
    dbFile();
    dbFile(const dbFile& cpy);
    dbFile& operator=(const dbFile& cpy);
    db_NODE *findNodeByName(QString name);
    db_NODE *findNodeByNameAndComment(QString fullname);
    db_NODE *findNodeByIdx(int idx);
    db_ATTRIBUTE *findAttributeByName(QString name, db_ATTRIBUTE_TYPE type = ATTR_TYPE_ANY);
    db_ATTRIBUTE *findAttributeByIdx(int idx);
    void findAttributesByType(db_ATTRIBUTE_TYPE typ, QList<db_ATTRIBUTE> *list);
    bool saveFile(QString);
    bool loadFile(QString);
    QString getFullFilename();
    QString getFilename();
    QString getFilenameNoExt();
    QString getPath();
    int getAssocBus();
    void setAssocBus(int bus);
    void setDirtyFlag();
    bool getDirtyFlag();
    void clearDirtyFlag();
    void sort();

    dbMessageHandler *messageHandler;
    QList<db_NODE> db_nodes;
    QList<db_ATTRIBUTE> db_attributes;
private:
    QString fileName;
    QString filePath;
    int assocBuses; //-1 = all buses, 0 = first bus, 1 = second bus, etc.
    bool isDirty; //has the file been modified?

    bool parseAttribute(QString inpString, db_ATTRIBUTE &attr);
    QVariant processAttributeVal(QString input, db_ATTRIBUTE_VAL_TYPE typ);
    DB_SIGNAL* parseSignalLine(QString line, db_MESSAGE *msg);
    bool parseSignalMultiplexValueLine(QString line);
    db_MESSAGE* parseMessageLine(QString line);
    bool parseValueLine(QString line);
    bool parseSignalValueTypeLine(QString line);
    bool parseAttributeLine(QString line);
    bool parseDefaultAttrLine(QString line);
};

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
    db_MESSAGE* findMessage(const CANFrame &frame);
    db_MESSAGE* findMessage(const QString msgName);
    db_MESSAGE* findMessage(const QString msgName, const QString fullyQualifiedNodeName);
    db_MESSAGE* findMessage(const QString msgName, const QString nodeName, const QString fileNameNoExt);
    db_MESSAGE* findMessage(uint32_t id);
    db_MESSAGE* findMessageForFilter(uint32_t id, MatchingCriteria_t * matchingCriteria);
    int getFileCount();
    dbFile* getFileByIdx(int idx);
    dbFile* getFileByName(QString name);
    int createBlankFile();
    dbFile* loadJSONFile(QString);
    dbFile* loadSecretCSVFile(QString);
    static dbHandler *getReference();

private:
    QList<dbFile> loadedFiles;

    dbHandler();
    static dbHandler *instance;
};

#endif // dbHANDLER_H
