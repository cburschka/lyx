// -*- C++ -*-
/**
 * \file math_unknowninset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_UNKNOWNINSET_H
#define MATH_UNKNOWNINSET_H

#include "math_diminset.h"


/// LaTeX names for objects that we really don't know
class MathUnknownInset : public MathDimInset {
public:
	///
	explicit MathUnknownInset(std::string const & name,
		bool final = true, bool black = false);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void setName(std::string const & name);
	///
	std::string name() const;
	/// identifies UnknownInsets
	MathUnknownInset const * asUnknownInset() const { return this; }
	/// identifies UnknownInsets
	MathUnknownInset * asUnknownInset() { return this; }

	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void finalize();
	///
	bool final() const;
private:
	virtual std::auto_ptr<InsetBase> doClone() const;
	///
	std::string name_;
	/// are we finished creating the name?
	bool final_;
	///
	bool black_;
};
#endif
