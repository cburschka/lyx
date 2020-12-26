// -*- C++ -*-
/**
 * \file InsetMathBrace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_BRACEINSET_H
#define MATH_BRACEINSET_H

#include "InsetMathNest.h"


namespace lyx {


/// Extra nesting
class InsetMathBrace : public InsetMathNest {
public:
	///
	explicit InsetMathBrace(Buffer * buf);
	///
	explicit InsetMathBrace(MathData const & ar);
	/// identifies brace insets
	InsetMathBrace * asBraceInset() override { return this; }
	/// identifies brace insets
	InsetMathBrace const * asBraceInset() const override { return this; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo &, int x, int y) const override;
	///
	void write(TeXMathStream & os) const override;
	/// write normalized content
	void normalize(NormalStream & ns) const override;
	///
	void maple(MapleStream &) const override;
	///
	void mathematica(MathematicaStream &) const override;
	///
	void octave(OctaveStream &) const override;
	///
	void mathmlize(MathMLStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void infoize(odocstream & os) const override;
	///
	InsetCode lyxCode() const override { return MATH_BRACE_CODE; }
private:
	Inset * clone() const override;
};


} // namespace lyx

#endif
