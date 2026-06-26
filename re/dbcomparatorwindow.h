#ifndef dbCOMPARATORWINDOW_H
#define dbCOMPARATORWINDOW_H

#include <QDialog>
#include <QDebug>
#include <QTreeWidget>
#include "framefileio.h"
#include "db/db_classes.h"
#include "db/dbhandler.h"
#include "utility.h"

namespace Ui {
class dbComparatorWindow;
}

class dbComparatorWindow : public QDialog
{
    Q_OBJECT

public:
    explicit dbComparatorWindow(QWidget *parent = 0);
    ~dbComparatorWindow();

private slots:
    void loadFirstFile();
    void loadSecondFile();
    void saveDetails();

private:
    Ui::dbComparatorWindow *ui;
    dbFile *firstdb;
    dbFile *seconddb;
    QString firstdbFilename;
    QString seconddbFilename;

    void calculateDetails();
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    QString loaddb(dbFile **file);
    void readSettings();
    void writeSettings();
};

#endif //dbCOMPARATORWINDOW_H
