// -*- C++ -*-
/**
 * \file math_fracinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_DFRACINSET_H
#define MATH_DFRACINSET_H

#include "math_fracinset.h"


/// \dfrac support
class MathDfracInset : public MathFracInset {
public:
	///
	MathDfracInset();
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	std::string name() const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
private:
	virtual std::auto_ptr<InsetBase> doClone() const;
};

#endif
