// -*- C++ -*-
#ifndef MATH_AMSARRAYINSET_H
#define MATH_AMSARRAYINSET_H

#include "math_gridinset.h"

#ifdef __GNUG__
#pragma interface
#endif


class MathAMSArrayInset : public MathGridInset {
public: 
	///
	MathAMSArrayInset(string const & name_, int m, int n);
	///
	MathAMSArrayInset(string const & name_);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter & pain, int x, int y) const;
	///
	MathAMSArrayInset * asAMSArrayInset() { return this; }

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maplize(MapleStream &) const;

private:
	///
	char const * name_left() const;
	///
	char const * name_right() const;

	///
	string name_;
};

#endif
