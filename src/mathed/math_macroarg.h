// -*- C++ -*-
/**
 * \file math_macroarg.h
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

#include "math_diminset.h"


/// A macro argument.
class MathMacroArgument : public MathDimInset {
public:
	///
	explicit MathMacroArgument(std::size_t);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	std::size_t number() const { return number_; }
	///
	InsetBase::Code lyxCode() const { return MATHMACROARG_CODE; }

	///
	void normalize(NormalStream &) const;
	///
	void write(WriteStream & os) const;

private:
	virtual std::auto_ptr<InsetBase> doClone() const;
	/// A number between 1 and 9
	std::size_t number_;
	///
	char str_[3];
};

#endif
