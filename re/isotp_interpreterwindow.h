#ifndef ISOTP_INTERPRETERWINDOW_H
#define ISOTP_INTERPRETERWINDOW_H

#include <QDialog>
#include "bus_protocols/isotp_handler.h"

class ISOTP_MESSAGE;
class ISOTP_HANDLER;

namespace Ui {
class ISOTP_InterpreterWindowLIN;
}

class ISOTP_InterpreterWindowLIN : public QDialog
{
    Q_OBJECT

public:
    explicit ISOTP_InterpreterWindowLIN(const QVector<LINFrame> *frames, QWidget *parent = 0);
    ~ISOTP_InterpreterWindowLIN();
    void showEvent(QShowEvent*);

private slots:
    void newISOMessage(ISOTP_MESSAGE msg);
    void newUDSMessage(UDS_MESSAGE msg);
    void showDetailView();
    void updatedFrames(int);
    void clearList();
    void saveList();
    void listFilterItemChanged(QListWidgetItem *item);
    void filterAll();
    void filterNone();
    void interpretCapturedFrames();
    void useExtendedAddressing(bool checked);
    void headerClicked(int logicalIndex);

private:
    Ui::ISOTP_InterpreterWindowLIN *ui;
    ISOTP_HANDLER *decoder;
    UDS_HANDLER *udsDecoder;

    const QVector<LINFrame> *modelFrames;
    QVector<ISOTP_MESSAGE> messages;
    QHash<int, bool> idFilters;

    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void readSettings();
    void writeSettings();

};

#endif // ISOTP_INTERPRETERWINDOW_H
