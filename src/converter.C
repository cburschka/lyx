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
#include "support/os.h"
#include "support/path.h"
#include "support/systemcall.h"

using lyx::support::addName;
using lyx::support::bformat;
using lyx::support::changeExtension;
using lyx::support::compare_ascii_no_case;
using lyx::support::contains;
using lyx::support::dirList;
using lyx::support::getExtension;
using lyx::support::isFileReadable;
using lyx::support::libFileSearch;
using lyx::support::libScriptSearch;
using lyx::support::makeRelPath;
using lyx::support::onlyFilename;
using lyx::support::onlyPath;
using lyx::support::Path;
using lyx::support::prefixIs;
using lyx::support::quoteName;
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

	if (bp.papersize != PAPER_CUSTOM) {
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
			 string & to_file, ErrorList & errorList, bool try_default)
{
	string const to_ext = formats.extension(to_format);
	to_file = changeExtension(to_file_base, to_ext);

	if (from_format == to_format)
		return move(from_format, from_file, to_file, false);

	Graph::EdgePath edgepath = getPath(from_format, to_format);
	if (edgepath.empty()) {
		if (try_default) {
			// if no special converter defined, then we take the
			// default one from ImageMagic.
			string const from_ext = from_format.empty() ?
				getExtension(from_file) :
				formats.extension(from_format);
			string const command =
				lyx::support::os::python() + ' ' +
				quoteName(libFileSearch("scripts", "convertDefault.py")) +
				' ' +
				quoteName(from_ext + ':' + from_file) +
				' ' +
				quoteName(to_ext + ':' + to_file);
			lyxerr[Debug::FILES]
				<< "No converter defined! "
				   "I use convertDefault.py:\n\t"
				<< command << endl;
			Systemcall one;
			one.startscript(Systemcall::Wait, command);
			if (isFileReadable(to_file)) {
				return true;
			}
		}
		Alert::error(lyx::to_utf8(_("Cannot convert file")),
			     bformat(lyx::to_utf8(_("No information for converting %1$s "
						    "format files to %2$s.\n"
						    "Define a convertor in the preferences.")),
			from_format, to_format));
		return false;
	}
	OutputParams runparams;
	runparams.flavor = getFlavor(edgepath);
	string path = onlyPath(from_file);
	Path p(path);
	// empty the error list before any new conversion takes place.
	errorList.clear();

	bool run_latex = false;
	string from_base = changeExtension(from_file, "");
	string to_base = changeExtension(to_file, "");
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
			? changeExtension(from_file, conv.To->extension())
			: addName(subst(conv.result_dir,
					token_base, from_base),
				  subst(conv.result_file,
					token_base, onlyFilename(from_base)));

		// if input and output files are equal, we use a
		// temporary file as intermediary (JMarc)
		string real_outfile;
		if (outfile == infile) {
			real_outfile = infile;
			outfile = addName(buffer->temppath(), "tmpfile.out");
		}

		if (conv.latex) {
			run_latex = true;
			string const command = subst(conv.command, token_from, "");
			lyxerr[Debug::FILES] << "Running " << command << endl;
			if (!runLaTeX(*buffer, command, runparams, errorList))
				return false;
		} else {
			if (conv.need_aux && !run_latex
			    && !latex_command_.empty()) {
				lyxerr[Debug::FILES]
					<< "Running " << latex_command_
					<< " to update aux file"<<  endl;
				runLaTeX(*buffer, latex_command_, runparams, errorList);
			}

			string const infile2 = (conv.original_dir)
				? infile : makeRelPath(infile, path);
			string const outfile2 = (conv.original_dir)
				? outfile : makeRelPath(outfile, path);

			string command = conv.command;
			command = subst(command, token_from, quoteName(infile2));
			command = subst(command, token_base, quoteName(from_base));
			command = subst(command, token_to, quoteName(outfile2));
			command = libScriptSearch(command);

			if (!conv.parselog.empty())
				command += " 2> " + quoteName(infile2 + ".out");

			if (conv.from == "dvi" && conv.to == "ps")
				command = add_options(command,
						      buffer->params().dvips_options());
			else if (conv.from == "dvi" && prefixIs(conv.to, "pdf"))
				command = add_options(command,
						      dvipdfm_options(buffer->params()));

			lyxerr[Debug::FILES] << "Calling " << command << endl;
			if (buffer)
				buffer->message(lyx::to_utf8(_("Executing command: "))
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
				// Finally, don't forget to tell any future
				// converters to use the renamed file...
				outfile = real_outfile;
			}

			if (!conv.parselog.empty()) {
				string const logfile =  infile2 + ".log";
				string const script = libScriptSearch(conv.parselog);
				string const command2 = script +
					" < " + quoteName(infile2 + ".out") +
					" > " + quoteName(logfile);
				one.startscript(Systemcall::Wait, command2);
				if (!scanLog(*buffer, command, logfile, errorList))
					return false;
			}

			if (res) {
				if (conv.to == "program") {
					Alert::error(lyx::to_utf8(_("Build errors")),
						lyx::to_utf8(_("There were errors during the build process.")));
				} else {
// FIXME: this should go out of here. For example, here we cannot say if
// it is a document (.lyx) or something else. Same goes for elsewhere.
				Alert::error(lyx::to_utf8(_("Cannot convert file")),
					bformat(lyx::to_utf8(_("An error occurred whilst running %1$s")),
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
		to_file = addName(subst(conv.result_dir, token_base, to_base),
				  subst(conv.result_file,
					token_base, onlyFilename(to_base)));
		if (from_base != to_base) {
			string const from = subst(conv.result_dir,
					    token_base, from_base);
			string const to = subst(conv.result_dir,
					  token_base, to_base);
			Mover const & mover = movers(conv.from);
			if (!mover.rename(from, to)) {
				Alert::error(lyx::to_utf8(_("Cannot convert file")),
					bformat(lyx::to_utf8(_("Could not move a temporary file from %1$s to %2$s.")),
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
	string const path = onlyPath(from);
	string const base = onlyFilename(changeExtension(from, ""));
	string const to_base = changeExtension(to, "");
	string const to_extension = getExtension(to);

	vector<string> files = dirList(onlyPath(from), getExtension(from));
	for (vector<string>::const_iterator it = files.begin();
	     it != files.end(); ++it)
		if (prefixIs(*it, base)) {
			string const from2 = path + *it;
			string to2 = to_base + it->substr(base.length());
			to2 = changeExtension(to2, to_extension);
			lyxerr[Debug::FILES] << "moving " << from2
					     << " to " << to2 << endl;

			Mover const & mover = movers(fmt);
			bool const moved = copy
				? mover.copy(from2, to2)
				: mover.rename(from2, to2);
			if (!moved && no_errors) {
				Alert::error(lyx::to_utf8(_("Cannot convert file")),
					bformat(copy ?
						lyx::to_utf8(_("Could not copy a temporary file from %1$s to %2$s.")) :
						lyx::to_utf8(_("Could not move a temporary file from %1$s to %2$s.")),
						from2, to2));
				no_errors = false;
			}
		}
	return no_errors;
}


bool Converters::convert(Buffer const * buffer,
			 string const & from_file, string const & to_file_base,
			 string const & from_format, string const & to_format,
			 ErrorList & errorList, bool try_default)
{
	string to_file;
	return convert(buffer, from_file, to_file_base, from_format, to_format,
		       to_file, errorList, try_default);
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
			 string const & filename, ErrorList & errorList)
{
	OutputParams runparams;
	runparams.flavor = OutputParams::LATEX;
	LaTeX latex("", runparams, filename, "");
	TeXErrors terr;
	int const result = latex.scanLogFile(terr);

	if (result & LaTeX::ERRORS)
		bufferErrors(buffer, terr, errorList);

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
			  OutputParams const & runparams, ErrorList & errorList)
{
	buffer.busy(true);
	buffer.message(lyx::to_utf8(_("Running LaTeX...")));

	runparams.document_language = buffer.params().language->babel();

	// do the LaTeX run(s)
	string const name = buffer.getLatexName();
	LaTeX latex(command, runparams, name, buffer.filePath());
	TeXErrors terr;
	showMessage show(buffer);
	latex.message.connect(show);
	int const result = latex.run(terr);

	if (result & LaTeX::ERRORS)
		bufferErrors(buffer, terr, errorList);

	// check return value from latex.run().
	if ((result & LaTeX::NO_LOGFILE)) {
		string const str =
			bformat(lyx::to_utf8(_("LaTeX did not run successfully. "
					       "Additionally, LyX could not locate "
					       "the LaTeX log %1$s.")), name);
		Alert::error(lyx::to_utf8(_("LaTeX failed")), str);
	} else if (result & LaTeX::NO_OUTPUT) {
		Alert::warning(lyx::to_utf8(_("Output is empty")),
			       lyx::to_utf8(_("An empty output file was generated.")));
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
