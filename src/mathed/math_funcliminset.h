// -*- C++ -*-
#ifndef MATH_FUNCLIMINSET_H
#define MATH_FUNCLIMINSET_H

#include "math_diminset.h"

struct latexkeys;

// "normal" symbols that don't take limits and don't grow in displayed
// formulae

class MathFuncLimInset : public MathDimInset {
public:
	///
	explicit MathFuncLimInset(latexkeys const *);
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
	bool isScriptable() const { return true; }

private:
	///
	latexkeys const * sym_;
};
#endif
