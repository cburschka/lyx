/**
 * \file math_arrayinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_arrayinset.h"
#include "math_data.h"
#include "math_parser.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "support/std_sstream.h"

#include <iterator>

using std::getline;

using std::string;
using std::auto_ptr;
using std::istringstream;
using std::istream_iterator;
using std::vector;


MathArrayInset::MathArrayInset(string const & name, int m, int n)
	: MathGridInset(m, n), name_(name)
{}


MathArrayInset::MathArrayInset(string const & name, int m, int n,
		char valign, string const & halign)
	: MathGridInset(m, n, valign, halign), name_(name)
{}


MathArrayInset::MathArrayInset(string const & name, char valign,
		string const & halign)
	: MathGridInset(valign, halign), name_(name)
{}


MathArrayInset::MathArrayInset(string const & name, string const & str)
	: MathGridInset(1, 1), name_(name)
{
	vector< vector<string> > dat;
	istringstream is(str);
	string line;
	while (getline(is, line)) {
		istringstream ls(line);
		typedef istream_iterator<string> iter;
		vector<string> v = vector<string>(iter(ls), iter());
		if (v.size())
			dat.push_back(v);
	}

	for (row_type row = 1; row < dat.size(); ++row)
		addRow(0);
	for (col_type col = 1; col < dat[0].size(); ++col)
		addCol(0);
	for (row_type row = 0; row < dat.size(); ++row)
		for (col_type col = 0; col < dat[0].size(); ++col)
			mathed_parse_cell(cell(index(row, col)), dat[row][col]);
}


auto_ptr<InsetBase> MathArrayInset::clone() const
{
	return auto_ptr<InsetBase>(new MathArrayInset(*this));
}


void MathArrayInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	ArrayChanger dummy(mi.base);
	MathGridInset::metrics(mi, dim);
}


void MathArrayInset::draw(PainterInfo & pi, int x, int y) const
{
	ArrayChanger dummy(pi.base);
	MathGridInset::draw(pi, x + 1, y);
}


void MathArrayInset::write(WriteStream & os) const
{
	if (os.fragile())
		os << "\\protect";
	os << "\\begin{" << name_ << '}';

	if (v_align_ == 't' || v_align_ == 'b')
		os << '[' << char(v_align_) << ']';
	os << '{' << halign() << "}\n";

	MathGridInset::write(os);

	if (os.fragile())
		os << "\\protect";
	os << "\\end{" << name_ << '}';
	// adding a \n here is bad if the array is the last item
	// in an \eqnarray...
}


void MathArrayInset::infoize(std::ostream & os) const
{
	os << "Array";
}


void MathArrayInset::normalize(NormalStream & os) const
{
	os << '[' << name_ << ' ';
	MathGridInset::normalize(os);
	os << ']';
}


void MathArrayInset::maple(MapleStream & os) const
{
	os << "array(";
	MathGridInset::maple(os);
	os << ')';
}
