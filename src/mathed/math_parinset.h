// -*- C++ -*-
/**
 * \file math_parinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_PARINSET_H
#define MATH_PARINSET_H


#include "math_hullinset.h"

class MathParInset : public MathHullInset {
public:
	///
	MathParInset() {}
	///
	MathParInset(MathArray const & ar);
	///
	mode_type currentMode() const { return TEXT_MODE; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	void infoize(std::ostream & os) const;
	///
	void write(WriteStream & os) const;
};

#endif
