// -*- C++ -*-
/**
 * \file Color.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

/* structs RGBColor and HSVColor to enable simple conversion between
 * color spaces.
 */

#ifndef COLOR_H
#define COLOR_H

#include <string>


class LColor_color;


/** Given col, fills r, g, b in the range 0-255.
    The function returns true if successful.
    It returns false on failure and sets r, g, b to 0. */
bool getRGBColor(LColor_color col,
		 unsigned int & r, unsigned int & g, unsigned int & b);

struct RGBColor;
/// returns a string of form #rrggbb, given an RGBColor struct
std::string const X11hexname(RGBColor const & col);

struct HSVColor {
	double h;
	double s;
	double v;
	HSVColor() : h(0.0), s(0.0), v(0.0) {}
	HSVColor(double hue, double sat, double val)
		: h(hue), s(sat), v(val) {}
	HSVColor(RGBColor const &);
};

struct RGBColor {
	unsigned int r;
	unsigned int g;
	unsigned int b;
	RGBColor() : r(0), g(0), b(0) {}
	RGBColor(unsigned int red, unsigned int green, unsigned int blue)
		: r(red), g(green), b(blue) {}
	RGBColor(HSVColor const &);
	/// \param x11hexname is of the form "#ffa071"
	RGBColor(std::string const & x11hexname);
};

struct NamedColor : public RGBColor {
	std::string lyxname;
	std::string guiname;
	NamedColor() : RGBColor() {}
	NamedColor(std::string const & lyx, std::string const & gui,
		   RGBColor const & c)
		: RGBColor(c), lyxname(lyx), guiname(gui) {}
	RGBColor const & color() const { return *this; }
};

inline
bool operator==(RGBColor const & c1, RGBColor const & c2)
{
	return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b);
}


inline
bool operator!=(RGBColor const & c1, RGBColor const & c2)
{
	return !(c1 == c2);
}

#endif
