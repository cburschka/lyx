/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <cctype>

#include "converter.h"
#include "lyxrc.h"
#include "support/syscall.h"
#include "support/path.h"
#include "support/filetools.h"
#include "buffer.h"
#include "bufferview_funcs.h"
#include "LaTeX.h"
#include "LyXView.h"
#include "lyx_gui_misc.h"
#include "lyx_cb.h" // ShowMessage()
#include "support/lyxfunctional.h"
#include "frontends/Alert.h"
#include "gettext.h"
#include "BufferView.h"
#include "debug.h"

using std::vector;
using std::queue;
using std::endl;
using std::fill;
using std::find_if;
using std::reverse;
using std::sort;

namespace {

string const token_from("$$i");
string const token_base("$$b");
string const token_to("$$o");

//////////////////////////////////////////////////////////////////////////////

inline
string const add_options(string const & command, string const & options)
{
	string head;
	string const tail = split(command, head, ' ');
	return head + ' ' + options + ' ' + tail;
}

} // namespace anon

//////////////////////////////////////////////////////////////////////////////

bool Format::dummy() const
{
	return extension().empty();
}


bool Format::isChildFormat() const
{
	if (name_.empty())
		return false;
        return isdigit(name_[name_.length() - 1]);
}


string const Format::parentFormat() const
{
	return name_.substr(0, name_.length() - 1);
}

//////////////////////////////////////////////////////////////////////////////

// This method should return a reference, and throw an exception
// if the format named name cannot be found (Lgb)
Format const * Formats::getFormat(string const & name) const
{
	FormatList::const_iterator cit =
		find_if(formatlist.begin(), formatlist.end(),
			lyx::compare_memfun(&Format::name, name));
	if (cit != formatlist.end())
		return &(*cit);
	else
		return 0;
}


int Formats::getNumber(string const & name) const
{
	FormatList::const_iterator cit =
		find_if(formatlist.begin(), formatlist.end(),
			lyx::compare_memfun(&Format::name, name));
	if (cit != formatlist.end())
		return cit - formatlist.begin();
	else
		return -1;
}


void Formats::add(string const & name)
{
	if (!getFormat(name))
		add(name, name, name, string());
}


void Formats::add(string const & name, string const & extension, 
		  string const & prettyname, string const & shortcut)
{
	FormatList::iterator it = 
		find_if(formatlist.begin(), formatlist.end(),
			lyx::compare_memfun(&Format::name, name));
	if (it == formatlist.end())
		formatlist.push_back(Format(name, extension, prettyname,
					    shortcut, ""));
	else {
		string viewer = it->viewer();
		*it = Format(name, extension, prettyname, shortcut, viewer);
	}
}


void Formats::erase(string const & name)
{
	FormatList::iterator it = 
		find_if(formatlist.begin(), formatlist.end(),
			lyx::compare_memfun(&Format::name, name));
	if (it != formatlist.end())
		formatlist.erase(it);
}


void Formats::sort()
{
	std::sort(formatlist.begin(), formatlist.end());
}


void Formats::setViewer(string const & name, string const & command)
{
	add(name);
	FormatList::iterator it =
		find_if(formatlist.begin(), formatlist.end(),
			lyx::compare_memfun(&Format::name, name));
	if (it != formatlist.end())
		it->setViewer(command);
}


bool Formats::view(Buffer const * buffer, string const & filename,
		   string const & format_name) const
{
	if (filename.empty())
		return false;

	Format const * format = getFormat(format_name);
	if (format && format->viewer().empty() &&
	    format->isChildFormat())
		format = getFormat(format->parentFormat());
	if (!format || format->viewer().empty()) {
		Alert::alert(_("Cannot view file"),
			   _("No information for viewing ")
			   + prettyName(format_name));
			   return false;
	}

	string command = format->viewer();

	if (format_name == "dvi" &&
	    !lyxrc.view_dvi_paper_option.empty()) {
		command += " " + lyxrc.view_dvi_paper_option;
		string paper_size = converters.papersize(buffer);
		if (paper_size == "letter")
			paper_size = "us";
		command += " " + paper_size;
		if (buffer->params.orientation 
		    == BufferParams::ORIENTATION_LANDSCAPE)
			command += 'r';
        }

	command += " " + QuoteName(OnlyFilename((filename)));

	lyxerr[Debug::FILES] << "Executing command: " << command << endl;
	ShowMessage(buffer, _("Executing command:"), command);

	Path p(OnlyPath(filename));
	Systemcalls one;
	int const res = one.startscript(Systemcalls::SystemDontWait, command);

	if (res) {
		Alert::alert(_("Cannot view file"),
			   _("Error while executing"),
			   command.substr(0, 50));
		return false;
	}
	return true;
}


string const Formats::prettyName(string const & name) const
{
	Format const * format = getFormat(name);
	if (format)
		return format->prettyname();
	else
		return name;
}


string const Formats::extension(string const & name) const
{
	Format const * format = getFormat(name);
	if (format)
		return format->extension();
	else
		return name;
}

//////////////////////////////////////////////////////////////////////////////

void Converter::readFlags()
{
	string flag_list(flags);
	while (!flag_list.empty()) {
		string flag_name, flag_value;
		flag_list = split(flag_list, flag_value, ',');
		flag_value = split(flag_value, flag_name, '=');
		if (flag_name == "latex")
			latex = true;
		else if (flag_name == "originaldir")
			original_dir = true;
		else if (flag_name == "needaux")
			need_aux = true;
		else if (flag_name == "resultdir")
			result_dir = (flag_value.empty())
				? token_base : flag_value;
		else if (flag_name == "resultfile")
			result_file = flag_value;
		else if (flag_name == "parselog")
			parselog = flag_value;
	}
	if (!result_dir.empty() && result_file.empty())
		result_file = "index." + formats.extension(to);
	//if (!contains(command, token_from))
	//	latex = true;
}


bool operator<(Converter const & a, Converter const & b)
{
	int const i = compare_no_case(a.From->prettyname(),
				      b.From->prettyname());
	if (i == 0)
		return compare_no_case(a.To->prettyname(), b.To->prettyname())
			< 0;
	else
		return i < 0;
}

//////////////////////////////////////////////////////////////////////////////

class compare_Converter {
public:
	compare_Converter(string const & f, string const & t)
		: from(f), to(t) {}
	bool operator()(Converter const & c) {
		return c.from == from && c.to == to;
	}
private:
	string const & from;
	string const & to;
};


Converter const * Converters::getConverter(string const & from,
					    string const & to)
{
	ConverterList::const_iterator cit =
		find_if(converterlist_.begin(), converterlist_.end(),
			compare_Converter(from, to));
	if (cit != converterlist_.end())
		return &(*cit);
	else
		return 0;
}


int Converters::getNumber(string const & from, string const & to)
{
	ConverterList::const_iterator cit =
		find_if(converterlist_.begin(), converterlist_.end(),
			compare_Converter(from, to));
	if (cit != converterlist_.end())
		return cit - converterlist_.begin();
	else
		return -1;
}


void Converters::add(string const & from, string const & to,
		     string const & command, string const & flags)
{
	formats.add(from);
	formats.add(to);
	ConverterList::iterator it = find_if(converterlist_.begin(),
					     converterlist_.end(),
					     compare_Converter(from, to));

	Converter converter(from, to, command, flags);
	if (it != converterlist_.end() && !flags.empty() && flags[0] == '*') {
		converter = *it;
		converter.command = command;
		converter.flags = flags;
	}
	converter.readFlags();
	
	if (converter.latex && (latex_command_.empty() || to == "dvi"))
		latex_command_ = subst(command, token_from, "");
	// If we have both latex & pdflatex, we set latex_command to latex.
	// The latex_command is used to update the .aux file when running
	// a converter that uses it.

	if (it == converterlist_.end()) {
		converterlist_.push_back(converter);
	} else {
		converter.From = it->From;
		converter.To = it->To;
		*it = converter;
	}
}


void Converters::erase(string const & from, string const & to)
{
	ConverterList::iterator it = find_if(converterlist_.begin(),
					     converterlist_.end(),
					     compare_Converter(from, to));
	if (it != converterlist_.end())
		converterlist_.erase(it);
}


// This method updates the pointers From and To in all the converters.
// The code is not very efficient, but it doesn't matter as the number
// of formats and converters is small.
// Furthermore, this method is called only on startup, or after 
// adding/deleting a format in FormPreferences (the latter calls can be
// eliminated if the formats in the Formats class are stored using a map or
// a list (instead of a vector), but this will cause other problems). 
void Converters::update(Formats const & formats)
{
	ConverterList::iterator it = converterlist_.begin();
	ConverterList::iterator end = converterlist_.end();
	for (; it != end; ++it) {
		it->From = formats.getFormat(it->from);
		it->To = formats.getFormat(it->to);
	}
}


// This method updates the pointers From and To in the last converter.
// It is called when adding a new converter in FormPreferences
void Converters::updateLast(Formats const & formats)
{
	if (converterlist_.begin() != converterlist_.end()) {
		ConverterList::iterator it = converterlist_.end() - 1;
		it->From = formats.getFormat(it->from);
		it->To = formats.getFormat(it->to);
	}
}


void Converters::sort()
{
	std::sort(converterlist_.begin(), converterlist_.end());
}


int Converters::bfs_init(string const & start, bool clear_visited)
{
	int const s = formats.getNumber(start);
	if (s < 0)
		return s;

	Q_ = queue<int>();
	if (clear_visited)
		fill(visited_.begin(), visited_.end(), false);
	if (visited_[s] == false) {
		Q_.push(s);
		visited_[s] = true;
	}
	return s;
}


vector<Format const *> const
Converters::getReachableTo(string const & target, bool clear_visited)
{
	vector<Format const *> result;
	int const s = bfs_init(target, clear_visited);
	if (s < 0)
		return result;

	while (!Q_.empty()) {
		int const i = Q_.front();
		Q_.pop();
		if (i != s || target != "lyx") {
			result.push_back(&formats.get(i));
		}
		
		vector<int>::iterator it = vertices_[i].in_vertices.begin();
		vector<int>::iterator end = vertices_[i].in_vertices.end();
		for (; it != end; ++it) {
			if (!visited_[*it]) {
				visited_[*it] = true;
				Q_.push(*it);
			}
		}
	}

	return result;
}


vector<Format const *> const
Converters::getReachable(string const & from, bool only_viewable,
			 bool clear_visited)
{
	vector<Format const *> result;

	if (bfs_init(from, clear_visited) < 0)
		return result;

	while (!Q_.empty()) {
		int const i = Q_.front();
		Q_.pop();
		Format const & format = formats.get(i);
		if (format.name() == "lyx")
			continue;
		if (!only_viewable || !format.viewer().empty() ||
		    format.isChildFormat())
			result.push_back(&format);

		vector<int>::const_iterator cit =
			vertices_[i].out_vertices.begin();
		vector<int>::const_iterator end =
			vertices_[i].out_vertices.end();
		for (; cit != end; ++cit)
			if (!visited_[*cit]) {
				visited_[*cit] = true;
				Q_.push(*cit);
			}
	}

	return result;
}


bool Converters::isReachable(string const & from, string const & to)
{
	if (from == to)
		return true;

	int const s = bfs_init(from);
	int const t = formats.getNumber(to);
	if (s < 0 || t < 0)
		return false;

	while (!Q_.empty()) {
		int const i = Q_.front();
		Q_.pop();
		if (i == t)
			return true;

		vector<int>::const_iterator cit =
			vertices_[i].out_vertices.begin();
		vector<int>::const_iterator end =
			vertices_[i].out_vertices.end();
		for (; cit != end; ++cit) {
			if (!visited_[*cit]) {
				visited_[*cit] = true;
				Q_.push(*cit);
			}
		}
	}

	return false;
}


Converters::EdgePath const
Converters::getPath(string const & from, string const & to)
{
	EdgePath path;
	if (from == to)
		return path;

	int const s = bfs_init(from);
	int t = formats.getNumber(to);
	if (s < 0 || t < 0)
		return path;

	vector<int> prev_edge(formats.size());
	vector<int> prev_vertex(formats.size());

	bool found = false;
	while (!Q_.empty()) {
		int const i = Q_.front();
		Q_.pop();
		if (i == t) {
			found = true;
			break;
		}
		
		vector<int>::const_iterator beg =
			vertices_[i].out_vertices.begin();
		vector<int>::const_iterator cit = beg;
		vector<int>::const_iterator end =
			vertices_[i].out_vertices.end();
		for (; cit != end; ++cit)
			if (!visited_[*cit]) {
				int const j = *cit;
				visited_[j] = true;
				Q_.push(j);
				int const k = cit - beg;
				prev_edge[j] = vertices_[i].out_edges[k];
				prev_vertex[j] = i;
			}
	}
	if (!found)
		return path;

	while (t != s) {
		path.push_back(prev_edge[t]);
		t = prev_vertex[t];
	}
	reverse(path.begin(), path.end());
	return path;
}


bool Converters::usePdflatex(EdgePath const & path)
{
	for (EdgePath::const_iterator cit = path.begin();
	     cit != path.end(); ++cit) {
		Converter const & conv = converterlist_[*cit];
		if (conv.latex)
			return contains(conv.to, "pdf");
	}
	return false;
}


bool Converters::convert(Buffer const * buffer,
			 string const & from_file, string const & to_file_base,
			 string const & from_format, string const & to_format,
			 string & to_file)
{
	to_file = ChangeExtension(to_file_base,
				  formats.extension(to_format));

	if (from_format == to_format)
		return move(from_file, to_file, false);

	EdgePath edgepath = getPath(from_format, to_format);
	if (edgepath.empty()) {
		Alert::alert(_("Cannot convert file"),
			   _("No information for converting from ")
			   + formats.prettyName(from_format) + _(" to ")
			   + formats.prettyName(to_format));
		return false;
	}

	string path = OnlyPath(from_file);
	Path p(path);

	bool run_latex = false;
	string from_base = ChangeExtension(from_file, "");
	string to_base = ChangeExtension(to_file, "");
	string infile;
	string outfile = from_file;
	for (EdgePath::const_iterator cit = edgepath.begin();
	     cit != edgepath.end(); ++cit) {
		Converter const & conv = converterlist_[*cit];
		bool dummy = conv.To->dummy() && conv.to != "program";
		if (!dummy)
			lyxerr[Debug::FILES] << "Converting from  "
			       << conv.from << " to " << conv.to << endl;
		infile = outfile;
		outfile = conv.result_dir.empty()
			? ChangeExtension(from_file, conv.To->extension())
			: AddName(subst(conv.result_dir,
					token_base, from_base),
				  subst(conv.result_file,
					token_base, OnlyFilename(from_base)));

		if (conv.latex) {
			run_latex = true;
			string command = subst(conv.command, token_from, "");
			lyxerr[Debug::FILES] << "Running " << command << endl;
			if (!runLaTeX(buffer, command))
				return false;
		} else {
			if (conv.need_aux && !run_latex
			    && !latex_command_.empty()) {
				lyxerr[Debug::FILES] 
					<< "Running " << latex_command_ 
					<< " to update aux file"<<  endl;
				runLaTeX(buffer, latex_command_);
			}

			string infile2 = (conv.original_dir)
				? infile : MakeRelPath(infile, path);
			string outfile2 = (conv.original_dir)
				? outfile : MakeRelPath(outfile, path);

			string command = conv.command;
			command = subst(command, token_from, QuoteName(infile2));
			command = subst(command, token_base, QuoteName(from_base));
			command = subst(command, token_to, QuoteName(outfile2));

			if (!conv.parselog.empty())
				command += " 2> " + QuoteName(infile2 + ".out");

			if (conv.from == "dvi" && conv.to == "ps")
				command = add_options(command,
						      dvips_options(buffer));
			else if (conv.from == "dvi" && prefixIs(conv.to, "pdf"))
				command = add_options(command,
						      dvipdfm_options(buffer));

			lyxerr[Debug::FILES] << "Calling " << command << endl;
			if (buffer)
				ShowMessage(buffer, _("Executing command:"), command);

			Systemcalls::Starttype type = (dummy)
				? Systemcalls::SystemDontWait : Systemcalls::System;
			Systemcalls one;
			int res;
			if (conv.original_dir && buffer) {
				Path p(buffer->filePath());
				res = one.startscript(type, command);
			} else
				res = one.startscript(type, command);

			if (!conv.parselog.empty()) {
				string const logfile =  infile2 + ".log";
				string const command2 = conv.parselog +
					" < " + QuoteName(infile2 + ".out") +
					" > " + QuoteName(logfile);
				one.startscript(Systemcalls::System, command2);
				if (!scanLog(buffer, command, logfile))
					return false;
			}

			if (res) {
				if (conv.to == "program")
					Alert::alert(_("There were errors during the Build process."),
						   _("You should try to fix them."));
				else
					Alert::alert(_("Cannot convert file"),
						   "Error while executing",
						   command.substr(0, 50));
				return false;
			}
		}
	}

	Converter const & conv = converterlist_[edgepath.back()];
	if (conv.To->dummy())
		return true;


	if (!conv.result_dir.empty()) {
		to_file = AddName(subst(conv.result_dir, token_base, to_base),
				  subst(conv.result_file,
					token_base, OnlyFilename(to_base)));
		if (from_base != to_base) {
			string from = subst(conv.result_dir,
					    token_base, from_base);
			string to = subst(conv.result_dir,
					  token_base, to_base);
			if (!lyx::rename(from, to)) {
				Alert::alert(_("Error while trying to move directory:"),
					   from, ("to ") + to);
				return false;
			}
		}
		return true;
	} else 
		return move(outfile, to_file, conv.latex);
}

// If from = /path/file.ext and to = /path2/file2.ext2 then this method 
// moves each /path/file*.ext file to /path2/file2*.ext2'
bool Converters::move(string const & from, string const & to, bool copy)
{
	if (from == to)
		return true;

	bool no_errors = true;
	string const path = OnlyPath(from);
	string const base = OnlyFilename(ChangeExtension(from, ""));
	string const to_base = ChangeExtension(to, "");
	string const to_extension = GetExtension(to);

	vector<string> files = DirList(OnlyPath(from), GetExtension(from));
	for (vector<string>::const_iterator it = files.begin();
	     it != files.end(); ++it)
		if (prefixIs(*it, base)) {
			string from2 = path + *it;
			string to2 = to_base + it->substr(base.length());
			to2 = ChangeExtension(to2, to_extension);
			lyxerr[Debug::FILES] << "moving " << from2 
					     << " to " << to2 << endl;
			bool moved = (copy)
				? lyx::copy(from2, to2)
				: lyx::rename(from2, to2);
			if (!moved && no_errors) {
				Alert::alert(_("Error while trying to move file:"),
					   from2, _("to ") + to2);
				no_errors = false;
			}
		}
	return no_errors;
}


bool Converters::convert(Buffer const * buffer,
			string const & from_file, string const & to_file_base,
			string const & from_format, string const & to_format)
{
	string to_file;
	return convert(buffer, from_file, to_file_base, from_format, to_format,
		       to_file);
}


void Converters::buildGraph()
{
	vertices_ = vector<Vertex>(formats.size());
	visited_.resize(formats.size());

	for (ConverterList::iterator it = converterlist_.begin();
	     it != converterlist_.end(); ++it) {
		int const s = formats.getNumber(it->from);
		int const t = formats.getNumber(it->to);
		vertices_[t].in_vertices.push_back(s);
		vertices_[s].out_vertices.push_back(t);
		vertices_[s].out_edges.push_back(it - converterlist_.begin());
	}
}


bool Converters::formatIsUsed(string const & format)
{
	ConverterList::const_iterator cit = converterlist_.begin();
	ConverterList::const_iterator end = converterlist_.end();
	for (; cit != end; ++cit) {
		if (cit->from == format || cit->to == format)
			return true;
	}
	return false;
}


bool Converters::scanLog(Buffer const * buffer, string const & command,
			string const & filename)
{
	if (!buffer)
		return false;

	BufferView * bv = buffer->getUser();
	bool need_redraw = false;
	if (bv) {
		bv->owner()->prohibitInput();
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
			bv->fitCursor();
		}
		bv->owner()->allowInput();
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
		Alert::alert(_("There were errors during running of ") + head,
			   s, t);
		return false;
	} else if (result & LaTeX::NO_OUTPUT) {
		string const s = _("The operation resulted in");
		string const t = _("an empty file.");
		Alert::alert(_("Resulting file is empty"), s, t);
		return false;
	}
	return true;
}


bool Converters::runLaTeX(Buffer const * buffer, string const & command)
{
	if (!buffer)
		return false;

	BufferView * bv = buffer->getUser();
	string name = buffer->getLatexName();
	bool need_redraw = false;

	if (bv) {
		bv->owner()->prohibitInput();
		bv->owner()->message(_("Running LaTeX..."));
		// Remove all error insets
		need_redraw = bv->removeAutoInsets();
	}


	// do the LaTex run(s)
	TeXErrors terr;
	LaTeX latex(command, name, buffer->filePath());
	int result = latex.run(terr,
			       bv ? bv->owner()->getLyXFunc() : 0);
	

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
			bv->fitCursor();
		}
	}

	// check return value from latex.run().
	if ((result & LaTeX::NO_LOGFILE)) {
		Alert::alert(_("LaTeX did not work!"),
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
		Alert::alert(_("There were errors during the LaTeX run."),
			   s, t);
	}  else if (result & LaTeX::NO_OUTPUT) {
		string const s = _("The operation resulted in");
		string const t = _("an empty file.");
		Alert::alert(_("Resulting file is empty"), s, t);
	}

	if (bv)
		bv->owner()->allowInput();
 
	int const ERROR_MASK = 
			LaTeX::NO_LOGFILE |
			LaTeX::ERRORS |
			LaTeX::NO_OUTPUT;
	
	return (result & ERROR_MASK) == 0;

}


string const Converters::papersize(Buffer const * buffer)
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
		return "letter";
	}
}


string const Converters::dvips_options(Buffer const * buffer)
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
		string const paper_option = papersize(buffer);
		if (paper_option != "letter" ||
		    buffer->params.orientation != BufferParams::ORIENTATION_LANDSCAPE) {
			// dvips won't accept -t letter -t landscape.  In all other
			// cases, include the paper size explicitly.
			result = lyxrc.print_paper_flag;
			result += ' ' + paper_option;
		}
	}
	if (buffer->params.orientation == BufferParams::ORIENTATION_LANDSCAPE &&
	    buffer->params.papersize2 != BufferParams::VM_PAPER_CUSTOM)
		result += ' ' + lyxrc.print_landscape_flag;
	return result;
}


string const Converters::dvipdfm_options(Buffer const * buffer)
{
	string result;
	if (!buffer)
		return result;

	if (buffer->params.papersize2 != BufferParams::VM_PAPER_CUSTOM) {
		string const paper_size = papersize(buffer);
		if (paper_size != "b5" && paper_size != "foolscap")
			result = "-p "+ paper_size;

		if (buffer->params.orientation == BufferParams::ORIENTATION_LANDSCAPE)
			result += " -l";
	}

	return result;
}


vector<Converters::Vertex> Converters::vertices_;


/// The global instance
Formats formats;
Converters converters;

// The global copy after reading lyxrc.defaults
Formats system_formats;
Converters system_converters;
