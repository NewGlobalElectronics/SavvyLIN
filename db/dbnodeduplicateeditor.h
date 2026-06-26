#ifndef dbNODEDUPLICATEEDITOR_H
#define dbNODEDUPLICATEEDITOR_H

#include <QDialog>
#include "db_classes.h"
#include "dbhandler.h"

namespace Ui {
class dbNodeDuplicateEditor;
}

class dbNodeDuplicateEditor : public QDialog
{
    Q_OBJECT

public:
    explicit dbNodeDuplicateEditor(QWidget *parent = nullptr);
    ~dbNodeDuplicateEditor();
    void showEvent(QShowEvent*);
    void setNodeRef(db_NODE *node);
    void setFileIdx(int idx);
    bool refreshView();

signals:
    void updatedTreeInfo(db_MESSAGE *msg);
    void createNode(QString nodeName);
    void cloneMessageToNode(db_NODE *parentNode, db_MESSAGE *source, uint newMsgId);
    void nodeAdded();

private:
    Ui::dbNodeDuplicateEditor *ui;

    dbHandler *dbHandler;
    db_NODE *dbNode;
    dbFile *dbFile;

    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void readSettings();
    void writeSettings();

    uint lowestMsgId;
};

#endif // dbNODEDUPLICATEEDITOR_H
