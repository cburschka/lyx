include $(top_srcdir)/config/common.am

EXTRA_DIST = pch.h test_biblio

BUILT_SOURCES = $(PCH_FILE)

TESTS = \
	test_biblio

check_PROGRAMS = \
	biblio

AM_CPPFLAGS += $(PCH_FLAGS) $(BOOST_INCLUDES)

biblio_LDADD = $(BOOST_REGEX)
biblio_SOURCES = \
	biblio.cpp \
	boost.cpp

makeregfiles: ${check_PROGRAMS}
	for all in ${check_PROGRAMS} ; do \
		./$$all > ${srcdir}/regfiles/$$all ; \
	done
