// -*- C++ -*-
#ifndef MATH_SPACEINSET_H
#define MATH_SPACEINSET_H

#include "math_inset.h"


/// Smart spaces
class MathSpaceInset : public MathInset {
public:
	///
	explicit MathSpaceInset(int sp);
	///
	explicit MathSpaceInset(string const & name);
	///
	MathInset * clone() const;
	///
	MathSpaceInset const * asSpaceInset() const { return this; }
	///
	MathSpaceInset * asSpaceInset() { return this; }
	///
	void incSpace();
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;

	///
	void normalize(NormalStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void write(WriteStream & os) const;
private:
	///
	int space_;
};
#endif
