// -*- C++ -*-
#ifndef MATH_FUNCINSET_H
#define MATH_FUNCINSET_H

#include "math_diminset.h"
#include "math_defs.h"

#ifdef __GNUG__
#pragma interface
#endif

/**
 Functions or LaTeX names for objects that I don't know how to draw.
 */
class MathFuncInset : public MathDimInset {
public:
	///
	explicit MathFuncInset(string const & nm);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(WriteStream &) const;
	///
	void normalize(NormalStream &) const;
	///
	string const & name() const;
	///
	void setName(string const &);
	///
	void maplize(MapleStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octavize(OctaveStream &) const;
private:
	///
	string name_;
	///
	mutable MathMetricsInfo mi_;
};
#endif
