// -*- C++ -*-

#ifndef MATHEDXARRAY_H
#define MATHEDXARRAY_H

#include <iosfwd>
#include "array.h"

#ifdef __GNUG__
#pragma interface
#endif

class Painter;

class MathXArray
{
public:
	///
	MathXArray();
	///
	void Metrics(MathStyles st);
	///
	void draw(Painter & pain, int x, int y);

	///
	int xo() const { return xo_; }
	///
	int yo() const { return yo_; }
	///
	int pos2x(int pos) const;
	///
	int x2pos(int pos) const;
	///
	int width(int pos) const;

	///
	int ascent() const { return ascent_; }
	///
	int descent() const { return descent_; }
	///
	int height() const { return ascent_ + descent_; }
	///
	int width() const { return width_; }
	///
	MathStyles style() const { return style_; }
	

	///
	MathArray data_;
	///
	int width_;
	///
	int ascent_;
	///
	int descent_;
	///
	int xo_;
	///
	int yo_;
	///
	MathStyles style_;
};

std::ostream & operator<<(std::ostream & os, MathArray const & ar);

#endif
