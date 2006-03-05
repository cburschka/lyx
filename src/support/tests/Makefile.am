include $(top_srcdir)/config/common.am

EXTRA_DIST = pch.h test_convert test_filetools test_lstrings regfiles

BUILT_SOURCES = $(PCH_FILE)

TESTS = \
	test_convert \
	test_filetools \
	test_lstrings

check_PROGRAMS = \
	convert \
	filetools \
	lstrings

AM_CPPFLAGS += $(BOOST_INCLUDES)

convert_LDADD = ../convert.o
convert_SOURCES = \
	convert.C \
	boost.C

filetools_LDADD = ../../debug.o ../libsupport.la $(BOOST_REGEX) $(BOOST_FILESYSTEM)
filetools_SOURCES = \
	filetools.C \
	boost.C

lstrings_LDADD = ../lstrings.o
lstrings_SOURCES = \
	lstrings.C \
	boost.C

makeregfiles: ${check_PROGRAMS}
	for all in ${check_PROGRAMS} ; do \
		./$$all > ${srcdir}/regfiles/$$all ; \
	done
