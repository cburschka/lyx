// -*- C++ -*-
/**
 * \file TextClass.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXTEXTCLASS_H
#define LYXTEXTCLASS_H

#include "Color.h"
#include "Layout.h"
#include "lyxlayout_ptr_fwd.h"

#include <boost/shared_ptr.hpp>

#include <vector>
#include <set>
#include <map>

namespace lyx {

namespace support { class FileName; }

class Lexer;
class Counters;
class FloatList;


///
class InsetLayout {
public:
	std::string name;
	std::string lyxtype;
	docstring labelstring;
	std::string decoration;
	std::string latextype;
	std::string latexname;
	std::string latexparam;
	Font font;
	Font labelfont;
	Color::color bgcolor;
	std::string preamble;
};


/// List of semantically defined character style insets
typedef std::vector<InsetLayout> CharStyles;

/// List of inset layouts
typedef std::map<docstring, InsetLayout> InsetLayouts;

/// Stores the layout specification of a LyX document class.
class TextClass {
public:
	/// The individual styles comprising the document class
	typedef std::vector<LayoutPtr> LayoutList;
	/// Enumerate the paragraph styles.
	typedef LayoutList::const_iterator const_iterator;
	/// Construct a layout with default values. Actual values loaded later.
	explicit
	TextClass(std::string const & = std::string(),
		     std::string const & = std::string(),
		     std::string const & = std::string(),
		     bool = false);

	/// check whether the TeX class is available
	bool isTeXClassAvailable() const;

	/// paragraph styles begin iterator.
	const_iterator begin() const { return layoutlist_.begin(); }
	/// paragraph styles end iterator
	const_iterator end() const { return layoutlist_.end(); }

	/// Performs the read of the layout file.
	bool read(support::FileName const & filename, bool merge = false);
	///
	void readOutputType(Lexer &);
	///
	void readTitleType(Lexer &);
	///
	void readMaxCounter(Lexer &);
	///
	void readClassOptions(Lexer &);
	///
	void readCharStyle(Lexer &, std::string const &);
	///
	void readInsetLayout(Lexer &, docstring const &);
	///
	void readFloat(Lexer &);
	///
	void readCounter(Lexer &);
	///
	bool hasLayout(docstring const & name) const;

	///
	LayoutPtr const & operator[](docstring const & vname) const;

	/// Sees to that the textclass structure has been loaded
	bool load(std::string const & path = std::string()) const;
	/// Has this layout file been loaded yet?
	bool loaded() const { return loaded_; }

	/// the list of floats defined in the document class
	FloatList & floats();
	/// the list of floats defined in the document class
	FloatList const & floats() const;
	/// The Counters present in this document class.
	Counters & counters() const;
	/// CharStyles of this doc class
	CharStyles & charstyles() const { return charstylelist_; };
	/// Inset layouts of this doc class
	InsetLayouts & insetlayouts() const { return insetlayoutlist_; };
	///
	InsetLayout const & insetlayout(docstring const & name) const;
	///
	docstring const & defaultLayoutName() const;
	///
	LayoutPtr const & defaultLayout() const;
	///
	std::string const & name() const;
	///
	docstring const & labelstring() const;
	///
	std::string const & latexname() const;
	///
	std::string const & description() const;
	///
	std::string const & opt_fontsize() const;
	///
	std::string const & opt_pagestyle() const;
	///
	std::string const & options() const;
	///
	std::string const & class_header() const;
	///
	std::string const & pagestyle() const;
	///
	docstring const & preamble() const;

	/// is this feature already provided by the class?
	bool provides(std::string const & p) const;

	///
	unsigned int columns() const;
	///
	enum PageSides {
		///
		OneSide,
		///
		TwoSides
	};
	///
	PageSides sides() const;
	///
	int secnumdepth() const;
	///
	int tocdepth() const;

	/// Can be LaTeX, DocBook, etc.
	OutputType outputType() const;

	///
	Font const & defaultfont() const;

	/// Text that dictates how wide the left margin is on the screen
	docstring const & leftmargin() const;

	/// Text that dictates how wide the right margin is on the screen
	docstring const & rightmargin() const;

	/// The type of command used to produce a title
	LYX_TITLE_LATEX_TYPES titletype() const;
	/// The name of the title command
	std::string const & titlename() const;

	///
	int size() const;
	/// The minimal TocLevel of sectioning layouts
	int min_toclevel() const;
	/// The maximal TocLevel of sectioning layouts
	int max_toclevel() const;
	/// returns true if the class has a ToC structure
	bool hasTocLevels() const;
private:
	///
	bool delete_layout(docstring const &);
	///
	bool do_readStyle(Lexer &, Layout &);
	/// Layout file name
	std::string name_;
	/// document class name
	std::string latexname_;
	/// document class description
	std::string description_;
	/// Specific class options
	std::string opt_fontsize_;
	///
	std::string opt_pagestyle_;
	///
	std::string options_;
	///
	std::string pagestyle_;
	///
	std::string class_header_;
	///
	docstring defaultlayout_;
	/// preamble text to support layout styles
	docstring preamble_;
	/// latex packages loaded by document class.
	std::set<std::string> provides_;
	///
	unsigned int columns_;
	///
	PageSides sides_;
	/// header depth to have numbering
	int secnumdepth_;
	/// header depth to appear in table of contents
	int tocdepth_;
	/// Can be LaTeX, DocBook, etc.
	OutputType outputType_;
	/** Base font. The paragraph and layout fonts are resolved against
	    this font. This has to be fully instantiated. Attributes
	    Font::INHERIT, Font::IGNORE, and Font::TOGGLE are
	    extremely illegal.
	*/
	Font defaultfont_;
	/// Text that dictates how wide the left margin is on the screen
	docstring leftmargin_;

	/// Text that dictates how wide the right margin is on the screen
	docstring rightmargin_;

	/// The type of command used to produce a title
	LYX_TITLE_LATEX_TYPES titletype_;
	/// The name of the title command
	std::string titlename_;

	/// Paragraph styles used in this layout
	LayoutList layoutlist_;
	/// CharStyles available to this layout
	mutable CharStyles charstylelist_;

	/// Input layouts available to this layout
	mutable InsetLayouts insetlayoutlist_;

	/// available types of float, eg. figure, algorithm.
	boost::shared_ptr<FloatList> floatlist_;

	/// Types of counters, eg. sections, eqns, figures, avail. in document class.
	boost::shared_ptr<Counters> counters_;

	/// Has this layout file been loaded yet?
	mutable bool loaded_;

	/// Is the TeX class available?
	bool texClassAvail_;

	/// The minimal TocLevel of sectioning layouts
	int min_toclevel_;
	/// The maximal TocLevel of sectioning layouts
	int max_toclevel_;
};


/// convert page sides option to text 1 or 2
std::ostream & operator<<(std::ostream & os, TextClass::PageSides p);


} // namespace lyx

#endif
