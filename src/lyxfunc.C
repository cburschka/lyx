/* This file is part of
* ======================================================
* 
*           LyX, The Document Processor
* 	 
*	    Copyright (C) 1995 Matthias Ettrich
*           Copyright (C) 1995-1998 The LyX Team.
*
*======================================================*/

#include <config.h>

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "definitions.h"
#include "lyxlookup.h"
#include "kbmap.h"
#include "lyxfunc.h"
#include "bufferlist.h"
#include "lyxserver.h"
#include "lyx.h"
#include "intl.h"
#include "lyx_main.h"
#include "lyx_cb.h"
#include "insets/insetlatex.h"
#include "insets/inseturl.h"
#include "insets/insetlatexaccent.h"
#include "insets/insettoc.h"
#include "insets/insetlof.h"
#include "insets/insetloa.h"
#include "insets/insetlot.h"
#include "insets/insetref.h"
#include "insets/insetparent.h"
#include "mathed/formulamacro.h"
#include "toolbar.h"
#include "spellchecker.h" // RVDK_PATCH_5
#include "minibuffer.h"
#include "vspace.h"
#include "insets/insetbib.h"
#include "LyXView.h"
#include "insets/insetindex.h"
#include "insets/insetinclude.h"
#include "filedlg.h"
#include "lyx_gui_misc.h"
#include "filetools.h"
#include "FileInfo.h"
#include "lyxscreen.h"
#include "error.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "gettext.h"
#include "trans_mgr.h"
#include "ImportLaTeX.h"
#include "ImportNoweb.h"

// 	$Id: lyxfunc.C,v 1.1 1999/09/27 18:44:37 larsbj Exp $	

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: lyxfunc.C,v 1.1 1999/09/27 18:44:37 larsbj Exp $";
#endif /* lint */

extern bool cursor_follows_scrollbar;

extern void InsertAsciiFile(LString const &, bool);
extern void math_insert_symbol(char const*);
extern Bool math_insert_greek(char const); // why "Bool"?
extern BufferList bufferlist;
extern LyXServer *lyxserver;
extern short greek_kb_flag;
extern FD_form_toc *fd_form_toc;
extern bool selection_possible;

extern kb_keymap *toplevel_keymap;

extern void BeforeChange();
extern void MenuWrite(Buffer*);
extern void MenuWriteAs(Buffer*);
extern int  MenuRunLaTeX(Buffer*);
extern int  MenuBuildProg(Buffer*);
extern int  MenuRunChktex(Buffer*);
extern bool MenuRunDvips(Buffer *,bool);
extern void MenuPrint(Buffer*);
extern void MenuSendto();
extern void QuitLyX();
extern void MenuFax(Buffer *);
extern void MenuMakeLaTeX(Buffer *);
extern void MenuMakeLinuxDoc(Buffer *);
extern void MenuMakeDocBook(Buffer *);
extern void MenuMakeAscii(Buffer *);
extern void MenuPasteSelection(char at);
extern LyXAction lyxaction;
// (alkis)
extern tex_accent_struct get_accent(kb_action action);

extern void AutoSave();
extern void MenuSearch();
extern void CopyCB();
extern void CopyEnvironmentCB();
extern void PasteEnvironmentCB();
extern void GotoNote();
extern void NoteCB();
extern void OpenStuff();
extern void HyphenationPoint();
extern void Ldots();
extern void EndOfSentenceDot();
extern void MenuSeparator();
extern void HFill();
extern void MenuUndo();
extern void MenuRedo();
extern void SmallUpdate(signed char);
extern void SetUpdateTimer(float timer= 0.3);
extern void FreeUpdateTimer();
extern bool MenuPreview(Buffer*);
extern bool MenuPreviewPS(Buffer*);
extern void MenuInsertLabel(const char *);
extern void MenuInsertRef();
extern void MenuLayoutCharacter();
extern void MenuLayoutParagraph();
extern void MenuLayoutDocument();
extern void MenuLayoutPaper();
extern void MenuLayoutTable(int flag);
extern void MenuLayoutQuotes();
extern void MenuLayoutPreamble();
extern void MenuLayoutSave();
extern void bulletForm();

extern Buffer * NewLyxFile(LString const &);
extern void LoadLyXFile(LString const &);
extern void Reconfigure();

extern int current_layout;
extern int getISOCodeFromLaTeX(char *);

extern int UnlockInset(UpdatableInset*);

extern void ShowLatexLog();

extern void UpdateInset(Inset* inset, bool mark_dirty = true);

/* === globals =========================================================== */

bool LyXFunc::show_sc = true;


LyXFunc::LyXFunc(LyXView *o)
	:owner(o)
{
	meta_fake_bit = 0;
	lyx_dead_action = LFUN_NOACTION;
	lyx_calling_dead_action = LFUN_NOACTION;
	setupLocalKeymap();
}


LyXFunc::~LyXFunc()
{
}


LString LyXFunc::argAsString(char const *const argument)
{
	LString tmp(argument);

	if (tmp.empty()) {
		// get the arg from somewhere else, a popup, or ask for
		// it in the minibuffer.
	}
	lyxerr.debug("argAsString: <" + tmp + '>');
	return tmp;
}


// I changed this func slightly. I commented out the ...FinishUndo(),
// this means that all places that used to have a moveCursorUpdate, now
// have a ...FinishUndo() as the preceeding statement. I have also added
// a moveCursorUpdate to some of the functions that updated the cursor, but
// that did not show its new position.
inline
void LyXFunc::moveCursorUpdate(bool selecting)
{
	if (selecting || owner->currentBuffer()->text->mark_set) {
		owner->currentBuffer()->text->SetSelection();
		owner->currentView()->getScreen()->ToggleToggle();
		owner->currentBuffer()->update(0);
	} else {
		owner->currentBuffer()->update(-2); // this IS necessary
		// (Matthias) 
	}
	owner->currentView()->getScreen()->ShowCursor();
	
	/* ---> Everytime the cursor is moved, show the current font state. */
	// should this too me moved out of this func?
	//owner->getMiniBuffer()->Set(CurrentState());
}


int LyXFunc::processKeyEvent(XEvent *ev)
{
	char s_r[10];
	s_r[9] = '\0';
	int num_bytes;
	int action; 
	char *argument = 0;
	XKeyEvent *keyevent = &ev->xkey;
	KeySym keysym_return;

	num_bytes = LyXLookupString(ev, s_r, 10, &keysym_return);

	if (lyxerr.debugging(Error::KEY)) {
		lyxerr.print(LString("KeySym is ")
			     + XKeysymToString(keysym_return)
			     + "["
			     + int(keysym_return) + "]"
			     + " and num_bytes is "
			     + num_bytes
			     + " the string returned is \""
			     + LString(s_r) + '\"');
	}
	// Do nothing if we have nothing (JMarc)
	if (num_bytes == 0 && keysym_return == NoSymbol) {
		lyxerr.debug("Empty kbd action (probably composing)",
			     Error::KEY);
		return 0;
	}
	
	// this function should be used always [asierra060396]
	if (owner->currentView()->available() &&
	    owner->currentBuffer()->the_locking_inset &&
	    keysym_return == XK_Escape) {
		UnlockInset(owner->currentBuffer()->the_locking_inset);
		owner->currentBuffer()->text->CursorRight();
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

	action = cancel_meta_seq.addkey(keysym_return, keyevent->state
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

	if (lyxerr.debugging(Error::KEY)) {
		char buf[100];
		keyseq.print(buf,100);
		lyxerr.print(LString("Key [")
			     + int(action) + "]["
			     + buf + "]["
			     + num_bytes +"]");
	}

	// already here we know if it any point in going further
	// why not return already here if action == -1 and
	// num_bytes == 0? (Lgb)

	if(keyseq.length>1 || keyseq.length<-1){
		char buf[100];
		keyseq.print(buf,100);
		owner->getMiniBuffer()->Set(buf);
	}

	if (action==-1) {
		if (keyseq.length<-1) { // unknown key sequence...
			char buf[100];
			LyXBell();
			keyseq.print(buf,100);
			owner->getMiniBuffer()->Set(_("Unknown sequence:"),buf);
			return 0;
		}
	
		char isochar = keyseq.getiso();
		if (!(keyevent->state&ControlMask) &&
		    !(keyevent->state&Mod1Mask) &&
		    (isochar && keysym_return < 0xF000)) {
			argument = &s_r[0]; // shall this be here
			argument[0] = isochar;
			argument[1] = 0;
		}
		if (!argument) {
			lyxerr.debug("Empty argument!");
			// This can`t possibly be of any use
			// so we`ll skip the dispatch.
			return 0;
		}
	} else
		if (action==LFUN_SELFINSERT) {
			argument = &s_r[0];
			argument[1] = 0;
		}
    
        bool tmp_sc = show_sc;
	show_sc = false;
	Dispatch(action, argument);
	show_sc = tmp_sc;
	
	// Need this for deadkeys (alkis)
	//keyseq.length=0;
	// ...but that breaks the minibuffer's display of "M-"...
#ifdef WITH_WARNINGS
#warning How does the deadkeys react to this?
#endif
	return 0;
} 


LString LyXFunc::Dispatch(LString const &cmd, LString const &arg)
{
	return Dispatch(lyxaction.LookupFunc(cmd.c_str()),
			arg.c_str());
}


LString LyXFunc::Dispatch(int ac,
			  char const *do_not_use_this_arg)
{
	char const * argument = 0;
	kb_action action;
        

	FL_OBJECT *ob = 0;  // This will disapear soon
    
        // we have not done anything wrong yet.
        errorstat = false;
	dispatch_buffer = LString();
	
	// if action is a pseudo-action, we need the real action
	if (lyxaction.isPseudoAction(ac)) {
		char const *tmparg = 0;
		action = (kb_action)lyxaction.retrieveActionArg(ac, &tmparg);
		if (tmparg)
			argument = tmparg;
	} else {
		action = (kb_action)ac;
		argument = do_not_use_this_arg; // except here
	}
    
	selection_possible = false;
	
	if (owner->currentView()->available() 
	    && owner->currentView()->getScreen())
		owner->currentView()->getScreen()->HideCursor();

	if(!owner->currentView()->available()) {
		// This lists the allowed funcs when we have no
		// buffer loaded
		switch(action){
		case LFUN_MENU_OPEN_BY_NAME:
		case LFUN_PREFIX:
		case LFUN_MENUNEW:
		case LFUN_MENUNEWTMPLT:
		case LFUN_MENUOPEN:
		case LFUN_QUIT:
		case LFUN_PUSH_TOOLBAR:
		case LFUN_ADD_TO_TOOLBAR:
		case LFUN_EXEC_COMMAND:
		case LFUN_DROP_LAYOUTS_CHOICE:
		case LFUN_FILE_NEW:
		case LFUN_FILE_OPEN:
		case LFUN_IMPORT:
		case LFUN_RECONFIGURE:
		case LFUN_CANCEL:
		case LFUN_APROPOS:
		case LFUN_META_FAKE:

			break;
		case LFUN_UNKNOWN_ACTION:
			setErrorMessage(N_("Unknown action"));
			goto exit_with_message;
		default:
			setErrorMessage(N_("Command not allowed with"
					"out any document open"));
			goto exit_with_message;  // action not allowed
		}
	}
	
	commandshortcut = LString();
	
	if (lyxrc->display_shortcuts && show_sc) {
		if (action != LFUN_SELFINSERT) {
			// Put name of command and list of shortcuts
			// for it in minibuffer
			LString comname = lyxaction.getActionName(action);

			int pseudoaction = action;
			bool argsadded = false;

			LString argu = argument;
			if (!argu.empty()) {
				// If we have the command with argument, 
				// this is better
				pseudoaction = 
					lyxaction.searchActionArg(action,
							  	  argument);

				if (pseudoaction == -1) {
					pseudoaction = action;
				} else {
					comname += " " + argu;
					argsadded = true;
				}
			}

			LString shortcuts = toplevel_keymap->findbinding(pseudoaction);

			if (!shortcuts.empty()) {
				comname += ": " + shortcuts;
			} else if (!argsadded) {
				comname += " " + argu;
			}

			if (!comname.empty()) {
				comname.strip();
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

        // Now that we know which action, if the buffer is RO let's check 
	// whether the action is legal.  Alejandro 970603
        if (owner->currentView()->available() && 
            owner->currentBuffer()->isReadonly() && 
            lyxaction.isFuncRO(action)) {
		setErrorMessage(N_("Document is read-only"));
		lyxerr.debug("Error: Document is read-only.");
		goto exit_with_message;
	}

	// If in math mode pass the control to
	// the math inset [asierra060396]
	if (owner->currentView()->available() &&
	    owner->currentBuffer()->the_locking_inset) {
		if (action>1 || (action==LFUN_UNKNOWN_ACTION && keyseq.length>=-1)) {
		        char s[8]="";
			if (action==LFUN_UNKNOWN_ACTION && !argument) {
				sprintf(s, "%c", keyseq.getiso());
				argument = &s[0];
			}
			// Undo/Redo pre 0.13 is a bit tricky for insets.		    
		        if (action==LFUN_UNDO) {
				int slx, sly;
				UpdatableInset* inset =
					owner->currentBuffer()->the_locking_inset;
				inset->GetCursorPos(slx, sly);
				UnlockInset(inset);
				MenuUndo();
				inset = (UpdatableInset*)owner->currentBuffer()->text->cursor.par->GetInset(owner->currentBuffer()->text->cursor.pos);
				if (inset) 
					inset->Edit(slx, sly);
				return LString();
			} else 
				if (action==LFUN_REDO) {
					int slx, sly;
					UpdatableInset* inset = owner->currentBuffer()->the_locking_inset;
					inset->GetCursorPos(slx, sly);
					UnlockInset(inset);
					MenuRedo();
					inset = (UpdatableInset*)owner->currentBuffer()->text->cursor.par->GetInset(owner->currentBuffer()->text->cursor.pos);
					if (inset)
						inset->Edit(slx, sly);
					return LString();
				} else
					if (owner->currentBuffer()->the_locking_inset->LocalDispatch(action, argument))
						return LString();
					else {
						setMessage(N_("Text mode"));
						if (action==LFUN_RIGHT || action==-1)
							owner->currentBuffer()->text->CursorRight();
						if (action==LFUN_LEFT || action==LFUN_RIGHT)
							return LString();
					}
		}
	}

	switch(action) {
		// --- Misc -------------------------------------------
	case LFUN_WORDFINDFORWARD  : 
	case LFUN_WORDFINDBACKWARD : { 
		LyXText *ltCur;
		static LString last_search;
		LString searched_string;
	    
		LString arg = argAsString(argument);
			
		if (!arg.empty()) {
			last_search = arg;
			searched_string = arg;
		} else {
			searched_string = last_search;
		}

		ltCur =	 owner->currentView()->currentBuffer()->text ;	 

		if (!searched_string.empty() &&
		    (	 (action == LFUN_WORDFINDBACKWARD) ? 
			 ltCur->SearchBackward( searched_string.c_str() ) :
			 ltCur->SearchForward(	searched_string.c_str() ) 
			 )){

			// ??? What is that ???
			owner->currentView()->currentBuffer()->update(-2);

			// ??? Needed ???
			// clear the selection (if there is any) 
			owner->currentView()->getScreen()->ToggleSelection();
			owner->currentView()->currentBuffer()->text->ClearSelection();

			// Move cursor so that successive C-s 's will not stand in place. 
			if( action == LFUN_WORDFINDFORWARD ) 
				owner->currentBuffer()->text->CursorRightOneWord();
			owner->currentBuffer()->text->FinishUndo();
			moveCursorUpdate(false);

			// ??? Needed ???
			// set the new selection 
			// SetSelectionOverLenChars(owner->currentView()->currentBuffer()->text, iLenSelected);
			owner->currentView()->getScreen()->ToggleSelection(false);
		} else 
			LyXBell();	
	 
		// REMOVED : if (owner->currentView()->getWorkArea()->focus)
		owner->currentView()->getScreen()->ShowCursor();
	}
	break;

	case LFUN_PREFIX:
	{
		if (owner->currentView()->available()
		    && owner->currentView()->getScreen()) {
			owner->currentBuffer()->update(-2);
		}
		char buf[100];
		keyseq.print(buf,100, true);
		owner->getMiniBuffer()->Set(buf, LString(), LString(), 1);
	}
	break;

	// --- Misc -------------------------------------------
	case LFUN_EXEC_COMMAND:
		owner->getMiniBuffer()->ExecCommand(); 
		break;
		
	case LFUN_CANCEL:                   // RVDK_PATCH_5
		keyseq.reset();
		meta_fake_bit = 0;
		if(owner->currentView()->available())
			// cancel any selection
			Dispatch(int(LFUN_MARK_OFF), NULL);
		setMessage(N_("Cancel"));
		break;

	case LFUN_META_FAKE:                                 // RVDK_PATCH_5
	{
		meta_fake_bit = Mod1Mask;
		char buf[100];
		keyseq.print(buf, 98, true);
		LString res = LString("M-") + buf;
		setMessage(buf); // RVDK_PATCH_5
	}
	break;  

	case LFUN_READ_ONLY_TOGGLE:
		if (owner->currentBuffer()->lyxvc.inUse()) {
			owner->currentBuffer()->lyxvc.toggleReadOnly();
		} else {
			owner->currentBuffer()->setReadonly(
				!owner->currentBuffer()->isReadonly());
		}
		break;
		
	case LFUN_CENTER: // this is center and redraw.
		BeforeChange();
		if (owner->currentBuffer()->text->cursor.y >
		    owner->currentView()->getWorkArea()->h / 2)	{
			owner->currentView()->getScreen()->
				Draw(owner->currentBuffer()->text->cursor.y -
				     owner->currentView()->getWorkArea()->h/2);
		} else { // <=
			owner->currentView()->getScreen()->
				Draw(0);
		}
		owner->currentBuffer()->update(0);
		owner->currentView()->redraw();
		break;
		
	case LFUN_APPENDIX:
		if (owner->currentView()->available()) {
			owner->currentBuffer()->text->toggleAppendix();
			owner->currentBuffer()->update(1);
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
		MenuWrite(owner->currentBuffer());
		break;
		
	case LFUN_MENUWRITEAS:
		MenuWriteAs(owner->currentBuffer());
		break;
		
	case LFUN_MENURELOAD:
		reloadBuffer();
		break;
		
	case LFUN_PREVIEW:
		MenuPreview(owner->currentBuffer());
		break;
			
	case LFUN_PREVIEWPS:
		MenuPreviewPS(owner->currentBuffer());
		break;
		
	case LFUN_RUNLATEX:
		MenuRunLaTeX(owner->currentBuffer());
		break;
		
        case LFUN_BUILDPROG:
                MenuBuildProg(owner->currentBuffer());
                break;
                
 	case LFUN_RUNCHKTEX:
		MenuRunChktex(owner->currentBuffer());
		break;
		
	case LFUN_RUNDVIPS:
		MenuRunDvips(owner->currentBuffer(), false);
		break;
		
	case LFUN_MENUPRINT:
		MenuPrint(owner->currentBuffer());
		break;
		
	case LFUN_FAX:
		MenuFax(owner->currentBuffer());
		break;
			
	case LFUN_EXPORT:
	{
		//needs argument as string
		LString extyp=argAsString(argument);
		
		// latex
		if (extyp == "latex") {
			// make sure that this buffer is not linuxdoc
			MenuMakeLaTeX(owner->currentBuffer());
		}
		// linuxdoc
		else if (extyp == "linuxdoc") {
			// make sure that this buffer is not latex
			MenuMakeLinuxDoc(owner->currentBuffer());
		}
		// docbook
		else if (extyp == "docbook") {
			// make sure that this buffer is not latex or linuxdoc
			MenuMakeDocBook(owner->currentBuffer());
		}
		// dvi
		else if (extyp == "dvi") {
			// Run LaTeX as "Update dvi..." Bernhard.
			// We want the dvi in the current directory. This
			// is achieved by temporarily disabling use of
			// temp directory. As a side-effect, we get
			// *.log and *.aux files also. (Asger)
			bool flag = lyxrc->use_tempdir;
			lyxrc->use_tempdir = false;
			MenuRunLaTeX(owner->currentBuffer());
			lyxrc->use_tempdir = flag;
		}
		// postscript
		else if (extyp == "postscript") {
			// Start Print-dialog. Not as good as dvi... Bernhard.
			MenuPrint(owner->currentBuffer());
			// Since the MenuPrint is a pop-up, we can't use
			// the same trick as above. (Asger)
			// MISSING: Move of ps-file :-(
		}
		// ascii
		else if (extyp == "ascii") {
			MenuMakeAscii(owner->currentBuffer());
		}
		else if (extyp == "custom") {
			MenuSendto();
			break;
		}
		else {
			setErrorMessage(LString(N_("Unknown export type: "))
					+ extyp);
		}
	}
	break;

	case LFUN_IMPORT:
	{
		//needs argument as string
		LString imtyp=argAsString(argument);
		
		// latex
		if (imtyp == "latex") {
			doImportLaTeX(false);
		}
		// ascii
		else if (imtyp == "ascii") {
			doImportASCII(false);
		} else if (imtyp == "asciiparagraph") {
			doImportASCII(true);
		// noweb
		} else if (imtyp == "noweb") {
			doImportLaTeX(true);
		} else {
			setErrorMessage(LString(N_("Unknown import type: "))
					+ imtyp);
		}
		break;
	}
		
	case LFUN_QUIT:
		QuitLyX();
		break;
		
	case LFUN_TOCVIEW:
		TocUpdateCB(ob, 0);
		if (fd_form_toc->form_toc->visible) {
			fl_raise_form(fd_form_toc->form_toc);
		} else {
			fl_show_form(fd_form_toc->form_toc,
				     FL_PLACE_MOUSE |
				     FL_FREE_SIZE, FL_FULLBORDER,
				     _("Table of Contents"));
		}
		break;
		
	case LFUN_TOC_INSERT:
	{
		Inset *new_inset =
			new InsetTOC(owner->currentBuffer());
		owner->currentBuffer()->insertInset(new_inset,
						    "Standard", true);
		break;
	}
	
	case LFUN_LOF_INSERT:
	{
		Inset *new_inset =
			new InsetLOF(owner->currentBuffer());
		owner->currentBuffer()->insertInset(new_inset,
						    "Standard", true);
		break;
	}
	
	case LFUN_LOA_INSERT:
	{
		Inset *new_inset =
			new InsetLOA(owner->currentBuffer());
		owner->currentBuffer()->insertInset(new_inset,
						    "Standard", true);
		break;
	}

	case LFUN_LOT_INSERT:
	{
		Inset *new_inset =
			new InsetLOT(owner->currentBuffer());
		owner->currentBuffer()->insertInset(new_inset,
						    "Standard", true);
		break;
	}
		
	case LFUN_TABLE:
		TableCB(ob, 0);
		break;
		
	case LFUN_FIGURE:
		FigureCB(ob, 0);
		break;
		
	case LFUN_AUTOSAVE:
		AutoSave();
		break;
		
	case LFUN_UNDO:
		MenuUndo();
		break;
		
	case LFUN_REDO:
		MenuRedo();
		break;
		
	case LFUN_MENUSEARCH:
		MenuSearch();
		break;
		
	case LFUN_PASTE:
		PasteCB();
		break;
		
	case LFUN_PASTESELECTION:
	{
	        bool asPara = false;
		if (LString(argument) == "paragraph") asPara = true;
		MenuPasteSelection(asPara);
		break;
	}

	case LFUN_CUT:
		CutCB();
		break;
		
	case LFUN_COPY:
		CopyCB();
		break;
		
	case LFUN_LAYOUT_COPY:
		CopyEnvironmentCB();
		break;
		
	case LFUN_LAYOUT_PASTE:
		PasteEnvironmentCB();
		break;
		
	case LFUN_GOTOERROR:
		owner->currentView()->gotoError();
		break;
		
	case LFUN_REMOVEERRORS:
		if (owner->currentBuffer()->removeAutoInsets()) {
			owner->currentView()->redraw();
			owner->currentView()->fitCursor();
			owner->currentView()->updateScrollbar();
		}
		break;
		
	case LFUN_GOTONOTE:
		GotoNote();
		break;
		
	case LFUN_OPENSTUFF:
		OpenStuff();
		break;
		
	case LFUN_HYPHENATION:
		HyphenationPoint();
		break;
		
	case LFUN_LDOTS:
		Ldots();
		break;
		
	case LFUN_END_OF_SENTENCE:
		EndOfSentenceDot();
		break;

	case LFUN_MENU_SEPARATOR:
		MenuSeparator();
		break;
		
	case LFUN_HFILL:
		HFill();
		break;
		
	case LFUN_DEPTH:
		DepthCB(ob, 0);
		break;
		
	case LFUN_DEPTH_MIN:
		DepthCB(ob, -1);
		break;
		
	case LFUN_DEPTH_PLUS:
		DepthCB(ob, 1);
		break;
		
	case LFUN_FREE:
		FreeCB();
		break;
		
	case LFUN_TEX:
		TexCB();
		break;
		
	case LFUN_MELT:
		MeltCB(ob, 0);
		break;
		
	case LFUN_RECONFIGURE:
		Reconfigure();
		break;

	case LFUN_FOOTMELT:
		if (owner->currentView()->available()
		    && !owner->currentBuffer()->text->selection
		    && owner->currentBuffer()->text->cursor.par->footnoteflag
		    != LyXParagraph::NO_FOOTNOTE)
		{ // only melt footnotes with FOOTMELT, not margins etc
		  if(owner->currentBuffer()->text->cursor.par->footnotekind == LyXParagraph::FOOTNOTE)
			MeltCB(ob,0);
		}
		else
			FootCB(ob, 0); 
		break;

	case LFUN_MARGINMELT:
		if (owner->currentView()->available()
		    && !owner->currentBuffer()->text->selection
		    && owner->currentBuffer()->text->cursor.par->footnoteflag
		    != LyXParagraph::NO_FOOTNOTE)
		{ // only melt margins
		  if(owner->currentBuffer()->text->cursor.par->footnotekind == LyXParagraph::MARGIN)
			MeltCB(ob,0);
		}
		else
			MarginCB(ob, 0); 
		break;
		
		// --- version control -------------------------------
	case LFUN_VC_REGISTER:
	{
		if (!owner->currentBuffer()->lyxvc.inUse())
			owner->currentBuffer()->lyxvc.registrer();
	}
	break;
		
	case LFUN_VC_CHECKIN:
	{
		if (owner->currentBuffer()->lyxvc.inUse()
		    && !owner->currentBuffer()->isReadonly())
			owner->currentBuffer()->lyxvc.checkIn();
	}
	break;
		
	case LFUN_VC_CHECKOUT:
	{
		if (owner->currentBuffer()->lyxvc.inUse()
		    && owner->currentBuffer()->isReadonly())
			owner->currentBuffer()->lyxvc.checkOut();
	}
	break;
	
	case LFUN_VC_REVERT:
	{
		owner->currentBuffer()->lyxvc.revert();
	}
	break;
		
	case LFUN_VC_UNDO:
	{
		owner->currentBuffer()->lyxvc.undoLast();
	}
	break;
		
	case LFUN_VC_HISTORY:
	{
		owner->currentBuffer()->lyxvc.showLog();
		break;
	}
	
	// --- buffers ----------------------------------------
	case LFUN_PREVBUFFER:
#ifdef WITH_WARNINGS
#warning fix this please
#endif
		// it is the LyXView or the BufferView that should
		// remember the previous buffer, not bufferlist.
// 			if (owner->currentView()->available()){	  
// 				BeforeChange();
// 				owner->currentBuffer()->update(-2);
// 			}
// 			owner->currentView()->setBuffer(bufferlist.prev());

// 			owner->currentView()->
// 				resizeCurrentBufferPseudoExpose();
		break;
			
	case LFUN_FILE_INSERT:
	{
		// needs argument as string
		LString fil = argAsString(argument);
		MenuInsertLyXFile(fil);
	}
	break;
	
	case LFUN_FILE_INSERT_ASCII:
	{
	        bool asPara = false;
		asPara = (LString(argument) == "paragraph");
		InsertAsciiFile(LString(), asPara);
	}
	break;
	
	case LFUN_FILE_NEW:
	{
		// servercmd: argument must be <file>:<template>
		Buffer * tmpbuf = 0;
		tmpbuf = NewLyxFile(argument);
		if (tmpbuf)
			owner->currentView()->setBuffer(tmpbuf);
	}
		break;
			
	case LFUN_FILE_OPEN:
		owner->currentView()->setBuffer(
			bufferlist.loadLyXFile(argument));
		break;

	case LFUN_LATEX_LOG:
		ShowLatexLog();
		break;
		
	case LFUN_LAYOUTNO:
	{
		lyxerr.debug("LFUN_LAYOUTNO: (arg) " + LString(argument));
		int sel = atoi(argument);
		lyxerr.debug(LString("LFUN_LAYOUTNO: (sel) ") + int(sel));
		
		// Should this give a setMessage instead?
		if (sel == 0) 
			return LString(); // illegal argument

		sel--; // sel 1..., but layout 0...

		// Pretend we got the name instead.
		Dispatch(int(LFUN_LAYOUT), 
			 lyxstyle.NameOfLayout(owner->currentBuffer()->
					       text->parameters->
					       textclass,
					       sel).c_str());
		return LString();
	}
		
	case LFUN_LAYOUT:
	{
		lyxerr.debug("LFUN_LAYOUT: (arg) "
			     + LString(argument));
		
		// Derive layout number from given argument (string)
		// and current buffer's textclass (number). */    
		int layoutno = -1;
		layoutno =
			lyxstyle.NumberOfLayout(owner->
						currentBuffer()->
						text->parameters->
						textclass,
						argument);

		// see if we found the layout number:
		if (layoutno == -1) {
			setErrorMessage(LString(N_("Layout ")) + argument + 
					N_(" not known"));
			break;
		}
			
		if (current_layout != layoutno) {
			owner->currentView()->getScreen()->HideCursor();
			current_layout = layoutno;
			owner->currentBuffer()->update(-2);
			owner->currentBuffer()->text->
				SetLayout(layoutno);
			owner->getToolbar()->combox->
				select(owner->currentBuffer()->
				       text->cursor.par->
				       GetLayout() + 1);
			owner->currentBuffer()->update(1);
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
	        if (LString(argument) == "true") flag = 1;
		MenuLayoutTable(flag);
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
		owner->getToolbar()->combox->Show();
		break;

	case LFUN_EMPH:
		EmphCB();
		break;
		
	case LFUN_BOLD:
		BoldCB();
		break;
		
	case LFUN_NOUN:
		NounCB();
		break;
		
	case LFUN_CODE:
		CodeCB();
		break;
		
	case LFUN_SANS:
		SansCB();
		break;
		
	case LFUN_ROMAN:
		RomanCB();
		break;
		
	case LFUN_DEFAULT:
		StyleResetCB();
		break;
		
	case LFUN_UNDERLINE:
		UnderlineCB();
		break;
		
	case LFUN_FONT_SIZE:
		FontSizeCB(argument);
		break;
		
	case LFUN_FONT_STATE:
		setMessage(CurrentState());
		break;
		
	case LFUN_UPCASE_WORD:
		owner->currentBuffer()->update(-2);
		FreeUpdateTimer();
		owner->currentBuffer()->text->ChangeWordCase(2);
		owner->currentBuffer()->update(1);
		SetUpdateTimer();
		break;
		
	case LFUN_LOWCASE_WORD:
		owner->currentBuffer()->update(-2);
		FreeUpdateTimer();
		owner->currentBuffer()->text->ChangeWordCase(0);
		owner->currentBuffer()->update(1);
		SetUpdateTimer();
		break;
		
	case LFUN_CAPITALIZE_WORD:
		owner->currentBuffer()->update(-2);
		FreeUpdateTimer();
		owner->currentBuffer()->text->ChangeWordCase(1);
		owner->currentBuffer()->update(1);
		SetUpdateTimer();
		break;
		
	case LFUN_INSERT_LABEL:
		MenuInsertLabel(argument);
		break;
		
	case LFUN_INSERT_REF:
		MenuInsertRef();
		break;
		
	case LFUN_REFTOGGLE:
	{
		InsetRef *inset =
			(InsetRef*)getInsetByCode(Inset::REF_CODE);
		if (inset) {
			if (inset->getFlag()==InsetRef::REF)
				inset->setFlag(InsetRef::PAGE_REF);
			else
				inset->setFlag(InsetRef::REF);
			UpdateInset(inset);
		} else {
			setErrorMessage(N_("No cross-reference to toggle"));
		}
	}
	break;
	
	case LFUN_REFBACK:
	{
		owner->currentView()->restorePosition();
	}
	break;

	case LFUN_REFGOTO:
	{
		LString label(argument);
		if (label.empty()) {
			InsetRef *inset =
				(InsetRef*)getInsetByCode(Inset::REF_CODE);
			if (inset)
                                label = inset->getContents();
		}
		
		if (!label.empty()) {
			owner->currentView()->savePosition();
			owner->currentBuffer()->gotoLabel(label.c_str());
		}
	}
	break;
		
	case LFUN_MENU_OPEN_BY_NAME:
		owner->getMenus()->openByName(argument);
		break; // RVDK_PATCH_5
		
	case LFUN_SPELLCHECK:
		if (lyxrc->isp_command != "none")
			ShowSpellChecker();
		break; // RVDK_PATCH_5
		
		// --- Cursor Movements -----------------------------
	case LFUN_RIGHT:
	{
		Buffer *tmpbuffer = owner->currentBuffer();
		LyXText *tmptext = owner->currentBuffer()->text;
		if(!tmptext->mark_set)
			BeforeChange();
		tmpbuffer->update(-2);
		if (tmptext->cursor.pos < tmptext->cursor.par->Last()
		    && tmptext->cursor.par->GetChar(tmptext->cursor.pos)
		    == LYX_META_INSET
		    && tmptext->cursor.par->GetInset(tmptext->cursor.pos)
		    && tmptext->cursor.par->GetInset(tmptext->cursor.pos)->Editable()==2){
			Inset* tmpinset = tmptext->cursor.par->GetInset(tmptext->cursor.pos);
			setMessage(tmpinset->EditMessage());
			tmpinset->Edit(0,0);
			break;
		}
		tmptext->CursorRight();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
	}
	break;
		
	case LFUN_LEFT:
	{
		// This is soooo ugly. Isn`t it possible to make
		// it simpler? (Lgb)
		LyXText *txt= owner->currentBuffer()->text;
		if(!txt->mark_set) BeforeChange();
		owner->currentBuffer()->update(-2);
		txt->CursorLeft();
		if (txt->cursor.pos < txt->cursor.par->Last()
		    && txt->cursor.par->GetChar(txt->cursor.pos)
		    == LYX_META_INSET
		    && txt->cursor.par->GetInset(txt->cursor.pos)
		    && txt->cursor.par->GetInset(txt->cursor.pos)->Editable()==2) {
			Inset* tmpinset = txt->cursor.par->GetInset(txt->cursor.pos);
			setMessage(tmpinset->EditMessage());
			tmpinset->Edit(tmpinset->Width(txt->GetFont(txt->cursor.par,
								    txt->cursor.pos)),0);
			//			tmpinset->Edit(-1, 0);  // -1 means go rightmost
			break;
		}
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
	}
	break;
		
	case LFUN_UP:
		if(!owner->currentBuffer()->text->mark_set) BeforeChange();
		owner->currentBuffer()->update(-3);
		owner->currentBuffer()->text->CursorUp();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_DOWN:
		if(!owner->currentBuffer()->text->mark_set)
			BeforeChange();
		owner->currentBuffer()->update(-3);
		owner->currentBuffer()->text->CursorDown();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;

	case LFUN_UP_PARAGRAPH:
		if(!owner->currentBuffer()->text->mark_set)
			BeforeChange();
		owner->currentBuffer()->update(-3);
		owner->currentBuffer()->text->CursorUpParagraph();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_DOWN_PARAGRAPH:
		if(!owner->currentBuffer()->text->mark_set)
			BeforeChange();
		owner->currentBuffer()->update(-3);
		owner->currentBuffer()->text->CursorDownParagraph();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_PRIOR:
		if(!owner->currentBuffer()->text->mark_set)
			BeforeChange();
		owner->currentBuffer()->update(-3);
		owner->currentView()->cursorPrevious();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_NEXT:
		if(!owner->currentBuffer()->text->mark_set)
			BeforeChange();
		owner->currentBuffer()->update(-3);
		owner->currentView()->cursorNext();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_HOME:
		if(!owner->currentBuffer()->text->mark_set)
			BeforeChange();
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorHome();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_END:
		if(!owner->currentBuffer()->text->mark_set)
			BeforeChange();
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorEnd();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_TAB:
		if(!owner->currentBuffer()->text->mark_set)
			BeforeChange();
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorTab();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_WORDRIGHT:
		if(!owner->currentBuffer()->text->mark_set)
			BeforeChange();
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorRightOneWord();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_WORDLEFT:
		if(!owner->currentBuffer()->text->mark_set)
			BeforeChange();
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorLeftOneWord();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_BEGINNINGBUF:
		if(!owner->currentBuffer()->text->mark_set)
			BeforeChange();
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorTop();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_ENDBUF:
		if(!owner->currentBuffer()->text->mark_set)
			BeforeChange();
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorBottom();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;

      
		/* cursor selection ---------------------------- */
	case LFUN_RIGHTSEL:
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorRight();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_LEFTSEL:
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorLeft();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_UPSEL:
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorUp();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_DOWNSEL:
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorDown();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;

	case LFUN_UP_PARAGRAPHSEL:
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorUpParagraph();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_DOWN_PARAGRAPHSEL:
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorDownParagraph();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_PRIORSEL:
		owner->currentBuffer()->update(-2);
		owner->currentView()->cursorPrevious();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_NEXTSEL:
		owner->currentBuffer()->update(-2);
		owner->currentView()->cursorNext();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_HOMESEL:
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorHome();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_ENDSEL:
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorEnd();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_WORDRIGHTSEL:
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorRightOneWord();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_WORDLEFTSEL:
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorLeftOneWord();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_BEGINNINGBUFSEL:
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorTop();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_ENDBUFSEL:
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->CursorBottom();
		owner->currentBuffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;

		// --- text changing commands ------------------------
	case LFUN_BREAKLINE:
		BeforeChange();
		owner->currentBuffer()->text->InsertChar(LYX_META_NEWLINE);
		SmallUpdate(1);
		SetUpdateTimer(0.01);
		moveCursorUpdate(false);
		break;
		
	case LFUN_PROTECTEDSPACE:
		BeforeChange();
		owner->currentBuffer()->text->
			InsertChar(LYX_META_PROTECTED_SEPARATOR);
		SmallUpdate(1);
		SetUpdateTimer();
                moveCursorUpdate(false);
		break;
		
	case LFUN_SETMARK:
		if(owner->currentBuffer()->text->mark_set) {
			BeforeChange();
			owner->currentBuffer()->update(0);
			setMessage(N_("Mark removed"));
		} else {
			BeforeChange();
			owner->currentBuffer()->text->mark_set = 1;
			owner->currentBuffer()->update(0);
			setMessage(N_("Mark set"));
		}
		owner->currentBuffer()->text->sel_cursor =
			owner->currentBuffer()->text->cursor;
		break;
		
	case LFUN_DELETE:
		FreeUpdateTimer();
		if (!owner->currentBuffer()->text->selection) {
			owner->currentBuffer()->text->Delete();
			owner->currentBuffer()->text->sel_cursor =
				owner->currentBuffer()->text->cursor;
			SmallUpdate(1);
			// It is possible to make it a lot faster still
			// just comment out the lone below...
			owner->currentView()->getScreen()->ShowCursor();
		} else {
			CutCB();
		}
		SetUpdateTimer();
		break;

	case LFUN_DELETE_SKIP:
	{
		// Reverse the effect of LFUN_BREAKPARAGRAPH_SKIP.
		
		LyXCursor cursor = owner->currentBuffer()->text->cursor;

		FreeUpdateTimer();
		if (!owner->currentBuffer()->text->selection) {
			if (cursor.pos == cursor.par->Last()) {
				owner->currentBuffer()->text->CursorRight();
				cursor = owner->currentBuffer()->text->cursor;
				if (cursor.pos == 0
				    && !(cursor.par->added_space_top 
					 == VSpace (VSpace::NONE))) {
					owner->currentBuffer()->text->SetParagraph
						(cursor.par->line_top,
						 cursor.par->line_bottom,
						 cursor.par->pagebreak_top, 
						 cursor.par->pagebreak_bottom,
						 VSpace(VSpace::NONE), 
						 cursor.par->added_space_bottom,
						 cursor.par->align, 
						 cursor.par->labelwidthstring, 0);
					owner->currentBuffer()->text->CursorLeft();
					owner->currentBuffer()->update (1);
				} else {
					owner->currentBuffer()->text->CursorLeft();
					owner->currentBuffer()->text->Delete();
					owner->currentBuffer()->text->sel_cursor =
						owner->currentBuffer()->text->cursor;
					SmallUpdate(1);
				}
			} else {
				owner->currentBuffer()->text->Delete();
				owner->currentBuffer()->text->sel_cursor =
					owner->currentBuffer()->text->cursor;
				SmallUpdate(1);
			}
		} else {
			CutCB();
		}
		SetUpdateTimer();
	}
	break;

	/* -------> Delete word forward. */
	case LFUN_DELETE_WORD_FORWARD:
		owner->currentBuffer()->update(-2);
		FreeUpdateTimer();
		owner->currentBuffer()->text->DeleteWordForward();
		owner->currentBuffer()->update( 1 );
		SetUpdateTimer();
		moveCursorUpdate(false);
		break;

		/* -------> Delete word backward. */
	case LFUN_DELETE_WORD_BACKWARD:
		owner->currentBuffer()->update(-2);
		FreeUpdateTimer();
		owner->currentBuffer()->text->DeleteWordBackward();
		owner->currentBuffer()->update( 1 );
		SetUpdateTimer();
		moveCursorUpdate(false);
		break;
		
		/* -------> Kill to end of line. */
	case LFUN_DELETE_LINE_FORWARD:
		FreeUpdateTimer();
		owner->currentBuffer()->update(-2);
		owner->currentBuffer()->text->DeleteLineForward();
		owner->currentBuffer()->update( 1 );
		SetUpdateTimer();
		moveCursorUpdate(false);
		break;
		
		/* -------> Set mark off. */
	case LFUN_MARK_OFF:
		BeforeChange();
		owner->currentBuffer()->update(0);
		owner->currentBuffer()->text->sel_cursor =
			owner->currentBuffer()->text->cursor;
		setMessage(N_("Mark off"));
		break;

		/* -------> Set mark on. */
	case LFUN_MARK_ON:
		BeforeChange();
		owner->currentBuffer()->text->mark_set = 1;
		owner->currentBuffer()->update( 0 );
		owner->currentBuffer()->text->sel_cursor =
			owner->currentBuffer()->text->cursor;
		setMessage(N_("Mark on"));
		break;
		
	case LFUN_BACKSPACE:
	{
		FreeUpdateTimer();
		if (!owner->currentBuffer()->text->selection) {
			if (owner->getIntl()->getTrans()->backspace()) {
				owner->currentBuffer()->text->Backspace();
				owner->currentBuffer()->text->sel_cursor =
					owner->currentBuffer()->text->cursor;
				SmallUpdate(1);
				// It is possible to make it a lot faster still
				// just comment out the lone below...
				owner->currentView()->getScreen()->ShowCursor();
			}
		} else {
			CutCB();
		}
		SetUpdateTimer();
	}
	break;

	case LFUN_BACKSPACE_SKIP:
	{
		// Reverse the effect of LFUN_BREAKPARAGRAPH_SKIP.
		
		LyXCursor cursor = owner->currentBuffer()->text->cursor;
		
		FreeUpdateTimer();
		if (!owner->currentBuffer()->text->selection) {
			if (cursor.pos == 0 
			    && !(cursor.par->added_space_top 
				 == VSpace (VSpace::NONE))) {
				owner->currentBuffer()->text->SetParagraph 
					(cursor.par->line_top,      
					 cursor.par->line_bottom,
					 cursor.par->pagebreak_top, 
					 cursor.par->pagebreak_bottom,
					 VSpace(VSpace::NONE), cursor.par->added_space_bottom,
					 cursor.par->align, 
					 cursor.par->labelwidthstring, 0);
				owner->currentBuffer()->update (1);
			} else {
				owner->currentBuffer()->text->Backspace();
				owner->currentBuffer()->text->sel_cursor 
					= cursor;
				SmallUpdate (1);
			}
		} else
			CutCB();
		SetUpdateTimer();
	}
	break;

	case LFUN_BREAKPARAGRAPH:
	{
		BeforeChange();
		owner->currentBuffer()->text->BreakParagraph(0);
		SmallUpdate(1);
		SetUpdateTimer(0.01);
		owner->currentBuffer()->text->sel_cursor =
			owner->currentBuffer()->text->cursor;
		break;
	}

	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
	{
		BeforeChange();
		owner->currentBuffer()->text->BreakParagraph(1);
		SmallUpdate(1);
		SetUpdateTimer(0.01);
		owner->currentBuffer()->text->sel_cursor =
			owner->currentBuffer()->text->cursor;
		break;
	}
	
	case LFUN_BREAKPARAGRAPH_SKIP:
	{
		// When at the beginning of a paragraph, remove
		// indentation and add a "defskip" at the top.
		// Otherwise, do the same as LFUN_BREAKPARAGRAPH.
		
		LyXCursor cursor = owner->currentBuffer()->text->cursor;
		
		BeforeChange();
		if (cursor.pos == 0) {
			if (cursor.par->added_space_top == VSpace(VSpace::NONE)) {
				owner->currentBuffer()->text->SetParagraph
					(cursor.par->line_top,      
					 cursor.par->line_bottom,
					 cursor.par->pagebreak_top, 
					 cursor.par->pagebreak_bottom,
					 VSpace(VSpace::DEFSKIP), cursor.par->added_space_bottom,
					 cursor.par->align, 
					 cursor.par->labelwidthstring, 1);
				owner->currentBuffer()->update(1);
			} 
		}
		else {
			owner->currentBuffer()->text->BreakParagraph(0);
			SmallUpdate(1);
		}
		SetUpdateTimer(0.01);
		owner->currentBuffer()->text->sel_cursor = cursor;
	}
	break;
	
	case LFUN_QUOTE:
		BeforeChange();
		owner->currentBuffer()->text->InsertChar('\"');  // This " matches the single quote in the code
		SmallUpdate(1);
		SetUpdateTimer();
                moveCursorUpdate(false);
		break;

	case LFUN_HTMLURL:
	case LFUN_URL:
	{
		InsetCommand *new_inset;
		if (action == LFUN_HTMLURL)
			new_inset = new InsetUrl("htmlurl", "", "");
		else
			new_inset = new InsetUrl("url", "", "");
		owner->currentBuffer()->insertInset(new_inset);
		new_inset->Edit(0, 0);
	}
	break;

	// --- lyxserver commands ----------------------------

	case LFUN_CHARATCURSOR:
	{
		int pos = owner->currentBuffer()->text->cursor.pos;
		if(pos < owner->currentBuffer()->text->cursor.par->last)
			dispatch_buffer = owner->currentBuffer()->text->
				cursor.par->text[pos];
		else
			dispatch_buffer = "EOF";
	}
	break;
	
	case LFUN_GETXY:
		dispatch_buffer = LString()
			+ owner->currentBuffer()->text->cursor.x + ' '
			+ owner->currentBuffer()->text->cursor.y;
		break;
		
	case LFUN_SETXY:
	{
		int  x;
		long y;
		sscanf(argument, " %d %ld", &x, &y);
		owner->currentBuffer()->text->SetCursorFromCoordinates(x, y);
	}
	break;
	
	case LFUN_GETLAYOUT:
		dispatch_buffer = LString() + 
			int(owner->currentBuffer()->text->cursor.par->layout);
		break;
			
	case LFUN_GETFONT:
	{
		LyXFont *font = &(owner->currentBuffer()->text->current_font);
                if(font->shape() == LyXFont::ITALIC_SHAPE)
			dispatch_buffer = 'E';
                else if(font->shape() == LyXFont::SMALLCAPS_SHAPE)
			dispatch_buffer = 'N';
                else
			dispatch_buffer = '0';

	}
	break;

	case LFUN_GETLATEX:
	{
		LyXFont *font = &(owner->currentBuffer()->text->current_font);
                if(font->latex() == LyXFont::ON)
			dispatch_buffer = 'L';
                else
			dispatch_buffer = '0';
	}
	break;

	case LFUN_GETNAME:
		setMessage(owner->currentBuffer()->getFileName());
		lyxerr.debug(LString("FNAME[") +
			     owner->currentBuffer()->getFileName() +
			     "] ");
		break;
		
	case LFUN_NOTIFY:
	{
		char buf[100];
		keyseq.print(buf,100);
		dispatch_buffer = buf;
		lyxserver->notifyClient(dispatch_buffer);
	}
	break;

	case LFUN_GOTOFILEROW:
	{
	        char file_name[100];
		int  row;
		sscanf(argument, " %s %d", file_name, &row);

		// Must replace extension of the file to be .lyx and get full path
		LString s = ChangeExtension(LString(file_name), ".lyx", false);

		// Either change buffer or load the file
		if (bufferlist.exists(s))
		        owner->currentView()->setBuffer(bufferlist.getBuffer(s));
		else
		        owner->currentView()->setBuffer(bufferlist.loadLyXFile(s));

		// Set the cursor  
		owner->currentBuffer()->setCursorFromRow(row);

		// Recenter screen
		BeforeChange();
		if (owner->currentBuffer()->text->cursor.y >
		    owner->currentView()->getWorkArea()->h / 2)	{
			owner->currentView()->getScreen()->
				Draw(owner->currentBuffer()->text->cursor.y -
				     owner->currentView()->getWorkArea()->h/2);
		} else { // <=
			owner->currentView()->getScreen()->
				Draw(0);
		}
		owner->currentBuffer()->update(0);
		owner->currentView()->redraw();
	}
	break;

	case LFUN_APROPOS:
	case LFUN_GETTIP:
	{
		int qa = lyxaction.LookupFunc((const char*)argument);
		setMessage(lyxaction.helpText((kb_action)qa));
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
		char c;
		
		if (keyseq.length==-1 && keyseq.getiso()!=0) 
			c=keyseq.getiso();
		else
			c=0;
		
		owner->getIntl()->getTrans()->
			deadkey(c, get_accent(action).accent, 
				owner->currentBuffer()->text);
		
		// Need to reset, in case the minibuffer calls these
		// actions
		keyseq.reset();
		keyseq.length=0;
		
		// copied verbatim from do_accent_char
		SmallUpdate(1);
		SetUpdateTimer();
		owner->currentBuffer()->text->sel_cursor = 
			owner->currentBuffer()->text->cursor;
	}   
	break;
	
	// --- toolbar ----------------------------------
	case LFUN_PUSH_TOOLBAR:
	{
		int nth = atoi(argument);
		if (lyxerr.debugging(Error::TOOLBAR)) {
			lyxerr.print(LString("LFUN_PUSH_TOOLBAR: argument = `")
				     + argument + "'");
			lyxerr.print(LString("LFUN_PUSH_TOOLBAR: nth = `")
				     + nth + "'");
		}
		
		if (nth <= 0) {
			LyXBell();
			setErrorMessage(N_("Push-toolbar needs argument > 0"));
		} else {
			owner->getToolbar()->push(nth);
		}
	}
	break;
	
	case LFUN_ADD_TO_TOOLBAR:
	{
		if (lyxerr.debugging(Error::TOOLBAR)) {
			lyxerr.print(LString("LFUN_ADD_TO_TOOLBAR:"
					     "argument = `")+ argument + '\'');
		}
		LString tmp(argument);
		//lyxerr.print(LString("Argument: ") + argument);
		//lyxerr.print(LString("Tmp     : ") + tmp);
		if (tmp.empty()) {
			LyXBell();
			setErrorMessage(N_("Usage: toolbar-add-to <LyX command>"));
		} else {
			owner->getToolbar()->add(argument, false);
			owner->getToolbar()->set();
		}
	}
	break;
	
	// --- insert characters ----------------------------------------

	case LFUN_INSERT_INSET_LATEX:
	{
		Inset *new_inset = new InsetLatex(argument);
		owner->currentBuffer()->insertInset(new_inset);
	}
	break;

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
		greek_kb_flag = (greek_kb_flag) ? 0: 2;
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
		if (owner->currentView()->available()) { 
			owner->currentBuffer()->
				open_new_inset(new InsetFormula(false));
			owner->currentBuffer()->
				the_locking_inset->LocalDispatch(action, argument);
		}
	}	   
	break;
	       
	case LFUN_INSERT_MATH:
	{
		math_insert_symbol(argument);
	}
	break;
	
	case LFUN_MATH_DISPLAY:
	{
		if (owner->currentView()->available())
			owner->currentBuffer()->open_new_inset(new InsetFormula(true));
		break;
	}
		    
	case LFUN_MATH_MACRO:
	{
		if (owner->currentView()->available()) {
			LString s(argument);
		        if (s.empty())
		            setErrorMessage(N_("Missing argument"));
		        else {
			    LString s1 = s.token(' ', 1);
			    int na = s1.empty() ? 0: atoi(s1.c_str());
			    owner->currentBuffer()->
			      open_new_inset(new InsetFormulaMacro(s.token(' ', 0), na));
			}
		}
	}
	break;

	case LFUN_MATH_MODE:   // Open or create a math inset
	{
		
		if (owner->currentView()->available())
			owner->currentBuffer()->open_new_inset(new InsetFormula);
		setMessage(N_("Math editor mode"));
	}
	break;
	  
	case LFUN_MATH_NUMBER:
	case LFUN_MATH_LIMITS:
	{
		setErrorMessage(N_("This is only allowed in math mode!"));
	}
	break;
	
	case LFUN_INSERT_CITATION:
	{   
		InsetCitation *new_inset = new InsetCitation();
		// ale970405
		// The note, if any, must be after the key, delimited
		// by a | so both key and remark can have spaces.
		if (argument) {
			LString lsarg(argument);
			if (lsarg.contains("|")) {
				new_inset->setContents(lsarg.token('|', 0));
				new_inset->setOptions(lsarg.token('|', 1));
			} else
				new_inset->setContents(lsarg);
			owner->currentBuffer()->insertInset(new_inset);
		} else {
			owner->currentBuffer()->insertInset(new_inset);
			new_inset->Edit(0,0);
		}
	}
	break;
		    
	case LFUN_INSERT_BIBTEX:
	{   
		// ale970405+lasgoutt970425
		// The argument can be up to two tokens separated 
		// by a space. The first one is the bibstyle.
		LString lsarg(argument);
		LString bibstyle = lsarg.token(' ', 1);
		if (bibstyle.empty())
			bibstyle = "plain";
		InsetBibtex *new_inset 
			= new InsetBibtex(lsarg.token(' ', 0),
					  bibstyle,
					  owner->currentBuffer());
		
		owner->currentBuffer()->insertInset(new_inset);
		if (lsarg.empty()) {
			new_inset->Edit(0,0);
		}
	}
	break;
		
	// BibTeX data bases
	case LFUN_BIBDB_ADD:
	{
		InsetBibtex *inset =
			(InsetBibtex*)getInsetByCode(Inset::BIBTEX_CODE);
		if (inset) {
			inset->addDatabase(argument);
		}
	}
	break;
		    
	case LFUN_BIBDB_DEL:
	{
		InsetBibtex *inset =
			(InsetBibtex*)getInsetByCode(Inset::BIBTEX_CODE);
		if (inset) {
			inset->delDatabase(argument);
		}
	}
	break;
	
	case LFUN_BIBTEX_STYLE:
	{
		InsetBibtex *inset =
			(InsetBibtex*)getInsetByCode(Inset::BIBTEX_CODE);
		if (inset) {
			inset->setOptions(argument);
		}
	}
	break;
		
	case LFUN_INDEX_INSERT:
	case LFUN_INDEX_INSERT_LAST:
	{
		InsetIndex *new_inset = new InsetIndex();
		if (argument) {
  			LString lsarg(argument);
			new_inset->setContents(lsarg);
			owner->currentBuffer()->insertInset(new_inset);
		} else {
		  //reh 98/09/21
		  //get the current word for an argument
		  
		  
		  // grab a word

		  int lastpos =owner->currentBuffer()->text->cursor.pos-1;

		  //this shouldn't happen, but let's be careful
		  if (lastpos < 0) lastpos=0;
		  
		  // get the current word
		  // note that this must be done before 
		  // inserting the inset, or the inset will break
		  // the word
		  LString curstring(owner->currentBuffer()->text->cursor.par->GetWord(lastpos));

		  //make the new inset and write the current word into it
		  InsetIndex *new_inset = new InsetIndex();

		  new_inset->setContents(curstring);

		  //don't edit it if the call was to INSERT_LAST
		  if(action!=LFUN_INDEX_INSERT_LAST) {
		      new_inset->Edit(0,0);
		  } else {
		      //it looks blank on the screen unless
		      //we do  something.  put it here.

		      // move the cursor to the returned value of lastpos
		      // but only for the auto-insert
		      owner->currentBuffer()->text->cursor.pos=lastpos;
		  }

		  //put the new inset into the buffer.
		  // there should be some way of knowing the user
		  //cancelled & avoiding this, but i don't know how
		  owner->currentBuffer()->insertInset(new_inset);
		}
	}
	break;

	case LFUN_INDEX_PRINT:
	{
		Inset *new_inset = new InsetPrintIndex(owner->currentBuffer());
		owner->currentBuffer()->insertInset(new_inset, "Standard", true);
	}
	break;

	case LFUN_PARENTINSERT:
	{
		lyxerr.print(LString("arg ") + argument);
		Inset *new_inset = new InsetParent(argument, owner->currentBuffer());
		owner->currentBuffer()->insertInset(new_inset, "Standard", true);
	}
	break;

	case LFUN_CHILDINSERT:
	{
		Inset *new_inset = new InsetInclude(argument,owner->currentBuffer());
		owner->currentBuffer()->insertInset(new_inset, "Standard", true);
		new_inset->Edit(0,0);
	}
	break;

	case LFUN_CHILDOPEN:
	{
		LString filename = MakeAbsPath(argument, 
					       OnlyPath(owner->currentBuffer()->getFileName()));
		setMessage(N_("Opening child document ") +
			   MakeDisplayPath(filename) + "...");
		owner->currentView()->savePosition();
		if (bufferlist.exists(filename))
		  owner->currentView()->setBuffer(bufferlist.getBuffer(filename));
		else
		  owner->currentView()->setBuffer(bufferlist.loadLyXFile(filename));
	}
	break;

	case LFUN_INSERT_NOTE:
		NoteCB();
		break;
		
	case LFUN_INSERTFOOTNOTE: 
	{
		LyXParagraph::footnote_kind kind;
		LString arg = argument;
		if (arg == "footnote")
			{ kind = LyXParagraph::FOOTNOTE; }
		else if (arg == "margin")
			{ kind = LyXParagraph::MARGIN; }
		else if (arg == "figure")
			{ kind = LyXParagraph::FIG; }
		else if (arg == "table")
			{ kind = LyXParagraph::TAB; }
		else if (arg == "wide-fig")
			{ kind = LyXParagraph::WIDE_FIG; }
		else if (arg == "wide-tab")
			{ kind = LyXParagraph::WIDE_TAB; }
		else if (arg == "algorithm")
			{ kind = LyXParagraph::ALGORITHM; }
		else {
			setErrorMessage(N_("Unknown kind of footnote"));
			break;
		}
		owner->currentBuffer()->text->InsertFootnoteEnvironment(kind);
		owner->currentBuffer()->update(1);
	}
	break;
	
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
		LString const text = argument;
		for (int i=0; i<text.length(); i++) {
			owner->currentBuffer()->text->InsertChar(text[i]);
			// This needs to be in the loop, or else we
			// won't break lines correctly. (Asger)
			SmallUpdate(1);
		}
		SetUpdateTimer();
		owner->currentBuffer()->text->sel_cursor =
			owner->currentBuffer()->text->cursor;
		moveCursorUpdate(false);
	}
	break;

	case LFUN_UNKNOWN_ACTION:
	{
		if (owner->currentBuffer()->isReadonly()) {
			LyXBell();
			setErrorMessage(N_("Document is read only"));
			break;
		}
			 
		if (argument) {
			
			/* Automatically delete the currently selected
			 * text and replace it with what is being
			 * typed in now. Depends on lyxrc settings
			 * "auto_region_delete", which defaults to
			 * true (on). */
		
			if ( lyxrc->auto_region_delete ) {
				if (owner->currentBuffer()->text->selection){
					owner->currentBuffer()->text->CutSelection(false);
					owner->currentBuffer()->update(-1);
				}
			}
			
			BeforeChange();
			for(char const *p = argument; *p; p++) {
				if (greek_kb_flag) {
					if (!math_insert_greek(*p))
						owner->getIntl()->getTrans()->TranslateAndInsert(*p, owner->currentBuffer()->text);
				} else
					owner->getIntl()->getTrans()->TranslateAndInsert(*p, owner->currentBuffer()->text);
			}
			
			SmallUpdate(1);
			SetUpdateTimer();

			owner->currentBuffer()->text->sel_cursor =
				owner->currentBuffer()->text->cursor;
			moveCursorUpdate(false);
			return LString();
		} else {
			// why is an "Unknown action" with empty
			// argument even dispatched in the first
			// place? I`ll probably change that. (Lgb)
			LyXBell();
			setErrorMessage(N_("Unknown action"));
		}
		break;
	default:
		lyxerr.print("A truly unknown func!");
		break;
	}
	} // end of switch
  exit_with_message:

	LString res=getMessage();

	if (res.empty()) {
		if (!commandshortcut.empty()) {
			LString newbuf = owner->getMiniBuffer()->GetText();
			if (newbuf != commandshortcut) {
				owner->getMiniBuffer()->Set(newbuf
							    + " " +
							    commandshortcut);
			}
		}
	} else {
		owner->getMiniBuffer()->Set(LString(_(res.c_str()))
					    + " " + commandshortcut);
	}

	return getMessage();
}
	    

void LyXFunc::setupLocalKeymap()
{
	keyseq.stdmap = keyseq.curmap = toplevel_keymap;
	cancel_meta_seq.stdmap = cancel_meta_seq.curmap = toplevel_keymap;
}


void LyXFunc::MenuNew(bool fromTemplate)
{
	LString fname, initpath = lyxrc->document_path;
	LyXFileDlg fileDlg;

	if (owner->currentView()->available()) {
		LString trypath = owner->currentBuffer()->filepath;
		// If directory is writeable, use this as default.
		if (IsDirWriteable(trypath) == 1)
			initpath = trypath;
	}

	ProhibitInput();
	fileDlg.SetButton(0, _("Documents"), lyxrc->document_path);
	fileDlg.SetButton(1, _("Templates"), lyxrc->template_path);
	fname = fileDlg.Select(_("Enter Filename for new document"), 
			       initpath, "*.lyx", _("newfile"));
 	AllowInput();
	
	if (fname.empty()) {
		owner->getMiniBuffer()->Set(_("Canceled."));
		lyxerr.debug("New Document Cancelled.");
		return;
	}
        
	// get absolute path of file and make sure the filename ends
	// with .lyx
	LString s = MakeAbsPath(fname);
	if (!IsLyXFilename(s))
		s += ".lyx";

	// Check if the document already is open
	if (bufferlist.exists(s)){
		switch(AskConfirmation(_("Document is already open:"), 
				       MakeDisplayPath(s,50),
				       _("Do you want to close that document now?\n"
					 "('No' will just switch to the open version)")))
		{
		case 1: // Yes: close the document
			if (!bufferlist.close(bufferlist.getBuffer(s)))
				// If close is canceled, we cancel here too.
				return;
			break;
		case 2: // No: switch to the open document
			owner->currentView()->setBuffer(bufferlist.getBuffer(s));
			return;
		case 3: // Cancel: Do nothing
			owner->getMiniBuffer()->Set(_("Canceled."));
			return;
		}
	}
        
	// Check whether the file already exists
	if (IsLyXFilename(s)) {
		FilePtr myfile(s, FilePtr::read);
		if (myfile() &&
		    AskQuestion(_("File already exists:"), 
				MakeDisplayPath(s,50),
				_("Do you want to open the document?"))) {
			// loads document
			owner->getMiniBuffer()->Set(_("Opening document"), 
						    MakeDisplayPath(s), "...");
			XFlush(fl_display);
			owner->currentView()->setBuffer(
				bufferlist.loadLyXFile(s));
			owner->getMiniBuffer()->Set(_("Document"),
						    MakeDisplayPath(s),
						    _("opened."));
			return;
		}
	}

	// The template stuff
	LString templname;
	if (fromTemplate) {
		ProhibitInput();
		fname = fileDlg.Select(_("Choose template"),
				       lyxrc->template_path,
				       "*.lyx");
                templname = fname;
		AllowInput();
	}
  
	// find a free buffer
	lyxerr.debug("Find a free buffer.");
	owner->currentView()->setBuffer(bufferlist.newFile(s,templname));
}


void LyXFunc::MenuOpen()
{
	LString initpath = lyxrc->document_path;
	LyXFileDlg fileDlg;
  
	if (owner->currentView()->available()) {
		LString trypath = owner->currentBuffer()->filepath;
		// If directory is writeable, use this as default.
		if (IsDirWriteable(trypath) == 1)
			initpath = trypath;
	}

	// launches dialog
	ProhibitInput();
	fileDlg.SetButton(0, _("Documents"), lyxrc->document_path);
	fileDlg.SetButton(1, _("Examples"), 
			  AddPath(system_lyxdir, "examples"));
	LString filename = fileDlg.Select(_("Select Document to Open"),
					  initpath, "*.lyx");
	AllowInput();
 
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
		owner->currentView()->setBuffer(openbuf);
		owner->getMiniBuffer()->Set(_("Document"),
					    MakeDisplayPath(filename),
					    _("opened."));
	} else {
		owner->getMiniBuffer()->Set(_("Could not open document"),
					    MakeDisplayPath(filename));
	}
}


void LyXFunc::doImportASCII(bool linorpar)
{
	LString initpath = lyxrc->document_path;
	LyXFileDlg fileDlg;
  
	if (owner->currentView()->available()) {
		LString trypath = owner->currentBuffer()->filepath;
		// If directory is writeable, use this as default.
		if (IsDirWriteable(trypath) == 1)
			initpath = trypath;
	}

	// launches dialog
	ProhibitInput();
	fileDlg.SetButton(0, _("Documents"), lyxrc->document_path);
	fileDlg.SetButton(1, _("Examples"), 
			  AddPath(system_lyxdir, "examples"));
	LString filename = fileDlg.Select(_("Select ASCII file to Import"),
					  initpath, "*.txt");
	AllowInput();
 
	// check selected filename
	if (filename.empty()) {
		owner->getMiniBuffer()->Set(_("Canceled."));
		return;
	}

	// get absolute path of file
	filename = MakeAbsPath(filename);

	LString s = ChangeExtension(filename, ".lyx", false);

	// Check if the document already is open
	if (bufferlist.exists(s)){
		switch(AskConfirmation(_("Document is already open:"), 
				       MakeDisplayPath(s,50),
				       _("Do you want to close that document now?\n"
					 "('No' will just switch to the open version)")))
		{
		case 1: // Yes: close the document
			if (!bufferlist.close(bufferlist.getBuffer(s)))
				// If close is canceled, we cancel here too.
				return;
			break;
		case 2: // No: switch to the open document
			owner->currentView()->setBuffer(bufferlist.getBuffer(s));
			return;
		case 3: // Cancel: Do nothing
			owner->getMiniBuffer()->Set(_("Canceled."));
			return;
		}
	}

	// Check if a LyX document by the same root exists in filesystem
	FileInfo f(s, true);
	if (f.exist() && !AskQuestion(_("A document by the name"), 
				      MakeDisplayPath(s),
				      _("already exists. Overwrite?"))) {
		owner->getMiniBuffer()->Set(_("Canceled."));
		return;
	}

	owner->currentView()->setBuffer(bufferlist.newFile(s,LString()));
	owner->getMiniBuffer()->Set(_("Importing ASCII file"),
				    MakeDisplayPath(filename), "...");
	// Insert ASCII file
	InsertAsciiFile(filename,linorpar);
	owner->getMiniBuffer()->Set(_("ASCII file "),
				    MakeDisplayPath(filename),
				    _("imported."));
}


void LyXFunc::doImportLaTeX(bool isnoweb)
{
	LString initpath = lyxrc->document_path;
	LyXFileDlg fileDlg;
  
	if (owner->currentView()->available()) {
		LString trypath = owner->currentBuffer()->filepath;
		// If directory is writeable, use this as default.
		if (IsDirWriteable(trypath) == 1)
			initpath = trypath;
	}

	// launches dialog
	ProhibitInput();
	fileDlg.SetButton(0, _("Documents"), lyxrc->document_path);
	fileDlg.SetButton(1, _("Examples"), 
			  AddPath(system_lyxdir, "examples"));
	LString filename;
	if (isnoweb) {
		filename = fileDlg.Select(_("Select Noweb file to Import"),
					  initpath, "*.nw");
	} else {
		filename = fileDlg.Select(_("Select LaTeX file to Import"),
					  initpath, "*.tex");
	}
	
	AllowInput();
 
	// check selected filename
	if (filename.empty()) {
		owner->getMiniBuffer()->Set(_("Canceled."));
		return;
	}

	// get absolute path of file
	filename = MakeAbsPath(filename);

	// Check if the document already is open
	LString LyXfilename = ChangeExtension(filename, ".lyx", false);
	if (bufferlist.exists(LyXfilename)){
		switch(AskConfirmation(_("Document is already open:"), 
				       MakeDisplayPath(LyXfilename,50),
				       _("Do you want to close that document now?\n"
					 "('No' will just switch to the open version)")))
		{
		case 1: // Yes: close the document
			if (!bufferlist.close(bufferlist.getBuffer(LyXfilename)))
				// If close is canceled, we cancel here too.
				return;
			break;
		case 2: // No: switch to the open document
			owner->currentView()->setBuffer(
				bufferlist.getBuffer(LyXfilename));
			return;
		case 3: // Cancel: Do nothing
			owner->getMiniBuffer()->Set(_("Canceled."));
			return;
		}
	}

	// Check if a LyX document by the same root exists in filesystem
	FileInfo f(LyXfilename, true);
	if (f.exist() && !AskQuestion(_("A document by the name"), 
				      MakeDisplayPath(LyXfilename),
				      _("already exists. Overwrite?"))) {
		owner->getMiniBuffer()->Set(_("Canceled."));
		return;
	}

	// loads document
	Buffer * openbuf;
	if (!isnoweb) {
		owner->getMiniBuffer()->Set(_("Importing LaTeX file"),
					    MakeDisplayPath(filename), "...");
		ImportLaTeX myImport(filename);
		openbuf = myImport.run();
	} else {
		owner->getMiniBuffer()->Set(_("Importing Noweb file"),
					    MakeDisplayPath(filename), "...");
		ImportNoweb myImport(filename);
		openbuf = myImport.run();
	}
	if (openbuf) {
		owner->currentView()->setBuffer(openbuf);
		owner->getMiniBuffer()->Set(isnoweb ?
					    _("Noweb file ") : _("LateX file "),
					    MakeDisplayPath(filename),
					    _("imported."));
	} else {
		owner->getMiniBuffer()->Set(isnoweb ?
					    _("Could not import Noweb file") :
					    _("Could not import LaTeX file"),
					    MakeDisplayPath(filename));
	}
}


void LyXFunc::MenuInsertLyXFile(LString const & filen)
{
	LString filename = filen;

	if (filename.empty()) {
		// Launch a file browser
		LString initpath = lyxrc->document_path;
		LyXFileDlg fileDlg;

		if (owner->currentView()->available()) {
			LString trypath = owner->currentBuffer()->filepath;
			// If directory is writeable, use this as default.
			if (IsDirWriteable(trypath) == 1)
				initpath = trypath;
		}

		// launches dialog
		ProhibitInput();
		fileDlg.SetButton(0, _("Documents"), lyxrc->document_path);
		fileDlg.SetButton(1, _("Examples"), 
				  AddPath(system_lyxdir, "examples"));
		filename = fileDlg.Select(_("Select Document to Insert"),
					  initpath, "*.lyx");
		AllowInput();

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
	bool res = owner->currentBuffer()->insertLyXFile(filename);
	if (res) {
		owner->getMiniBuffer()->Set(_("Document"),
					    MakeDisplayPath(filename),
					    _("inserted."));
	} else {
		owner->getMiniBuffer()->Set(_("Could not insert document"),
					    MakeDisplayPath(filename));
	}
}


void LyXFunc::reloadBuffer()
{
	LString fn = owner->currentBuffer()->getFileName();
	if (bufferlist.close(owner->currentBuffer()))
		owner->currentView()->setBuffer(bufferlist.loadLyXFile(fn));
}


void LyXFunc::CloseBuffer()
{
	if (bufferlist.close(owner->currentBuffer()) && !quitting) {
		if (bufferlist.isEmpty()) {
			// need this otherwise SEGV may occur while trying to
			// set variables that don't exist
			// since there's no current buffer
			CloseAllBufferRelatedPopups();
		}
		else {
			owner->currentView()->setBuffer(bufferlist.first());
		}
	}
}


Inset* LyXFunc::getInsetByCode(Inset::Code code)
{
	bool found = false;
	Inset* inset = 0;
	LyXCursor cursor = owner->currentBuffer()->text->cursor;
	int pos = cursor.pos;
	LyXParagraph *par = cursor.par;
	
	while (par && !found) {
		while ((inset = par->ReturnNextInsetPointer(pos))){
			if (inset->LyxCode()==code) {
				found = true;
				break;
			}
			pos++;
		} 
		par = par->next;
	}
	return (found) ? inset: 0;
}


// Each "owner" should have it's own message method. lyxview and
// the minibuffer would use the minibuffer, but lyxserver would
// send an ERROR signal to its client.  Alejandro 970603
// This func is bit problematic when it comes to NLS, to make the
// lyx servers client be language indepenent we must not translate
// strings sent to this func.
void LyXFunc::setErrorMessage(LString const &m) 
{
	dispatch_buffer = m;
	errorstat = true;
}


void LyXFunc::setMessage(LString const & m)
{
	dispatch_buffer = m;
}
