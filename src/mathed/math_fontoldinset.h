// -*- C++ -*-
#ifndef MATH_FONTOLDINSET_H
#define MATH_FONTOLDINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Old-style font changes
    \author André Pönitz
 */

class latexkeys;

class MathFontOldInset : public MathNestInset {
public:
	///
	explicit MathFontOldInset(latexkeys const * key);
	///
	MathInset * clone() const;
	/// we are in text mode.
	mode_type currentMode() const { return TEXT_MODE; }
	/// we write extra braces in any case...
	bool extraBraces() const { return true; }
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi) const;
	///
	void drawT(TextPainter & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void infoize(std::ostream & os) const;

private:
	/// the font to be used on screen
	latexkeys const * key_;
};
#endif
