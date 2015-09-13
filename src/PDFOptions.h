// -*- C++ -*-
/**
 * \file src/PDFOptions.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Pavel Sanda
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PDFOPTIONS_H
#define PDFOPTIONS_H

#include "OutputParams.h"

#include "support/strfwd.h"

#include <string>

namespace lyx {

class Lexer;

// FIXME UNICODE
// Write functions need to use odostream instead of ostream before
// we can use docstring instead of string.

/// Options for PDF generation
class PDFOptions {
public:
	///
	PDFOptions() { clear(); }
	/// check whether user added any settings for hyperref
	bool empty() const;
	/// output to lyx header
	void writeFile(std::ostream &) const;
	/// output to tex header
	void writeLaTeX(OutputParams &, otexstream &,
			bool hyperref_already_provided) const;
	/// read tokens from lyx header
	std::string readToken(Lexer &lex, std::string const & token);
	/// set implicit settings for hyperref
	void clear();

	///
	bool use_hyperref;
	///
	std::string title;
	///
	std::string author;
	///
	std::string subject;
	///
	std::string keywords;
	/**
		* A set of Acrobat bookmarks are written, in a manner similar to the
		* table of contents.
		* bookmarks         boolean true
		*/
	bool bookmarks;
	/**
		* If Acrobat bookmarks are requested, include section numbers.
		* bookmarksnumbered    boolean false
		*/
	bool bookmarksnumbered;
	/**
		* If Acrobat bookmarks are requested, show them with all the subtrees
		* expanded.
		* bookmarksopen     boolean false
		*/
	bool bookmarksopen;
	/**
		* Level (\maxdimen) to which bookmarks are open
		* bookmarksopenlevel    parameter
		*/
	int bookmarksopenlevel;
	/**
		* Allows link text to break across lines.
		* breaklinks        boolean false
		*/
	bool breaklinks;
	/**
		* The style of box around links; defaults to a box with lines of 1pt
		* thickness, but the colorlinks option resets it to produce no border.
		* pdfborder              "0 0 1" / "0 0 0"
		* Note that the color of link borders can be specified only as 3
		* numbers in the range 0..1, giving an RGB color.
		* You cannot use colors defined in TEX.
		*
		* We represent here only the last bit, there is no semantics in the
		* first two. Morover the aim is not to represent the whole pdfborder,
		* but just to deny the ugly boxes around pdf links.
		*/
	bool pdfborder;
	/**
		* colorlinks        boolean false
		*/
	bool colorlinks;
	/**
		* Adds backlink text to the end of each item in the bibliography,
		* as a list of section/slide/page numbers.
		* This can only work properly if there is a blank line after each
		* \bibitem.
		* backref   string  empty(="section"), "false", "section", "slide", "page"
		*
		* Internally we use false/section/slide/pages. See also bug 5340.
		*/
	std::string backref;
	/**
		* Determines how the file is opening in Acrobat;
		* the possibilities are None, UseThumbs (show thumbnails), UseOutlines
		* (show bookmarks), and FullScreen.
		* If no mode if explicitly chosen, but the bookmarks option is set,
		* UseOutlines is used.
		* pagemode          text    empty
		*
		* We currently implement only FullScreen, but all modes can be saved
		* here, lyx format & latex writer is prepared.
		* The only thing needed in such a case is wider Settings
		* dialog -> PDFOptions.pagemode .
		*/
	std::string pagemode;
	/**
		* Flag indicating whether hyperref tries to derive the values for
		* pdftitle and pdfauthor from \title and \author.
		* pdfusetitle       boolean false
		*
		* Note that we use true as default value instead. The option is also
		* used in latex output only when title and author is not filled.
	*/
	bool pdfusetitle;
	///latex string
	static const std::string pagemode_fullscreen;
	/**
		* Additional parameters for hyperref given from user.
		*/
	std::string quoted_options;
	/**
		* Possible syntax check of users additional parameters here.
		* Returns repaired string. For the time being only newlines
		* are checked.
		*/
	std::string quoted_options_check(std::string const & str) const;
};

} // namespace lyx

#endif // PDFOPTIONS_H
