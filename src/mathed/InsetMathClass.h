// -*- C++ -*-
/**
 * \file InsetMathClass.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_CLASSINSET_H
#define MATH_CLASSINSET_H

#include "MathClass.h"

#include "InsetMathNest.h"


namespace lyx {


/// Support for LaTeX's \\mathxxx class-changing commands

class InsetMathClass : public InsetMathNest {
public:
	///
	InsetMathClass(Buffer * buf, MathClass);
	///
	docstring name() const override;
	///
	MathClass mathClass() const override { return math_class_; }
	/// The default limits value in \c display style
	Limits defaultLimits(bool display) const override;
	/// whether the inset has limit-like sub/superscript
	Limits limits() const override { return limits_; }
	/// sets types of sub/superscripts
	void limits(Limits lim) override { limits_ = lim; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void write(WriteStream & os) const override;
	///
	void infoize(odocstream & os) const override;
	///
	InsetCode lyxCode() const override { return MATH_CLASS_CODE; }
	///
	InsetMathClass * asClassInset() { return this; }
	///
	InsetMathClass const * asClassInset() const override { return this; }

private:
	Inset * clone() const override;
	///
	MathClass math_class_;
	///
	Limits limits_ = AUTO_LIMITS;
};


} // namespace lyx
#endif
