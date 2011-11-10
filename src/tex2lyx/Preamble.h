/**
 * \file Preamble.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

// {[(

#ifndef LYX_PREAMBLE_H
#define LYX_PREAMBLE_H

#include "Author.h"

#include <iosfwd>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>


namespace lyx {

class Parser;
class TeX2LyXDocClass;

class Preamble
{
public:
	Preamble();

	///
	std::string inputencoding() const { return h_inputencoding; }
	///
	std::string notefontcolor() const { return h_notefontcolor; }
	///
	bool indentParagraphs() const;
	///
	bool isPackageUsed(std::string const & package) const;
	///
	std::vector<std::string>
	getPackageOptions(std::string const & package) const;
	/// Tell that \p package will be loaded automatically by LyX.
	/// This has only an effect if \p package is prepared for
	/// autoloading in parse().
	void registerAutomaticallyLoadedPackage(std::string const & package);
	///
	void addModule(std::string const & module);
	///
	void suppressDate(bool suppress);
	/// Register an author named \p name in the author list
	void registerAuthor(std::string const & name);
	/// Get author named \p name (must be registered first)
	Author const & getAuthor(std::string const & name) const;


	/// Parses the LaTeX preamble into internal data
	void parse(Parser & p, std::string const & forceclass,
	           TeX2LyXDocClass & tc);
	/// Writes the LyX file header from internal data
	bool writeLyXHeader(std::ostream & os);

private:
	///
	std::map<std::string, std::vector<std::string> > used_packages;
	/// Packages that will be loaded automatically by LyX
	std::set<std::string> auto_packages;
	///
	std::vector<std::string> used_modules;

	/// needed to handle encodings with babel
	bool one_language;

	std::ostringstream h_preamble;
	std::string h_backgroundcolor;
	std::string h_boxbgcolor;
	std::string h_cite_engine;
	std::string h_defskip;
	std::string h_float_placement;
	std::string h_fontcolor;
	std::string h_fontencoding;
	std::string h_font_roman;
	std::string h_font_sans;
	std::string h_font_typewriter;
	std::string h_font_default_family;
	std::string h_font_sc;
	std::string h_font_osf;
	std::string h_font_sf_scale;
	std::string h_font_tt_scale;
	std::string h_graphics;
	std::string h_html_be_strict;
	std::string h_html_css_as_file;
	std::string h_html_math_output;
	std::string h_inputencoding;
	std::string h_language;
	std::string h_language_package;
	std::string h_listings_params;
	std::string h_margins;
	std::string h_notefontcolor;
	std::string h_options;
	std::string h_output_changes;
	std::string h_papercolumns;
	std::string h_paperfontsize;
	std::string h_paperorientation;
	std::string h_paperpagestyle;
	std::string h_papersides;
	std::string h_papersize;
	std::string h_paragraph_indentation;
	/// necessary to set the separation when \setlength is parsed
	std::string h_paragraph_separation;
	std::string h_pdf_title;
	std::string h_pdf_author;
	std::string h_pdf_subject;
	std::string h_pdf_keywords;
	std::string h_pdf_bookmarks;
	std::string h_pdf_bookmarksnumbered;
	std::string h_pdf_bookmarksopen;
	std::string h_pdf_bookmarksopenlevel;
	std::string h_pdf_breaklinks;
	std::string h_pdf_pdfborder;
	std::string h_pdf_colorlinks;
	std::string h_pdf_backref;
	std::string h_pdf_pdfusetitle;
	std::string h_pdf_pagemode;
	std::string h_pdf_quoted_options;
	std::string h_quotes_language;
	std::string h_secnumdepth;
	std::string h_spacing;
	std::string h_suppress_date;
	std::string h_textclass;
	std::string h_tocdepth;
	std::string h_tracking_changes;
	std::string h_use_bibtopic;
	std::string h_use_geometry;
	std::string h_use_amsmath;
	std::string h_use_default_options;
	std::string h_use_esint;
	std::string h_use_hyperref;
	std::string h_use_mhchem;
	std::string h_use_mathdots;
	std::string h_use_refstyle;
	std::string h_use_undertilde;

	/*!
	 * Add package \p name with options \p options to used_packages.
	 * Remove options from \p options that we don't want to output.
	 */
	void add_package(std::string const & name,
	                 std::vector<std::string> & options);
	///
	void handle_hyperref(std::vector<std::string> & options);
	///
	void handle_package(Parser &p, std::string const & name,
	                    std::string const & opts, bool in_lyx_preamble);
	///
	void handle_if(Parser & p, bool in_lyx_preamble);

	AuthorList authors_;
};


extern Preamble preamble;

// }])


} // namespace lyx

#endif
