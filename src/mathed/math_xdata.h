// -*- C++ -*-

#ifndef MATHEDXARRAY_H
#define MATHEDXARRAY_H

#include <iosfwd>

#include "math_data.h"
//#include "math_metricsinfo.h"
#include "dimension.h"

#ifdef __GNUG__
#pragma interface
#endif

class MathMetricsInfo;
class MathPainterInfo;
class TextMetricsInfo;
class TextPainter;


/** This class extends a MathArray by drawing routines and caches for
 * metric information.
 */
class MathXArray : private MathArray 
{
public:
	// re-use inherited stuff
	using MathArray::size_type;
	using MathArray::const_iterator;
	using MathArray::begin;
	using MathArray::end;
	using MathArray::operator[];
	using MathArray::clear;
	using MathArray::size;
	using MathArray::empty;
	using MathArray::back;
	MathArray & data() { return *this; }
	MathArray const & data() const { return *this; }

	/// constructor
	MathXArray();
	/// rebuild cached metrics information
	Dimension const & metrics(MathMetricsInfo & mi) const;
	/// redraw cell using cache metrics information
	void draw(MathPainterInfo & pi, int x, int y) const;
	/// rebuild cached metrics information
	Dimension const & metricsT(TextMetricsInfo const & mi) const;
	/// redraw cell using cache metrics information
	void drawT(TextPainter & pi, int x, int y) const;
	/// mark cell for re-drawing
	void touch() const;

	/// access to cached x coordinate of last drawing
	int xo() const { return xo_; }
	/// access to cached y coordinate of last drawing
	int yo() const { return yo_; }
	/// access to cached x coordinate of mid point of last drawing
	int xm() const { return xo_ + dim_.w / 2; }
	/// access to cached y coordinate of mid point of last drawing
	int ym() const { return yo_ + (dim_.d - dim_.a) / 2; }
	/// returns x coordinate of given position in the array
	int pos2x(size_type pos) const;
	/// returns position of given x coordinate
	int pos2x(size_type pos1, size_type pos2, int glue) const;
	/// returns position of given x coordinate
	size_type x2pos(int pos) const;
	/// returns position of given x coordinate fstarting from a certain pos
	size_type x2pos(size_type startpos, int targetx, int glue) const;
	/// returns distance of this cell to the point given by x and y
	// assumes valid position and size cache
	int dist(int x, int y) const;

	/// ascent of this cell above the baseline
	int ascent() const { return dim_.a; }
	/// descent of this cell below the baseline
	int descent() const { return dim_.d; }
	/// height of the cell
	int height() const { return dim_.a + dim_.d; }
	/// width of this cell
	int width() const { return dim_.w; }
	/// dimensions of cell
	Dimension const & dim() const	{ return dim_; }
	/// dimensions of cell
	void setDim(Dimension const & d) const { dim_ = d; }
	/// bounding box of this cell
	void boundingBox(int & xlow, int & xhigh, int & ylow, int & yhigh);
	/// find best position to do things
	//void findPos(PosFinder &) const;
	/// gives center coordinates
	void center(int & x, int & y) const;
	/// adjust (x,y) to point on boundary on a straight line from the center
	void towards(int & x, int & y) const;


private:
	/// cached dimensions of cell
	mutable Dimension dim_;
	/// cached x coordinate of last drawing
	mutable int xo_;
	/// cached y coordinate of last drawing
	mutable int yo_;
	/// cached cleaness of cell
	mutable bool clean_;
	/// cached draw status of cell
	mutable bool drawn_;
};

/// output cell on a stream
std::ostream & operator<<(std::ostream & os, MathXArray const & ar);

#endif
