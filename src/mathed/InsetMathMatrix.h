// -*- C++ -*-
/**
 * \file InsetMathMatrix.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_MATRIXINSET_H
#define MATH_MATRIXINSET_H

#include "InsetMathGrid.h"
#include "support/strfwd.h"


namespace lyx {


// "shortcut" for DelimInset("(",ArrayInset,")") used by MathExtern

class InsetMathMatrix : public InsetMathGrid {
public:
	///
	explicit InsetMathMatrix(InsetMathGrid const &, 
			docstring const & left, docstring const & right);
	/// identifies MatrixInsets
	InsetMathMatrix const * asMatrixInset() const { return this; }

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void maxima(MaximaStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathStream &) const;
	///
	void htmlize(HtmlStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	InsetCode lyxCode() const { return MATH_MATRIX_CODE; }

private:
	virtual Inset * clone() const;
	///
	docstring left_;
	///
	docstring right_;
};



} // namespace lyx
#endif
