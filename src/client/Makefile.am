include $(top_srcdir)/config/common.am

EXTRA_DIST = lyxclient.man

man_MANS = lyxclient.1

bin_PROGRAMS = lyxclient

AM_CPPFLAGS = $(PCH_FLAGS) -I$(srcdir)/.. $(BOOST_INCLUDES)

BOOST_LIBS = $(BOOST_REGEX) $(BOOST_FILESYSTEM)

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
