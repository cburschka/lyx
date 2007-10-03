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
namespace fs = boost::filesystem;

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
using frontend::LyXView;

namespace Alert = frontend::Alert;

//
// Copyright CHT Software Service GmbH
// Uwe C. Schroeder
//
// create new file with template
// SERVERCMD !
//
void newFile(LyXView & lv, string const & filename)
{
	// Split argument by :
	string name;
	string tmpname = split(filename, name, ':');
	LYXERR(Debug::INFO) << "Arg is " << filename
			    << "\nName is " << name
			    << "\nTemplate is " << tmpname << endl;

	Buffer * const b = newFile(name, tmpname);
	if (b)
		lv.setBuffer(b);
}


// Insert plain text file (if filename is empty, prompt for one)
void insertPlaintextFile(BufferView * bv, string const & f, bool asParagraph)
{
	docstring const tmpstr =
	  getContentsOfPlaintextFile(bv, f, asParagraph);

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
				   ( (asParagraph)
				     ? LFUN_FILE_INSERT_PLAINTEXT_PARA 
				     : LFUN_FILE_INSERT_PLAINTEXT) );

		FileDialog::Result result =
			fileDlg.open(from_utf8(bv->buffer().filePath()),
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
		docstring const text =
		  bformat(_("Could not read the specified document\n"
			    "%1$s\ndue to the error: %2$s"), file, error);
		Alert::error(_("Could not read file"), text);
		return docstring();
	}

	ifstream ifs(fname.toFilesystemEncoding().c_str());
	if (!ifs) {
		docstring const error = from_ascii(strerror(errno));
		docstring const file = makeDisplayPath(fname.absFilename(), 50);
		docstring const text =
		  bformat(_("Could not open the specified document\n"
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
	one.startscript(Systemcall::Wait, configure_command);
	p.pop();
	// emit message signal.
	lv.message(_("Reloading configuration..."));
	lyxrc.read(libFileSearch(string(), "lyxrc.defaults"));
	// Re-read packages.lst
	LaTeXFeatures::getAvailable();

	Alert::information(_("System reconfigured"),
			   _("The system has been reconfigured.\n"
			     "You need to restart LyX to make use of any\n"
			     "updated document class specifications."));
}


} // namespace lyx
