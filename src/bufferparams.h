// -*- C++ -*-
/**
 * \file bufferparams.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BUFFERPARAMS_H
#define BUFFERPARAMS_H

#include "lyxtextclass.h"
#include "paper.h"

#include "insets/insetquotes.h"

#include "support/copied_ptr.h"
#include "support/types.h"

#include "support/std_string.h"
#include <vector>


class AuthorList;
class BranchList;
class Bullet;
class LyXLex;
class LatexFeatures;
class Spacing;
class TexRow;
class VSpace;
struct Language;


/** Buffer parameters.
 *  This class contains all the parameters for this a buffer uses. Some
 *  work needs to be done on this class to make it nice. Now everything
 *  is in public.
 */
class BufferParams {
public:
	///
	enum PARSEP {
		///
		PARSEP_INDENT,
		///
		PARSEP_SKIP
	};
	///
	BufferParams();
	~BufferParams();

	/// read a header token, if unrecognised, return it or an unknown class name
	string const readToken(LyXLex & lex, string const & token);

	///
	void writeFile(std::ostream &) const;

	/** \returns true if the babel package is used (interogates
	 *  the BufferParams and a LyXRC variable).
	 *  This returned value can then be passed to the insets...
	 */
	bool writeLaTeX(std::ostream &, LaTeXFeatures &, TexRow &) const;

	///
	void setPaperStuff();

	///
	void useClassDefaults();

	///
	bool hasClassDefaults() const;

	///
	VSpace const & getDefSkip() const;

	///
	void setDefSkip(VSpace const & vs);

	/** Wether paragraphs are separated by using a indent like in
	 *  articles or by using a little skip like in letters.
	 */
	PARSEP paragraph_separation;
	///
	InsetQuotes::quote_language quotes_language;
	///
	InsetQuotes::quote_times quotes_times;
	///
	string fontsize;
	///
	lyx::textclass_type textclass;
	///
	LyXTextClass const & getLyXTextClass() const;

	/* this are for the PaperLayout */
	/// the general papersize (papersize2 or paperpackage
	PAPER_SIZE papersize;
	///  the selected Geometry papersize
	VMARGIN_PAPER_TYPE papersize2;
	/// a special paperpackage .sty-file
	PAPER_PACKAGES paperpackage;
	///
	PAPER_ORIENTATION orientation;
	///
	bool use_geometry;
	///
	string paperwidth;
	///
	string paperheight;
	///
	string leftmargin;
	///
	string topmargin;
	///
	string rightmargin;
	///
	string bottommargin;
	///
	string headheight;
	///
	string headsep;
	///
	string footskip;

	/* some LaTeX options */
	/// The graphics driver
	string graphicsDriver;
	///
	string fonts;
	///
	Spacing & spacing();
	Spacing const & spacing() const;
	///
	int secnumdepth;
	///
	int tocdepth;
	///
	Language const * language;
	/// BranchList:
	BranchList & branchlist();
	BranchList const & branchlist() const;
	///
	string inputenc;
	///
	string preamble;
	///
	string options;
	///
	string float_placement;
	///
	unsigned int columns;
	///
	LyXTextClass::PageSides sides;
	///
	string pagestyle;
	/// \param index should lie in the range 0 <= \c index <= 3.
	Bullet & temp_bullet(lyx::size_type index);
	Bullet const & temp_bullet(lyx::size_type index) const;
	/// \param index should lie in the range 0 <= \c index <= 3.
	Bullet & user_defined_bullet(lyx::size_type index);
	Bullet const & user_defined_bullet(lyx::size_type index) const;
	///
	void readPreamble(LyXLex &);
	///
	void readLanguage(LyXLex &);
	///
	void readGraphicsDriver(LyXLex &);

	/// use AMS package, not, or auto
	enum AMS {
		AMS_OFF,
		AMS_AUTO,
		AMS_ON
	};
	AMS use_amsmath;
	///
	bool use_natbib;
	///
	bool use_numerical_citations;
	/// revision tracking for this buffer ?
	bool tracking_changes;
	/// Time ago we agreed that this was a buffer property [ale990407]
	string parentname;
	///
	bool compressed;

	/// the author list for the document
	AuthorList & authors();
	AuthorList const & authors() const;

	/// map of the file's author IDs to buffer author IDs
	std::vector<int> author_map;
	///
	string const dvips_options() const;
	///
	string const paperSizeName() const;

private:
	/** Use the Pimpl idiom to hide those member variables that would otherwise
	 *  drag in other header files.
	 */
	class Impl;
	struct MemoryTraits {
		static Impl * clone(Impl const *);
		static void destroy(Impl *);
	};
	lyx::support::copied_ptr<Impl, MemoryTraits> pimpl_;
};

#endif
