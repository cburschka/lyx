#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_amsarrayinset.h"
#include "math_mathmlstream.h"
#include "math_metricsinfo.h"
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
	if (name_ == "pmatrix")
		return "(";
	return ".";
}


char const * MathAMSArrayInset::name_right() const
{
	if (name_ == "bmatrix")
		return "]";
	if (name_ == "vmatrix")
		return "|";
	if (name_ == "Vmatrix")
		return "Vert";
	if (name_ == "pmatrix")
		return ")";
	return ".";
}


void MathAMSArrayInset::metrics(MathMetricsInfo & mi) const
{
	MathMetricsInfo m = mi;
	if (m.base.style == LM_ST_DISPLAY)
		m.base.style = LM_ST_TEXT;
	MathGridInset::metrics(m);
	dim_.w += 12;
}


void MathAMSArrayInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathGridInset::draw(pi, x + 6, y);
	int const yy = y - ascent();
	mathed_draw_deco(pi, x + 1, yy, 5, height(), name_left());
	mathed_draw_deco(pi, x + width() - 6, yy, 5, height(), name_right());
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

