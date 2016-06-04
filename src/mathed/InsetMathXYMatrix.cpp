/**
 * \file InsetMathXYMatrix.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathXYMatrix.h"

#include "MathStream.h"

#include "LaTeXFeatures.h"
#include "MetricsInfo.h"

#include <ostream>

namespace lyx {


InsetMathXYMatrix::InsetMathXYMatrix(Buffer * buf, Length const & s, char c,
	bool e) : InsetMathGrid(buf, 1, 1), spacing_(s), spacing_code_(c),
	equal_spacing_(e)
{
}


Inset * InsetMathXYMatrix::clone() const
{
	return new InsetMathXYMatrix(*this);
}


int InsetMathXYMatrix::colsep() const
{
	return 40;
}


int InsetMathXYMatrix::rowsep() const
{
	return 40;
}


void InsetMathXYMatrix::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (mi.base.style == LM_ST_DISPLAY)
		mi.base.style = LM_ST_TEXT;
	InsetMathGrid::metrics(mi, dim);
}


void InsetMathXYMatrix::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	os << "\\xymatrix";
	bool open = os.startOuterRow();
	if (equal_spacing_) {
		os << "@!";
		switch (spacing_code_) {
		case '0':
		case 'R':
		case 'C':
			os << spacing_code_;
		}
	} else {
		switch (spacing_code_) {
		case 'R':
		case 'C':
		case 'M':
		case 'W':
		case 'H':
		case 'L':
			os << '@' << spacing_code_ << '='
			   << from_ascii(spacing_.asLatexString());
			break;
		default:
			if (!spacing_.empty())
				os << "@=" << from_ascii(spacing_.asLatexString());
		}
	}
	os << '{';
	InsetMathGrid::write(os);
	os << "}";
	if (open)
		os.startOuterRow();
	os << "\n";
}


void InsetMathXYMatrix::infoize(odocstream & os) const
{
	os << "xymatrix ";
	if (equal_spacing_) {
		switch (spacing_code_) {
		case '0':
		case 'R':
		case 'C':
			os << '!' << spacing_code_ << ' ';
		}
	} else {
		switch (spacing_code_) {
		case 'R':
		case 'C':
		case 'M':
		case 'W':
		case 'H':
		case 'L':
			os << spacing_code_ << ' '
			   << from_ascii(spacing_.asLatexString()) << ' ';
			break;
		default:
			if (!spacing_.empty())
				os << from_ascii(spacing_.asLatexString()) << ' ';
		}
	}
	InsetMathGrid::infoize(os);
}


void InsetMathXYMatrix::normalize(NormalStream & os) const
{
	os << "[xymatrix ";
	InsetMathGrid::normalize(os);
	os << ']';
}


void InsetMathXYMatrix::maple(MapleStream & os) const
{
	os << "xymatrix(";
	InsetMathGrid::maple(os);
	os << ')';
}


void InsetMathXYMatrix::validate(LaTeXFeatures & features) const
{
	features.require("xy");
	InsetMathGrid::validate(features);
}


void InsetMathXYMatrix::mathmlize(MathStream &) const
{
	throw MathExportException();
}


void InsetMathXYMatrix::htmlize(HtmlStream &) const 
{
	throw MathExportException(); 
}


} // namespace lyx
