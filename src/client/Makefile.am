include $(top_srcdir)/config/common.am

CLEANFILES += $(man_MANS)

EXTRA_DIST = pch.h lyxclient.man

BUILT_SOURCES = $(PCH_FILE)

man_MANS = lyxclient.1

bin_PROGRAMS = lyxclient

AM_CPPFLAGS += $(PCH_FLAGS) -I$(srcdir)/.. $(BOOST_INCLUDES)

lyxclient_LDADD = \
	$(top_builddir)/src/support/liblyxsupport.la \
	$(BOOST_LIBS) $(INTLLIBS) @LIBS@ $(SOCKET_LIBS) 

# everything below the line containing a single backslash and the 
# spellchecker stuff is an ugly hack and needed because of the
# linking problems described in ../Makefile.am
# These four objects are linked as object files as they are not
# referenced within the core and therefore are not picked up
# by the linker without looping over libs. We do not want that,
# and in fact libtools seems not able to do that.

lyxclient_SOURCES = \
  boost.cpp \
  client.cpp \
  debug.cpp \
  debug.h \
  gettext.cpp \
  Messages.cpp \
  Messages.h \
	\
	../Box.cpp \
	../Box.h \
	../Dimension.cpp \
	../Dimension.h \
	../PrinterParams.cpp \
	../PrinterParams.h 

lyxclient.1:
	cp -p $(srcdir)/lyxclient.man lyxclient.1
