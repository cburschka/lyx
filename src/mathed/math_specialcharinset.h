// -*- C++ -*-
#ifndef MATH_SPECIALCHARINSET_H
#define MATH_SPECIALCHARINSET_H

#include "math_diminset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** An inset for characters like {, #, and $ that need to be escaped
    when written out, but can be inserted by a single keystroke
    \author André Pönitz
 */

class MathSpecialCharInset : public MathDimInset {
public:
	///
	explicit MathSpecialCharInset(char c);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & st) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;

private:
	/// the character
	char char_;
};
#endif
