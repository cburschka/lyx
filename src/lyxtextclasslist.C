/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "lyxtextclasslist.h"
#include "lyxtextclass.h"
#include "debug.h"
#include "lyxlex.h"
#include "gettext.h"

#include "frontends/Alert.h"

#include "support/lyxfunctional.h"
#include "support/LAssert.h"
#include "support/filetools.h"

#include <utility>

using lyx::textclass_type;
using std::pair;
using std::make_pair;
using std::endl;
using std::find_if;
using std::sort;


// Gets textclass number from name
pair<bool, textclass_type> const
LyXTextClassList::NumberOfClass(string const & textclass) const
{
	ClassList::const_iterator cit =
		find_if(classlist.begin(), classlist.end(),
			lyx::compare_memfun(&LyXTextClass::name, textclass));
	return cit != classlist.end() ?
		make_pair(true, textclass_type(cit - classlist.begin())) :
		make_pair(false, textclass_type(0));
}


// Gets a textclass structure from number
LyXTextClass const &
LyXTextClassList::operator[](textclass_type textclass) const
{
	classlist[textclass].load();
	if (textclass < classlist.size())
		return classlist[textclass];
	else
		return classlist[0];
}


void LyXTextClassList::Add(LyXTextClass const & t)
{
	classlist.push_back(t);
}


// used when sorting the textclass list.
class less_textclass_desc {
public:
	int operator()(LyXTextClass const & tc1, LyXTextClass const & tc2) {
		return tc1.description() < tc2.description();
	}
};


// Reads LyX textclass definitions according to textclass config file
bool LyXTextClassList::Read ()
{
	LyXLex lex(0, 0);
	string real_file = LibFileSearch("", "textclass.lst");
	lyxerr[Debug::TCLASS] << "Reading textclasses from `"
			      << real_file << "'" << endl;

	if (real_file.empty()) {
		lyxerr << "LyXTextClassList::Read: unable to find "
			"textclass file  `" << MakeDisplayPath(real_file, 1000)
		       << "'. Exiting." << endl;

		Alert::alert(_("LyX wasn't able to find its layout descriptions!"),
			   _("Check that the file \"textclass.lst\""),
			   _("is installed correctly. Sorry, has to exit :-("));
		return false;
		// This causes LyX to end... Not a desirable behaviour. Lgb
		// What do you propose? That the user gets a file dialog
		// and is allowed to hunt for the file? (Asger)
		// more that we have a layout for minimal.cls statically
		// compiled in... (Lgb)
	}

	if (!lex.setFile(real_file)) {
		lyxerr << "LyXTextClassList::Read: "
			"lyxlex was not able to set file: "
		       << real_file << endl;
	}
	
	if (!lex.isOK()) {
		lyxerr << "LyXTextClassList::Read: unable to open "
			"textclass file  `" << MakeDisplayPath(real_file, 1000)
		       << "'\nCheck your installation. LyX can't continue."
		       << endl;
 		return false;
	}

	bool finished = false;
	// Parse config-file
	lyxerr[Debug::TCLASS] << "Starting parsing of textclass.lst" << endl;
	while (lex.isOK() && !finished) {
		lyxerr[Debug::TCLASS] << "\tline by line" << endl;
		switch (lex.lex()) {
		case LyXLex::LEX_FEOF:
			finished = true;
			break;
		default:
			string const fname = lex.getString();
			lyxerr[Debug::TCLASS] << "Fname: " << fname << endl;
			if (lex.next()) {
				string const clname = lex.getString();
				lyxerr[Debug::TCLASS]
					<< "Clname: " << clname << endl;
				if (lex.next()) {
					      string const desc = lex.getString();
					      lyxerr[Debug::TCLASS]
						      << "Desc: " << desc << endl;
					      // This code is run when we have
					      // fname, clname and desc
					      LyXTextClass tmpl(fname,
								clname,
								desc);
					      if (lyxerr.
						  debugging(Debug::TCLASS)) {
						      tmpl.load();
					      }
					      Add (tmpl);
				}
			}
		}
	}
	lyxerr[Debug::TCLASS] << "End of parsing of textclass.lst" << endl;

	if (classlist.empty()) {
		lyxerr << "LyXTextClassList::Read: no textclasses found!"
		       << endl;
		Alert::alert(_("LyX wasn't able to find any layout description!"),
			   _("Check the contents of the file \"textclass.lst\""),
			   _("Sorry, has to exit :-("));
		return false;
	}
	// Ok everything loaded ok, now sort the list.
	sort(classlist.begin(), classlist.end(), less_textclass_desc());
	return true;
}


// Global variable: textclass table.
LyXTextClassList textclasslist;

// Reads the style files
void LyXSetStyle()
{
	lyxerr[Debug::TCLASS] << "LyXSetStyle: parsing configuration...\n";
	
	if (!textclasslist.Read()) {
		lyxerr[Debug::TCLASS] << "LyXSetStyle: an error occured "
			"during parsing.\n             Exiting." << endl;
		exit(1);
	}

	lyxerr[Debug::TCLASS] << "LyXSetStyle: configuration parsed." << endl;
}
