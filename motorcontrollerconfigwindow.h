#ifndef MOTORCONTROLLERCONFIGWINDOW_H
#define MOTORCONTROLLERCONFIGWINDOW_H

#include <QDialog>
#include <QTimer>
#include "can_structs.h"

namespace Ui {
class MotorControllerConfigWindowLIN;
}

//Serial_Number_EEPROM                     , 0x0113, uint, dec, 16, 6, spr, spr, spr

/*enum PARAM_TYPE
{
    DEC,
    HEX,
    ASCII,
};*/
enum PARAM_TYPE
{
    PARAM_DEC = 0,
    PARAM_HEX = 1,
    PARAM_ASCII = 2
};
enum PARAM_SIGNED
{
    UNSIGNED,
    SIGNED,
    Q15
};

class PARAM
{
public:
    QString paramName;
    uint32_t paramID;
    PARAM_TYPE paramType;
    PARAM_SIGNED signedType;
    uint16_t value;
};


class MotorControllerConfigWindowLIN : public QDialog
{
    Q_OBJECT

public:
    explicit MotorControllerConfigWindowLIN(const QVector<LINFrame> *frames, QWidget *parent = 0);
    ~MotorControllerConfigWindowLIN();

signals:
    void sendCANFrame(const LINFrame *, int);
    void sendFrameBatch(const QList<LINFrame> *);

private slots:
    void updatedFrames(int numFrames);
    void refreshData();
    void saveData();
    void timerTick();
    void loadFile();

private:
    Ui::MotorControllerConfigWindowLIN *ui;
    const QVector<LINFrame> *modelFrames;
    QTimer timer;
    LINFrame outFrame;
    bool doingRequest;
    int transmitStep;
    QList<PARAM> params;

};

#endif // MOTORCONTROLLERCONFIGWINDOW_H
