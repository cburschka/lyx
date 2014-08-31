include $(top_srcdir)/config/common.am

SUBDIRS = $(FRONTENDS_SUBDIRS) .

DIST_SUBDIRS = qt4 .

noinst_LIBRARIES = liblyxfrontends.a

AM_CPPFLAGS += -I$(srcdir)/..  $(BOOST_INCLUDES)

liblyxfrontends_a_SOURCES = \
	alert.h \
	Application.h \
	FontLoader.h \
	FontMetrics.h \
	Delegates.h \
	KeyModifier.h \
	KeySymbol.h \
	Painter.h \
	Clipboard.h \
	Selection.h \
	WorkArea.h \
	WorkAreaManager.cpp \
	WorkAreaManager.h \
	mouse_state.h

################################# Tests ################################

EXTRA_DIST = \
	CMakeLists.txt \
	tests/CMakeLists.txt \
	tests/test_biblio.cmake \
	tests/test_biblio \
	tests/regfiles/biblio

TESTS = \
	tests/test_biblio

check_PROGRAMS = \
	biblio

biblio_LDADD = $(BOOST_LIBS)
biblio_SOURCES = \
	tests/biblio.cpp \
	tests/boost.cpp

makeregfiles: ${check_PROGRAMS}
	for all in ${check_PROGRAMS} ; do \
		./$$all > ${srcdir}/tests/regfiles/$$all ; \
	done
