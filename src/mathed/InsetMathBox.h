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


namespace lyx {

/// Support for \\mbox
class InsetMathBox : public InsetMathNest {
public:
	///
	explicit InsetMathBox(Buffer * buf, docstring const & name);
	///
	mode_type currentMode() const override { return TEXT_MODE; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void write(WriteStream & os) const override;
	///
	void normalize(NormalStream & ns) const override;
	///
	void mathmlize(MathStream & ms) const override;
	///
	void htmlize(HtmlStream & ms) const override;
	///
	void infoize(odocstream & os) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	InsetCode lyxCode() const override { return MATH_BOX_CODE; }

private:
	Inset * clone() const override { return new InsetMathBox(*this); }
	///
	docstring name_;
};


/// Non-AMS-style frame
class InsetMathFBox : public InsetMathNest {
public:
	///
	explicit InsetMathFBox(Buffer * buf);
	///
	mode_type currentMode() const override { return TEXT_MODE; }
	///
	marker_type marker(BufferView const *) const override { return NO_MARKER; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void write(WriteStream & os) const override;
	/// write normalized content
	void normalize(NormalStream & ns) const override;
	///
	void mathmlize(MathStream & ms) const override;
	///
	void htmlize(HtmlStream & ms) const override;
	///
	void infoize(odocstream & os) const override;
	///
	void validate(LaTeXFeatures & features) const override;
private:
	///
	Inset * clone() const override { return new InsetMathFBox(*this); }
};


/// Extra nesting: \\makebox or \\framebox.
class InsetMathMakebox : public InsetMathNest {
public:
	///
	InsetMathMakebox(Buffer * buf, bool framebox);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void write(WriteStream & os) const override;
	/// write normalized content
	void normalize(NormalStream & ns) const override;
	///
	void mathmlize(MathStream & ms) const override;
	///
	void htmlize(HtmlStream & ms) const override;
	///
	mode_type currentMode() const override { return TEXT_MODE; }
	///
	void infoize(odocstream & os) const override;
	///
	void validate(LaTeXFeatures & features) const override;
private:
	Inset * clone() const override { return new InsetMathMakebox(*this); }
	///
	bool framebox_;
};



/// AMS-style frame
class InsetMathBoxed : public InsetMathNest {
public:
	///
	explicit InsetMathBoxed(Buffer * buf);
	///
	marker_type marker(BufferView const *) const override { return NO_MARKER; }
	///
	void validate(LaTeXFeatures & features) const override;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void write(WriteStream & os) const override;
	///
	void mathmlize(MathStream & ms) const override;
	///
	void htmlize(HtmlStream & ms) const override;
	/// write normalized content
	void normalize(NormalStream & ns) const override;
	///
	void infoize(odocstream & os) const override;
private:
	Inset * clone() const override { return new InsetMathBoxed(*this); }
};


} // namespace lyx

#endif // MATH_MBOX
