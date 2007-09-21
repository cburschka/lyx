// -*- C++ -*-
/**
 * \file InsetMathFrac.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_DFRACINSET_H
#define MATH_DFRACINSET_H

#include "InsetMathFrac.h"


namespace lyx {


/// \dfrac support
class InsetMathDFrac : public InsetMathFrac {
public:
	///
	InsetMathDFrac();
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	docstring name() const;
	///
	void mathmlize(MathStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
private:
	virtual Inset * clone() const;
};


} // namespace lyx

#endif
