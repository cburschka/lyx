// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2002 The LyX Team.
 *
 * ======================================================
 *
 * \file toc.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 * \author Jean-Marc Lasgouttes <lasgouttes@freesurf.fr>
 */

#ifndef TOC_H
#define TOC_H

#ifdef __GNUG__
#pragma interface
#endif

#include <config.h>
 
#include "support/LOstream.h"
#include "LString.h"

#include <map>
#include <vector>

class Buffer;
class LyXView;
class Paragraph;

/** Nice functions and objects to handle TOCs
 */
namespace toc 
{

///
struct TocItem {
	TocItem(Paragraph * p, int d, string const & s)
		: par(p), depth(d), str(s) {}
	///
	string const asString() const;
	/// set cursor in LyXView to this TocItem
	void goTo(LyXView & lv_) const;
	/// the action corresponding to the goTo above
	int action() const;
	///
	Paragraph * par;
	///
	int depth;
	///
	string str;
};

///
typedef std::vector<TocItem> Toc;
///
typedef std::map<string, Toc> TocList;

///
TocList const getTocList(Buffer const *);

///
std::vector<string> const getTypes(Buffer const *);

///
void asciiTocList(string const &, Buffer const *, std::ostream &);
	
/** Given the cmdName of the TOC param, returns the type used
    by ControlToc::getContents() */
string const getType(string const & cmdName);

///
inline
bool operator==(TocItem const & a, TocItem const & b)
{
	return a.par == b.par && a.str == b.str;
	// No need to compare depth.
}


///
inline
bool operator!=(TocItem const & a, TocItem const & b)
{
	return !(a == b);
	// No need to compare depth.
}


} // namespace toc

#endif // CONTROLTOC_H
