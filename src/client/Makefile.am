include $(top_srcdir)/config/common.am

CLEANFILES = $(man_MANS)

man_MANS = lyxclient.1

bin_PROGRAMS = lyxclient

EXTRA_DIST = lyxclient.1in CMakeLists.txt

AM_CPPFLAGS += -I$(srcdir)/.. \
	$(BOOST_INCLUDES) $(ICONV_INCLUDES) $(ZLIB_INCLUDES) $(NOD_INCLUDES)

lyxclient_LDADD = \
	$(top_builddir)/src/support/liblyxsupport.a \
	@LIBS@ $(ICONV_LIBS) $(ZLIB_LIBS) $(SOCKET_LIBS) \
	$(QT_LIB) $(QT_LDFLAGS) $(LIBSHLWAPI) $(LIBPSAPI)

if INSTALL_MACOSX
lyxclient_LDFLAGS = -framework AppKit \
	-Wl,-rpath,@loader_path/../Frameworks \
	-Wl,-rpath,@executable_path/../Frameworks \
	-Wl,-headerpad_max_install_names
endif

SOURCEFILES = \
  boost.cpp \
  client.cpp

HEADERFILES =

lyxclient_SOURCES = $(SOURCEFILES) $(HEADERFILES)

