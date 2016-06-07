// -*- C++ -*-
/**
 * \file InsetMathFontOld.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_FONTOLDINSET_H
#define MATH_FONTOLDINSET_H

#include "InsetMathNest.h"


namespace lyx {


class latexkeys;

/// Old-style font changes
class InsetMathFontOld : public InsetMathNest {
public:
	///
	explicit InsetMathFontOld(Buffer * buf, latexkeys const * key);
	/// we inherit the mode
	mode_type currentMode() const { return current_mode_; }
	/// we write extra braces in any case...
	bool extraBraces() const { return true; }
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
	void normalize(NormalStream &) const;
	///
	void infoize(odocstream & os) const;
	///
	int kerning(BufferView const * bv) const { return cell(0).kerning(bv); }
	///
	InsetCode lyxCode() const { return MATH_FONTOLD_CODE; }

private:
	std::string font() const;
	///
	virtual Inset * clone() const;
	/// the font to be used on screen
	latexkeys const * key_;
	/// the inherited mode
	mutable mode_type current_mode_;
};


} // namespace lyx
#endif
