#include <QCanBus>
#include <QSerialPortInfo>
#include "newconnectiondialog.h"
#include "ui_newconnectiondialog.h"

// Constructeur : les paramètres ne sont plus nécessaires, mais on les garde pour compatibilité
NewConnectionDialog::NewConnectionDialog(QVector<QString>* /*gvretips*/, QVector<QString>* /*kayakhosts*/, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewConnectionDialog)
{
    ui->setupUi(this);

    // Cacher tous les autres types de connexion
   // ui->rbGVRET->setVisible(false);
    //ui->rbSocketCAN->setVisible(false);
   // ui->rbRemote->setVisible(false);
   // ui->rbKayak->setVisible(false);
  //  ui->rbMQTT->setVisible(false);
   // ui->rbLawicel->setVisible(false);
   // ui->rbCANserver->setVisible(false);
   // ui->rbCanlogserver->setVisible(false);

    // Sélectionner et afficher uniquement NGE
    ui->rbNgeProtocol->setChecked(true);
    ui->rbNgeProtocol->setVisible(true);

    // Connecter uniquement le bouton NGE et le OK
    connect(ui->rbNgeProtocol, &QAbstractButton::clicked, this, &NewConnectionDialog::handleConnTypeChanged);
    connect(ui->btnOK, &QPushButton::clicked, this, &NewConnectionDialog::handleCreateButton);

    // Cacher les widgets inutiles
    ui->lblDeviceType->setVisible(false);
    ui->cbDeviceType->setVisible(false);

    // Initialiser l'interface NGE
    selectNGE();
}

NewConnectionDialog::~NewConnectionDialog()
{
    delete ui;
}

void NewConnectionDialog::handleCreateButton()
{
    accept();
}

void NewConnectionDialog::handleConnTypeChanged()
{
    // Seul NGE est présent
    if (ui->rbNgeProtocol->isChecked())
        selectNGE();
}

void NewConnectionDialog::selectNGE()
{
    ui->lPort->setText("Serial Port:");
    ui->lblDeviceType->setVisible(false);
    ui->cbDeviceType->setVisible(false);

    ui->cbPort->clear();
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (int i = 0; i < ports.count(); ++i)
        ui->cbPort->addItem(ports[i].portName());
}

// Retourne toujours NGE
CANCon::type NewConnectionDialog::getConnectionType()
{
    return CANCon::NGE;
}

QString NewConnectionDialog::getPortName()
{
    return ui->cbPort->currentText();
}

// Les autres méthodes sont simplifiées ou retournent des valeurs par défaut
QString NewConnectionDialog::getDriverName()
{
    return "N/A";
}

int NewConnectionDialog::getSerialSpeed()
{
    return 0;
}

int NewConnectionDialog::getBusSpeed()
{
    return 0;
}

int NewConnectionDialog::getDataRate()
{
    return 0;
}

bool NewConnectionDialog::isCanFd()
{
    return false;
}

bool NewConnectionDialog::isSerialBusAvailable()
{
    return false; // plus utilisé
}

void NewConnectionDialog::setPortName(CANCon::type pType, QString pPortName, QString /*pDriver*/)
{
    if (pType == CANCon::NGE) {
        ui->rbNgeProtocol->setChecked(true);
        selectNGE();
        int idx = ui->cbPort->findText(pPortName);
        if (idx < 0) idx = 0;
        ui->cbPort->setCurrentIndex(idx);
    }
}

// Les méthodes suivantes ne sont plus appelées mais conservées pour éviter des erreurs de lien
void NewConnectionDialog::selectLawicel() {}
void NewConnectionDialog::selectSerial() {}
void NewConnectionDialog::selectSocketCan() {}
void NewConnectionDialog::selectRemote() {}
void NewConnectionDialog::selectKayak() {}
void NewConnectionDialog::selectMQTT() {}
void NewConnectionDialog::selectCANserver() {}
void NewConnectionDialog::selectCANlogserver() {}
void NewConnectionDialog::handleDeviceTypeChanged() {}