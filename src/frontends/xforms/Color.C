/**
 * \file Color.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Color.h"

#include "lyx_forms.h"

#include "support/std_sstream.h"

#include <cmath>
#include <iomanip>


#ifndef CXX_GLOBAL_CSTD
using std::floor;
#endif

using std::max;
using std::min;
using std::setw;

using std::istringstream;
using std::ostringstream;


namespace {

int const nohue = -1;

int hexstrToInt(string const & str)
{
        int val = 0;
        istringstream is(str);
        is >> std::setbase(16) >> val;
        return val;
}

} // namespace anon



bool getRGBColor(LColor::color col,
		 unsigned int & r, unsigned int & g, unsigned int & b)
{
	string const name = lcolor.getX11Name(col);
	Display * const display = fl_get_display();
	Colormap const cmap = fl_state[fl_get_vclass()].colormap;
	XColor xcol, ccol;

	if (XLookupColor(display, cmap, name.c_str(), &xcol, &ccol) == 0) {
		r = 0;
		g = 0;
		b = 0;
		return false;
	}

	r = xcol.red   / 256;
	g = xcol.green / 256;
	b = xcol.blue  / 256;
	return true;
}


string const X11hexname(RGBColor const & col)
{
	ostringstream ostr;

	ostr << '#' << std::setbase(16) << std::setfill('0')
	     << setw(2) << col.r
	     << setw(2) << col.g
	     << setw(2) << col.b;

	return STRCONV(ostr.str());
}


RGBColor::RGBColor(string const & x11hexname)
	: r(0), g(0), b(0)
{
	BOOST_ASSERT(x11hexname.size() == 7 && x11hexname[0] == '#');
	r = hexstrToInt(x11hexname.substr(1,2));
	g = hexstrToInt(x11hexname.substr(3,2));
	b = hexstrToInt(x11hexname.substr(5,2));
}


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
		//if (j < 0) j = 0;

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

	r = static_cast<int>(::floor((rd * 255.0) + 0.5));
	g = static_cast<int>(::floor((gd * 255.0) + 0.5));
	b = static_cast<int>(::floor((bd * 255.0) + 0.5));
}


HSVColor::HSVColor(RGBColor const & rgb)
{
	double const r = rgb.r / 255.0;
	double const g = rgb.g / 255.0;
	double const b = rgb.b / 255.0;

	double const maxval = max(max(r, g), b);
	double const minval = min(min(r, g), b);

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
