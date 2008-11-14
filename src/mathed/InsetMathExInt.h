// -*- C++ -*-
/**
 * \file InsetMathExInt.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_EXINTINSET_H
#define MATH_EXINTINSET_H


// \int_l^u f(x) dx in one block (as opposed to 'f','(','x',')' or 'f','x')
// or \sum, \prod...  for interfacing external programs

#include "InsetMathNest.h"


namespace lyx {

// cell(0) is stuff before the 'd', cell(1) the stuff after
class InsetMathExInt : public InsetMathNest {
public:
	///
	explicit InsetMathExInt(docstring const & name_);
	///
	void symbol(docstring const &);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;

	///
	void normalize(NormalStream &) const;
	///
	void maxima(MaximaStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathStream &) const;
	///
	void write(WriteStream & os) const;
private:
	virtual Inset * clone() const;
	///
	bool hasScripts() const;

	///
	docstring symbol_;
};


} // namespace lyx


#endif
