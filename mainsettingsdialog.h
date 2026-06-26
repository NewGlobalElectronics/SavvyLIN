#ifndef MAINSETTINGSDIALOG_H
#define MAINSETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class MainSettingsDialogLIN;
}

class MainSettingsDialogLIN : public QDialog
{
    Q_OBJECT

public:
    explicit MainSettingsDialogLIN(QWidget *parent = 0);
    ~MainSettingsDialogLIN();

signals:
    void updatedSettings();

public slots:
    void updateSettings();

private:
    Ui::MainSettingsDialogLIN *ui;

    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // MAINSETTINGSDIALOG_H
