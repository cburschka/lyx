// -*- C++ -*-
/**
 * \file InsetMathColor.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_COLORINSET_H
#define MATH_COLORINSET_H

#include "InsetMathNest.h"


namespace lyx {

/// Change colours.

class InsetMathColor : public InsetMathNest {
public:
	/// Create a color inset from LyX color number
	InsetMathColor(Buffer * buf, bool oldstyle, ColorCode color = Color_none);
	/// Create a color inset from LaTeX color name
	InsetMathColor(Buffer * buf, bool oldstyle, docstring const & color);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	/// we write extra braces in any case...
	/// FIXME Why? Are they necessary if oldstyle_ == false?
	bool extraBraces() const { return true; }
	///
	void draw(PainterInfo & pi, int x, int y) const;
	/// we need package color
	void validate(LaTeXFeatures & features) const;
	/// we inherit the mode
	mode_type currentMode() const override { return current_mode_; }
	///
	void write(WriteStream & os) const;
	/// FIXME XHTML For now, we do nothing with color.
	void mathmlize(MathStream &) const {}
	/// FIXME XHTML For now, we do nothing with color.
	void htmlize(HtmlStream &) const {}
	///
	/// write normalized content
	void normalize(NormalStream & ns) const;
	///
	void infoize(odocstream & os) const;
	///
	InsetCode lyxCode() const { return MATH_COLOR_CODE; }
private:
	virtual Inset * clone() const;
	///
	bool oldstyle_;
	/// Our color. Only valid LaTeX colors are allowed.
	docstring color_;
	/// the inherited mode
	mutable mode_type current_mode_;
};


} // namespace lyx

#endif
