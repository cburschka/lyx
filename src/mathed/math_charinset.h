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
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & st) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & st) const;
	///
	void drawT(TextPainter &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream & ns) const;
	///
	void octavize(OctaveStream & os) const;
	/// identifies Charinsets
	MathCharInset const * asCharInset() const { return this; }
	///
	char getChar() const { return char_; }
	///
	bool isRelOp() const;
	///
	bool match(MathInset const *) const;

private:
	/// the character
	char char_;
};
#endif
