/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich,
 *          Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include <fstream>
#include <algorithm>
#include <iostream>

#include "LString.h"
#include FORMS_H_LOCATION
#include "lyx.h"
#include "layout_forms.h"
#include "bullet_forms.h"
#include "form1.h"
#include "lyx_main.h"
#include "lyx_cb.h"
#include "insets/insetref.h"
#include "insets/insetlabel.h"
#include "insets/figinset.h"
#include "lyxfunc.h"
#include "minibuffer.h"
#include "combox.h"
#include "bufferlist.h"
#include "filedlg.h"
#include "lyx_gui_misc.h"
#include "LyXView.h"
#include "BufferView.h"
#include "lastfiles.h"
#include "bufferview_funcs.h"
#include "support/FileInfo.h"
#include "support/syscall.h"
#include "support/filetools.h"
#include "support/path.h"
#include "lyxserver.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "CutAndPaste.h"

using std::ifstream;
using std::copy;
using std::back_inserter;
using std::endl;
using std::cout;
using std::ios;
using std::istream_iterator;
using std::pair;
using std::vector;
using std::sort;
using std::equal;

extern Combox * combo_language;
extern Combox * combo_language2;
extern BufferList bufferlist;
extern void show_symbols_form();
extern FD_form_title * fd_form_title;
extern FD_form_paragraph * fd_form_paragraph;
extern FD_form_character * fd_form_character;
extern FD_form_document * fd_form_document;
extern FD_form_quotes * fd_form_quotes;
extern FD_form_preamble * fd_form_preamble;
extern FD_form_table * fd_form_table;
extern FD_form_figure * fd_form_figure;
extern FD_form_bullet * fd_form_bullet;

extern BufferView * current_view; // called too many times in this file...

extern void DeleteSimpleCutBuffer(); /* for the cleanup when exiting */

extern bool send_fax(string const & fname, string const & sendcmd);
extern void MenuSendto();

extern LyXServer * lyxserver;

// this should be static, but I need it in buffer.C
bool quitting;	// flag, that we are quitting the program
extern bool finished; // all cleanup done just let it run through now.

char ascii_type; /* for selection notify callbacks */

bool scrolling = false;

// This is used to make the dreaded font toggle problem hopefully go
// away. Definitely not the best solution, but I think it sorta works.
bool toggleall = true;

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

//void UpdateInset(BufferView * bv, Inset * inset, bool mark_dirty = true);

/* these functions return 1 if an error occured, 
   otherwise 0 */
// Now they work only for updatable insets. [Alejandro 080596]
//int LockInset(UpdatableInset * inset);
void ToggleLockedInsetCursor(long x, long y, int asc, int desc);
//void FitLockedInsetCursor(long x, long y, int asc, int desc);
//int UnlockInset(UpdatableInset * inset);
//void LockedInsetStoreUndo(Undo::undo_kind kind);

/* this is for asyncron updating. UpdateInsetUpdateList will be called
   automatically from LyX. Just insert the Inset into the Updatelist */
//void UpdateInsetUpdateList();
//void PutInsetIntoInsetUpdateList(Inset * inset);

//InsetUpdateStruct * InsetUpdateList = 0;


/*
  -----------------------------------------------------------------------
 */

/* some function prototypes */

int RunLinuxDoc(BufferView *, int, string const &);
int RunDocBook(int, string const &);
bool MenuWrite(Buffer * buf);
bool MenuWriteAs(Buffer * buffer);
void MenuReload(Buffer * buf);
void MenuLayoutSave();


void ShowMessage(Buffer * buf,
		 string const & msg1,
		 string const & msg2 = string(),
		 string const & msg3 = string(), int delay = 6)
{
	if (lyxrc.use_gui) {
		buf->getUser()->owner()->getMiniBuffer()->Set(msg1, msg2,
							      msg3, delay);
	} else {
		// can somebody think of something more clever? cerr?
		cout << msg1 << msg2 << msg3 << endl;
	}
}


//
// Menu callbacks
//

//
// File menu
//

// should be moved to lyxfunc.C
bool MenuWrite(Buffer * buffer)
{
	XFlush(fl_display);
	if (!buffer->save()) {
		string fname = buffer->fileName();
		string s = MakeAbsPath(fname);
		if (AskQuestion(_("Save failed. Rename and try again?"),
				MakeDisplayPath(s, 50),
				_("(If not, document is not saved.)"))) {
			return MenuWriteAs(buffer);
		}
		return false;
	} else {
		lastfiles->newFile(buffer->fileName());
	}
	return true;
}


// should be moved to BufferView.C
// Half of this func should be in LyXView, the rest in BufferView.
bool MenuWriteAs(Buffer * buffer)
{
	// Why do we require BufferView::text to be able to write a
	// document? I see no point in that. (Lgb)
	//if (!bv->text) return;

	string fname = buffer->fileName();
	string oldname = fname;
	LyXFileDlg fileDlg;

	ProhibitInput(current_view);
	fileDlg.SetButton(0, _("Documents"), lyxrc.document_path);
	fileDlg.SetButton(1, _("Templates"), lyxrc.template_path);

	if (!IsLyXFilename(fname))
		fname += ".lyx";

	if (buffer->isUnnamed()) {
		fname = fileDlg.Select(_("Enter Filename to Save Document as"), 
				       "",
				       "*.lyx", 
				       "");
	} else {
		fname = fileDlg.Select(_("Enter Filename to Save Document as"), 
				       OnlyPath(fname),
				       "*.lyx", 
				       OnlyFilename(fname));
	}
	AllowInput(current_view);

	if (fname.empty()) {
		return false;
	}
	// Make sure the absolute filename ends with appropriate suffix
	string s = MakeAbsPath(fname);
	if (!IsLyXFilename(s))
		s += ".lyx";

	// Same name as we have already?
	if (s == oldname) {
		if (!AskQuestion(_("Same name as document already has:"),
				 MakeDisplayPath(s, 50),
				 _("Save anyway?")))
			return false;
		// Falls through to name change and save
	} 
	// No, but do we have another file with this name open?
	else if (bufferlist.exists(s)) {
		if (AskQuestion(_("Another document with same name open!"),
				MakeDisplayPath(s, 50),
				_("Replace with current document?")))
			{
				bufferlist.close(bufferlist.getBuffer(s));

				// Ok, change the name of the buffer, but don't save!
				buffer->fileName(s);
				buffer->markDirty();

				ShowMessage(buffer, _("Document renamed to '"),
						MakeDisplayPath(s), _("', but not saved..."));
			}
		return false;
	} // Check whether the file exists
	else {
		FileInfo myfile(s);
		if (myfile.isOK() && !AskQuestion(_("Document already exists:"), 
						  MakeDisplayPath(s, 50),
						  _("Replace file?")))
			return false;
	}

	// Ok, change the name of the buffer
	buffer->fileName(s);
	buffer->markDirty();
	bool unnamed = buffer->isUnnamed();
	buffer->setUnnamed(false);
	// And save
	// Small bug: If the save fails, we have irreversible changed the name
	// of the document.
	// Hope this is fixed this way! (Jug)
	if (!MenuWrite(buffer)) {
	    buffer->fileName(oldname);
	    buffer->setUnnamed(unnamed);
	    ShowMessage(buffer, _("Document could not be saved!"),
			_("Holding the old name."), MakeDisplayPath(oldname));
	    return false;
	}
	// now remove the oldname autosave file if existant!
	removeAutosaveFile(oldname);
	return true;
}


int MenuRunLaTeX(Buffer * buffer)
{
	int ret = 0;

	if (buffer->isLinuxDoc())
		ret = RunLinuxDoc(buffer->getUser(), 1, buffer->fileName());
	else if (buffer->isLiterate())
	        ret = buffer->runLiterate();
	else if (buffer->isDocBook())
		ret = RunDocBook(1, buffer->fileName());
	else
		ret = buffer->runLaTeX();
   
	if (ret > 0) {
		string s;
		string t;
		if (ret == 1) {
			s = _("One error detected");
			t = _("You should try to fix it.");
		} else {
			s += tostr(ret);
			s += _(" errors detected.");
			t = _("You should try to fix them.");
		}
		WriteAlert(_("There were errors during the LaTeX run."), s, t);
	}
	return ret;
}


int MenuBuildProg(Buffer * buffer)
{
	int ret = 0;
	
	if (buffer->isLiterate())
		ret = buffer->buildProgram();
	else {
		string s = _("Wrong type of document");
		string t = _("The Build operation is not allowed in this document");
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
			s += tostr(ret);
			s += _(" errors detected.");
			t = _("You should try to fix them.");
		}
		WriteAlert(_("There were errors during the Build process."), s, t);
	}
	return ret;
}


int MenuRunChktex(Buffer * buffer)
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
			s += tostr(ret);
			s += _(" warnings found.");
			t = _("Use 'Edit->Go to Error' to find them.");
		}
		WriteAlert(_("Chktex run successfully"), s, t);
	} else {
		WriteAlert(_("Error!"), _("It seems chktex does not work."));
	}
	return ret;
}

 
int MakeLaTeXOutput(Buffer * buffer)
{
	// Who cares?
	//if (!bv->text)
	//	return 1;
	int ret = 0;
	string path = OnlyPath(buffer->fileName());
	if (lyxrc.use_tempdir || (IsDirWriteable(path) < 1)) {
		path = buffer->tmppath;
	}
		
    Path p(path);
	ret = MenuRunLaTeX(buffer);
	
    return ret;
}


/* wait == false means don't wait for termination */
/* wait == true means wait for termination       */
// The bool should be placed last on the argument line. (Lgb)
// Returns false if we fail.
bool RunScript(Buffer * buffer, bool wait,
	       string const & command,
	       string const & orgname = string(),
	       bool need_shell = true)
{
	string path;
	string cmd;
	string name = orgname;
	int result = 0;
	
	if (MakeLaTeXOutput(buffer) > 0)
		return false;
	/* get DVI-Filename */
	if (name.empty())
		name = ChangeExtension(buffer->getLatexName(), ".dvi");

	path = OnlyPath(name);
	if (lyxrc.use_tempdir || (IsDirWriteable(path) < 1)) {
		path = buffer->tmppath;
	}
	Path p(path);
	// At this point we check whether the command contains the
	// filename parameter $$FName and if that's the case we
	// substitute the real file name otherwise the filename is
	// simply appended. rokrau 1/12/00
	cmd = command;
	string::size_type i;
 	if ( (i=command.find("$$FName")) != string::npos)
	{
		cmd.replace(i,7,QuoteName(name));
	}
	else
		cmd = command + ' ' + QuoteName(name);

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
                        sh = lowercase(sh);
                        if (contains(sh, "cmd.exe")
			    || contains(sh, "4os2.exe"))
                                cmd = "start /min/n " + cmd;
                        else
                                cmd += " &";
                }
#endif
		// It seems that, if wait is false, we never get back
		// the return code of the command. This means that all
		// the code I added in PrintApplyCB is currently
		// useless...
		// CHECK What should we do here?
		ShowMessage(buffer, _("Executing command:"), cmd);
		result = one.startscript(Systemcalls::System, cmd);
	} else {
		ShowMessage(buffer, _("Executing command:"), cmd);
		result = one.startscript(wait ? Systemcalls::Wait
					 : Systemcalls::DontWait, cmd);
	}
	return result == 0;
}


// Returns false if we fail
bool CreatePostscript(Buffer * buffer, bool wait = false)
{
	// Who cares?
	//if (!bv->text)
	//	return false;

	ProhibitInput(current_view);

	// Generate dvi file
        if (MakeLaTeXOutput(buffer) > 0) {
            	AllowInput(current_view);
		return false;
        }
	// Generate postscript file
	string psname = OnlyFilename(ChangeExtension (buffer->fileName(),
					 ".ps_tmp"));

	string paper;

	// Wrong type
	char real_papersize = buffer->params.papersize;
	if (real_papersize == BufferParams::PAPER_DEFAULT)
		real_papersize = lyxrc.default_papersize;

	switch (real_papersize) {
	case BufferParams::PAPER_USLETTER:
		paper = "letter";
		break;
	case BufferParams::PAPER_A3PAPER:
		paper = "a3";
		break;
	case BufferParams::PAPER_A4PAPER:
		paper = "a4";
		break;
	case BufferParams::PAPER_A5PAPER:
		paper = "a5";
		break;
	case BufferParams::PAPER_B5PAPER:
		paper = "b5";
		break;
	case BufferParams::PAPER_EXECUTIVEPAPER:
		paper = "foolscap";
		break;
	case BufferParams::PAPER_LEGALPAPER:
		paper = "legal";
		break;
	default: /* If nothing else fits, keep an empty value... */
		break;
	}

	// Make postscript file.
	string command = lyxrc.dvi_to_ps_command + ' ' + lyxrc.print_to_file + ' ';
	command += QuoteName(psname);
	if (buffer->params.use_geometry
	    && buffer->params.papersize2 == BufferParams::VM_PAPER_CUSTOM
	    && !lyxrc.print_paper_dimension_flag.empty()
	    && !buffer->params.paperwidth.empty()
	    && !buffer->params.paperheight.empty()) {
		// using a custom papersize
		command += ' ';
		command += lyxrc.print_paper_dimension_flag + ' ';
		command += buffer->params.paperwidth + ',';
		command += buffer->params.paperheight;
	} else if (!paper.empty()
		   && (real_papersize != BufferParams::PAPER_USLETTER ||
		       buffer->params.orientation == BufferParams::ORIENTATION_PORTRAIT)) {
		// dvips won't accept -t letter -t landscape.  In all other
		// cases, include the paper size explicitly.
		command += ' ';
		command += lyxrc.print_paper_flag + ' ' + paper;
	}
	if (buffer->params.orientation == BufferParams::ORIENTATION_LANDSCAPE) {
		command += ' ';
		command += lyxrc.print_landscape_flag;
	}
	// push directorypath, if necessary 
        string path = OnlyPath(buffer->fileName());
        if (lyxrc.use_tempdir || (IsDirWriteable(path) < 1)){
		path = buffer->tmppath;
        }
        Path p(path);
	bool ret = RunScript(buffer, wait, command);
	AllowInput(current_view);
	return ret;
}


// Returns false if we fail
//bool MenuPreviewPS(Buffer * buffer)
bool PreviewPostscript(Buffer * buffer)
{
	// Who cares?
	//if (!bv->text)
	//	return false;

	// Generate postscript file
	if (!CreatePostscript(buffer, true)) {
		return false;
	}

	// Start postscript viewer
	ProhibitInput(current_view);
	string ps = OnlyFilename(ChangeExtension (buffer->fileName(),
				     ".ps_tmp"));
	// push directorypath, if necessary 
        string path = OnlyPath(buffer->fileName());
        if (lyxrc.use_tempdir || (IsDirWriteable(path) < 1)){
		path = buffer->tmppath;
        }
        Path p(path);
	bool ret = RunScript(buffer, false, lyxrc.view_ps_command, ps);
	AllowInput(current_view);
	return ret;
}


void MenuFax(Buffer * buffer)
{
	// Who cares?
	//if (!bv->text)
	//	return;

	// Generate postscript file
	if (!CreatePostscript(buffer, true)) {
		return;
	}

	// Send fax
	string ps = OnlyFilename(ChangeExtension (buffer->fileName(), 
						  ".ps_tmp"));
	string path = OnlyPath (buffer->fileName());
	if (lyxrc.use_tempdir || (IsDirWriteable(path) < 1)) {
		path = buffer->tmppath;
	}
	Path p(path);
	if (!lyxrc.fax_program.empty()) {
                string help2 = subst(lyxrc.fax_program, "$$FName", ps);
                help2 += " &";
                Systemcalls one(Systemcalls::System, help2);
	} else
		send_fax(ps, lyxrc.fax_command);
}


// Returns false if we fail
bool PreviewDVI(Buffer * buffer)
{
	// Who cares?
	//if (!bv->text)
	//	return false;

	string paper = lyxrc.view_dvi_paper_option;
	if (!paper.empty()) {
		// wrong type
		char real_papersize = buffer->params.papersize;
		if (real_papersize == BufferParams::PAPER_DEFAULT)
			real_papersize = lyxrc.default_papersize;
  
		switch (real_papersize) {
		case BufferParams::PAPER_USLETTER:
			paper += " us";
			break;
		case BufferParams::PAPER_A3PAPER:
			paper += " a3";
			break;
		case BufferParams::PAPER_A4PAPER:
			paper += " a4";
			break;
		case BufferParams::PAPER_A5PAPER:
			paper += " a5";
			break;
		case BufferParams::PAPER_B5PAPER:
			paper += " b5";
			break;
		case BufferParams::PAPER_EXECUTIVEPAPER:
			paper += " foolscap";
			break;
		case BufferParams::PAPER_LEGALPAPER:
			paper += " legal";
			break;
		default: /* If nothing else fits, keep the empty value */
			break;
		}
		if (real_papersize==' ') {
			//      if (paper.empty()) {
		  	if (buffer->params.orientation 
			    == BufferParams::ORIENTATION_LANDSCAPE)
			  // we HAVE to give a size when the page is in
			  // landscape, so use USletter.          
				paper = " -paper usr";
		} else {
			// paper = " -paper " + paper;
			if (buffer->params.orientation 
			    == BufferParams::ORIENTATION_LANDSCAPE)
                               paper+= 'r';
		}
        }
        // push directorypath, if necessary 
	string path = OnlyPath(buffer->fileName());
	if (lyxrc.use_tempdir || (IsDirWriteable(path) < 1)) {
		path = buffer->tmppath;
	}
	Path p(path);
        // Run dvi-viewer
	string command = lyxrc.view_dvi_command + " " + paper;
	bool ret = RunScript(buffer, false, command);
	return ret;
}


bool AskOverwrite(Buffer * buffer, string const & s)
{
	if (lyxrc.use_gui) {
		// be friendly if there is a gui
		FileInfo fi(s);
		if (fi.readable() &&
				!AskQuestion(_("File already exists:"), 
				 MakeDisplayPath(s, 50),
				 _("Do you want to overwrite the file?"))) {
			ShowMessage(buffer, _("Canceled"));
			return false;
		}
	}
	return true;
}


void MenuMakeLaTeX(Buffer * buffer)
{
	// Why care about this?
	//if (!bv->text)
	//	return;
	
	// Get LaTeX-Filename
	string s = buffer->getLatexName(false);
	
	if (!AskOverwrite(buffer, s))
		return; 
	
	if (buffer->isDocBook())
		ShowMessage(buffer, _("DocBook does not have a latex backend"));
	else {
		if (buffer->isLinuxDoc())
			RunLinuxDoc(buffer->getUser(), 0, buffer->fileName());
		else
			buffer->makeLaTeXFile(s, string(), true);
		ShowMessage(buffer, _("Nice LaTeX file saved as"), 
			    MakeDisplayPath(s));
	}
}


void MenuMakeLinuxDoc(Buffer * buffer)
{
	// Who cares?
	//if (!bv->text) return;
	
	if (!buffer->isLinuxDoc()) {
		WriteAlert(_("Error!"), _("Document class must be linuxdoc."));
		return;
	}
	
	// Get LinuxDoc-Filename
	string s = ChangeExtension(buffer->fileName(), ".sgml");

	if (!AskOverwrite(buffer, s))
		return;
	
	ShowMessage(buffer, _("Building LinuxDoc SGML file `"),
					  MakeDisplayPath(s),"'...");
	
	buffer->makeLinuxDocFile(s, true);
	buffer->redraw();
	ShowMessage(buffer, _("LinuxDoc SGML file save as"),
					  MakeDisplayPath(s)); 
}


void MenuMakeDocBook(Buffer * buffer)
{
	// Who cares?
	//if (!bv->text) return;
	
	if (!buffer->isDocBook()) {
		WriteAlert(_("Error!"),
			   _("Document class must be docbook."));
		return;
	}
	
	// Get DocBook-Filename
	string s = ChangeExtension(buffer->fileName(), ".sgml");

	if (!AskOverwrite(buffer, s))
		return;
	
	ShowMessage(buffer, _("Building DocBook SGML file `"),
					  MakeDisplayPath(s), "'..."); 
	
	buffer->makeDocBookFile(s, true);
	buffer->redraw();
	ShowMessage(buffer, _("DocBook SGML file save as"),
					  MakeDisplayPath(s)); 
}


void MenuMakeAscii(Buffer * buffer)
{
	// Who cares?
	//if (!bv->text) return;
	
	/* get LaTeX-Filename */
	string s = ChangeExtension (buffer->fileName(), ".txt");
	

	if (!AskOverwrite(buffer, s))
		return;
	
	buffer->writeFileAscii(s, lyxrc.ascii_linelen);
	
	ShowMessage(buffer, _("Ascii file saved as"), MakeDisplayPath(s));
}


void MenuMakeHTML(Buffer * buffer)
{
	// First, create LaTeX file
	MenuMakeLaTeX(buffer);

	// And now, run the converter
	string file = buffer->fileName();
	Path path(OnlyPath(file));
	// the tex file name has to be correct for
	// latex, but the html file name can be
	// anything.
	string result = ChangeExtension(file, ".html");
	string infile = buffer->getLatexName(false);
	string tmp = lyxrc.html_command;
	tmp = subst(tmp, "$$FName", infile);
	tmp = subst(tmp, "$$OutName", result);
	Systemcalls one;
	int res = one.startscript(Systemcalls::System, tmp);
	if (res == 0) {
		ShowMessage(buffer, _("Document exported as HTML to file `")
						  + MakeDisplayPath(result) +'\'');
	} else {
		ShowMessage(buffer, _("Unable to convert to HTML the file `")
						  + MakeDisplayPath(infile) 
						  + '\'');
	}

}


void MenuMakeHTML_LinuxDoc(Buffer * buffer)
{
	// First, create LinuxDoc file
	MenuMakeLinuxDoc(buffer);

	// And now, run the converter
	string file = buffer->fileName();

	string result = ChangeExtension(file, ".html");
	string infile = ChangeExtension(file, ".sgml");
	string tmp = lyxrc.linuxdoc_to_html_command;
	tmp = subst(tmp, "$$FName", infile);
	tmp = subst(tmp, "$$OutName", result);
	Systemcalls one;
	int res = one.startscript(Systemcalls::System, tmp);
	if (res == 0) {
		ShowMessage(buffer,_("Document exported as HTML to file `")
						  + MakeDisplayPath(result) +'\'');
	} else {
		ShowMessage(buffer,_("Unable to convert to HTML the file `")
						  + MakeDisplayPath(infile) 
						  + '\'');
	}

}

void MenuMakeHTML_DocBook(Buffer * buffer)
{
	// First, create LaTeX file
	MenuMakeDocBook(buffer);

	// And now, run the converter
	string file = buffer->fileName();
	string result = ChangeExtension(file, ".html");
	string infile = ChangeExtension(file, ".sgml");
	string tmp = lyxrc.docbook_to_html_command;
	tmp = subst(tmp, "$$FName", infile);
	tmp = subst(tmp, "$$OutName", result);
	Systemcalls one;
	int res = one.startscript(Systemcalls::System, tmp);
	if (res == 0) {
		ShowMessage(buffer,_("Document exported as HTML to file `")
						  + MakeDisplayPath(result) +'\'');
	} else {
		ShowMessage(buffer,_("Unable to convert to HTML the file `")
						  + MakeDisplayPath(infile) 
						  + '\'');
	}

}


void MenuExport(Buffer * buffer, string const & extyp) 
{
	// latex
	if (extyp == "latex") {
		// make sure that this buffer is not linuxdoc
		MenuMakeLaTeX(buffer);
	}
	// linuxdoc
	else if (extyp == "linuxdoc") {
		// make sure that this buffer is not latex
		MenuMakeLinuxDoc(buffer);
	}
	// docbook
	else if (extyp == "docbook") {
		// make sure that this buffer is not latex or linuxdoc
		MenuMakeDocBook(buffer);
	}
	// dvi
	else if (extyp == "dvi") {
		// Run LaTeX as "Update dvi..." Bernhard.
		// We want the dvi in the current directory. This
		// is achieved by temporarily disabling use of
		// temp directory. As a side-effect, we get
		// *.log and *.aux files also. (Asger)
		bool flag = lyxrc.use_tempdir;
		lyxrc.use_tempdir = false;
		MenuRunLaTeX(buffer);
		lyxrc.use_tempdir = flag;
	}
	// postscript
	else if (extyp == "postscript") {
		// Start Print-dialog. Not as good as dvi... Bernhard.
	//should start lyxview->getDialogs()->showPrint();
	// to get same as before
	//		MenuPrint(buffer);
		// Since the MenuPrint is a pop-up, we can't use
		// the same trick as above. (Asger)
		// MISSING: Move of ps-file :-(
		// And MenuPrint should not be used for this at all...
	}
	// ascii
	else if (extyp == "ascii") {
		MenuMakeAscii(buffer);
	}
	else if (extyp == "custom") {
		MenuSendto();
	}
	// HTML
	else if (extyp == "html") {
		if (buffer->isLinuxDoc())
			MenuMakeHTML_LinuxDoc(buffer);
		else if (buffer->isDocBook())
			MenuMakeHTML_DocBook(buffer);
		else
			MenuMakeHTML(buffer);
	}
	else {
		ShowMessage(buffer, _("Unknown export type: ") + extyp);
	}
}


void QuitLyX()
{
	lyxerr.debug() << "Running QuitLyX." << endl;

	if (!bufferlist.QwriteAll())
		return;

	lastfiles->writeFile(lyxrc.lastfiles);

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



void AutoSave(BufferView * bv)
	// should probably be moved into BufferList (Lgb)
	// Perfect target for a thread...
{
	if (!bv->available())
		return;

	if (bv->buffer()->isBakClean() || bv->buffer()->isReadonly()) {
		// We don't save now, but we'll try again later
		bv->owner()->resetAutosaveTimer();
		return;
	}

	bv->owner()->getMiniBuffer()->Set(_("Autosaving current document..."));
	
	// create autosave filename
	string fname = 	OnlyPath(bv->buffer()->fileName());
	fname += "#";
	fname += OnlyFilename(bv->buffer()->fileName());
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
			bv->buffer()->writeFile(tmp_ret, 1);
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
			if (!bv->buffer()->writeFile(fname, 1)) {
				// It is dangerous to do this in the child,
				// but safe in the parent, so...
				if (pid == -1)
					bv->owner()->getMiniBuffer()->Set(_("Autosave Failed!"));
			}
		}
		if (pid == 0) { // we are the child so...
			_exit(0);
		}
	}
	
	bv->buffer()->markBakClean();
	bv->owner()->resetAutosaveTimer();
}


//
// Copyright CHT Software Service GmbH
// Uwe C. Schroeder
//
// create new file with template
// SERVERCMD !
//
Buffer * NewLyxFile(string const & filename)
{
	// Split argument by :
	string name;
	string tmpname = split(filename, name, ':');
#ifdef __EMX__ // Fix me! lyx_cb.C may not be low level enough to allow this.
	if (name.length() == 1
	    && isalpha(static_cast<unsigned char>(name[0]))
	    && (prefixIs(tmpname, "/") || prefixIs(tmpname, "\\"))) {
		name += ':';
		name += token(tmpname, ':', 0);
		tmpname = split(tmpname, ':');
	}
#endif
	lyxerr.debug() << "Arg is " << filename
		       << "\nName is " << name
		       << "\nTemplate is " << tmpname << endl;

	// find a free buffer 
	Buffer * tmpbuf = bufferlist.newFile(name, tmpname);
	if (tmpbuf)
		lastfiles->newFile(tmpbuf->fileName());
	return tmpbuf;
}


// Insert ascii file (if filename is empty, prompt for one)
void InsertAsciiFile(BufferView * bv, string const & f, bool asParagraph)
{
	string fname = f;
	LyXFileDlg fileDlg;
 
	if (!bv->available()) return;
     
	if (fname.empty()) {
		ProhibitInput(bv);
		fname = fileDlg.Select(_("File to Insert"), 
				       bv->owner()->buffer()->filepath,
				       "*");
  		AllowInput(bv);
		if (fname.empty()) return;
	}

	FileInfo fi(fname);

	if (!fi.readable()) {
		WriteFSAlert(_("Error! Specified file is unreadable: "),
			     MakeDisplayPath(fname, 50));
		return;
	}

	ifstream ifs(fname.c_str());
	if (!ifs) {
		WriteFSAlert(_("Error! Cannot open specified file: "),
			     MakeDisplayPath(fname, 50));
		return;
	}

	ifs.unsetf(ios::skipws);
	istream_iterator<char> ii(ifs);
	istream_iterator<char> end;
#if !defined(USE_INCLUDED_STRING) && !defined(STD_STRING_IS_GOOD)
	// We use this until the compilers get better...
	vector<char> tmp;
	copy(ii, end, back_inserter(tmp));
	string tmpstr(tmp.begin(), tmp.end());
#else
	// This is what we want to use and what we will use once the
	// compilers get good enough. 
	//string tmpstr(ii, end); // yet a reason for using std::string
	// alternate approach to get the file into a string:
	string tmpstr;
	copy(ii, end, back_inserter(tmpstr));
#endif
	// insert the string
	current_view->hideCursor();
	
	// clear the selection
	bv->beforeChange();
	if (!asParagraph)
		bv->text->InsertStringA(bv, tmpstr);
	else
		bv->text->InsertStringB(bv, tmpstr);
	bv->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
}


void MenuInsertLabel(char const * arg)
{
	string label = arg;
	ProhibitInput(current_view);
	if (label.empty()) {
		pair<bool, string>
			result = askForText(_("Enter new label to insert:"));
		if (result.first) {
			label = frontStrip(strip(result.second));
		}
	}
	if (!label.empty()) {
		InsetCommandParams p( "label", label );
		InsetLabel * inset = new InsetLabel( p );
		current_view->insertInset( inset );
	}
	AllowInput(current_view);
}


// candidate for move to LyXView
// This is only used in toolbar.C
void LayoutsCB(int sel, void *)
{
	string tmp = tostr(sel);
	current_view->owner()->getLyXFunc()->Dispatch(LFUN_LAYOUTNO,
						      tmp.c_str());
}


/*
 * SGML Linuxdoc support:
 * (flag == 0) make TeX output
 * (flag == 1) make dvi output
 */
int RunLinuxDoc(BufferView * bv, int flag, string const & filename)
{
	string s2;
	string add_flags;

	int errorcode = 0;

	/* generate a path-less extension name */
	string name = OnlyFilename(ChangeExtension (filename, ".sgml"));
	string path = OnlyPath (filename);
	if (lyxrc.use_tempdir || (IsDirWriteable(path) < 1)) {
		path = bv->buffer()->tmppath;
	}
	Path p(path);
	
	if (!bv->available())
		return 0;
	bv->buffer()->makeLinuxDocFile(name, false);

	// CHECK remove this once we have a proper geometry class

	BufferParams::PAPER_SIZE ps =
		static_cast<BufferParams::PAPER_SIZE>(bv->buffer()->params.papersize);
	switch (ps) {
	case BufferParams::PAPER_A4PAPER:
		add_flags = "-p a4";
		break;
	case BufferParams::PAPER_USLETTER:
		add_flags = "-p letter";
		break;
	default: /* nothing to be done yet ;-) */     break; 
	}
	
	ProhibitInput(bv);
	
	Systemcalls one;
	switch (flag) {
	case 0: /* TeX output asked */
	      bv->owner()->getMiniBuffer()->Set(_("Converting LinuxDoc SGML to TeX file..."));
		s2 = lyxrc.linuxdoc_to_latex_command + ' ' + add_flags + " -o tex " + ' ' + name;
		if (one.startscript(Systemcalls::System, s2)) 
			errorcode = 1;
		break;
	case 1: /* dvi output asked */
		bv->owner()->getMiniBuffer()->Set(_("Converting LinuxDoc SGML to dvi file..."));
		s2 = lyxrc.linuxdoc_to_latex_command + ' ' + add_flags + " -o dvi " + ' ' + name;
		if (one.startscript(Systemcalls::System, s2)) {
			errorcode = 1;
		} 
		break;
	default: /* unknown output */
		break;
	}
	
	AllowInput(bv);

        bv->buffer()->redraw();
	return errorcode;
}


/*
 * SGML DocBook support:
 * (flag == 1) make dvi output
 */
int RunDocBook(int flag, string const & filename)
{
	/* generate a path-less extension name */
	string name = OnlyFilename(ChangeExtension (filename, ".sgml"));
	string path = OnlyPath (filename);
	if (lyxrc.use_tempdir || (IsDirWriteable(path) < 1)) {
		path = current_view->buffer()->tmppath;
	}
	Path p(path);

	if (!current_view->available())
		return 0;
	
	current_view->buffer()->makeDocBookFile(name, false);

	// Shall this code go or should it stay? (Lgb)
	// This code is a placeholder for future implementation. (Jose')
//  	string add_flags;
//  	LYX_PAPER_SIZE ps = (LYX_PAPER_SIZE) current_view->buffer()->params.papersize;
//  	switch (ps) {
//  	case BufferParams::PAPER_A4PAPER:  add_flags = "-p a4";     break;
//  	case BufferParams::PAPER_USLETTER: add_flags = "-p letter"; break;
//  	default: /* nothing to be done yet ;-) */     break; 
//  	}
	ProhibitInput(current_view);
	
	int errorcode = 0;
	Systemcalls one;
	switch (flag) {
	case 1: /* dvi output asked */
	{
		current_view->owner()->getMiniBuffer()->Set(_("Converting DocBook SGML to dvi file..."));
		string s2 = lyxrc.docbook_to_dvi_command + ' ' + name;
		if (one.startscript(Systemcalls::System, s2)) {
			errorcode = 1;
		}
	}
	break;
	default: /* unknown output */
		break;
	}
	
	AllowInput(current_view);

        current_view->buffer()->redraw();
	return errorcode;
}


void MenuLayoutCharacter()
{
	static int ow = -1, oh;

	if (fd_form_character->form_character->visible) {
		fl_raise_form(fd_form_character->form_character);
	} else {
		fl_show_form(fd_form_character->form_character,
			     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_FULLBORDER,
			     _("Character Style"));
		if (ow < 0) {
			ow = fd_form_character->form_character->w;
			oh = fd_form_character->form_character->h;
		}
		fl_set_form_minsize(fd_form_character->form_character, ow, oh);
	}
}


inline
void DeactivateParagraphButtons ()
{
	fl_deactivate_object (fd_form_paragraph->button_ok);
	fl_deactivate_object (fd_form_paragraph->button_apply);
	fl_set_object_lcol (fd_form_paragraph->button_ok, FL_INACTIVE);
	fl_set_object_lcol (fd_form_paragraph->button_apply, FL_INACTIVE);
}


inline
void ActivateParagraphButtons ()
{
	fl_activate_object (fd_form_paragraph->button_ok);
	fl_activate_object (fd_form_paragraph->button_apply);
	fl_set_object_lcol (fd_form_paragraph->button_ok, FL_BLACK);
	fl_set_object_lcol (fd_form_paragraph->button_apply, FL_BLACK);
}


inline
void DisableParagraphLayout ()
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


inline
void EnableParagraphLayout ()
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
	if (!current_view->available()) {
		if (fd_form_paragraph->form_paragraph->visible) 
			fl_hide_form(fd_form_paragraph->form_paragraph);
		return false;
	}

	Buffer * buf = current_view->buffer();
	LyXText * text = 0;
	if (current_view->the_locking_inset)
	    text = current_view->the_locking_inset->getLyXText(current_view);
	if (!text)
	    text = current_view->text;

	fl_set_input(fd_form_paragraph->input_labelwidth,
		     text->cursor.par()->GetLabelWidthString().c_str());
	fl_set_button(fd_form_paragraph->radio_align_right, 0);
	fl_set_button(fd_form_paragraph->radio_align_left, 0);
	fl_set_button(fd_form_paragraph->radio_align_center, 0);
	fl_set_button(fd_form_paragraph->radio_align_block, 0);

	int align = text->cursor.par()->GetAlign();
	if (align == LYX_ALIGN_LAYOUT)
		align = textclasslist.Style(buf->params.textclass,
					    text->cursor.par()->GetLayout()).align;
	 
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

#ifndef NEW_INSETS
	fl_set_button(fd_form_paragraph->check_lines_top,
		      text->cursor.par()->FirstPhysicalPar()->line_top);

	fl_set_button(fd_form_paragraph->check_lines_bottom,
		      text->cursor.par()->FirstPhysicalPar()->line_bottom);

	fl_set_button(fd_form_paragraph->check_pagebreaks_top,
		      text->cursor.par()->FirstPhysicalPar()->pagebreak_top);

	fl_set_button(fd_form_paragraph->check_pagebreaks_bottom,
		      text->cursor.par()->FirstPhysicalPar()->pagebreak_bottom);
	fl_set_button(fd_form_paragraph->check_noindent,
		      text->cursor.par()->FirstPhysicalPar()->noindent);
#else
	fl_set_button(fd_form_paragraph->check_lines_top,
		      text->cursor.par()->line_top);
	fl_set_button(fd_form_paragraph->check_lines_bottom,
		      text->cursor.par()->line_bottom);
	fl_set_button(fd_form_paragraph->check_pagebreaks_top,
		      text->cursor.par()->pagebreak_top);
	fl_set_button(fd_form_paragraph->check_pagebreaks_bottom,
		      text->cursor.par()->pagebreak_bottom);
	fl_set_button(fd_form_paragraph->check_noindent,
		      text->cursor.par()->noindent);
#endif
	fl_set_input (fd_form_paragraph->input_space_above, "");

#ifndef NEW_INSETS
	switch (text->cursor.par()->FirstPhysicalPar()->added_space_top.kind()) {
#else
	switch (text->cursor.par()->added_space_top.kind()) {
#endif

	case VSpace::NONE:
		fl_set_choice (fd_form_paragraph->choice_space_above, 1);
		break;
	case VSpace::DEFSKIP:
		fl_set_choice (fd_form_paragraph->choice_space_above, 2);
		break;
	case VSpace::SMALLSKIP:
		fl_set_choice (fd_form_paragraph->choice_space_above, 3);
		break;
	case VSpace::MEDSKIP:
		fl_set_choice (fd_form_paragraph->choice_space_above, 4);
		break;
	case VSpace::BIGSKIP:
		fl_set_choice (fd_form_paragraph->choice_space_above, 5);
		break;
	case VSpace::VFILL:
		fl_set_choice (fd_form_paragraph->choice_space_above, 6);
		break;
	case VSpace::LENGTH:
		fl_set_choice (fd_form_paragraph->choice_space_above, 7);
#ifndef NEW_INSETS
		fl_set_input  (fd_form_paragraph->input_space_above, 
			       text->cursor.par()->FirstPhysicalPar()->added_space_top.length().asString().c_str());
#else
		fl_set_input  (fd_form_paragraph->input_space_above, 
			       text->cursor.par()->added_space_top.length().asString().c_str());
#endif
		break;
	}
#ifndef NEW_INSETS
	fl_set_button (fd_form_paragraph->check_space_above,
		       text->cursor.par()->FirstPhysicalPar()->added_space_top.keep());
	fl_set_input (fd_form_paragraph->input_space_below, "");
	switch (text->cursor.par()->FirstPhysicalPar()->added_space_bottom.kind()) {
#else
	fl_set_button (fd_form_paragraph->check_space_above,
		       text->cursor.par()->added_space_top.keep());
	fl_set_input (fd_form_paragraph->input_space_below, "");
	switch (text->cursor.par()->added_space_bottom.kind()) {
#endif
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
#ifndef NEW_INSETS
		fl_set_input  (fd_form_paragraph->input_space_below, 
			       text->cursor.par()->FirstPhysicalPar()->added_space_bottom.length().asString().c_str());
		break;
	}
	fl_set_button (fd_form_paragraph->check_space_below,
		       text->cursor.par()->FirstPhysicalPar()->added_space_bottom.keep());

	fl_set_button(fd_form_paragraph->check_noindent,
		      text->cursor.par()->FirstPhysicalPar()->noindent);
#else
		fl_set_input  (fd_form_paragraph->input_space_below, 
			       text->cursor.par()->added_space_bottom.length().asString().c_str());
		break;
	}
	fl_set_button (fd_form_paragraph->check_space_below,
		       text->cursor.par()->added_space_bottom.keep());

	fl_set_button(fd_form_paragraph->check_noindent,
		      text->cursor.par()->noindent);
#endif
	if (current_view->buffer()->isReadonly()) {
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
				     FL_PLACE_MOUSE, FL_FULLBORDER,
				     _("Paragraph Environment"));
		}
	}
}

#ifdef USE_OLD_DOCUMENT_LAYOUT
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


bool UpdateLayoutDocument(BufferParams * params)
{
	if (!current_view->available()) {
		if (fd_form_document->form_document->visible) 
			fl_hide_form(fd_form_document->form_document);
		return false;
	}		

	if (params == 0)
		params = &current_view->buffer()->params;
	LyXTextClass const & tclass = textclasslist.TextClass(params->textclass);
	
	fl_set_choice_text(fd_form_document->choice_class, 
			   textclasslist.DescOfClass(params->textclass).c_str());
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
			tclass.opt_fontsize().c_str());
	fl_set_choice(fd_form_document->choice_fontsize, 
		      tokenPos(tclass.opt_fontsize(), '|', params->fontsize) + 2);

	// ale970405+lasgoutt970513
	fl_clear_choice(fd_form_document->choice_pagestyle);
	fl_addto_choice(fd_form_document->choice_pagestyle, "default");
	fl_addto_choice(fd_form_document->choice_pagestyle, 
			tclass.opt_pagestyle().c_str());
    
	fl_set_choice(fd_form_document->choice_pagestyle,
		      tokenPos(tclass.opt_pagestyle(), '|', params->pagestyle) + 2);

	fl_set_button(fd_form_document->radio_indent, 0);
	fl_set_button(fd_form_document->radio_skip, 0);
    
        
	fl_set_button(fd_form_document->check_use_amsmath, params->use_amsmath);

	if (params->paragraph_separation == BufferParams::PARSEP_INDENT)
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
   
	switch (params->sides) {
	case LyXTextClass::OneSide:
		fl_set_button(fd_form_document->radio_sides_one, 1);
		break;
	case LyXTextClass::TwoSides:
		fl_set_button(fd_form_document->radio_sides_two, 1);
		break;
	}
   
	fl_set_button(fd_form_document->radio_columns_one, 0);
	fl_set_button(fd_form_document->radio_columns_two, 0);
   
	if (params->columns == 2)
		fl_set_button(fd_form_document->radio_columns_two, 1);
	else
		fl_set_button(fd_form_document->radio_columns_one, 1);
   
	fl_set_input(fd_form_document->input_spacing, "");
	switch (params->spacing.getSpace()) {
	case Spacing::Default: // nothing bad should happen with this
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
		//char sval[20];
		//sprintf(sval, "%g", params->spacing.getValue()); 
#ifdef HAVE_SSTREAM
		std::ostringstream sval;
		sval << params->spacing.getValue(); // setw?
		fl_set_input(fd_form_document->input_spacing,
			     sval.str().c_str());
#else
		char tval[20];
		ostrstream sval(tval, 20);
		sval << params->spacing.getValue() << '\0'; // setw?
		fl_set_input(fd_form_document->input_spacing, sval.str());
#endif
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

	if (current_view->buffer()->isSGML()) {
		// bullets not used in SGML derived documents
		fl_deactivate_object(fd_form_document->button_bullets);
		fl_set_object_lcol(fd_form_document->button_bullets,
				   FL_INACTIVE);
	} else {
		fl_activate_object(fd_form_document->button_bullets);
		fl_set_object_lcol(fd_form_document->button_bullets,
				   FL_BLACK);
	}

	if (current_view->buffer()->isReadonly()) {
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
				     FL_PLACE_MOUSE, FL_FULLBORDER,
				     _("Document Layout"));
		}
	}
}


bool UpdateLayoutQuotes()
{
	bool update = true;
	if (!current_view->available()
	    || current_view->buffer()->isReadonly())
		update = false;
	
	if (update) {
		fl_set_choice(fd_form_quotes->choice_quotes_language,
			      current_view->buffer()->params.quotes_language + 1);
		fl_set_button(fd_form_quotes->radio_single, 0);
		fl_set_button(fd_form_quotes->radio_double, 0);
	
		if (current_view->buffer()->params.quotes_times == InsetQuotes::SingleQ)
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
				     FL_PLACE_MOUSE, FL_FULLBORDER,
				     _("Quotes"));
		}
	}
}
#endif


bool UpdateLayoutPreamble()
{
	bool update = true;
	if (!current_view->available())
		update = false;

	if (update) {
		fl_set_input(fd_form_preamble->input_preamble,
			     current_view->buffer()->params.preamble.c_str());

		if (current_view->buffer()->isReadonly()) {
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
	static int ow = -1, oh;

	if (UpdateLayoutPreamble()) {
		if (fd_form_preamble->form_preamble->visible) {
			fl_raise_form(fd_form_preamble->form_preamble);
		} else {
			fl_show_form(fd_form_preamble->form_preamble,
				     FL_PLACE_MOUSE | FL_FREE_SIZE,
				     FL_FULLBORDER,
				     _("LaTeX Preamble"));
			if (ow < 0) {
				ow = fd_form_preamble->form_preamble->w;
				oh = fd_form_preamble->form_preamble->h;
			}
			fl_set_form_minsize(fd_form_preamble->form_preamble,
					    ow, oh);
		}
	}
}


void MenuLayoutSave()
{
	if (!current_view->available())
		return;

	if (AskQuestion(_("Do you want to save the current settings"),
			_("for Character, Document, Paper and Quotes"),
			_("as default for new documents?")))
		current_view->buffer()->saveParamsAsDefaults();
}


// This is both GUI and LyXFont dependent. Don't know where to put it. (Asger)
// Well, it's mostly GUI dependent, so I guess it will stay here. (Asger)
LyXFont UserFreeFont(BufferParams const & params)
{
	LyXFont font(LyXFont::ALL_IGNORE);

	int pos = fl_get_choice(fd_form_character->choice_family);
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
	case 1: font.setColor(LColor::ignore); break;
	case 2: font.setColor(LColor::none); break;
	case 3: font.setColor(LColor::black); break;
	case 4: font.setColor(LColor::white); break;
	case 5: font.setColor(LColor::red); break;
	case 6: font.setColor(LColor::green); break;
	case 7: font.setColor(LColor::blue); break;
	case 8: font.setColor(LColor::cyan); break;
	case 9: font.setColor(LColor::magenta); break;
	case 10: font.setColor(LColor::yellow); break;
	case 11: font.setColor(LColor::inherit); break;
	}

	int choice = combo_language2->get();
	if (choice == 1)
		font.setLanguage(ignore_language);
	else if (choice == 2)
		font.setLanguage(params.language_info);
	else
		font.setLanguage(&languages[combo_language2->getline()]);

	return font; 
}


/* callbacks for form form_title */
extern "C" void TimerCB(FL_OBJECT *, long)
{
	// only if the form still exists
	if (lyxrc.show_banner && fd_form_title->form_title != 0) {
		if (fd_form_title->form_title->visible) {
			fl_hide_form(fd_form_title->form_title);
		}
		fl_free_form(fd_form_title->form_title);
		fd_form_title->form_title = 0;
	}
}


/* callbacks for form form_paragraph */

extern "C" void ParagraphVSpaceCB(FL_OBJECT * obj, long )
{
	// "Synchronize" the choices and input fields, making it
	// impossible to commit senseless data.

	FD_form_paragraph const * fp = fd_form_paragraph;

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


extern "C" void ParagraphApplyCB(FL_OBJECT *, long)
{
	if (!current_view->available())
		return;
	
	VSpace space_top, space_bottom;
	LyXAlignment align;
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
   
	bool line_top = fl_get_button(fd_form_paragraph->check_lines_top);
	bool line_bottom = fl_get_button(fd_form_paragraph->check_lines_bottom);
	bool pagebreak_top = fl_get_button(fd_form_paragraph->check_pagebreaks_top);
	bool pagebreak_bottom = fl_get_button(fd_form_paragraph->check_pagebreaks_bottom);
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

	LyXText * text = 0;
	if (current_view->the_locking_inset)
	    text = current_view->the_locking_inset->getLyXText(current_view);
	if (!text)
	    text = current_view->text;
	text->SetParagraph(current_view,
			   line_top,
			   line_bottom,
			   pagebreak_top,
			   pagebreak_bottom,
			   space_top,
			   space_bottom,
			   align, 
			   labelwidthstring,
			   noindent);
	current_view->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
	current_view->owner()->getMiniBuffer()->Set(_("Paragraph layout set"));
}


extern "C" void ParagraphCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_paragraph->form_paragraph);
}


extern "C" void ParagraphOKCB(FL_OBJECT *ob, long data)
{
	ParagraphApplyCB(ob, data);
	ParagraphCancelCB(ob, data);
}


/* callbacks for form form_character */

extern "C" void CharacterApplyCB(FL_OBJECT *, long)
{
	// we set toggleall locally here, since it should be true for
	// all other uses of ToggleAndShow() (JMarc)
	toggleall = fl_get_button(fd_form_character->check_toggle_all);
	ToggleAndShow(current_view, UserFreeFont(current_view->buffer()->params));
	current_view->setState();
	toggleall = true;
}


extern "C" void CharacterCloseCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_character->form_character);
}


extern "C" void CharacterOKCB(FL_OBJECT *ob, long data)
{
	CharacterApplyCB(ob, data);
	CharacterCloseCB(ob, data);
}


#ifdef USE_OLD_DOCUMENT_LAYOUT
/* callbacks for form form_document */
void UpdateDocumentButtons(BufferParams const & params) 
{
	fl_set_choice(fd_form_document->choice_pagestyle, 1);

	switch (params.sides) {
	case LyXTextClass::OneSide:
		fl_set_button(fd_form_document->radio_sides_one, 1);
		break;
	case LyXTextClass::TwoSides:
		fl_set_button(fd_form_document->radio_sides_two, 1);
		break;
	}
   
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

extern "C" void ChoiceClassCB(FL_OBJECT * ob, long)
{
	ProhibitInput(current_view);
	if (textclasslist.Load(fl_get_choice(ob)-1)) {
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
			      current_view->buffer()->params.textclass + 1);
	}
	AllowInput(current_view);
}


extern "C" void DocumentDefskipCB(FL_OBJECT * obj, long)
{
	// "Synchronize" the choice and the input field, so that it
	// is impossible to commit senseless data.
	FD_form_document const * fd = fd_form_document;

	if (obj == fd->choice_default_skip) {
		if (fl_get_choice (fd->choice_default_skip) != 4) {
			fl_set_input (fd->input_default_skip, "");
			ActivateDocumentButtons();
		}
	} else if (obj == fd->input_default_skip) {

		char const * input = fl_get_input (fd->input_default_skip);

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


extern "C" void DocumentSpacingCB(FL_OBJECT * obj, long)
{
	// "Synchronize" the choice and the input field, so that it
	// is impossible to commit senseless data.
	FD_form_document const * fd = fd_form_document;

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


extern "C" void DocumentApplyCB(FL_OBJECT *, long)
{
	bool redo = false;
	BufferParams * params = &(current_view->buffer()->params);

	Language const * old_language = params->language_info;
	params->language = combo_language->getline();
	Languages::iterator lit = languages.find(params->language);

	Language const * new_language;
	if (lit != languages.end()) 
		new_language = &(*lit).second;
	else
		new_language = default_language;

	if (current_view->available()) {
		if (old_language != new_language
		    && old_language->RightToLeft() == new_language->RightToLeft()
		    && !current_view->buffer()->isMultiLingual())
			current_view->buffer()->ChangeLanguage(old_language,
							       new_language);
		if (old_language != new_language) {
				//current_view->buffer()->redraw();
			redo = true;
		}
	}
	params->language_info = new_language;

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

        current_view->text->SetCursor(current_view,
				      current_view->text->cursor.par(),
                                      current_view->text->cursor.pos());
        current_view->setState();

	LyXTextClassList::ClassList::size_type new_class =
		fl_get_choice(fd_form_document->choice_class) - 1;

	if (params->textclass != new_class) {
		// try to load new_class
		if (textclasslist.Load(new_class)) {
			// successfully loaded
			redo = true;
			current_view->owner()->getMiniBuffer()->
				Set(_("Converting document to new document class..."));
			CutAndPaste cap;
			int ret = cap.SwitchLayoutsBetweenClasses(
				current_view->buffer()->params.textclass,
				new_class,
				current_view->buffer()->paragraph);

			if (ret) {
				string s;
				if (ret == 1)
					s = _("One paragraph couldn't be converted");
				else {
					s += tostr(ret);
					s += _(" paragraphs couldn't be converted");
				}
				WriteAlert(_("Conversion Errors!"), s,
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
		params->paragraph_separation = BufferParams::PARSEP_INDENT;
	else
		params->paragraph_separation = BufferParams::PARSEP_SKIP;
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
		params->sides = LyXTextClass::TwoSides;
	else
		params->sides = LyXTextClass::OneSide;

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
	
	signed char tmpchar =  
		static_cast<signed char>(fl_get_counter_value(fd_form_document->slider_secnumdepth));
	if (params->secnumdepth != tmpchar)
		redo = true;
	params->secnumdepth = tmpchar;
   
	params->tocdepth =  
		static_cast<int>(fl_get_counter_value(fd_form_document->slider_tocdepth));

	params->float_placement = 
		fl_get_input(fd_form_document->input_float_placement);

	// More checking should be done to ensure the string doesn't have
	// spaces or illegal placement characters in it. (thornley)

	if (redo)
		current_view->redoCurrentBuffer();
   
	current_view->owner()->getMiniBuffer()->Set(_("Document layout set"));
	current_view->buffer()->markDirty();
	
        params->options = 
		fl_get_input(fd_form_document->input_extra);
}


extern "C" void DocumentCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_document->form_document);
}


extern "C" void DocumentOKCB(FL_OBJECT * ob, long data)
{
	DocumentCancelCB(ob, data);
	DocumentApplyCB(ob, data);
}


extern "C" void DocumentBulletsCB(FL_OBJECT *, long)
{
	bulletForm();
	// bullet callbacks etc. in bullet_panel.C -- ARRae
}

/* callbacks for form form_quotes */

extern "C" void QuotesApplyCB(FL_OBJECT *, long)
{
	if (!current_view->available())
		return;
	
	current_view->owner()->getMiniBuffer()->Set(_("Quotes type set"));
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
	current_view->buffer()->params.quotes_language = lga;
	if (fl_get_button(fd_form_quotes->radio_single))   
		current_view->buffer()->
			params.quotes_times = InsetQuotes::SingleQ;
	else
		current_view->buffer()->
			params.quotes_times = InsetQuotes::DoubleQ;
}


extern "C" void QuotesCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_quotes->form_quotes);
}


extern "C" void QuotesOKCB(FL_OBJECT * ob, long data)
{
	QuotesApplyCB(ob, data);
	QuotesCancelCB(ob, data);
}
#else
// this is needed for now!
extern "C" void ChoiceClassCB(FL_OBJECT *, long) {}
extern "C" void DocumentDefskipCB(FL_OBJECT *, long) {}
extern "C" void DocumentSpacingCB(FL_OBJECT *, long) {}
extern "C" void DocumentApplyCB(FL_OBJECT *, long) {}
extern "C" void DocumentCancelCB(FL_OBJECT *, long) {}
extern "C" void DocumentOKCB(FL_OBJECT *, long) {}
extern "C" void DocumentBulletsCB(FL_OBJECT *, long) {}
extern "C" void QuotesApplyCB(FL_OBJECT *, long) {}
extern "C" void QuotesCancelCB(FL_OBJECT *, long) {}
extern "C" void QuotesOKCB(FL_OBJECT *, long) {}
#endif


/* callbacks for form form_preamble */

extern "C" void PreambleCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_preamble->form_preamble);
}


extern "C" void PreambleApplyCB(FL_OBJECT *, long)
{
	if (!current_view->available())
		return;
	
	current_view->buffer()->params.preamble = 
		fl_get_input(fd_form_preamble->input_preamble);
	current_view->buffer()->markDirty();
	current_view->owner()->getMiniBuffer()->Set(_("LaTeX preamble set"));
}

   
extern "C" void PreambleOKCB(FL_OBJECT * ob, long data)
{
	PreambleApplyCB(ob, data);
	PreambleCancelCB(ob, data);
}


/* callbacks for form form_table */

extern "C"
void TableApplyCB(FL_OBJECT *, long)
{
	if (!current_view->available())
		return;

#ifndef NEW_TABULAR
	// check for tables in tables
	if (current_view->text->cursor.par()->table){
		WriteAlert(_("Impossible Operation!"),
			   _("Cannot insert table in table."),
			   _("Sorry."));
		return;
	}
#endif
	current_view->owner()->getMiniBuffer()->Set(_("Inserting table..."));

	int ysize = int(fl_get_slider_value(fd_form_table->slider_columns) + 0.5);
	int xsize = int(fl_get_slider_value(fd_form_table->slider_rows) + 0.5);
   
   
	current_view->hideCursor();
	current_view->beforeChange();
	current_view->update(BufferView::SELECT|BufferView::FITCUR);
   
	current_view->text->SetCursorParUndo(current_view->buffer()); 
	current_view->text->FreezeUndo();

	current_view->text->BreakParagraph(current_view);
	current_view->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
   
	if (current_view->text->cursor.par()->Last()) {
		current_view->text->CursorLeft(current_view);
      
		current_view->text->BreakParagraph(current_view);
		current_view->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
	}

	current_view->text->current_font.setLatex(LyXFont::OFF);
	//if (!fl_get_button(fd_form_table->check_latex)){
	// insert the new wysiwy table
	current_view->text->SetLayout(current_view, 0); // standard layout
#ifndef NEW_INSETS
	if (current_view->text->cursor.par()->footnoteflag == 
	    LyXParagraph::NO_FOOTNOTE) {
#endif
		current_view->text
			->SetParagraph(current_view, 0, 0,
				       0, 0,
				       VSpace (0.3 * current_view->buffer()->
					       params.spacing.getValue(),
					       LyXLength::CM),
				       VSpace (0.3 * current_view->buffer()->
					       params.spacing.getValue(),
					       LyXLength::CM),
				       LYX_ALIGN_CENTER,
				       string(),
				       0);
#ifndef NEW_INSETS
	} else {
		current_view->text
			->SetParagraph(current_view, 0, 0,
				       0, 0,
				       VSpace(VSpace::NONE),
				       VSpace(VSpace::NONE),
				       LYX_ALIGN_CENTER, 
				       string(),
				       0);
	}
#endif
#ifndef NEW_TABULAR
	current_view->text->cursor.par()->table =
		new LyXTable(xsize, ysize);
#endif
	Language const * lang = 
		current_view->text->cursor.par()->getParLanguage(current_view->buffer()->params);
	LyXFont font(LyXFont::ALL_INHERIT, lang);
	for (int i = 0; i < xsize * ysize - 1; ++i) {
		current_view->text->cursor.par()
			->InsertChar(0, LyXParagraph::META_NEWLINE, font);
	}
	current_view->text->RedoParagraph(current_view);
   
	current_view->text->UnFreezeUndo();
     
	current_view->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
	current_view->owner()->getMiniBuffer()->Set(_("Table inserted"));
	current_view->setState();
}


extern "C"
void TableCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_table->form_table);
}


extern "C"
void TableOKCB(FL_OBJECT * ob, long data)
{
	TableApplyCB(ob, data);
	TableCancelCB(ob, data);
}


void Figure()
{
	if (fd_form_figure->form_figure->visible) {
		fl_raise_form(fd_form_figure->form_figure);
	} else {
		fl_show_form(fd_form_figure->form_figure,
			     FL_PLACE_MOUSE, FL_FULLBORDER,
			     _("Insert Figure"));
	}
}


void Table()
{
	if (fd_form_table->form_table->visible) {
		fl_raise_form(fd_form_table->form_table);
	} else {
		fl_show_form(fd_form_table->form_table,
			     FL_PLACE_MOUSE, FL_FULLBORDER,
			     _("Insert Table"));
	}
}


/* callbacks for form form_figure */
extern "C"
void FigureApplyCB(FL_OBJECT *, long)
{
	if (!current_view->available())
		return;

	Buffer * buffer = current_view->buffer();
	if(buffer->isReadonly()) // paranoia
		return;
	
	current_view->owner()->getMiniBuffer()->Set(_("Inserting figure..."));
	if (fl_get_button(fd_form_figure->radio_inline)
#ifndef NEW_TABULAR
	    || current_view->text->cursor.par()->table
#endif
		) {
		InsetFig * new_inset = new InsetFig(100, 20, buffer);
		current_view->insertInset(new_inset);
		current_view->owner()->getMiniBuffer()->Set(_("Figure inserted"));
		new_inset->Edit(current_view, 0, 0, 0);
		return;
	}
	
	current_view->hideCursor();
	current_view->update(BufferView::SELECT|BufferView::FITCUR);
	current_view->beforeChange();
      
	current_view->text->SetCursorParUndo(current_view->buffer()); 
	current_view->text->FreezeUndo();

	current_view->text->BreakParagraph(current_view);
	current_view->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
      
	if (current_view->text->cursor.par()->Last()) {
		current_view->text->CursorLeft(current_view);
	 
		current_view->text->BreakParagraph(current_view);
		current_view->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
	}

	// The standard layout should always be numer 0;
	current_view->text->SetLayout(current_view, 0);

#ifndef NEW_INSETS
	if (current_view->text->cursor.par()->footnoteflag == 
	    LyXParagraph::NO_FOOTNOTE) {
#endif
		current_view->text->
			SetParagraph(current_view, 0, 0,
				     0, 0,
				     VSpace (0.3 * buffer->params.spacing.getValue(),
					     LyXLength::CM),
				     VSpace (0.3 *
					     buffer->params.spacing.getValue(),
					     LyXLength::CM),
				     LYX_ALIGN_CENTER, string(), 0);
#ifndef NEW_INSETS
	} else {
		current_view->text->SetParagraph(current_view, 0, 0,
						 0, 0,
						 VSpace(VSpace::NONE),
						 VSpace(VSpace::NONE),
						 LYX_ALIGN_CENTER, 
						 string(),
						 0);
	}
#endif
	
	current_view->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
      
	Inset * new_inset = new InsetFig(100, 100, buffer);
	current_view->insertInset(new_inset);
	new_inset->Edit(current_view, 0, 0, 0);
	current_view->update(BufferView::SELECT|BufferView::FITCUR);
	current_view->owner()->getMiniBuffer()->Set(_("Figure inserted"));
	current_view->text->UnFreezeUndo();
	current_view->setState();
}


extern "C" void FigureCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_figure->form_figure);
}


extern "C" void FigureOKCB(FL_OBJECT * ob, long data)
{
	FigureApplyCB(ob, data);
	FigureCancelCB(ob, data);
}


// This function runs "configure" and then rereads lyx.defaults to
// reconfigure the automatic settings.
void Reconfigure(BufferView * bv)
{
	bv->owner()->getMiniBuffer()->Set(_("Running configure..."));

	// Run configure in user lyx directory
	Path p(user_lyxdir);
	Systemcalls one(Systemcalls::System, 
			AddName(system_lyxdir, "configure"));
	p.pop();
	bv->owner()->getMiniBuffer()->Set(_("Reloading configuration..."));
	lyxrc.read(LibFileSearch(string(), "lyxrc.defaults"));
	WriteAlert(_("The system has been reconfigured."), 
		   _("You need to restart LyX to make use of any"),
		   _("updated document class specifications."));
}
