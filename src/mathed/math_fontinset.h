// -*- C++ -*-
#ifndef MATH_FONTINSET_H
#define MATH_FONTINSET_H

#include "math_nestinset.h"


/** Inset for font changes
 *  \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
 */

class latexkeys;

class MathFontInset : public MathNestInset {
public:
	///
	explicit MathFontInset(latexkeys const * key);
	///
	InsetBase * clone() const;
	///
	MathFontInset * asFontInset() { return this; }
	///
	MathFontInset const * asFontInset() const { return this; }
	/// are we in math mode, text mode, or unsure?
	mode_type currentMode() const;
	///
	string name() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const;
	///
	void drawT(TextPainter & pi, int x, int y) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	void infoize(std::ostream & os) const;

private:
	/// the font to be used on screen
	latexkeys const * key_;
};
#endif
