/********************************************************************************
** Form generated from reading UI file 'dbcmessageeditor.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DBCMESSAGEEDITOR_H
#define UI_DBCMESSAGEEDITOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DBCMessageEditor
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *lineFrameID;
    QLineEdit *lineMsgName;
    QLabel *label_2;
    QLineEdit *lineFrameLen;
    QLabel *label_3;
    QLabel *label_5;
    QLabel *label_7;
    QLabel *label_8;
    QLineEdit *lineComment;
    QLabel *label_9;
    QComboBox *comboSender;
    QPushButton *btnTextColor;
    QPushButton *btnBackgroundColor;
    QLabel *label_4;
    QListWidget *listSample;

    void setupUi(QDialog *DBCMessageEditor)
    {
        if (DBCMessageEditor->objectName().isEmpty())
            DBCMessageEditor->setObjectName("DBCMessageEditor");
        DBCMessageEditor->resize(311, 371);
        verticalLayout = new QVBoxLayout(DBCMessageEditor);
        verticalLayout->setObjectName("verticalLayout");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        label = new QLabel(DBCMessageEditor);
        label->setObjectName("label");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, label);

        lineFrameID = new QLineEdit(DBCMessageEditor);
        lineFrameID->setObjectName("lineFrameID");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, lineFrameID);

        lineMsgName = new QLineEdit(DBCMessageEditor);
        lineMsgName->setObjectName("lineMsgName");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, lineMsgName);

        label_2 = new QLabel(DBCMessageEditor);
        label_2->setObjectName("label_2");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, label_2);

        lineFrameLen = new QLineEdit(DBCMessageEditor);
        lineFrameLen->setObjectName("lineFrameLen");

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, lineFrameLen);

        label_3 = new QLabel(DBCMessageEditor);
        label_3->setObjectName("label_3");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, label_3);

        label_5 = new QLabel(DBCMessageEditor);
        label_5->setObjectName("label_5");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, label_5);

        label_7 = new QLabel(DBCMessageEditor);
        label_7->setObjectName("label_7");

        formLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, label_7);

        label_8 = new QLabel(DBCMessageEditor);
        label_8->setObjectName("label_8");

        formLayout->setWidget(6, QFormLayout::ItemRole::LabelRole, label_8);

        lineComment = new QLineEdit(DBCMessageEditor);
        lineComment->setObjectName("lineComment");

        formLayout->setWidget(6, QFormLayout::ItemRole::FieldRole, lineComment);

        label_9 = new QLabel(DBCMessageEditor);
        label_9->setObjectName("label_9");

        formLayout->setWidget(5, QFormLayout::ItemRole::LabelRole, label_9);

        comboSender = new QComboBox(DBCMessageEditor);
        comboSender->setObjectName("comboSender");
        comboSender->setEditable(true);

        formLayout->setWidget(5, QFormLayout::ItemRole::FieldRole, comboSender);

        btnTextColor = new QPushButton(DBCMessageEditor);
        btnTextColor->setObjectName("btnTextColor");

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, btnTextColor);

        btnBackgroundColor = new QPushButton(DBCMessageEditor);
        btnBackgroundColor->setObjectName("btnBackgroundColor");

        formLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, btnBackgroundColor);


        verticalLayout->addLayout(formLayout);

        label_4 = new QLabel(DBCMessageEditor);
        label_4->setObjectName("label_4");
        label_4->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_4);

        listSample = new QListWidget(DBCMessageEditor);
        listSample->setObjectName("listSample");

        verticalLayout->addWidget(listSample);


        retranslateUi(DBCMessageEditor);

        QMetaObject::connectSlotsByName(DBCMessageEditor);
    } // setupUi

    void retranslateUi(QDialog *DBCMessageEditor)
    {
        DBCMessageEditor->setWindowTitle(QCoreApplication::translate("DBCMessageEditor", "Message Editor", nullptr));
        label->setText(QCoreApplication::translate("DBCMessageEditor", "Frame ID:", nullptr));
        label_2->setText(QCoreApplication::translate("DBCMessageEditor", "Message Name:", nullptr));
        label_3->setText(QCoreApplication::translate("DBCMessageEditor", "Frame Length:", nullptr));
        label_5->setText(QCoreApplication::translate("DBCMessageEditor", "Text Color:", nullptr));
        label_7->setText(QCoreApplication::translate("DBCMessageEditor", "Background Color:", nullptr));
        label_8->setText(QCoreApplication::translate("DBCMessageEditor", "Comments:", nullptr));
        label_9->setText(QCoreApplication::translate("DBCMessageEditor", "Sending Node:", nullptr));
        btnTextColor->setText(QCoreApplication::translate("DBCMessageEditor", "Set", nullptr));
        btnBackgroundColor->setText(QCoreApplication::translate("DBCMessageEditor", "Set", nullptr));
        label_4->setText(QCoreApplication::translate("DBCMessageEditor", "Sample Colored Output:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DBCMessageEditor: public Ui_DBCMessageEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DBCMESSAGEEDITOR_H
