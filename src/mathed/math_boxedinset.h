// -*- C++ -*-
/**
 * \file math_boxedinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_BOXEDINSET_H
#define MATH_BOXEDINSET_H

#include "math_nestinset.h"


/// Non-AMS-style frame
class MathBoxedInset : public MathNestInset {
public:
	///
	MathBoxedInset();
	///
	void validate(LaTeXFeatures & features) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
	///
	void infoize(std::ostream & os) const;
private:
	virtual std::auto_ptr<InsetBase> doClone() const;
};

#endif
