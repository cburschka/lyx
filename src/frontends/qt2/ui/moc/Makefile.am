AUTOMAKE_OPTIONS = foreign 1.4
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
DISTCLEANFILES = $(libqt2uimoc_la_SOURCES) *.orig *.rej *~ *.bak core
ETAGS_ARGS = --lang=c++

noinst_LTLIBRARIES = libqt2uimoc.la

BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${top_srcdir}/src/ \
	-I${top_srcdir}/src/frontends/ \
	-I${top_srcdir}/src/frontends/controllers \
	${SIGC_CFLAGS} \
	${FRONTEND_INCLUDES} \
	$(BOOST_INCLUDES)
AM_CXXFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR

include ../../Makefile.dialogs

libqt2uimoc_la_SOURCES := $(UIMOCDIALOGS)

libqt2uimoc.la: $(libqt2uimoc_la_OBJECTS) $(libqt2uimoc_la_DEPENDENCIES)

%_moc.C: ../%.h
	$(MOC) -o $@ $<

