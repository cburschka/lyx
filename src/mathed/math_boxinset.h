// -*- C++ -*-
/**
 * \file math_boxinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_BOXINSET_H
#define MATH_BOXINSET_H

#include "math_nestinset.h"
#include "LString.h"


class LyXFont;

/// Support for \\mbox

class MathBoxInset : public MathNestInset {
public:
	///
	explicit MathBoxInset(string const & name);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	mode_type currentMode() const { return TEXT_MODE; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream & ns) const;
	///
	void infoize(std::ostream & os) const;

private:
	///
	string name_;
};


#endif
