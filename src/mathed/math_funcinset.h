// -*- C++ -*-
#ifndef MATH_FUNCINSET_H
#define MATH_FUNCINSET_H

#include "math_diminset.h"

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
	void metrics(MathMetricsInfo & st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	string const & name() const;
	/// identifies FuncInsets
	MathFuncInset * asFuncInset() { return this; }
	///
	void setName(string const &);
	///
	bool match(MathInset * p) const;

	///
	void normalize(NormalStream &) const;
	///
	void maplize(MapleStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octavize(OctaveStream &) const;
	///
	void write(WriteStream &) const;
private:
	///
	string name_;
	///
	mutable LyXFont font_;
};
#endif
