// -*- C++ -*-
/**
 * \file xforms/Dialogs_impl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef DIALOGS_IMPL_H
#define DIALOGS_IMP_H

#include "Dialogs.h"
#include "controllers/GUI.h"

#include "Tooltips.h"
#include "xformsBC.h"
#include "gnomeBC.h"
#include "combox.h"

#include "ControlAboutlyx.h"
#include "GAbout.h"

#include "ControlBibitem.h"
#include "FormBibitem.h"
#include "forms/form_bibitem.h"

#include "ControlBibtex.h"
#include "FormBibtex.h"
#include "forms/form_bibtex.h"

#include "FormBrowser.h"
#include "forms/form_browser.h"

#include "ControlCharacter.h"
#include "FormCharacter.h"
#include "forms/form_character.h"

#include "ControlCitation.h"
#include "FormCitation.h"
#include "forms/form_citation.h"

#include "ControlChanges.h"
#include "GChanges.h"

#include "ControlDocument.h"
#include "FormDocument.h"
#include "forms/form_document.h"

#include "ControlError.h"
#include "GError.h"

#include "ControlERT.h"
#include "GERT.h"

#include "ControlExternal.h"
#include "FormExternal.h"
#include "forms/form_external.h"

#include "ControlFloat.h"
#include "GFloat.h"

#include "ControlForks.h"
#include "FormForks.h"
#include "forms/form_forks.h"

#include "ControlGraphics.h"
#include "FormGraphics.h"
#include "forms/form_graphics.h"

#include "ControlInclude.h"
#include "FormInclude.h"
#include "forms/form_include.h"

#include "ControlIndex.h"
#include "FormIndex.h"
#include "forms/form_index.h"

#include "ControlLog.h"
#include "GLog.h"

#include "ControlShowFile.h"
#include "FormShowFile.h"

#include "ControlMath.h"
#include "FormMathsPanel.h"
#include "forms/form_maths_panel.h"

#include "ControlMinipage.h"
#include "FormMinipage.h"
#include "forms/form_minipage.h"

#include "ControlParagraph.h"
#include "FormParagraph.h"
#include "forms/form_paragraph.h"

#include "ControlPreamble.h"
#include "GPreamble.h"

#include "ControlPrefs.h"
#include "FormPreferences.h"
#include "forms/form_preferences.h"

#include "ControlPrint.h"
#include "FormPrint.h"
#include "forms/form_print.h"

#include "ControlRef.h"
#include "FormRef.h"
#include "forms/form_ref.h"

#include "ControlSearch.h"
#include "FormSearch.h"
#include "forms/form_search.h"

#include "ControlSendto.h"
#include "FormSendto.h"
#include "forms/form_sendto.h"

#include "ControlSpellchecker.h"
#include "FormSpellchecker.h"
#include "forms/form_spellchecker.h"

#include "ControlTabular.h"
#include "FormTabular.h"
#include "forms/form_tabular.h"

#include "ControlTabularCreate.h"
#include "GTabularCreate.h"

#include "ControlTexinfo.h"
#include "FormTexinfo.h"
#include "forms/form_texinfo.h"

#ifdef HAVE_LIBAIKSAURUS
#include "ControlThesaurus.h"
#include "FormThesaurus.h"
#include "forms/form_thesaurus.h"
#endif

#include "ControlToc.h"
#include "FormToc.h"
#include "forms/form_toc.h"

#include "ControlUrl.h"
#include "GUrl.h"

#include "ControlVCLog.h"
#include "FormVCLog.h"

#include "ControlWrap.h"
#include "FormWrap.h"
#include "forms/form_wrap.h"


typedef GUI<ControlAboutlyx, GAbout, OkCancelPolicy, gnomeBC>
AboutlyxDialog;

typedef GUI<ControlBibitem, FormBibitem, OkCancelReadOnlyPolicy, xformsBC>
BibitemDialog;

typedef GUI<ControlBibtex, FormBibtex, NoRepeatedApplyReadOnlyPolicy, xformsBC>
BibtexDialog;

typedef GUI<ControlCharacter, FormCharacter, OkApplyCancelReadOnlyPolicy, xformsBC>
CharacterDialog;

typedef GUI<ControlChanges, GChanges, NoRepeatedApplyReadOnlyPolicy, gnomeBC>
ChangesDialog;

typedef GUI<ControlCitation, FormCitation, NoRepeatedApplyReadOnlyPolicy, xformsBC>
CitationDialog;

typedef GUI<ControlDocument, FormDocument, NoRepeatedApplyReadOnlyPolicy, xformsBC>
DocumentDialog;

typedef GUI<ControlError, GErrorDialog, OkCancelPolicy, gnomeBC>
ErrorDialog;

typedef GUI<ControlERT, GERT, NoRepeatedApplyReadOnlyPolicy, gnomeBC>
ERTDialog;

typedef GUI<ControlExternal, FormExternal, OkApplyCancelReadOnlyPolicy, xformsBC>
ExternalDialog;

typedef GUI<ControlShowFile, FormShowFile, OkCancelPolicy, xformsBC>
FileDialog;

typedef GUI<ControlFloat, GFloat, NoRepeatedApplyReadOnlyPolicy, gnomeBC>
FloatDialog;

typedef GUI<ControlForks, FormForks, OkApplyCancelPolicy, xformsBC>
ForksDialog;

typedef GUI<ControlGraphics, FormGraphics, NoRepeatedApplyReadOnlyPolicy, xformsBC>
GraphicsDialog;

typedef GUI<ControlInclude, FormInclude, OkCancelReadOnlyPolicy, xformsBC>
IncludeDialog;

typedef GUI<ControlIndex, FormIndex, NoRepeatedApplyReadOnlyPolicy, xformsBC>
IndexDialog;

typedef GUI<ControlLog, GLog, OkCancelPolicy, gnomeBC>
LogFileDialog;

typedef GUI<ControlMath, FormMathsPanel, OkCancelReadOnlyPolicy, xformsBC>
MathPanelDialog;

typedef GUI<ControlMinipage, FormMinipage, NoRepeatedApplyReadOnlyPolicy, xformsBC>
MinipageDialog;

typedef GUI<ControlParagraph, FormParagraph, OkApplyCancelReadOnlyPolicy, xformsBC>
ParagraphDialog;

typedef GUI<ControlPreamble, GPreamble, NoRepeatedApplyReadOnlyPolicy, gnomeBC>
PreambleDialog;

typedef GUI<ControlPrefs, FormPreferences, OkApplyCancelPolicy, xformsBC>
PreferencesDialog;

typedef GUI<ControlPrint, FormPrint, OkApplyCancelPolicy, xformsBC>
PrintDialog;

typedef GUI<ControlRef, FormRef, NoRepeatedApplyReadOnlyPolicy, xformsBC>
RefDialog;

typedef GUI<ControlSearch, FormSearch, NoRepeatedApplyReadOnlyPolicy, xformsBC>
SearchDialog;

typedef GUI<ControlSendto, FormSendto, OkApplyCancelPolicy, xformsBC>
SendtoDialog;

typedef GUI<ControlSpellchecker, FormSpellchecker, NoRepeatedApplyReadOnlyPolicy, xformsBC>
SpellcheckerDialog;

typedef GUI<ControlTabular, FormTabular, NoRepeatedApplyReadOnlyPolicy, xformsBC>
TabularDialog;

typedef GUI<ControlTabularCreate, GTabularCreate, OkApplyCancelReadOnlyPolicy, gnomeBC>
TabularCreateDialog;

typedef GUI<ControlTexinfo, FormTexinfo, OkCancelPolicy, xformsBC>
TexinfoDialog;

#ifdef HAVE_LIBAIKSAURUS
typedef GUI<ControlThesaurus, FormThesaurus, OkApplyCancelReadOnlyPolicy, xformsBC>
ThesaurusDialog;
#endif

typedef GUI<ControlToc, FormToc, OkCancelPolicy, xformsBC>
TocDialog;

typedef GUI<ControlUrl, GUrl, NoRepeatedApplyReadOnlyPolicy, gnomeBC>
UrlDialog;

typedef GUI<ControlVCLog, FormVCLog, OkCancelPolicy, xformsBC>
VCLogFileDialog;

typedef GUI<ControlWrap, FormWrap, NoRepeatedApplyReadOnlyPolicy, xformsBC>
WrapDialog;

struct Dialogs::Impl {
	Impl(LyXView & lv, Dialogs & d);

	AboutlyxDialog      aboutlyx;
	BibitemDialog       bibitem;
	BibtexDialog        bibtex;
	CharacterDialog     character;
	ChangesDialog       changes;
	CitationDialog      citation;
	DocumentDialog      document;
	ErrorDialog         error;
	ERTDialog           ert;
	ExternalDialog      external;
	FileDialog          file;
	FloatDialog         floats;
	ForksDialog         forks;
	GraphicsDialog      graphics;
	IncludeDialog       include;
	IndexDialog         index;
	LogFileDialog       logfile;
	MathPanelDialog     mathpanel;
	MinipageDialog      minipage;
	ParagraphDialog     paragraph;
	PreambleDialog      preamble;
	PreferencesDialog   preferences;
	PrintDialog         print;
	RefDialog           ref;
	SearchDialog        search;
	SendtoDialog        sendto;
	SpellcheckerDialog  spellchecker;
	TabularDialog       tabular;
	TabularCreateDialog tabularcreate;
	TexinfoDialog       texinfo;

#ifdef HAVE_LIBAIKSAURUS
	ThesaurusDialog     thesaurus;
#endif

	TocDialog           toc;
	UrlDialog           url;
	VCLogFileDialog     vclogfile;
	WrapDialog          wrap;
};

#endif // DIALOGS_IMPL_H
