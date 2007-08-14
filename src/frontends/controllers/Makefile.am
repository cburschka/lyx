include $(top_srcdir)/config/common.am

EXTRA_DIST = pch.h BCView.tmpl

BUILT_SOURCES = $(PCH_FILE)

pkglib_LTLIBRARIES = liblyxcontrollers.la

AM_CPPFLAGS += $(PCH_FLAGS) -I$(top_srcdir)/src $(BOOST_INCLUDES)

liblyxcontrollers_la_SOURCES= \
	Dialog.cpp \
	Dialog.h \
	Kernel.cpp \
	Kernel.h \
	BCView.h \
	BCView.cpp \
	ButtonController.cpp \
	ButtonController.h \
	ButtonPolicy.cpp \
	ButtonPolicy.h \
	ControlAboutlyx.cpp \
	ControlAboutlyx.h \
	ControlBibtex.cpp \
	ControlBibtex.h \
	ControlBox.cpp \
	ControlBox.h \
	ControlBranch.cpp \
	ControlBranch.h \
	ControlCharacter.cpp \
	ControlCharacter.h \
	ControlChanges.cpp \
	ControlChanges.h \
	ControlCitation.cpp \
	ControlCitation.h \
	ControlCommand.cpp \
	ControlCommand.h \
	ControlCommandBuffer.cpp \
	ControlCommandBuffer.h \
	ControlDocument.cpp \
	ControlDocument.h \
	ControlErrorList.cpp \
	ControlErrorList.h \
	ControlERT.cpp \
	ControlERT.h \
	ControlExternal.cpp \
	ControlExternal.h \
	ControlFloat.cpp \
	ControlFloat.h \
	ControlGraphics.cpp \
	ControlGraphics.h \
	ControlInclude.cpp \
	ControlListings.h \
	ControlListings.cpp \
	ControlInclude.h \
	ControlLog.cpp \
	ControlLog.h \
	ControlViewSource.cpp \
	ControlViewSource.h \
	ControlMath.cpp \
	ControlMath.h \
	ControlNote.cpp \
	ControlNote.h \
	ControlParagraph.cpp \
	ControlParagraph.h \
	ControlPrefs.cpp \
	ControlPrefs.h \
	ControlPrint.cpp \
	ControlPrint.h \
	ControlRef.cpp \
	ControlRef.h \
	ControlSearch.cpp \
	ControlSearch.h \
	ControlSendto.cpp \
	ControlSendto.h \
	ControlShowFile.cpp \
	ControlShowFile.h \
	ControlSpellchecker.cpp \
	ControlSpellchecker.h \
	ControlTabular.cpp \
	ControlTabular.h \
	ControlTabularCreate.cpp \
	ControlTabularCreate.h \
	ControlTexinfo.cpp \
	ControlTexinfo.h \
	ControlThesaurus.cpp \
	ControlThesaurus.h \
	ControlToc.cpp \
	ControlToc.h \
	ControlVSpace.cpp \
	ControlVSpace.h \
	ControlWrap.cpp \
	ControlWrap.h \
	frontend_helpers.cpp \
	frontend_helpers.h


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
