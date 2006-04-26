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

#include "TocBackend.h"

class LCursor;

namespace lyx {
namespace toc {

typedef TocBackend::Item TocItem;
typedef TocBackend::Toc::const_iterator TocIterator;
typedef TocBackend::Toc Toc;
typedef TocBackend::TocList TocList;

///
void updateToc(Buffer const &);

///
TocList const & getTocList(Buffer const &);

///
Toc const & getToc(Buffer const & buf, std::string const & type);

///
std::vector<std::string> const & getTypes(Buffer const &);

/// Return the first TocItem before the cursor
TocIterator const getCurrentTocItem(Buffer const &, LCursor const &,
									  std::string const & type);

///
void asciiTocList(std::string const &, Buffer const &, std::ostream &);

/** Given the cmdName of the TOC param, returns the type used
    by ControlToc::getContents() */
std::string const getType(std::string const & cmdName);

/** Returns the guiname from a given @c type
    The localization of the names will be done in the frontends */
std::string const getGuiName(std::string const & type, Buffer const &);

/// the type of outline operation
enum OutlineOp {
	UP, // Move this header with text down
	DOWN,   // Move this header with text up
	IN, // Make this header deeper
	OUT // Make this header shallower
};


void outline(OutlineOp, LCursor &);


} // namespace toc
} // namespace lyx

#endif // CONTROLTOC_H
