include $(top_srcdir)/config/common.am

DISTCLEANFILES += $(libqt2uimoc_la_SOURCES)

noinst_LTLIBRARIES = libqt2uimoc.la

INCLUDES = -I$(top_srcdir)/src/ \
	-I$(top_srcdir)/src/frontends/ \
	-I$(top_srcdir)/src/frontends/controllers \
	$(SIGC_CFLAGS) \
	$(FRONTEND_INCLUDES) \
	$(BOOST_INCLUDES)

AM_CXXFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR

include ../../Makefile.dialogs

libqt2uimoc_la_SOURCES := $(UIMOCDIALOGS)

libqt2uimoc.la: $(libqt2uimoc_la_OBJECTS) $(libqt2uimoc_la_DEPENDENCIES)

%_moc.C: ../%.h
	$(MOC) -o $@ $<
