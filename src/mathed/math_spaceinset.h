// -*- C++ -*-
/**
 * \file math_spaceinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_SPACEINSET_H
#define MATH_SPACEINSET_H

#include "math_inset.h"


/// Smart spaces
class MathSpaceInset : public MathInset {
public:
	///
	explicit MathSpaceInset(int sp);
	///
	explicit MathSpaceInset(std::string const & name);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	MathSpaceInset const * asSpaceInset() const { return this; }
	///
	MathSpaceInset * asSpaceInset() { return this; }
	///
	void incSpace();
	///
	int ascent() const;
	///
	int descent() const;
	///
	int width() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;

	///
	void normalize(NormalStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void write(WriteStream & os) const;
private:
	///
	int space_;
};
#endif
