
include(../config.pri)

TEMPLATE = app

DESTDIR = ../bin

TARGET = lyx$${DEBUGSUFFIX}

SOURCES += $${BUILD_BASE_SOURCE_DIR}/src/main.cpp

CONFIG += link_prl
CONFIG += debug_and_release

LIBS += -L../lib
LIBS += -llyxmathed$${DEBUGSUFFIX}
LIBS += -llyxinsets$${DEBUGSUFFIX}
LIBS += -llyxgraphics$${DEBUGSUFFIX}
LIBS += -llyxsupport$${DEBUGSUFFIX}
LIBS += -llyxfrontends$${DEBUGSUFFIX}
LIBS += -llyxcontrollers$${DEBUGSUFFIX}
LIBS += -llyxqt4$${DEBUGSUFFIX}
LIBS += -llyxboost$${DEBUGSUFFIX}
LIBS += -llyxsrc$${DEBUGSUFFIX}
