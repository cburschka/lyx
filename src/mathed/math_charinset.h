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
	explicit MathCharInset(char c);
	///
	MathCharInset(char c, MathTextCodes t);
	///
	MathInset * clone() const;
	///
	MathTextCodes nativeCode(char c) const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void writeHeader(std::ostream &) const;
	///
	void writeTrailer(std::ostream &) const;
	///
	void writeRaw(std::ostream &) const;
	///
	void normalize(NormalStream &) const;
	/// 
	int ascent() const;
	///
	int descent() const;
	///
	int width() const;
	/// identifies Charinsets
	MathCharInset const * asCharInset() const { return this; }
	///
	char getChar() const { return char_; }
	///
	MathTextCodes code() const { return code_; }
	///
	bool isRelOp() const;
	///
	void handleFont(MathTextCodes t);

private:
	/// the character
	char char_;
	/// the font to be used on screen
	MathTextCodes code_;
	///
	mutable MathMetricsInfo mi_;
};
#endif
