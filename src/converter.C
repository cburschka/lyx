/**
 * \file converter.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "converter.h"

#include "buffer.h"
#include "buffer_funcs.h"
#include "bufferparams.h"
#include "debug.h"
#include "format.h"
#include "gettext.h"
#include "language.h"
#include "LaTeX.h"
#include "mover.h"

#include "frontends/Alert.h"

#include "support/filetools.h"
#include "support/lyxlib.h"
#include "support/path.h"
#include "support/systemcall.h"

using lyx::support::AddName;
using lyx::support::bformat;
using lyx::support::ChangeExtension;
using lyx::support::compare_ascii_no_case;
using lyx::support::contains;
using lyx::support::DirList;
using lyx::support::GetExtension;
using lyx::support::IsFileReadable;
using lyx::support::LibFileSearch;
using lyx::support::LibScriptSearch;
using lyx::support::MakeRelPath;
using lyx::support::OnlyFilename;
using lyx::support::OnlyPath;
using lyx::support::Path;
using lyx::support::prefixIs;
using lyx::support::QuoteName;
using lyx::support::split;
using lyx::support::subst;
using lyx::support::Systemcall;

using std::endl;
using std::find_if;
using std::string;
using std::vector;
using std::distance;


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


string const dvipdfm_options(BufferParams const & bp)
{
	string result;

	if (bp.papersize2 != VM_PAPER_CUSTOM) {
		string const paper_size = bp.paperSizeName();
		if (paper_size != "b5" && paper_size != "foolscap")
			result = "-p "+ paper_size;

		if (bp.orientation == ORIENTATION_LANDSCAPE)
			result += " -l";
	}

	return result;
}


class ConverterEqual : public std::binary_function<string, string, bool> {
public:
	ConverterEqual(string const & from, string const & to)
		: from_(from), to_(to) {}
	bool operator()(Converter const & c) const {
		return c.from == from_ && c.to == to_;
	}
private:
	string const from_;
	string const to_;
};

} // namespace anon


Converter::Converter(string const & f, string const & t,
		     string const & c, string const & l)
	: from(f), to(t), command(c), flags(l),
	  From(0), To(0), latex(false), xml(false),
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
		else if (flag_name == "xml")
			xml = true;
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
		return compare_ascii_no_case(a.To->prettyname(),
					     b.To->prettyname()) < 0;
	else
		return i < 0;
}


Converter const * Converters::getConverter(string const & from,
					    string const & to) const
{
	ConverterList::const_iterator const cit =
		find_if(converterlist_.begin(), converterlist_.end(),
			ConverterEqual(from, to));
	if (cit != converterlist_.end())
		return &(*cit);
	else
		return 0;
}


int Converters::getNumber(string const & from, string const & to) const
{
	ConverterList::const_iterator const cit =
		find_if(converterlist_.begin(), converterlist_.end(),
			ConverterEqual(from, to));
	if (cit != converterlist_.end())
		return distance(converterlist_.begin(), cit);
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
					     ConverterEqual(from , to));

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
	ConverterList::iterator const it =
		find_if(converterlist_.begin(),
			converterlist_.end(),
			ConverterEqual(from, to));
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


OutputParams::FLAVOR Converters::getFlavor(Graph::EdgePath const & path)
{
	for (Graph::EdgePath::const_iterator cit = path.begin();
	     cit != path.end(); ++cit) {
		Converter const & conv = converterlist_[*cit];
		if (conv.latex)
			if (contains(conv.to, "pdf"))
				return OutputParams::PDFLATEX;
		if (conv.xml)
			return OutputParams::XML;
	}
	return OutputParams::LATEX;
}


bool Converters::convert(Buffer const * buffer,
			 string const & from_file, string const & to_file_base,
			 string const & from_format, string const & to_format,
			 string & to_file, bool try_default)
{
	string const to_ext = formats.extension(to_format);
	to_file = ChangeExtension(to_file_base, to_ext);

	if (from_format == to_format)
		return move(from_format, from_file, to_file, false);

	Graph::EdgePath edgepath = getPath(from_format, to_format);
	if (edgepath.empty()) {
		if (try_default) {
			// if no special converter defined, then we take the
			// default one from ImageMagic.
			string const from_ext = formats.extension(from_format);
			string const command =
				"sh " +
				QuoteName(LibFileSearch("scripts", "convertDefault.sh")) +
				' ' +
				QuoteName(from_ext + ':' + from_file) +
				' ' +
				QuoteName(to_ext + ':' + to_file);
			lyxerr[Debug::FILES]
				<< "No converter defined! "
				   "I use convertDefault.sh:\n\t"
				<< command << endl;
			Systemcall one;
			one.startscript(Systemcall::Wait, command);
			if (IsFileReadable(to_file)) {
				return true;
			}
		}
		Alert::error(_("Cannot convert file"),
			bformat(_("No information for converting %1$s "
				"format files to %2$s.\n"
				"Try defining a convertor in the preferences."),
			from_format, to_format));
		return false;
	}
	OutputParams runparams;
	runparams.flavor = getFlavor(edgepath);
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
			outfile = AddName(buffer->temppath(), "tmpfile.out");
		}

		if (conv.latex) {
			run_latex = true;
			string const command = subst(conv.command, token_from, "");
			lyxerr[Debug::FILES] << "Running " << command << endl;
			if (!runLaTeX(*buffer, command, runparams))
				return false;
		} else {
			if (conv.need_aux && !run_latex
			    && !latex_command_.empty()) {
				lyxerr[Debug::FILES]
					<< "Running " << latex_command_
					<< " to update aux file"<<  endl;
				runLaTeX(*buffer, latex_command_, runparams);
			}

			string const infile2 = (conv.original_dir)
				? infile : MakeRelPath(infile, path);
			string const outfile2 = (conv.original_dir)
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
						      buffer->params().dvips_options());
			else if (conv.from == "dvi" && prefixIs(conv.to, "pdf"))
				command = add_options(command,
						      dvipdfm_options(buffer->params()));

			lyxerr[Debug::FILES] << "Calling " << command << endl;
			if (buffer)
				buffer->message(_("Executing command: ")
					+ command);

			Systemcall::Starttype const type = (dummy)
				? Systemcall::DontWait : Systemcall::Wait;
			Systemcall one;
			int res;
			if (conv.original_dir) {
				Path p(buffer->filePath());
				res = one.startscript(type, command);
			} else
				res = one.startscript(type, command);

			if (!real_outfile.empty()) {
				Mover const & mover = movers(conv.to);
				if (!mover.rename(outfile, real_outfile))
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
				if (!scanLog(*buffer, command, logfile))
					return false;
			}

			if (res) {
				if (conv.to == "program") {
					Alert::error(_("Build errors"),
						_("There were errors during the build process."));
				} else {
// FIXME: this should go out of here. For example, here we cannot say if
// it is a document (.lyx) or something else. Same goes for elsewhere.
				Alert::error(_("Cannot convert file"),
					bformat(_("An error occurred whilst running %1$s"),
						command.substr(0, 50)));
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
			string const from = subst(conv.result_dir,
					    token_base, from_base);
			string const to = subst(conv.result_dir,
					  token_base, to_base);
			Mover const & mover = movers(conv.from);
			if (!mover.rename(from, to)) {
				Alert::error(_("Cannot convert file"),
					bformat(_("Could not move a temporary file from %1$s to %2$s."),
						from, to));
				return false;
			}
		}
		return true;
	} else
		return move(conv.to, outfile, to_file, conv.latex);
}


bool Converters::move(string const & fmt,
		      string const & from, string const & to, bool copy)
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

			Mover const & mover = movers(fmt);
			bool const moved = copy
				? mover.copy(from2, to2)
				: mover.rename(from2, to2);
			if (!moved && no_errors) {
				Alert::error(_("Cannot convert file"),
					bformat(copy ?
					        _("Could not copy a temporary file from %1$s to %2$s.") :
					        _("Could not move a temporary file from %1$s to %2$s."),
						from2, to2));
				no_errors = false;
			}
		}
	return no_errors;
}


bool Converters::convert(Buffer const * buffer,
			 string const & from_file, string const & to_file_base,
			 string const & from_format, string const & to_format,
                         bool try_default)
{
	string to_file;
	return convert(buffer, from_file, to_file_base, from_format, to_format,
		       to_file, try_default);
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


bool Converters::scanLog(Buffer const & buffer, string const & /*command*/,
			 string const & filename)
{
	OutputParams runparams;
	runparams.flavor = OutputParams::LATEX;
	LaTeX latex("", runparams, filename, "");
	TeXErrors terr;
	int const result = latex.scanLogFile(terr);

	if (result & LaTeX::ERRORS)
		bufferErrors(buffer, terr);

	return true;
}


namespace {

class showMessage : public std::unary_function<string, void>, public boost::signals::trackable {
public:
	showMessage(Buffer const & b) : buffer_(b) {};
	void operator()(string const & m) const
	{
		buffer_.message(m);
	}
private:
	Buffer const & buffer_;
};

}


bool Converters::runLaTeX(Buffer const & buffer, string const & command,
			  OutputParams const & runparams)
{
	buffer.busy(true);
	buffer.message(_("Running LaTeX..."));

	runparams.document_language = buffer.params().language->babel();

	// do the LaTeX run(s)
	string const name = buffer.getLatexName();
	LaTeX latex(command, runparams, name, buffer.filePath());
	TeXErrors terr;
	showMessage show(buffer);
	latex.message.connect(show);
	int const result = latex.run(terr);

	if (result & LaTeX::ERRORS)
		bufferErrors(buffer, terr);

	// check return value from latex.run().
	if ((result & LaTeX::NO_LOGFILE)) {
		string const str =
			bformat(_("LaTeX did not run successfully. "
				  "Additionally, LyX could not locate "
				  "the LaTeX log %1$s."), name);
		Alert::error(_("LaTeX failed"), str);
	} else if (result & LaTeX::NO_OUTPUT) {
		Alert::warning(_("Output is empty"),
			       _("An empty output file was generated."));
	}


	buffer.busy(false);

	int const ERROR_MASK =
			LaTeX::NO_LOGFILE |
			LaTeX::ERRORS |
			LaTeX::NO_OUTPUT;

	return (result & ERROR_MASK) == 0;

}



void Converters::buildGraph()
{
	G_.init(formats.size());
	ConverterList::iterator beg = converterlist_.begin();
	ConverterList::iterator const end = converterlist_.end();
	for (ConverterList::iterator it = beg; it != end ; ++it) {
		int const s = formats.getNumber(it->from);
		int const t = formats.getNumber(it->to);
		G_.addEdge(s,t);
	}
}


std::vector<Format const *> const
Converters::intToFormat(std::vector<int> const & input)
{
	vector<Format const *> result(input.size());

	vector<int>::const_iterator it = input.begin();
	vector<int>::const_iterator const end = input.end();
	vector<Format const *>::iterator rit = result.begin();
	for ( ; it != end; ++it, ++rit) {
		*rit = &formats.get(*it);
	}
	return result;
}


vector<Format const *> const
Converters::getReachableTo(string const & target, bool const clear_visited)
{
	vector<int> const & reachablesto =
		G_.getReachableTo(formats.getNumber(target), clear_visited);

	return intToFormat(reachablesto);
}


vector<Format const *> const
Converters::getReachable(string const & from, bool const only_viewable,
			 bool const clear_visited)
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
