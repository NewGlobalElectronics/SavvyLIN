#ifndef SCRIPTINGWINDOW_H
#define SCRIPTINGWINDOW_H

#include "scriptcontainer.h"
#include "can_structs.h"
#include "connections/canconnection.h"
#include "jsedit.h"

#include <QDialog>
#include <QJSEngine>

class ScriptContainer;

namespace Ui {
class ScriptingWindowLIN;
}

class ScriptingWindowLIN : public QDialog
{
    Q_OBJECT

public:
    explicit ScriptingWindowLIN(const QVector<LINFrame> *frames, QWidget *parent = 0);
    void showEvent(QShowEvent*);
    ~ScriptingWindowLIN();

public slots:
    void log(QString text);

signals:
    void updateValueTable(QTableWidget *widget);
    void updatedParameter(QString name, QString value);

private slots:
    void loadNewScript();
    void createNewScript();
    void deleteCurrentScript();
    void refreshSourceWindow();
    void saveScript();
    void saveAsScript();
    void revertScript();
    void reloadScript();
    void recompileScript();
    void changeCurrentScript();
    void newFrames(const CANConnection*, const QVector<LINFrame>&);
    void clickedLogClear();
    void valuesTimerElapsed();
    void updatedValue(int row, int col);

private:
    void closeEvent(QCloseEvent *event);
    void readSettings();
    void writeSettings();
    void saveLog();
    bool eventFilter(QObject *obj, QEvent *event);

    Ui::ScriptingWindowLIN *ui;
    JSEdit *editor;
    QList<ScriptContainer *> scripts;
    ScriptContainer *currentScript;
    const QVector<LINFrame> *modelFrames;
    QElapsedTimer elapsedTime;
    QTimer valuesTimer;
};

#endif // SCRIPTINGWINDOW_H
