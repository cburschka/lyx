
include(../config.pri)

TEMPLATE = app

QT = core gui

DESTDIR = ../bin

TARGET = lyx$${DEBUGSUFFIX}

INCLUDEPATH += $${BUILD_BASE_SOURCE_DIR}/src
INCLUDEPATH += $${BUILD_BASE_SOURCE_DIR}/boost

SOURCES += $${BUILD_BASE_SOURCE_DIR}/src/main.cpp

CONFIG += link_prl
CONFIG += debug_and_release

LIBS += -lAiksaurus

LIBS += -L../lib

LIBS += -Wl,--start-group
LIBS += -llyxmathed$${DEBUGSUFFIX}
LIBS += -llyxinsets$${DEBUGSUFFIX}
LIBS += -llyxgraphics$${DEBUGSUFFIX}
LIBS += -llyxfrontends$${DEBUGSUFFIX}
LIBS += -llyxcontrollers$${DEBUGSUFFIX}
LIBS += -llyxqt4$${DEBUGSUFFIX}
LIBS += -llyxsrc$${DEBUGSUFFIX}
LIBS += -llyxsupport$${DEBUGSUFFIX}
LIBS += -Wl,--end-group

LIBS += -llyxboost$${DEBUGSUFFIX}
