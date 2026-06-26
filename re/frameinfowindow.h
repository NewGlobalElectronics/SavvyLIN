#ifndef FRAMEINFOWINDOW_H
#define FRAMEINFOWINDOW_H

#include <QDialog>
#include <QFile>
#include <QListWidget>
#include <QTreeWidget>
#include <candatagrid.h>
#include "can_structs.h"
#include "bus_protocols/j1939_handler.h"
#include "db/dbhandler.h"

#include "qcustomplot.h"

namespace Ui {
class FrameInfoWindowLIN;
}

class FrameInfoWindowLIN : public QDialog
{
    Q_OBJECT

public:
    explicit FrameInfoWindowLIN(const QVector<LINFrame> *frames, QWidget *parent = 0);
    ~FrameInfoWindowLIN();
    void showEvent(QShowEvent*);

private slots:
    void updateDetailsWindow(QString);
    void updatedFrames(int);
    void saveDetails();
    void mousePress();
    void mouseWheel();
    void mouseDoubleClick();

private:
    Ui::FrameInfoWindowLIN *ui;
    QCustomPlot *graphByte[8];
    QCustomPlot *graphHistogram;
    CANDataGrid *heatmap;

    QList<int> foundID;
    QList<LINFrame> frameCache;
    const QVector<LINFrame> *modelFrames;
    bool useOpenGL;
    bool useHexTicker;
    static const QColor byteGraphColors[8];
    static QPen bytePens[8];
    dbHandler *dbHandler;

    QCPGraph *graphRef[8];

    void refreshIDList();
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void setupByteGraph(QCustomPlot *plot, int num);
    void readSettings();
    void writeSettings();
    void dumpNode(QTreeWidgetItem* item, QFile *file, int indent);
};

#endif // FRAMEINFOWINDOW_H
