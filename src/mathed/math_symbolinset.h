// -*- C++ -*-
#ifndef MATH_SYMBOLINSET_H
#define MATH_SYMBOLINSET_H

#include "math_diminset.h"

struct latexkeys;

/// big operators
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
	void metrics(MathStyles st);
	///
	void draw(Painter &, int, int);
	///
	bool isScriptable() const { return true; }

private:
	///
	latexkeys const * sym_;
	///
	string ssym_;
	///
	MathTextCodes code_;
};
#endif
