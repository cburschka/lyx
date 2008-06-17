/**
 * \file InsetMathGrid.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include <algorithm>

#include "InsetMathGrid.h"

#include "MathData.h"
#include "MathParser.h"
#include "MathStream.h"
#include "MetricsInfo.h"

#include "BufferView.h"
#include "CutAndPaste.h"
#include "FuncStatus.h"
#include "Cursor.h"
#include "FuncRequest.h"

#include "frontends/Clipboard.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include "support/lassert.h"

#include <sstream>

using namespace std;
using namespace lyx::support;


namespace lyx {

static docstring verboseHLine(int n)
{
	docstring res;
	for (int i = 0; i < n; ++i)
		res += "\\hline";
	if (n)
		res += ' ';
	return res;
}


static int extractInt(istream & is)
{
	int num = 1;
	is >> num;
	return (num == 0) ? 1 : num;
}



//////////////////////////////////////////////////////////////


InsetMathGrid::CellInfo::CellInfo()
	: dummy_(false)
{}



//////////////////////////////////////////////////////////////


InsetMathGrid::RowInfo::RowInfo()
	: lines_(0), skip_(0), allow_newpage_(true)
{}



int InsetMathGrid::RowInfo::skipPixels() const
{
	return crskip_.inBP();
}



//////////////////////////////////////////////////////////////


InsetMathGrid::ColInfo::ColInfo()
	: align_('c'), lines_(0)
{}


//////////////////////////////////////////////////////////////


InsetMathGrid::InsetMathGrid()
	: InsetMathNest(1),
	  rowinfo_(1 + 1),
		colinfo_(1 + 1),
		cellinfo_(1),
		v_align_('c')
{
	setDefaults();
}


InsetMathGrid::InsetMathGrid(col_type m, row_type n)
	: InsetMathNest(m * n),
	  rowinfo_(n + 1),
		colinfo_(m + 1),
		cellinfo_(m * n),
		v_align_('c')
{
	setDefaults();
}


InsetMathGrid::InsetMathGrid(col_type m, row_type n, char v, docstring const & h)
	: InsetMathNest(m * n),
	  rowinfo_(n + 1),
	  colinfo_(m + 1),
		cellinfo_(m * n),
		v_align_(v)
{
	setDefaults();
	setVerticalAlignment(v);
	setHorizontalAlignments(h);
}


Inset * InsetMathGrid::clone() const
{
	return new InsetMathGrid(*this);
}


InsetMath::idx_type InsetMathGrid::index(row_type row, col_type col) const
{
	return col + ncols() * row;
}


void InsetMathGrid::setDefaults()
{
	if (ncols() <= 0)
		lyxerr << "positive number of columns expected" << endl;
	//if (nrows() <= 0)
	//	lyxerr << "positive number of rows expected" << endl;
	for (col_type col = 0; col < ncols(); ++col) {
		colinfo_[col].align_ = defaultColAlign(col);
		colinfo_[col].skip_  = defaultColSpace(col);
		colinfo_[col].special_.clear();
	}
}


void InsetMathGrid::setHorizontalAlignments(docstring const & hh)
{
	col_type col = 0;
	for (docstring::const_iterator it = hh.begin(); it != hh.end(); ++it) {
		char_type c = *it;
		if (c == '|') {
			colinfo_[col].lines_++;
		} else if ((c == 'p' || c == 'm' || c == 'b'||
		            c == '!' || c == '@' || c == '>' || c == '<') &&
		           it + 1 != hh.end() && *(it + 1) == '{') {
			// @{decl.} and p{width} are standard LaTeX, the
			// others are extensions by array.sty
			bool const newcolumn = c == 'p' || c == 'm' || c == 'b';
			if (newcolumn) {
				// this declares a new column
				if (col >= ncols())
					// Only intercolumn stuff is allowed
					// in the last dummy column
					break;
				colinfo_[col].align_ = 'l';
			} else {
				// this is intercolumn stuff
				if (colinfo_[col].special_.empty())
					// Overtake possible lines
					colinfo_[col].special_ = docstring(colinfo_[col].lines_, '|');
			}
			int brace_open = 0;
			int brace_close = 0;
			while (it != hh.end()) {
				c = *it;
				colinfo_[col].special_ += c;
				if (c == '{')
					++brace_open;
				else if (c == '}')
					++brace_close;
				++it;
				if (brace_open > 0 && brace_open == brace_close)
					break;
			}
			--it;
			if (newcolumn) {
				colinfo_[col].lines_ = count(
					colinfo_[col].special_.begin(),
					colinfo_[col].special_.end(), '|');
				LYXERR(Debug::MATHED, "special column separator: `"
					<< to_utf8(colinfo_[col].special_) << '\'');
				++col;
				colinfo_[col].lines_ = 0;
				colinfo_[col].special_.clear();
			}
		} else if (col >= ncols()) {
			// Only intercolumn stuff is allowed in the last
			// dummy column
			break;
		} else if (c == 'c' || c == 'l' || c == 'r') {
			colinfo_[col].align_ = static_cast<char>(c);
			if (!colinfo_[col].special_.empty()) {
				colinfo_[col].special_ += c;
				colinfo_[col].lines_ = count(
						colinfo_[col].special_.begin(),
						colinfo_[col].special_.end(), '|');
				LYXERR(Debug::MATHED, "special column separator: `"
					<< to_utf8(colinfo_[col].special_) << '\'');
			}
			++col;
			colinfo_[col].lines_ = 0;
			colinfo_[col].special_.clear();
		} else {
			lyxerr << "unknown column separator: '" << c << "'" << endl;
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


InsetMathGrid::col_type InsetMathGrid::guessColumns(docstring const & hh)
{
	col_type col = 0;
	for (docstring::const_iterator it = hh.begin(); it != hh.end(); ++it)
		if (*it == 'c' || *it == 'l' || *it == 'r'||
		    *it == 'p' || *it == 'm' || *it == 'b')
			++col;
	// let's have at least one column, even if we did not recognize its
	// alignment
	if (col == 0)
		col = 1;
	return col;
}


void InsetMathGrid::setHorizontalAlignment(char h, col_type col)
{
	colinfo_[col].align_ = h;
	if (!colinfo_[col].special_.empty()) {
		char_type & c = colinfo_[col].special_[colinfo_[col].special_.size() - 1];
		if (c == 'l' || c == 'c' || c == 'r')
			c = h;
	}
	// FIXME: Change alignment of p, m and b columns, too
}


char InsetMathGrid::horizontalAlignment(col_type col) const
{
	return colinfo_[col].align_;
}


docstring InsetMathGrid::horizontalAlignments() const
{
	docstring res;
	for (col_type col = 0; col < ncols(); ++col) {
		if (colinfo_[col].special_.empty()) {
			res += docstring(colinfo_[col].lines_, '|');
			res += colinfo_[col].align_;
		} else
			res += colinfo_[col].special_;
	}
	if (colinfo_[ncols()].special_.empty())
		return res + docstring(colinfo_[ncols()].lines_, '|');
	return res + colinfo_[ncols()].special_;
}


void InsetMathGrid::setVerticalAlignment(char c)
{
	v_align_ = c;
}


char InsetMathGrid::verticalAlignment() const
{
	return v_align_;
}


InsetMathGrid::col_type InsetMathGrid::ncols() const
{
	return colinfo_.size() - 1;
}


InsetMathGrid::row_type InsetMathGrid::nrows() const
{
	return rowinfo_.size() - 1;
}


InsetMathGrid::col_type InsetMathGrid::col(idx_type idx) const
{
	return idx % ncols();
}


InsetMathGrid::row_type InsetMathGrid::row(idx_type idx) const
{
	return idx / ncols();
}


void InsetMathGrid::vcrskip(Length const & crskip, row_type row)
{
	rowinfo_[row].crskip_ = crskip;
}


Length InsetMathGrid::vcrskip(row_type row) const
{
	return rowinfo_[row].crskip_;
}


void InsetMathGrid::metrics(MetricsInfo & mi, Dimension & dim) const
{
	// let the cells adjust themselves
	InsetMathNest::metrics(mi);

	BufferView & bv = *mi.base.bv;

	// compute absolute sizes of vertical structure
	for (row_type row = 0; row < nrows(); ++row) {
		int asc  = 0;
		int desc = 0;
		for (col_type col = 0; col < ncols(); ++col) {
			Dimension const & dimc = cell(index(row, col)).dimension(bv);
			asc  = max(asc,  dimc.asc);
			desc = max(desc, dimc.des);
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
			wid = max(wid, cell(index(row, col)).dimension(bv).wid);
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


	dim.wid   =   colinfo_[ncols() - 1].offset_
		       + colinfo_[ncols() - 1].width_
		 + vlinesep() * colinfo_[ncols()].lines_
		       + border();

	dim.asc  = - rowinfo_[0].offset_
		       + rowinfo_[0].ascent_
		 + hlinesep() * rowinfo_[0].lines_
		       + border();

	dim.des =   rowinfo_[nrows() - 1].offset_
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
	metricsMarkers2(dim);
	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


void InsetMathGrid::draw(PainterInfo & pi, int x, int y) const
{
	drawWithMargin(pi, x, y, 1, 1);
}


void InsetMathGrid::drawWithMargin(PainterInfo & pi, int x, int y,
	int lmargin, int rmargin) const
{
	Dimension const dim = dimension(*pi.base.bv);
	BufferView const & bv = *pi.base.bv;

	for (idx_type idx = 0; idx < nargs(); ++idx)
		cell(idx).draw(pi, x + lmargin + cellXOffset(bv, idx),
			y + cellYOffset(idx));

	for (row_type row = 0; row <= nrows(); ++row)
		for (unsigned int i = 0; i < rowinfo_[row].lines_; ++i) {
			int yy = y + rowinfo_[row].offset_ - rowinfo_[row].ascent_
				- i * hlinesep() - hlinesep()/2 - rowsep()/2;
			pi.pain.line(x + lmargin + 1, yy,
				     x + dim.width() - rmargin - 1, yy,
				     Color_foreground);
		}

	for (col_type col = 0; col <= ncols(); ++col)
		for (unsigned int i = 0; i < colinfo_[col].lines_; ++i) {
			int xx = x + lmargin + colinfo_[col].offset_
				- i * vlinesep() - vlinesep()/2 - colsep()/2;
			pi.pain.line(xx, y - dim.ascent() + 1,
				     xx, y + dim.descent() - 1,
				     Color_foreground);
		}
	drawMarkers2(pi, x, y);
}


void InsetMathGrid::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	// let the cells adjust themselves
	//InsetMathNest::metrics(mi);
	for (idx_type i = 0; i < nargs(); ++i)
		cell(i).metricsT(mi, dim);

	// compute absolute sizes of vertical structure
	for (row_type row = 0; row < nrows(); ++row) {
		int asc  = 0;
		int desc = 0;
		for (col_type col = 0; col < ncols(); ++col) {
			//MathData const & c = cell(index(row, col));
			// FIXME: BROKEN!
			Dimension dimc;
			asc  = max(asc,  dimc.ascent());
			desc = max(desc, dimc.descent());
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
		for (row_type row = 0; row < nrows(); ++row) {
			// FIXME: BROKEN!
			//wid = max(wid, cell(index(row, col)).width());
		}
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


	dim.wid  =  colinfo_[ncols() - 1].offset_
		       + colinfo_[ncols() - 1].width_
		 //+ vlinesep() * colinfo_[ncols()].lines_
		       + 2;

	dim.asc  = -rowinfo_[0].offset_
		       + rowinfo_[0].ascent_
		 //+ hlinesep() * rowinfo_[0].lines_
		       + 1;

	dim.des  =  rowinfo_[nrows() - 1].offset_
		       + rowinfo_[nrows() - 1].descent_
		 //+ hlinesep() * rowinfo_[nrows()].lines_
		       + 1;
}


void InsetMathGrid::drawT(TextPainter & /*pain*/, int /*x*/, int /*y*/) const
{
//	for (idx_type idx = 0; idx < nargs(); ++idx)
//		cell(idx).drawT(pain, x + cellXOffset(idx), y + cellYOffset(idx));
}


docstring InsetMathGrid::eolString(row_type row, bool emptyline, bool fragile) const
{
	docstring eol;

	if (!rowinfo_[row].crskip_.zero())
		eol += '[' + from_utf8(rowinfo_[row].crskip_.asLatexString()) + ']';
	else if(!rowinfo_[row].allow_newpage_)
		eol += '*';

	// make sure an upcoming '[' does not break anything
	if (row + 1 < nrows()) {
		MathData const & c = cell(index(row + 1, 0));
		if (c.size() && c.front()->getChar() == '[')
			//eol += "[0pt]";
			eol += "{}";
	}

	// only add \\ if necessary
	if (eol.empty() && row + 1 == nrows() && (nrows() == 1 || !emptyline))
		return docstring();

	return (fragile ? "\\protect\\\\" : "\\\\") + eol;
}


docstring InsetMathGrid::eocString(col_type col, col_type lastcol) const
{
	if (col + 1 == lastcol)
		return docstring();
	return from_ascii(" & ");
}


void InsetMathGrid::addRow(row_type row)
{
	rowinfo_.insert(rowinfo_.begin() + row + 1, RowInfo());
	cells_.insert
		(cells_.begin() + (row + 1) * ncols(), ncols(), MathData());
	cellinfo_.insert
		(cellinfo_.begin() + (row + 1) * ncols(), ncols(), CellInfo());
}


void InsetMathGrid::appendRow()
{
	rowinfo_.push_back(RowInfo());
	//cells_.insert(cells_.end(), ncols(), MathData());
	for (col_type col = 0; col < ncols(); ++col) {
		cells_.push_back(cells_type::value_type());
		cellinfo_.push_back(CellInfo());
	}
}


void InsetMathGrid::delRow(row_type row)
{
	if (nrows() == 1)
		return;

	cells_type::iterator it = cells_.begin() + row * ncols();
	cells_.erase(it, it + ncols());

	vector<CellInfo>::iterator jt = cellinfo_.begin() + row * ncols();
	cellinfo_.erase(jt, jt + ncols());

	rowinfo_.erase(rowinfo_.begin() + row);
}


void InsetMathGrid::copyRow(row_type row)
{
	addRow(row);
	for (col_type col = 0; col < ncols(); ++col)
		cells_[(row + 1) * ncols() + col] = cells_[row * ncols() + col];
}


void InsetMathGrid::swapRow(row_type row)
{
	if (nrows() == 1)
		return;
	if (row + 1 == nrows())
		--row;
	for (col_type col = 0; col < ncols(); ++col)
		swap(cells_[row * ncols() + col], cells_[(row + 1) * ncols() + col]);
}


void InsetMathGrid::addCol(col_type newcol)
{
	const col_type nc = ncols();
	const row_type nr = nrows();
	cells_type new_cells((nc + 1) * nr);
	vector<CellInfo> new_cellinfo((nc + 1) * nr);

	for (row_type row = 0; row < nr; ++row)
		for (col_type col = 0; col < nc; ++col) {
			new_cells[row * (nc + 1) + col + (col >= newcol)]
				= cells_[row * nc + col];
			new_cellinfo[row * (nc + 1) + col + (col >= newcol)]
				= cellinfo_[row * nc + col];
		}
	swap(cells_, new_cells);
	swap(cellinfo_, new_cellinfo);

	ColInfo inf;
	inf.skip_  = defaultColSpace(newcol);
	inf.align_ = defaultColAlign(newcol);
	colinfo_.insert(colinfo_.begin() + newcol, inf);
}


void InsetMathGrid::delCol(col_type col)
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


void InsetMathGrid::copyCol(col_type col)
{
	addCol(col);
	for (row_type row = 0; row < nrows(); ++row)
		cells_[row * ncols() + col + 1] = cells_[row * ncols() + col];
}


void InsetMathGrid::swapCol(col_type col)
{
	if (ncols() == 1)
		return;
	if (col + 1 == ncols())
		--col;
	for (row_type row = 0; row < nrows(); ++row)
		swap(cells_[row * ncols() + col], cells_[row * ncols() + col + 1]);
}


int InsetMathGrid::cellXOffset(BufferView const & bv, idx_type idx) const
{
	col_type c = col(idx);
	int x = colinfo_[c].offset_;
	char align = colinfo_[c].align_;
	Dimension const & celldim = cell(idx).dimension(bv);
	if (align == 'r' || align == 'R')
		x += colinfo_[c].width_ - celldim.wid;
	if (align == 'c' || align == 'C')
		x += (colinfo_[c].width_ - celldim.wid) / 2;
	return x;
}


int InsetMathGrid::cellYOffset(idx_type idx) const
{
	return rowinfo_[row(idx)].offset_;
}


bool InsetMathGrid::idxUpDown(Cursor & cur, bool up) const
{
	if (up) {
		if (cur.row() == 0)
			return false;
		cur.idx() -= ncols();
	} else {
		if (cur.row() + 1 >= nrows())
			return false;
		cur.idx() += ncols();
	}
	cur.pos() = cur.cell().x2pos(&cur.bv(), cur.x_target() - cur.cell().xo(cur.bv()));
	return true;
}


bool InsetMathGrid::idxBackward(Cursor & cur) const
{
	// leave matrix if at the front edge
	if (cur.col() == 0)
		return false;
	--cur.idx();
	cur.pos() = cur.lastpos();
	return true;
}


bool InsetMathGrid::idxForward(Cursor & cur) const
{
	// leave matrix if at the back edge
	if (cur.col() + 1 == ncols())
		return false;
	++cur.idx();
	cur.pos() = 0;
	return true;
}


bool InsetMathGrid::idxFirst(Cursor & cur) const
{
	switch (v_align_) {
		case 't':
			cur.idx() = 0;
			break;
		case 'b':
			cur.idx() = (nrows() - 1) * ncols();
			break;
		default:
			cur.idx() = ((nrows() - 1) / 2) * ncols();
	}
	cur.pos() = 0;
	return true;
}


bool InsetMathGrid::idxLast(Cursor & cur) const
{
	switch (v_align_) {
		case 't':
			cur.idx() = ncols() - 1;
			break;
		case 'b':
			cur.idx() = nargs() - 1;
			break;
		default:
			cur.idx() = ((nrows() - 1) / 2 + 1) * ncols() - 1;
	}
	cur.pos() = cur.lastpos();
	return true;
}


bool InsetMathGrid::idxDelete(idx_type & idx)
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
		swap(cell(i), cell(i + ncols()));

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
void InsetMathGrid::idxGlue(idx_type idx)
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


InsetMathGrid::RowInfo const & InsetMathGrid::rowinfo(row_type row) const
{
	return rowinfo_[row];
}


InsetMathGrid::RowInfo & InsetMathGrid::rowinfo(row_type row)
{
	return rowinfo_[row];
}


bool InsetMathGrid::idxBetween(idx_type idx, idx_type from, idx_type to) const
{
	row_type const ri = row(idx);
	row_type const r1 = min(row(from), row(to));
	row_type const r2 = max(row(from), row(to));
	col_type const ci = col(idx);
	col_type const c1 = min(col(from), col(to));
	col_type const c2 = max(col(from), col(to));
	return r1 <= ri && ri <= r2 && c1 <= ci && ci <= c2;
}



void InsetMathGrid::normalize(NormalStream & os) const
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


void InsetMathGrid::mathmlize(MathStream & os) const
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


void InsetMathGrid::write(WriteStream & os) const
{
	MathEnsurer ensurer(os, false);
	docstring eol;
	for (row_type row = 0; row < nrows(); ++row) {
		os << verboseHLine(rowinfo_[row].lines_);
		// don't write & and empty cells at end of line
		col_type lastcol = 0;
		bool emptyline = true;
		for (col_type col = 0; col < ncols(); ++col)
			if (!cell(index(row, col)).empty()) {
				lastcol = col + 1;
				emptyline = false;
			}
		for (col_type col = 0; col < lastcol; ++col) {
			os << cell(index(row, col));
			if (os.pendingBrace())
				ModeSpecifier specifier(os, TEXT_MODE);
			os << eocString(col, lastcol);
		}
		eol = eolString(row, emptyline, os.fragile());
		os << eol;
		// append newline only if line wasn't completely empty
		// and this was not the last line in the grid
		if (!emptyline && row + 1 < nrows())
			os << "\n";
	}
	docstring const s = verboseHLine(rowinfo_[nrows()].lines_);
	if (!s.empty()) {
		if (eol.empty()) {
			if (os.fragile())
				os << "\\protect";
			os << "\\\\";
		}
		os << s;
	}
}


int InsetMathGrid::colsep() const
{
	return 6;
}


int InsetMathGrid::rowsep() const
{
	return 6;
}


int InsetMathGrid::hlinesep() const
{
	return 3;
}


int InsetMathGrid::vlinesep() const
{
	return 3;
}


int InsetMathGrid::border() const
{
	return 1;
}


void InsetMathGrid::splitCell(Cursor & cur)
{
	if (cur.idx() == cur.lastidx())
		return;
	MathData ar = cur.cell();
	ar.erase(0, cur.pos());
	cur.cell().erase(cur.pos(), cur.lastpos());
	++cur.idx();
	cur.pos() = 0;
	cur.cell().insert(0, ar);
}


void InsetMathGrid::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	//lyxerr << "*** InsetMathGrid: request: " << cmd << endl;
	switch (cmd.action) {

	// insert file functions
	case LFUN_LINE_DELETE:
		// FIXME: We use recordUndoInset when a change reflects more
		// than one cell, because recordUndo does not work for
		// multiple cells. Unfortunately this puts the cursor in front
		// of the inset after undo. This is (especilally for large
		// grids) annoying.
		cur.recordUndoInset();
		//autocorrect_ = false;
		//macroModeClose();
		//if (selection_) {
		//	selDel();
		//	break;
		//}
		if (nrows() > 1)
			delRow(cur.row());
		if (cur.idx() > cur.lastidx())
			cur.idx() = cur.lastidx();
		if (cur.pos() > cur.lastpos())
			cur.pos() = cur.lastpos();
		break;

	case LFUN_CELL_SPLIT:
		cur.recordUndo();
		splitCell(cur);
		break;

	case LFUN_CELL_BACKWARD:
		// See below.
		cur.selection() = false;
		if (!idxPrev(cur)) {
			cmd = FuncRequest(LFUN_FINISHED_BACKWARD);
			cur.undispatched();
		}
		break;

	case LFUN_CELL_FORWARD:
		// Can't handle selection by additional 'shift' as this is
		// hard bound to LFUN_CELL_BACKWARD
		cur.selection() = false;
		if (!idxNext(cur)) {
			cmd = FuncRequest(LFUN_FINISHED_FORWARD);
			cur.undispatched();
		}
		break;

	case LFUN_NEWLINE_INSERT: {
		cur.recordUndoInset();
		row_type const r = cur.row();
		addRow(r);

		// split line
		for (col_type c = col(cur.idx()) + 1; c < ncols(); ++c)
			swap(cell(index(r, c)), cell(index(r + 1, c)));

		// split cell
		splitCell(cur);
		swap(cell(cur.idx()), cell(cur.idx() + ncols() - 1));
		if (cur.idx() > 0)
			--cur.idx();
		cur.pos() = cur.lastpos();

		//mathcursor->normalize();
		//cmd = FuncRequest(LFUN_FINISHED_BACKWARD);
		break;
	}

	case LFUN_TABULAR_FEATURE: {
		cur.recordUndoInset();
		//lyxerr << "handling tabular-feature " << to_utf8(cmd.argument()) << endl;
		istringstream is(to_utf8(cmd.argument()));
		string s;
		is >> s;
		if (s == "valign-top")
			setVerticalAlignment('t');
		else if (s == "valign-middle")
			setVerticalAlignment('c');
		else if (s == "valign-bottom")
			setVerticalAlignment('b');
		else if (s == "align-left")
			setHorizontalAlignment('l', cur.col());
		else if (s == "align-right")
			setHorizontalAlignment('r', cur.col());
		else if (s == "align-center")
			setHorizontalAlignment('c', cur.col());
		else if (s == "append-row")
			for (int i = 0, n = extractInt(is); i < n; ++i)
				addRow(cur.row());
		else if (s == "delete-row") {
			cur.clearSelection(); // bug 4323
			for (int i = 0, n = extractInt(is); i < n; ++i) {
				delRow(cur.row());
				if (cur.idx() >= nargs())
					cur.idx() -= ncols();
			}
			cur.pos() = 0; // trick, see below
		}
		else if (s == "copy-row") {
			// Here (as later) we save the cursor col/row
			// in order to restore it after operation.
			row_type const r = cur.row();
			col_type const c = cur.col();
			for (int i = 0, n = extractInt(is); i < n; ++i)
				copyRow(cur.row());
			cur.idx() = index(r, c);
		}
		else if (s == "swap-row") {
			swapRow(cur.row());
			// Trick to suppress same-idx-means-different-cell
			// assertion crash:
			cur.pos() = 0;
		}
		else if (s == "add-hline-above")
			rowinfo_[cur.row()].lines_++;
		else if (s == "add-hline-below")
			rowinfo_[cur.row()+1].lines_++;
		else if (s == "delete-hline-above")
			rowinfo_[cur.row()].lines_--;
		else if (s == "delete-hline-below")
			rowinfo_[cur.row()+1].lines_--;
		else if (s == "append-column") {
			row_type const r = cur.row();
			col_type const c = cur.col();
			for (int i = 0, n = extractInt(is); i < n; ++i)
				addCol(cur.col() + 1);
			cur.idx() = index(r, c);
		}
		else if (s == "delete-column") {
			cur.clearSelection(); // bug 4323
			row_type const r = cur.row();
			col_type const c = cur.col();
			for (int i = 0, n = extractInt(is); i < n; ++i)
				delCol(col(cur.idx()));
			cur.idx() = index(r, min(c, cur.ncols() - 1));
			cur.pos() = 0; // trick, see above
		}
		else if (s == "copy-column") {
			row_type const r = cur.row();
			col_type const c = cur.col();
			copyCol(cur.col());
			cur.idx() = index(r, c);
		}
		else if (s == "swap-column") {
			swapCol(cur.col());
			cur.pos() = 0; // trick, see above
		}
		else if (s == "add-vline-left") {
			colinfo_[cur.col()].lines_++;
			if (!colinfo_[cur.col()].special_.empty())
				colinfo_[cur.col()].special_ += '|';
		}
		else if (s == "add-vline-right") {
			colinfo_[cur.col()+1].lines_++;
			if (!colinfo_[cur.col()+1].special_.empty())
				colinfo_[cur.col()+1].special_.insert(0, 1, '|');
		}
		else if (s == "delete-vline-left") {
			colinfo_[cur.col()].lines_--;
			docstring & special = colinfo_[cur.col()].special_;
			if (!special.empty()) {
				docstring::size_type i = special.rfind('|');
				LASSERT(i != docstring::npos, /**/);
				special.erase(i, 1);
			}
		}
		else if (s == "delete-vline-right") {
			colinfo_[cur.col()+1].lines_--;
			docstring & special = colinfo_[cur.col()+1].special_;
			if (!special.empty()) {
				docstring::size_type i = special.find('|');
				LASSERT(i != docstring::npos, /**/);
				special.erase(i, 1);
			}
		}
		else {
			cur.undispatched();
			break;
		}
		// perhaps this should be FINISHED_BACKWARD -- just for clarity?
		lyxerr << "returning FINISHED_LEFT" << endl;
		break;
	}

	case LFUN_PASTE: {
		cur.message(_("Paste"));
		cap::replaceSelection(cur);
		docstring topaste;
		if (cmd.argument().empty() && !theClipboard().isInternal())
			topaste = theClipboard().getAsText();
		else {
			idocstringstream is(cmd.argument());
			int n = 0;
			is >> n;
			topaste = cap::selection(n);
		}
		InsetMathGrid grid(1, 1);
		if (!topaste.empty())
			mathed_parse_normal(grid, topaste);

		if (grid.nargs() == 1) {
			// single cell/part of cell
			cur.recordUndo();
			cur.cell().insert(cur.pos(), grid.cell(0));
			cur.pos() += grid.cell(0).size();
		} else {
			// multiple cells
			cur.recordUndoInset();
			col_type const numcols =
				min(grid.ncols(), ncols() - col(cur.idx()));
			row_type const numrows =
				min(grid.nrows(), nrows() - cur.row());
			for (row_type r = 0; r < numrows; ++r) {
				for (col_type c = 0; c < numcols; ++c) {
					idx_type i = index(r + cur.row(), c + col(cur.idx()));
					cell(i).insert(0, grid.cell(grid.index(r, c)));
				}
				// append the left over horizontal cells to the last column
				idx_type i = index(r + cur.row(), ncols() - 1);
				for (InsetMath::col_type c = numcols; c < grid.ncols(); ++c)
					cell(i).append(grid.cell(grid.index(r, c)));
			}
			// append the left over vertical cells to the last _cell_
			idx_type i = nargs() - 1;
			for (row_type r = numrows; r < grid.nrows(); ++r)
				for (col_type c = 0; c < grid.ncols(); ++c)
					cell(i).append(grid.cell(grid.index(r, c)));
		}
		cur.clearSelection(); // bug 393
		cur.finishUndo();
		break;
	}

	case LFUN_LINE_BEGIN_SELECT:
	case LFUN_LINE_BEGIN:
	case LFUN_WORD_BACKWARD_SELECT:
	case LFUN_WORD_BACKWARD:
	case LFUN_WORD_LEFT_SELECT:
	case LFUN_WORD_LEFT:
		cur.selHandle(cmd.action == LFUN_WORD_BACKWARD_SELECT ||
				cmd.action == LFUN_WORD_LEFT_SELECT ||
				cmd.action == LFUN_LINE_BEGIN_SELECT);
		cur.macroModeClose();
		if (cur.pos() != 0) {
			cur.pos() = 0;
		} else if (cur.idx() % cur.ncols() != 0) {
			cur.idx() -= cur.idx() % cur.ncols();
			cur.pos() = 0;
		} else if (cur.idx() != 0) {
			cur.idx() = 0;
			cur.pos() = 0;
		} else {
			cmd = FuncRequest(LFUN_FINISHED_BACKWARD);
			cur.undispatched();
		}
		break;

	case LFUN_WORD_FORWARD_SELECT:
	case LFUN_WORD_FORWARD:
	case LFUN_WORD_RIGHT_SELECT:
	case LFUN_WORD_RIGHT:
	case LFUN_LINE_END_SELECT:
	case LFUN_LINE_END:
		cur.selHandle(cmd.action == LFUN_WORD_FORWARD_SELECT ||
				cmd.action == LFUN_WORD_RIGHT_SELECT ||
				cmd.action == LFUN_LINE_END_SELECT);
		cur.macroModeClose();
		cur.clearTargetX();
		if (cur.pos() != cur.lastpos()) {
			cur.pos() = cur.lastpos();
		} else if ((cur.idx() + 1) % cur.ncols() != 0) {
			cur.idx() += cur.ncols() - 1 - cur.idx() % cur.ncols();
			cur.pos() = cur.lastpos();
		} else if (cur.idx() != cur.lastidx()) {
			cur.idx() = cur.lastidx();
			cur.pos() = cur.lastpos();
		} else {
			cmd = FuncRequest(LFUN_FINISHED_FORWARD);
			cur.undispatched();
		}
		break;

	default:
		InsetMathNest::doDispatch(cur, cmd);
	}
}


bool InsetMathGrid::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & status) const
{
	switch (cmd.action) {
	case LFUN_TABULAR_FEATURE: {
		string const s = to_utf8(cmd.argument());
		if (nrows() <= 1 && (s == "delete-row" || s == "swap-row")) {
			status.setEnabled(false);
			status.message(from_utf8(N_("Only one row")));
			return true;
		}
		if (ncols() <= 1 &&
		    (s == "delete-column" || s == "swap-column")) {
			status.setEnabled(false);
			status.message(from_utf8(N_("Only one column")));
			return true;
		}
		if ((rowinfo_[cur.row()].lines_ == 0 &&
		     s == "delete-hline-above") ||
		    (rowinfo_[cur.row() + 1].lines_ == 0 &&
		     s == "delete-hline-below")) {
			status.setEnabled(false);
			status.message(from_utf8(N_("No hline to delete")));
			return true;
		}

		if ((colinfo_[cur.col()].lines_ == 0 &&
		     s == "delete-vline-left") ||
		    (colinfo_[cur.col() + 1].lines_ == 0 &&
		     s == "delete-vline-right")) {
			status.setEnabled(false);
			status.message(from_utf8(N_("No vline to delete")));
			return true;
		}
		if (s == "valign-top" || s == "valign-middle" ||
		    s == "valign-bottom" || s == "align-left" ||
		    s == "align-right" || s == "align-center" ||
		    s == "append-row" || s == "delete-row" ||
		    s == "copy-row" || s == "swap-row" ||
		    s == "add-hline-above" || s == "add-hline-below" ||
		    s == "delete-hline-above" || s == "delete-hline-below" ||
		    s == "append-column" || s == "delete-column" ||
		    s == "copy-column" || s == "swap-column" ||
		    s == "add-vline-left" || s == "add-vline-right" ||
		    s == "delete-vline-left" || s == "delete-vline-right")
			status.setEnabled(true);
		else {
			status.setEnabled(false);
			status.message(bformat(
				from_utf8(N_("Unknown tabular feature '%1$s'")), lyx::from_ascii(s)));
		}

		char const ha = horizontalAlignment(cur.col());
		char const va = verticalAlignment();
		status.setOnOff((s == "align-left" && ha == 'l')
			   || (s == "align-right"   && ha == 'r')
			   || (s == "align-center"  && ha == 'c')
			   || (s == "valign-top"    && va == 't')
			   || (s == "valign-bottom" && va == 'b')
			   || (s == "valign-middle" && va == 'm'));

#if 0
		// FIXME: What did this code do?
		// Please check whether it is still needed!
		// should be more precise
		if (v_align_ == '\0') {
			status.enable(true);
			break;
		}
		if (cmd.argument().empty()) {
			status.enable(false);
			break;
		}
		if (!contains("tcb", cmd.argument()[0])) {
			status.enable(false);
			break;
		}
		status.setOnOff(cmd.argument()[0] == v_align_);
		status.setEnabled(true);
#endif
		return true;
	}

	case LFUN_CELL_SPLIT:
		status.setEnabled(true);
		return true;

	case LFUN_CELL_BACKWARD:
	case LFUN_CELL_FORWARD:
		status.setEnabled(true);
		return true;

	default:
		return InsetMathNest::getStatus(cur, cmd, status);
	}
}


} // namespace lyx
