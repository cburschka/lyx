#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_arrayinset.h"
#include "math_parser.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "Lsstream.h"

#include <iterator>

using std::vector;
using std::istringstream;
using std::getline;


MathArrayInset::MathArrayInset(int m, int n)
	: MathGridInset(m, n)
{}


MathArrayInset::MathArrayInset(int m, int n, char valign, string const & halign)
	: MathGridInset(m, n, valign, halign)
{}


MathArrayInset::MathArrayInset(char valign, string const & halign)
	: MathGridInset(valign, halign)
{}


MathArrayInset::MathArrayInset(string const & str)
	: MathGridInset(1, 1)
{
	vector< vector<string> > dat;
	istringstream is(str.c_str());
	while (is) {
		string line;
		getline(is, line);
		istringstream ls(line.c_str());
		typedef std::istream_iterator<string> iter;
		vector<string> v = vector<string>(iter(ls), iter());
		if (v.size())
			dat.push_back(v);
	}

	for (row_type row = 1; row < dat.size(); ++row)
		addRow(0);
	for (col_type col = 1; col < dat[0].size(); ++col)
		addCol(0);
	for (row_type row = 0; row < dat.size(); ++row)
		for (col_type col = 0; col < dat[row].size(); ++col)
			mathed_parse_cell(cell(index(row, col)), dat[row][col]);
}


MathInset * MathArrayInset::clone() const
{
	return new MathArrayInset(*this);
}


void MathArrayInset::metrics(MathMetricsInfo const & st) const
{
	MathMetricsInfo mi = st;
	if (mi.style == LM_ST_DISPLAY)
		mi.style = LM_ST_TEXT;
	MathGridInset::metrics(mi);
}


void MathArrayInset::write(WriteStream & os) const
{
	if (os.fragile())
		os << "\\protect";
	os << "\\begin{array}";

	if (v_align_ == 't' || v_align_ == 'b') 
		os << '[' << char(v_align_) << ']';
	os << '{' << halign() << "}\n";

	MathGridInset::write(os);

	if (os.fragile())
		os << "\\protect";
	os << "\\end{array}\n";
}


void MathArrayInset::normalize(NormalStream & os) const
{
	os << "[array ";
	MathGridInset::normalize(os);
	os << "]";
}


void MathArrayInset::maplize(MapleStream & os) const
{
	os << "array(";
	MathGridInset::maplize(os);
	os << ")";
}
