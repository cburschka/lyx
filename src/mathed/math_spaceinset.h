// -*- C++ -*-
#ifndef MATH_SPACEINSET_H
#define MATH_SPACEINSET_H

#include "math_diminset.h"
#include "math_defs.h"

#ifdef __GNUG__
#pragma interface
#endif

/// Smart spaces
class MathSpaceInset : public MathDimInset {
public:
	///
	explicit MathSpaceInset(int sp);
	///
	MathInset * clone() const;
	///
	MathSpaceInset const * asSpaceInset() const { return this; }
	///
	MathSpaceInset * asSpaceInset() { return this; }
	///
	void incSpace();
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;

	///
	void normalize(NormalStream &) const;
	///
	void maplize(MapleStream &) const;
	///
	void octavize(OctaveStream &) const;
	///
	void write(WriteStream & os) const;
private:
	///
	int space_;
};
#endif
