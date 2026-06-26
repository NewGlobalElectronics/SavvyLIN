#ifndef TEMPORALGRAPHWINDOW_H
#define TEMPORALGRAPHWINDOW_H

#include <QDialog>
#include "qcustomplot.h"
#include "can_structs.h"

namespace Ui {
class TemporalGraphWindowLIN;
}

class HexTicker : public QCPAxisTicker
{
    QString getTickLabel (double tick, const QLocale& locale, QChar formatChar, int precision);
};

class TemporalGraphWindowLIN : public QDialog
{
    Q_OBJECT

public:
    explicit TemporalGraphWindowLIN(const QVector<LINFrame> *, QWidget *parent = nullptr);
    ~TemporalGraphWindowLIN();
    void showEvent(QShowEvent*);

private slots:
    void updatedFrames(int);
    void mousePress();
    void mouseWheel();
    void resetView();
    void zoomIn();
    void zoomOut();
    void selectionChanged();

private:
    Ui::TemporalGraphWindowLIN *ui;    
    const QVector<LINFrame> *modelFrames;
    bool useOpenGL;
    bool followGraphEnd;
    QCPGraph *graph;
    double xminval, xmaxval, yminval, ymaxval;
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void readSettings();
    void writeSettings();
    void generateGraph();

};

#endif // TEMPORALGRAPHWINDOW_H
