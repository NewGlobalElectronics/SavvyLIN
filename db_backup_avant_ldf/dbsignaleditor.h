#ifndef dbSIGNALEDITOR_H
#define dbSIGNALEDITOR_H

#include <QDialog>
#include "dbhandler.h"
#include "utility.h"

namespace Ui {
class dbSignalEditor;
}

class dbSignalEditor : public QDialog
{
    Q_OBJECT

public:
    explicit dbSignalEditor(QWidget *parent = 0);
    void setMessageRef(db_MESSAGE *msg);
    void setFileIdx(int idx);
    void setSignalRef(DB_SIGNAL *sig);
    void showEvent(QShowEvent*);
    ~dbSignalEditor();
    void refreshView();

signals:
    void updatedTreeInfo(DB_SIGNAL *sig);

private slots:
    void bitfieldLeftClicked(int bit);
    void bitfieldRightClicked(int bit);
    void onValuesCellChanged(int row,int col);
    void onCustomMenuValues(QPoint);
    void deleteCurrentValue();

private:
    Ui::dbSignalEditor *ui;
    dbHandler *dbHandler;
    db_MESSAGE *dbMessage;
    DB_SIGNAL *currentSignal;
    QList<DB_SIGNAL> undoBuffer;
    dbFile *dbFile;
    bool inhibitCellChanged;
    bool inhibitMsgProc;

    void fillSignalForm(DB_SIGNAL *sig);
    void fillValueTable(DB_SIGNAL *sig);
    void generateUsedBits();
    void refreshBitGrid();

    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void readSettings();
    void writeSettings();
    void pushToUndoBuffer();
    void popFromUndoBuffer();
};

#endif // dbSIGNALEDITOR_H
