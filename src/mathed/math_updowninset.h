// -*- C++ -*-
#ifndef MATH_UPDOWNINSET_H
#define MATH_UPDOWNINSET_H

#include "math_inset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Abstract base class for super- and subscripts and mathop inset
    \author André Pönitz
 */

class MathUpDownInset : public MathInset {
public:
	///
	MathUpDownInset();
	///
	MathUpDownInset(bool up, bool down);
	///
	MathInset * clone() const;
	///
	void Write(std::ostream &, bool fragile) const;
	///
	void Metrics(MathStyles st, int asc = 0, int des = 0);
	///
	void draw(Painter &, int x, int baseline);
	///
	bool idxUp(int & idx, int & pos) const;
	///
	bool idxDown(int & idx, int & pos) const;
	///
	bool idxLeft(int & idx, int & pos) const;
	///
	bool idxRight(int & idx, int & pos) const;
	///
	bool idxFirst(int & idx, int & pos) const;
	///
	bool idxFirstUp(int & idx, int & pos) const;
	///
	bool idxFirstDown(int & idx, int & pos) const;
	///
	bool idxLast(int & idx, int & pos) const;
	///
	bool idxLastUp(int & idx, int & pos) const;
	///
	bool idxLastDown(int & idx, int & pos) const;
	///
	bool up() const;
	///
	bool down() const;
	///
	void up(bool);
	///
	void down(bool);
	///
	bool isActive() const { return false; }
	/// Identifies ScriptInsets
	bool isUpDownInset() const { return true; }
	///
	void idxDelete(int & idx, bool & popit, bool & deleteit);
private:
	///
	bool up_;
	///
	bool down_;
protected:
	///
	int dy0_;
	///
	int dy1_;
};

#endif
