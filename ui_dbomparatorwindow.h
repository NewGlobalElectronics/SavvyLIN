/********************************************************************************
** Form generated from reading UI file 'dbomparatorwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DBOMPARATORWINDOW_H
#define UI_DBOMPARATORWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_dbComparatorWindow
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_2;
    QLabel *lblFirstFile;
    QPushButton *btndbFile1;
    QFrame *line;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_4;
    QLabel *lblSecondFile;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *btndbFile2;
    QFrame *line_2;
    QTreeWidget *treeDetails;
    QPushButton *btnSaveDetails;

    void setupUi(QDialog *dbComparatorWindow)
    {
        if (dbComparatorWindow->objectName().isEmpty())
            dbComparatorWindow->setObjectName("dbComparatorWindow");
        dbComparatorWindow->resize(720, 631);
        verticalLayout = new QVBoxLayout(dbComparatorWindow);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName("verticalLayout_3");
        label_2 = new QLabel(dbComparatorWindow);
        label_2->setObjectName("label_2");
        QFont font;
        font.setPointSize(16);
        label_2->setFont(font);
        label_2->setAlignment(Qt::AlignCenter);

        verticalLayout_3->addWidget(label_2);

        lblFirstFile = new QLabel(dbComparatorWindow);
        lblFirstFile->setObjectName("lblFirstFile");

        verticalLayout_3->addWidget(lblFirstFile);

        btndbFile1 = new QPushButton(dbComparatorWindow);
        btndbFile1->setObjectName("btndbFile1");

        verticalLayout_3->addWidget(btndbFile1);


        horizontalLayout->addLayout(verticalLayout_3);

        line = new QFrame(dbComparatorWindow);
        line->setObjectName("line");
        line->setFrameShape(QFrame::Shape::VLine);
        line->setFrameShadow(QFrame::Shadow::Sunken);

        horizontalLayout->addWidget(line);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName("verticalLayout_2");
        label_4 = new QLabel(dbComparatorWindow);
        label_4->setObjectName("label_4");
        label_4->setFont(font);
        label_4->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(label_4);

        lblSecondFile = new QLabel(dbComparatorWindow);
        lblSecondFile->setObjectName("lblSecondFile");

        verticalLayout_2->addWidget(lblSecondFile);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        btndbFile2 = new QPushButton(dbComparatorWindow);
        btndbFile2->setObjectName("btndbFile2");

        horizontalLayout_2->addWidget(btndbFile2);


        verticalLayout_2->addLayout(horizontalLayout_2);


        horizontalLayout->addLayout(verticalLayout_2);

        horizontalLayout->setStretch(0, 8);
        horizontalLayout->setStretch(2, 8);

        verticalLayout->addLayout(horizontalLayout);

        line_2 = new QFrame(dbComparatorWindow);
        line_2->setObjectName("line_2");
        line_2->setFrameShape(QFrame::Shape::HLine);
        line_2->setFrameShadow(QFrame::Shadow::Sunken);

        verticalLayout->addWidget(line_2);

        treeDetails = new QTreeWidget(dbComparatorWindow);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        treeDetails->setHeaderItem(__qtreewidgetitem);
        treeDetails->setObjectName("treeDetails");

        verticalLayout->addWidget(treeDetails);

        btnSaveDetails = new QPushButton(dbComparatorWindow);
        btnSaveDetails->setObjectName("btnSaveDetails");

        verticalLayout->addWidget(btnSaveDetails);

        QWidget::setTabOrder(btndbFile1, btndbFile2);
        QWidget::setTabOrder(btndbFile2, treeDetails);
        QWidget::setTabOrder(treeDetails, btnSaveDetails);

        retranslateUi(dbComparatorWindow);

        QMetaObject::connectSlotsByName(dbComparatorWindow);
    } // setupUi

    void retranslateUi(QDialog *dbComparatorWindow)
    {
        dbComparatorWindow->setWindowTitle(QCoreApplication::translate("dbComparatorWindow", "db Comparator", nullptr));
        label_2->setText(QCoreApplication::translate("dbComparatorWindow", "Side 1", nullptr));
        lblFirstFile->setText(QCoreApplication::translate("dbComparatorWindow", "TextLabel", nullptr));
        btndbFile1->setText(QCoreApplication::translate("dbComparatorWindow", "Load New File", nullptr));
        label_4->setText(QCoreApplication::translate("dbComparatorWindow", "Side 2", nullptr));
        lblSecondFile->setText(QCoreApplication::translate("dbComparatorWindow", "TextLabel", nullptr));
        btndbFile2->setText(QCoreApplication::translate("dbComparatorWindow", "Load A File", nullptr));
        btnSaveDetails->setText(QCoreApplication::translate("dbComparatorWindow", "Save Details to File", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dbComparatorWindow: public Ui_dbComparatorWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DBOMPARATORWINDOW_H
