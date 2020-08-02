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
	docstring name() const;
	///
	MathClass mathClass() const { return math_class_; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void infoize(odocstream & os) const;
	///
	InsetCode lyxCode() const { return MATH_CLASS_CODE; }
	///
	InsetMathClass * asClassInset() { return this; }
	///
	InsetMathClass const * asClassInset() const { return this; }

private:
	virtual Inset * clone() const;
	///
	MathClass math_class_;
};


} // namespace lyx
#endif
