/**
 * \file tex_helpers.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "tex_helpers.h"

#include "debug.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/systemcall.h"
#include "support/path.h"
#include "support/lyxalgo.h"
#include "support/path_defines.h"

#include <boost/cregex.hpp>
#include <fstream>

using lyx::support::contains;
using lyx::support::GetFileContents;
using lyx::support::getVectorFromString;
using lyx::support::LibFileSearch;
using lyx::support::OnlyFilename;
using lyx::support::Path;
using lyx::support::Path;
using lyx::support::split;
using lyx::support::Systemcall;
using lyx::support::token;
using lyx::support::user_lyxdir;

using std::string;
using std::endl;


// build filelists of all availabe bst/cls/sty-files. done through
// kpsewhich and an external script, saved in *Files.lst
void rescanTexStyles()
{
	// Run rescan in user lyx directory
	Path p(user_lyxdir());
	Systemcall one;
	one.startscript(Systemcall::Wait,
			LibFileSearch("scripts", "TeXFiles.sh"));
}


void texhash()
{
	// Run texhash in user lyx directory
	Path p(user_lyxdir());

	//path to texhash through system
	Systemcall one;
	one.startscript(Systemcall::Wait,"texhash");
}


void getTexFileList(string const & filename, std::vector<string> & list)
{
	list.clear();
	string const file = LibFileSearch("", filename);
	if (file.empty())
		return;

	list = getVectorFromString(GetFileContents(file), "\n");

	// Normalise paths like /foo//bar ==> /foo/bar
	boost::RegEx regex("/{2,}");
	std::vector<string>::iterator it  = list.begin();
	std::vector<string>::iterator end = list.end();
	for (; it != end; ++it) {
		*it = regex.Merge((*it), "/");
	}

	lyx::eliminate_duplicates(list);
}


string const getListOfOptions(string const & classname, string const & type)
{
	string const filename = getTexFileFromList(classname,type);
	string optionList = string();
	std::ifstream is(filename.c_str());
	while (is) {
		string s;
		is >> s;
		if (contains(s,"DeclareOption")) {
			s = s.substr(s.find("DeclareOption"));
			s = split(s,'{');		// cut front
			s = token(s,'}',0);		// cut end
			optionList += (s + '\n');
		}
	}
	return optionList;
}


string const getTexFileFromList(string const & file,
			    string const & type)
{
	string const file_ = (type == "cls") ? file + ".cls" : file + ".sty";

	lyxerr << "Search for classfile " << file_ << endl;

	string const lstfile =
		((type == "cls") ? "clsFiles.lst" : "styFiles.lst");
	string const allClasses = GetFileContents(LibFileSearch(string(),
								lstfile));
	int entries = 0;
	string classfile = token(allClasses, '\n', entries);
	int count = 0;
	while ((!contains(classfile, file) ||
		(OnlyFilename(classfile) != file)) &&
		(++count < 1000)) {
		classfile = token(allClasses, '\n', ++entries);
	}

	// now we have filename with full path
	lyxerr << "with full path: " << classfile << endl;

	return classfile;
}
