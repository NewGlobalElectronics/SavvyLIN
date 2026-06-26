#-------------------------------------------------
#
# Project created by QtCreator 2015-04-25T22:57:44
#
#-------------------------------------------------

!versionAtLeast(QT_VERSION, 5.14.0) {
    error("Current version of Qt ($${QT_VERSION}) is too old, this project requires Qt 5.14 or newer")
}

QT = core gui printsupport qml serialbus serialport widgets help network opengl

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

CONFIG += c++17
CONFIG += NO_UNIT_TESTS

DEFINES += QCUSTOMPLOT_USE_OPENGL

TARGET = SavvyLIN
TEMPLATE = app

QMAKE_INFO_PLIST = Info.plist.template
ICON = icons/SavvyIcon.icns

SOURCES += main.cpp\
    canbridgewindow.cpp \
    connections/canlogserver.cpp \
    connections/canserver.cpp \
    connections/lawicel_serial.cpp \
    connections/mqtt_bus.cpp \
    connections/ngeconnection.cpp \
    db/dbnodeduplicateeditor.cpp \
    framesenderobject.cpp \
    mqtt/qmqtt_client.cpp \
    mqtt/qmqtt_client_p.cpp \
    mqtt/qmqtt_frame.cpp \
    mqtt/qmqtt_message.cpp \
    mqtt/qmqtt_network.cpp \
    mqtt/qmqtt_router.cpp \
    mqtt/qmqtt_routesubscription.cpp \
    mqtt/qmqtt_socket.cpp \
    mqtt/qmqtt_ssl_socket.cpp \
    mqtt/qmqtt_timer.cpp \
    mqtt/qmqtt_websocket.cpp \
    mqtt/qmqtt_websocketiodevice.cpp \
    qcpaxistickerhex.cpp \
    #re/dbcomparatorwindow.cpp \
    mainwindow.cpp \
    canframemodel.cpp \
    simplecrypt.cpp \
    triggerdialog.cpp \
    utility.cpp \
    qcustomplot.cpp \
    frameplaybackwindow.cpp \
    candatagrid.cpp \
    framesenderwindow.cpp \
    framefileio.cpp \
    mainsettingsdialog.cpp \
    firmwareuploaderwindow.cpp \
    scriptingwindow.cpp \
    scriptcontainer.cpp \
    canfilter.cpp \
    can_structs.cpp \
    motorcontrollerconfigwindow.cpp \
    connections/canconnection.cpp \
    connections/serialbusconnection.cpp \
    connections/canconfactory.cpp \
    connections/gvretserial.cpp \
    connections/socketcand.cpp \
    connections/canconmanager.cpp \
    re/sniffer/snifferitem.cpp \
    re/sniffer/sniffermodel.cpp \
    re/sniffer/snifferwindow.cpp \
    db/dbmessageeditor.cpp \
    db/db_classes.cpp \
    db/dbhandler.cpp \
    db/dbloadsavewindow.cpp \
    db/dbmaineditor.cpp \
    db/dbnodeeditor.cpp \
    db/dbsignaleditor.cpp \
    db/dbnoderebaseeditor.cpp \
    re/discretestatewindow.cpp \
    re/filecomparatorwindow.cpp \
    re/flowviewwindow.cpp \
    re/frameinfowindow.cpp \
    re/fuzzingwindow.cpp \
    re/isotp_interpreterwindow.cpp \
    re/rangestatewindow.cpp \
    re/udsscanwindow.cpp \
    connections/canbus.cpp \
    connections/canconnectionmodel.cpp \
    connections/connectionwindow.cpp \
    re/graphingwindow.cpp \
    re/newgraphdialog.cpp \
    bisectwindow.cpp \
    signalviewerwindow.cpp \
    bus_protocols/isotp_handler.cpp \
    bus_protocols/j1939_handler.cpp \
    bus_protocols/uds_handler.cpp \
    jsedit.cpp \
    frameplaybackobject.cpp \
    helpwindow.cpp \
    blfhandler.cpp \
    re/sniffer/SnifferDelegate.cpp \
    connections/newconnectiondialog.cpp \
    re/temporalgraphwindow.cpp \
    filterutility.cpp \
    pcaplite.cpp \
    ldf/LDFDatabaseManager.cpp \
    ldf/LDFUtility.cpp \
    ldf/LDFCommonTableWidget.cpp \
    ldf/ldfeditor.cpp \
    ldf/LDFElementView.cpp \
    ldf/LDFNetworkView.cpp \
    ldf/LDFPropertyView.cpp \
    ldf/LDFTableWidget.cpp \
    ldf/ldfviewer.cpp \
    ldf/LDFHighlighter.cpp \
    ldf/AboutLDFEditor.cpp \
    ldf/ldfadditionalview.cpp \
    ldf/NodeConfigurationDlg.cpp \
    ldf/CodingDlg.cpp \
    ldf/ScheduleTableDlg.cpp \
    ldf/SignalGroupDlg.cpp \
    ldf/SignalDlg.cpp \
    ldf/LDFCLusterPropsDlg.cpp \
    ldf/MasterEditDlg.cpp \
    ldf/SlaveDlg.cpp \
    ldf/EventFrameEditDlg.cpp \
    ldf/FaultSignalsDlg.cpp \
    ldf/MapSignalsDlg.cpp \
    ldf/ScheduleComboWidget.cpp \
    ldf/LineEditWidget.cpp \
    ldf/UnconditionalFrameEditDlg.cpp \
    ldf/SporadicFrameEditDlg.cpp
HEADERS  += mainwindow.h \
    can_structs.h \
    canbridgewindow.h \
    canframemodel.h \
    connections/canlogserver.h \
    connections/canserver.h \
    connections/lawicel_serial.h \
    connections/ngeconnection.h \
    connections/socketcand.h \
    connections/mqtt_bus.h \
    db/dbnodeduplicateeditor.h \
    db/dbnoderebaseeditor.h \
    framesenderobject.h \
    mqtt/qmqtt.h \
    mqtt/qmqtt_client.h \
    mqtt/qmqtt_client_p.h \
    mqtt/qmqtt_frame.h \
    mqtt/qmqtt_global.h \
    mqtt/qmqtt_message.h \
    mqtt/qmqtt_message_p.h \
    mqtt/qmqtt_network_p.h \
    mqtt/qmqtt_networkinterface.h \
    mqtt/qmqtt_routedmessage.h \
    mqtt/qmqtt_router.h \
    mqtt/qmqtt_routesubscription.h \
    mqtt/qmqtt_socket_p.h \
    mqtt/qmqtt_socketinterface.h \
    mqtt/qmqtt_ssl_socket_p.h \
    mqtt/qmqtt_timer_p.h \
    mqtt/qmqtt_timerinterface.h \
    mqtt/qmqtt_websocket_p.h \
    mqtt/qmqtt_websocketiodevice_p.h \
    qcpaxistickerhex.h \
   #re/dbcomparatorwindow.h \
    simplecrypt.h \
    triggerdialog.h \
    utility.h \
    qcustomplot.h \
    frameplaybackwindow.h \
    candatagrid.h \
    framesenderwindow.h \
    can_trigger_structs.h \
    framefileio.h \
    config.h \
    mainsettingsdialog.h \
    firmwareuploaderwindow.h \
    scriptingwindow.h \
    scriptcontainer.h \
    canfilter.h \
    utils/lfqueue.h \
    motorcontrollerconfigwindow.h \
    connections/canconnection.h \
    connections/serialbusconnection.h \
    connections/canconconst.h \
    connections/canconfactory.h \
    connections/gvretserial.h \
    connections/canconmanager.h \
    re/sniffer/snifferitem.h \
    re/sniffer/sniffermodel.h \
    re/sniffer/snifferwindow.h \
    db/db_classes.h \
    db/dbhandler.h \
    db/dbloadsavewindow.h \
    db/dbmaineditor.h \
    db/dbsignaleditor.h \
    db/dbmessageeditor.h \
    db/dbnodeeditor.h \
    re/discretestatewindow.h \
    re/filecomparatorwindow.h \
    re/flowviewwindow.h \
    re/frameinfowindow.h \
    re/fuzzingwindow.h \
    re/isotp_interpreterwindow.h \
    re/rangestatewindow.h \
    re/udsscanwindow.h \
    connections/canbus.h \
    connections/canconnectionmodel.h \
    connections/connectionwindow.h \
    re/graphingwindow.h \
    re/newgraphdialog.h \
    bisectwindow.h \
    signalviewerwindow.h \
    bus_protocols/isotp_handler.h \
    bus_protocols/j1939_handler.h \
    bus_protocols/uds_handler.h \
    bus_protocols/isotp_message.h \
    jsedit.h \
    frameplaybackobject.h \
    helpwindow.h \
    blfhandler.h \
    re/sniffer/SnifferDelegate.h \
    connections/newconnectiondialog.h \
    re/temporalgraphwindow.h \
    filterutility.h \
    pcaplite.h \
    ldf/LDFDatabaseManager.h \
    ldf/LDFDefines.h \
    ldf/LDFUtility.h \
    ldf/LDFCommonTableWidget.h \
    ldf/ldfeditor.h \
    ldf/LDFElementView.h \
    ldf/LDFNetworkView.h \
    ldf/LDFPropertyView.h \
    ldf/LDFTableWidget.h \
    ldf/ldfviewer.h \
    ldf/LDFHighlighter.h \
    ldf/AboutLDFEditor.h \
    ldf/ldfadditionalview.h \
    ldf/NodeConfigurationDlg.h \
    ldf/CodingDlg.h \
    ldf/ScheduleTableDlg.h \
    ldf/SignalGroupDlg.h \
    ldf/SignalDlg.h \
    ldf/LDFCLusterPropsDlg.h \
    ldf/MasterEditDlg.h \
    ldf/SlaveDlg.h \
    ldf/EventFrameEditDlg.h \
    ldf/FaultSignalsDlg.h \
    ldf/MapSignalsDlg.h \
    ldf/ScheduleComboWidget.h \
    ldf/LineEditWidget.h \
    ldf/UnconditionalFrameEditDlg.h \
    ldf/SporadicFrameEditDlg.h
    #ldf/LDFAdapter.h
FORMS    += ui/candatagrid.ui \
    triggerdialog.ui \
    ui/canbridgewindow.ui \
    ui/dbnodeduplicateeditor.ui \
    #ui/dbcomparatorwindow.ui \
    ui/dbmessageeditor.ui \
    ui/connectionwindow.ui \
    ui/dbloadsavewindow.ui \
    ui/dbmaineditor.ui \
    ui/dbnoderebaseeditor.ui \
    ui/dbsignaleditor.ui \
    ui/dbnodeeditor.ui \
    ui/discretestatewindow.ui \
    ui/filecomparatorwindow.ui \
    ui/firmwareuploaderwindow.ui \
    ui/flowviewwindow.ui \
    ui/frameinfowindow.ui \
    ui/frameplaybackwindow.ui \
    ui/framesenderwindow.ui \
    ui/fuzzingwindow.ui \
    ui/graphingwindow.ui \
    ui/isotp_interpreterwindow.ui \
    ui/mainsettingsdialog.ui \
    ui/mainwindow.ui \
    ui/motorcontrollerconfigwindow.ui \
    ui/newgraphdialog.ui \
    ui/rangestatewindow.ui \
    ui/scriptingwindow.ui \
    ui/snifferwindow.ui \
    ui/udsscanwindow.ui \
    ui/bisectwindow.ui \
    ui/signalviewerwindow.ui \
    ui/helpwindow.ui \
    ui/newconnectiondialog.ui \
    ui/temporalgraphwindow.ui \
    ldf/LDFEditor.ui \
    ldf/ldfviewer.ui \
    ldf/AboutLDFEditor.ui \
    ldf/CodingDlg.ui \
    ldf/EventFrameEditDlg.ui \
    ldf/FaultSignalsDlg.ui \
    ldf/LDFCLusterPropsDlg.ui \
    ldf/MapSignalsDlg.ui \
    ldf/MasterEditDlg.ui \
    ldf/NodeConfiguration.ui \
    ldf/ScheduleTableDlg.ui \
    ldf/SignalDlg.ui \
    ldf/SignalGroupDlg.ui \
    ldf/SlaveDlg.ui \
    ldf/SlaveDlg_layout.ui \
    ldf/SporadicFrameEditDlg.ui \
    ldf/UnconditionalFrameEditDlg.ui
RESOURCES += \
    icons.qrc \
    images.qrc \
    ldf/ldfeditor.qrc
win32-msvc* {
   LIBS += opengl32.lib
}

win32-g++ {
    QMAKE_CXXFLAGS_DEBUG += -Wa,-mbig-obj
    LIBS += libopengl32
}
INCLUDEPATH += \
    $$PWD/ldf \
    $$PWD/ldf/ProtocolDefinitions
unix {
   isEmpty(PREFIX) {
      PREFIX=/usr/local
   }
   target.path = $$PREFIX/bin
   shortcutfiles.files=SavvyLIN.desktop
   shortcutfiles.path = $$PREFIX/share/applications
   INSTALLS += shortcutfiles
   DISTFILES += SavvyLIN.desktop
}

windows {
RC_ICONS=icons/SavvyIcon.ico
}

examplefiles.files=examples
examplefiles.path = $$PREFIX/share/savvycan/examples
INSTALLS += examplefiles

iconfiles.files=icons
iconfiles.path = $$PREFIX/share
INSTALLS += iconfiles

helpfiles.files=help/*
helpfiles.path = $$PREFIX/bin/help
INSTALLS += helpfiles

INSTALLS += target
# Compilation 32 bits
win32 {
    #QMAKE_CXXFLAGS += -m32
    #QMAKE_CFLAGS   += -m32
    #QMAKE_LFLAGS   += -m32
    DBManagerDll.files = $$PWD/DBManager.dll
    DBManagerDll.path  = $$OUT_PWD/release
    INSTALLS += DBManagerDll
}
