/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 2000-2001 The LyX Team.
 *
 *           @author: Jürgen Vigna
 *
 * ====================================================== 
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

// temporary until verified (08/08/2001 Jug)
#define SPECIAL_COLUM_HANDLING 1

#include "tabular.h"
#include "debug.h"
#include "vspace.h"
#include "layout.h"
#include "buffer.h"
#include "BufferView.h"
#include "Painter.h"
#include "LaTeXFeatures.h"
#include "insets/insettabular.h"
#include "insets/insettext.h"
#include "support/lstrings.h"
#include "support/lyxmanip.h"
#include "support/LAssert.h"
#include "frontends/Alert.h"
#include "gettext.h"
#include "tabular_funcs.h"

#include <algorithm>
#include <cstdlib>

using std::ostream;
using std::istream;
using std::getline;
using std::max;
using std::endl;
using std::vector;

#ifndef CXX_GLOBAL_CSTD
using std::strlen;
#endif

namespace {

	int const WIDTH_OF_LINE = 5;

} // namespace

/// Define a few methods for the inner structs

LyXTabular::cellstruct::cellstruct() 
{
	cellno = 0;
	width_of_cell = 0;
	multicolumn = LyXTabular::CELL_NORMAL;
	alignment = LYX_ALIGN_CENTER;
	valignment = LYX_VALIGN_TOP;
	top_line = true;
	bottom_line = false;
	left_line = true;
	right_line = false;
	usebox = BOX_NONE;
	rotate = false;
}


LyXTabular::rowstruct::rowstruct() 
{
	top_line = true;
	bottom_line = false;
	ascent_of_row = 0;
	descent_of_row = 0;
	endhead = false;
	endfirsthead = false;
	endfoot = false;
	endlastfoot = false;
	newpage = false;
}


LyXTabular::columnstruct::columnstruct() 
{
	left_line = true;
	right_line = false;
	alignment = LYX_ALIGN_CENTER;
	valignment = LYX_VALIGN_TOP;
	width_of_column = 0;
}


LyXTabular::lttype::lttype()
{
	topDL = false;
	bottomDL = false;
	empty = false;
}


/* konstruktor */
LyXTabular::LyXTabular(InsetTabular * inset, int rows_arg, int columns_arg)
{
	owner_ = inset;
	cur_cell = -1;
	Init(rows_arg, columns_arg);
}


LyXTabular::LyXTabular(InsetTabular * inset, LyXTabular const & lt,
                       bool same_id)
{
	owner_ = inset;
	cur_cell = -1;
	Init(lt.rows_, lt.columns_, &lt);
	// we really should change again to have InsetText as a pointer
	// and allocate it then we would not have to do this stuff all
	// double!
	if (same_id) {
		for (int i = 0; i < rows_; ++i) {
			for (int j = 0; j < columns_; ++j) {
				cell_info[i][j].inset.id(lt.cell_info[i][j].inset.id());
				cell_info[i][j].inset.setParagraphData(lt.cell_info[i][j].inset.paragraph(),true);
			}
		}
	}
#if 0
#ifdef WITH_WARNINGS
#warning Jürgen, can you make it the other way round. So that copy assignment depends on the copy constructor and not the other way. (Lgb)
#endif
	operator=(lt);
#endif
}


LyXTabular::LyXTabular(Buffer const * buf, InsetTabular * inset, LyXLex & lex)
{
	owner_ = inset;
	cur_cell = -1;
	Read(buf, lex);
}


LyXTabular & LyXTabular::operator=(LyXTabular const & lt)
{
#if 0
#warning This while method should look like this: (Lgb)

		LyXTabular tmp(lt);
		tmp.swap(*this);
#else
	// If this and lt is not of the same size we have a serious bug
	// So then it is ok to throw an exception, or for now
	// call abort()
	lyx::Assert(rows_ == lt.rows_ && columns_ == lt.columns_);
	cur_cell = -1;
	cell_info = lt.cell_info;
	row_info = lt.row_info;
	column_info = lt.column_info;
	SetLongTabular(lt.is_long_tabular);
	rotate = lt.rotate;

	Reinit();
#endif
	return *this;
}


LyXTabular * LyXTabular::clone(InsetTabular * inset, bool same_id)
{
	LyXTabular * result = new LyXTabular(inset, *this, same_id);
#if 0
	// don't know if this is good but I need to Clone also
	// the text-insets here, this is for the Undo-facility!
	for (int i = 0; i < rows_; ++i) {
		for (int j = 0; j < columns_; ++j) {
			result->cell_info[i][j].inset = cell_info[i][j].inset;
			result->cell_info[i][j].inset.setOwner(inset);
		}
	}
#endif
	return result;
}


/* activates all lines and sets all widths to 0 */ 
void LyXTabular::Init(int rows_arg, int columns_arg, LyXTabular const * lt)
{
	rows_ = rows_arg;
	columns_ = columns_arg;
	row_info = row_vector(rows_, rowstruct());
	column_info = column_vector(columns_, columnstruct());
	cell_info = cell_vvector(rows_, cell_vector(columns_, cellstruct()));

	if (lt) {
		operator=(*lt);
		return;
	}

	int cellno = 0;
	for (int i = 0; i < rows_; ++i) {
		for (int j = 0; j < columns_; ++j) {
			cell_info[i][j].inset.setOwner(owner_);
			cell_info[i][j].inset.setDrawFrame(0, InsetText::LOCKED);
			cell_info[i][j].cellno = cellno++;
		}
		cell_info[i].back().right_line = true;
	}
	row_info.back().bottom_line = true;
	row_info.front().bottom_line = true;

	for (int i = 0; i < columns_; ++i) {
		calculate_width_of_column(i);
	}
	column_info.back().right_line = true;
   
	calculate_width_of_tabular();

	rowofcell = vector<int>();
	columnofcell = vector<int>();
	set_row_column_number_info();
	is_long_tabular = false;
	rotate = false;
}


void LyXTabular::AppendRow(int cell)
{
	++rows_;
   
	int row = row_of_cell(cell);

	row_vector::iterator rit = row_info.begin() + row;
	row_info.insert(rit, rowstruct());
	// now set the values of the row before
	row_info[row] = row_info[row+1];

#if 0
	cell_vvector::iterator cit = cell_info.begin() + row;
	cell_info.insert(cit, vector<cellstruct>(columns_, cellstruct()));
#else
	cell_vvector c_info = cell_vvector(rows_, cell_vector(columns_,
														  cellstruct()));

	for (int i = 0; i <= row; ++i) {
		for (int j = 0; j < columns_; ++j) {
			c_info[i][j] = cell_info[i][j];
		}
	}
	for (int i = row + 1; i < rows_; ++i) {
		for (int j = 0; j < columns_; ++j) {
			c_info[i][j] = cell_info[i-1][j];
		}
	}
	cell_info = c_info;
	++row;
	for (int j = 0; j < columns_; ++j) {
		cell_info[row][j].inset.clear();
	}
#endif
	Reinit();
}


void LyXTabular::DeleteRow(int row)
{
	if (rows_ == 1) return; // Not allowed to delete last row
	
	row_info.erase(row_info.begin() + row); //&row_info[row]);
	cell_info.erase(cell_info.begin() + row); //&cell_info[row]);
	--rows_;
	Reinit();
}


void LyXTabular::AppendColumn(int cell)
{
	++columns_;
   
	cell_vvector c_info = cell_vvector(rows_, cell_vector(columns_,
														  cellstruct()));
	int const column = column_of_cell(cell);
	column_vector::iterator cit = column_info.begin() + column + 1;
	column_info.insert(cit, columnstruct());
	// set the column values of the column before
	column_info[column+1] = column_info[column];

	for (int i = 0; i < rows_; ++i) {
		for (int j = 0; j <= column; ++j) {
			c_info[i][j] = cell_info[i][j];
		}
		for (int j = column + 1; j < columns_; ++j) {
			c_info[i][j] = cell_info[i][j - 1];
		}
		// care about multicolumns
		if (c_info[i][column + 1].multicolumn==CELL_BEGIN_OF_MULTICOLUMN)
		{
			c_info[i][column + 1].multicolumn = CELL_PART_OF_MULTICOLUMN;
		}
		if ((column + 2) >= columns_ ||
			c_info[i][column + 2].multicolumn != CELL_PART_OF_MULTICOLUMN)
		{
			c_info[i][column + 1].multicolumn = LyXTabular::CELL_NORMAL;
		}
	}
	cell_info = c_info;
	//++column;
	for (int i = 0; i < rows_; ++i) {
		//cell_info[i][column].inset.clear();
		cell_info[i][column + 1].inset.clear();
	}
	Reinit();
}


void LyXTabular::DeleteColumn(int column)
{
	// Similar to DeleteRow
	//if (!(columns_ - 1))
	//return;
	if (columns_ == 1) return; // Not allowed to delete last column
	 
	column_info.erase(column_info.begin() + column);
	for (int i = 0; i < rows_; ++i) {
		cell_info[i].erase(cell_info[i].begin() + column);
	}
	--columns_;
	Reinit();
}


void LyXTabular::reinit()
{
	Reinit(false);
}


void LyXTabular::Reinit(bool reset_widths)
{
	if (reset_widths) {
		for (int i = 0; i < rows_; ++i) {
			for (int j = 0; j < columns_; ++j) {
				cell_info[i][j].width_of_cell = 0;
				cell_info[i][j].inset.setOwner(owner_);
			}
		}
	}
  
	for (int i = 0; i < columns_; ++i) {
		calculate_width_of_column(i);
	}
	calculate_width_of_tabular();

	set_row_column_number_info();
}


void LyXTabular::set_row_column_number_info(bool oldformat)
{
	numberofcells = -1;
	for (int row = 0; row < rows_; ++row) {
		for (int column = 0; column<columns_; ++column) {
			if (cell_info[row][column].multicolumn
				!= LyXTabular::CELL_PART_OF_MULTICOLUMN)
				++numberofcells;
			cell_info[row][column].cellno = numberofcells;
		}
	}
	++numberofcells; // because this is one more than as we start from 0

	rowofcell.resize(numberofcells);
	columnofcell.resize(numberofcells);

	for (int row = 0, column = 0, c = 0;
		 c < numberofcells && row < rows_ && column < columns_;) {
		rowofcell[c] = row;
		columnofcell[c] = column;
		++c;
		do {
			++column;
		} while (column < columns_ &&
				 cell_info[row][column].multicolumn
				 == LyXTabular::CELL_PART_OF_MULTICOLUMN);
		if (column == columns_) {
			column = 0;
			++row;
		}
	}

	for (int row = 0; row < rows_; ++row) {
		for (int column = 0; column < columns_; ++column) {
			if (IsPartOfMultiColumn(row,column))
				continue;
			// now set the right line of multicolumns right for oldformat read
			if (oldformat &&
				cell_info[row][column].multicolumn==CELL_BEGIN_OF_MULTICOLUMN)
			{
				int cn=cells_in_multicolumn(cell_info[row][column].cellno);
				cell_info[row][column].right_line =
					cell_info[row][column+cn-1].right_line;
			}
			cell_info[row][column].inset.setAutoBreakRows(
				!GetPWidth(GetCellNumber(row, column)).zero());
		}
	}
}


int LyXTabular::GetNumberOfCells() const
{
	return numberofcells;
}


int LyXTabular::NumberOfCellsInRow(int cell) const
{
	int const row = row_of_cell(cell);
	int result = 0;
	for (int i = 0; i < columns_; ++i) {
		if (cell_info[row][i].multicolumn != LyXTabular::CELL_PART_OF_MULTICOLUMN)
			++result;
	}
	return result;
}


/* returns 1 if there is a topline, returns 0 if not */ 
bool LyXTabular::TopLine(int cell, bool onlycolumn) const
{
	int const row = row_of_cell(cell);
	
	if (!onlycolumn && IsMultiColumn(cell))
		return cellinfo_of_cell(cell)->top_line;
	return row_info[row].top_line;
}


bool LyXTabular::BottomLine(int cell, bool onlycolumn) const
{
	// no bottom line underneath non-existent cells if you please
	// Isn't that a programming error? Is so this should
	// be an Assert instead. (Lgb)
	if (cell >= numberofcells)
		return false;

	if (!onlycolumn && IsMultiColumn(cell))
		return cellinfo_of_cell(cell)->bottom_line;
	return row_info[row_of_cell(cell)].bottom_line;
}


bool LyXTabular::LeftLine(int cell, bool onlycolumn) const
{
	if (!onlycolumn && IsMultiColumn(cell)) {
#ifdef SPECIAL_COLUM_HANDLING
		if (cellinfo_of_cell(cell)->align_special.empty())
			return cellinfo_of_cell(cell)->left_line;
		return prefixIs(frontStrip(cellinfo_of_cell(cell)->align_special), "|");
#else
		return cellinfo_of_cell(cell)->left_line;
#endif
	}
#ifdef SPECIAL_COLUM_HANDLING
	if (column_info[column_of_cell(cell)].align_special.empty())
		return column_info[column_of_cell(cell)].left_line;
	return prefixIs(frontStrip(column_info[column_of_cell(cell)].align_special), "|");
#else
	return column_info[column_of_cell(cell)].left_line;
#endif
}


bool LyXTabular::RightLine(int cell, bool onlycolumn) const
{
	if (!onlycolumn && IsMultiColumn(cell)) {
#ifdef SPECIAL_COLUM_HANDLING
		if (cellinfo_of_cell(cell)->align_special.empty())
			return cellinfo_of_cell(cell)->right_line;
		return suffixIs(strip(cellinfo_of_cell(cell)->align_special), "|");
#else
		return cellinfo_of_cell(cell)->right_line;
#endif
	}
#ifdef SPECIAL_COLUM_HANDLING
	if (column_info[column_of_cell(cell)].align_special.empty())
		return column_info[right_column_of_cell(cell)].right_line;
	return suffixIs(strip(column_info[column_of_cell(cell)].align_special), "|");
#else
	return column_info[right_column_of_cell(cell)].right_line;
#endif
}


bool LyXTabular::TopAlreadyDrawed(int cell) const
{
	int row = row_of_cell(cell);
	if ((row > 0) && !GetAdditionalHeight(row)) {
		int column = column_of_cell(cell);
		--row;
		while (column
			   && cell_info[row][column].multicolumn
			   == LyXTabular::CELL_PART_OF_MULTICOLUMN)
			--column;
		if (cell_info[row][column].multicolumn == LyXTabular::CELL_NORMAL)
			return row_info[row].bottom_line;
		else
			return cell_info[row][column].bottom_line;
	}
	return false;
}


bool LyXTabular::LeftAlreadyDrawed(int cell) const
{
	int column = column_of_cell(cell);
	if (column > 0) {
		int row = row_of_cell(cell);
		while (--column &&
			   (cell_info[row][column].multicolumn ==
				LyXTabular::CELL_PART_OF_MULTICOLUMN));
		if (GetAdditionalWidth(cell_info[row][column].cellno))
			return false;
#ifdef SPECIAL_COLUM_HANDLING
		return column_info[column].right_line;
#else
		return RightLine(cell_info[row][column].cellno, true);
#endif
	}
	return false;
}


bool LyXTabular::IsLastRow(int cell) const
{
	return (row_of_cell(cell) == rows_ - 1);
}


int LyXTabular::GetAdditionalHeight(int row) const
{
	if (!row || row >= rows_)
		return 0;

	bool top = true;
	bool bottom = true;

	for (int column = 0; column < columns_ - 1 && bottom; ++column) {
		switch (cell_info[row - 1][column].multicolumn) {
		case LyXTabular::CELL_BEGIN_OF_MULTICOLUMN:
			bottom = cell_info[row - 1][column].bottom_line;
			break;
		case LyXTabular::CELL_NORMAL:
			bottom = row_info[row - 1].bottom_line;
		}
	}
	for (int column = 0; column < columns_ - 1 && top; ++column) {
		switch (cell_info[row][column].multicolumn){
		case LyXTabular::CELL_BEGIN_OF_MULTICOLUMN:
			top = cell_info[row][column].top_line;
			break;
		case LyXTabular::CELL_NORMAL:
			top = row_info[row].top_line;
		}
	}
	if (top && bottom)
		return WIDTH_OF_LINE;
	return 0;
}


int LyXTabular::GetAdditionalWidth(int cell) const
{
	// internally already set in SetWidthOfCell
	// used to get it back in text.C
	int const col = right_column_of_cell(cell);
	int const row = row_of_cell(cell);
	if (col < columns_ - 1 && RightLine(cell, true) &&
		LeftLine(cell_info[row][col+1].cellno, true)) // column_info[col+1].left_line)
	{
		return WIDTH_OF_LINE;
	} else {
		return 0;
	}
}


// returns the maximum over all rows 
int LyXTabular::GetWidthOfColumn(int cell) const
{
	int const column1 = column_of_cell(cell);
	int const column2 = right_column_of_cell(cell);
	int result = 0;
	for (int i = column1; i <= column2; ++i) {
		result += column_info[i].width_of_column;
	}
	return result;
}


int LyXTabular::GetWidthOfTabular() const
{
	return width_of_tabular;
}


/* returns 1 if a complete update is necessary, otherwise 0 */ 
bool LyXTabular::SetWidthOfMulticolCell(int cell, int new_width)
{
	if (!IsMultiColumn(cell))
		return false;
	
	int const row = row_of_cell(cell);
	int const column1 = column_of_cell(cell);
	int const column2 = right_column_of_cell(cell);

	// first set columns to 0 so we can calculate the right width
	for (int i = column1; i <= column2; ++i) {
		cell_info[row][i].width_of_cell = 0;
	}
	// set the width to MAX_WIDTH until width > 0
	int width = (new_width + 2 * WIDTH_OF_LINE);

	int i = column1;
	for (; i < column2 && width > column_info[i].width_of_column; ++i) {
		cell_info[row][i].width_of_cell = column_info[i].width_of_column;
		width -= column_info[i].width_of_column;
	}
	if (width > 0) {
		cell_info[row][i].width_of_cell = width;
	}
	return true;
}


void LyXTabular::recalculateMulticolCells(int cell, int new_width)
{
	int const row = row_of_cell(cell);
	int const column1 = column_of_cell(cell);
	int const column2 = right_column_of_cell(cell);

	// first set columns to 0 so we can calculate the right width
	int i = column1;
	for (; i <= column2; ++i)
		cell_info[row][i].width_of_cell = 0;
	for (i = cell + 1; (i < numberofcells) && (!IsMultiColumn(i)); ++i)
		;
	if (i < numberofcells)
		recalculateMulticolCells(i, GetWidthOfCell(i) - (2 * WIDTH_OF_LINE));
	SetWidthOfMulticolCell(cell, new_width);
}


/* returns 1 if a complete update is necessary, otherwise 0 */ 
bool LyXTabular::SetWidthOfCell(int cell, int new_width)
{
	int const row = row_of_cell(cell);
	int const column1 = column_of_cell(cell);
	bool tmp = false;
	int width = 0;
	int add_width = 0;

#ifdef SPECIAL_COLUM_HANDLING
	if (RightLine(cell_info[row][column1].cellno, true) &&
		(column1 < columns_-1) &&
		LeftLine(cell_info[row][column1+1].cellno, true))
#else
	if (column_info[column1].right_line && (column1 < columns_-1) &&
		column_info[column1+1].left_line) // additional width
#endif
	{
		// additional width
		add_width = WIDTH_OF_LINE;
	}
	if (GetWidthOfCell(cell) == (new_width+2*WIDTH_OF_LINE+add_width)) {
		return false;
	}
	if (IsMultiColumn(cell, true)) {
		tmp = SetWidthOfMulticolCell(cell, new_width);
	} else {
		width = (new_width + 2*WIDTH_OF_LINE + add_width);
		cell_info[row][column1].width_of_cell = width;
		tmp = calculate_width_of_column_NMC(column1);
	}
	if (tmp) {
		int i = 0;
		for (; i<columns_; ++i)
			calculate_width_of_column_NMC(i);
		for (i = 0; (i < numberofcells) && !IsMultiColumn(i); ++i)
			;
		if (i < numberofcells)
			recalculateMulticolCells(i, GetWidthOfCell(i)-(2 * WIDTH_OF_LINE));
		for (i = 0; i < columns_; ++i)
			calculate_width_of_column(i);
		calculate_width_of_tabular();
		return true;
	}
	return false;
}


bool LyXTabular::SetAlignment(int cell, LyXAlignment align, bool onlycolumn)
{
	if (!IsMultiColumn(cell) || onlycolumn)
		column_info[column_of_cell(cell)].alignment = align;
	if (!onlycolumn)
		cellinfo_of_cell(cell)->alignment = align;
	return true;
}


bool LyXTabular::SetVAlignment(int cell, VAlignment align, bool onlycolumn)
{
	if (!IsMultiColumn(cell) || onlycolumn)
		column_info[column_of_cell(cell)].valignment = align;
	if (!onlycolumn)
		cellinfo_of_cell(cell)->valignment = align;
	return true;
}


bool LyXTabular::SetColumnPWidth(int cell, LyXLength const & width)
{
	bool flag = !width.zero();
	int const j = column_of_cell(cell);

	column_info[j].p_width = width;
	if (flag) // do this only if there is a width
		SetAlignment(cell, LYX_ALIGN_LEFT);
	for (int i = 0; i < rows_; ++i) {
		int c = GetCellNumber(i, j);
		flag = !GetPWidth(c).zero(); // because of multicolumns!
		GetCellInset(c)->setAutoBreakRows(flag);
	}
	return true;
}


bool LyXTabular::SetMColumnPWidth(int cell, LyXLength const & width)
{
	bool const flag = !width.zero();

	cellinfo_of_cell(cell)->p_width = width;
	if (IsMultiColumn(cell)) {
		GetCellInset(cell)->setAutoBreakRows(flag);
		return true;
	}
	return false;
}


bool LyXTabular::SetAlignSpecial(int cell, string const & special,
                                 LyXTabular::Feature what)
{
	if (what == SET_SPECIAL_MULTI)
		cellinfo_of_cell(cell)->align_special = special;
	else
		column_info[column_of_cell(cell)].align_special = special;
	return true;
}


bool LyXTabular::SetAllLines(int cell, bool line)
{
	SetTopLine(cell, line);
	SetBottomLine(cell, line);
	SetRightLine(cell, line);
	SetLeftLine(cell, line);
	return true;
}


bool LyXTabular::SetTopLine(int cell, bool line, bool onlycolumn)
{
	int const row = row_of_cell(cell);

	if (onlycolumn || !IsMultiColumn(cell))
		row_info[row].top_line = line;
	else
		cellinfo_of_cell(cell)->top_line = line;
	return true;
}


bool LyXTabular::SetBottomLine(int cell, bool line, bool onlycolumn)
{
	if (onlycolumn || !IsMultiColumn(cell))
		row_info[row_of_cell(cell)].bottom_line = line;
	else
		cellinfo_of_cell(cell)->bottom_line = line;
	return true;
}


bool LyXTabular::SetLeftLine(int cell, bool line, bool onlycolumn)
{
	if (onlycolumn || !IsMultiColumn(cell))
		column_info[column_of_cell(cell)].left_line = line;
	else
		cellinfo_of_cell(cell)->left_line = line;
	return true;
}


bool LyXTabular::SetRightLine(int cell, bool line, bool onlycolumn)
{
	if (onlycolumn || !IsMultiColumn(cell))
		column_info[right_column_of_cell(cell)].right_line = line;
	else
		cellinfo_of_cell(cell)->right_line = line;
	return true;
}


LyXAlignment LyXTabular::GetAlignment(int cell, bool onlycolumn) const
{
	if (!onlycolumn && IsMultiColumn(cell))
		return cellinfo_of_cell(cell)->alignment;
	else
		return column_info[column_of_cell(cell)].alignment;
}


LyXTabular::VAlignment
LyXTabular::GetVAlignment(int cell, bool onlycolumn) const
{
	if (!onlycolumn && IsMultiColumn(cell))
		return cellinfo_of_cell(cell)->valignment;
	else
		return column_info[column_of_cell(cell)].valignment;
}


LyXLength const LyXTabular::GetPWidth(int cell) const
{
	if (IsMultiColumn(cell))
		return cellinfo_of_cell(cell)->p_width;
	return column_info[column_of_cell(cell)].p_width;
}


LyXLength const LyXTabular::GetColumnPWidth(int cell) const
{
	return column_info[column_of_cell(cell)].p_width;
}


LyXLength const LyXTabular::GetMColumnPWidth(int cell) const
{
	if (IsMultiColumn(cell))
		return cellinfo_of_cell(cell)->p_width;
	return LyXLength();
}


string const LyXTabular::GetAlignSpecial(int cell, int what) const
{
	if (what == SET_SPECIAL_MULTI)
		return cellinfo_of_cell(cell)->align_special;
	return column_info[column_of_cell(cell)].align_special;
}


int LyXTabular::GetWidthOfCell(int cell) const
{
	int const row = row_of_cell(cell);
	int const column1 = column_of_cell(cell);
	int const column2 = right_column_of_cell(cell);
	int result = 0;
	for (int i = column1; i <= column2; ++i) {
		result += cell_info[row][i].width_of_cell;
	}
	return result;
}


int LyXTabular::GetBeginningOfTextInCell(int cell) const
{
	int x = 0;
   
	switch (GetAlignment(cell)){
	case LYX_ALIGN_CENTER:
		x += (GetWidthOfColumn(cell) - GetWidthOfCell(cell)) / 2;
		break;
	case LYX_ALIGN_RIGHT:
		x += GetWidthOfColumn(cell) - GetWidthOfCell(cell);
		// + GetAdditionalWidth(cell);
		break;
	default: /* LYX_ALIGN_LEFT: nothing :-) */ 
		break;
	}
	
	// the LaTeX Way :-(
	x += WIDTH_OF_LINE;
	return x;
}


bool LyXTabular::IsFirstCellInRow(int cell) const
{
	return column_of_cell(cell) == 0;
}


int LyXTabular::GetFirstCellInRow(int row) const
{
	if (row > (rows_-1))
		row = rows_ - 1;
	return cell_info[row][0].cellno;
}

bool LyXTabular::IsLastCellInRow(int cell) const
{
	return (right_column_of_cell(cell) == (columns_ - 1));
}


int LyXTabular::GetLastCellInRow(int row) const
{
	if (row > (rows_-1))
		row = rows_ - 1;
	return cell_info[row][columns_-1].cellno;
}


bool LyXTabular::calculate_width_of_column(int column)
{
	int const old_column_width = column_info[column].width_of_column;
	int maximum = 0;
	
	for (int i = 0; i < rows_; ++i) {
		maximum = max(cell_info[i][column].width_of_cell, maximum);
	}
	column_info[column].width_of_column = maximum;
	return (column_info[column].width_of_column != old_column_width);
}


//
// calculate the with of the column without regarding REAL MultiColumn
// cells. This means MultiColumn-cells spanning more than 1 column.
//
bool LyXTabular::calculate_width_of_column_NMC(int column)
{
	int const old_column_width = column_info[column].width_of_column;
	int max = 0;
	for (int i = 0; i < rows_; ++i) {
		if (!IsMultiColumn(GetCellNumber(i, column), true) &&
			(cell_info[i][column].width_of_cell > max)) {
			max = cell_info[i][column].width_of_cell;
		}
	}
	column_info[column].width_of_column = max;
	return (column_info[column].width_of_column != old_column_width);
}


void LyXTabular::calculate_width_of_tabular()
{
	width_of_tabular = 0;
	for (int i = 0; i < columns_; ++i) {
		width_of_tabular += column_info[i].width_of_column;
	}
}


int LyXTabular::row_of_cell(int cell) const
{
	if (cell >= numberofcells)
		return rows_ - 1;
	else if (cell < 0)
		return 0;
	return rowofcell[cell];
}


int LyXTabular::column_of_cell(int cell) const
{
	if (cell >= numberofcells)
		return columns_ - 1;
	else if (cell < 0)
		return 0;
	return columnofcell[cell];
}


int LyXTabular::right_column_of_cell(int cell) const
{
	int const row = row_of_cell(cell);
	int column = column_of_cell(cell);
	while (column < (columns_ - 1) &&
		   cell_info[row][column + 1].multicolumn == LyXTabular::CELL_PART_OF_MULTICOLUMN)
		++column;
	return column;
}


void LyXTabular::Write(Buffer const * buf, ostream & os) const
{
	// header line
	os << "<lyxtabular"
	   << write_attribute("version", 3)
	   << write_attribute("rows", rows_)
	   << write_attribute("columns", columns_)
	   << ">\n";
	// global longtable options
	os << "<features"
	   << write_attribute("rotate", tostr(rotate))
	   << write_attribute("islongtable", tostr(is_long_tabular))
	   << write_attribute("firstHeadTopDL", tostr(endfirsthead.topDL))
	   << write_attribute("firstHeadBottomDL", tostr(endfirsthead.bottomDL))
	   << write_attribute("firstHeadEmpty", tostr(endfirsthead.empty))
	   << write_attribute("headTopDL", tostr(endhead.topDL))
	   << write_attribute("headBottomDL", tostr(endhead.bottomDL))
	   << write_attribute("footTopDL", tostr(endfoot.topDL))
	   << write_attribute("footBottomDL", tostr(endfoot.bottomDL))
	   << write_attribute("lastFootTopDL", tostr(endlastfoot.topDL))
	   << write_attribute("lastFootBottomDL", tostr(endlastfoot.bottomDL))
	   << write_attribute("lastFootEmpty", tostr(endlastfoot.empty))
	   << ">\n";
	for (int j = 0; j < columns_; ++j) {
		os << "<column"
		   << write_attribute("alignment", tostr(column_info[j].alignment))
		   << write_attribute("valignment", tostr(column_info[j].valignment))
		   << write_attribute("leftline", tostr(column_info[j].left_line))
		   << write_attribute("rightline", tostr(column_info[j].right_line))
		   << write_attribute("width", column_info[j].p_width.asString())
		   << write_attribute("special", column_info[j].align_special)
		   << ">\n";
	}
	for (int i = 0; i < rows_; ++i) {
		os << "<row"
		   << write_attribute("topline", tostr(row_info[i].top_line))
		   << write_attribute("bottomline", tostr(row_info[i].bottom_line))
		   << write_attribute("endhead", tostr(row_info[i].endhead))
		   << write_attribute("endfirsthead", tostr(row_info[i].endfirsthead))
		   << write_attribute("endfoot", tostr(row_info[i].endfoot))
		   << write_attribute("endlastfoot", tostr(row_info[i].endlastfoot))
		   << write_attribute("newpage", tostr(row_info[i].newpage))
		   << ">\n";
		for (int j = 0; j < columns_; ++j) {
			os << "<cell"
			   << write_attribute("multicolumn", cell_info[i][j].multicolumn)
			   << write_attribute("alignment", tostr(cell_info[i][j].alignment))
			   << write_attribute("valignment", tostr(cell_info[i][j].valignment))
			   << write_attribute("topline", tostr(cell_info[i][j].top_line))
			   << write_attribute("bottomline", tostr(cell_info[i][j].bottom_line))
			   << write_attribute("leftline", tostr(cell_info[i][j].left_line))
			   << write_attribute("rightline", tostr(cell_info[i][j].right_line))
			   << write_attribute("rotate", tostr(cell_info[i][j].rotate))
			   << write_attribute("usebox", tostr(cell_info[i][j].usebox))
			   << write_attribute("width", cell_info[i][j].p_width)
			   << write_attribute("special", cell_info[i][j].align_special)
			   << ">\n";
			os << "\\begin_inset ";
			cell_info[i][j].inset.write(buf, os);
			os << "\n\\end_inset \n"
			   << "</cell>\n";
		}
		os << "</row>\n";
	}
	os << "</lyxtabular>\n";
}


void LyXTabular::Read(Buffer const * buf, LyXLex & lex)
{
	string line;
	istream & is = lex.getStream();

	l_getline(is, line);
	if (!prefixIs(line, "<lyxtabular ")
		&& !prefixIs(line, "<LyXTabular ")) {
		OldFormatRead(lex, line);
		return;
	}

	int version;
	if (!getTokenValue(line, "version", version))
		return;
	if (version == 1)
		ReadOld(buf, is, lex, line);
	else if (version >= 2)
		ReadNew(buf, is, lex, line, version);
}

void LyXTabular::setHeaderFooterRows(int hr, int fhr, int fr, int lfr)
{
	// set header info
	while(hr > 0) {
		row_info[--hr].endhead = true;
	}
	// set firstheader info
	if (fhr && (fhr < rows_)) {
		if (row_info[fhr].endhead) {
			while(fhr > 0) {
				row_info[--fhr].endfirsthead = true;
				row_info[fhr].endhead = false;
			}
		} else if (row_info[fhr-1].endhead) {
			endfirsthead.empty = true;
		} else {
			while((fhr > 0) && !row_info[--fhr].endhead) {
				row_info[fhr].endfirsthead = true;
			}
		}
	}
	// set footer info
	if (fr && (fr < rows_)) {
		if (row_info[fr].endhead && row_info[fr-1].endhead) {
			while((fr > 0) && !row_info[--fr].endhead) {
				row_info[fr].endfoot = true;
				row_info[fr].endhead = false;
			}
		} else if (row_info[fr].endfirsthead && row_info[fr-1].endfirsthead) {
			while((fr > 0) && !row_info[--fr].endfirsthead) {
				row_info[fr].endfoot = true;
				row_info[fr].endfirsthead = false;
			}
		} else if (!row_info[fr-1].endhead && !row_info[fr-1].endfirsthead) {
			while((fr > 0) && !row_info[--fr].endhead &&
				  !row_info[fr].endfirsthead)
			{
				row_info[fr].endfoot = true;
			}
		}
	}
	// set lastfooter info
	if (lfr && (lfr < rows_)) {
		if (row_info[lfr].endhead && row_info[lfr-1].endhead) {
			while((lfr > 0) && !row_info[--lfr].endhead) {
				row_info[lfr].endlastfoot = true;
				row_info[lfr].endhead = false;
			}
		} else if (row_info[lfr].endfirsthead &&
				   row_info[lfr-1].endfirsthead)
		{
			while((lfr > 0) && !row_info[--lfr].endfirsthead) {
				row_info[lfr].endlastfoot = true;
				row_info[lfr].endfirsthead = false;
			}
		} else if (row_info[lfr].endfoot && row_info[lfr-1].endfoot) {
			while((lfr > 0) && !row_info[--lfr].endfoot) {
				row_info[lfr].endlastfoot = true;
				row_info[lfr].endfoot = false;
			}
		} else if (!row_info[fr-1].endhead && !row_info[fr-1].endfirsthead &&
				   !row_info[fr-1].endfoot)
		{
			while((lfr > 0) &&
				  !row_info[--lfr].endhead && !row_info[lfr].endfirsthead &&
				  !row_info[lfr].endfoot)
			{
				row_info[lfr].endlastfoot = true;
			}
		} else if (haveLTFoot()) {
			endlastfoot.empty = true;
		}
	}
}

void LyXTabular::ReadNew(Buffer const * buf, istream & is,
						 LyXLex & lex, string const & l, int const version)
{
	string line(l);
	int rows_arg;
	if (!getTokenValue(line, "rows", rows_arg))
		return;
	int columns_arg;
	if (!getTokenValue(line, "columns", columns_arg))
		return;
	Init(rows_arg, columns_arg);
	l_getline(is, line);
	if (!prefixIs(line, "<features")) {
		lyxerr << "Wrong tabular format (expected <features ...> got" <<
			line << ")" << endl;
		return;
	}
	getTokenValue(line, "rotate", rotate);
	getTokenValue(line, "islongtable", is_long_tabular);
	// compatibility read for old longtable options. Now we can make any
	// row part of the header/footer type we want before it was strict
	// sequential from the first row down (as LaTeX does it!). So now when
	// we find a header/footer line we have to go up the rows and set it
	// on all preceding rows till the first or one with already a h/f option
	// set. If we find a firstheader on the same line as a header or a
	// lastfooter on the same line as a footer then this should be set empty.
	// (Jug 20011220)
	if (version < 3) {
		int hrow;
		int fhrow;
		int frow;
		int lfrow;

		getTokenValue(line, "endhead", hrow);
		getTokenValue(line, "endfirsthead", fhrow);
		getTokenValue(line, "endfoot", frow);
		getTokenValue(line, "endlastfoot", lfrow);
		setHeaderFooterRows(abs(hrow), abs(fhrow), abs(frow), abs(lfrow));
	} else {
	   getTokenValue(line, "firstHeadTopDL", endfirsthead.topDL);
	   getTokenValue(line, "firstHeadBottomDL", endfirsthead.bottomDL);
	   getTokenValue(line, "firstHeadEmpty", endfirsthead.empty);
	   getTokenValue(line, "headTopDL", endhead.topDL);
	   getTokenValue(line, "headBottomDL", endhead.bottomDL);
	   getTokenValue(line, "footTopDL", endfoot.topDL);
	   getTokenValue(line, "footBottomDL", endfoot.bottomDL);
	   getTokenValue(line, "lastFootTopDL", endlastfoot.topDL);
	   getTokenValue(line, "lastFootBottomDL", endlastfoot.bottomDL);
	   getTokenValue(line, "lastFootEmpty", endlastfoot.empty);
	}
	for (int j = 0; j < columns_; ++j) {
		l_getline(is,line);
		if (!prefixIs(line,"<column")) {
			lyxerr << "Wrong tabular format (expected <column ...> got" <<
				line << ")" << endl;
			return;
		}
		getTokenValue(line, "alignment", column_info[j].alignment);
		getTokenValue(line, "valignment", column_info[j].valignment);
		getTokenValue(line, "leftline", column_info[j].left_line);
		getTokenValue(line, "rightline", column_info[j].right_line);
		getTokenValue(line, "width", column_info[j].p_width);
		getTokenValue(line, "special", column_info[j].align_special);
	}

	for (int i = 0; i < rows_; ++i) {
		l_getline(is, line);
		if (!prefixIs(line, "<row")) {
			lyxerr << "Wrong tabular format (expected <row ...> got" <<
				line << ")" << endl;
			return;
		}
		getTokenValue(line, "topline", row_info[i].top_line);
		getTokenValue(line, "bottomline", row_info[i].bottom_line);
		getTokenValue(line, "endfirsthead", row_info[i].endfirsthead);
		getTokenValue(line, "endhead", row_info[i].endhead);
		getTokenValue(line, "endfoot", row_info[i].endfoot);
		getTokenValue(line, "endlastfoot", row_info[i].endlastfoot);
		getTokenValue(line, "newpage", row_info[i].newpage);
		for (int j = 0; j < columns_; ++j) {
			l_getline(is, line);
			if (!prefixIs(line, "<cell")) {
				lyxerr << "Wrong tabular format (expected <cell ...> got" <<
					line << ")" << endl;
				return;
			}
			getTokenValue(line, "multicolumn", cell_info[i][j].multicolumn);
			getTokenValue(line, "alignment", cell_info[i][j].alignment);
			getTokenValue(line, "valignment", cell_info[i][j].valignment);
			getTokenValue(line, "topline", cell_info[i][j].top_line);
			getTokenValue(line, "bottomline", cell_info[i][j].bottom_line);
			getTokenValue(line, "leftline", cell_info[i][j].left_line);
			getTokenValue(line, "rightline", cell_info[i][j].right_line);
			getTokenValue(line, "rotate", cell_info[i][j].rotate);
			getTokenValue(line, "usebox", cell_info[i][j].usebox);
			getTokenValue(line, "width", cell_info[i][j].p_width);
			getTokenValue(line, "special", cell_info[i][j].align_special);
			l_getline(is, line);
			if (prefixIs(line, "\\begin_inset")) {
				cell_info[i][j].inset.read(buf, lex);
				l_getline(is, line);
			}
			if (!prefixIs(line, "</cell>")) {
				lyxerr << "Wrong tabular format (expected </cell> got" <<
					line << ")" << endl;
				return;
			}
		}
		l_getline(is, line);
		if (!prefixIs(line, "</row>")) {
			lyxerr << "Wrong tabular format (expected </row> got" <<
				line << ")" << endl;
			return;
		}
	}
	while (!prefixIs(line, "</lyxtabular>")) {
		l_getline(is, line);
	}
	set_row_column_number_info();
}


void LyXTabular::OldFormatRead(LyXLex & lex, string const & fl)
{
	int version;
	int i;
	int j;
	int rows_arg = 0;
	int columns_arg = 0;
	int is_long_tabular_arg = false;
	int rotate_arg = false;
	int a = -1;
	int b = -1;
	int c = -1;
	int d = -1;
	int e = 0;
	int f = 0;
	int g = 0;
	
	istream & is = lex.getStream();
	string s(fl);
	if (s.length() > 8)
		version = lyx::atoi(s.substr(8, string::npos));
	else
		version = 1;

	vector<int> cont_row_info;

	if (version < 5) {
		lyxerr << "Tabular format < 5 is not supported anymore\n"
			"Get an older version of LyX (< 1.1.x) for conversion!"
			   << endl;
		Alert::alert(_("Warning:"),
				   _("Tabular format < 5 is not supported anymore\n"),
				   _("Get an older version of LyX (< 1.1.x) for conversion!"));
		if (version > 2) {
			is >> rows_arg >> columns_arg >> is_long_tabular_arg
			   >> rotate_arg >> a >> b >> c >> d;
		} else
			is >> rows_arg >> columns_arg;
		Init(rows_arg, columns_arg);
		cont_row_info = vector<int>(rows_arg);
		SetLongTabular(is_long_tabular_arg);
		SetRotateTabular(rotate_arg);
		string tmp;
		for (i = 0; i < rows_; ++i) {
			getline(is, tmp);
			cont_row_info[i] = false;
		}
		for (i = 0; i < columns_; ++i) {
			getline(is, tmp);
		}
		for (i = 0; i < rows_; ++i) {
			for (j = 0; j < columns_; ++j) {
				getline(is, tmp);
			}
		}
	} else {
		is >> rows_arg >> columns_arg >> is_long_tabular_arg
		   >> rotate_arg >> a >> b >> c >> d;
		Init(rows_arg, columns_arg);
		cont_row_info = vector<int>(rows_arg);
		SetLongTabular(is_long_tabular_arg);
		SetRotateTabular(rotate_arg);
		setHeaderFooterRows(a+1, b+1 , c+1, d+1);
		for (i = 0; i < rows_; ++i) {
			a = b = c = d = e = f = g = 0;
			is >> a >> b >> c >> d;
			row_info[i].top_line = a;
			row_info[i].bottom_line = b;
			cont_row_info[i] = c;
			row_info[i].newpage = d;
		}
		for (i = 0; i < columns_; ++i) {
			string s1;
			string s2;
			is >> a >> b >> c;
#if 1
			char ch; // skip '"'
			is >> ch;
#else
			// ignore is buggy but we will use it later (Lgb)
			is.ignore(); // skip '"'
#endif    
			getline(is, s1, '"');
#if 1
			is >> ch; // skip '"'
#else
			// ignore is buggy but we will use it later (Lgb)
			is.ignore(); // skip '"'
#endif
			getline(is, s2, '"');
			column_info[i].alignment = static_cast<LyXAlignment>(a);
			column_info[i].left_line = b;
			column_info[i].right_line = c;
			column_info[i].p_width = LyXLength(s1);
			column_info[i].align_special = s2;
		}
		for (i = 0; i < rows_; ++i) {
			for (j = 0; j < columns_; ++j) {
				string s1;
				string s2;
				is >> a >> b >> c >> d >> e >> f >> g;
#if 1
				char ch;
				is >> ch; // skip '"'
#else
				// ignore is buggy but we will use it later (Lgb)
				is.ignore(); // skip '"'
#endif
				getline(is, s1, '"');
#if 1
				is >> ch; // skip '"'
#else
				// ignore is buggy but we will use it later (Lgb)
				is.ignore(); // skip '"'
#endif
				getline(is, s2, '"');
				cell_info[i][j].multicolumn = static_cast<char>(a);
				cell_info[i][j].alignment = static_cast<LyXAlignment>(b);
				cell_info[i][j].top_line = static_cast<char>(c);
				cell_info[i][j].bottom_line = static_cast<char>(d);
				cell_info[i][j].left_line = column_info[j].left_line;
				cell_info[i][j].right_line = column_info[j].right_line;
				cell_info[i][j].rotate = static_cast<bool>(f);
				cell_info[i][j].usebox = static_cast<BoxType>(g);
				cell_info[i][j].align_special = s1;
				cell_info[i][j].p_width = LyXLength(s2);
			}
		}
	}
	set_row_column_number_info(true);

	Paragraph * par = new Paragraph;
	Paragraph * return_par = 0;

	string tmptok;
	int pos = 0;
	Paragraph::depth_type depth = 0;
	LyXFont font(LyXFont::ALL_INHERIT);
	font.setLanguage(owner_->bufferOwner()->getLanguage());

	while (lex.isOK()) {
		lex.nextToken();
		string const token = lex.getString();
		if (token.empty())
			continue;
		if (token == "\\layout"
			|| token == "\\end_float"
			|| token == "\\end_deeper") {
			lex.pushToken(token);
#ifndef NO_COMPABILITY
			// Here we need to insert the inset_ert_contents into the last
			// cell of the tabular.
			owner_->bufferOwner()->insertErtContents(par, pos, font);
#endif
			break;
		}
		if (owner_->bufferOwner()->parseSingleLyXformat2Token(lex, par,
															  return_par,
															  token, pos,
															  depth, font)) {
			// the_end read
			lex.pushToken(token);
			break;
		}
		if (return_par) {
			lex.printError("New Paragraph allocated! This should not happen!");
			lex.pushToken(token);
			delete par;
			par = return_par;
			break;
		}
	}
	// now we have the par we should fill the insets with this!
	int cell = 0;
	InsetText * inset = GetCellInset(cell);
	int row;

	for (int i = 0; i < par->size(); ++i) {
		if (par->isNewline(i)) {
			++cell;
			if (cell > numberofcells) {
				lyxerr << "Some error in reading old table format occured!" <<
					endl << "Terminating when reading cell[" << cell << "]!" <<
					endl;
				delete par;
				return;
			}
			row = row_of_cell(cell);
			if (cont_row_info[row]) {
				DeleteRow(row);
				cont_row_info.erase(cont_row_info.begin() + row); //&cont_row_info[row]);
				while (!IsFirstCellInRow(--cell));
			} else {
				inset = GetCellInset(cell);
				continue;
			}
			inset = GetCellInset(cell);
			row = row_of_cell(cell);
			if (!cell_info[row_of_cell(cell)][column_of_cell(cell)].usebox)
			{
				// insert a space instead
				par->erase(i);
				par->insertChar(i, ' ');
			}
		}
		par->copyIntoMinibuffer(*owner_->bufferOwner(), i);
		inset->paragraph()->insertFromMinibuffer(inset->paragraph()->size());
	}
	delete par;
	Reinit();
}


bool LyXTabular::IsMultiColumn(int cell, bool real) const
{
	return ((!real || (column_of_cell(cell) != right_column_of_cell(cell))) &&
			(cellinfo_of_cell(cell)->multicolumn != LyXTabular::CELL_NORMAL));
}


LyXTabular::cellstruct * LyXTabular::cellinfo_of_cell(int cell) const
{
	int const row = row_of_cell(cell);
	int const column = column_of_cell(cell);
	return  &cell_info[row][column];
}


void LyXTabular::SetMultiColumn(int cell, int number)
{
	cellinfo_of_cell(cell)->multicolumn = CELL_BEGIN_OF_MULTICOLUMN;
	cellinfo_of_cell(cell)->alignment = column_info[column_of_cell(cell)].alignment;
	cellinfo_of_cell(cell)->top_line = row_info[row_of_cell(cell)].top_line;
	cellinfo_of_cell(cell)->bottom_line = row_info[row_of_cell(cell)].bottom_line;
	for (number--; number > 0; --number) {
		cellinfo_of_cell(cell+number)->multicolumn = CELL_PART_OF_MULTICOLUMN;
	}
	set_row_column_number_info();
}


int LyXTabular::cells_in_multicolumn(int cell) const
{
	int const row = row_of_cell(cell);
	int column = column_of_cell(cell);
	int result = 1;
	++column;
	while ((column < columns_) &&
		   cell_info[row][column].multicolumn == CELL_PART_OF_MULTICOLUMN)
	{
		++result;
		++column;
	}
	return result;
}


int LyXTabular::UnsetMultiColumn(int cell)
{
	int const row = row_of_cell(cell);
	int column = column_of_cell(cell);
	
	int result = 0;
	
	if (cell_info[row][column].multicolumn == CELL_BEGIN_OF_MULTICOLUMN) {
		cell_info[row][column].multicolumn = CELL_NORMAL;
		++column;
		while ((column < columns_) &&
			   (cell_info[row][column].multicolumn ==CELL_PART_OF_MULTICOLUMN))
		{
			cell_info[row][column].multicolumn = CELL_NORMAL;
			++column;
			++result;
		}
	}
	set_row_column_number_info();
	return result;
}


void LyXTabular::SetLongTabular(bool what)
{
	is_long_tabular = what;
}


bool LyXTabular::IsLongTabular() const
{
	return is_long_tabular;
}


void LyXTabular::SetRotateTabular(bool flag)
{
	rotate = flag;
}


bool LyXTabular::GetRotateTabular() const
{
	return rotate;
}


void LyXTabular::SetRotateCell(int cell, bool flag)
{
	cellinfo_of_cell(cell)->rotate = flag;
}


bool LyXTabular::GetRotateCell(int cell) const
{
	return cellinfo_of_cell(cell)->rotate;
}


bool LyXTabular::NeedRotating() const
{
	if (rotate)
		return true;
	for (int i = 0; i < rows_; ++i) {
		for (int j = 0; j < columns_; ++j) {
			if (cell_info[i][j].rotate)
				return true;
		}
	}
	return false;
}


bool LyXTabular::IsLastCell(int cell) const
{
	if ((cell + 1) < numberofcells)
		return false;
	return true;
}


int LyXTabular::GetCellAbove(int cell) const
{
	if (row_of_cell(cell) > 0)
		return cell_info[row_of_cell(cell)-1][column_of_cell(cell)].cellno;
	return cell;
}


int LyXTabular::GetCellBelow(int cell) const
{
	if (row_of_cell(cell) + 1 < rows_)
		return cell_info[row_of_cell(cell)+1][column_of_cell(cell)].cellno;
	return cell;
}


int LyXTabular::GetLastCellAbove(int cell) const
{
	if (row_of_cell(cell) <= 0)
		return cell;
	if (!IsMultiColumn(cell))
		return GetCellAbove(cell);
	return cell_info[row_of_cell(cell) - 1][right_column_of_cell(cell)].cellno;
}


int LyXTabular::GetLastCellBelow(int cell) const
{
	if (row_of_cell(cell) + 1 >= rows_)
		return cell;
	if (!IsMultiColumn(cell))
		return GetCellBelow(cell);
	return cell_info[row_of_cell(cell) + 1][right_column_of_cell(cell)].cellno;
}


int LyXTabular::GetCellNumber(int row, int column) const
{
#if 0
	if (column >= columns_)
		column = columns_ - 1;
	else if (column < 0)
		column = 0;
	if (row >= rows_)
		row = rows_ - 1;
	else if (row < 0)
		row = 0;
#else
	lyx::Assert(column >= 0 || column < columns_ || row >= 0 || row < rows_);
#endif
	return cell_info[row][column].cellno;
}


void LyXTabular::SetUsebox(int cell, BoxType type)
{
	cellinfo_of_cell(cell)->usebox = type;
}


LyXTabular::BoxType LyXTabular::GetUsebox(int cell) const
{
	if (column_info[column_of_cell(cell)].p_width.zero() &&
		!(IsMultiColumn(cell) && !cellinfo_of_cell(cell)->p_width.zero()))
		return BOX_NONE;
	if (cellinfo_of_cell(cell)->usebox > 1)
		return cellinfo_of_cell(cell)->usebox;
	return UseParbox(cell);
}


///
//  This are functions used for the longtable support
///
void LyXTabular::SetLTHead(int row, bool flag, ltType const & hd, bool first)
{
	if (first) {
		endfirsthead = hd;
		if (hd.set)
			row_info[row].endfirsthead = flag;
	} else {
		endhead = hd;
		if (hd.set)
			row_info[row].endhead = flag;
	}
}


bool LyXTabular::GetRowOfLTHead(int row, ltType & hd) const
{
	hd = endhead;
	hd.set = haveLTHead();
	return row_info[row].endhead;
}


bool LyXTabular::GetRowOfLTFirstHead(int row, ltType & hd) const
{
	hd = endfirsthead;
	hd.set = haveLTFirstHead();
	return row_info[row].endfirsthead;
}


void LyXTabular::SetLTFoot(int row, bool flag, ltType const & fd, bool last)
{
	if (last) {
		endlastfoot = fd;
		if (fd.set)
			row_info[row].endlastfoot = flag;
	} else {
		endfoot = fd;
		if (fd.set)
			row_info[row].endfoot = flag;
	}
}


bool LyXTabular::GetRowOfLTFoot(int row, ltType & fd) const
{
	fd = endfoot;
	fd.set = haveLTFoot();
	return row_info[row].endfoot;
}


bool LyXTabular::GetRowOfLTLastFoot(int row, ltType & fd) const
{
	fd = endlastfoot;
	fd.set = haveLTLastFoot();
	return row_info[row].endlastfoot;
}


void LyXTabular::SetLTNewPage(int row, bool what)
{
	row_info[row].newpage = what;
}


bool LyXTabular::GetLTNewPage(int row) const
{
	return row_info[row].newpage;
}


bool LyXTabular::haveLTHead() const
{
	for(int i=0; i < rows_; ++i) {
		if (row_info[i].endhead)
			return true;
	}
	return false;
}


bool LyXTabular::haveLTFirstHead() const
{
	if (endfirsthead.empty)
		return false;
	for(int i=0; i < rows_; ++i) {
		if (row_info[i].endfirsthead)
			return true;
	}
	return false;
}


bool LyXTabular::haveLTFoot() const
{
	for(int i=0; i < rows_; ++i) {
		if (row_info[i].endfoot)
			return true;
	}
	return false;
}


bool LyXTabular::haveLTLastFoot() const
{
	if (endlastfoot.empty)
		return false;
	for(int i=0; i < rows_; ++i) {
		if (row_info[i].endlastfoot)
			return true;
	}
	return false;
}


// end longtable support functions

bool LyXTabular::SetAscentOfRow(int row, int height)
{
	if ((row >= rows_) || (row_info[row].ascent_of_row == height))
		return false;
	row_info[row].ascent_of_row = height;
	return true;
}


bool LyXTabular::SetDescentOfRow(int row, int height)
{
	if ((row >= rows_) || (row_info[row].descent_of_row == height))
		return false;
	row_info[row].descent_of_row = height;
	return true;
}


int LyXTabular::GetAscentOfRow(int row) const
{
	if (row >= rows_)
		return 0;
	return row_info[row].ascent_of_row;
}


int LyXTabular::GetDescentOfRow(int row) const
{
	if (row >= rows_)
		return 0;
	return row_info[row].descent_of_row;
}


int LyXTabular::GetHeightOfTabular() const
{
	int height = 0;

	for (int row = 0; row < rows_; ++row)
		height += GetAscentOfRow(row) + GetDescentOfRow(row) +
			GetAdditionalHeight(row);
	return height;
}


bool LyXTabular::IsPartOfMultiColumn(int row, int column) const
{
	if ((row >= rows_) || (column >= columns_))
		return false;
	return (cell_info[row][column].multicolumn == CELL_PART_OF_MULTICOLUMN);
}


int LyXTabular::TeXTopHLine(ostream & os, int row) const
{
	if ((row < 0) || (row >= rows_))
		return 0;

	int const fcell = GetFirstCellInRow(row);
	int const n = NumberOfCellsInRow(fcell) + fcell;
	int tmp = 0;

	for (int i = fcell; i < n; ++i) {
		if (TopLine(i))
			++tmp;
	}
	if (tmp == (n - fcell)){
		os << "\\hline ";
	} else if (tmp) {
		for (int i = fcell; i < n; ++i) {
			if (TopLine(i)) {
				os << "\\cline{"
				   << column_of_cell(i) + 1
				   << '-'
				   << right_column_of_cell(i) + 1
				   << "} ";
			}
		}
	} else {
		return 0;
	}
	os << "\n";
	return 1;
}


int LyXTabular::TeXBottomHLine(ostream & os, int row) const
{
	if ((row < 0) || (row >= rows_))
		return 0;

	int const fcell = GetFirstCellInRow(row);
	int const n = NumberOfCellsInRow(fcell) + fcell;
	int tmp = 0;

	for (int i = fcell; i < n; ++i) {
		if (BottomLine(i))
			++tmp;
	}
	if (tmp == (n-fcell)){
		os << "\\hline";
	} else if (tmp) {
		for (int i = fcell; i < n; ++i) {
			if (BottomLine(i)) {
				os << "\\cline{"
				   << column_of_cell(i) + 1
				   << '-'
				   << right_column_of_cell(i) + 1
				   << "} ";
			}
		}
	} else {
		return 0;
	}
	os << "\n";
	return 1;
}


int LyXTabular::TeXCellPreamble(ostream & os, int cell) const
{
	int ret = 0;

	if (GetRotateCell(cell)) {
		os << "\\begin{sideways}\n";
		++ret;
	}
	if (IsMultiColumn(cell)) {
		os << "\\multicolumn{" << cells_in_multicolumn(cell) << "}{";
		if (!cellinfo_of_cell(cell)->align_special.empty()) {
			os << cellinfo_of_cell(cell)->align_special << "}{";
		} else {
			if (LeftLine(cell) &&
				(IsFirstCellInRow(cell) || 
				 (!IsMultiColumn(cell-1) && !LeftLine(cell, true) &&
				  !RightLine(cell-1, true))))
			{
				os << '|';
			}
			if (!GetPWidth(cell).zero()) {
				switch (GetVAlignment(cell)) {
				case LYX_VALIGN_TOP:
					os << "p";
					break;
				case LYX_VALIGN_CENTER:
					os << "m";
					break;
				case LYX_VALIGN_BOTTOM:
					os << "b";
					break;
				}
				os << "{" << GetPWidth(cell).asLatexString() << '}';
			} else {
				switch (GetAlignment(cell)) {
				case LYX_ALIGN_LEFT:
					os << 'l';
					break;
				case LYX_ALIGN_RIGHT:
					os << 'r';
					break;
				default:
					os << 'c';
					break;
				}
			}
			if (RightLine(cell))
				os << '|';
			if (((cell + 1) < numberofcells) && !IsFirstCellInRow(cell+1) &&
				LeftLine(cell+1))
				os << '|';
			os << "}{";
		}
	}
	if (GetUsebox(cell) == BOX_PARBOX) {
		os << "\\parbox[";
		switch (GetVAlignment(cell)) {
		case LYX_VALIGN_TOP:
			os << "t";
			break;
		case LYX_VALIGN_CENTER:
			os << "c";
			break;
		case LYX_VALIGN_BOTTOM:
			os << "b";
			break;
		}
		os << "]{" << GetPWidth(cell).asLatexString() << "}{";
	} else if (GetUsebox(cell) == BOX_MINIPAGE) {
		os << "\\begin{minipage}[";
		switch (GetVAlignment(cell)) {
		case LYX_VALIGN_TOP:
			os << "t";
			break;
		case LYX_VALIGN_CENTER:
			os << "m";
			break;
		case LYX_VALIGN_BOTTOM:
			os << "b";
			break;
		}
		os << "]{" << GetPWidth(cell).asLatexString() << "}\n";
		++ret;
	}
	return ret;
}


int LyXTabular::TeXCellPostamble(ostream & os, int cell) const
{
	int ret = 0;

	// usual cells
	if (GetUsebox(cell) == BOX_PARBOX)
		os << "}";
	else if (GetUsebox(cell) == BOX_MINIPAGE) {
		os << "%\n\\end{minipage}";
		ret += 2;
	}
	if (IsMultiColumn(cell)){
		os << '}';
	}
	if (GetRotateCell(cell)) {
		os << "%\n\\end{sideways}";
		++ret;
	}
	return ret;
}


int LyXTabular::TeXLongtableHeaderFooter(ostream & os, Buffer const * buf,
                                         bool fragile, bool fp) const
{
	if (!is_long_tabular)
		return 0;

	int ret = 0;
	// output header info
	if (haveLTHead()) {
		if (endhead.topDL) {
			os << "\\hline\n";
			++ret;
		}
		for (int i = 0; i < rows_; ++i) {
			if (row_info[i].endhead) {
				ret += TeXRow(os, i, buf, fragile, fp);
			}
		}
		if (endhead.bottomDL) {
			os << "\\hline\n";
			++ret;
		}
		os << "\\endhead\n";
		++ret;
		if (endfirsthead.empty) {
			os << "\\endfirsthead\n";
			++ret;
		}
	}
	// output firstheader info
	if (haveLTFirstHead()) {
		if (endfirsthead.topDL) {
			os << "\\hline\n";
			++ret;
		}
		for (int i = 0; i < rows_; ++i) {
			if (row_info[i].endfirsthead) {
				ret += TeXRow(os, i, buf, fragile, fp);
			}
		}
		if (endfirsthead.bottomDL) {
			os << "\\hline\n";
			++ret;
		}
		os << "\\endfirsthead\n";
		++ret;
	}
	// output footer info
	if (haveLTFoot()) {
		if (endfoot.topDL) {
			os << "\\hline\n";
			++ret;
		}
		for (int i = 0; i < rows_; ++i) {
			if (row_info[i].endfoot) {
				ret += TeXRow(os, i, buf, fragile, fp);
			}
		}
		if (endfoot.bottomDL) {
			os << "\\hline\n";
			++ret;
		}
		os << "\\endfoot\n";
		++ret;
		if (endlastfoot.empty) {
			os << "\\endlastfoot\n";
			++ret;
		}
	}
	// output lastfooter info
	if (haveLTLastFoot()) {
		if (endlastfoot.topDL) {
			os << "\\hline\n";
			++ret;
		}
		for (int i = 0; i < rows_; ++i) {
			if (row_info[i].endlastfoot) {
				ret += TeXRow(os, i, buf, fragile, fp);
			}
		}
		if (endlastfoot.bottomDL) {
			os << "\\hline\n";
			++ret;
		}
		os << "\\endlastfoot\n";
		++ret;
	}
	return ret;
}


bool LyXTabular::isValidRow(int const row) const
{
	if (!is_long_tabular)
		return true;
	return (!row_info[row].endhead && !row_info[row].endfirsthead &&
			!row_info[row].endfoot && !row_info[row].endlastfoot);
}


int LyXTabular::TeXRow(ostream & os, int const i, Buffer const * buf,
                       bool fragile, bool fp) const
{
	int ret = 0;
	int cell = GetCellNumber(i, 0);

	ret += TeXTopHLine(os, i);
	for (int j = 0; j < columns_; ++j) {
		if (IsPartOfMultiColumn(i,j))
			continue;
		ret += TeXCellPreamble(os, cell);
		InsetText * inset = GetCellInset(cell);

		bool rtl = inset->paragraph()->isRightToLeftPar(buf->params) &&
			inset->paragraph()->size() > 0 && GetPWidth(cell).zero();

		if (rtl)
			os << "\\R{";
		ret += inset->latex(buf, os, fragile, fp);
		if (rtl)
			os << "}";

		ret += TeXCellPostamble(os, cell);
		if (!IsLastCellInRow(cell)) { // not last cell in row
			os << "&\n";
			++ret;
		}
		++cell;
	}
	os << "\\\\\n";
	++ret;
	ret += TeXBottomHLine(os, i);
	return ret;
}


int LyXTabular::latex(Buffer const * buf,
					  ostream & os, bool fragile, bool fp) const
{
	int ret = 0;

	//+---------------------------------------------------------------------
	//+                      first the opening preamble                    +
	//+---------------------------------------------------------------------

	if (rotate) {
		os << "\\begin{sideways}\n";
		++ret;
	}
	if (is_long_tabular)
		os << "\\begin{longtable}{";
	else
		os << "\\begin{tabular}{";
	for (int i = 0; i < columns_; ++i) {
		if (!column_info[i].align_special.empty()) {
			os << column_info[i].align_special;
		} else { 
			if (column_info[i].left_line)
				os << '|';
			if (!column_info[i].p_width.zero()) {
				switch (column_info[i].valignment) {
				case LYX_VALIGN_TOP:
					os << "p";
					break;
				case LYX_VALIGN_CENTER:
					os << "m";
					break;
				case LYX_VALIGN_BOTTOM:
					os << "b";
					break;
			}
				os << "{"
				   << column_info[i].p_width.asLatexString()
				   << '}';
			} else {
				switch (column_info[i].alignment) {
				case LYX_ALIGN_LEFT:
					os << 'l';
					break;
				case LYX_ALIGN_RIGHT:
				os << 'r';
				break;
				default:
					os << 'c';
					break;
				}
			}
			if (column_info[i].right_line)
				os << '|';
		}
	}
	os << "}\n";
	++ret;

	ret += TeXLongtableHeaderFooter(os, buf, fragile, fp);
	
	//+---------------------------------------------------------------------
	//+                      the single row and columns (cells)            +
	//+---------------------------------------------------------------------

	for (int i = 0; i < rows_; ++i) {
		if (isValidRow(i)) {
			ret += TeXRow(os, i, buf, fragile, fp);
			if (is_long_tabular && row_info[i].newpage) {
				os << "\\newpage\n";
				++ret;
			}
		}
	}

	//+---------------------------------------------------------------------
	//+                      the closing of the tabular                    +
	//+---------------------------------------------------------------------

	if (is_long_tabular)
		os << "\\end{longtable}";
	else
		os << "\\end{tabular}";
	if (rotate) {
		os << "\n\\end{sideways}";
		++ret;
	}

	return ret;
}


int LyXTabular::docbookRow(Buffer const * buf, ostream & os, int row) const
{
	int ret = 0;
	int cell = GetFirstCellInRow(row);
	
	os << "<row>\n";
	for (int j = 0; j < columns_; ++j) {
		if (IsPartOfMultiColumn(row, j))
			continue;

		os << "<entry align=\"";
		switch (GetAlignment(cell)) {
		case LYX_ALIGN_LEFT:
			os << "left";
			break;
		case LYX_ALIGN_RIGHT:
			os << "right";
			break;
		default:
			os << "center";
			break;
		}

		os << "\" valign=\"";
		switch (GetVAlignment(cell)) {
		case LYX_VALIGN_TOP:
			os << "top";
			break;
		case LYX_VALIGN_BOTTOM:
			os << "bottom";
			break;
		case LYX_VALIGN_CENTER:
			os << "middle";
		}
		os << "\"";

		if (IsMultiColumn(cell)) {
			os << " namest=\"col" << j << "\" ";
			os << "nameend=\"col" << j + cells_in_multicolumn(cell) - 1<< "\"";
		}

		os << ">";
		ret += GetCellInset(cell)->docbook(buf, os);
		os << "</entry>\n";
		++cell;
	}
	os << "</row>\n";
	return ret;
}	


int LyXTabular::docBook(Buffer const * buf, ostream & os) const
{
	int ret = 0;

	//+---------------------------------------------------------------------
	//+                      first the opening preamble                    +
	//+---------------------------------------------------------------------

	os << "<tgroup cols=\"" << columns_
	   << "\" colsep=\"1\" rowsep=\"1\">\n";
	
	for (int i = 0; i < columns_; ++i) {
		os << "<colspec colname=\"col" << i << "\" align=\"";
		switch (column_info[i].alignment) {
		case LYX_ALIGN_LEFT:
			os << "left";
			break;
		case LYX_ALIGN_RIGHT:
			os << "right";
			break;
		default:
			os << "center";
			break;
		}
		os << "\">\n";
		++ret;
	}

	//+---------------------------------------------------------------------
	//+                      Long Tabular case                             +
	//+---------------------------------------------------------------------

#warning Jose please have a look here I changed the longtable header/footer
#warning ---- options so I had to disable the docbook code (Jug 20011219)
#if 0
	if ( IsLongTabular() ) {
		// Header
		if(endhead.row || endfirsthead.row ) {
			os << "<thead>\n";
			if( endfirsthead.row ) {
				docbookRow( buf, os, endfirsthead.row - 1);
			}
			if( endhead.row && endhead.row != endfirsthead.row) {
				docbookRow(buf, os, endhead.row - 1);
			}
			os << "</thead>\n";
		}

		// Footer
		if( endfoot.row || endlastfoot.row ) {
			os << "<tfoot>\n";
			if( endfoot.row ) {
				docbookRow( buf, os, endfoot.row - 1);
			}
			if( endlastfoot.row && endlastfoot.row != endfoot.row) {
				docbookRow( buf, os, endlastfoot.row - 1);
			}
			os << "</tfoot>\n";
		}
	}
#endif
	//+---------------------------------------------------------------------
	//+                      the single row and columns (cells)            +
	//+---------------------------------------------------------------------

	os << "<tbody>\n";
	for (int i = 0; i < rows_; ++i) {
		if(!IsLongTabular() || (
		   !row_info[i].endhead && !row_info[i].endfirsthead &&
		   !row_info[i].endfoot && !row_info[i].endlastfoot)) {
			docbookRow( buf, os, i);
		}
	}
	os << "</tbody>\n";
	//+---------------------------------------------------------------------
	//+                      the closing of the tabular                    +
	//+---------------------------------------------------------------------

	os << "</tgroup>";
	++ret;

	return ret;
}

//--
// ASCII export function and helpers
//--
int LyXTabular::asciiTopHLine(ostream & os, int row,
                              vector<unsigned int> const & clen) const
{
	int const fcell = GetFirstCellInRow(row);
	int const n = NumberOfCellsInRow(fcell) + fcell;
	int tmp = 0;

	for (int i = fcell; i < n; ++i) {
		if (TopLine(i)) {
			++tmp;
			break;
		}
	}
	if (!tmp)
		return 0;

	unsigned char ch;
	for (int i = fcell; i < n; ++i) {
		if (TopLine(i)) {
			if (LeftLine(i))
				os << "+-";
			else
				os << "--";
			ch = '-';
		} else {
			os << "  ";
			ch = ' ';
		}
		int column = column_of_cell(i);
		int len = clen[column];
		while (IsPartOfMultiColumn(row, ++column))
			len += clen[column] + 4;
		os << string(len, ch);
		if (TopLine(i)) {
			if (RightLine(i))
				os << "-+";
			else
				os << "--";
		} else {
			os << "  ";
		}
	}
	os << endl;
	return 1;
}


int LyXTabular::asciiBottomHLine(ostream & os, int row,
                                 vector<unsigned int> const & clen) const
{
	int const fcell = GetFirstCellInRow(row);
	int const n = NumberOfCellsInRow(fcell) + fcell;
	int tmp = 0;

	for (int i = fcell; i < n; ++i) {
		if (BottomLine(i)) {
			++tmp;
			break;
		}
	}
	if (!tmp)
		return 0;

	unsigned char ch;
	for (int i = fcell; i < n; ++i) {
		if (BottomLine(i)) {
			if (LeftLine(i))
				os << "+-";
			else
				os << "--";
			ch = '-';
		} else {
			os << "  ";
			ch = ' ';
		}
		int column = column_of_cell(i);
		int len = clen[column];
		while (IsPartOfMultiColumn(row, ++column))
			len += clen[column] + 4;
		os << string(len, ch);
		if (BottomLine(i)) {
			if (RightLine(i))
				os << "-+";
			else
				os << "--";
		} else {
			os << "  ";
		}
	}
	os << endl;
	return 1;
}


int LyXTabular::asciiPrintCell(Buffer const * buf, ostream & os,
                               int cell, int row, int column,
                               vector<unsigned int> const & clen,
                               bool onlydata) const
{
	ostringstream sstr;
	int ret = GetCellInset(cell)->ascii(buf, sstr, 0);

	if (onlydata) {
		os << sstr.str();
		return ret;
	}
	
	if (LeftLine(cell))
		os << "| ";
	else
		os << "  ";

	unsigned int len1 = sstr.str().length();
	unsigned int len2 = clen[column];
	while (IsPartOfMultiColumn(row, ++column))
		len2 += clen[column] + 4;
	len2 -= len1;

	switch (GetAlignment(cell)) {
	default:
	case LYX_ALIGN_LEFT:
		len1 = 0;
		break;
	case LYX_ALIGN_RIGHT:
		len1 = len2;
		len2 = 0;
		break;
	case LYX_ALIGN_CENTER:
		len1 = len2 / 2;
		len2 -= len1;
		break;
	}

	for (unsigned int i = 0; i < len1; ++i)
		os << " ";
	os << sstr.str();
	for (unsigned int i = 0; i < len2; ++i)
		os << " ";
	if (RightLine(cell))
		os << " |";
	else
		os << "  ";

	return ret;
}


int LyXTabular::ascii(Buffer const * buf, ostream & os, int const depth,
					  bool onlydata, unsigned char delim) const
{
	int ret = 0;

	//+---------------------------------------------------------------------
	//+           first calculate the width of the single columns          +
	//+---------------------------------------------------------------------
	vector<unsigned int> clen(columns_);

	if (!onlydata) {
		// first all non (real) multicolumn cells!
		for (int j = 0; j < columns_; ++j) {
			clen[j] = 0;
			for (int i = 0; i < rows_; ++i) {
				int cell = GetCellNumber(i, j);
				if (IsMultiColumn(cell, true))
					continue;
				ostringstream sstr;
				GetCellInset(cell)->ascii(buf, sstr, 0);
				if (clen[j] < sstr.str().length())
					clen[j] = sstr.str().length();
			}
		}
		// then all (real) multicolumn cells!
		for (int j = 0; j < columns_; ++j) {
			for (int i = 0; i < rows_; ++i) {
				int cell = GetCellNumber(i, j);
				if (!IsMultiColumn(cell, true) || IsPartOfMultiColumn(i, j))
					continue;
				ostringstream sstr;
				GetCellInset(cell)->ascii(buf, sstr, 0);
				int len = int(sstr.str().length());
				int const n = cells_in_multicolumn(cell);
				for (int k = j; (len > 0) && (k < (j + n - 1)); ++k)
					len -= clen[k];
				if (len > int(clen[j + n - 1]))
					clen[j + n - 1] = len;
			}
		}
	}
	int cell = 0;
	for (int i = 0; i < rows_; ++i) {
		if (!onlydata) {
			if (asciiTopHLine(os, i, clen)) {
				for (int j = 0; j < depth; ++j)
					os << "  ";
			}
		}
		for (int j = 0; j < columns_; ++j) {
			if (IsPartOfMultiColumn(i,j))
				continue;
			if (onlydata && j > 0)
				os << delim;
			ret += asciiPrintCell(buf, os, cell, i, j, clen, onlydata);
			++cell;
		}
		os << endl;
		if (!onlydata) {
			for (int j = 0; j < depth; ++j)
				os << "  ";
			if (asciiBottomHLine(os, i, clen)) {
				for (int j = 0; j < depth; ++j)
					os << "  ";
			}
		}
	}
	return ret;
}
//--
// end ascii export
//--


InsetText * LyXTabular::GetCellInset(int cell) const
{
	cur_cell = cell;
	return & cell_info[row_of_cell(cell)][column_of_cell(cell)].inset;
}


InsetText * LyXTabular::GetCellInset(int row, int column) const
{
	cur_cell = GetCellNumber(row, column);
	return & cell_info[row][column].inset;
}


void LyXTabular::Validate(LaTeXFeatures & features) const
{
	if (IsLongTabular())
		features.require("longtable");
	if (NeedRotating())
		features.require("rotating");
	for (int cell = 0; !features.isRequired("array") && (cell < numberofcells); ++cell) {
		if (GetVAlignment(cell) != LYX_VALIGN_TOP)
			features.require("array");
		GetCellInset(cell)->validate(features);
	}
}


std::vector<string> const LyXTabular::getLabelList() const
{
	std::vector<string> label_list;
	for (int i = 0; i < rows_; ++i)
		for (int j = 0; j < columns_; ++j) {
			std::vector<string> const l =
				GetCellInset(i, j)->getLabelList();
			label_list.insert(label_list.end(),
					  l.begin(), l.end());
		}
	return label_list;
}

			
LyXTabular::BoxType LyXTabular::UseParbox(int cell) const
{
	Paragraph * par = GetCellInset(cell)->paragraph();

	for (; par; par = par->next()) {
		for (int i = 0; i < par->size(); ++i) {
			if (par->getChar(i) == Paragraph::META_NEWLINE)
				return BOX_PARBOX;
		}
	}
	return BOX_NONE;
}
