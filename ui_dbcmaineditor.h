/********************************************************************************
** Form generated from reading UI file 'dbcmaineditor.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DBCMAINEDITOR_H
#define UI_DBCMAINEDITOR_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DBCMainEditor
{
public:
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_2;
    QToolButton *btnNewNode;
    QToolButton *btnNewMessage;
    QToolButton *btnNewSignal;
    QToolButton *btnDelete;
    QSpacerItem *horizontalSpacer;
    QVBoxLayout *verticalLayout_2;
    QTreeWidget *treeDBC;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label_3;
    QLineEdit *lineSearch;
    QPushButton *btnSearch;
    QHBoxLayout *horizontalLayout_3;
    QLabel *lblSearchPos;
    QPushButton *btnSearchNext;
    QPushButton *btnSearchPrev;

    void setupUi(QDialog *DBCMainEditor)
    {
        if (DBCMainEditor->objectName().isEmpty())
            DBCMainEditor->setObjectName("DBCMainEditor");
        DBCMainEditor->resize(1103, 571);
        verticalLayout_3 = new QVBoxLayout(DBCMainEditor);
        verticalLayout_3->setObjectName("verticalLayout_3");
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        btnNewNode = new QToolButton(DBCMainEditor);
        btnNewNode->setObjectName("btnNewNode");
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/images/node_new.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnNewNode->setIcon(icon);
        btnNewNode->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_2->addWidget(btnNewNode);

        btnNewMessage = new QToolButton(DBCMainEditor);
        btnNewMessage->setObjectName("btnNewMessage");
        btnNewMessage->setEnabled(false);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/images/message_new.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnNewMessage->setIcon(icon1);
        btnNewMessage->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_2->addWidget(btnNewMessage);

        btnNewSignal = new QToolButton(DBCMainEditor);
        btnNewSignal->setObjectName("btnNewSignal");
        btnNewSignal->setEnabled(false);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/images/signal_new.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnNewSignal->setIcon(icon2);
        btnNewSignal->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_2->addWidget(btnNewSignal);

        btnDelete = new QToolButton(DBCMainEditor);
        btnDelete->setObjectName("btnDelete");
        btnDelete->setEnabled(false);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/images/skull.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnDelete->setIcon(icon3);
        btnDelete->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        horizontalLayout_2->addWidget(btnDelete);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);


        verticalLayout_3->addLayout(horizontalLayout_2);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName("verticalLayout_2");
        treeDBC = new QTreeWidget(DBCMainEditor);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        treeDBC->setHeaderItem(__qtreewidgetitem);
        treeDBC->setObjectName("treeDBC");
        treeDBC->setDragEnabled(false);
        treeDBC->setDragDropMode(QAbstractItemView::NoDragDrop);
        treeDBC->setDefaultDropAction(Qt::IgnoreAction);
        treeDBC->setSelectionMode(QAbstractItemView::SingleSelection);
        treeDBC->setIndentation(28);
        treeDBC->setHeaderHidden(false);
        treeDBC->setExpandsOnDoubleClick(false);
        treeDBC->setColumnCount(1);
        treeDBC->header()->setVisible(true);
        treeDBC->header()->setStretchLastSection(true);

        verticalLayout_2->addWidget(treeDBC);


        verticalLayout_3->addLayout(verticalLayout_2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        label_3 = new QLabel(DBCMainEditor);
        label_3->setObjectName("label_3");

        horizontalLayout->addWidget(label_3);

        lineSearch = new QLineEdit(DBCMainEditor);
        lineSearch->setObjectName("lineSearch");

        horizontalLayout->addWidget(lineSearch);

        btnSearch = new QPushButton(DBCMainEditor);
        btnSearch->setObjectName("btnSearch");

        horizontalLayout->addWidget(btnSearch);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        lblSearchPos = new QLabel(DBCMainEditor);
        lblSearchPos->setObjectName("lblSearchPos");

        horizontalLayout_3->addWidget(lblSearchPos);

        btnSearchNext = new QPushButton(DBCMainEditor);
        btnSearchNext->setObjectName("btnSearchNext");

        horizontalLayout_3->addWidget(btnSearchNext);

        btnSearchPrev = new QPushButton(DBCMainEditor);
        btnSearchPrev->setObjectName("btnSearchPrev");

        horizontalLayout_3->addWidget(btnSearchPrev);


        verticalLayout->addLayout(horizontalLayout_3);


        verticalLayout_3->addLayout(verticalLayout);

        verticalLayout_3->setStretch(1, 5);
        verticalLayout_3->setStretch(2, 1);

        retranslateUi(DBCMainEditor);

        QMetaObject::connectSlotsByName(DBCMainEditor);
    } // setupUi

    void retranslateUi(QDialog *DBCMainEditor)
    {
        DBCMainEditor->setWindowTitle(QCoreApplication::translate("DBCMainEditor", "DBC Editing Window", nullptr));
        btnNewNode->setText(QCoreApplication::translate("DBCMainEditor", "New Node", nullptr));
        btnNewMessage->setText(QCoreApplication::translate("DBCMainEditor", "New Message", nullptr));
        btnNewSignal->setText(QCoreApplication::translate("DBCMainEditor", "New Signal", nullptr));
        btnDelete->setText(QCoreApplication::translate("DBCMainEditor", "Delete", nullptr));
        label_3->setText(QCoreApplication::translate("DBCMainEditor", "Find:", nullptr));
        btnSearch->setText(QCoreApplication::translate("DBCMainEditor", "Search", nullptr));
        lblSearchPos->setText(QCoreApplication::translate("DBCMainEditor", "Search Results: 0 of 0", nullptr));
        btnSearchNext->setText(QCoreApplication::translate("DBCMainEditor", "Next", nullptr));
        btnSearchPrev->setText(QCoreApplication::translate("DBCMainEditor", "Previous", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DBCMainEditor: public Ui_DBCMainEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DBCMAINEDITOR_H
