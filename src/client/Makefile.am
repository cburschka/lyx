include $(top_srcdir)/config/common.am

EXTRA_DIST = lyxclient.man

man_MANS = lyxclient.1

bin_PROGRAMS = lyxclient

AM_CPPFLAGS = -I$(srcdir)/../ $(BOOST_INCLUDES)

AM_CXXFLAGS = $(PCH_FLAGS)

if USE_INCLUDED_BOOST
BOOST_LIBS = $(top_builddir)/boost/libs/filesystem/src/libboostfilesystem.la \
	$(top_builddir)/boost/libs/regex/src/libboostregex.la
else
BOOST_LIBS = -lboost_regex -lboost_filesystem
endif

lyxclient_LDADD = \
	$(top_builddir)/src/support/libsupport.la \
	$(BOOST_LIBS) $(INTLLIBS)

lyxclient_SOURCES = \
	boost.C \
	client.C \
	debug.C \
	debug.h \
	gettext.C \
	gettext.h \
	messages.C \
	messages.h

lyxclient.1:
	cp -p $(srcdir)/lyxclient.man lyxclient.1
