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

#include <string>


namespace lyx {

// f(x) in one block (as opposed to 'f','(','x',')' or 'f','x')
// for interfacing external programs

class InsetMathExFunc : public InsetMathNest {
public:
	///
	explicit InsetMathExFunc(docstring const & name);
	///
	InsetMathExFunc(docstring const & name, MathData const & ar);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	docstring name() const;

	///
	void maple(MapleStream &) const;
	///
	void maxima(MaximaStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathStream &) const;
	///
	void octave(OctaveStream &) const;

private:
	virtual Inset * clone() const;
	///
	docstring const name_;
};

} // namespace lyx

#endif
