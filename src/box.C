/**
 * \file box.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

// Code moved out of line and out of box.h by Angus (7 Jan 2002)

#include <config.h>

#include "box.h"

#include "support/LOstream.h"

using std::ostream;


Box::Box(int x1_, int x2_, int y1_, int y2_) :
	x1(x1_), x2(x2_), y1(y1_), y2(y2_)
{}


bool Box::contained(int x, int y)
{
	return (x1 < x && x2 > x && y1 < y && y2 > y);
}


ostream & operator<<(ostream & os, Box const & b)
{
	return os << "x1,y1: " << b.x1 << ',' << b.y1
		 << " x2,y2: " << b.x2 << ',' << b.y2
		 << std::endl;
}
