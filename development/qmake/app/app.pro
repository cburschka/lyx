
include(../config.pri)

TEMPLATE = app

DESTDIR = ../bin

TARGET = lyx

SOURCES += $${BUILD_BASE_SOURCE_DIR}/src/main.cpp

CONFIG += link_prl
CONFIG += debug_and_release

LIBS += -L../lib -lsrc
LIBS += -lsrc
LIBS += -lmathed
LIBS += -lfrontends
LIBS += -lgraphics
LIBS += -linsets
LIBS += -lcontrollers
LIBS += -lqt4
LIBS += -lsupport
LIBS += -lboost
