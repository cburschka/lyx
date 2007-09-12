/**
 * \file ControlTexinfo.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlTexinfo.h"
#include "FuncRequest.h"
#include "debug.h"

#include "support/filetools.h"
#include "support/FileName.h"
#include "support/lstrings.h"

#include <algorithm>
#include <fstream>

using std::string;
using std::vector;
using std::endl;

namespace lyx {
namespace frontend {

using support::FileName;
using support::contains;
using support::split;
using support::token;
using support::getExtension;
using support::libFileSearch;
using support::onlyFilename;


ControlTexinfo::ControlTexinfo(Dialog & parent)
	: Controller(parent)
{}


void ControlTexinfo::viewFile(string const & filename) const
{
	string const arg = "file " + filename;
	dispatch(FuncRequest(LFUN_DIALOG_SHOW, arg));
}


/// get a class with full path from the list
string const getTexFileFromList(string const & file, string const & type)
{
	string file_ = file;
	// do we need to add the suffix?
	if (!(getExtension(file) == type))
		file_ += '.' + type;

	lyxerr << "Searching for file " << file_ << endl;

	string lstfile = type + "Files.lst";
	if (type == "cls")
		lstfile = "clsFiles.lst";
	else if (type == "sty")
		lstfile = "styFiles.lst";
	else if (type == "bst")
		lstfile = "bstFiles.lst";
	else if (type == "bib")
		lstfile = "bibFiles.lst";
	FileName const abslstfile = libFileSearch(string(), lstfile);
	if (abslstfile.empty()) {
		lyxerr << "File `'" << lstfile << "' not found." << endl;
		return string();
	}
	string const allClasses = getFileContents(abslstfile);
	int entries = 0;
	string classfile = token(allClasses, '\n', entries);
	int count = 0;
	while ((!contains(classfile, file) ||
		(onlyFilename(classfile) != file)) &&
		(++count < 1000)) {
		classfile = token(allClasses, '\n', ++entries);
	}

	// now we have filename with full path
	lyxerr << "with full path: " << classfile << endl;

	return classfile;
}


string const ControlTexinfo::getClassOptions(string const & classname) const
{
	FileName const filename(getTexFileFromList(classname, "cls"));
	if (filename.empty())
		return string();
	string optionList = string();
	std::ifstream is(filename.toFilesystemEncoding().c_str());
	while (is) {
		string s;
		is >> s;
		if (contains(s, "DeclareOption")) {
			s = s.substr(s.find("DeclareOption"));
			s = split(s, '{');		// cut front
			s = token(s, '}', 0);		// cut end
			optionList += (s + '\n');
		}
	}
	return optionList;
}


string const ControlTexinfo::getFileType(ControlTexinfo::texFileSuffix type) const
{
	string ftype;
	switch (type) {
	case ControlTexinfo::bst:
		ftype = "bst";
		break;
	case ControlTexinfo::cls:
		ftype = "cls";
		break;
	case ControlTexinfo::sty:
		ftype = "sty";
		break;
	}
	return ftype;
}

} // namespace frontend
} // namespace lyx
