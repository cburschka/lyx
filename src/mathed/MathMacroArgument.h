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


namespace lyx {


/// A macro argument.
class MathMacroArgument : public InsetMath {
public:
	///
	explicit MathMacroArgument(std::size_t);
	///
	bool metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	std::size_t number() const { return number_; }
	///
	Inset::Code lyxCode() const { return MATHMACROARG_CODE; }

	///
	void normalize(NormalStream &) const;
	///
	void write(WriteStream & os) const;

private:
	virtual Inset * clone() const;
	/// A number between 1 and 9
	std::size_t number_;
	///
	docstring str_;
};


} // namespace lyx

#endif
