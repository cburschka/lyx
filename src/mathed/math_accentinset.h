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
	MathAccentInset(byte, MathTextCodes, int);
	///
	MathAccentInset(MathInset *, int);
	///
	~MathAccentInset();
	///
	MathInset *  clone() const;
	///
	void draw(Painter &, int, int);
	///
	void Write(std::ostream &, bool fragile) const;
	///
	void WriteNormal(std::ostream &) const;
	///
	void Metrics(MathStyles st);
	///
	int getAccentCode() const;
	///
	bool isAccentInset() const { return true; }
protected:
	///
	byte c;
	///
	MathTextCodes fn;
	///
	int code;
	///
	MathInset * inset;
	///
	int dh, dy;
};

#endif
