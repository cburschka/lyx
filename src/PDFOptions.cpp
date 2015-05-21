/**
 * \file PDFoptions.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Pavel Sanda
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "PDFOptions.h"

#include "Encoding.h"
#include "Lexer.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/lstrings.h"

#include <sstream>
#include <string>
#include <algorithm>

using namespace std;
using namespace lyx::support;

namespace lyx {


const string PDFOptions::pagemode_fullscreen("FullScreen");

bool PDFOptions::empty() const
{
	PDFOptions x; //implicit hyperref settings

	return  author == x.author
		&& title == x.title
		&& subject == x.subject
		&& keywords == x.keywords
		&& pagemode == x.pagemode
		&& quoted_options == x.quoted_options
		&& bookmarks == x.bookmarks
		&& bookmarksnumbered == x.bookmarksnumbered
		&& bookmarksopen == x.bookmarksopen
		&& bookmarksopenlevel == x.bookmarksopenlevel
		&& breaklinks == x.breaklinks
		&& pdfborder == x.pdfborder
		&& colorlinks == x.colorlinks
		&& backref == x.backref
		&& pdfusetitle == x.pdfusetitle;
}


void PDFOptions::writeFile(ostream & os) const
{
	os << "\\use_hyperref " << convert<string>(use_hyperref) << '\n';
	if (!use_hyperref && empty())
		return;
	
	if (!title.empty() )
		os << "\\pdf_title \"" << title << "\"\n";
	if (!author.empty())
		os << "\\pdf_author \"" << author << "\"\n";
	if (!subject.empty())
		os << "\\pdf_subject \"" << subject << "\"\n";
	if (!keywords.empty())
		os << "\\pdf_keywords \"" << keywords << "\"\n";
	
	
	os << "\\pdf_bookmarks " << convert<string>(bookmarks) << '\n';
	os << "\\pdf_bookmarksnumbered " << convert<string>(bookmarksnumbered) << '\n';
	os << "\\pdf_bookmarksopen " << convert<string>(bookmarksopen) << '\n';
	os << "\\pdf_bookmarksopenlevel " << bookmarksopenlevel << '\n';
	
	os << "\\pdf_breaklinks "  << convert<string>(breaklinks)  << '\n';
	os << "\\pdf_pdfborder "   << convert<string>(pdfborder)   << '\n';
	os << "\\pdf_colorlinks "  << convert<string>(colorlinks)  << '\n';
	os << "\\pdf_backref "     << backref << '\n';
	os << "\\pdf_pdfusetitle " << convert<string>(pdfusetitle) << '\n';
	
	if (!pagemode.empty())
		os << "\\pdf_pagemode " << pagemode << '\n';
	
	if (!quoted_options.empty())
		os << "\\pdf_quoted_options \"" << quoted_options << "\"\n";
}


void PDFOptions::writeLaTeX(OutputParams & runparams, otexstream & os,
			    bool hyperref_already_provided) const
{
	// FIXME Unicode
	string opt;
	string hyperset;
	
	// since LyX uses unicode, also set the PDF strings to unicode strings with the
	// hyperref option "unicode"
	opt += "unicode=true,";

	// only use the hyperref settings if hyperref is enabled by the user
	// see bug #7052
	if (use_hyperref) {
		// try to extract author and title from document when none is
		// explicitly given
		if (pdfusetitle && title.empty() && author.empty())
			opt += "pdfusetitle,";
		opt += "\n ";
		opt += "bookmarks=" + convert<string>(bookmarks) + ',';
		if (bookmarks) {
			opt += "bookmarksnumbered=" + convert<string>(bookmarksnumbered) + ',';
			opt += "bookmarksopen=" + convert<string>(bookmarksopen) + ',';
			if (bookmarksopen)
				opt += "bookmarksopenlevel="
				+ convert<string>(bookmarksopenlevel) + ',';
		}
		opt += "\n ";
		opt += "breaklinks=" + convert<string>(breaklinks) + ',';
		opt += "pdfborder={0 0 ";
		opt += (pdfborder ? '0' : '1');
		opt += "},";
		if (pdfborder)
			opt += "pdfborderstyle={},";
		opt += "backref=" + backref + ',';
		opt += "colorlinks=" + convert<string>(colorlinks) + ',';
		if (!pagemode.empty())
			opt += "pdfpagemode=" + pagemode + ',';

		// load the pdftitle etc. as hypersetup, otherwise you'll get
		// LaTeX-errors when using non-latin characters
		if (!title.empty())
			hyperset += "pdftitle={" + title + "},";
		if (!author.empty())			
			hyperset += "\n pdfauthor={" + author + "},";
		if (!subject.empty())
			hyperset += "\n pdfsubject={" + subject + "},";
		if (!keywords.empty())
			hyperset += "\n pdfkeywords={" + keywords + "},";
		if (!quoted_options.empty()){
			hyperset += "\n ";
			hyperset += quoted_options;
		}
		hyperset = rtrim(hyperset,",");
	}

	// check if the hyperref settings use an encoding that exceeds
	// ours. If so, we have to switch to utf8.
	Encoding const * const enc = runparams.encoding;
	docstring const hs = from_utf8(hyperset);
	bool need_unicode = false;
	if (enc) {
		for (size_t n = 0; n < hs.size(); ++n) {
			if (!enc->encodable(hs[n]))
				need_unicode = true;
		}
	}

	// use in \\usepackage parameter as not all options can be handled inside \\hypersetup
	if (!hyperref_already_provided) {
		opt = rtrim(opt, ",");
		opt = "\\usepackage[" + opt + "]\n {hyperref}\n";

		if (!hyperset.empty())
			opt += "\\hypersetup{" + hyperset + "}\n";
	} else {
		// only in case hyperref is already loaded by the current text class
		// try to put it into hyperset
		//
		// FIXME: rename in this case the PDF settings dialog checkbox
		//  label from "Use Hyperref" to "Customize Hyperref Settings"
		//  as discussd in bug #6293
		opt = "\\hypersetup{" + rtrim(opt + hyperset, ",") + "}\n";
	}

	// hyperref expects utf8!
	if (need_unicode && enc && enc->iconvName() != "UTF-8"
	    &&!runparams.isFullUnicode()) {
		os << "\\inputencoding{utf8}\n"
		   << setEncoding("UTF-8");
	}
	// If hyperref is loaded by the document class, we output
	// \hypersetup \AtBeginDocument if hypersetup is not (yet)
	// defined. In this case, the class loads hyperref late
	// (see bug #7048).
	if (hyperref_already_provided && !opt.empty()) {
		os << "\\ifx\\hypersetup\\undefined\n"
		   << "  \\AtBeginDocument{%\n    "
		   << from_utf8(opt)
		   << "  }\n"
		   << "\\else\n  "
		   << from_utf8(opt)
		   << "\\fi\n";
	} else
		os << from_utf8(opt);

	if (need_unicode && enc && enc->iconvName() != "UTF-8"
	    &&!runparams.isFullUnicode()) {
		os << setEncoding(enc->iconvName())
		   << "\\inputencoding{" << from_ascii(enc->latexName()) << "}\n";
	}
}


string PDFOptions::readToken(Lexer &lex, string const & token)
{
	if (token == "\\use_hyperref") {
		lex >> use_hyperref;
	} else if (token == "\\pdf_title") {
		lex >> title;
	} else if (token == "\\pdf_author") {
		lex >> author;
	} else if (token == "\\pdf_subject") {
		lex >> subject;
	} else if (token == "\\pdf_keywords") {
		lex >> keywords;
	} else if (token == "\\pdf_bookmarks") {
		lex >> bookmarks;
	} else if (token == "\\pdf_bookmarksnumbered") {
		lex >> bookmarksnumbered;
	} else if (token == "\\pdf_bookmarksopen") {
		lex >> bookmarksopen;
	} else if (token == "\\pdf_bookmarksopenlevel") {
		lex >> bookmarksopenlevel;
	} else if (token == "\\pdf_breaklinks") {
		lex >> breaklinks;
	} else if (token == "\\pdf_pdfborder") {
		lex >> pdfborder;
	} else if (token == "\\pdf_colorlinks") {
		lex >> colorlinks;
	} else if (token == "\\pdf_backref") {
		lex >> backref;
	} else if (token == "\\pdf_pdfusetitle") {
		lex >> pdfusetitle;
	} else if (token == "\\pdf_pagemode") {
		lex >> pagemode;
	} else if (token == "\\pdf_quoted_options") {
		lex >> quoted_options;
	} else {
		return token;
	}
	return string();
}


// check the string from UI
string PDFOptions::quoted_options_check(string const & str) const
{
	return subst(str, "\n", "");
}


// set implicit settings for hyperref
void PDFOptions::clear()
{
	use_hyperref            = false;
	title.clear();
	author.clear();
	subject.clear();
	keywords.clear();
	bookmarks               = true;
	bookmarksnumbered       = false;
	bookmarksopen           = false;
	bookmarksopenlevel	= 1;
	breaklinks              = false;
	pdfborder               = false;
	colorlinks              = false;
	backref                 = "false";
	pagemode.clear();
	quoted_options.clear();
	pdfusetitle             = true;  //in contrast with hyperref
}

} // namespace lyx
