/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"

#include "GUI.h"
#include "xformsBC.h"

#include "combox.h"       // needed for clean destruction of boost::scoped_ptr
#include "form_aboutlyx.h"
#include "form_bibitem.h"
#include "form_bibtex.h"
#include "form_browser.h"
#include "form_character.h"
#include "form_citation.h"
#include "form_error.h"
#include "form_ert.h"
#include "form_external.h"
#include "form_float.h"
#include "form_forks.h"
#include "form_graphics.h"
#include "form_include.h"
#include "form_index.h"
#include "form_minipage.h"
#include "form_preamble.h"
#include "form_print.h"
#include "form_ref.h"
#include "form_search.h"
#include "form_spellchecker.h"
#include "form_tabular_create.h"
#include "form_texinfo.h"
#include "form_thesaurus.h"
#include "form_toc.h"
#include "form_url.h"

#include "FormAboutlyx.h"
#include "FormBibitem.h"
#include "FormBibtex.h"
#include "FormCharacter.h"
#include "FormCitation.h"
#include "FormError.h"
#include "FormERT.h"
#include "FormExternal.h"
#include "FormFloat.h"
#include "FormForks.h"
#include "FormGraphics.h"
#include "FormInclude.h"
#include "FormIndex.h"
#include "FormLog.h"
#include "FormMinipage.h"
#include "FormPreamble.h"
#include "FormPrint.h"
#include "FormRef.h"
#include "FormSearch.h"
#include "FormShowFile.h"
#include "FormSpellchecker.h"
#include "FormTabularCreate.h"
#include "FormTexinfo.h" 
#include "FormThesaurus.h" 
#include "FormToc.h"
#include "FormUrl.h"
#include "FormVCLog.h"

#include "FormDocument.h"
#include "FormMathsPanel.h"
#include "FormParagraph.h"
#include "FormPreferences.h"
#include "FormTabular.h"

#include "graphics/GraphicsImageXPM.h"
//#include "xformsGraphicsImage.h"

// Signal enabling all visible dialogs to be redrawn if so desired.
// E.g., when the GUI colours have been remapped.
SigC::Signal0<void> Dialogs::redrawGUI;

Dialogs::Dialogs(LyXView * lv)
{
	add(new GUIAboutlyx<FormAboutlyx, xformsBC>(*lv, *this));
	add(new GUIBibitem<FormBibitem, xformsBC>(*lv, *this));
	add(new GUIBibtex<FormBibtex, xformsBC>(*lv, *this));
	add(new GUICharacter<FormCharacter, xformsBC>(*lv, *this));
	add(new GUICitation<FormCitation, xformsBC>(*lv, *this));
	add(new GUIError<FormError, xformsBC>(*lv, *this));
	add(new GUIERT<FormERT, xformsBC>(*lv, *this));
	add(new GUIExternal<FormExternal, xformsBC>(*lv, *this));
	add(new GUIForks<FormForks, xformsBC>(*lv, *this));
	add(new GUIGraphics<FormGraphics, xformsBC>(*lv, *this));
	add(new GUIInclude<FormInclude, xformsBC>(*lv, *this));
	add(new GUIIndex<FormIndex, xformsBC>(*lv, *this));
	add(new GUILog<FormLog, xformsBC>(*lv, *this));
	add(new GUIMinipage<FormMinipage, xformsBC>(*lv, *this));
	add(new GUIFloat<FormFloat, xformsBC>(*lv, *this));
	add(new GUIPreamble<FormPreamble, xformsBC>(*lv, *this));
	add(new GUIPrint<FormPrint, xformsBC>(*lv, *this));
	add(new GUIRef<FormRef, xformsBC>(*lv, *this));
	add(new GUISearch<FormSearch, xformsBC>(*lv, *this));
	add(new GUIShowFile<FormShowFile, xformsBC>(*lv, *this));
	add(new GUISpellchecker<FormSpellchecker, xformsBC>(*lv, *this));
	add(new GUITabularCreate<FormTabularCreate, xformsBC>(*lv, *this));
#ifdef HAVE_LIBAIKSAURUS
	add(new GUIThesaurus<FormThesaurus, xformsBC>(*lv, *this));
#endif
	add(new GUITexinfo<FormTexinfo, xformsBC>(*lv, *this));
	add(new GUIToc<FormToc, xformsBC>(*lv, *this));
	add(new GUIUrl<FormUrl, xformsBC>(*lv, *this));
	add(new GUIVCLog<FormVCLog, xformsBC>(*lv, *this));

	add(new FormDocument(lv, this));
 	add(new FormMathsPanel(lv, this));
	add(new FormParagraph(lv, this));
	add(new FormPreferences(lv, this));
	add(new FormTabular(lv, this));
	
	// reduce the number of connections needed in
	// dialogs by a simple connection here.
	hideAll.connect(hideBufferDependent.slot());
}

// Called bu the graphics cache to connect the approriate frontend
// image loading routines to the LyX kernel.
void Dialogs::initialiseGraphics()
{
	using namespace grfx;
	using SigC::slot;
    
	// connect the image loader based on the XPM library
	GImage::newImage.connect(slot(&GImageXPM::newImage));
	GImage::loadableFormats.connect(slot(&GImageXPM::loadableFormats));
	// connect the image loader based on the xforms library
//	GImage::newImage.connect(slot(&xformsGImage::newImage));
//	GImage::loadableFormats.connect(slot(&xformsGImage::loadableFormats));
}
