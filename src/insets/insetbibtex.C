/**
 * \file insetbibtex.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 *
 * Full author contact details are available in file CREDITS
 */
#include <config.h>


#include "insetbibtex.h"
#include "buffer.h"
#include "BufferView.h"
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"

#include "support/filetools.h"
#include "support/path.h"
#include "support/os.h"
#include "support/lstrings.h"
#include "support/LAssert.h"

#include <fstream>
#include <cstdlib>

using std::ostream;
using std::ifstream;
using std::getline;
using std::endl;
using std::vector;
using std::pair;


InsetBibtex::InsetBibtex(InsetCommandParams const & p, bool)
	: InsetCommand(p)
{}


InsetBibtex::~InsetBibtex()
{
	InsetCommandMailer mailer("bibtex", *this);
	mailer.hideDialog();
}


dispatch_result InsetBibtex::localDispatch(FuncRequest const & cmd)
{
	Inset::RESULT result = UNDISPATCHED;

	switch (cmd.action) {
	case LFUN_INSET_MODIFY: {
		InsetCommandParams p;
		InsetCommandMailer::string2params(cmd.argument, p);
		if (p.getCmdName().empty())
			break;

		if (view() && p.getContents() != params().getContents()) {
			view()->ChangeCitationsIfUnique(params().getContents(),
							p.getContents());
		}

		setParams(p);
		cmd.view()->updateInset(this, true);
		result = DISPATCHED;
	}
	break;
	default:
		result = InsetCommand::localDispatch(cmd);
	}

	return result;
}

string const InsetBibtex::getScreenLabel(Buffer const *) const
{
	return _("BibTeX Generated References");
}


int InsetBibtex::latex(Buffer const * buffer, ostream & os,
		       bool /*fragile*/, bool/*fs*/) const
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

	if (!buffer->niceFile
	    && IsFileReadable(MakeAbsPath(style, buffer->filePath()) + ".bst")) {
		style = MakeAbsPath(style, buffer->filePath());
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
		if (!contains(buffer->params.getLyXTextClass().name(),
			      "art")) {
			if (buffer->params.sides == LyXTextClass::OneSide) {
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
		if (!buffer->niceFile &&
		    IsFileReadable(MakeAbsPath(adb, buffer->filePath())+".bib"))
			 adb = os::external_path(MakeAbsPath(adb, buffer->filePath()));
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
	// Doesn't appear to be used (Angus, 31 July 2001)
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
void InsetBibtex::fillWithBibKeys
	(Buffer const * buffer, vector<pair<string, string> > & keys) const
{
	lyx::Assert(buffer);
	vector<string> const files = getFiles(*buffer);
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


void InsetBibtex::edit(BufferView *, int, int, mouse_button::state)
{
	InsetCommandMailer mailer("bibtex", *this);
	mailer.showDialog();
}


void InsetBibtex::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, mouse_button::none);
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
