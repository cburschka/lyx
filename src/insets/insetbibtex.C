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
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"
#include "latexrunparams.h"
#include "metricsinfo.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/path.h"

#include <fstream>

using lyx::support::ascii_lowercase;
using lyx::support::ChangeExtension;
using lyx::support::contains;
using lyx::support::findtexfile;
using lyx::support::IsFileReadable;
using lyx::support::ltrim;
using lyx::support::MakeAbsPath;
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

using std::ifstream;
using std::ostream;
using std::pair;
using std::vector;


InsetBibtex::InsetBibtex(InsetCommandParams const & p)
	: InsetCommand(p)
{}


InsetBibtex::~InsetBibtex()
{
	InsetCommandMailer("bibtex", *this).hideDialog();
}


std::auto_ptr<InsetBase> InsetBibtex::clone() const
{
	return std::auto_ptr<InsetBase>(new InsetBibtex(*this));
}


void InsetBibtex::metrics(MetricsInfo & mi, Dimension & dim) const
{
	InsetCommand::metrics(mi, dim);
	int center_indent = (mi.base.textwidth - dim.wid) / 2;
	Box b(center_indent, center_indent + dim.wid, -dim.asc, dim.des);
	button().setBox(b);
	dim.wid = mi.base.textwidth;
	dim_ = dim;
}


void InsetBibtex::draw(PainterInfo & pi, int x, int y) const
{
	InsetCommand::draw(pi, x + button().box().x1, y);
}


dispatch_result InsetBibtex::localDispatch(FuncRequest const & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_DIALOG_SHOW:
		InsetCommandMailer("bibtex", *this).showDialog(cmd.view());
		return DISPATCHED;
	case LFUN_MOUSE_RELEASE:
		if (button().box().contains(cmd.x, cmd.y))
			InsetCommandMailer("bibtex", *this).showDialog(cmd.view());
		return DISPATCHED;

	case LFUN_INSET_MODIFY: {
		InsetCommandParams p;
		InsetCommandMailer::string2params(cmd.argument, p);
		if (p.getCmdName().empty())
			return DISPATCHED;
		setParams(p);
		return  DISPATCHED;
	}

	default:
		return InsetCommand::localDispatch(cmd);
	}

}

string const InsetBibtex::getScreenLabel(Buffer const &) const
{
	return _("BibTeX Generated References");
}


int InsetBibtex::latex(Buffer const & buffer, ostream & os,
		       LatexRunParams const & runparams) const
{
	// changing the sequence of the commands
	// 1. \bibliographystyle{style}
	// 2. \addcontentsline{...} - if option bibtotoc set
	// 3. \bibliography{database}
	string adb;
	string db_in = getContents();
	db_in = split(db_in, adb, ',');

	// Style-Options
	string style = getOptions(); // maybe empty! and with bibtotoc
	string bibtotoc;
	if (prefixIs(style, "bibtotoc")) {
		bibtotoc = "bibtotoc";
		if (contains(style, ',')) {
			style = split(style, bibtotoc, ',');
		}
	}

	if (!runparams.nice
	    && IsFileReadable(MakeAbsPath(style, buffer.filePath()) + ".bst")) {
		style = MakeAbsPath(style, buffer.filePath());
	}

	if (!style.empty()) { // we want no \biblio...{}
		os << "\\bibliographystyle{" << style << "}\n";
	}

	// bibtotoc-Option
	if (!bibtotoc.empty()) {
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

	// database
	// If we generate in a temp dir, we might need to give an
	// absolute path there. This is a bit complicated since we can
	// have a comma-separated list of bibliographies
	string db_out;
	while (!adb.empty()) {
		if (!runparams.nice &&
		    IsFileReadable(MakeAbsPath(adb, buffer.filePath())+".bib"))
			 adb = os::external_path(MakeAbsPath(adb, buffer.filePath()));
		db_out += adb;
		db_out += ',';
		db_in = split(db_in, adb,',');
	}
	db_out = rtrim(db_out, ",");
	os   << "\\bibliography{" << db_out << "}\n";
	return 2;
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
