// -*- C++ -*-
#ifndef MATH_CHARINSET_H
#define MATH_CHARINSET_H

#include "math_diminset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** The base character inset.
    \author André Pönitz
 */

class MathCharInset : public MathDimInset {
public:
	///
	explicit MathCharInset(char c);
	///
	MathCharInset(char c, MathTextCodes t);
	///
	MathInset * clone() const;
	///
	static MathTextCodes nativeCode(char c);
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & st) const;
	///
	void drawT(TextPainter &, int x, int y) const;
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
	/// identifies Charinsets
	MathCharInset const * asCharInset() const { return this; }
	///
	char getChar() const { return char_; }
	///
	MathTextCodes code() const { return code_; }
	///
	bool isRelOp() const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	void handleFont(MathTextCodes t);
	///
	bool match(MathInset *) const;
	/// identifies complicated things that need braces if used as arg
	bool needsBraces() const { return false; }

private:
	/// the character
	char char_;
	/// the font to be used on screen
	MathTextCodes code_;
	///
	mutable LyXFont font_;
};
#endif
