// -*- C++ -*-
/**
 * \file MathMacroArgument.h
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


/// A macro argument.
class MathMacroArgument : public InsetMath {
public:
	///
	explicit MathMacroArgument(int number);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	int number() const { return number_; }
	///
	void setNumber(int n);
	///
	InsetCode lyxCode() const { return MATHMACROARG_CODE; }

	///
	void normalize(NormalStream &) const;
	///
	void write(WriteStream & os) const;

private:
	Inset * clone() const;
	/// A number between 1 and 9
	int number_;
	///
	docstring str_;
};


} // namespace lyx

#endif
