include $(top_srcdir)/config/common.am
include $(srcdir)/../Makefile.dialogs

DISTCLEANFILES += *.C

AM_CPPFLAGS = -DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR -DQT_NO_TRANSLATION \
	$(PCH_FLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/src/frontends/controllers \
	-I$(top_builddir)/src/frontends/qt2 \
	$(QT_INCLUDES) \
	$(BOOST_INCLUDES)

noinst_LTLIBRARIES = libqt2moc.la

nodist_libqt2moc_la_SOURCES = $(MOCFILES:.C=_moc.C)
# The trick.
libqt2moc_la_LINK = touch libqt2moc.la ; echo

%_moc.C: $(srcdir)/../%.h
	$(MOC) -o $@ $<
