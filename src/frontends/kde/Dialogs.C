/**
 * \file Dialogs.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Jürgen Vigna
 * \author John Levon
 */

// real GUI implementations. Have to go at the top
// because Qt is stupid.
#include "citationdlg.h" 
#include "copyrightdlg.h"
#include "indexdlg.h"
#include "logdlg.h"
#include "printdlg.h"
#include "refdlg.h"
#include "tabcreatedlg.h"
//#include "tocdlg.h"
#include "urldlg.h"
#include "vclogdlg.h" 
 
#include <config.h>
#include FORMS_H_LOCATION

#include "QtLyXView.h"
 
#include "combox.h"
#include "form_bibitem.h"
#include "form_bibtex.h"
#include "form_browser.h"
#include "form_character.h"
#include "form_credits.h"
#include "form_error.h"
#include "form_external.h" 
#include "form_graphics.h"
#include "form_include.h" 
#include "form_minipage.h"
#include "form_preamble.h"
#include "form_search.h"
#include "form_splash.h"
 
#include "Dialogs.h"
#include "kdeBC.h"
#include "GUI.h"
 
// controllers
#include "ControlBibitem.h"
#include "ControlBibtex.h"
#include "ControlCharacter.h"
#include "ControlCitation.h"
#include "ControlCopyright.h"
#include "ControlCredits.h"
#include "ControlError.h"
#include "ControlExternal.h" 
#include "ControlGraphics.h"
#include "ControlInclude.h"
#include "ControlIndex.h"
#include "ControlLog.h"
#include "ControlMinipage.h"
#include "ControlPreamble.h"
#include "ControlPrint.h"
#include "ControlRef.h"
#include "ControlSearch.h"
#include "ControlSplash.h"
#include "ControlTabularCreate.h"
#include "ControlUrl.h"
#include "ControlVCLog.h"

// dialogs
#include "FormBibitem.h"
#include "FormBibtex.h"
#include "FormCharacter.h" 
#include "FormCitation.h"
#include "FormCopyright.h"
#include "FormCredits.h"
#include "FormDocument.h"
#include "FormError.h"
#include "FormExternal.h" 
#include "FormGraphics.h"
#include "FormInclude.h"
#include "FormIndex.h"
#include "FormLog.h"
#include "FormMinipage.h" 
#include "FormMathsPanel.h" 
#include "FormParagraph.h"
#include "FormPreamble.h"
#include "FormPreferences.h"
#include "FormPrint.h"
#include "FormRef.h"
#include "FormSearch.h" 
#include "FormSplash.h"
#include "FormTabular.h"
#include "FormTabularCreate.h"
//#include "FormToc.h"
#include "FormUrl.h"
#include "FormVCLog.h"


#ifdef __GNUG__
#pragma implementation
#endif

/* We don't implement this, but it's needed whilst we
 * still rely on xforms
 */
SigC::Signal0<void> Dialogs::redrawGUI;

Dialogs::Dialogs(LyXView * lv)
{
	splash_.reset(new GUISplash<FormSplash>(*this));

	add(new GUIBibitem<FormBibitem, xformsBC>(*lv, *this));
	add(new GUIBibtex<FormBibtex, xformsBC>(*lv, *this));
	add(new GUICharacter<FormCharacter, xformsBC>(*lv, *this));
	add(new GUICitation<FormCitation, kdeBC>(*lv, *this));
	add(new GUICopyright<FormCopyright, kdeBC>(*lv, *this));
	add(new GUICredits<FormCredits, xformsBC>(*lv, *this));
	add(new GUIError<FormError, xformsBC>(*lv, *this));
	add(new GUIExternal<FormExternal, xformsBC>(*lv, *this));
	add(new GUIGraphics<FormGraphics, xformsBC>(*lv, *this));
	add(new GUIInclude<FormInclude, kdeBC>(*lv, *this));
	add(new GUIIndex<FormIndex, kdeBC>(*lv, *this));
	add(new GUILog<FormLog, kdeBC>(*lv, *this));
	add(new GUIMinipage<FormMinipage, xformsBC>(*lv, *this));
	add(new GUIPreamble<FormPreamble, xformsBC>(*lv, *this));
	add(new GUIPrint<FormPrint, kdeBC>(*lv, *this));
	add(new GUIRef<FormRef, kdeBC>(*lv, *this));
	add(new GUISearch<FormSearch, xformsBC>(*lv, *this));
	add(new GUITabularCreate<FormTabularCreate, kdeBC>(*lv, *this));
	add(new GUIUrl<FormUrl, kdeBC>(*lv, *this));
	add(new GUIVCLog<FormVCLog, kdeBC>(*lv, *this));
	//add(new GUIToc<FormToc, kdeBC>(*lv, *this));

	add(new FormDocument(lv, this));
 	add(new FormMathsPanel(lv, this));
	add(new FormParagraph(lv, this));
	add(new FormPreferences(lv, this));
	add(new FormTabular(lv, this));
	
	// reduce the number of connections needed in
	// dialogs by a simple connection here.
	hideAll.connect(hideBufferDependent.slot());
}
