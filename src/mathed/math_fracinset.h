// -*- C++ -*-
#ifndef MATH_FRACINSET_H
#define MATH_FRACINSET_H

#include "math_parinset.h"

/** Fraction like objects (frac, stackrel, binom)
    \author Alejandro Aguilar Sierra
 */
class MathFracInset: public MathParInset {
public:
	///
	MathFracInset(short ot = LM_OT_FRAC);
	///
	~MathFracInset();
	///
	MathedInset * Clone();
	///
	void draw(Painter &, int x, int baseline);
	///
	void Write(std::ostream &, bool fragile);
	///
	void Metrics();
	
	/** This does the same that SetData(MathedArray*) but for both
	    numerator and denominator at once.
	*/
	void SetData(MathedArray *, MathedArray *);
	///
	void setData(MathedArray *);
	///
	void GetXY(int & x, int & y) const;
	///
	void SetFocus(int, int);
	///
	bool Inside(int, int);
	///
	MathedArray * GetData();
	///
	bool setArgumentIdx(int i); // was bool Up/down(void);
	///
	int getArgumentIdx() const;
	///
	int getMaxArgumentIdx() const;
	///
	void  SetStyle(short);
private:
	///
	int idx_;
	///
	MathParInset * den_;
	///
	int w0_;
	///
	int w1_;
	///
	int des0_;
	///
	int dh_;
};


inline
int MathFracInset::getArgumentIdx() const
{
  return idx_;
}


inline
int MathFracInset::getMaxArgumentIdx() const
{
  return 1;
}
#endif
