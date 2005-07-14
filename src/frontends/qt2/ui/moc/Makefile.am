include $(top_srcdir)/config/common.am
include $(srcdir)/../../Makefile.dialogs

DISTCLEANFILES += $(UIFILES:.ui=_moc.C)

EXTRA_DIST = pch.h

BUILT_SOUCES = $(PCH_FILE)

AM_CPPFLAGS += \
	$(QT_CPPFLAGS) -DQT_NO_TRANSLATION \
	$(PCH_FLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/src/frontends/controllers \
	$(QT_INCLUDES) \
	$(BOOST_INCLUDES)

noinst_LTLIBRARIES = libqt2uimoc.la

nodist_libqt2uimoc_la_SOURCES = $(UIFILES:.ui=_moc.C)
# The trick.
libqt2uimoc_la_LINK = touch libqt2uimoc.la ; echo

%_moc.C: ../%.h
	$(MOC) -o $@ $<
