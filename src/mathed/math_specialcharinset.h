// -*- C++ -*-
#ifndef MATH_SPECIALCHARINSET_H
#define MATH_SPECIALCHARINSET_H

#include "math_inset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** An inset for characters like {, #, and $ that need to be escaped 
    when written out, but can be inserted by a single keystroke
    \author André Pönitz
 */

class MathSpecialCharInset : public MathInset {
public:
	///
	explicit MathSpecialCharInset(char c);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(MathWriteInfo & os) const;
	///
	void writeNormal(NormalStream &) const;
	/// 
	int ascent() const;
	///
	int descent() const;
	///
	int width() const;

private:
	/// the character
	char char_;
	///
	mutable MathMetricsInfo mi_;
};
#endif
