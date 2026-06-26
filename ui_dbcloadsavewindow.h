/********************************************************************************
** Form generated from reading UI file 'dbcloadsavewindow.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DBCLOADSAVEWINDOW_H
#define UI_DBCLOADSAVEWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DBCLoadSaveWindow
{
public:
    QVBoxLayout *verticalLayout;
    QTableWidget *tableFiles;
    QPushButton *btnNewDBC;
    QHBoxLayout *horizontalLayout;
    QPushButton *btnLoad;
    QPushButton *btnRemove;
    QPushButton *btnMoveUp;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *btnSave;
    QPushButton *btnEdit;
    QPushButton *btnMoveDown;

    void setupUi(QDialog *DBCLoadSaveWindow)
    {
        if (DBCLoadSaveWindow->objectName().isEmpty())
            DBCLoadSaveWindow->setObjectName("DBCLoadSaveWindow");
        DBCLoadSaveWindow->resize(680, 665);
        verticalLayout = new QVBoxLayout(DBCLoadSaveWindow);
        verticalLayout->setObjectName("verticalLayout");
        tableFiles = new QTableWidget(DBCLoadSaveWindow);
        tableFiles->setObjectName("tableFiles");

        verticalLayout->addWidget(tableFiles);

        btnNewDBC = new QPushButton(DBCLoadSaveWindow);
        btnNewDBC->setObjectName("btnNewDBC");

        verticalLayout->addWidget(btnNewDBC);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        btnLoad = new QPushButton(DBCLoadSaveWindow);
        btnLoad->setObjectName("btnLoad");

        horizontalLayout->addWidget(btnLoad);

        btnRemove = new QPushButton(DBCLoadSaveWindow);
        btnRemove->setObjectName("btnRemove");

        horizontalLayout->addWidget(btnRemove);

        btnMoveUp = new QPushButton(DBCLoadSaveWindow);
        btnMoveUp->setObjectName("btnMoveUp");

        horizontalLayout->addWidget(btnMoveUp);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        btnSave = new QPushButton(DBCLoadSaveWindow);
        btnSave->setObjectName("btnSave");

        horizontalLayout_2->addWidget(btnSave);

        btnEdit = new QPushButton(DBCLoadSaveWindow);
        btnEdit->setObjectName("btnEdit");

        horizontalLayout_2->addWidget(btnEdit);

        btnMoveDown = new QPushButton(DBCLoadSaveWindow);
        btnMoveDown->setObjectName("btnMoveDown");

        horizontalLayout_2->addWidget(btnMoveDown);


        verticalLayout->addLayout(horizontalLayout_2);


        retranslateUi(DBCLoadSaveWindow);

        QMetaObject::connectSlotsByName(DBCLoadSaveWindow);
    } // setupUi

    void retranslateUi(QDialog *DBCLoadSaveWindow)
    {
        DBCLoadSaveWindow->setWindowTitle(QCoreApplication::translate("DBCLoadSaveWindow", "DBC File Manager", nullptr));
        btnNewDBC->setText(QCoreApplication::translate("DBCLoadSaveWindow", "Create new DBC", nullptr));
        btnLoad->setText(QCoreApplication::translate("DBCLoadSaveWindow", "Load", nullptr));
        btnRemove->setText(QCoreApplication::translate("DBCLoadSaveWindow", "Remove", nullptr));
        btnMoveUp->setText(QCoreApplication::translate("DBCLoadSaveWindow", "Move Up", nullptr));
        btnSave->setText(QCoreApplication::translate("DBCLoadSaveWindow", "Save", nullptr));
        btnEdit->setText(QCoreApplication::translate("DBCLoadSaveWindow", "Edit", nullptr));
        btnMoveDown->setText(QCoreApplication::translate("DBCLoadSaveWindow", "Move Down", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DBCLoadSaveWindow: public Ui_DBCLoadSaveWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DBCLOADSAVEWINDOW_H
