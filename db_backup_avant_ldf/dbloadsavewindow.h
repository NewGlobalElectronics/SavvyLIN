#ifndef dbLOADSAVEWINDOW_H
#define dbLOADSAVEWINDOW_H

#include <QDialog>
#include <QTableWidget>
#include <QComboBox>
#include "dbhandler.h"
#include "dbmaineditor.h"

namespace Ui {
class dbLoadSaveWindow;
}

class dbLoadSaveWindow : public QDialog
{
    Q_OBJECT

public:
    explicit dbLoadSaveWindow(const QVector<CANFrame> *frames, QWidget *parent = 0);
    ~dbLoadSaveWindow();

private slots:
    void loadFile();
    void loadJSON();
    void saveFile();
    void removeFile();
    void moveUp();
    void moveDown();
    void editFile();
    void cellChanged(int row, int col);
    void cellDoubleClicked(int row, int col);
    void matchingCriteriaChanged(int index);
    void newFile();

signals:
    void updateddbSettings();

private:
    Ui::dbLoadSaveWindow *ui;
    dbHandler *dbHandler;
    dbFile *currentlyEditingFile;
    const QVector<CANFrame> *referenceFrames;
    dbMainEditor *editorWindow;
    bool inhibitCellProcessing;

    void swapTableRows(bool up);
    QList<QTableWidgetItem*> takeRow(int row);
    void setRow(int row, const QList<QTableWidgetItem*>& rowItems);
    bool eventFilter(QObject *obj, QEvent *event);
    void updateSettings();
    QComboBox * addMatchingCriteriaCombobox(int row);
};

#endif // dbLOADSAVEWINDOW_H

