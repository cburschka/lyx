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

#include "converter.h"
#include "lyxrc.h"
#include "support/syscall.h"
#include "support/path.h"
#include "support/filetools.h"
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
using std::endl;
using std::find;
using std::find_if;

//////////////////////////////////////////////////////////////////////////////

vector<Command> Converter::commands;
string Converter::latex_command;

static inline
string const add_options(string const & command, string const & options)
{
	string head;
	string const tail = split(command, head, ' ');
	return head + ' ' + options + ' ' + tail;
}

//////////////////////////////////////////////////////////////////////////////


bool Format::dummy() const
{
	return extension.empty();
}


void Formats::Add(string const & name)
{
	if (formats.find(name) == formats.end())
		formats[name] = Format(name, name, name, "", "");
}


void Formats::Add(string const & name, string const & extension, 
		  string const & prettyname, string const & shortcut)
{

	if (prettyname.empty()) {
		FormatList::iterator it = formats.find(name);
		if (it != formats.end())
			formats.erase(it);
		return;
	}

	string const old_viewer = formats[name].viewer;
	formats[name] = Format(name, extension, prettyname, shortcut,
			       old_viewer);
}


void Formats::SetViewer(string const & name, string const & command)
{
	string command2 = command;
	if (!command2.empty() && !contains(command2,"$$FName"))
		command2 += " $$FName";

	Add(name);
	GetFormat(name)->viewer = command2;
}


bool Formats::View(Buffer const * buffer, string const & filename,
		   string const & format_name)
{
	if (filename.empty())
		return false;

	Format const * format = GetFormat(format_name);
	if (!format || format->viewer.empty()) {
		WriteAlert(_("Can not view file"),
			   _("No information for viewing ")
			   + PrettyName(format_name));
			   return false;
	}

	string command = format->viewer;

	if (format_name == "dvi" &&
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

	command = subst(command, "$$FName", QuoteName(filename));
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


// This method should return a reference, and throw an exception
// if the format named name cannot be found (Lgb)
Format * Formats::GetFormat(string const & name)
{
	FormatList::iterator it = formats.find(name);
	if (it != formats.end())
		return &it->second;
	else
		return 0;
}


string const Formats::PrettyName(string const & name)
{
	Format const * format = GetFormat(name);
	if (format)
		return format->prettyname;
	else
		return name;
}


string const Formats::Extension(string const & name)
{
	Format const * format = GetFormat(name);
	if (format)
		return format->extension;
	else
		return name;
}


vector<Format> 
const Formats::GetAllFormats() const
{
	vector<Format> result;
	for (FormatList::const_iterator cit = formats.begin(); 
	     cit != formats.end(); ++cit)
		result.push_back(cit->second);
	return result;
}


//////////////////////////////////////////////////////////////////////////////

// Instead of storing an object we could just store an const reference.
// _but_ that is not guaranteed to work in all cases. (Lgb)
class compare_Command {
public:
	compare_Command(Command const & c) : com(c) {}
	bool operator()(Command const & c) {
		return c.from == com.from && c.to == com.to;
	}
private:
	Command com;
};


void Converter::Add(string const & from, string const & to,
		    string const & command, string const & flags)
{
	formats.Add(from);
	formats.Add(to);
	Command Com(formats.GetFormat(from), formats.GetFormat(to), command);
	vector<Command>::iterator it = find_if(commands.begin(),
					       commands.end(),
					       compare_Command(Com));

	if (command.empty() || command == "none") {
		if (it != commands.end())
			commands.erase(it);
		return;
	}


	// Read the flags
	string flag_list(flags);
	while (!flag_list.empty()) {
		string flag_name, flag_value;
		flag_list = split(flag_list, flag_value, ',');
		flag_value = split(flag_value, flag_name, '=');
		if (flag_name == "*") {
			if (it != commands.end()) {
				Com = *it;
				Com.command = command;
			}
		} 
		else if (flag_name == "importer")
			Com.importer = true;
		else if (flag_name == "latex")
			Com.latex = true;
		else if (flag_name == "originaldir")
			Com.original_dir = true;
		else if (flag_name == "needaux")
			Com.need_aux = true;
		else if (flag_name == "resultdir")
			Com.result_dir = (flag_value.empty())
				? "$$BaseName" : flag_value;
		else if (flag_name == "resultfile")
			Com.result_file = flag_value;
		else if (flag_name == "parselog")
			Com.parselog = flag_value;
		else if (flag_name == "disable") {
			while (!flag_value.empty()) {
				string tmp;
				flag_value = split(flag_value, tmp, '&');
				Com.disable.push_back(tmp);
			}
		}
	}
	if (!Com.result_dir.empty() && Com.result_file.empty())
		Com.result_file = "index." + to;
	//if (!contains(command, "$$FName"))
	//	Com.latex = true;

	if (Com.latex && (latex_command.empty() || to == "dvi"))
		latex_command = command;
	// If we have both latex & pdflatex, we set latex_command to latex.
	// The latex_command is used to update the .aux file when running
	// a converter that uses it.

	if (it != commands.end()) {
		*it = Com;
		return;
	}
	commands.push_back(Com);
}


static inline
bool enable(vector<Command>::iterator it, string const & from)
{
	return find(it->disable.begin(), it->disable.end(), from)
		== it->disable.end();
}


vector<FormatPair> const
Converter::GetReachableTo(string const & target)
{
	vector<FormatPair> result;

	queue< vector<Command>::iterator > Q;
	for (vector<Command>::iterator it = commands.begin();
	     it != commands.end(); ++it)
		if (it->to->name == target && it->importer) {
			Q.push(it);
			it->visited = true;
		} else
			it->visited = false;

	while (!Q.empty()) {
		vector<Command>::iterator it = Q.front();
		Q.pop();
		result.push_back(FormatPair(it->from, 0, ""));
		for (vector<Command>::iterator it2 = commands.begin();
		     it2 != commands.end(); ++it2)
			if (!it2->visited && it->from == it2->to &&
			    it2->importer) {
				Q.push(it2);
				it2->visited = true;
			}
	}

	return result;
}


vector<FormatPair> const
Converter::GetReachable(string const & from, bool only_viewable)
{
	vector<FormatPair> result;
	Format const * format = formats.GetFormat(from);
	if (!format)
		return result;

	if (!only_viewable || !format->viewer.empty())
		result.push_back(FormatPair(format, 0, ""));

	queue< vector<Command>::iterator > Q;
	for (vector<Command>::iterator it = commands.begin();
	     it != commands.end(); ++it)
		if (it->from->name == from && enable(it, from) 
		    && !it->importer) {
			Q.push(it);
			it->visited = true;
		} else
			it->visited = false;

	while (!Q.empty()) {
		vector<Command>::iterator it = Q.front();
		Q.pop();
		if (!only_viewable || !it->to->viewer.empty())
			result.push_back(FormatPair(it->to, it->from,
						    it->command));
		for (vector<Command>::iterator it2 = commands.begin();
		     it2 != commands.end(); ++it2)
			if (!it2->visited && it->to == it2->from &&
			    enable(it2, from) && !it2->importer) {
				Q.push(it2);
				it2->visited = true;
			}
	}

	return result;
}


bool Converter::IsReachable(string const & from, string const & to)
{
	if (from == to)
		return true;

	queue< vector<Command>::iterator > Q;
	for (vector<Command>::iterator it = commands.begin();
	     it != commands.end(); ++it)
		if (it->from->name == from && enable(it, from)) {
			Q.push(it);
			it->visited = true;
		} else
			it->visited = false;

	while (!Q.empty()) {
		vector<Command>::iterator it = Q.front();
		Q.pop();
		if (it->to->name == to)
			return true;
		for (vector<Command>::iterator it2 = commands.begin();
		     it2 != commands.end(); ++it2)
			if (!it2->visited && it->to == it2->from &&
			    enable(it2, from)) {
				Q.push(it2);
				it2->visited = true;
			}
	}
	return false;
}


bool Converter::Convert(Buffer const * buffer,
			string const & from_file, string const & to_file_base,
			string const & from_format, string const & to_format,
			string const & using_format, string & to_file)
{
	to_file = ChangeExtension(to_file_base,
				  formats.Extension(to_format));

	if (from_format == to_format)
		if (from_file != to_file)
			return lyx::rename(from_file, to_file);
		else
			return true;

	queue< vector<Command>::iterator > Q;
	for (vector<Command>::iterator it = commands.begin();
	     it != commands.end(); ++it)
		if (it->from->name == from_format && enable(it, from_format)) {
			Q.push(it);
			it->visited = true;
			it->previous = commands.end();
		} else
			it->visited = false;

	if (Q.empty()) {
		WriteAlert(_("Can not convert file"),
			   ("Unknown format ") + from_format);
		return false;
	}

	bool found = false;
	vector<Command>::iterator it;
	while (!Q.empty()) {
		it = Q.front();
		if (it->to->name == to_format &&
		    (using_format.empty() || using_format == it->from->name)) {
			found = true;
			break;
		}
		Q.pop();
		for (vector<Command>::iterator it2 = commands.begin();
		     it2 != commands.end(); ++it2)
			if (!it2->visited && it->to == it2->from &&
			    enable(it2, from_format)) {
				Q.push(it2);
				it2->visited = true;
				it2->previous = it;
			}
	}

	if (!found) {
		WriteAlert(_("Can not convert file"),
			   _("No information for converting from ")
			   + formats.PrettyName(from_format) + _(" to ")
			   + formats.PrettyName(to_format));
		return false;
	}

	vector< vector<Command>::iterator > S;
	while (it != commands.end()) {
		S.push_back(it);
		it = it->previous;
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
		bool dummy = it->to->dummy() && it->to->name != "program";
		if (!dummy)
			lyxerr << "Converting from  "
			       << it->from->name << " to " << it->to->name << endl;
		infile = outfile;
		outfile = it->result_dir.empty()
			? ChangeExtension(from_file, it->to->extension)
			: AddName(subst(it->result_dir,
					"$$BaseName", from_base),
				  subst(it->result_file,
					"$$BaseName", OnlyFilename(from_base)));

		if (it->latex) {
			lyxrc.pdf_mode = it->to->name == "pdf";
			lyxerr << "Running " << it->command << endl;
			run_latex = true;
			if (!runLaTeX(buffer, it->command))
				return false;
		} else {
			if (it->need_aux && !run_latex
			    && !latex_command.empty()) {
				lyxerr << "Running " << latex_command 
				       << " to update aux file"<<  endl;
				runLaTeX(buffer, latex_command);
			}

			string infile2 = (it->original_dir)
				? infile : MakeRelPath(infile, path);
			string outfile2 = (it->original_dir)
				? outfile : MakeRelPath(outfile, path);

			string command = it->command;
			command = subst(command, "$$FName", QuoteName(infile2));
			command = subst(command, "$$BaseName", QuoteName(from_base));
			command = subst(command, "$$OutName", QuoteName(outfile2));

			if (!it->parselog.empty())
				command += " 2> " + QuoteName(infile2 + ".out");

			if (it->from->name == "dvi" && it->to->name == "ps")
				command = add_options(command,
						      dvips_options(buffer));

			lyxerr << "Calling " << command << endl;
			if (buffer)
				ShowMessage(buffer, _("Executing command:"), command);

			Systemcalls::Starttype type = (dummy)
				? Systemcalls::SystemDontWait : Systemcalls::System;
			Systemcalls one;
			int res;
			if (it->original_dir && buffer) {
				Path p(buffer->filepath);
				res = one.startscript(type, command);
			} else
				res = one.startscript(type, command);

			if (!it->parselog.empty()) {
				string const logfile =  infile2 + ".log";
				string const command2 = it->parselog +
					" < " + QuoteName(infile2 + ".out") +
					" > " + QuoteName(logfile);
				one.startscript(Systemcalls::System, command2);
				if (!scanLog(buffer, command, logfile))
					return false;
			}

			if (res) {
				if (it->to->name == "program")
					WriteAlert(_("There were errors during the Build process."),
						   _("You should try to fix them."));
				else
					WriteAlert(_("Can not convert file"),
						   "Error while executing",
						   command.substr(0, 50));
				return false;
			}
		}
	}

	if (it->to->dummy())
		return true;


	if (!it->result_dir.empty()) {
		to_file = AddName(subst(it->result_dir,
					"$$BaseName", to_base),
				  subst(it->result_file,
					"$$BaseName", OnlyFilename(to_base)));
		if (from_base != to_base) {
			string from = subst(it->result_dir,
					    "$$BaseName", from_base);
			string to = subst(it->result_dir,
					  "$$BaseName", to_base);
			if (!lyx::rename(from, to)) {
				WriteAlert(_("Error while trying to move directory:"),
					   from, ("to ") + to);
				return false;
			}
		}
	} else if (outfile != to_file) {
		bool moved = (it->latex)
			? lyx::copy(outfile, to_file)
			: lyx::rename(outfile, to_file);
		if (!moved) {
			WriteAlert(_("Error while trying to move file:"),
				   outfile, _("to ") + to_file);
			return false;
		}
	}

        return true;
}


bool Converter::Convert(Buffer const * buffer,
			string const & from_file, string const & to_file_base,
			string const & from_format, string const & to_format,
			string const & using_format)
{
	string to_file;
	return Convert(buffer, from_file, to_file_base, from_format, to_format,
		using_format, to_file);
}


string const Converter::SplitFormat(string const & str, string & format)
{
	string const using_format = split(str, format, ':');
	if (format.empty())
		format = "dvi";
	return using_format;
}


bool Converter::scanLog(Buffer const * buffer, string const & command,
			string const & filename)
{
	if (!buffer)
		return false;

	BufferView * bv = buffer->getUser();
	bool need_redraw = false;
	if (bv) {
		ProhibitInput(bv);
		// Remove all error insets
		need_redraw = bv->removeAutoInsets();
	}

	LaTeX latex("", filename, "");
	TeXErrors terr;
	int result = latex.scanLogFile(terr);
	if (bv) {
		if ((result & LaTeX::ERRORS)) {
			// Insert all errors as errors boxes
			bv->insertErrors(terr);
			need_redraw = true;
		}
		if (need_redraw) {
			bv->redraw();
			bv->fitCursor(bv->text);
		}
		AllowInput(bv);
	}

	if ((result & LaTeX::ERRORS)) {
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
		string head;
		split(command, head, ' ');
		WriteAlert(_("There were errors during running of ") + head,
			   s, t);
		return false;
	} else if (result & LaTeX::NO_OUTPUT) {
		string const s = _("The operation resulted in");
		string const t = _("an empty file.");
		WriteAlert(_("Resulting file is empty"), s, t);
		return false;
	}
	return true;
}


bool Converter::runLaTeX(Buffer const * buffer, string const & command)
{
	if (!buffer)
		return false;

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
	}  else if (result & LaTeX::NO_OUTPUT) {
		string const s = _("The operation resulted in");
		string const t = _("an empty file.");
		WriteAlert(_("Resulting file is empty"), s, t);
	}

	if (bv)
		AllowInput(bv);
 
	int const ERROR_MASK = 
			LaTeX::NO_LOGFILE |
			LaTeX::ERRORS |
			LaTeX::NO_OUTPUT;
	
	return (result & ERROR_MASK) == 0;

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
	if (!buffer)
		return result;

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


void Converter::init()
{
}

vector<Command> const Converter::GetAllCommands()
{
	vector<Command> result;
	for (vector<Command>::iterator it = commands.begin(); 
	     it != commands.end(); ++it)
		result.push_back(*it);
	return result;
}


/// The global instance
Formats formats;

// The global copy of the system lyxrc entries (everything except preferences)
Formats system_formats;
