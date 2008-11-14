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

#include "support/docstream.h"

namespace lyx {

/// Change colours.

class InsetMathColor : public InsetMathNest {
public:
	/// Create a color inset from LyX color number
	explicit InsetMathColor(bool oldstyle,
		ColorCode color = Color_none);
	/// Create a color inset from LaTeX color name
	explicit InsetMathColor(bool oldstyle, docstring const & color);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	/// we write extra braces in any case...
	/// FIXME Why? Are they necessary if oldstyle_ == false?
	bool extraBraces() const { return true; }
	///
	void draw(PainterInfo & pi, int x, int y) const;
	/// we need package color
	void validate(LaTeXFeatures & features) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
	///
	void infoize(odocstream & os) const;
private:
	virtual Inset * clone() const;
	/// width of '[' in current font
	mutable int w_;
	///
	bool oldstyle_;
	/// Our color. Only valid LaTeX colors are allowed.
	docstring color_;
};


} // namespace lyx

#endif
