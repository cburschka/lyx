// -*- C++ -*-
/**
 * \file math_rootinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_ROOT_H
#define MATH_ROOT_H

#include "math_nestinset.h"


/// The general n-th root inset.
class MathRootInset : public MathNestInset {
public:
	///
	MathRootInset();
	///
	bool idxUpDown(LCursor & cur, bool up) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void octave(OctaveStream &) const;
private:
	virtual std::auto_ptr<InsetBase> doClone() const;
};

#endif
