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

#include "math_nestinset.h"


/// A macro argument.
class MathMacroArgument : public MathNestInset {
public:
	///
	explicit MathMacroArgument(int);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	bool isActive() const { return false; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	void substitute(MathMacro const & macro);

	///
	void normalize(NormalStream &) const;
	///
	void write(WriteStream & os) const;

private:
	/// A number between 1 and 9
	int number_;
	///
	char str_[3];
	///
	bool expanded_;
};

#endif
