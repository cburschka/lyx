// -*- C++ -*-
/**
 * \file math_xarrowinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_XARROWINSET_H
#define MATH_XARROWINSET_H

#include "math_fracbase.h"
#include "support/std_string.h"


/// Wide arrows like \xrightarrow
class MathXArrowInset : public MathFracbaseInset {
public:
	///
	explicit MathXArrowInset(string const & name);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void normalize(NormalStream & os) const;

private:
	///
	bool upper() const;
	///
	string const name_;
};
#endif
