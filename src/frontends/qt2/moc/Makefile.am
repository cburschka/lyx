include $(top_srcdir)/config/common.am
include $(srcdir)/../Makefile.dialogs

DISTCLEANFILES += *.C

noinst_LTLIBRARIES = libqt2moc.la

INCLUDES = -I$(top_srcdir)/src \
	-I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/src/frontends/controllers \
	-I$(top_builddir)/src/frontends/qt2 \
	$(FRONTEND_INCLUDES) \
	$(BOOST_INCLUDES)

AM_CXXFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR -DQT_NO_TRANSLATION

nodist_libqt2moc_la_SOURCES = $(MOCFILES:.C=_moc.C)

libqt2moc.la: $(libqt2moc_la_OBJECTS) $(libqt2moc_la_DEPENDENCIES)

%_moc.C: $(srcdir)/../%.h
	$(MOC) -o $@ $<
