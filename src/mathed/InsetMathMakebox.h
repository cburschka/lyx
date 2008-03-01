// -*- C++ -*-
/**
 * \file InsetMathMakebox.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Ling Li
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_MAKEBOXINSET_H
#define MATH_MAKEBOXINSET_H

#include "InsetMathNest.h"


namespace lyx {

/// Extra nesting: \\makebox.
// consolidate with InsetMathFrameBox?

class InsetMathMakebox : public InsetMathNest {
public:
	///
	InsetMathMakebox();
	///
	bool metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
	///
	mode_type currentMode() const { return TEXT_MODE; }
	///
	void infoize(odocstream & os) const;
private:
	virtual std::auto_ptr<Inset> doClone() const;
};



} // namespace lyx
#endif
