/**
 * \file xforms_helpers.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xforms_helpers.h"

#include "lyxlex.h"
#include "gettext.h"
#include "lyxlength.h"
#include "lyxgluelength.h"

#include "support/LAssert.h"
#include "support/FileInfo.h"
#include "support/filetools.h"
#include "support/lstrings.h" // frontStrip, strip

#include <algorithm>
#include <fstream>
#include <vector>
#include FORMS_H_LOCATION

using std::ofstream;
using std::pair;
using std::vector;

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


// Given an fl_choice or an fl_browser, create a vector of its entries
vector<string> const getVector(FL_OBJECT * ob)
{
	vector <string> vec;

	switch (ob->objclass) {
	case FL_CHOICE:
		for(int i = 0; i < fl_get_choice_maxitems(ob); ++i) {
			string const text = fl_get_choice_item_text(ob, i+1);
			vec.push_back(trim(text));
		}
		break;
	case FL_BROWSER:
		for(int i = 0; i < fl_get_browser_maxline(ob); ++i) {
			string const text = fl_get_browser_line(ob, i+1);
			vec.push_back(trim(text));
		}
		break;
	default:
		lyx::Assert(0);
	}

	return vec;
}


///
string const getString(FL_OBJECT * ob, int line)
{
	char const * tmp = 0;

	switch (ob->objclass) {
	case FL_INPUT:
		lyx::Assert(line == -1);
		tmp = fl_get_input(ob);
		break;
	case FL_BROWSER:
		if (line == -1)
			line = fl_get_browser(ob);

		if (line >= 1 && line <= fl_get_browser_maxline(ob))
			tmp = fl_get_browser_line(ob, line);
		break;

	case FL_CHOICE:
		if (line == -1)
			line = fl_get_choice(ob);

		if (line >= 1 && line <= fl_get_choice_maxitems(ob))
			tmp = fl_get_choice_item_text(ob, line);
		break;

	default:
		lyx::Assert(0);
	}

	return (tmp) ? trim(tmp) : string();
}

string getLengthFromWidgets(FL_OBJECT * input, FL_OBJECT * choice)
{
	// Paranoia check
	lyx::Assert(input  && input->objclass  == FL_INPUT &&
		    choice && choice->objclass == FL_CHOICE);

	string const length = trim(fl_get_input(input));
	if (length.empty())
		return string();

	//don't return unit-from-choice if the input(field) contains a unit
	if (isValidGlueLength(length))
		return length;

	string unit = trim(fl_get_choice_text(choice));
	unit = subst(unit, "%%", "%");

	return length + unit;
}


void updateWidgetsFromLengthString(FL_OBJECT * input, FL_OBJECT * choice,
				   string const & str,
				   string const & default_unit)
{
	// use input field only for gluelengths
	if (!isValidLength(str) && !isStrDbl(str)) {
		fl_set_input(input, str.c_str());
		// we assume that "default_unit" is in the choice as "we"
		// have control over that!
		// No need to check for its presence in the choice, therefore.
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

		// Set the choice to the desired unit, if present in the choice.
		// Else set the choice to the default unit.
		string const unit = subst(stringFromUnit(len.unit()),"%","%%");

		vector<string> const vec = getVector(choice);
		vector<string>::const_iterator it =
			std::find(vec.begin(), vec.end(), unit);
		if (it != vec.end()) {
			fl_set_choice_text(choice, unit.c_str());
		} else {
			fl_set_choice_text(choice, default_unit.c_str());
		}
	}
}


// Take a string and add breaks so that it fits into a desired label width, w
string formatted(string const & sin, int w, int size, int style)
{
	string sout;
	if (sin.empty()) return sout;

	string::size_type curpos = 0;
	string line;
	for(;;) {
		string::size_type const nxtpos1 = sin.find(' ',  curpos);
		string::size_type const nxtpos2 = sin.find('\n', curpos);
		string::size_type const nxtpos = std::min(nxtpos1, nxtpos2);

		string const word = nxtpos == string::npos ?
			sin.substr(curpos) : sin.substr(curpos, nxtpos-curpos);

		bool const newline = (nxtpos2 != string::npos &&
				      nxtpos2 < nxtpos1);

		string const line_plus_word =
			line.empty() ? word : line + ' ' + word;

		int const length =
			fl_get_string_width(style, size,
					    line_plus_word.c_str(),
					    int(line_plus_word.length()));

		if (length >= w) {
			sout += line + '\n';
			if (newline) {
				sout += word + '\n';
				line.erase();
			} else {
				line = word;
			}

		} else if (newline) {
			sout += line_plus_word + '\n';
			line.erase();

		} else {
			if (!line.empty())
				line += ' ';
			line += word;
		}

		if (nxtpos == string::npos) {
			if (!line.empty())
				sout += line;
			break;
		}

		curpos = nxtpos+1;
	}

	return sout;
}


void setCursorColor(int color)
{
	fl_set_cursor_color(FL_DEFAULT_CURSOR, color, FL_WHITE);
	fl_set_cursor_color(XC_xterm,          color, FL_WHITE);
	fl_set_cursor_color(XC_watch,          color, FL_WHITE);
	fl_set_cursor_color(XC_sb_right_arrow, color, FL_WHITE);
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
			setCursorColor(FL_FREE_COL16);
		}
	}

	return true;
}


bool XformsColor::write(string const & filename)
{
	ofstream os(filename.c_str());
	if (!os)
		return false;

	os << "###"
	   << "### file " << filename << "\n\n"
	   << "### This file is written by LyX, if you want to make your own\n"
	   << "### modifications you should do them from inside LyX and save\n"
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
		error_message = _("The absolute path is required.");
		return false;
	}

	FileInfo const tp(name);
	if (!tp.isOK() || !tp.isDir()) {
		error_message = _("Directory does not exist.");
		return false;
	}

	if (!tp.writable()) {
		error_message = _("Cannot write to this directory.");
		return false;
	}

	return true;
}


bool RWInfo::ReadableDir(string const & name)
{
	error_message.erase();

	if (!AbsolutePath(name)) {
		error_message = _("The absolute path is required.");
		return false;
	}

	FileInfo const tp(name);
	if (!tp.isOK() || !tp.isDir()) {
		error_message = _("Directory does not exist.");
		return false;
	}

	if (!tp.readable()) {
		error_message = _("Cannot read this directory.");
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
		error_message = _("No file input.");
		return false;
	}

	string const dir = OnlyPath(name);
	if (!AbsolutePath(dir)) {
		error_message = _("The absolute path is required.");
		return false;
	}

	FileInfo d(name);

	if (!d.isOK() || !d.isDir()) {
		d.newFile(dir);
	}

	if (!d.isOK() || !d.isDir()) {
		error_message = _("Directory does not exist.");
		return false;
	}

	if (!d.writable()) {
		error_message = _("Cannot write to this directory.");
		return false;
	}

	FileInfo f(name);
	if (dir == name || (f.isOK() && f.isDir())) {
		error_message = _("A file is required, not a directory.");
		return false;
	}

	if (f.isOK() && f.exist() && !f.writable()) {
		error_message = _("Cannot write to this file.");
		return false;
	}

	return true;
}


bool RWInfo::ReadableFile(string const & name)
{
	error_message.erase();

	if (name.empty()) {
		error_message = _("No file input.");
		return false;
	}

	string const dir = OnlyPath(name);
	if (!AbsolutePath(dir)) {
		error_message = _("The absolute path is required.");
		return false;
	}

	FileInfo d(name);

	if (!d.isOK() && !d.isDir()) {
		d.newFile(dir);
	}

	if (!d.isOK() || !d.isDir()) {
		error_message = _("Directory does not exist.");
		return false;
	}

	if (!d.readable()) {
		error_message = _("Cannot read from this directory.");
		return false;
	}

	FileInfo f(name);
	if (dir == name || (f.isOK() && f.isDir())) {
		error_message = _("A file is required, not a directory.");
		return false;
	}

	if (!f.exist()) {
		error_message = _("File does not exist.");
		return false;
	}

	if (!f.readable()) {
		error_message = _("Cannot read from this file.");
		return false;
	}

	return true;
}
