// -*- C++ -*-
#ifndef MATH_BIGOPINSET_H
#define MATH_BIGOPINSET_H

#include "math_inset.h"

/// big operators
class MathBigopInset: public MathedInset {
public:
	///
	MathBigopInset(string const &, int, short st = LM_ST_TEXT);
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
	///
	inline void SetLimits(bool);
protected:
	///
	int lims;
	///
	int sym;   
};
#endif
