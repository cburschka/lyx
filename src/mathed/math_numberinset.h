// -*- C++ -*-
/**
 * \file math_numberinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_NUMBERINSET_H
#define MATH_NUMBERINSET_H

#include "math_inset.h"


/** Some inset that "is" a number
 *  mainly for math-extern
 */
class MathNumberInset : public MathInset {
public:
	///
	explicit MathNumberInset(string const & s);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	string str() const { return str_; }
	///
	MathNumberInset * asNumberInset() { return this; }

	///
	void normalize(NormalStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void write(WriteStream & os) const;

private:
	/// the number as string
	string str_;
};
#endif
