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
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	int kerning(BufferView const *) const { return kerning_; }

	///
	mode_type currentMode() const;
	///
	MathClass mathClass() const;
	///
	bool isOrdAlpha() const;
	/// The default limits value
	Limits defaultLimits() const;
	/// whether the inset has limit-like sub/superscript
	Limits limits() const { return limits_; }
	/// sets types of sub/superscripts
	void limits(Limits lim) { limits_ = lim; }
	/// identifies SymbolInset as such
	InsetMathSymbol const * asSymbolInset() const { return this; }
	/// the LaTeX name of the symbol (without the backslash)
	docstring name() const;
	/// request "external features"
	void validate(LaTeXFeatures & features) const;

	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void maxima(MaximaStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathStream &) const;
	///
	void htmlize(HtmlStream &) const;
	/// \param spacing controls whether we print spaces around
	/// "operator"-type symbols or just print them raw
	void htmlize(HtmlStream &, bool spacing) const;
	///
	void octave(OctaveStream &) const;
	///
	void write(WriteStream & os) const;
	///
	void infoize2(odocstream & os) const;
	///
	InsetCode lyxCode() const { return MATH_SYMBOL_CODE; }

private:
	virtual Inset * clone() const;
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
