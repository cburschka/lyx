include $(top_srcdir)/config/common.am
include $(srcdir)/../../Makefile.dialogs

DISTCLEANFILES += $(UIFILES:.ui=_moc.C)

noinst_LTLIBRARIES = libqt2uimoc.la

INCLUDES = -I$(top_srcdir)/src \
	-I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/src/frontends/controllers \
	$(FRONTEND_INCLUDES) \
	$(BOOST_INCLUDES)

AM_CXXFLAGS = $(QT_CPPFLAGS) -DQT_NO_TRANSLATION

nodist_libqt2uimoc_la_SOURCES = $(UIFILES:.ui=_moc.C)

libqt2uimoc.la: $(libqt2uimoc_la_OBJECTS) $(libqt2uimoc_la_DEPENDENCIES)

%_moc.C: ../%.h
	$(MOC) -o $@ $<
