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
#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"

#include "ControlBibitem.h"
#include "ControlCitation.h"
#include "xformsBC.h"

#include "FormBibitem.h"
#include "FormBibtex.h"
#include "FormCitation.h"
#include "FormCharacter.h"
#include "FormCopyright.h"
#include "FormCredits.h"
#include "FormDocument.h"
#include "FormError.h"
#include "FormExternal.h" 
#include "FormGraphics.h"
#include "FormInclude.h"
#include "FormIndex.h"
#include "FormLog.h"
#include "FormMaths.h"
#include "FormParagraph.h"
#include "FormPreamble.h"
#include "FormPreferences.h"
#include "FormPrint.h"
#include "FormRef.h"
#include "FormSearch.h"
#include "FormSplash.h"
#include "FormTabular.h"
#include "FormTabularCreate.h"
#include "FormToc.h"
#include "FormUrl.h"
#include "FormVCLog.h"
#include "FormMinipage.h"

// Signal enabling all visible popups to be redrawn if so desired.
// E.g., when the GUI colours have been remapped.
SigC::Signal0<void> Dialogs::redrawGUI;

Dialogs::Dialogs(LyXView * lv)
{
	splash_.reset(new FormSplash(lv, this));

	add(new GUICitation<FormCitation, xformsBC>(*lv, *this));
	add(new GUIBibitem<FormBibitem, xformsBC>(*lv, *this));

	add(new FormBibtex(lv, this));
	add(new FormCharacter(lv, this));
	add(new FormCopyright(lv, this));
	add(new FormCredits(lv, this));
	add(new FormDocument(lv, this));
	add(new FormError(lv, this));
	add(new FormExternal(lv, this));
	add(new FormGraphics(lv, this));
	add(new FormInclude(lv, this));
	add(new FormIndex(lv, this));
	add(new FormLog(lv, this));
 	add(new FormMaths(lv, this));
	add(new FormParagraph(lv, this));
	add(new FormPreamble(lv, this));
	add(new FormPreferences(lv, this));
	add(new FormPrint(lv, this));
	add(new FormRef(lv, this));
	add(new FormSearch(lv, this));
	add(new FormSplash(lv, this));
	add(new FormTabular(lv, this));
	add(new FormTabularCreate(lv, this));
	add(new FormToc(lv, this));
	add(new FormUrl(lv, this));
	add(new FormVCLog(lv, this));
	add(new FormMinipage(lv, this));
	
	// reduce the number of connections needed in
	// dialogs by a simple connection here.
	hideAll.connect(hideBufferDependent.slot());
}

/*****************************************************************************

Q.  WHY does Dialogs::Dialogs pass `this' to dialog constructors?

A.  To avoid a segfault.
    The dialog constructors need to connect to their
    respective showSomeDialog signal(*) but in order to do
    that they need to get the address of the Dialogs instance
    from LyXView::getDialogs().  However, since the Dialogs
    instance is still being constructed at that time
    LyXView::getDialogs() will *not* return the correct
    address because it hasn't finished being constructed.
    A Catch-22 situation (or is that the chicken and the egg...).
    So to get around the problem we pass the address of
    the newly created Dialogs instance using `this'.

(*) -- I'm using signals exclusively to guarantee that the gui code
       remains hidden from the rest of the system.  In fact the only 
       header related to dialogs that anything in the non-gui-specific
       code gets to see is Dialogs.h!  Even Dialogs.h doesn't know what a 
       FormCopyright class looks like or that its even going to be used!

       No other gui dialog headers are seen outside of the gui-specific
       directories!  This ensures that the gui is completely separate from
       the rest of LyX.  All this through the use of a few simple signals.
       BUT, the price is that during construction we need to connect the
       implementations show() method to the showSomeDialog signal and this
       requires that we have an instance of Dialogs and the problem mentioned
       above.

       Almost all other dialogs should be able to operate using the same style
       of signalling used for Copyright.  Exceptions should be handled
       by adding a specific show or update signal.  For example, spellchecker
       needs to set the next suspect word and its options/replacements so we
       need a:
                 Signal0<void> updateSpellChecker;

       Since we would have to have a
                 Signal0<void> showSpellChecker;

       in order to just see the spellchecker and let the user push the [Start]
       button then the updateSpellChecker signal will make the SpellChecker
       dialog get the new word and replacements list from LyX.  If you really,
       really wanted to you could define a signal that would pass the new
       word and replacements:
                 Signal2<void, string, vector<string> > updateSpellChecker;

       (or something similar) but, why bother when the spellchecker can get
       it anyway with a LyXFunc call or two.  Besides if someone extends
       what a dialog does then they also have to change code in the rest of 
       LyX to pass more parameters or get the extra info via a function 
       call anyway.  Thus reducing the independence of the two code bases.

       We don't need a separate update signal for each dialog because most of 
       them will be changed only when the buffer is changed (either by closing
       the current open buffer or switching to another buffer in the current
       LyXView -- different BufferView same LyXView or same BufferView same
       LyXView).

       So we minimise signals but maximise independence and programming 
       simplicity, understandability and maintainability.  It's also
       extremely easy to add support for Qt or gtk-- because they use
       signals already. Guis that use callbacks, like xforms, must have their
       code wrapped up like that in the form_copyright.[Ch] which is awkward
       but will at least allow multiple instances of the same dialog.

       Signals will also be a great help in controlling the splashscreen --
       once signalled to hide it can disconnect from the signal and remove
       itself from memory.

       LyXFuncs will be used for requesting/setting LyX internal info.  This
       will ensure that scripts or LyXServer-connected applications can all
       have access to the same calls as the internal user-interface.

******************************************************************************/
