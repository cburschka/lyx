/**
 * \file math_amsarrayinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_amsarrayinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"

using std::auto_ptr;


MathAMSArrayInset::MathAMSArrayInset(string const & name, int m, int n)
	: MathGridInset(m, n), name_(name)
{}


MathAMSArrayInset::MathAMSArrayInset(string const & name)
	: MathGridInset(1, 1), name_(name)
{}


auto_ptr<InsetBase> MathAMSArrayInset::clone() const
{
	return auto_ptr<InsetBase>(new MathAMSArrayInset(*this));
}


char const * MathAMSArrayInset::name_left() const
{
	if (name_ == "bmatrix")
		return "[";
	if (name_ == "Bmatrix")
		return "{";
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
	if (name_ == "Bmatrix")
		return "}";
	if (name_ == "vmatrix")
		return "|";
	if (name_ == "Vmatrix")
		return "Vert";
	if (name_ == "pmatrix")
		return ")";
	return ".";
}


void MathAMSArrayInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	MetricsInfo m = mi;
	if (m.base.style == LM_ST_DISPLAY)
		m.base.style = LM_ST_TEXT;
	MathGridInset::metrics(m);
	dim_.wid += 12;
	dim = dim_;
}


void MathAMSArrayInset::draw(PainterInfo & pi, int x, int y) const
{
	MathGridInset::draw(pi, x + 6, y);
	int const yy = y - dim_.ascent();
	mathed_draw_deco(pi, x + 1, yy, 5, dim_.height(), name_left());
	mathed_draw_deco(pi, x + dim_.width() - 6, yy, 5, dim_.height(), name_right());
}


void MathAMSArrayInset::write(WriteStream & os) const
{
	os << "\\begin{" << name_ << '}';
	MathGridInset::write(os);
	os << "\\end{" << name_ << '}';
}


void MathAMSArrayInset::normalize(NormalStream & os) const
{
	os << '[' << name_ << ' ';
	MathGridInset::normalize(os);
	os << ']';
}
