// -*- C++ -*-
/**
 * \file math_exfuncinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_EXFUNCINSET_H
#define MATH_EXFUNCINSET_H


#include "math_nestinset.h"

#include <string>


// f(x) in one block (as opposed to 'f','(','x',')' or 'f','x')
// for interfacing external programs

class MathExFuncInset : public MathNestInset {
public:
	///
	explicit MathExFuncInset(std::string const & name);
	///
	MathExFuncInset(std::string const & name, MathArray const & ar);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	std::string name() const;

	///
	void maple(MapleStream &) const;
	///
	void maxima(MaximaStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octave(OctaveStream &) const;

private:
	virtual std::auto_ptr<InsetBase> doClone() const;
	///
	std::string const name_;
};
#endif
