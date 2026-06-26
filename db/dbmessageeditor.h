#ifndef dbMESSAGEEDITOR_H
#define dbMESSAGEEDITOR_H

#include <QDialog>
#include "db_classes.h"
#include "dbhandler.h"

namespace Ui {
class dbMessageEditor;
}

class dbMessageEditor : public QDialog
{
    Q_OBJECT

public:
    explicit dbMessageEditor(QWidget *parent = nullptr);
    ~dbMessageEditor();
    void showEvent(QShowEvent*);
    void setMessageRef(db_MESSAGE *msg);
    void setFileIdx(int idx);
    void refreshView();

signals:
    void updatedTreeInfo(db_MESSAGE *msg);

private:
    Ui::dbMessageEditor *ui;

    dbHandler *dbHandler;
    db_MESSAGE *dbMessage;
    dbFile *dbFile;
    bool suppressEditCallbacks;

    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void readSettings();
    void writeSettings();
    void generateSampleText();
};

#endif // dbMESSAGEEDITOR_H
