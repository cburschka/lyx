// -*- C++ -*-

/**
 *  \file dimension.h
 *
 *  This file is part of LyX, the document processor.
 *  Licence details can be found in the file COPYING.
 *
 *  \author André Pönitz
 *
 *  Full author contact details are available in file CREDITS.
 */

#ifndef DIMENSION_H
#define DIMENSION_H


class LyXFont;

/// Simple wrapper around three ints
struct Dimension {
public:
	/// constructor
	Dimension() : w(0), a(0), d(0) {}
	/// initialize data
	Dimension(int ww, int aa, int dd) : w(ww), a(aa), d(dd) {}

	/// glue horizontally
	void operator+=(Dimension const & dim);
	/// set to empty box
	void clear() { w = a = d = 0; }
	/// set to empty box suitble for given font
	void clear(LyXFont const & font);
	/// get height
	int height() const { return a + d; }
	/// get ascent
	int ascent() const { return a; }
	/// get descent
	int descent() const { return d; }
	/// get width
	int width() const { return w; }

public:
	/// these are intentionally public as things like
	///
	///   dim.a += 20; 
	///
	/// are used all over the place and "hiding" those behind
	///
	///   dim.ascent(dim.ascent() + 20);
	///
	/// makes the code neither faster nor clearer
	/// width
	int w;
	/// ascent
	int a;
	/// descent
	int d;
};

#endif
