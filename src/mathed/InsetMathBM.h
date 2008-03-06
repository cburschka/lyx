// -*- C++ -*-
/**
 * \file InsetMathBM.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bernhard Roider
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_BMINSET_H
#define MATH_BMINSET_H

#include "InsetMathNest.h"


namespace lyx {


/// Inset for \bm
class InsetMathBM : public InsetMathNest {
public:
	///
	InsetMathBM();
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
	void write(WriteStream & os) const;
	///
	void infoize(odocstream & os) const;
private:
	virtual Inset * clone() const;
};


} // namespace lyx

#endif
