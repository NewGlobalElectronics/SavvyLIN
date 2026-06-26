#ifndef FUZZINGWINDOW_H
#define FUZZINGWINDOW_H

#include <QDialog>
#include <QListWidget>
#include <QTimer>
#include "can_structs.h"

namespace Ui {
class FuzzingWindowLIN;
}

namespace BitSequenceType
{
    enum
    {
        Sequential,
        Sweeping,
        Random
    };
}

class FuzzingWindowLIN : public QDialog
{
    Q_OBJECT

public:
    explicit FuzzingWindowLIN(const QVector<LINFrame> *frames, QWidget *parent = 0);
    ~FuzzingWindowLIN();

signals:
    void sendCANFrame(const LINFrame *);
    void sendFrameBatch(const QList<LINFrame> *);

private slots:
    void changePlaybackSpeed(int newSpeed);
    void timerTriggered();
    void clearAllFilters();
    void setAllFilters();
    void toggleFuzzing();
    void idListChanged(QListWidgetItem *item);
    void bitfieldClicked(int);
    void changedNumDataBytes(int newVal);
    void updatedFrames(int numFrames);
    void markAllHigh();
    void markAllLow();
    void markAllAuto();

private:
    Ui::FuzzingWindowLIN *ui;
    const QVector<LINFrame> *modelFrames;
    QTimer *fuzzTimer;
    QList<int> foundIDs;
    QList<int> selectedIDs;
    QList<LINFrame> sendingBuffer;
    int startID, endID, currentID, currentIdx;
    bool seqIDScan, rangeIDSelect;
    int bitSequenceType;
    bool currentlyFuzzing;
    uint8_t currentBytes[64];
    uint8_t bitGrid[512];
    uint8_t numBits;
    uint64_t bitAccum;
    int numSentFrames;

    void refreshIDList();
    void calcNextID();
    void calcNextBitPattern();
    void redrawGrid();
    bool eventFilter(QObject *obj, QEvent *event);
    void changedDataByteText(int which, QString valu);
};

#endif // FUZZINGWINDOW_H
