/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
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
#include "gnomeBC.h"

#include "frontends/LyXView.h"

#include "GError.h"
#include "GERT.h"
#include "GPreamble.h"
#include "GTabularCreate.h"
#include "GUrl.h"


#include "Tooltips.h"

bool Dialogs::tooltipsEnabled()
{
	return Tooltips::enabled();
}
Dialogs::Dialogs(LyXView * lv)
{

	add(new GUI<ControlError,GErrorDialog, 
	    OkCancelPolicy, gnomeBC>(*lv, *this));
	add(new GUI<ControlERT, GERT,
	    NoRepeatedApplyReadOnlyPolicy, gnomeBC>(*lv, *this));
	add(new GUI<ControlUrl, GUrl,
	    NoRepeatedApplyReadOnlyPolicy, gnomeBC>(*lv,*this));
	add(new GUI<ControlPreamble, GPreamble,
	    NoRepeatedApplyReadOnlyPolicy, gnomeBC>(*lv, *this));
	add(new GUI<ControlTabularCreate, GTabularCreate,
	    OkApplyCancelReadOnlyPolicy, gnomeBC>(*lv, *this));

	// reduce the number of connections needed in
	// dialogs by a simple connection here.
	hideAll.connect(hideBufferDependent);
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

       LyXFuncs will be used for requesting/setting LyX internal info.  This
       will ensure that scripts or LyXServer-connected applications can all
       have access to the same calls as the internal user-interface.

******************************************************************************/
