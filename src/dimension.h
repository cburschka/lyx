// -*- C++ -*-
/**
 * \file dimension.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DIMENSION_H
#define DIMENSION_H


class LyXFont;

/// Simple wrapper around three ints
struct Dimension {
public:
	/// constructor
	Dimension() : wid(0), asc(0), des(0) {}
	/// initialize data
	Dimension(int w, int a, int d) : wid(w), asc(a), des(d) {}

	/// glue horizontally
	void operator+=(Dimension const & dim);
	/// set to empty box
	void clear() { wid = asc = des = 0; }
	/// set to empty box suitble for given font
	void clear(LyXFont const & font);
	/// get height
	int height() const { return asc + des; }
	/// get ascent
	int ascent() const { return asc; }
	/// get descent
	int descent() const { return des; }
	/// get width
	int width() const { return wid; }

	/// add space for a frame
	//void addFrame(int frame) const;
	/// add space for bottom part of a frame
	//void addFrameBottom(int frame) const;

public:
	/// these are intentionally public as things like
	///
	///   dim.asc += 20;
	///
	/// are used all over the place and "hiding" those behind
	///
	///   dim.ascent(dim.ascent() + 20);
	///
	/// makes the code neither faster nor clearer
	/// width
	int wid;
	/// ascent
	int asc;
	/// descent
	int des;
};

#endif
