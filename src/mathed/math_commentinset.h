// -*- C++ -*-
#ifndef MATH_COMMENTINSET_H
#define MATH_COMMENTINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Inset for end-of-line comments
    \author André Pönitz
 */

class latexkeys;

class MathCommentInset : public MathNestInset {
public:
	///
	MathCommentInset();
	///
	explicit MathCommentInset(string const &);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi) const;
	///
	void drawT(TextPainter & pi, int x, int y) const;

	///
	void write(WriteStream & os) const;
	///
	void maplize(MapleStream &) const;
	///
	void mathematicize(MathematicaStream &) const;
	///
	void octavize(OctaveStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void infoize(std::ostream & os) const;
};
#endif
