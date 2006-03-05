include $(top_srcdir)/config/common.am
include $(srcdir)/../Makefile.dialogs

CLEANFILES += *.C *Base.h *Module.h *Dialog.h

SUBDIRS = .

EXTRA_DIST = pch.h $(UIFILES)

BUILT_SOURCES = $(PCH_FILE) $(UIFILES:.ui=.h)

AM_CPPFLAGS += \
	$(QT4_CPPFLAGS) -DQT_NO_TRANSLATION \
	$(PCH_FLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/src/frontends/qt4 \
	$(QT4_INCLUDES) $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers

noinst_LTLIBRARIES = libqt4ui.la

nodist_libqt4ui_la_SOURCES = $(UIFILES:.ui=.h)
# The trick.
libqt4ui_la_LINK = touch libqt4ui.la ; echo

# Use _() for localization instead of tr() or trUtf8()
UICFLAGS=-tr qt_

%.h: %.ui
	$(UIC4) $(UIC4FLAGS) $< -o $@
