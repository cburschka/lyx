include $(top_srcdir)/config/common.am
include $(srcdir)/../Makefile.dialogs

CLEANFILES += *.C *Base.h

SUBDIRS = . moc

EXTRA_DIST = QPreambleDialogBase.ui $(UIFILES)

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/src/frontends/qt2 \
	$(QT_INCLUDES) $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers

AM_CPPFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR -DQT_NO_TRANSLATION
AM_CXXFLAGS = $(PCH_FLAGS)

noinst_LTLIBRARIES = libqt2ui.la

nodist_libqt2ui_la_SOURCES = $(UIFILES:.ui=.h) $(UIFILES:.ui=.C)
# The trick.
libqt2ui_la_LINK = touch libqt2ui.la

# Use _() for localization instead of tr() or trUtf8()
UICFLAGS=-tr qt_

%.h: %.ui
	$(UIC) $(UICFLAGS) $< -o $@
%.C: %.h %.ui $(PCH_FILE)
	$(UIC) $(UICFLAGS) -impl `echo $^ | sed 's/pch.h.gch//'` -o $@
