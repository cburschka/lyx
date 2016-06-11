include $(top_srcdir)/config/common.am

DIST_SUBDIRS = boost hunspell libiconv zlib

if USE_INCLUDED_BOOST
if LYX_USE_STD_REGEX
else
BOOST = boost
endif
endif

if USE_INCLUDED_HUNSPELL
HUNSPELL = hunspell
endif

if USE_INCLUDED_ICONV
ICONV = libiconv
endif

if USE_INCLUDED_ZLIB
ZLIB = zlib
endif

SUBDIRS = $(BOOST) $(HUNSPELL) $(ICONV) $(ZLIB)
