// -*- C++ -*-
/**
 * \file Color.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

/* structs RGBColor and HSVColor to enable simple conversion between
 * color spaces.
 */

#ifndef COLOR_H
#define COLOR_H


#include "LString.h"
#include "LColor.h"

/** Given col, fills r, g, b in the range 0-255.
    The function returns true if successful.
    It returns false on failure and sets r, g, b to 0. */
bool getRGBColor(LColor::color col,
		 unsigned int & r, unsigned int & g, unsigned int & b);

struct RGBColor;

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
};

struct NamedColor : public RGBColor {
	string name;
	NamedColor() : RGBColor() {}
	NamedColor(string const & n, RGBColor const & c)
		: RGBColor(c), name(n) {}
	RGBColor const & color() const { return *this; }
	string const & getname() const { return name; }
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
