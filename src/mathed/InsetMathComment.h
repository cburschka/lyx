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

class latexkeys;

/// Inset for end-of-line comments
class InsetMathComment : public InsetMathNest {
public:
	///
	InsetMathComment();
	///
	explicit InsetMathComment(docstring const &);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const;
	///
	void drawT(TextPainter & pi, int x, int y) const;

	///
	void write(WriteStream & os) const;
	///
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void mathmlize(MathStream &) const;
	///
	void infoize(odocstream & os) const;
private:
	virtual Inset * clone() const;
};

} // namespace lyx

#endif
