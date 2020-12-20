include $(top_srcdir)/config/common.am

noinst_LIBRARIES = liblyxhunspell.a

EXTRA_DIST = \
	CMakeLists.txt \
	1.7.0/AUTHORS \
	1.7.0/ChangeLog \
	1.7.0/COPYING \
	1.7.0/COPYING.LESSER \
	1.7.0/license.hunspell \
	1.7.0/NEWS \
	1.7.0/README \
	1.7.0/THANKS \
	1.7.0/src/hunspell/hunvisapi.h.in

AM_CPPFLAGS += -DHUNSPELL_STATIC @STDLIB_DEBUG@

liblyxhunspell_a_SOURCES = \
	1.7.0/src/hunspell/affentry.cxx \
	1.7.0/src/hunspell/affentry.hxx \
	1.7.0/src/hunspell/affixmgr.cxx \
	1.7.0/src/hunspell/affixmgr.hxx \
	1.7.0/src/hunspell/atypes.hxx \
	1.7.0/src/hunspell/baseaffix.hxx \
	1.7.0/src/hunspell/csutil.cxx \
	1.7.0/src/hunspell/csutil.hxx \
	1.7.0/src/hunspell/filemgr.cxx \
	1.7.0/src/hunspell/filemgr.hxx \
	1.7.0/src/hunspell/hashmgr.cxx \
	1.7.0/src/hunspell/hashmgr.hxx \
	1.7.0/src/hunspell/htypes.hxx \
	1.7.0/src/hunspell/hunspell.cxx \
	1.7.0/src/hunspell/hunspell.h \
	1.7.0/src/hunspell/hunspell.hxx \
	1.7.0/src/hunspell/hunvisapi.h \
	1.7.0/src/hunspell/hunzip.cxx \
	1.7.0/src/hunspell/hunzip.hxx \
	1.7.0/src/hunspell/langnum.hxx \
	1.7.0/src/hunspell/phonet.cxx \
	1.7.0/src/hunspell/phonet.hxx \
	1.7.0/src/hunspell/replist.cxx \
	1.7.0/src/hunspell/replist.hxx \
	1.7.0/src/hunspell/suggestmgr.cxx \
	1.7.0/src/hunspell/suggestmgr.hxx \
	1.7.0/src/hunspell/utf_info.hxx \
	1.7.0/src/hunspell/w_char.hxx \
	1.7.0/src/win_api/config.h
