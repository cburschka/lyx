// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG_
#pragma implementation
#endif

#include <algorithm> // max
#include <cmath> // floor
#include "Color.h"

using std::max;
using std::min;

static int const nohue = -1;

RGB::RGB( HSV const & hsv )
{
	double h = hsv.h;
	double s = hsv.s;
	double v = hsv.v;
	
	double rd, gd, bd;
	
	if( h == nohue || s == 0.0 ) {
		rd = gd = bd = v;
	} else {
		if( h == 360.0 ) h = 0.0;
		h /= 60.0;

		int j = static_cast<int>( ::floor(h) );
		if( j < 0 ) j = 0;

		double f = h - j;
		double p = v * (1.0 - s);
		double q = v * (1.0 - (s*f));
		double t = v * (1.0 - (s*(1.0 - f)));

		switch( j ) {
		case 0:
			rd = v;
			gd = t;
			bd = p;
			break;
		case 1:
			rd = q;
			gd = v;
			bd = p;
			break;
		case 2:
			rd = p;
			gd = v;
			bd = t;
			break;
		case 3:
			rd = p;
			gd = q;
			bd = v;
			break;
		case 4:
			rd = t;
			gd = p;
			bd = v;
			break;
		case 5:
			rd = v;
			gd = p;
			bd = q;
			break;
		default:
			rd = v;
			gd = t;
			bd = p;
			break;  // should never happen.
		}
	}

	r = static_cast<int>( ::floor((rd * 255.0) + 0.5) );
	g = static_cast<int>( ::floor((gd * 255.0) + 0.5) );
	b = static_cast<int>( ::floor((bd * 255.0) + 0.5) );
}


HSV::HSV( RGB const & rgb )
{
	// r, g, b lie in the range 0-1, not 0-255.
	double r = rgb.r / 255.0;
	double g = rgb.g / 255.0;
	double b = rgb.b / 255.0;

	double maxval = max( max( r, g ), b );
	double minval = max( min( r, g ), b );

	v = maxval;

	double diff = maxval - minval;
	if( maxval != 0.0 )
		s = diff / maxval;
	else
		s = 0.0;

	h = nohue;
	if( s != 0.0 ) {
		double rc = (maxval - r) / diff;
		double gc = (maxval - g) / diff;
		double bc = (maxval - b) / diff;

		if( r == maxval )
			h = bc - gc;
		else if( g == maxval )
			h = 2.0 + rc - bc;
		else if( b == maxval )
			h = 4.0 + gc - rc;

		h *= 60.0;
		if ( h < 0 )
			h += 360;
	}
}

