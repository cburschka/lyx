/** Collection of some useful xform helper functions
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
#include "gettext.h"

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

	
// Take a string and add breaks so that it fits into a desired label width, w
string formatted(string const & sin, int w, int size, int style)
{
	// FIX: Q: Why cant this be done by a one pass algo? (Lgb)

	string sout;
	if (sin.empty()) return sout;

	// break sin up into a vector of individual words
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
	if (!word.empty() ) sentence.push_back(word);

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

		if (!line_plus_word.empty() ) line_plus_word += ' ';
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
	{ "\\gui_background", FL_COL1 },
	{ "\\gui_buttonbottom", FL_BOTTOM_BCOL },
	{ "\\gui_buttonleft", FL_LEFT_BCOL },
	{ "\\gui_buttonright", FL_RIGHT_BCOL },
	{ "\\gui_buttontop", FL_TOP_BCOL },
	{ "\\gui_inactive", FL_INACTIVE },
	{ "\\gui_push_button", FL_YELLOW },
	{ "\\gui_selected", FL_MCOL },	
	{ "\\gui_text", FL_BLACK }
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

		RGBColor col;

		if (!lexrc.next()) break;
		col.r = lexrc.getInteger();

		if (!lexrc.next()) break;
		col.g = lexrc.getInteger();

		if (!lexrc.next()) break;
		col.b = lexrc.getInteger();

		fl_mapcolor(le, col.r, col.g, col.b);
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
	   << "###          Copyright 1995-2001 The LyX Team.\n"
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
	if (!tp.isDir()) {
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
	if (!tp.isDir()) {
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
	if (!d.isDir()) {
		d.newFile(dir);
	}

	if (!d.isDir()) {
		error_message = N_("Directory does not exist.");
		return false;
	}
	
	if (!d.writable()) {
		error_message = N_("Cannot write to this directory.");
		return false;
	}

	FileInfo f(name);
	if (dir == name || f.isDir()) {
		error_message = N_("A file is required, not a directory.");
		return false;
	}

	if (f.exist() && !f.writable()) {
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
	if (!d.isDir()) {
		d.newFile(dir);
	}

	if (!d.isDir()) {
		error_message = N_("Directory does not exist.");
		return false;
	}
	
	if (!d.readable()) {
		error_message = N_("Cannot read from this directory.");
		return false;
	}

	FileInfo f(name);
	if (dir == name || f.isDir()) {
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
