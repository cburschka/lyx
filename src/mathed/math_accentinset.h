// -*- C++ -*-
#ifndef MATH_ACCENTINSET_H
#define MATH_ACCENTINSET_H

#include "math_inset.h"
#include "mathed/support.h"
#include "math_defs.h"

/// Accents
class MathAccentInset : public MathInset {
public:
	///
	explicit MathAccentInset(int);
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int, int);
	///
	void Write(std::ostream &, bool fragile) const;
	///
	void WriteNormal(std::ostream &) const;
	///
	void Metrics(MathStyles st, int asc = 0, int des = 0);
	///
	int getAccentCode() const;
	///
	bool isAccentInset() const { return true; }
private:
	///
	int code;
	///
	int dh;
	///
	int dy;
};

#endif
