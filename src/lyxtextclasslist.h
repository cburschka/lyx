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

#ifndef LYXTEXTCLASSLIST_H
#define LYXTEXTCLASSLIST_H

#ifdef __GNUG__
#pragma interface
#endif

#include "support/types.h"

#include <boost/utility.hpp>

#include "LString.h"

#include <vector>

class LyXTextClass;
class LyXLayout;

/// Reads the style files
extern void LyXSetStyle();

///
class LyXTextClassList : boost::noncopyable {
public:
	///
	typedef std::vector<LyXTextClass> ClassList;
	///
	typedef ClassList::const_iterator const_iterator;
	///
	const_iterator begin() const { return classlist.begin(); }
	///
	const_iterator end() const { return classlist.end(); }
	
	/// Gets layout structure from layout number and textclass number
	LyXLayout const & Style(lyx::textclass_type textclass,
	                        lyx::layout_type layout) const;

	/// Gets layout number from textclass number and layout name
	std::pair<bool, lyx::layout_type> const
	NumberOfLayout(lyx::textclass_type textclass, string const & name) const;

	/// Gets a layout name from layout number and textclass number
	string const &
	NameOfLayout(lyx::textclass_type textclass, lyx::layout_type layout) const;

	/** Gets textclass number from name.
	    Returns -1 if textclass name does not exist
	*/
	std::pair<bool, lyx::textclass_type> const
	NumberOfClass(string const & textclass) const;

	///
	string const & NameOfClass(lyx::textclass_type number) const;

	///
	string const & LatexnameOfClass(lyx::textclass_type number) const;

	///
	string const & DescOfClass(lyx::textclass_type number) const;

	///
	LyXTextClass const & TextClass(lyx::textclass_type textclass) const;

	/** Read textclass list.
	    Returns false if this fails
	*/
	bool Read();

	/** Load textclass.
	    Returns false if this fails
	*/
	bool Load(lyx::textclass_type number) const;
private:
	///
	mutable ClassList classlist;
	///
	void Add (LyXTextClass const &);
};

/// 
extern LyXTextClassList textclasslist;

#endif
