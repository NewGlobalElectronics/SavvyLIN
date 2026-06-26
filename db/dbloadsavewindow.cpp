#include "dbloadsavewindow.h"
#include "ui_dbloadsavewindow.h"
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <qevent.h>
#include "helpwindow.h"
#include "connections/canconmanager.h"

dbLoadSaveWindow::dbLoadSaveWindow(const QVector<LINFrame> *frames, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dbLoadSaveWindow)
{
    setWindowFlags(Qt::Window);

    dbHandler = dbHandler::getReference();
    referenceFrames = frames;

    ui->setupUi(this);

    inhibitCellProcessing = true;

    QStringList header;
    header << "Filename" << "Associated Bus" << "Matching criteria" << "Label filters";
    ui->tableFiles->setColumnCount(4);
    ui->tableFiles->setHorizontalHeaderLabels(header);
    ui->tableFiles->setColumnWidth(0, 265);
    ui->tableFiles->setColumnWidth(1, 125);
    ui->tableFiles->setColumnWidth(2, 120);
    ui->tableFiles->setColumnWidth(3, 90);
    ui->tableFiles->horizontalHeader()->setStretchLastSection(true);

    // Populate table
    for (int idx=0; idx<dbHandler->getFileCount(); idx++)
    {
        dbFile * file = dbHandler->getFileByIdx(idx);
        ui->tableFiles->insertRow(ui->tableFiles->rowCount());
        ui->tableFiles->setItem(idx, 0, new QTableWidgetItem(file->getFilename()));
        QString bus = QString::number(file->getAssocBus() );
        ui->tableFiles->setItem(idx, 1, new QTableWidgetItem(bus));

        QComboBox * mc_item = addMatchingCriteriaCombobox(idx);
        int mc = (int)file->messageHandler->getMatchingCriteria();
        mc_item->setCurrentIndex(mc);

        QTableWidgetItem *item = new QTableWidgetItem("");
        ui->tableFiles->setItem(idx, 3, item);
        bool filterLabeling = file->messageHandler->filterLabeling();
        if (filterLabeling)
        {
            item->setCheckState(Qt::Checked);
        }
        else
        {
            item->setCheckState(Qt::Unchecked);
        }

        qDebug() << "Populate db table:" << file->getFullFilename() << " (bus:" << bus << " - Matching Criteria:" << mc
                 << "Filter labeling: " << (filterLabeling?"enabled":"disabled") << ")";
    }

    connect(ui->btnEdit, &QAbstractButton::clicked, this, &dbLoadSaveWindow::editFile);
    connect(ui->btnLoad, &QAbstractButton::clicked, this, &dbLoadSaveWindow::loadFile);
    connect(ui->btnMoveDown, &QAbstractButton::clicked, this, &dbLoadSaveWindow::moveDown);
    connect(ui->btnMoveUp, &QAbstractButton::clicked, this, &dbLoadSaveWindow::moveUp);
    connect(ui->btnRemove, &QAbstractButton::clicked, this, &dbLoadSaveWindow::removeFile);
    connect(ui->btnSave, &QAbstractButton::clicked, this, &dbLoadSaveWindow::saveFile);
    connect(ui->btnNewdb, &QAbstractButton::clicked, this, &dbLoadSaveWindow::newFile);
    connect(ui->tableFiles, &QTableWidget::cellChanged, this, &dbLoadSaveWindow::cellChanged);
    connect(ui->tableFiles, &QTableWidget::cellDoubleClicked, this, &dbLoadSaveWindow::cellDoubleClicked);

    editorWindow = new dbMainEditor(frames, this);
    currentlyEditingFile = nullptr;

    inhibitCellProcessing = false;

    installEventFilter(this);
}

QComboBox * dbLoadSaveWindow::addMatchingCriteriaCombobox(int row)
{
    QComboBox *item = new QComboBox();
    item->addItem("Exact");
    // J1939 and GMLAN are disabled for LIN
    // item->addItem("J1939");
    // item->addItem("GMLAN");
    ui->tableFiles->setCellWidget(row, 2, item);
    connect(item, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [this](int box_idx) { matchingCriteriaChanged(box_idx); } );
    return item;
}

dbLoadSaveWindow::~dbLoadSaveWindow()
{
    removeEventFilter(this);
    delete ui;
}

void dbLoadSaveWindow::updateSettings()
{
    QSettings settings;
    int filecount = ui->tableFiles->rowCount();
    settings.setValue("db/FileCount", filecount);
    for (int i=0; i<filecount; i++)
    {
        dbFile * file = dbHandler->getFileByIdx(i);
        if (file)
        {
            qDebug() << "Save db settings #" << i << " File: " << file->getFullFilename()
            << "Bus: " << file->getAssocBus() << "MC: " << file->messageHandler->getMatchingCriteria()
            << "Filter Labeling: " << (file->messageHandler->filterLabeling() ? "enabled" : "disabled");
            settings.setValue("db/Filename_" + QString::number(i), file->getFullFilename());
            settings.setValue("db/AssocBus_" + QString::number(i), file->getAssocBus());
            settings.setValue("db/MatchingCriteria_" + QString::number(i), file->messageHandler->getMatchingCriteria());
            settings.setValue("db/FilterLabeling_" + QString::number(i), file->messageHandler->filterLabeling() ? 1 : 0);
        }
    }
    emit updateddbSettings();
}

bool dbLoadSaveWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key())
        {
        case Qt::Key_F1:
            HelpWindow::getRef()->showHelp("db_manager.md");
            break;
        }
        return true;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
    return false;
}

void dbLoadSaveWindow::newFile()
{
    int idx = dbHandler->createBlankFile();
    idx = ui->tableFiles->rowCount();
    ui->tableFiles->insertRow(ui->tableFiles->rowCount());
    ui->tableFiles->setItem(idx, 0, new QTableWidgetItem("UNNAMEDFILE"));
    ui->tableFiles->setItem(idx, 1, new QTableWidgetItem("0"));

    QComboBox * mc_item = addMatchingCriteriaCombobox(idx);
    mc_item->setCurrentIndex(EXACT_DB);

    QTableWidgetItem *item = new QTableWidgetItem("");
    item->setCheckState(Qt::Checked);
    ui->tableFiles->setItem(idx, 3, item);
}

void dbLoadSaveWindow::loadFile()
{
    dbFile *file = nullptr;
    QString filename;
    QFileDialog dialog;
    QSettings settings;

    // Only support LIN Description Files (.ldf)
    QStringList filters;
    filters.append(QString(tr("LIN Description File (*.ldf)")));

    dialog.setDirectory(settings.value("db/LoadSaveDirectory", dialog.directory().path()).toString());
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilters(filters);
    dialog.setViewMode(QFileDialog::Detail);

    if (dialog.exec() == QDialog::Accepted)
    {
        filename = dialog.selectedFiles()[0];
        settings.setValue("db/LoadSaveDirectory", dialog.directory().path());

        // Load the .ldf file
        file = dbHandler->loaddbFile(filename);
    }

    if(file) {
        inhibitCellProcessing=true;
        int idx = ui->tableFiles->rowCount();
        ui->tableFiles->insertRow(ui->tableFiles->rowCount());
        ui->tableFiles->setItem(idx, 0, new QTableWidgetItem(file->getFilename()));
        ui->tableFiles->setItem(idx, 1, new QTableWidgetItem("0"));

        db_ATTRIBUTE *attr = file->findAttributeByName("matchingcriteria");
        QComboBox * mc_item = addMatchingCriteriaCombobox(idx);
        if (attr && attr->defaultValue.toInt() > 0)
        {
            mc_item->setCurrentIndex(attr->defaultValue.toInt());
        }

        attr = file->findAttributeByName("filterlabeling");
        QTableWidgetItem *item = new QTableWidgetItem("");
        ui->tableFiles->setItem(idx, 3, item);
        if (attr && attr->defaultValue.toInt() > 0)
        {
            item->setCheckState(Qt::Checked);
        }
        else
        {
            item->setCheckState(Qt::Unchecked);
        }
        inhibitCellProcessing=false;

        updateSettings();
    }
}

void dbLoadSaveWindow::loadJSON()
{
    // Not used in LIN version
}

void dbLoadSaveWindow::saveFile()
{
    int idx = ui->tableFiles->currentRow();
    if (idx < 0) return;
    dbHandler->savedbFile(idx);
    // then update the list to show the new file name (if it changed)
    ui->tableFiles->setItem(idx, 0, new QTableWidgetItem(dbHandler->getFileByIdx(idx)->getFilename()));
}

void dbLoadSaveWindow::removeFile()
{
    bool bContinue = true;
    int idx = ui->tableFiles->currentRow();
    if (idx < 0) return;

    if (currentlyEditingFile == dbHandler->getFileByIdx(idx))
    {
        bContinue = false;
        QMessageBox::StandardButton confirmDialog;
        confirmDialog = QMessageBox::question(this, "Confirm Deletion", "This db is currently open for editing.\nMake sure you've saved any changes!\nAre you sure you want to remove this db?",
                                              QMessageBox::Yes|QMessageBox::No);
        if (confirmDialog == QMessageBox::Yes) bContinue = true;
    }

    if (bContinue)
    {
        editorWindow->close();
        dbHandler->removedbFile(idx);
        ui->tableFiles->removeRow(idx);
    }
    updateSettings();
}

void dbLoadSaveWindow::moveUp()
{
    int idx = ui->tableFiles->currentRow();
    if (idx < 1) return;
    dbHandler->swapFiles(idx - 1, idx);
    swapTableRows(true);
    updateSettings();
}

void dbLoadSaveWindow::moveDown()
{
    int idx = ui->tableFiles->currentRow();
    if (idx < 0) return;
    if (idx > (dbHandler->getFileCount() - 2)) return;
    dbHandler->swapFiles(idx, idx + 1);
    swapTableRows(false);
    updateSettings();
}

void dbLoadSaveWindow::editFile()
{
    int idx = ui->tableFiles->currentRow();
    if (idx < 0) return;

    editorWindow->setFileIdx(idx);
    editorWindow->show();
}

void dbLoadSaveWindow::matchingCriteriaChanged(int index)
{
    Q_UNUSED(index)

    if (inhibitCellProcessing) return;
    // We don't know which combobox changed, so we just update all of them
    for (int row=0; row<ui->tableFiles->rowCount(); row++)
    {
        dbFile *file = dbHandler->getFileByIdx(row);
        if (file)
        {
            QComboBox *item = (QComboBox*)ui->tableFiles->cellWidget(row, 2);
            MatchingCriteria_t matchingCriteria = (MatchingCriteria_t) item->currentIndex();
            db_ATTRIBUTE *attr = file->findAttributeByName("matchingcriteria");
            if (attr)
            {
                attr->defaultValue = matchingCriteria;
                file->messageHandler->setMatchingCriteria(matchingCriteria);
            }
            else
            {
                db_ATTRIBUTE attr;

                attr.attrType = ATTR_TYPE_MESSAGE;
                attr.defaultValue = matchingCriteria;
                attr.enumVals.clear();
                attr.lower = 0;
                attr.upper = 0;
                attr.name = "matchingcriteria";
                attr.valType = ATTR_INT;
                file->db_attributes.append(attr);
                file->messageHandler->setMatchingCriteria(matchingCriteria);
            }
        }
    }
    updateSettings();
}

void dbLoadSaveWindow::cellChanged(int row, int col)
{
    if (inhibitCellProcessing) return;
    if (col == 1) // the bus column
    {
        dbFile *file = dbHandler->getFileByIdx(row);
        int bus = ui->tableFiles->item(row, col)->text().toInt();
        if (bus > -2)
        {
            file->setAssocBus(bus);
        }
        updateSettings();
    }
    else if (col == 3) // label filters
    {
        dbFile *file = dbHandler->getFileByIdx(row);
        if (file)
        {
            bool labelFilters = ui->tableFiles->item(row, col)->checkState() == Qt::Checked;
            db_ATTRIBUTE *attr = file->findAttributeByName("filterlabeling");
            if (attr)
            {
                attr->defaultValue = labelFilters ? 1 : 0;
                file->messageHandler->setFilterLabeling(labelFilters);
            }
            else
            {
                db_ATTRIBUTE attr;

                attr.attrType = ATTR_TYPE_MESSAGE;
                attr.defaultValue = labelFilters ? 1 : 0;
                attr.enumVals.clear();
                attr.lower = 0;
                attr.upper = 0;
                attr.name = "filterlabeling";
                attr.valType = ATTR_INT;
                file->db_attributes.append(attr);
                file->messageHandler->setFilterLabeling(labelFilters);
            }
            updateSettings();
        }
    }
}

void dbLoadSaveWindow::cellDoubleClicked(int row, int col)
{
    Q_UNUSED(col)
    currentlyEditingFile = dbHandler->getFileByIdx(row);
    editorWindow->setFileIdx(row);
    editorWindow->show();
}

void dbLoadSaveWindow::swapTableRows(bool up)
{
    int idx = ui->tableFiles->currentRow();
    const int destIdx = (up ? idx-1 : idx+1);
    Q_ASSERT(destIdx >= 0 && destIdx < ui->tableFiles->rowCount());

    inhibitCellProcessing = true;
    // take whole rows
    QList<QTableWidgetItem*> sourceItems = takeRow(idx);
    QList<QTableWidgetItem*> destItems = takeRow(destIdx);

    // QCombobox needs separate handling
    int sourceMC = ((QComboBox*)ui->tableFiles->cellWidget(idx,2))->currentIndex();
    int destMC = ((QComboBox*)ui->tableFiles->cellWidget(destIdx,2))->currentIndex();

    // set back in reverse order
    setRow(idx, destItems);
    setRow(destIdx, sourceItems);

    ((QComboBox*)ui->tableFiles->cellWidget(idx,2))->setCurrentIndex(destMC);
    ((QComboBox*)ui->tableFiles->cellWidget(destIdx,2))->setCurrentIndex(sourceMC);

    inhibitCellProcessing = false;
}

QList<QTableWidgetItem*> dbLoadSaveWindow::takeRow(int row)
{
    QList<QTableWidgetItem*> rowItems;
    for (int col = 0; col < ui->tableFiles->columnCount(); ++col)
    {
        rowItems << ui->tableFiles->takeItem(row, col);
    }
    return rowItems;
}

void dbLoadSaveWindow::setRow(int row, const QList<QTableWidgetItem*>& rowItems)
{
    for (int col = 0; col < ui->tableFiles->columnCount(); ++col)
    {
        ui->tableFiles->setItem(row, col, rowItems.at(col));
    }
}