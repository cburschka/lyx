include $(top_srcdir)/config/common.am

noinst_LIBRARIES = liblyxhunspell.a

EXTRA_DIST = \
	CMakeLists.txt \
	1.6.2/AUTHORS \
	1.6.2/BUGS \
	1.6.2/ChangeLog \
	1.6.2/COPYING \
	1.6.2/COPYING.LESSER \
	1.6.2/license.hunspell \
	1.6.2/README.md \
	1.6.2/THANKS \
	1.6.2/TODO \
	1.6.2/src/hunspell/hunvisapi.h.in \
	1.6.2/src/hunspell/utf_info.cxx

AM_CPPFLAGS += -DHUNSPELL_STATIC @STDLIB_DEBUG@

liblyxhunspell_a_SOURCES = \
	1.6.2/src/hunspell/affentry.cxx \
	1.6.2/src/hunspell/affentry.hxx \
	1.6.2/src/hunspell/affixmgr.cxx \
	1.6.2/src/hunspell/affixmgr.hxx \
	1.6.2/src/hunspell/atypes.hxx \
	1.6.2/src/hunspell/baseaffix.hxx \
	1.6.2/src/hunspell/csutil.cxx \
	1.6.2/src/hunspell/csutil.hxx \
	1.6.2/src/hunspell/filemgr.cxx \
	1.6.2/src/hunspell/filemgr.hxx \
	1.6.2/src/hunspell/hashmgr.cxx \
	1.6.2/src/hunspell/hashmgr.hxx \
	1.6.2/src/hunspell/htypes.hxx \
	1.6.2/src/hunspell/hunspell.cxx \
	1.6.2/src/hunspell/hunspell.h \
	1.6.2/src/hunspell/hunspell.hxx \
	1.6.2/src/hunspell/hunvisapi.h \
	1.6.2/src/hunspell/hunzip.cxx \
	1.6.2/src/hunspell/hunzip.hxx \
	1.6.2/src/hunspell/langnum.hxx \
	1.6.2/src/hunspell/phonet.cxx \
	1.6.2/src/hunspell/phonet.hxx \
	1.6.2/src/hunspell/replist.cxx \
	1.6.2/src/hunspell/replist.hxx \
	1.6.2/src/hunspell/suggestmgr.cxx \
	1.6.2/src/hunspell/suggestmgr.hxx \
	1.6.2/src/hunspell/w_char.hxx \
	1.6.2/src/parsers/firstparser.cxx \
	1.6.2/src/parsers/firstparser.hxx \
	1.6.2/src/parsers/htmlparser.cxx \
	1.6.2/src/parsers/htmlparser.hxx \
	1.6.2/src/parsers/latexparser.cxx \
	1.6.2/src/parsers/latexparser.hxx \
	1.6.2/src/parsers/manparser.cxx \
	1.6.2/src/parsers/manparser.hxx \
	1.6.2/src/parsers/odfparser.cxx \
	1.6.2/src/parsers/odfparser.hxx \
	1.6.2/src/parsers/testparser.cxx \
	1.6.2/src/parsers/textparser.cxx \
	1.6.2/src/parsers/textparser.hxx \
	1.6.2/src/parsers/xmlparser.cxx \
	1.6.2/src/parsers/xmlparser.hxx \
	1.6.2/src/win_api/config.h

