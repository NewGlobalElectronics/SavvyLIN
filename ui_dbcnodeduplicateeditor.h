/********************************************************************************
** Form generated from reading UI file 'dbcnodeduplicateeditor.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DBCNODEDUPLICATEEDITOR_H
#define UI_DBCNODEDUPLICATEEDITOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DBCNodeDuplicateEditor
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
    QPushButton *btnDuplicate;
    QPushButton *btnCancel;

    void setupUi(QDialog *DBCNodeDuplicateEditor)
    {
        if (DBCNodeDuplicateEditor->objectName().isEmpty())
            DBCNodeDuplicateEditor->setObjectName("DBCNodeDuplicateEditor");
        DBCNodeDuplicateEditor->resize(314, 144);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(DBCNodeDuplicateEditor->sizePolicy().hasHeightForWidth());
        DBCNodeDuplicateEditor->setSizePolicy(sizePolicy);
        layoutWidget = new QWidget(DBCNodeDuplicateEditor);
        layoutWidget->setObjectName("layoutWidget");
        layoutWidget->setGeometry(QRect(10, 10, 290, 116));
        formLayout = new QFormLayout(layoutWidget);
        formLayout->setObjectName("formLayout");
        formLayout->setContentsMargins(0, 0, 0, 0);
        label_2 = new QLabel(layoutWidget);
        label_2->setObjectName("label_2");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, label_2);

        lineNodeName = new QLineEdit(layoutWidget);
        lineNodeName->setObjectName("lineNodeName");

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

        btnDuplicate = new QPushButton(layoutWidget);
        btnDuplicate->setObjectName("btnDuplicate");

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, btnDuplicate);

        btnCancel = new QPushButton(layoutWidget);
        btnCancel->setObjectName("btnCancel");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, btnCancel);


        retranslateUi(DBCNodeDuplicateEditor);

        QMetaObject::connectSlotsByName(DBCNodeDuplicateEditor);
    } // setupUi

    void retranslateUi(QDialog *DBCNodeDuplicateEditor)
    {
        DBCNodeDuplicateEditor->setWindowTitle(QCoreApplication::translate("DBCNodeDuplicateEditor", "Duplicate Node", nullptr));
        label_2->setText(QCoreApplication::translate("DBCNodeDuplicateEditor", "New Node Name:", nullptr));
        label_8->setText(QCoreApplication::translate("DBCNodeDuplicateEditor", "Current Base ID:", nullptr));
        label->setText(QCoreApplication::translate("DBCNodeDuplicateEditor", "New Base ID:", nullptr));
        btnDuplicate->setText(QCoreApplication::translate("DBCNodeDuplicateEditor", "Duplicate Node", nullptr));
        btnCancel->setText(QCoreApplication::translate("DBCNodeDuplicateEditor", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DBCNodeDuplicateEditor: public Ui_DBCNodeDuplicateEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DBCNODEDUPLICATEEDITOR_H
