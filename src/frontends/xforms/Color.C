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

#include <algorithm> // max
#include <cmath> // floor

#include FORMS_H_LOCATION

#ifdef __GNUG_
#pragma implementation
#endif

#include "Color.h"

using std::max;
using std::min;

static int const nohue = -1;

RGBColor::RGBColor(HSVColor const & hsv)
{
	double h = hsv.h;
	double const s = hsv.s;
	double const v = hsv.v;
	
	double rd, gd, bd;
	
	if (h == nohue || s == 0.0) {
		rd = gd = bd = v;
	} else {
		if (h == 360.0) h = 0.0;
		h /= 60.0;

		int const j = max(0, static_cast<int>(::floor(h)));
		//if( j < 0 ) j = 0;

		double const f = h - j;
		double const p = v * (1.0 - s);
		double const q = v * (1.0 - (s * f));
		double const t = v * (1.0 - (s * (1.0 - f)));

		switch (j) {
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


HSVColor::HSVColor(RGBColor const & rgb)
{
	double const r = rgb.r / 255.0;
	double const g = rgb.g / 255.0;
	double const b = rgb.b / 255.0;

	double const maxval = max( max( r, g ), b );
	double const minval = min( min( r, g ), b );

	v = maxval;

	double const diff = maxval - minval;
	if (maxval != 0.0)
		s = diff / maxval;
	else
		s = 0.0;

	h = nohue;
	if (s != 0.0) {
		double const rc = (maxval - r) / diff;
		double const gc = (maxval - g) / diff;
		double const bc = (maxval - b) / diff;

		if (r == maxval)
			h = bc - gc;
		else if (g == maxval)
			h = 2.0 + rc - bc;
		else if (b == maxval)
			h = 4.0 + gc - rc;

		h *= 60.0;
		if (h < 0)
			h += 360;
	}
}
