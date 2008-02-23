// -*- C++ -*-
/**
 * \file TextClass.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TEXTCLASS_H
#define TEXTCLASS_H

#include "ColorCode.h"
#include "FontInfo.h"
#include "LayoutEnums.h"
#include "LayoutPtr.h"

#include "insets/InsetLayout.h"

#include "support/docstring.h"
#include "support/types.h"

#include <boost/shared_ptr.hpp>

#include <vector>
#include <set>
#include <map>

namespace lyx {

namespace support { class FileName; }

class Layout;
class Lexer;
class Counters;
class FloatList;

/// List of inset layouts
typedef std::map<docstring, InsetLayout> InsetLayouts;

/// Index in globel text class list. Basically a 'strong typedef'/
class TextClassIndex
{
public:
	///
	TextClassIndex(size_t t) : data_(t) {}
	///
	operator size_t() const { return data_; }
private:
	///
	size_t data_;
};



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
		     bool texClassAvail = false);
	
	/// check whether the TeX class is available
	bool isTeXClassAvailable() const;

	/// paragraph styles begin iterator.
	const_iterator begin() const { return layoutlist_.begin(); }
	/// paragraph styles end iterator
	const_iterator end() const { return layoutlist_.end(); }

	///Enum used with TextClass::read
	enum ReadType { 
		BASECLASS, //>This is a base class, i.e., top-level layout file
		MERGE, //>This is a file included in a layout file
		MODULE //>This is a layout module
	};
	/// Performs the read of the layout file.
	/// \return true on success.
	bool read(support::FileName const & filename, ReadType rt = BASECLASS);
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
	/// Inset layouts of this doc class
	InsetLayouts & insetlayouts() const { return insetlayoutlist_; };
	///
	InsetLayout const & insetlayout(docstring const & name) const;
	///
	docstring const & defaultLayoutName() const;
	///
	LayoutPtr const & defaultLayout() const;
	/// returns a special layout for use when we don't really want one,
	/// e.g., in table cells
	LayoutPtr const & emptyLayout() const 
			{ return operator[](emptylayout_); };
	///
	docstring const & emptyLayoutName() const 
			{ return emptylayout_; }
	///
	std::string const & name() const;
	///
	docstring const & labelstring() const;
	///
	std::string const & latexname() const;
	///
	std::string const & description() const;
	///
	bool isModular() const { return modular_; }
	/// Sets the layout as a modular one. There is never any
	/// need to reset this.
	void markAsModular() { modular_ = true; }
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
	/// features required by the class?
	std::set<std::string> const & requires() const { return requires_; }

	///
	unsigned int columns() const;
	///
	PageSides sides() const;
	///
	int secnumdepth() const;
	///
	int tocdepth() const;

	/// Can be LaTeX, DocBook, etc.
	OutputType outputType() const;

	///
	FontInfo const & defaultfont() const;

	/// Text that dictates how wide the left margin is on the screen
	docstring const & leftmargin() const;

	/// Text that dictates how wide the right margin is on the screen
	docstring const & rightmargin() const;

	/// The type of command used to produce a title
	TitleLatexType titletype() const;
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
	///
	static InsetLayout const & emptyInsetLayout() { return empty_insetlayout_; }
private:
	///
	bool deleteLayout(docstring const &);
	/// \return true for success.
	bool readStyle(Lexer &, Layout &);
	/// Layout file name
	std::string name_;
	/// document class name
	std::string latexname_;
	/// document class description
	std::string description_;
	/// whether this is a modular layout, i.e., whether it has been
	/// modified by loading of layout modules.
	bool modular_;
	///
	std::string opt_fontsize_;
	///
	std::string opt_pagestyle_;
	/// Specific class options
	std::string options_;
	///
	std::string pagestyle_;
	///
	std::string class_header_;
	///
	docstring defaultlayout_;
	/// name of empty layout
	static const docstring emptylayout_;
	/// preamble text to support layout styles
	docstring preamble_;
	/// latex packages loaded by document class.
	std::set<std::string> provides_;
	/// latex packages requested by document class.
	std::set<std::string> requires_;
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
	    FONT_INHERIT, FONT_IGNORE, and FONT_TOGGLE are
	    extremely illegal.
	*/
	FontInfo defaultfont_;
	/// Text that dictates how wide the left margin is on the screen
	docstring leftmargin_;

	/// Text that dictates how wide the right margin is on the screen
	docstring rightmargin_;

	/// The type of command used to produce a title
	TitleLatexType titletype_;
	/// The name of the title command
	std::string titlename_;

	/// Paragraph styles used in this layout
	LayoutList layoutlist_;

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
	///
	static InsetLayout empty_insetlayout_;
};


/// convert page sides option to text 1 or 2
std::ostream & operator<<(std::ostream & os, PageSides p);

} // namespace lyx

#endif
