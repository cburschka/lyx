include $(top_srcdir)/config/common.am
include $(srcdir)/../Makefile.dialogs

DISTCLEANFILES += *.C *.h

SUBDIRS = . moc

EXTRA_DIST = $(UIFILES)

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/src/frontends/qt2 \
	$(QT_INCLUDES) $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers

AM_CXXFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR -DQT_NO_TRANSLATION

noinst_LTLIBRARIES = libqt2ui.la

nodist_libqt2ui_la_SOURCES = $(UIFILES:.ui=.h) $(UIFILES:.ui=.C)
# The trick.
libqt2ui_la_LINK = touch libqt2ui.la

# Use _() for localization instead of tr() or trUtf8()
UICFLAGS=-tr qt_

%.h: %.ui
	$(UIC) $(UICFLAGS) $< -o $@
%.C: %.h %.ui
	$(UIC) $(UICFLAGS) -impl $^ -o $@
