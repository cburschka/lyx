/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"

#include "FormCharacter.h"
#include "FormCitation.h"
#include "FormCredits.h"
//#include "FormCopyright.h"
//#include "FormDocument.h"
#include "FormError.h"
#include "FormGraphics.h"
//#include "FormIndex.h"
#include "FormParagraph.h"
#include "FormPreferences.h"
#include "FormPrint.h"
//#include "FormRef.h"
#include "FormSearch.h"
#include "FormSplash.h"
#include "FormTabular.h"
#include "FormTabularCreate.h"
//#include "FormToc.h"
//#include "FormUrl.h"


// Signal enabling all visible popups to be redrawn if so desired.
// E.g., when the GUI colours have been remapped.
SigC::Signal0<void> Dialogs::redrawGUI;


Dialogs::Dialogs(LyXView * lv)
{
    splash_.reset( new FormSplash(lv, this) );
    add( new FormCharacter(lv, this));
    add( new FormCitation(lv, this));
    //	add(new FormCopyright(lv, this));

    // REMOVED THIS UNTIL CHANGED TO NEW SCHEME -- Kalle, 2001-03-22
    //    add( new FormCredits(lv, this));

    //	add(new FormDocument(lv, this));
    add(new FormError(lv, this));
    add(new FormGraphics(lv, this));
    //	add(new FormIndex(lv, this));
    add(new FormParagraph(lv, this));
    add(new FormPreferences(lv, this));
    add(new FormPrint(lv, this));
    //	add(new FormRef(lv, this));
    add(new FormSearch(lv, this));
    add(new FormTabular(lv, this));
    add(new FormTabularCreate(lv, this));
    //	add(new FormToc(lv, this));
    //	add(new FormUrl(lv, this));
    
    // reduce the number of connections needed in
    // dialogs by a simple connection here.
    hideAll.connect(hideBufferDependent.slot());
}


