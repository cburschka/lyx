// -*- C++ -*-
#ifndef MATH_BIGOPINSET_H
#define MATH_BIGOPINSET_H

#include "math_scriptinset.h"

/// big operators
class MathBigopInset : public MathInset {
public:
	///
	MathBigopInset(string const &, int);
	///
	MathInset * Clone() const;
	///
	void draw(Painter &, int, int);
	///
	void Write(std::ostream &, bool fragile) const;
	///
	void WriteNormal(std::ostream &) const;
	///
	void Metrics(MathStyles st);
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
