// -*- C++ -*-
#ifndef MATH_SYMBOLINSET_H
#define MATH_SYMBOLINSET_H

#include "math_diminset.h"

struct latexkeys;

// "normal" symbols that don't take limits and don't grow in displayed
// formulae

class MathSymbolInset : public MathDimInset {
public:
	///
	explicit MathSymbolInset(latexkeys const *);
	///
	explicit MathSymbolInset(char const *);
	///
	MathInset * clone() const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	bool isRelOp() const;
	///
	bool isScriptable() const;
	/// identifies things that can get \limits or \nolimits
	bool takesLimits() const;

	///
	void maplize(MapleStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octavize(OctaveStream &) const;
private:
	///
	MathTextCodes code() const;
	///
	MathTextCodes code2() const;

	///
	latexkeys const * sym_;
	///
	mutable int h_;
	///
	mutable MathMetricsInfo mi_;
};
#endif
