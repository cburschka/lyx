#ifdef __GNUG__
#pragma implementation
#endif

#include "math_gridinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "lyxfont.h"
#include "Painter.h"
#include "debug.h"


namespace {

///
int const COLSEP = 6;
///
int const ROWSEP = 6;
///
int const HLINESEP = 3;
///
int const VLINESEP = 3;
///
int const BORDER = 2;


string verboseHLine(int n)
{
	string res;
	for (int i = 0; i < n; ++i)
		res += "\\hline";
	return res + ' ';
}

}


////////////////////////////////////////////////////////////// 


MathGridInset::RowInfo::RowInfo()
	: lines_(0), skip_(0)
{}



int MathGridInset::RowInfo::skipPixels() const
{
#ifdef WITH_WARNINGS
#warning fix this once the interface to LyXLength has improved
#endif
	return int(crskip_.value());
}



////////////////////////////////////////////////////////////// 


MathGridInset::ColInfo::ColInfo()
	: align_('c'), leftline_(false), rightline_(false), lines_(0)
{}


////////////////////////////////////////////////////////////// 


MathGridInset::MathGridInset(char v, string const & h)
	: MathNestInset(guessColumns(h)), rowinfo_(2), colinfo_(guessColumns(h) + 1)
{
	setDefaults();
 	valign(v);
	halign(h);
}


MathGridInset::MathGridInset(col_type m, row_type n)
	: MathNestInset(m * n), rowinfo_(n + 1), colinfo_(m + 1), v_align_('c')
{
	setDefaults();
}


MathGridInset::MathGridInset(col_type m, row_type n, char v, string const & h)
	: MathNestInset(m * n), rowinfo_(n + 1), colinfo_(m + 1), v_align_(v)
{
	setDefaults();
 	valign(v);
	halign(h);
}


MathInset::idx_type MathGridInset::index(row_type row, col_type col) const
{
	return col + ncols() * row;
}


void MathGridInset::setDefaults()
{
	if (ncols() <= 0)
		lyxerr << "positive number of columns expected\n";
	if (nrows() <= 0)
		lyxerr << "positive number of rows expected\n";
	for (col_type col = 0; col < ncols(); ++col) {
		colinfo_[col].align_ = defaultColAlign(col);
		colinfo_[col].skip_  = defaultColSpace(col);
	}
}


void MathGridInset::halign(string const & hh)
{
	col_type col = 0;
	for (string::const_iterator it = hh.begin(); it != hh.end(); ++it) {
		char c = *it;
		if (c == '|') {
			colinfo_[col].lines_++;
		} else if (c == 'c' || c == 'l' || c == 'r') {
			colinfo_[col].align_ = c;
			++col;
			colinfo_[col].lines_ = 0;
		} else {
			lyxerr << "unkown column separator: '" << c << "'\n";
		}
	}
			
/*
	col_type n = hh.size();
	if (n > ncols())
		n = ncols();
	for (col_type col = 0; col < n; ++col)
		colinfo_[col].align_ = hh[col];
*/
}


MathGridInset::col_type MathGridInset::guessColumns(string const & hh) const
{
	col_type col = 0;
	for (string::const_iterator it = hh.begin(); it != hh.end(); ++it)
		if (*it == 'c' || *it == 'l' || *it == 'r')
			++col;
	return col;
}


void MathGridInset::halign(char h, col_type col)
{
	colinfo_[col].align_ = h;
}


char MathGridInset::halign(col_type col) const
{
	return colinfo_[col].align_;
}


string MathGridInset::halign() const
{
	string res;
	for (col_type col = 0; col < ncols(); ++col) {
		res += string(colinfo_[col].lines_, '|');
		res += colinfo_[col].align_;
	} 
	return res + string(colinfo_[ncols()].lines_, '|');
}


void MathGridInset::valign(char c)
{
	v_align_ = c;
}


char MathGridInset::valign() const
{
	return v_align_;
}


MathGridInset::col_type MathGridInset::ncols() const
{
	return colinfo_.size() - 1;
}


MathGridInset::row_type MathGridInset::nrows() const
{
	return rowinfo_.size() - 1;
}


MathGridInset::col_type MathGridInset::col(idx_type idx) const
{
	return idx % ncols();
}


MathGridInset::row_type MathGridInset::row(idx_type idx) const
{
	return idx / ncols();
}


void MathGridInset::vcrskip(LyXLength const & crskip, row_type row)
{
	rowinfo_[row].crskip_ = crskip;
}


LyXLength MathGridInset::vcrskip(row_type row) const
{
	return rowinfo_[row].crskip_;
}


void MathGridInset::metrics(MathMetricsInfo const & mi) const
{
	// let the cells adjust themselves
	MathNestInset::metrics(mi);

	// compute absolute sizes of vertical structure
	for (row_type row = 0; row < nrows(); ++row) {
		int asc  = 0;
		int desc = 0;
		for (col_type col = 0; col < ncols(); ++col) {
			MathXArray const & c = xcell(index(row, col));
			asc  = std::max(asc,  c.ascent());
			desc = std::max(desc, c.descent());
		}
		rowinfo_[row].ascent_  = asc;
		rowinfo_[row].descent_ = desc;
	}
	rowinfo_[0].ascent_       += HLINESEP * rowinfo_[0].lines_;
	rowinfo_[nrows()].ascent_  = 0;
	rowinfo_[nrows()].descent_ = 0;

	// compute vertical offsets
	rowinfo_[0].offset_ = 0;
	for (row_type row = 1; row <= nrows(); ++row) {
		rowinfo_[row].offset_  =	
			rowinfo_[row - 1].offset_  +
			rowinfo_[row - 1].descent_ +
			rowinfo_[row - 1].skipPixels() +
			ROWSEP +
			rowinfo_[row].lines_ * HLINESEP +
			rowinfo_[row].ascent_;
	}

	// adjust vertical offset
	int h = 0;
	switch (v_align_) {
		case 't':
			h = 0;
			break;
		case 'b':
			h = rowinfo_[nrows() - 1].offset_;
			break;
		default:
			h = rowinfo_[nrows() - 1].offset_ / 2;
	}
	for (row_type row = 0; row <= nrows(); ++row)
		rowinfo_[row].offset_ -= h;
	

	// compute absolute sizes of horizontal structure
	for (col_type col = 0; col < ncols(); ++col) {
		int wid = 0;
		for (row_type row = 0; row < nrows(); ++row) 
			wid = std::max(wid, xcell(index(row, col)).width());
		colinfo_[col].width_ = wid;
	}
	colinfo_[ncols()].width_  = 0;

	// compute horizontal offsets
	colinfo_[0].offset_ = BORDER;
	for (col_type col = 1; col <= ncols(); ++col) {
		colinfo_[col].offset_ =
			colinfo_[col - 1].offset_ +
			colinfo_[col - 1].width_ + 
			colinfo_[col - 1].skip_ +
			COLSEP + 
			colinfo_[col].lines_ * VLINESEP;
	}


	width_   =   colinfo_[ncols() - 1].offset_      
	               + colinfo_[ncols() - 1].width_
                 + VLINESEP * colinfo_[ncols()].lines_
	               + BORDER;

	ascent_  = - rowinfo_[0].offset_          
	               + rowinfo_[0].ascent_
                 + HLINESEP * rowinfo_[0].lines_
	               + BORDER;

	descent_ =   rowinfo_[nrows() - 1].offset_
	               + rowinfo_[nrows() - 1].descent_
                 + HLINESEP * rowinfo_[nrows()].lines_
	               + BORDER;


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
	width = COLSEP;
	for (cxrow = row_.begin(); cxrow; ++cxrow) {   
		int rg = COLSEP;
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
			rg = ws_[i] - ww + COLSEP;
			if (cxrow == row_.begin())
				width += ws_[i] + COLSEP;
		}
		cxrow->setBaseline(cxrow->getBaseline() - ascent);
	}
*/
}


void MathGridInset::draw(Painter & pain, int x, int y) const
{
	for (idx_type idx = 0; idx < nargs(); ++idx)
		xcell(idx).draw(pain, x + cellXOffset(idx), y + cellYOffset(idx));

	for (row_type row = 0; row <= nrows(); ++row)
		for (int i = 0; i < rowinfo_[row].lines_; ++i) {
			int yy = y + rowinfo_[row].offset_ - rowinfo_[row].ascent_
				- i * HLINESEP - HLINESEP/2 - ROWSEP/2;
			pain.line(x + 1, yy, x + width_ - 1, yy);
		}

	for (col_type col = 0; col <= ncols(); ++col)
		for (int i = 0; i < colinfo_[col].lines_; ++i) {
			int xx = x + colinfo_[col].offset_
				- i * VLINESEP - VLINESEP/2 - COLSEP/2;
			pain.line(xx, y - ascent_ + 1, xx, y + descent_ - 1);
		}
}


string MathGridInset::eolString(row_type row) const
{
	string eol;

	if (!rowinfo_[row].crskip_.zero())
		eol += "[" + rowinfo_[row].crskip_.asLatexString() + "]";

	// make sure an upcoming '[' does not break anything
	if (row + 1 < nrows()) {
		MathArray const & c = cell(index(row + 1, 0));
		if (c.size() && c.front()->getChar() == '[')
			eol += "[0pt]";
	}

	// only add \\ if necessary
	if (eol.empty() && row + 1 == nrows())
		return string();

	return "\\\\" + eol + '\n';
}


string MathGridInset::eocString(col_type col) const
{
	if (col + 1 == ncols())
		return string();
	return " & ";
}


void MathGridInset::addRow(row_type row)
{
	rowinfo_.insert(rowinfo_.begin() + row + 1, RowInfo());
	cells_.insert(cells_.begin() + (row + 1) * ncols(), ncols(), MathXArray());
}


void MathGridInset::appendRow()
{
	rowinfo_.push_back(RowInfo());
	//cells_.insert(cells_.end(), ncols(), MathXArray());
	for (col_type col = 0; col < ncols(); ++col)
		cells_.push_back(cells_type::value_type());
}


void MathGridInset::delRow(row_type row)
{
	if (nrows() == 1)
		return;

	cells_type::iterator it = cells_.begin() + row * ncols(); 
	cells_.erase(it, it + ncols());

	rowinfo_.erase(rowinfo_.begin() + row);
}


void MathGridInset::addCol(col_type newcol)
{
	const col_type nc = ncols();
	const row_type nr = nrows();
	cells_type new_cells((nc + 1) * nr);
	
	for (row_type row = 0; row < nr; ++row)
		for (col_type col = 0; col < nc; ++col)
			new_cells[row * (nc + 1) + col + (col > newcol)]
				= cells_[row * nc + col];
	std::swap(cells_, new_cells);

	ColInfo inf;
	inf.skip_  = defaultColSpace(newcol);
	inf.align_ = defaultColAlign(newcol);
	colinfo_.insert(colinfo_.begin() + newcol, inf);
}


void MathGridInset::delCol(col_type col)
{
	if (ncols() == 1)
		return;

	cells_type tmpcells;
	for (col_type i = 0; i < nargs(); ++i) 
		if (i % ncols() != col)
			tmpcells.push_back(cells_[i]);
	std::swap(cells_, tmpcells);

	colinfo_.erase(colinfo_.begin() + col);
}


int MathGridInset::cellXOffset(idx_type idx) const
{
	col_type c = col(idx);
	int x = colinfo_[c].offset_;
	char align = colinfo_[c].align_;
	if (align == 'r' || align == 'R')
		x += colinfo_[c].width_ - xcell(idx).width(); 
	if (align == 'c' || align == 'C')
		x += (colinfo_[c].width_ - xcell(idx).width()) / 2; 
	return x;
}


int MathGridInset::cellYOffset(idx_type idx) const
{
	return rowinfo_[row(idx)].offset_;
}


bool MathGridInset::idxUp(idx_type & idx, pos_type & pos) const
{
	if (idx < ncols())
		return false;
	idx -= ncols();
	return true;
}

	
bool MathGridInset::idxDown(idx_type & idx, pos_type & pos) const
{
	if (idx >= ncols() * (nrows() - 1))
		return false;
	idx += ncols();
	return true;
}
	
	
bool MathGridInset::idxLeft(idx_type & idx, pos_type & pos) const
{
	// leave matrix if on the left hand edge
	if (col(idx) == 0)
		return false;
	--idx;
	pos = cell(idx).size();
	return true;
}
	
	
bool MathGridInset::idxRight(idx_type & idx, pos_type & pos) const
{
	// leave matrix if on the right hand edge
	if (col(idx) + 1 == ncols())
		return false;
	++idx;
	pos = 0;
	return true;
}


bool MathGridInset::idxFirst(idx_type & idx, pos_type & pos) const
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


bool MathGridInset::idxLast(idx_type & idx, pos_type & pos) const
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


bool MathGridInset::idxHome(idx_type & idx, pos_type & pos) const
{
	if (pos > 0) {
		pos = 0;
		return true;
	}
	if (col(idx) > 0) {
		idx -= idx % ncols();
		pos = 0;
		return true;
	}
	if (idx > 0) {
		idx = 0;
		pos = 0;
		return true;
	}
	return false;
}


bool MathGridInset::idxEnd(idx_type & idx, pos_type & pos) const
{
	if (pos < cell(idx).size()) {
		pos = cell(idx).size();
		return true;
	}
	if (col(idx) < ncols() - 1) {
		idx = idx - idx % ncols() + ncols() - 1;
		pos = cell(idx).size();
		return true;
	}
	if (idx < nargs() - 1) {
		idx = nargs() - 1;
		pos = cell(idx).size();
		return true;
	}
	return false;
}


void MathGridInset::idxDelete(idx_type & idx, bool & popit, bool & deleteit)
{
	popit    = false;
	deleteit = false;

	// nothing to do if we are in the last row of the inset
	if (row(idx) + 1 == nrows())
		return;

	// try to delete entire sequence of ncols() empty cells if possible
	for (idx_type i = idx; i < idx + ncols(); ++i)
		if (cell(i).size())
			return;

	// move cells if necessary
	for (idx_type i = index(row(idx), 0); i < idx; ++i)
		cell(i).swap(cell(i + ncols()));
		
	delRow(row(idx));

	if (idx >= nargs())
		idx = nargs() - 1;

	// undo effect of Ctrl-Tab (i.e. pull next cell)
	//if (idx + 1 != nargs()) 
	//	cell(idx).swap(cell(idx + 1));
}


void MathGridInset::idxDeleteRange(idx_type /*from*/, idx_type /*to*/)
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


MathGridInset::RowInfo const & MathGridInset::rowinfo(row_type row) const
{
	return rowinfo_[row];
}


MathGridInset::RowInfo & MathGridInset::rowinfo(row_type row)
{
	return rowinfo_[row];
}


std::vector<MathInset::idx_type>
	MathGridInset::idxBetween(idx_type from, idx_type to) const
{
	row_type r1 = std::min(row(from), row(to));
	row_type r2 = std::max(row(from), row(to));
	col_type c1 = std::min(col(from), col(to));
	col_type c2 = std::max(col(from), col(to));
	std::vector<idx_type> res;
	for (row_type i = r1; i <= r2; ++i)
		for (col_type j = c1; j <= c2; ++j)
			res.push_back(index(i, j));
	return res;
}



void MathGridInset::normalize(NormalStream & os) const
{
	os << "[grid ";
	for (row_type row = 0; row < nrows(); ++row) {
		os << "[row ";
		for (col_type col = 0; col < ncols(); ++col)
			os << "[cell " << cell(index(row, col)) << ']';
		os << ']';
	}
	os << ']';
}


void MathGridInset::mathmlize(MathMLStream & os) const
{
	os << MTag("mtable");
	for (row_type row = 0; row < nrows(); ++row) {
		os << MTag("mtr");
		for (col_type col = 0; col < ncols(); ++col) 
			os << cell(index(row, col));
		os << ETag("mtr");
	}
	os << ETag("mtable");
}


void MathGridInset::write(WriteStream & os) const
{
	for (row_type row = 0; row < nrows(); ++row) {
		os << verboseHLine(rowinfo_[row].lines_);
		for (col_type col = 0; col < ncols(); ++col) 
			os << cell(index(row, col)) << eocString(col);
		os << eolString(row);
	}
	string const s = verboseHLine(rowinfo_[nrows()].lines_);
	if (!s.empty())
		os << "\\\\" << s;
}

