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

#include "FormCitationDialogImpl.h"
#include "FormCopyrightDialogImpl.h"
#undef emit

#include "../xforms/FormCredits.h"
#include "../xforms/FormError.h"
#include "../xforms/FormGraphics.h"
#include "../xforms/FormPreferences.h"
#include "../xforms/FormTabular.h"

#include "FormCharacter.h"
#include "FormCitation.h"
#include "FormCopyright.h"
//#include "FormDocument.h"
//#include "FormIndex.h"
#include "FormParagraph.h"
#include "FormPrint.h"
//#include "FormRef.h"
#include "FormSearch.h"
#include "FormSplash.h"
#include "FormTabularCreate.h"
//#include "FormToc.h"
//#include "FormUrl.h"

#include "BufferView.h"
#include "buffer.h"

#include "controllers/ControlCitation.h"
#include "controllers/ControlCopyright.h"

#include "GUI.h"

#include "qt2BC.h"

// Signal enabling all visible popups to be redrawn if so desired.
// E.g., when the GUI colours have been remapped.
SigC::Signal0<void> Dialogs::redrawGUI;


Dialogs::Dialogs(LyXView * lv)
{
    splash_.reset( new FormSplash(lv, this) );

    // dialogs that have been converted to new scheme
    add( new GUICitation<FormCitation, qt2BC>( *lv, *this ) );
    add( new GUICopyright<FormCopyright, qt2BC>( *lv, *this ) );

    // ------------------------------------------

    // dialogs that are still old-style
    add( new FormCharacter(lv, this));

    // REMOVED THIS UNTIL CHANGED TO NEW SCHEME -- Kalle, 2001-03-22
    //    add( new FormCredits(lv, this));

    //	add(new FormDocument(lv, this));


    // REMOVED THIS UNTIL CHANGED TO NEW SCHEME -- Kalle, 2001-03-23
    // add(new FormError(lv, this));

    // REMOVED THIS UNTIL CHANGED TO NEW SCHEME -- Kalle, 2001-03-28
    //    add(new FormGraphics(lv, this));
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


