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


// FIXME XHTML
// We need more information here, so we can output the correct
// delimiters, which will not always be "[". To do this, we need
// to make some changes to InsetMathMatrix, adding a member to 
// record the type of delimiter, and then make the extractMatrices 
// routine in MathExtern give us this information.
void InsetMathMatrix::mathmlize(MathStream & os) const
{
	os << "<mo form='prefix' fence='true' stretchy='true' symmetric='true' lspace='thinmathspace'>[</mo>";
	os << MTag("mtable");
	for (row_type row = 0; row < nrows(); ++row) {
		os << MTag("mtr");
		for (col_type col = 0; col < ncols(); ++col)
			os << MTag("mtd") << cell(index(row, col)) << ETag("mtd");
		os << ETag("mtr");
	}
	os << ETag("mtable");
	os << "<mo form='postfix' fence='true' stretchy='true' symmetric='true' lspace='thinmathspace'>]</mo>";
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
