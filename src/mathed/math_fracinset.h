// -*- C++ -*-
#ifndef MATH_FRACINSET_H
#define MATH_FRACINSET_H

#include "math_parinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Fraction like objects (frac, stackrel, binom)
    \author Alejandro Aguilar Sierra
 */
class MathFracInset : public MathParInset {
public:
	///
	MathFracInset(short ot = LM_OT_FRAC);
	///
	MathedInset * Clone();
	///
	void draw(Painter &, int x, int baseline);
	///
	void Write(std::ostream &, bool fragile);
	///
	void WriteNormal(std::ostream &);
	///
	void Metrics();
	
	/** This does the same that SetData(MathedArray const &) but for both
	    numerator and denominator at once.
	*/
	void SetData(MathedArray const &, MathedArray const &);
	///
	void setData(MathedArray const &);
	///
	void GetXY(int & x, int & y) const;
	///
	void SetFocus(int, int);
	///
	bool Inside(int, int);
	///
	MathedArray & GetData();
	///
	MathedArray const & GetData() const;
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
	MathParInset den_;
	///
	int w0_;
	///
	int w1_;
	///
	int des0_;
	///
	int dh_;
};

#endif
