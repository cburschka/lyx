// -*- C++ -*-
#ifndef MATH_SCRIPTINSET_H
#define MATH_SCRIPTINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Inset for super- and subscripts
    \author André Pönitz
 */

class MathScriptInset : public MathNestInset {
public:
	///
	explicit MathScriptInset(bool up);
	///
	MathInset * clone() const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void metrics(MathStyles st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	MathScriptInset const * asScriptInset() const;
	///
	bool up() const { return up_; }
	///
	bool down() const { return !up_; }
private:
	///
	bool up_;
};

#endif

