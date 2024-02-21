include (../../../../caQtDM_Viewer/qtdefs.pri)

QT += core gui network

contains(QT_VER_MAJ, 4) {
      CONFIG += designer
}
contains(QT_VER_MAJ, 5) {
      QT += uitools
}
contains(QT_VER_MAJ, 6) {
      QT += uitools
}

CONFIG += archive_plugin

include (../../../../caQtDM.pri)

MOC_DIR = ./moc
VPATH += ./src

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += .
INCLUDEPATH    += ../
INCLUDEPATH    += ../../
INCLUDEPATH    += ../../../src
INCLUDEPATH    += ../../../../caQtDM_QtControls/src/
INCLUDEPATH    += $(QWTINCLUDE)

android {
   INCLUDEPATH += $(ANDROIDFUNCTIONSINCLUDE)
}

HEADERS         = ../../controlsinterface.h archiveHTTP_plugin.h httpRetrieval.h ../archiverCommon.h \
    urlhandlerhttp.h \
    workerHttp.h \
    workerHttpThread.h
SOURCES         =  archiveHTTP_plugin.cpp httpRetrieval.cpp ../archiverCommon.cpp \
    urlhandlerhttp.cpp
TARGET          = archiveHTTP_plugin

DISTFILES +=


