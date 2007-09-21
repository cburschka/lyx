// -*- C++ -*-
/**
 * \file InsetMathBinom.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_BINOMINSET_H
#define MATH_BINOMINSET_H

#include "InsetMathFracBase.h"


namespace lyx {


/// Binom like objects
class InsetMathBinom : public InsetMathFracBase {
public:
	///
	explicit InsetMathBinom(bool choose = false);
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	/// draw decorations.
	void drawDecoration(PainterInfo & pi, int x, int y) const
	{ drawMarkers2(pi, x, y); }
	///
	bool extraBraces() const;
private:
	virtual Inset * clone() const;
	///
	int dw() const;
	///
	bool choose_;
};


} // namespace lyx

#endif
