AUTOMAKE_OPTIONS = foreign 1.4
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libqt2uimoc.o
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${top_srcdir}/src/ \
	-I${top_srcdir}/src/frontends/ \
	-I${top_srcdir}/src/frontends/controllers \
	${SIGC_CFLAGS} \
	${FRONTEND_INCLUDES} \
	$(BOOST_INCLUDES)
AM_CXXFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR

include ../../Makefile.dialogs

libqt2uimoc_o_SOURCES := $(UIMOCDIALOGS)
 
DISTCLEANFILES = $(libqt2uimoc_la_SOURCES) *.orig *.rej *~ *.bak core

ETAGS_ARGS = --lang=c++

%_moc.C: ../%.h
	$(MOC) -o $@ $<

