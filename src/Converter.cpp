/**
 * \file Converter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Converter.h"

#include "ConverterCache.h"
#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "ErrorList.h"
#include "Format.h"
#include "Language.h"
#include "LaTeX.h"
#include "Mover.h"

#include "frontends/alert.h"

#include "support/debug.h"
#include "support/FileNameList.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/Path.h"
#include "support/Systemcall.h"

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = lyx::frontend::Alert;


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
		string const paper_size = bp.paperSizeName(BufferParams::DVIPDFM);
		if (!paper_size.empty())
			result = "-p "+ paper_size;

		if (bp.orientation == ORIENTATION_LANDSCAPE)
			result += " -l";
	}

	return result;
}


class ConverterEqual {
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
	  need_aux(false)
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
			 FileName const & from_file, FileName const & to_file,
			 FileName const & orig_from,
			 string const & from_format, string const & to_format,
			 ErrorList & errorList, int conversionflags)
{
	if (from_format == to_format)
		return move(from_format, from_file, to_file, false);

	if ((conversionflags & try_cache) &&
	    ConverterCache::get().inCache(orig_from, to_format))
		return ConverterCache::get().copy(orig_from, to_format, to_file);

	Graph::EdgePath edgepath = getPath(from_format, to_format);
	if (edgepath.empty()) {
		if (conversionflags & try_default) {
			// if no special converter defined, then we take the
			// default one from ImageMagic.
			string const from_ext = from_format.empty() ?
				getExtension(from_file.absFilename()) :
				formats.extension(from_format);
			string const to_ext = formats.extension(to_format);
			string const command =
				os::python() + ' ' +
				quoteName(libFileSearch("scripts", "convertDefault.py").toFilesystemEncoding()) +
				' ' +
				quoteName(from_ext + ':' + from_file.toFilesystemEncoding()) +
				' ' +
				quoteName(to_ext + ':' + to_file.toFilesystemEncoding());
			LYXERR(Debug::FILES, "No converter defined! "
				   "I use convertDefault.py:\n\t" << command);
			Systemcall one;
			one.startscript(Systemcall::Wait, command);
			if (to_file.isReadableFile()) {
				if (conversionflags & try_cache)
					ConverterCache::get().add(orig_from,
							to_format, to_file);
				return true;
			}
		}
		Alert::error(_("Cannot convert file"),
			     bformat(_("No information for converting %1$s "
						    "format files to %2$s.\n"
						    "Define a converter in the preferences."),
							from_ascii(from_format), from_ascii(to_format)));
		return false;
	}

	// buffer is only invalid for importing, and then runparams is not
	// used anyway.
	OutputParams runparams(buffer ? &buffer->params().encoding() : 0);
	runparams.flavor = getFlavor(edgepath);

	// Some converters (e.g. lilypond) can only output files to the
	// current directory, so we need to change the current directory.
	// This has the added benefit that all other files that may be
	// generated by the converter are deleted when LyX closes and do not
	// clutter the real working directory.
	string const path(onlyPath(from_file.absFilename()));
	// Prevent the compiler from optimizing away p
	FileName pp(path);
	PathChanger p(pp);

	// empty the error list before any new conversion takes place.
	errorList.clear();

	bool run_latex = false;
	string from_base = changeExtension(from_file.absFilename(), "");
	string to_base = changeExtension(to_file.absFilename(), "");
	FileName infile;
	FileName outfile = from_file;
	for (Graph::EdgePath::const_iterator cit = edgepath.begin();
	     cit != edgepath.end(); ++cit) {
		Converter const & conv = converterlist_[*cit];
		bool dummy = conv.To->dummy() && conv.to != "program";
		if (!dummy) {
			LYXERR(Debug::FILES, "Converting from  "
			       << conv.from << " to " << conv.to);
		}
		infile = outfile;
		outfile = FileName(conv.result_dir.empty()
			? changeExtension(from_file.absFilename(), conv.To->extension())
			: addName(subst(conv.result_dir,
					token_base, from_base),
				  subst(conv.result_file,
					token_base, onlyFilename(from_base))));

		// if input and output files are equal, we use a
		// temporary file as intermediary (JMarc)
		FileName real_outfile;
		if (outfile == infile) {
			real_outfile = infile;
			// when importing, a buffer does not necessarily exist
			if (buffer)
				outfile = FileName(addName(buffer->temppath(), "tmpfile.out"));
			else
				outfile = FileName(addName(package().temp_dir().absFilename(),
						   "tmpfile.out"));
		}

		if (conv.latex) {
			run_latex = true;
			string const command = subst(conv.command, token_from, "");
			LYXERR(Debug::FILES, "Running " << command);
			if (!runLaTeX(*buffer, command, runparams, errorList))
				return false;
		} else {
			if (conv.need_aux && !run_latex
			    && !latex_command_.empty()) {
				LYXERR(Debug::FILES, "Running " << latex_command_
					<< " to update aux file");
				runLaTeX(*buffer, latex_command_, runparams, errorList);
			}

			// FIXME UNICODE
			string const infile2 = 
				to_utf8(makeRelPath(from_utf8(infile.absFilename()), from_utf8(path)));
			string const outfile2 = 
				to_utf8(makeRelPath(from_utf8(outfile.absFilename()), from_utf8(path)));

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

			LYXERR(Debug::FILES, "Calling " << command);
			if (buffer)
				buffer->message(_("Executing command: ")
				+ from_utf8(command));

			Systemcall one;
			int res;
			if (dummy) {
				res = one.startscript(Systemcall::DontWait,
					to_filesystem8bit(from_utf8(command)));
				// We're not waiting for the result, so we can't do anything
				// else here.
			} else {
				res = one.startscript(Systemcall::Wait,
						to_filesystem8bit(from_utf8(command)));
				if (!real_outfile.empty()) {
					Mover const & mover = getMover(conv.to);
					if (!mover.rename(outfile, real_outfile))
						res = -1;
					else
						LYXERR(Debug::FILES, "renaming file " << outfile
							<< " to " << real_outfile);
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
					one.startscript(Systemcall::Wait,
						to_filesystem8bit(from_utf8(command2)));
					if (!scanLog(*buffer, command, makeAbsPath(logfile, path), errorList))
						return false;
				}
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
						from_utf8(command.substr(0, 50))));
				}
				return false;
			}
		}
	}

	Converter const & conv = converterlist_[edgepath.back()];
	if (conv.To->dummy())
		return true;

	if (!conv.result_dir.empty()) {
		// The converter has put the file(s) in a directory.
		// In this case we ignore the given to_file.
		if (from_base != to_base) {
			string const from = subst(conv.result_dir,
					    token_base, from_base);
			string const to = subst(conv.result_dir,
					  token_base, to_base);
			Mover const & mover = getMover(conv.from);
			if (!mover.rename(FileName(from), FileName(to))) {
				Alert::error(_("Cannot convert file"),
					bformat(_("Could not move a temporary directory from %1$s to %2$s."),
						from_utf8(from), from_utf8(to)));
				return false;
			}
		}
		return true;
	} else {
		if (conversionflags & try_cache)
			ConverterCache::get().add(orig_from, to_format, outfile);
		return move(conv.to, outfile, to_file, conv.latex);
	}
}


bool Converters::move(string const & fmt,
		      FileName const & from, FileName const & to, bool copy)
{
	if (from == to)
		return true;

	bool no_errors = true;
	string const path = onlyPath(from.absFilename());
	string const base = onlyFilename(removeExtension(from.absFilename()));
	string const to_base = removeExtension(to.absFilename());
	string const to_extension = getExtension(to.absFilename());

	support::FileNameList const files = FileName(path).dirList(getExtension(from.absFilename()));
	for (support::FileNameList::const_iterator it = files.begin();
	     it != files.end(); ++it) {
		string const from2 = it->absFilename();
		string const file2 = onlyFilename(from2);
		if (prefixIs(file2, base)) {
			string const to2 = changeExtension(
				to_base + file2.substr(base.length()),
				to_extension);
			LYXERR(Debug::FILES, "moving " << from2 << " to " << to2);

			Mover const & mover = getMover(fmt);
			bool const moved = copy
				? mover.copy(*it, FileName(to2))
				: mover.rename(*it, FileName(to2));
			if (!moved && no_errors) {
				Alert::error(_("Cannot convert file"),
					bformat(copy ?
						_("Could not copy a temporary file from %1$s to %2$s.") :
						_("Could not move a temporary file from %1$s to %2$s."),
						from_utf8(from2), from_utf8(to2)));
				no_errors = false;
			}
		}
	}
	return no_errors;
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
			 FileName const & filename, ErrorList & errorList)
{
	OutputParams runparams(0);
	runparams.flavor = OutputParams::LATEX;
	LaTeX latex("", runparams, filename);
	TeXErrors terr;
	int const result = latex.scanLogFile(terr);

	if (result & LaTeX::ERRORS)
		buffer.bufferErrors(terr, errorList);

	return true;
}


namespace {

class ShowMessage
	: public boost::signals::trackable {
public:
	ShowMessage(Buffer const & b) : buffer_(b) {}
	void operator()(docstring const & msg) const { buffer_.message(msg); }
private:
	Buffer const & buffer_;
};

}


bool Converters::runLaTeX(Buffer const & buffer, string const & command,
			  OutputParams const & runparams, ErrorList & errorList)
{
	buffer.setBusy(true);
	buffer.message(_("Running LaTeX..."));

	runparams.document_language = buffer.params().language->babel();

	// do the LaTeX run(s)
	string const name = buffer.latexName();
	LaTeX latex(command, runparams, FileName(makeAbsPath(name)));
	TeXErrors terr;
	ShowMessage show(buffer);
	latex.message.connect(show);
	int const result = latex.run(terr);

	if (result & LaTeX::ERRORS)
		buffer.bufferErrors(terr, errorList);

	// check return value from latex.run().
	if ((result & LaTeX::NO_LOGFILE)) {
		docstring const str =
			bformat(_("LaTeX did not run successfully. "
					       "Additionally, LyX could not locate "
					       "the LaTeX log %1$s."), from_utf8(name));
		Alert::error(_("LaTeX failed"), str);
	} else if (result & LaTeX::NO_OUTPUT) {
		Alert::warning(_("Output is empty"),
			       _("An empty output file was generated."));
	}


	buffer.setBusy(false);

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


vector<Format const *> const
Converters::intToFormat(vector<int> const & input)
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


Graph::EdgePath Converters::getPath(string const & from, string const & to)
{
	return G_.getPath(formats.getNumber(from),
			  formats.getNumber(to));
}


vector<Format const *> Converters::importableFormats()
{
	vector<string> l = loaders();
	vector<Format const *> result = getReachableTo(l[0], true);
	for (vector<string>::const_iterator it = l.begin() + 1;
	     it != l.end(); ++it) {
		vector<Format const *> r = getReachableTo(*it, false);
		result.insert(result.end(), r.begin(), r.end());
	}
	return result;
}


vector<string> Converters::loaders() const
{
	vector<string> v;
	v.push_back("lyx");
	v.push_back("text");
	v.push_back("textparagraph");
	return v;
}


} // namespace lyx
