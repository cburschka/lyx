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
#include "filedlg.h" // LyXFileDlg
#include "support/FileInfo.h"
#include "support/filetools.h"
#include "lyx_gui_misc.h" // WriteAlert
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
	string sout;
	if (sin.empty()) return sout;

	// break sin up into a vector of individual words
	vector<string> sentence;
	string word;
	for (string::const_iterator sit = sin.begin();
	     sit != sin.end(); ++sit) {
		if ((*sit) == ' ' || (*sit) == '\n') {
			sentence.push_back(word);
			word.erase();
		} else {
			word += (*sit);
		}
	}
	// Flush remaining contents of word
	if (!word.empty() ) sentence.push_back(word);

	string line, l1;
	for (vector<string>::const_iterator vit = sentence.begin();
	     vit != sentence.end(); ++vit) {
		if (!l1.empty() ) l1 += ' ';
		l1 += (*vit);
		int length = fl_get_string_width(style, size, l1.c_str(),
						 int(l1.length()));
		if (length >= w) {
			if (!sout.empty() ) sout += '\n';
			sout += line;
			l1 = (*vit);
		}

		line = l1;
	}
	// Flush remaining contents of line
	if (!line.empty()) {
		if (!sout.empty() ) sout += '\n';
		sout += line;
	}
	
	return sout;
}


string const browseFile(string const & filename,
			string const & title,
			string const & pattern, 
			pair<string,string> const & dir1,
			pair<string,string> const & dir2)
{
	string lastPath = ".";
	if (!filename.empty()) lastPath = OnlyPath(filename);

	LyXFileDlg fileDlg;

	if (!dir1.second.empty()) {
		FileInfo fileInfo(dir1.second);
		if (fileInfo.isOK() && fileInfo.isDir())
			fileDlg.SetButton(0, _(dir1.first), dir1.second);
	}

	if (!dir2.second.empty()) {
		FileInfo fileInfo(dir2.second);
		if (fileInfo.isOK() && fileInfo.isDir())
		    fileDlg.SetButton(1, _(dir2.first), dir2.second);
	}

	bool error = false;
	string buf;
	do {
		string p = fileDlg.Select(_(title),
		                          lastPath,
		                          pattern, OnlyFilename(filename));

		if (p.empty()) return p;

		lastPath = OnlyPath(p);

		if (p.find_first_of("#~$% ") != string::npos) {
			WriteAlert(_("Filename can't contain any "
			             "of these characters:"),
			           _("space, '#', '~', '$' or '%'."));
			error = true;
		} else {
			error = false;
			buf = p;
		}
	} while (error);

	return buf;
}


// sorted by hand to prevent LyXLex from complaining on read().
static
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


static const int xformCount = sizeof(xformTags) / sizeof(keyword_item);


bool XformsColor::read(string const & filename)
{
	LyXLex lexrc(xformTags, xformCount);
	if (!lexrc.setFile(filename))
		return false;

	while (lexrc.IsOK()) {
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
		col.r = lexrc.GetInteger();

		if (!lexrc.next()) break;
		col.g = lexrc.GetInteger();

		if (!lexrc.next()) break;
		col.b = lexrc.GetInteger();

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
	   << "###          Copyright 1995-2000 The LyX Team.\n"
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
