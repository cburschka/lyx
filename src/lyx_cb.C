/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *	    Copyright 1995 Matthias Ettrich,
 *          Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include "lyx_cb.h"
#include "lyx_main.h"
#include "buffer.h"
#include "bufferlist.h"
#include "bufferview_funcs.h"
#include "debug.h"
#include "lastfiles.h"
#include "LyXView.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "gettext.h"
#include "BufferView.h"
#include "lyxtextclasslist.h"

#include "insets/insetlabel.h"

#include "frontends/Alert.h"
#include "frontends/FileDialog.h"
#include "frontends/GUIRunTime.h"

#include "support/FileInfo.h"
#include "support/filetools.h"
#include "support/path.h"
#include "support/systemcall.h"
#include "support/lstrings.h"

#include <fstream>
#include <algorithm>
#include <utility>
#include <iostream>

using std::vector;
using std::ifstream;
using std::copy;
using std::endl;
using std::ios;
using std::back_inserter;
using std::istream_iterator;
using std::pair;
using std::make_pair;

extern BufferList bufferlist;
// this should be static, but I need it in buffer.C
bool quitting;	// flag, that we are quitting the program
extern bool finished; // all cleanup done just let it run through now.


void ShowMessage(Buffer const * buf,
		 string const & msg1,
		 string const & msg2,
		 string const & msg3)
{
	if (lyxrc.use_gui
	    && buf && buf->getUser() && buf->getUser()->owner()) {
			string const str = msg1 + ' ' + msg2 + ' ' + msg3;
			buf->getUser()->owner()->message(str);
	} else
		lyxerr << msg1 << msg2 << msg3 << endl;
}


//
// Menu callbacks
//

//
// File menu
//
// should be moved to lyxfunc.C
bool MenuWrite(BufferView * bv, Buffer * buffer)
{
	// FIXME: needed ?
	XFlush(GUIRunTime::x11Display());

	if (!buffer->save()) {
		if (Alert::askQuestion(_("Save failed. Rename and try again?"),
				MakeDisplayPath(buffer->fileName(), 50),
				_("(If not, document is not saved.)"))) {
			return WriteAs(bv, buffer);
		}
		return false;
	} else
		lastfiles->newFile(buffer->fileName());
	return true;
}



// should be moved to BufferView.C
// Half of this func should be in LyXView, the rest in BufferView.
bool WriteAs(BufferView * bv, Buffer * buffer, string const & filename)
{
	string fname = buffer->fileName();
	string const oldname = fname;

	if (filename.empty()) {

		FileDialog fileDlg(bv->owner(),
				   _("Choose a filename to save document as"),
			LFUN_WRITEAS,
			make_pair(string(_("Documents|#o#O")),
				  string(lyxrc.document_path)),
			make_pair(string(_("Templates|#T#t")),
				  string(lyxrc.template_path)));

		if (!IsLyXFilename(fname))
			fname += ".lyx";

		FileDialog::Result result =
			fileDlg.Select(OnlyPath(fname),
				       _("*.lyx|LyX Documents (*.lyx)"),
				       OnlyFilename(fname));

		if (result.first == FileDialog::Later)
			return false;

		fname = result.second;

		if (fname.empty())
			return false;

		// Make sure the absolute filename ends with appropriate suffix
		fname = MakeAbsPath(fname);
		if (!IsLyXFilename(fname))
			fname += ".lyx";
	} else
		fname = filename;

	// Same name as we have already?
	if (!buffer->isUnnamed() && fname == oldname) {
		if (!Alert::askQuestion(_("Same name as document already has:"),
				 MakeDisplayPath(fname, 50),
				 _("Save anyway?")))
			return false;
		// Falls through to name change and save
	}
	// No, but do we have another file with this name open?
	else if (!buffer->isUnnamed() && bufferlist.exists(fname)) {
		if (Alert::askQuestion(_("Another document with same name open!"),
				MakeDisplayPath(fname, 50),
				_("Replace with current document?")))
			{
				bufferlist.close(bufferlist.getBuffer(fname));

				// Ok, change the name of the buffer, but don't save!
				buffer->setFileName(fname);
				buffer->markDirty();

				ShowMessage(buffer, _("Document renamed to '"),
						MakeDisplayPath(fname), _("', but not saved..."));
		}
		return false;
	} // Check whether the file exists
	else {
		FileInfo const myfile(fname);
		if (myfile.isOK() && !Alert::askQuestion(_("Document already exists:"),
						  MakeDisplayPath(fname, 50),
						  _("Replace file?")))
			return false;
	}

	// Ok, change the name of the buffer
	buffer->setFileName(fname);
	buffer->markDirty();
	bool unnamed = buffer->isUnnamed();
	buffer->setUnnamed(false);

	if (!MenuWrite(bv, buffer)) {
	    buffer->setFileName(oldname);
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
		Alert::alert(_("Chktex does not work with SGML derived documents."));
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
			t = _("Use `Navigate->Error' to find it.");
		} else {
			s += tostr(ret);
			s += _(" warnings found.");
			t = _("Use `Navigate->Error' to find them.");
		}
		Alert::alert(_("Chktex run successfully"), s, t);
	} else {
		Alert::alert(_("Error!"), _("It seems chktex does not work."));
	}
	return ret;
}


void QuitLyX()
{
	lyxerr[Debug::INFO] << "Running QuitLyX." << endl;

	if (lyxrc.use_gui) {
		if (!bufferlist.qwriteAll())
			return;

		lastfiles->writeFile(lyxrc.lastfiles);
	}

	// Set a flag that we do quitting from the program,
	// so no refreshes are necessary.
	quitting = true;

	// close buffers first
	bufferlist.closeAll();

	// do any other cleanup procedures now
	lyxerr[Debug::INFO] << "Deleting tmp dir " << system_tempdir << endl;

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

	bv->owner()->message(_("Autosaving current document..."));

	// create autosave filename
	string fname =	bv->buffer()->filePath();
	fname += "#";
	fname += OnlyFilename(bv->buffer()->fileName());
	fname += "#";

	// tmp_ret will be located (usually) in /tmp
	// will that be a problem?
	pid_t const pid = fork(); // If you want to debug the autosave
	// you should set pid to -1, and comment out the
	// fork.
	if (pid == 0 || pid == -1) {
		// pid = -1 signifies that lyx was unable
		// to fork. But we will do the save
		// anyway.
		bool failed = false;

		string const tmp_ret = lyx::tempName(string(), "lyxauto");
		if (!tmp_ret.empty()) {
			bv->buffer()->writeFile(tmp_ret, 1);
			// assume successful write of tmp_ret
			if (!lyx::rename(tmp_ret, fname)) {
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
					bv->owner()->message(_("Autosave failed!"));
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
	lyxerr[Debug::INFO] << "Arg is " << filename
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
	if (!bv->available())
		return;

	string const tmpstr = getContentsOfAsciiFile(bv, f, asParagraph);
	if (tmpstr.empty())
		return;

	// insert the string
	bv->hideCursor();

	// clear the selection
	bool flag = (bv->text == bv->getLyXText());
	if (flag)
		bv->beforeChange(bv->text);
	if (!asParagraph)
		bv->getLyXText()->insertStringAsLines(bv, tmpstr);
	else
		bv->getLyXText()->insertStringAsParagraphs(bv, tmpstr);
	if (flag)
		bv->update(bv->text,
			   BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
}


// Insert ascii file (if filename is empty, prompt for one)
string getContentsOfAsciiFile(BufferView * bv, string const & f, bool asParagraph)
{
	string fname = f;

	if (fname.empty()) {
		FileDialog fileDlg(bv->owner(), _("Select file to insert"),
			(asParagraph) ? LFUN_FILE_INSERT_ASCII_PARA : LFUN_FILE_INSERT_ASCII);

		FileDialog::Result result = fileDlg.Select(bv->owner()->buffer()->filePath());

		if (result.first == FileDialog::Later)
			return string();

		fname = result.second;

		if (fname.empty())
			return string();
	}

	FileInfo fi(fname);

	if (!fi.readable()) {
		Alert::err_alert(_("Error! Specified file is unreadable: "),
			     MakeDisplayPath(fname, 50));
		return string();
	}

	ifstream ifs(fname.c_str());
	if (!ifs) {
		Alert::err_alert(_("Error! Cannot open specified file: "),
			     MakeDisplayPath(fname, 50));
		return string();
	}

	ifs.unsetf(ios::skipws);
	istream_iterator<char> ii(ifs);
	istream_iterator<char> end;
#if !defined(USE_INCLUDED_STRING) && !defined(STD_STRING_IS_GOOD)
	// We use this until the compilers get better...
	vector<char> tmp;
	copy(ii, end, back_inserter(tmp));
	string const tmpstr(tmp.begin(), tmp.end());
#else
	// This is what we want to use and what we will use once the
	// compilers get good enough.
	//string tmpstr(ii, end); // yet a reason for using std::string
	// alternate approach to get the file into a string:
	string tmpstr;
	copy(ii, end, back_inserter(tmpstr));
#endif

	return tmpstr;
}


void MenuInsertLabel(BufferView * bv, string const & arg)
{
	string label(arg);
	bv->owner()->prohibitInput();
	if (label.empty()) {
		Paragraph * par = bv->getLyXText()->cursor.par();
		LyXTextClass const & tclass =
			textclasslist[bv->buffer()->params.textclass];
		LyXLayout const * layout = &tclass[par->layout()];

		if (layout->latextype == LATEX_PARAGRAPH && par->previous()) {
			Paragraph * par2 = par->previous();
			LyXLayout const * layout2 = &tclass[par2->layout()];
			if (layout2->latextype != LATEX_PARAGRAPH) {
				par = par2;
				layout = layout2;
			}
		}
		string text = layout->latexname().substr(0, 3);
		if (layout->latexname() == "theorem")
			text = "thm"; // Create a correct prefix for prettyref

		text += ":";
		if (layout->latextype == LATEX_PARAGRAPH ||
		    lyxrc.label_init_length < 0)
			text.erase();
		string par_text = par->asString(bv->buffer(), false);
		for (int i = 0; i < lyxrc.label_init_length; ++i) {
			if (par_text.empty())
				break;
			string head;
			par_text = split(par_text, head, ' ');
			if (i > 0)
				text += '-'; // Is it legal to use spaces in
					     // labels ?
			text += head;
		}

		pair<bool, string> result =
			Alert::askForText(_("Enter new label to insert:"), text);
		if (result.first) {
			label = frontStrip(strip(result.second));
		}
	}
	if (!label.empty()) {
		InsetCommandParams p("label", label);
		InsetLabel * inset = new InsetLabel(p);
		bv->insertInset(inset);
	}
	bv->owner()->allowInput();
}


// This function runs "configure" and then rereads lyx.defaults to
// reconfigure the automatic settings.
void Reconfigure(BufferView * bv)
{
	bv->owner()->message(_("Running configure..."));

	// Run configure in user lyx directory
	Path p(user_lyxdir);
	Systemcall one;
	one.startscript(Systemcall::Wait,
			AddName(system_lyxdir, "configure"));
	p.pop();
	bv->owner()->message(_("Reloading configuration..."));
	lyxrc.read(LibFileSearch(string(), "lyxrc.defaults"));
	Alert::alert(_("The system has been reconfigured."),
		   _("You need to restart LyX to make use of any"),
		   _("updated document class specifications."));
}
