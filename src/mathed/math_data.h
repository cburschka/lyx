// -*- C++ -*-
/*
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

/** \class MathArray
    \brief Low level container for math insets
    \author Alejandro Aguilar Sierra
    \author André Pönitz
    \author Lars Gullik Bjønnes
    \version February 2001
  */

#ifndef MATH_DATA_H
#define MATH_DATA_H

#include <iosfwd>
#include <vector>

#include "math_atom.h"
#include "dimension.h"

class MathMacro;
class LaTeXFeatures;
class ReplaceData;
class MathMetricsInfo;
class MathPainterInfo;
class TextMetricsInfo;
class TextPainter;


#ifdef __GNUG__
#pragma interface
#endif


class MathArray : private std::vector<MathAtom> {
public:
	/// re-use inhertited stuff
	typedef std::vector<MathAtom> base_type;
	using base_type::const_iterator;
	using base_type::iterator;
	using base_type::size_type;
	using base_type::difference_type;
	using base_type::size;
	using base_type::empty;
	using base_type::clear;
	using base_type::begin;
	using base_type::end;
	using base_type::push_back;
	using base_type::pop_back;
	using base_type::back;
	using base_type::front;
	using base_type::swap;
	///
	typedef size_type idx_type;
	typedef size_type pos_type;
	
public:
	///
	MathArray();
	///
	MathArray(const_iterator from, const_iterator to);
	///
	void append(MathArray const & ar);

	/// inserts single atom at position pos
	void insert(size_type pos, MathAtom const & at);
	/// inserts multiple atoms at position pos
	void insert(size_type pos, MathArray const & ar);

	/// erase range from pos1 to pos2
	void erase(iterator pos1, iterator pos2);
	/// erase single atom
	void erase(iterator pos);
	/// erase range from pos1 to pos2
	void erase(size_type pos1, size_type pos2);
	/// erase single atom
	void erase(size_type pos);

	///
	void dump() const;
	///
	void dump2() const;
	///
	void substitute(MathMacro const & macro);
	///
	void replace(ReplaceData &);

	/// looks for exact match
	bool match(MathArray const & ar) const;
	/// looks for inclusion match starting at pos
	bool matchpart(MathArray const & ar, pos_type pos) const;
	/// looks for containment, return == size mean not found
	size_type find(MathArray const & ar) const;
	/// looks for containment, return == size mean not found
	size_type find_last(MathArray const & ar) const;
	///
	bool contains(MathArray const & ar) const;
	///
	void validate(LaTeXFeatures &) const;

	/// checked write access
	MathAtom & operator[](pos_type);
	/// checked read access
	MathAtom const & operator[](pos_type) const;
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
	/// gives center coordinates
	void center(int & x, int & y) const;
	/// adjust (x,y) to point on boundary on a straight line from the center
	void towards(int & x, int & y) const;

private:
	/// is this an exact match at this position?
	bool find1(MathArray const & ar, size_type pos) const;

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

///
std::ostream & operator<<(std::ostream & os, MathArray const & ar);


#endif
