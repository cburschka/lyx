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

#include "support/types.h"

#include "LString.h"

#include <vector>

class LyXLex;



///
class LyXTextClass {
public:
	///
	typedef std::vector<LyXLayout> LayoutList;
	///
	typedef LayoutList::const_iterator const_iterator;
	///
	explicit
	LyXTextClass (string const & = string(), 
		      string const & = string(), 
		      string const & = string());

	///
	const_iterator begin() const { return layoutlist.begin(); }
	///
	const_iterator end() const { return layoutlist.end(); }
	
	///
	bool Read(string const & filename, bool merge = false);
	///
	void readOutputType(LyXLex &);
	///
	void readMaxCounter(LyXLex &);
	///
	void readClassOptions(LyXLex &);
	///
	bool hasLayout(string const & name) const;

	///
	LyXLayout const & GetLayout(string const & vname) const;

	///
	LyXLayout & GetLayout(string const & vname);

	/// Sees to that the textclass structure has been loaded
	void load();

	///
	string const & name() const { return name_; }
	///
	string const & latexname() const { return latexname_; }
	///
	string const & description() const { return description_; }
	///
	string const & opt_fontsize() const { return opt_fontsize_; }
	///
	string const & opt_pagestyle() const { return opt_pagestyle_; }
	///
	string const & options() const { return options_; }
	///
	string const & pagestyle() const { return pagestyle_; }
	///
	string const & preamble() const { return preamble_; }

	/// Packages that are already loaded by the class
	enum Provides {
		///
		nothing = 0,
		///
		amsmath = 1,
		///
		makeidx = 2,
		///
		url = 4
	};
	///
	bool provides(Provides p) const { return provides_ & p; }
	
	///
	unsigned int columns() const { return columns_; }
	///
	enum PageSides {
		///
		OneSide,
		///
		TwoSides
	};
	///
	PageSides sides() const { return sides_; }
	///
	int secnumdepth() const { return secnumdepth_; }
	///
	int tocdepth() const { return tocdepth_; }

	///
	OutputType outputType() const { return outputType_; }

	///
	LyXFont const & defaultfont() const;

	/// Text that dictates how wide the left margin is on the screen
	string const & leftmargin() const;

	/// Text that dictates how wide the right margin is on the screen
	string const & rightmargin() const;
        ///
	int maxcounter() const { return maxcounter_; }
	///
	lyx::layout_type numLayouts() const { return layoutlist.size(); }
	///
	LyXLayout const & operator[](lyx::layout_type i) const {
		return layoutlist[i];
	}
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
	LayoutList layoutlist;

	/// Has this layout file been loaded yet?
	bool loaded;
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
