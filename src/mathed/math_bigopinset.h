// -*- C++ -*-
#ifndef MATH_BIGOPINSET_H
#define MATH_BIGOPINSET_H

#include "math_inset.h"
#include "math_defs.h"

/// big operators
class MathBigopInset : public MathedInset {
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
	void WriteNormal(std::ostream &);
	///
	void Metrics();
	///
	bool GetLimits() const;
	///
	void SetLimits(bool);
private:
	///
	int lims_;
	///
	int sym_;
};
#endif
