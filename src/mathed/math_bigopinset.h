// -*- C++ -*-
#ifndef MATH_BIGOPINSET_H
#define MATH_BIGOPINSET_H

#include "math_inset.h"

/// big operators
class MathBigopInset : public MathInset {
public:
	///
	MathBigopInset(string const &, int);
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
	int sym_;
	///
	string ssym_;
	///
	MathTextCodes code_;
};
#endif
