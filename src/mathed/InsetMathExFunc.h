// -*- C++ -*-
/**
 * \file InsetMathExFunc.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_EXFUNCINSET_H
#define MATH_EXFUNCINSET_H


#include "InsetMathNest.h"


namespace lyx {

// f(x) in one block (as opposed to 'f','(','x',')' or 'f','x')
// for interfacing external programs

class InsetMathExFunc : public InsetMathNest {
public:
	///
	InsetMathExFunc(Buffer * buf, docstring const & name);
	///
	InsetMathExFunc(Buffer * buf, docstring const & name, MathData const & ar);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	docstring name() const override;

	///
	void maple(MapleStream &) const override;
	///
	void maxima(MaximaStream &) const override;
	///
	void mathematica(MathematicaStream &) const override;
	///
	void mathmlize(MathStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void octave(OctaveStream &) const override;
	///
	InsetCode lyxCode() const override { return MATH_EXFUNC_CODE; }

private:
	Inset * clone() const override;
	///
	docstring const name_;
};

} // namespace lyx

#endif
