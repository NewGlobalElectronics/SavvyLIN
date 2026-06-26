#ifndef dbNODEEDITOR_H
#define dbNODEEDITOR_H

#include <QDialog>
#include "db_classes.h"
#include "dbhandler.h"

namespace Ui {
class dbNodeEditor;
}

class dbNodeEditor : public QDialog
{
    Q_OBJECT

public:
    explicit dbNodeEditor(QWidget *parent = nullptr);
    ~dbNodeEditor();
    void showEvent(QShowEvent*);
    void setNodeRef(db_NODE *node);
    void setFileIdx(int idx);
    void refreshView();

signals:
    void updatedTreeInfo(db_NODE *node);

private:
    Ui::dbNodeEditor *ui;

    dbHandler *dbHandler;
    db_NODE *dbNode;
    dbFile *dbFile;

    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void readSettings();
    void writeSettings();
    void generateSampleText();
};

#endif // dbNODEEDITOR_H
