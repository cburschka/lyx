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

#include<utility> // for pair

#ifdef __GNUG_
#pragma interface
#endif

#include "LString.h"

class HSV;
class RGB;

struct HSV {
	double h;
	double s;
	double v;
	HSV() : h(0.0), s(0.0), v(0.0) {}
	HSV(double hue, double sat, double val) : h(hue), s(sat), v(val) {}
	HSV( RGB const & );
};

struct RGB {
	int r;
	int g;
	int b;
	RGB() : r(0), g(0), b(0) {}
	RGB(int red, int green, int blue) : r(red), g(green), b(blue) {}
	RGB( HSV const & );
};

typedef std::pair<string, RGB> X11Color;

/// struct holding xform-specific colors
struct XFormColor {
	string name;
	int colorID;
	RGB col;
	XFormColor() : colorID(0) {}
	string const getname() { return name; }
};

inline
bool operator==(RGB const & c1, RGB const & c2)
{
	return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b);
}


inline
bool operator!=(RGB const & c1, RGB const & c2)
{
	return !(c1 == c2);
}

#endif
