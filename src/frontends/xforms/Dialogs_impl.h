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

#ifdef __GNUG__
#pragma interface
#endif

#include "Dialogs.h"
#include "controllers/GUI.h"

#include "Tooltips.h"
#include "xformsBC.h"
#include "combox.h"

#include "ControlAboutlyx.h"
#include "FormAboutlyx.h"
#include "forms/form_aboutlyx.h"

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

#include "FormDocument.h"
#include "forms/form_document.h"

#include "ControlError.h"
#include "FormError.h"
#include "forms/form_error.h"

#include "ControlERT.h"
#include "FormERT.h"
#include "forms/form_ert.h"

#include "ControlExternal.h"
#include "FormExternal.h"
#include "forms/form_external.h"

#include "ControlFloat.h"
#include "FormFloat.h"
#include "forms/form_float.h"

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
#include "FormLog.h"

#include "ControlShowFile.h"
#include "FormShowFile.h"

#include "FormMathsBitmap.h"

#include "FormMathsPanel.h"
#include "forms/form_maths_panel.h"

#include "FormMathsDeco.h"
#include "forms/form_maths_deco.h"

#include "FormMathsDelim.h"
#include "forms/form_maths_delim.h"

#include "FormMathsMatrix.h"
#include "forms/form_maths_matrix.h"

#include "FormMathsSpace.h"
#include "forms/form_maths_space.h"

#include "FormMathsStyle.h"
#include "forms/form_maths_style.h"

#include "ControlMinipage.h"
#include "FormMinipage.h"
#include "forms/form_minipage.h"

#include "ControlParagraph.h"
#include "FormParagraph.h"
#include "forms/form_paragraph.h"

#include "ControlPreamble.h"
#include "FormPreamble.h"
#include "forms/form_preamble.h"

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

#include "FormTabular.h"
#include "forms/form_tabular.h"

#include "ControlTabularCreate.h"
#include "FormTabularCreate.h"
#include "forms/form_tabular_create.h"

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
#include "FormUrl.h"
#include "forms/form_url.h"

#include "ControlVCLog.h"
#include "FormVCLog.h"

#include "ControlWrap.h"
#include "FormWrap.h"
#include "forms/form_wrap.h"


typedef GUI<ControlAboutlyx, FormAboutlyx, OkCancelPolicy, xformsBC>
AboutlyxDialog;

typedef GUI<ControlBibitem, FormBibitem, OkCancelReadOnlyPolicy, xformsBC>
BibitemDialog;

typedef GUI<ControlBibtex, FormBibtex, OkApplyCancelReadOnlyPolicy, xformsBC>
BibtexDialog;

typedef GUI<ControlCharacter, FormCharacter, OkApplyCancelReadOnlyPolicy, xformsBC>
CharacterDialog;

typedef GUI<ControlCitation, FormCitation, NoRepeatedApplyReadOnlyPolicy, xformsBC>
CitationDialog;

typedef FormDocument DocumentDialog;

typedef GUI<ControlError, FormError, OkCancelPolicy, xformsBC>
ErrorDialog;

typedef GUI<ControlERT, FormERT, NoRepeatedApplyReadOnlyPolicy, xformsBC>
ERTDialog;

typedef GUI<ControlExternal, FormExternal, OkApplyCancelReadOnlyPolicy, xformsBC>
ExternalDialog;

typedef GUI<ControlShowFile, FormShowFile, OkCancelPolicy, xformsBC>
FileDialog;

typedef GUI<ControlFloat, FormFloat, NoRepeatedApplyReadOnlyPolicy, xformsBC>
FloatDialog;

typedef GUI<ControlForks, FormForks, OkApplyCancelPolicy, xformsBC>
ForksDialog;

typedef GUI<ControlGraphics, FormGraphics, NoRepeatedApplyReadOnlyPolicy, xformsBC>
GraphicsDialog;

typedef GUI<ControlInclude, FormInclude, OkCancelReadOnlyPolicy, xformsBC>
IncludeDialog;

typedef GUI<ControlIndex, FormIndex, NoRepeatedApplyReadOnlyPolicy, xformsBC>
IndexDialog;

typedef GUI<ControlLog, FormLog, OkCancelPolicy, xformsBC>
LogFileDialog;

typedef FormMathsPanel MathPanelDialog;

typedef GUI<ControlMinipage, FormMinipage, NoRepeatedApplyReadOnlyPolicy, xformsBC>
MinipageDialog;

typedef GUI<ControlParagraph, FormParagraph, OkApplyCancelReadOnlyPolicy, xformsBC>
ParagraphDialog;

typedef GUI<ControlPreamble, FormPreamble, NoRepeatedApplyReadOnlyPolicy, xformsBC>
PreambleDialog;

typedef FormPreferences PreferencesDialog;

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

typedef FormTabular TabularDialog;

typedef GUI<ControlTabularCreate, FormTabularCreate, OkApplyCancelReadOnlyPolicy, xformsBC>
TabularCreateDialog;

typedef GUI<ControlTexinfo, FormTexinfo, OkCancelPolicy, xformsBC>
TexinfoDialog;

#ifdef HAVE_LIBAIKSAURUS
typedef GUI<ControlThesaurus, FormThesaurus, OkApplyCancelReadOnlyPolicy, xformsBC>
ThesaurusDialog;
#endif

typedef GUI<ControlToc, FormToc, OkCancelPolicy, xformsBC>
TocDialog;

typedef GUI<ControlUrl, FormUrl, NoRepeatedApplyReadOnlyPolicy, xformsBC>
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
