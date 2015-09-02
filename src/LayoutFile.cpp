/**
 * \file LayoutFileList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LayoutFile.h"
#include "Counters.h"
#include "Floating.h"
#include "FloatList.h"
#include "Lexer.h"
#include "TextClass.h"

#include "frontends/alert.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

#include "support/bind.h"
#include "support/regex.h"
#include "support/TempFile.h"

#include <fstream>

using namespace std;
using namespace lyx::support;

namespace lyx {



LayoutFile::LayoutFile(string const & fn, string const & cln,
		       string const & desc, string const & prereq,
		       string const & category, bool texclassavail) 
{
	name_ = fn;
	latexname_ = cln;
	description_ = desc;
	prerequisites_ = prereq;
	category_ = category;
	tex_class_avail_ = texclassavail;
}


LayoutFileList::~LayoutFileList()
{
	ClassMap::const_iterator it = classmap_.begin();
	ClassMap::const_iterator en = classmap_.end();
	for (; it != en; ++it) {
		delete it->second;
	}
}


LayoutFileList & LayoutFileList::get() 
{
	static LayoutFileList baseclasslist;
	return baseclasslist;
}


bool LayoutFileList::haveClass(string const & classname) const
{
	ClassMap::const_iterator it = classmap_.begin();
	ClassMap::const_iterator en = classmap_.end();
	for (; it != en; ++it) {
		if (it->first == classname)
			return true;
	}
	return false;
}


LayoutFile const & LayoutFileList::operator[](string const & classname) const
{
	LATTEST(haveClass(classname));
	// safe to continue, since we will make an empty LayoutFile
	return *classmap_[classname];
}


LayoutFile & LayoutFileList::operator[](string const & classname)
{
	LATTEST(haveClass(classname));
	// safe to continue, since we will make an empty LayoutFile
	return *classmap_[classname];
}


// Reads LyX textclass definitions according to textclass config file
bool LayoutFileList::read()
{
	bool success = false;
	Lexer lex;
	FileName const real_file = libFileSearch("", "textclass.lst");
	LYXERR(Debug::TCLASS, "Reading textclasses from `" << real_file << "'.");

	if (real_file.empty()) {
		LYXERR0("LayoutFileList::Read: unable to find textclass file  "
		    << "`textclass.lst'.");
	} else if (!lex.setFile(real_file)) {
		LYXERR0("LayoutFileList::Read: lyxlex was not able to set file: "
		       << real_file << '.');
	} else if (!lex.isOK()) {
		LYXERR0("LayoutFileList::Read: unable to open textclass file  `"
		       << makeDisplayPath(real_file.absFileName(), 1000)
		       << "'\nCheck your installation.");
	} else {
		// we have a file we can read.
		bool finished = false;
		LYXERR(Debug::TCLASS, "Starting parsing of textclass.lst");
		while (lex.isOK() && !finished) {
			LYXERR(Debug::TCLASS, "\tline by line");
			switch (lex.lex()) {
			case Lexer::LEX_FEOF:
				finished = true;
				break;
			default:
				string const fname = lex.getString();
				LYXERR(Debug::TCLASS, "Fname: " << fname);
				if (!lex.next()) 
					break;
				string const clname = lex.getString();
				LYXERR(Debug::TCLASS, "Clname: " << clname);
				if (!lex.next()) 
					break;
				string const desc = lex.getString();
				LYXERR(Debug::TCLASS, "Desc: " << desc);
				if (!lex.next()) 
					break;
				bool avail = lex.getBool();
				LYXERR(Debug::TCLASS, "Avail: " << avail);
				if (!lex.next()) 
					break;
				string const prereq = lex.getString();
				LYXERR(Debug::TCLASS, "Prereq: " << prereq);
				if (!lex.next()) 
					break;
				string const category = lex.getString();
				LYXERR(Debug::TCLASS, "Category: " << category);
				// This code is run when we have
				// fname, clname, desc, prereq, and avail
				LayoutFile * tmpl = new LayoutFile(fname, clname, desc, prereq, category, avail);
				if (lyxerr.debugging(Debug::TCLASS)) {
					// only system layout files are loaded here so no
					// buffer path is needed.
					tmpl->load();
				}
				classmap_[fname] = tmpl;
			} // end of switch
		} // end of while loop
		LYXERR(Debug::TCLASS, "End parsing of textclass.lst");
		success = true;
	} // end of else

	// LyX will start with an empty classmap_. This is OK because
	// (a) we will give the user a chance to reconfigure (see bug 2829) and
	// (b) even if that fails, we can use addEmptyClass() to get some basic
	// functionality.
	if (classmap_.empty())
		LYXERR0("LayoutFileList::Read: no textclasses found!");
	return success;
}


std::vector<LayoutFileIndex> LayoutFileList::classList() const
{
	std::vector<LayoutFileIndex> cl;
	ClassMap::const_iterator it = classmap_.begin();
	ClassMap::const_iterator en = classmap_.end();
	for (; it != en; ++it)
		cl.push_back(it->first);
	return cl;
}


void LayoutFileList::reset(LayoutFileIndex const & classname)
{
	LATTEST(haveClass(classname));
	// safe to continue, since we will make an empty LayoutFile
	LayoutFile * tc = classmap_[classname];
	LayoutFile * tmpl = 
		new LayoutFile(tc->name(), tc->latexname(), tc->description(),
		               tc->prerequisites(), tc->category(),
			       tc->isTeXClassAvailable());
	classmap_[classname] = tmpl;
	delete tc;
}


namespace {

string layoutpost =			
		"Columns      1\n"
		"Sides        1\n"
		"SecNumDepth  2\n"
		"TocDepth     2\n"
		"DefaultStyle	Standard\n\n"
		"Style Standard\n"
		"	Category              MainText\n"
		"	Margin                Static\n"
		"	LatexType             Paragraph\n"
		"	LatexName             dummy\n"
		"	ParIndent             MM\n"
		"	ParSkip               0.4\n"
		"	Align                 Block\n"
		"	AlignPossible         Block, Left, Right, Center\n"
		"	LabelType             No_Label\n"
		"End\n";
	
}


LayoutFileIndex LayoutFileList::addEmptyClass(string const & textclass)
{
	// FIXME This could be simplified a bit to call TextClass::read(string, ReadType).

	TempFile tempfile("basicXXXXXX.layout");
	FileName const tempLayout = tempfile.name();
	ofstream ofs(tempLayout.toFilesystemEncoding().c_str());
	// This writes a very basic class, but it also attempts to include 
	// stdclass.inc. That would give us something moderately usable.
	ofs << "# This layout is automatically generated\n"
	       "# \\DeclareLaTeXClass{" << textclass << "}\n\n"
	       "Format " << LAYOUT_FORMAT << "\n"
	       "Input stdclass.inc\n\n"
	    << layoutpost;
	ofs.close();

	// We do not know if a LaTeX class is available for this document, but setting
	// the last parameter to true will suppress a warning message about missing
	// tex class.
	LayoutFile * tc = new LayoutFile(textclass, textclass, 
			"Unknown text class " + textclass, textclass + ".cls", "", true);

	if (!tc->load(tempLayout.absFileName())) {
		// The only way this happens is because the hardcoded layout file 
		// above is wrong or stdclass.inc cannot be found. So try again 
		// without stdclass.inc and without stdinsets.inc. 
		ofstream ofs2(tempLayout.toFilesystemEncoding().c_str());
		ofs2 << "# This layout is automatically generated\n"
		        "# \\DeclareLaTeXClass{" << textclass << "}\n\n"
		        "Format " << LAYOUT_FORMAT << "\n"
		        "Provides stdinsets 1\n"
		     << layoutpost;
		ofs2.close();
		if (!tc->load(tempLayout.absFileName())) {
			// This can only happen if the hardcoded file above is wrong
			// or there is some weird filesystem error.
			LATTEST(false); // We will get an empty layout or something.
		}
	}

	classmap_[textclass] = tc;
	return textclass;
}


LayoutFileIndex  LayoutFileList::addLocalLayout(
	string const & textclass, string const & path, string const & oldpath)
{
	// FIXME  There is a bug here: 4593
	//
	// only check for textclass.layout file, .cls can be anywhere in $TEXINPUTS
	// NOTE: latex class name is defined in textclass.layout, which can be 
	// different from textclass
	string fullName = addName(path, textclass + ".layout");
	
	FileName layout_file(fullName);
	bool moved = false;

	if (!layout_file.exists()) {
		if (oldpath.empty())
			return string();
		// The document has been moved to a different directory.
		// However, oldpath always points to the right spot, unless
		// the user also moved the layout file.
		fullName = addName(oldpath, textclass + ".layout");
		layout_file.set(fullName);
		layout_file.refresh();
		if (!layout_file.exists())
			return string();
		moved = true;
	}

	LYXERR(Debug::TCLASS, "Adding class " << textclass << " from directory " << path);
	// Read .layout file and get description, real latex classname etc
	//
	// This is a C++ version of function processLayoutFile in configure.py,
	// which uses the following regex
	//     \Declare(LaTeX|DocBook)Class\s*(\[([^,]*)(,.*)*\])*\s*{(.*)}
	ifstream ifs(layout_file.toFilesystemEncoding().c_str());
	static regex const reg("^\\s*#\\s*\\\\Declare(LaTeX|DocBook)Class\\s*"
		"(?:\\[([^,]*)(?:,.*)*\\])*\\s*\\{(.*)\\}\\s*");
	static regex const catreg("^\\s*#\\s*\\\\DeclareCategory\\{(.*)\\}\\s*");
	string line;
	string class_name;
	string class_prereq;
	string category;
	bool have_declaration = false;
	while (getline(ifs, line)) {
		// look for the \DeclareXXXClass line
		smatch sub;
		if (regex_match(line, sub, reg)) {
			// returns: whole string, classtype (not used here), class name, description
			// LASSERT: Why would this fail?
			LASSERT(sub.size() == 4, /**/);
			// now, create a TextClass with description containing path information
			class_name = (sub.str(2) == "" ? textclass : sub.str(2));
			class_prereq = class_name + ".cls";
			have_declaration = true;
		}
		else if (regex_match(line, sub, catreg)) {
			category = sub.str(1);
		}
		if (have_declaration && !category.empty())
			break;
	}

	if (!have_declaration)
		return string();

	LayoutFile * tmpl =
		new LayoutFile(textclass, class_name, textclass, class_prereq, category, true);
	//FIXME: The prerequisites are available from the layout file and
	//       can be extracted from the above regex, but for now this
	//       field is simply set to class_name + ".cls"
	// This textclass is added on request so it will definitely be
	// used. Load it now because other load() calls may fail if they
	// are called in a context without buffer path information.
	tmpl->load(moved ? oldpath : path);
	// There will be only one textclass with this name, even if different
	// layout files are loaded from different directories.
	if (haveClass(textclass)) {
		// Unconditionally issuing the warning may be confusing when
		// saving the document with a different name, as it is exactly
		// the same textclass that is being re-established.
		LYXERR(Debug::TCLASS, "Existing textclass " << textclass << " is redefined by " << fullName);
		delete classmap_[textclass];
	}
	classmap_[textclass] = tmpl;
	return removeExtension(fullName);
}


bool LayoutFileList::load(string const & name, string const & buf_path)
{
	if (!haveClass(name)) {
		LYXERR0("Document class \"" << name << "\" does not exist.");
		return false;
	}

	LayoutFile * tc = classmap_[name];
	return tc->load(buf_path);
}


LayoutFileIndex defaultBaseclass()
{
	if (LayoutFileList::get().haveClass("article"))
		return string("article");
	if (LayoutFileList::get().empty())
		// we'll call it that, since this gives the user a chance to
		// have a functioning document when things improve.
		return string("article");
	return LayoutFileList::get().classList().front();
}

} // namespace lyx
