/********************************************************************************
** Form generated from reading UI file 'ldfviewer.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LDFVIEWER_H
#define UI_LDFVIEWER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LDFViewerClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *LDFViewerClass)
    {
        if (LDFViewerClass->objectName().isEmpty())
            LDFViewerClass->setObjectName("LDFViewerClass");
        LDFViewerClass->resize(600, 400);
        menuBar = new QMenuBar(LDFViewerClass);
        menuBar->setObjectName("menuBar");
        LDFViewerClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(LDFViewerClass);
        mainToolBar->setObjectName("mainToolBar");
        LDFViewerClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(LDFViewerClass);
        centralWidget->setObjectName("centralWidget");
        LDFViewerClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(LDFViewerClass);
        statusBar->setObjectName("statusBar");
        LDFViewerClass->setStatusBar(statusBar);

        retranslateUi(LDFViewerClass);

        QMetaObject::connectSlotsByName(LDFViewerClass);
    } // setupUi

    void retranslateUi(QMainWindow *LDFViewerClass)
    {
        LDFViewerClass->setWindowTitle(QCoreApplication::translate("LDFViewerClass", "LDFViewer", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LDFViewerClass: public Ui_LDFViewerClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LDFVIEWER_H
