// -*- C++ -*-
/**
 * \file toc.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TOC_H
#define TOC_H

#include "support/LOstream.h"
#include "LString.h"

#include <map>
#include <vector>

class Buffer;
class LyXView;
class Paragraph;

/** Nice functions and objects to handle TOCs
 */
namespace lyx {
namespace toc {

///
struct TocItem {
	TocItem(int par_id, int d, string const & s)
		: id_(par_id), depth(d), str(s) {}
	///
	string const asString() const;
	/// set cursor in LyXView to this TocItem
	void goTo(LyXView & lv_) const;
	/// the action corresponding to the goTo above
	int action() const;
	/// Paragraph ID containing this item
	int id_;
	/// nesting depth
	int depth;
	///
	string str;
};

///
typedef std::vector<TocItem> Toc;
///
typedef std::map<string, Toc> TocList;

///
TocList const getTocList(Buffer const &);

///
std::vector<string> const getTypes(Buffer const &);

///
void asciiTocList(string const &, Buffer const &, std::ostream &);

/** Given the cmdName of the TOC param, returns the type used
    by ControlToc::getContents() */
string const getType(string const & cmdName);

inline
bool operator==(TocItem const & a, TocItem const & b)
{
	return a.id_ == b.id_ && a.str == b.str;
	// No need to compare depth.
}


inline
bool operator!=(TocItem const & a, TocItem const & b)
{
	return !(a == b);
}


} // namespace toc
} // namespace lyx

#endif // CONTROLTOC_H
