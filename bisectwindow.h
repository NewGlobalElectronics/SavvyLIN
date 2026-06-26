#ifndef BISECTWINDOW_H
#define BISECTWINDOW_H

#include <QDialog>
#include "can_structs.h"

namespace Ui {
class BisectWindowLIN;
}

class BisectWindowLIN : public QDialog
{
    Q_OBJECT

public:
    explicit BisectWindowLIN(const QVector<LINFrame> *frames, QWidget *parent = 0);
    ~BisectWindowLIN();
    void showEvent(QShowEvent*);

signals:
    void sendCANFrame(const LINFrame *, int);
    void sendFrameBatch(const QList<LINFrame> *);

private slots:
    void updatedFrames(int numFrames);
    void handleSaveButton();
    void handleReplaceButton();
    void handleCalculateButton();
    void updateFrameNumSlider();
    void updatePercentSlider();
    void updateFrameNumText();
    void updatePercentText();
    void updateSectionsText();

private:
    Ui::BisectWindowLIN *ui;
    const QVector<LINFrame> *modelFrames;
    QVector<LINFrame> splitFrames;
    QList<int> foundID;

    void refreshIDList();
    void refreshFrameNumbers();
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // BISECTWINDOW_H


