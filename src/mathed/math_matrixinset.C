/**
 * \file math_matrixinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_matrixinset.h"
#include "math_parser.h"
#include "math_mathmlstream.h"
#include "Lsstream.h"

using std::auto_ptr;


MathMatrixInset::MathMatrixInset(MathGridInset const & p)
	: MathGridInset(p)
{}


auto_ptr<InsetBase> MathMatrixInset::clone() const
{
	return auto_ptr<InsetBase>(new MathMatrixInset(*this));
}


void MathMatrixInset::write(WriteStream & os) const
{
	MathGridInset::write(os);
}


void MathMatrixInset::normalize(NormalStream & os) const
{
	MathGridInset::normalize(os);
}


void MathMatrixInset::maple(MapleStream & os) const
{
	os << "matrix(" << int(nrows()) << ',' << int(ncols()) << ",[";
	for (idx_type idx = 0; idx < nargs(); ++idx) {
		if (idx)
			os << ',';
		os << cell(idx);
	}
	os << "])";
}


void MathMatrixInset::maxima(MaximaStream & os) const
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


void MathMatrixInset::mathmlize(MathMLStream & os) const
{
	MathGridInset::mathmlize(os);
}


void MathMatrixInset::octave(OctaveStream & os) const
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
