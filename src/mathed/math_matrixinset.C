#ifdef __GNUG__
#pragma implementation
#endif

#include "math_matrixinset.h"
#include "math_parser.h"
#include "math_mathmlstream.h"
#include "Lsstream.h"


MathMatrixInset::MathMatrixInset(MathArrayInset const & p)
	: MathArrayInset(p)
{}


MathInset * MathMatrixInset::clone() const
{
	return new MathMatrixInset(*this);
}


void MathMatrixInset::write(WriteStream & os) const
{
	MathArrayInset::write(os);
}


void MathMatrixInset::normalize(NormalStream & os) const
{
	MathArrayInset::normalize(os);
}


void MathMatrixInset::maplize(MapleStream & os) const
{
	os << "matrix(" << int(nrows()) << ',' << int(ncols()) << ",[";
	for (idx_type idx = 0; idx < nargs(); ++idx) {
		if (idx)
			os << ',';
		os << cell(idx);
	}
	os << "])";
}


void MathMatrixInset::mathmlize(MathMLStream & os) const
{
	os << MTag("mtable");
	for (row_type row = 0; row < nrows(); ++row) {
		os << MTag("mtr");
		for (col_type col = 0; col < ncols(); ++col) 
			os << cell(index(row, col));
		os << ETag("mtr");
	}
	os << ETag("mtable");
}


void MathMatrixInset::octavize(OctaveStream & os) const
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
