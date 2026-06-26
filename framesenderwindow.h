#ifndef FRAMESENDERWINDOW_H
#define FRAMESENDERWINDOW_H

#include <QDialog>
#include <QTimer>
#include <QElapsedTimer>
#include <QTime>
#include <QMutex>
#include <QTableWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMap>
#include "can_structs.h"
#include "can_trigger_structs.h"
#include "db/dbhandler.h"
#include "triggerdialog.h"

namespace Ui { class FrameSenderWindowLIN; }

// ─── Colonnes de la table unifiée (enum class évite les conflits) ─
enum class FSCol : int {
    En      = 0,
    Type    = 1,   // QComboBox : Read / Write / Slave
    Bus     = 2,
    Id      = 3,
    MsgName = 4,
    Len     = 5,
    Data    = 6,
    Trigger = 7,
    Mods    = 8,
    Count   = 9,
    Remove  = 10,
    NUM_COLS = 11
};

// Helper pour passer FSCol à l'API Qt qui attend int
inline int col(FSCol c) { return static_cast<int>(c); }

// ─── Structure réponse slave ──────────────────────────────────────
struct SlaveResponse {
    bool       enabled = false;
    uint8_t    id      = 0;
    uint8_t    dlc     = 0;
    QByteArray data;
};

// ─── Dialog "Add Frame" ───────────────────────────────────────────
class AddFrameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddFrameDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle("Add LIN Frame");
        setMinimumWidth(380);

        QFormLayout *form = new QFormLayout();

        typeCombo = new QComboBox();
        typeCombo->addItem("Request an in frame response");
        typeCombo->addItem("Transmit message            ");
        typeCombo->addItem("In frame response           ");
        form->addRow("Service:", typeCombo);

        idEdit = new QLineEdit("0x20");
        form->addRow("ID (hex):", idEdit);

        dataEdit = new QLineEdit("");
        dataEdit->setPlaceholderText("hex bytes separated by spaces: 01 02 03 04");
        dataLabel = new QLabel("Data:");
        form->addRow(dataLabel, dataEdit);

        periodSpin = new QSpinBox();
        periodSpin->setRange(1, 60000);
        periodSpin->setValue(1000);
        periodSpin->setSuffix(" ms");
        periodLabel = new QLabel("Period:");
        form->addRow(periodLabel, periodSpin);

        QDialogButtonBox *btns = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

        QVBoxLayout *main = new QVBoxLayout(this);
        main->addLayout(form);
        main->addWidget(btns);

        connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &AddFrameDialog::onTypeChanged);
        onTypeChanged(0);
    }

    int     frameType() const { return typeCombo->currentIndex(); } // 0=Read 1=Write 2=Slave
    QString id()        const { return idEdit->text().trimmed(); }
    QString data()      const { return dataEdit->text().trimmed(); }
    int     period()    const { return periodSpin->value(); }

private slots:
    void onTypeChanged(int idx)
    {
        bool needData   = (idx == 1 || idx == 2);
        bool needPeriod = (idx == 0 || idx == 1);
        dataEdit->setVisible(needData);
        dataLabel->setVisible(needData);
        periodSpin->setVisible(needPeriod);
        periodLabel->setVisible(needPeriod);
        adjustSize();
    }

private:
    QComboBox *typeCombo;
    QLineEdit *idEdit;
    QLineEdit *dataEdit;
    QLabel    *dataLabel;
    QSpinBox  *periodSpin;
    QLabel    *periodLabel;
    // Dans private:
    QList<int> m_rowTypes;   // 0=Read, 1=Write, 2=Slave

};

// ─── Fenêtre principale ───────────────────────────────────────────
class FrameSenderWindowLIN : public QDialog
{
    Q_OBJECT

public:
    explicit FrameSenderWindowLIN(const QVector<LINFrame> *frames,
                               QWidget *parent = nullptr);
    ~FrameSenderWindowLIN();

private slots:
    void onAddFrameClicked();
    void onCellChanged(int row, int col);
    void onCellDoubleTap(int row, int col);
    void handleTick();
    void updatedFrames(int numFrames);
    void enableAll();
    void disableAll();
    void clearGrid();
    void saveGrid();
    void loadGrid();
    void removeRow();
    void sendSlaveModeCommand();
    void onLinStartClickedForRow(int i);

    // ✅ Gardez CES déclarations ici (ou déplacez-les dans private:)
    void sendLinReadToDevice(uint8_t id, uint16_t period_ms);
    //void sendLinWriteToDevice(uint8_t id, const QByteArray &data);
    void sendLinWriteToDevice(uint8_t id, const QByteArray &data, uint16_t period_ms);
    void removeFrameFromDevice(uint8_t id, int rowType);
    void sendLinSlaveToDevice(uint8_t id, const QByteArray &data);

private:
    // ── UI ─────────────────────────────────────────────────────────
    Ui::FrameSenderWindowLIN *ui;
    bool m_disablingOtherModes = false;

    // ── Table ──────────────────────────────────────────────────────
    void setupGrid();
    void appendBlankRow(int typeIndex = 0);
    void refreshRowStyle(int row);
    void disableOtherModes(int activeRow, int activeType);
    bool isPublishedByMaster(uint32_t id);
    // ── Cell logic ─────────────────────────────────────────────────
    void processCellChange(int line, int c);
    void processTriggerText(int line);
    void processModifierText(int line);
    void parseOperandString(QStringList tokens, ModifierOperand &operand);
    ModifierOperationType parseOperation(QString op);
    void doModifiers(int idx);
    int  fetchOperand(int idx, ModifierOperand op);
    void updateGridRow(int idx);

    // ── Frame cache ────────────────────────────────────────────────
    void buildFrameCache();
    void processIncomingFrame(LINFrame *frame);
    LINFrame *lookupFrame(int ID, int bus);

    // ── LIN slave protocol ─────────────────────────────────────────
    void sendSlaveResponse(int row);
    void removeSlaveResponse(uint8_t id);

    // ── Save / Load ────────────────────────────────────────────────
    void saveSenderFile(QString filename);
    void loadSenderFile(QString filename);

    void pushRowToFirmware(int row);



    // LIN Slave
    void sendLinSlaveMode();                 // 0x04
    void sendLinSlaveResponse(uint8_t id, const QByteArray &data); // 0x05
    void removeLinSlaveResponse(uint8_t id); // 0xF6
    bool eventFilter(QObject *obj, QEvent *event) override;

    // ── Data ───────────────────────────────────────────────────────
    const QVector<LINFrame> *modelFrames = nullptr;
    dbHandler               *dbHandler   = nullptr;

    QList<FrameSendData>     sendingData;
    QList<SlaveResponse>     slaveData;
    QMap<uint32_t, LINFrame> frameCache;

    QTimer        *intervalTimer  = nullptr;
    QElapsedTimer  elapsedTimer;
    QMutex         mutex;

    bool           inhibitChanged = false;
    int            m_lastSentMode = -1;

    FrameSendData *sendData       = nullptr;
    TriggerDialog *td             = nullptr;
};

#endif // FRAMESENDERWINDOW_H