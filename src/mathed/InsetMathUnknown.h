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
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void setName(docstring const & name);
	///
	docstring name() const;

	///
	docstring const & selection() const { return selection_; }
	
	/// identifies UnknownInsets
	InsetMathUnknown const * asUnknownInset() const { return this; }
	/// identifies UnknownInsets
	InsetMathUnknown * asUnknownInset() { return this; }

	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathStream &) const;
	///
	void htmlize(HtmlStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void finalize();
	///
	bool final() const;
	///
	int kerning(BufferView const *) const { return kerning_; }
	///
	InsetCode lyxCode() const { return MATH_UNKNOWN_CODE; }

private:
	///
	Inset * clone() const { return new InsetMathUnknown(*this); }

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
