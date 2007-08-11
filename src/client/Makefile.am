include $(top_srcdir)/config/common.am

CLEANFILES += $(man_MANS)

EXTRA_DIST = pch.h lyxclient.man

BUILT_SOURCES = $(PCH_FILE)

man_MANS = lyxclient.1

bin_PROGRAMS = lyxclient

AM_CPPFLAGS += $(PCH_FLAGS) -I$(srcdir)/.. $(BOOST_INCLUDES)

lyxclient_LDADD = \
	$(top_builddir)/src/liblyxcore.la \
	$(top_builddir)/src/liblyxinsets.la \
	$(top_builddir)/src/liblyxmathed.la \
	$(top_builddir)/src/liblyxgraphics.la \
	$(top_builddir)/src/frontends/liblyxfrontends.la \
	$(top_builddir)/src/frontends/controllers/liblyxcontrollers.la \
	$(top_builddir)/src/frontends/qt4/liblyxqt4.la \
	$(top_builddir)/src/support/liblyxsupport.la \
	$(BOOST_LIBS) $(INTLLIBS) @LIBS@ $(SOCKET_LIBS)

lyxclient_SOURCES = \
  boost.cpp \
  client.cpp \
  debug.cpp \
  debug.h \
  gettext.cpp \
  Messages.cpp \
  Messages.h

lyxclient.1:
	cp -p $(srcdir)/lyxclient.man lyxclient.1
