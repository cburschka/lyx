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
	void Write(std::ostream &, bool fragile) const;
	///
	void WriteNormal(std::ostream &) const;
	///
	void Metrics(MathStyles st);
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
