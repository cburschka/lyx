include $(top_srcdir)/config/common.am
include $(srcdir)/../../Makefile.dialogs

DISTCLEANFILES += $(UIMOCDIALOGS)

noinst_LTLIBRARIES = libqt2uimoc.la

INCLUDES = -I$(top_srcdir)/src/ \
	-I$(top_srcdir)/src/frontends/ \
	-I$(top_srcdir)/src/frontends/controllers \
	$(SIGC_CFLAGS) \
	$(FRONTEND_INCLUDES) \
	$(BOOST_INCLUDES)

AM_CXXFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR

nodist_libqt2uimoc_la_SOURCES = $(UIMOCDIALOGS)

libqt2uimoc.la: $(libqt2uimoc_la_OBJECTS) $(libqt2uimoc_la_DEPENDENCIES)

%_moc.C: ../%.h
	$(MOC) -o $@ $<
