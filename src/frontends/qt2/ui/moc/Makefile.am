include $(top_srcdir)/config/common.am
include $(srcdir)/../../Makefile.dialogs

DISTCLEANFILES += $(UIFILES:.ui=_moc.C)

AM_CPPFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR -DQT_NO_TRANSLATION \
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
