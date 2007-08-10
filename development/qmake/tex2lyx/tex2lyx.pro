
include(../config.pri)

TEMPLATE = app

TARGET = ../bin/tex2lyx$${DEBUGSUFFIX}

QT =

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

INC += src
INC += boost

HPP += Context.h
HPP += Font.h
HPP += Parser.h
HPP += Spacing.h
HPP += pch.h
HPP += tex2lyx.h

CPP += Context.cpp
CPP += Font.cpp
CPP += Parser.cpp
CPP += boost.cpp
CPP += gettext.cpp
CPP += lengthcommon.cpp
CPP += math.cpp
CPP += preamble.cpp
CPP += table.cpp
CPP += tex2lyx.cpp
CPP += text.cpp

# linked files 
HPP += ../Layout.h
HPP += ../TextClass.h

CPP += ../FloatList.cpp
CPP += ../Floating.cpp
CPP += ../Counters.cpp
CPP += ../Layout.cpp
CPP += ../TextClass.cpp
CPP += ../Lexer.cpp 


for(FILE,CPP) { SOURCES += $${BUILD_BASE_SOURCE_DIR}/src/tex2lyx/$${FILE} }
for(FILE,HPP) { HEADERS += $${BUILD_BASE_SOURCE_DIR}/src/tex2lyx/$${FILE} }
for(PATH,INC) { INCLUDEPATH += $${BUILD_BASE_SOURCE_DIR}/$${PATH} }
