// -*- C++ -*-
#ifndef MATH_BIGOPINSET_H
#define MATH_BIGOPINSET_H

#include "math_scriptinset.h"

/// big operators
class MathBigopInset : public MathScriptInset {
public:
	///
	MathBigopInset(string const &, int);
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int, int);
	///
	void Write(std::ostream &, bool fragile) const;
	///
	void WriteNormal(std::ostream &) const;
	///
	void Metrics(MathStyles st);
	///
	int limits() const;
	///
	bool hasLimits() const;
	///
	void limits(int);
	///
	bool idxDelete(int idx);
private:
	/// 1: \limits, -1: \nolimits, 0: use default
	int lims_;
	///
	int sym_;
};
#endif
