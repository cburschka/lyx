// -*- C++ -*-
#ifndef MATH_COMMENTINSET_H
#define MATH_COMMENTINSET_H

#include "math_nestinset.h"


/** Inset for end-of-line comments
 *  \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
 */

class latexkeys;

class MathCommentInset : public MathNestInset {
public:
	///
	MathCommentInset();
	///
	explicit MathCommentInset(string const &);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
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
	void mathmlize(MathMLStream &) const;
	///
	void infoize(std::ostream & os) const;
};
#endif
