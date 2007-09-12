include $(top_srcdir)/config/common.am

AM_CPPFLAGS += -I$(top_srcdir)/src $(BOOST_INCLUDES)

EXTRA_DIST = tests/regfiles/biblio

pkglib_LTLIBRARIES = liblyxcontrollers.la

SOURCEFILES = \
	Dialog.cpp \
	ButtonPolicy.cpp \
	ControlBibtex.cpp \
	ControlBox.cpp \
	ControlBranch.cpp \
	ControlCharacter.cpp \
	ControlChanges.cpp \
	ControlCitation.cpp \
	ControlCommand.cpp \
	ControlCommandBuffer.cpp \
	ControlDocument.cpp \
	ControlEmbeddedFiles.cpp \
	ControlErrorList.cpp \
	ControlERT.cpp \
	ControlExternal.cpp \
	ControlFloat.cpp \
	ControlGraphics.cpp \
	ControlInclude.cpp \
	ControlListings.cpp \
	ControlLog.cpp \
	ControlViewSource.cpp \
	ControlMath.cpp \
	ControlNote.cpp \
	ControlParagraph.cpp \
	ControlPrefs.cpp \
	ControlPrint.cpp \
	ControlRef.cpp \
	ControlSearch.cpp \
	ControlSendto.cpp \
	ControlShowFile.cpp \
	ControlSpellchecker.cpp \
	ControlTabular.cpp \
	ControlTabularCreate.cpp \
	ControlTexinfo.cpp \
	ControlThesaurus.cpp \
	ControlToc.cpp \
	ControlVSpace.cpp \
	ControlWrap.cpp \
	frontend_helpers.cpp 

HEADERFILES = \
	ButtonPolicy.h \
	ControlBibtex.h \
	ControlBox.h \
	ControlBranch.h \
	ControlCharacter.h \
	ControlChanges.h \
	ControlCitation.h \
	ControlCommand.h \
	ControlCommandBuffer.h \
	ControlDocument.h \
	ControlErrorList.h \
	ControlERT.h \
	ControlEmbeddedFiles.h \
	ControlExternal.h \
	ControlFloat.h \
	ControlGraphics.h \
	ControlListings.h \
	ControlInclude.h \
	ControlLog.h \
	ControlViewSource.h \
	ControlMath.h \
	ControlNote.h \
	ControlParagraph.h \
	ControlPrefs.h \
	ControlPrint.h \
	ControlRef.h \
	ControlSearch.h \
	ControlSendto.h \
	ControlShowFile.h \
	ControlSpellchecker.h \
	ControlTabular.h \
	ControlTabularCreate.h \
	ControlTexinfo.h \
	ControlThesaurus.h \
	ControlToc.h \
	ControlVSpace.h \
	ControlWrap.h \
	frontend_helpers.h

if MONOLITHIC_CONTROLLERS

lyxcontrollers.cpp:
	@echo -e '$(SOURCEFILES:%=\n#include "%")\n' > $@

BUILT_SOURCES = lyxcontrollers.cpp

liblyxcontrollers_la_SOURCES = lyxcontrollers.cpp $(HEADERFILES) 

else

EXTRA_DIST += pch.h
BUILT_SOURCES = $(PCH_FILE)
AM_CPPFLAGS += $(PCH_FLAGS)

liblyxcontrollers_la_SOURCES = $(SOURCEFILES) $(HEADERFILES)

endif


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
