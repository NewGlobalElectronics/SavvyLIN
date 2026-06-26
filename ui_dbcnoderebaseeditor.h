/********************************************************************************
** Form generated from reading UI file 'dbcnoderebaseeditor.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DBCNODEREBASEEDITOR_H
#define UI_DBCNODEREBASEEDITOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DBCNodeRebaseEditor
{
public:
    QWidget *layoutWidget;
    QFormLayout *formLayout;
    QLabel *label_2;
    QLineEdit *lineNodeName;
    QLabel *label_8;
    QLineEdit *lineOriginalBaseId;
    QLabel *label;
    QLineEdit *lineNewBaseId;
    QPushButton *btnDoRebase;
    QPushButton *btnCancel;

    void setupUi(QDialog *DBCNodeRebaseEditor)
    {
        if (DBCNodeRebaseEditor->objectName().isEmpty())
            DBCNodeRebaseEditor->setObjectName("DBCNodeRebaseEditor");
        DBCNodeRebaseEditor->resize(303, 148);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(DBCNodeRebaseEditor->sizePolicy().hasHeightForWidth());
        DBCNodeRebaseEditor->setSizePolicy(sizePolicy);
        layoutWidget = new QWidget(DBCNodeRebaseEditor);
        layoutWidget->setObjectName("layoutWidget");
        layoutWidget->setGeometry(QRect(10, 20, 283, 116));
        formLayout = new QFormLayout(layoutWidget);
        formLayout->setObjectName("formLayout");
        formLayout->setContentsMargins(0, 0, 0, 0);
        label_2 = new QLabel(layoutWidget);
        label_2->setObjectName("label_2");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, label_2);

        lineNodeName = new QLineEdit(layoutWidget);
        lineNodeName->setObjectName("lineNodeName");
        lineNodeName->setEnabled(true);
        lineNodeName->setReadOnly(true);

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, lineNodeName);

        label_8 = new QLabel(layoutWidget);
        label_8->setObjectName("label_8");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, label_8);

        lineOriginalBaseId = new QLineEdit(layoutWidget);
        lineOriginalBaseId->setObjectName("lineOriginalBaseId");
        lineOriginalBaseId->setEnabled(true);
        lineOriginalBaseId->setReadOnly(true);

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, lineOriginalBaseId);

        label = new QLabel(layoutWidget);
        label->setObjectName("label");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, label);

        lineNewBaseId = new QLineEdit(layoutWidget);
        lineNewBaseId->setObjectName("lineNewBaseId");

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, lineNewBaseId);

        btnDoRebase = new QPushButton(layoutWidget);
        btnDoRebase->setObjectName("btnDoRebase");

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, btnDoRebase);

        btnCancel = new QPushButton(layoutWidget);
        btnCancel->setObjectName("btnCancel");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, btnCancel);


        retranslateUi(DBCNodeRebaseEditor);

        QMetaObject::connectSlotsByName(DBCNodeRebaseEditor);
    } // setupUi

    void retranslateUi(QDialog *DBCNodeRebaseEditor)
    {
        DBCNodeRebaseEditor->setWindowTitle(QCoreApplication::translate("DBCNodeRebaseEditor", "Rebase All Node Messages", nullptr));
        label_2->setText(QCoreApplication::translate("DBCNodeRebaseEditor", "Node Name:", nullptr));
        label_8->setText(QCoreApplication::translate("DBCNodeRebaseEditor", "Current Base ID:", nullptr));
        label->setText(QCoreApplication::translate("DBCNodeRebaseEditor", "New Base ID:", nullptr));
        btnDoRebase->setText(QCoreApplication::translate("DBCNodeRebaseEditor", "Rebase All Messages in Node", nullptr));
        btnCancel->setText(QCoreApplication::translate("DBCNodeRebaseEditor", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DBCNodeRebaseEditor: public Ui_DBCNodeRebaseEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DBCNODEREBASEEDITOR_H
