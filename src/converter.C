/**
 * \file converter.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "converter.h"
#include "graph.h"
#include "format.h"
#include "lyxrc.h"
#include "buffer.h"
#include "bufferview_funcs.h"
#include "LaTeX.h"
#include "lyx_cb.h" // ShowMessage()
#include "gettext.h"
#include "BufferView.h"
#include "debug.h"

#include "frontends/Alert.h"
#include "frontends/LyXView.h"

#include "support/filetools.h"
#include "support/lyxfunctional.h"
#include "support/path.h"
#include "support/systemcall.h"

#include "support/BoostFormat.h"

#include <cctype>

#ifndef CXX_GLOBAL_CSTD
using std::isdigit;
#endif

using std::vector;
using std::endl;
using std::find_if;

namespace {

string const token_from("$$i");
string const token_base("$$b");
string const token_to("$$o");
string const token_path("$$p");



string const add_options(string const & command, string const & options)
{
	string head;
	string const tail = split(command, head, ' ');
	return head + ' ' + options + ' ' + tail;
}

} // namespace anon


Converter::Converter(string const & f, string const & t, string const & c,
	  string const & l): from(f), to(t), command(c), flags(l),
			     From(0), To(0), latex(false),
			     original_dir(false), need_aux(false)
{}

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
	// use the compare_ascii_no_case instead of compare_no_case,
	// because in turkish, 'i' is not the lowercase version of 'I',
	// and thus turkish locale breaks parsing of tags.
	int const i = compare_ascii_no_case(a.From->prettyname(),
					    b.From->prettyname());
	if (i == 0)
		return compare_ascii_no_case(a.To->prettyname(), b.To->prettyname())
			< 0;
	else
		return i < 0;
}



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


bool Converters::usePdflatex(Graph::EdgePath const & path)
{
	for (Graph::EdgePath::const_iterator cit = path.begin();
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

	Graph::EdgePath edgepath = getPath(from_format, to_format);
	if (edgepath.empty()) {
		return false;
	}

	string path = OnlyPath(from_file);
	Path p(path);

	bool run_latex = false;
	string from_base = ChangeExtension(from_file, "");
	string to_base = ChangeExtension(to_file, "");
	string infile;
	string outfile = from_file;
	for (Graph::EdgePath::const_iterator cit = edgepath.begin();
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

		// if input and output files are equal, we use a
		// temporary file as intermediary (JMarc)
		string real_outfile;
		if (outfile == infile) {
			real_outfile = infile;
			outfile = AddName(buffer->tmppath, "tmpfile.out");
		}

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
			command = LibScriptSearch(command);

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

			Systemcall::Starttype type = (dummy)
				? Systemcall::DontWait : Systemcall::Wait;
			Systemcall one;
			int res;
			if (conv.original_dir && buffer) {
				Path p(buffer->filePath());
				res = one.startscript(type, command);
			} else
				res = one.startscript(type, command);

			if (!real_outfile.empty()) {
				if (!lyx::rename(outfile, real_outfile))
					res = -1;
				else
					lyxerr[Debug::FILES]
						<< "renaming file " << outfile
						<< " to " << real_outfile
						<< endl;
			}

			if (!conv.parselog.empty()) {
				string const logfile =  infile2 + ".log";
				string const script = LibScriptSearch(conv.parselog);
				string const command2 = script +
					" < " + QuoteName(infile2 + ".out") +
					" > " + QuoteName(logfile);
				one.startscript(Systemcall::Wait, command2);
				if (!scanLog(buffer, command, logfile))
					return false;
			}

			if (res) {
				if (conv.to == "program") {
					Alert::error(_("Build errors"),
						_("There were errors during the build process."));
				} else {
					Alert::alert(_("Cannot convert file"),
						   _("Error while executing"),
						   command.substr(0, 50));
				}
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
#if USE_BOOST_FORMAT
				Alert::alert(_("Error while trying to move directory:"),
					   from, boost::io::str(boost::format(_("to %1$s")) % to));
#else
				Alert::alert(_("Error while trying to move directory:"),
					   from, _("to ") + to);
#endif
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
			string const from2 = path + *it;
			string to2 = to_base + it->substr(base.length());
			to2 = ChangeExtension(to2, to_extension);
			lyxerr[Debug::FILES] << "moving " << from2
					     << " to " << to2 << endl;
			bool const moved = (copy)
				? lyx::copy(from2, to2)
				: lyx::rename(from2, to2);
			if (!moved && no_errors) {
#if USE_BOOST_FORMAT
				Alert::alert(_("Error while trying to move file:"),
					   from2, boost::io::str(boost::format(_("to %1$s")) % to2));
#else
				Alert::alert(_("Error while trying to move file:"),
					   from2, _("to ") + to2);
#endif
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


namespace {

void alertErrors(string const & prog, int nr_errors)
{
	string s;
#if USE_BOOST_FORMAT
	if (nr_errors == 1) {
		boost::format fmt(_("One error detected when running %1$s.\n"));
		fmt % prog;
		s = fmt.str();
	} else {
		boost::format fmt(_("%1$s errors detected when running %2$s.\n"));
		fmt % tostr(nr_errors);
		fmt % prog;
		s = fmt.str();
	}
#else
	if (nr_errors == 1) {
		s = _("One error detected");
	} else {
		s = tostr(nr_errors);
		s += _(" errors detected.");
	}
#endif
	Alert::error(_("Errors found"), s);
}

}


bool Converters::scanLog(Buffer const * buffer, string const & command,
			string const & filename)
{
	if (!buffer)
		return false;

	BufferView * bv = buffer->getUser();
	if (bv) {
		bv->owner()->busy(true);
		// all error insets should have been removed by now
	}

	LaTeX latex("", filename, "");
	TeXErrors terr;
	int result = latex.scanLogFile(terr);
	if (bv) {
		if ((result & LaTeX::ERRORS)) {
			// Insert all errors as errors boxes
			bv->insertErrors(terr);
#warning repaint() or update() or nothing ?
			bv->repaint();
			bv->fitCursor();
		}
		bv->owner()->busy(false);
	}

	if ((result & LaTeX::ERRORS)) {
		string head;
		split(command, head, ' ');
		alertErrors(head, latex.getNumErrors());
		return false;
	} else if (result & LaTeX::NO_OUTPUT) {
		Alert::warning(_("Output is empty"),
			_("An empty output file was generated."));
		return false;
	}
	return true;
}


bool Converters::runLaTeX(Buffer const * buffer, string const & command)
{
	if (!buffer)
		return false;

	BufferView * bv = buffer->getUser();

	if (bv) {
		bv->owner()->busy(true);
		bv->owner()->message(_("Running LaTeX..."));
		// all the autoinsets have already been removed
	}

	// do the LaTeX run(s)
	string name = buffer->getLatexName();
	LaTeX latex(command, name, buffer->filePath());
	TeXErrors terr;
	int result = latex.run(terr,
			       bv ? &bv->owner()->getLyXFunc() : 0);

	if (bv) {
		if ((result & LaTeX::ERRORS)) {
			// Insert all errors as errors boxes
			bv->insertErrors(terr);
#warning repaint() or update() or nothing ?
			bv->repaint();
			bv->fitCursor();
		}
	}

	// check return value from latex.run().
	if ((result & LaTeX::NO_LOGFILE)) {
		Alert::alert(_("LaTeX did not work!"),
			   _("Missing log file:"), name);
	} else if ((result & LaTeX::ERRORS)) {
		alertErrors("LaTeX", latex.getNumErrors());
	}  else if (result & LaTeX::NO_OUTPUT) {
		Alert::warning(_("Output is empty"),
			_("An empty output file was generated."));
	}

	if (bv)
		bv->owner()->busy(false);

	int const ERROR_MASK =
			LaTeX::NO_LOGFILE |
			LaTeX::ERRORS |
			LaTeX::NO_OUTPUT;

	return (result & ERROR_MASK) == 0;

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

void Converters::buildGraph()
{
	G_.init(formats.size());
	ConverterList::iterator beg = converterlist_.begin();
	ConverterList::iterator end = converterlist_.end();
	for (ConverterList::iterator it = beg; it != end ; ++it) {
		int const s = formats.getNumber(it->from);
		int const t = formats.getNumber(it->to);
		G_.addEdge(s,t);
	}
}

vector<Format const *> const
Converters::intToFormat(std::vector<int> const & input)
{
	vector<Format const *> result(input.size());

	vector<int>::const_iterator it = input.begin();
	vector<int>::const_iterator end = input.end();
	vector<Format const *>::iterator rit = result.begin();
	for ( ; it != end; ++it, ++rit) {
		*rit = &formats.get(*it);
	}
	return result;
}

vector<Format const *> const
Converters::getReachableTo(string const & target, bool clear_visited)
{
	vector<int> const & reachablesto =
		G_.getReachableTo(formats.getNumber(target), clear_visited);

	return intToFormat(reachablesto);
}

vector<Format const *> const
Converters::getReachable(string const & from, bool only_viewable,
	     bool clear_visited)
{
	vector<int> const & reachables =
		G_.getReachable(formats.getNumber(from),
				only_viewable,
				clear_visited);

	return intToFormat(reachables);
}

bool Converters::isReachable(string const & from, string const & to)
{
	return G_.isReachable(formats.getNumber(from),
			      formats.getNumber(to));
}

Graph::EdgePath const
Converters::getPath(string const & from, string const & to)
{
	return G_.getPath(formats.getNumber(from),
			  formats.getNumber(to));
}

/// The global instance
Converters converters;

// The global copy after reading lyxrc.defaults
Converters system_converters;
