// -*- C++ -*-
/**
 * \file RowFlags.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef ROWFLAGS_H
#define ROWFLAGS_H

// Do not include anything here

namespace lyx {

/* The list of possible flags, that can be combined.
 * Some flags that should logically be here (e.g.,
 * CanBreakBefore), do not exist. This is because the need has not
 * been identitfied yet.
 *
 * Priorities when before/after disagree:
 *      AlwaysBreak* > NoBreak* > Break* or CanBreak*.
 */
enum RowFlags {
	// Do not break before or after this element, except if really
	// needed (between NoBreak* and CanBreak*).
	Inline = 0,
	// break row before this element if the row is not empty
	BreakBefore = 1 << 0,
	// Avoid breaking row before this element
	NoBreakBefore = 1 << 1,
	// force new (maybe empty) row after this element
	AlwaysBreakAfter = 1 << 2,
	// break row after this element if there are more elements
	BreakAfter = 1 << 3,
	// break row whenever needed after this element
	CanBreakAfter = 1 << 4,
	// Avoid breaking row after this element
	NoBreakAfter = 1 << 5,
	// The contents of the row may be broken in two (e.g. string)
	CanBreakInside = 1 << 6,
	// specify an alignment (left, right) for a display element
	// (default is center)
	AlignLeft = 1 << 7,
	AlignRight = 1 << 8,
	// A display element breaks row at both ends
	Display = BreakBefore | BreakAfter,
	// Flags that concern breaking after element
	AfterFlags = AlwaysBreakAfter | BreakAfter | CanBreakAfter | NoBreakAfter
};

} // namespace lyx

#endif
