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
#include "lyxlength.h"


namespace lyx {


/// The \kern primitive
/// Some hack for visual effects

class InsetMathKern : public InsetMath {
public:
	///
	InsetMathKern();
	///
	explicit InsetMathKern(LyXLength const & wid);
	///
	explicit InsetMathKern(docstring const & wid);
	///
	bool metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream & ns) const;
	///
	int width() const;
private:
	virtual std::auto_ptr<InsetBase> doClone() const;
	/// width in em
	LyXLength wid_;
	/// in pixels
	mutable int wid_pix_;

};


} // namespace lyx
#endif
