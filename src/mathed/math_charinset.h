// -*- C++ -*-
#ifndef MATH_CHARINSET_H
#define MATH_CHARINSET_H

#include "math_inset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** The base character inset.
    \author André Pönitz
 */

class MathCharInset : public MathInset {
public:
	///
	MathCharInset(char c, MathTextCodes t);
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int x, int baseline);
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;
	/// 
	int ascent() const;
	///
	int descent() const;
	///
	int width() const;
	/// identifies Charinsets
	bool isCharInset() const { return true; }
	///
	char getChar() const { return char_; }

private:
	/// the character
	char char_;
};
#endif
