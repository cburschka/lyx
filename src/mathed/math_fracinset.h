#ifndef MATH_FRACINSET_H
#define MATH_FRACINSET_H

#include "math_parinset.h"

/// Fraction like objects (frac, stackrel, binom) 
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
	void SetData(MathedArray *);
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
	int getArgumentIdx() const { return idx; }
	///
	int getMaxArgumentIdx() const { return 1; }
	///
	void  SetStyle(short);
protected:
	///
	int idx;
	///
	MathParInset * den;
	///
	int w0, w1, des0, dh;
};
#endif
