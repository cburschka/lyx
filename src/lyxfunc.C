/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef HAVE_SSTREAM
#include <sstream>
using std::istringstream;
#else
#include <strstream>
#endif

#include <time.h>
#include <locale.h>

#include <cstdlib>
#include <cctype>
#include <cstring>

#ifdef __GNUG__
#pragma implementation
#endif

#include "version.h"
#include "lyxlookup.h"
#include "kbmap.h"
#include "lyxfunc.h"
#include "bufferlist.h"
#include "ColorHandler.h"
#include "lyxserver.h"
#include "lyx.h"
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
#include "filedlg.h"
#include "lyx_gui_misc.h"
#include "support/filetools.h"
#include "support/FileInfo.h"
#include "support/syscall.h"
#include "support/lstrings.h"
#include "support/path.h"
#include "debug.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "gettext.h"
#include "trans_mgr.h"
#include "ImportLaTeX.h"
#include "ImportNoweb.h"
#include "layout.h"
#include "WorkArea.h"
#include "lyxfr1.h"
#include "bufferview_funcs.h"
#include "frontends/Dialogs.h"
#include "frontends/Toolbar.h"
#ifdef NEW_MENUBAR
#include "frontends/Menubar.h"
#else
#include "menus.h"
#endif
#include "FloatList.h"

using std::pair;
using std::endl;

extern bool cursor_follows_scrollbar;

extern void InsertAsciiFile(BufferView *, string const &, bool);
extern void math_insert_symbol(char const *);
extern bool math_insert_greek(char);
extern BufferList bufferlist;
extern LyXServer * lyxserver;
extern int greek_kb_flag;
extern bool selection_possible;

extern kb_keymap * toplevel_keymap;

extern void MenuWrite(Buffer *);
extern bool MenuWriteAs(Buffer *);
extern int  MenuRunLaTeX(Buffer *);
extern int  MenuBuildProg(Buffer *);
extern int  MenuRunChktex(Buffer *);
extern bool CreatePostscript(Buffer *, bool);
extern void MenuPrint(Buffer *);
extern void MenuSendto();
extern void QuitLyX();
extern void MenuFax(Buffer *);
extern void MenuExport(Buffer *, string const &);
extern void show_symbols_form(LyXFunc *);

extern LyXAction lyxaction;
// (alkis)
extern tex_accent_struct get_accent(kb_action action);

extern void AutoSave(BufferView *);
extern bool PreviewDVI(Buffer *);
extern bool PreviewPostscript(Buffer *);
extern void MenuInsertLabel(char const *);
extern void MenuLayoutCharacter();
extern void MenuLayoutParagraph();
extern void MenuLayoutDocument();
extern void MenuLayoutPaper();
extern void MenuLayoutTable(int flag);
extern void MenuLayoutQuotes();
extern void MenuLayoutPreamble();
extern void MenuLayoutSave();
extern void bulletForm();

extern Buffer * NewLyxFile(string const &);
extern void LoadLyXFile(string const &);
extern void Reconfigure(BufferView *);

extern LyXTextClass::size_type current_layout;
extern int getISOCodeFromLaTeX(char *);

extern void ShowLatexLog();

/* === globals =========================================================== */

bool LyXFunc::show_sc = true;


LyXFunc::LyXFunc(LyXView * o)
	: owner(o)
{
	meta_fake_bit = 0;
	lyx_dead_action = LFUN_NOACTION;
	lyx_calling_dead_action = LFUN_NOACTION;
	setupLocalKeymap();
}


// I changed this func slightly. I commented out the ...FinishUndo(),
// this means that all places that used to have a moveCursorUpdate, now
// have a ...FinishUndo() as the preceeding statement. I have also added
// a moveCursorUpdate to some of the functions that updated the cursor, but
// that did not show its new position.
inline
void LyXFunc::moveCursorUpdate(bool selecting)
{
	if (selecting || owner->view()->text->mark_set) {
		owner->view()->text->SetSelection();
		owner->view()->toggleToggle();
	}
	owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
	owner->view()->showCursor();
	
	/* ---> Everytime the cursor is moved, show the current font state. */
	// should this too me moved out of this func?
	//owner->showState();
	owner->view()->setState();
}


int LyXFunc::processKeyEvent(XEvent * ev)
{
	char s_r[10];
	string argument;
	XKeyEvent * keyevent = &ev->xkey;
	KeySym keysym_return = 0;

	int num_bytes = LyXLookupString(ev, s_r, 10, &keysym_return);
	s_r[num_bytes] = '\0';

	if (lyxerr.debugging(Debug::KEY)) {
		char * tmp = XKeysymToString(keysym_return);
		string stm = (tmp ? tmp : "");
		lyxerr << "KeySym is "
		       << stm
		       << "["
		       << keysym_return << "]"
		       << " and num_bytes is "
		       << num_bytes
		       << " the string returned is \""
		       << s_r << '\"' << endl;
	}
	// Do nothing if we have nothing (JMarc)
	if (num_bytes == 0 && keysym_return == NoSymbol) {
		lyxerr[Debug::KEY] << "Empty kbd action (probably composing)"
				   << endl;
		return 0;
	}
	
	// this function should be used always [asierra060396]
	UpdatableInset * tli = owner->view()->the_locking_inset;
	if (owner->view()->available() && tli && (keysym_return==XK_Escape)) {
		if (tli == tli->GetLockingInset()) {
			owner->view()->unlockInset(tli);
			owner->view()->text->CursorRight(owner->view());
			moveCursorUpdate(false);
			owner->showState();
		} else {
			tli->UnlockInsetInInset(owner->view(),
						tli->GetLockingInset(),true);
		}
		return 0;
	}

	// Can we be sure that this will work for all X-Windows
	// implementations? (Lgb)
	// This code snippet makes lyx ignore some keys. Perhaps
	// all of them should be explictly mentioned?
	if((keysym_return >= XK_Shift_L && keysym_return <= XK_Hyper_R)
	   || keysym_return == XK_Mode_switch || keysym_return == 0x0)
		return 0;

	// Do a one-deep top-level lookup for
	// cancel and meta-fake keys. RVDK_PATCH_5
	cancel_meta_seq.reset();

	int action = cancel_meta_seq.addkey(keysym_return, keyevent->state
					    &(ShiftMask|ControlMask
					      |Mod1Mask)); 

	// When not cancel or meta-fake, do the normal lookup. 
	// Note how the meta_fake Mod1 bit is OR-ed in and reset afterwards.
	// Mostly, meta_fake_bit = 0. RVDK_PATCH_5.
	if ( (action != LFUN_CANCEL) && (action != LFUN_META_FAKE) ) {

		// remove Caps Lock and Mod2 as a modifiers
		action = keyseq.addkey(keysym_return,
				       (keyevent->state | meta_fake_bit)
				       &(ShiftMask|ControlMask
					 |Mod1Mask));      
	}
	// Dont remove this unless you know what you are doing.
	meta_fake_bit = 0;
		
	if (action == 0) action = LFUN_PREFIX;

	if (lyxerr.debugging(Debug::KEY)) {
		string buf;
		keyseq.print(buf);
		lyxerr << "Key ["
		       << action << "]["
		       << buf << "]["
		       << num_bytes << "]" << endl;
	}

	// already here we know if it any point in going further
	// why not return already here if action == -1 and
	// num_bytes == 0? (Lgb)

	if(keyseq.length > 1 || keyseq.length < -1) {
		string buf;
		keyseq.print(buf);
		owner->getMiniBuffer()->Set(buf);
	}

	if (action == -1) {
		if (keyseq.length < -1) { // unknown key sequence...
			string buf;
			LyXBell();
			keyseq.print(buf);
			owner->getMiniBuffer()->Set(_("Unknown sequence:"), buf);
			return 0;
		}
	
		char isochar = keyseq.getiso();
		if (!(keyevent->state&ControlMask) &&
		    !(keyevent->state&Mod1Mask) &&
		    (isochar && keysym_return < 0xF000)) {
			argument += isochar;
		}
		if (argument.empty()) {
			lyxerr.debug() << "Empty argument!" << endl;
			// This can`t possibly be of any use
			// so we`ll skip the dispatch.
			return 0;
		}
	} else
		if (action == LFUN_SELFINSERT) {
			argument = s_r[0];
		}
    
        bool tmp_sc = show_sc;
	show_sc = false;
	Dispatch(action, argument.c_str());
	show_sc = tmp_sc;
	
	return 0;
} 


LyXFunc::func_status LyXFunc::getStatus(int ac) const
{
        kb_action action;
        func_status flag = LyXFunc::OK;
        string argument;
        Buffer * buf = owner->buffer();
	
 	if (lyxaction.isPseudoAction(ac)) 
		action = lyxaction.retrieveActionArg(ac, argument);
	else
		action = static_cast<kb_action>(ac);
	
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
        static bool noLaTeX = lyxrc.latex_command == "none";
        bool disable = false;
        switch (action) {
	case LFUN_PREVIEW:
		disable = noLaTeX || lyxrc.view_dvi_command == "none";
		break;
	case LFUN_PREVIEWPS:
		disable = noLaTeX || lyxrc.view_ps_command == "none";
		break;
	case LFUN_RUNLATEX:
	case LFUN_RUNDVIPS:
		disable = noLaTeX;
		break;
	case LFUN_MENUPRINT:
		disable = noLaTeX || lyxrc.print_command == "none";
		break;
	case LFUN_FAX:
		disable = noLaTeX || lyxrc.fax_command == "none";
		break;
	case LFUN_IMPORT:
		if (argument == "latex" || argument == "noweb")
			disable = lyxrc.relyx_command == "none";
		else if (argument == "linuxdoc")
			disable = lyxrc.linuxdoc_to_lyx_command == "none";
		break;
	case LFUN_EXPORT:
		if (argument == "latex")
			disable = (! buf->isLatex() && ! buf->isLiterate()) ;
		else if (argument == "linuxdoc")
			disable = ! buf->isLinuxDoc();
		else if (argument == "docbook")
			disable = ! buf->isDocBook();
		else if (argument == "dvi" || argument == "postscript")
			disable = noLaTeX;
		else if (argument == "html")
			disable = (buf->isLinuxDoc() 
				   && lyxrc.linuxdoc_to_html_command == "none")
				|| (buf->isDocBook() 
				    && lyxrc.docbook_to_html_command == "none")
				|| (! buf->isLinuxDoc() && ! buf->isDocBook() 
				    && lyxrc.html_command == "none");
		else if (argument == "custom")
			disable = (! buf->isLatex() && ! buf->isLiterate());
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
		disable = (lyxrc.literate_command == "none" 
			   || ! buf->isLiterate());
		break;

#ifndef NEW_TABULAR
	case LFUN_LAYOUT_TABLE:
		disable = ! owner->view()->text->cursor.par()->table;
		break;
#endif
	case LFUN_LAYOUT_TABULAR:
		disable = true;
		if (owner->view()->the_locking_inset) {
			disable = (owner->view()->the_locking_inset->LyxCode() != Inset::TABULAR_CODE) &&
				!owner->view()->the_locking_inset->GetFirstLockingInsetOfType(Inset::TABULAR_CODE);
		}
		break;

	case LFUN_TABULAR_FEATURE:
		disable = true;
		if (owner->view()->the_locking_inset) {
			int ret = 0;
			if (owner->view()->the_locking_inset->LyxCode() == Inset::TABULAR_CODE) {
				ret = static_cast<InsetTabular *>
					(owner->view()->the_locking_inset)->
					getStatus(argument);
			} else if (owner->view()->the_locking_inset->GetFirstLockingInsetOfType(Inset::TABULAR_CODE)) {
				ret = static_cast<InsetTabular *>
					(owner->view()->the_locking_inset->
					GetFirstLockingInsetOfType(Inset::TABULAR_CODE))->
					getStatus(argument);
			}
			switch(ret) {
			case 0:
				break;
			case 1:
				disable = false;
				break;
			case 2:
				disable = false;
				flag |= LyXFunc::ToggleOn;
				break;
			case 3:
				disable = false;
				flag |= LyXFunc::ToggleOff;
				break;
			}
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
	default:
		break;
        }
        if (disable)
                flag |= LyXFunc::Disabled;

	if (buf) {
		func_status box = LyXFunc::ToggleOff;
		LyXFont font = owner->view()->text->real_current_font;
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


string LyXFunc::Dispatch(string const & s) 
{
	// Split command string into command and argument
	string cmd, line = frontStrip(s);
	string arg = strip(frontStrip(split(line, cmd, ' ')));

	return Dispatch(lyxaction.LookupFunc(cmd.c_str()), arg.c_str());
}


string LyXFunc::Dispatch(int ac,
			 char const * do_not_use_this_arg)
{
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
		if (do_not_use_this_arg)
			argument = do_not_use_this_arg; // except here
	}
    
	selection_possible = false;
	
	if (owner->view()->available())
		owner->view()->hideCursor();

	// We cannot use this function here
	if (getStatus(ac) & Disabled)
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
							  	  argument.c_str());

				if (pseudoaction == -1) {
					pseudoaction = action;
				} else {
					comname += " " + argument;
					argsadded = true;
				}
			}

			string shortcuts = toplevel_keymap->findbinding(pseudoaction);

			if (!shortcuts.empty()) {
				comname += ": " + shortcuts;
			} else if (!argsadded) {
				comname += " " + argument;
			}

			if (!comname.empty()) {
				comname = strip(comname);
				commandshortcut = "(" + comname + ')';
				owner->getMiniBuffer()->Set(commandshortcut);
				// Here we could even add a small pause,
				// to annoy the user and make him learn
				// the shortcuts.
				// No! That will just annoy, not teach
				// anything. The user will read the messages
				// if they are interested. (Asger)
			}
		}
        }

	// If in math mode pass the control to
	// the math inset [asierra060396]
	if (owner->view()->available() &&
	    owner->view()->the_locking_inset) {
		UpdatableInset::RESULT result;
		if ((action > 1) || ((action == LFUN_UNKNOWN_ACTION) &&
				     (keyseq.length >= -1)))
		{
			if ((action==LFUN_UNKNOWN_ACTION) && argument.empty()){
				argument = keyseq.getiso();
			}
			// Undo/Redo pre 0.13 is a bit tricky for insets.
		        if (action == LFUN_UNDO) {
				int slx, sly;
				UpdatableInset * inset = 
					owner->view()->the_locking_inset;
				inset->GetCursorPos(owner->view(), slx, sly);
				owner->view()->unlockInset(inset);
				owner->view()->menuUndo();
				if (owner->view()->text->cursor.par()->
				    IsInset(owner->view()->text->cursor.pos())) {
					inset = static_cast<UpdatableInset*>(
						owner->view()->text->cursor.par()->
						GetInset(owner->view()->text->
							 cursor.pos()));
				} else {
					inset = 0;
				}
				if (inset)
					inset->Edit(owner->view(),slx,sly,0);
				return string();
			} else if (action == LFUN_REDO) {
				int slx, sly;
				UpdatableInset * inset = owner->view()->
					the_locking_inset;
				inset->GetCursorPos(owner->view(), slx, sly);
				owner->view()->unlockInset(inset);
				owner->view()->menuRedo();
				inset = static_cast<UpdatableInset*>(
					owner->view()->text->cursor.par()->
					GetInset(owner->view()->text->
						 cursor.pos()));
				if (inset)
					inset->Edit(owner->view(),slx,sly,0); 
				return string();
			} else if (((result=owner->view()->the_locking_inset->
				   LocalDispatch(owner->view(), action,
						 argument)) ==
				   UpdatableInset::DISPATCHED) ||
				   (result == UpdatableInset::DISPATCHED_NOUPDATE))
				return string();
			else {
				setMessage(N_("Text mode"));
				switch(action) {
				case LFUN_UNKNOWN_ACTION:
				case LFUN_BREAKPARAGRAPH:
				case LFUN_BREAKLINE:
					owner->view()->text->CursorRight(owner->view());
					owner->view()->setState();
					owner->showState();
					break;
				case LFUN_RIGHT:
					if (!owner->view()->text->cursor.par()->isRightToLeftPar(owner->buffer()->params)) {
						owner->view()->text->CursorRight(owner->view());
						moveCursorUpdate(false);
						owner->showState();
					}
					return string();
				case LFUN_LEFT: 
					if (owner->view()->text->cursor.par()->isRightToLeftPar(owner->buffer()->params)) {
						owner->view()->text->CursorRight(owner->view());
						moveCursorUpdate(false);
						owner->showState();
					}
					return string();
				case LFUN_DOWN:
					owner->view()->text->CursorDown(owner->view());
					moveCursorUpdate(false);
					owner->showState();
					return string();
				default:
					break;
				}
			}
		}
	}

	switch(action) {
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

		LyXText * ltCur = owner->view()->text ;

		if (!searched_string.empty() &&
		    ((action == LFUN_WORDFINDBACKWARD) ? 
		     ltCur->SearchBackward(owner->view(), searched_string.c_str()) :
		     ltCur->SearchForward(owner->view(), searched_string.c_str()))) {

			// ??? What is that ???
			owner->view()->update(BufferView::SELECT|BufferView::FITCUR);

			// ??? Needed ???
			// clear the selection (if there is any) 
			owner->view()->toggleSelection();
			owner->view()->text->ClearSelection();

			// Move cursor so that successive C-s 's will not stand in place. 
			if( action == LFUN_WORDFINDFORWARD ) 
				owner->view()->text->CursorRightOneWord(owner->view());
			owner->view()->text->FinishUndo();
			moveCursorUpdate(false);

			// ??? Needed ???
			// set the new selection 
			// SetSelectionOverLenChars(owner->view()->currentBuffer()->text, iLenSelected);
			owner->view()->toggleSelection(false);
		} else 
			LyXBell();	
	 
		// REMOVED : if (owner->view()->getWorkArea()->focus)
		owner->view()->showCursor();
	}
	break;

	case LFUN_PREFIX:
	{
		if (owner->view()->available()) {
			owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		}
		string buf;
		keyseq.print(buf, true);
		owner->getMiniBuffer()->Set(buf, string(), string(), 1);
	}
	break;

	// --- Misc -------------------------------------------
	case LFUN_EXEC_COMMAND:
		owner->getMiniBuffer()->ExecCommand(); 
		break;
		
	case LFUN_CANCEL:                   // RVDK_PATCH_5
		keyseq.reset();
		meta_fake_bit = 0;
		if(owner->view()->available())
			// cancel any selection
			Dispatch(LFUN_MARK_OFF, 0);
		setMessage(N_("Cancel"));
		break;

	case LFUN_META_FAKE:                                 // RVDK_PATCH_5
	{
		meta_fake_bit = Mod1Mask;
		string buf;
		keyseq.print(buf, true);
		string res = string("M-") + buf;
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
		
	case LFUN_APPENDIX:
		if (owner->view()->available()) {
			owner->view()->text->toggleAppendix(owner->view());
			owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		}
		break;

		// --- Menus -----------------------------------------------
	case LFUN_MENUNEW:
		MenuNew(false);
		break;
		
	case LFUN_MENUNEWTMPLT:
		MenuNew(true);
		break;
		
	case LFUN_MENUOPEN:
		MenuOpen();
		break;
		
	case LFUN_CLOSEBUFFER:
		CloseBuffer();
		break;
		
	case LFUN_MENUWRITE:
		if (!owner->buffer()->isUnnamed()) {
			owner->getMiniBuffer()->Set(_("Saving document"),
						    MakeDisplayPath(owner->buffer()->fileName()),
						    "...");
			MenuWrite(owner->buffer());
			//owner->getMiniBuffer()-> {
			//	Set(_("Document saved as"),
			//	    MakeDisplayPath(owner->buffer()->fileName()));
			//} else {
			//owner->getMiniBuffer()->Set(_("Save failed!"));
			//}
		} else {
			MenuWriteAs(owner->buffer());
		}
		break;
		
	case LFUN_MENUWRITEAS:
		MenuWriteAs(owner->buffer());
		break;
		
	case LFUN_MENURELOAD:
		reloadBuffer();
		break;
		
	case LFUN_PREVIEW:
		PreviewDVI(owner->buffer());
		break;
			
	case LFUN_PREVIEWPS:
		PreviewPostscript(owner->buffer());
		break;
		
	case LFUN_RUNLATEX:
		MenuRunLaTeX(owner->buffer());
		break;
		
        case LFUN_BUILDPROG:
                MenuBuildProg(owner->buffer());
                break;
                
 	case LFUN_RUNCHKTEX:
		MenuRunChktex(owner->buffer());
		break;
		
	case LFUN_RUNDVIPS:
		CreatePostscript(owner->buffer(), false);
		break;
		
	case LFUN_MENUPRINT:
		owner->getDialogs()->showPrint();
		break;

	case LFUN_FAX:
		MenuFax(owner->buffer());
		break;
			
	case LFUN_EXPORT:
		MenuExport(owner->buffer(), argument);
		break;

	case LFUN_IMPORT:
		doImport(argument);
		break;
		
	case LFUN_QUIT:
		QuitLyX();
		break;
		
	case LFUN_TOCVIEW:
	case LFUN_LOFVIEW:
	case LFUN_LOTVIEW:
	case LFUN_LOAVIEW:
	{
		InsetCommandParams p;
		
		if( action == LFUN_TOCVIEW )
			p.setCmdName( "tableofcontents" );
		else if( action == LFUN_LOAVIEW )
			p.setCmdName( "listofalgorithms" );
		else if( action == LFUN_LOFVIEW )
			p.setCmdName( "listoffigures" );
		else
			p.setCmdName( "listoftables" );

		owner->getDialogs()->createTOC( p.getAsString() );
		break;
	}	

	case LFUN_TOC_INSERT:
	case LFUN_LOA_INSERT:
	case LFUN_LOF_INSERT:
	case LFUN_LOT_INSERT:
	{
		InsetCommandParams p;
		
		if( action == LFUN_TOC_INSERT )
			p.setCmdName( "tableofcontents" );
		else if( action == LFUN_LOA_INSERT )
			p.setCmdName( "listofalgorithms" );
		else if( action == LFUN_LOF_INSERT )
			p.setCmdName( "listoffigures" );
		else
			p.setCmdName( "listoftables" );

		Inset * inset = new InsetTOC( p );
		if( !owner->view()->insertInset( inset, "Standard", true ) )
			delete inset;
		break;
	}
		
	case LFUN_TABLE:
#ifndef NEW_TABULAR
		Table();
#else
		owner->getDialogs()->showTabularCreate();
#endif
		break;
		
	case LFUN_TABULAR_FEATURE:
	case LFUN_SCROLL_INSET:
		// this is not handled here as this funktion is only aktive
		// if we have a locking_inset and that one is (or contains)
		// a tabular-inset
		break;

	case LFUN_FIGURE:
		Figure();
		break;

	case LFUN_INSET_GRAPHICS:
	{
		Inset * new_inset = new InsetGraphics;
		if (!owner->view()->insertInset(new_inset)) {
			delete new_inset;
		} else {
			// this is need because you don't use a inset->Edit()
			owner->view()->updateInset(new_inset, true);
			new_inset->Edit(owner->view(), 0, 0, 0);
		}
		break;
	}
	
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
	{
		// Ok this is one _very_ bad solution, but I think that some
		// of this will be rewritten as part of GUI indep anyway.
		// Lgb
		static LyXFindReplace FR_;
		FR_.StartSearch(owner->view());
	}
	break;
		
	case LFUN_PASTE:
		owner->view()->paste();
		owner->view()->setState();
		break;
		
	case LFUN_PASTESELECTION:
	{
	        bool asPara = false;
		if (argument == "paragraph") asPara = true;
		owner->view()->pasteClipboard(asPara);
	}
	break;
	
	case LFUN_CUT:
		owner->view()->cut();
		break;
		
	case LFUN_COPY:
		owner->view()->copy();
		break;
		
	case LFUN_LAYOUT_COPY:
		owner->view()->copyEnvironment();
		break;
		
	case LFUN_LAYOUT_PASTE:
		owner->view()->pasteEnvironment();
		owner->view()->setState();
		break;
		
	case LFUN_GOTOERROR:
		owner->view()->gotoError();
		break;
		
	case LFUN_REMOVEERRORS:
		if (owner->view()->removeAutoInsets()) {
			owner->view()->redraw();
			owner->view()->fitCursor();
			//owner->view()->updateScrollbar();
		}
		break;
		
	case LFUN_GOTONOTE:
		owner->view()->gotoNote();
		break;

#ifndef NEW_INSETS
	case LFUN_OPENSTUFF:
		owner->view()->openStuff();
		break;
#endif
		
	case LFUN_HYPHENATION:
		owner->view()->hyphenationPoint();
		break;
		
	case LFUN_LDOTS:
		owner->view()->ldots();
		break;
		
	case LFUN_END_OF_SENTENCE:
		owner->view()->endOfSentenceDot();
		break;

	case LFUN_MENU_SEPARATOR:
		owner->view()->menuSeparator();
		break;
		
	case LFUN_HFILL:
		owner->view()->hfill();
		break;
		
	case LFUN_DEPTH:
		changeDepth(owner->view(), 0);
		break;
		
	case LFUN_DEPTH_MIN:
		changeDepth(owner->view(), -1);
		break;
		
	case LFUN_DEPTH_PLUS:
		changeDepth(owner->view(), 1);
		break;
		
	case LFUN_FREE:
		Free(owner->view());
		owner->view()->setState();
		owner->showState();
		break;
		
	case LFUN_TEX:
		Tex(owner->view());
		owner->view()->setState();
		owner->showState();
		break;
#ifndef NEW_INSETS		
	case LFUN_MELT:
		Melt(owner->view());
		break;
#endif
	case LFUN_RECONFIGURE:
		Reconfigure(owner->view());
		break;
#ifndef NEW_INSETS
	case LFUN_FOOTMELT:
		if (owner->view()->available()
		    && !owner->view()->text->selection
		    && owner->view()->text->cursor.par()->footnoteflag
		    != LyXParagraph::NO_FOOTNOTE)
			{ // only melt footnotes with FOOTMELT, not margins etc
				if(owner->view()->text->cursor.par()->footnotekind == LyXParagraph::FOOTNOTE)
					Melt(owner->view());
			}
		else
			Foot(owner->view()); 
		owner->view()->setState();
		break;

	case LFUN_MARGINMELT:
		if (owner->view()->available()
		    && !owner->view()->text->selection
		    && owner->view()->text->cursor.par()->footnoteflag
		    != LyXParagraph::NO_FOOTNOTE) {
			// only melt margins
			if(owner->view()->text->cursor.par()->footnotekind == LyXParagraph::MARGIN)
				Melt(owner->view());
		} else
			Margin(owner->view()); 
		owner->view()->setState();
		break;
#endif
	case LFUN_HELP_COPYRIGHT:
		owner->getDialogs()->showCopyright();
		break;

	case LFUN_HELP_CREDITS:
		owner->getDialogs()->showCredits();
		break;

        case LFUN_HELP_OPEN: {
		string arg = argument;
		if (arg.empty()) {
			setErrorMessage(N_("Missing argument"));
			break;
		}
		ProhibitInput(owner->view());
		string fname = i18nLibFileSearch("doc", arg, "lyx");
		if (fname.empty()) {
			lyxerr << "LyX: unable to find documentation file `"
			       << arg << "'. Bad installation?" << endl;
			AllowInput(owner->view());
			break;
		}
		owner->getMiniBuffer()->Set(_("Opening help file"),
					    MakeDisplayPath(fname),"...");
		owner->view()->buffer(bufferlist.loadLyXFile(fname,false));
		AllowInput(owner->view());
		break;
        }

	case LFUN_HELP_VERSION: 
		ProhibitInput(owner->view());
		fl_show_message((string(_("LyX Version ")) + LYX_VERSION 
				 + _(" of ") + LYX_RELEASE).c_str(),
				(_("Library directory: ")
				 + MakeDisplayPath(system_lyxdir)).c_str(),
				(_("User directory: ") 
				 + MakeDisplayPath(user_lyxdir)).c_str());
		AllowInput(owner->view());
		break;

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
		owner->buffer()->lyxvc.showLog();
		break;
	}
	
	// --- buffers ----------------------------------------

        case LFUN_SWITCHBUFFER:
                owner->view()->buffer(bufferlist.getBuffer(argument));
		break;


	case LFUN_FILE_INSERT:
	{
		MenuInsertLyXFile(argument);
	}
	break;
	
	case LFUN_FILE_INSERT_ASCII:
	{
		bool asPara = (argument == "paragraph");
		InsertAsciiFile(owner->view(), string(), asPara);
	}
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
		owner->view()->buffer(bufferlist.loadLyXFile(argument));
		break;

	case LFUN_LATEX_LOG:
		ShowLatexLog();
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
						    sel).c_str());
		return string();
	}
		
	case LFUN_LAYOUT:
	{
		lyxerr.debug() << "LFUN_LAYOUT: (arg) "
			       << argument << endl;
		
		// Derive layout number from given argument (string)
		// and current buffer's textclass (number). */    
		LyXTextClassList::ClassList::size_type tclass =
			owner->view()->buffer()->params.textclass;
		pair <bool, LyXTextClass::size_type> layout = 
			textclasslist.NumberOfLayout(tclass, argument);

		// If the entry is obsolete, use the new one instead.
		if (layout.first) {
			string obs = textclasslist.Style(tclass,layout.second)
			      .obsoleted_by();
			if (!obs.empty()) 
				layout = 
				  textclasslist.NumberOfLayout(tclass, obs);
		}

		// see if we found the layout number:
		if (!layout.first) {
			setErrorMessage(string(N_("Layout ")) + argument + 
					N_(" not known"));
			break;
		}

		if (current_layout != layout.second) {
			owner->view()->hideCursor();
			current_layout = layout.second;
			owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
			owner->view()->text->
				SetLayout(owner->view(), layout.second);
			owner->setLayout(layout.second);
			owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
			owner->view()->setState();
		}
	}
	break;

	case LFUN_LAYOUT_DOCUMENT:
		MenuLayoutDocument();
		break;
		
	case LFUN_LAYOUT_PARAGRAPH:
		MenuLayoutParagraph();
		break;
		
	case LFUN_LAYOUT_CHARACTER:
		MenuLayoutCharacter();
		break;
		
	case LFUN_LAYOUT_TABLE:
	{
	        int flag = 0;
	        if (argument == "true") flag = 1;
		MenuLayoutTable(flag);
	}
	break;
		
	case LFUN_LAYOUT_TABULAR:
	    if (owner->view()->the_locking_inset) {
		if (owner->view()->the_locking_inset->LyxCode()==Inset::TABULAR_CODE) {
		    InsetTabular * inset = static_cast<InsetTabular *>
			(owner->view()->the_locking_inset);
		    inset->OpenLayoutDialog(owner->view());
		} else if (owner->view()->the_locking_inset->
			   GetFirstLockingInsetOfType(Inset::TABULAR_CODE)!=0) {
		    InsetTabular * inset = static_cast<InsetTabular *>(
			owner->view()->the_locking_inset->GetFirstLockingInsetOfType(Inset::TABULAR_CODE));
		    inset->OpenLayoutDialog(owner->view());
		}
	    }
	    break;

	case LFUN_LAYOUT_PAPER:
		MenuLayoutPaper();
		break;
		
	case LFUN_LAYOUT_QUOTES:
		MenuLayoutQuotes();
		break;
		
	case LFUN_LAYOUT_PREAMBLE:
		MenuLayoutPreamble();
		break;
		
	case LFUN_LAYOUT_SAVE_DEFAULT:
		MenuLayoutSave();
		break;
		
	case LFUN_DROP_LAYOUTS_CHOICE:
		owner->getToolbar()->openLayoutList();
		break;

	case LFUN_LANGUAGE:
		Lang(owner->view(), argument);
		owner->view()->setState();
		owner->showState();
		break;

	case LFUN_EMPH:
		Emph(owner->view());
		owner->showState();
		break;

	case LFUN_BOLD:
		Bold(owner->view());
		owner->showState();
		break;
		
	case LFUN_NOUN:
		Noun(owner->view());
		owner->showState();
		break;
		
	case LFUN_CODE:
		Code(owner->view());
		owner->showState();
		break;
		
	case LFUN_SANS:
		Sans(owner->view());
		owner->showState();
		break;
		
	case LFUN_ROMAN:
		Roman(owner->view());
		owner->showState();
		break;
		
	case LFUN_DEFAULT:
		StyleReset(owner->view());
		owner->showState();
		break;
		
	case LFUN_UNDERLINE:
		Underline(owner->view());
		owner->showState();
		break;
		
	case LFUN_FONT_SIZE:
		FontSize(owner->view(), argument);
		owner->showState();
		break;
		
	case LFUN_FONT_STATE:
		setMessage(CurrentState(owner->view()));
		break;
		
	case LFUN_UPCASE_WORD:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->ChangeWordCase(owner->view(),
						    LyXText::text_uppercase);
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		break;
		
	case LFUN_LOWCASE_WORD:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->ChangeWordCase(owner->view(),
						    LyXText::text_lowercase);
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		break;
		
	case LFUN_CAPITALIZE_WORD:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->ChangeWordCase(owner->view(),
						    LyXText::text_capitalization);
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		break;
		
	case LFUN_INSERT_LABEL:
		MenuInsertLabel(argument.c_str());
		break;
		
	case LFUN_REF_CREATE:
	{
		InsetCommandParams p( "ref" );
		owner->getDialogs()->createRef( p.getAsString() );
	}
	break;
		
	case LFUN_REF_INSERT:
	{
		InsetCommandParams p;
		p.setFromString( argument );

		InsetRef * inset = new InsetRef( p );
		if (!owner->view()->insertInset(inset))
			delete inset;
		else
			owner->view()->updateInset( inset, true );
	}
	break;
		    
	case LFUN_REF_BACK:
	{
		owner->view()->restorePosition();
	}
	break;

	case LFUN_REF_GOTO:
	{
		string label(argument);
		if (label.empty()) {
			InsetRef * inset = 
				static_cast<InsetRef*>(getInsetByCode(Inset::REF_CODE));
			if (inset)
                                label = inset->getContents();
		}
		
		if (!label.empty()) {
			owner->view()->savePosition();
			if (!owner->view()->gotoLabel(label))
				WriteAlert(_("Error"), 
					   _("Couldn't find this label"), 
					   _("in current document."));
		}
	}
	break;
		
	case LFUN_MENU_OPEN_BY_NAME:
#ifdef NEW_MENUBAR
		owner->getMenubar()->openByName(argument);
#else
		owner->getMenus()->openByName(argument);
#endif
		break; // RVDK_PATCH_5
		
	case LFUN_SPELLCHECK:
		if (lyxrc.isp_command != "none")
			ShowSpellChecker(owner->view());
		break; // RVDK_PATCH_5
		
		// --- Cursor Movements -----------------------------
	case LFUN_RIGHT:
	{
		LyXText * tmptext = owner->view()->text;
		bool is_rtl = tmptext->cursor.par()->isRightToLeftPar(owner->buffer()->params);
		if(!tmptext->mark_set)
			owner->view()->beforeChange();
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		if (is_rtl)
			tmptext->CursorLeft(owner->view(), false);
		if (tmptext->cursor.pos() < tmptext->cursor.par()->Last()
		    && tmptext->cursor.par()->GetChar(tmptext->cursor.pos())
		    == LyXParagraph::META_INSET
		    && tmptext->cursor.par()->GetInset(tmptext->cursor.pos())
		    && tmptext->cursor.par()->GetInset(tmptext->cursor.pos())->Editable() == Inset::HIGHLY_EDITABLE){
			Inset * tmpinset = tmptext->cursor.par()->GetInset(tmptext->cursor.pos());
			setMessage(tmpinset->EditMessage());
			tmpinset->Edit(owner->view(), 0, 0, 0);
			break;
		}
		if (!is_rtl)
			tmptext->CursorRight(owner->view(), false);
		owner->view()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->showState();
	}
	break;
		
	case LFUN_LEFT:
	{
		// This is soooo ugly. Isn`t it possible to make
		// it simpler? (Lgb)
		LyXText * txt = owner->view()->text;
		bool is_rtl = txt->cursor.par()->isRightToLeftPar(owner->buffer()->params);
		if(!txt->mark_set) owner->view()->beforeChange();
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		if (!is_rtl)
			txt->CursorLeft(owner->view(), false);
		if (txt->cursor.pos() < txt->cursor.par()->Last()
		    && txt->cursor.par()->GetChar(txt->cursor.pos())
		    == LyXParagraph::META_INSET
		    && txt->cursor.par()->GetInset(txt->cursor.pos())
		    && txt->cursor.par()->GetInset(txt->cursor.pos())->Editable() == Inset::HIGHLY_EDITABLE) {
			Inset * tmpinset = txt->cursor.par()->GetInset(txt->cursor.pos());
			setMessage(tmpinset->EditMessage());
			LyXFont font = txt->GetFont(owner->view()->buffer(),
						    txt->cursor.par(),
						    txt->cursor.pos());
			tmpinset->Edit(owner->view(),
				       tmpinset->x() +
				       tmpinset->width(owner->view(),font),
				       tmpinset->descent(owner->view(),font),
				       0);
			break;
		}
		if  (is_rtl)
			txt->CursorRight(owner->view(), false);

		owner->view()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->showState();
	}
	break;
		
	case LFUN_UP:
		if(!owner->view()->text->mark_set) owner->view()->beforeChange();
		owner->view()->update(BufferView::UPDATE);
		owner->view()->text->CursorUp(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->showState();
		break;
		
	case LFUN_DOWN:
		if(!owner->view()->text->mark_set)
			owner->view()->beforeChange();
		owner->view()->update(BufferView::UPDATE);
		owner->view()->text->CursorDown(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->showState();
		break;

	case LFUN_UP_PARAGRAPH:
		if(!owner->view()->text->mark_set)
			owner->view()->beforeChange();
		owner->view()->update(BufferView::UPDATE);
		owner->view()->text->CursorUpParagraph(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->showState();
		break;
		
	case LFUN_DOWN_PARAGRAPH:
		if(!owner->view()->text->mark_set)
			owner->view()->beforeChange();
		owner->view()->update(BufferView::UPDATE);
		owner->view()->text->CursorDownParagraph(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->showState();
		break;
		
	case LFUN_PRIOR:
		if(!owner->view()->text->mark_set)
			owner->view()->beforeChange();
		owner->view()->update(BufferView::UPDATE);
		owner->view()->cursorPrevious();
		owner->view()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->showState();
		break;
		
	case LFUN_NEXT:
		if(!owner->view()->text->mark_set)
			owner->view()->beforeChange();
		owner->view()->update(BufferView::UPDATE);
		owner->view()->cursorNext();
		owner->view()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->showState();
		break;
		
	case LFUN_HOME:
		if(!owner->view()->text->mark_set)
			owner->view()->beforeChange();
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->CursorHome(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->showState();
		break;
		
	case LFUN_END:
		if(!owner->view()->text->mark_set)
			owner->view()->beforeChange();
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->CursorEnd(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->showState();
		break;
		
	case LFUN_SHIFT_TAB:
	case LFUN_TAB:
		if(!owner->view()->text->mark_set)
			owner->view()->beforeChange();
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->CursorTab(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->showState();
		break;
		
	case LFUN_WORDRIGHT:
		if(!owner->view()->text->mark_set)
			owner->view()->beforeChange();
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		if (owner->view()->text->cursor.par()->isRightToLeftPar(owner->buffer()->params))
			owner->view()->text->CursorLeftOneWord(owner->view());
		else
			owner->view()->text->CursorRightOneWord(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->showState();
		break;
		
	case LFUN_WORDLEFT:
		if(!owner->view()->text->mark_set)
			owner->view()->beforeChange();
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		if (owner->view()->text->cursor.par()->isRightToLeftPar(owner->buffer()->params))
			owner->view()->text->CursorRightOneWord(owner->view());
		else
			owner->view()->text->CursorLeftOneWord(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->showState();
		break;
		
	case LFUN_BEGINNINGBUF:
		if(!owner->view()->text->mark_set)
			owner->view()->beforeChange();
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->CursorTop(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->showState();
		break;
		
	case LFUN_ENDBUF:
		if(!owner->view()->text->mark_set)
			owner->view()->beforeChange();
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->CursorBottom(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->showState();
		break;

      
		/* cursor selection ---------------------------- */
	case LFUN_RIGHTSEL:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		if (owner->view()->text->cursor.par()->isRightToLeftPar(owner->buffer()->params))
			owner->view()->text->CursorLeft(owner->view());
		else
			owner->view()->text->CursorRight(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->showState();
		break;
		
	case LFUN_LEFTSEL:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		if (owner->view()->text->cursor.par()->isRightToLeftPar(owner->buffer()->params))
			owner->view()->text->CursorRight(owner->view());
		else
			owner->view()->text->CursorLeft(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->showState();
		break;
		
	case LFUN_UPSEL:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->CursorUp(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->showState();
		break;
		
	case LFUN_DOWNSEL:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->CursorDown(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->showState();
		break;

	case LFUN_UP_PARAGRAPHSEL:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->CursorUpParagraph(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->showState();
		break;
		
	case LFUN_DOWN_PARAGRAPHSEL:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->CursorDownParagraph(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->showState();
		break;
		
	case LFUN_PRIORSEL:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->cursorPrevious();
		owner->view()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->showState();
		break;
		
	case LFUN_NEXTSEL:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->cursorNext();
		owner->view()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->showState();
		break;
		
	case LFUN_HOMESEL:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->CursorHome(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->showState();
		break;
		
	case LFUN_ENDSEL:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->CursorEnd(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->showState();
		break;
		
	case LFUN_WORDRIGHTSEL:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		if (owner->view()->text->cursor.par()->isRightToLeftPar(owner->buffer()->params))
			owner->view()->text->CursorLeftOneWord(owner->view());
		else
			owner->view()->text->CursorRightOneWord(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->showState();
		break;
		
	case LFUN_WORDLEFTSEL:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		if (owner->view()->text->cursor.par()->isRightToLeftPar(owner->buffer()->params))
			owner->view()->text->CursorRightOneWord(owner->view());
		else
			owner->view()->text->CursorLeftOneWord(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->showState();
		break;
		
	case LFUN_BEGINNINGBUFSEL:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->CursorTop(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->showState();
		break;
		
	case LFUN_ENDBUFSEL:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->CursorBottom(owner->view());
		owner->view()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->showState();
		break;

		// --- text changing commands ------------------------
	case LFUN_BREAKLINE:
		owner->view()->beforeChange();
		owner->view()->text->InsertChar(owner->view(), LyXParagraph::META_NEWLINE);
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		moveCursorUpdate(false);
		break;
		
	case LFUN_PROTECTEDSPACE:
	{
		LyXLayout const & style =
			textclasslist.Style(owner->view()->buffer()->params.textclass,
					    owner->view()->text->cursor.par()->GetLayout());

		if (style.free_spacing) {
			owner->view()->text->InsertChar(owner->view(), ' ');
			owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		} else {
			owner->view()->protectedBlank();
		}
		moveCursorUpdate(false);
	}
	break;
		
	case LFUN_SETMARK:
		if(owner->view()->text->mark_set) {
			owner->view()->beforeChange();
			owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
			setMessage(N_("Mark removed"));
		} else {
			owner->view()->beforeChange();
			owner->view()->text->mark_set = 1;
			owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
			setMessage(N_("Mark set"));
		}
		owner->view()->text->sel_cursor = 
			owner->view()->text->cursor;
		break;
		
	case LFUN_DELETE:
		if (!owner->view()->text->selection) {
			owner->view()->text->Delete(owner->view());
			owner->view()->text->sel_cursor = 
				owner->view()->text->cursor;
			owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
			// It is possible to make it a lot faster still
			// just comment out the lone below...
			owner->view()->showCursor();
		} else {
			owner->view()->cut();
		}
		moveCursorUpdate(false);
		owner->showState();
		owner->view()->setState();
		break;

	case LFUN_DELETE_SKIP:
	{
		// Reverse the effect of LFUN_BREAKPARAGRAPH_SKIP.
		
		LyXCursor cursor = owner->view()->text->cursor;

		if (!owner->view()->text->selection) {
			if (cursor.pos() == cursor.par()->Last()) {
				owner->view()->text->CursorRight(owner->view());
				cursor = owner->view()->text->cursor;
				if (cursor.pos() == 0
				    && !(cursor.par()->added_space_top 
					 == VSpace (VSpace::NONE))) {
					owner->view()->text->SetParagraph
						(owner->view(),
						 cursor.par()->line_top,
						 cursor.par()->line_bottom,
						 cursor.par()->pagebreak_top, 
						 cursor.par()->pagebreak_bottom,
						 VSpace(VSpace::NONE), 
						 cursor.par()->added_space_bottom,
						 cursor.par()->align, 
						 cursor.par()->labelwidthstring, 0);
					owner->view()->text->CursorLeft(owner->view());
					owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
				} else {
					owner->view()->text->CursorLeft(owner->view());
					owner->view()->text->Delete(owner->view());
					owner->view()->text->sel_cursor = 
						owner->view()->text->cursor;
					owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
				}
			} else {
				owner->view()->text->Delete(owner->view());
				owner->view()->text->sel_cursor = 
					owner->view()->text->cursor;
				owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
			}
		} else {
			owner->view()->cut();
		}
	}
	break;

	/* -------> Delete word forward. */
	case LFUN_DELETE_WORD_FORWARD:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->DeleteWordForward(owner->view());
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		moveCursorUpdate(false);
		owner->showState();
		break;

		/* -------> Delete word backward. */
	case LFUN_DELETE_WORD_BACKWARD:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->DeleteWordBackward(owner->view());
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		moveCursorUpdate(false);
		owner->showState();
		break;
		
		/* -------> Kill to end of line. */
	case LFUN_DELETE_LINE_FORWARD:
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->DeleteLineForward(owner->view());
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		moveCursorUpdate(false);
		break;
		
		/* -------> Set mark off. */
	case LFUN_MARK_OFF:
		owner->view()->beforeChange();
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->sel_cursor = 
			owner->view()->text->cursor;
		setMessage(N_("Mark off"));
		break;

		/* -------> Set mark on. */
	case LFUN_MARK_ON:
		owner->view()->beforeChange();
		owner->view()->text->mark_set = 1;
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR);
		owner->view()->text->sel_cursor = 
			owner->view()->text->cursor;
		setMessage(N_("Mark on"));
		break;
		
	case LFUN_BACKSPACE:
	{
		if (!owner->view()->text->selection) {
			if (owner->getIntl()->getTrans()->backspace()) {
				owner->view()->text->Backspace(owner->view());
				owner->view()->text->sel_cursor = 
					owner->view()->text->cursor;
				owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
				// It is possible to make it a lot faster still
				// just comment out the lone below...
				owner->view()->showCursor();
			}
		} else {
			owner->view()->cut();
		}
		owner->showState();
		owner->view()->setState();
	}
	break;

	case LFUN_BACKSPACE_SKIP:
	{
		// Reverse the effect of LFUN_BREAKPARAGRAPH_SKIP.
		
		LyXCursor cursor = owner->view()->text->cursor;
		
		if (!owner->view()->text->selection) {
			if (cursor.pos() == 0 
			    && !(cursor.par()->added_space_top 
				 == VSpace (VSpace::NONE))) {
				owner->view()->text->SetParagraph 
					(owner->view(),
					 cursor.par()->line_top,      
					 cursor.par()->line_bottom,
					 cursor.par()->pagebreak_top, 
					 cursor.par()->pagebreak_bottom,
					 VSpace(VSpace::NONE), cursor.par()->added_space_bottom,
					 cursor.par()->align, 
					 cursor.par()->labelwidthstring, 0);
				owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
			} else {
				owner->view()->text->Backspace(owner->view());
				owner->view()->text->sel_cursor 
					= cursor;
				owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
			}
		} else
			owner->view()->cut();
	}
	break;

	case LFUN_BREAKPARAGRAPH:
	{
		owner->view()->beforeChange();
		owner->view()->text->BreakParagraph(owner->view(), 0);
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		owner->view()->text->sel_cursor = 
			owner->view()->text->cursor;
		owner->view()->setState();
		owner->showState();
		break;
	}

	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
	{
		owner->view()->beforeChange();
		owner->view()->text->BreakParagraph(owner->view(), 1);
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		owner->view()->text->sel_cursor = 
			owner->view()->text->cursor;
		owner->view()->setState();
		owner->showState();
		break;
	}
	
	case LFUN_BREAKPARAGRAPH_SKIP:
	{
		// When at the beginning of a paragraph, remove
		// indentation and add a "defskip" at the top.
		// Otherwise, do the same as LFUN_BREAKPARAGRAPH.
		
		LyXCursor cursor = owner->view()->text->cursor;
		
		owner->view()->beforeChange();
		if (cursor.pos() == 0) {
			if (cursor.par()->added_space_top == VSpace(VSpace::NONE)) {
				owner->view()->text->SetParagraph
					(owner->view(),
					 cursor.par()->line_top,      
					 cursor.par()->line_bottom,
					 cursor.par()->pagebreak_top, 
					 cursor.par()->pagebreak_bottom,
					 VSpace(VSpace::DEFSKIP), cursor.par()->added_space_bottom,
					 cursor.par()->align, 
					 cursor.par()->labelwidthstring, 1);
				//owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
			} 
		}
		else {
			owner->view()->text->BreakParagraph(owner->view(), 0);
			//owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		}

		owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		owner->view()->text->sel_cursor = cursor;
		owner->view()->setState();
		owner->showState();
	}
	break;

	case LFUN_PARAGRAPH_SPACING:
	{
		LyXParagraph * par = owner->view()->text->cursor.par();
		Spacing::Space cur_spacing = par->spacing.getSpace();
		float cur_value = 1.0;
		if (cur_spacing == Spacing::Other) {
			cur_value = par->spacing.getValue();
		}
		
#ifdef HAVE_SSTREAM
		istringstream istr(argument);
#else
		istrstream istr(argument.c_str());
#endif
		string tmp;
		istr >> tmp;
		Spacing::Space new_spacing = cur_spacing;
		float new_value = cur_value;
		if (tmp.empty()) {
			lyxerr << "Missing argument to `paragraph-spacing'"
			       << endl;
		} else if (tmp == "single") {
			new_spacing = Spacing::Single;
		} else if (tmp == "onehalf") {
			new_spacing = Spacing::Onehalf;
		} else if (tmp == "double") {
			new_spacing = Spacing::Double;
		} else if (tmp == "other") {
			new_spacing = Spacing::Other;
			float tmpval = 0.0;
			istr >> tmpval;
			lyxerr << "new_value = " << tmpval << endl;
			if (tmpval != 0.0)
				new_value = tmpval;
		} else if (tmp == "default") {
			new_spacing = Spacing::Default;
		} else {
			lyxerr << _("Unknown spacing argument: ")
			       << argument << endl;
		}
		if (cur_spacing != new_spacing || cur_value != new_value) {
			par->spacing.set(new_spacing, new_value);
			owner->view()->text->RedoParagraph(owner->view());
			owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		}
	}
	break;
	
	case LFUN_QUOTE:
		owner->view()->beforeChange();
		owner->view()->text->InsertChar(owner->view(), '\"');  // This " matches the single quote in the code
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
                moveCursorUpdate(false);
		break;

	case LFUN_HTMLURL:
	case LFUN_URL:
	{
		InsetCommandParams p;
		if (action == LFUN_HTMLURL)
			p.setCmdName("htmlurl");
		else
			p.setCmdName("url");
		owner->getDialogs()->createUrl( p.getAsString() );
	}
	break;
		    
	case LFUN_INSERT_URL:
	{
		InsetCommandParams p;
		p.setFromString( argument );

		InsetUrl * inset = new InsetUrl( p );
		if (!owner->view()->insertInset(inset))
			delete inset;
		else
			owner->view()->updateInset( inset, true );
	}
	break;
		    
	case LFUN_INSET_TEXT:
	{
		InsetText * new_inset = new InsetText;
		if (owner->view()->insertInset(new_inset))
			new_inset->Edit(owner->view(), 0, 0, 0);
		else
			delete new_inset;
	}
	break;
	
	case LFUN_INSET_ERT:
	{
		InsetERT * new_inset = new InsetERT;
		if (owner->view()->insertInset(new_inset))
			new_inset->Edit(owner->view(), 0, 0, 0);
		else
			delete new_inset;
	}
	break;
	
	case LFUN_INSET_EXTERNAL:
	{
		InsetExternal * new_inset = new InsetExternal;
		if (owner->view()->insertInset(new_inset))
			new_inset->Edit(owner->view(), 0, 0, 0);
		else
			delete new_inset;
	}
	break;
	
	case LFUN_INSET_FOOTNOTE:
	{
		InsetFoot * new_inset = new InsetFoot;
		if (owner->view()->insertInset(new_inset))
			new_inset->Edit(owner->view(), 0, 0, 0);
		else
			delete new_inset;
	}
	break;

	case LFUN_INSET_MARGINAL:
	{
		InsetMarginal * new_inset = new InsetMarginal;
		if (owner->view()->insertInset(new_inset))
			new_inset->Edit(owner->view(), 0, 0, 0);
		else
			delete new_inset;
	}
	break;

	case LFUN_INSET_MINIPAGE:
	{
		InsetMinipage * new_inset = new InsetMinipage;
		if (owner->view()->insertInset(new_inset))
			new_inset->Edit(owner->view(), 0, 0, 0);
		else
			delete new_inset;
	}
	break;

	case LFUN_INSET_FLOAT:
	{
		// check if the float type exist
		if (floatList.typeExist(argument)) {
			InsetFloat * new_inset = new InsetFloat(argument);
			if (owner->view()->insertInset(new_inset))
				new_inset->Edit(owner->view(), 0, 0, 0);
			else
				delete new_inset;
		} else {
			lyxerr << "Non-existant float type: "
			       << argument << endl;
		}
		
	}
	break;

	case LFUN_INSET_LIST:
	{
		InsetList * new_inset = new InsetList;
		if (owner->view()->insertInset(new_inset))
			new_inset->Edit(owner->view(), 0, 0, 0);
		else
			delete new_inset;
	}
	break;

	case LFUN_INSET_THEOREM:
	{
		InsetTheorem * new_inset = new InsetTheorem;
		if (owner->view()->insertInset(new_inset))
			new_inset->Edit(owner->view(), 0, 0, 0);
		else
			delete new_inset;
	}
	break;

	case LFUN_INSET_CAPTION:
	{
		// Do we have a locking inset...
		if (owner->view()->the_locking_inset) {
			lyxerr << "Locking inset code: "
			       << static_cast<int>(owner->view()->the_locking_inset->LyxCode());
			InsetCaption * new_inset = new InsetCaption;
			new_inset->setOwner(owner->view()->the_locking_inset);
			new_inset->SetAutoBreakRows(true);
			new_inset->SetDrawFrame(0, InsetText::LOCKED);
			new_inset->SetFrameColor(0, LColor::footnoteframe);
			if (owner->view()->insertInset(new_inset))
				new_inset->Edit(owner->view(), 0, 0, 0);
			else
				delete new_inset;
		}
	}
	break;
	
	case LFUN_INSET_TABULAR:
	{
		int r = 2, c = 2;
		if (!argument.empty())
			sscanf(argument.c_str(),"%d%d", &r, &c);
		InsetTabular * new_inset =
			new InsetTabular(owner->buffer(), r, c);
		if (owner->view()->insertInset(new_inset))
			new_inset->Edit(owner->view(), 0, 0, 0);
		else
			delete new_inset;
	}
	break;

	// --- lyxserver commands ----------------------------

	case LFUN_CHARATCURSOR:
	{
		LyXParagraph::size_type pos = 
			owner->view()->text->cursor.pos();
		if(pos < owner->view()->text->cursor.par()->size())
			//dispatch_buffer = owner->view()->text->
			//	cursor.par()->text[pos];
			dispatch_buffer =
				owner->view()->text->
				cursor.par()->GetChar(pos);
		else
			dispatch_buffer = "EOF";
	}
	break;
	
	case LFUN_GETXY:
		dispatch_buffer = 
			tostr(owner->view()->text->cursor.x()) + ' '
			+ tostr(owner->view()->text->cursor.y());
		break;
		
	case LFUN_SETXY:
	{
		int  x;
		long y;
		sscanf(argument.c_str(), " %d %ld", &x, &y);
		owner->view()->text->SetCursorFromCoordinates(owner->view(), x, y);
	}
	break;
	
	case LFUN_GETLAYOUT:
		dispatch_buffer =  
			tostr(owner->view()->text->cursor.par()->layout);
		break;
			
	case LFUN_GETFONT:
	{
		LyXFont & font = owner->view()->text->current_font;
                if(font.shape() == LyXFont::ITALIC_SHAPE)
			dispatch_buffer = 'E';
                else if(font.shape() == LyXFont::SMALLCAPS_SHAPE)
			dispatch_buffer = 'N';
                else
			dispatch_buffer = '0';

	}
	break;

	case LFUN_GETLATEX:
	{
		LyXFont & font = owner->view()->text->current_font;
                if(font.latex() == LyXFont::ON)
			dispatch_buffer = 'L';
                else
			dispatch_buffer = '0';
	}
	break;

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
		int  row;
		sscanf(argument.c_str(), " %s %d", file_name, &row);

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

	case LFUN_APROPOS:
	case LFUN_GETTIP:
	{
		int qa = lyxaction.LookupFunc(argument.c_str());
		setMessage(lyxaction.helpText(static_cast<kb_action>(qa)));
	}
	break;

	// --- accented characters ---------------------------
		
	case LFUN_UMLAUT:
	case LFUN_CIRCUMFLEX:
	case LFUN_GRAVE:
	case LFUN_ACUTE:
	case LFUN_TILDE:
	case LFUN_CEDILLA:
	case LFUN_MACRON:
	case LFUN_DOT:
	case LFUN_UNDERDOT:
	case LFUN_UNDERBAR:
	case LFUN_CARON:
	case LFUN_SPECIAL_CARON:
	case LFUN_BREVE:
	case LFUN_TIE:
	case LFUN_HUNG_UMLAUT:
	case LFUN_CIRCLE:
	case LFUN_OGONEK:
	{
		char c = 0;
		
		if (keyseq.length == -1 && keyseq.getiso() != 0) 
			c = keyseq.getiso();
		
		owner->getIntl()->getTrans()->
			deadkey(c, get_accent(action).accent, 
				owner->view()->text);
		
		// Need to reset, in case the minibuffer calls these
		// actions
		keyseq.reset();
		keyseq.length = 0;
		
		// copied verbatim from do_accent_char
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);

		owner->view()->text->sel_cursor = 
			owner->view()->text->cursor;
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
	
	case LFUN_MATH_DELIM:     
	case LFUN_INSERT_MATRIX:
	{ 	   
		if (owner->view()->available()) { 
			owner->view()->
				open_new_inset(new InsetFormula(false));
			owner->view()
				->the_locking_inset
				->LocalDispatch(owner->view(),
						action,
						argument);
		}
	}	   
	break;
	       
	case LFUN_INSERT_MATH:
	{
		math_insert_symbol(argument.c_str());
	}
	break;
	
	case LFUN_MATH_DISPLAY:
	{
		if (owner->view()->available())
			owner->view()->open_new_inset(new InsetFormula(true));
		break;
	}
		    
	case LFUN_MATH_MACRO:
	{
		if (owner->view()->available()) {
			string s(argument);
		        if (s.empty())
				setErrorMessage(N_("Missing argument"));
		        else {
				string s1 = token(s, ' ', 1);
				int na = s1.empty() ? 0: atoi(s1.c_str());
				owner->view()->
					open_new_inset(new InsetFormulaMacro(token(s, ' ', 0), na));
			}
		}
	}
	break;

	case LFUN_MATH_MODE:   // Open or create a math inset
	{		
		if (owner->view()->available())
			owner->view()->open_new_inset(new InsetFormula);
		setMessage(N_("Math editor mode"));
	}
	break;
	  
	case LFUN_MATH_NUMBER:
	case LFUN_MATH_LIMITS:
	{
		setErrorMessage(N_("This is only allowed in math mode!"));
	
	}
	break;

	case LFUN_MATH_PANEL:
	{
		show_symbols_form(this);
	}
	break;
	
	case LFUN_CITATION_CREATE:
	{
		// Should do this "at source"
		InsetCommandParams p( "cite" );
		
		if (contains(argument, "|")) {
			p.setContents( token(argument, '|', 0) );
			p.setOptions(  token(argument, '|', 1) );
		} else {
			p.setContents( argument );
		}

		owner->getDialogs()->createCitation( p.getAsString() );
	}
	break;
		    
	case LFUN_CITATION_INSERT:
	{
		InsetCommandParams p;
		p.setFromString( argument );

		InsetCitation * inset = new InsetCitation( p );
		if (!owner->view()->insertInset(inset))
			delete inset;
		else
			owner->view()->updateInset( inset, true );
	}
	break;
		    
	case LFUN_INSERT_BIBTEX:
	{   
		// ale970405+lasgoutt970425
		// The argument can be up to two tokens separated 
		// by a space. The first one is the bibstyle.
		string db       = token(argument, ' ', 0);
		string bibstyle = token(argument, ' ', 1);
		if (bibstyle.empty())
			bibstyle = "plain";

		InsetCommandParams p( "BibTeX", db, bibstyle );
		InsetBibtex * inset = new InsetBibtex(p, owner->buffer());
		
		if (owner->view()->insertInset(inset)) {
			if (argument.empty())
				inset->Edit(owner->view(), 0, 0, 0);
		} else
			delete inset;
	}
	break;
		
	// BibTeX data bases
	case LFUN_BIBDB_ADD:
	{
		InsetBibtex * inset = 
			static_cast<InsetBibtex*>(getInsetByCode(Inset::BIBTEX_CODE));
		if (inset) {
			inset->addDatabase(argument);
		}
	}
	break;
		    
	case LFUN_BIBDB_DEL:
	{
		InsetBibtex * inset = 
			static_cast<InsetBibtex*>(getInsetByCode(Inset::BIBTEX_CODE));
		if (inset) {
			inset->delDatabase(argument);
		}
	}
	break;
	
	case LFUN_BIBTEX_STYLE:
	{
		InsetBibtex * inset = 
			static_cast<InsetBibtex*>(getInsetByCode(Inset::BIBTEX_CODE));
		if (inset) {
			inset->setOptions(argument);
		}
	}
	break;
		
	case LFUN_INDEX_CREATE:
	{
		InsetCommandParams p( "index" );
		
		if( argument.empty() ) {
			// Get the word immediately preceding the cursor
			LyXParagraph::size_type curpos = 
				owner->view()->text->cursor.pos() - 1;

			string curstring;
			if( curpos >= 0 )
				curstring = owner->view()->text
					    ->cursor.par()->GetWord(curpos);

			p.setContents( curstring );
		} else {
			p.setContents( argument );
		}

		owner->getDialogs()->createIndex( p.getAsString() );
	}
	break;
		    
	case LFUN_INDEX_INSERT:
	{
		InsetCommandParams p;
		p.setFromString( argument );
		InsetIndex * inset = new InsetIndex( p );

		if (!owner->view()->insertInset(inset))
			delete inset;
		else
			owner->view()->updateInset( inset, true );
	}
	break;
		    
	case LFUN_INDEX_INSERT_LAST:
	{
		// Get word immediately preceding the cursor
		LyXParagraph::size_type curpos = 
			owner->view()->text->cursor.pos() - 1;
	  	// Can't do that at the beginning of a paragraph
	  	if( curpos < 0 ) break;

		string curstring( owner->view()->text
				  ->cursor.par()->GetWord(curpos) );

		InsetCommandParams p( "index", curstring );
		InsetIndex * inset = new InsetIndex( p );

		if (!owner->view()->insertInset(inset))
			delete inset;
		else
			owner->view()->updateInset( inset, true );
	}
	break;
		    
	case LFUN_INDEX_PRINT:
	{
		InsetCommandParams p( "printindex" );
		Inset * inset = new InsetPrintIndex(p);
		if (!owner->view()->insertInset(inset, "Standard", true))
			delete inset;
	}
	break;

	case LFUN_PARENTINSERT:
	{
		lyxerr << "arg " << argument << endl;
		InsetCommandParams p( "lyxparent", argument );
		Inset * inset = new InsetParent(p, owner->buffer());
		if (!owner->view()->insertInset(inset, "Standard", true))
			delete inset;
	}
	break;

	case LFUN_CHILDINSERT:
	{
		InsetCommandParams p( "Include", argument );
		Inset * inset = new InsetInclude(p, owner->buffer());
		if (owner->view()->insertInset(inset, "Standard", true))
			inset->Edit(owner->view(), 0, 0, 0);
		else
			delete inset;
	}
	break;

	case LFUN_CHILDOPEN:
	{
		string filename =
			MakeAbsPath(argument, 
				    OnlyPath(owner->buffer()->fileName()));
		setMessage(N_("Opening child document ") +
			   MakeDisplayPath(filename) + "...");
		owner->view()->savePosition();
		if (bufferlist.exists(filename))
			owner->view()->buffer(bufferlist.getBuffer(filename));
		else
			owner->view()->buffer(bufferlist.loadLyXFile(filename));
	}
	break;

	case LFUN_INSERT_NOTE:
		owner->view()->insertNote();
		break;
#ifndef NEW_INSETS
	case LFUN_INSERTFOOTNOTE: 
	{
		LyXParagraph::footnote_kind kind;
		if (argument == "footnote")
			{ kind = LyXParagraph::FOOTNOTE; }
		else if (argument == "margin")
			{ kind = LyXParagraph::MARGIN; }
		else if (argument == "figure")
			{ kind = LyXParagraph::FIG; }
		else if (argument == "table")
			{ kind = LyXParagraph::TAB; }
		else if (argument == "wide-fig")
			{ kind = LyXParagraph::WIDE_FIG; }
		else if (argument == "wide-tab")
			{ kind = LyXParagraph::WIDE_TAB; }
		else if (argument == "algorithm")
			{ kind = LyXParagraph::ALGORITHM; }
		else {
			setErrorMessage(N_("Unknown kind of footnote"));
			break;
		}
		owner->view()->text->InsertFootnoteEnvironment(owner->view(), kind);
		owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		owner->view()->setState();
	}
	break;
#endif	
	case LFUN_BUFFERBULLETSSELECT:
		bulletForm();
		break;
		
	case LFUN_TOGGLECURSORFOLLOW:
		cursor_follows_scrollbar = !cursor_follows_scrollbar;
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

	case LFUN_SELFINSERT:
	{
		for (string::size_type i = 0; i < argument.length(); ++i) {
			owner->view()->text->InsertChar(owner->view(), argument[i]);
			// This needs to be in the loop, or else we
			// won't break lines correctly. (Asger)
			owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		}
		owner->view()->text->sel_cursor = 
			owner->view()->text->cursor;
		moveCursorUpdate(false);
	}
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

	case LFUN_DATE_INSERT:  // jdblair: date-insert cmd
	{
		struct tm * now_tm;
		
		time_t now_time_t = time(NULL);
		now_tm = localtime(&now_time_t);
		setlocale(LC_TIME, "");
		string arg;
		if (!argument.empty())
			arg = argument;
		else 
			arg = lyxrc.date_insert_format;
		char datetmp[32];
		int datetmp_len = strftime(datetmp, 32, arg.c_str(), now_tm);
		for (int i = 0; i < datetmp_len; i++) {
			owner->view()->text->InsertChar(owner->view(), datetmp[i]);
			owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		}

		owner->view()->text->sel_cursor = owner->view()->text->cursor;
		moveCursorUpdate(false);
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

	case LFUN_SET_COLOR:
	{
		string lyx_name, x11_name;
		x11_name = split(argument, lyx_name, ' ');
		if (lyx_name.empty() || x11_name.empty()) {
			LyXBell();
			setErrorMessage(N_("Syntax: set-color <lyx_name>"
						" <x11_name>"));
			break;
			}

		if (!lcolor.setColor(lyx_name, x11_name)) {
			static string err1 (N_("Set-color \""));
			static string err2 (N_("\" failed - color is undefined "
						"or may not be redefined"));
			LyXBell();
			setErrorMessage(err1 + lyx_name + err2);
			break;
		}
		lyxColorHandler->updateColor(lcolor.getFromLyXName(lyx_name));
		owner->view()->redraw();
		break;
	}

	case LFUN_UNKNOWN_ACTION:
	{
		if(!owner->buffer()) {
			LyXBell();
			setErrorMessage(N_("No document open"));
			break;
		}

		if (owner->buffer()->isReadonly()) {
			LyXBell();
			setErrorMessage(N_("Document is read only"));
			break;
		}
			 
		if (!argument.empty()) {
			
			/* Automatically delete the currently selected
			 * text and replace it with what is being
			 * typed in now. Depends on lyxrc settings
			 * "auto_region_delete", which defaults to
			 * true (on). */
		
			if ( lyxrc.auto_region_delete ) {
				if (owner->view()->text->selection){
					owner->view()->text->CutSelection(owner->view(), false);
					owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
				}
			}
			
			owner->view()->beforeChange();
			
			for (string::size_type i = 0;
			     i < argument.length(); ++i) {
				if (greek_kb_flag) {
					if (!math_insert_greek(argument[i]))
						owner->getIntl()->getTrans()->TranslateAndInsert(argument[i], owner->view()->text);
				} else
					owner->getIntl()->getTrans()->TranslateAndInsert(argument[i], owner->view()->text);
			}

			owner->view()->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);

			owner->view()->text->sel_cursor = 
				owner->view()->text->cursor;
			moveCursorUpdate(false);
			return string();
		} else {
			// why is an "Unknown action" with empty
			// argument even dispatched in the first
			// place? I`ll probably change that. (Lgb)
			LyXBell();
			setErrorMessage(N_("Unknown action"));
		}
		break;
	default:
		lyxerr << "A truly unknown func!" << endl;
		break;
	}
	} // end of switch
  exit_with_message:

	string res = getMessage();

	if (res.empty()) {
		if (!commandshortcut.empty()) {
			string newbuf = owner->getMiniBuffer()->GetText();
			if (newbuf != commandshortcut) {
				owner->getMiniBuffer()->Set(newbuf
							    + " " +
							    commandshortcut);
			}
		}
	} else {
		owner->getMiniBuffer()->Set(string(_(res.c_str()))
					    + " " + commandshortcut);
	}

	return res;
}


void LyXFunc::setupLocalKeymap()
{
	keyseq.stdmap = keyseq.curmap = toplevel_keymap;
	cancel_meta_seq.stdmap = cancel_meta_seq.curmap = toplevel_keymap;
}


void LyXFunc::MenuNew(bool fromTemplate)
{
	string fname, initpath = lyxrc.document_path;
	LyXFileDlg fileDlg;

	if (owner->view()->available()) {
		string trypath = owner->buffer()->filepath;
		// If directory is writeable, use this as default.
		if (IsDirWriteable(trypath) == 1)
			initpath = trypath;
	}

	static int newfile_number = 0;
	string s;

	if (lyxrc.new_ask_filename) {
		ProhibitInput(owner->view());
		fileDlg.SetButton(0, _("Documents"), lyxrc.document_path);
		fileDlg.SetButton(1, _("Templates"), lyxrc.template_path);
		fname = fileDlg.Select(_("Enter Filename for new document"), 
				       initpath, "*.lyx", _("newfile"));
		AllowInput(owner->view());
	
		if (fname.empty()) {
			owner->getMiniBuffer()->Set(_("Canceled."));
			lyxerr.debug() << "New Document Cancelled." << endl;
			return;
		}
	
		// get absolute path of file and make sure the filename ends
		// with .lyx
		s = MakeAbsPath(fname);
		if (!IsLyXFilename(s))
			s += ".lyx";

		// Check if the document already is open
		if (bufferlist.exists(s)) {
			switch(AskConfirmation(_("Document is already open:"), 
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
				owner->getMiniBuffer()->Set(_("Canceled."));
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
				owner->getMiniBuffer()->Set(_("Opening document"), 
							    MakeDisplayPath(s), "...");
				XFlush(fl_display);
				owner->view()->buffer(
					bufferlist.loadLyXFile(s));
				owner->getMiniBuffer()->Set(_("Document"),
							    MakeDisplayPath(s),
							    _("opened."));
				return;
			}
		}
	} else {
		s = lyxrc.document_path + "newfile" + tostr(++newfile_number);
		FileInfo fi(s);
		while (bufferlist.exists(s) || fi.readable()) {
			++newfile_number;
			s = lyxrc.document_path + "newfile" +
				tostr(newfile_number);
			fi.newFile(s);
		}
	}

	// The template stuff
	string templname;
	if (fromTemplate) {
		ProhibitInput(owner->view());
		fname = fileDlg.Select(_("Choose template"),
				       lyxrc.template_path,
				       "*.lyx");
                templname = fname;
		AllowInput(owner->view());
	}
  
	// find a free buffer
	lyxerr.debug() << "Find a free buffer." << endl;
	owner->view()->buffer(bufferlist.newFile(s, templname));
}


void LyXFunc::MenuOpen()
{
	string initpath = lyxrc.document_path;
	LyXFileDlg fileDlg;
  
	if (owner->view()->available()) {
		string trypath = owner->buffer()->filepath;
		// If directory is writeable, use this as default.
		if (IsDirWriteable(trypath) == 1)
			initpath = trypath;
	}

	// launches dialog
	ProhibitInput(owner->view());
	fileDlg.SetButton(0, _("Documents"), lyxrc.document_path);
	fileDlg.SetButton(1, _("Examples"), 
			  AddPath(system_lyxdir, "examples"));
	string filename = fileDlg.Select(_("Select Document to Open"),
					 initpath, "*.lyx");
	AllowInput(owner->view());
 
	// check selected filename
	if (filename.empty()) {
		owner->getMiniBuffer()->Set(_("Canceled."));
		return;
	}

	// get absolute path of file and make sure the filename ends
	// with .lyx
	filename = MakeAbsPath(filename);
	if (!IsLyXFilename(filename))
		filename += ".lyx";

	// loads document
	owner->getMiniBuffer()->Set(_("Opening document"),
				    MakeDisplayPath(filename), "...");
	Buffer * openbuf = bufferlist.loadLyXFile(filename);
	if (openbuf) {
		owner->view()->buffer(openbuf);
		owner->getMiniBuffer()->Set(_("Document"),
					    MakeDisplayPath(filename),
					    _("opened."));
	} else {
		owner->getMiniBuffer()->Set(_("Could not open document"),
					    MakeDisplayPath(filename));
	}
}

// returns filename if file must be imported,
// empty string if either file not found or already loaded
// checks for running without gui are missing.

void LyXFunc::doImportHelper(
	string const & file,          // filename (possibly empty)
	string const & text,          // info when asking for filename
	string const & pattern,       // filetype
	bool func(BufferView *, string const &)     // the real import function
)
{
	string filename = file;

	if (filename.empty()) { // need user interaction
		string initpath = lyxrc.document_path;
		LyXFileDlg fileDlg;
		
		if (owner->view()->available()) {
			string trypath = owner->buffer()->filepath;
			// If directory is writeable, use this as default.
			if (IsDirWriteable(trypath) == 1)
				initpath = trypath;
		}

		// launches dialog
		ProhibitInput(owner->view());
		fileDlg.SetButton(0, _("Documents"), lyxrc.document_path);
		fileDlg.SetButton(1, _("Examples"), 
					AddPath(system_lyxdir, "examples"));
		filename = fileDlg.Select(text, initpath, pattern);
		AllowInput(owner->view());
 
		// check selected filename
		if (filename.empty()) 
			owner->getMiniBuffer()->Set(_("Canceled."));
	}

	// still no filename? abort
	if (filename.empty()) 
		return;

	// get absolute path of file
	filename = MakeAbsPath(filename);

	string lyxfile = ChangeExtension(filename, ".lyx");

	// Check if the document already is open
	if (bufferlist.exists(lyxfile)) {
		switch(AskConfirmation(_("Document is already open:"), 
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
				owner->getMiniBuffer()->Set(_("Canceled."));
				return;
			}
	}

	// Check if a LyX document by the same root exists in filesystem
	FileInfo f(lyxfile, true);
	if (f.exist() && !AskQuestion(_("A document by the name"), 
				      MakeDisplayPath(lyxfile),
				      _("already exists. Overwrite?"))) {
		owner->getMiniBuffer()->Set(_("Canceled."));
		return;
	}
	// filename should be valid now

	// notify user of import ahead
	string displaypath = MakeDisplayPath(filename);
	owner->getMiniBuffer()->Set(_("Importing"), displaypath, "...");

	// call real importer
	bool result = func(owner->view(), filename);

	// we are done
	if (result)
		owner->getMiniBuffer()->Set(displaypath, _("imported."));
	else
		owner->getMiniBuffer()->Set(displaypath, _(": import failed."));
}

static
bool doImportASCIIasLines(BufferView * view, string const & filename)
{
	view->buffer(bufferlist.newFile(filename, string()));
	InsertAsciiFile(view, filename, false);
	return true;
}

static
bool doImportASCIIasParagraphs(BufferView * view, string const & filename)
{
	view->buffer(bufferlist.newFile(filename, string()));
	InsertAsciiFile(view, filename, true);
	return true;
}

static
bool doImportLaTeX(BufferView * view, string const & filename)
{
	ImportLaTeX myImport(filename);
	Buffer * openbuf = myImport.run();
	if (openbuf) { 
		view->buffer(openbuf);
		return true;
	}
	else
		return false;
}

static
bool doImportNoweb(BufferView * view, string const & filename)
{
	ImportNoweb myImport(filename);
	Buffer * openbuf = myImport.run();
	if (openbuf) { 
		view->buffer(openbuf);
		return true;
	}
	else
		return false;
}

static
bool doImportLinuxDoc(BufferView *, string const & filename)
{
	// run sgml2lyx
	string tmp = lyxrc.linuxdoc_to_lyx_command + filename;
	Systemcalls one;
	Buffer * buf = 0;

	int result = one.startscript(Systemcalls::System, tmp);
	if (result == 0) {
		string filename = ChangeExtension(filename, ".lyx");
		// File was generated without problems. Load it.
		buf = bufferlist.loadLyXFile(filename);
	}

	return result == 0;
}


void LyXFunc::MenuInsertLyXFile(string const & filen)
{
	string filename = filen;

	if (filename.empty()) {
		// Launch a file browser
		string initpath = lyxrc.document_path;
		LyXFileDlg fileDlg;

		if (owner->view()->available()) {
			string trypath = owner->buffer()->filepath;
			// If directory is writeable, use this as default.
			if (IsDirWriteable(trypath) == 1)
				initpath = trypath;
		}

		// launches dialog
		ProhibitInput(owner->view());
		fileDlg.SetButton(0, _("Documents"), lyxrc.document_path);
		fileDlg.SetButton(1, _("Examples"), 
				  AddPath(system_lyxdir, "examples"));
		filename = fileDlg.Select(_("Select Document to Insert"),
					  initpath, "*.lyx");
		AllowInput(owner->view());

		// check selected filename
		if (filename.empty()) {
			owner->getMiniBuffer()->Set(_("Canceled."));
			return;
		}
	} 

	// get absolute path of file and make sure the filename ends
	// with .lyx
	filename = MakeAbsPath(filename);
	if (!IsLyXFilename(filename))
		filename += ".lyx";

	// Inserts document
	owner->getMiniBuffer()->Set(_("Inserting document"),
				    MakeDisplayPath(filename), "...");
	bool res = owner->view()->insertLyXFile(filename);
	if (res) {
		owner->getMiniBuffer()->Set(_("Document"),
					    MakeDisplayPath(filename),
					    _("inserted."));
	} else {
		owner->getMiniBuffer()->Set(_("Could not insert document"),
					    MakeDisplayPath(filename));
	}
}

void LyXFunc::doImport(string const & argument)
{
	string type;
	string filename = split(argument, type, ' ');
	lyxerr.debug() << "LyXFunc::doImport: " << type 
		       << " file: " << filename << endl;

	if (type == "latex") 
		doImportHelper(filename,
			       _("Select LaTeX file to import"), "*.tex", 
			       doImportLaTeX);
	else if (type == "ascii") 
		doImportHelper(filename,
			       _("Select ASCII file to import"), "*.txt", 
			       doImportASCIIasLines);
	else if (type == "asciiparagraph") 
		doImportHelper(filename,
			       _("Select ASCII file to import"), "*.txt", 
			       doImportASCIIasParagraphs);
	else if (type == "noweb") 
		doImportHelper(filename,
			       _("Select NoWeb file to import"), "*.nw", 
			       doImportNoweb);
	else if (type == "linuxdoc") 
		doImportHelper(filename,
			       _("Select LinuxDoc file to import"), "*.doc", 
			       doImportLinuxDoc);
	else 
		setErrorMessage(string(N_("Unknown import type: ")) + type);
}

void LyXFunc::reloadBuffer()
{
	string fn = owner->buffer()->fileName();
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
		}
		else {
			owner->view()->buffer(bufferlist.first());
		}
	}
}


Inset * LyXFunc::getInsetByCode(Inset::Code code)
{
	LyXCursor cursor = owner->view()->text->cursor;
	Buffer * buffer = owner->view()->buffer();
	for (Buffer::inset_iterator it = Buffer::inset_iterator(cursor.par(),
								cursor.pos());
	     it != buffer->inset_iterator_end(); ++it) {
		if ((*it)->LyxCode() == code)
			return *it;
	}
	return 0;
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
