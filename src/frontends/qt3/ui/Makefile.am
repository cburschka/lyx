include $(top_srcdir)/config/common.am
include $(srcdir)/../Makefile.dialogs

CLEANFILES += *.C *Base.h *Module.h *Dialog.h

SUBDIRS = . moc

EXTRA_DIST = pch.h $(UIFILES)

BUILT_SOURCES = $(PCH_FILE)

AM_CPPFLAGS += \
	$(QT_CPPFLAGS) -DQT_NO_TRANSLATION \
	$(PCH_FLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/src/frontends/qt3 \
	$(QT_INCLUDES) $(BOOST_INCLUDES) \
	-I$(top_srcdir)/src/frontends/controllers

noinst_LTLIBRARIES = libqt3ui.la

nodist_libqt3ui_la_SOURCES = $(UIFILES:.ui=.h) $(UIFILES:.ui=.C)
# The trick.
libqt3ui_la_LINK = touch libqt3ui.la ; echo

# Use _() for localization instead of tr() or trUtf8()
UICFLAGS=-tr qt_

%.h: %.ui
	$(UIC) $(UICFLAGS) $< -o $@
%.C: %.h %.ui $(PCH_FILE)
	$(UIC) $(UICFLAGS) -impl `echo $^ | sed 's/[^ ]*pch.h.gch//'` -o $@
