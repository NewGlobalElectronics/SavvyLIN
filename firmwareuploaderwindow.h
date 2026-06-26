#ifndef FIRMWAREUPLOADERWINDOW_H
#define FIRMWAREUPLOADERWINDOW_H

#include <QDialog>
#include <QTimer>
#include "can_structs.h"
#include "connections/canconmanager.h"
#include "utility.h"

namespace Ui {
class FirmwareUploaderWindowLIN;
}

class FirmwareUploaderWindowLIN : public QDialog
{
    Q_OBJECT

public:
    explicit FirmwareUploaderWindowLIN(const QVector<LINFrame> *frames, QWidget *parent = 0);
    ~FirmwareUploaderWindowLIN();

public slots:
    void gotTargettedFrame(LINFrame frame);

private slots:
    void handleLoadFile();
    void handleStartStopTransfer();
    void updatedFrames(int);
    void timerElapsed();

private:
    void updateProgress();
    void loadBinaryFile(QString);
    void sendFirmwareChunk();
    void sendFirmwareEnding();

    Ui::FirmwareUploaderWindowLIN *ui;
    bool transferInProgress;
    bool startedProcess;
    int firmwareSize;
    int currentSendingPosition;
    int baseAddress;
    int bus;
    uint32_t token;
    QByteArray firmwareData;
    const QVector<LINFrame> *modelFrames;
    QTimer *timer;
};

#endif // FIRMWAREUPLOADERWINDOW_H
