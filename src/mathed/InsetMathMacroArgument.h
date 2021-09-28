// -*- C++ -*-
/**
 * \file InsetMathMacroArgument.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATHMACROARGUMENT_H
#define MATHMACROARGUMENT_H

#include "InsetMath.h"

#include "support/docstring.h"


namespace lyx {


// A # that failed to parse
class InsetMathHash : public InsetMath {
public:
	explicit InsetMathHash(docstring const & str = docstring()) : str_('#' + str) {}
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo &, int x, int y) const override;
	///
	void normalize(NormalStream &) const override;
	///
	void write(TeXMathStream & os) const override;

private:
	Inset * clone() const override;

protected:
	///
	docstring str_;
};


/// A macro argument.
class InsetMathMacroArgument : public InsetMathHash {
public:
	/// Assumes 0 < number <= 9
	explicit InsetMathMacroArgument(int number);
	///
	int number() const { return number_; }
	/// Assumes 0 < n <= 9
	void setNumber(int n);
	///
	InsetCode lyxCode() const override { return MATH_MACROARG_CODE; }

	///
	void normalize(NormalStream &) const override;

private:
	Inset * clone() const override;
	/// A number between 1 and 9
	int number_;
};


} // namespace lyx

#endif
