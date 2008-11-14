// -*- C++ -*-
/**
 * \file InsetMathAMSArray.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_AMSARRAYINSET_H
#define MATH_AMSARRAYINSET_H

#include "InsetMathGrid.h"


namespace lyx {

/// Inset for things like [pbvV]matrix, psmatrix etc
class InsetMathAMSArray : public InsetMathGrid {
public:
	///
	InsetMathAMSArray(docstring const & name, int m, int n);
	///
	InsetMathAMSArray(docstring const & name);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pain, int x, int y) const;
	///
	InsetMathAMSArray * asAMSArrayInset() { return this; }
	///
	InsetMathAMSArray const * asAMSArrayInset() const { return this; }

	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const;
	///
	void write(WriteStream & os) const;
	///
	void infoize(odocstream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
private:
	virtual Inset * clone() const;
	///
	char const * name_left() const;
	///
	char const * name_right() const;

	///
	docstring name_;
};

} // namespace lyx

#endif
