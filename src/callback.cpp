/**
 * \file callback.cpp
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

#include "callback.h"

#include "Buffer.h"
#include "BufferList.h"
#include "BufferView.h"
#include "buffer_funcs.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "gettext.h"
#include "Session.h"
#include "LaTeXFeatures.h"
#include "LyX.h"
#include "Layout.h"
#include "LyXRC.h"
#include "Text.h"
#include "Undo.h"

#include "frontends/alert.h"
#include "frontends/Application.h"
#include "frontends/FileDialog.h"
#include "frontends/LyXView.h"

#include "support/FileFilterList.h"
#include "support/filetools.h"
#include "support/Forkedcall.h"
#include "support/fs_extras.h"
#include "support/lyxlib.h"
#include "support/Package.h"
#include "support/Path.h"
#include "support/Systemcall.h"

#if !defined (HAVE_FORK)
# define fork() -1
#endif

#include <boost/shared_ptr.hpp>
#include <boost/filesystem/operations.hpp>

#include <cerrno>
#include <fstream>

using std::back_inserter;
using std::copy;
using std::endl;
using std::make_pair;
using std::string;
using std::ifstream;
using std::ios;
using std::istream_iterator;

using boost::shared_ptr;


namespace lyx {

using support::bformat;
using support::FileFilterList;
using support::FileName;
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

namespace Alert = frontend::Alert;
namespace fs = boost::filesystem;

// this should be static, but I need it in Buffer.cpp
bool quitting;	// flag, that we are quitting the program

//
// Menu callbacks
//

bool menuWrite(Buffer * buffer)
{
	if (buffer->save()) {
		LyX::ref().session().lastFiles().add(FileName(buffer->fileName()));
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



/** Write a buffer to a new file name and rename the buffer
    according to the new file name.

    This function is e.g. used by menu callbacks and
    LFUN_BUFFER_WRITE_AS.

    If 'newname' is empty (the default), the user is asked via a
    dialog for the buffer's new name and location.

    If 'newname' is non-empty and has an absolute path, that is used.
    Otherwise the base directory of the buffer is used as the base
    for any relative path in 'newname'.
*/

bool writeAs(Buffer * buffer, string const & newname)
{
	string fname = buffer->fileName();
	string const oldname = fname;

	if (newname.empty()) {	/// No argument? Ask user through dialog

		// FIXME UNICODE
		FileDialog fileDlg(_("Choose a filename to save document as"),
				   LFUN_BUFFER_WRITE_AS,
				   make_pair(_("Documents|#o#O"), 
					     from_utf8(lyxrc.document_path)),
				   make_pair(_("Templates|#T#t"), 
					     from_utf8(lyxrc.template_path)));

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
		fname = makeAbsPath(fname).absFilename();
		if (!isLyXFilename(fname))
			fname += ".lyx";

	} else 
		fname = makeAbsPath(newname, onlyPath(oldname)).absFilename();

	if (fs::exists(FileName(fname).toFilesystemEncoding())) {
		docstring const file = makeDisplayPath(fname, 30);
		docstring text = bformat(_("The document %1$s already "
					   "exists.\n\nDo you want to "
					   "overwrite that document?"), 
					 file);
		int const ret = Alert::prompt(_("Overwrite document?"),
			text, 0, 1, _("&Overwrite"), _("&Cancel"));

		if (ret == 1)
			return false;
	}

	// Ok, change the name of the buffer
	buffer->setFileName(fname);
	buffer->markDirty();
	bool unnamed = buffer->isUnnamed();
	buffer->setUnnamed(false);
	buffer->saveCheckSum(fname);

	if (!menuWrite(buffer)) {
		buffer->setFileName(oldname);
		buffer->setUnnamed(unnamed);
		buffer->saveCheckSum(oldname);
		return false;
	}

	removeAutosaveFile(oldname);
	return true;
}


namespace {

class AutoSaveBuffer : public ForkedProcess {
public:
	///
	AutoSaveBuffer(BufferView & bv, FileName const & fname)
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
	FileName fname_;
};


int AutoSaveBuffer::start()
{
	command_ = to_utf8(bformat(_("Auto-saving %1$s"), from_utf8(fname_.absFilename())));
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

		FileName const tmp_ret(tempName(FileName(), "lyxauto"));
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

	AutoSaveBuffer autosave(*bv, FileName(fname));
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
	LYXERR(Debug::INFO) << "Arg is " << filename
			    << "\nName is " << name
			    << "\nTemplate is " << tmpname << endl;

	Buffer * const b = newFile(name, tmpname);
	if (b)
		bv->setBuffer(b);
}


// Insert plain text file (if filename is empty, prompt for one)
void insertPlaintextFile(BufferView * bv, string const & f, bool asParagraph)
{
	if (!bv->buffer())
		return;

	docstring const tmpstr = getContentsOfPlaintextFile(bv, f, asParagraph);
	if (tmpstr.empty())
		return;

	Cursor & cur = bv->cursor();
	cap::replaceSelection(cur);
	recordUndo(cur);
	if (asParagraph)
		cur.innerText()->insertStringAsParagraphs(cur, tmpstr);
	else
		cur.innerText()->insertStringAsLines(cur, tmpstr);
}


docstring const getContentsOfPlaintextFile(BufferView * bv, string const & f,
		bool asParagraph)
{
	FileName fname(f);

	if (fname.empty()) {
		FileDialog fileDlg(_("Select file to insert"),
			(asParagraph) ? LFUN_FILE_INSERT_PLAINTEXT_PARA : LFUN_FILE_INSERT_PLAINTEXT);

		FileDialog::Result result =
			fileDlg.open(from_utf8(bv->buffer()->filePath()),
				     FileFilterList(), docstring());

		if (result.first == FileDialog::Later)
			return docstring();

		fname = makeAbsPath(to_utf8(result.second));

		if (fname.empty())
			return docstring();
	}

	if (!fs::is_readable(fname.toFilesystemEncoding())) {
		docstring const error = from_ascii(strerror(errno));
		docstring const file = makeDisplayPath(fname.absFilename(), 50);
		docstring const text = bformat(_("Could not read the specified document\n"
							   "%1$s\ndue to the error: %2$s"), file, error);
		Alert::error(_("Could not read file"), text);
		return docstring();
	}

	ifstream ifs(fname.toFilesystemEncoding().c_str());
	if (!ifs) {
		docstring const error = from_ascii(strerror(errno));
		docstring const file = makeDisplayPath(fname.absFilename(), 50);
		docstring const text = bformat(_("Could not open the specified document\n"
							   "%1$s\ndue to the error: %2$s"), file, error);
		Alert::error(_("Could not open file"), text);
		return docstring();
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

	// FIXME UNICODE: We don't know the encoding of the file
	docstring file_content = from_utf8(tmpstr);
	if (file_content.empty()) {
		Alert::error(_("Reading not UTF-8 encoded file"),
					_("The file is not UTF-8 encoded.\n"
					"It will be read as local 8Bit-encoded.\n"
					"If this does not give the correct result\n"
					"then please change the encoding of the file\n"
					"to UTF-8 with a program other than LyX.\n"));
		file_content = from_local8bit(tmpstr);
	}

	return normalize_c(file_content);
}


// This function runs "configure" and then rereads lyx.defaults to
// reconfigure the automatic settings.
void reconfigure(LyXView & lv, string const & option)
{
	// emit message signal.
	lv.message(_("Running configure..."));

	// Run configure in user lyx directory
	support::Path p(package().user_support());
	string configure_command = package().configure_command();
	configure_command += option;
	Systemcall one;
	int ret = one.startscript(Systemcall::Wait, configure_command);
	p.pop();
	// emit message signal.
	lv.message(_("Reloading configuration..."));
	lyxrc.read(libFileSearch(string(), "lyxrc.defaults"));
	// Re-read packages.lst
	LaTeXFeatures::getAvailable();

	if (ret)
		Alert::information(_("System reconfiguration failed"),
			   _("The system reconfiguration has failed.\n"
					  "Default textclass is used but LyX may not "
					  "be able to work properly.\n"
					  "Please reconfigure again if needed."));
	else
		Alert::information(_("System reconfigured"),
			   _("The system has been reconfigured.\n"
					  "You need to restart LyX to make use of any\n"
					  "updated document class specifications."));
}


} // namespace lyx
