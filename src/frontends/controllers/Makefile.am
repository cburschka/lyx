AUTOMAKE_OPTIONS = foreign 1.4
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libcontrollers.o
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${top_srcdir}/src/ \
	-I${top_srcdir}/src/frontends/ \
	${SIGC_CFLAGS} $(BOOST_INCLUDES)
LIBS =
ETAGS_ARGS = --lang=c++

EXTRA_DIST = ButtonController.tmpl ControlDialog.tmpl ControlInset.tmpl

libcontrollers_o_SOURCES=\
	biblio.C \
	biblio.h \
	character.C \
	character.h \
	ButtonController.h \
	ButtonControllerBase.C \
	ButtonControllerBase.h \
	ButtonPolicies.C \
	ButtonPolicies.h \
	ControlAboutlyx.C \
	ControlAboutlyx.h \
	ControlBibitem.C \
	ControlBibitem.h \
	ControlBibtex.C \
	ControlBibtex.h \
	ControlButtons.C \
	ControlButtons.h \
	ControlCharacter.C \
	ControlCharacter.h \
	ControlCitation.C \
	ControlCitation.h \
	ControlCommand.C \
	ControlCommand.h \
	ControlConnections.C \
	ControlConnections.h \
	ControlDialog.h \
	ControlDialog_impl.C \
	ControlDialog_impl.h \
	ControlError.h \
	ControlError.C \
	ControlERT.h \
	ControlERT.C \
	ControlExternal.h \
	ControlExternal.C \
	ControlFloat.h \
	ControlFloat.C \
	ControlGraphics.h \
	ControlGraphics.C \
	ControlInclude.C \
	ControlInclude.h \
	ControlIndex.C \
	ControlIndex.h \
	ControlInset.h \
	ControlLog.C \
	ControlLog.h \
	ControlMinipage.C \
	ControlMinipage.h \
	ControlPreamble.C \
	ControlPreamble.h \
	ControlPrint.C \
	ControlPrint.h \
	ControlRef.C \
	ControlRef.h \
	ControlSearch.C \
	ControlSearch.h \
	ControlShowFile.C \
	ControlShowFile.h \
	ControlSpellchecker.C \
	ControlSpellchecker.h \
	ControlTabularCreate.C \
	ControlTabularCreate.h \
	ControlTexinfo.C \
	ControlTexinfo.h \
	ControlThesaurus.C \
	ControlThesaurus.h \
	ControlToc.C \
	ControlToc.h \
	ControlUrl.C \
	ControlUrl.h \
	ControlVCLog.C \
	ControlVCLog.h \
	GUI.h \
	ViewBase.h \
	helper_funcs.C \
	helper_funcs.h

libcontrollers.o: $(libcontrollers_o_OBJECTS) $(libcontrollers_o_DEPENDENCIES)
	rm -f ../libcontrollers.objects.new
	for fil in $(libcontrollers_o_OBJECTS) ; do \
		echo controllers/$$fil >> ../libcontrollers.objects.new ; \
	done
	if [ -f ../libcontrollers.objects ] ; then \
		cmp -s ../libcontrollers.objects ../libcontrollers.objects.new || mv ../libcontrollers.objects.new ../libcontrollers.objects ; \
	else \
		mv ../libcontrollers.objects.new ../libcontrollers.objects ; \
	fi
