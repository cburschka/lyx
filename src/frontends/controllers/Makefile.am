include $(top_srcdir)/config/common.am

noinst_LTLIBRARIES = libcontrollers.la

INCLUDES = -I$(top_srcdir)/src/ \
	$(SIGC_CFLAGS) $(BOOST_INCLUDES)

EXTRA_DIST = ButtonController.tmpl ControlDialog.tmpl ControlInset.tmpl

libcontrollers_la_SOURCES= \
	biblio.C \
	biblio.h \
	character.C \
	character.h \
	frnt_lang.C \
	frnt_lang.h \
	tex_helpers.C \
	tex_helpers.h \
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
	ControlCommandBuffer.C \
	ControlCommandBuffer.h \
	ControlConnections.C \
	ControlConnections.h \
	ControlDialog.h \
	ControlDialog_impl.C \
	ControlDialog_impl.h \
	ControlError.C \
	ControlError.h \
	ControlERT.C \
	ControlERT.h \
	ControlExternal.C \
	ControlExternal.h \
	ControlFloat.C \
	ControlFloat.h \
	ControlForks.C \
	ControlForks.h \
	ControlGraphics.C \
	ControlGraphics.h \
	ControlInclude.C \
	ControlInclude.h \
	ControlIndex.C \
	ControlIndex.h \
	ControlInset.h \
	ControlLog.C \
	ControlLog.h \
	ControlMinipage.C \
	ControlMinipage.h \
	ControlParagraph.C \
	ControlParagraph.h \
	ControlPreamble.C \
	ControlPreamble.h \
	ControlPrint.C \
	ControlPrint.h \
	ControlRef.C \
	ControlRef.h \
	ControlSearch.C \
	ControlSearch.h \
	ControlSendto.C \
	ControlSendto.h \
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

libcontrollers.la: $(libcontrollers_la_OBJECTS) $(libcontrollers_la_DEPENDENCIES)
