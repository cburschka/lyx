// -*- C++ -*-
#ifndef MATH_UNKNOWNINSET_H
#define MATH_UNKNOWNINSET_H

#include "math_diminset.h"


/**
 Unknowntions or LaTeX names for objects that we really don't know
 */
class MathUnknownInset : public MathDimInset {
public:
	///
	explicit MathUnknownInset(string const & name,
		bool final = true, bool black = false);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;
	///
	void setName(string const & name);
	///
	string name() const;
	/// identifies UnknownInsets
	MathUnknownInset const * asUnknownInset() const { return this; }
	/// identifies UnknownInsets
	MathUnknownInset * asUnknownInset() { return this; }
	///
	bool match(MathAtom const & at) const;

	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void finalize();
	///
	bool final() const;
private:
	///
	string name_;
	/// are we finished creating the name?
	bool final_;
	///
	bool black_;
};
#endif
