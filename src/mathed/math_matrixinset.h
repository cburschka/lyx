// -*- C++ -*-
#ifndef MATH_MATRIXINSET_H
#define MATH_MATRIXINSET_H

#include "math_gridinset.h"


// "shortcut" for DelimInset("(",ArrayInset,")")

class MathMatrixInset : public MathGridInset {
public:
	///
	MathMatrixInset(MathGridInset const &);
	///
	MathMatrixInset(string const & str);
	///
	MathInset * clone() const;
	///
	void metrics(MetricsInfo &) const {}
	/// identifies MatrixInsets
	MathMatrixInset const * asMatrixInset() const { return this; }

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void maxima(MaximaStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octave(OctaveStream &) const;
};

#endif
