// -*- C++ -*-
/**
 * \file math_fontinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_FONTINSET_H
#define MATH_FONTINSET_H

#include "math_nestinset.h"


class latexkeys;

/// Inset for font changes
class MathFontInset : public MathNestInset {
public:
	///
	explicit MathFontInset(latexkeys const * key);
	///
	MathFontInset * asFontInset() { return this; }
	///
	MathFontInset const * asFontInset() const { return this; }
	/// are we in math mode, text mode, or unsure?
	mode_type currentMode() const;
	///
	std::string name() const;
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
	virtual std::auto_ptr<InsetBase> doClone() const;
	/// the font to be used on screen
	latexkeys const * key_;
};
#endif
