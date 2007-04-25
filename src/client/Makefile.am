include $(top_srcdir)/config/common.am

CLEANFILES += $(man_MANS)

EXTRA_DIST = pch.h lyxclient.man

BUILT_SOURCES = $(PCH_FILE)

man_MANS = lyxclient.1

bin_PROGRAMS = lyxclient

AM_CPPFLAGS += $(PCH_FLAGS) -I$(srcdir)/.. $(BOOST_INCLUDES)

BOOST_LIBS = $(BOOST_REGEX) $(BOOST_FILESYSTEM)

lyxclient_LDADD = \
	$(top_builddir)/src/support/libsupport.la \
	$(BOOST_LIBS) $(LIBICONV) $(INTLLIBS) @LIBS@ $(SOCKET_LIBS)

lyxclient_SOURCES = \
	boost.cpp \
	client.C \
	debug.C \
	debug.h \
	gettext.C \
	messages.C \
	messages.h

lyxclient.1:
	cp -p $(srcdir)/lyxclient.man lyxclient.1
