/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include "Lsstream.h"

#include <time.h>
#include <locale.h>
#include <utility> 
#include <algorithm> 

#include <cstdlib>
#include <cctype>
//#include <cstring>

#ifdef __GNUG__
#pragma implementation
#endif

#include "support/lyxalgo.h"
#include "version.h"
#include "kbmap.h"
#include "lyxfunc.h"
#include "bufferlist.h"
#include "ColorHandler.h"
#include "lyxserver.h"
#include "figure_form.h"
#include "intl.h"
#include "lyx_main.h"
#include "lyx_cb.h"
#include "LyXAction.h"
#include "insets/inseturl.h"
#include "insets/insetlatexaccent.h"
#include "insets/insettoc.h"
#include "insets/insetref.h"
#include "insets/insetparent.h"
#include "insets/insetindex.h"
#include "insets/insetinclude.h"
#include "insets/insetbib.h"
#include "insets/insetcite.h"
#include "insets/insettext.h"
#include "insets/insetert.h"
#include "insets/insetexternal.h"
#include "insets/insetgraphics.h"
#include "insets/insetfoot.h"
#include "insets/insetmarginal.h"
#include "insets/insetminipage.h"
#include "insets/insetfloat.h"
#include "insets/insetlist.h"
#include "insets/insettabular.h"
#include "insets/insettheorem.h"
#include "insets/insetcaption.h"
#include "mathed/formulamacro.h"
#include "spellchecker.h" // RVDK_PATCH_5
#include "minibuffer.h"
#include "vspace.h"
#include "LyXView.h"
#include "lyx_gui_misc.h"
#include "support/filetools.h"
#include "support/FileInfo.h"
#include "support/syscall.h"
#include "support/lstrings.h"
#include "support/path.h"
#include "support/lyxfunctional.h"
#include "debug.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "gettext.h"
#include "trans_mgr.h"
#include "layout.h"
#include "WorkArea.h"
#include "bufferview_funcs.h"
#include "frontends/FileDialog.h"
#include "frontends/Dialogs.h"
#include "frontends/Toolbar.h"
#include "frontends/Menubar.h"
#include "FloatList.h"
#include "converter.h"
#include "exporter.h"
#include "importer.h"
#include "FontLoader.h"
#include "TextCache.h"
#include "lyxfind.h"

using std::pair;
using std::make_pair; 
using std::endl;
using std::find_if;

extern BufferList bufferlist;
extern LyXServer * lyxserver;
extern int greek_kb_flag;
extern bool selection_possible;
extern void MenuSendto();

extern boost::scoped_ptr<kb_keymap> toplevel_keymap;

extern void show_symbols_form(LyXFunc *);

extern LyXAction lyxaction;
// (alkis)
extern tex_accent_struct get_accent(kb_action action);

extern LyXTextClass::size_type current_layout;

extern void ShowLatexLog();


#if 0

///
class MiniBufferController : public SigC::Object {
public:
	///
	MiniBufferController() {
		minibuffer
			.cmdReady
			.connect(slot(this,
				      &MiniBufferController::receiveCommand));
		minibuffer
			.argReady
			.connect(slot(this,
				      &MiniBufferController::receiveArg));
	}
	///
	void receiveCmd(string const & cmd) {}
	///
	void receiveArg(string const & arg) {}
	
	
private:
};

namespace {

MiniBufferController mb_ctrl;

}
#endif


/* === globals =========================================================== */

// Initialization of static member var
bool LyXFunc::show_sc = true;


LyXFunc::LyXFunc(LyXView * o)
	: owner(o)
{
	meta_fake_bit = 0;
	lyx_dead_action = LFUN_NOACTION;
	lyx_calling_dead_action = LFUN_NOACTION;
	setupLocalKeymap();
}


inline
LyXText * LyXFunc::TEXT(bool flag = true) const
{
	if (flag)
		return owner->view()->text;
	return owner->view()->getLyXText();
}


// I changed this func slightly. I commented out the ...FinishUndo(),
// this means that all places that used to have a moveCursorUpdate, now
// have a ...FinishUndo() as the preceeding statement. I have also added
// a moveCursorUpdate to some of the functions that updated the cursor, but
// that did not show its new position.
inline
void LyXFunc::moveCursorUpdate(bool flag, bool selecting)
{
	if (selecting || TEXT(flag)->selection.mark()) {
		TEXT(flag)->SetSelection(owner->view());
		if (TEXT(flag)->bv_owner)
		    owner->view()->toggleToggle();
	}
	owner->view()->update(TEXT(flag), BufferView::SELECT|BufferView::FITCUR);
	owner->view()->showCursor();
	
	/* ---> Everytime the cursor is moved, show the current font state. */
	// should this too me moved out of this func?
	//owner->showState();
	owner->view()->setState();
}


void LyXFunc::handleKeyFunc(kb_action action)
{
	char c = keyseq.getiso();

	if (keyseq.length != -1) c = 0;
	
	owner->getIntl()->getTrans()
		.deadkey(c, get_accent(action).accent, TEXT(false));
	// Need to reset, in case the minibuffer calls these
	// actions
	keyseq.reset();
	keyseq.length = 0;
	// copied verbatim from do_accent_char
	owner->view()->update(TEXT(false),
	       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
	TEXT(false)->selection.cursor = TEXT(false)->cursor;
}


void LyXFunc::processKeySym(KeySym keysym, unsigned int state) 
{
	string argument;
	
	if (lyxerr.debugging(Debug::KEY)) {
		char const * tmp = XKeysymToString(keysym);
		string const stm = (tmp ? tmp : "");
		lyxerr << "KeySym is "
		       << stm
		       << "["
		       << keysym << "] State is ["
		       << state << "]"
		       << endl;
	}
	// Do nothing if we have nothing (JMarc)
	if (keysym == NoSymbol) {
		lyxerr[Debug::KEY] << "Empty kbd action (probably composing)"
				   << endl;
		//return 0;
		//return FL_PREEMPT;
		return;
	}

	// Can we be sure that this will work for all X-Windows
	// implementations? (Lgb)
	// This code snippet makes lyx ignore some keys. Perhaps
	// all of them should be explictly mentioned?
	if ((keysym >= XK_Shift_L && keysym <= XK_Hyper_R)
	    || keysym == XK_Mode_switch || keysym == 0x0) {
		//return 0;
		return;
	}
	
	// Do a one-deep top-level lookup for
	// cancel and meta-fake keys. RVDK_PATCH_5
	cancel_meta_seq.reset();

	int action = cancel_meta_seq.addkey(keysym, state
					    &(ShiftMask|ControlMask
					      |Mod1Mask)); 
	if (lyxerr.debugging(Debug::KEY)) {
		lyxerr << "action first set to [" << action << "]" << endl;
	}
	
	// When not cancel or meta-fake, do the normal lookup. 
	// Note how the meta_fake Mod1 bit is OR-ed in and reset afterwards.
	// Mostly, meta_fake_bit = 0. RVDK_PATCH_5.
	if ((action != LFUN_CANCEL) && (action != LFUN_META_FAKE)) {
		if (lyxerr.debugging(Debug::KEY)) {
			lyxerr << "meta_fake_bit is ["
			       << meta_fake_bit << "]" << endl;
		}
		// remove Caps Lock and Mod2 as a modifiers
		action = keyseq.addkey(keysym,
				       (state | meta_fake_bit)
				       &(ShiftMask|ControlMask
					 |Mod1Mask));
		if (lyxerr.debugging(Debug::KEY)) {
			lyxerr << "action now set to ["
			       << action << "]" << endl;
		}
	}
	// Dont remove this unless you know what you are doing.
	meta_fake_bit = 0;
		
	if (action == 0) action = LFUN_PREFIX;

	if (lyxerr.debugging(Debug::KEY)) {
		string buf;
		keyseq.print(buf);
		lyxerr << "Key [action="
		       << action << "]["
		       << buf << "]"
		       << endl;
	}

	// already here we know if it any point in going further
	// why not return already here if action == -1 and
	// num_bytes == 0? (Lgb)

	if (keyseq.length > 1 || keyseq.length < -1) {
		string buf;
		keyseq.print(buf);
		owner->message(buf);
	}

	if (action == -1) {
		// It is unknown, but what if we remove all
		// the modifiers? (Lgb)
		action = keyseq.addkey(keysym, 0);

		if (lyxerr.debugging(Debug::KEY)) {
			lyxerr << "Removing modifiers...\n"
			       << "Action now set to ["
			       << action << "]" << endl;
		}
		if (action == -1) {
			owner->message(_("Unknown function."));
			//return 0;
			return;
		}
	}

	if (action == LFUN_SELFINSERT) {
		// This is very X dependant.
		unsigned int c = keysym;
		
		switch (c & 0x0000FF00) {
			// latin 1 byte 3 = 0
		case 0x00000000: break;
			// latin 2 byte 3 = 1
		case 0x00000100:
			// latin 3 byte 3 = 2
		case 0x00000200:
			// latin 4 byte 3 = 3
		case 0x00000300:
			// latin 8 byte 3 = 18 (0x12)
		case 0x00001200:
			// latin 9 byte 3 = 19 (0x13)
		case 0x00001300:
			c &= 0x000000FF;
			break;
		default:
			c = 0;
			break;
		}
		if (c > 0)
			argument = static_cast<char>(c);
		lyxerr[Debug::KEY] << "SelfInsert arg[`"
				   << argument << "']" << endl;
	}
	
        bool tmp_sc = show_sc;
	show_sc = false;
	Dispatch(action, argument);
	show_sc = tmp_sc;
	
	//return 0;
} 


LyXFunc::func_status LyXFunc::getStatus(int ac) const
{
	return getStatus(ac, string());
}


LyXFunc::func_status LyXFunc::getStatus(int ac,
					string const & not_to_use_arg) const
{
        kb_action action;
        func_status flag = LyXFunc::OK;
        string argument;
        Buffer * buf = owner->buffer();
	
 	if (lyxaction.isPseudoAction(ac)) 
		action = lyxaction.retrieveActionArg(ac, argument);
	else {
		action = static_cast<kb_action>(ac);
		if (!not_to_use_arg.empty())
			argument = not_to_use_arg; // exept here
	}
	
	if (action == LFUN_UNKNOWN_ACTION) {
		setErrorMessage(N_("Unknown action"));
		return LyXFunc::Unknown;
	}
	
	// Check whether we need a buffer
	if (!lyxaction.funcHasFlag(action, LyXAction::NoBuffer)) {
		// Yes we need a buffer, do we have one?
		if (buf) {
			// yes
			// Can we use a readonly buffer?
			if (buf->isReadonly() && 
			    !lyxaction.funcHasFlag(action,
						   LyXAction::ReadOnly)) {
				// no
				setErrorMessage(N_("Document is read-only"));
				flag |= LyXFunc::Disabled;
			}
		} else {
			// no
			setErrorMessage(N_("Command not allowed with"
					   "out any document open"));
			flag |= LyXFunc::Disabled;
			return flag;
		}
	}

	// I would really like to avoid having this switch and rather try to
	// encode this in the function itself.
        bool disable = false;
        switch (action) {
	case LFUN_MENUPRINT:
		disable = !Exporter::IsExportable(buf, "dvi")
			|| lyxrc.print_command == "none";
		break;
	case LFUN_EXPORT:
		disable = argument == "fax" &&
			!Exporter::IsExportable(buf, argument);
		break;
	case LFUN_UNDO:
		disable = buf->undostack.empty();
		break;
	case LFUN_REDO:
		disable = buf->redostack.empty();
		break;
	case LFUN_SPELLCHECK:
		disable = lyxrc.isp_command == "none";
		break;
	case LFUN_RUNCHKTEX:
		disable = lyxrc.chktex_command == "none";
		break;
	case LFUN_BUILDPROG:
		disable = !Exporter::IsExportable(buf, "program");
		break;

	case LFUN_INSERTFOOTNOTE:
		// Disable insertion of floats in a tabular.
		disable = false;
		if (owner->view()->theLockingInset()) {
			disable = (owner->view()->theLockingInset()->LyxCode() == Inset::TABULAR_CODE) ||
				owner->view()->theLockingInset()->GetFirstLockingInsetOfType(Inset::TABULAR_CODE);
		}
		break;

	case LFUN_LAYOUT_TABULAR:
		disable = true;
		if (owner->view()->theLockingInset()) {
			disable = (owner->view()->theLockingInset()->LyxCode() != Inset::TABULAR_CODE) &&
				!owner->view()->theLockingInset()->GetFirstLockingInsetOfType(Inset::TABULAR_CODE);
		}
		break;

	case LFUN_TABULAR_FEATURE:
		disable = true;
		if (owner->view()->theLockingInset()) {
			func_status ret = LyXFunc::Disabled;
			if (owner->view()->theLockingInset()->LyxCode() == Inset::TABULAR_CODE) {
				ret = static_cast<InsetTabular *>
					(owner->view()->theLockingInset())->
					getStatus(argument);
			} else if (owner->view()->theLockingInset()->GetFirstLockingInsetOfType(Inset::TABULAR_CODE)) {
				ret = static_cast<InsetTabular *>
					(owner->view()->theLockingInset()->
					GetFirstLockingInsetOfType(Inset::TABULAR_CODE))->
					getStatus(argument);
			}
			flag |= ret;
			disable = false;
		} else {
		    static InsetTabular inset(*owner->buffer(), 1, 1);
		    func_status ret;

		    disable = true;
		    ret = inset.getStatus(argument);
		    if ((ret & LyXFunc::ToggleOn) ||
			(ret & LyXFunc::ToggleOff))
			flag |= LyXFunc::ToggleOff;
		}
		break;

	case LFUN_VC_REGISTER:
		disable = buf->lyxvc.inUse();
		break;
	case LFUN_VC_CHECKIN:
		disable = !buf->lyxvc.inUse() || buf->isReadonly();
		break;
	case LFUN_VC_CHECKOUT:
		disable = !buf->lyxvc.inUse() || !buf->isReadonly();
		break;
	case LFUN_VC_REVERT:
	case LFUN_VC_UNDO:
	case LFUN_VC_HISTORY:
		disable = !buf->lyxvc.inUse();
		break;
	case LFUN_BOOKMARK_GOTO:
		disable =  !owner->view()->
			isSavedPosition(strToUnsignedInt(argument));
	default:
		break;
        }
        if (disable)
                flag |= LyXFunc::Disabled;

	if (buf) {
		func_status box = LyXFunc::ToggleOff;
		LyXFont const & font =
			TEXT(false)->real_current_font;
		switch (action) {
		case LFUN_EMPH:
			if (font.emph() == LyXFont::ON)
				box = LyXFunc::ToggleOn;
			break;
		case LFUN_NOUN:
			if (font.noun() == LyXFont::ON)
				box = LyXFunc::ToggleOn;
			break;
		case LFUN_BOLD:
			if (font.series() == LyXFont::BOLD_SERIES)
				box = LyXFunc::ToggleOn;
			break;
		case LFUN_TEX:
			if (font.latex() == LyXFont::ON)
				box = LyXFunc::ToggleOn;
			break;
		default:
			box = LyXFunc::OK;
			break;
		}
		flag |= box;
	}

	return flag;
}


// temporary dispatch method
void LyXFunc::miniDispatch(string const & s) 
{
	Dispatch(s);
}


string const LyXFunc::Dispatch(string const & s) 
{
	// Split command string into command and argument
	string cmd;
	string line = frontStrip(s);
	string arg = strip(frontStrip(split(line, cmd, ' ')));

	return Dispatch(lyxaction.LookupFunc(cmd), arg);
}


string const LyXFunc::Dispatch(int ac,
			       string const & do_not_use_this_arg)
{
	lyxerr[Debug::ACTION] << "LyXFunc::Dispatch: action[" << ac
			      <<"] arg[" << do_not_use_this_arg << "]" << endl;
	
	string argument;
	kb_action action;
        
        // we have not done anything wrong yet.
        errorstat = false;
	dispatch_buffer.erase();
	
	// if action is a pseudo-action, we need the real action
	if (lyxaction.isPseudoAction(ac)) {
		string tmparg;
		action = static_cast<kb_action>
			(lyxaction.retrieveActionArg(ac, tmparg));
		if (!tmparg.empty())
			argument = tmparg;
	} else {
		action = static_cast<kb_action>(ac);
		if (!do_not_use_this_arg.empty())
			argument = do_not_use_this_arg; // except here
	}
    
	selection_possible = false;
	
	if (owner->view()->available())
		owner->view()->hideCursor();

	// We cannot use this function here
	if (getStatus(ac, do_not_use_this_arg) & Disabled)
		goto exit_with_message;

	commandshortcut.erase();
	
	if (lyxrc.display_shortcuts && show_sc) {
		if (action != LFUN_SELFINSERT) {
			// Put name of command and list of shortcuts
			// for it in minibuffer
			string comname = lyxaction.getActionName(action);

			int pseudoaction = action;
			bool argsadded = false;

			if (!argument.empty()) {
				// If we have the command with argument, 
				// this is better
				pseudoaction = 
					lyxaction.searchActionArg(action,
							  	  argument);

				if (pseudoaction == -1) {
					pseudoaction = action;
				} else {
					comname += " " + argument;
					argsadded = true;
				}
			}

			string const shortcuts =
				toplevel_keymap->findbinding(pseudoaction);

			if (!shortcuts.empty()) {
				comname += ": " + shortcuts;
			} else if (!argsadded) {
				comname += " " + argument;
			}

			if (!comname.empty()) {
				comname = strip(comname);
				commandshortcut = "(" + comname + ')';
				owner->message(commandshortcut);

				// Here we could even add a small pause,
				// to annoy the user and make him learn
				// the shortcuts.
				// No! That will just annoy, not teach
				// anything. The user will read the messages
				// if they are interested. (Asger)
			}
		}
        }

	if (owner->view()->available() && owner->view()->theLockingInset()) {
		UpdatableInset::RESULT result;
		if ((action > 1) || ((action == LFUN_UNKNOWN_ACTION) &&
				     (keyseq.length >= -1)))
		{
			if ((action==LFUN_UNKNOWN_ACTION) && argument.empty()){
				argument = keyseq.getiso();
			}
			// Undo/Redo pre 0.13 is a bit tricky for insets.
		        if (action == LFUN_UNDO) {
				int slx;
				int sly;
				UpdatableInset * inset = 
					owner->view()->theLockingInset();
				inset->GetCursorPos(owner->view(), slx, sly);
				owner->view()->unlockInset(inset);
				owner->view()->menuUndo();
				if (TEXT()->cursor.par()->
				    IsInset(TEXT()->cursor.pos())) {
					inset = static_cast<UpdatableInset*>(
						TEXT()->cursor.par()->
						GetInset(TEXT()->
							 cursor.pos()));
				} else {
					inset = 0;
				}
				if (inset)
					inset->Edit(owner->view(),slx,sly,0);
				return string();
			} else if (action == LFUN_REDO) {
				int slx;
				int sly;
				UpdatableInset * inset = owner->view()->
					theLockingInset();
				inset->GetCursorPos(owner->view(), slx, sly);
				owner->view()->unlockInset(inset);
				owner->view()->menuRedo();
				inset = static_cast<UpdatableInset*>(
					TEXT()->cursor.par()->
					GetInset(TEXT()->
						 cursor.pos()));
				if (inset)
					inset->Edit(owner->view(),slx,sly,0); 
				return string();
			} else if (((result=owner->view()->theLockingInset()->
				   LocalDispatch(owner->view(), action,
						 argument)) ==
				   UpdatableInset::DISPATCHED) ||
				   (result == UpdatableInset::DISPATCHED_NOUPDATE))
				return string();
			else {
				//setMessage(N_("Text mode"));
				switch (action) {
				case LFUN_UNKNOWN_ACTION:
				case LFUN_BREAKPARAGRAPH:
				case LFUN_BREAKLINE:
					TEXT()->CursorRight(owner->view());
					owner->view()->setState();
					owner->showState();
					break;
				case LFUN_RIGHT:
					if (!TEXT()->cursor.par()->isRightToLeftPar(owner->buffer()->params)) {
						TEXT()->CursorRight(owner->view());
						moveCursorUpdate(true, false);
						owner->showState();
					}
					return string();
				case LFUN_LEFT: 
					if (TEXT()->cursor.par()->isRightToLeftPar(owner->buffer()->params)) {
						TEXT()->CursorRight(owner->view());
						moveCursorUpdate(true, false);
						owner->showState();
					}
					return string();
				case LFUN_DOWN:
					TEXT()->CursorDown(owner->view());
					moveCursorUpdate(true, false);
					owner->showState();
					return string();
				default:
					break;
				}
			}
		}
	}

	lyx::Assert(action != LFUN_SELECT_FILE_SYNC);

	switch (action) {
		
	case LFUN_ESCAPE:
	{
		if (!owner->view()->available()) break;
		
		// this function should be used always [asierra060396]
		UpdatableInset * tli =
			owner->view()->theLockingInset();
		if (tli) {
			UpdatableInset * lock = tli->GetLockingInset();
			
			if (tli == lock) {
				owner->view()->unlockInset(tli);
				TEXT()->CursorRight(owner->view());
				moveCursorUpdate(true, false);
				owner->showState();
			} else {
				tli->UnlockInsetInInset(owner->view(),
							lock,
							true);
			}
		}
	}
	break;
			
		// --- Misc -------------------------------------------
	case LFUN_WORDFINDFORWARD  : 
	case LFUN_WORDFINDBACKWARD : {
		static string last_search;
		string searched_string;
	    
		if (!argument.empty()) {
			last_search = argument;
			searched_string = argument;
		} else {
			searched_string = last_search;
		}

		if (!searched_string.empty() &&
		    ((action == LFUN_WORDFINDBACKWARD) ? 
		     SearchBackward(owner->view(), searched_string) :
		     SearchForward(owner->view(), searched_string))) {

			// ??? What is that ???
			owner->view()->update(TEXT(), BufferView::SELECT|BufferView::FITCUR);

			// ??? Needed ???
			// clear the selection (if there is any) 
			owner->view()->toggleSelection();
			TEXT()->ClearSelection(owner->view());

			// Move cursor so that successive C-s 's will not stand in place. 
			if (action == LFUN_WORDFINDFORWARD ) 
				TEXT()->CursorRightOneWord(owner->view());
			TEXT()->FinishUndo();
			moveCursorUpdate(true, false);

			// ??? Needed ???
			// set the new selection 
			// SetSelectionOverLenChars(owner->view()->currentBuffer()->text, iLenSelected);
			owner->view()->toggleSelection(false);
		}
	 
		// REMOVED : if (owner->view()->getWorkArea()->focus)
		owner->view()->showCursor();
	}
	break;
		
	case LFUN_PREFIX:
	{
		if (owner->view()->available()) {
			owner->view()->update(TEXT(),
					      BufferView::SELECT|BufferView::FITCUR);
		}
		string buf;
		keyseq.print(buf, true);
		owner->message(buf);
	}
	break;

	// --- Misc -------------------------------------------
	case LFUN_EXEC_COMMAND:
	{
		std::vector<string> allCmds;
		std::transform(lyxaction.func_begin(), lyxaction.func_end(),
			       std::back_inserter(allCmds), lyx::firster());
		static std::vector<string> hist;
		owner->getMiniBuffer()->getString(MiniBuffer::spaces,
						  allCmds, hist);
	}
	break;
		
	case LFUN_CANCEL:                   // RVDK_PATCH_5
		keyseq.reset();
		meta_fake_bit = 0;
		if (owner->view()->available())
			// cancel any selection
			Dispatch(LFUN_MARK_OFF);
		setMessage(N_("Cancel"));
		break;

	case LFUN_META_FAKE:                                 // RVDK_PATCH_5
	{
		meta_fake_bit = Mod1Mask;
		string buf;
		keyseq.print(buf, true);
		setMessage(buf); // RVDK_PATCH_5
	}
	break;  

	case LFUN_READ_ONLY_TOGGLE:
		if (owner->buffer()->lyxvc.inUse()) {
			owner->buffer()->lyxvc.toggleReadOnly();
		} else {
			owner->buffer()->setReadonly(
				!owner->buffer()->isReadonly());
		}
		break;
		
	case LFUN_CENTER: // this is center and redraw.
		owner->view()->center();
		break;
		
		// --- Menus -----------------------------------------------
	case LFUN_MENUNEW:
		MenuNew(false);
		break;
		
	case LFUN_MENUNEWTMPLT:
		MenuNew(true);
		break;
		
	case LFUN_CLOSEBUFFER:
		CloseBuffer();
		break;
		
	case LFUN_MENUWRITE:
		if (!owner->buffer()->isUnnamed()) {
			string const s1 = _("Saving document") + ' '
				+ MakeDisplayPath(owner->buffer()->fileName()
						  + "...");
			
			owner->message(s1);
			MenuWrite(owner->view(), owner->buffer());
		} else
			WriteAs(owner->view(), owner->buffer());
		break;
		
	case LFUN_WRITEAS:
		WriteAs(owner->view(), owner->buffer(), argument);
		break;
		
	case LFUN_MENURELOAD:
		reloadBuffer();
		break;
		
	case LFUN_UPDATE:
		Exporter::Export(owner->buffer(), argument, true);
		break;

	case LFUN_PREVIEW:
		Exporter::Preview(owner->buffer(), argument);
		break;
		
        case LFUN_BUILDPROG:
		Exporter::Export(owner->buffer(), "program", true);
                break;
                
 	case LFUN_RUNCHKTEX:
		MenuRunChktex(owner->buffer());
		break;
				
	case LFUN_MENUPRINT:
		owner->getDialogs()->showPrint();
		break;

	case LFUN_EXPORT:
		if (argument == "custom")
			MenuSendto();
		else
			Exporter::Export(owner->buffer(), argument, false);
		break;

	case LFUN_IMPORT:
		doImport(argument);
		break;
		
	case LFUN_QUIT:
		QuitLyX();
		break;
		
	case LFUN_TOCVIEW:
#if 0
	case LFUN_LOFVIEW:
	case LFUN_LOTVIEW:
	case LFUN_LOAVIEW:
#endif
	{
		InsetCommandParams p;

#if 0
		if (action == LFUN_TOCVIEW)
#endif
			p.setCmdName("tableofcontents");
#if 0
		else if (action == LFUN_LOAVIEW )
			p.setCmdName("listof{algorithm}{List of Algorithms}");
		else if (action == LFUN_LOFVIEW)
			p.setCmdName("listoffigures");
		else
			p.setCmdName("listoftables");
#endif
		owner->getDialogs()->createTOC(p.getAsString());
		break;
	}	

	case LFUN_DIALOG_TABULAR_INSERT:
		owner->getDialogs()->showTabularCreate();
		break;
		
	case LFUN_FIGURE:
		Figure();
		break;

	case LFUN_AUTOSAVE:
		AutoSave(owner->view());
		break;
		
	case LFUN_UNDO:
		owner->view()->menuUndo();
		break;
		
	case LFUN_REDO:
		owner->view()->menuRedo();
		break;
		
	case LFUN_MENUSEARCH:
		owner->getDialogs()->showSearch();
		break;
		
	case LFUN_REMOVEERRORS:
		if (owner->view()->removeAutoInsets()) {
			owner->view()->redraw();
			owner->view()->fitCursor(TEXT());
		}
		break;

	case LFUN_DEPTH:
		changeDepth(owner->view(), TEXT(false), 0);
		break;
		
	case LFUN_DEPTH_MIN:
		changeDepth(owner->view(), TEXT(false), -1);
		break;
		
	case LFUN_DEPTH_PLUS:
		changeDepth(owner->view(), TEXT(false), 1);
		break;
		
	case LFUN_FREE:
		owner->getDialogs()->setUserFreeFont();
		break;
		
	case LFUN_TEX:
		Tex(owner->view());
		owner->view()->setState();
		owner->showState();
		break;

	case LFUN_RECONFIGURE:
		Reconfigure(owner->view());
		break;

#if 0
	case LFUN_FLOATSOPERATE:
		if (argument == "openfoot")
			owner->view()->allFloats(1,0);
		else if (argument == "closefoot")
			owner->view()->allFloats(0,0);
		else if (argument == "openfig")
			owner->view()->allFloats(1,1);
		else if (argument == "closefig")
			owner->view()->allFloats(0,1);
		break;
#else
#ifdef WITH_WARNINGS
#warning Find another implementation here (or another lyxfunc)!
#endif
#endif
	case LFUN_HELP_COPYRIGHT:
		owner->getDialogs()->showCopyright();
		break;

	case LFUN_HELP_CREDITS:
		owner->getDialogs()->showCredits();
		break;

        case LFUN_HELP_OPEN:
	{
		string const arg = argument;
		if (arg.empty()) {
			setErrorMessage(N_("Missing argument"));
			break;
		}
		ProhibitInput(owner->view());
		string const fname = i18nLibFileSearch("doc", arg, "lyx");
		if (fname.empty()) {
			lyxerr << "LyX: unable to find documentation file `"
			       << arg << "'. Bad installation?" << endl;
			AllowInput(owner->view());
			break;
		}
		string const str = _("Opening help file") + ' '
			+ MakeDisplayPath(fname) + "...";
		
		owner->message(str);
		owner->view()->buffer(bufferlist.loadLyXFile(fname,false));
		AllowInput(owner->view());
		break;
        }

	case LFUN_HELP_VERSION: {
		ProhibitInput(owner->view());
		string msg(_("LyX Version "));
		msg += LYX_VERSION;
		msg += " of ";
		msg += LYX_RELEASE;
		fl_show_message(msg.c_str(),
				(_("Library directory: ")
				 + MakeDisplayPath(system_lyxdir)).c_str(),
				(_("User directory: ") 
				 + MakeDisplayPath(user_lyxdir)).c_str());
		AllowInput(owner->view());
		break;
	}
	
		// --- version control -------------------------------
	case LFUN_VC_REGISTER:
	{
		if (!owner->buffer()->lyxvc.inUse())
			owner->buffer()->lyxvc.registrer();
	}
	break;
		
	case LFUN_VC_CHECKIN:
	{
		if (owner->buffer()->lyxvc.inUse()
		    && !owner->buffer()->isReadonly())
			owner->buffer()->lyxvc.checkIn();
	}
	break;
		
	case LFUN_VC_CHECKOUT:
	{
		if (owner->buffer()->lyxvc.inUse()
		    && owner->buffer()->isReadonly())
			owner->buffer()->lyxvc.checkOut();
	}
	break;
	
	case LFUN_VC_REVERT:
	{
		owner->buffer()->lyxvc.revert();
	}
	break;
		
	case LFUN_VC_UNDO:
	{
		owner->buffer()->lyxvc.undoLast();
	}
	break;
		
	case LFUN_VC_HISTORY:
	{
		owner->getDialogs()->showVCLogFile();
		break;
	}
	
	// --- buffers ----------------------------------------

        case LFUN_SWITCHBUFFER:
                owner->view()->buffer(bufferlist.getBuffer(argument));
		break;

	case LFUN_FILE_NEW:
	{
		// servercmd: argument must be <file>:<template>
		Buffer * tmpbuf = NewLyxFile(argument);
		if (tmpbuf)
			owner->view()->buffer(tmpbuf);
	}
	break;
			
	case LFUN_FILE_OPEN:
		Open(argument);
		break;

	case LFUN_LATEX_LOG:
		owner->getDialogs()->showLogFile();
		break;
		
	case LFUN_LAYOUTNO:
	{
		lyxerr.debug() << "LFUN_LAYOUTNO: (arg) " << argument << endl;
		int sel = strToInt(argument);
		lyxerr.debug() << "LFUN_LAYOUTNO: (sel) "<< sel << endl;
		
		// Should this give a setMessage instead?
		if (sel == 0) 
			return string(); // illegal argument

		--sel; // sel 1..., but layout 0...

		// Pretend we got the name instead.
		Dispatch(int(LFUN_LAYOUT), 
			 textclasslist.NameOfLayout(owner->view()
						    ->buffer()->params.textclass,
						    sel));
		return string();
	}
		
	case LFUN_LAYOUT_DOCUMENT:
		owner->getDialogs()->showLayoutDocument();
		break;
		
	case LFUN_LAYOUT_PARAGRAPH:
		owner->getDialogs()->showLayoutParagraph();
		break;
		
	case LFUN_LAYOUT_CHARACTER:
		owner->getDialogs()->showLayoutCharacter();
		break;

	case LFUN_LAYOUT_TABULAR:
	    if (owner->view()->theLockingInset()) {
		if (owner->view()->theLockingInset()->LyxCode()==Inset::TABULAR_CODE) {
		    InsetTabular * inset = static_cast<InsetTabular *>
			(owner->view()->theLockingInset());
		    inset->OpenLayoutDialog(owner->view());
		} else if (owner->view()->theLockingInset()->
			   GetFirstLockingInsetOfType(Inset::TABULAR_CODE)!=0) {
		    InsetTabular * inset = static_cast<InsetTabular *>(
			owner->view()->theLockingInset()->GetFirstLockingInsetOfType(Inset::TABULAR_CODE));
		    inset->OpenLayoutDialog(owner->view());
		}
	    }
	    break;

	case LFUN_LAYOUT_PREAMBLE:
		owner->getDialogs()->showPreamble();
		break;
		
	case LFUN_LAYOUT_SAVE_DEFAULT:
		MenuLayoutSave(owner->view());
		break;
		
	case LFUN_DROP_LAYOUTS_CHOICE:
		owner->getToolbar()->openLayoutList();
		break;

	case LFUN_MENU_OPEN_BY_NAME:
		owner->getMenubar()->openByName(argument);
		break; // RVDK_PATCH_5
		
	case LFUN_SPELLCHECK:
		if (lyxrc.isp_command != "none")
			ShowSpellChecker(owner->view());
		break; // RVDK_PATCH_5
		
	// --- lyxserver commands ----------------------------


	case LFUN_GETNAME:
		setMessage(owner->buffer()->fileName());
		lyxerr.debug() << "FNAME["
			       << owner->buffer()->fileName()
			       << "] " << endl;
		break;
		
	case LFUN_NOTIFY:
	{
		string buf;
		keyseq.print(buf);
		dispatch_buffer = buf;
		lyxserver->notifyClient(dispatch_buffer);
	}
	break;

	case LFUN_GOTOFILEROW:
	{
	        char file_name[100];
		int row;
		::sscanf(argument.c_str(), " %s %d", file_name, &row);

		// Must replace extension of the file to be .lyx and get full path
		string s = ChangeExtension(string(file_name), ".lyx");

		// Either change buffer or load the file
		if (bufferlist.exists(s))
		        owner->view()->buffer(bufferlist.getBuffer(s));
		else
		        owner->view()->buffer(bufferlist.loadLyXFile(s));

		// Set the cursor  
		owner->view()->setCursorFromRow(row);

		// Recenter screen
		owner->view()->center();
	}
	break;

	case LFUN_GOTO_PARAGRAPH:
	{
                istringstream istr(argument.c_str());

		int id;
		istr >> id;
		LyXParagraph * par = TEXT()->GetParFromID(id);
		if (par == 0)
			break;

		// Set the cursor
		TEXT()->SetCursor(owner->view(), par, 0);
		owner->view()->setState();
		owner->showState();

		// Recenter screen
		owner->view()->center();
	}
	break;

	case LFUN_APROPOS:
	case LFUN_GETTIP:
	{
		int const qa = lyxaction.LookupFunc(argument);
		setMessage(lyxaction.helpText(static_cast<kb_action>(qa)));
	}
	break;

	// --- toolbar ----------------------------------
	case LFUN_PUSH_TOOLBAR:
	{
		int nth = strToInt(argument);
		if (nth <= 0) {
			setErrorMessage(N_("Push-toolbar needs argument > 0"));
		} else {
			owner->getToolbar()->push(nth);
		}
	}
	break;
	
	case LFUN_ADD_TO_TOOLBAR:
	{
		if (lyxerr.debugging(Debug::GUI)) {
			lyxerr << "LFUN_ADD_TO_TOOLBAR:"
				"argument = `" << argument << '\'' << endl;
		}
		string tmp(argument);
		//lyxerr <<string("Argument: ") + argument);
		//lyxerr <<string("Tmp     : ") + tmp);
		if (tmp.empty()) {
			setErrorMessage(N_("Usage: toolbar-add-to <LyX command>"));
		} else {
			owner->getToolbar()->add(argument, false);
			owner->getToolbar()->set();
		}
	}
	break;
	
	// --- insert characters ----------------------------------------

	// ---  Mathed stuff. If we are here, there is no locked inset yet.
	
	// Greek mode     
	case LFUN_GREEK:
	{
		if (!greek_kb_flag) {
			greek_kb_flag = 1;
			setMessage(N_("Math greek mode on"));
		} else
			greek_kb_flag = 0;
	}  
	break;
      
	// Greek keyboard      
	case LFUN_GREEK_TOGGLE:
	{
		greek_kb_flag = greek_kb_flag ? 0 : 2;
		if (greek_kb_flag) {
			setMessage(N_("Math greek keyboard on"));
		} else {
			setMessage(N_("Math greek keyboard off"));
		}
	}
	break;
	
	case LFUN_MATH_EXTERN:
	case LFUN_MATH_NUMBER:
	case LFUN_MATH_LIMITS:
	{
		setErrorMessage(N_("This is only allowed in math mode!"));
	}
	break;

	case LFUN_MATH_PANEL:
		owner->getDialogs()->showMathPanel();
		break;
	
	case LFUN_CITATION_CREATE:
	{
		InsetCommandParams p( "cite" );
		
		if (!argument.empty()) {
			// This should be set at source, ie when typing
			// "citation-insert foo" in the minibuffer.
			// Question: would pybibliographer also need to be
			// changed. Suspect so. Leave as-is therefore.
			if (contains(argument, "|")) {
				p.setContents( token(argument, '|', 0) );
				p.setOptions(  token(argument, '|', 1) );
			} else {
				p.setContents( argument );
			}
			Dispatch(LFUN_CITATION_INSERT, p.getAsString());
		} else
			owner->getDialogs()->createCitation( p.getAsString() );
	}
	break;
		    
	case LFUN_CHILDOPEN:
	{
		string const filename =
			MakeAbsPath(argument, 
				    OnlyPath(owner->buffer()->fileName()));
		setMessage(N_("Opening child document ") +
			   MakeDisplayPath(filename) + "...");
		owner->view()->savePosition(0);
		if (bufferlist.exists(filename))
			owner->view()->buffer(bufferlist.getBuffer(filename));
		else
			owner->view()->buffer(bufferlist.loadLyXFile(filename));
	}
	break;

	case LFUN_TOGGLECURSORFOLLOW:
		lyxrc.cursor_follows_scrollbar = !lyxrc.cursor_follows_scrollbar;
		break;
		
	case LFUN_KMAP_OFF:		// keymap off
		owner->getIntl()->KeyMapOn(false);
		break;
		
	case LFUN_KMAP_PRIM:	// primary keymap
		owner->getIntl()->KeyMapPrim();
		break;
		
	case LFUN_KMAP_SEC:		// secondary keymap
		owner->getIntl()->KeyMapSec();
		break;
		
	case LFUN_KMAP_TOGGLE:	// toggle keymap
		owner->getIntl()->ToggleKeyMap();
		break;

	case LFUN_SEQUENCE: 
	{
		// argument contains ';'-terminated commands
		while (argument.find(';') != string::npos) {
			string first;
			argument = split(argument, first, ';');
			Dispatch(first);
		}
	}
	break;

	case LFUN_DIALOG_PREFERENCES:
		owner->getDialogs()->showPreferences();
		break;
		
	case LFUN_SAVEPREFERENCES:
	{
		Path p(user_lyxdir);
		lyxrc.write("preferences");
	}
	break;

	case LFUN_SCREEN_FONT_UPDATE:
	{
		// handle the screen font changes.
		// 
		lyxrc.set_font_norm_type();
		fontloader.update();
		// Of course we should only do the resize and the textcache.clear
		// if values really changed...but not very important right now. (Lgb)
		// All buffers will need resize
		bufferlist.resize();
		// We also need to empty the textcache so that
		// the buffer will be formatted correctly after
		// a zoom change.
		textcache.clear();
	}
	break;

	case LFUN_SET_COLOR:
	{
		string lyx_name;
		string const x11_name = split(argument, lyx_name, ' ');
		if (lyx_name.empty() || x11_name.empty()) {
			setErrorMessage(N_("Syntax: set-color <lyx_name>"
						" <x11_name>"));
			break;
			}

		if (!lcolor.setColor(lyx_name, x11_name)) {
			static string const err1 (N_("Set-color \""));
			static string const err2 (
				N_("\" failed - color is undefined "
				   "or may not be redefined"));
			setErrorMessage(_(err1) + lyx_name + _(err2));
			break;
		}
		lyxColorHandler->updateColor(lcolor.getFromLyXName(lyx_name));
		owner->view()->redraw();
		break;
	}

	case LFUN_MESSAGE:
		owner->message(argument);
		break;

	case LFUN_MESSAGE_PUSH:
		owner->messagePush(argument);
		break;

	case LFUN_MESSAGE_POP:
		owner->messagePop();
		break;

	default:
		// Then if it was none of the above
		if (!owner->view()->Dispatch(action, argument))
			lyxerr << "A truly unknown func ["
			       << action << "]!" << endl;
		break;
	} // end of switch

exit_with_message:

	string const res = getMessage();

	if (res.empty()) {
		if (!commandshortcut.empty()) {
			owner->getMiniBuffer()->addSet(commandshortcut);
		}
	} else {
		string const msg(_(res) + ' ' + commandshortcut);
		owner->message(msg);
	}

	return res;
}


void LyXFunc::setupLocalKeymap()
{
	keyseq.stdmap = keyseq.curmap = toplevel_keymap.get();
	cancel_meta_seq.stdmap = cancel_meta_seq.curmap = toplevel_keymap.get();
}


void LyXFunc::MenuNew(bool fromTemplate)
{
	string initpath = lyxrc.document_path;

	if (owner->view()->available()) {
		string const trypath = owner->buffer()->filepath;
		// If directory is writeable, use this as default.
		if (IsDirWriteable(trypath) == 1)
			initpath = trypath;
	}

	static int newfile_number = 0;
	string s;
	
	if (lyxrc.new_ask_filename) {
		FileDialog fileDlg(owner, _("Enter filename for new document"),
				   LFUN_SELECT_FILE_SYNC,
			make_pair(string(_("Documents")),
				  string(lyxrc.document_path)),
			make_pair(string(_("Templates")),
				  string(lyxrc.template_path)));

		FileDialog::Result result =
			fileDlg.Select(initpath,
				       _("*.lyx|LyX Documents (*.lyx)"),
				       _("newfile"));
	
		if (result.second.empty()) {
			owner->message(_("Canceled."));
			lyxerr.debug() << "New Document Cancelled." << endl;
			return;
		}
	
		// get absolute path of file and make sure the filename ends
		// with .lyx
		s = MakeAbsPath(result.second);
		if (!IsLyXFilename(s))
			s += ".lyx";

		// Check if the document already is open
		if (bufferlist.exists(s)) {
			switch (AskConfirmation(_("Document is already open:"),
						MakeDisplayPath(s, 50),
						_("Do you want to close that document now?\n"
						  "('No' will just switch to the open version)")))
			{
			case 1: // Yes: close the document
				if (!bufferlist.close(bufferlist.getBuffer(s)))
				// If close is canceled, we cancel here too.
					return;
				break;
			case 2: // No: switch to the open document
				owner->view()->buffer(bufferlist.getBuffer(s));
				return;
			case 3: // Cancel: Do nothing
				owner->message(_("Canceled."));
				return;
			}
		}
		// Check whether the file already exists
		if (IsLyXFilename(s)) {
			FileInfo fi(s);
			if (fi.readable() &&
			    AskQuestion(_("File already exists:"), 
					MakeDisplayPath(s, 50),
					_("Do you want to open the document?"))) {
				// loads document
				owner->message(_("Opening document") + ' '
					       + MakeDisplayPath(s) + "...");
				XFlush(fl_get_display());
				owner->view()->buffer(
					bufferlist.loadLyXFile(s));
				owner->message(_("Document") + ' '
					       + MakeDisplayPath(s) + ' '
					       + _("opened."));
				return;
			}
		}
	} else {
		s = AddName(lyxrc.document_path,
			    "newfile" + tostr(++newfile_number) + ".lyx");
		FileInfo fi(s);
		while (bufferlist.exists(s) || fi.readable()) {
			++newfile_number;
			s = AddName(lyxrc.document_path,
				    "newfile" +	tostr(newfile_number) +
				    ".lyx");
			fi.newFile(s);
		}
	}

	// The template stuff
	string templname;
	if (fromTemplate) {
		FileDialog fileDlg(owner, _("Select template file"),
			LFUN_SELECT_FILE_SYNC,
			make_pair(string(_("Documents")),
				  string(lyxrc.document_path)),
			make_pair(string(_("Templates")),
				  string(lyxrc.template_path)));

		FileDialog::Result result =
			fileDlg.Select(initpath,
				       _("*.lyx|LyX Documents (*.lyx)"));
	
		if (result.first == FileDialog::Later)
			return;

		string const fname = result.second;

		if (fname.empty()) 
			return;
                templname = fname;
	}
  
	// find a free buffer
	lyxerr.debug() << "Find a free buffer." << endl;
	owner->view()->buffer(bufferlist.newFile(s, templname));
}


void LyXFunc::Open(string const & fname)
{
	string initpath = lyxrc.document_path;
  
	if (owner->view()->available()) {
		string const trypath = owner->buffer()->filepath;
		// If directory is writeable, use this as default.
		if (IsDirWriteable(trypath) == 1)
			initpath = trypath;
	}

	string filename;
 
	if (fname.empty()) {
		FileDialog fileDlg(owner, _("Select document to open"),
			LFUN_FILE_OPEN,
			make_pair(string(_("Documents")),
				  string(lyxrc.document_path)),
			make_pair(string(_("Examples")),
				  string(AddPath(system_lyxdir, "examples"))));

		FileDialog::Result result =
			fileDlg.Select(initpath,
				       "*.lyx|LyX Documents (*.lyx)");
	
		if (result.first == FileDialog::Later)
			return;

		filename = result.second;
 
		// check selected filename
		if (filename.empty()) {
			owner->message(_("Canceled."));
			return;
		}
	} else
		filename = fname;

	// get absolute path of file and make sure the filename ends
	// with .lyx
	filename = MakeAbsPath(filename);
	if (!IsLyXFilename(filename))
		filename += ".lyx";

	// loads document
	owner->message(_("Opening document") + ' '
		       + MakeDisplayPath(filename) + "...");
	Buffer * openbuf = bufferlist.loadLyXFile(filename);
	if (openbuf) {
		owner->view()->buffer(openbuf);
		owner->message(_("Document") + ' '
			       + MakeDisplayPath(filename)
			       + ' ' + _("opened."));
	} else {
		owner->message(_("Could not open document") + ' '
			       + MakeDisplayPath(filename));
	}
}


// checks for running without gui are missing.
void LyXFunc::doImport(string const & argument)
{
	string format;
	string filename = split(argument, format, ' ');
	lyxerr.debug() << "LyXFunc::doImport: " << format 
		       << " file: " << filename << endl;

	if (filename.empty()) { // need user interaction
		string initpath = lyxrc.document_path;
		
		if (owner->view()->available()) {
			string const trypath = owner->buffer()->filepath;
			// If directory is writeable, use this as default.
			if (IsDirWriteable(trypath) == 1)
				initpath = trypath;
		}

		string const text = _("Select ") + formats.PrettyName(format)
			+ _(" file to import");

		FileDialog fileDlg(owner, text, 
			LFUN_IMPORT,
			make_pair(string(_("Documents")),
				  string(lyxrc.document_path)),
			make_pair(string(_("Examples")),
				  string(AddPath(system_lyxdir, "examples"))));
			
		string const extension = "*." + formats.Extension(format)
			+ "| " + formats.PrettyName(format)
			+ " (*." + formats.Extension(format) + ")";

		FileDialog::Result result = fileDlg.Select(initpath,
							   extension);

		if (result.first == FileDialog::Later)
			return;

		filename = result.second;
 
		// check selected filename
		if (filename.empty())
			owner->message(_("Canceled."));
	}

	// still no filename? abort
	if (filename.empty()) 
		return;

	// get absolute path of file
	filename = MakeAbsPath(filename);

	string const lyxfile = ChangeExtension(filename, ".lyx");

	// Check if the document already is open
	if (bufferlist.exists(lyxfile)) {
		switch (AskConfirmation(_("Document is already open:"), 
					MakeDisplayPath(lyxfile, 50),
					_("Do you want to close that document now?\n"
					  "('No' will just switch to the open version)")))
			{
			case 1: // Yes: close the document
				if (!bufferlist.close(bufferlist.getBuffer(lyxfile)))
				// If close is canceled, we cancel here too.
					return;
				break;
			case 2: // No: switch to the open document
				owner->view()->buffer(bufferlist.getBuffer(lyxfile));
				return;
			case 3: // Cancel: Do nothing
				owner->message(_("Canceled."));
				return;
			}
	}

	// Check if a LyX document by the same root exists in filesystem
	FileInfo const f(lyxfile, true);
	if (f.exist() && !AskQuestion(_("A document by the name"), 
				      MakeDisplayPath(lyxfile),
				      _("already exists. Overwrite?"))) {
		owner->message(_("Canceled"));
		return;
	}
	// filename should be valid now
	
	Importer::Import(owner, filename, format);
}


void LyXFunc::reloadBuffer()
{
	string const fn = owner->buffer()->fileName();
	if (bufferlist.close(owner->buffer()))
		owner->view()->buffer(bufferlist.loadLyXFile(fn));
}


void LyXFunc::CloseBuffer()
{
	if (bufferlist.close(owner->buffer()) && !quitting) {
		if (bufferlist.empty()) {
			// need this otherwise SEGV may occur while trying to
			// set variables that don't exist
			// since there's no current buffer
			owner->getDialogs()->hideBufferDependent();
		} else {
			owner->view()->buffer(bufferlist.first());
		}
	}
}


// Each "owner" should have it's own message method. lyxview and
// the minibuffer would use the minibuffer, but lyxserver would
// send an ERROR signal to its client.  Alejandro 970603
// This func is bit problematic when it comes to NLS, to make the
// lyx servers client be language indepenent we must not translate
// strings sent to this func.
void LyXFunc::setErrorMessage(string const & m) const
{
	dispatch_buffer = m;
	errorstat = true;
}


void LyXFunc::setMessage(string const & m)
{
	dispatch_buffer = m;
}


void LyXFunc::initMiniBuffer() 
{
	string text = _("Welcome to LyX!");
	
	// When meta-fake key is pressed, show the key sequence so far + "M-".
	if (wasMetaKey()) {
		keyseqStr();
		text += "M-";
	}

	// Else, when a non-complete key sequence is pressed,
	// show the available options.
	else if (keyseqUncomplete()) 
		text = keyseqOptions();
   
	// Else, show the buffer state.
	else if (owner->view()->available()) {
		Buffer * tmpbuf = owner->buffer();
		
		string const nicename = 
			MakeDisplayPath(tmpbuf->fileName());
		// Should we do this instead? (kindo like emacs)
		// leaves more room for other information
		text = "LyX: ";
		text += nicename;
		if (tmpbuf->lyxvc.inUse()) {
			text += " [";
			text += tmpbuf->lyxvc.version();
			text += ' ';
			text += tmpbuf->lyxvc.locker();
			if (tmpbuf->isReadonly())
				text += " (RO)";
			text += ']';
		} else if (tmpbuf->isReadonly())
			text += " [RO]";
		if (!tmpbuf->isLyxClean())
			text += _(" (Changed)");
	} else {
		if (text != _("Welcome to LyX!")) // this is a hack
			text = _("* No document open *");
	}
	
	owner->message(text);
}

