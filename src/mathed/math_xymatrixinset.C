#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_xymatrixinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"


MathXYMatrixInset::MathXYMatrixInset()
	: MathGridInset(1, 1)
{}


MathInset * MathXYMatrixInset::clone() const
{
	return new MathXYMatrixInset(*this);
}


int MathXYMatrixInset::colsep() const
{
	return 40;
}


int MathXYMatrixInset::rowsep() const
{
	return 40;
}


void MathXYMatrixInset::metrics(MathMetricsInfo const & st) const
{
	MathMetricsInfo mi = st;
	if (mi.style == LM_ST_DISPLAY)
		mi.style = LM_ST_TEXT;
	MathGridInset::metrics(mi);
}


void MathXYMatrixInset::write(WriteStream & os) const
{
	os << "\\xymatrix{";
	MathGridInset::write(os);
	os << "}\n";
}


void MathXYMatrixInset::normalize(NormalStream & os) const
{
	os << "[xymatrix ";
	MathGridInset::normalize(os);
	os << "]";
}


void MathXYMatrixInset::maplize(MapleStream & os) const
{
	os << "xymatrix(";
	MathGridInset::maplize(os);
	os << ")";
}
