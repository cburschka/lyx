// -*- C++ -*-
/**
 * \file InsetMathUnknown.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_UNKNOWNINSET_H
#define MATH_UNKNOWNINSET_H

#include "InsetMath.h"


namespace lyx {

/// LaTeX names for objects that we really don't know
class InsetMathUnknown : public InsetMath {
public:
	///
	explicit InsetMathUnknown(docstring const & name,
		docstring const & selection = empty_docstring(),
		bool final = true, bool black = false);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void setName(docstring const & name);
	///
	docstring name() const override;

	///
	docstring const & selection() const { return selection_; }

	/// identifies UnknownInsets
	InsetMathUnknown const * asUnknownInset() const override { return this; }
	/// identifies UnknownInsets
	InsetMathUnknown * asUnknownInset() override { return this; }

	///
	void normalize(NormalStream &) const override;
	///
	void maple(MapleStream &) const override;
	///
	void mathematica(MathematicaStream &) const override;
	///
	void mathmlize(MathMLStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void octave(OctaveStream &) const override;
	///
	void finalize();
	///
	bool final() const;
	///
	int kerning(BufferView const *) const override { return kerning_; }
	///
	InsetCode lyxCode() const override { return MATH_UNKNOWN_CODE; }

private:
	///
	Inset * clone() const override { return new InsetMathUnknown(*this); }

	///
	docstring name_;
	/// are we finished creating the name?
	bool final_;
	///
	bool black_;
	///
	mutable int kerning_;
	/// the selection which was replaced by this
	docstring selection_;
};

} // namespace lyx

#endif
