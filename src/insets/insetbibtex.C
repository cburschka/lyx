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
#include "LaTeXFeatures.h"
#include "gettext.h"
#include "metricsinfo.h"
#include "outputparams.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/path.h"

#include <fstream>

using lyx::support::AbsolutePath;
using lyx::support::ascii_lowercase;
using lyx::support::ChangeExtension;
using lyx::support::contains;
using lyx::support::findtexfile;
using lyx::support::IsFileReadable;
using lyx::support::latex_path;
using lyx::support::ltrim;
using lyx::support::MakeAbsPath;
using lyx::support::MakeRelPath;
using lyx::support::Path;
using lyx::support::prefixIs;
using lyx::support::rtrim;
using lyx::support::split;
using lyx::support::subst;
using lyx::support::tokenPos;
using lyx::support::trim;

namespace os = lyx::support::os;

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
		InsetCommandParams p;
		InsetCommandMailer::string2params("bibtex", cmd.argument, p);
		if (!p.getCmdName().empty())
			setParams(p);
		break;
	}

	default:
		InsetCommand::doDispatch(cur, cmd);
		break;
	}
}


string const InsetBibtex::getScreenLabel(Buffer const &) const
{
	return _("BibTeX Generated Bibliography");
}


namespace {

string normalize_name(Buffer const & buffer, OutputParams const & runparams,
		      string const & name, string const & ext)
{
	string const fname = MakeAbsPath(name, buffer.filePath());
	if (AbsolutePath(name) || !IsFileReadable(fname + ext))
		return name;
	else if (!runparams.nice)
		return fname;
	else
		return MakeRelPath(fname, buffer.getMasterBuffer()->filePath());
}

}


int InsetBibtex::latex(Buffer const & buffer, ostream & os,
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

	// the database string
	string adb;
	string db_in = getContents();
	db_in = split(db_in, adb, ',');
	// If we generate in a temp dir, we might need to give an
	// absolute path there. This is a bit complicated since we can
	// have a comma-separated list of bibliographies
	string db_out;
	while (!adb.empty()) {
		db_out += latex_path(normalize_name(buffer, runparams, adb, ".bib"));
		db_out += ',';
		db_in = split(db_in, adb,',');
	}
	db_out = rtrim(db_out, ",");

	// Style-Options
	string style = getOptions(); // maybe empty! and with bibtotoc
	string bibtotoc;
	if (prefixIs(style, "bibtotoc")) {
		bibtotoc = "bibtotoc";
		if (contains(style, ',')) {
			style = split(style, bibtotoc, ',');
		}
	}

	// line count
	int i = 0;

	if (!style.empty()) {
		os << "\\bibliographystyle{"
		   << latex_path(normalize_name(buffer, runparams, style, ".bst"))
		   << "}\n";
		i += 1;
	}

	if (buffer.params().use_bibtopic){
		os << "\\begin{btSect}{" << db_out << "}\n";
		string btprint = getSecOptions();
		if (btprint.empty())
			// default
			btprint = "btPrintCited";
		os << "\\" << btprint << "\n"
		   << "\\end{btSect}\n";
		i += 3;
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

	if (!buffer.params().use_bibtopic){
		os << "\\bibliography{" << db_out << "}\n";
		i += 1;
	}

	return i;
}


vector<string> const InsetBibtex::getFiles(Buffer const & buffer) const
{
	Path p(buffer.filePath());

	vector<string> vec;

	string tmp;
	string bibfiles = getContents();
	bibfiles = split(bibfiles, tmp, ',');
	while (!tmp.empty()) {
		string file = findtexfile(ChangeExtension(tmp, "bib"), "bib");
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
	string contents(getContents());
	if (!contains(contents, db)) {
		if (!contents.empty())
			contents += ',';
		setContents(contents + db);
		return true;
	}
	return false;
}


bool InsetBibtex::delDatabase(string const & db)
{
	if (contains(getContents(), db)) {
		string bd = db;
		int const n = tokenPos(getContents(), ',', bd);
		if (n > 0) {
			// Weird code, would someone care to explain this?(Lgb)
			string tmp(", ");
			tmp += bd;
			setContents(subst(getContents(), tmp, ", "));
		} else if (n == 0)
			setContents(split(getContents(), bd, ','));
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
