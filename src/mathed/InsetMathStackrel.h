// -*- C++ -*-
/**
 * \file InsetMathStackrel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_STACKRELINSET_H
#define MATH_STACKRELINSET_H

#include "InsetMathFrac.h"


namespace lyx {

class InsetMathStackrel : public InsetMathFracBase {
public:
	///
	InsetMathStackrel(Buffer * buf, bool sub);
	///
	bool idxUpDown(Cursor &, bool up) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void mathmlize(MathStream &) const;
	///
	void htmlize(HtmlStream &) const;
	/// 
	void validate(LaTeXFeatures &) const;
	///
	InsetCode lyxCode() const { return MATH_STACKREL_CODE; }

private:
	virtual Inset * clone() const;
};

} // namespace lyx
#endif
