// -*- C++ -*-
#ifndef MATH_MATRIXINSET_H
#define MATH_MATRIXINSET_H

#include "math_arrayinset.h"

#ifdef __GNUG__
#pragma interface
#endif

// "shortcut" for DelimInset("(",ArrayInset,")")

class MathMatrixInset : public MathArrayInset {
public:
	///
	MathMatrixInset(MathArrayInset const &);
	///
	MathMatrixInset(string const & str);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo const &) const {}
	/// identifies MatrixInsets
	MathMatrixInset const * asMatrixInset() const { return this; }

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maplize(MapleStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octavize(OctaveStream &) const;
};

#endif
