// -*- C++ -*-

#ifndef MATHEDXARRAY_H
#define MATHEDXARRAY_H

#include <iosfwd>

#include "math_data.h"
#include "math_metricsinfo.h"
#include "dimension.h"

#ifdef __GNUG__
#pragma interface
#endif

class MathPainterInfo;
class TextPainter;


/** This class extends a MathArray by drawing routines and caches for
 * metric information.
 */
class MathXArray
{
public:
	/// type for positions and sizes
	typedef MathArray::size_type       size_type;
	/// const iterator into the underlying MathArray
	typedef MathArray::const_iterator  const_iterator;

	// helper structure for external metrics computations as done
	// in parboxes
	struct Row {
		/// constructor
		Row() {}
		/// first position of this row
		size_type begin; 
		/// last position of this row plus one
		size_type end; 
		/// y offset relative to yo
		int yo;
		/// dimensions of this row
		Dimension dim;
		/// glue between words
		int glue;
	};

	/// constructor
	MathXArray();
	/// rebuild cached metrics information
	Dimension const & metrics(MathMetricsInfo & mi) const;
	/// rebuild cached metrics information
	void metricsExternal(MathMetricsInfo & mi, std::vector<Dimension> &) const;
	/// redraw cell using cache metrics information
	void draw(MathPainterInfo & pi, int x, int y) const;
	/// redraw cell using external metrics information
	void drawExternal(MathPainterInfo & pi, int x, int y,
		std::vector<MathXArray::Row> const &) const;
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
	/// bounding box of this cell
	void boundingBox(int & xlow, int & xhigh, int & ylow, int & yhigh);
	/// find best position to do things
	//void findPos(PosFinder &) const;
	/// gives center coordinates
	void center(int & x, int & y) const;
	/// adjust (x,y) to point on boundary on a straight line from the center
	void towards(int & x, int & y) const;

	/// begin iterator of the underlying MathArray
	const_iterator begin() const { return data_.begin(); }
	/// end iterator of the underlying MathArray
	const_iterator end() const { return data_.end(); }
	/// access to data
	MathArray const & data() const { return data_; }
	/// access to data
	MathArray & data() { return data_; }

private:
	/// the underlying MathArray
	MathArray data_;
	/// cached dimensions of cell
	mutable Dimension dim_;
	/// cached x coordinate of last drawing
	mutable int xo_;
	/// cached y coordinate of last drawing
	mutable int yo_;
	/// cache size information of last drawing
	mutable MathMetricsInfo size_;
	/// cached cleaness of cell
	mutable bool clean_;
	/// cached draw status of cell
	mutable bool drawn_;
};

/// output cell on a stream
std::ostream & operator<<(std::ostream & os, MathXArray const & ar);

#endif
