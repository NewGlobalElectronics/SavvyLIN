/********************************************************************************
** Form generated from reading UI file 'dbcnodeeditor.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DBCNODEEDITOR_H
#define UI_DBCNODEEDITOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DBCNodeEditor
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *label_2;
    QLineEdit *lineMsgName;
    QLabel *label_8;
    QLineEdit *lineComment;

    void setupUi(QDialog *DBCNodeEditor)
    {
        if (DBCNodeEditor->objectName().isEmpty())
            DBCNodeEditor->setObjectName("DBCNodeEditor");
        DBCNodeEditor->resize(311, 88);
        verticalLayout = new QVBoxLayout(DBCNodeEditor);
        verticalLayout->setObjectName("verticalLayout");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        label_2 = new QLabel(DBCNodeEditor);
        label_2->setObjectName("label_2");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, label_2);

        lineMsgName = new QLineEdit(DBCNodeEditor);
        lineMsgName->setObjectName("lineMsgName");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, lineMsgName);

        label_8 = new QLabel(DBCNodeEditor);
        label_8->setObjectName("label_8");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, label_8);

        lineComment = new QLineEdit(DBCNodeEditor);
        lineComment->setObjectName("lineComment");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, lineComment);


        verticalLayout->addLayout(formLayout);


        retranslateUi(DBCNodeEditor);

        QMetaObject::connectSlotsByName(DBCNodeEditor);
    } // setupUi

    void retranslateUi(QDialog *DBCNodeEditor)
    {
        DBCNodeEditor->setWindowTitle(QCoreApplication::translate("DBCNodeEditor", "Node Editor", nullptr));
        label_2->setText(QCoreApplication::translate("DBCNodeEditor", "Node Name:", nullptr));
        label_8->setText(QCoreApplication::translate("DBCNodeEditor", "Comments:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DBCNodeEditor: public Ui_DBCNodeEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DBCNODEEDITOR_H
