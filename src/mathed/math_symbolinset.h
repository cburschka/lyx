// -*- C++ -*-
#ifndef MATH_SYMBOLINSET_H
#define MATH_SYMBOLINSET_H

#include "math_diminset.h"
#include "LString.h"

struct latexkeys;

// "normal" symbols that don't take limits and don't grow in displayed
// formulae

class MathSymbolInset : public MathDimInset {
public:
	///
	explicit MathSymbolInset(latexkeys const *);
	///
	MathInset * clone() const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathStyles st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	bool isRelOp() const;

private:
	///
	latexkeys const * sym_;
	/// cache for the symbol's onscreen string representation
	mutable string ssym_;
};
#endif
