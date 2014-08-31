include $(top_srcdir)/config/common.am

CLEANFILES += $(man_MANS)

man_MANS = lyxclient.1

bin_PROGRAMS = lyxclient

EXTRA_DIST = lyxclient.1in CMakeLists.txt

AM_CPPFLAGS += -I$(srcdir)/.. $(BOOST_INCLUDES)

lyxclient_LDADD = \
	$(top_builddir)/src/support/liblyxsupport.a \
	$(BOOST_LIBS) @LIBS@ $(SOCKET_LIBS) \
	$(QT_LIB) $(QT_LDFLAGS) $(LIBSHLWAPI) $(LIBPSAPI)

if INSTALL_MACOSX
lyxclient_LDFLAGS = -framework AppKit
endif

# everything below the line containing the single backslashs
# an ugly hack and needed because of the
# linking problems described in ../Makefile.am
# These four objects are linked as object files as they are not
# referenced within the core and therefore are not picked up
# by the linker without looping over libs. We do not want that,
# and in fact libtools seems not able to do that.

SOURCEFILES = \
  boost.cpp \
  client.cpp

HEADERFILES = 


lyxclient.cpp:
	@echo -e '$(SOURCEFILES:%=\n#include "%")\n' > $@

if MONOLITHIC_CLIENT

AM_CPPFLAGS += -I$(srcdir)/.. $(BOOST_INCLUDES)
BUILT_SOURCES = lyxclient.cpp
CLEANFILES += lyxclient.cpp

lyxclient_SOURCES = lyxclient.cpp $(HEADERFILES)

else

EXTRA_DIST += pch.h
AM_CPPFLAGS += $(PCH_FLAGS)
BUILT_SOURCES = $(PCH_FILE)

lyxclient_SOURCES = $(SOURCEFILES) $(HEADERFILES)

endif
