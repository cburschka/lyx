// -*- C++ -*-
#ifndef MATH_AMSARRAYINSET_H
#define MATH_AMSARRAYINSET_H

#include "math_gridinset.h"

#ifdef __GNUG__
#pragma interface
#endif

//
// used for [pbvV]matrix, psmatrix etc
//


class MathAMSArrayInset : public MathGridInset {
public:
	///
	MathAMSArrayInset(string const & name, int m, int n);
	///
	MathAMSArrayInset(string const & name);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & st) const;
	///
	void draw(MathPainterInfo & pain, int x, int y) const;
	///
	MathAMSArrayInset * asAMSArrayInset() { return this; }
	///
	MathAMSArrayInset const * asAMSArrayInset() const { return this; }

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;

private:
	///
	char const * name_left() const;
	///
	char const * name_right() const;

	///
	string name_;
};

#endif
