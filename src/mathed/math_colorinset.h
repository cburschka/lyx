// -*- C++ -*-
/**
 * \file math_colorinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_COLORINSET_H
#define MATH_COLORINSET_H

#include "math_nestinset.h"

/// Change colours.

class MathColorInset : public MathNestInset {
public:
	///
	explicit MathColorInset(bool oldstyle);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	/// we write extra braces in any case...
	bool extraBraces() const { return true; }
	///
	void draw(PainterInfo & pi, int x, int y) const;
	/// we need package color
	void validate(LaTeXFeatures & features) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
	///
	void infoize(std::ostream & os) const;
private:
	virtual std::auto_ptr<InsetBase> doClone() const;
	/// width of '[' in current font
	mutable int w_;
	///
	bool oldstyle_;
};

#endif
