// -*- C++ -*-
/**
 * \file math_fboxinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_FBOXINSET_H
#define MATH_FBOXINSET_H

#include "math_nestinset.h"
#include "metricsinfo.h"


class latexkeys;

/// Extra nesting
class MathFboxInset : public MathNestInset {
public:
	///
	MathFboxInset(latexkeys const * key);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	mode_type currentMode() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
private:
	///
	latexkeys const * key_;
};

#endif
