// -*- C++ -*-
#ifndef MATH_BIGOPINSET_H
#define MATH_BIGOPINSET_H

// This inset is for things like "\sum" and "\int" that get displayed
// larger in displayed formulae and take limits

#include "math_diminset.h"

struct latexkeys;

/// big operators
class MathBigopInset : public MathDimInset {
public:
	///
	explicit MathBigopInset(latexkeys const *);
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
