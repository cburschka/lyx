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

#ifndef COLOR_H
#define COLOR_H

#include <utility> // for pair

#ifdef __GNUG_
#pragma interface
#endif

#include "LString.h"

class HSVColor;
class RGBColor;

struct HSVColor {
	double h;
	double s;
	double v;
	HSVColor() : h(0.0), s(0.0), v(0.0) {}
	HSVColor(double hue, double sat, double val) : h(hue), s(sat), v(val) {}
	HSVColor( RGBColor const & );
};

struct RGBColor {
	int r;
	int g;
	int b;
	RGBColor() : r(0), g(0), b(0) {}
	RGBColor(int red, int green, int blue) : r(red), g(green), b(blue) {}
	RGBColor( HSVColor const & );
};

typedef std::pair<string, RGBColor> X11Color;

/// struct holding xform-specific colors
struct XformColor {
	string name;
	int colorID;
	RGBColor col;
	XformColor() : colorID(0) {}
	string const getname() { return name; }
	static bool read( string const & );
	static bool write( string const & );
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
