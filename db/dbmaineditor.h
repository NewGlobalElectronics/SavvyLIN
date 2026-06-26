#ifndef dbMAINEDITOR_H
#define dbMAINEDITOR_H

#include <QDialog>
#include <QDebug>
#include <QIcon>
#include <QTreeWidget>
#include <QRandomGenerator>
#include "dbhandler.h"
#include "dbsignaleditor.h"
#include "dbmessageeditor.h"
#include "dbnodeeditor.h"
#include "dbnoderebaseeditor.h"
#include "dbnodeduplicateeditor.h"
#include "utility.h"

namespace Ui {
class dbMainEditor;
}

enum dbItemTypes
{
    NODE = 1,
    MESG = 2,
    SIG = 3
};

class dbMainEditor : public QDialog
{
    Q_OBJECT

public:
    explicit dbMainEditor(const QVector<LINFrame> *frames, QWidget *parent = 0);
    ~dbMainEditor();
    void setFileIdx(int idx);

public slots:
    void updatedNode(db_NODE *node);
    void updatedMessage(db_MESSAGE *msg);
    void updatedSignal(DB_SIGNAL *sig);

private slots:
    void onTreeDoubleClicked(const QModelIndex &index);
    void onTreeContextMenu(const QPoint & pos);
    void currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *prev);
    //void onCustomMenuTree(QPoint);
    void deleteCurrentTreeItem();
    void deleteNode(db_NODE *node);
    void deleteMessage(db_MESSAGE *msg);
    void deleteSignal(DB_SIGNAL *sig);
    void handleSearch();
    void handleSearchForward();
    void handleSearchBackward();
    void newNode(QString nodeName);
    void newNode();
    void copyMessageToNode(db_NODE *node, db_MESSAGE *source, uint newMsgId);
    void newMessage();
    void newSignal();    
    void onRebaseMessages();
    void onDuplicateNode();

private:
    Ui::dbMainEditor *ui;
    dbHandler *dbHandler;
    const QVector<LINFrame> *referenceFrames;
    dbSignalEditor *sigEditor;
    dbMessageEditor *msgEditor;
    dbNodeEditor *nodeEditor;
    dbNodeRebaseEditor *nodeRebaseEditor;
    dbNodeDuplicateEditor *nodeDuplicateEditor;
    dbFile *dbFile;
    int fileIdx;
    QIcon nodeIcon;
    QIcon messageIcon;
    QIcon signalIcon;
    QIcon multiplexorSignalIcon;
    QIcon multiplexedSignalIcon;
    QList<QTreeWidgetItem *> searchItems;
    int searchItemPos;
    //bidirectional mapping of QTreeWidget items back and forth to db objects
    QMap<db_NODE*, QTreeWidgetItem *> nodeToItem;
    QMap<db_MESSAGE*, QTreeWidgetItem *> messageToItem;
    QMap<DB_SIGNAL*, QTreeWidgetItem *> signalToItem;
    QMap<QTreeWidgetItem*, db_NODE*> itemToNode;
    QMap<QTreeWidgetItem*, db_MESSAGE*> itemToMessage;
    QMap<QTreeWidgetItem*, DB_SIGNAL*> itemToSignal;
    QRandomGenerator randGen;

    void showEvent(QShowEvent* event);
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void readSettings();
    void writeSettings();
    void refreshTree();
    void processSignalToTree(QTreeWidgetItem *parent, DB_SIGNAL *sig);
    uint32_t getParentMessageID(QTreeWidgetItem *cell);
    QString createSignalText(DB_SIGNAL *sig);
};

#endif // dbMAINEDITOR_H
