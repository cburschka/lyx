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

	/** Gets textclass number from name.
	    Returns -1 if textclass name does not exist
	*/
	std::pair<bool, lyx::textclass_type> const
	NumberOfClass(string const & textclass) const;

	///
	LyXTextClass const & operator[](lyx::textclass_type textclass) const;

	/** Read textclass list.
	    Returns false if this fails
	*/
	bool Read();
private:
	///
	mutable ClassList classlist;
	///
	void Add(LyXTextClass const &);
};

/// 
extern LyXTextClassList textclasslist;

#endif
