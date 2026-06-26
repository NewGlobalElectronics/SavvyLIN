#ifndef CONNECTIONWINDOW_H
#define CONNECTIONWINDOW_H



#include <QDialog>
#include <QSerialPortInfo>
#include <QDebug>
#include <QSettings>
#include <QTimer>
#include <QItemSelection>
#include <QCanBusDeviceInfo>
#include <QUdpSocket>
#include "canconnectionmodel.h"
#include "connections/canconnection.h"


class CANConnectionModel;

namespace Ui {
class ConnectionWindowLIN;
}


class ConnectionWindowLIN : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionWindowLIN(QWidget *parent = 0);
    ~ConnectionWindowLIN();

signals:
    void updateBusSettings(CANBus *bus);
    void updatePortName(QString port);
    void sendDebugData(QByteArray bytes);

public slots:
    void getDebugText(QString debugText);
    void setSuspendAll(bool pSuspend);


private slots:
    void currentRowChanged(const QModelIndex &current, const QModelIndex &previous);
    void currentTabChanged(int newIdx);
    void consoleEnableChanged(bool checked);
    void handleRemoveConn();
    void handleNewConn();
    void handleResetConn();
    void handleClearDebugText();
    void handleSendHex();
    void handleSendText();
    void saveBusSettings();
    void moveConnUp();
    void moveConnDown();
    void connectionStatus(CANConStatus);
    void readPendingDatagrams();

private:
    Ui::ConnectionWindowLIN *ui;    
    QSettings *settings;
    CANConnectionModel *connModel;
    QUdpSocket *rxBroadcastGVRET;
    QUdpSocket *rxBroadcastKayak;
    QVector<QString> remoteDeviceIPGVRET;
    QVector<QString> remoteDeviceKayak;

    CANConnection* create(CANCon::type pTye, QString pPortName, QString pDriver, int pSerialSpeed, int pBusSpeed, bool pCanFd, int pDataRate);
    void populateBusDetails(int offset);
    void loadConnections();
    void saveConnections();
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void readSettings();
    void writeSettings();
};

#endif // CONNECTIONWINDOW_H
