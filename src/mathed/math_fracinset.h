// -*- C++ -*-
#ifndef MATH_FRACINSET_H
#define MATH_FRACINSET_H

#include "math_inset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Fraction like objects (frac, stackrel, binom)
    \author Alejandro Aguilar Sierra
 */
class MathFracInset : public MathInset {
public:
	///
	explicit MathFracInset(short ot = LM_OT_FRAC);
	///
	virtual MathInset * Clone() const;
	///
	virtual void Write(std::ostream &, bool fragile) const;
	///
	virtual void WriteNormal(std::ostream &) const;
	///
	virtual void Metrics(MathStyles st);
	///
	virtual void draw(Painter &, int x, int baseline);
	///
	virtual bool idxUp(int &, int &) const;
	///
	virtual bool idxDown(int &, int &) const;
	///
	virtual bool idxLeft(int &, int &) const;
	///
	virtual bool idxRight(int &, int &) const;
};

#endif
