#ifndef MATH_DECORATIONINSET_H
#define MATH_DECORATIONINSET_H

#include "math_parinset.h"

/// Decorations over (below) a math object
class MathDecorationInset: public MathParInset {
public:
	///
	MathDecorationInset(int, short st = LM_ST_TEXT);
	///
	MathedInset * Clone();
	///
	void draw(Painter &, int, int);
	///
	void Write(std::ostream &, bool fragile);
	///
	void Metrics();
	///
	inline bool GetLimits() const;
protected:
	///
	int deco;
	///
	bool upper;
	///
	int dw, dh, dy;
};
#endif
