#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_amsarrayinset.h"
#include "math_mathmlstream.h"
#include "math_support.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "Lsstream.h"


MathAMSArrayInset::MathAMSArrayInset(string const & name, int m, int n)
	: MathGridInset(m, n), name_(name)
{}


MathAMSArrayInset::MathAMSArrayInset(string const & name)
	: MathGridInset(1, 1), name_(name)
{}


MathInset * MathAMSArrayInset::clone() const
{
	return new MathAMSArrayInset(*this);
}


char const * MathAMSArrayInset::name_left() const
{
	if (name_ == "bmatrix")
		return "[";
	if (name_ == "vmatrix")
		return "|";
	if (name_ == "Vmatrix")
		return "Vert";
	return "(";
}


char const * MathAMSArrayInset::name_right() const
{
	if (name_ == "bmatrix")
		return "]";
	if (name_ == "vmatrix")
		return "|";
	if (name_ == "Vmatrix")
		return "Vert";
	return ")";
}


void MathAMSArrayInset::metrics(MathMetricsInfo const & st) const
{
	MathMetricsInfo mi = st;
	if (mi.style == LM_ST_DISPLAY)
		mi.style = LM_ST_TEXT;
	MathGridInset::metrics(mi);
	width_ += 12;
}


void MathAMSArrayInset::draw(Painter & pain, int x, int y) const
{ 
	MathGridInset::draw(pain, x + 6, y);
	int yy = y - ascent_;
	mathed_draw_deco(pain, x + 1, yy, 5, height(), name_left());
	mathed_draw_deco(pain, x + width_ - 6, yy, 5, height(), name_right());
}


void MathAMSArrayInset::write(WriteStream & os) const
{
	os << "\\begin{" << name_ << "}";
	MathGridInset::write(os);
	os << "\\end{" << name_ << "}";
}


void MathAMSArrayInset::normalize(NormalStream & os) const
{
	os << "[" << name_ << " ";
	MathGridInset::normalize(os);
	os << "]";
}


void MathAMSArrayInset::maplize(MapleStream & os) const
{
	os << name_ << "(";
	MathGridInset::maplize(os);
	os << ")";
}
