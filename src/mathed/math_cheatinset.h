// -*- C++ -*-
#ifndef MATH_CHEATINSET_H
#define MATH_CHEATINSET_H

#include "math_diminset.h"

#ifdef __GNUG__
#pragma interface
#endif

/// Some cheating for displaying things like \ll etc

class MathCheatInset : public MathDimInset {
public:
	///
	explicit MathCheatInset(double wid);
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathStyles st) const;
private:
	/// width in em
	double wid_;
};
#endif
