/**
 * \file box.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

// Code moved out of line and out of box.h by Angus (7 Jan 2002)

#include "box.h"

#include "support/LOstream.h"

using std::ostream;

Box::Box(int x1_, int x2_, int y1_, int y2_) :
	x1(x1_), x2(x2_), y1(y1_), y2(y2_)
{}

bool Box::contained(int x, int y)
{
	return (x1 < x && x2 > x &&
		y1 < y && y2 > y);
}

	 
ostream & operator<<(ostream & o, Box & b)
{
	return o << "x1,y1: " << b.x1 << "," << b.y1
		<< " x2,y2: " << b.x2 << "," << b.y2 << std::endl;
}
 
