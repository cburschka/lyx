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

#include "xformsBC.h"

#include "ControlAboutlyx.h"
#include "ControlBibitem.h"
#include "ControlBibtex.h"
#include "ControlCharacter.h"
#include "ControlCitation.h"
#include "ControlError.h"
#include "ControlExternal.h" 
#include "ControlGraphics.h"
#include "insets/insetgraphicsParams.h" 
#include "ControlInclude.h"
#include "ControlIndex.h"
#include "ControlLog.h"
#include "ControlMinipage.h"
#include "ControlPreamble.h"
#include "ControlPrint.h"
#include "ControlRef.h"
#include "ControlSearch.h"
#include "ControlSpellchecker.h"
#include "ControlSplash.h"
#include "ControlTabularCreate.h"
#include "ControlThesaurus.h"
#include "ControlToc.h"
#include "ControlUrl.h"
#include "ControlVCLog.h"

#include "GUI.h"

#include "combox.h"       // needed for clean destruction of boost::scoped_ptr
#include "form_aboutlyx.h"
#include "form_bibitem.h"
#include "form_bibtex.h"
#include "form_browser.h"
#include "form_character.h"
#include "form_citation.h"
#include "form_error.h"
#include "form_external.h" 
#include "form_graphics.h"
#include "form_include.h"
#include "form_index.h"
#include "form_minipage.h"
#include "form_preamble.h"
#include "form_print.h"
#include "form_ref.h"
#include "form_search.h"
#include "form_spellchecker.h"
#include "form_splash.h"
#include "form_tabular_create.h"
#include "form_thesaurus.h"
#include "form_toc.h"
#include "form_url.h"

#include "FormAboutlyx.h"
#include "FormBibitem.h"
#include "FormBibtex.h"
#include "FormCharacter.h"
#include "FormCitation.h"
#include "FormError.h"
#include "FormExternal.h" 
#include "FormGraphics.h"
#include "FormInclude.h"
#include "FormIndex.h"
#include "FormLog.h"
#include "FormMinipage.h"
#include "FormPreamble.h"
#include "FormPrint.h"
#include "FormRef.h"
#include "FormSearch.h"
#include "FormSpellchecker.h"
#include "FormSplash.h"
#include "FormTabularCreate.h"
#include "FormThesaurus.h" 
#include "FormToc.h"
#include "FormUrl.h"
#include "FormVCLog.h"

#include "FormDocument.h"
#include "FormMathsPanel.h"
#include "FormParagraph.h"
#include "FormPreferences.h"
#include "FormTabular.h"

// Signal enabling all visible dialogs to be redrawn if so desired.
// E.g., when the GUI colours have been remapped.
SigC::Signal0<void> Dialogs::redrawGUI;

Dialogs::Dialogs(LyXView * lv)
{
	splash_.reset(new GUISplash<FormSplash>(*this));

	add(new GUIAboutlyx<FormAboutlyx, xformsBC>(*lv, *this));
	add(new GUIBibitem<FormBibitem, xformsBC>(*lv, *this));
	add(new GUIBibtex<FormBibtex, xformsBC>(*lv, *this));
	add(new GUICharacter<FormCharacter, xformsBC>(*lv, *this));
	add(new GUICitation<FormCitation, xformsBC>(*lv, *this));
	add(new GUIError<FormError, xformsBC>(*lv, *this));
	add(new GUIExternal<FormExternal, xformsBC>(*lv, *this));
	add(new GUIGraphics<FormGraphics, xformsBC>(*lv, *this));
	add(new GUIInclude<FormInclude, xformsBC>(*lv, *this));
	add(new GUIIndex<FormIndex, xformsBC>(*lv, *this));
	add(new GUILog<FormLog, xformsBC>(*lv, *this));
	add(new GUIMinipage<FormMinipage, xformsBC>(*lv, *this));
	add(new GUIPreamble<FormPreamble, xformsBC>(*lv, *this));
	add(new GUIPrint<FormPrint, xformsBC>(*lv, *this));
	add(new GUIRef<FormRef, xformsBC>(*lv, *this));
	add(new GUISearch<FormSearch, xformsBC>(*lv, *this));
	add(new GUISpellchecker<FormSpellchecker, xformsBC>(*lv, *this));
	add(new GUITabularCreate<FormTabularCreate, xformsBC>(*lv, *this));
#ifdef HAVE_LIBAIKSAURUS
	add(new GUIThesaurus<FormThesaurus, xformsBC>(*lv, *this));
#endif
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
