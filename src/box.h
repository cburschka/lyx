// -*- C++ -*-
/**
 * \file box.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BOX_H
#define BOX_H

#include <iosfwd>

/**
 * A simple class representing rectangular regions.
 * It is expected that the box be constructed in
 * normalised form, that is to say : x1,y1 is top-left,
 * x2,y2 is bottom-right.
 *
 * Negative values are allowed.
 */
struct Box {
	int x1;
	int x2;
	int y1;
	int y2;

	/// Initialise the member variables.
	Box(int x1_, int x2_, int y1_, int y2_);

	/**
	 * Returns true if the given co-ordinates are within
	 * the box. Check is exclusive (point on a border
	 * returns false).
	 */
	bool contained(int x, int y);
};


std::ostream & operator<<(std::ostream &, Box const &);

#endif // BOX_H
