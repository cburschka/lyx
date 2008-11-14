/**
 * \file InsetMathMatrix.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathMatrix.h"
#include "MathData.h"
#include "MathStream.h"


namespace lyx {

InsetMathMatrix::InsetMathMatrix(InsetMathGrid const & p)
	: InsetMathGrid(p)
{}


Inset * InsetMathMatrix::clone() const
{
	return new InsetMathMatrix(*this);
}


void InsetMathMatrix::write(WriteStream & os) const
{
	InsetMathGrid::write(os);
}


void InsetMathMatrix::normalize(NormalStream & os) const
{
	InsetMathGrid::normalize(os);
}


void InsetMathMatrix::maple(MapleStream & os) const
{
	os << "matrix(" << int(nrows()) << ',' << int(ncols()) << ",[";
	for (idx_type idx = 0; idx < nargs(); ++idx) {
		if (idx)
			os << ',';
		os << cell(idx);
	}
	os << "])";
}


void InsetMathMatrix::maxima(MaximaStream & os) const
{
	os << "matrix(";
	for (row_type row = 0; row < nrows(); ++row) {
		if (row)
			os << ',';
		os << '[';
		for (col_type col = 0; col < ncols(); ++col) {
			if (col)
				os << ',';
			os << cell(index(row, col));
		}
		os << ']';
	}
	os << ')';
}


void InsetMathMatrix::mathematica(MathematicaStream & os) const
{
	os << '{';
	for (row_type row = 0; row < nrows(); ++row) {
		if (row)
			os << ',';
		os << '{';
		for (col_type col = 0; col < ncols(); ++col) {
			if (col)
				os << ',';
			os << cell(index(row, col));
		}
		os << '}';
	}
	os << '}';
}


void InsetMathMatrix::mathmlize(MathStream & os) const
{
	InsetMathGrid::mathmlize(os);
}


void InsetMathMatrix::octave(OctaveStream & os) const
{
	os << '[';
	for (row_type row = 0; row < nrows(); ++row) {
		if (row)
			os << ';';
		os << '[';
		for (col_type col = 0; col < ncols(); ++col)
			os << cell(index(row, col)) << ' ';
		os << ']';
	}
	os << ']';
}


} // namespace lyx
