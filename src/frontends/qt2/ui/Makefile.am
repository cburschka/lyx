AUTOMAKE_OPTIONS = foreign 1.4
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libqt2ui.la
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${top_srcdir}/src/ -I${top_srcdir}/src/frontends/ -I$(top_srcdir)/src/frontends/qt2/ \
	${SIGC_CFLAGS} ${FRONTEND_INCLUDES} $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers
AM_CXXFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR
ETAGS_ARGS = --lang=c++
SUBDIRS=. moc

include ../Makefile.dialogs

DISTCLEANFILES = $(UIDIALOGS) *.orig *.rej *~ *.bak core

libqt2ui_la_SOURCES = $(UIDIALOGS)

libqt2ui.la: $(libqt2ui_la_OBJECTS) $(libqt2ui_la_DEPENDENCIES)

# a pattern to change tr() to _()

SED=sed
SEDREPLACETR=s,[^A-Za-z]tr[ ]*(,_(,g
SEDREMOVEEMPTY=s,_[ ]*([ ]*""[ ]*),"",g
 
QAskForTextDialog.h: QAskForTextDialog.ui
	$(UIC) $< -o $@
QAskForTextDialog.C: QAskForTextDialog.h QAskForTextDialog.ui
	$(UIC) -impl $^ | $(SED) '$(SEDREPLACETR);$(SEDREMOVEEMPTY)' > $@
 
%Base.h: %.ui
	$(UIC) $< -o $@
%Base.C: %Base.h %.ui
	$(UIC) -impl $^ | $(SED) '$(SEDREPLACETR);$(SEDREMOVEEMPTY)' > $@
