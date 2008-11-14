// -*- C++ -*-
/**
 * \file InsetMathSpace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_SPACEINSET_H
#define MATH_SPACEINSET_H

#include "InsetMath.h"


namespace lyx {


/// Smart spaces
class InsetMathSpace : public InsetMath {
public:
	///
	explicit InsetMathSpace(int sp);
	///
	explicit InsetMathSpace(docstring const & name);
	///
	InsetMathSpace const * asSpaceInset() const { return this; }
	///
	InsetMathSpace * asSpaceInset() { return this; }
	///
	void incSpace();
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;

	///
	void normalize(NormalStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void write(WriteStream & os) const;
private:
	virtual Inset * clone() const;
	///
	int space_;
	///
	Dimension dim_;
};


} // namespace lyx
#endif
