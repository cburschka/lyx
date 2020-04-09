include $(top_srcdir)/config/common.am

DIST_SUBDIRS = boost hunspell mythes libiconv zlib

if USE_INCLUDED_BOOST
if LYX_USE_STD_REGEX
else
BOOST = boost
endif
endif

if USE_INCLUDED_HUNSPELL
HUNSPELL = hunspell
endif

if USE_INCLUDED_MYTHES
MYTHES = mythes
endif

if USE_INCLUDED_ICONV
ICONV = libiconv
endif

if USE_INCLUDED_ZLIB
ZLIB = zlib
endif

if BUILD_INCLUDED_DTL
DTL=dtl
endif

SUBDIRS = $(BOOST) $(DTL) $(HUNSPELL) $(MYTHES) $(ICONV) $(ZLIB)

EXTRA_DIST = \
	scripts/evince_sync/evince_backward_search \
	scripts/evince_sync/evince_forward_search \
	scripts/evince_sync/evince_sync_lyx \
	scripts/evince_sync/README

