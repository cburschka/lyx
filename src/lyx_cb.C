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
#include "buffer_funcs.h"
#include "bufferlist.h"
#include "bufferview_funcs.h"
#include "debug.h"
#include "lastfiles.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "gettext.h"
#include "BufferView.h"
#include "Lsstream.h"

#include "insets/insetlabel.h"

#include "frontends/lyx_gui.h"
#include "frontends/LyXView.h"
#include "frontends/Alert.h"
#include "frontends/FileDialog.h"

#include "support/FileInfo.h"
#include "support/filetools.h"
#include "support/forkedcall.h"
#include "support/path.h"
#include "support/systemcall.h"
#include "support/lstrings.h"

#include <fstream>
#include <algorithm>
#include <utility>
#include <cerrno>

using namespace lyx::support;

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


//
// Menu callbacks
//

bool MenuWrite(Buffer * buffer)
{
	if (buffer->save()) {
		lastfiles->newFile(buffer->fileName());
		return true;
	}

	// FIXME: we don't tell the user *WHY* the save failed !!

	string const file = MakeDisplayPath(buffer->fileName(), 30);

	string text = bformat(_("The document %1$s could not be saved.\n\n"
		"Do you want to rename the document and try again?"), file);
	int const ret = Alert::prompt(_("Rename and save?"),
		text, 0, 1, _("&Rename"), _("&Cancel"));

	if (ret == 0)
		return WriteAs(buffer);
	return false;
}



bool WriteAs(Buffer * buffer, string const & filename)
{
	string fname = buffer->fileName();
	string const oldname = fname;

	if (filename.empty()) {

		FileDialog fileDlg(_("Choose a filename to save document as"),
			LFUN_WRITEAS,
			make_pair(string(_("Documents|#o#O")),
				  string(lyxrc.document_path)),
			make_pair(string(_("Templates|#T#t")),
				  string(lyxrc.template_path)));

		if (!IsLyXFilename(fname))
			fname += ".lyx";

		FileDialog::Result result =
			fileDlg.save(OnlyPath(fname),
				       _("*.lyx| LyX Documents (*.lyx)"),
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

	FileInfo const myfile(fname);
	if (myfile.isOK()) {
		string const file = MakeDisplayPath(fname, 30);
		string text = bformat(_("The document %1$s already exists.\n\n"
			"Do you want to over-write that document?"), file);
		int const ret = Alert::prompt(_("Over-write document?"),
			text, 0, 1, _("&Over-write"), _("&Cancel"));

		if (ret == 1)
			return false;
	}

	// Ok, change the name of the buffer
	buffer->setFileName(fname);
	buffer->markDirty();
	bool unnamed = buffer->isUnnamed();
	buffer->setUnnamed(false);

	if (!MenuWrite(buffer)) {
		buffer->setFileName(oldname);
		buffer->setUnnamed(unnamed);
		return false;
	}

	removeAutosaveFile(oldname);
	return true;
}


void QuitLyX()
{
	lyxerr[Debug::INFO] << "Running QuitLyX." << endl;

	if (lyx_gui::use_gui) {
		if (!bufferlist.quitWriteAll())
			return;

		lastfiles->writeFile(lyxrc.lastfiles);
	}

	// Set a flag that we do quitting from the program,
	// so no refreshes are necessary.
	quitting = true;

	// close buffers first
	bufferlist.closeAll();

	// do any other cleanup procedures now
	lyxerr[Debug::INFO] << "Deleting tmp dir " << os::getTmpDir() << endl;

	if (destroyDir(os::getTmpDir()) != 0) {
		string msg = bformat(_("Could not remove the temporary directory %1$s"),
			os::getTmpDir());
		Alert::warning(_("Could not remove temporary directory"), msg);
	}

	lyx_gui::exit();
}


namespace {

class AutoSaveBuffer : public ForkedProcess {
public:
	///
	AutoSaveBuffer(BufferView & bv, string const & fname)
		: bv_(bv), fname_(fname) {}
	///
	virtual ForkedProcess * clone() const {
		return new AutoSaveBuffer(*this);
	}
	///
	int start();
private:
	///
	virtual int generateChild();
	///
	BufferView & bv_;
	string fname_;
};


int AutoSaveBuffer::start()
{
	command_ = bformat(_("Auto-saving %1$s"), fname_);
	return runNonBlocking();
}


int AutoSaveBuffer::generateChild()
{
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

		string const tmp_ret = tempName(string(), "lyxauto");
		if (!tmp_ret.empty()) {
			bv_.buffer()->writeFile(tmp_ret);
			// assume successful write of tmp_ret
			if (!rename(tmp_ret, fname_)) {
				failed = true;
				// most likely couldn't move between filesystems
				// unless write of tmp_ret failed
				// so remove tmp file (if it exists)
				unlink(tmp_ret);
			}
		} else {
			failed = true;
		}

		if (failed) {
			// failed to write/rename tmp_ret so try writing direct
			if (!bv_.buffer()->writeFile(fname_)) {
				// It is dangerous to do this in the child,
				// but safe in the parent, so...
				if (pid == -1)
					bv_.owner()->message(_("Autosave failed!"));
			}
		}
		if (pid == 0) { // we are the child so...
			_exit(0);
		}
	}
	return pid;
}

} // namespace anon


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
	string fname = bv->buffer()->filePath();
	fname += '#';
	fname += OnlyFilename(bv->buffer()->fileName());
	fname += '#';

	AutoSaveBuffer autosave(*bv, fname);
	autosave.start();

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
void NewFile(BufferView * bv, string const & filename)
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

	bv->newFile(name, tmpname);
}


// Insert ascii file (if filename is empty, prompt for one)
void InsertAsciiFile(BufferView * bv, string const & f, bool asParagraph)
{
	if (!bv->available())
		return;

	string const tmpstr = getContentsOfAsciiFile(bv, f, asParagraph);
	if (tmpstr.empty())
		return;

	// clear the selection
	bool flag = (bv->text == bv->getLyXText());
	if (flag)
		bv->beforeChange(bv->text);
	if (!asParagraph)
		bv->getLyXText()->insertStringAsLines(tmpstr);
	else
		bv->getLyXText()->insertStringAsParagraphs(tmpstr);
	if (flag)
		bv->update(BufferView::SELECT);
}


// Insert ascii file (if filename is empty, prompt for one)
string getContentsOfAsciiFile(BufferView * bv, string const & f, bool asParagraph)
{
	string fname = f;

	if (fname.empty()) {
		FileDialog fileDlg(_("Select file to insert"),
			(asParagraph) ? LFUN_FILE_INSERT_ASCII_PARA : LFUN_FILE_INSERT_ASCII);

		FileDialog::Result result = fileDlg.open(bv->owner()->buffer()->filePath());

		if (result.first == FileDialog::Later)
			return string();

		fname = result.second;

		if (fname.empty())
			return string();
	}

	FileInfo fi(fname);

	if (!fi.readable()) {
		string const error = strerror(errno);
		string const file = MakeDisplayPath(fname, 50);
		string const text = bformat(_("Could not read the specified document\n"
			"%1$s\ndue to the error: %2$s"), file, error);
		Alert::error(_("Could not read file"), text);
		return string();
	}

	ifstream ifs(fname.c_str());
	if (!ifs) {
		string const error = strerror(errno);
		string const file = MakeDisplayPath(fname, 50);
		string const text = bformat(_("Could not open the specified document\n"
			"%1$s\ndue to the error: %2$s"), file, error);
		Alert::error(_("Could not open file"), text);
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


string const getPossibleLabel(BufferView const & bv)
{
	ParagraphList::iterator pit = bv.getLyXText()->cursor.par();
	ParagraphList & plist = bv.getLyXText()->ownerParagraphs();

	LyXLayout_ptr layout = pit->layout();

	if (layout->latextype == LATEX_PARAGRAPH && pit != plist.begin()) {
		ParagraphList::iterator pit2 = boost::prior(pit);

		LyXLayout_ptr const & layout2 = pit2->layout();

		if (layout2->latextype != LATEX_PARAGRAPH) {
			pit = pit2;
			layout = layout2;
		}
	}

	string text = layout->latexname().substr(0, 3);
	if (layout->latexname() == "theorem")
		text = "thm"; // Create a correct prefix for prettyref

	text += ':';
	if (layout->latextype == LATEX_PARAGRAPH ||
	    lyxrc.label_init_length < 0)
		text.erase();

	string par_text = pit->asString(bv.buffer(), false);
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

	return text;
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

	Alert::information(_("System reconfigured"),
		_("The system has been reconfigured.\n"
		"You need to restart LyX to make use of any \n"
		"updated document class specifications."));
}
