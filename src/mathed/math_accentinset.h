// -*- C++ -*-
#ifndef MATH_ACCENTINSET_H
#define MATH_ACCENTINSET_H

#include "math_inset.h"
#include "mathed/support.h"
#include "math_defs.h"

/// Accents
class MathAccentInset : public MathedInset {
public:
	///
	MathAccentInset(byte, MathedTextCodes, int, short st = LM_ST_TEXT);
	///
	MathAccentInset(MathedInset *, int, short st = LM_ST_TEXT);
	///
	~MathAccentInset();
	///
	MathedInset * Clone();
	///
	void draw(Painter &, int, int);
	///
	void Write(std::ostream &, bool fragile);
	///
	void WriteNormal(std::ostream &);
	///
	void Metrics();
	///
	int getAccentCode() const;
protected:
	///
	byte c;
	///
	MathedTextCodes fn;
	///
	int code;
	///
	MathedInset * inset;
	///
	int dh, dy;
};


inline
int MathAccentInset::getAccentCode() const
{
	return code;
}
#endif
