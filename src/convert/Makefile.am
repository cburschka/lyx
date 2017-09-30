include $(top_srcdir)/config/common.am

#man_MANS = lyxconvert.1

DEFAULT_INCLUDES =

if INSTALL_MACOSX

bin_PROGRAMS = lyxconvert

AM_CPPFLAGS += -I$(top_srcdir)/src/convert \
	$(QT_CPPFLAGS) \
	-DQT_NO_CAST_TO_ASCII \
	-DQT_NO_STL \
	$(QT_INCLUDES)

lyxconvert_SOURCES = \
	lyxconvert.cpp

lyxconvert_LDADD = \
	$(QT_LIB) $(QT_LDFLAGS) \
	$(ICONV_LIBS) $(ZLIB_LIBS) $(LIBSHLWAPI) $(LIBPSAPI)

lyxconvert_LDFLAGS = -framework AppKit \
	-Wl,-rpath,@loader_path/../Frameworks \
	-Wl,-rpath,@executable_path/../Frameworks

else

bin_PROGRAMS =

endif
