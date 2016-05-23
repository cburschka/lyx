/**
 * \file InsetMathArray.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathArray.h"

#include "LaTeXFeatures.h"
#include "MathData.h"
#include "MathParser.h"
#include "MathStream.h"
#include "MetricsInfo.h"

#include "support/lstrings.h"

#include <iterator>
#include <sstream>

using namespace std;

namespace lyx {


InsetMathArray::InsetMathArray(Buffer * buf, docstring const & name, int m,
		int n)
	: InsetMathGrid(buf, m, n), name_(name)
{}


InsetMathArray::InsetMathArray(Buffer * buf, docstring const & name, int m,
		int n, char valign, docstring const & halign)
	: InsetMathGrid(buf, m, n, valign, halign), name_(name)
{}


InsetMathArray::InsetMathArray(Buffer * buf, docstring const & name,
		docstring const & str)
	: InsetMathGrid(buf, 1, 1), name_(name)
{
	vector< vector<string> > dat;
	istringstream is(to_utf8(str));
	string line;
	while (getline(is, line)) {
		istringstream ls(line);
		typedef istream_iterator<string> iter;
		vector<string> v = vector<string>(iter(ls), iter());
		if (!v.empty())
			dat.push_back(v);
	}

	for (row_type row = 1; row < dat.size(); ++row)
		addRow(0);
	for (col_type col = 1; col < dat[0].size(); ++col)
		addCol(0);
	for (row_type row = 0; row < dat.size(); ++row)
		for (col_type col = 0; col < dat[0].size(); ++col)
			mathed_parse_cell(cell(index(row, col)),
					  from_utf8(dat[row][col]), Parse::NORMAL);
}


Inset * InsetMathArray::clone() const
{
	return new InsetMathArray(*this);
}


void InsetMathArray::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Changer dummy =
		mi.base.changeStyle(LM_ST_TEXT, mi.base.style == LM_ST_DISPLAY);
	InsetMathGrid::metrics(mi, dim);
	dim.wid += 6;
}


Dimension const InsetMathArray::dimension(BufferView const & bv) const
{
	Dimension dim = InsetMathGrid::dimension(bv);
	dim.wid += 6;
	return dim;
}


void InsetMathArray::draw(PainterInfo & pi, int x, int y) const
{
	setPosCache(pi, x, y);
	Changer dummy =
		pi.base.changeStyle(LM_ST_TEXT, pi.base.style == LM_ST_DISPLAY);
	InsetMathGrid::drawWithMargin(pi, x, y, 4, 2);
}


void InsetMathArray::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);

	if (os.fragile())
		os << "\\protect";
	os << "\\begin{" << name_ << '}';
	bool open = os.startOuterRow();

	char const v = verticalAlignment();
	if (v == 't' || v == 'b')
		os << '[' << v << ']';
	os << '{' << horizontalAlignments() << "}\n";

	InsetMathGrid::write(os);

	if (os.fragile())
		os << "\\protect";
	os << "\\end{" << name_ << '}';
	if (open)
		os.startOuterRow();
	// adding a \n here is bad if the array is the last item
	// in an \eqnarray...
}


void InsetMathArray::infoize(odocstream & os) const
{
	docstring name = name_;
	name[0] = support::uppercase(name[0]);
	os << name << ' ';
}


void InsetMathArray::normalize(NormalStream & os) const
{
	os << '[' << name_ << ' ';
	InsetMathGrid::normalize(os);
	os << ']';
}


void InsetMathArray::maple(MapleStream & os) const
{
	os << "array(";
	InsetMathGrid::maple(os);
	os << ')';
}


void InsetMathArray::validate(LaTeXFeatures & features) const
{
	if (name_ == "subarray")
		features.require("amsmath");
	InsetMathGrid::validate(features);
}


} // namespace lyx
