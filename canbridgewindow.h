#ifndef CANBRIDGEWINDOW_H
#define CANBRIDGEWINDOW_H

#include <QDialog>
#include "connections/canconmanager.h"

namespace Ui {
class LINBridgeWindow;
}

class LINBridgeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LINBridgeWindow(const QVector<LINFrame> *frames, QWidget *parent = nullptr);
    ~LINBridgeWindow();
    void showEvent(QShowEvent*);


private slots:
    void updatedFrames(int);
    void recalcSides();

private:
    Ui::LINBridgeWindow *ui;
    const QVector<LINFrame> *modelFrames;
    QMap<int, bool> foundIDSide1;
    QMap<int, bool> foundIDSide2;
    int side1BusNum;
    int side2BusNum;

    void processIncomingFrame(LINFrame *frame);
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // CANBRIDGEWINDOW_H
