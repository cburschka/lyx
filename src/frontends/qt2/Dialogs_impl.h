// -*- C++ -*-
/**
 * \file qt2/Dialogs_impl.h
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

#include "ControlAboutlyx.h"
#include "ControlChanges.h"
#include "ControlCharacter.h"
#include "ControlDocument.h"
#include "ControlExternal.h"
#include "ControlFloat.h"
#include "ControlForks.h"
#include "ControlGraphics.h"
#include "insets/insetgraphicsParams.h"
#include "ControlInclude.h"
#include "ControlLog.h"
#include "ControlMinipage.h"
#include "ControlParagraph.h"
#include "ControlPrefs.h"
#include "ControlPrint.h"
#include "ControlSearch.h"
#include "ControlSendto.h"
#include "ControlShowFile.h"
#include "ControlSpellchecker.h"
#include "ControlTabularCreate.h"
#include "ControlTabular.h"
#include "ControlTexinfo.h"
#include "ControlVCLog.h"
#include "ControlWrap.h"

#include "QAbout.h"
#include "QAboutDialog.h"
#include "QChanges.h"
#include "QChangesDialog.h"
#include "QCharacter.h"
#include "QCharacterDialog.h"
#include "QDocument.h"
#include "QDocumentDialog.h"
#include "QExternal.h"
#include "QExternalDialog.h"
#include "QFloat.h"
#include "QFloatDialog.h"
//#include "QForks.h"
// Here would be an appropriate point to lecture on the evils
// of the Qt headers, those most fucked up of disgusting ratholes.
// But I won't.
#undef signals
#include "QGraphics.h"
#include "QGraphicsDialog.h"
#include "QInclude.h"
#include "QIncludeDialog.h"
#include "QLog.h"
#include "QLogDialog.h"
#include "QMinipage.h"
#include "QMinipageDialog.h"
#include "QParagraph.h"
#include "QParagraphDialog.h"
#include "QPrefs.h"
#include "QPrefsDialog.h"
#include "QPrint.h"
#include "QLPrintDialog.h"
#include "QSearch.h"
#include "QSearchDialog.h"
#include "QSendto.h"
#include "QSendtoDialog.h"
#include "QShowFile.h"
#include "QShowFileDialog.h"
#include "QSpellchecker.h"
#include "QSpellcheckerDialog.h"
#include "QTabularCreate.h"
#include "QTabularCreateDialog.h"
#include "QTabular.h"
#include "QTabularDialog.h"
#include "QTexinfo.h"
#include "QTexinfoDialog.h"

#ifdef HAVE_LIBAIKSAURUS
#include "ControlThesaurus.h"
#include "QThesaurus.h"
#include "QThesaurusDialog.h"
#endif

#include "QVCLog.h"
#include "QVCLogDialog.h"
#include "QWrap.h"
#include "QWrapDialog.h"

#include "Qt2BC.h"



typedef GUI<ControlAboutlyx, QAbout, OkCancelPolicy, Qt2BC>
AboutlyxDialog;

typedef GUI<ControlChanges, QChanges, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
ChangesDialog;

typedef GUI<ControlCharacter, QCharacter, OkApplyCancelReadOnlyPolicy, Qt2BC>
CharacterDialog;

typedef GUI<ControlDocument, QDocument, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
DocumentDialog;

typedef GUI<ControlExternal, QExternal, OkApplyCancelReadOnlyPolicy, Qt2BC>
ExternalDialog;

typedef GUI<ControlShowFile, QShowFile, OkCancelPolicy, Qt2BC>
FileDialog;

typedef GUI<ControlFloat, QFloat, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
FloatDialog;

typedef GUI<ControlGraphics, QGraphics, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
GraphicsDialog;

typedef GUI<ControlInclude, QInclude, OkCancelReadOnlyPolicy, Qt2BC>
IncludeDialog;

typedef GUI<ControlLog, QLog, OkCancelPolicy, Qt2BC>
LogFileDialog;

typedef GUI<ControlMinipage, QMinipage, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
MinipageDialog;

typedef GUI<ControlParagraph, QParagraph, OkApplyCancelReadOnlyPolicy, Qt2BC>
ParagraphDialog;

typedef GUI<ControlPrefs, QPrefs, OkApplyCancelPolicy, Qt2BC>
PrefsDialog;

typedef GUI<ControlPrint, QPrint, OkApplyCancelPolicy, Qt2BC>
PrintDialog;

typedef GUI<ControlSearch, QSearch, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
SearchDialog;

typedef GUI<ControlSendto, QSendto, OkApplyCancelPolicy, Qt2BC>
SendtoDialog;

typedef GUI<ControlSpellchecker, QSpellchecker, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
SpellcheckerDialog;

typedef GUI<ControlTabularCreate, QTabularCreate, OkApplyCancelReadOnlyPolicy, Qt2BC>
TabularCreateDialog;

typedef GUI<ControlTabular, QTabular, OkApplyCancelReadOnlyPolicy, Qt2BC>
TabularDialog;

typedef GUI<ControlTexinfo, QTexinfo, OkCancelPolicy, Qt2BC>
TexinfoDialog;

#ifdef HAVE_LIBAIKSAURUS
typedef GUI<ControlThesaurus, QThesaurus, OkApplyCancelReadOnlyPolicy, Qt2BC>
ThesaurusDialog;
#endif

typedef GUI<ControlVCLog, QVCLog, OkCancelPolicy, Qt2BC>
VCLogFileDialog;

typedef GUI<ControlWrap, QWrap, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
WrapDialog;


struct Dialogs::Impl {
	Impl(LyXView & lv, Dialogs & d);

	AboutlyxDialog      aboutlyx;
	ChangesDialog       changes;
	CharacterDialog     character;
	DocumentDialog      document;
	ExternalDialog      external;
	FileDialog          file;
	FloatDialog         floats;
	GraphicsDialog      graphics;
	IncludeDialog       include;
	LogFileDialog       logfile;
	MinipageDialog      minipage;
	ParagraphDialog     paragraph;
	PrefsDialog         prefs;
	PrintDialog         print;
	SearchDialog        search;
	SendtoDialog        sendto;
	SpellcheckerDialog  spellchecker;
	TabularCreateDialog tabularcreate;
	TabularDialog       tabular;
	TexinfoDialog       texinfo;

#ifdef HAVE_LIBAIKSAURUS
	ThesaurusDialog     thesaurus;
#endif

	VCLogFileDialog     vclogfile;
	WrapDialog          wrap;
};

#endif // DIALOGS_IMPL_H
