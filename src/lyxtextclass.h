// -*- C++ -*-
/**
 * \file lyxtextclass.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef LYXTEXTCLASS_H
#define LYXTEXTCLASS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxlayout.h"
#include "LString.h"
#include "lyxlayout_ptr_fwd.h"

#include "support/types.h"

#include <boost/shared_ptr.hpp>

#include <vector>

class LyXLex;
class Counters;
class FloatList;

/// Stores the layout specification of a LyX document class.
class LyXTextClass {
public:
	/// The individual styles comprising the document class
	typedef std::vector<LyXLayout_ptr> LayoutList;
	/// Enumerate the paragraph styles.
	typedef LayoutList::const_iterator const_iterator;
	/// Construct a layout with default values. Actual values loaded later.
	explicit
	LyXTextClass(string const & = string(),
		     string const & = string(),
		     string const & = string());

	/// paragraph styles begin iterator.
	const_iterator begin() const { return layoutlist_.begin(); }
	/// paragraph styles end iterator
	const_iterator end() const { return layoutlist_.end(); }

	/// Performs the read of the layout file.
	bool Read(string const & filename, bool merge = false);
	///
	void readOutputType(LyXLex &);
	///
	void readTitleType(LyXLex &);
	///
	void readMaxCounter(LyXLex &);
	///
	void readClassOptions(LyXLex &);
	///
	void readFloat(LyXLex &);
	///
	void readCounter(LyXLex &);
	///
	bool hasLayout(string const & name) const;

	///
	LyXLayout_ptr const & operator[](string const & vname) const;

	/// Sees to that the textclass structure has been loaded
	bool load() const;

	/// the list of floats defined in the document class
	FloatList & floats();
	/// the list of floats defined in the document class
	FloatList const & floats() const;
	/// The Counters present in this document class.
	Counters & counters() const;
	///
	string const & defaultLayoutName() const;
	///
	LyXLayout_ptr const & defaultLayout() const;
	///
	string const & name() const;
	///
	string const & latexname() const;
	///
	string const & description() const;
	///
	string const & opt_fontsize() const;
	///
	string const & opt_pagestyle() const;
	///
	string const & options() const;
	///
	string const & pagestyle() const;
	///
	string const & preamble() const;

	/// Packages that are already loaded by the class
	enum Provides {
		///
		nothing = 0,
		///
		amsmath = 1,
		///
		makeidx = 2,
		///
		url = 4,
		///
		natbib = 8
	};
	///
	bool provides(Provides p) const;

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

	/// Can be LaTeX, LinuxDoc, etc.
	OutputType outputType() const;

	///
	LyXFont const & defaultfont() const;

	/// Text that dictates how wide the left margin is on the screen
	string const & leftmargin() const;

	/// Text that dictates how wide the right margin is on the screen
	string const & rightmargin() const;
	///
	int maxcounter() const;

	/// The type of command used to produce a title
	LYX_TITLE_LATEX_TYPES titletype() const;
	/// The name of the title command
	string const & titlename() const;

	///
	int size() const;
private:
	///
	bool delete_layout(string const &);
	///
	bool do_readStyle(LyXLex &, LyXLayout &);
	/// Layout file name
	string name_;
	/// document class name
	string latexname_;
	/// document class description
	string description_;
	/// Specific class options
	string opt_fontsize_;
	///
	string opt_pagestyle_;
	///
	string options_;
	///
	string pagestyle_;
	///
	string defaultlayout_;
	/// preamble text to support layout styles
	string preamble_;
	/// latex packages loaded by document class.
	Provides provides_;
	///
	unsigned int columns_;
	///
	PageSides sides_;
	/// header depth to have numbering
	int secnumdepth_;
	/// header depth to appear in table of contents
	int tocdepth_;
	/// Can be LaTeX, LinuxDoc, etc.
	OutputType outputType_;
	/** Base font. The paragraph and layout fonts are resolved against
	    this font. This has to be fully instantiated. Attributes
	    LyXFont::INHERIT, LyXFont::IGNORE, and LyXFont::TOGGLE are
	    extremely illegal.
	*/
	LyXFont defaultfont_;
	/// Text that dictates how wide the left margin is on the screen
	string leftmargin_;

	/// Text that dictates how wide the right margin is on the screen
	string rightmargin_;
	/// highest header level used in this layout.
	int maxcounter_; // add approp. signedness

	/// The type of command used to produce a title
	LYX_TITLE_LATEX_TYPES titletype_;
	/// The name of the title command
	string titlename_;

	/// Paragraph styles used in this layout
	LayoutList layoutlist_;

	/// available types of float, eg. figure, algorithm.
	boost::shared_ptr<FloatList> floatlist_;

	/// Types of counters, eg. sections, eqns, figures, avail. in document class.
	boost::shared_ptr<Counters> ctrs_;

	/// Has this layout file been loaded yet?
	mutable bool loaded;
};


/// Merge two different provides flags into one bit field record
inline
void operator|=(LyXTextClass::Provides & p1, LyXTextClass::Provides p2)
{
	p1 = static_cast<LyXTextClass::Provides>(p1 | p2);
}


/// convert page sides option to text 1 or 2
std::ostream & operator<<(std::ostream & os, LyXTextClass::PageSides p);

#endif
