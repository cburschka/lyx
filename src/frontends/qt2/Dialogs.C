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
#include "QSpellcheckerDialog.h"
#include "QTabularCreateDialog.h"
#include "QTexinfoDialog.h"
#include "QThesaurusDialog.h"
#include "QTocDialog.h"
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
#include "QSpellchecker.h"
#include "QTabularCreate.h"
#include "QTexinfo.h"
#include "QToc.h"
#include "QThesaurus.h"
#include "QURL.h"
#include "QVCLog.h"

#include "QtLyXView.h"
#include "Dialogs.h"
#include "BufferView.h"
#include "buffer.h"
#include "Qt2BC.h"

// xforms stuff
#include "xforms/FormBrowser.h"
#include "xforms/form_browser.h"
#include "xforms/FormDocument.h"
#include "xforms/FormFloat.h"
#include "xforms/form_float.h"
#include "xforms/FormMathsPanel.h"
#include "xforms/FormParagraph.h"
#include "xforms/FormPreferences.h"
#include "xforms/FormShowFile.h"
#include "xforms/FormTabular.h"
 
#include "GUI.h"

// this makes no real sense for Qt2
SigC::Signal0<void> Dialogs::redrawGUI;


Dialogs::Dialogs(LyXView * lv)
{
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
	add(new GUISpellchecker<QSpellchecker, Qt2BC>(*lv, *this));
	add(new GUITabularCreate<QTabularCreate, Qt2BC>(*lv, *this));
	add(new GUITexinfo<QTexinfo, Qt2BC>(*lv, *this));
	add(new GUIThesaurus<QThesaurus, Qt2BC>(*lv, *this));
	add(new GUIToc<QToc, Qt2BC>(*lv, *this));
	add(new GUIUrl<QURL, Qt2BC>(*lv, *this));
	add(new GUIVCLog<QVCLog, Qt2BC>(*lv, *this));

	// dialogs not yet converted
	add(new GUIFloat<FormFloat, xformsBC>(*lv, *this));
	add(new GUIShowFile<FormShowFile, xformsBC>(*lv, *this));

	// dialogs not yet MVCd
	add(new FormDocument(lv, this));
 	add(new FormMathsPanel(lv, this));
	add(new FormParagraph(lv, this));
	add(new FormPreferences(lv, this));
	add(new FormTabular(lv, this));
 
	// reduce the number of connections needed in
	// dialogs by a simple connection here.
	hideAll.connect(hideBufferDependent.slot());
}
