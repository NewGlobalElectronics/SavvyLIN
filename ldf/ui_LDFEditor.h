/********************************************************************************
** Form generated from reading UI file 'LDFEditor.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LDFEDITOR_H
#define UI_LDFEDITOR_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "LDFElementView.h"
#include "LDFNetworkView.h"
#include "ldfadditionalview.h"
#include "ldfpropertyview.h"

QT_BEGIN_NAMESPACE

class Ui_LDFEditor
{
public:
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionSave_As;
    QAction *actionExit;
    QAction *actionNewLIN_1_3;
    QAction *actionNewLIN_2_0;
    QAction *actionNewLIN_2_1;
    QAction *actionHex;
    QAction *actionPreview_LDF_File;
    QAction *action_About;
    QAction *actionNew;
    QAction *actionRecent;
    QAction *action1;
    QAction *actionHelp;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_2;
    QSplitter *splitter_main;
    LDFElementView *m_pouLDFElementView;
    QSplitter *splitter;
    LDFPropertyView *m_pouLDFPropertyView;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QLabel *labelAdditionalViewHelp;
    LDFAdditionalView *m_pouLDFAdditionalView;
    LDFNetworkView *m_pouLDFNetworkView;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menu_New;
    QMenu *menu_Recent_Files;
    QMenu *menuHelp;
    QMenu *menuView;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QDockWidget *dockPaneWarning;
    QWidget *dockWidgetContents_3;

    void setupUi(QMainWindow *LDFEditor)
    {
        if (LDFEditor->objectName().isEmpty())
            LDFEditor->setObjectName("LDFEditor");
        LDFEditor->resize(851, 716);
        QFont font;
        font.setFamilies({QString::fromUtf8("Courier New")});
        font.setPointSize(11);
        LDFEditor->setFont(font);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/LDFEditorIcons/Resources/icons/32x32/Application.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        LDFEditor->setWindowIcon(icon);
        LDFEditor->setWindowOpacity(1.000000000000000);
        LDFEditor->setIconSize(QSize(32, 32));
        actionOpen = new QAction(LDFEditor);
        actionOpen->setObjectName("actionOpen");
        QFont font1;
        font1.setFamilies({QString::fromUtf8("Courier New")});
        font1.setPointSize(10);
        actionOpen->setFont(font1);
        actionOpen->setMenuRole(QAction::ApplicationSpecificRole);
        actionSave = new QAction(LDFEditor);
        actionSave->setObjectName("actionSave");
        actionSave->setFont(font1);
        actionSave_As = new QAction(LDFEditor);
        actionSave_As->setObjectName("actionSave_As");
        actionSave_As->setFont(font1);
        actionExit = new QAction(LDFEditor);
        actionExit->setObjectName("actionExit");
        actionExit->setFont(font1);
        actionNewLIN_1_3 = new QAction(LDFEditor);
        actionNewLIN_1_3->setObjectName("actionNewLIN_1_3");
        actionNewLIN_1_3->setFont(font1);
        actionNewLIN_2_0 = new QAction(LDFEditor);
        actionNewLIN_2_0->setObjectName("actionNewLIN_2_0");
        actionNewLIN_2_0->setFont(font1);
        actionNewLIN_2_1 = new QAction(LDFEditor);
        actionNewLIN_2_1->setObjectName("actionNewLIN_2_1");
        actionNewLIN_2_1->setFont(font1);
        actionHex = new QAction(LDFEditor);
        actionHex->setObjectName("actionHex");
        actionPreview_LDF_File = new QAction(LDFEditor);
        actionPreview_LDF_File->setObjectName("actionPreview_LDF_File");
        action_About = new QAction(LDFEditor);
        action_About->setObjectName("action_About");
        actionNew = new QAction(LDFEditor);
        actionNew->setObjectName("actionNew");
        actionNew->setCheckable(true);
        actionRecent = new QAction(LDFEditor);
        actionRecent->setObjectName("actionRecent");
        action1 = new QAction(LDFEditor);
        action1->setObjectName("action1");
        actionHelp = new QAction(LDFEditor);
        actionHelp->setObjectName("actionHelp");
        centralWidget = new QWidget(LDFEditor);
        centralWidget->setObjectName("centralWidget");
        verticalLayout_2 = new QVBoxLayout(centralWidget);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName("verticalLayout_2");
        splitter_main = new QSplitter(centralWidget);
        splitter_main->setObjectName("splitter_main");
        splitter_main->setOrientation(Qt::Horizontal);
        m_pouLDFElementView = new LDFElementView(splitter_main);
        m_pouLDFElementView->setObjectName("m_pouLDFElementView");
        m_pouLDFElementView->setFont(font1);
        splitter_main->addWidget(m_pouLDFElementView);
        splitter = new QSplitter(splitter_main);
        splitter->setObjectName("splitter");
        splitter->setOrientation(Qt::Vertical);
        m_pouLDFPropertyView = new LDFPropertyView(splitter);
        if (m_pouLDFPropertyView->columnCount() < 2)
            m_pouLDFPropertyView->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        __qtablewidgetitem->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter|Qt::AlignCenter);
        __qtablewidgetitem->setFont(font1);
        m_pouLDFPropertyView->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        __qtablewidgetitem1->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter|Qt::AlignCenter);
        __qtablewidgetitem1->setFont(font1);
        m_pouLDFPropertyView->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        m_pouLDFPropertyView->setObjectName("m_pouLDFPropertyView");
        m_pouLDFPropertyView->setEditTriggers(QAbstractItemView::EditKeyPressed);
        m_pouLDFPropertyView->setSelectionMode(QAbstractItemView::SingleSelection);
        m_pouLDFPropertyView->setSelectionBehavior(QAbstractItemView::SelectRows);
        splitter->addWidget(m_pouLDFPropertyView);
        layoutWidget = new QWidget(splitter);
        layoutWidget->setObjectName("layoutWidget");
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        labelAdditionalViewHelp = new QLabel(layoutWidget);
        labelAdditionalViewHelp->setObjectName("labelAdditionalViewHelp");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(labelAdditionalViewHelp->sizePolicy().hasHeightForWidth());
        labelAdditionalViewHelp->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(labelAdditionalViewHelp);

        m_pouLDFAdditionalView = new LDFAdditionalView(layoutWidget);
        m_pouLDFAdditionalView->setObjectName("m_pouLDFAdditionalView");
        m_pouLDFAdditionalView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_pouLDFAdditionalView->setSelectionMode(QAbstractItemView::SingleSelection);
        m_pouLDFAdditionalView->setSelectionBehavior(QAbstractItemView::SelectRows);

        verticalLayout->addWidget(m_pouLDFAdditionalView);

        splitter->addWidget(layoutWidget);
        splitter_main->addWidget(splitter);
        m_pouLDFNetworkView = new LDFNetworkView(splitter_main);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("LIN Network View"));
        m_pouLDFNetworkView->setHeaderItem(__qtreewidgetitem);
        m_pouLDFNetworkView->setObjectName("m_pouLDFNetworkView");
        m_pouLDFNetworkView->setFont(font1);
        splitter_main->addWidget(m_pouLDFNetworkView);

        verticalLayout_2->addWidget(splitter_main);

        LDFEditor->setCentralWidget(centralWidget);
        menubar = new QMenuBar(LDFEditor);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 851, 23));
        QFont font2;
        font2.setPointSize(10);
        menubar->setFont(font2);
        menuFile = new QMenu(menubar);
        menuFile->setObjectName("menuFile");
        menuFile->setFont(font1);
        menuFile->setContextMenuPolicy(Qt::DefaultContextMenu);
        menu_New = new QMenu(menuFile);
        menu_New->setObjectName("menu_New");
        menu_Recent_Files = new QMenu(menuFile);
        menu_Recent_Files->setObjectName("menu_Recent_Files");
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName("menuHelp");
        menuHelp->setFont(font2);
        menuView = new QMenu(menubar);
        menuView->setObjectName("menuView");
        menuView->setFont(font1);
        LDFEditor->setMenuBar(menubar);
        mainToolBar = new QToolBar(LDFEditor);
        mainToolBar->setObjectName("mainToolBar");
        mainToolBar->setFont(font2);
        LDFEditor->addToolBar(Qt::ToolBarArea::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(LDFEditor);
        statusBar->setObjectName("statusBar");
        LDFEditor->setStatusBar(statusBar);
        dockPaneWarning = new QDockWidget(LDFEditor);
        dockPaneWarning->setObjectName("dockPaneWarning");
        dockWidgetContents_3 = new QWidget();
        dockWidgetContents_3->setObjectName("dockWidgetContents_3");
        dockPaneWarning->setWidget(dockWidgetContents_3);
        LDFEditor->addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, dockPaneWarning);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuView->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menuFile->addAction(menu_New->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionSave);
        menuFile->addAction(actionSave_As);
        menuFile->addAction(menu_Recent_Files->menuAction());
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        menu_New->addAction(actionNewLIN_1_3);
        menu_New->addAction(actionNewLIN_2_0);
        menu_New->addAction(actionNewLIN_2_1);
        menuHelp->addAction(actionHelp);
        menuHelp->addAction(action_About);
        menuView->addSeparator();
        menuView->addAction(actionHex);
        menuView->addSeparator();
        menuView->addAction(actionPreview_LDF_File);
        mainToolBar->addAction(actionOpen);
        mainToolBar->addAction(actionSave);
        mainToolBar->addAction(actionSave_As);
        mainToolBar->addSeparator();

        retranslateUi(LDFEditor);

        QMetaObject::connectSlotsByName(LDFEditor);
    } // setupUi

    void retranslateUi(QMainWindow *LDFEditor)
    {
        LDFEditor->setWindowTitle(QCoreApplication::translate("LDFEditor", "BUSMASTER LDF Editor", nullptr));
        actionOpen->setText(QCoreApplication::translate("LDFEditor", "&Open...", nullptr));
        actionSave->setText(QCoreApplication::translate("LDFEditor", "&Save", nullptr));
        actionSave_As->setText(QCoreApplication::translate("LDFEditor", "Save &As...", nullptr));
        actionExit->setText(QCoreApplication::translate("LDFEditor", "E&xit", nullptr));
        actionNewLIN_1_3->setText(QCoreApplication::translate("LDFEditor", "LIN 1.3", nullptr));
        actionNewLIN_2_0->setText(QCoreApplication::translate("LDFEditor", "LIN 2.0", nullptr));
        actionNewLIN_2_1->setText(QCoreApplication::translate("LDFEditor", "LIN 2.1", nullptr));
        actionHex->setText(QCoreApplication::translate("LDFEditor", "&Hex", nullptr));
        actionPreview_LDF_File->setText(QCoreApplication::translate("LDFEditor", "&Preview LDF File", nullptr));
        action_About->setText(QCoreApplication::translate("LDFEditor", "&About", nullptr));
        actionNew->setText(QCoreApplication::translate("LDFEditor", "New", nullptr));
#if QT_CONFIG(tooltip)
        actionNew->setToolTip(QCoreApplication::translate("LDFEditor", "Create New LDF File", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionNew->setShortcut(QCoreApplication::translate("LDFEditor", "Ctrl+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actionRecent->setText(QCoreApplication::translate("LDFEditor", "Recent", nullptr));
        action1->setText(QCoreApplication::translate("LDFEditor", "1", nullptr));
        actionHelp->setText(QCoreApplication::translate("LDFEditor", "Help", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = m_pouLDFElementView->headerItem();
        ___qtreewidgetitem->setText(0, QCoreApplication::translate("LDFEditor", "LDF Element View", nullptr));
        QTableWidgetItem *___qtablewidgetitem = m_pouLDFPropertyView->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("LDFEditor", "Property", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = m_pouLDFPropertyView->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("LDFEditor", "Value", nullptr));
        labelAdditionalViewHelp->setText(QString());
        menuFile->setTitle(QCoreApplication::translate("LDFEditor", "&File", nullptr));
        menu_New->setTitle(QCoreApplication::translate("LDFEditor", "&New", nullptr));
        menu_Recent_Files->setTitle(QCoreApplication::translate("LDFEditor", "&Recent Files", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("LDFEditor", "&Help", nullptr));
        menuView->setTitle(QCoreApplication::translate("LDFEditor", "&View", nullptr));
        dockPaneWarning->setWindowTitle(QCoreApplication::translate("LDFEditor", "Warnings Window", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LDFEditor: public Ui_LDFEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LDFEDITOR_H
