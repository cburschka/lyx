/**
 * \file insetbibtex.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetbibtex.h"
#include "metricsinfo.h"
#include "buffer.h"
#include "BufferView.h"
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"
#include "latexrunparams.h"
#include "lyxlex.h"
#include "Lsstream.h"
#include "metricsinfo.h"

#include "support/filetools.h"
#include "support/path.h"
#include "support/os.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"
#include "support/LAssert.h"
#include "support/tostr.h"

#include <fstream>

using namespace lyx;
using support::FileName;

using std::ostream;
using std::ifstream;
using std::endl;
using std::vector;
using std::pair;


InsetBibtexParams::InsetBibtexParams()
	: bibtotoc(false)
{}


bool InsetBibtexParams::empty() const
{
	return databases.empty();
}


void InsetBibtexParams::erase()
{
	databases.clear();
	style.erase();
	bibtotoc = false;
}


void InsetBibtexParams::write(Buffer const & buffer, std::ostream & os) const
{
	os << "Bibtex\n";
	
	vector<FileName>::const_iterator it  = databases.begin();
	vector<FileName>::const_iterator end = databases.end();
	for (; it != end; ++it) {
		os << "\tfilename "
		   << it->outputFilename(buffer.filePath())
		   << '\n';
	}
	if (!style.empty())
		os << "\tstyle " << style << '\n';
	if (bibtotoc)
		os << "\tbibtotoc " << tostr(bibtotoc) << '\n';
}


void InsetBibtexParams::read(Buffer const & buffer, LyXLex & lex)
{
	enum BibtexTags {
		BIB_FILENAME = 1,
		BIB_STYLE,
		BIB_BIBTOTOC,
		BIB_END
	};

	keyword_item bibtex_tags[] = {
		{ "\\end_inset", BIB_END },
		{ "bibtotoc",    BIB_BIBTOTOC },
		{ "filename",    BIB_FILENAME},
		{ "style",       BIB_STYLE}
	};

	pushpophelper pph(lex, bibtex_tags, BIB_END);

	bool found_end  = false;
	bool read_error = false;

	while (lex.isOK()) {
		switch (lex.lex()) {
		case BIB_FILENAME: {
			lex.next();
			FileName filename;
			filename.set(lex.getString(), buffer.filePath());
			databases.push_back(filename);
			break;
		}

		case BIB_STYLE: {
			lex.next();
			style = lex.getString();
			break;
		}

		case BIB_BIBTOTOC: {
			lex.next();
			bibtotoc = lex.getBool();
			break;
		}

		case BIB_END:
			found_end = true;
			break;

		default:
			lex.printError("BibtexInset::read: "
				       "Wrong tag: $$Token");
			read_error = true;
			break;
		}

		if (found_end || read_error)
			break;
	}

	if (!found_end) {
		lex.printError("BibtexInset::read: "
			       "Missing \\end_inset.");
	}
}


InsetBibtex::InsetBibtex()
	: set_label_(false), center_indent_(0)
{}



InsetBibtex::InsetBibtex(InsetBibtexParams const & p)
	: params_(p), set_label_(false), center_indent_(0)
{}


InsetBibtex::~InsetBibtex()
{
	InsetBibtexMailer(*this).hideDialog();
}


std::auto_ptr<InsetBase> InsetBibtex::clone() const
{
	return std::auto_ptr<InsetBase>(new InsetBibtex(*this));
}


dispatch_result InsetBibtex::localDispatch(FuncRequest const & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_EDIT:
		InsetBibtexMailer(*this).showDialog(cmd.view());
		return DISPATCHED;

	case LFUN_INSET_MODIFY: {
		Buffer const * buffer = cmd.view()->buffer();
		InsetBibtexParams p;
		InsetBibtexMailer::string2params(cmd.argument, *buffer, p);
		setParams(p);
		return  DISPATCHED;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		InsetBibtexMailer(*this).updateDialog(cmd.view());
		return DISPATCHED;

	case LFUN_MOUSE_RELEASE:
		return localDispatch(FuncRequest(cmd.view(), LFUN_INSET_EDIT));

	default:
		return InsetOld::localDispatch(cmd);
	}

}

string const InsetBibtex::getScreenLabel(Buffer const &) const
{
	return _("BibTeX Generated References");
}


void InsetBibtex::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (!set_label_) {
		set_label_ = true;
		button_.update(getScreenLabel(*mi.base.bv->buffer()),
			       editable() != NOT_EDITABLE);
	}
	button_.metrics(mi, dim);
	center_indent_ = (mi.base.textwidth - dim.wid) / 2;
	dim.wid = mi.base.textwidth;
	dim_ = dim;
}


void InsetBibtex::draw(PainterInfo & pi, int x, int y) const
{
	button_.draw(pi, x + center_indent_, y);
}


void InsetBibtex::write(Buffer const & buffer, std::ostream & os) const
{
	params().write(buffer, os);
}


void InsetBibtex::read(Buffer const & buffer, LyXLex & lex)
{
	InsetBibtexParams p;
	p.read(buffer, lex);

	// Replace the inset's store
	setParams(p);
}


int InsetBibtex::latex(Buffer const & buffer, ostream & os,
		       LatexRunParams const & runparams) const
{
	ostringstream ss;
	// 1. \bibliographystyle{style}
	if (!params().style.empty()) { // we want no \biblio...{}
		string style = params().style;

		string const abs_style = 
			support::MakeAbsPath(style, buffer.filePath());
		if (!runparams.nice && support::IsFileReadable(abs_style + ".bst"))
			style = abs_style;

		ss << "\\bibliographystyle{" << style << "}\n";
	}

	// 2. \addcontentsline{...} - if option bibtotoc set
	if (params().bibtotoc) {
		// Assumption: if the textclass name does not contain "art",
		// then it's a book.
		BufferParams const & bp = buffer.params;
		if (!support::contains(bp.getLyXTextClass().name(), "art")) {
			if (bp.sides == LyXTextClass::OneSide) {
				ss << "\\clearpage";
			} else {
				ss << "\\cleardoublepage";
			}

			// book class
			ss << "\\addcontentsline{toc}{chapter}{\\bibname}\n";
		} else {
			// article class
			ss << "\\addcontentsline{toc}{section}{\\refname}\n";
		}
	}

	// 3. \bibliography{database}
	// If we generate in a temp dir, we _need_ to use the absolute path,
	// else rely on the user.
	ss << "\\bibliography{";
	vector<FileName>::const_iterator begin = params().databases.begin();
	vector<FileName>::const_iterator end   = params().databases.end();
	vector<FileName>::const_iterator it    = begin;
	for (; it != end; ++it) {
		if (it != begin)
			ss << ',';
		string db = it->outputFilename(buffer.filePath());
		if (!runparams.nice &&
		    support::IsFileReadable(it->absFilename())+".bib")
			db = support::os::external_path(it->absFilename());

		ss << db;
	}
	ss << '}';

	string const output = STRCONV(ss.str());
	os << output;
	return int(lyx::count(output.begin(), output.end(),'\n') + 1);
}


int InsetBibtex::ascii(Buffer const &, std::ostream &, int) const
{
	return 0;
}


int InsetBibtex::linuxdoc(Buffer const &, std::ostream &) const
{
	return 0;
}


int InsetBibtex::docbook(Buffer const &, std::ostream &, bool) const
{
	return 0;
}


vector<string> const InsetBibtex::getFiles(Buffer const & buffer) const
{
	support::Path p(buffer.filePath());

	vector<string> files;
	vector<FileName>::const_iterator it  = params().databases.begin();
	vector<FileName>::const_iterator end = params().databases.end();
	for (; it != end; ++it) {
		// I really do need to pass the buffer path here...
		// FileName needs extending it would seem.
		string file_in = it->relFilename(buffer.filePath());
		string file_out = support::findtexfile(
			support::ChangeExtension(file_in, "bib"), "bib");
		lyxerr[Debug::LATEX] << "Bibfile: " << file_in
				     << ' ' << file_out << endl;

		// If we don't find a matching file name just fail silently
		if (!file_out.empty())
			files.push_back(file_out);
	}

	return files;
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
			string linebuf = support::trim(linebuf0);
			if (linebuf.empty()) continue;
			if (support::prefixIs(linebuf, "@")) {
				linebuf = support::subst(linebuf, '{', '(');
				string tmp;
				linebuf = support::split(linebuf, tmp, '(');
				tmp = support::ascii_lowercase(tmp);
				if (!support::prefixIs(tmp, "@string")
				    && !support::prefixIs(tmp, "@preamble")) {
					linebuf = support::split(linebuf, tmp, ',');
					tmp = support::ltrim(tmp, " \t");
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


bool InsetBibtex::addDatabase(string const & /* db */)
{
#ifdef WITH_WARNINGS
#warning addDatabase is currently disabled (no LFUN).
#endif
#if 0
	vector<string>
	string contents(getContents());
	if (!support::contains(contents, db)) {
		if (!contents.empty())
			contents += ',';
		setContents(contents + db);
		return true;
	}
#endif
	return false;
}


bool InsetBibtex::delDatabase(string const & /* db */)
{
#ifdef WITH_WARNINGS
#warning delDatabase is currently disabled (no LFUN).
#endif
#if 0
	if (support::contains(getContents(), db)) {
		string bd = db;
		int const n = tokenPos(getContents(), ',', bd);
		if (n > 0) {
			// Weird code, would someone care to explain this?(Lgb)
			string tmp(", ");
			tmp += bd;
			setContents(support::subst(getContents(), tmp, ", "));
		} else if (n == 0)
			setContents(support::split(getContents(), bd, ','));
		else
			return false;
	}
#endif
	return true;
}


void InsetBibtex::setParams(InsetBibtexParams const & params)
{
	params_ = params;
}


string const InsetBibtexMailer::name_ = "bibtex";


InsetBibtexMailer::InsetBibtexMailer(InsetBibtex & inset)
	: inset_(inset)
{}


string const InsetBibtexMailer::inset2string(Buffer const & buffer) const
{
	return params2string(inset_.params(), buffer);
}


void InsetBibtexMailer::string2params(string const & in,
				      Buffer const & buffer,
				      InsetBibtexParams & params)
{
	params = InsetBibtexParams();

	if (in.empty())
		return;

	istringstream data(STRCONV(in));
	LyXLex lex(0,0);
	lex.setStream(data);

	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token != name_)
			return;
	}

	// This is part of the inset proper that is usually swallowed
	// by Buffer::readInset
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token != "Bibtex")
			return;
	}

	if (lex.isOK()) {
		params.read(buffer, lex);
	}
}


string const InsetBibtexMailer::params2string(InsetBibtexParams const & params,
					      Buffer const & buffer)
{
	ostringstream data;
	data << name_ << ' ';
	params.write(buffer, data);
	data << "\\end_inset\n";
	return STRCONV(data.str());
}
