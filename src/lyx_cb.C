/**
 * \file lyx_cb.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyx_cb.h"

#include "buffer.h"
#include "bufferlist.h"
#include "BufferView.h"
#include "buffer_funcs.h"
#include "cursor.h"
#include "debug.h"
#include "gettext.h"
#include "session.h"
#include "LaTeXFeatures.h"
#include "lyx_main.h"
#include "lyxlayout.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "paragraph.h"

#include "frontends/Alert.h"
#include "frontends/Application.h"
#include "frontends/FileDialog.h"

#include "support/filefilterlist.h"
#include "support/filetools.h"
#include "support/fontutils.h"
#include "support/forkedcall.h"
#include "support/fs_extras.h"
#include "support/lyxlib.h"
#include "support/package.h"
#include "support/path.h"
#include "support/systemcall.h"

#if !defined (HAVE_FORK)
# define fork() -1
#endif

#include <boost/shared_ptr.hpp>
#include <boost/filesystem/operations.hpp>

#include <cerrno>
#include <fstream>


namespace lyx {

using support::addName;
using support::bformat;
using support::FileFilterList;
using support::ForkedProcess;
using support::isLyXFilename;
using support::libFileSearch;
using support::makeAbsPath;
using support::makeDisplayPath;
using support::onlyFilename;
using support::onlyPath;
using support::package;
using support::removeAutosaveFile;
using support::rename;
using support::split;
using support::Systemcall;
using support::tempName;
using support::unlink;

using boost::shared_ptr;

namespace Alert = frontend::Alert;
namespace fs = boost::filesystem;

using std::back_inserter;
using std::copy;
using std::endl;
using std::make_pair;
using std::string;
using std::ifstream;
using std::ios;
using std::istream_iterator;


// this should be static, but I need it in buffer.C
bool quitting;	// flag, that we are quitting the program

//
// Menu callbacks
//

bool menuWrite(Buffer * buffer)
{
	if (buffer->save()) {
		LyX::ref().session().LastFiles().add(buffer->fileName());
		return true;
	}

	// FIXME: we don't tell the user *WHY* the save failed !!

	docstring const file = makeDisplayPath(buffer->fileName(), 30);

	docstring text = bformat(_("The document %1$s could not be saved.\n\n"
					     "Do you want to rename the document and try again?"), file);
	int const ret = Alert::prompt(_("Rename and save?"),
		text, 0, 1, _("&Rename"), _("&Cancel"));

	if (ret == 0)
		return writeAs(buffer);
	return false;
}



bool writeAs(Buffer * buffer, string const & filename)
{
	string fname = buffer->fileName();
	string const oldname = fname;

	if (filename.empty()) {

		// FIXME UNICODE
		FileDialog fileDlg(_("Choose a filename to save document as"),
			LFUN_BUFFER_WRITE_AS,
			make_pair(_("Documents|#o#O"), from_utf8(lyxrc.document_path)),
			make_pair(_("Templates|#T#t"), from_utf8(lyxrc.template_path)));

		if (!isLyXFilename(fname))
			fname += ".lyx";

		FileFilterList const filter (_("LyX Documents (*.lyx)"));

		FileDialog::Result result =
			fileDlg.save(from_utf8(onlyPath(fname)),
				     filter,
				     from_utf8(onlyFilename(fname)));

		if (result.first == FileDialog::Later)
			return false;

		fname = to_utf8(result.second);

		if (fname.empty())
			return false;

		// Make sure the absolute filename ends with appropriate suffix
		fname = makeAbsPath(fname);
		if (!isLyXFilename(fname))
			fname += ".lyx";
	} else
		fname = filename;

	if (fs::exists(fname)) {
		docstring const file = makeDisplayPath(fname, 30);
		docstring text = bformat(_("The document %1$s already exists.\n\n"
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

	if (!menuWrite(buffer)) {
		buffer->setFileName(oldname);
		buffer->setUnnamed(unnamed);
		return false;
	}

	removeAutosaveFile(oldname);
	return true;
}


namespace {

class AutoSaveBuffer : public ForkedProcess {
public:
	///
	AutoSaveBuffer(BufferView & bv, string const & fname)
		: bv_(bv), fname_(fname) {}
	///
	virtual shared_ptr<ForkedProcess> clone() const
	{
		return shared_ptr<ForkedProcess>(new AutoSaveBuffer(*this));
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
	command_ = to_utf8(bformat(_("Auto-saving %1$s"), from_utf8(fname_)));
	return run(DontWait);
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
					// emit message signal.
					bv_.buffer()->message(_("Autosave failed!"));
			}
		}
		if (pid == 0) { // we are the child so...
			_exit(0);
		}
	}
	return pid;
}

} // namespace anon


void autoSave(BufferView * bv)
	// should probably be moved into BufferList (Lgb)
	// Perfect target for a thread...
{
	if (!bv->buffer())
		return;

	if (bv->buffer()->isBakClean() || bv->buffer()->isReadonly()) {
		// We don't save now, but we'll try again later
		bv->buffer()->resetAutosaveTimers();
		return;
	}

	// emit message signal.
	bv->buffer()->message(_("Autosaving current document..."));

	// create autosave filename
	string fname = bv->buffer()->filePath();
	fname += '#';
	fname += onlyFilename(bv->buffer()->fileName());
	fname += '#';

	AutoSaveBuffer autosave(*bv, fname);
	autosave.start();

	bv->buffer()->markBakClean();
	bv->buffer()->resetAutosaveTimers();
}


//
// Copyright CHT Software Service GmbH
// Uwe C. Schroeder
//
// create new file with template
// SERVERCMD !
//
void newFile(BufferView * bv, string const & filename)
{
	// Split argument by :
	string name;
	string tmpname = split(filename, name, ':');
	lyxerr[Debug::INFO] << "Arg is " << filename
			    << "\nName is " << name
			    << "\nTemplate is " << tmpname << endl;

	Buffer * const b = newFile(name, tmpname);
	if (b)
		bv->setBuffer(b);
}


// Insert ascii file (if filename is empty, prompt for one)
void insertAsciiFile(BufferView * bv, string const & f, bool asParagraph)
{
	if (!bv->buffer())
		return;

	// FIXME: We don't know the encoding of the file
	docstring const tmpstr = from_utf8(getContentsOfAsciiFile(bv, f, asParagraph));
	if (tmpstr.empty())
		return;

	// clear the selection
	LyXText const & text = bv->buffer()->text();
	if (&text == bv->getLyXText())
		bv->cursor().clearSelection();
	if (asParagraph)
		bv->getLyXText()->insertStringAsParagraphs(bv->cursor(), tmpstr);
	else
		bv->getLyXText()->insertStringAsLines(bv->cursor(), tmpstr);
	bv->update();
}


// Insert ascii file (if filename is empty, prompt for one)
string getContentsOfAsciiFile(BufferView * bv, string const & f, bool asParagraph)
{
	string fname = f;

	if (fname.empty()) {
		FileDialog fileDlg(_("Select file to insert"),
			(asParagraph) ? LFUN_FILE_INSERT_ASCII_PARA : LFUN_FILE_INSERT_ASCII);

		FileDialog::Result result =
			fileDlg.open(from_utf8(bv->buffer()->filePath()),
				     FileFilterList(), docstring());

		if (result.first == FileDialog::Later)
			return string();

		fname = to_utf8(result.second);

		if (fname.empty())
			return string();
	}

	if (!fs::is_readable(fname)) {
		docstring const error = from_ascii(strerror(errno));
		docstring const file = makeDisplayPath(fname, 50);
		docstring const text = bformat(_("Could not read the specified document\n"
							   "%1$s\ndue to the error: %2$s"), file, error);
		Alert::error(_("Could not read file"), text);
		return string();
	}

	ifstream ifs(fname.c_str());
	if (!ifs) {
		docstring const error = from_ascii(strerror(errno));
		docstring const file = makeDisplayPath(fname, 50);
		docstring const text = bformat(_("Could not open the specified document\n"
							   "%1$s\ndue to the error: %2$s"), file, error);
		Alert::error(_("Could not open file"), text);
		return string();
	}

	ifs.unsetf(ios::skipws);
	istream_iterator<char> ii(ifs);
	istream_iterator<char> end;
#if !defined(USE_INCLUDED_STRING) && !defined(STD_STRING_IS_GOOD)
	// We use this until the compilers get better...
	std::vector<char> tmp;
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


// This function runs "configure" and then rereads lyx.defaults to
// reconfigure the automatic settings.
void reconfigure(BufferView * bv)
{
	// emit message signal.
	bv->buffer()->message(_("Running configure..."));

	// Run configure in user lyx directory
	support::Path p(package().user_support());
	string const configure_command = package().configure_command();
	Systemcall one;
	one.startscript(Systemcall::Wait, configure_command);
	p.pop();
	// emit message signal.
	bv->buffer()->message(_("Reloading configuration..."));
	lyxrc.read(libFileSearch(string(), "lyxrc.defaults"));
	// Re-read packages.lst
	LaTeXFeatures::getAvailable();

	Alert::information(_("System reconfigured"),
			   _("The system has been reconfigured.\n"
					  "You need to restart LyX to make use of any\n"
					  "updated document class specifications."));
}


} // namespace lyx
