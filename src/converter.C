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
#include <algorithm> // sort()

#include "converter.h"
#include "lyxrc.h"
#include "support/syscall.h"
#include "support/path.h"
#include "buffer.h"
#include "bufferview_funcs.h"
#include "LaTeX.h"
#include "LyXView.h"
#include "minibuffer.h"
#include "lyx_gui_misc.h"
#include "lyx_cb.h" // ShowMessage()

using std::map;
using std::vector;
using std::queue;
using std::pair;
using std::sort;
using std::endl;

//////////////////////////////////////////////////////////////////////////////

map<string, Format> Formats::formats;
vector<Command> Converter::commands;
string Converter::latex_command;

inline
string const add_options(string const & command, string const & options)
{
	string head;
	string tail = split(command, head, ' ');
	return head + ' ' + options + ' ' + tail;
}

//////////////////////////////////////////////////////////////////////////////

Format::Format(string const & n)
	: name(n), in_degree(0)
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

	string command2 = subst(command, "$$FName", "'$$FName'");
	if (!contains(command,"$$FName"))
		command2 += " '$$FName'";

	Add(name);
	GetFormat(name)->viewer = command2;
}


bool Formats::View(Buffer * buffer, string const & filename)
{
	if (filename.empty())
		return false;

	string extension = GetExtension(filename);
	Format * format = GetFormat(extension);
	if (!format || format->viewer.empty()) {
		WriteAlert(_("Can not view file"),
			   _("No information for viewing ")
			   + Formats::PrettyName(extension));
			   return false;
	}

	string command = format->viewer;

	if (extension == "dvi" &&
	    !lyxrc.view_dvi_paper_option.empty()) {
		string options = lyxrc.view_dvi_paper_option;
		options += " " + Converter::dvi_papersize(buffer);
		if (buffer->params.orientation 
		    == BufferParams::ORIENTATION_LANDSCAPE)
			options += 'r';
		command = add_options(command, options);
        }

	string command2 = subst(command, "$$FName", OnlyFilename(filename));
	lyxerr << "Executing command: " << command2 << endl;
	ShowMessage(buffer, _("Executing command:"), command2);

	command = subst(command, "$$FName", filename);
	Systemcalls one;
	int res = one.startscript(Systemcalls::SystemDontWait, command);

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


string const Formats::PrettyName(string const & name)
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
	if (command == "none")
		return;

	string command2 = 
		   subst(command, "$$FName", "'$$FName'");
	command2 = subst(command2, "$$BaseName", "'$$BaseName'");
	command2 = subst(command2, "$$OutName", "'$$OutName'");
	Command Com(from, to, command2);

	if (from == "tex" &&
	    (to == "dvi" ||
	     (to == "pdf" && latex_command.empty())))
		latex_command = command2;

	// Read the flags
	string flag_name,flag_value;
	string flag_list(flags);
	while (!flag_list.empty()) {
		flag_list = split(flag_list, flag_value,',');
		flag_value = split(flag_value, flag_name, '=');
		if (flag_name == "originaldir")
			Com.original_dir = true;
		else if (flag_name == "needaux")
			Com.need_aux = true;
		else if (flag_name == "resultdir")
			Com.result_dir = (flag_value.empty())
				? "$$BaseName" : flag_value;
		else if (flag_name == "resultfile")
			Com.result_file = flag_value;
	}
	if (!Com.result_dir.empty() && Com.result_file.empty())
		Com.result_file = "index." + to;

	for (vector<Command>::iterator it = commands.begin();
	     it != commands.end(); ++it)
		if ((*it).from == from && (*it).to == to) {
			*it = Com;
			return;
		}
	commands.push_back(Com);
	Formats::Add(from);
	Formats::Add(to);
	++Formats::GetFormat(to)->in_degree;
}


vector< pair<string,string> > const
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
			prettyname  += _(" (using ") + tmp + ")";	
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


bool Converter::Convert(Buffer * buffer, string const & from_file,
			string const & to_file, string const & using_format,
			string * view_file)
{
	if (view_file)
		*view_file = to_file;

	string from_format = GetExtension(from_file);
	string to_format = GetExtension(to_file);
	if (from_format == to_format)
		if (from_file != to_file)
			return lyx::rename(from_file, to_file);
		else
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
		if ((*it).to == to_format &&
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

	vector< vector<Command>::iterator > S;
	while (it != commands.end()) {
		S.push_back(it);
		it = (*it).previous;
	}

	string path = OnlyPath(from_file);
	Path p(path);

	bool run_latex = false;
	string from_base = ChangeExtension(from_file, "");
	string to_base = ChangeExtension(to_file, "");
	string infile;
	string outfile = from_file;
	for (vector< vector<Command>::iterator >::reverse_iterator rit =
		     S.rbegin(); rit != S.rend(); ++rit) {
		it = *rit;
		lyxerr << "Converting from  "
		       << (*it).from << " to " << (*it).to << endl;
		infile = outfile;
		outfile = (*it).result_dir.empty()
			? ChangeExtension(from_file, (*it).to)
			: AddName(subst((*it).result_dir,
					"$$BaseName", from_base),
				  subst((*it).result_file,
					"$$BaseName", OnlyFilename(from_base)));

		if ((*it).from == "tex" &&
		    ( (*it).to == "dvi" || (*it).to == "pdf") ) {
			lyxrc.pdf_mode = (*it).to == "pdf";
			lyxerr << "Running " << (*it).command << endl;
			run_latex = true;
			if (!runLaTeX(buffer, (*it).command))
				return false;
		} else {
			if ((*it).need_aux && !run_latex
			    && !latex_command.empty()) {
				lyxerr << "Running " << latex_command 
				       << " to update aux file"<<  endl;
				runLaTeX(buffer, latex_command);
			}

			string infile2 = ((*it).original_dir)
				? infile : MakeRelPath(infile, path);
			string outfile2 = ((*it).original_dir)
				? outfile : MakeRelPath(outfile, path);

			string command = (*it).command;
			command = subst(command, "$$FName", infile2);
			command = subst(command, "$$BaseName", from_base);
			command = subst(command, "$$OutName", outfile2);

			if ((*it).from == "dvi" && (*it).to == "ps")
				command = add_options(command,
						      dvips_options(buffer));

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

	if (!(*it).result_dir.empty()) {
		if (view_file)
			*view_file = AddName(subst((*it).result_dir,
						   "$$BaseName", to_base),
					     subst((*it).result_file,
						   "$$BaseName", OnlyFilename(to_base)));
		if (from_base != to_base) {
			string from = subst((*it).result_dir,
					    "$$BaseName", from_base);
			string to = subst((*it).result_dir,
					  "$$BaseName", to_base);
			return lyx::rename(from, to);
		}

	} else if (outfile != to_file)
		if ((*it).from == "tex" &&
		    ( (*it).to == "dvi" || (*it).to == "pdf") )
			return lyx::copy(outfile, to_file);
		else
			return lyx::rename(outfile, to_file);

        return true;
}


string const Converter::SplitFormat(string const & str, string & format)
{
	string using_format = split(str, format, ':');
	if (format.empty())
		format = "dvi";
	return using_format;
}


bool Converter::runLaTeX(Buffer * buffer, string const & command)
{
	
	BufferView * bv = buffer->getUser();
	string name = buffer->getLatexName();
	bool need_redraw = false;

	if (bv) {
		ProhibitInput(bv);
		bv->owner()->getMiniBuffer()->Set(_("Running LaTeX..."));
		// Remove all error insets
		need_redraw = bv->removeAutoInsets();
	}


	// do the LaTex run(s)
	TeXErrors terr;
	LaTeX latex(command, name, buffer->filepath);
	int result = latex.run(terr,
			       bv ? bv->owner()->getMiniBuffer() : 0);
	

	if (bv) {
		if ((result & LaTeX::ERRORS)) {
			// Insert all errors as errors boxes
			bv->insertErrors(terr);
			need_redraw = true;
		}

		// if we removed error insets before we ran LaTeX or if we inserted
		// error insets after we ran LaTeX this must be run:
		if (need_redraw) {
			bv->redraw();
			bv->fitCursor(bv->text);
		}
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

	if (bv)
		AllowInput(bv);
 
        return (result & (LaTeX::NO_LOGFILE | LaTeX::ERRORS)) == 0;

}


string const Converter::dvi_papersize(Buffer const * buffer)
{
	char real_papersize = buffer->params.papersize;
	if (real_papersize == BufferParams::PAPER_DEFAULT)
		real_papersize = lyxrc.default_papersize;

	switch (real_papersize) {
	case BufferParams::PAPER_A3PAPER:
		return "a3";
	case BufferParams::PAPER_A4PAPER:
		return "a4";
	case BufferParams::PAPER_A5PAPER:
		return "a5";
	case BufferParams::PAPER_B5PAPER:
		return "b5";
	case BufferParams::PAPER_EXECUTIVEPAPER:
		return "foolscap";
	case BufferParams::PAPER_LEGALPAPER:
		return "legal";
	case BufferParams::PAPER_USLETTER:
	default:
		return "us";
	}
}


string const Converter::dvips_options(Buffer const * buffer)
{
	string result;
	if (buffer->params.use_geometry
	    && buffer->params.papersize2 == BufferParams::VM_PAPER_CUSTOM
	    && !lyxrc.print_paper_dimension_flag.empty()
	    && !buffer->params.paperwidth.empty()
	    && !buffer->params.paperheight.empty()) {
		// using a custom papersize
		result = lyxrc.print_paper_dimension_flag;
		result += ' ' + buffer->params.paperwidth;
		result += ',' + buffer->params.paperheight;
	} else {
		string paper_option = dvi_papersize(buffer);
		if (paper_option == "us")
			paper_option = "letter";
		if (paper_option != "letter" ||
		    buffer->params.orientation != BufferParams::ORIENTATION_LANDSCAPE) {
			// dvips won't accept -t letter -t landscape.  In all other
			// cases, include the paper size explicitly.
			result = lyxrc.print_paper_flag;
			result += ' ' + paper_option;
		}
	}
	if (buffer->params.orientation == BufferParams::ORIENTATION_LANDSCAPE)
		result += ' ' + lyxrc.print_landscape_flag;
	return result;
}
