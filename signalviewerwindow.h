#ifndef SIGNALVIEWERWINDOW_H
#define SIGNALVIEWERWINDOW_H

#include <QDialog>
#include "db/dbhandler.h"

namespace Ui {
class SignalViewerWindowLIN;
}

class SignalViewerWindowLIN : public QDialog
{
    Q_OBJECT

public:
    explicit SignalViewerWindowLIN(const QVector<LINFrame> *frames, QWidget *parent = 0);
    ~SignalViewerWindowLIN();

private slots:
    void loadNodes();
    void loadMessages(int idx);
    void loadSignals(int idx);
    void addSignal();
    void addSignal(DB_SIGNAL *sig);
    void removeSelectedSignal();
    void updatedFrames(int);
    void saveSignalsFile();
    void loadSignalsFile();
    void appendSignalsFile();
    void clearSignalsTable();
    void clearSignalsTable(bool);
    void saveDefinitions();
    void loadDefinitions(bool);

private:
    Ui::SignalViewerWindowLIN *ui;
    dbHandler *dbHandler;

    db_MESSAGE *currentlySelectedMsg;

    QList<DB_SIGNAL *> signalList;
    const QVector<LINFrame> *modelFrames;

    void processFrame(LINFrame &frame);
};

#endif // SIGNALVIEWERWINDOW_H
