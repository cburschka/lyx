#ifdef __GNUG__
#pragma implementation
#endif

#include "math_arrayinset.h"
#include "math_parser.h"
#include "support/LOstream.h"
#include "Lsstream.h"

using std::vector;


MathArrayInset::MathArrayInset(int m, int n)
	: MathGridInset(m, n)
{}


MathArrayInset::MathArrayInset(int m, int n, char valign, string const & halign)
	: MathGridInset(m, n, valign, halign)
{}


MathArrayInset::MathArrayInset(string const & str)
	: MathGridInset(1, 1)
{
	vector< vector<string> > dat;
	istringstream is(str);
	while (is) {
		string line;
		getline(is, line);
		istringstream ls(line);
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


void MathArrayInset::write(MathWriteInfo & os) const
{
	if (os.fragile)
		os << "\\protect";
	os << "\\begin{array}";

	if (v_align_ == 't' || v_align_ == 'b') 
		os << '[' << char(v_align_) << ']';

	os << '{';
	for (col_type col = 0; col < ncols(); ++col)
		os << colinfo_[col].align_;
	os << "}\n";

	MathGridInset::write(os);

	if (os.fragile)
		os << "\\protect";
	os << "\\end{array}\n";
}


void MathArrayInset::writeNormal(std::ostream & os) const
{
	os << "[array ";
	MathGridInset::writeNormal(os);
	os << "]";
}


void MathArrayInset::metrics(MathMetricsInfo const & st) const
{
	MathMetricsInfo mi = st;
	if (mi.style == LM_ST_DISPLAY)
		mi.style = LM_ST_TEXT;
	MathGridInset::metrics(mi);
}
