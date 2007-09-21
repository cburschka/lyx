// -*- C++ -*-
/**
 * \file InsetMathFBox.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_FBOXINSET_H
#define MATH_FBOXINSET_H

#include "InsetMathNest.h"


namespace lyx {


/// Non-AMS-style frame
class InsetMathFBox : public InsetMathNest {
public:
	///
	InsetMathFBox();
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
	///
	void infoize(odocstream & os) const;
private:
	virtual Inset * clone() const;
};



} // namespace lyx
#endif
