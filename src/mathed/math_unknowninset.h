// -*- C++ -*-
#ifndef MATH_UNKNOWNINSET_H
#define MATH_UNKNOWNINSET_H

#include "math_diminset.h"
#include "math_defs.h"

#ifdef __GNUG__
#pragma interface
#endif

/**
 Unknowntions or LaTeX names for objects that we really don't know
 */
class MathUnknownInset : public MathDimInset {
public:
	///
	explicit MathUnknownInset(string const & nm);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	string const & name() const;
	/// identifies UnknownInsets
	MathUnknownInset const * asUnknownInset() const { return this; }
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
	mutable MathMetricsInfo mi_;
};
#endif
