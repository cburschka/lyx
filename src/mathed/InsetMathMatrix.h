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
	InsetMathMatrix const * asMatrixInset() const override { return this; }

	///
	void write(WriteStream & os) const override;
	///
	void normalize(NormalStream &) const override;
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
	InsetCode lyxCode() const override { return MATH_MATRIX_CODE; }
	///
	bool handlesMulticolumn() const override { return true; }

private:
	Inset * clone() const override;
	///
	docstring left_;
	///
	docstring right_;
};



} // namespace lyx
#endif
