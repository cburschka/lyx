// -*- C++ -*-
#ifndef MATH_FONTINSET_H
#define MATH_FONTINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

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
	MathInset * clone() const;
	/// are we in math mode, text mode, or unsure?
	mode_type currentMode() const;
	///
	string name() const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi) const;
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
