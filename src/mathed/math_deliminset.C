/**
 * \file math_deliminset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_deliminset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"


using std::string;
using std::max;
using std::auto_ptr;

namespace {

string convertDelimToLatexName(string const & name)
{
	if (name == "<")
		return name;
	if (name == "(")
		return name;
	if (name == "[")
		return name;
	if (name == ".")
		return name;
	if (name == ">")
		return name;
	if (name == ")")
		return name;
	if (name == "]")
		return name;
	if (name == "/")
		return name;
	if (name == "|")
		return name;
	return '\\' + name + ' ';
}

}



MathDelimInset::MathDelimInset(string const & l, string const & r)
	: MathNestInset(1), left_(l), right_(r)
{}


MathDelimInset::MathDelimInset
		(string const & l, string const & r, MathArray const & ar)
	: MathNestInset(1), left_(l), right_(r)
{
	cell(0) = ar;
}


auto_ptr<InsetBase> MathDelimInset::doClone() const
{
	return auto_ptr<InsetBase>(new MathDelimInset(*this));
}


void MathDelimInset::write(WriteStream & os) const
{
	os << "\\left" << convertDelimToLatexName(left_) << cell(0)
	   << "\\right" << convertDelimToLatexName(right_);
}


void MathDelimInset::normalize(NormalStream & os) const
{
	os << "[delim " << convertDelimToLatexName(left_) << ' '
	   << convertDelimToLatexName(right_) << ' ' << cell(0) << ']';
}


void MathDelimInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi);
	Dimension t;
	mathed_char_dim(mi.base.font, 'I', t);
	int h0 = (t.asc + t.des) / 2;
	int a0 = max(cell(0).ascent(), t.asc)   - h0;
	int d0 = max(cell(0).descent(), t.des)  + h0;
	dw_ = cell(0).height() / 5;
	if (dw_ > 8)
		dw_ = 8;
	if (dw_ < 4)
		dw_ = 4;
	dim_.wid = cell(0).width() + 2 * dw_ + 8;
	dim_.asc = max(a0, d0) + h0;
	dim_.des = max(a0, d0) - h0;
	dim = dim_;
}


void MathDelimInset::draw(PainterInfo & pi, int x, int y) const
{
	int const b = y - dim_.asc;
	cell(0).draw(pi, x + dw_ + 4, y);
	mathed_draw_deco(pi, x + 4, b, dw_, dim_.height(), left_);
	mathed_draw_deco(pi, x + dim_.width() - dw_ - 4,
		b, dw_, dim_.height(), right_);
}


bool MathDelimInset::isParanthesis() const
{
	return left_ == "(" && right_ == ")";
}


bool MathDelimInset::isBrackets() const
{
	return left_ == "[" && right_ == "]";
}


bool MathDelimInset::isAbs() const
{
	return left_ == "|" && right_ == "|";
}


void MathDelimInset::maple(MapleStream & os) const
{
	if (isAbs()) {
		if (cell(0).size() == 1 && cell(0).front()->asMatrixInset())
			os << "linalg[det](" << cell(0) << ')';
		else
			os << "abs(" << cell(0) << ')';
	}
	else
		os << left_ << cell(0) << right_;
}

void MathDelimInset::maxima(MaximaStream & os) const
{
	if (isAbs()) {
		if (cell(0).size() == 1 && cell(0).front()->asMatrixInset())
			os << "determinant(" << cell(0) << ')';
		else
			os << "abs(" << cell(0) << ')';
	}
	else
		os << left_ << cell(0) << right_;
}


void MathDelimInset::mathematica(MathematicaStream & os) const
{
	if (isAbs()) {
		if (cell(0).size() == 1 && cell(0).front()->asMatrixInset())
			os << "Det" << cell(0) << ']';
		else
			os << "Abs[" << cell(0) << ']';
	}
	else
		os << left_ << cell(0) << right_;
}


void MathDelimInset::mathmlize(MathMLStream & os) const
{
	os << "<fenced open=\"" << left_ << "\" close=\""
		<< right_ << "\">" << cell(0) << "</fenced>";
}


void MathDelimInset::octave(OctaveStream & os) const
{
	if (isAbs())
		os << "det(" << cell(0) << ')';
	else
		os << left_ << cell(0) << right_;
}
