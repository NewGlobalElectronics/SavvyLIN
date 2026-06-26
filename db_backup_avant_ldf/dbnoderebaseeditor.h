#ifndef dbNODEREBASEEDITOR_H
#define dbNODEREBASEEDITOR_H

#include <QDialog>
#include "db_classes.h"
#include "dbhandler.h"

namespace Ui {
class dbNodeRebaseEditor;
}

class dbNodeRebaseEditor : public QDialog
{
    Q_OBJECT

public:
    explicit dbNodeRebaseEditor(QWidget *parent = nullptr);
    ~dbNodeRebaseEditor();
    void showEvent(QShowEvent*);
    void setNodeRef(db_NODE *node);
    void setFileIdx(int idx);
    bool refreshView();

signals:
    void updatedTreeInfo(db_MESSAGE *msg);

private:
    Ui::dbNodeRebaseEditor *ui;

    dbHandler *dbHandler;
    db_NODE *dbNode;
    dbFile *dbFile;

    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void readSettings();
    void writeSettings();

    uint lowestMsgId;
};

#endif // dbNODEREBASEEDITOR_H
