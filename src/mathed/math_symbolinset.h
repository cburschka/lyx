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
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	bool isRelOp() const;
	/// do we take scripts?
	bool isScriptable() const;
	/// do we take \limits or \nolimits?
	bool takesLimits() const;
	/// identifies SymbolInset as such
	MathSymbolInset const * asSymbolInset() const { return this; }
	/// the LaTeX name of the symbol (without the backslash)
	string name() const;
	///
	bool match(MathInset *) const;

	///
	void normalize(NormalStream &) const;
	///
	void maplize(MapleStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octavize(OctaveStream &) const;
	///
	void write(WriteStream & os) const;

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
