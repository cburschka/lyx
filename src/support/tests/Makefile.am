include $(top_srcdir)/config/common.am

EXTRA_DIST = test_convert test_lstrings regfiles

TESTS = \
	test_convert \
	test_lstrings

check_PROGRAMS = \
	convert \
	lstrings

AM_CPPFLAGS = $(BOOST_INCLUDES)

convert_LDADD = ../convert.o
convert_SOURCES = \
	convert.C \
	boost.C

lstrings_LDADD = ../lstrings.o
lstrings_SOURCES = \
	lstrings.C \
	boost.C

makeregfiles: ${check_PROGRAMS}
	for all in ${check_PROGRAMS} ; do \
		./$$all > ${srcdir}/regfiles/$$all ; \
	done
