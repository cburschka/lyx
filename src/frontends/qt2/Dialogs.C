/**
 * \file Dialogs.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

// the dialog definitions
#include "QAboutDialog.h"
#include "QBibitemDialog.h"
#include "QBibtexDialog.h"
#include "QCharacterDialog.h"
#include "QCitationDialog.h"
#include "QErrorDialog.h"
#include "QERTDialog.h"
#include "QExternalDialog.h"
#include "QGraphicsDialog.h"
#include "QIncludeDialog.h"
#include "QIndexDialog.h"
#include "QLogDialog.h"
#include "QMinipageDialog.h"
#include "QPreambleDialog.h"
#include "QPrintDialog.h"
#include "QRefDialog.h"
#include "QSearchDialog.h"
#include "QTabularCreateDialog.h"
#include "QThesaurusDialog.h"
#include "QURLDialog.h"
#include "QVCLogDialog.h"

#include "QAbout.h"
#include "QBibitem.h"
#include "QBibtex.h"
#include "QCharacter.h"
#include "QCitation.h"
#include "QError.h"
#include "QERT.h"
#include "QExternal.h"
#include "QGraphics.h"
#include "QInclude.h"
#include "QIndex.h"
#include "QLog.h"
#include "QMinipage.h"
#include "QParagraph.h"
#include "QPreamble.h"
#include "QPrint.h"
#include "QRef.h"
#include "QSearch.h"
#include "QSplash.h"
#include "QTabularCreate.h"
#include "QThesaurus.h"
#include "QURL.h"
#include "QVCLog.h"

#include "QtLyXView.h"
#include "Dialogs.h"
#include "BufferView.h"
#include "buffer.h"
#include "Qt2BC.h"

// the controllers
#include "controllers/ControlAboutlyx.h"
#include "controllers/ControlBibitem.h"
#include "controllers/ControlBibtex.h"
#include "controllers/ControlCharacter.h"
#include "controllers/ControlCitation.h"
#include "controllers/ControlError.h"
#include "controllers/ControlERT.h"
#include "controllers/ControlExternal.h"
#include "controllers/ControlGraphics.h"
#include "controllers/ControlInclude.h"
#include "controllers/ControlIndex.h"
#include "controllers/ControlLog.h"
#include "controllers/ControlMinipage.h"
#include "controllers/ControlPreamble.h"
#include "controllers/ControlPrint.h"
#include "controllers/ControlRef.h"
#include "controllers/ControlSearch.h"
#include "controllers/ControlSplash.h"
#include "controllers/ControlTabularCreate.h"
#include "controllers/ControlThesaurus.h"
#include "controllers/ControlUrl.h"
#include "controllers/ControlVCLog.h"
#if 0
#include "controllers/ControlButtons.h"
#include "controllers/ControlCitation.h"
#include "controllers/ControlFloat.h"
#include "controllers/ControlLabel.h"
#include "controllers/ControlRef.h"
#include "controllers/ControlSpellchecker.h"
#include "controllers/ControlToc.h"
#endif

#include "GUI.h"

// this makes no real sense for Qt2
SigC::Signal0<void> Dialogs::redrawGUI;


Dialogs::Dialogs(LyXView * lv)
{
	splash_.reset(new QSplash(lv, this));

	// dialogs that have been converted to new scheme
	add(new GUICitation<QCitation, Qt2BC>(*lv, *this));
	add(new GUIAboutlyx<QAbout, Qt2BC>(*lv, *this));
	add(new GUIBibitem<QBibitem, Qt2BC>(*lv, *this));
	add(new GUIBibtex<QBibtex, Qt2BC>(*lv, *this));
	add(new GUICharacter<QCharacter, Qt2BC>(*lv, *this));
	add(new GUIError<QError, Qt2BC>(*lv, *this));
	add(new GUIERT<QERT, Qt2BC>(*lv, *this));
	add(new GUIExternal<QExternal, Qt2BC>(*lv, *this));
	add(new GUIGraphics<QGraphics, Qt2BC>(*lv, *this));
	add(new GUIInclude<QInclude, Qt2BC>(*lv, *this));
	add(new GUIIndex<QIndex, Qt2BC>(*lv, *this));
	add(new GUILog<QLog, Qt2BC>(*lv, *this));
	add(new GUIMinipage<QMinipage, Qt2BC>(*lv, *this));
	add(new GUIPreamble<QPreamble, Qt2BC>(*lv, *this)); 
	add(new GUIPrint<QPrint, Qt2BC>(*lv, *this));
	add(new GUIRef<QRef, Qt2BC>(*lv, *this));
	add(new GUISearch<QSearch, Qt2BC>(*lv, *this)); 
	add(new GUITabularCreate<QTabularCreate, Qt2BC>(*lv, *this));
	add(new GUIThesaurus<QThesaurus, Qt2BC>(*lv, *this));
	add(new GUIUrl<QURL, Qt2BC>(*lv, *this));
	add(new GUIVCLog<QVCLog, Qt2BC>(*lv, *this));

	// reduce the number of connections needed in
	// dialogs by a simple connection here.
	hideAll.connect(hideBufferDependent.slot());
}
