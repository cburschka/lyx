/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team.
 *
 * ======================================================*/

#include <config.h>

#include <cstdlib>
#include <cctype>
#include <cstring>

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
#include "support/filetools.h"
#include "support/FileInfo.h"
#include "support/LAssert.h"
#include "lyxscreen.h"
#include "debug.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "gettext.h"
#include "trans_mgr.h"
#include "ImportLaTeX.h"
#include "ImportNoweb.h"
#include "support/syscall.h"
#include "support/lstrings.h"
#include "support/path.h"
#include "layout.h"

extern bool cursor_follows_scrollbar;

extern void InsertAsciiFile(string const &, bool);
extern void math_insert_symbol(char const*);
extern Bool math_insert_greek(char const); // why "Bool"?
extern BufferList bufferlist;
extern LyXServer * lyxserver;
extern short greek_kb_flag;
extern FD_form_toc * fd_form_toc;
extern bool selection_possible;

extern kb_keymap * toplevel_keymap;

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

extern Buffer * NewLyxFile(string const &);
extern void LoadLyXFile(string const &);
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


// I changed this func slightly. I commented out the ...FinishUndo(),
// this means that all places that used to have a moveCursorUpdate, now
// have a ...FinishUndo() as the preceeding statement. I have also added
// a moveCursorUpdate to some of the functions that updated the cursor, but
// that did not show its new position.
inline
void LyXFunc::moveCursorUpdate(bool selecting)
{
	if (selecting || owner->buffer()->text->mark_set) {
		owner->buffer()->text->SetSelection();
		owner->currentView()->getScreen()->ToggleToggle();
		owner->buffer()->update(0);
	} else {
		owner->buffer()->update(-2); // this IS necessary
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
	string argument;
	XKeyEvent *keyevent = &ev->xkey;
	KeySym keysym_return;

	num_bytes = LyXLookupString(ev, s_r, 10, &keysym_return);

	if (lyxerr.debugging(Debug::KEY)) {
		lyxerr << "KeySym is "
		       << XKeysymToString(keysym_return)
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
	if (owner->currentView()->available() &&
	    owner->buffer()->the_locking_inset &&
	    keysym_return == XK_Escape) {
		UnlockInset(owner->buffer()->the_locking_inset);
		owner->buffer()->text->CursorRight();
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

	if (lyxerr.debugging(Debug::KEY)) {
		char buf[100];
		keyseq.print(buf,100);
		lyxerr << "Key ["
		       << action << "]["
		       << buf << "]["
		       << num_bytes << "]" << endl;
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
			argument += isochar;
		}
		if (argument.empty()) {
			lyxerr.debug() << "Empty argument!" << endl;
			// This can`t possibly be of any use
			// so we`ll skip the dispatch.
			return 0;
		}
	} else
		if (action==LFUN_SELFINSERT) {
			argument = s_r[0];
		}
    
        bool tmp_sc = show_sc;
	show_sc = false;
	Dispatch(action, argument.c_str());
	show_sc = tmp_sc;
	
	return 0;
} 


string LyXFunc::Dispatch(string const &cmd, string const &arg)
{
	return Dispatch(lyxaction.LookupFunc(cmd.c_str()),
			arg.c_str());
}


string LyXFunc::Dispatch(int ac,
			  char const *do_not_use_this_arg)
{
	string argument;
	kb_action action;
        

	FL_OBJECT *ob = 0;  // This will disapear soon
    
        // we have not done anything wrong yet.
        errorstat = false;
	dispatch_buffer = string();
	
	// if action is a pseudo-action, we need the real action
	if (lyxaction.isPseudoAction(ac)) {
		char const *tmparg = 0;
		action = (kb_action)lyxaction.retrieveActionArg(ac, &tmparg);
		if (tmparg)
			argument = tmparg;
	} else {
		action = (kb_action)ac;
		if (do_not_use_this_arg)
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
	
	commandshortcut = string();
	
	if (lyxrc->display_shortcuts && show_sc) {
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

        // Now that we know which action, if the buffer is RO let's check 
	// whether the action is legal.  Alejandro 970603
        if (owner->currentView()->available() && 
            owner->buffer()->isReadonly() && 
            lyxaction.isFuncRO(action)) {
		setErrorMessage(N_("Document is read-only"));
		lyxerr.debug() << "Error: Document is read-only." << endl;
		goto exit_with_message;
	}

	// If in math mode pass the control to
	// the math inset [asierra060396]
	if (owner->currentView()->available() &&
	    owner->buffer()->the_locking_inset) {
		if (action>1 || (action==LFUN_UNKNOWN_ACTION && keyseq.length>=-1)) {
			if (action==LFUN_UNKNOWN_ACTION && argument.empty()) {
				argument = keyseq.getiso();
			}
			// Undo/Redo pre 0.13 is a bit tricky for insets.		    
		        if (action==LFUN_UNDO) {
				int slx, sly;
				UpdatableInset* inset =
					owner->buffer()->the_locking_inset;
				inset->GetCursorPos(slx, sly);
				UnlockInset(inset);
				MenuUndo();
				inset = (UpdatableInset*)owner->buffer()->text->cursor.par->GetInset(owner->buffer()->text->cursor.pos);
				if (inset) 
					inset->Edit(slx, sly);
				return string();
			} else 
				if (action==LFUN_REDO) {
					int slx, sly;
					UpdatableInset* inset = owner->buffer()->the_locking_inset;
					inset->GetCursorPos(slx, sly);
					UnlockInset(inset);
					MenuRedo();
					inset = (UpdatableInset*)owner->buffer()->text->cursor.par->GetInset(owner->buffer()->text->cursor.pos);
					if (inset)
						inset->Edit(slx, sly);
					return string();
				} else
					if (owner->buffer()->the_locking_inset->LocalDispatch(action, argument.c_str()))
						return string();
					else {
						setMessage(N_("Text mode"));
						if (action==LFUN_RIGHT || action==-1)
							owner->buffer()->text->CursorRight();
						if (action==LFUN_LEFT || action==LFUN_RIGHT)
							return string();
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

		LyXText * ltCur = owner->currentView()->buffer()->text ;

		if (!searched_string.empty() &&
		    (	 (action == LFUN_WORDFINDBACKWARD) ? 
			 ltCur->SearchBackward( searched_string.c_str() ) :
			 ltCur->SearchForward(	searched_string.c_str() ) 
			 )){

			// ??? What is that ???
			owner->currentView()->buffer()->update(-2);

			// ??? Needed ???
			// clear the selection (if there is any) 
			owner->currentView()->getScreen()->ToggleSelection();
			owner->currentView()->buffer()->text->ClearSelection();

			// Move cursor so that successive C-s 's will not stand in place. 
			if( action == LFUN_WORDFINDFORWARD ) 
				owner->buffer()->text->CursorRightOneWord();
			owner->buffer()->text->FinishUndo();
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
			owner->buffer()->update(-2);
		}
		char buf[100];
		keyseq.print(buf,100, true);
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
		if(owner->currentView()->available())
			// cancel any selection
			Dispatch(int(LFUN_MARK_OFF), 0);
		setMessage(N_("Cancel"));
		break;

	case LFUN_META_FAKE:                                 // RVDK_PATCH_5
	{
		meta_fake_bit = Mod1Mask;
		char buf[100];
		keyseq.print(buf, 98, true);
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
		BeforeChange();
		if (owner->buffer()->text->cursor.y >
		    owner->currentView()->getWorkArea()->h / 2)	{
			owner->currentView()->getScreen()->
				Draw(owner->buffer()->text->cursor.y -
				     owner->currentView()->getWorkArea()->h/2);
		} else { // <=
			owner->currentView()->getScreen()->
				Draw(0);
		}
		owner->buffer()->update(0);
		owner->currentView()->redraw();
		break;
		
	case LFUN_APPENDIX:
		if (owner->currentView()->available()) {
			owner->buffer()->text->toggleAppendix();
			owner->buffer()->update(1);
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
		MenuWrite(owner->buffer());
		break;
		
	case LFUN_MENUWRITEAS:
		MenuWriteAs(owner->buffer());
		break;
		
	case LFUN_MENURELOAD:
		reloadBuffer();
		break;
		
	case LFUN_PREVIEW:
		MenuPreview(owner->buffer());
		break;
			
	case LFUN_PREVIEWPS:
		MenuPreviewPS(owner->buffer());
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
		MenuRunDvips(owner->buffer(), false);
		break;
		
	case LFUN_MENUPRINT:
		MenuPrint(owner->buffer());
		break;
		
	case LFUN_FAX:
		MenuFax(owner->buffer());
		break;
			
	case LFUN_EXPORT:
	{
		//needs argument as string
		string extyp=argument;
		
		// latex
		if (extyp == "latex") {
			// make sure that this buffer is not linuxdoc
			MenuMakeLaTeX(owner->buffer());
		}
		// linuxdoc
		else if (extyp == "linuxdoc") {
			// make sure that this buffer is not latex
			MenuMakeLinuxDoc(owner->buffer());
		}
		// docbook
		else if (extyp == "docbook") {
			// make sure that this buffer is not latex or linuxdoc
			MenuMakeDocBook(owner->buffer());
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
			MenuRunLaTeX(owner->buffer());
			lyxrc->use_tempdir = flag;
		}
		// postscript
		else if (extyp == "postscript") {
			// Start Print-dialog. Not as good as dvi... Bernhard.
			MenuPrint(owner->buffer());
			// Since the MenuPrint is a pop-up, we can't use
			// the same trick as above. (Asger)
			// MISSING: Move of ps-file :-(
		}
		// ascii
		else if (extyp == "ascii") {
			MenuMakeAscii(owner->buffer());
		}
		else if (extyp == "custom") {
			MenuSendto();
			break;
		}
		// HTML
		else if (extyp == "html" && lyxrc->html_command != "none") {
			// First, create LaTeX file
			MenuMakeLaTeX(owner->buffer());

			// And now, run the converter
			string file = owner->buffer()->getFileName();
			Path path(OnlyPath(file));
			// the tex file name has to be correct for
			// latex, but the html file name can be
			// anything.
			string result = ChangeExtension(file, ".html", false);
			file = ChangeExtension(SpaceLess(file), ".tex", false);
			string tmp = lyxrc->html_command;
			tmp = subst(tmp, "$$FName", file);
			tmp = subst(tmp, "$$OutName", result);
			Systemcalls one;
			int res = one.startscript(Systemcalls::System, tmp);
			if (res == 0) {
				setMessage(_("Document exported as HTML to file `")
					   + MakeDisplayPath(result) +'\'');
			} else {
				setErrorMessage(_("Unable to convert to HTML the file `")
						+ MakeDisplayPath(file) 
						+ '\'');
			}
		}
		else {
			setErrorMessage(_("Unknown export type: ")
					+ extyp);
		}
	}
	break;

	case LFUN_IMPORT:
	{
		//needs argument as string
		string imtyp=argument;
		
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
			setErrorMessage(string(N_("Unknown import type: "))
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
			static int ow = -1, oh;
			fl_show_form(fd_form_toc->form_toc,
				     FL_PLACE_MOUSE |
				     FL_FREE_SIZE, FL_FULLBORDER,
				     _("Table of Contents"));
			if (ow < 0) {
				ow = fd_form_toc->form_toc->w;
				oh = fd_form_toc->form_toc->h;
			}
			fl_set_form_minsize(fd_form_toc->form_toc, ow, oh);
		}
		break;
		
	case LFUN_TOC_INSERT:
	{
		Inset *new_inset =
			new InsetTOC(owner->buffer());
		owner->buffer()->insertInset(new_inset,
						    "Standard", true);
		break;
	}
	
	case LFUN_LOF_INSERT:
	{
		Inset *new_inset =
			new InsetLOF(owner->buffer());
		owner->buffer()->insertInset(new_inset,
						    "Standard", true);
		break;
	}
	
	case LFUN_LOA_INSERT:
	{
		Inset *new_inset =
			new InsetLOA(owner->buffer());
		owner->buffer()->insertInset(new_inset,
						    "Standard", true);
		break;
	}

	case LFUN_LOT_INSERT:
	{
		Inset *new_inset =
			new InsetLOT(owner->buffer());
		owner->buffer()->insertInset(new_inset,
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
		if (string(argument) == "paragraph") asPara = true;
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
		if (owner->buffer()->removeAutoInsets()) {
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
		    && !owner->buffer()->text->selection
		    && owner->buffer()->text->cursor.par->footnoteflag
		    != LyXParagraph::NO_FOOTNOTE)
		{ // only melt footnotes with FOOTMELT, not margins etc
		  if(owner->buffer()->text->cursor.par->footnotekind == LyXParagraph::FOOTNOTE)
			MeltCB(ob,0);
		}
		else
			FootCB(ob, 0); 
		break;

	case LFUN_MARGINMELT:
		if (owner->currentView()->available()
		    && !owner->buffer()->text->selection
		    && owner->buffer()->text->cursor.par->footnoteflag
		    != LyXParagraph::NO_FOOTNOTE)
		{ // only melt margins
		  if(owner->buffer()->text->cursor.par->footnotekind == LyXParagraph::MARGIN)
			MeltCB(ob,0);
		}
		else
			MarginCB(ob, 0); 
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
	case LFUN_PREVBUFFER:
#ifdef WITH_WARNINGS
#warning fix this please
#endif
		// it is the LyXView or the BufferView that should
		// remember the previous buffer, not bufferlist.
// 			if (owner->currentView()->available()){	  
// 				BeforeChange();
// 				owner->buffer()->update(-2);
// 			}
// 			owner->currentView()->setBuffer(bufferlist.prev());

// 			owner->currentView()->
// 				resizeCurrentBufferPseudoExpose();
		break;
			
	case LFUN_FILE_INSERT:
	{
		MenuInsertLyXFile(argument);
	}
	break;
	
	case LFUN_FILE_INSERT_ASCII:
	{
	        bool asPara = false;
		asPara = (string(argument) == "paragraph");
		InsertAsciiFile(string(), asPara);
	}
	break;
	
	case LFUN_FILE_NEW:
	{
		// servercmd: argument must be <file>:<template>
		Buffer * tmpbuf = 0;
		tmpbuf = NewLyxFile(argument);
		if (tmpbuf)
			owner->currentView()->buffer(tmpbuf);
	}
		break;
			
	case LFUN_FILE_OPEN:
		owner->currentView()->buffer(
			bufferlist.loadLyXFile(argument));
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

		sel--; // sel 1..., but layout 0...

		// Pretend we got the name instead.
		Dispatch(int(LFUN_LAYOUT), 
			 textclasslist.NameOfLayout(owner->buffer()->
					       text->parameters->
					       textclass,
					       sel).c_str());
		return string();
	}
		
	case LFUN_LAYOUT:
	{
		lyxerr.debug() << "LFUN_LAYOUT: (arg) "
			       << argument << endl;
		
		// Derive layout number from given argument (string)
		// and current buffer's textclass (number). */    
		int layoutno = -1;
		layoutno =
			textclasslist.NumberOfLayout(owner->
						buffer()->
						text->parameters->
						textclass,
						argument).second;

		// see if we found the layout number:
		if (layoutno == -1) {
			setErrorMessage(string(N_("Layout ")) + argument + 
					N_(" not known"));
			break;
		}
			
		if (current_layout != layoutno) {
			owner->currentView()->getScreen()->HideCursor();
			current_layout = layoutno;
			owner->buffer()->update(-2);
			owner->buffer()->text->
				SetLayout(layoutno);
			owner->getToolbar()->combox->
				select(owner->buffer()->
				       text->cursor.par->
				       GetLayout() + 1);
			owner->buffer()->update(1);
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
	        if (string(argument) == "true") flag = 1;
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
		owner->buffer()->update(-2);
		FreeUpdateTimer();
		owner->buffer()->text->ChangeWordCase(LyXText::text_uppercase);
		owner->buffer()->update(1);
		SetUpdateTimer();
		break;
		
	case LFUN_LOWCASE_WORD:
		owner->buffer()->update(-2);
		FreeUpdateTimer();
		owner->buffer()->text->ChangeWordCase(LyXText::text_lowercase);
		owner->buffer()->update(1);
		SetUpdateTimer();
		break;
		
	case LFUN_CAPITALIZE_WORD:
		owner->buffer()->update(-2);
		FreeUpdateTimer();
		owner->buffer()->text->ChangeWordCase(LyXText::text_capitalization);
		owner->buffer()->update(1);
		SetUpdateTimer();
		break;
		
	case LFUN_INSERT_LABEL:
		MenuInsertLabel(argument.c_str());
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
		string label(argument);
		if (label.empty()) {
			InsetRef *inset =
				(InsetRef*)getInsetByCode(Inset::REF_CODE);
			if (inset)
                                label = inset->getContents();
		}
		
		if (!label.empty()) {
			owner->currentView()->savePosition();
			owner->buffer()->gotoLabel(label.c_str());
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
		Buffer *tmpbuffer = owner->buffer();
		LyXText *tmptext = owner->buffer()->text;
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
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
	}
	break;
		
	case LFUN_LEFT:
	{
		// This is soooo ugly. Isn`t it possible to make
		// it simpler? (Lgb)
		LyXText *txt= owner->buffer()->text;
		if(!txt->mark_set) BeforeChange();
		owner->buffer()->update(-2);
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
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
	}
	break;
		
	case LFUN_UP:
		if(!owner->buffer()->text->mark_set) BeforeChange();
		owner->buffer()->update(-3);
		owner->buffer()->text->CursorUp();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_DOWN:
		if(!owner->buffer()->text->mark_set)
			BeforeChange();
		owner->buffer()->update(-3);
		owner->buffer()->text->CursorDown();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;

	case LFUN_UP_PARAGRAPH:
		if(!owner->buffer()->text->mark_set)
			BeforeChange();
		owner->buffer()->update(-3);
		owner->buffer()->text->CursorUpParagraph();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_DOWN_PARAGRAPH:
		if(!owner->buffer()->text->mark_set)
			BeforeChange();
		owner->buffer()->update(-3);
		owner->buffer()->text->CursorDownParagraph();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_PRIOR:
		if(!owner->buffer()->text->mark_set)
			BeforeChange();
		owner->buffer()->update(-3);
		owner->currentView()->cursorPrevious();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_NEXT:
		if(!owner->buffer()->text->mark_set)
			BeforeChange();
		owner->buffer()->update(-3);
		owner->currentView()->cursorNext();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_HOME:
		if(!owner->buffer()->text->mark_set)
			BeforeChange();
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorHome();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_END:
		if(!owner->buffer()->text->mark_set)
			BeforeChange();
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorEnd();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_TAB:
		if(!owner->buffer()->text->mark_set)
			BeforeChange();
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorTab();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_WORDRIGHT:
		if(!owner->buffer()->text->mark_set)
			BeforeChange();
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorRightOneWord();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_WORDLEFT:
		if(!owner->buffer()->text->mark_set)
			BeforeChange();
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorLeftOneWord();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_BEGINNINGBUF:
		if(!owner->buffer()->text->mark_set)
			BeforeChange();
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorTop();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_ENDBUF:
		if(!owner->buffer()->text->mark_set)
			BeforeChange();
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorBottom();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(false);
		owner->getMiniBuffer()->Set(CurrentState());
		break;

      
		/* cursor selection ---------------------------- */
	case LFUN_RIGHTSEL:
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorRight();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_LEFTSEL:
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorLeft();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_UPSEL:
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorUp();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_DOWNSEL:
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorDown();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;

	case LFUN_UP_PARAGRAPHSEL:
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorUpParagraph();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_DOWN_PARAGRAPHSEL:
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorDownParagraph();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_PRIORSEL:
		owner->buffer()->update(-2);
		owner->currentView()->cursorPrevious();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_NEXTSEL:
		owner->buffer()->update(-2);
		owner->currentView()->cursorNext();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_HOMESEL:
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorHome();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_ENDSEL:
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorEnd();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_WORDRIGHTSEL:
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorRightOneWord();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_WORDLEFTSEL:
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorLeftOneWord();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_BEGINNINGBUFSEL:
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorTop();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;
		
	case LFUN_ENDBUFSEL:
		owner->buffer()->update(-2);
		owner->buffer()->text->CursorBottom();
		owner->buffer()->text->FinishUndo();
		moveCursorUpdate(true);
		owner->getMiniBuffer()->Set(CurrentState());
		break;

		// --- text changing commands ------------------------
	case LFUN_BREAKLINE:
		BeforeChange();
		owner->buffer()->text->InsertChar(LYX_META_NEWLINE);
		SmallUpdate(1);
		SetUpdateTimer(0.01);
		moveCursorUpdate(false);
		break;
		
	case LFUN_PROTECTEDSPACE:
		BeforeChange();
		owner->buffer()->text->
			InsertChar(LYX_META_PROTECTED_SEPARATOR);
		SmallUpdate(1);
		SetUpdateTimer();
                moveCursorUpdate(false);
		break;
		
	case LFUN_SETMARK:
		if(owner->buffer()->text->mark_set) {
			BeforeChange();
			owner->buffer()->update(0);
			setMessage(N_("Mark removed"));
		} else {
			BeforeChange();
			owner->buffer()->text->mark_set = 1;
			owner->buffer()->update(0);
			setMessage(N_("Mark set"));
		}
		owner->buffer()->text->sel_cursor =
			owner->buffer()->text->cursor;
		break;
		
	case LFUN_DELETE:
		FreeUpdateTimer();
		if (!owner->buffer()->text->selection) {
			owner->buffer()->text->Delete();
			owner->buffer()->text->sel_cursor =
				owner->buffer()->text->cursor;
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
		
		LyXCursor cursor = owner->buffer()->text->cursor;

		FreeUpdateTimer();
		if (!owner->buffer()->text->selection) {
			if (cursor.pos == cursor.par->Last()) {
				owner->buffer()->text->CursorRight();
				cursor = owner->buffer()->text->cursor;
				if (cursor.pos == 0
				    && !(cursor.par->added_space_top 
					 == VSpace (VSpace::NONE))) {
					owner->buffer()->text->SetParagraph
						(cursor.par->line_top,
						 cursor.par->line_bottom,
						 cursor.par->pagebreak_top, 
						 cursor.par->pagebreak_bottom,
						 VSpace(VSpace::NONE), 
						 cursor.par->added_space_bottom,
						 cursor.par->align, 
						 cursor.par->labelwidthstring, 0);
					owner->buffer()->text->CursorLeft();
					owner->buffer()->update (1);
				} else {
					owner->buffer()->text->CursorLeft();
					owner->buffer()->text->Delete();
					owner->buffer()->text->sel_cursor =
						owner->buffer()->text->cursor;
					SmallUpdate(1);
				}
			} else {
				owner->buffer()->text->Delete();
				owner->buffer()->text->sel_cursor =
					owner->buffer()->text->cursor;
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
		owner->buffer()->update(-2);
		FreeUpdateTimer();
		owner->buffer()->text->DeleteWordForward();
		owner->buffer()->update( 1 );
		SetUpdateTimer();
		moveCursorUpdate(false);
		break;

		/* -------> Delete word backward. */
	case LFUN_DELETE_WORD_BACKWARD:
		owner->buffer()->update(-2);
		FreeUpdateTimer();
		owner->buffer()->text->DeleteWordBackward();
		owner->buffer()->update( 1 );
		SetUpdateTimer();
		moveCursorUpdate(false);
		break;
		
		/* -------> Kill to end of line. */
	case LFUN_DELETE_LINE_FORWARD:
		FreeUpdateTimer();
		owner->buffer()->update(-2);
		owner->buffer()->text->DeleteLineForward();
		owner->buffer()->update( 1 );
		SetUpdateTimer();
		moveCursorUpdate(false);
		break;
		
		/* -------> Set mark off. */
	case LFUN_MARK_OFF:
		BeforeChange();
		owner->buffer()->update(0);
		owner->buffer()->text->sel_cursor =
			owner->buffer()->text->cursor;
		setMessage(N_("Mark off"));
		break;

		/* -------> Set mark on. */
	case LFUN_MARK_ON:
		BeforeChange();
		owner->buffer()->text->mark_set = 1;
		owner->buffer()->update( 0 );
		owner->buffer()->text->sel_cursor =
			owner->buffer()->text->cursor;
		setMessage(N_("Mark on"));
		break;
		
	case LFUN_BACKSPACE:
	{
		FreeUpdateTimer();
		if (!owner->buffer()->text->selection) {
			if (owner->getIntl()->getTrans()->backspace()) {
				owner->buffer()->text->Backspace();
				owner->buffer()->text->sel_cursor =
					owner->buffer()->text->cursor;
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
		
		LyXCursor cursor = owner->buffer()->text->cursor;
		
		FreeUpdateTimer();
		if (!owner->buffer()->text->selection) {
			if (cursor.pos == 0 
			    && !(cursor.par->added_space_top 
				 == VSpace (VSpace::NONE))) {
				owner->buffer()->text->SetParagraph 
					(cursor.par->line_top,      
					 cursor.par->line_bottom,
					 cursor.par->pagebreak_top, 
					 cursor.par->pagebreak_bottom,
					 VSpace(VSpace::NONE), cursor.par->added_space_bottom,
					 cursor.par->align, 
					 cursor.par->labelwidthstring, 0);
				owner->buffer()->update (1);
			} else {
				owner->buffer()->text->Backspace();
				owner->buffer()->text->sel_cursor 
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
		owner->buffer()->text->BreakParagraph(0);
		SmallUpdate(1);
		SetUpdateTimer(0.01);
		owner->buffer()->text->sel_cursor =
			owner->buffer()->text->cursor;
		break;
	}

	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
	{
		BeforeChange();
		owner->buffer()->text->BreakParagraph(1);
		SmallUpdate(1);
		SetUpdateTimer(0.01);
		owner->buffer()->text->sel_cursor =
			owner->buffer()->text->cursor;
		break;
	}
	
	case LFUN_BREAKPARAGRAPH_SKIP:
	{
		// When at the beginning of a paragraph, remove
		// indentation and add a "defskip" at the top.
		// Otherwise, do the same as LFUN_BREAKPARAGRAPH.
		
		LyXCursor cursor = owner->buffer()->text->cursor;
		
		BeforeChange();
		if (cursor.pos == 0) {
			if (cursor.par->added_space_top == VSpace(VSpace::NONE)) {
				owner->buffer()->text->SetParagraph
					(cursor.par->line_top,      
					 cursor.par->line_bottom,
					 cursor.par->pagebreak_top, 
					 cursor.par->pagebreak_bottom,
					 VSpace(VSpace::DEFSKIP), cursor.par->added_space_bottom,
					 cursor.par->align, 
					 cursor.par->labelwidthstring, 1);
				owner->buffer()->update(1);
			} 
		}
		else {
			owner->buffer()->text->BreakParagraph(0);
			SmallUpdate(1);
		}
		SetUpdateTimer(0.01);
		owner->buffer()->text->sel_cursor = cursor;
	}
	break;
	
	case LFUN_QUOTE:
		BeforeChange();
		owner->buffer()->text->InsertChar('\"');  // This " matches the single quote in the code
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
		owner->buffer()->insertInset(new_inset);
		new_inset->Edit(0, 0);
	}
	break;

	// --- lyxserver commands ----------------------------

	case LFUN_CHARATCURSOR:
	{
#ifdef NEW_TEXT
		LyXParagraph::size_type pos =
		  owner->buffer()->text->cursor.pos;
		if(pos < owner->buffer()->text->cursor.par->size())
#else
		int pos = owner->buffer()->text->cursor.pos;
		if(pos < owner->buffer()->text->cursor.par->last)
#endif
			dispatch_buffer = owner->buffer()->text->
				cursor.par->text[pos];
		else
			dispatch_buffer = "EOF";
	}
	break;
	
	case LFUN_GETXY:
		dispatch_buffer = 
			 tostr(owner->buffer()->text->cursor.x) + ' '
			+ tostr(owner->buffer()->text->cursor.y);
		break;
		
	case LFUN_SETXY:
	{
		int  x;
		long y;
		sscanf(argument.c_str(), " %d %ld", &x, &y);
		owner->buffer()->text->SetCursorFromCoordinates(x, y);
	}
	break;
	
	case LFUN_GETLAYOUT:
		dispatch_buffer =  
			tostr(owner->buffer()->text->cursor.par->layout);
		break;
			
	case LFUN_GETFONT:
	{
		LyXFont *font = &(owner->buffer()->text->current_font);
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
		LyXFont *font = &(owner->buffer()->text->current_font);
                if(font->latex() == LyXFont::ON)
			dispatch_buffer = 'L';
                else
			dispatch_buffer = '0';
	}
	break;

	case LFUN_GETNAME:
		setMessage(owner->buffer()->getFileName());
		lyxerr.debug() << "FNAME["
			       << owner->buffer()->getFileName()
			       << "] " << endl;
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
		sscanf(argument.c_str(), " %s %d", file_name, &row);

		// Must replace extension of the file to be .lyx and get full path
		string s = ChangeExtension(string(file_name), ".lyx", false);

		// Either change buffer or load the file
		if (bufferlist.exists(s))
		        owner->currentView()->buffer(bufferlist.getBuffer(s));
		else
		        owner->currentView()->buffer(bufferlist.loadLyXFile(s));

		// Set the cursor  
		owner->buffer()->setCursorFromRow(row);

		// Recenter screen
		BeforeChange();
		if (owner->buffer()->text->cursor.y >
		    owner->currentView()->getWorkArea()->h / 2)	{
			owner->currentView()->getScreen()->
				Draw(owner->buffer()->text->cursor.y -
				     owner->currentView()->getWorkArea()->h/2);
		} else { // <=
			owner->currentView()->getScreen()->
				Draw(0);
		}
		owner->buffer()->update(0);
		owner->currentView()->redraw();
	}
	break;

	case LFUN_APROPOS:
	case LFUN_GETTIP:
	{
		int qa = lyxaction.LookupFunc(argument.c_str());
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
				owner->buffer()->text);
		
		// Need to reset, in case the minibuffer calls these
		// actions
		keyseq.reset();
		keyseq.length=0;
		
		// copied verbatim from do_accent_char
		SmallUpdate(1);
		SetUpdateTimer();
		owner->buffer()->text->sel_cursor = 
			owner->buffer()->text->cursor;
	}   
	break;
	
	// --- toolbar ----------------------------------
	case LFUN_PUSH_TOOLBAR:
	{
		int nth = strToInt(argument);
		if (lyxerr.debugging(Debug::TOOLBAR)) {
			lyxerr << "LFUN_PUSH_TOOLBAR: argument = `"
			       << argument << "'\n"
			       << "LFUN_PUSH_TOOLBAR: nth = `"
			       << nth << "'" << endl;
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
		if (lyxerr.debugging(Debug::TOOLBAR)) {
			lyxerr << "LFUN_ADD_TO_TOOLBAR:"
				"argument = `" << argument << '\'' << endl;
		}
		string tmp(argument);
		//lyxerr <<string("Argument: ") + argument);
		//lyxerr <<string("Tmp     : ") + tmp);
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
		owner->buffer()->insertInset(new_inset);
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
			owner->buffer()->
				open_new_inset(new InsetFormula(false));
			owner->buffer()->
				the_locking_inset->LocalDispatch(action, argument.c_str());
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
		if (owner->currentView()->available())
			owner->buffer()->open_new_inset(new InsetFormula(true));
		break;
	}
		    
	case LFUN_MATH_MACRO:
	{
		if (owner->currentView()->available()) {
			string s(argument);
		        if (s.empty())
		            setErrorMessage(N_("Missing argument"));
		        else {
			    string s1 = token(s, ' ', 1);
			    int na = s1.empty() ? 0: atoi(s1.c_str());
			    owner->buffer()->
			      open_new_inset(new InsetFormulaMacro(token(s, ' ', 0), na));
			}
		}
	}
	break;

	case LFUN_MATH_MODE:   // Open or create a math inset
	{
		
		if (owner->currentView()->available())
			owner->buffer()->open_new_inset(new InsetFormula);
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
		if (!argument.empty()) {
			string lsarg(argument);
			if (contains(lsarg, "|")) {
				new_inset->setContents(token(lsarg, '|', 0));
				new_inset->setOptions(token(lsarg, '|', 1));
			} else
				new_inset->setContents(lsarg);
			owner->buffer()->insertInset(new_inset);
		} else {
			owner->buffer()->insertInset(new_inset);
			new_inset->Edit(0,0);
		}
	}
	break;
		    
	case LFUN_INSERT_BIBTEX:
	{   
		// ale970405+lasgoutt970425
		// The argument can be up to two tokens separated 
		// by a space. The first one is the bibstyle.
		string lsarg(argument);
		string bibstyle = token(lsarg, ' ', 1);
		if (bibstyle.empty())
			bibstyle = "plain";
		InsetBibtex *new_inset 
			= new InsetBibtex(token(lsarg, ' ', 0),
					  bibstyle,
					  owner->buffer());
		
		owner->buffer()->insertInset(new_inset);
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
		InsetIndex * new_inset = new InsetIndex();
		if (!argument.empty()) {
  			string lsarg(argument);
			new_inset->setContents(lsarg);
			owner->buffer()->insertInset(new_inset);
		} else {
		  //reh 98/09/21
		  //get the current word for an argument
		  
		  
		  // grab a word

#ifdef NEW_TEXT
		  LyXParagraph::size_type lastpos =
			  owner->buffer()->text->cursor.pos - 1;
		  // If this can't happen, let's make sure that it really don't
		  Assert(owner->buffer()->text->cursor.pos - 1 >= 0);
#else
		  int lastpos =owner->buffer()->text->cursor.pos - 1;
		  //this shouldn't happen, but let's be careful
		  if (lastpos < 0) lastpos=0;
#endif
		  // get the current word
		  // note that this must be done before 
		  // inserting the inset, or the inset will break
		  // the word
		  string curstring(owner->buffer()
				   ->text->cursor.par->GetWord(lastpos));

		  //make the new inset and write the current word into it
		  InsetIndex * new_inset = new InsetIndex();

		  new_inset->setContents(curstring);

		  //don't edit it if the call was to INSERT_LAST
		  if(action!=LFUN_INDEX_INSERT_LAST) {
		      new_inset->Edit(0,0);
		  } else {
		      //it looks blank on the screen unless
		      //we do  something.  put it here.

		      // move the cursor to the returned value of lastpos
		      // but only for the auto-insert
		      owner->buffer()->text->cursor.pos=lastpos;
		  }

		  //put the new inset into the buffer.
		  // there should be some way of knowing the user
		  //cancelled & avoiding this, but i don't know how
		  owner->buffer()->insertInset(new_inset);
		}
	}
	break;

	case LFUN_INDEX_PRINT:
	{
		Inset *new_inset = new InsetPrintIndex(owner->buffer());
		owner->buffer()->insertInset(new_inset, "Standard", true);
	}
	break;

	case LFUN_PARENTINSERT:
	{
		lyxerr << "arg " << argument << endl;
		Inset *new_inset = new InsetParent(argument, owner->buffer());
		owner->buffer()->insertInset(new_inset, "Standard", true);
	}
	break;

	case LFUN_CHILDINSERT:
	{
		Inset *new_inset = new InsetInclude(argument,owner->buffer());
		owner->buffer()->insertInset(new_inset, "Standard", true);
		new_inset->Edit(0,0);
	}
	break;

	case LFUN_CHILDOPEN:
	{
		string filename = MakeAbsPath(argument, 
					       OnlyPath(owner->buffer()->getFileName()));
		setMessage(N_("Opening child document ") +
			   MakeDisplayPath(filename) + "...");
		owner->currentView()->savePosition();
		if (bufferlist.exists(filename))
		  owner->currentView()->buffer(bufferlist.getBuffer(filename));
		else
		  owner->currentView()->buffer(bufferlist.loadLyXFile(filename));
	}
	break;

	case LFUN_INSERT_NOTE:
		NoteCB();
		break;
		
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
		owner->buffer()->text->InsertFootnoteEnvironment(kind);
		owner->buffer()->update(1);
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
		for (string::size_type i = 0; i < argument.length(); ++i) {
			owner->buffer()->text->InsertChar(argument[i]);
			// This needs to be in the loop, or else we
			// won't break lines correctly. (Asger)
			SmallUpdate(1);
		}
		SetUpdateTimer();
		owner->buffer()->text->sel_cursor =
			owner->buffer()->text->cursor;
		moveCursorUpdate(false);
	}
	break;

	case LFUN_UNKNOWN_ACTION:
	{
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
		
			if ( lyxrc->auto_region_delete ) {
				if (owner->buffer()->text->selection){
					owner->buffer()->text->CutSelection(false);
					owner->buffer()->update(-1);
				}
			}
			
			BeforeChange();
			for (string::size_type i = 0; i < argument.length(); ++i) {
				if (greek_kb_flag) {
					if (!math_insert_greek(argument[i]))
						owner->getIntl()->getTrans()->TranslateAndInsert(argument[i], owner->buffer()->text);
				} else
					owner->getIntl()->getTrans()->TranslateAndInsert(argument[i], owner->buffer()->text);
			}
			
			SmallUpdate(1);
			SetUpdateTimer();

			owner->buffer()->text->sel_cursor =
				owner->buffer()->text->cursor;
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

	string res=getMessage();

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

	return getMessage();
}
	    

void LyXFunc::setupLocalKeymap()
{
	keyseq.stdmap = keyseq.curmap = toplevel_keymap;
	cancel_meta_seq.stdmap = cancel_meta_seq.curmap = toplevel_keymap;
}


void LyXFunc::MenuNew(bool fromTemplate)
{
	string fname, initpath = lyxrc->document_path;
	LyXFileDlg fileDlg;

	if (owner->currentView()->available()) {
		string trypath = owner->buffer()->filepath;
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
		lyxerr.debug() << "New Document Cancelled." << endl;
		return;
	}
        
	// get absolute path of file and make sure the filename ends
	// with .lyx
	string s = MakeAbsPath(fname);
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
			owner->currentView()->buffer(bufferlist.getBuffer(s));
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
			owner->currentView()->buffer(
				bufferlist.loadLyXFile(s));
			owner->getMiniBuffer()->Set(_("Document"),
						    MakeDisplayPath(s),
						    _("opened."));
			return;
		}
	}

	// The template stuff
	string templname;
	if (fromTemplate) {
		ProhibitInput();
		fname = fileDlg.Select(_("Choose template"),
				       lyxrc->template_path,
				       "*.lyx");
                templname = fname;
		AllowInput();
	}
  
	// find a free buffer
	lyxerr.debug() << "Find a free buffer." << endl;
	owner->currentView()->buffer(bufferlist.newFile(s,templname));
}


void LyXFunc::MenuOpen()
{
	string initpath = lyxrc->document_path;
	LyXFileDlg fileDlg;
  
	if (owner->currentView()->available()) {
		string trypath = owner->buffer()->filepath;
		// If directory is writeable, use this as default.
		if (IsDirWriteable(trypath) == 1)
			initpath = trypath;
	}

	// launches dialog
	ProhibitInput();
	fileDlg.SetButton(0, _("Documents"), lyxrc->document_path);
	fileDlg.SetButton(1, _("Examples"), 
			  AddPath(system_lyxdir, "examples"));
	string filename = fileDlg.Select(_("Select Document to Open"),
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
		owner->currentView()->buffer(openbuf);
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
	string initpath = lyxrc->document_path;
	LyXFileDlg fileDlg;
  
	if (owner->currentView()->available()) {
		string trypath = owner->buffer()->filepath;
		// If directory is writeable, use this as default.
		if (IsDirWriteable(trypath) == 1)
			initpath = trypath;
	}

	// launches dialog
	ProhibitInput();
	fileDlg.SetButton(0, _("Documents"), lyxrc->document_path);
	fileDlg.SetButton(1, _("Examples"), 
			  AddPath(system_lyxdir, "examples"));
	string filename = fileDlg.Select(_("Select ASCII file to Import"),
					  initpath, "*.txt");
	AllowInput();
 
	// check selected filename
	if (filename.empty()) {
		owner->getMiniBuffer()->Set(_("Canceled."));
		return;
	}

	// get absolute path of file
	filename = MakeAbsPath(filename);

	string s = ChangeExtension(filename, ".lyx", false);

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
			owner->currentView()->buffer(bufferlist.getBuffer(s));
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

	owner->currentView()->buffer(bufferlist.newFile(s,string()));
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
	string initpath = lyxrc->document_path;
	LyXFileDlg fileDlg;
  
	if (owner->currentView()->available()) {
		string trypath = owner->buffer()->filepath;
		// If directory is writeable, use this as default.
		if (IsDirWriteable(trypath) == 1)
			initpath = trypath;
	}

	// launches dialog
	ProhibitInput();
	fileDlg.SetButton(0, _("Documents"), lyxrc->document_path);
	fileDlg.SetButton(1, _("Examples"), 
			  AddPath(system_lyxdir, "examples"));
	string filename;
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
	string LyXfilename = ChangeExtension(filename, ".lyx", false);
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
			owner->currentView()->buffer(
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
		owner->currentView()->buffer(openbuf);
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


void LyXFunc::MenuInsertLyXFile(string const & filen)
{
	string filename = filen;

	if (filename.empty()) {
		// Launch a file browser
		string initpath = lyxrc->document_path;
		LyXFileDlg fileDlg;

		if (owner->currentView()->available()) {
			string trypath = owner->buffer()->filepath;
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
	bool res = owner->buffer()->insertLyXFile(filename);
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
	string fn = owner->buffer()->getFileName();
	if (bufferlist.close(owner->buffer()))
		owner->currentView()->buffer(bufferlist.loadLyXFile(fn));
}


void LyXFunc::CloseBuffer()
{
	if (bufferlist.close(owner->buffer()) && !quitting) {
		if (bufferlist.empty()) {
			// need this otherwise SEGV may occur while trying to
			// set variables that don't exist
			// since there's no current buffer
			CloseAllBufferRelatedPopups();
		}
		else {
			owner->currentView()->buffer(bufferlist.first());
		}
	}
}


Inset * LyXFunc::getInsetByCode(Inset::Code code)
{
	bool found = false;
	Inset * inset = 0;
	LyXCursor cursor = owner->buffer()->text->cursor;
#ifdef NEW_TEXT
	LyXParagraph::size_type pos = cursor.pos;
#else
	int pos = cursor.pos;
#endif
	LyXParagraph * par = cursor.par;
	
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
void LyXFunc::setErrorMessage(string const &m) 
{
	dispatch_buffer = m;
	errorstat = true;
}


void LyXFunc::setMessage(string const & m)
{
	dispatch_buffer = m;
}
