AUTOMAKE_OPTIONS = foreign 1.4
DISTCLEANFILES= *.orig *.rej *~ *.bak core
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
noinst_LTLIBRARIES = libcontrollers.la
BOOST_INCLUDES = -I$(top_srcdir)/boost
INCLUDES = -I${top_srcdir}/src/ \
	-I${top_srcdir}/src/frontends/ \
	${SIGC_CFLAGS} $(BOOST_INCLUDES)
LIBS =
SUBDIRS = 
ETAGS_ARGS = --lang=c++
libcontrollers_la_SOURCES=\
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
	ControlDialogs.h \
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
	ControlSpellchecker.C \
	ControlSpellchecker.h \
	ControlSplash.C \
	ControlSplash.h \
	ControlTabularCreate.C \
	ControlTabularCreate.h \
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

# just copied from old lyx repository
dist-hook:
	for subdir in $(LYXDATADIRS) ; do \
	test -d $(distdir)/$$subdir \
	|| mkdir $(distdir)/$$subdir \
	|| exit 1; \
	chmod 777 $(distdir)/$$subdir; \
	list=`(cd $(srcdir)/$$subdir && ls -1 | grep -v CVS)`; \
	echo $$list ; \
	for fil in $$list ; do \
	cp -p $(srcdir)/$$subdir/$$fil $(distdir)/$$subdir ; \
	done ; \
	done
