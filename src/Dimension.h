// -*- C++ -*-
/**
 * \file Dimension.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DIMENSION_H
#define DIMENSION_H

namespace lyx {

/// Simple wrapper around three ints
class Dimension {
public:
	/// constructor
	Dimension() = default;
	/// initialize data
	Dimension(int w, int a, int d) : wid(w), asc(a), des(d) {}
	///
	Dimension & operator=(Dimension const & dim) = default;

	/// glue horizontally
	void operator+=(Dimension const & dim);
	/// set to empty box
	void clear() { wid = asc = des = 0; }
	/// check whether box is empty
	bool empty() const { return wid == 0 && asc == 0 && des == 0; }
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
	int wid = 0;
	/// ascent
	int asc = 0;
	/// descent
	int des = 0;
};

inline
bool operator==(Dimension const & a, Dimension const & b)
{
	return a.wid == b.wid && a.asc == b.asc && a.des == b.des;
}


inline
bool operator!=(Dimension const & a, Dimension const & b)
{
	return !(a == b);
}

class Point {
public:
	Point() = default;
	Point(int x, int y);

	int x_ = 0;
	int y_ = 0;
};

} // namespace lyx

#endif
