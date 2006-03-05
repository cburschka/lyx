include $(top_srcdir)/config/common.am
include $(srcdir)/../Makefile.dialogs

DISTCLEANFILES += *.C

EXTRA_DIST = pch.h

BUILT_SOURCES = $(PCH_FILE)

AM_CPPFLAGS += \
	$(QT4_CPPFLAGS) -DQT_NO_TRANSLATION \
	$(PCH_FLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/frontends \
	-I$(top_srcdir)/src/frontends/controllers \
	-I$(top_srcdir)/src/frontends/qt4 \
	-I$(top_builddir)/src/frontends/qt4 \
	$(QT4_INCLUDES) \
	$(BOOST_INCLUDES)

noinst_LTLIBRARIES = libqt4moc.la

nodist_libqt4moc_la_SOURCES = $(MOCFILES:.C=_moc.C)
# The trick.
libqt4moc_la_LINK = touch libqt4moc.la ; echo

%_moc.C: $(srcdir)/../%.h
	$(MOC4) -o $@ $<
