// -*- C++ -*-
/**
 * \file math_ertinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_ERTINSET_H
#define MATH_ERTINSET_H


#include "math_textinset.h"

// implements support for \parbox

class MathErtInset : public MathTextInset {
public:
	///
	MathErtInset() {}
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	mode_type currentMode() const { return TEXT_MODE; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void infoize(std::ostream & os) const;
	///
	void write(WriteStream & os) const;
};

#endif
