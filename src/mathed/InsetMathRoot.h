// -*- C++ -*-
/**
 * \file InsetMathRoot.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_ROOT_H
#define MATH_ROOT_H

#include "InsetMathNest.h"


namespace lyx {


/// The general n-th root inset.
class InsetMathRoot : public InsetMathNest {
public:
	///
	explicit InsetMathRoot(Buffer * buf);
	///
	bool idxUpDown(Cursor & cur, bool up) const override;
	///
	bool idxForward(Cursor & cur) const override;
	///
	bool idxBackward(Cursor & cur) const override;
	///
	bool idxFirst(Cursor &) const override;
	///
	bool idxLast(Cursor &) const override;

	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;

	///
	void write(TeXMathStream & os) const override;
	///
	void normalize(NormalStream &) const override;
	///
	void mathmlize(MathMLStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void maple(MapleStream &) const override;
	///
	void mathematica(MathematicaStream &) const override;
	///
	void octave(OctaveStream &) const override;
	///
	InsetCode lyxCode() const override { return MATH_ROOT_CODE; }
	///
	void validate(LaTeXFeatures &) const override;

private:
	Inset * clone() const override;
};

void mathed_root_metrics(MetricsInfo & mi, MathData const & nucleus,
                         MathData const * root, Dimension & dim);

void mathed_draw_root(PainterInfo & pi, int x, int y, MathData const & nucleus,
                      MathData const * root, Dimension const & dim);


} // namespace lyx
#endif
