include $(top_srcdir)/config/common.am

EXTRA_DIST = lyxclient.man

man_MANS = lyxclient.1

bin_PROGRAMS = lyxclient

INCLUDES = -I$(srcdir)/../ $(BOOST_INCLUDES)

AM_CXXFLAGS = $(PCH_FLAGS)

lyxclient_LDADD = \
	$(top_builddir)/src/support/libsupport.la \
	$(top_builddir)/boost/libs/filesystem/src/libboostfilesystem.la \
	$(top_builddir)/boost/libs/regex/src/libboostregex.la

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
