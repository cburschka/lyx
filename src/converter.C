/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <queue>
#include <stack>
#include <algorithm>
#include <stdio.h>

#include "converter.h"
#include "lyxrc.h"
#include "support/syscall.h"
#include "support/path.h"
#include "debug.h"
#include "buffer.h"

#include "bufferview_funcs.h"
#include "LaTeX.h"
#include "LyXView.h"
#include "minibuffer.h"
#include "lyx_gui_misc.h"

using std::map;
using std::vector;
using std::queue;
using std::stack;

extern void ShowMessage(Buffer * buf,
		 string const & msg1,
		 string const & msg2 = string(),
		 string const & msg3 = string(), int delay = 6);

//////////////////////////////////////////////////////////////////////////////

map<string, Format> Formats::formats;
vector<Command> Converter::commands;

//////////////////////////////////////////////////////////////////////////////

Format::Format(string const & n) : name(n), in_degree(0)
{
	struct Item {
		char const * name;
		char const * prettyname;
	};
	Item items[] = {
		{ "tex", "LaTeX" },
		{ "dvi", "DVI" },
		{ "ps", "PostScript" },
		{ "txt", "Ascii" },
		{ "html", "HTML" },
		{ "pdf", "PDF" },
		{ 0, 0}
	};

	prettyname = n;
	for (int i = 0; items[i].name != 0; ++i)
		if (items[i].name == n) {
			prettyname = items[i].prettyname;
			break;
		}
}

void Formats::Add(string const & name)
{
	if (formats.find(name) == formats.end())
		formats[name] = Format(name);
}

void Formats::SetViewer(string const & name, string const & command)
{
	Add(name);
	Format * f = GetFormat(name);
	if (!f->viewer.empty())
		lyxerr << "Error: a viewer for " << name
		       << " is already defined!" << endl;
	else
		f->viewer = command;
}


bool Formats::View(string const & filename)
{
	string extension = GetExtension(filename);
	Format * format = GetFormat(extension);
	if (!format || format->viewer.empty()) {
		WriteAlert(_("Can not view file"),
			   _("No information for viewing ")
			   + Formats::PrettyName(extension));
			   return false;
	}

	string command = format->viewer;
	command = subst(command, "$$FName", filename);
#ifndef __EMX__
	command += " &";
#else
	// OS/2 cmd.exe has another use for '&'
	// This is not NLS safe, but it's OK, I think.
	string sh = OnlyFilename(GetEnvPath("EMXSHELL"));
	if (sh.empty()) {
		// COMSPEC is set, unless user unsets 
		sh = OnlyFilename(GetEnvPath("COMSPEC"));
		if (sh.empty())
			sh = "cmd.exe";
	}
	sh = lowercase(sh);
	if (contains(sh, "cmd.exe")
	    || contains(sh, "4os2.exe"))
		command = "start /min/n " + command;
	else
		command += " &";
#endif
	lyxerr << "Executing command: " << command << endl;
	//ShowMessage(buffer, _("Executing command:"), command);
	Systemcalls one;
	int res = one.startscript(Systemcalls::System, command);

	if (res) {
		WriteAlert(_("Can not view file"),
			   _("Error while executing"),
			   command.substr(0, 50));
		return false;
	}
	return true;
}


Format * Formats::GetFormat(string const & name)
{
	map<string, Format>::iterator it = formats.find(name);
	if (it != formats.end())
		return &(*it).second;
	else
		return 0;
}

string Formats::PrettyName(string const & name)
{

	string format;
	Converter::SplitFormat(name, format);
	Format * f = GetFormat(format);
	if (f)
		return f->prettyname;
	else
		return format;
}

//////////////////////////////////////////////////////////////////////////////
void Converter::Add(string const & from, string const & to,
		    string const & command, string const & flags)
{
	for (vector<Command>::const_iterator cit = commands.begin();
	     cit != commands.end(); ++cit)
		if ((*cit).from == from && (*cit).to == to) {
			lyxerr << "Error: Convertor from " << from
			       << " to " << to
			       << " already exists!" << endl;
			return;
		}
	bool original_dir = flags == "origdir";
	commands.push_back(Command(from, to, command, original_dir));

	Formats::Add(from);
	Formats::Add(to);
	++Formats::GetFormat(to)->in_degree;
}

vector< pair<string,string> > 
Converter::GetReachable(string const & from, bool only_viewable)
{
	vector< pair<string,string> > result;
	Format * format = Formats::GetFormat(from);
	if (!format)
		return result;

	int sort_start = 0;
	if (!only_viewable || !format->viewer.empty()) {
		result.push_back(pair<string,string>(from, format->prettyname));
		sort_start = 1;
	}

	queue< vector<Command>::iterator > Q;
	for (vector<Command>::iterator it = commands.begin();
	     it != commands.end(); ++it)
		if ((*it).from == from) {
			Q.push(it);
			(*it).visited = true;
		} else
			(*it).visited = false;

	while (!Q.empty()) {
		vector<Command>::iterator it = Q.front();
		format = Formats::GetFormat((*it).to);
		string name = format->name;
		string prettyname = format->prettyname;
		if (format->in_degree > 1) {
			name += ":" + (*it).from;
			string tmp;
			split((*it).command, tmp, ' ');
			prettyname  += _("(using ") + tmp + ")";	
		}
		if (!only_viewable || !format->viewer.empty())
			result.push_back(pair<string,string>(name, prettyname));
		Q.pop();
		for (vector<Command>::iterator it2 = commands.begin();
		     it2 != commands.end(); ++it2)
			if (!(*it2).visited && (*it).to == (*it2).from) {
				Q.push(it2);
				(*it2).visited = true;
			}
	}

	sort(result.begin() + sort_start, result.end());
	return result;
}


bool Converter::convert(Buffer * buffer, string const & from_file,
			string const & to_format)

{
	string using_format, format;
	using_format = SplitFormat(to_format, format);
	string from_format = GetExtension(from_file);
	if (from_format == format)
		return true;

	queue< vector<Command>::iterator > Q;
	for (vector<Command>::iterator it = commands.begin();
	     it != commands.end(); ++it)
		if ((*it).from == from_format) {
			Q.push(it);
			(*it).visited = true;
			(*it).previous = commands.end();
		} else
			(*it).visited = false;

	if (Q.empty()) {
		WriteAlert(_("Can not convert file"),
			   ("Unknown format ") + from_format);
		return false;
	}

	bool found = false;
	vector<Command>::iterator it;
	while (!Q.empty()) {
		it = Q.front();
		if ((*it).to == format &&
		    (using_format.empty() || using_format == (*it).from)) {
			found = true;
			break;
		}
		Q.pop();
		for (vector<Command>::iterator it2 = commands.begin();
		     it2 != commands.end(); ++it2)
			if (!(*it2).visited && (*it).to == (*it2).from) {
				Q.push(it2);
				(*it2).visited = true;
				(*it2).previous = it;
			}
	}

	if (!found) {
		WriteAlert(_("Can not convert file"),
			   _("No information for converting from ")
			   + Formats::PrettyName(from_format) + _(" to ")
			   + Formats::PrettyName(to_format));
		return false;
	}

	stack< vector<Command>::iterator > S;
	while (it != commands.end()) {
		S.push(it);
		it = (*it).previous;
	}

	//Path p(OnlyPath(buffer->fileName()));
	Path p(OnlyPath(from_file));

	string basename = ChangeExtension(from_file, "");
	while (!S.empty()) {
		it = S.top();
		S.pop();
		lyxerr << "Converting from  "
		       << (*it).from << " to " << (*it).to << endl;

		if ((*it).from == "tex" &&
		    ( (*it).to == "dvi" || (*it).to == "pdf") ) {
			lyxrc.pdf_mode = (*it).to == "pdf";
			if (!runLaTeX(buffer, (*it).command))
				return false;
		} else {
			string infile = ChangeExtension(from_file, (*it).from);
			if (!(*it).original_dir)
				infile = OnlyFilename(infile);
			string outfile = ChangeExtension(infile, (*it).to);

			string command = (*it).command;
			command = subst(command, "$$FName", infile);
			command = subst(command, "$$BaseName", basename);
			command = subst(command, "$$OutName", outfile);
			lyxerr << "Calling " << command << endl;
			ShowMessage(buffer, _("Executing command:"), command);
			
			Systemcalls one;
			int res;
			if ((*it).original_dir) {
				Path p(buffer->filepath);
				res = one.startscript(Systemcalls::System, command);
			} else
				res = one.startscript(Systemcalls::System, command);
			if (res) {
				WriteAlert(_("Can not convert file"),
					   "Error while executing",
					   command.substr(0, 50));
				return false;
			}
		}
	}

        return true;
}


string Converter::SplitFormat(string const & str, string & format)
{
	string using_format = split(str, format, ':');
	if (format.empty())
		format = "dvi";
	return using_format;
}

bool Converter::runLaTeX(Buffer * buffer, string const & command)
{
	
	BufferView * bv = buffer->getUser();

	if (!bv->text) return 0;

	ProhibitInput(bv);

	string name = buffer->getLatexName();

	bv->owner()->getMiniBuffer()->Set(_("Running LaTeX..."));   

	// Remove all error insets
	bool a = bv->removeAutoInsets();

	// do the LaTex run(s)
	TeXErrors terr;
	LaTeX latex(command, name, buffer->filepath);
	int result = latex.run(terr,
			    bv->owner()->getMiniBuffer()); // running latex

	if ((result & LaTeX::ERRORS)) {
		// Insert all errors as errors boxes
		bv->insertErrors(terr);
	}

	// if we removed error insets before we ran LaTeX or if we inserted
	// error insets after we ran LaTeX this must be run:
        if (a || (result & LaTeX::ERRORS)){
                bv->redraw();
                bv->fitCursor();
                //bv->updateScrollbar();
        }

	// check return value from latex.run().
	if ((result & LaTeX::NO_LOGFILE)) {
		WriteAlert(_("LaTeX did not work!"),
			   _("Missing log file:"), name);
	} else if ((result & LaTeX::ERRORS)) {
		int num_errors = latex.getNumErrors();
		string s;
		string t;
		if (num_errors == 1) {
			s = _("One error detected");
			t = _("You should try to fix it.");
		} else {
			s = tostr(num_errors);
			s += _(" errors detected.");
			t = _("You should try to fix them.");
		}
		WriteAlert(_("There were errors during the LaTeX run."),
			   s, t);
	}
        AllowInput(bv);
 
        return (result & (LaTeX::NO_LOGFILE | LaTeX::ERRORS)) == 0;

}

