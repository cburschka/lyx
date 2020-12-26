// -*- C++ -*-
/**
 * \file InsetMathSymbol.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_SYMBOLINSET_H
#define MATH_SYMBOLINSET_H

#include "InsetMath.h"

namespace lyx {

class latexkeys;


// \xxx symbols that may take limits or grow in displayed formulæ.
class InsetMathSymbol : public InsetMath {
public:
	///
	explicit InsetMathSymbol(latexkeys const * l);
	///
	explicit InsetMathSymbol(char const * name);
	///
	explicit InsetMathSymbol(docstring const & name);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo &, int x, int y) const override;
	///
	int kerning(BufferView const *) const override { return kerning_; }

	///
	mode_type currentMode() const override;
	///
	MathClass mathClass() const override;
	///
	bool isOrdAlpha() const;
	/// The default limits value
	Limits defaultLimits(bool display) const override;
	/// whether the inset has limit-like sub/superscript
	Limits limits() const override { return limits_; }
	/// sets types of sub/superscripts
	void limits(Limits lim) override { limits_ = lim; }
	/// identifies SymbolInset as such
	InsetMathSymbol const * asSymbolInset() const override { return this; }
	/// the LaTeX name of the symbol (without the backslash)
	docstring name() const override;
	/// request "external features"
	void validate(LaTeXFeatures & features) const override;

	///
	void normalize(NormalStream &) const override;
	///
	void maple(MapleStream &) const override;
	///
	void maxima(MaximaStream &) const override;
	///
	void mathematica(MathematicaStream &) const override;
	///
	void mathmlize(MathMLStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	/// \param spacing controls whether we print spaces around
	/// "operator"-type symbols or just print them raw
	void htmlize(HtmlStream &, bool spacing) const;
	///
	void octave(OctaveStream &) const override;
	///
	void write(TeXMathStream & os) const override;
	///
	void infoize2(odocstream & os) const override;
	///
	InsetCode lyxCode() const override { return MATH_SYMBOL_CODE; }

private:
	Inset * clone() const override;
	///
	latexkeys const * sym_;
	///
	Limits limits_ = AUTO_LIMITS;

	// FIXME: these depend on BufferView

	///
	mutable int h_ = 0;
	/// cached superscript kerning
	mutable int kerning_ = 0;
};

} // namespace lyx

#endif
