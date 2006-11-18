/**
 * \file insetbibtex.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetbibtex.h"

#include "buffer.h"
#include "bufferparams.h"
#include "dispatchresult.h"
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "metricsinfo.h"
#include "outputparams.h"

#include "frontends/Alert.h"

#include "support/filename.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/os.h"
#include "support/path.h"

#include <boost/tokenizer.hpp>

#include <fstream>
#include <sstream>


namespace lyx {

using support::absolutePath;
using support::ascii_lowercase;
using support::changeExtension;
using support::contains;
using support::copy;
using support::DocFileName;
using support::findtexfile;
using support::isFileReadable;
using support::latex_path;
using support::ltrim;
using support::makeAbsPath;
using support::makeRelPath;
using support::Path;
using support::prefixIs;
using support::removeExtension;
using support::rtrim;
using support::split;
using support::subst;
using support::tokenPos;
using support::trim;

namespace Alert = frontend::Alert;
namespace os = support::os;

using std::endl;
using std::getline;
using std::string;
using std::ifstream;
using std::ostream;
using std::pair;
using std::vector;


InsetBibtex::InsetBibtex(InsetCommandParams const & p)
	: InsetCommand(p, "bibtex")
{}


std::auto_ptr<InsetBase> InsetBibtex::doClone() const
{
	return std::auto_ptr<InsetBase>(new InsetBibtex(*this));
}


void InsetBibtex::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p("bibtex");
		InsetCommandMailer::string2params("bibtex", to_utf8(cmd.argument()), p);
		if (!p.getCmdName().empty()) {
			setParams(p);
			cur.buffer().updateBibfilesCache();
		} else
			cur.noUpdate();
		break;
	}

	default:
		InsetCommand::doDispatch(cur, cmd);
		break;
	}
}


docstring const InsetBibtex::getScreenLabel(Buffer const &) const
{
	return _("BibTeX Generated Bibliography");
}


namespace {

string normalize_name(Buffer const & buffer, OutputParams const & runparams,
		      string const & name, string const & ext)
{
	string const fname = makeAbsPath(name, buffer.filePath());
	if (absolutePath(name) || !isFileReadable(fname + ext))
		return name;
	else if (!runparams.nice)
		return fname;
	else
		return makeRelPath(fname, buffer.getMasterBuffer()->filePath());
}

}


int InsetBibtex::latex(Buffer const & buffer, odocstream & os,
		       OutputParams const & runparams) const
{
	// the sequence of the commands:
	// 1. \bibliographystyle{style}
	// 2. \addcontentsline{...} - if option bibtotoc set
	// 3. \bibliography{database}
	// and with bibtopic:
	// 1. \bibliographystyle{style}
	// 2. \begin{btSect}{database}
	// 3. \btPrint{Cited|NotCited|All}
	// 4. \end{btSect}

	// Database(s)
	// If we are processing the LaTeX file in a temp directory then
	// copy the .bib databases to this temp directory, mangling their
	// names in the process. Store this mangled name in the list of
	// all databases.
	// (We need to do all this because BibTeX *really*, *really*
	// can't handle "files with spaces" and Windows users tend to
	// use such filenames.)
	// Otherwise, store the (maybe absolute) path to the original,
	// unmangled database name.
	typedef boost::char_separator<char_type> Separator;
	typedef boost::tokenizer<Separator, docstring::const_iterator, docstring> Tokenizer;

	Separator const separator(from_ascii(",").c_str());
	// The tokenizer must not be called with temporary strings, since
	// it does not make a copy and uses iterators of the string further
	// down. getParam returns a reference, so this is OK.
	Tokenizer const tokens(getParam("bibfiles"), separator);
	Tokenizer::const_iterator const begin = tokens.begin();
	Tokenizer::const_iterator const end = tokens.end();

	odocstringstream dbs;
	for (Tokenizer::const_iterator it = begin; it != end; ++it) {
		docstring const input = trim(*it);
		// FIXME UNICODE
		string utf8input(to_utf8(input));
		string database =
			normalize_name(buffer, runparams, utf8input, ".bib");
		string const in_file = database + ".bib";

		if (!runparams.inComment && !runparams.dryrun && !runparams.nice &&
		    isFileReadable(in_file)) {

			// mangledFilename() needs the extension
			database = removeExtension(DocFileName(in_file).mangledFilename());
			string const out_file = makeAbsPath(database + ".bib",
					buffer.getMasterBuffer()->temppath());

			bool const success = copy(in_file, out_file);
			if (!success) {
				lyxerr << "Failed to copy '" << in_file
				       << "' to '" << out_file << "'"
				       << endl;
			}
		}

		if (it != begin)
			dbs << ',';
		// FIXME UNICODE
		dbs << from_utf8(latex_path(database));
	}
	docstring const db_out = dbs.str();

	// Post this warning only once.
	static bool warned_about_spaces = false;
	if (!warned_about_spaces &&
	    runparams.nice && db_out.find(' ') != docstring::npos) {
		warned_about_spaces = true;

		Alert::warning(_("Export Warning!"),
			       _("There are spaces in the paths to your BibTeX databases.\n"
					      "BibTeX will be unable to find them."));

	}

	// Style-Options
	string style = to_utf8(getParam("options")); // maybe empty! and with bibtotoc
	string bibtotoc;
	if (prefixIs(style, "bibtotoc")) {
		bibtotoc = "bibtotoc";
		if (contains(style, ',')) {
			style = split(style, bibtotoc, ',');
		}
	}

	// line count
	int nlines = 0;

	if (!style.empty()) {
		string base =
			normalize_name(buffer, runparams, style, ".bst");
		string const in_file = base + ".bst";
		// If this style does not come from texmf and we are not
		// exporting to .tex copy it to the tmp directory.
		// This prevents problems with spaces and 8bit charcaters
		// in the file name.
		if (!runparams.inComment && !runparams.dryrun && !runparams.nice &&
		    isFileReadable(in_file)) {
			// use new style name
			base = removeExtension(
					DocFileName(in_file).mangledFilename());
			string const out_file = makeAbsPath(base + ".bst",
					buffer.getMasterBuffer()->temppath());
			bool const success = copy(in_file, out_file);
			if (!success) {
				lyxerr << "Failed to copy '" << in_file
				       << "' to '" << out_file << "'"
				       << endl;
			}
		}
		// FIXME UNICODE
		os << "\\bibliographystyle{"
		   << from_utf8(latex_path(normalize_name(buffer, runparams, base, ".bst")))
		   << "}\n";
		nlines += 1;
	}

	// Post this warning only once.
	static bool warned_about_bst_spaces = false;
	if (!warned_about_bst_spaces && runparams.nice && contains(style, ' ')) {
		warned_about_bst_spaces = true;
		Alert::warning(_("Export Warning!"),
			       _("There are spaces in the path to your BibTeX style file.\n"
					      "BibTeX will be unable to find it."));
	}

	if (!db_out.empty() && buffer.params().use_bibtopic){
		os << "\\begin{btSect}{" << db_out << "}\n";
		docstring btprint = getParam("btprint");
		if (btprint.empty())
			// default
			btprint = from_ascii("btPrintCited");
		os << "\\" << btprint << "\n"
		   << "\\end{btSect}\n";
		nlines += 3;
	}

	// bibtotoc-Option
	if (!bibtotoc.empty() && !buffer.params().use_bibtopic) {
		// maybe a problem when a textclass has no "art" as
		// part of its name, because it's than book.
		// For the "official" lyx-layouts it's no problem to support
		// all well
		if (!contains(buffer.params().getLyXTextClass().name(),
			      "art")) {
			if (buffer.params().sides == LyXTextClass::OneSide) {
				// oneside
				os << "\\clearpage";
			} else {
				// twoside
				os << "\\cleardoublepage";
			}

			// bookclass
			os << "\\addcontentsline{toc}{chapter}{\\bibname}";

		} else {
			// article class
			os << "\\addcontentsline{toc}{section}{\\refname}";
		}
	}

	if (!db_out.empty() && !buffer.params().use_bibtopic){
		os << "\\bibliography{" << db_out << "}\n";
		nlines += 1;
	}

	return nlines;
}


vector<string> const InsetBibtex::getFiles(Buffer const & buffer) const
{
	Path p(buffer.filePath());

	vector<string> vec;

	string tmp;
	// FIXME UNICODE
	string bibfiles = to_utf8(getParam("bibfiles"));
	bibfiles = split(bibfiles, tmp, ',');
	while (!tmp.empty()) {
		string file = findtexfile(changeExtension(tmp, "bib"), "bib");
		lyxerr[Debug::LATEX] << "Bibfile: " << file << endl;

		// If we didn't find a matching file name just fail silently
		if (!file.empty())
			vec.push_back(file);

		// Get next file name
		bibfiles = split(bibfiles, tmp, ',');
	}

	return vec;
}


// This method returns a comma separated list of Bibtex entries
void InsetBibtex::fillWithBibKeys(Buffer const & buffer,
				  std::vector<std::pair<string, string> > & keys) const
{
	vector<string> const files = getFiles(buffer);
	for (vector<string>::const_iterator it = files.begin();
	     it != files.end(); ++ it) {
		// This is a _very_ simple parser for Bibtex database
		// files. All it does is to look for lines starting
		// in @ and not being @preamble and @string entries.
		// It does NOT do any syntax checking!
		ifstream ifs(it->c_str());
		string linebuf0;
		while (getline(ifs, linebuf0)) {
			string linebuf = trim(linebuf0);
			if (linebuf.empty()) continue;
			if (prefixIs(linebuf, "@")) {
				linebuf = subst(linebuf, '{', '(');
				string tmp;
				linebuf = split(linebuf, tmp, '(');
				tmp = ascii_lowercase(tmp);
				if (!prefixIs(tmp, "@string")
				    && !prefixIs(tmp, "@preamble")) {
					linebuf = split(linebuf, tmp, ',');
					tmp = ltrim(tmp, " \t");
					if (!tmp.empty()) {
						keys.push_back(pair<string,string>(tmp,string()));
					}
				}
			} else if (!keys.empty()) {
				keys.back().second += linebuf + "\n";
			}
		}
	}
}


bool InsetBibtex::addDatabase(string const & db)
{
	// FIXME UNICODE
	string bibfiles(to_utf8(getParam("bibfiles")));
	if (tokenPos(bibfiles, ',', db) == -1) {
		if (!bibfiles.empty())
			bibfiles += ',';
		setParam("bibfiles", from_utf8(bibfiles + db));
		return true;
	}
	return false;
}


bool InsetBibtex::delDatabase(string const & db)
{
	// FIXME UNICODE
	string bibfiles(to_utf8(getParam("bibfiles")));
	if (contains(bibfiles, db)) {
		int const n = tokenPos(bibfiles, ',', db);
		string bd = db;
		if (n > 0) {
			// this is not the first database
			string tmp = ',' + bd;
			setParam("bibfiles", from_utf8(subst(bibfiles, tmp, string())));
		} else if (n == 0)
			// this is the first (or only) database
			setParam("bibfiles", from_utf8(split(bibfiles, bd, ',')));
		else
			return false;
	}
	return true;
}


void InsetBibtex::validate(LaTeXFeatures & features) const
{
	if (features.bufferParams().use_bibtopic)
		features.require("bibtopic");
}


} // namespace lyx
