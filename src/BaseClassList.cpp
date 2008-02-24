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

// Gets textclass number from name
pair<bool, BaseClassIndex> const
BaseClassList::numberOfClass(string const & textclass) const
{
	ClassList::const_iterator cit =
		find_if(classlist_.begin(), classlist_.end(),
			bind(equal_to<string>(),
			     bind(&TextClass::name, _1),
			     textclass));

	return cit != classlist_.end() ?
		make_pair(true, BaseClassIndex(cit - classlist_.begin())) :
		make_pair(false, BaseClassIndex(0));
}


// Gets a textclass structure from number
TextClass const &
BaseClassList::operator[](BaseClassIndex textclass) const
{
	if (textclass >= classlist_.size())
		return classlist_[0];
	
	//FIXME I don't believe the following line is actually necessary (rgh)
	classlist_[textclass].load();
	return classlist_[textclass];
}


// used when sorting the textclass list.
class less_textclass_avail_desc
	: public binary_function<TextClass, TextClass, int>
{
public:
	int operator()(TextClass const & tc1,
		       TextClass const & tc2) const
	{
		// Ordering criteria:
		//   1. Availability of text class
		//   2. Description (lexicographic)

		return (tc1.isTeXClassAvailable() && !tc2.isTeXClassAvailable()) ||
			(tc1.isTeXClassAvailable() == tc2.isTeXClassAvailable() &&
			 tc1.description() < tc2.description());
	}
};


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
						classlist_.push_back(tmpl);
					}
				}
			}
		}
	}
	LYXERR(Debug::TCLASS, "End of parsing of textclass.lst");

	// lyx will start with an empty classlist_, but only reconfigure is allowed
	// in this case. This gives users a second chance to configure lyx if
	// initial configuration fails. (c.f. bug 2829)
	if (classlist_.empty())
		lyxerr << "BaseClassList::Read: no textclasses found!"
		       << endl;
	else 
		// Ok everything loaded ok, now sort the list.
		sort(classlist_.begin(), classlist_.end(), less_textclass_avail_desc());
	return true;
}


void BaseClassList::reset(BaseClassIndex const textclass) {
	if (textclass >= classlist_.size())
		return;
	TextClass const & tc = classlist_[textclass];
	TextClass tmpl(tc.name(), tc.latexname(), tc.description(), 
	               tc.isTeXClassAvailable());
	classlist_[textclass] = tmpl;
}


pair<bool, BaseClassIndex> const
BaseClassList::addTextClass(string const & textclass, string const & path)
{
	// only check for textclass.layout file, .cls can be anywhere in $TEXINPUTS
	// NOTE: latex class name is defined in textclass.layout, which can be different from textclass
	FileName const layout_file(addName(path, textclass + ".layout"));
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
				// returns: whole string, classtype (not used here), first option, description
				BOOST_ASSERT(sub.size()==4);
				// now, create a TextClass with description containing path information
				TextClass tmpl(textclass, sub.str(2)==""?textclass:sub.str(2),
					sub.str(3) + " <" + path + ">", true);
				if (lyxerr.debugging(Debug::TCLASS))
					tmpl.load(path);
				// Do not add this local TextClass to classlist_ if it has
				// already been loaded by, for example, a master buffer.
				pair<bool, lyx::BaseClassIndex> pp =
					baseclasslist.numberOfClass(textclass);
				// only layouts from the same directory are considered to be identical.
				if (pp.first && classlist_[pp.second].description() == tmpl.description())
					return pp;
				classlist_.push_back(tmpl);
				// This textclass is added on request so it will definitely be
				// used. Load it now because other load() calls may fail if they
				// are called in a context without buffer path information.
				classlist_.back().load(path);
				return make_pair(true, classlist_.size() - 1);
			}
		}
	}
	// If .layout is not in local directory, or an invalid layout is found, return false
	return make_pair(false, BaseClassIndex(0));
}


// Global variable: textclass table.
BaseClassList baseclasslist;


BaseClassIndex defaultBaseclass()
{
	// We want to return the article class. if `first' is
	// true in the returned pair, then `second' is the textclass
	// number; if it is false, second is 0. In both cases, second
	// is what we want.
	return baseclasslist.numberOfClass("article").second;
}



// Reads the style files
bool LyXSetStyle()
{
	LYXERR(Debug::TCLASS, "LyXSetStyle: parsing configuration...");

	if (!baseclasslist.read()) {
		LYXERR(Debug::TCLASS, "LyXSetStyle: an error occured "
			"during parsing.\n             Exiting.");
		return false;
	}

	LYXERR(Debug::TCLASS, "LyXSetStyle: configuration parsed.");
	return true;
}


} // namespace lyx
