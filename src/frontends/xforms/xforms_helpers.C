/**
 * \file xforms_helpers.C
 * Copyright 2000-2002 The LyX Team.
 * See the file COPYING.
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#include FORMS_H_LOCATION

#include <fstream> // ofstream
#include <vector>

#ifdef __GNUG_
#pragma implementation
#endif
 
#include "xforms_helpers.h"
#include "lyxlex.h"
#include "support/FileInfo.h"
#include "support/filetools.h"
#include "support/lstrings.h" // frontStrip, strip
#include "gettext.h"
#include "support/LAssert.h"
#include "lyxlength.h"
#include "lyxgluelength.h"

using std::ofstream;
using std::pair;
using std::vector;

// Extract shortcut from <ident>|<shortcut> string
char const * flyx_shortcut_extract(char const * sc)
{
	// Find '|' in the sc and return the string after that.
	register char const * sd = sc;
	while (sd[0]!= 0 && sd[0] != '|') ++sd;

	if (sd[0] == '|') {
		++sd;
		//lyxerr << sd << endl;
		return sd;
	}
	return "";
}


// Extract identifier from <ident>|<shortcut> string
char const * flyx_ident_extract(char const * sc)
{
	register char const * se = sc;
	while (se[0]!= 0 && se[0] != '|') ++se;

	if (se[0] == 0) return sc;
	
	char * sb = new char[se - sc + 1];
	int index = 0;
	register char const * sd = sc;
	while (sd != se) {
		sb[index] = sd[0];
		++index; ++sd;
	}
	sb[index] = 0;
	return sb;
}


// Set an FL_OBJECT to activated or deactivated
void setEnabled(FL_OBJECT * ob, bool enable)
{
	if (enable) {
		fl_activate_object(ob);
		fl_set_object_lcol(ob, FL_BLACK);
	} else {
		fl_deactivate_object(ob);
		fl_set_object_lcol(ob, FL_INACTIVE);
	}
}

	
// Given an fl_choice, create a vector of its entries
vector<string> const getVectorFromChoice(FL_OBJECT * ob)
{
	vector<string> vec;
	if (!ob || ob->objclass != FL_CHOICE)
		return vec;

	for(int i = 0; i < fl_get_choice_maxitems(ob); ++i) {
		string const text = fl_get_choice_item_text(ob, i+1);
		vec.push_back(strip(frontStrip(text)));
	}

	return vec;
}


/// Given an fl_input, return its contents.
string const getStringFromInput(FL_OBJECT * ob)
{
	if (!ob || ob->objclass != FL_INPUT)
		return string();

	char const * tmp = fl_get_input(ob);
	return (tmp) ? tmp : string();
}


// Given an fl_browser, return the contents of line
string const getStringFromBrowser(FL_OBJECT * ob, int line)
{
	if (!ob || ob->objclass != FL_BROWSER || 
	    line < 1 || line > fl_get_browser_maxline(ob))
		return string();

	char const * tmp = fl_get_browser_line(ob, line);
	return (tmp) ? tmp : string();
}

// Given an fl_browser, return the contents of the currently
// highlighted line.
// If nothing is selected, return an empty string
string const getSelectedStringFromBrowser(FL_OBJECT * ob)
{
	if (!ob || ob->objclass != FL_BROWSER)
		return string();

	int const line = fl_get_browser(ob);
	if (line < 1 || line > fl_get_browser_maxline(ob))
		return string();

	if (!fl_isselected_browser_line(ob, line))
		return string();

	char const * tmp = fl_get_browser_line(ob, line);
	return (tmp) ? tmp : string();
}


// Given an fl_browser, create a vector of its entries
vector<string> const getVectorFromBrowser(FL_OBJECT * ob)
{
	vector<string> vec;
	if (!ob || ob->objclass != FL_BROWSER)
		return vec;

	for(int i = 0; i < fl_get_browser_maxline(ob); ++i) {
		string const text = fl_get_browser_line(ob, i+1);
		vec.push_back(strip(frontStrip(text)));
	}

	return vec;
}


string getLengthFromWidgets(FL_OBJECT * input, FL_OBJECT * choice)
{
	// Paranoia check
	lyx::Assert(input  && input->objclass  == FL_INPUT &&
		    choice && choice->objclass == FL_CHOICE);

	string const length = strip(frontStrip(fl_get_input(input)));
	if (length.empty())
		return string();

	//don't return unit-from-choice if the input(field) contains a unit
	if (isValidGlueLength(length))
		return length;

	string unit = strip(frontStrip(fl_get_choice_text(choice)));
	unit = subst(unit, "%%", "%");

	return length + unit;
}
	

#if 0
// old code which can be deleted if the new one, now enabled,
// works satisfyingly (JSpitzm, 11/02/2002)
// this should definitely be the other way around!!!
void updateWidgetsFromLength(FL_OBJECT * input, FL_OBJECT * choice,
			     LyXLength const & len,
			     string const & default_unit)
{
	if (len.zero())
		updateWidgetsFromLengthString(input, choice,
					      string(), default_unit);
	// use input field only for gluelengths
	else if (!isValidLength(len) && !isStrDbl(len)) {
		fl_set_input(input, len.c_str());
		fl_set_choice_text(choice, default_unit.c_str());
	}
	else
		updateWidgetsFromLengthString(input, choice,
					      len.asString(), default_unit);

}


// Most of the code here is a poor duplication of the parser code
// which is in LyXLength. Use that instead
void updateWidgetsFromLengthString(FL_OBJECT * input, FL_OBJECT * choice,
				   string const & str,
				   string const & default_unit)
{
	// Paranoia check
	lyx::Assert(input  && input->objclass  == FL_INPUT &&
		    choice && choice->objclass == FL_CHOICE);

	if (str.empty()) {
		fl_set_input(input, "");
		int unitpos = 1; // xforms has Fortran-style indexing
		for(int i = 0; i < fl_get_choice_maxitems(choice); ++i) {
			string const text = fl_get_choice_item_text(choice,i+1);
			if (default_unit ==
			    lowercase(strip(frontStrip(text)))) {
				unitpos = i+1;
				break;
			}
		}
		fl_set_choice(choice, unitpos);
		return;
	}

	// The unit is presumed to begin at the first char a-z
	// or with the char '%'
	string const tmp = lowercase(strip(frontStrip(str)));

	string::const_iterator p = tmp.begin();
	for (; p != tmp.end(); ++p) {
		if ((*p >= 'a' && *p <= 'z') || *p == '%')
			break;
	}

	string len = "0";
	int unitpos = 1; // xforms has Fortran-style indexing

	if (p == tmp.end()) {
		if (isStrDbl(tmp))
			len = tmp;

	} else {
		string tmplen = string(tmp.begin(), p);
		if (isStrDbl(tmplen))
			len = tmplen;
		string unit = string(p, tmp.end());
		unit = subst(unit, "%", "%%");

		for(int i = 0; i < fl_get_choice_maxitems(choice); ++i) {
			string const text = fl_get_choice_item_text(choice,i+1);
			if (unit == lowercase(strip(frontStrip(text)))) {
				unitpos = i+1;
				break;
			}
		}
	}
	
	fl_set_input(input,   len.c_str());
	fl_set_choice(choice, unitpos);
}
#else
void updateWidgetsFromLengthString(FL_OBJECT * input, FL_OBJECT * choice,
				   string const & str,
				   string const & default_unit)
{
	// use input field only for gluelengths
	if (!isValidLength(str) && !isStrDbl(str)) {
		fl_set_input(input, str.c_str());
		fl_set_choice_text(choice, default_unit.c_str());
	} else {
		updateWidgetsFromLength(input, choice,
				LyXLength(str), default_unit);
	}
}


void updateWidgetsFromLength(FL_OBJECT * input, FL_OBJECT * choice,
			     LyXLength const & len,
			     string const & default_unit)
{
	// Paranoia check
	lyx::Assert(input  && input->objclass  == FL_INPUT &&
		    choice && choice->objclass == FL_CHOICE);

	if (len.zero()) {
		fl_set_input(input, "");
		fl_set_choice_text(choice, default_unit.c_str());
	} else {
		ostringstream buffer;
		buffer << len.value();
		fl_set_input(input, buffer.str().c_str());
		fl_set_choice_text(choice, 
		    subst(stringFromUnit(len.unit()),"%","%%").c_str());
	}
}
#endif


// Take a string and add breaks so that it fits into a desired label width, w
string formatted(string const & sin, int w, int size, int style)
{
	// FIX: Q: Why cant this be done by a one pass algo? (Lgb)

	string sout;
	if (sin.empty()) return sout;

	// breaks in up into a vector of individual words
	vector<string> sentence;
	string word;
	for (string::const_iterator sit = sin.begin();
	     sit != sin.end(); ++sit) {
		if ((*sit) == ' ' || (*sit) == '\n') {
			if (!word.empty()) {
				sentence.push_back(word);
				word.erase();
			}
			if ((*sit) == '\n') word += '\n';
			
		} else {
			word += (*sit);
		}
	}

	// Flush remaining contents of word
	if (!word.empty()) sentence.push_back(word);

	string line;
	string line_plus_word;
	for (vector<string>::const_iterator vit = sentence.begin();
	     vit != sentence.end(); ++vit) {
		string word(*vit);

		char c = word[0];
		if (c == '\n') {
			sout += line + '\n';
			word.erase(0,1);
			line_plus_word.erase();
			line.erase();
		}

		if (!line_plus_word.empty()) line_plus_word += ' ';
		line_plus_word += word;

		int const length = fl_get_string_width(style, size,
						       line_plus_word.c_str(),
						       int(line_plus_word.length()));
		if (length >= w) {
			sout += line + '\n';
			line_plus_word = word;
		}

		line = line_plus_word;
	}
	// Flush remaining contents of line
	if (!line.empty()) {
		sout += line;
	}

	if (sout[sout.length() - 1] == '\n')
		sout.erase(sout.length() - 1);

	return sout;
}


namespace {

// sorted by hand to prevent LyXLex from complaining on read().
keyword_item xformTags[] = {
	{ "\\gui_background",   FL_COL1 },
	{ "\\gui_buttonbottom", FL_BOTTOM_BCOL },
	{ "\\gui_buttonleft",   FL_LEFT_BCOL },
	{ "\\gui_buttonright",  FL_RIGHT_BCOL },
	{ "\\gui_buttontop",    FL_TOP_BCOL },
	{ "\\gui_inactive",     FL_INACTIVE },
	{ "\\gui_pointer",      FL_FREE_COL16 },
	{ "\\gui_push_button",  FL_YELLOW },
	{ "\\gui_selected",     FL_MCOL },	
	{ "\\gui_text",         FL_BLACK }
};


const int xformCount = sizeof(xformTags) / sizeof(keyword_item);

} // namespace anon


bool XformsColor::read(string const & filename)
{
	LyXLex lexrc(xformTags, xformCount);
	if (!lexrc.setFile(filename))
		return false;

	while (lexrc.isOK()) {
		int const le = lexrc.lex();

		switch (le) {
		case LyXLex::LEX_UNDEF:
			lexrc.printError("Unknown tag `$$Token'");
			continue; 
		case LyXLex::LEX_FEOF:
			continue;
		default: break;
		}

		string const tag = lexrc.getString();

		RGBColor col;

		if (!lexrc.next()) break;
		col.r = lexrc.getInteger();

		if (!lexrc.next()) break;
		col.g = lexrc.getInteger();

		if (!lexrc.next()) break;
		col.b = lexrc.getInteger();

		fl_mapcolor(le, col.r, col.g, col.b);

		if (tag == "\\gui_pointer") {
			fl_set_cursor_color(FL_DEFAULT_CURSOR,
					    FL_FREE_COL16, FL_WHITE);
			fl_set_cursor_color(XC_question_arrow,
					    FL_FREE_COL16, FL_WHITE);
		}
	}
		
	return true;
}


bool XformsColor::write(string const & filename)
{
	ofstream os(filename.c_str());
	if (!os)
		return false;

	os << "### This file is part of\n"
	   << "### ========================================================\n"
	   << "###          LyX, The Document Processor\n"
	   << "###\n"
	   << "###          Copyright 1995 Matthias Ettrich\n"
	   << "###          Copyright 1995-2002 The LyX Team.\n"
	   << "###\n"
	   << "### ========================================================\n"
	   << "\n"
	   << "# This file is written by LyX, if you want to make your own\n"
	   << "# modifications you should do them from inside LyX and save\n"
	   << "\n";

	for (int i = 0; i < xformCount; ++i) {
		string const tag  = xformTags[i].tag;
		int const colorID = xformTags[i].code;
		RGBColor color;

		fl_getmcolor(colorID, &color.r, &color.g, &color.b);

		os << tag << " "
		   << color.r << " " << color.g << " " << color.b << "\n";
	}

	return true;
}


string  RWInfo::error_message;

bool RWInfo::WriteableDir(string const & name)
{
	error_message.erase();

	if (!AbsolutePath(name)) {
		error_message = N_("The absolute path is required.");
		return false;
	}

	FileInfo const tp(name);
	if (!tp.isOK() || !tp.isDir()) {
		error_message = N_("Directory does not exist.");
		return false;
	}

	if (!tp.writable()) {
		error_message = N_("Cannot write to this directory.");
		return false;
	}

	return true;
}


bool RWInfo::ReadableDir(string const & name)
{
	error_message.erase();

	if (!AbsolutePath(name)) {
		error_message = N_("The absolute path is required.");
		return false;
	}

	FileInfo const tp(name);
	if (!tp.isOK() || !tp.isDir()) {
		error_message = N_("Directory does not exist.");
		return false;
	}

	if (!tp.readable()) {
		error_message = N_("Cannot read this directory.");
		return false;
	}

	return true;
}


bool RWInfo::WriteableFile(string const & name)
{
	// A writeable file is either:
	// * An existing file to which we have write access, or
	// * A file that doesn't yet exist but that would exist in a writeable
	//   directory.

	error_message.erase();

	if (name.empty()) {
		error_message = N_("No file input.");
		return false;
	}

	string const dir = OnlyPath(name);
	if (!AbsolutePath(dir)) {
		error_message = N_("The absolute path is required.");
		return false;
	}

	FileInfo d(name);

	if (!d.isOK() || !d.isDir()) {
		d.newFile(dir);
	}

	if (!d.isOK() || !d.isDir()) {
		error_message = N_("Directory does not exist.");
		return false;
	}
	
	if (!d.writable()) {
		error_message = N_("Cannot write to this directory.");
		return false;
	}

	FileInfo f(name);
	if (dir == name || (f.isOK() && f.isDir())) {
		error_message = N_("A file is required, not a directory.");
		return false;
	}

	if (f.isOK() && f.exist() && !f.writable()) {
		error_message = N_("Cannot write to this file.");
		return false;
	}
	
	return true;
}


bool RWInfo::ReadableFile(string const & name)
{
	error_message.erase();

	if (name.empty()) {
		error_message = N_("No file input.");
		return false;
	}

	string const dir = OnlyPath(name);
	if (!AbsolutePath(dir)) {
		error_message = N_("The absolute path is required.");
		return false;
	}

	FileInfo d(name);

	if (!d.isOK() && !d.isDir()) {
		d.newFile(dir);
	}

	if (!d.isOK() || !d.isDir()) {
		error_message = N_("Directory does not exist.");
		return false;
	}
	
	if (!d.readable()) {
		error_message = N_("Cannot read from this directory.");
		return false;
	}

	FileInfo f(name);
	if (dir == name || (f.isOK() && f.isDir())) {
		error_message = N_("A file is required, not a directory.");
		return false;
	}

	if (!f.exist()) {
		error_message = N_("File does not exist.");
		return false;
	}
	
	if (!f.readable()) {
		error_message = N_("Cannot read from this file.");
		return false;
	}

	return true;
}
