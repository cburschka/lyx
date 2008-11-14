// -*- C++ -*-
/**
 * \file InsetMathKern.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_CHEATINSET_H
#define MATH_CHEATINSET_H

#include "InsetMath.h"
#include "Length.h"


namespace lyx {


/// The \kern primitive
/// Some hack for visual effects

class InsetMathKern : public InsetMath {
public:
	///
	InsetMathKern();
	///
	explicit InsetMathKern(Length const & wid);
	///
	explicit InsetMathKern(docstring const & wid);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream & ns) const;
private:
	virtual Inset * clone() const;
	/// width in em
	Length wid_;
};


} // namespace lyx
#endif
