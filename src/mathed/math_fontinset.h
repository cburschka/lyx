// -*- C++ -*-
#ifndef MATH_FONTINSET_H
#define MATH_FONTINSET_H

#include "math_nestinset.h"
#include "LString.h"
#include "lyxfont.h"

#ifdef __GNUG__
#pragma interface
#endif

/** The base character inset.
    \author André Pönitz
 */

class MathFontInset : public MathNestInset {
public:
	///
	explicit MathFontInset(string const & name);
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
	void normalize(NormalStream &) const;
	/// identifies Fontinsets
	MathFontInset const * asFontInset() const { return this; }
	///
	void validate(LaTeXFeatures & features) const;
	///
	void infoize(std::ostream & os) const;

private:
	/// the font to be used on screen
	string name_;
};
#endif
