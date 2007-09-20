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


#include <string>
#include <debug.h>
#include "support/docstream.h"

using std::string;

namespace lyx {

class Lexer;

/// Options for PDF generation
class PDFOptions {
public:
       PDFOptions() { clear(); }
       ///
       bool use_hyperref;
       ///
       string title;
       ///
       string author;
       ///
       string subject;
       ///
       string keywords;
       /*!
        * A set of Acrobat bookmarks are written, in a manner similar to the
		* table of contents.
        * bookmarks            boolean true
        */
       bool bookmarks;
       /*!
        * If Acrobat bookmarks are requested, include section numbers.
        * bookmarksnumbered    boolean false
        */
       bool bookmarksnumbered;
       /*!
        * If Acrobat bookmarks are requested, show them with all the subtrees
		* expanded.
		* bookmarksopen        boolean false
        */
       bool bookmarksopen;
       /*!
        * Level (\maxdimen) to which bookmarks are open
        * bookmarksopenlevel    parameter
        */
       string bookmarksopenlevel;
       /*!
        * Allows link text to break across lines.
        * breaklinks           boolean false
        */
       bool breaklinks;
       /*!
        * The style of box around links; defaults to a box with lines of 1pt
		* thickness, but the colorlinks option resets it to produce no border.
        * pdfborder                    "0 0 1" / "0 0 0"
        * Note that the color of link borders can be specified only as 3
		* numbers in the range 0..1, giving an RGB color.
		* You cannot use colors defined in TEX.
        *
        * We represent here only the last bit, there is no semantics in the
		* first two. Morover the aim is not to represent the whole pdfborder,
		* but just to deny the ugly boxes around pdf links.
        */
       bool pdfborder;
       /*!
        * colorlinks           boolean false
        */
       bool colorlinks;
       /*!
        * Adds backlink text to the end of each item in the bibliography,
		* as a list of section numbers.
        * This can only work properly if there is a blank line after each
		* \bibitem.
        * backref              boolean false
        */
       bool backref;
       /*!
        * Adds backlink text to the end of each item in the bibliography,
		* as a list of page numbers.
        * pagebackref          boolean false
        */
       bool pagebackref;
       /*!
        * Determines how the file is opening in Acrobat;
        * the possibilities are None, UseThumbs (show thumbnails), UseOutlines
		* (show bookmarks), and FullScreen.
        * If no mode if explicitly chosen, but the bookmarks option is set,
		* UseOutlines is used.
        * pagemode             text    empty
        *
        * We currently implement only FullScreen, but all modes can be saved
		* here, lyx format & latex writer is prepared.
        * The only thing needed in such a case is wider Settings
		* dialog -> PDFOptions.pagemode .
        */
       string pagemode;
       ///latex string
       static const string pagemode_fullscreen;
       /*!
        * Additional parameters for hyperref given from user.
        */
       string quoted_options;
       /*!
        * Possible syntax check of users additional parameters here.
        */
       string quoted_options_get() const;


       /*!
        * Flag indicating whether user made some input into PDF preferences.
        * We want to save options, when user decide to switch off PDF support
		* for a while.
        */
       bool store_options;
       /// check for string settings
       bool empty() const;
       /// output to lyx header
       void writeFile(std::ostream &) const;
       /// output to tex header
       void writeLaTeX(odocstream &) const;
       /// read tokens from lyx header
       string readToken(Lexer &lex, string const & token);
       /// keep implicit hyperref settings
       void clear();

};

} // namespace lyx
