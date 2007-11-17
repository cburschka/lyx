include $(top_srcdir)/config/common.am

SUBDIRS = $(FRONTENDS_SUBDIRS) .

DIST_SUBDIRS = qt4 .

noinst_LTLIBRARIES = liblyxfrontends.la

AM_CPPFLAGS += -I$(srcdir)/..  $(BOOST_INCLUDES)

liblyxfrontends_la_SOURCES = \
	alert.h \
	Application.cpp \
	Application.h \
	NoGuiFontLoader.h \
	NoGuiFontMetrics.h \
	Dialogs.h \
	FileDialog.h \
	FontLoader.h \
	FontMetrics.h \
	KeyModifier.h \
	KeySymbol.h \
	LyXView.h \
	Painter.cpp \
	Painter.h \
	Clipboard.h \
	Selection.h \
	WorkArea.h \
	WorkAreaManager.cpp \
	WorkAreaManager.h \
	mouse_state.h

################################# Tests ################################

EXTRA_DIST = tests/test_biblio

TESTS = \
	test_biblio

check_PROGRAMS = \
	biblio

biblio_LDADD = $(BOOST_REGEX)
biblio_SOURCES = \
	tests/biblio.cpp \
	tests/boost.cpp

makeregfiles: ${check_PROGRAMS}
	for all in ${check_PROGRAMS} ; do \
		./$$all > ${srcdir}/regfiles/$$all ; \
	done
