#ifdef __GNUG__
#pragma implementation
#endif

#include "math_gridinset.h"
#include "support/LOstream.h"
#include "debug.h"


namespace {

///
int const MATH_COLSEP = 10;
///
int const MATH_ROWSEP = 10;
///
int const MATH_BORDER = 2;

}


MathGridInset::RowInfo::RowInfo()
	: upperline_(false), lowerline_(false)
{}



int MathGridInset::RowInfo::skipPixels() const
{
#ifdef WITH_WARNINGS
#warning fix this once the interface to LyXLength has improved
#endif
	return int(skip_.value());
}



MathGridInset::ColInfo::ColInfo()
	: align_('c'), leftline_(false), rightline_(false), skip_(MATH_COLSEP)
{}


MathGridInset::MathGridInset(unsigned int m, unsigned int n)
	: MathNestInset(m * n), rowinfo_(n), colinfo_(m), v_align_('c')
{
	if (m <= 0)
		lyxerr << "positve number of columns expected\n";
	if (n <= 0)
		lyxerr << "positve number of rows expected\n";
	setDefaults();
}


unsigned int MathGridInset::index(unsigned int row, unsigned int col) const
{
	return col + ncols() * row;
}


void MathGridInset::setDefaults()
{
	for (unsigned int col = 0; col < ncols(); ++col) {
		colinfo_[col].align_ = defaultColAlign(col);
		colinfo_[col].skip_  = defaultColSpace(col);
	}
}


void MathGridInset::halign(string const & hh)
{
	unsigned int n = hh.size();
	if (n > ncols())
		n = ncols();
	for (unsigned int i = 0; i < n; ++i)
		colinfo_[i].align_ = hh[i];
}


void MathGridInset::halign(char h, unsigned int col)
{
	colinfo_[col].align_ = h;
}


char MathGridInset::halign(unsigned int col) const
{
	return colinfo_[col].align_;
}



void MathGridInset::valign(char c)
{
	v_align_ = c;
}


char MathGridInset::valign() const
{
	return v_align_;
}



void MathGridInset::vskip(LyXLength const & skip, unsigned int row)
{
	rowinfo_[row].skip_ = skip;
}


LyXLength MathGridInset::vskip(unsigned int row) const
{
	return rowinfo_[row].skip_;
}


void MathGridInset::metrics(MathStyles st) const
{
	// let the cells adjust themselves
	MathNestInset::metrics(st);
	size_ = st;

	// adjust vertical structure
	for (unsigned int row = 0; row < nrows(); ++row) {
		int asc  = 0;
		int desc = 0;
		for (unsigned int col = 0; col < ncols(); ++col) {
			MathXArray const & c = xcell(index(row, col));
			asc  = std::max(asc,  c.ascent());
			desc = std::max(desc, c.descent());
		}
		rowinfo_[row].ascent_  = asc;
		rowinfo_[row].descent_ = desc;

		if (row) 
			rowinfo_[row].offset_ = 
				rowinfo_[row - 1].offset_ +
				rowinfo_[row - 1].descent_ +
				rowinfo_[row - 1].skipPixels() +
				MATH_ROWSEP +
				rowinfo_[row].ascent_;
		else 
			rowinfo_[row].offset_ = 0;
	}

	// adjust vertical offset
	int h = 0;
	switch (v_align_) {
	case 't':
		h = 0;
		break;
	case 'b':
		h = rowinfo_.back().offset_;
		break;
	default:
 		h = rowinfo_.back().offset_ / 2;
	}

	for (unsigned int row = 0; row < nrows(); ++row) {
		rowinfo_[row].offset_ -= h;
		rowinfo_[row].offset_ += MATH_BORDER;
	}
	
	// adjust horizontal structure
	for (unsigned int col = 0; col < ncols(); ++col) {
		int wid  = 0;
		for (unsigned int row = 0; row < nrows(); ++row) 
			wid = std::max(wid, xcell(index(row, col)).width());
		colinfo_[col].width_  = wid;
		colinfo_[col].offset_ = colinfo_[col].width_;

		if (col) 
			colinfo_[col].offset_ =
				colinfo_[col - 1].offset_ +
				colinfo_[col - 1].width_ + 
				colinfo_[col - 1].skip_;
		else
			colinfo_[col].offset_ = 0;

		colinfo_[col].offset_ += MATH_BORDER;
	}

	width_   =   colinfo_.back().offset_  + colinfo_.back().width_;
	ascent_  = - rowinfo_.front().offset_ + rowinfo_.front().ascent_;
	descent_ =   rowinfo_.back().offset_  + rowinfo_.back().descent_;
	
/*	
	// Increase ws_[i] for 'R' columns (except the first one)
	for (int i = 1; i < nc_; ++i)
		if (align_[i] == 'R')
			ws_[i] += 10 * df_width;
	// Increase ws_[i] for 'C' column
	if (align_[0] == 'C')
		if (ws_[0] < 7 * workwidth / 8)
			ws_[0] = 7 * workwidth / 8;
	
	// Adjust local tabs
	width = MATH_COLSEP;
	for (cxrow = row_.begin(); cxrow; ++cxrow) {   
		int rg = MATH_COLSEP;
		int lf = 0;
		for (int i = 0; i < nc_; ++i) {
			bool isvoid = false;
			if (cxrow->getTab(i) <= 0) {
				cxrow->setTab(i, df_width);
				isvoid = true;
			}
			switch (align_[i]) {
			case 'l':
				lf = 0;
				break;
			case 'c':
				lf = (ws_[i] - cxrow->getTab(i))/2; 
				break;
			case 'r':
			case 'R':
				lf = ws_[i] - cxrow->getTab(i);
				break;
			case 'C':
				if (cxrow == row_.begin())
					lf = 0;
				else if (cxrow.is_last())
					lf = ws_[i] - cxrow->getTab(i);
				else
					lf = (ws_[i] - cxrow->getTab(i))/2; 
				break;
			}
			int const ww = (isvoid) ? lf : lf + cxrow->getTab(i);
			cxrow->setTab(i, lf + rg);
			rg = ws_[i] - ww + MATH_COLSEP;
			if (cxrow == row_.begin())
				width += ws_[i] + MATH_COLSEP;
		}
		cxrow->setBaseline(cxrow->getBaseline() - ascent);
	}
*/
}


void MathGridInset::draw(Painter & pain, int x, int y) const
{
	xo(x);
	yo(y);
	for (unsigned int idx = 0; idx < nargs(); ++idx)
		xcell(idx).draw(pain, x + cellXOffset(idx), y + cellYOffset(idx));
}


void MathGridInset::write(std::ostream & os, bool fragile) const
{
	for (unsigned int row = 0; row < nrows(); ++row) {
		for (unsigned int col = 0; col < ncols(); ++col) {
			cell(index(row, col)).write(os, fragile);
			os << eocString(col);
		}
		os << eolString(row);
	}
}


string MathGridInset::eolString(unsigned int row) const
{
	if (row + 1 == nrows())	
		return "";

	if (rowinfo_[row].skip_.value() != 0)
		return "\\\\[" + rowinfo_[row].skip_.asLatexString() + "]\n";

	// make sure an upcoming '[' does not break anything
	MathArray const & c = cell(index(row + 1, 0));
	if (c.size() && c.begin()->nucleus()->getChar() == '[')
		return "\\\\[0pt]\n";

	return "\\\\\n";
}


string MathGridInset::eocString(unsigned int col) const
{
	if (col + 1 == ncols())
		return "";
	return " & ";
}


void MathGridInset::addRow(unsigned int row)
{
	rowinfo_.insert(rowinfo_.begin() + row + 1, RowInfo());
	cells_.insert(cells_.begin() + (row + 1) * ncols(), ncols(), MathXArray());
}


void MathGridInset::appendRow()
{
	rowinfo_.push_back(RowInfo());
	for (unsigned int i = 0; i < ncols(); ++i)
		cells_.push_back(cells_type::value_type());
}


void MathGridInset::delRow(unsigned int row)
{
	if (nrows() == 1)
		return;

	cells_type::iterator it = cells_.begin() + row * ncols(); 
	cells_.erase(it, it + ncols());

	rowinfo_.erase(rowinfo_.begin() + row);
}


void MathGridInset::addCol(unsigned int newcol)
{
	unsigned int const nc = ncols();
	unsigned int const nr = nrows();
	cells_type new_cells((nc + 1) * nr);
	
	for (unsigned int row = 0; row < nr; ++row)
		for (unsigned int col = 0; col < nc; ++col)
			new_cells[row * (nc + 1) + col + (col > newcol)]
				= cells_[row * nc + col];
	std::swap(cells_, new_cells);

	ColInfo inf;
	inf.skip_  = defaultColSpace(newcol);
	inf.align_ = defaultColAlign(newcol);
	colinfo_.insert(colinfo_.begin() + newcol, inf);
}


void MathGridInset::delCol(unsigned int col)
{
	if (ncols() == 1)
		return;

	cells_type tmpcells;
	for (unsigned int i = 0; i < nargs(); ++i) 
		if (i % ncols() != col)
			tmpcells.push_back(cells_[i]);
	std::swap(cells_, tmpcells);

	colinfo_.erase(colinfo_.begin() + col);
}


int MathGridInset::cellXOffset(unsigned int idx) const
{
	unsigned int c = col(idx);
	int x = colinfo_[c].offset_;
	char align = colinfo_[c].align_;
	if (align == 'r' || align == 'R')
		x += colinfo_[c].width_ - xcell(idx).width(); 
	if (align == 'c' || align == 'C')
		x += (colinfo_[c].width_ - xcell(idx).width()) / 2; 
	return x;
}


int MathGridInset::cellYOffset(unsigned int idx) const
{
	return rowinfo_[row(idx)].offset_;
}


bool MathGridInset::idxUp(unsigned int & idx, unsigned int & pos) const
{
	if (idx < ncols())
		return false;
	idx -= ncols();
	pos = 0;
	return true;
}

	
bool MathGridInset::idxDown(unsigned int & idx, unsigned int & pos) const
{
	if (idx >= ncols() * (nrows() - 1))
		return false;
	idx += ncols();
	pos = 0;
	return true;
}
	
	
bool MathGridInset::idxLeft(unsigned int & idx, unsigned int & pos) const
{
	// leave matrix if on the left hand edge
	if (col(idx) == 0)
		return false;
	idx--;
	pos = cell(idx).size();
	return true;
}
	
	
bool MathGridInset::idxRight(unsigned int & idx, unsigned int & pos) const
{
	// leave matrix if on the right hand edge
	if (col(idx) == ncols() - 1)
		return false;
	idx++;
	pos = 0;
	return true;
}


bool MathGridInset::idxFirst(unsigned int & idx, unsigned int & pos) const
{
	switch (v_align_) {
		case 't':
			idx = 0;
			break;
		case 'b':
			idx = (nrows() - 1) * ncols();
			break;
		default: 
			idx = (nrows() / 2) * ncols();
	}
	pos = 0;
	return true;
}


bool MathGridInset::idxLast(unsigned int & idx, unsigned int & pos) const
{
	switch (v_align_) {
		case 't':
			idx = ncols() - 1;
			break;
		case 'b':
			idx = nargs() - 1;
			break;
		default:
			idx = (nrows() / 2 + 1) * ncols() - 1;
	}
	pos = cell(idx).size();
	return true;
}


void MathGridInset::idxDelete(unsigned int & idx, bool & popit, bool & deleteit)
{
	popit    = false;
	deleteit = false;

	// delete entire row if in first cell of empty row
	if (col(idx) == 0 && nrows() > 1) {
		bool deleterow = true;
		for (unsigned int i = idx; i < idx + ncols(); ++i)
			if (cell(i).size()) {
				deleterow = false;
				break;
			}
		if (deleterow) 
			delRow(row(idx));

		if (idx >= nargs())
			idx = nargs() - 1;
		return;
	}

	// undo effect of Ctrl-Tab (i.e. pull next cell)
	//if (idx != nargs() - 1) 
	//	cell(idx).swap(cell(idx + 1));
}


void MathGridInset::idxDeleteRange(unsigned int /*from*/, unsigned int /*to*/)
{
// leave this unimplemented unless someone wants to have it.
/*
	int n = (to - from) / ncols();
	int r = from / ncols();

	if (n >= 1) {
		cells_type::iterator it = cells_.begin() + from;
		cells_.erase(it, it + n * ncols());
		rowinfo_.erase(rowinfo_.begin() + r, rowinfo_.begin() + r + n);
	}
*/
}


MathGridInset::RowInfo const & MathGridInset::rowinfo(unsigned int i) const
{
	return rowinfo_[i];
}


MathGridInset::RowInfo & MathGridInset::rowinfo(unsigned int i)
{
	return rowinfo_[i];
}


std::vector<unsigned int>
	MathGridInset::idxBetween(unsigned int from, unsigned int to) const
{
	unsigned int r1 = std::min(row(from), row(to));
	unsigned int r2 = std::max(row(from), row(to));
	unsigned int c1 = std::min(col(from), col(to));
	unsigned int c2 = std::max(col(from), col(to));
	std::vector<unsigned int> res;
	for (unsigned int i = r1; i <= r2; ++i)
		for (unsigned int j = c1; j <= c2; ++j)
			res.push_back(index(i, j));
	return res;
}
