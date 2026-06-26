#ifndef NEWGRAPHDIALOG_H
#define NEWGRAPHDIALOG_H

#include <QDialog>
#include "graphingwindow.h"
#include "db/dbhandler.h"

namespace Ui {
class NewGraphDialog;
}

class NewGraphDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewGraphDialog(dbHandler *handler, QWidget *parent = 0);
    ~NewGraphDialog();
    void showEvent(QShowEvent*);
    void getParams(GraphParams &);
    void setParams(GraphParams &);
    void clearParams();

private slots:
    void addButtonClicked();
    void colorSwatchClick();
    void fillSwatchClick();
    void loadNodes();
    void loadMessages(int idx);
    void loadSignals(int idx);
    void bitfieldClicked(int);
    void handleDataLenUpdate();
    void drawBitfield();
    void copySignalToParamsUI();

private:
    bool eventFilter(QObject *obj, QEvent *event);
    void checkSignalAgreement();

    Ui::NewGraphDialog *ui;
    dbHandler *m_dbHandler;
    DB_SIGNAL *assocSignal;
    int startBit, dataLen;
    bool shownFromPlotEdit;
};

#endif // NEWGRAPHDIALOG_H
