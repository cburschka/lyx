#ifdef __GNUG__
#pragma implementation
#endif

#include "math_gridinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "lyxfont.h"
#include "funcrequest.h"
#include "frontends/Painter.h"
#include "debug.h"


using std::swap;
using std::max;
using std::min;
using std::vector;


void mathed_parse_normal(MathGridInset &, string const & argument);

namespace {

string verboseHLine(int n)
{
	string res;
	for (int i = 0; i < n; ++i)
		res += "\\hline";
	return res + ' ';
}

}

//////////////////////////////////////////////////////////////


MathGridInset::CellInfo::CellInfo()
	: dummy_(false)
{}




//////////////////////////////////////////////////////////////


MathGridInset::RowInfo::RowInfo()
	: lines_(0), skip_(0)
{}



int MathGridInset::RowInfo::skipPixels() const
{
	return crskip_.inBP();
}



//////////////////////////////////////////////////////////////


MathGridInset::ColInfo::ColInfo()
	: align_('c'), leftline_(false), rightline_(false), lines_(0)
{}


//////////////////////////////////////////////////////////////


MathGridInset::MathGridInset(char v, string const & h)
	: MathNestInset(guessColumns(h)),
	  rowinfo_(2),
	  colinfo_(guessColumns(h) + 1),
	  cellinfo_(1 * guessColumns(h))
{
	setDefaults();
	valign(v);
	halign(h);
	//lyxerr << "created grid with " << ncols() << " columns\n";
}


MathGridInset::MathGridInset()
	: MathNestInset(1),
	  rowinfo_(1 + 1),
		colinfo_(1 + 1),
		cellinfo_(1),
		v_align_('c')
{
	setDefaults();
}


MathGridInset::MathGridInset(col_type m, row_type n)
	: MathNestInset(m * n),
	  rowinfo_(n + 1),
		colinfo_(m + 1),
		cellinfo_(m * n),
		v_align_('c')
{
	setDefaults();
}


MathGridInset::MathGridInset(col_type m, row_type n, char v, string const & h)
	: MathNestInset(m * n),
	  rowinfo_(n + 1),
	  colinfo_(m + 1),
		cellinfo_(m * n),
		v_align_(v)
{
	setDefaults();
	valign(v);
	halign(h);
}


MathInset * MathGridInset::clone() const
{
	return new MathGridInset(*this);
}


MathInset::idx_type MathGridInset::index(row_type row, col_type col) const
{
	return col + ncols() * row;
}


void MathGridInset::setDefaults()
{
	if (ncols() <= 0)
		lyxerr << "positive number of columns expected\n";
	//if (nrows() <= 0)
	//	lyxerr << "positive number of rows expected\n";
	for (col_type col = 0; col < ncols(); ++col) {
		colinfo_[col].align_ = defaultColAlign(col);
		colinfo_[col].skip_  = defaultColSpace(col);
	}
}


void MathGridInset::halign(string const & hh)
{
	col_type col = 0;
	for (string::const_iterator it = hh.begin(); it != hh.end(); ++it) {
		if (col >= ncols())
			break;
		char c = *it;
		if (c == '|') {
			colinfo_[col].lines_++;
		} else if (c == 'c' || c == 'l' || c == 'r') {
			colinfo_[col].align_ = c;
			++col;
			colinfo_[col].lines_ = 0;
		} else {
			lyxerr << "unknown column separator: '" << c << "'\n";
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
	// let's have at least one column, even if we did not recognize its 
	// alignment
	if (col == 0)
		col = 1;
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


void MathGridInset::metrics(MathMetricsInfo & mi) const
{
	// let the cells adjust themselves
	MathNestInset::metrics(mi);

	// compute absolute sizes of vertical structure
	for (row_type row = 0; row < nrows(); ++row) {
		int asc  = 0;
		int desc = 0;
		for (col_type col = 0; col < ncols(); ++col) {
			MathArray const & c = cell(index(row, col));
			asc  = max(asc,  c.ascent());
			desc = max(desc, c.descent());
		}
		rowinfo_[row].ascent_  = asc;
		rowinfo_[row].descent_ = desc;
	}
	rowinfo_[0].ascent_       += hlinesep() * rowinfo_[0].lines_;
	rowinfo_[nrows()].ascent_  = 0;
	rowinfo_[nrows()].descent_ = 0;

	// compute vertical offsets
	rowinfo_[0].offset_ = 0;
	for (row_type row = 1; row <= nrows(); ++row) {
		rowinfo_[row].offset_  =
			rowinfo_[row - 1].offset_  +
			rowinfo_[row - 1].descent_ +
			rowinfo_[row - 1].skipPixels() +
			rowsep() +
			rowinfo_[row].lines_ * hlinesep() +
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
			wid = max(wid, cell(index(row, col)).width());
		colinfo_[col].width_ = wid;
	}
	colinfo_[ncols()].width_  = 0;

	// compute horizontal offsets
	colinfo_[0].offset_ = border();
	for (col_type col = 1; col <= ncols(); ++col) {
		colinfo_[col].offset_ =
			colinfo_[col - 1].offset_ +
			colinfo_[col - 1].width_ +
			colinfo_[col - 1].skip_ +
			colsep() +
			colinfo_[col].lines_ * vlinesep();
	}


	dim_.w   =   colinfo_[ncols() - 1].offset_
		       + colinfo_[ncols() - 1].width_
		 + vlinesep() * colinfo_[ncols()].lines_
		       + border();

	dim_.a  = - rowinfo_[0].offset_
		       + rowinfo_[0].ascent_
		 + hlinesep() * rowinfo_[0].lines_
		       + border();

	dim_.d =   rowinfo_[nrows() - 1].offset_
		       + rowinfo_[nrows() - 1].descent_
		 + hlinesep() * rowinfo_[nrows()].lines_
		       + border();


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
	width = colsep();
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
			rg = ws_[i] - ww + colsep();
			if (cxrow == row_.begin())
				width += ws_[i] + colsep();
		}
		cxrow->setBaseline(cxrow->getBaseline() - ascent);
	}
*/
}


void MathGridInset::draw(MathPainterInfo & pi, int x, int y) const
{
	for (idx_type idx = 0; idx < nargs(); ++idx)
		cell(idx).draw(pi, x + cellXOffset(idx), y + cellYOffset(idx));

	for (row_type row = 0; row <= nrows(); ++row)
		for (int i = 0; i < rowinfo_[row].lines_; ++i) {
			int yy = y + rowinfo_[row].offset_ - rowinfo_[row].ascent_
				- i * hlinesep() - hlinesep()/2 - rowsep()/2;
			pi.pain.line(x + 1, yy, x + width() - 1, yy);
		}

	for (col_type col = 0; col <= ncols(); ++col)
		for (int i = 0; i < colinfo_[col].lines_; ++i) {
			int xx = x + colinfo_[col].offset_
				- i * vlinesep() - vlinesep()/2 - colsep()/2;
			pi.pain.line(xx, y - ascent() + 1, xx, y + descent() - 1);
		}
}


void MathGridInset::metricsT(TextMetricsInfo const & mi) const
{
	// let the cells adjust themselves
	//MathNestInset::metrics(mi);
	for (idx_type i = 0; i < nargs(); ++i)
		cell(i).metricsT(mi);

	// compute absolute sizes of vertical structure
	for (row_type row = 0; row < nrows(); ++row) {
		int asc  = 0;
		int desc = 0;
		for (col_type col = 0; col < ncols(); ++col) {
			MathArray const & c = cell(index(row, col));
			asc  = max(asc,  c.ascent());
			desc = max(desc, c.descent());
		}
		rowinfo_[row].ascent_  = asc;
		rowinfo_[row].descent_ = desc;
	}
	//rowinfo_[0].ascent_       += hlinesep() * rowinfo_[0].lines_;
	rowinfo_[nrows()].ascent_  = 0;
	rowinfo_[nrows()].descent_ = 0;

	// compute vertical offsets
	rowinfo_[0].offset_ = 0;
	for (row_type row = 1; row <= nrows(); ++row) {
		rowinfo_[row].offset_  =
			rowinfo_[row - 1].offset_  +
			rowinfo_[row - 1].descent_ +
			//rowinfo_[row - 1].skipPixels() +
			1 + //rowsep() +
			//rowinfo_[row].lines_ * hlinesep() +
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
			wid = max(wid, cell(index(row, col)).width());
		colinfo_[col].width_ = wid;
	}
	colinfo_[ncols()].width_  = 0;

	// compute horizontal offsets
	colinfo_[0].offset_ = border();
	for (col_type col = 1; col <= ncols(); ++col) {
		colinfo_[col].offset_ =
			colinfo_[col - 1].offset_ +
			colinfo_[col - 1].width_ +
			colinfo_[col - 1].skip_ +
			1 ; //colsep() +
			//colinfo_[col].lines_ * vlinesep();
	}


	dim_.w  =  colinfo_[ncols() - 1].offset_
		       + colinfo_[ncols() - 1].width_
		 //+ vlinesep() * colinfo_[ncols()].lines_
		       + 2;

	dim_.a  = -rowinfo_[0].offset_
		       + rowinfo_[0].ascent_
		 //+ hlinesep() * rowinfo_[0].lines_
		       + 1;

	dim_.d  =  rowinfo_[nrows() - 1].offset_
		       + rowinfo_[nrows() - 1].descent_
		 //+ hlinesep() * rowinfo_[nrows()].lines_
		       + 1;

}


void MathGridInset::drawT(TextPainter & pain, int x, int y) const
{
	for (idx_type idx = 0; idx < nargs(); ++idx)
		cell(idx).drawT(pain, x + cellXOffset(idx), y + cellYOffset(idx));
}


string MathGridInset::eolString(row_type row, bool fragile) const
{
	string eol;

	if (!rowinfo_[row].crskip_.zero())
		eol += "[" + rowinfo_[row].crskip_.asLatexString() + "]";

	// make sure an upcoming '[' does not break anything
	if (row + 1 < nrows()) {
		MathArray const & c = cell(index(row + 1, 0));
		if (c.size() && c.front()->getChar() == '[')
			//eol += "[0pt]";
			eol += "{}";
	}

	// only add \\ if necessary
	if (eol.empty() && row + 1 == nrows())
		return string();

	return (fragile ? "\\protect\\\\" : "\\\\") + eol + '\n';
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
	cells_.insert
		(cells_.begin() + (row + 1) * ncols(), ncols(), MathArray());
	cellinfo_.insert
		(cellinfo_.begin() + (row + 1) * ncols(), ncols(), CellInfo());
}


void MathGridInset::appendRow()
{
	rowinfo_.push_back(RowInfo());
	//cells_.insert(cells_.end(), ncols(), MathArray());
	for (col_type col = 0; col < ncols(); ++col) {
		cells_.push_back(cells_type::value_type());
		cellinfo_.push_back(CellInfo());
	}
}


void MathGridInset::delRow(row_type row)
{
	if (nrows() == 1)
		return;

	cells_type::iterator it = cells_.begin() + row * ncols();
	cells_.erase(it, it + ncols());

	vector<CellInfo>::iterator jt = cellinfo_.begin() + row * ncols();
	cellinfo_.erase(jt, jt + ncols());

	rowinfo_.erase(rowinfo_.begin() + row);
}


void MathGridInset::addCol(col_type newcol)
{
	const col_type nc = ncols();
	const row_type nr = nrows();
	cells_type new_cells((nc + 1) * nr);
	vector<CellInfo> new_cellinfo((nc + 1) * nr);

	for (row_type row = 0; row < nr; ++row)
		for (col_type col = 0; col < nc; ++col) {
			new_cells[row * (nc + 1) + col + (col > newcol)]
				= cells_[row * nc + col];
			new_cellinfo[row * (nc + 1) + col + (col > newcol)]
				= cellinfo_[row * nc + col];
		}
	swap(cells_, new_cells);
	swap(cellinfo_, new_cellinfo);

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
	vector<CellInfo> tmpcellinfo;
	for (col_type i = 0; i < nargs(); ++i)
		if (i % ncols() != col) {
			tmpcells.push_back(cells_[i]);
			tmpcellinfo.push_back(cellinfo_[i]);
		}
	swap(cells_, tmpcells);
	swap(cellinfo_, tmpcellinfo);

	colinfo_.erase(colinfo_.begin() + col);
}


int MathGridInset::cellXOffset(idx_type idx) const
{
	col_type c = col(idx);
	int x = colinfo_[c].offset_;
	char align = colinfo_[c].align_;
	if (align == 'r' || align == 'R')
		x += colinfo_[c].width_ - cell(idx).width();
	if (align == 'c' || align == 'C')
		x += (colinfo_[c].width_ - cell(idx).width()) / 2;
	return x;
}


int MathGridInset::cellYOffset(idx_type idx) const
{
	return rowinfo_[row(idx)].offset_;
}


bool MathGridInset::idxUpDown(idx_type & idx, pos_type & pos, bool up,
	int targetx) const
{
	if (up) {
		if (idx < ncols())
			return false;
		idx -= ncols();
		pos = cell(idx).x2pos(targetx - cell(idx).xo());
		return true;
	} else {
		if (idx >= ncols() * (nrows() - 1))
			return false;
		idx += ncols();
		pos = cell(idx).x2pos(targetx - cell(idx).xo());
		return true;
	}
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
			idx = ((nrows() - 1) / 2) * ncols();
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
			idx = ((nrows() - 1) / 2 + 1) * ncols() - 1;
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


bool MathGridInset::idxDelete(idx_type & idx)
{
	// nothing to do if we have just one row
	if (nrows() == 1)
		return false;

	// nothing to do if we are in the middle of the last row of the inset
	if (idx + ncols() > nargs())
		return false;

	// try to delete entire sequence of ncols() empty cells if possible
	for (idx_type i = idx; i < idx + ncols(); ++i)
		if (cell(i).size())
			return false;

	// move cells if necessary
	for (idx_type i = index(row(idx), 0); i < idx; ++i)
		std::swap(cell(i), cell(i + ncols()));

	delRow(row(idx));

	if (idx >= nargs())
		idx = nargs() - 1;

	// undo effect of Ctrl-Tab (i.e. pull next cell)
	//if (idx + 1 != nargs())
	//	cell(idx).swap(cell(idx + 1));

	// we handled the event..
	return true;
}


// reimplement old behaviour when pressing Delete in the last position
// of a cell
void MathGridInset::idxGlue(idx_type idx)
{
	col_type c = col(idx);
	if (c + 1 == ncols()) {
		if (row(idx) + 1 != nrows()) {
			for (col_type cc = 0; cc < ncols(); ++cc)
				cell(idx).append(cell(idx + cc + 1));
			delRow(row(idx) + 1);
		}
	} else {
		cell(idx).append(cell(idx + 1));
		for (col_type cc = c + 2; cc < ncols(); ++cc)
			cell(idx - c + cc - 1) = cell(idx - c + cc);
		cell(idx - c + ncols() - 1).clear();
	}
}


MathGridInset::RowInfo const & MathGridInset::rowinfo(row_type row) const
{
	return rowinfo_[row];
}


MathGridInset::RowInfo & MathGridInset::rowinfo(row_type row)
{
	return rowinfo_[row];
}


bool MathGridInset::idxBetween(idx_type idx, idx_type from, idx_type to) const
{
	row_type const ri = row(idx);
	row_type const r1 = min(row(from), row(to));
	row_type const r2 = max(row(from), row(to));
	col_type const ci = col(idx);
	col_type const c1 = min(col(from), col(to));
	col_type const c2 = max(col(from), col(to));
	return r1 <= ri && ri <= r2 && c1 <= ci && ci <= c2;
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
		// don't write & and empty cells at end of line
		for (col_type col = 0; col < ncols(); ++col)
			os << cell(index(row, col)) << eocString(col);
		os << eolString(row, os.fragile());
	}
	string const s = verboseHLine(rowinfo_[nrows()].lines_);
	if (!s.empty() && s != " ") {
		if (os.fragile())
			os << "\\protect";
		os << "\\\\" << s;
	}
}


int MathGridInset::colsep() const
{
	return 6;
}


int MathGridInset::rowsep() const
{
	return 6;
}


int MathGridInset::hlinesep() const
{
	return 3;
}


int MathGridInset::vlinesep() const
{
	return 3;
}


int MathGridInset::border() const
{
	return 1;
}


void MathGridInset::splitCell(idx_type & idx, pos_type & pos)
{
	if (idx + 1 == nargs())
		return;
	MathArray ar = cell(idx);
	ar.erase(0, pos);
	cell(idx).erase(pos, cell(idx).size());
	++idx;
	pos = 0;
	cell(idx).insert(0, ar);
}


MathInset::result_type MathGridInset::dispatch
	(FuncRequest const & cmd, idx_type & idx, pos_type & pos)
{
	switch (cmd.action) {

		case LFUN_DELETE_LINE_FORWARD:
			//autocorrect_ = false;
			//macroModeClose();
			//if (selection_) {
			//	selDel();
			//	return;
			//}
			if (nrows() > 1)
				delRow(row(idx));
			if (idx >= nargs())
				idx = nargs() - 1;
			if (pos > cell(idx).size())
				pos = cell(idx).size();
			return DISPATCHED_POP;

		case LFUN_TABINSERT:
			//bv->lockedInsetStoreUndo(Undo::EDIT);
			splitCell(idx, pos);
			//updateLocal(bv, true);
			return DISPATCHED_POP;

		case LFUN_BREAKLINE: {
			//bv->lockedInsetStoreUndo(Undo::INSERT);
			row_type const r = row(idx);
			addRow(r);

			// split line
			for (col_type c = col(idx) + 1; c < ncols(); ++c)
				std::swap(cell(index(r, c)), cell(index(r + 1, c)));

			// split cell
			splitCell(idx, pos);
			std::swap(cell(idx), cell(idx + ncols() - 1));
		
			//mathcursor->normalize();
			//updateLocal(bv, true);
			return DISPATCHED_POP;
		}

		case LFUN_MATH_HALIGN:
			halign((cmd.argument + "c")[0], col(idx));
			return DISPATCHED_POP;

		case LFUN_MATH_VALIGN:
			valign((cmd.argument + "c")[0]);
			return DISPATCHED_POP;

		case LFUN_MATH_ROW_INSERT:
			addRow(row(idx));
			return DISPATCHED_POP;

		case LFUN_MATH_ROW_DELETE:
			delRow(row(idx));
			if (idx > nargs())
				idx -= ncols();
			return DISPATCHED_POP;

		case LFUN_MATH_COLUMN_INSERT: {
			row_type r = row(idx);
			col_type c = col(idx);
			addFancyCol(c);
			idx = index(r, c);
			return DISPATCHED_POP;
		}

		case LFUN_MATH_COLUMN_DELETE: {
			row_type r = row(idx);
			col_type c = col(idx);
			delFancyCol(col(idx));
			idx = index(r, c);
			if (idx > nargs())
				idx -= ncols();
			return DISPATCHED_POP;
		}

		case LFUN_PASTE: {
			//lyxerr << "pasting '" << cmd.argument << "'\n";
			MathGridInset grid(1, 1);
			mathed_parse_normal(grid, cmd.argument);
			if (grid.nargs() == 1) {
				// single cell/part of cell
				cell(idx).insert(pos, grid.cell(0));
				pos += grid.cell(0).size();
			} else {
				// multiple cells
				col_type const numcols = min(grid.ncols(), ncols() - col(idx));
				row_type const numrows = min(grid.nrows(), nrows() - row(idx));
				for (row_type r = 0; r < numrows; ++r) {
					for (col_type c = 0; c < numcols; ++c) {
						idx_type i = index(r + row(idx), c + col(idx));
						cell(i).append(grid.cell(grid.index(r, c)));
					}
					// append the left over horizontal cells to the last column
					idx_type i = index(r + row(idx), ncols() - 1);
					for (MathInset::col_type c = numcols; c < grid.ncols(); ++c)
						cell(i).append(grid.cell(grid.index(r, c)));
				}
				// append the left over vertical cells to the last _cell_
				idx_type i = nargs() - 1;
				for (row_type r = numrows; r < grid.nrows(); ++r)
					for (col_type c = 0; c < grid.ncols(); ++c)
						cell(i).append(grid.cell(grid.index(r, c)));
			}
			return DISPATCHED_POP;
		}

		default:	
			return MathNestInset::dispatch(cmd, idx, pos);
	}
	return UNDISPATCHED;
}
