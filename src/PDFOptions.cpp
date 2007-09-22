/**
 * \file PDFoptions.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Pavel Sanda
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "PDFOptions.h"

#include "support/convert.h"
#include "support/lstrings.h"
#include "debug.h"
#include "Lexer.h"

#include <sstream>
#include <string>

namespace lyx {


using std::ostream;
using std::ostringstream;
using std::string;

const string PDFOptions::pagemode_fullscreen("FullScreen");

bool PDFOptions::empty() const
{
	return author.empty()
		&& title.empty()
		&& subject.empty()
		&& keywords.empty()
		&& pagemode.empty()
		&& bookmarksopenlevel.empty()
		&& quoted_options.empty();
}

void PDFOptions::writeFile(ostream & os) const
{
	os << "\\use_hyperref " << convert<string>(use_hyperref) << '\n';
	os << "\\pdf_store_options " << convert<string>(store_options) << '\n';
	if (!use_hyperref && !store_options)
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
	if (!bookmarksopenlevel.empty())
		os << "\\pdf_bookmarksopenlevel \"" << bookmarksopenlevel << "\"\n";
	
	os << "\\pdf_breaklinks "  << convert<string>(breaklinks)  << '\n';
	os << "\\pdf_pdfborder "   << convert<string>(pdfborder)   << '\n';
	os << "\\pdf_colorlinks "  << convert<string>(colorlinks)  << '\n';
	os << "\\pdf_backref "     << convert<string>(backref)     << '\n';
	os << "\\pdf_pagebackref " << convert<string>(pagebackref) << '\n';
	
	if (!pagemode.empty())
		os << "\\pdf_pagemode " << pagemode << '\n';
	
	if (!quoted_options.empty())
		os << "\\pdf_quoted_options \"" << quoted_options << "\"\n";
}

void PDFOptions::writeLaTeX(odocstringstream &os) const
{
	if (!use_hyperref)
		return;
	
	string opt;
	
	opt = "\\usepackage[";
	if (!title.empty())
		opt += "pdftitle={"   + title + "},\n";
	if (!author.empty())
		opt += "pdfauthor={"  + author + "},\n";
	if (!subject.empty())
		opt += "pdfsubject={" + subject + "},\n";
	if (!keywords.empty())
		opt += "pdfkeywords={" + keywords + "},\n";
	
	opt += "bookmarks=" + convert<string>(bookmarks) + ',';
	if (bookmarks) {
		opt += "bookmarksnumbered=" + convert<string>(bookmarksnumbered) + ',';
		opt += "bookmarksopen=" + convert<string>(bookmarksopen) + ',';
	
		if (bookmarksopen && !bookmarksopenlevel.empty())
			opt += "bookmarksopenlevel=" + bookmarksopenlevel + ',';
	}
	
	opt += "breaklinks="     + convert<string>(breaklinks) + ',';
	
	opt += "pdfborder={0 0 " ;
	opt += (pdfborder ?'0':'1');
	opt += "},\n";
	
	opt += "colorlinks="     + convert<string>(colorlinks) + ',';
	opt += "backref="        + convert<string>(backref) + ',';
	opt += "pagebackref="    + convert<string>(pagebackref) + ',';
	
	if (!pagemode.empty())
		opt += "pdfpagemode=" + pagemode + ',';
	opt += quoted_options_get();
	
	opt = support::rtrim(opt,",");
	opt += "]{hyperref}\n";
	
	// FIXME UNICODE
	os << from_utf8(opt);
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
	} else if (token == "\\pdf_pagebackref") {
		lex >> pagebackref;
	} else if (token == "\\pdf_pagemode") {
		lex >> pagemode;
	} else if (token == "\\pdf_quoted_options") {
		lex >> quoted_options;
	} else if (token == "\\pdf_store_options") {
		lex >> store_options;
	} else {
		return token;
	}
	return string();
}


//prepared for check
string PDFOptions::quoted_options_get() const
{
	return quoted_options;
}


// Keep implicit hyperref settings
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
	bookmarksopenlevel.clear();
	breaklinks              = false;
	pdfborder               = false;
	colorlinks              = false;
	backref                 = false;
	pagebackref             = false;
	pagemode.clear();
	quoted_options.clear();
	store_options           = false;
}

} // namespace lyx
