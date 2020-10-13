// -*- C++ -*-
/**
 * \file InsetMathComment.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_COMMENTINSET_H
#define MATH_COMMENTINSET_H

#include "InsetMathNest.h"


namespace lyx {

/// Inset for end-of-line comments
class InsetMathComment : public InsetMathNest {
public:
	///
	explicit InsetMathComment(Buffer * buf);
	///
	explicit InsetMathComment(MathData const & ar);
	///
	InsetMathComment(Buffer * buf, docstring const &);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const override;
	///
	void drawT(TextPainter & pi, int x, int y) const override;

	///
	void write(WriteStream & os) const override;
	///
	void maple(MapleStream &) const override;
	///
	void mathematica(MathematicaStream &) const override {}
	///
	void octave(OctaveStream &) const override {}
	///
	void mathmlize(MathStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void infoize(odocstream & os) const override;
	///
	InsetCode lyxCode() const override { return MATH_COMMENT_CODE; }
private:
	Inset * clone() const override;
};

} // namespace lyx

#endif
