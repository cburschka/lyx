// -*- C++ -*-
/**
 * \file math_exintinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_EXINTINSET_H
#define MATH_EXINTINSET_H


// \int_l^u f(x) dx in one block (as opposed to 'f','(','x',')' or 'f','x')
// or \sum, \prod...  for interfacing external programs

#include "math_nestinset.h"

// cell(0) is stuff before the 'd', cell(1) the stuff after
class MathExIntInset : public MathNestInset {
public:
	///
	explicit MathExIntInset(string const & name_);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void symbol(string const &);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;

	///
	void normalize(NormalStream &) const;
	///
	void maxima(MaximaStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void write(WriteStream & os) const;
private:
	///
	bool hasScripts() const;

	///
	string symbol_;
};

#endif
