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

#include FORMS_H_LOCATION
#include "lyx.h"
#include "layout_forms.h"
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
#include "exporter.h"

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
extern FD_form_character * fd_form_character;
extern FD_form_preamble * fd_form_preamble;
extern FD_form_figure * fd_form_figure;

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
void ToggleLockedInsetCursor(int x, int y, int asc, int desc);
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


void ShowMessage(Buffer const * buf,
		 string const & msg1,
		 string const & msg2,
		 string const & msg3, int delay)
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


void MenuFax(Buffer * buffer)
{
	// Generate postscript file
	if (!Exporter::Export(buffer, "ps", true))
		return;

	// Send fax
	string const ps = OnlyFilename(ChangeExtension(buffer->fileName(), 
						       ".ps"));

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
			if (lyx::rename(tmp_ret, fname)) {
				failed = true;
				// most likely couldn't move between filesystems
				// unless write of tmp_ret failed
				// so remove tmp file (if it exists)
				lyx::unlink(tmp_ret);
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
	bv->hideCursor();
	
	// clear the selection
	bv->beforeChange();
	if (!asParagraph)
		bv->text->InsertStringA(bv, tmpstr);
	else
		bv->text->InsertStringB(bv, tmpstr);
	bv->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
}


void MenuInsertLabel(string const & arg)
{
	string label(arg);
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


// This is _only_ used in Toolbar_pimpl.C, move it there and get rid of
// current_view. (Lgb)
void LayoutsCB(int sel, void *, Combox *)
{
	string tmp = tostr(sel);
	current_view->owner()->getLyXFunc()->Dispatch(LFUN_LAYOUTNO,
						      tmp);
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
LyXFont const UserFreeFont(BufferParams const & params)
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
		font.setLanguage(params.language);
	else
		font.setLanguage(languages.getLanguage(combo_language2->getline()));

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
	if (fl_get_button(fd_form_figure->radio_inline)) {
		InsetFig * new_inset = new InsetFig(100, 20, *buffer);
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
      
	Inset * new_inset = new InsetFig(100, 100, *buffer);
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
