include $(top_srcdir)/config/common.am

SUBDIRS = $(FRONTENDS_SUBDIRS)

DIST_SUBDIRS = qt4

EXTRA_DIST = pch.h

BUILT_SOURCES = $(PCH_FILE)

noinst_LTLIBRARIES = liblyxfrontends.la

AM_CPPFLAGS += $(PCH_FLAGS) -I$(srcdir)/..  $(BOOST_INCLUDES)

liblyxfrontends_la_SOURCES = \
	ButtonPolicy.cpp \
	ButtonPolicy.h \
	Dialog.cpp \
	Dialog.h \
	frontend_helpers.cpp \
	frontend_helpers.h \
	alert.cpp \
	alert.h \
	Alert_pimpl.h \
	Application.cpp \
	Application.h \
	NoGuiFontLoader.h \
	NoGuiFontMetrics.h \
	Dialogs.cpp \
	Dialogs.h \
	FileDialog.h \
	FontLoader.h \
	FontMetrics.h \
	KeyModifier.h \
	KeySymbol.h \
	LyXView.cpp \
	LyXView.h \
	Painter.cpp \
	Painter.h \
	Clipboard.h \
	Gui.h \
	Selection.h \
	WorkArea.cpp \
	WorkArea.h \
	WorkAreaManager.cpp \
	WorkAreaManager.h \
	mouse_state.h

################################# Tests ################################

EXTRA_DIST += tests/test_biblio

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
