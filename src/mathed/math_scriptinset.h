// -*- C++ -*-
#ifndef MATH_SCRIPTINSET_H
#define MATH_SCRIPTINSET_H

#include "math_inset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Inset for super- and subscripts
    \author André Pönitz
 */

class MathScriptInset : public MathInset {
public:
	///
	MathScriptInset();
	///
	MathScriptInset(bool up, bool down);
	///
	MathInset * Clone() const;
	///
	void Write(std::ostream &, bool fragile) const;
	///
	void WriteNormal(std::ostream &) const;
	///
	void Metrics(MathStyles st);
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
	bool idxLast(int & idx, int & pos) const;
	///
	bool up() const;
	///
	bool down() const;
	///
	void up(bool);
	///
	void down(bool);
private:
	///
	bool up_;
	///
	bool down_;
};

#endif
