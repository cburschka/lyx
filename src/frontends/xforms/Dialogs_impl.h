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
#include "combox.h"

#include "ControlAboutlyx.h"
#include "FormAboutlyx.h"
#include "forms/form_aboutlyx.h"

#include "FormBrowser.h"
#include "forms/form_browser.h"

#include "ControlChanges.h"
#include "FormChanges.h"
#include "forms/form_changes.h"

#include "ControlCharacter.h"
#include "FormCharacter.h"
#include "forms/form_character.h"

#include "ControlDocument.h"
#include "FormDocument.h"
#include "forms/form_document.h"

#include "ControlForks.h"
#include "FormForks.h"
#include "forms/form_forks.h"

#include "ControlLog.h"
#include "FormLog.h"

#include "ControlShowFile.h"
#include "FormShowFile.h"

#include "ControlMath.h"
#include "FormMathsPanel.h"
#include "forms/form_maths_panel.h"

#include "ControlParagraph.h"
#include "FormParagraph.h"
#include "forms/form_paragraph.h"

#include "ControlPreamble.h"
#include "FormPreamble.h"
#include "forms/form_preamble.h"

#include "ControlPrefs.h"
#include "FormPreferences.h"
#include "forms/form_preferences.h"

#include "ControlPrint.h"
#include "FormPrint.h"
#include "forms/form_print.h"

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

#include "ControlVCLog.h"
#include "FormVCLog.h"

typedef GUI<ControlAboutlyx, FormAboutlyx, OkCancelPolicy, xformsBC>
AboutlyxDialog;

typedef GUI<ControlChanges, FormChanges, NoRepeatedApplyReadOnlyPolicy, xformsBC>
ChangesDialog;

typedef GUI<ControlCharacter, FormCharacter, OkApplyCancelReadOnlyPolicy, xformsBC>
CharacterDialog;

typedef GUI<ControlDocument, FormDocument, NoRepeatedApplyReadOnlyPolicy, xformsBC>
DocumentDialog;

typedef GUI<ControlShowFile, FormShowFile, OkCancelPolicy, xformsBC>
FileDialog;

typedef GUI<ControlForks, FormForks, OkApplyCancelPolicy, xformsBC>
ForksDialog;

typedef GUI<ControlLog, FormLog, OkCancelPolicy, xformsBC>
LogFileDialog;

typedef GUI<ControlMath, FormMathsPanel, OkCancelReadOnlyPolicy, xformsBC>
MathPanelDialog;

typedef GUI<ControlParagraph, FormParagraph, OkApplyCancelReadOnlyPolicy, xformsBC>
ParagraphDialog;

typedef GUI<ControlPreamble, FormPreamble, NoRepeatedApplyReadOnlyPolicy, xformsBC>
PreambleDialog;

typedef GUI<ControlPrefs, FormPreferences, OkApplyCancelPolicy, xformsBC>
PreferencesDialog;

typedef GUI<ControlPrint, FormPrint, OkApplyCancelPolicy, xformsBC>
PrintDialog;

typedef GUI<ControlSearch, FormSearch, NoRepeatedApplyReadOnlyPolicy, xformsBC>
SearchDialog;

typedef GUI<ControlSendto, FormSendto, OkApplyCancelPolicy, xformsBC>
SendtoDialog;

typedef GUI<ControlSpellchecker, FormSpellchecker, NoRepeatedApplyReadOnlyPolicy, xformsBC>
SpellcheckerDialog;

typedef GUI<ControlTabular, FormTabular, NoRepeatedApplyReadOnlyPolicy, xformsBC>
TabularDialog;

typedef GUI<ControlTabularCreate, FormTabularCreate, OkApplyCancelReadOnlyPolicy, xformsBC>
TabularCreateDialog;

typedef GUI<ControlTexinfo, FormTexinfo, OkCancelPolicy, xformsBC>
TexinfoDialog;

#ifdef HAVE_LIBAIKSAURUS
typedef GUI<ControlThesaurus, FormThesaurus, OkApplyCancelReadOnlyPolicy, xformsBC>
ThesaurusDialog;
#endif

typedef GUI<ControlVCLog, FormVCLog, OkCancelPolicy, xformsBC>
VCLogFileDialog;

struct Dialogs::Impl {
	Impl(LyXView & lv, Dialogs & d);

	AboutlyxDialog      aboutlyx;
	ChangesDialog       changes;
        CharacterDialog     character;
	DocumentDialog      document;
	FileDialog          file;
	ForksDialog         forks;
	LogFileDialog       logfile;
	MathPanelDialog     mathpanel;
	ParagraphDialog     paragraph;
	PreambleDialog      preamble;
	PreferencesDialog   preferences;
	PrintDialog         print;
	SearchDialog        search;
	SendtoDialog        sendto;
	SpellcheckerDialog  spellchecker;
	TabularDialog       tabular;
	TabularCreateDialog tabularcreate;
	TexinfoDialog       texinfo;

#ifdef HAVE_LIBAIKSAURUS
	ThesaurusDialog     thesaurus;
#endif

	VCLogFileDialog     vclogfile;
};

#endif // DIALOGS_IMPL_H
