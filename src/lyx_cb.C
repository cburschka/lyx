/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich,
 *          Copyright 1995-1999 The LyX Team.
 *
 * ======================================================*/

#include <config.h>

#include <cctype>
#include <unistd.h>
#include <csignal>
#include <cstring>
#include <cstdlib>

#include "LString.h"
#include "lyx_main.h"
#include FORMS_H_LOCATION
#include "lyx.h"
#include "layout_forms.h"
#include "bullet_forms.h"
#include "print_form.h"
#include "form1.h"
#include "spellchecker.h"
#include "version.h"
#include "lyx_cb.h"
#include "credits.h"
#include "insets/insetref.h"
#include "insets/insetquotes.h"
#include "insets/insetlatex.h"
#include "insets/insetlabel.h"
#include "insets/insetinfo.h"
#include "insets/insetspecialchar.h"
#include "insets/figinset.h"
#include "lyxfunc.h"
#include "latexoptions.h"
#include "lyxfont.h"
#include "minibuffer.h"
#include "combox.h"
#include "bufferlist.h"
#include "support/filetools.h"
#include "pathstack.h"
#include "filedlg.h"
#include "lyx_gui_misc.h"
#include "LyXView.h" // only because of form_main
#include "lastfiles.h"
#include "support/FileInfo.h"
#include "lyxscreen.h"
#include "debug.h"
#include "support/syscall.h"
#include "support/lyxlib.h"
#include "lyxserver.h"
#include "FontLoader.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "gettext.h"

extern MiniBuffer *minibuffer;
extern Combox *combo_language;
extern BufferList bufferlist;
extern void show_symbols_form();
extern FD_form_main *fd_form_main;
extern FD_form_title *fd_form_title;
extern FD_form_paragraph *fd_form_paragraph;
extern FD_form_character *fd_form_character;
extern FD_form_document *fd_form_document;
extern FD_form_quotes *fd_form_quotes;
extern FD_form_preamble *fd_form_preamble;
extern FD_form_table *fd_form_table;
extern FD_form_print *fd_form_print;
extern FD_form_figure *fd_form_figure;
extern FD_form_screen *fd_form_screen;
extern FD_form_toc *fd_form_toc;
extern FD_form_ref *fd_form_ref;
extern FD_LaTeXOptions *fd_latex_options;
extern FD_form_bullet *fd_form_bullet;

extern BufferView *current_view; // called too many times in this file...

extern void DeleteSimpleCutBuffer(); /* for the cleanup when exiting */

extern bool send_fax(string const &fname, string const &sendcmd);

extern LyXServer *lyxserver;
extern FontLoader fontloader;

// this should be static, but I need it in buffer.C
bool quitting;	// flag, that we are quitting the program
extern bool finished; // all cleanup done just let it run through now.

char ascii_type; /* for selection notify callbacks */

bool scrolling = false;

char updatetimer = 0;

/* whether the work area should get callbacks */ 
bool input_prohibited = false;

/* the selection possible is needed, that only motion events are 
* used, where the bottom press event was on the drawing area too */
bool selection_possible = false;

// This is used to make the dreaded font toggle problem hopefully go
// away. Definitely not the best solution, but I think it sorta works.
bool toggleall = true;

void InsertCorrectQuote();


/* 
   This is the inset locking stuff needed for mathed --------------------

   an inset can simple call LockInset in it's edit call and *ONLY* in it's
   edit call.
   Inset::Edit() can only be called by the main lyx module.

   Then the inset may modify the menu's and/or iconbars. 

   Unlocking is either done by LyX or the inset itself with a UnlockInset-call

   During the lock, all button and keyboard events will be modified
   and send to the inset through the following inset-features. Note that
   Inset::InsetUnlock will be called from inside UnlockInset. It is meant
   to contain the code for restoring the menus and things like this.

   
   virtual void InsetButtonPress(int x, int y, int button);
   virtual void InsetButtonRelease(int x, int y, int button);
   virtual void InsetKeyPress(XKeyEvent *ev);
   virtual void InsetMotionNotify(int x, int y, int state);
   virtual void InsetUnlock();

   If a inset wishes any redraw and/or update it just has to call
   UpdateInset(this).
   It's is completly irrelevant, where the inset is. UpdateInset will
   find it in any paragraph in any buffer. 
   Of course the_locking_inset and the insets in the current paragraph/buffer
   are checked first, so no performance problem should occur.
   
   Hope that's ok for the beginning, Alejandro,
   sorry that I needed so much time,

                  Matthias
   */

void UpdateInset(Inset* inset, bool mark_dirty = true);
/* these functions return 1 if an error occured, 
   otherwise 0 */
// Now they work only for updatable insets. [Alejandro 080596]
int LockInset(UpdatableInset* inset);
void ToggleLockedInsetCursor(long x, long y, int asc, int desc);
void FitLockedInsetCursor(long x, long y, int asc, int desc);
int UnlockInset(UpdatableInset* inset);
void LockedInsetStoreUndo(Undo::undo_kind kind);

/* this is for asyncron updating. UpdateInsetUpdateList will be called
   automatically from LyX. Just insert the Inset into the Updatelist */
void UpdateInsetUpdateList();
void PutInsetIntoInsetUpdateList(Inset* inset);

InsetUpdateStruct *InsetUpdateList = 0;


/*
  -----------------------------------------------------------------------
 */

/* some function prototypes */

void GotoNote();
void OpenStuff();
void ToggleFloat();
void MenuUndo();
void MenuRedo();
void HyphenationPoint();
void MenuSeparator();
void HFill();
void Newline();
void ProtectedBlank();
void CopyCB();
int RunLinuxDoc(int, string const &);
int RunDocBook(int, string const &);
void MenuWrite(Buffer* buf);
void MenuWriteAs(Buffer *buffer);
void MenuReload(Buffer *buf);
void MenuLayoutSave();

unsigned char GetCurrentTextClass()
	// Who are we asking?
	// Shouldn't this question be directed to the buffer?
	// Indeed it should. Asger.
{
	return current_view->currentBuffer()->params.textclass;
}


// How should this actually work? Should it prohibit input in all BufferViews,
// or just in the current one? If "just the current one", then it should be
// placed in BufferView. If "all BufferViews" then LyXGUI (I think) should
// run "ProhibitInput" on all LyXViews which will run prohibitInput on all
// BufferViews. Or is it perhaps just the (input in) BufferViews in the
// current LyxView that should be prohibited (Lgb) (This applies to
// "AllowInput" as well.)
void ProhibitInput()
{
	input_prohibited = true;
	if (current_view->getScreen())
		current_view->getScreen()->HideCursor();

	static Cursor cursor;
	static bool cursor_undefined = true;
   
	if (cursor_undefined){
		cursor = XCreateFontCursor(fl_display, XC_watch);
		XFlush(fl_display);
		cursor_undefined = false;
	}
   
	/* set the cursor to the watch for all forms and the canvas */ 
	XDefineCursor(fl_display, fd_form_main->form_main->window, cursor);
	if (fd_form_paragraph->form_paragraph->visible)
		XDefineCursor(fl_display,
			      fd_form_paragraph->form_paragraph->window,
			      cursor);
	if (fd_form_character->form_character->visible)
		XDefineCursor(fl_display,
			      fd_form_character->form_character->window,
			      cursor);

	XFlush(fl_display);
	fl_deactivate_all_forms();
}


// Should find a way to move this into BufferView.C
void SetXtermCursor(Window win)
{
	static Cursor cursor;
	static char cursor_undefined = 1;
	if (cursor_undefined){
		cursor = XCreateFontCursor(fl_display, XC_xterm);
		XFlush(fl_display);
		cursor_undefined = 0;
	}
	XDefineCursor(fl_display, win, cursor);
	XFlush(fl_display);
}


void AllowInput()
{
	input_prohibited = false;

	/* reset the cursor from the watch for all forms and the canvas */
   
	XUndefineCursor(fl_display, fd_form_main->form_main->window);
	if (fd_form_paragraph->form_paragraph->visible)
		XUndefineCursor(fl_display,
				fd_form_paragraph->form_paragraph->window);
	if (fd_form_character->form_character->visible)
		XUndefineCursor(fl_display,
				fd_form_character->form_character->window);
	if (current_view->getWorkArea()->belowmouse)
		SetXtermCursor(fd_form_main->form_main->window);

	XFlush(fl_display);
	fl_activate_all_forms();
}


void FreeUpdateTimer()
{
	/* a real free timer would be better but I don't know 
	 * how to do this with xforms */
	updatetimer = 0;
}


void SetUpdateTimer(float time)
{
	fl_set_timer(fd_form_main->timer_update, time);
	updatetimer = 1;
}


void BeforeChange()
{
	current_view->getScreen()->ToggleSelection();
	current_view->currentBuffer()->text->ClearSelection();
	FreeUpdateTimer();
}


void SmallUpdate(signed char f)
{
	current_view->getScreen()->SmallUpdate();
	if (current_view->getScreen()->TopCursorVisible()
	    != current_view->getScreen()->first){
		current_view->currentBuffer()->update(f);
		return;
	}

	current_view->fitCursor();
	current_view->updateScrollbar();
   
	if (!current_view->currentBuffer()->text->selection)
		current_view->currentBuffer()->text->sel_cursor = 
			current_view->currentBuffer()->text->cursor;

	if (f==1 || f==-1) {
		if (current_view->currentBuffer()->isLyxClean()) {
			current_view->currentBuffer()->markDirty();
			minibuffer->setTimer(4);
		}
		else {
			current_view->currentBuffer()->markDirty();
		}
	}
}


//
// Menu callbacks
//

//
// File menu
//

// should be moved to lyxfunc.C
void MenuWrite(Buffer* buf)
{
	XFlush(fl_display);
	if (!bufferlist.write(buf)) {
		string fname = buf->getFileName();
		string s = MakeAbsPath(fname);
		if (AskQuestion(_("Save failed. Rename and try again?"),
				 MakeDisplayPath(s,50),
				 _("(If not, document is not saved.)"))) {
			MenuWriteAs(buf);
		}
	} else {
		lastfiles->newFile(buf->getFileName());
	}
}


// should be moved to BufferView.C
void MenuWriteAs(Buffer *buffer)
{
	if (!buffer->text) return;

	string fname = buffer->getFileName();
	string oldname = fname;
	LyXFileDlg fileDlg;

	ProhibitInput();
	fileDlg.SetButton(0, _("Documents"), lyxrc->document_path);
	fileDlg.SetButton(1, _("Templates"), lyxrc->template_path);

	if (!IsLyXFilename(fname))
		fname += ".lyx";

	fname = fileDlg.Select(_("Enter Filename to Save Document as"), 
			       OnlyPath(fname),
			       "*.lyx", 
			       OnlyFilename(fname));
	AllowInput();

	if (fname.empty()) {
		minibuffer->Set(_("Canceled."));
		return;
	}

	// Make sure the absolute filename ends with appropriate suffix
	string s= MakeAbsPath(fname);
	if (!IsLyXFilename(s))
		s += ".lyx";

	// Same name as we have already?
	if (s == oldname) {
		if (!AskQuestion(_("Same name as document already has:"),
				 MakeDisplayPath(s,50),
				 _("Save anyway?")))
			return;
		// Falls through to name change and save
	} 
	// No, but do we have another file with this name open?
	else if (bufferlist.exists(s)) {
		if (AskQuestion(_("Another document with same name open!"),
				MakeDisplayPath(s,50),
				_("Replace with current document?")))
			{
				bufferlist.close(bufferlist.getBuffer(s));

				// Ok, change the name of the buffer, but don't save!
				buffer->setFileName(s);
				buffer->markDirty();

				minibuffer->Set(_("Document renamed to '"),
						MakeDisplayPath(s),
						_("', but not saved..."));
			}
		return;
	} // Check whether the file exists
	else {
		FileInfo myfile(s);
		if (myfile.isOK() && !AskQuestion(_("Document already exists:"), 
						  MakeDisplayPath(s,50),
						  _("Replace file?")))
			return;
	}

	// Ok, change the name of the buffer
	buffer->setFileName(s);
	buffer->markDirty();
	// And save
	// Small bug: If the save fails, we have irreversible changed the name
	// of the document.
	MenuWrite(buffer);
}    

#if 0
extern bool gsworking();
#endif

int MenuRunLaTeX(Buffer *buffer)
{
	int ret = 0;

#if 0
	if (gsworking()) {
		WriteAlert(_("Sorry, can't do this while pictures are being rendered."),
			   _("Please wait a few seconds for this to finish and try again."),
			   _("(or kill runaway gs processes by hand and try again.)"));
		return 1;
	}
	extern pid_t isp_pid; // from spellchecker.C
	if(isp_pid != -1)
	{
	  WriteAlert(_("Can't do this while the spellchecker is running."),
	    _("Stop the spellchecker first."));
	  return 1;
	}
#endif
	
	if (buffer->isLinuxDoc())
		ret = RunLinuxDoc(1, buffer->getFileName());
	else if (buffer->isLiterate())
	        ret = buffer->runLiterate();
	else if (buffer->isDocBook())
		ret = RunDocBook(1, buffer->getFileName());
	else
		ret = buffer->runLaTeX();
   
	if (ret > 0) {
		string s;
		string t;
		if (ret == 1) {
			s = _("One error detected");
			t = _("You should try to fix it.");
		} else {
			s += ret;
			s += _(" errors detected.");
			t = _("You should try to fix them.");
		}
		WriteAlert(_("There were errors during the LaTeX run."), s, t);
	}
	return ret;
}


int MenuBuildProg(Buffer *buffer)
{
       int ret = 0;

       if (buffer->isLiterate())
	       ret = buffer->buildProgram();
       else {
	       string s;
	       string t;
	       s = _("Wrong type of document");
	       t = _("The Build operation is not allowed in this document");
               WriteAlert(_("There were errors during the Build process."), s, t);
	       return 1;
       }
   
       if (ret > 0) {
               string s;
               string t;
               if (ret == 1) {
                       s = _("One error detected");
                       t = _("You should try to fix it.");
               } else {
                       s += ret;
                       s += _(" errors detected.");
                       t = _("You should try to fix them.");
               }
               WriteAlert(_("There were errors during the Build process."), s, t);
       }
       return ret;
}


int MenuRunChktex(Buffer *buffer)
{
	int ret;

	if (buffer->isSGML()) {
		WriteAlert(_("Chktex does not work with SGML derived documents."));
		return 0;
	} else 
		ret = buffer->runChktex();
   
	if (ret >= 0) {
		string s;
		string t;
		if (ret == 0) {
			s = _("No warnings found.");
		} else if (ret == 1) {
			s = _("One warning found.");
			t = _("Use 'Edit->Go to Error' to find it.");
		} else {
			s += ret;
			s += _(" warnings found.");
			t = _("Use 'Edit->Go to Error' to find them.");
		}
		WriteAlert(_("Chktex run successfully"), s, t);
	} else {
		WriteAlert(_("Error!"),_("It seems chktex does not work."));
	}
	return ret;
}

 
int MakeDVIOutput(Buffer *buffer)
{
	if (!(buffer->text))
		return 1;

	int ret = 0;

	string path = OnlyPath(buffer->getFileName());
	if (lyxrc->use_tempdir || (IsDirWriteable(path) < 1)) {
		path = buffer->tmppath;
	}
	if (!buffer->isDviClean()) {
		PathPush(path);
		ret = MenuRunLaTeX(buffer);
		PathPop();
	}
	return ret;
}


/* wait == false means don't wait for termination */
/* wait == true means wait for termination       */
// The bool should be placed last on the argument line. (Lgb)
// Returns false if we fail.
bool RunScript(Buffer *buffer, bool wait,
	       string const & command, string const & orgname = string(),
	       bool need_shell=true)
{
	string path;
	string cmd;
	string name= orgname;
	int result = 0;
	
	if (MakeDVIOutput(buffer) > 0)
		return false;
	/* get DVI-Filename */
	if (name.empty())
		name = ChangeExtension(buffer->getFileName(),
				       ".dvi", true);

	path = OnlyPath(name);
	if (lyxrc->use_tempdir || (IsDirWriteable(path) < 1)) {
		path = buffer->tmppath;
	}
	PathPush(path);

	cmd = command + ' ' + SpaceLess(name);
	Systemcalls one;

	if (need_shell) {
#ifndef __EMX__
		if (!wait)
			cmd += " &";
#else
		// OS/2 cmd.exe has another use for '&'
		if (!wait) {
                        // This is not NLS safe, but it's OK, I think.
                        string sh = OnlyFilename(GetEnvPath("EMXSHELL"));
                        if (sh.empty()) {
                                // COMSPEC is set, unless user unsets 
                                sh = OnlyFilename(GetEnvPath("COMSPEC"));
				if (sh.empty())
					sh = "cmd.exe";
			}
                        sh = lowercase(sh);
                        if (sh.contains("cmd.exe") || sh.contains("4os2.exe"))
                                cmd = "start /min/n " + cmd;
                        else
                                cmd += " &";
                }
#endif
		// It seems that, if wait is false, we never get back
		// the return code of the command. This means that all
		// the code I added in PrintApplyCB is currently
		// useless...
#ifdef WITH_WARNINGS
#warning What should we do here?
#endif		
		minibuffer->Set(_("Executing command:"), cmd);
		result = one.startscript(Systemcalls::System, cmd);
	} else {
		minibuffer->Set(_("Executing command:"), cmd);
		result = one.startscript(wait ? Systemcalls::Wait
					: Systemcalls::DontWait, cmd);
	}
	PathPop();
	return (result==0);
}


// Returns false if we fail
bool MenuRunDvips(Buffer *buffer, bool wait=false)
{
	if (!buffer->text)
		return false;

	ProhibitInput();

	// Generate dvi file
        if (MakeDVIOutput(buffer) > 0) {
            	AllowInput();
		return false;
        }
	// Generate postscript file
	string ps = ChangeExtension (buffer->getFileName(),
				      ".ps_tmp", true);

	string paper;
	
	char real_papersize = buffer->params.papersize;
	if (real_papersize == PAPER_DEFAULT)
		real_papersize = lyxrc->default_papersize;

	switch (real_papersize) {
	case PAPER_USLETTER:
		paper = "letter";
		break;
	case PAPER_A3PAPER:
		paper = "a3";
		break;
	case PAPER_A4PAPER:
		paper = "a4";
		break;
	case PAPER_A5PAPER:
		paper = "a5";
		break;
	case PAPER_B5PAPER:
		paper = "b5";
		break;
	case PAPER_EXECUTIVEPAPER:
		paper = "foolscap";
		break;
	case PAPER_LEGALPAPER:
		paper = "legal";
		break;
	default: /* If nothing else fits, keep an empty value... */
		break;
	}

	// Make postscript file.
	string command = "dvips " + lyxrc->print_to_file + ' ';
	command += SpaceLess(ps);
	if (buffer->params.use_geometry
	    && buffer->params.papersize2 == VM_PAPER_CUSTOM
	    && !lyxrc->print_paper_dimension_flag.empty()
	    && !buffer->params.paperwidth.empty()
	    && !buffer->params.paperheight.empty()) {
		// using a custom papersize
		command += ' ';
		command += lyxrc->print_paper_dimension_flag + ' ';
		command += buffer->params.paperwidth + ',';
		command += buffer->params.paperheight;
	} else if (!paper.empty()
		   && (real_papersize != PAPER_USLETTER ||
		       buffer->params.orientation == ORIENTATION_PORTRAIT)) {
		// dvips won't accept -t letter -t landscape.  In all other
		// cases, include the paper size explicitly.
		command += ' ';
		command += lyxrc->print_paper_flag + ' ' + paper;
	}
	if (buffer->params.orientation == ORIENTATION_LANDSCAPE) {
		command += ' ';
		command += lyxrc->print_landscape_flag;
	}
	// push directorypath, if necessary 
        string path = OnlyPath(buffer->getFileName());
        if (lyxrc->use_tempdir || (IsDirWriteable(path) < 1)){
		path = buffer->tmppath;
        }
        PathPush(path);
	bool ret = RunScript(buffer, wait, command);
	AllowInput();
        PathPop();
	return ret;
}


// Returns false if we fail
bool MenuPreviewPS(Buffer *buffer)
{
	if (!buffer->text)
		return false;

	// Generate postscript file
	if (!MenuRunDvips(buffer, true)) {
		return false;
	}

	// Start postscript viewer
	ProhibitInput();
	string ps = ChangeExtension (buffer->getFileName(),
				      ".ps_tmp", true);
	// push directorypath, if necessary 
        string path = OnlyPath(buffer->getFileName());
        if (lyxrc->use_tempdir || (IsDirWriteable(path) < 1)){
		path = buffer->tmppath;
        }
        PathPush(path);
	bool ret = RunScript(buffer, false, lyxrc->view_ps_command, ps);
        PathPop();
	AllowInput();
	return ret;
}


void MenuFax(Buffer *buffer)
{
	if (!buffer->text)
		return;

	// Generate postscript file
	if (!MenuRunDvips(buffer, true)) {
		return;
	}

	// Send fax
	string ps = ChangeExtension (buffer->getFileName(), ".ps_tmp", true);
	string path = OnlyPath (buffer->getFileName());
	if (lyxrc->use_tempdir || (IsDirWriteable(path) < 1)) {
		path = buffer->tmppath;
	}
	PathPush(path);
	if (!lyxrc->fax_program.empty()) {
		string help2 = lyxrc->fax_program;
                subst(help2, "$$FName",ps);
                help2 += " &";
                Systemcalls one(Systemcalls::System, help2);
	} else
		send_fax(ps,lyxrc->fax_command);
	PathPop();
}


// Returns false if we fail
bool MenuPreview(Buffer *buffer)
{
	if (!buffer->text)
		return false;
   
	string paper;
	
	char real_papersize = buffer->params.papersize;
	if (real_papersize == PAPER_DEFAULT)
		real_papersize = lyxrc->default_papersize;
   
	switch (real_papersize) {
	case PAPER_USLETTER:
		paper = "us";
		break;
	case PAPER_A3PAPER:
		paper = "a3";
		break;
	case PAPER_A4PAPER:
		paper = "a4";
		break;
	case PAPER_A5PAPER:
		paper = "a5";
		break;
	case PAPER_B5PAPER:
		paper = "b5";
		break;
	case PAPER_EXECUTIVEPAPER:
		paper = "foolscap";
		break;
	case PAPER_LEGALPAPER:
		paper = "legal";
		break;
	default: /* If nothing else fits, keep the empty value */
		break;
	}
   
	if (paper.empty()) {
		if (buffer->params.orientation == ORIENTATION_LANDSCAPE)
			// we HAVE to give a size when the page is in
			// landscape, so use USletter.		
			paper = " -paper usr";
	} else {
		paper = " -paper " + paper;
		if (buffer->params.orientation == ORIENTATION_LANDSCAPE)
			paper+='r';
	}

	// push directorypath, if necessary 
        string path = OnlyPath(buffer->getFileName());
        if (lyxrc->use_tempdir || (IsDirWriteable(path) < 1)){
		path = buffer->tmppath;
        }
        PathPush(path);
	// Run dvi-viewer
	string command = lyxrc->view_dvi_command + paper ;
	bool ret = RunScript(buffer, false, command);
        PathPop();
	return ret;
}


void MenuMakeLaTeX(Buffer *buffer)
{
	if (buffer->text) {
		// Get LaTeX-Filename
		string s = SpaceLess(ChangeExtension(
						buffer->getFileName(),
						".tex", false));

		FilePtr myfile(s, FilePtr::read);
		if (myfile() &&
		    !AskQuestion(_("File already exists:"), 
				MakeDisplayPath(s,50),
				_("Do you want to overwrite the file?"))) {
			minibuffer->Set(_("Canceled"));
			return;
		}

		if (buffer->isDocBook())
			minibuffer->Set(_("DocBook does not have a latex backend"));
		else {
			if (buffer->isLinuxDoc())
				RunLinuxDoc(0, buffer->getFileName());
			else
				buffer->makeLaTeXFile(s, string(), true);
			minibuffer->Set(_("Nice LaTeX file saved as"),
					MakeDisplayPath(s));
			buffer->markDviDirty();
		}
	}
}


void MenuMakeLinuxDoc(Buffer *buffer)
{
	if (buffer->text) {

		if (!buffer->isLinuxDoc()) {
			WriteAlert(_("Error!"), _("Document class must be linuxdoc."));
			return;
		}

		// Get LinuxDoc-Filename
		string s = ChangeExtension (buffer->getFileName(), 
					     ".sgml", false);

		FilePtr myfile(s, FilePtr::read);
		if (myfile() &&
		    !AskQuestion(_("File already exists:"), 
				MakeDisplayPath(s,50),
				_("Do you want to overwrite the file?"))) {
			minibuffer->Set(_("Canceled"));
			return;
		}

		minibuffer->Set(_("Building LinuxDoc SGML file `"),
				MakeDisplayPath(s), "'..."); 

		buffer->makeLinuxDocFile(s, 65);
                buffer->redraw();
		minibuffer->Set(_("LinuxDoc SGML file save as"),
				MakeDisplayPath(s)); 
	}
}


void MenuMakeDocBook(Buffer *buffer)
{
	if (buffer->text) {

		if (!buffer->isDocBook()) {
			WriteAlert(_("Error!"), _("Document class must be docbook."));
			return;
		}

		// Get DocBook-Filename
		string s = ChangeExtension (buffer->getFileName(), 
					     ".sgml", false);

		FilePtr myfile(s, FilePtr::read);
		if (myfile() &&
		    !AskQuestion(_("File already exists:"), 
				MakeDisplayPath(s,50),
				_("Do you want to overwrite the file?"))) {
			minibuffer->Set(_("Canceled"));
			return;
		}

		minibuffer->Set(_("Building DocBook SGML file `"),
				MakeDisplayPath(s), "'..."); 

		buffer->makeDocBookFile(s, 65);
                buffer->redraw();
		minibuffer->Set(_("DocBook SGML file save as"),
				MakeDisplayPath(s)); 
	}
}


void MenuMakeAscii(Buffer *buffer)
{
	if (buffer->text) {
		/* get LaTeX-Filename */
		string s = ChangeExtension (buffer->getFileName(),
					     ".txt", false);

		FilePtr myfile(s, FilePtr::read);
		if (myfile() &&
		    !AskQuestion(_("File already exists:"), 
				MakeDisplayPath(s,50),
				_("Do you want to overwrite the file?"))) {
			minibuffer->Set(_("Canceled"));
			return;
		}

		buffer->writeFileAscii(s, lyxrc->ascii_linelen);

		minibuffer->Set(_("Ascii file saved as"), MakeDisplayPath(s));
	}
}


void MenuPrint(Buffer *buffer)
{
	string input_file;

	if (!buffer->text) 
		return;
	
	input_file = ChangeExtension(buffer->getFileName(),
				     lyxrc->print_file_extension,
				     true);
	fl_set_input(fd_form_print->input_file, input_file.c_str());
	
	if (fd_form_print->form_print->visible) {
		fl_raise_form(fd_form_print->form_print);
	} 
	else {
		fl_show_form(fd_form_print->form_print,
			     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_FULLBORDER,
			     _("Print"));
	}
}


void QuitLyX()
{
	lyxerr.debug() << "Running QuitLyX." << endl;

	if (!bufferlist.QwriteAll())
		return;

	lastfiles->writeFile(lyxrc->lastfiles);

	// Set a flag that we do quitting from the program,
	// so no refreshes are necessary.
	quitting = true;

	// close buffers first
	bufferlist.closeAll();

	// do any other cleanup procedures now
	lyxerr.debug() << "Deleting tmp dir " << system_tempdir << endl;

	DestroyLyXTmpDir(system_tempdir);

	finished = true;
}



void AutoSave()
	// should probably be moved into BufferList (Lgb)
	// Perfect target for a thread...
{
	if (!current_view->getScreen() || !current_view->available())
		return;

	if (current_view->currentBuffer()->isBakClean()
	    || current_view->currentBuffer()->isReadonly()) {
		// We don't save now, but we'll try again later
		current_view->getOwner()->resetAutosaveTimer();
		return;
	}

	minibuffer->Set(_("Autosaving current document..."));
	
	// create autosave filename
	string fname =	OnlyPath(current_view->currentBuffer()->getFileName());
	fname += "#";
	fname += OnlyFilename(current_view->currentBuffer()->getFileName());
	fname += "#";
	
	// tmp_ret will be located (usually) in /tmp
	// will that be a problem?
	string tmp_ret = tmpnam(0);
	
	pid_t pid = fork(); // If you want to debug the autosave
	// you should set pid to -1, and comment out the
	// fork.
	if (pid == 0 || pid == -1) {
		// pid = -1 signifies that lyx was unable
		// to fork. But we will do the save
		// anyway.
		bool failed = false;
		if (!tmp_ret.empty()) {
			current_view->currentBuffer()->writeFile(tmp_ret, 1);
			// assume successful write of tmp_ret
			if (rename(tmp_ret.c_str(), fname.c_str()) == -1) {
				failed = true;
				// most likely couldn't move between filesystems
				// unless write of tmp_ret failed
				// so remove tmp file (if it exists)
				remove(tmp_ret.c_str());
			}
		} else {
			failed = true;
		}
		
		if (failed) {
			// failed to write/rename tmp_ret so try writing direct
			if (!current_view->currentBuffer()->writeFile(fname,
								      1)) {
				// It is dangerous to do this in the child,
				// but safe in the parent, so...
				if (pid == -1)
					minibuffer->Set(_("Autosave Failed!"));
			}
		}
		if (pid == 0) { // we are the child so...
			_exit(0);
		}
	}
	
	current_view->currentBuffer()->markBakClean();
	current_view->getOwner()->resetAutosaveTimer();
}


//
// (c) CHT Software Service GmbH
// Uwe C. Schroeder
//
// create new file with template
// SERVERCMD !
//
Buffer * NewLyxFile(string const & filename)
{
	// Split argument by :
	string name;
	string tmpname=split(filename, name, ':');
#ifdef __EMX__ // Fix me! lyx_cb.C may not be low level enough to allow this.
	if (name.length() == 1 && isalpha((unsigned char) name[0]) &&
	    (tmpname.prefixIs("/") || tmpname.prefixIs("\\"))) {
		name += ':';
		name += tmpname.token(':');
		tmpname = split(tmpname, ':');
	}
#endif
	lyxerr.debug() << "Arg is " << filename
		       << "\nName is " << name
		       << "\nTemplate is " << tmpname << endl;

	// find a free buffer 
	Buffer *tmpbuf = bufferlist.newFile(name,tmpname);
	if (tmpbuf)
		lastfiles->newFile(tmpbuf->getFileName());
	return tmpbuf;
}


// Insert ascii file (if filename is empty, prompt for one)
void InsertAsciiFile(string const & f, bool asParagraph)
{
	string fname = f;
	LyXParagraph *tmppar;
	LyXFileDlg fileDlg;
 
	if (!current_view->getScreen()) return;
     
	if (fname.empty()) {
		ProhibitInput();
		fname = fileDlg.Select(_("File to Insert"), 
				       current_view->getOwner()->currentBuffer()->filepath,
				       "*");
  		AllowInput();
		if (fname.empty()) return;
	}

	FileInfo fi(fname);
	FilePtr myfile(fname, FilePtr::read);

	if (!fi.exist() || !fi.readable() || !myfile()) {
		WriteFSAlert(_("Error! Cannot open specified file:"),
			     MakeDisplayPath(fname,50));
		return;
	}
	
	tmppar = new LyXParagraph();
	tmppar->readSimpleWholeFile(myfile);
	
	// set the end of the string
	tmppar->InsertChar(tmppar->last-1,'\0');
      
	// insert the string
	current_view->getScreen()->HideCursor();
      
	// clear the selection
	BeforeChange();
	if (!asParagraph)
		current_view->currentBuffer()->text->InsertStringA(tmppar->text);
	else
		current_view->currentBuffer()->text->InsertStringB(tmppar->text);
	delete tmppar;
	current_view->currentBuffer()->update(1);
}


void MenuShowTableOfContents()
{
	TocUpdateCB(0, 0);
	if (fd_form_toc->form_toc->visible) {
		fl_raise_form(fd_form_toc->form_toc);
	} else {
		fl_show_form(fd_form_toc->form_toc,
			     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_FULLBORDER,
			     _("Table Of Contents"));
	}
}


void MenuInsertLabel(const char *arg)
{
	string label = arg;
	ProhibitInput();
	//string label = fl_show_input(_("Enter new label to insert:"),"");
	if (label.empty())
		label = frontStrip(strip(askForText(_("Enter new label to insert:"))));
	if (!label.empty()) {
		InsetLabel *new_inset = new InsetLabel;
		new_inset->setContents(label);
		current_view->currentBuffer()->insertInset(new_inset);
	}
	AllowInput();
}


void MenuInsertRef()
{
	RefUpdateCB(0, 0);
	if (fd_form_ref->form_ref->visible) {
		fl_raise_form(fd_form_ref->form_ref);
	} else {
		fl_show_form(fd_form_ref->form_ref,
			     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_FULLBORDER,
			     _("Insert Reference"));
	}
}


void MenuPasteSelection(char at)
{
	if (!current_view->getScreen())
		return;

	ascii_type = at;
  
	Atom data_prop = XInternAtom(fl_display, 
				     "LyX_Primary",
				     false);
	if (data_prop == None) 
		return;
	XConvertSelection(fl_display,
			  XA_PRIMARY, XA_STRING, data_prop, 
			  fd_form_main->form_main->window, 0);
	XFlush(fl_display);
}


void FootCB(FL_OBJECT*, long)
{
	if (!current_view->available()) 
		return;
	
	minibuffer->Set(_("Inserting Footnote..."));
	current_view->getScreen()->HideCursor();
	current_view->currentBuffer()->update(-2);
	current_view->currentBuffer()->text->InsertFootnoteEnvironment(LyXParagraph::FOOTNOTE);
	current_view->currentBuffer()->update(1);
}


void LayoutsCB(int sel, void *)
{
	string tmp;
	tmp += sel;
	current_view->getOwner()->getLyXFunc()->Dispatch(LFUN_LAYOUTNO,
							 tmp.c_str());
}


/*
 * SGML Linuxdoc support:
 * (flag == -1) import SGML file
 * (flag == 0) make TeX output
 * (flag == 1) make dvi output
 */
int RunLinuxDoc(int flag, string const & filename)
{
	string name;
	string s2;
	string path;
	string add_flags;

	int errorcode = 0;

	/* generate a path-less extension name */
	name = ChangeExtension (filename, ".sgml", true);
	path = OnlyPath (filename);
	if (lyxrc->use_tempdir || (IsDirWriteable(path) < 1)) {
		path = current_view->currentBuffer()->tmppath;
	}
	PathPush (path);
	
	if (flag != -1) {
		if (!current_view->available())
			return 0;
		current_view->currentBuffer()->makeLinuxDocFile(name,0);
		LYX_PAPER_SIZE ps = (LYX_PAPER_SIZE) current_view->currentBuffer()->params.papersize;
		switch (ps) {
		case PAPER_A4PAPER:  add_flags = "-p a4";     break;
		case PAPER_USLETTER: add_flags = "-p letter"; break;
		default: /* nothing to be done yet ;-) */     break; 
		}
	}
	
	ProhibitInput();
	
	Systemcalls one;
	switch (flag) {
	case -1: /* Import file */
		minibuffer->Set(_("Importing LinuxDoc SGML file `"), 
				MakeDisplayPath(filename), "'...");
		s2 = "sgml2lyx " + lyxrc->sgml_extra_options + ' ' 
			+ name;
		if (one.startscript(Systemcalls::System, s2)) 
			errorcode = 1;
		break;
	case 0: /* TeX output asked */
		minibuffer->Set(_("Converting LinuxDoc SGML to TeX file..."));
		s2 = "sgml2latex " + add_flags + " -o tex "
			+ lyxrc->sgml_extra_options + ' ' + name;
		if (one.startscript(Systemcalls::System, s2)) 
			errorcode = 1;
		break;
	case 1: /* dvi output asked */
		minibuffer->Set(_("Converting LinuxDoc SGML to dvi file..."));
		s2 = "sgml2latex " + add_flags + " -o dvi "
			+ lyxrc->sgml_extra_options + ' ' + name;
		if (one.startscript(Systemcalls::System, s2)) {
			errorcode = 1;
		} else
			current_view->currentBuffer()->markDviClean();
		break;
	default: /* unknown output */
		break;
	}
	
	PathPop();
	AllowInput();

        current_view->currentBuffer()->redraw();
	return errorcode;
}


/*
 * SGML DocBook support:
 * (flag == 1) make dvi output
 */
int RunDocBook(int flag, string const & filename)
{
	string name;
	string s2;
	string path;

	int errorcode = 0;

	/* generate a path-less extension name */
	name = ChangeExtension (filename, ".sgml", true);
	path = OnlyPath (filename);
	if (lyxrc->use_tempdir || (IsDirWriteable(path) < 1)) {
		path = current_view->currentBuffer()->tmppath;
	}
	PathPush (path);

	if (!current_view->available())
		return 0;
	
	current_view->currentBuffer()->makeDocBookFile(name,0);
#if 0
	string add_flags;
	LYX_PAPER_SIZE ps = (LYX_PAPER_SIZE) current_view->currentBuffer()->params.papersize;
	switch (ps) {
	case PAPER_A4PAPER:  add_flags = "-p a4";     break;
	case PAPER_USLETTER: add_flags = "-p letter"; break;
	default: /* nothing to be done yet ;-) */     break; 
	}
#endif	
	ProhibitInput();
	
	Systemcalls one;
	switch (flag) {
	case 1: /* dvi output asked */
		minibuffer->Set(_("Converting DocBook SGML to dvi file..."));
		s2 = "sgmltools --backend dvi " + name;
		if (one.startscript(Systemcalls::System, s2)) {
			errorcode = 1;
		} else
			current_view->currentBuffer()->markDviClean();
		break;
	default: /* unknown output */
		break;
	}
	
	PathPop();
	AllowInput();

        current_view->currentBuffer()->redraw();
	return errorcode;
}


void AllFloats(char flag, char figmar)
{
	if (!current_view->available())
		return;
   
	LyXCursor cursor = current_view->currentBuffer()->text->cursor;

	if (!flag && cursor.par->footnoteflag != LyXParagraph::NO_FOOTNOTE
	    && ((figmar 
		 && cursor.par->footnotekind != LyXParagraph::FOOTNOTE 
		 && cursor.par->footnotekind != LyXParagraph::MARGIN)
		|| (!figmar
		    && cursor.par->footnotekind != LyXParagraph::FIG 
		    && cursor.par->footnotekind != LyXParagraph::TAB
 		    && cursor.par->footnotekind != LyXParagraph::WIDE_FIG 
 		    && cursor.par->footnotekind != LyXParagraph::WIDE_TAB
		    && cursor.par->footnotekind != LyXParagraph::ALGORITHM)))
		ToggleFloat();
	else
		BeforeChange();

	LyXCursor tmpcursor = cursor;
	cursor.par = tmpcursor.par->ParFromPos(tmpcursor.pos);
	cursor.pos = tmpcursor.par->PositionInParFromPos(tmpcursor.pos);

	LyXParagraph *par = current_view->currentBuffer()->paragraph;
	while (par) {
		if (flag) {
			if (par->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE
			    && (
				    (figmar 
				     &&
				     par->footnotekind != LyXParagraph::FOOTNOTE 
				     &&
				     par->footnotekind !=  LyXParagraph::MARGIN
					    )
				    ||
				    (!figmar
				     &&
				     par->footnotekind != LyXParagraph::FIG 
				     &&
				     par->footnotekind != LyXParagraph::TAB
				     &&
 				     par->footnotekind != LyXParagraph::WIDE_FIG 
 				     &&
 				     par->footnotekind != LyXParagraph::WIDE_TAB
 				     &&
				     par->footnotekind != LyXParagraph::ALGORITHM
					    )
				    )
				){
				if (par->previous
				    && par->previous->footnoteflag !=
				    LyXParagraph::CLOSED_FOOTNOTE){ /* should be */ 
					current_view->currentBuffer()->text->SetCursorIntern(par->previous,
								      0);
					current_view->currentBuffer()->text->OpenFootnote();
				}
			}
		}
		else  {
			if (par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE
			    && (
				    (figmar 
				     &&
				     par->footnotekind != LyXParagraph::FOOTNOTE 
				     &&
				     par->footnotekind !=  LyXParagraph::MARGIN
					    )
				    ||
				    (!figmar
				     &&
				     par->footnotekind != LyXParagraph::FIG 
				     &&
				     par->footnotekind != LyXParagraph::TAB
				     &&
 				     par->footnotekind != LyXParagraph::WIDE_FIG 
 				     &&
 				     par->footnotekind != LyXParagraph::WIDE_TAB
 				     &&
				     par->footnotekind != LyXParagraph::ALGORITHM
					    )
				    )
				){
				current_view->currentBuffer()->text->SetCursorIntern(par, 0);
				current_view->currentBuffer()->text->CloseFootnote();
			}
		}
		par = par->next;
	}

	current_view->currentBuffer()->text->SetCursorIntern(cursor.par, cursor.pos);
	current_view->redraw();
	current_view->fitCursor();
	current_view->updateScrollbar();
}


void MenuLayoutCharacter()
{
	if (fd_form_character->form_character->visible) {
		fl_raise_form(fd_form_character->form_character);
	} else {
		fl_show_form(fd_form_character->form_character,
			     FL_PLACE_MOUSE | FL_FREE_SIZE,FL_FULLBORDER,
			     _("Character Style"));
	}
}


inline void DeactivateParagraphButtons ()
{
	fl_deactivate_object (fd_form_paragraph->button_ok);
	fl_deactivate_object (fd_form_paragraph->button_apply);
	fl_set_object_lcol (fd_form_paragraph->button_ok, FL_INACTIVE);
	fl_set_object_lcol (fd_form_paragraph->button_apply, FL_INACTIVE);
}

inline void ActivateParagraphButtons ()
{
	fl_activate_object (fd_form_paragraph->button_ok);
	fl_activate_object (fd_form_paragraph->button_apply);
	fl_set_object_lcol (fd_form_paragraph->button_ok, FL_BLACK);
	fl_set_object_lcol (fd_form_paragraph->button_apply, FL_BLACK);
}

inline void DisableParagraphLayout ()
{
        DeactivateParagraphButtons();
	fl_deactivate_object (fd_form_paragraph->input_labelwidth);
	fl_deactivate_object (fd_form_paragraph->check_lines_top);
	fl_deactivate_object (fd_form_paragraph->check_lines_bottom);
	fl_deactivate_object (fd_form_paragraph->check_pagebreaks_top);
	fl_deactivate_object (fd_form_paragraph->check_pagebreaks_bottom);
	fl_deactivate_object (fd_form_paragraph->check_noindent);
	fl_deactivate_object (fd_form_paragraph->group_radio_alignment);
	fl_deactivate_object (fd_form_paragraph->radio_align_right);
	fl_deactivate_object (fd_form_paragraph->radio_align_left);
	fl_deactivate_object (fd_form_paragraph->radio_align_block);
	fl_deactivate_object (fd_form_paragraph->radio_align_center);
	fl_deactivate_object (fd_form_paragraph->input_space_above);
	fl_deactivate_object (fd_form_paragraph->input_space_below);
	fl_deactivate_object (fd_form_paragraph->choice_space_above);
	fl_deactivate_object (fd_form_paragraph->choice_space_below);
	fl_deactivate_object (fd_form_paragraph->check_space_above);
	fl_deactivate_object (fd_form_paragraph->check_space_below);
}

inline void EnableParagraphLayout ()
{
        ActivateParagraphButtons();
	fl_activate_object (fd_form_paragraph->input_labelwidth);
	fl_activate_object (fd_form_paragraph->check_lines_top);
	fl_activate_object (fd_form_paragraph->check_lines_bottom);
	fl_activate_object (fd_form_paragraph->check_pagebreaks_top);
	fl_activate_object (fd_form_paragraph->check_pagebreaks_bottom);
	fl_activate_object (fd_form_paragraph->check_noindent);
	fl_activate_object (fd_form_paragraph->group_radio_alignment);
	fl_activate_object (fd_form_paragraph->radio_align_right);
	fl_activate_object (fd_form_paragraph->radio_align_left);
	fl_activate_object (fd_form_paragraph->radio_align_block);
	fl_activate_object (fd_form_paragraph->radio_align_center);
	fl_activate_object (fd_form_paragraph->input_space_above);
	fl_activate_object (fd_form_paragraph->input_space_below);
	fl_activate_object (fd_form_paragraph->choice_space_above);
	fl_activate_object (fd_form_paragraph->choice_space_below);
	fl_activate_object (fd_form_paragraph->check_space_above);
	fl_activate_object (fd_form_paragraph->check_space_below);
}

bool UpdateLayoutParagraph()
{
	if (!current_view->getScreen() || !current_view->available()) {
		if (fd_form_paragraph->form_paragraph->visible) 
			fl_hide_form(fd_form_paragraph->form_paragraph);
		return false;
	}

	Buffer * buf = current_view->currentBuffer();

	fl_set_input(fd_form_paragraph->input_labelwidth,
		     buf->text->cursor.par->GetLabelWidthString().c_str());
	fl_set_button(fd_form_paragraph->radio_align_right, 0);
	fl_set_button(fd_form_paragraph->radio_align_left, 0);
	fl_set_button(fd_form_paragraph->radio_align_center, 0);
	fl_set_button(fd_form_paragraph->radio_align_block, 0);

	int align = buf->text->cursor.par->GetAlign();
	if (align == LYX_ALIGN_LAYOUT)
		align =	lyxstyle.Style(buf->params.textclass,
				       buf->text->cursor.par->GetLayout())->align;
	 
	switch (align) {
	case LYX_ALIGN_RIGHT:
		fl_set_button(fd_form_paragraph->radio_align_right, 1);
		break;
	case LYX_ALIGN_LEFT:
		fl_set_button(fd_form_paragraph->radio_align_left, 1);
		break;
	case LYX_ALIGN_CENTER:
		fl_set_button(fd_form_paragraph->radio_align_center, 1);
		break;
	default:
		fl_set_button(fd_form_paragraph->radio_align_block, 1);
		break;
	}
	 
	fl_set_button(fd_form_paragraph->check_lines_top,
		      buf->text->cursor.par->FirstPhysicalPar()->line_top);
	fl_set_button(fd_form_paragraph->check_lines_bottom,
		      buf->text->cursor.par->FirstPhysicalPar()->line_bottom);
	fl_set_button(fd_form_paragraph->check_pagebreaks_top,
		      buf->text->cursor.par->FirstPhysicalPar()->pagebreak_top);
	fl_set_button(fd_form_paragraph->check_pagebreaks_bottom,
		      buf->text->cursor.par->FirstPhysicalPar()->pagebreak_bottom);
	fl_set_button(fd_form_paragraph->check_noindent,
		      buf->text->cursor.par->FirstPhysicalPar()->noindent);
	fl_set_input (fd_form_paragraph->input_space_above, "");
	
	switch (buf->text->cursor.par->FirstPhysicalPar()->added_space_top.kind()) {
	case VSpace::NONE:
		fl_set_choice (fd_form_paragraph->choice_space_above,
			       1);
		break;
	case VSpace::DEFSKIP:
		fl_set_choice (fd_form_paragraph->choice_space_above,
			       2);
		break;
	case VSpace::SMALLSKIP:
		fl_set_choice (fd_form_paragraph->choice_space_above,
			       3);
		break;
	case VSpace::MEDSKIP:
		fl_set_choice (fd_form_paragraph->choice_space_above,
			       4);
		break;
	case VSpace::BIGSKIP:
		fl_set_choice (fd_form_paragraph->choice_space_above,
			       5);
		break;
	case VSpace::VFILL:
		fl_set_choice (fd_form_paragraph->choice_space_above,
			       6);
		break;
	case VSpace::LENGTH:
		fl_set_choice (fd_form_paragraph->choice_space_above,
			       7); 
		fl_set_input  (fd_form_paragraph->input_space_above, 
			       buf->text->cursor.par->FirstPhysicalPar()->added_space_top.length().asString().c_str());
		break;
	}
	fl_set_button (fd_form_paragraph->check_space_above,
		       buf->text->cursor.par->FirstPhysicalPar()->added_space_top.keep());
	fl_set_input (fd_form_paragraph->input_space_below, "");
	switch (buf->text->cursor.par->FirstPhysicalPar()->added_space_bottom.kind()) {
	case VSpace::NONE:
		fl_set_choice (fd_form_paragraph->choice_space_below,
			       1);
		break;
	case VSpace::DEFSKIP:
		fl_set_choice (fd_form_paragraph->choice_space_below,
			       2);
		break;
	case VSpace::SMALLSKIP:
		fl_set_choice (fd_form_paragraph->choice_space_below,
			       3);
		break;
	case VSpace::MEDSKIP:
		fl_set_choice (fd_form_paragraph->choice_space_below,
			       4);
		break;
	case VSpace::BIGSKIP:
		fl_set_choice (fd_form_paragraph->choice_space_below,
			       5);
		break;
	case VSpace::VFILL:
		fl_set_choice (fd_form_paragraph->choice_space_below,
			       6);
		break;
	case VSpace::LENGTH:
		fl_set_choice (fd_form_paragraph->choice_space_below,
			       7); 
		fl_set_input  (fd_form_paragraph->input_space_below, 
			       buf->text->cursor.par->FirstPhysicalPar()->added_space_bottom.length().asString().c_str());
		break;
	}
	fl_set_button (fd_form_paragraph->check_space_below,
		       buf->text->cursor.par->FirstPhysicalPar()->added_space_bottom.keep());

	fl_set_button(fd_form_paragraph->check_noindent,
		      buf->text->cursor.par->FirstPhysicalPar()->noindent);

	if (current_view->currentBuffer()->isReadonly()) {
		DisableParagraphLayout();
	} else {
		EnableParagraphLayout();
	}
	return true;
}

void MenuLayoutParagraph()
{
	if (UpdateLayoutParagraph()) {
		if (fd_form_paragraph->form_paragraph->visible) {
			fl_raise_form(fd_form_paragraph->form_paragraph);
		} else {
			fl_show_form(fd_form_paragraph->form_paragraph,
				     FL_PLACE_MOUSE | FL_FREE_SIZE,FL_FULLBORDER,
				     _("Paragraph Environment"));
		}
	}
}


inline
void DeactivateDocumentButtons ()
{
	fl_deactivate_object (fd_form_document->button_ok);
	fl_deactivate_object (fd_form_document->button_apply);
	fl_set_object_lcol (fd_form_document->button_ok, FL_INACTIVE);
	fl_set_object_lcol (fd_form_document->button_apply, FL_INACTIVE);
}


inline
void ActivateDocumentButtons ()
{
	fl_activate_object (fd_form_document->button_ok);
	fl_activate_object (fd_form_document->button_apply);
	fl_set_object_lcol (fd_form_document->button_ok, FL_BLACK);
	fl_set_object_lcol (fd_form_document->button_apply, FL_BLACK);
}

inline
void DisableDocumentLayout ()
{
        DeactivateDocumentButtons ();
	fl_deactivate_object (fd_form_document->group_radio_separation);
	fl_deactivate_object (fd_form_document->radio_indent);
	fl_deactivate_object (fd_form_document->radio_skip);
	fl_deactivate_object (fd_form_document->choice_class);
	fl_deactivate_object (fd_form_document->choice_pagestyle);
	fl_deactivate_object (fd_form_document->choice_fonts);
	fl_deactivate_object (fd_form_document->choice_fontsize);
	fl_deactivate_object (fd_form_document->input_float_placement);
	fl_deactivate_object (fd_form_document->choice_postscript_driver);
	fl_deactivate_object (fd_form_document->choice_inputenc);
	fl_deactivate_object (fd_form_document->group_radio_sides);
	fl_deactivate_object (fd_form_document->radio_sides_one);
	fl_deactivate_object (fd_form_document->radio_sides_two);
	fl_deactivate_object (fd_form_document->group_radio_columns);
	fl_deactivate_object (fd_form_document->radio_columns_one);
	fl_deactivate_object (fd_form_document->radio_columns_two);
	fl_deactivate_object (fd_form_document->input_extra);
	fl_deactivate_object (fd_form_document->choice_language);
	combo_language->deactivate();
	fl_deactivate_object (fd_form_document->input_default_skip);
	fl_deactivate_object (fd_form_document->choice_default_skip);
	fl_deactivate_object (fd_form_document->slider_secnumdepth);
	fl_deactivate_object (fd_form_document->slider_tocdepth);
	fl_deactivate_object (fd_form_document->choice_spacing);
	fl_deactivate_object (fd_form_document->input_spacing);
	fl_deactivate_object (fd_form_document->check_use_amsmath);
}

inline
void EnableDocumentLayout ()
{
        ActivateDocumentButtons ();
	fl_activate_object (fd_form_document->group_radio_separation);
	fl_activate_object (fd_form_document->radio_indent);
	fl_activate_object (fd_form_document->radio_skip);
	fl_activate_object (fd_form_document->choice_class);
	fl_activate_object (fd_form_document->choice_pagestyle);
	fl_activate_object (fd_form_document->choice_fonts);
	fl_activate_object (fd_form_document->choice_fontsize);
	fl_activate_object (fd_form_document->input_float_placement);
	fl_activate_object (fd_form_document->choice_postscript_driver);
	fl_activate_object (fd_form_document->choice_inputenc);
	fl_activate_object (fd_form_document->group_radio_sides);
	fl_activate_object (fd_form_document->radio_sides_one);
	fl_activate_object (fd_form_document->radio_sides_two);
	fl_activate_object (fd_form_document->group_radio_columns);
	fl_activate_object (fd_form_document->radio_columns_one);
	fl_activate_object (fd_form_document->radio_columns_two);
	fl_activate_object (fd_form_document->input_extra);
	fl_activate_object (fd_form_document->choice_language);
	combo_language->activate();
	fl_activate_object (fd_form_document->input_default_skip);
	fl_activate_object (fd_form_document->choice_default_skip);
	fl_activate_object (fd_form_document->slider_secnumdepth);
	fl_activate_object (fd_form_document->slider_tocdepth);
	fl_activate_object (fd_form_document->choice_spacing);
	fl_activate_object (fd_form_document->input_spacing);
	fl_activate_object (fd_form_document->check_use_amsmath);
}

bool UpdateLayoutDocument(BufferParams *params)
{
	if (!current_view->getScreen() || !current_view->available()) {
		if (fd_form_document->form_document->visible) 
			fl_hide_form(fd_form_document->form_document);
		return false;
	}		

	if (params == 0)
		params = &current_view->currentBuffer()->params;
	LyXTextClass *tclass = lyxstyle.TextClass(params->textclass);
	
	fl_set_choice_text(fd_form_document->choice_class, 
			   lyxstyle.DescOfClass(params->textclass).c_str());
	combo_language->select_text(params->language.c_str());
	
	fl_set_choice_text(fd_form_document->choice_fonts, 
			   params->fonts.c_str());
	fl_set_choice_text(fd_form_document->choice_inputenc, 
			   params->inputenc.c_str());
	fl_set_choice_text(fd_form_document->choice_postscript_driver, 
			   params->graphicsDriver.c_str());

	// ale970405+lasgoutt970513
	fl_clear_choice(fd_form_document->choice_fontsize);
	fl_addto_choice(fd_form_document->choice_fontsize, "default");
	fl_addto_choice(fd_form_document->choice_fontsize, 
			tclass->opt_fontsize.c_str());
	fl_set_choice(fd_form_document->choice_fontsize, 
		      tokenPos(tclass->opt_fontsize, '|', params->fontsize) + 2);

	// ale970405+lasgoutt970513
	fl_clear_choice(fd_form_document->choice_pagestyle);
	fl_addto_choice(fd_form_document->choice_pagestyle, "default");
	fl_addto_choice(fd_form_document->choice_pagestyle, 
			tclass->opt_pagestyle.c_str());
    
	fl_set_choice(fd_form_document->choice_pagestyle,
		      tokenPos(tclass->opt_pagestyle, '|', params->pagestyle) + 2);

	fl_set_button(fd_form_document->radio_indent, 0);
	fl_set_button(fd_form_document->radio_skip, 0);
    
        
	fl_set_button(fd_form_document->check_use_amsmath, params->use_amsmath);

	if (params->paragraph_separation == LYX_PARSEP_INDENT)
		fl_set_button(fd_form_document->radio_indent, 1);
	else
		fl_set_button(fd_form_document->radio_skip, 1);

	switch (params->getDefSkip().kind()) {
	case VSpace::SMALLSKIP: 
		fl_set_choice (fd_form_document->choice_default_skip, 1);
		break;
	case VSpace::MEDSKIP: 
		fl_set_choice (fd_form_document->choice_default_skip, 2);
		break;
	case VSpace::BIGSKIP: 
		fl_set_choice (fd_form_document->choice_default_skip, 3);
		break;
	case VSpace::LENGTH: 
		fl_set_choice (fd_form_document->choice_default_skip, 4);
		fl_set_input (fd_form_document->input_default_skip,
			      params->getDefSkip().asLyXCommand().c_str());
		break;
	default:
		fl_set_choice (fd_form_document->choice_default_skip, 2);
		break;
	}
   
	fl_set_button(fd_form_document->radio_sides_one, 0);
	fl_set_button(fd_form_document->radio_sides_two, 0);
   
	if (params->sides == 2)
		fl_set_button(fd_form_document->radio_sides_two, 1);
	else
		fl_set_button(fd_form_document->radio_sides_one, 1);
   
	fl_set_button(fd_form_document->radio_columns_one, 0);
	fl_set_button(fd_form_document->radio_columns_two, 0);
   
	if (params->columns == 2)
		fl_set_button(fd_form_document->radio_columns_two, 1);
	else
		fl_set_button(fd_form_document->radio_columns_one, 1);
   
	fl_set_input(fd_form_document->input_spacing, "");
	switch (params->spacing.getSpace()) {
	case Spacing::Single:
		{
			// \singlespacing
			fl_set_choice(fd_form_document->choice_spacing, 1);
			break;
		}
	case Spacing::Onehalf:
		{
			// \onehalfspacing
			fl_set_choice(fd_form_document->choice_spacing, 2);
			break;
		}
	case Spacing::Double:
		{
			// \ doublespacing
			fl_set_choice(fd_form_document->choice_spacing, 3);
			break;
		}
	case Spacing::Other:
		{
			fl_set_choice(fd_form_document->choice_spacing, 4);
			char sval[20];
			sprintf(sval,"%g",params->spacing.getValue()); 
			fl_set_input(fd_form_document->input_spacing, sval);
			break;
		}
	}


	fl_set_counter_value(fd_form_document->slider_secnumdepth, 
			     params->secnumdepth);
	fl_set_counter_value(fd_form_document->slider_tocdepth, 
			     params->tocdepth);
	if (!params->float_placement.empty()) { // buffer local (Lgb)
		fl_set_input(fd_form_document->input_float_placement,
			     params->float_placement.c_str());
	} else {
		fl_set_input(fd_form_document->input_float_placement, "");
	}
	if (!params->options.empty())
		fl_set_input(fd_form_document->input_extra,
			     params->options.c_str());
	else
		fl_set_input(fd_form_document->input_extra, "");

	if (current_view->currentBuffer()->isSGML()) {
		// bullets not used in SGML derived documents
		fl_deactivate_object(fd_form_document->button_bullets);
		fl_set_object_lcol(fd_form_document->button_bullets,
				   FL_INACTIVE);
	} else {
		fl_activate_object(fd_form_document->button_bullets);
		fl_set_object_lcol(fd_form_document->button_bullets,
				   FL_BLACK);
	}

	if (current_view->currentBuffer()->isReadonly()) {
		DisableDocumentLayout();
	} else {
		EnableDocumentLayout();
	}

	return true;
}

void MenuLayoutDocument()
{
	if (UpdateLayoutDocument()) {
		if (fd_form_document->form_document->visible) {
			fl_raise_form(fd_form_document->form_document);
		} else {
			fl_show_form(fd_form_document->form_document,
				     FL_PLACE_MOUSE | FL_FREE_SIZE,
				     FL_FULLBORDER,
				     _("Document Layout"));
		}
	}
}


bool UpdateLayoutQuotes()
{
	bool update = true;
	if (!current_view->getScreen()
	    || !current_view->available()
	    || current_view->currentBuffer()->isReadonly())
		update = false;
	
	if (update) {
		fl_set_choice(fd_form_quotes->choice_quotes_language,
		      current_view->currentBuffer()->params.quotes_language + 1);
		fl_set_button(fd_form_quotes->radio_single, 0);
		fl_set_button(fd_form_quotes->radio_double, 0);
	
		if (current_view->currentBuffer()->params.quotes_times == InsetQuotes::SingleQ)
			fl_set_button(fd_form_quotes->radio_single, 1);
		else
			fl_set_button(fd_form_quotes->radio_double, 1);
	} else if (fd_form_quotes->form_quotes->visible) {
		fl_hide_form(fd_form_quotes->form_quotes);
	}
	return update;
}

void MenuLayoutQuotes()
{
	if (UpdateLayoutQuotes()) {
		if (fd_form_quotes->form_quotes->visible) {
			fl_raise_form(fd_form_quotes->form_quotes);
		} else {
			fl_show_form(fd_form_quotes->form_quotes,
				     FL_PLACE_MOUSE | FL_FREE_SIZE,
				     FL_FULLBORDER,
				     _("Quotes"));
		}
	}
}


bool UpdateLayoutPreamble()
{
	bool update = true;
	if (!current_view->getScreen() || ! current_view->available())
	    update = false;

	if (update) {
		fl_set_input(fd_form_preamble->input_preamble,
		     current_view->currentBuffer()->params.preamble.c_str());

		if (current_view->currentBuffer()->isReadonly()) {
		  fl_deactivate_object(fd_form_preamble->input_preamble);
		  fl_deactivate_object(fd_form_preamble->button_ok);
		  fl_deactivate_object(fd_form_preamble->button_apply);
		  fl_set_object_lcol(fd_form_preamble->button_ok, FL_INACTIVE);
		  fl_set_object_lcol(fd_form_preamble->button_apply, FL_INACTIVE);
		}
		else {
		  fl_activate_object(fd_form_preamble->input_preamble);
		  fl_activate_object(fd_form_preamble->button_ok);
		  fl_activate_object(fd_form_preamble->button_apply);
		  fl_set_object_lcol(fd_form_preamble->button_ok, FL_BLACK);
		  fl_set_object_lcol(fd_form_preamble->button_apply, FL_BLACK);
		}
	} else if (fd_form_preamble->form_preamble->visible) {
		fl_hide_form(fd_form_preamble->form_preamble);
	}
	return update;
}

void MenuLayoutPreamble()
{
	if (UpdateLayoutPreamble()) {
		if (fd_form_preamble->form_preamble->visible) {
			fl_raise_form(fd_form_preamble->form_preamble);
		} else {
			fl_show_form(fd_form_preamble->form_preamble,
				     FL_PLACE_MOUSE | FL_FREE_SIZE,
				     FL_FULLBORDER,
				     _("LaTeX Preamble"));
		}
	}
}

void MenuLayoutSave()
{
	if (!current_view->getScreen() || ! current_view->available())
	    return;

	if (AskQuestion(_("Do you want to save the current settings"),
			_("for Character, Document, Paper and Quotes"),
			_("as default for new documents?")))
		current_view->currentBuffer()->saveParamsAsDefaults();
}


void NoteCB()
{
	InsetInfo *new_inset = new InsetInfo();
	current_view->currentBuffer()->insertInset(new_inset);
	new_inset->Edit(0,0);
	//current_view->currentBuffer()->update(-1);
}


void OpenStuff()
{
	if (current_view->available()) {
		minibuffer->Set(_("Open/Close..."));
		current_view->getScreen()->HideCursor();
		BeforeChange();
		current_view->currentBuffer()->update(-2);
		current_view->currentBuffer()->text->OpenStuff();
		current_view->currentBuffer()->update(0);
	}
}

void ToggleFloat()
{
	if (current_view->available()) {
		minibuffer->Set(_("Open/Close..."));
		current_view->getScreen()->HideCursor();
		BeforeChange();
		current_view->currentBuffer()->update(-2);
		current_view->currentBuffer()->text->ToggleFootnote();
		current_view->currentBuffer()->update(0);
	}
}


void MenuUndo()
{
/*	if (current_view->currentBuffer()->the_locking_inset) {
		minibuffer->Set(_("Undo not yet supported in math mode"));
		return;
	}*/
   
	if (current_view->available()) {
		minibuffer->Set(_("Undo"));
		current_view->getScreen()->HideCursor();
		BeforeChange();
		current_view->currentBuffer()->update(-2);
		if (!current_view->currentBuffer()->text->TextUndo())
			minibuffer->Set(_("No further undo information"));
		else
			current_view->currentBuffer()->update(-1);
	}
}


void MenuRedo()
{
	if (current_view->currentBuffer()->the_locking_inset) {
		minibuffer->Set(_("Redo not yet supported in math mode"));
		return;
	}    
   
	if (current_view->available()) {
		minibuffer->Set(_("Redo"));
		current_view->getScreen()->HideCursor();
		BeforeChange();
		current_view->currentBuffer()->update(-2);
		if (!current_view->currentBuffer()->text->TextRedo())
			minibuffer->Set(_("No further redo information"));
		else
			current_view->currentBuffer()->update(-1);
	}
}


void HyphenationPoint()
{
	if (current_view->available())  {
		current_view->getScreen()->HideCursor();
		current_view->currentBuffer()->update(-2);
		InsetSpecialChar *new_inset = 
			new InsetSpecialChar(InsetSpecialChar::HYPHENATION);
		current_view->currentBuffer()->insertInset(new_inset);
		//current_view->currentBuffer()->update(-1);
	}
}


void Ldots()
{
	if (current_view->available())  {
		current_view->getScreen()->HideCursor();
		current_view->currentBuffer()->update(-2);
		InsetSpecialChar *new_inset = 
			new InsetSpecialChar(InsetSpecialChar::LDOTS);
		current_view->currentBuffer()->insertInset(new_inset);
	}
}


void EndOfSentenceDot()
{
	if (current_view->available())  {
		current_view->getScreen()->HideCursor();
		current_view->currentBuffer()->update(-2);
		InsetSpecialChar *new_inset = 
			new InsetSpecialChar(InsetSpecialChar::END_OF_SENTENCE);
		current_view->currentBuffer()->insertInset(new_inset);
	}
}


void MenuSeparator()
{
	if (current_view->available())  {
		current_view->getScreen()->HideCursor();
		current_view->currentBuffer()->update(-2);
		InsetSpecialChar *new_inset = 
			new InsetSpecialChar(InsetSpecialChar::MENU_SEPARATOR);
		current_view->currentBuffer()->insertInset(new_inset);
		//current_view->currentBuffer()->update(-1);
	}
}


void Newline()
{
	if (current_view->available())  {
		current_view->getScreen()->HideCursor();
		current_view->currentBuffer()->update(-2);
		current_view->currentBuffer()->text->InsertChar(LYX_META_NEWLINE);
		current_view->currentBuffer()->update(-1);
	}
}


void ProtectedBlank()
{
	if (current_view->available())  {
		current_view->getScreen()->HideCursor();
		current_view->currentBuffer()->update(-2);
		current_view->currentBuffer()->text->InsertChar(LYX_META_PROTECTED_SEPARATOR);
		current_view->currentBuffer()->update(-1);
	}
}


void HFill()
{
	if (current_view->available())  {
		current_view->getScreen()->HideCursor();
		current_view->currentBuffer()->update(-2);
		current_view->currentBuffer()->text->InsertChar(LYX_META_HFILL);
		current_view->currentBuffer()->update(-1);
	}
}


/* -------> These CB's use ToggleFree() as the (one and only?) font-changer. 
			They also show the current font state. */

static
void ToggleAndShow(LyXFont const &);


void FontSizeCB(string const & size)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setGUISize(size);
	ToggleAndShow(font);
}


void EmphCB()
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setEmph(LyXFont::TOGGLE);
	ToggleAndShow(font);
}


void NounCB()
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setNoun(LyXFont::TOGGLE);
	ToggleAndShow(font);
}


void BoldCB()
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setSeries(LyXFont::BOLD_SERIES);
	ToggleAndShow(font);
}


void UnderlineCB()
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setUnderbar(LyXFont::TOGGLE);
	ToggleAndShow(font);
}


void CodeCB()
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setFamily(LyXFont::TYPEWRITER_FAMILY); // no good
	ToggleAndShow(font);
}


void SansCB()
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setFamily(LyXFont::SANS_FAMILY);
	ToggleAndShow(font);
}


void RomanCB()
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setFamily(LyXFont::ROMAN_FAMILY);
	ToggleAndShow(font);
}


void TexCB()
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setLatex (LyXFont::TOGGLE);
	ToggleAndShow(font);
}


void StyleResetCB()
{
	LyXFont font(LyXFont::ALL_INHERIT);
	ToggleAndShow(font);
}


/* -------> Returns the current font and depth by printing a message. In the
 * future perhaps we could try to implement a callback to the button-bar.
 * That is, `light' the bold button when the font is currently bold, etc.
 */
string CurrentState()
{
	string state;
	if (current_view->available()) { 
		// I think we should only show changes from the default
		// font. (Asger)
		Buffer * buffer = current_view->currentBuffer();
		LyXFont font = buffer->text->real_current_font;
		LyXFont defaultfont = lyxstyle.TextClass(buffer->
							 params.textclass)->defaultfont;
		font.reduce(defaultfont);
		state = _("Font: ") + font.stateText();

		int depth = buffer->text->GetDepth();
		if (depth>0) 
			state += string(_(", Depth: ")) + tostr(depth);
	}
	return state;
}


/* -------> Does the actual toggle job of the XxxCB() calls above.
 * Also shows the current font state.
 */
static
void ToggleAndShow(LyXFont const & font)
{
	if (current_view->available()) { 
		current_view->getScreen()->HideCursor();
		current_view->currentBuffer()->update(-2);
  		current_view->currentBuffer()->text->ToggleFree(font, toggleall);
		current_view->currentBuffer()->update(1);
	}
	// removed since it overrides the ToggleFree Message about the style
	// Since Styles are more "High Level" than raw fonts I think the user
	// prefers it like this               Matthias
	// FontStateShowCB( 0, 0 );
}


void MarginCB(FL_OBJECT *, long)
{
	if (current_view->available()) {
		minibuffer->Set(_("Inserting margin note..."));
		current_view->getScreen()->HideCursor();
		current_view->currentBuffer()->update(-2);
		current_view->currentBuffer()->text->InsertFootnoteEnvironment(LyXParagraph::MARGIN);
		current_view->currentBuffer()->update(1);
	}
}


void FigureCB(FL_OBJECT *, long)
{
	if (fd_form_figure->form_figure->visible) {
		fl_raise_form(fd_form_figure->form_figure);
	} else {
		fl_show_form(fd_form_figure->form_figure,
			     FL_PLACE_MOUSE, FL_FULLBORDER,
			     _("Insert Figure"));
	}
}


void TableCB(FL_OBJECT *, long)
{
	if (fd_form_table->form_table->visible) {
		fl_raise_form(fd_form_table->form_table);
	} else {
		fl_show_form(fd_form_table->form_table,
			     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_FULLBORDER,
			     _("Insert Table"));
	}
}


void CopyEnvironmentCB()
{
	if (current_view->available()) {
		current_view->currentBuffer()->text->copyEnvironmentType();
		/* clear the selection, even if mark_set */ 
		current_view->getScreen()->ToggleSelection();
		current_view->currentBuffer()->text->ClearSelection();
		current_view->currentBuffer()->update(-2);
		minibuffer->Set(_("Paragraph environment type copied"));
	}
}


void PasteEnvironmentCB()
{
	if (current_view->available()) {
		current_view->currentBuffer()->text->pasteEnvironmentType();
		minibuffer->Set(_("Paragraph environment type set"));
		current_view->currentBuffer()->update(1);
	}
}


void CopyCB()
{
	if (current_view->available()) {
		current_view->currentBuffer()->text->CopySelection();
		/* clear the selection, even if mark_set */ 
		current_view->getScreen()->ToggleSelection();
		current_view->currentBuffer()->text->ClearSelection();
		current_view->currentBuffer()->update(-2);
		minibuffer->Set(_("Copy"));
	}
}


void CutCB()
{
	if (current_view->available()) {
		current_view->getScreen()->HideCursor();
		current_view->currentBuffer()->update(-2);
		current_view->currentBuffer()->text->CutSelection();
		current_view->currentBuffer()->update(1);
		minibuffer->Set(_("Cut"));
	}
}


void PasteCB()
{
	if (!current_view->available()) return;
	
	minibuffer->Set(_("Paste"));
	current_view->getScreen()->HideCursor();
	/* clear the selection */ 
	current_view->getScreen()->ToggleSelection();
	current_view->currentBuffer()->text->ClearSelection();
	current_view->currentBuffer()->update(-2);
	
	/* paste */ 
	current_view->currentBuffer()->text->PasteSelection();
	current_view->currentBuffer()->update(1);
	
	/* clear the selection */ 
	current_view->getScreen()->ToggleSelection();
	current_view->currentBuffer()->text->ClearSelection();
	current_view->currentBuffer()->update(-2);
}


void MeltCB(FL_OBJECT *, long)
{
	if (!current_view->available()) return;
	
	minibuffer->Set(_("Melt"));
	current_view->getScreen()->HideCursor();
	BeforeChange();
	current_view->currentBuffer()->update(-2);
	current_view->currentBuffer()->text->MeltFootnoteEnvironment();
	current_view->currentBuffer()->update(1);
}


// Change environment depth.
// if decInc == 0, depth change taking mouse button number into account
// if decInc == 1, increment depth
// if decInc == -1, decrement depth
void DepthCB(FL_OBJECT *ob, long decInc)
{
	int button = 1;

	/* When decInc != 0, fake a mouse button. This allows us to
	   implement depth-plus and depth-min commands. RVDK_PATCH_5. */
	/* check out wether ob is defined, too (Matthias) */ 
	if ( decInc < 0 )
		button = 0;
	else if (!decInc && ob) {
		button = fl_get_button_numb(ob);
	}
  
	if (current_view->available()) {
		current_view->getScreen()->HideCursor();
		current_view->currentBuffer()->update(-2);
		if (button == 1)
			current_view->currentBuffer()->text->IncDepth();
		else
			current_view->currentBuffer()->text->DecDepth();
		current_view->currentBuffer()->update(1);
		minibuffer->Set(_("Changed environment depth"
				  " (in possible range, maybe not)"));
	}
}


// This is both GUI and LyXFont dependent. Don't know where to put it. (Asger)
// Well, it's mostly GUI dependent, so I guess it will stay here. (Asger)
LyXFont UserFreeFont()
{
	LyXFont font(LyXFont::ALL_IGNORE);
	int pos;

	pos = fl_get_choice(fd_form_character->choice_family);
	switch(pos) {
	case 1: font.setFamily(LyXFont::IGNORE_FAMILY); break;
	case 2: font.setFamily(LyXFont::ROMAN_FAMILY); break;
	case 3: font.setFamily(LyXFont::SANS_FAMILY); break;
	case 4: font.setFamily(LyXFont::TYPEWRITER_FAMILY); break;
	case 5: font.setFamily(LyXFont::INHERIT_FAMILY); break;
	}

	pos = fl_get_choice(fd_form_character->choice_series);
	switch(pos) {
	case 1: font.setSeries(LyXFont::IGNORE_SERIES); break;
	case 2: font.setSeries(LyXFont::MEDIUM_SERIES); break;
	case 3: font.setSeries(LyXFont::BOLD_SERIES); break;
	case 4: font.setSeries(LyXFont::INHERIT_SERIES); break;
	}

	pos = fl_get_choice(fd_form_character->choice_shape);
	switch(pos) {
	case 1: font.setShape(LyXFont::IGNORE_SHAPE); break;
	case 2: font.setShape(LyXFont::UP_SHAPE); break;
	case 3: font.setShape(LyXFont::ITALIC_SHAPE); break;
	case 4: font.setShape(LyXFont::SLANTED_SHAPE); break;
	case 5: font.setShape(LyXFont::SMALLCAPS_SHAPE); break;
	case 6: font.setShape(LyXFont::INHERIT_SHAPE); break;
	}

	pos = fl_get_choice(fd_form_character->choice_size);
	switch(pos) {
	case 1: font.setSize(LyXFont::IGNORE_SIZE); break;
	case 2: font.setSize(LyXFont::SIZE_TINY); break;
	case 3: font.setSize(LyXFont::SIZE_SCRIPT); break;
	case 4: font.setSize(LyXFont::SIZE_FOOTNOTE); break;
	case 5: font.setSize(LyXFont::SIZE_SMALL); break;
	case 6: font.setSize(LyXFont::SIZE_NORMAL); break;
	case 7: font.setSize(LyXFont::SIZE_LARGE); break;
	case 8: font.setSize(LyXFont::SIZE_LARGER); break;
	case 9: font.setSize(LyXFont::SIZE_LARGEST); break;
	case 10: font.setSize(LyXFont::SIZE_HUGE); break;
	case 11: font.setSize(LyXFont::SIZE_HUGER); break;
	case 12: font.setSize(LyXFont::INCREASE_SIZE); break;
	case 13: font.setSize(LyXFont::DECREASE_SIZE); break;
	case 14: font.setSize(LyXFont::INHERIT_SIZE); break;
	}

	pos = fl_get_choice(fd_form_character->choice_bar);
	switch(pos) {
	case 1: font.setEmph(LyXFont::IGNORE);
		font.setUnderbar(LyXFont::IGNORE);
		font.setNoun(LyXFont::IGNORE);
		font.setLatex(LyXFont::IGNORE);
		break;
	case 2: font.setEmph(LyXFont::TOGGLE); break;
	case 3: font.setUnderbar(LyXFont::TOGGLE); break;
	case 4: font.setNoun(LyXFont::TOGGLE); break;
	case 5: font.setLatex(LyXFont::TOGGLE); break;
	case 6: font.setEmph(LyXFont::INHERIT);
		font.setUnderbar(LyXFont::INHERIT);
		font.setNoun(LyXFont::INHERIT);
		font.setLatex(LyXFont::INHERIT);
		break;
	}

	pos = fl_get_choice(fd_form_character->choice_color);
	switch(pos) {
	case 1: font.setColor(LyXFont::IGNORE_COLOR); break;
	case 2: font.setColor(LyXFont::NONE); break;
	case 3: font.setColor(LyXFont::BLACK); break;
	case 4: font.setColor(LyXFont::WHITE); break;
	case 5: font.setColor(LyXFont::RED); break;
	case 6: font.setColor(LyXFont::GREEN); break;
	case 7: font.setColor(LyXFont::BLUE); break;
	case 8: font.setColor(LyXFont::CYAN); break;
	case 9: font.setColor(LyXFont::MAGENTA); break;
	case 10: font.setColor(LyXFont::YELLOW); break;
	case 11: font.setColor(LyXFont::INHERIT_COLOR); break;
	}

	return font; 
}


void FreeCB()
{
	ToggleAndShow(UserFreeFont());
}


/* callbacks for form form_title */
void TimerCB(FL_OBJECT *, long)
{
	// only if the form still exists
	if (fd_form_title->form_title != 0) {
		if (fd_form_title->form_title->visible) {
			fl_hide_form(fd_form_title->form_title);
		}
		fl_free_form(fd_form_title->form_title);
		fd_form_title->form_title = 0;
	}
}


/* callbacks for form form_paragraph */

void ParagraphVSpaceCB(FL_OBJECT* obj, long )
{
	// "Synchronize" the choices and input fields, making it
	// impossible to commit senseless data.

	const FD_form_paragraph* fp = fd_form_paragraph;

	if (obj == fp->choice_space_above) {
		if (fl_get_choice (fp->choice_space_above) != 7) {
			fl_set_input (fp->input_space_above, "");
			ActivateParagraphButtons();
		}
	} else if (obj == fp->choice_space_below) {
		if (fl_get_choice (fp->choice_space_below) != 7) {
			fl_set_input (fp->input_space_below, "");
			ActivateParagraphButtons();
		}
	} else if (obj == fp->input_space_above) {
		string input = fl_get_input (fp->input_space_above);

		if (input.empty()) {
			fl_set_choice (fp->choice_space_above, 1);
			ActivateParagraphButtons();
		}
		else if (isValidGlueLength (input)) {
			fl_set_choice (fp->choice_space_above, 7);
			ActivateParagraphButtons();
		}
		else {
			fl_set_choice (fp->choice_space_above, 7);
			DeactivateParagraphButtons();
		}
	} else if (obj == fp->input_space_below) {
		string input = fl_get_input (fp->input_space_below);

		if (input.empty()) {
			fl_set_choice (fp->choice_space_below, 1);
			ActivateParagraphButtons();
		}
		else if (isValidGlueLength (input)) {
			fl_set_choice (fp->choice_space_below, 7);
			ActivateParagraphButtons();
		}
		else {
			fl_set_choice (fp->choice_space_below, 7);
			DeactivateParagraphButtons();
		}
	}
}


void ParagraphApplyCB(FL_OBJECT *, long)
{
	if (!current_view->available())
		return;
	
	bool line_top, line_bottom;
	bool pagebreak_top, pagebreak_bottom;
	VSpace space_top, space_bottom;
	char align;
	string labelwidthstring;
	bool noindent;

	// If a vspace kind is "Length" but there's no text in
	// the input field, reset the kind to "None". 
	if (fl_get_choice (fd_form_paragraph->choice_space_above) == 7
	    && !*(fl_get_input (fd_form_paragraph->input_space_above))) {
		fl_set_choice (fd_form_paragraph->choice_space_above, 1);
	}
	if (fl_get_choice (fd_form_paragraph->choice_space_below) == 7
	    && !*(fl_get_input (fd_form_paragraph->input_space_below))) {
		fl_set_choice (fd_form_paragraph->choice_space_below, 1);
	}
   
	line_top = fl_get_button(fd_form_paragraph->check_lines_top);
	line_bottom = fl_get_button(fd_form_paragraph->check_lines_bottom);
	pagebreak_top = fl_get_button(fd_form_paragraph->check_pagebreaks_top);
	pagebreak_bottom = fl_get_button(fd_form_paragraph->check_pagebreaks_bottom);
	switch (fl_get_choice (fd_form_paragraph->choice_space_above)) {
	case 1: space_top = VSpace(VSpace::NONE); break;
	case 2: space_top = VSpace(VSpace::DEFSKIP); break;
	case 3: space_top = VSpace(VSpace::SMALLSKIP); break;
	case 4: space_top = VSpace(VSpace::MEDSKIP); break;
	case 5: space_top = VSpace(VSpace::BIGSKIP); break;
	case 6: space_top = VSpace(VSpace::VFILL); break;
	case 7: space_top = VSpace(LyXGlueLength (fl_get_input (fd_form_paragraph->input_space_above))); break;
	}
	if (fl_get_button (fd_form_paragraph->check_space_above))
	  space_top.setKeep (true);
	switch (fl_get_choice (fd_form_paragraph->choice_space_below)) {
	case 1: space_bottom = VSpace(VSpace::NONE); break;
	case 2: space_bottom = VSpace(VSpace::DEFSKIP); break;
	case 3: space_bottom = VSpace(VSpace::SMALLSKIP); break;
	case 4: space_bottom = VSpace(VSpace::MEDSKIP); break;
	case 5: space_bottom = VSpace(VSpace::BIGSKIP); break;
	case 6: space_bottom = VSpace(VSpace::VFILL); break;
	case 7: space_bottom = VSpace(LyXGlueLength (fl_get_input (fd_form_paragraph->input_space_below))); break;
	}
	if (fl_get_button (fd_form_paragraph->check_space_below))
	  space_bottom.setKeep (true);

	if (fl_get_button(fd_form_paragraph->radio_align_left))
		align = LYX_ALIGN_LEFT;
	else if (fl_get_button(fd_form_paragraph->radio_align_right))
		align = LYX_ALIGN_RIGHT;
	else if (fl_get_button(fd_form_paragraph->radio_align_center))
		align = LYX_ALIGN_CENTER;
	else 
		align = LYX_ALIGN_BLOCK;
   
	labelwidthstring = fl_get_input(fd_form_paragraph->input_labelwidth);
	noindent = fl_get_button(fd_form_paragraph->check_noindent);
   
	current_view->currentBuffer()->text->SetParagraph(line_top,
							  line_bottom,
							  pagebreak_top,
							  pagebreak_bottom,
							  space_top,
							  space_bottom,
							  align, 
							  labelwidthstring,
							  noindent);
	current_view->currentBuffer()->update(1);
	minibuffer->Set(_("Paragraph layout set"));
}


void ParagraphCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_paragraph->form_paragraph);
}


void ParagraphOKCB(FL_OBJECT *ob, long data)
{
	ParagraphApplyCB(ob, data);
	ParagraphCancelCB(ob, data);
}


/* callbacks for form form_character */

void CharacterApplyCB(FL_OBJECT *, long)
{
	// we set toggleall locally here, since it should be true for
	// all other uses of ToggleAndShow() (JMarc)
	toggleall = fl_get_button(fd_form_character->check_toggle_all);
	ToggleAndShow( UserFreeFont());
	toggleall = true;
}


void CharacterCloseCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_character->form_character);
}


void CharacterOKCB(FL_OBJECT *ob, long data)
{
	CharacterApplyCB(ob,data);
	CharacterCloseCB(ob,data);
}


/* callbacks for form form_document */

void UpdateDocumentButtons(BufferParams const &params) 
{
	fl_set_choice(fd_form_document->choice_pagestyle, 1);
	
	if (params.sides == 2)
		fl_set_button(fd_form_document->radio_sides_two, 1);
	else
		fl_set_button(fd_form_document->radio_sides_one, 1);
	
	if (params.columns == 2)
		fl_set_button(fd_form_document->radio_columns_two, 1);
	else
		fl_set_button(fd_form_document->radio_columns_one, 1);
	
	fl_set_input(fd_form_document->input_extra, params.options.c_str());
	fl_set_counter_value(fd_form_document->slider_secnumdepth, 
			     params.secnumdepth);
	fl_set_counter_value(fd_form_document->slider_tocdepth, 
			     params.tocdepth);
	
}

void ChoiceClassCB(FL_OBJECT *ob, long)
{
	ProhibitInput();
	if (lyxstyle.Load(fl_get_choice(ob)-1)) {
		if (AskQuestion(_("Should I set some parameters to"),
				fl_get_choice_text(ob),
				_("the defaults of this document class?"))) {
			BufferParams params = BufferParams();
			params.textclass = fl_get_choice(ob)-1;
			params.useClassDefaults();
			UpdateLayoutDocument(&params);
			UpdateDocumentButtons(params);
		}
	} else {
		// unable to load new style
		WriteAlert(_("Conversion Errors!"),
			   _("Unable to switch to new document class."),
			   _("Reverting to original document class."));
		fl_set_choice(fd_form_document->choice_class, 
			      GetCurrentTextClass() + 1);
	}
	AllowInput();
}


void DocumentDefskipCB(FL_OBJECT *obj, long)
{
	// "Synchronize" the choice and the input field, so that it
	// is impossible to commit senseless data.
	const FD_form_document* fd = fd_form_document;

	if (obj == fd->choice_default_skip) {
		if (fl_get_choice (fd->choice_default_skip) != 4) {
			fl_set_input (fd->input_default_skip, "");
			ActivateDocumentButtons();
		}
	} else if (obj == fd->input_default_skip) {

		const char* input = fl_get_input (fd->input_default_skip);

		if (!*input) {
			fl_set_choice (fd->choice_default_skip, 2);
			ActivateDocumentButtons();
		} else if (isValidGlueLength (input)) {
			fl_set_choice (fd->choice_default_skip, 4);
			ActivateDocumentButtons();
		} else {
			fl_set_choice (fd->choice_default_skip, 4);
			DeactivateDocumentButtons();
		}
	}
}


void DocumentSpacingCB(FL_OBJECT *obj, long)
{
	// "Synchronize" the choice and the input field, so that it
	// is impossible to commit senseless data.
	const FD_form_document* fd = fd_form_document;

	if (obj == fd->choice_spacing
	    && fl_get_choice (fd->choice_spacing) != 4) {
		fl_set_input(fd->input_spacing, "");
	} else if (obj == fd->input_spacing) {

		const char* input = fl_get_input (fd->input_spacing);

		if (!*input) {
			fl_set_choice (fd->choice_spacing, 1);
		} else {
			fl_set_choice (fd->choice_spacing, 4);
		}
	}
}


void DocumentApplyCB(FL_OBJECT *, long)
{
	bool redo = false;
	BufferParams *params = &(current_view->currentBuffer()->params);
	current_view->currentBuffer()->params.language =
		combo_language->getline();

	// If default skip is a "Length" but there's no text in the
	// input field, reset the kind to "Medskip", which is the default.
	if (fl_get_choice (fd_form_document->choice_default_skip) == 4
	    && !*(fl_get_input (fd_form_document->input_default_skip))) {
		fl_set_choice (fd_form_document->choice_default_skip, 2);
	}

	/* this shouldn't be done automatically IMO. For example I write german
	 * documents with an american keyboard very often. Matthias */
   
	/* ChangeKeymap(buffer->parameters.language, TRUE, false,
	   fl_get_choice(fd_form_document->choice_language)); */
	params->fonts =
		fl_get_choice_text(fd_form_document->choice_fonts);
	params->inputenc =
		fl_get_choice_text(fd_form_document->choice_inputenc);
	params->fontsize =
		fl_get_choice_text(fd_form_document->choice_fontsize);
	params->pagestyle =
		fl_get_choice_text(fd_form_document->choice_pagestyle);
	params->graphicsDriver =
		fl_get_choice_text(fd_form_document->choice_postscript_driver);
	params->use_amsmath =
		fl_get_button(fd_form_document->check_use_amsmath);
   
	if (!current_view->available())
		return;

	char new_class = fl_get_choice(fd_form_document->choice_class) - 1;
	if (params->textclass != new_class) {
		// try to load new_class
		if (lyxstyle.Load(new_class)) {
			// successfully loaded
			redo = true;
			minibuffer->Set(_("Converting document to new document class..."));
			int ret = current_view->currentBuffer()->
				text->
				SwitchLayoutsBetweenClasses(current_view->currentBuffer()->
							    params.textclass,
							    new_class,
							    current_view->currentBuffer()->
							    paragraph);

			if (ret){
				string s;
				if (ret==1)
					s= _("One paragraph couldn't be converted");
				else {
					s += ret;
					s += _(" paragraphs couldn't be converted");
				}
				WriteAlert(_("Conversion Errors!"),s,
					   _("into chosen document class"));
			}

			params->textclass = new_class;
		} else {
			// problem changing class -- warn user and retain old style
			WriteAlert(_("Conversion Errors!"),
				   _("Unable to switch to new document class."),
				   _("Reverting to original document class."));
			fl_set_choice(fd_form_document->choice_class, params->textclass + 1);
		}
	}

	char tmpsep = params->paragraph_separation;
	if (fl_get_button(fd_form_document->radio_indent))
		params->paragraph_separation = LYX_PARSEP_INDENT;
	else
		params->paragraph_separation = LYX_PARSEP_SKIP;
	if (tmpsep != params->paragraph_separation)
		redo = true;
   
	VSpace tmpdefskip = params->getDefSkip();
	switch (fl_get_choice (fd_form_document->choice_default_skip)) {
	case 1: params->setDefSkip(VSpace(VSpace::SMALLSKIP)); break;
	case 2: params->setDefSkip(VSpace(VSpace::MEDSKIP)); break;
	case 3: params->setDefSkip(VSpace(VSpace::BIGSKIP)); break;
	case 4: params->setDefSkip( 
		VSpace (LyXGlueLength (fl_get_input 
				       (fd_form_document->input_default_skip))));
	break;
	// DocumentDefskipCB assures that this never happens
	default: params->setDefSkip(VSpace(VSpace::MEDSKIP)); break;
	}
	if (!(tmpdefskip == params->getDefSkip()))
		redo = true;

	if (fl_get_button(fd_form_document->radio_columns_two))
		params->columns = 2;
	else
		params->columns = 1;
	if (fl_get_button(fd_form_document->radio_sides_two))
		params->sides = 2;
	else
		params->sides = 1;

	Spacing tmpSpacing = params->spacing;
	switch(fl_get_choice(fd_form_document->choice_spacing)) {
	case 1:
		lyxerr.debug() << "Spacing: SINGLE" << endl;
		params->spacing.set(Spacing::Single);
		break;
	case 2:
		lyxerr.debug() << "Spacing: ONEHALF" << endl;
		params->spacing.set(Spacing::Onehalf);
		break;
	case 3:
		lyxerr.debug() << "Spacing: DOUBLE" << endl;
		params->spacing.set(Spacing::Double);
		break;
	case 4:
		lyxerr.debug() << "Spacing: OTHER" << endl;
		params->spacing.set(Spacing::Other, 
				    fl_get_input(fd_form_document->input_spacing));
		break;
	}
	if (tmpSpacing != params->spacing)
		redo = true;
	
	signed char tmpchar = (signed char) 
		fl_get_counter_value(fd_form_document->slider_secnumdepth);;
	if (params->secnumdepth != tmpchar)
		redo = true;
	params->secnumdepth = tmpchar;
   
	params->tocdepth = (signed char) 
		fl_get_counter_value(fd_form_document->slider_tocdepth);;

	params->float_placement =
		fl_get_input(fd_form_document->input_float_placement);

	// More checking should be done to ensure the string doesn't have
	// spaces or illegal placement characters in it. (thornley)

	if (redo)
		current_view->redoCurrentBuffer();
   
	minibuffer->Set(_("Document layout set"));
	current_view->currentBuffer()->markDirty();

        params->options =
		fl_get_input(fd_form_document->input_extra);
   
}


void DocumentCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_document->form_document);
}


void DocumentOKCB(FL_OBJECT *ob, long data)
{
	DocumentCancelCB(ob,data);
	DocumentApplyCB(ob,data);
}


void DocumentBulletsCB(FL_OBJECT *, long)
{
	bulletForm();
	// bullet callbacks etc. in bullet_panel.C -- ARRae
}


void GotoNote()
{
	if (!current_view->getScreen())
		return;
   
	current_view->getScreen()->HideCursor();
	BeforeChange();
	current_view->currentBuffer()->update(-2);
	LyXCursor tmp;
   
	if (!current_view->currentBuffer()->text->GotoNextNote()) {
		if (current_view->currentBuffer()->text->cursor.pos 
		    || current_view->currentBuffer()->text->cursor.par !=
		    current_view->currentBuffer()->text->FirstParagraph())
			{
				tmp = current_view->currentBuffer()->text->cursor;
				current_view->currentBuffer()->text->cursor.par =
					current_view->currentBuffer()->text->FirstParagraph();
				current_view->currentBuffer()->text->cursor.pos = 0;
				if (!current_view->currentBuffer()->text->GotoNextNote()) {
					current_view->currentBuffer()->text->cursor = tmp;
					minibuffer->Set(_("No more notes"));
					LyXBell();
				}
			} else {
				minibuffer->Set(_("No more notes"));
				LyXBell();
			}
	}
	current_view->currentBuffer()->update(0);
	current_view->currentBuffer()->text->sel_cursor =
		current_view->currentBuffer()->text->cursor;
}


void InsertCorrectQuote()
{
	Buffer *cbuffer = current_view->currentBuffer();
	char c;

	if  (cbuffer->text->cursor.pos )
		c = cbuffer->text->cursor.par->GetChar(cbuffer->text->cursor.pos - 1);
	else 
		c = ' ';

	cbuffer->insertInset(new InsetQuotes(c, cbuffer->params));
}


/* callbacks for form form_quotes */

void QuotesApplyCB(FL_OBJECT *, long)
{
	if (!current_view->available())
		return;
	
	minibuffer->Set(_("Quotes type set"));
	//current_view->currentBuffer()->params.quotes_language =
	//	fl_get_choice(fd_form_quotes->choice_quotes_language) - 1;
	InsetQuotes::quote_language lga = InsetQuotes::EnglishQ;
	switch(fl_get_choice(fd_form_quotes->choice_quotes_language) - 1) {
	case 0:
		lga = InsetQuotes::EnglishQ;
		break;
	case 1:
		lga = InsetQuotes::SwedishQ;
		break;
	case 2:
		lga = InsetQuotes::GermanQ;
		break;
	case 3:
		lga = InsetQuotes::PolishQ;
		break;
	case 4:
		lga = InsetQuotes::FrenchQ;
		break;
	case 5:
		lga = InsetQuotes::DanishQ;
		break;
	}
	current_view->currentBuffer()->params.quotes_language = lga;
	if (fl_get_button(fd_form_quotes->radio_single))   
		current_view->currentBuffer()->
			params.quotes_times = InsetQuotes::SingleQ;
	else
		current_view->currentBuffer()->
			params.quotes_times = InsetQuotes::DoubleQ;
}


void QuotesCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_quotes->form_quotes);
}


void QuotesOKCB(FL_OBJECT *ob, long data)
{
	QuotesApplyCB(ob, data);
	QuotesCancelCB(ob, data);
}



/* callbacks for form form_preamble */

void PreambleCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_preamble->form_preamble);
}


void PreambleApplyCB(FL_OBJECT *, long)
{
	if (!current_view->available())
		return;
	
	current_view->currentBuffer()->params.preamble =
		fl_get_input(fd_form_preamble->input_preamble);
	current_view->currentBuffer()->markDirty();
	minibuffer->Set(_("LaTeX preamble set"));
}

   
void PreambleOKCB(FL_OBJECT *ob, long data)
{
	PreambleApplyCB(ob, data);
	PreambleCancelCB(ob, data);
}


/* callbacks for form form_table */

void TableApplyCB(FL_OBJECT *, long)
{
	int xsize,ysize;
	if (!current_view->getScreen())
		return;
   
	// check for tables in tables
	if (current_view->currentBuffer()->text->cursor.par->table){
		WriteAlert(_("Impossible Operation!"),
			   _("Cannot insert table in table."),
			   _("Sorry."));
		return;
	}
 
	minibuffer->Set(_("Inserting table..."));

	ysize = (int)(fl_get_slider_value(fd_form_table->slider_columns) + 0.5);
	xsize = (int)(fl_get_slider_value(fd_form_table->slider_rows) + 0.5);
   
   
	current_view->getScreen()->HideCursor();
	BeforeChange();
	current_view->currentBuffer()->update(-2);
   
	current_view->currentBuffer()->text->SetCursorParUndo(); 
	current_view->currentBuffer()->text->FreezeUndo();

	current_view->currentBuffer()->text->BreakParagraph();
	current_view->currentBuffer()->update(-1);
   
	if (current_view->currentBuffer()->text->cursor.par->Last()) {
		current_view->currentBuffer()->text->CursorLeft();
      
		current_view->currentBuffer()->text->BreakParagraph();
		current_view->currentBuffer()->update(-1);
	}

	current_view->currentBuffer()->text->current_font.setLatex(LyXFont::OFF);
	//if (!fl_get_button(fd_form_table->check_latex)){
	// insert the new wysiwy table
	current_view->currentBuffer()->text->SetLayout(0); // standard layout
	if (current_view->currentBuffer()->text->cursor.par->footnoteflag ==
	    LyXParagraph::NO_FOOTNOTE) {
		current_view->currentBuffer()->
			text->SetParagraph(0, 0,
					   0, 0,
					   VSpace (0.3 * current_view->currentBuffer()->
						   params.spacing.getValue(),
						   LyXLength::CM),
					   VSpace (0.3 * current_view->currentBuffer()->
						   params.spacing.getValue(),
						   LyXLength::CM),
					   LYX_ALIGN_CENTER,
					   string(),
					   0);
	}
	else
		current_view->currentBuffer()->text->SetParagraph(0, 0,
								  0, 0,
								  VSpace(VSpace::NONE),
								  VSpace(VSpace::NONE),
					   LYX_ALIGN_CENTER, 
					   string(),
					   0);

	current_view->currentBuffer()->text->cursor.par->table = new LyXTable(xsize, ysize);
	int i;
	for (i=0; i<xsize * ysize - 1; i++)
		current_view->currentBuffer()->text->cursor.par->InsertChar(0,LYX_META_NEWLINE);
	current_view->currentBuffer()->text->RedoParagraph();
   
	current_view->currentBuffer()->text->UnFreezeUndo();
     
	current_view->currentBuffer()->update(1);
	minibuffer->Set(_("Table inserted"));
}


void TableCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_table->form_table);
}


void TableOKCB(FL_OBJECT *ob, long data)
{
	TableApplyCB(ob,data);
	TableCancelCB(ob,data);
}


/* callbacks for form form_print */

void PrintCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_print->form_print);
}

static bool stringOnlyContains (string const & LStr, const char * cset)
{
	const char * cstr = LStr.c_str() ;

	return strspn(cstr,cset) == strlen(cstr) ;
}

void PrintApplyCB(FL_OBJECT *, long)
{
	if (!current_view->available())
		return;
        Buffer *buffer = current_view->currentBuffer();
        string path = OnlyPath(buffer->getFileName());

	string pageflag;
	if (fl_get_button(fd_form_print->radio_even_pages))
		pageflag = lyxrc->print_evenpage_flag + ' ';
	else if (fl_get_button(fd_form_print->radio_odd_pages))
		pageflag = lyxrc->print_oddpage_flag + ' ';

// Changes by Stephan Witt (stephan.witt@beusen.de), 19-Jan-99
// User may give a page (range) list
// User may print multiple (unsorted) copies
	string pages = fl_get_input(fd_form_print->input_pages);
	subst(pages, ';',',');
	subst(pages, '+',',');
	pages = strip (pages) ;
	pages = frontStrip (pages) ;
	while (!pages.empty()) { // a page range was given
		string piece ;
		pages = split (pages, piece, ',') ;
		piece = strip(piece) ;
		piece = frontStrip(piece) ;
		if ( !stringOnlyContains (piece, "0123456789-") ) {
			WriteAlert(_("ERROR!  Unable to print!"),
				_("Check 'range of pages'!"));
			return;
		}
		if (piece.find('-') == string::npos) { // not found
			pageflag += lyxrc->print_pagerange_flag + piece + '-' + piece + ' ' ;
		} else if (suffixIs(piece, "-") ) { // missing last page
			pageflag += lyxrc->print_pagerange_flag + piece + "1000 ";
		} else if (prefixIs(piece, "-") ) { // missing first page
			pageflag += lyxrc->print_pagerange_flag + '1' + piece + ' ' ;
		} else {
			pageflag += lyxrc->print_pagerange_flag + piece + ' ' ;
		}
	}
   
	string copies = frontStrip(strip(fl_get_input(fd_form_print->input_copies)));
	if (!copies.empty()) { // a number of copies was given
		if ( !stringOnlyContains (copies, "0123456789") ) {
			WriteAlert(_("ERROR!  Unable to print!"),
				_("Check 'number of copies'!"));
			return;
		}
		if (fl_get_button(fd_form_print->do_unsorted))
			pageflag += lyxrc->print_copies_flag;
		else
			pageflag += lyxrc->print_collcopies_flag;
		pageflag += " " + copies + ' ' ;
	}

	string reverseflag;
	if (fl_get_button(fd_form_print->radio_order_reverse))
		reverseflag = lyxrc->print_reverse_flag + ' ';
   
	string orientationflag;
	if (buffer->params.orientation == ORIENTATION_LANDSCAPE)
		orientationflag = lyxrc->print_landscape_flag + ' ';
   
	string ps_file = SpaceLess(fl_get_input(fd_form_print->input_file));
	string printer = strip(fl_get_input(fd_form_print->input_printer));

	string printerflag;
	if (lyxrc->print_adapt_output // printer name should be passed to dvips
	    && ! printer.empty()) // a printer name has been given
		printerflag = lyxrc->print_to_printer + printer + ' ';
     
	string extraflags;
	if (!lyxrc->print_extra_options.empty())
		extraflags = lyxrc->print_extra_options + ' ';

	string command = lyxrc->print_command + ' ' 
		+ printerflag + pageflag + reverseflag 
		+ orientationflag + extraflags;
 
	char real_papersize = buffer->params.papersize;
	if (real_papersize == PAPER_DEFAULT)
		real_papersize = lyxrc->default_papersize;
        string
            paper;

	switch (real_papersize) {
	case PAPER_USLETTER:
		paper = "letter";
		break;
	case PAPER_A3PAPER:
		paper = "a3";
		break;
	case PAPER_A4PAPER:
		paper = "a4";
		break;
	case PAPER_A5PAPER:
		paper = "a5";
		break;
	case PAPER_B5PAPER:
		paper = "b5";
		break;
	case PAPER_EXECUTIVEPAPER:
		paper = "foolscap";
		break;
	case PAPER_LEGALPAPER:
		paper = "legal";
		break;
	default: /* If nothing else fits, keep an empty value... */
		break;
	}

	if (buffer->params.use_geometry
	    && buffer->params.papersize2 == VM_PAPER_CUSTOM
	    && !lyxrc->print_paper_dimension_flag.empty()
	    && !buffer->params.paperwidth.empty()
	    && !buffer->params.paperheight.empty()) {
		// using a custom papersize
		command += ' ';
		command += lyxrc->print_paper_dimension_flag + ' ';
		command += buffer->params.paperwidth + ',';
		command += buffer->params.paperheight + ' ';
	} else if (!lyxrc->print_paper_flag.empty()
		   && !paper.empty()
		   && (real_papersize != PAPER_USLETTER ||
		       buffer->params.orientation == ORIENTATION_PORTRAIT)) {
		command += " " + lyxrc->print_paper_flag + " " + paper + " ";
	}
	if (fl_get_button(fd_form_print->radio_file))
		command += lyxrc->print_to_file + '\"'
			+ MakeAbsPath(ps_file, path)
			+ '\"';
	else if (!lyxrc->print_spool_command.empty())
		command += lyxrc->print_to_file 
			+ '\"' + ps_file + '\"';
	
	// push directorypath, if necessary 
        if (lyxrc->use_tempdir || (IsDirWriteable(path) < 1)){
		path = buffer->tmppath;
        }
        PathPush(path);

	bool result;
	if (!lyxrc->print_spool_command.empty() && 
	    !fl_get_button(fd_form_print->radio_file)) {
	    	string command2 = lyxrc->print_spool_command + ' ';
		if (!printer.empty())
			command2 += lyxrc->print_spool_printerprefix 
				    + printer;
		// First run dvips and, if succesful, then spool command
		if ((result = RunScript(buffer, true, command))) {
			result = RunScript(buffer, false, command2, ps_file);
		}
        } else
		result = RunScript(buffer, false, command);
        PathPop();

	if (!result)
		WriteAlert(_("Error:"),
			   _("Unable to print"),
			   _("Check that your parameters are correct"));
}


void PrintOKCB(FL_OBJECT *ob, long data)
{
	PrintCancelCB(ob, data);  
	PrintApplyCB(ob,data);
}


/* callbacks for form form_figure */

void FigureApplyCB(FL_OBJECT *, long)
{
	if (!current_view->available())
		return;

	Buffer * buffer = current_view->currentBuffer();
	if(buffer->isReadonly()) // paranoia
		return;
	
	minibuffer->Set(_("Inserting figure..."));
	if (fl_get_button(fd_form_figure->radio_inline)
	    || buffer->text->cursor.par->table) {
		InsetFig * new_inset = new InsetFig(100, 20, buffer);
		buffer->insertInset(new_inset);
		minibuffer->Set(_("Figure inserted"));
		new_inset->Edit(0, 0);
		return;
	}
	
	current_view->getScreen()->HideCursor();
	buffer->update(-2);
	BeforeChange();
      
	buffer->text->SetCursorParUndo(); 
	buffer->text->FreezeUndo();

	buffer->text->BreakParagraph();
	buffer->update(-1);
      
	if (buffer->text->cursor.par->Last()) {
		buffer->text->CursorLeft();
	 
		buffer->text->BreakParagraph();
		buffer->update(-1);
	}

	// The standard layout should always be numer 0;
	buffer->text->SetLayout(0);
	
	if (buffer->text->cursor.par->footnoteflag ==
	    LyXParagraph::NO_FOOTNOTE) {
		buffer->text->SetParagraph(0, 0,
					   0, 0,
					   VSpace (0.3 * buffer->params.spacing.getValue(),
						   LyXLength::CM),
					   VSpace (0.3 *
						   buffer->params.spacing.getValue(),
						   LyXLength::CM),
					   LYX_ALIGN_CENTER, string(), 0);
	} else
		buffer->text->SetParagraph(0, 0,
					   0, 0,
					   VSpace(VSpace::NONE),
					   VSpace(VSpace::NONE),
					   LYX_ALIGN_CENTER, 
					   string(),
					   0);
	
	buffer->update(-1);
      
	Inset *new_inset = 0;
	
	new_inset = new InsetFig(100, 100, buffer);
	buffer->insertInset(new_inset);
	new_inset->Edit(0, 0);
	buffer->update(0);
	minibuffer->Set(_("Figure inserted"));
	buffer->text->UnFreezeUndo();
}

   
void FigureCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_figure->form_figure);
}


void FigureOKCB(FL_OBJECT *ob, long data)
{
	FigureApplyCB(ob,data);
	FigureCancelCB(ob,data);
}

void ScreenApplyCB(FL_OBJECT *, long)
{
	lyxrc->roman_font_name = fl_get_input(fd_form_screen->input_roman);
	lyxrc->sans_font_name = fl_get_input(fd_form_screen->input_sans);
	lyxrc->typewriter_font_name = fl_get_input(fd_form_screen->input_typewriter);
	lyxrc->font_norm = fl_get_input(fd_form_screen->input_font_norm);
	lyxrc->zoom = atoi(fl_get_input(fd_form_screen->intinput_size));
	fontloader.update();
   
	// All buffers will need resize
	bufferlist.resize();

	minibuffer->Set(_("Screen options set"));
}


void ScreenCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_screen->form_screen);
}


void ScreenOKCB(FL_OBJECT *ob, long data)
{
	ScreenCancelCB(ob,data);
	ScreenApplyCB(ob,data);
}


void LaTeXOptions()
{
	if (!current_view->available())
		return;

	fl_set_button(fd_latex_options->accents,
		      (int)current_view->currentBuffer()->params.allowAccents);
	
	if (fd_latex_options->LaTeXOptions->visible) {
		fl_raise_form(fd_latex_options->LaTeXOptions);
	} else {
		fl_show_form(fd_latex_options->LaTeXOptions,
			     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_FULLBORDER,
			     _("LaTeX Options"));
	}
}


// This function runs "configure" and then rereads lyx.defaults to
// reconfigure the automatic settings.
void Reconfigure()
{
	minibuffer->Set(_("Running configure..."));

	// Run configure in user lyx directory
	PathPush(user_lyxdir);
	Systemcalls one(Systemcalls::System, 
                          AddName(system_lyxdir,"configure"));
	PathPop();
	minibuffer->Set(_("Reloading configuration..."));
	lyxrc->Read(LibFileSearch(string(), "lyxrc.defaults"));
	WriteAlert(_("The system has been reconfigured."), 
		   _("You need to restart LyX to make use of any"),
		   _("updated document class specifications."));
}


/* these functions are for the spellchecker */ 
char* NextWord(float &value)
{
	if (!current_view->available()){
		value = 1;
		return 0;
	}
   
	char* string =  current_view->currentBuffer()->text->SelectNextWord(value);

	return string;
}

  
void SelectLastWord()
{
	if (!current_view->available())
		return;
   
	current_view->getScreen()->HideCursor();
	BeforeChange(); 
	current_view->currentBuffer()->text->SelectSelectedWord();
	current_view->getScreen()->ToggleSelection(false);
	current_view->currentBuffer()->update(0);
}


void EndOfSpellCheck()
{
	if (!current_view->available())
		return;
   
	current_view->getScreen()->HideCursor();
	BeforeChange(); 
	current_view->currentBuffer()->text->SelectSelectedWord();
	current_view->currentBuffer()->text->ClearSelection();
	current_view->currentBuffer()->update(0);
}


void ReplaceWord(string const & replacestring)
{
	if (!current_view->getScreen())
		return;

	current_view->getScreen()->HideCursor();
	current_view->currentBuffer()->update(-2);
   
	/* clear the selection (if there is any) */ 
	current_view->getScreen()->ToggleSelection(false);
	current_view->currentBuffer()->text->
		ReplaceSelectionWithString(replacestring.c_str());
   
	current_view->currentBuffer()->text->SetSelectionOverString(replacestring.c_str());

	// Go back so that replacement string is also spellchecked
	for (string::size_type i = 0; i < replacestring.length() + 1; ++i) {
		current_view->currentBuffer()->text->CursorLeftIntern();
	}
	current_view->currentBuffer()->update(1);
}
// End of spellchecker stuff



//
// Table of Contents
//

struct TocList {
	int counter[6];
	bool appendix;
	TocList *next;
};


static TocList* toclist = 0;


void TocSelectCB(FL_OBJECT *ob, long)
{
	if (!current_view->available())
		return;
   
	TocList* tmptoclist = toclist;
	int i = fl_get_browser(ob);
	int a = 0;

	for (a=1; a<i && tmptoclist->next; a++){
		tmptoclist = tmptoclist->next;
	}

	if (!tmptoclist)
		return;
     

	LyXParagraph *par = current_view->currentBuffer()->paragraph;
	while (par && (par->GetFirstCounter(0) != tmptoclist->counter[0] ||
		       par->GetFirstCounter(1) != tmptoclist->counter[1] ||
		       par->GetFirstCounter(2) != tmptoclist->counter[2] ||
		       par->GetFirstCounter(3) != tmptoclist->counter[3] ||
		       par->GetFirstCounter(4) != tmptoclist->counter[4] ||
		       par->GetFirstCounter(5) != tmptoclist->counter[5] ||
		       par->appendix != tmptoclist->appendix)) {
		par = par->LastPhysicalPar()->Next();
	}
   
	if (par) {
		BeforeChange();
		current_view->currentBuffer()->text->SetCursor(par, 0);
		current_view->currentBuffer()->text->sel_cursor =
			current_view->currentBuffer()->text->cursor;
		current_view->currentBuffer()->update(0);
	}
	else {
		WriteAlert(_("Error"), 
			   _("Couldn't find this label"), 
			   _("in current document."));
	}
	  
}


void TocCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_toc->form_toc);
}


void TocUpdateCB(FL_OBJECT *, long)
{
	static LyXParagraph* stapar = 0;
	TocList *tmptoclist = 0;
   
	/* deleted the toclist */ 
	if (toclist){
		while (toclist){
			tmptoclist = toclist->next;
			delete toclist;
			toclist = tmptoclist;
		}
	}
	toclist = 0;
	tmptoclist = toclist;


	fl_clear_browser(fd_form_toc->browser_toc);
	if (!current_view->available()) {
		fl_add_browser_line(fd_form_toc->browser_toc, _("*** No Document ***"));
		return;
	}
	fl_hide_object(fd_form_toc->browser_toc);
	/* get the table of contents */ 
	LyXParagraph *par = current_view->currentBuffer()->paragraph;
	char labeltype;
	char* line = new char[200];
	//int i = 0;
	int pos = 0;
	unsigned char c;
	int topline = 0;
   
	if (stapar == par)
		topline = fl_get_browser_topline(fd_form_toc->browser_toc);
	stapar = par;
   
	while (par) {
		labeltype = lyxstyle.Style(current_view->currentBuffer()->params.textclass, 
					   par->GetLayout())->labeltype;
      
		if (labeltype >= LABEL_COUNTER_CHAPTER
		    && labeltype <= LABEL_COUNTER_CHAPTER +
		    current_view->currentBuffer()->params.tocdepth) {
			/* insert this into the table of contents */ 
			/* first indent a little bit */ 
			
			for (pos=0; 
			     pos < (labeltype - 
				    lyxstyle.TextClass(current_view->currentBuffer()->
						       params.textclass)->maxcounter) * 4 + 2;
			     pos++)
				line[pos] = ' ';
			
			// Then the labestring
			//i = 0;
			if (!par->labelstring.empty()) {
				string::size_type i = 0;
				while (pos < 199 && i < par->labelstring.length()) {
					line[pos] = par->labelstring[i];
					i++;
					pos++;
				}
			}
	 
			line[pos] = ' ';
			pos++;
			
			/* now the contents */ 
			int i = 0;
			while (pos < 199 && i < par->last) {
				c = par->GetChar(i);
				if (isprint((unsigned char) c) || c >= 128) {
					line[pos] = c;
					pos++;
				}
				i++;
			}
			line[pos] = '\0';
			fl_add_browser_line(fd_form_toc->browser_toc, line);
			
			/* make a toclist entry */
			if (!tmptoclist){
				tmptoclist = new TocList;
				toclist = tmptoclist;
			} else {
				tmptoclist->next = new TocList;
				tmptoclist = tmptoclist->next;
			}
			
			tmptoclist->next = 0;
			int a = 0;
			for (a=0; a<6; a++){
				tmptoclist->counter[a] = par->GetFirstCounter(a);
			}
			tmptoclist->appendix = par->appendix;
		}
		par = par->LastPhysicalPar()->Next();
		
	}
	delete[] line;
	fl_set_browser_topline(fd_form_toc->browser_toc, topline);
	fl_show_object(fd_form_toc->browser_toc);
}


/* callbacks for form form_ref */
void RefSelectCB(FL_OBJECT *, long data)
{
	if (!current_view->available())
		return;

	string s =
		fl_get_browser_line(fd_form_ref->browser_ref,
				    fl_get_browser(fd_form_ref->browser_ref));
	string u = frontStrip(strip(fl_get_input(fd_form_ref->ref_name)));

	if (s.empty())
		return;

        if (data==2) {
                current_view->getOwner()->getLyXFunc()->Dispatch(LFUN_REFGOTO, s.c_str());
	        return;
	}
	    
	string t;
	if (data==0)
		t += "\\ref";
	else
		t += "\\pageref";

	if(current_view->currentBuffer()->isSGML())
		t += "[" + u + "]" + "{" + s + "}";
	else
		t += "{" + s + "}";

	Inset *new_inset =
		new InsetRef(t, current_view->currentBuffer());
	current_view->currentBuffer()->insertInset(new_inset);
}


void RefUpdateCB(FL_OBJECT *, long)
{
	if (!current_view->available()) {
		fl_clear_browser(fd_form_ref->browser_ref);
		return;
	}

	FL_OBJECT * brow = fd_form_ref->browser_ref;

	// Get the current line, in order to restore it later
	char const * const btmp=fl_get_browser_line(brow,
						     fl_get_browser(brow));
	string currentstr=btmp ? btmp : "";
	//string currentstr = fl_get_browser_line(brow,
	//					fl_get_browser(brow));

	fl_clear_browser(brow);

	string refs = current_view->currentBuffer()->getReferenceList('\n');
	int topline = 1;

#if FL_REVISION > 85
	fl_addto_browser_chars(brow,refs.c_str());
	int total_lines = fl_get_browser_maxline(brow);
	for (int i = 1; i <= total_lines ; i++) {
		if (fl_get_browser_line(brow, i) == currentstr) {
			topline = i;
			break;
		}
	}
	fl_set_browser_topline(brow, topline);
#else
	// Keep the old ugly code for xforms 0.81 compatibility
	string curr_ref;
	int ref_num = 0;
                                       
	while(true) {
		curr_ref = refs.token('\n',ref_num);
		if (curr_ref.empty())
			break;
		fl_add_browser_line(brow,curr_ref.c_str());
		ref_num++;
	}
#endif

	if (!fl_get_browser_maxline(brow)) {
		fl_add_browser_line(brow, 
				    _("*** No labels found in document ***"));
		fl_deactivate_object(brow);
	} else {
		fl_select_browser_line(brow, topline);
		fl_activate_object(brow);
	}
	if (current_view->currentBuffer()->isReadonly()) {
		// would be better to de/activate insert buttons
		// but that's more work... besides this works. ARRae
		fl_hide_form(fd_form_ref->form_ref);
	}
	if (!current_view->currentBuffer()->isSGML()) {
		fl_deactivate_object(fd_form_ref->ref_name);
		fl_set_object_lcol(fd_form_ref->ref_name, FL_INACTIVE);
	}
	else {
		fl_activate_object(fd_form_ref->ref_name);
		fl_set_object_lcol(fd_form_ref->ref_name, FL_BLACK);
	}
}


void RefHideCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_ref->form_ref);
}


void UpdateInset(Inset* inset, bool mark_dirty)
{
	if (!inset)
		return;

	/* very first check for locking insets*/
	if (current_view->currentBuffer()->the_locking_inset == inset){
		if (current_view->currentBuffer()->text->UpdateInset(inset)){
			current_view->update();
			if (mark_dirty){
				if (current_view->currentBuffer()->isLyxClean())
					minibuffer->setTimer(4);
				current_view->currentBuffer()->markDirty();
			}
			current_view->updateScrollbar();
			return;
		}
	}
  
	/* first check the current buffer */
	if (current_view->available()){
		current_view->getScreen()->HideCursor();
		current_view->currentBuffer()->update(-3);
		if (current_view->currentBuffer()->text->UpdateInset(inset)){
			if (mark_dirty)
				current_view->currentBuffer()->update(1);
			else 
				current_view->currentBuffer()->update(3);
			return;
		}
	}
  
	// check all buffers
	bufferlist.updateInset(inset, mark_dirty);

}


/* these functions return 1 if an error occured, 
   otherwise 0 */
int LockInset(UpdatableInset* inset)
{
	if (!current_view->currentBuffer()->the_locking_inset && inset){
		current_view->currentBuffer()->the_locking_inset = inset;
		return 0;
	}
	return 1;
}


void ShowLockedInsetCursor(long x, long y, int asc, int desc)
{
	if (current_view->currentBuffer()->the_locking_inset &&
	    current_view->getScreen()){
		y += current_view->currentBuffer()->text->cursor.y;
		current_view->getScreen()->ShowManualCursor(x, y,
							    asc, desc);
	}
}


void HideLockedInsetCursor(long x, long y, int asc, int desc)
{
	if (current_view->currentBuffer()->the_locking_inset &&
	    current_view->getScreen()){
		y += current_view->currentBuffer()->text->cursor.y;
		current_view->getScreen()->HideManualCursor(x, y,
							    asc, desc);
	}
}


void FitLockedInsetCursor(long x, long y, int asc, int desc)
{
	if (current_view->currentBuffer()->the_locking_inset &&
	    current_view->getScreen()){
		y += current_view->currentBuffer()->text->cursor.y;
		if (current_view->getScreen()->FitManualCursor(x, y, asc, desc))
			current_view->updateScrollbar();
	}
}


int UnlockInset(UpdatableInset* inset)
{
	if (inset &&
	    current_view->currentBuffer()->the_locking_inset == inset){
		inset->InsetUnlock();
		current_view->currentBuffer()->the_locking_inset = 0;
		current_view->currentBuffer()->text->FinishUndo();
		return 0;
	}
	return bufferlist.unlockInset(inset);
}


void LockedInsetStoreUndo(Undo::undo_kind kind)
{
	if (!current_view->currentBuffer()->the_locking_inset)
		return; // shouldn't happen
	if (kind == Undo::EDIT) // in this case insets would not be stored!
		kind = Undo::FINISH;
	current_view->currentBuffer()->text->SetUndo(kind,
			      current_view->currentBuffer()->text->cursor.par->
			      ParFromPos(current_view->currentBuffer()->text->cursor.pos)->previous, 
			      current_view->currentBuffer()->text->cursor.par->
			      ParFromPos(current_view->currentBuffer()->text->cursor.pos)->next); 
}


void PutInsetIntoInsetUpdateList(Inset* inset)
{
	if (inset) {
		InsetUpdateStruct* tmp = new InsetUpdateStruct();
		tmp->inset = inset;
		tmp->next = InsetUpdateList;
		InsetUpdateList = tmp;
	}
}


void UpdateInsetUpdateList()
{
	InsetUpdateStruct *tmp = InsetUpdateList;
	while (tmp) {
		UpdateInset(tmp->inset, false); // "false" because no document change
		tmp = tmp->next;
	}
  
	/* delete the update list */
	while (InsetUpdateList) {
		tmp = InsetUpdateList;
		InsetUpdateList = InsetUpdateList->next;
		delete tmp;
	}
	InsetUpdateList = 0;
}

#warning UGLY!!
// I know we shouldn't put anything in here but this seems the fastest
// way to do this (and the cleanest for now). This function just inserts
// a newline in the string and the inserts 'depth'-spaces so that the
// code is indented in the right way!!!
void addNewlineAndDepth(string &file, int const depth)
{
       file += '\n';
       for (int j=0;j< depth;j++)
               file += ' ';
}


