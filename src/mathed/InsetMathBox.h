// -*- C++ -*-
/**
 * \file InsetMathBox.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_BOXINSET_H
#define MATH_BOXINSET_H

#include "InsetMathNest.h"

#include <string>


namespace lyx {

/// Support for \\mbox
class InsetMathBox : public InsetMathNest {
public:
	///
	explicit InsetMathBox(docstring const & name);
	///
	mode_type currentMode() const { return TEXT_MODE; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream & ns) const;
	///
	void infoize(odocstream & os) const;
	///
	void validate(LaTeXFeatures & features) const;

private:
	Inset * clone() const { return new InsetMathBox(*this); }
	///
	docstring name_;
};


/// Non-AMS-style frame
class InsetMathFBox : public InsetMathNest {
public:
	///
	InsetMathFBox();
	///
	mode_type currentMode() const { return TEXT_MODE; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
	///
	void infoize(odocstream & os) const;
private:
	///
	Inset * clone() const { return new InsetMathFBox(*this); }
};


/// Extra nesting: \\makebox or \\framebox.
class InsetMathMakebox : public InsetMathNest {
public:
	///
	InsetMathMakebox(bool framebox);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
	///
	mode_type currentMode() const { return TEXT_MODE; }
	///
	void infoize(odocstream & os) const;
private:
	Inset * clone() const { return new InsetMathMakebox(*this); }
	///
	bool framebox_;
};



/// AMS-style frame
class InsetMathBoxed : public InsetMathNest {
public:
	///
	InsetMathBoxed();
	///
	void validate(LaTeXFeatures & features) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
	///
	void infoize(odocstream & os) const;
private:
	Inset * clone() const { return new InsetMathBoxed(*this); }
};


} // namespace lyx

#endif // MATH_MBOX
