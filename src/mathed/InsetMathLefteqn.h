// -*- C++ -*-
/**
 * \file InsetMathLefteqn.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_LEFTEQNINSET_H
#define MATH_LEFTEQNINSET_H

#include "InsetMathNest.h"


namespace lyx {


/// Support for LaTeX's \\lefteqn command

class InsetMathLefteqn : public InsetMathNest {
public:
	///
	explicit InsetMathLefteqn(Buffer * buf);
	///
	docstring name() const override;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void infoize(odocstream & os) const override;
	///
	InsetCode lyxCode() const override { return MATH_LEFTEQN_CODE; }

private:
	Inset * clone() const override;
};


} // namespace lyx
#endif
