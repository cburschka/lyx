include $(top_srcdir)/config/common.am

DIST_SUBDIRS = boost hunspell libiconv zlib

if USE_INCLUDED_BOOST
BOOST = boost
endif

if USE_INCLUDED_ICONV
ICONV = libiconv
endif

if USE_INCLUDED_ZLIB
ZLIB = zlib
endif

SUBDIRS = $(BOOST) $(ICONV) $(ZLIB)
