// -*- C++ -*-
/**
 * \file InsetMathFrameBox.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_FRAMEBOXINSET_H
#define MATH_FRAMEBOXINSET_H

#include "InsetMathNest.h"


namespace lyx {


/// Extra nesting
class InsetMathFrameBox : public InsetMathNest {
public:
	///
	InsetMathFrameBox();
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
private:
	virtual std::auto_ptr<Inset> doClone() const;
};



} // namespace lyx
#endif
