// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

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

///
class LyXTextClass {
public:
	///
	typedef std::vector<LyXLayout_ptr> LayoutList;
	///
	typedef LayoutList::const_iterator const_iterator;
	///
	explicit
	LyXTextClass(string const & = string(),
		     string const & = string(),
		     string const & = string());

	///
	const_iterator begin() const { return layoutlist_.begin(); }
	///
	const_iterator end() const { return layoutlist_.end(); }

	///
	bool Read(string const & filename, bool merge = false);
	///
	void readOutputType(LyXLex &);
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

	/// the list of floats defined in the class
	FloatList & floats();
	/// the list of floats defined in the class
	FloatList const & floats() const;
	/// The Counters present in this textclass.
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

	///
	OutputType outputType() const;

	///
	LyXFont const & defaultfont() const;

	/// Text that dictates how wide the left margin is on the screen
	string const & leftmargin() const;

	/// Text that dictates how wide the right margin is on the screen
	string const & rightmargin() const;
	///
	int maxcounter() const;
	///
	int size() const;
private:
	///
	bool delete_layout(string const &);
	///
	bool do_readStyle(LyXLex &, LyXLayout &);
	///
	string name_;
	///
	string latexname_;
	///
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
	///
	string preamble_;
	///
	Provides provides_;
	///
	unsigned int columns_;
	///
	PageSides sides_;
	///
	int secnumdepth_;
	///
	int tocdepth_;
	///
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
	///
	int maxcounter_; // add approp. signedness

	///
	LayoutList layoutlist_;

	///
	boost::shared_ptr<FloatList> floatlist_;

	///
	boost::shared_ptr<Counters> ctrs_;

	/// Has this layout file been loaded yet?
	mutable bool loaded;
};


///
inline
void operator|=(LyXTextClass::Provides & p1, LyXTextClass::Provides p2)
{
	p1 = static_cast<LyXTextClass::Provides>(p1 | p2);
}


///
std::ostream & operator<<(std::ostream & os, LyXTextClass::PageSides p);

#endif
