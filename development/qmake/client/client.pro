
include(../config.pri)

TEMPLATE = app

TARGET = ../bin/client$${DEBUGSUFFIX}

QT =

INC += src
INC += boost

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

#EXTRA_DIST = pch.h lyxclient.man
#BUILT_SOURCES = $(PCH_FILE)
#man_MANS = lyxclient.1
#BOOST_LIBS = $(BOOST_REGEX) $(BOOST_FILESYSTEM)
#lyxclient_LDADD = \
#	$(top_builddir)/src/support/libsupport.la \
#	$(BOOST_LIBS) $(INTLLIBS) @LIBS@ $(SOCKET_LIBS)
#lyxclient.1:
#	cp -p $(srcdir)/lyxclient.man lyxclient.1


CPP += boost.cpp
CPP += client.cpp
CPP += debug.cpp
CPP += gettext.cpp
CPP += Messages.cpp

HPP += debug.h
HPP += Messages.h

for(FILE,CPP) { SOURCES += $${BUILD_BASE_SOURCE_DIR}/src/client/$${FILE} }
for(FILE,HPP) { HEADERS += $${BUILD_BASE_SOURCE_DIR}/src/client/$${FILE} }
for(PATH,INC) { INCLUDEPATH += $${BUILD_BASE_SOURCE_DIR}/$${PATH} }
