// -*- C++ -*-

#ifndef MATHEDXARRAY_H
#define MATHEDXARRAY_H

#include <iosfwd>

#include "math_data.h"
#include "math_metricsinfo.h"

#ifdef __GNUG__
#pragma interface
#endif

class Painter;

class MathXArray
{
public:
	///
	typedef MathArray::size_type       size_type;
	///
	typedef MathArray::const_iterator  const_iterator;

	///
	MathXArray();
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter & pain, int x, int y) const;

	///
	int xo() const { return xo_; }
	///
	int yo() const { return yo_; }
	///
	int pos2x(size_type pos) const;
	///
	size_type x2pos(int pos) const;
	/// returns distance of this cell to the point given by x and y
	// assumes valid position and size cache
	int dist(int x, int y) const;

	///
	int ascent() const { return ascent_; }
	///
	int descent() const { return descent_; }
	///
	int height() const { return ascent_ + descent_; }
	///
	int width() const { return width_; }

	///
	const_iterator begin() const { return data_.begin(); }
	///
	const_iterator end() const { return data_.end(); }
	
public:
	///
	MathArray data_;
	///
	mutable int width_;
	///
	mutable int ascent_;
	///
	mutable int descent_;
	///
	mutable int xo_;
	///
	mutable int yo_;
	///
	mutable MathMetricsInfo size_;
};

std::ostream & operator<<(std::ostream & os, MathXArray const & ar);

#endif
