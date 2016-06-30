/**
 * \file InsetMathDelim.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathDelim.h"

#include "MathData.h"
#include "MathFactory.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "MetricsInfo.h"

#include "LaTeXFeatures.h"

#include "support/docstring.h"

#include "frontends/FontMetrics.h"

#include <algorithm>

using namespace std;

namespace lyx {

static docstring convertDelimToLatexName(docstring const & name)
{
	if (name.size() == 1) {
		char_type const c = name[0];
		if (c == '<' || c == '(' || c == '[' || c == '.'
		    || c == '>' || c == ')' || c == ']' || c == '/' || c == '|')
			return name;
	}
	return '\\' + name + ' ';
}


InsetMathDelim::InsetMathDelim(Buffer * buf, docstring const & l,
		docstring const & r)
	: InsetMathNest(buf, 1), left_(l), right_(r), dw_(0)
{}


InsetMathDelim::InsetMathDelim(Buffer * buf, docstring const & l, docstring const & r,
	MathData const & ar)
	: InsetMathNest(buf, 1), left_(l), right_(r), dw_(0)
{
	cell(0) = ar;
}


Inset * InsetMathDelim::clone() const
{
	return new InsetMathDelim(*this);
}


void InsetMathDelim::validate(LaTeXFeatures & features) const
{
	InsetMathNest::validate(features);
	// The delimiters may be used without \left or \right as well.
	// Therefore they are listed in lib/symbols, and if they have
	// requirements, we need to add them here.
	MathWordList const & words = mathedWordList();
	MathWordList::const_iterator it = words.find(left_);
	if (it != words.end())
	{
		string const req = it->second.requires;
		if (!req.empty())
			features.require(req);
	}
	it = words.find(right_);
	if (it != words.end())
	{
		string const req = it->second.requires;
		if (!req.empty())
			features.require(req);
	}
}


void InsetMathDelim::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	os << "\\left" << convertDelimToLatexName(left_) << cell(0)
	   << "\\right" << convertDelimToLatexName(right_);
}


void InsetMathDelim::normalize(NormalStream & os) const
{
	os << "[delim " << convertDelimToLatexName(left_) << ' '
	   << convertDelimToLatexName(right_) << ' ' << cell(0) << ']';
}


void InsetMathDelim::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Dimension dim0;
	cell(0).metrics(mi, dim0);
	Dimension t = theFontMetrics(mi.base.font).dimension('I');
	int h0 = (t.asc + t.des) / 2;
	int a0 = max(dim0.asc, t.asc)   - h0;
	int d0 = max(dim0.des, t.des)  + h0;
	dw_ = dim0.height() / 5;
	if (dw_ > 8)
		dw_ = 8;
	if (dw_ < 4)
		dw_ = 4;
	dim.wid = dim0.width() + 2 * dw_ + 8;
	dim.asc = max(a0, d0) + h0;
	dim.des = max(a0, d0) - h0;
}


void InsetMathDelim::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);
	int const b = y - dim.asc;
	cell(0).draw(pi, x + dw_ + 4, y);
	mathed_draw_deco(pi, x + 4, b, dw_, dim.height(), left_);
	mathed_draw_deco(pi, x + dim.width() - dw_ - 4,
		b, dw_, dim.height(), right_);
	setPosCache(pi, x, y);
}


bool InsetMathDelim::isParenthesis() const
{
	return left_ == "(" && right_ == ")";
}


bool InsetMathDelim::isBrackets() const
{
	return left_ == "[" && right_ == "]";
}


bool InsetMathDelim::isAbs() const
{
	return left_ == "|" && right_ == "|";
}


void InsetMathDelim::maple(MapleStream & os) const
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


void InsetMathDelim::maxima(MaximaStream & os) const
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


void InsetMathDelim::mathematica(MathematicaStream & os) const
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


void InsetMathDelim::mathmlize(MathStream & os) const
{
	os << "<mo form='prefix' fence='true' stretchy='true' symmetric='true'>"
	   << convertDelimToXMLEscape(left_) 
	   << "</mo>\n"
	   << cell(0) 
	   << "\n<mo form='postfix' fence='true' stretchy='true' symmetric='true'>" 
	   << convertDelimToXMLEscape(right_) 
	   << "</mo>\n";
}


void InsetMathDelim::htmlize(HtmlStream & os) const
{
	os << convertDelimToXMLEscape(left_) 
	   << cell(0) 
	   << convertDelimToXMLEscape(right_);
}


void InsetMathDelim::octave(OctaveStream & os) const
{
	if (isAbs())
		os << "det(" << cell(0) << ')';
	else
		os << left_ << cell(0) << right_;
}


} // namespace lyx
