// -*- C++ -*-
/**
 * \file Dialogs_impl.h
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

#include "ControlAboutlyx.h"
#include "ControlBibitem.h"
#include "ControlBibtex.h"
#include "ControlCharacter.h"
#include "ControlCitation.h"
#include "ControlError.h"
#include "ControlERT.h"
#include "ControlExternal.h"
#include "ControlFloat.h"
#include "ControlForks.h"
#include "ControlGraphics.h"
#include "insets/insetgraphicsParams.h"
#include "ControlInclude.h"
#include "ControlIndex.h"
#include "ControlLog.h"
#include "ControlMinipage.h"
#include "ControlParagraph.h"
#include "ControlPreamble.h"
#include "ControlPrint.h"
#include "ControlRef.h"
#include "ControlSearch.h"
#include "ControlSendto.h"
#include "ControlShowFile.h"
#include "ControlSpellchecker.h"
#include "ControlTabularCreate.h"
#include "ControlTexinfo.h"
#include "ControlToc.h"
#include "ControlUrl.h"
#include "ControlVCLog.h"
#include "ControlWrap.h"

#include "QAbout.h"
#include "QAboutDialog.h"
#include "QBibitem.h"
#include "QBibitemDialog.h"
#include "QBibtex.h"
#include "QBibtexDialog.h"
#include "QCharacter.h"
#include "QCharacterDialog.h"
#include "QCitation.h"
#include "QCitationDialog.h"
//#include "QDocument.h"
#include "QError.h"
#include "QErrorDialog.h"
#include "QERT.h"
#include "QERTDialog.h"
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
#include "QIndex.h"
#include "QIndexDialog.h"
#include "QLog.h"
#include "QLogDialog.h"
#include "QMinipage.h"
#include "QMinipageDialog.h"
#include "QParagraph.h"
#include "QParagraphDialog.h"
#include "QPreamble.h"
#include "QPreambleDialog.h"
//#include "QPreferences.h"
#include "QPrint.h"
#include "QPrintDialog.h"
#include "QRef.h"
#include "QRefDialog.h"
#include "QSearch.h"
#include "QSearchDialog.h"
//#include "QSendto.h"
#include "QShowFile.h"
#include "QShowFileDialog.h"
#include "QSpellchecker.h"
#include "QSpellcheckerDialog.h"
//#include "QTabular.h"
#include "QTabularCreate.h"
#include "QTabularCreateDialog.h"
#include "QTexinfo.h"
#include "QTexinfoDialog.h"

#ifdef HAVE_LIBAIKSAURUS
#include "ControlThesaurus.h"
#include "QThesaurus.h"
#include "QThesaurusDialog.h"
#endif

#include "QToc.h"
#include "QTocDialog.h"
#include "QURL.h"
#include "QURLDialog.h"
#include "QVCLog.h"
#include "QVCLogDialog.h"

#include "Qt2BC.h"



typedef GUI<ControlAboutlyx, QAbout, OkCancelPolicy, Qt2BC>
AboutlyxDialog;

typedef GUI<ControlBibitem, QBibitem, OkCancelReadOnlyPolicy, Qt2BC>
BibitemDialog;

typedef GUI<ControlBibtex, QBibtex, OkCancelReadOnlyPolicy, Qt2BC>
BibtexDialog;

typedef GUI<ControlCharacter, QCharacter, OkApplyCancelReadOnlyPolicy, Qt2BC>
CharacterDialog;

typedef GUI<ControlCitation, QCitation, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
CitationDialog;

typedef GUI<ControlError, QError, OkCancelPolicy, Qt2BC>
ErrorDialog;

typedef GUI<ControlERT, QERT, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
ERTDialog;

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

typedef GUI<ControlIndex, QIndex, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
IndexDialog;

typedef GUI<ControlLog, QLog, OkCancelPolicy, Qt2BC>
LogFileDialog;

typedef GUI<ControlMinipage, QMinipage, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
MinipageDialog;

typedef GUI<ControlParagraph, QParagraph, OkApplyCancelReadOnlyPolicy, Qt2BC>
ParagraphDialog;

typedef GUI<ControlPreamble, QPreamble, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
PreambleDialog;

typedef GUI<ControlPrint, QPrint, OkApplyCancelPolicy, Qt2BC>
PrintDialog;

typedef GUI<ControlRef, QRef, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
RefDialog;

typedef GUI<ControlSearch, QSearch, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
SearchDialog;

typedef GUI<ControlSpellchecker, QSpellchecker, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
SpellcheckerDialog;

typedef GUI<ControlTabularCreate, QTabularCreate, OkApplyCancelReadOnlyPolicy, Qt2BC>
TabularCreateDialog;

typedef GUI<ControlTexinfo, QTexinfo, OkCancelPolicy, Qt2BC>
TexinfoDialog;

#ifdef HAVE_LIBAIKSAURUS
typedef GUI<ControlThesaurus, QThesaurus, OkApplyCancelReadOnlyPolicy, Qt2BC>
ThesaurusDialog;
#endif

typedef GUI<ControlToc, QToc, OkCancelPolicy, Qt2BC>
TocDialog;

typedef GUI<ControlUrl, QURL, NoRepeatedApplyReadOnlyPolicy, Qt2BC>
UrlDialog;

typedef GUI<ControlVCLog, QVCLog, OkCancelPolicy, Qt2BC>
VCLogFileDialog;


struct Dialogs::Impl {
	Impl(LyXView & lv, Dialogs & d);

        AboutlyxDialog      aboutlyx;
	BibitemDialog       bibitem;
        BibtexDialog        bibtex;
        CharacterDialog     character;
	CitationDialog      citation;
        ErrorDialog         error;
	ERTDialog           ert;
        ExternalDialog      external;
	FileDialog          file;
        FloatDialog         floats;
        GraphicsDialog      graphics;
	IncludeDialog       include;
        IndexDialog         index;
	LogFileDialog       logfile;
	MinipageDialog      minipage;
	ParagraphDialog     paragraph;
	PreambleDialog      preamble;
	PrintDialog         print;
	RefDialog           ref;
	SearchDialog        search;
	SpellcheckerDialog  spellchecker;
	TabularCreateDialog tabularcreate;
	TexinfoDialog       texinfo;

#ifdef HAVE_LIBAIKSAURUS
	ThesaurusDialog     thesaurus;
#endif

	TocDialog           toc;
	UrlDialog           url;
	VCLogFileDialog     vclogfile;
};

#endif // DIALOGS_IMPL_H
