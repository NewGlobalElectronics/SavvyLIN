/********************************************************************************
** Form generated from reading UI file 'dbcsignaleditor.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DBCSIGNALEDITOR_H
#define UI_DBCSIGNALEDITOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include "candatagrid.h"

QT_BEGIN_NAMESPACE

class Ui_DBCSignalEditor
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QLineEdit *txtName;
    CANDataGrid *bitfield;
    QFormLayout *formLayout_2;
    QLabel *label_4;
    QLineEdit *txtBitLength;
    QLabel *label_5;
    QCheckBox *cbIntelFormat;
    QLabel *label_6;
    QComboBox *comboType;
    QLabel *label_7;
    QLineEdit *txtScale;
    QLabel *label_8;
    QLineEdit *txtBias;
    QLabel *label_9;
    QLineEdit *txtMinVal;
    QLabel *label_10;
    QLineEdit *txtMaxVal;
    QLabel *label_12;
    QLineEdit *txtUnitName;
    QLabel *label_11;
    QComboBox *comboReceiver;
    QLabel *label_14;
    QHBoxLayout *horizontalLayout_2;
    QRadioButton *rbNotMulti;
    QRadioButton *rbMultiplexed;
    QRadioButton *rbMultiplexor;
    QRadioButton *rbExtended;
    QLabel *label_15;
    QLineEdit *txtMultiplexValues;
    QLabel *label_16;
    QComboBox *cbMultiplexParent;
    QLabel *label_13;
    QLineEdit *txtComment;
    QLabel *label_3;
    QLabel *label_2;
    QTableWidget *valuesTable;

    void setupUi(QDialog *DBCSignalEditor)
    {
        if (DBCSignalEditor->objectName().isEmpty())
            DBCSignalEditor->setObjectName("DBCSignalEditor");
        DBCSignalEditor->resize(995, 644);
        verticalLayout = new QVBoxLayout(DBCSignalEditor);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName("verticalLayout_2");
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        label = new QLabel(DBCSignalEditor);
        label->setObjectName("label");

        horizontalLayout_3->addWidget(label);

        txtName = new QLineEdit(DBCSignalEditor);
        txtName->setObjectName("txtName");

        horizontalLayout_3->addWidget(txtName);


        verticalLayout_2->addLayout(horizontalLayout_3);

        bitfield = new CANDataGrid(DBCSignalEditor);
        bitfield->setObjectName("bitfield");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(bitfield->sizePolicy().hasHeightForWidth());
        bitfield->setSizePolicy(sizePolicy);
        bitfield->setMinimumSize(QSize(300, 300));

        verticalLayout_2->addWidget(bitfield);


        horizontalLayout->addLayout(verticalLayout_2);

        formLayout_2 = new QFormLayout();
        formLayout_2->setObjectName("formLayout_2");
        label_4 = new QLabel(DBCSignalEditor);
        label_4->setObjectName("label_4");

        formLayout_2->setWidget(0, QFormLayout::ItemRole::LabelRole, label_4);

        txtBitLength = new QLineEdit(DBCSignalEditor);
        txtBitLength->setObjectName("txtBitLength");

        formLayout_2->setWidget(0, QFormLayout::ItemRole::FieldRole, txtBitLength);

        label_5 = new QLabel(DBCSignalEditor);
        label_5->setObjectName("label_5");

        formLayout_2->setWidget(1, QFormLayout::ItemRole::LabelRole, label_5);

        cbIntelFormat = new QCheckBox(DBCSignalEditor);
        cbIntelFormat->setObjectName("cbIntelFormat");

        formLayout_2->setWidget(1, QFormLayout::ItemRole::FieldRole, cbIntelFormat);

        label_6 = new QLabel(DBCSignalEditor);
        label_6->setObjectName("label_6");

        formLayout_2->setWidget(2, QFormLayout::ItemRole::LabelRole, label_6);

        comboType = new QComboBox(DBCSignalEditor);
        comboType->setObjectName("comboType");

        formLayout_2->setWidget(2, QFormLayout::ItemRole::FieldRole, comboType);

        label_7 = new QLabel(DBCSignalEditor);
        label_7->setObjectName("label_7");

        formLayout_2->setWidget(3, QFormLayout::ItemRole::LabelRole, label_7);

        txtScale = new QLineEdit(DBCSignalEditor);
        txtScale->setObjectName("txtScale");

        formLayout_2->setWidget(3, QFormLayout::ItemRole::FieldRole, txtScale);

        label_8 = new QLabel(DBCSignalEditor);
        label_8->setObjectName("label_8");

        formLayout_2->setWidget(4, QFormLayout::ItemRole::LabelRole, label_8);

        txtBias = new QLineEdit(DBCSignalEditor);
        txtBias->setObjectName("txtBias");

        formLayout_2->setWidget(4, QFormLayout::ItemRole::FieldRole, txtBias);

        label_9 = new QLabel(DBCSignalEditor);
        label_9->setObjectName("label_9");

        formLayout_2->setWidget(5, QFormLayout::ItemRole::LabelRole, label_9);

        txtMinVal = new QLineEdit(DBCSignalEditor);
        txtMinVal->setObjectName("txtMinVal");

        formLayout_2->setWidget(5, QFormLayout::ItemRole::FieldRole, txtMinVal);

        label_10 = new QLabel(DBCSignalEditor);
        label_10->setObjectName("label_10");

        formLayout_2->setWidget(6, QFormLayout::ItemRole::LabelRole, label_10);

        txtMaxVal = new QLineEdit(DBCSignalEditor);
        txtMaxVal->setObjectName("txtMaxVal");

        formLayout_2->setWidget(6, QFormLayout::ItemRole::FieldRole, txtMaxVal);

        label_12 = new QLabel(DBCSignalEditor);
        label_12->setObjectName("label_12");

        formLayout_2->setWidget(7, QFormLayout::ItemRole::LabelRole, label_12);

        txtUnitName = new QLineEdit(DBCSignalEditor);
        txtUnitName->setObjectName("txtUnitName");

        formLayout_2->setWidget(7, QFormLayout::ItemRole::FieldRole, txtUnitName);

        label_11 = new QLabel(DBCSignalEditor);
        label_11->setObjectName("label_11");

        formLayout_2->setWidget(8, QFormLayout::ItemRole::LabelRole, label_11);

        comboReceiver = new QComboBox(DBCSignalEditor);
        comboReceiver->setObjectName("comboReceiver");

        formLayout_2->setWidget(8, QFormLayout::ItemRole::FieldRole, comboReceiver);

        label_14 = new QLabel(DBCSignalEditor);
        label_14->setObjectName("label_14");

        formLayout_2->setWidget(9, QFormLayout::ItemRole::LabelRole, label_14);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        rbNotMulti = new QRadioButton(DBCSignalEditor);
        rbNotMulti->setObjectName("rbNotMulti");

        horizontalLayout_2->addWidget(rbNotMulti);

        rbMultiplexed = new QRadioButton(DBCSignalEditor);
        rbMultiplexed->setObjectName("rbMultiplexed");

        horizontalLayout_2->addWidget(rbMultiplexed);

        rbMultiplexor = new QRadioButton(DBCSignalEditor);
        rbMultiplexor->setObjectName("rbMultiplexor");

        horizontalLayout_2->addWidget(rbMultiplexor);

        rbExtended = new QRadioButton(DBCSignalEditor);
        rbExtended->setObjectName("rbExtended");

        horizontalLayout_2->addWidget(rbExtended);


        formLayout_2->setLayout(9, QFormLayout::ItemRole::FieldRole, horizontalLayout_2);

        label_15 = new QLabel(DBCSignalEditor);
        label_15->setObjectName("label_15");

        formLayout_2->setWidget(10, QFormLayout::ItemRole::LabelRole, label_15);

        txtMultiplexValues = new QLineEdit(DBCSignalEditor);
        txtMultiplexValues->setObjectName("txtMultiplexValues");

        formLayout_2->setWidget(10, QFormLayout::ItemRole::FieldRole, txtMultiplexValues);

        label_16 = new QLabel(DBCSignalEditor);
        label_16->setObjectName("label_16");

        formLayout_2->setWidget(12, QFormLayout::ItemRole::LabelRole, label_16);

        cbMultiplexParent = new QComboBox(DBCSignalEditor);
        cbMultiplexParent->setObjectName("cbMultiplexParent");

        formLayout_2->setWidget(12, QFormLayout::ItemRole::FieldRole, cbMultiplexParent);

        label_13 = new QLabel(DBCSignalEditor);
        label_13->setObjectName("label_13");

        formLayout_2->setWidget(13, QFormLayout::ItemRole::LabelRole, label_13);

        txtComment = new QLineEdit(DBCSignalEditor);
        txtComment->setObjectName("txtComment");

        formLayout_2->setWidget(13, QFormLayout::ItemRole::FieldRole, txtComment);

        label_3 = new QLabel(DBCSignalEditor);
        label_3->setObjectName("label_3");

        formLayout_2->setWidget(11, QFormLayout::ItemRole::FieldRole, label_3);


        horizontalLayout->addLayout(formLayout_2);

        horizontalLayout->setStretch(0, 2);
        horizontalLayout->setStretch(1, 1);

        verticalLayout->addLayout(horizontalLayout);

        label_2 = new QLabel(DBCSignalEditor);
        label_2->setObjectName("label_2");

        verticalLayout->addWidget(label_2);

        valuesTable = new QTableWidget(DBCSignalEditor);
        valuesTable->setObjectName("valuesTable");

        verticalLayout->addWidget(valuesTable);

        verticalLayout->setStretch(0, 10);
        verticalLayout->setStretch(1, 1);
        verticalLayout->setStretch(2, 4);
        QWidget::setTabOrder(txtBitLength, cbIntelFormat);
        QWidget::setTabOrder(cbIntelFormat, comboType);
        QWidget::setTabOrder(comboType, txtScale);
        QWidget::setTabOrder(txtScale, txtBias);
        QWidget::setTabOrder(txtBias, txtMinVal);
        QWidget::setTabOrder(txtMinVal, txtMaxVal);
        QWidget::setTabOrder(txtMaxVal, txtUnitName);
        QWidget::setTabOrder(txtUnitName, comboReceiver);
        QWidget::setTabOrder(comboReceiver, rbNotMulti);
        QWidget::setTabOrder(rbNotMulti, rbMultiplexed);
        QWidget::setTabOrder(rbMultiplexed, rbMultiplexor);
        QWidget::setTabOrder(rbMultiplexor, rbExtended);
        QWidget::setTabOrder(rbExtended, txtMultiplexValues);
        QWidget::setTabOrder(txtMultiplexValues, cbMultiplexParent);
        QWidget::setTabOrder(cbMultiplexParent, txtComment);
        QWidget::setTabOrder(txtComment, valuesTable);

        retranslateUi(DBCSignalEditor);

        QMetaObject::connectSlotsByName(DBCSignalEditor);
    } // setupUi

    void retranslateUi(QDialog *DBCSignalEditor)
    {
        DBCSignalEditor->setWindowTitle(QCoreApplication::translate("DBCSignalEditor", "Signal Editor", nullptr));
        label->setText(QCoreApplication::translate("DBCSignalEditor", "Name:", nullptr));
        label_4->setText(QCoreApplication::translate("DBCSignalEditor", "Bit Length:", nullptr));
        label_5->setText(QCoreApplication::translate("DBCSignalEditor", "Byte Order", nullptr));
        cbIntelFormat->setText(QCoreApplication::translate("DBCSignalEditor", "LSB First (Little Endian)", nullptr));
        label_6->setText(QCoreApplication::translate("DBCSignalEditor", "Type:", nullptr));
        label_7->setText(QCoreApplication::translate("DBCSignalEditor", "Scale:", nullptr));
        label_8->setText(QCoreApplication::translate("DBCSignalEditor", "Bias:", nullptr));
        label_9->setText(QCoreApplication::translate("DBCSignalEditor", "Min Value:", nullptr));
        label_10->setText(QCoreApplication::translate("DBCSignalEditor", "Max Value:", nullptr));
        label_12->setText(QCoreApplication::translate("DBCSignalEditor", "Units Name:", nullptr));
        label_11->setText(QCoreApplication::translate("DBCSignalEditor", "Receiving Node:", nullptr));
        label_14->setText(QCoreApplication::translate("DBCSignalEditor", "Multiplexing", nullptr));
        rbNotMulti->setText(QCoreApplication::translate("DBCSignalEditor", "None", nullptr));
        rbMultiplexed->setText(QCoreApplication::translate("DBCSignalEditor", "Multiplexed", nullptr));
        rbMultiplexor->setText(QCoreApplication::translate("DBCSignalEditor", "Multiplexor", nullptr));
        rbExtended->setText(QCoreApplication::translate("DBCSignalEditor", "Extended", nullptr));
        label_15->setText(QCoreApplication::translate("DBCSignalEditor", "Multiplex value", nullptr));
        label_16->setText(QCoreApplication::translate("DBCSignalEditor", "Multiplex Parent", nullptr));
        label_13->setText(QCoreApplication::translate("DBCSignalEditor", "Comment:", nullptr));
        label_3->setText(QCoreApplication::translate("DBCSignalEditor", "Single values has to be separated by semicolons\n"
"and the value ranges by hypens\n"
"(e.g.: 1;2;5-7)", nullptr));
        label_2->setText(QCoreApplication::translate("DBCSignalEditor", "Value Table:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DBCSignalEditor: public Ui_DBCSignalEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DBCSIGNALEDITOR_H
