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
		bool final = true, bool black = false);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void setName(docstring const & name);
	///
	docstring name() const;
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
	void octave(OctaveStream &) const;
	///
	void finalize();
	///
	bool final() const;
	///
	int kerning(BufferView const * bv) const { return kerning_; }
private:
	virtual Inset * clone() const;
	///
	docstring name_;
	/// are we finished creating the name?
	bool final_;
	///
	bool black_;
	///
	mutable int kerning_;
};


} // namespace lyx
#endif
