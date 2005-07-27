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
 *
 * Nice functions and objects to handle TOCs
 */

#ifndef TOC_H
#define TOC_H

#include <map>
#include <iosfwd>
#include <vector>
#include <string>

class Buffer;
class LyXView;
class Paragraph;
class FuncRequest;

namespace lyx {
namespace toc {

///
class TocItem {
public:
	TocItem(int par_id, int d, std::string const & s)
		: id_(par_id), depth(d), str(s) {}
	///
	std::string const asString() const;
	/// set cursor in LyXView to this TocItem
	void goTo(LyXView & lv_) const;
	/// the action corresponding to the goTo above
	FuncRequest action() const;
	/// Paragraph ID containing this item
	int id_;
	/// nesting depth
	int depth;
	///
	std::string str;
};

///
typedef std::vector<TocItem> Toc;
///
typedef std::map<std::string, Toc> TocList;

///
TocList const getTocList(Buffer const &);

///
std::vector<std::string> const getTypes(Buffer const &);

///
void asciiTocList(std::string const &, Buffer const &, std::ostream &);

/** Given the cmdName of the TOC param, returns the type used
    by ControlToc::getContents() */
std::string const getType(std::string const & cmdName);

/** Returns the guiname from a given @c type
    The localization of the names will be done in the frontends */
std::string const getGuiName(std::string const & type, Buffer const &);

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
