/**
 * \file tex_helpers.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voss 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "tex_helpers.h"

#include "debug.h"
#include "gettext.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/systemcall.h"
#include "support/path.h"
#include "support/lyxalgo.h"

#include <vector>
#include <fstream>
#include <algorithm>

using std::vector;
using std::endl;
using std::sort;
using std::unique;

extern string user_lyxdir; // home of *Files.lst

namespace {

vector<string> listWithoutPath(vector<string> & dbase)
{
	vector<string>::iterator it = dbase.begin();
	vector<string>::iterator end = dbase.end();
	for (; it != end; ++it) 
		*it = OnlyFilename(*it);
	return dbase;
}

}

// build filelists of all availabe bst/cls/sty-files. done through
// kpsewhich and an external script, saved in *Files.lst
void rescanTexStyles()
{
	// Run rescan in user lyx directory
	Path p(user_lyxdir);
	Systemcall one;
	one.startscript(Systemcall::Wait,
			LibFileSearch("scripts", "TeXFiles.sh"));
	p.pop();
}


void texhash()
{
	// Run texhash in user lyx directory
	Path p(user_lyxdir);

	//path to texhash through system
	Systemcall one;
	one.startscript(Systemcall::Wait,"texhash"); 
	p.pop();
}

string const getTexFileList(string const & filename, bool withFullPath)
{
	string const file = LibFileSearch("", filename);
	if (file.empty())
		return string();
 
	vector<string> dbase =
		getVectorFromString(GetFileContents(file), "\n");

	lyx::eliminate_duplicates(dbase); 
	string const str_out = withFullPath ?
		getStringFromVector(dbase, "\n") :
		getStringFromVector(listWithoutPath(dbase), "\n");
	return str_out;
}


string const getListOfOptions(string const & classname,
			    string const & type)
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
 
	string const lstfile = (type == "cls") ? "clsFiles.lst" : "styFiles.lst";
	string const allClasses = GetFileContents(LibFileSearch(string(), lstfile));
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
