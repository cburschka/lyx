// -*- C++ -*-
/**
 * \file InsetMathSubstack.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_SUBSTACK_H
#define MATH_SUBSTACK_H

#include "InsetMathGrid.h"


namespace lyx {


/// support for AMS's \\substack

class InsetMathSubstack : public InsetMathGrid {
public:
	///
	InsetMathSubstack();
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	InsetMathSubstack const * asSubstackInset() const { return this; }

	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const;
	///
	void infoize(odocstream & os) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
private:
	virtual Inset * clone() const;
};



} // namespace lyx
#endif
