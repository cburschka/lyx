// -*- C++ -*-
/**
 * \file InsetMathXYMatrix.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_XYMATRIX_H
#define MATH_XYMATRIX_H

#include "lyxlength.h"
#include "InsetMathGrid.h"


class InsetMathXYMatrix : public InsetMathGrid {
public:
	///
	InsetMathXYMatrix();
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	InsetMathXYMatrix const * asXYMatrixInset() const { return this; }
	///
	virtual int colsep() const;
	///
	virtual int rowsep() const;

	///
	void normalize();
	///
	void write(WriteStream & os) const;
	///
	void infoize(std::ostream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
private:
	///
	virtual std::auto_ptr<InsetBase> doClone() const;
};

#endif
