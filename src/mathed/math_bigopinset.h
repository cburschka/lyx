// -*- C++ -*-
#ifndef MATH_BIGOPINSET_H
#define MATH_BIGOPINSET_H

#include "math_updowninset.h"

/// big operators
class MathBigopInset : public MathUpDownInset {
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
	void Metrics(MathStyles st, int asc = 0, int des = 0);
	///
	void draw(Painter &, int, int);
	///
	void limits(int);
	///
	int limits() const;
	/// Identifies BigopInsets
	bool isBigopInset() const { return true; }
private:
	///
	bool hasLimits() const;
	///
	int sym_;
	///
	string ssym_;
	///
	MathTextCodes code_;
	/// 1: \limits, -1: \nolimits, 0: use default
	int limits_;
	/// x offset for drawing the superscript
	int dx0_;
	/// x offset for drawing the subscript
	int dx1_;
	/// x offset for drawing the inner symbol
	int dxx_;
};
#endif
