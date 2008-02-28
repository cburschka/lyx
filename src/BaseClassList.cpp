/**
 * \file BaseClassList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BaseClassList.h"
#include "TextClass.h"
#include "Lexer.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/filetools.h"

#include <boost/bind.hpp>
#include <boost/regex.hpp>

#include <fstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

using boost::bind;
using boost::regex;
using boost::smatch;

BaseClassList & BaseClassList::get() 
{
	static BaseClassList baseclasslist;
	return baseclasslist;
}


bool BaseClassList::haveClass(string const & classname) const
{
	ClassMap::const_iterator it = classmap_.begin();
	ClassMap::const_iterator en = classmap_.end();
	for (; it != en; ++it) {
		if (it->first == classname)
			return true;
	}
	return false;
}


// Gets a textclass structure from string
TextClass const & 
	BaseClassList::operator[](string const & classname) const
{
	BOOST_ASSERT(haveClass(classname));
	return classmap_[classname];
}


// Reads LyX textclass definitions according to textclass config file
bool BaseClassList::read()
{
	Lexer lex(0, 0);
	FileName const real_file = libFileSearch("", "textclass.lst");
	LYXERR(Debug::TCLASS, "Reading textclasses from `" << real_file << '\'');

	if (real_file.empty()) {
		lyxerr << "BaseClassList::Read: unable to find "
			  "textclass file  `"
		       << to_utf8(makeDisplayPath(real_file.absFilename(), 1000))
		       << "'. Exiting." << endl;
		return false;
		// This causes LyX to end... Not a desirable behaviour. Lgb
		// What do you propose? That the user gets a file dialog
		// and is allowed to hunt for the file? (Asger)
		// more that we have a layout for minimal.cls statically
		// compiled in... (Lgb)
	}

	if (!lex.setFile(real_file)) {
		lyxerr << "BaseClassList::Read: "
			"lyxlex was not able to set file: "
		       << real_file << endl;
	}

	if (!lex.isOK()) {
		lyxerr << "BaseClassList::Read: unable to open "
			  "textclass file  `"
		       << to_utf8(makeDisplayPath(real_file.absFilename(), 1000))
		       << "'\nCheck your installation. LyX can't continue."
		       << endl;
		return false;
	}

	bool finished = false;
	// Parse config-file
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
			if (lex.next()) {
				string const clname = lex.getString();
				LYXERR(Debug::TCLASS, "Clname: " << clname);
				if (lex.next()) {
					string const desc = lex.getString();
					LYXERR(Debug::TCLASS, "Desc: " << desc);
					if (lex.next()) {
						bool avail = lex.getBool();
						LYXERR(Debug::TCLASS, "Avail: " << avail);
						// This code is run when we have
						// fname, clname, desc, and avail
						TextClass tmpl(fname, clname, desc, avail);
						if (lyxerr.debugging(Debug::TCLASS)) {
							// only system layout files are loaded here so no
							// buffer path is needed.
							tmpl.load();
						}
						classmap_[fname] = tmpl;
					}
				}
			}
		}
	}
	LYXERR(Debug::TCLASS, "End of parsing of textclass.lst");

	// lyx will start with an empty classmap_, but only reconfigure is allowed
	// in this case. This gives users a second chance to configure lyx if
	// initial configuration fails. (c.f. bug 2829)
	if (classmap_.empty())
		lyxerr << "BaseClassList::Read: no textclasses found!"
		       << endl;
	return true;
}


std::vector<BaseClassIndex> BaseClassList::classList() const
{
	std::vector<BaseClassIndex> cl;
	ClassMap::const_iterator it = classmap_.begin();
	ClassMap::const_iterator en = classmap_.end();
	for (; it != en; ++it)
		cl.push_back(it->first);
	return cl;
}


void BaseClassList::reset(BaseClassIndex const & classname) {
	BOOST_ASSERT(haveClass(classname));
	TextClass const & tc = classmap_[classname];
	TextClass tmpl(tc.name(), tc.latexname(), tc.description(), 
	               tc.isTeXClassAvailable());
	classmap_[classname] = tmpl;
}


string const BaseClassList::localPrefix = "LOCAL:";


BaseClassIndex 
	BaseClassList::addTextClass(string const & textclass, string const & path)
{
	// FIXME BUGS
	// There be bugs here. The way this presently works, the local class gets 
	// added to the global list of available document classes. It will then
	// appear on the list in Document>Settings, where it could be chosen in, 
	// say, a new document, with no real warning that the class may not be
	// available when the document is saved, since the new document may not be
	// in the same directory as the layout file.
	//
	// Another bug is this: If the Document>Settings dialog is open when a file
	// with a local layout is opened, the dialog doesn't update.
	//
	// only check for textclass.layout file, .cls can be anywhere in $TEXINPUTS
	// NOTE: latex class name is defined in textclass.layout, which can be 
	// different from textclass
	string fullName = addName(path, textclass + ".layout");
	string localIndex = localPrefix + textclass;

	// if the local file has already been loaded, return it
	if (haveClass(localIndex))
		return localIndex;

	FileName const layout_file(fullName);
	if (layout_file.exists()) {
		LYXERR(Debug::TCLASS, "Adding class " << textclass << " from directory " << path);
		// Read .layout file and get description, real latex classname etc
		//
		// This is a C++ version of function processLayoutFile in configure.py,
		// which uses the following regex
		//     \Declare(LaTeX|DocBook)Class\s*(\[([^,]*)(,.*)*\])*\s*{(.*)}
		ifstream ifs(layout_file.toFilesystemEncoding().c_str());
		static regex const reg("^#\\s*\\\\Declare(LaTeX|DocBook)Class\\s*"
			"(?:\\[([^,]*)(?:,.*)*\\])*\\s*\\{(.*)\\}\\s*");
		string line;
		while (getline(ifs, line)) {
			// look for the \DeclareXXXClass line
			smatch sub;
			if (regex_match(line, sub, reg)) {
				// returns: whole string, classtype (not used here), class name, description
				BOOST_ASSERT(sub.size() == 4);
				// now, create a TextClass with description containing path information
				TextClass tmpl(textclass, sub.str(2) == "" ? textclass : sub.str(2),
					sub.str(3) + " <" + path + ">", true);
				// Do not add this local TextClass to classmap_ if it has
				// already been loaded by, for example, a master buffer.
				if (haveClass(textclass)
						// FIXME I don't understand this comment (rgh)
						// only layouts from the same directory are considered to be identical.
						&& classmap_[textclass].description() == tmpl.description()
				   )
					return textclass;
				classmap_[localIndex] = tmpl;
				// This textclass is added on request so it will definitely be
				// used. Load it now because other load() calls may fail if they
				// are called in a context without buffer path information.
				classmap_[localIndex].load(path);
				return localIndex;
			}
		}
	}
	// If .layout is not in local directory, or an invalid layout is found, return null
	return string("");
}


BaseClassIndex defaultBaseclass()
{
	if (BaseClassList::get().haveClass("article"))
		return string("article");
	else 
		return string("");
}



// Reads the style files
bool LyXSetStyle()
{
	LYXERR(Debug::TCLASS, "LyXSetStyle: parsing configuration...");

	if (!BaseClassList::get().read()) {
		LYXERR(Debug::TCLASS, "LyXSetStyle: an error occured "
			"during parsing.\n             Exiting.");
		return false;
	}

	LYXERR(Debug::TCLASS, "LyXSetStyle: configuration parsed.");
	return true;
}


} // namespace lyx
