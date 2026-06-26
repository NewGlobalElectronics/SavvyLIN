#ifndef RANGESTATEWINDOW_H
#define RANGESTATEWINDOW_H

#include <QDialog>
#include <QMap>
#include "can_structs.h"

namespace Ui {
class RangeStateWindowLIN;
}

class RangeStateWindowLIN : public QDialog
{
    Q_OBJECT

public:
    explicit RangeStateWindowLIN(const QVector<LINFrame> *frames, QWidget *parent = 0);
    ~RangeStateWindowLIN();
    void showEvent(QShowEvent*);

private slots:
    void updatedFrames(int);
    void recalcButton();
    void clickedSignalList(int idx);

private:
    Ui::RangeStateWindowLIN *ui;
    const QVector<LINFrame> *modelFrames;
    QVector<LINFrame> frameCache;
    QList<int64_t> foundSignals;
    QMap<int, bool> idFilters;

    void refreshFilterList();
    void closeEvent(QCloseEvent *event);
    void readSettings();
    void writeSettings();
    void signalsFactory();
    bool processSignal(int startBit, int bitLength, int sensitivity, bool bigEndian, bool isSigned);
    void createGraph(QVector<int> values);
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // RANGESTATEWINDOW_H
