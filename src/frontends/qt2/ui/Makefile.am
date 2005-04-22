include $(top_srcdir)/config/common.am
include $(srcdir)/../Makefile.dialogs

EXTRA_DIST = $(UIFILES)

DISTCLEANFILES += *.C *.h

SUBDIRS = . moc

noinst_LTLIBRARIES = libqt2ui.la

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/src/frontends/qt2 \
	$(FRONTEND_INCLUDES) $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers

AM_CXXFLAGS = $(QT_CPPFLAGS) -DQT_NO_TRANSLATION

nodist_libqt2ui_la_SOURCES = $(UIFILES:.ui=.h) $(UIFILES:.ui=.C)

libqt2ui.la: $(libqt2ui_la_OBJECTS) $(libqt2ui_la_DEPENDENCIES)

# Use _() for localization instead of tr() or trUtf8()
UICFLAGS=-tr qt_

%.h: %.ui
	$(UIC) $(UICFLAGS) $< -o $@
%.C: %.h %.ui
	$(UIC) $(UICFLAGS) -impl $^ -o $@
