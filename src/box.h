/**
 * \file box.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef BOX_H
#define BOX_H

#include <config.h>
 
#include "debug.h" 
 
/**
 * A simple class representing rectangular regions.
 * It is expected that the box be constructed in
 * normalised form, that is to say : x1,y1 is top-left,
 * x2,y2 is bottom-right.
 */
struct Box {
	unsigned int x1;
	unsigned int x2;
	unsigned int y1;
	unsigned int y2;

	Box(unsigned int x1_, unsigned int x2_,
		unsigned int y1_, unsigned int y2_) :
		x1(x1_), x2(x2_), y1(y1_), y2(y2_) {}

	/**
	 * Returns true if the given co-ordinates are within
	 * the box. Check is exclusive (point on a border
	 * returns false).
	 */
	bool contained(unsigned int x, unsigned int y) {
		return (x1 < x && x2 > x &&
			y1 < y && y2 > y);
	}
};
 
#endif // BOX_H
