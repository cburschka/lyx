/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *        Copyright 2000 The LyX Team.
 *
 * ====================================================== 
 */

#include <config.h>

#include <algorithm>
#include <cstdlib>

#ifdef __GNUG__
#pragma implementation
#endif

#include "tabular.h"
#include "debug.h"
#include "vspace.h"
#include "layout.h"
#include "lyx_gui_misc.h"
#include "buffer.h"
#include "BufferView.h"
#include "Painter.h"
#include "LaTeXFeatures.h"
#include "support/lstrings.h"
#include "support/lyxmanip.h"
#include "insets/insettabular.h"
#include "insets/insettext.h"

using std::ostream;
using std::istream;
using std::getline;
using std::max;
using std::endl;
using std::vector;

static int const WIDTH_OF_LINE = 5;

/// Define a few methods for the inner structs

LyXTabular::cellstruct::cellstruct() 
{
    cellno = 0; //should be initilaized correctly later.
    width_of_cell = 0;
    multicolumn = LyXTabular::CELL_NORMAL;
    alignment = LYX_ALIGN_CENTER;
    top_line = true;
    bottom_line = false;
    rotate = false;
    linebreaks = false;
    inset = 0;
}


LyXTabular::cellstruct::~cellstruct() 
{
    delete inset;
}


LyXTabular::cellstruct::cellstruct(cellstruct const & cs)
{
    cellno = cs.cellno;
    width_of_cell = cs.width_of_cell;
    multicolumn = cs.multicolumn;
    alignment = cs.alignment;
    top_line = cs.top_line;
    bottom_line = cs.bottom_line;
    rotate = cs.rotate;
    linebreaks = cs.linebreaks;
    inset = 0;
}


LyXTabular::cellstruct & 
LyXTabular::cellstruct::operator=(cellstruct const & cs)
{
    cellno = cs.cellno;
    width_of_cell = cs.width_of_cell;
    multicolumn = cs.multicolumn;
    alignment = cs.alignment;
    top_line = cs.top_line;
    bottom_line = cs.bottom_line;
    rotate = cs.rotate;
    linebreaks = cs.linebreaks;
    return *this;
}


LyXTabular::rowstruct::rowstruct() 
{
    top_line = true;
    bottom_line = false;
    ascent_of_row = 0;
    descent_of_row = 0;
    newpage = false;
}


LyXTabular::columnstruct::columnstruct() 
{
    left_line = true;
    right_line = false;
    alignment = LYX_ALIGN_CENTER;
    width_of_column = 0;
}


/* konstruktor */
LyXTabular::LyXTabular(InsetTabular * inset, int rows_arg, int columns_arg)
{
    owner_ = inset;
    Init(rows_arg, columns_arg);
}


LyXTabular::LyXTabular(InsetTabular * inset, LyXTabular const & lt)
{
    owner_ = inset;
    Init(lt.rows_, lt.columns_);
    
    operator=(lt);
}


LyXTabular::LyXTabular(InsetTabular * inset, LyXLex & lex)
{
    owner_ = inset;
    Read(lex);
}


LyXTabular::~LyXTabular()
{
    delete[] rowofcell;
    delete[] columnofcell;
}


LyXTabular & LyXTabular::operator=(LyXTabular const & lt)
{
    // If this and lt is not of the same size we have a serious bug
    // So then it is ok to throw an exception, or for now
    // call abort()
    Assert(rows_ == lt.rows_ && columns_ == lt.columns_);

    cell_info = lt.cell_info;
    row_info = lt.row_info;
    column_info = lt.column_info;

    // long tabular stuff
    SetLongTabular(lt.is_long_tabular);
    endhead = lt.endhead;
    endfoot = lt.endfoot;
    endfirsthead = lt.endfirsthead;
    endlastfoot = lt.endlastfoot;

    rotate = lt.rotate;

    Reinit();
    
    return *this;
}


LyXTabular * LyXTabular::Clone(InsetTabular * inset)
{
    LyXTabular * result = new LyXTabular(inset, *this);
    ///
    // don't know if this is good but I need to Clone also
    // the text-insets here, this is for the Undo-facility!
    ///
    int i,j;
    for(i=0; i < rows_; ++i) {
	for(j=0; j < columns_; ++j) {
	    delete result->cell_info[i][j].inset;
	    result->cell_info[i][j].inset=new InsetText(*cell_info[i][j].inset,
							inset->BufferOwner());
	    result->cell_info[i][j].inset->setOwner(inset);
	}
    }
    return result;
}


/* activates all lines and sets all widths to 0 */ 
void LyXTabular::Init(int rows_arg, int columns_arg)
{
    int i, j;
    int cellno = 0;

    rows_ = rows_arg;
    columns_ = columns_arg;
    row_info = vector<rowstruct>(rows_, rowstruct());
    column_info = vector<columnstruct>(columns_, columnstruct());
    cell_info = vector<vector<cellstruct> >
	    (rows_, vector<cellstruct>(columns_, cellstruct()));

    // Jürgen, use iterators.
    for (i = 0; i < rows_; ++i) {
        for (j = 0; j < columns_; ++j) {
            cell_info[i][j].inset = new InsetText(owner_->BufferOwner());
	    cell_info[i][j].inset->setOwner(owner_);
	    cell_info[i][j].inset->SetDrawLockedFrame(true);
            cell_info[i][j].cellno = cellno++;
        }
    }
    row_info[i-1].bottom_line = true;
    row_info[0].bottom_line = true;

    for (i = 0; i < columns_; ++i) {
        calculate_width_of_column(i);
    }
    column_info[columns_-1].right_line = true;
   
    calculate_width_of_tabular();

    rowofcell = 0;
    columnofcell = 0;
    set_row_column_number_info();
    is_long_tabular = false;
    rotate = 0;
    endhead = -1;
    endfirsthead = -1;
    endfoot = -1;
    endlastfoot = -1;
}


void LyXTabular::AppendRow(int /* cell */)
{
#if 0
    int row = row_of_cell(cell);
    rowstruct * row_info2 = new rowstruct[rows_ + 1];
    cellstruct ** cell_info2 = new cellstruct * [rows_ + 1];
    int i;

    for (i = 0; i <= row; ++i) {
        cell_info2[i] = cell_info[i];
        row_info2[i] = row_info[i];
    }
    for (i = rows_ - 1; i >= row; --i) {
        cell_info2[i + 1] = cell_info[i];
        row_info2[i + 1] = row_info[i];
    }
    row_info2[row + 1].top_line = row_info[i].top_line;
    cell_info2[row + 1] = new cellstruct[columns_](owner_->BufferOwner());
    for (i = 0; i < columns_; ++i) {
        cell_info2[row + 1][i].width_of_cell = 0;
        cell_info2[row + 1][i] = cell_info2[row][i];
    }
   
    delete[] cell_info;
    cell_info = cell_info2;
    delete[] row_info;
    row_info = row_info2;
   
    ++rows_;
   
    Reinit();
#endif
}


void LyXTabular::DeleteRow(int row)
{
	row_info.erase(row_info.begin() + row); //&row_info[row]);
	cell_info.erase(cell_info.begin() + row); //&cell_info[row]);
	--rows_;
	Reinit();
}


void LyXTabular::AppendColumn(int /*cell*/)
{
#if 0
    int j;
    columnstruct * column_info2 = new columnstruct[columns_ + 1];
    int column = right_column_of_cell(cell);

    int i = 0;
    for (; i <= column; ++i) {
        column_info2[i] = column_info[i];
    }
    for (i = columns_ - 1; i >= column; --i) {
        column_info2[i + 1] = column_info[i];
    }
    
    delete[] column_info;
    column_info = column_info2;
    
    for (i = 0; i < rows_; ++i) {
        cellstruct * tmp = cell_info[i];
        cell_info[i] = new cellstruct[columns_ + 1](owner_->BufferOwner());
        for (j = 0; j <= column; ++j) {
            cell_info[i][j] = tmp[j];
        }
        for (j = column; j < columns_; ++j) {
            cell_info[i][j + 1] = tmp[j];
        }
        // care about multicolumns
        if (cell_info[i][column + 1].multicolumn
            == LyXTabular::CELL_BEGIN_OF_MULTICOLUMN){
            cell_info[i][column + 1].multicolumn = 
                LyXTabular::CELL_PART_OF_MULTICOLUMN;
        }
        if (column + 1 == columns_
            || cell_info[i][column + 2].multicolumn
            != LyXTabular::CELL_PART_OF_MULTICOLUMN){
            cell_info[i][column + 1].multicolumn = 
                LyXTabular::CELL_NORMAL;
        }
        delete[] tmp;
    }
    
    ++columns_;
    Reinit();
#endif
}


void LyXTabular::DeleteColumn(int /*cell*/)
{
#if 0
    int column1 = column_of_cell(cell);
    int column2 = right_column_of_cell(cell);
    
    if (column1 == 0 && column2 == columns_ - 1)
	return;
    
    for (int column = column1; column <= column2; ++column) {
	delete_column(column1);
    }
    Reinit();
#endif
}


void LyXTabular::delete_column(int /*column*/)
{
#if 0
    int i, j;
    columnstruct * column_info2 = new columnstruct[columns_-1];
   
    for (i = 0; i < column; ++i) {
        column_info2[i] = column_info[i];
    }
    for (i = column; i < columns_ - 1; ++i) {
        column_info2[i] = column_info[i + 1];
    }
   
    delete[] column_info;
    column_info = column_info2;

    for (i = 0; i < rows_; ++i) {
        cellstruct * tmp = cell_info[i];
        cell_info[i] = new cellstruct[columns_ - 1](owner_->BufferOwner());
        for (j = 0; j < column; ++j) {
            cell_info[i][j] = tmp[j];
        }
        for (j = column; j < columns_ - 1; ++j) {
            cell_info[i][j] = tmp[j + 1];
        }
        delete[] tmp;
    }

    --columns_;
    Reinit();
#endif
}


void LyXTabular::Reinit()
{   
    int j;

    int i = 0;

    // Jürgen, use iterators.
    for (; i < rows_; ++i) {
	for (j = 0; j < columns_; ++j) {
	    cell_info[i][j].width_of_cell = 0;
	}
    }
  
    for (i = 0; i < columns_; ++i) {
	calculate_width_of_column(i);
    }
    calculate_width_of_tabular();

    set_row_column_number_info();
}


void LyXTabular::set_row_column_number_info()
{
    int c = 0;
    int column = 0;
    numberofcells = -1;
    int row = 0;
    for (; row < rows_; ++row) {
	for (column = 0; column<columns_; ++column) {
	    if (cell_info[row][column].multicolumn
		!= LyXTabular::CELL_PART_OF_MULTICOLUMN)
		++numberofcells;
	    cell_info[row][column].cellno = numberofcells;
	}
    }
    ++numberofcells; // because this is one more than as we start from 0
    row = 0;
    column = 0;

    delete [] rowofcell;
    rowofcell = new int[numberofcells];
    delete [] columnofcell;
    columnofcell = new int[numberofcells];
  
    while (c < numberofcells && row < rows_ && column < columns_) {
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
    for (row = 0; row < rows_; ++row) {
	for (column = 0; column<columns_; ++column) {
	    if (IsPartOfMultiColumn(row,column))
		continue;
	    cell_info[row][column].inset->SetAutoBreakRows(
		!GetPWidth(GetCellNumber(row, column)).empty());
	}
    }
}


int LyXTabular::GetNumberOfCells() const
{
    return numberofcells;
}


int LyXTabular::NumberOfCellsInRow(int cell) const
{
    int row = row_of_cell(cell);
    int result = 0;
    for (int i = 0; i < columns_; ++i) {
	if (cell_info[row][i].multicolumn != LyXTabular::CELL_PART_OF_MULTICOLUMN)
	    ++result;
    }
    return result;
}


int LyXTabular::AppendCellAfterCell(int append_cell, int question_cell)
{
    return (right_column_of_cell(append_cell) ==
	    right_column_of_cell(question_cell));
}


int LyXTabular::DeleteCellIfColumnIsDeleted(int cell, int delete_column_cell)
{
    if (column_of_cell(delete_column_cell) == 0 && 
        right_column_of_cell(delete_column_cell) == columns_ - 1)
        return 0;
    else
        return
            (column_of_cell(cell) >= column_of_cell(delete_column_cell) &&
             column_of_cell(cell) <= right_column_of_cell(delete_column_cell));
}


/* returns 1 if there is a topline, returns 0 if not */ 
bool LyXTabular::TopLine(int cell) const
{
    int row = row_of_cell(cell);
    
    if (IsMultiColumn(cell))
        return cellinfo_of_cell(cell)->top_line;
    return row_info[row].top_line;
}


bool LyXTabular::BottomLine(int cell) const
{
    //no bottom line underneath non-existent cells if you please
    if(cell >= numberofcells)
	return false;

    if (IsMultiColumn(cell))
        return cellinfo_of_cell(cell)->bottom_line;
    return row_info[row_of_cell(cell)].bottom_line;
}


bool LyXTabular::LeftLine(int cell) const
{
    return column_info[column_of_cell(cell)].left_line;
}


bool LyXTabular::RightLine(int cell) const
{
    return column_info[right_column_of_cell(cell)].right_line;
}


bool LyXTabular::TopAlreadyDrawed(int cell) const
{
    if (GetAdditionalHeight(cell))
	return false;
    int row = row_of_cell(cell);
    if (row > 0) {
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
	return column_info[column].right_line;
    }
    return false;
}


bool LyXTabular::IsLastRow(int cell) const
{
    return (row_of_cell(cell) == rows_ - 1);
}


int LyXTabular::GetAdditionalHeight(int cell) const
{
    int row = row_of_cell(cell);
    if (!row) return 0;
	
    int top = 1; // bool top = true; ??
    int bottom = 1; // bool bottom = true; ??
    int column;

    for (column = 0; column < columns_ - 1 && bottom; ++column) {
	switch (cell_info[row - 1][column].multicolumn) {
	case LyXTabular::CELL_BEGIN_OF_MULTICOLUMN:
	    bottom = cell_info[row - 1][column].bottom_line;
	    break;
	case LyXTabular::CELL_NORMAL:
	    bottom = row_info[row - 1].bottom_line;
	}
    }
    for (column = 0; column < columns_ - 1 && top; ++column) {
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
    int col = right_column_of_cell(cell);
    if (col < columns_ - 1 && column_info[col].right_line &&
	column_info[col+1].left_line)
	return WIDTH_OF_LINE;
    else
	return 0;
}


// returns the maximum over all rows 
int LyXTabular::GetWidthOfColumn(int cell) const
{
    int column1 = column_of_cell(cell);
    int column2 = right_column_of_cell(cell);
    int result = 0;
    int i = column1;
    for (; i <= column2; ++i) {
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
    
    int row = row_of_cell(cell);
    int column1 = column_of_cell(cell);
    int column2 = right_column_of_cell(cell);

    // first set columns to 0 so we can calculate the right width
    int i = column1;
    for (; i <= column2; ++i) {
        cell_info[row][i].width_of_cell = 0;
    }
    // set the width to MAX_WIDTH until width > 0
    int width = (new_width + 2 * WIDTH_OF_LINE);
    for (i = column1; (i < column2) && (width > 0); ++i) {
        cell_info[row][i].width_of_cell = column_info[i].width_of_column;
        width -= column_info[i].width_of_column;
    }
    if (i == column2) {
        cell_info[row][i].width_of_cell = width;
    }
    return true;
}


void LyXTabular::recalculateMulticolCells(int cell, int new_width)
{
    int row = row_of_cell(cell);
    int column1 = column_of_cell(cell);
    int column2 = right_column_of_cell(cell);

    // first set columns to 0 so we can calculate the right width
    int i = column1;
    for (; i <= column2; ++i)
        cell_info[row][i].width_of_cell = 0;
    for(i = cell + 1; (i < numberofcells) && (!IsMultiColumn(i)); ++i)
        ;
    if (i < numberofcells)
        recalculateMulticolCells(i, GetWidthOfCell(i) - (2 * WIDTH_OF_LINE));
    SetWidthOfMulticolCell(cell, new_width);
}


/* returns 1 if a complete update is necessary, otherwise 0 */ 
bool LyXTabular::SetWidthOfCell(int cell, int new_width)
{
    int row = row_of_cell(cell);
    int column1 = column_of_cell(cell);
    int tmp = 0;
    int width = 0;

    if (IsMultiColumn(cell)) {
        tmp = SetWidthOfMulticolCell(cell, new_width);
    } else {
	width = (new_width + 2*WIDTH_OF_LINE);
        cell_info[row][column1].width_of_cell = width;
        if (column_info[column1].right_line && (column1 < columns_-1) &&
            column_info[column1+1].left_line) // additional width
            cell_info[row][column1].width_of_cell += WIDTH_OF_LINE;
        tmp = calculate_width_of_column_NMC(column1);
    }
    if (tmp) {
        int i;
        for(i = 0; i<columns_;++i)
            calculate_width_of_column_NMC(i);
        for(i = 0; (i<numberofcells) && !IsMultiColumn(i); ++i)
            ;
        if (i<numberofcells)
            recalculateMulticolCells(i, GetWidthOfCell(i)-(2*WIDTH_OF_LINE));
        for(i = 0; i<columns_;++i)
            calculate_width_of_column(i);
        calculate_width_of_tabular();
        return true;
    }
    return false;
}


bool LyXTabular::SetAlignment(int cell, char align)
{
    if (!IsMultiColumn(cell))
        column_info[column_of_cell(cell)].alignment = align;
    cellinfo_of_cell(cell)->alignment = align;
    return true;
}


bool LyXTabular::SetPWidth(int cell, string const & width)
{
    bool flag = !width.empty();

    if (IsMultiColumn(cell)) {
        cellinfo_of_cell(cell)->p_width = width;
	GetCellInset(cell)->SetAutoBreakRows(flag);
    } else {
	int j = column_of_cell(cell);
	int c;
        column_info[j].p_width = width;
	if (flag) // do this only if there is a width
		SetAlignment(cell, LYX_ALIGN_LEFT);
	for(int i=0; i < rows_; ++i) {
	    c = GetCellNumber(i, j);
	    flag = !GetPWidth(c).empty(); // because of multicolumns!
	    GetCellInset(c)->SetAutoBreakRows(flag);
	}
    }
    return true;
}


bool LyXTabular::SetAlignSpecial(int cell, string const & special, int what)
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


bool LyXTabular::SetTopLine(int cell, bool line)
{
    int row = row_of_cell(cell);

    if (!IsMultiColumn(cell))
        row_info[row].top_line = line;
    else
        cellinfo_of_cell(cell)->top_line = line;
    return true;
}


bool LyXTabular::SetBottomLine(int cell, bool line)
{
    if (!IsMultiColumn(cell))
        row_info[row_of_cell(cell)].bottom_line = line;
    else
        cellinfo_of_cell(cell)->bottom_line = line;
    return true;
}


bool LyXTabular::SetLeftLine(int cell, bool line)
{
    column_info[column_of_cell(cell)].left_line = line;
    return true;
}


bool LyXTabular::SetRightLine(int cell, bool line)
{
    column_info[right_column_of_cell(cell)].right_line = line;
    return true;
}


char LyXTabular::GetAlignment(int cell) const
{
    if (IsMultiColumn(cell))
	return cellinfo_of_cell(cell)->alignment;
    else
	return column_info[column_of_cell(cell)].alignment;
}


string LyXTabular::GetPWidth(int cell) const
{
    if (IsMultiColumn(cell))
	return cellinfo_of_cell(cell)->p_width;
    return column_info[column_of_cell(cell)].p_width;
}


string LyXTabular::GetAlignSpecial(int cell, int what) const
{
    if (what == SET_SPECIAL_MULTI)
        return cellinfo_of_cell(cell)->align_special;
    return column_info[column_of_cell(cell)].align_special;
}


int LyXTabular::GetWidthOfCell(int cell) const
{
    int row = row_of_cell(cell);
    int column1 = column_of_cell(cell);
    int column2 = right_column_of_cell(cell);
    int result = 0;
    int i = column1;
    for (; i <= column2; ++i) {
	result += cell_info[row][i].width_of_cell;
    }
    
//    result += GetAdditionalWidth(cell);
    
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
    return (column_of_cell(cell) == 0);
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
    int old_column_width = column_info[column].width_of_column;
    int maximum = 0;
    
    for (int i = 0; i < rows_; ++i) {
	maximum = max(cell_info[i][column].width_of_cell, maximum);
    }
    column_info[column].width_of_column = maximum;
    return (column_info[column].width_of_column != old_column_width);
}


bool LyXTabular::calculate_width_of_column_NMC(int column)
{
    int old_column_width = column_info[column].width_of_column;
    int max = 0;
    for (int i = 0; i < rows_; ++i) {
        if (!IsMultiColumn(GetCellNumber(i, column)) &&
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
        return rows_-1;
    else if (cell < 0)
        return 0;
    return rowofcell[cell];
}


int LyXTabular::column_of_cell(int cell) const
{
    if (cell >= numberofcells)
        return columns_-1;
    else if (cell < 0)
        return 0;
    return columnofcell[cell];
}


int LyXTabular::right_column_of_cell(int cell) const
{
    int row = row_of_cell(cell);
    int column = column_of_cell(cell);
    while (column < (columns_ - 1) &&
	   cell_info[row][column+1].multicolumn == LyXTabular::CELL_PART_OF_MULTICOLUMN)
	++column;
    return column;
}


void LyXTabular::Write(ostream & os) const
{
    int i, j;

    // header line
    os << "<LyXTabular version=1 rows=" << rows_ << " columns=" << columns_ <<
	">" << endl;
    // global longtable options
    os << "<Features rotate=" << rotate <<
	" islongtable=" << is_long_tabular <<
	" endhead=" << endhead << " endfirsthead=" << endfirsthead <<
	" endfoot=" << endfoot << " endlastfoot=" << endlastfoot <<
	">" << endl << endl;
    for (i = 0; i < rows_; ++i) {
	os << "<Row topline=" << row_info[i].top_line <<
	    " bottomline=" << row_info[i].bottom_line <<
	    " newpage=" << row_info[i].newpage <<
	    ">" << endl;
	for (j = 0; j < columns_; ++j) {
	    if (!i) {
		os << "<Column alignment=" << column_info[j].alignment <<
		    " leftline=" << column_info[j].left_line <<
		    " rightline=" << column_info[j].right_line <<
		    " width=\"" << VSpace(column_info[j].p_width).asLyXCommand() <<
		    "\" special=\"" << column_info[j].align_special <<
		    "\">" << endl;
	    } else {
		os << "<Column>" << endl;
	    }
	    os << "<Cell multicolumn=" << cell_info[i][j].multicolumn <<
		" alignment=" << cell_info[i][j].alignment <<
		" topline=" << cell_info[i][j].top_line <<
		" bottomline=" << cell_info[i][j].bottom_line <<
		" rotate=" << cell_info[i][j].rotate <<
		" linebreaks=" << cell_info[i][j].linebreaks <<
		" width=\"" << cell_info[i][j].p_width <<
	        "\" special=\"" << cell_info[i][j].align_special <<
		"\">" << endl;
	    os << "\\begin_inset ";
	    cell_info[i][j].inset->Write(os);
	    os << "\n\\end_inset " << endl;
	    os << "</Cell>" << endl;
	    os << "</Column>" << endl;
	}
	os << "</Row>" << endl;
    }
    os << "</LyXTabular>" << endl;
}


static
bool getTokenValue(string const str, const char * token, string & ret)
{
    int pos = str.find(token);
    char ch = str[pos+strlen(token)];

    if ((pos < 0) || (ch != '='))
	return false;
    ret.erase();
    pos += strlen(token)+1;
    ch = str[pos];
    if ((ch != '"') && (ch != '\'')) { // only read till next space
	ret += ch;
	ch = ' ';
    }
    while((pos < int(str.length()-1)) && (str[++pos] != ch))
	ret += str[pos];

    return true;
}


static
bool getTokenValue(string const str, const char * token, int & num)
{
    string ret;
    int pos = str.find(token);
    char ch = str[pos+strlen(token)];

    if ((pos < 0) || (ch != '='))
	return false;
    ret.erase();
    pos += strlen(token)+1;
    ch = str[pos];
    if ((ch != '"') && (ch != '\'')) { // only read till next space
	if (!isdigit(ch))
	    return false;
	ret += ch;
    }
    ++pos;
    while((pos < int(str.length()-1)) && isdigit(str[pos]))
	ret += str[pos++];

    num = strToInt(ret);
    return true;
}


static
bool getTokenValue(string const str, const char * token, bool & flag)
{
    string ret;
    int pos = str.find(token);
    char ch = str[pos+strlen(token)];

    if ((pos < 0) || (ch != '='))
	return false;
    ret.erase();
    pos += strlen(token)+1;
    ch = str[pos];
    if ((ch != '"') && (ch != '\'')) { // only read till next space
	if (!isdigit(ch))
	    return false;
	ret += ch;
    }
    ++pos;
    while((pos < int(str.length()-1)) && isdigit(str[pos]))
	ret += str[pos++];

    flag = strToInt(ret);
    return true;
}


void l_getline(istream & is, string & str)
{
    getline(is, str);
    while(str.empty())
	getline(is, str);
}


void LyXTabular::Read(LyXLex & lex)
{
    string line;
    istream & is = lex.getStream();

    l_getline(is, line);
    if (!prefixIs(line, "<LyXTabular ")) {
	OldFormatRead(lex, line);
	return;
    }

    int version;
    int rows_arg;
    int columns_arg;
    if (!getTokenValue(line, "version", version))
	return;
    if (!getTokenValue(line, "rows", rows_arg))
	return;
    if (!getTokenValue(line, "columns", columns_arg))
	return;
    Init(rows_arg, columns_arg);
    l_getline(is, line);
    if (!prefixIs(line, "<Features ")) {
	lyxerr << "Wrong tabular format (expected <Feture ...> got" <<
	    line << ")" << endl;
	return;
    }
    (void)getTokenValue(line, "islongtable", is_long_tabular);
    (void)getTokenValue(line, "endhead", endhead);
    (void)getTokenValue(line, "endfirsthead", endfirsthead);
    (void)getTokenValue(line, "endfoot", endfoot);
    (void)getTokenValue(line, "endlastfoot", endlastfoot);
    int i, j;
    for(i = 0; i < rows_; ++i) {
	l_getline(is, line);
	if (!prefixIs(line, "<Row ")) {
	    lyxerr << "Wrong tabular format (expected <Row ...> got" <<
		line << ")" << endl;
	    return;
	}
	(void)getTokenValue(line, "topline", row_info[i].top_line);
	(void)getTokenValue(line, "bottomline", row_info[i].bottom_line);
	(void)getTokenValue(line, "newpage", row_info[i].newpage);
	for (j = 0; j < columns_; ++j) {
	    l_getline(is,line);
	    if (!prefixIs(line,"<Column")) {
		lyxerr << "Wrong tabular format (expected <Column ...> got" <<
		    line << ")" << endl;
		return;
	    }
	    if (!i) {
		(void)getTokenValue(line, "alignment", column_info[j].alignment);
		(void)getTokenValue(line, "leftline", column_info[j].left_line);
		(void)getTokenValue(line, "rightline", column_info[j].right_line);
		(void)getTokenValue(line, "width", column_info[j].p_width);
		(void)getTokenValue(line, "special", column_info[j].align_special);
	    }
	    l_getline(is, line);
	    if (!prefixIs(line, "<Cell")) {
		lyxerr << "Wrong tabular format (expected <Cell ...> got" <<
		    line << ")" << endl;
		return;
	    }
	    (void)getTokenValue(line, "multicolumn", cell_info[i][j].multicolumn);
	    (void)getTokenValue(line, "alignment", cell_info[i][j].alignment);
	    (void)getTokenValue(line, "topline", cell_info[i][j].top_line);
	    (void)getTokenValue(line, "bottomline", cell_info[i][j].bottom_line);
	    (void)getTokenValue(line, "rotate", cell_info[i][j].rotate);
	    (void)getTokenValue(line, "linebreaks", cell_info[i][j].linebreaks);
	    (void)getTokenValue(line, "width", cell_info[i][j].p_width);
	    (void)getTokenValue(line, "special", cell_info[i][j].align_special);
	    l_getline(is, line);
	    if (prefixIs(line, "\\begin_inset")) {
		cell_info[i][j].inset->Read(lex);
		l_getline(is, line);
	    }
	    if (line != "</Cell>") {
		lyxerr << "Wrong tabular format (expected </Cell> got" <<
		    line << ")" << endl;
		return;
	    }
	    l_getline(is, line);
	    if (line != "</Column>") {
		lyxerr << "Wrong tabular format (expected </Column> got" <<
		    line << ")" << endl;
		return;
	    }
	}
	l_getline(is, line);
	if (line != "</Row>") {
	    lyxerr << "Wrong tabular format (expected </Row> got" <<
		line << ")" << endl;
	    return;
	}
    }
    while (line != "</LyXTabular>") {
	l_getline(is, line);
    }
    set_row_column_number_info();
}


void LyXTabular::OldFormatRead(LyXLex & lex, string const & fl)
{
    int version;
    int i, j;
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
    int h = 0;
	
    istream & is = lex.getStream();
    string s(fl);
    if (s.length() > 8)
	version = atoi(s.c_str() + 8);
    else
	version = 1;

    vector<int> cont_row_info;

    if (version < 5) {
	lyxerr << "Tabular format < 5 is not supported anymore\n"
	    "Get an older version of LyX (< 1.1.x) for conversion!"
	       << endl;
	WriteAlert(_("Warning:"),
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
	endhead = a;
	endfirsthead = b;
	endfoot = c;
	endlastfoot = d;
	for (i = 0; i < rows_; ++i) {
	    a = b = c = d = e = f = g = h = 0;
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
	    char ch; // skip '"'
	    is >> ch;
	    getline(is, s1, '"');
	    is >> ch; // skip '"'
	    getline(is, s2, '"');
	    column_info[i].alignment = static_cast<char>(a);
	    column_info[i].left_line = b;
	    column_info[i].right_line = c;
	    column_info[i].p_width = s1;
	    column_info[i].align_special = s2;
	}
	for (i = 0; i < rows_; ++i) {
	    for (j = 0; j < columns_; ++j) {
		string s1;
		string s2;
		is >> a >> b >> c >> d >> e >> f >> g;
		char ch;
		is >> ch; // skip '"'
		getline(is, s1, '"');
		is >> ch; // skip '"'
		getline(is, s2, '"');
		cell_info[i][j].multicolumn = static_cast<char>(a);
		cell_info[i][j].alignment = static_cast<char>(b);
		cell_info[i][j].top_line = static_cast<char>(c);
		cell_info[i][j].bottom_line = static_cast<char>(d);
		cell_info[i][j].rotate = static_cast<bool>(f);
		cell_info[i][j].linebreaks = static_cast<bool>(g);
		cell_info[i][j].align_special = s1;
		cell_info[i][j].p_width = s2;
	    }
	}
    }
    set_row_column_number_info();

    LyXParagraph * par = new LyXParagraph;
    LyXParagraph * return_par = 0;
    LyXParagraph::footnote_flag footnoteflag = LyXParagraph::NO_FOOTNOTE;
    LyXParagraph::footnote_kind footnotekind = LyXParagraph::FOOTNOTE;
    string token, tmptok;
    int pos = 0;
    char depth = 0;
    LyXFont font(LyXFont::ALL_SANE);

    while (lex.IsOK()) {
        lex.nextToken();
        token = lex.GetString();
        if (token.empty())
            continue;
	if ((token == "\\layout") || (token == "\\end_float") ||
	    (token == "\\end_deeper"))
	{
	    lex.pushToken(token);
	    break;
	}
	if (owner_->BufferOwner()->parseSingleLyXformat2Token(lex, par,
							      return_par,
							      token, pos,
							      depth, font,
							      footnoteflag,
							      footnotekind))
	{
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
    InsetText *inset = GetCellInset(cell);
    int row;

    for(int i=0; i < par->Last(); ++i) {
	if (par->IsNewline(i)) {
	    ++cell;
	    if (cell > GetNumberOfCells()) {
		lyxerr << "Some error in reading old table format occured!" <<
		    endl << "Terminating when reading cell[" << cell << "]!" <<
		    endl;
		return;
	    }
	    row = row_of_cell(cell);
	    if (cont_row_info[row]) {
		DeleteRow(row);
		cont_row_info.erase(cont_row_info.begin() + row); //&cont_row_info[row]);
		while(!IsFirstCellInRow(--cell));
	    } else {
		inset = GetCellInset(cell);
		continue;
	    }
	    inset = GetCellInset(cell);
	    row = row_of_cell(cell);
	    if (!cell_info[row_of_cell(cell)][column_of_cell(cell)].linebreaks)
	    {
		// insert a space instead
		par->Erase(i);
		par->InsertChar(i, ' ');
	    }
	}
	par->CopyIntoMinibuffer(i);
	inset->par->InsertFromMinibuffer(inset->par->Last());
    }
    Reinit();
}


char const * LyXTabular::GetDocBookAlign(int cell, bool isColumn) const
{
	int i = isColumn ? cell : column_of_cell(cell);
	
	//if (isColumn)
	//i = cell;
	//else
	//i = column_of_cell(cell);
    if (!isColumn && IsMultiColumn(cell)) {
       if (!cellinfo_of_cell(cell)->align_special.empty()) {
           return cellinfo_of_cell(cell)->align_special.c_str();
       } else {
           switch (GetAlignment(cell)) {
           case LYX_ALIGN_LEFT:
               return "left";
           case LYX_ALIGN_RIGHT:
               return "right";
           default:
               return "center";
           }
       }
    } else {
       if (!column_info[i].align_special.empty()) {
           return column_info[i].align_special.c_str();
       }
#ifdef IGNORE_THIS_FOR_NOW
       else if (!column_info[i].p_width.empty()) {
           file += "p{";
           file += column_info[i].p_width;
           file += '}';
       }
#endif
       else {
           switch (column_info[i].alignment) {
           case LYX_ALIGN_LEFT:
               return "left";
           case LYX_ALIGN_RIGHT:
               return "right";
           default:
               return "center";
           }
       }
    }
}


// cell <0 will tex the preamble
// returns the number of printed newlines
int LyXTabular::DocBookEndOfCell(ostream & os, int cell, int & depth) const
{
    int i;
    int ret = 0;
    //int tmp; // tmp2; // unused
    int nvcell; // fcell; // unused
    if (IsLastCell(cell)) {
	    os << newlineAndDepth(--depth)
	       << "</ENTRY>"
	       << newlineAndDepth(--depth)
	       << "</ROW>"
	       << newlineAndDepth(--depth)
	       << "</TBODY>"
	       << newlineAndDepth(--depth);
        if (is_long_tabular)
		os << "</TGROUP>";
        else
		os << "</TGROUP>"
		   << newlineAndDepth(--depth);
        ret += 4;
    } else {
        nvcell = cell + 1;
        if (cell < 0) {
            // preamble
            if (is_long_tabular)
		    os << "<TGROUP ";
            else
		    os << "<TGROUP ";
            os << "COLS='"
	       << columns_
	       << "' COLSEP='1' ROWSEP='1'>"
	       << newlineAndDepth(++depth);
            ++ret;
            for (i = 0; i < columns_; ++i) {
		    os << "<COLSPEC ALIGN='"
		       << GetDocBookAlign(i, true)
		       << "' COLNAME='col"
		       << i + 1
		       << "' COLNUM='"
		       << i + 1
		       << "' COLSEP='";
               if (i == (columns_-1)) {
		       os << '1';
               } else {
                   if (column_info[i].right_line ||
                       column_info[i+1].left_line)
			   os << '1';
                   else
			   os << '0';
               }
               os << "'>"
		  << newlineAndDepth(depth);
                ++ret;
#ifdef NOT_HANDLED_YET_AS_I_DONT_KNOW_HOW
                if (column_info[i].left_line)
			os << '|';
#endif
            }
            os << "<TBODY>"
	       << newlineAndDepth(++depth)
	       << "<ROW>"
	       << newlineAndDepth(++depth)
	       << "<ENTRY ALIGN='"
	       << GetDocBookAlign(0)
	       << "'";
           if (IsMultiColumn(0)) {
		   os << " NAMEST='col1' NAMEEND='col"
		      << cells_in_multicolumn(0)
		      << "'";
           }
	   os << ">"
	      << newlineAndDepth(++depth);
            ret += 3;
        } else {
            if (IsLastCellInRow(cell)) {
		    os << newlineAndDepth(--depth)
		       << "</ENTRY>"
		       << newlineAndDepth(--depth)
		       << "</ROW>"
		       << newlineAndDepth(depth)
		       << "<ROW>"
		       << newlineAndDepth(++depth)
		       << "<ENTRY ALIGN='"
		       << GetDocBookAlign(cell + 1)
		       << "' VALIGN='middle'";
               if (IsMultiColumn(cell + 1)) {
		       os << " NAMEST='col"
			  << column_of_cell(cell+1) + 1
			  << "' NAMEEND='col"
			  << column_of_cell(cell + 1) +
			       cells_in_multicolumn(cell + 1)
			  << "'";
               }
               os << ">"
		  << newlineAndDepth(++depth);
                ret += 4;
            } else {
		    os << newlineAndDepth(--depth)
		       << "</ENTRY>"
		       << newlineAndDepth(depth)
		       << "<ENTRY ALIGN='"
		       << GetDocBookAlign(cell + 1)
		       << "' VALIGN='middle'";
               if (IsMultiColumn(cell + 1)) {
		       os << " NAMEST='col"
			  << column_of_cell(cell+1) + 1
			  << "' NAMEEND='col"
			  << column_of_cell(cell+1) +
			       cells_in_multicolumn(cell+1)
			  << "'";
               }
               os << ">"
		  << newlineAndDepth(++depth);
                ret += 3;
            }
        }
    }
    return ret;
}


bool LyXTabular::IsMultiColumn(int cell) const
{
    return (cellinfo_of_cell(cell)->multicolumn != LyXTabular::CELL_NORMAL);
}


LyXTabular::cellstruct * LyXTabular::cellinfo_of_cell(int cell) const
{
    int row = row_of_cell(cell);
    int column = column_of_cell(cell);
    return  &cell_info[row][column];
}
   

void LyXTabular::SetMultiColumn(int cell, int number)
{
    int new_width = cellinfo_of_cell(cell)->width_of_cell;
    
    cellinfo_of_cell(cell)->multicolumn = LyXTabular::CELL_BEGIN_OF_MULTICOLUMN;
    cellinfo_of_cell(cell)->alignment = column_info[column_of_cell(cell)].alignment;
    cellinfo_of_cell(cell)->top_line = row_info[row_of_cell(cell)].top_line;
    cellinfo_of_cell(cell)->bottom_line = row_info[row_of_cell(cell)].bottom_line;
    for (number--; number > 0; --number) {
        cellinfo_of_cell(cell+number)->multicolumn = 
            LyXTabular::CELL_PART_OF_MULTICOLUMN;
        new_width += cellinfo_of_cell(cell+number)->width_of_cell;
    }
    set_row_column_number_info();
    SetWidthOfCell(cell, new_width);
}


int LyXTabular::cells_in_multicolumn(int cell) const
{
    int row = row_of_cell(cell);
    int column = column_of_cell(cell);
    int result = 1;
    ++column;
    while (column < columns_ && cell_info[row][column].multicolumn
           == LyXTabular::CELL_PART_OF_MULTICOLUMN){
        ++result;
        ++column;
    }
    return result;
}


int LyXTabular::UnsetMultiColumn(int cell)
{
    int row = row_of_cell(cell);
    int column = column_of_cell(cell);
    
    int result = 0;
    
    if (cell_info[row][column].multicolumn
        == LyXTabular::CELL_BEGIN_OF_MULTICOLUMN){
        cell_info[row][column].multicolumn = LyXTabular::CELL_NORMAL;
        ++column;
        while (column < columns_ &&
               cell_info[row][column].multicolumn
               == LyXTabular::CELL_PART_OF_MULTICOLUMN){
            cell_info[row][column].multicolumn = 
                LyXTabular::CELL_NORMAL;
            ++column;
            ++result;
        }
    }
    set_row_column_number_info();
    return result;
}


void LyXTabular::SetLongTabular(int what)
{
    is_long_tabular = what;
}


bool LyXTabular::IsLongTabular() const
{
    return is_long_tabular;
}


void LyXTabular::SetRotateTabular(int what)
{
    rotate = what;
}


bool LyXTabular::GetRotateTabular() const
{
    return rotate;
}


void LyXTabular::SetRotateCell(int cell, int what)
{
    cellinfo_of_cell(cell)->rotate = what;
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
    if (cell < GetNumberOfCells())
        return false;
    return true;
}


int LyXTabular::GetCellAbove(int cell) const
{
    if (row_of_cell(cell) > 0)
        return cell_info[row_of_cell(cell)-1][column_of_cell(cell)].cellno;
    return cell;
}


int LyXTabular::GetCellNumber(int row, int column) const
{
    if (column >= columns_)
        column = columns_ - 1;
    else if (column < 0)
        column = 0;
    if (row >= rows_)
        row = rows_ - 1;
    else if (row < 0)
        row = 0;
    
    return cell_info[row][column].cellno;
}


void LyXTabular::SetLinebreaks(int cell, bool what)
{
    cellinfo_of_cell(cell)->linebreaks = what;
}


bool LyXTabular::GetLinebreaks(int cell) const
{
    if (column_info[column_of_cell(cell)].p_width.empty() &&
        !(IsMultiColumn(cell) && !cellinfo_of_cell(cell)->p_width.empty()))
        return false;
    return cellinfo_of_cell(cell)->linebreaks;
}


void LyXTabular::SetLTHead(int cell, bool first)
{
    int row = row_of_cell(cell);

    if (first) {
        if (row == endfirsthead)
            endfirsthead = -1;
        else
            endfirsthead = row;
    } else {
        if (row == endhead)
            endhead = -1;
        else
            endhead = row;
    }
}


bool LyXTabular::GetRowOfLTHead(int cell) const
{
    if ((endhead+1) > rows_)
        return false;
    return (row_of_cell(cell) == endhead);
}


bool LyXTabular::GetRowOfLTFirstHead(int cell) const
{
    if ((endfirsthead+1) > rows_)
        return false;
    return (row_of_cell(cell) == endfirsthead);
}


void LyXTabular::SetLTFoot(int cell, bool last)
{
    int row = row_of_cell(cell);

    if (last) {
        if (row == endlastfoot)
            endlastfoot = -1;
        else
            endlastfoot = row;
    } else {
        if (row == endfoot)
            endfoot = -1;
        else
            endfoot = row;
    }
}


bool LyXTabular::GetRowOfLTFoot(int cell) const
{
    if ((endfoot+1) > rows_)
        return false;
    return (row_of_cell(cell) == endfoot);
}

bool LyXTabular::GetRowOfLTLastFoot(int cell) const
{
    if ((endlastfoot+1) > rows_)
        return false;
    return (row_of_cell(cell) == endlastfoot);
}


void LyXTabular::SetLTNewPage(int cell, bool what)
{
    row_info[row_of_cell(cell)].newpage = what;
}


bool LyXTabular::GetLTNewPage(int cell) const
{
    return row_info[row_of_cell(cell)].newpage;
}


void LyXTabular::SetAscentOfRow(int row, int height)
{
    if (row >= rows_)
        return;
    row_info[row].ascent_of_row = height;
}


void LyXTabular::SetDescentOfRow(int row, int height)
{
    if (row >= rows_)
        return;
    row_info[row].descent_of_row = height;
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

    for(int row = 0; row < rows_; ++row)
        height += GetAscentOfRow(row) + GetDescentOfRow(row) +
	    GetAdditionalHeight(GetCellNumber(row, 0));
    return height;
}


bool LyXTabular::IsPartOfMultiColumn(int row, int column) const
{
    if ((row >= rows_) || (column >= columns_))
        return false;
    return (cell_info[row][column].multicolumn==CELL_PART_OF_MULTICOLUMN);
}


int LyXTabular::TeXTopHLine(ostream & os, int row) const
{
    int fcell = GetFirstCellInRow(row);
    int n = NumberOfCellsInRow(fcell) + fcell;
    int tmp=0;
    int i;

    for (i = fcell; i < n; ++i) {
	if (TopLine(i))
	    ++tmp;
    }
    if (tmp == (n - fcell)){
	os << "\\hline ";
    } else {
	for (i = fcell; i < n; ++i) {
	    if (TopLine(i)) {
		os << "\\cline{"
		   << column_of_cell(i) + 1
		   << '-'
		   << right_column_of_cell(i) + 1
		   << "} ";
	    }
	}
    }
    if (tmp) {
	os << endl;
	return 1;
    }
    return 0;
}


int LyXTabular::TeXBottomHLine(ostream & os, int row) const
{
    int fcell = GetFirstCellInRow(row);
    int n = NumberOfCellsInRow(fcell) + fcell;
    int tmp = 0;
    int i;

    for (i = fcell; i < n; ++i) {
	if (BottomLine(i))
	    ++tmp;
    }
    if (tmp == (n-fcell)){
	os << "\\hline";
    } else {
	for (i = fcell; i < n; ++i) {
	    if (BottomLine(i)) {
		os << "\\cline{"
		   << column_of_cell(i) + 1
		   << '-'
		   << right_column_of_cell(i) + 1
		   << "} ";
	    }
	}
    }
    if (tmp) {
	os << endl;
	return 1;
    }
    return 0;
}


int LyXTabular::TeXCellPreamble(ostream & os, int cell) const
{
    int ret = 0;

    if (GetRotateCell(cell)) {
	os << "\\begin{sideways}" << endl;
	++ret;
    }
    if (IsMultiColumn(cell)) {
	os << "\\multicolumn{" << cells_in_multicolumn(cell) << "}{";
	if (!cellinfo_of_cell(cell+1)->align_special.empty()) {
	    os << cellinfo_of_cell(cell+1)->align_special << "}{";
	} else {
	    if (LeftLine(cell))
		os << '|';
	    if (!GetPWidth(cell).empty()) {
		os << "p{" << GetPWidth(cell) << '}';
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
    if (GetLinebreaks(cell)) {
	os << "\\parbox[t]{" << GetPWidth(cell) << "}{\\smallskip{}";
    }
    return ret;
}


int LyXTabular::TeXCellPostamble(ostream & os, int cell) const
{
    int ret = 0;

    // usual cells
    if (GetLinebreaks(cell))
	os << "\\smallskip{}}";
    if (IsMultiColumn(cell)){
	os << '}';
    }
    if (GetRotateCell(cell)) {
	os << endl << "\\end{sideways}";
	++ret;
    }
    return ret;
}


int LyXTabular::Latex(ostream & os, bool fragile, bool fp) const
{
    int ret = 0;
    int i,j;
    int cell = 0;

    //+---------------------------------------------------------------------
    //+                      first the opening preamble                    +
    //+---------------------------------------------------------------------

    if (rotate) {
	os << "\\begin{sideways}" << endl;
	++ret;
    }
    if (is_long_tabular)
	os << "\\begin{longtable}{";
    else
	os << "\\begin{tabular}{";
    for (i = 0; i < columns_; ++i) {
	if (column_info[i].left_line)
	    os << '|';
	if (!column_info[i].align_special.empty()) {
	    os << column_info[i].align_special;
	} else if (!column_info[i].p_width.empty()) {
	    os << "p{"
	       << column_info[i].p_width
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
    os << "}" << endl;
    ++ret;

    //+---------------------------------------------------------------------
    //+                      the single row and columns (cells)            +
    //+---------------------------------------------------------------------

    for(i=0; i < rows_; ++i) {
	ret += TeXTopHLine(os, i);
	for(j=0; j < columns_; ++j) {
	    if (IsPartOfMultiColumn(i,j))
		continue;
	    ret += TeXCellPreamble(os, cell);
	    ret += GetCellInset(cell)->Latex(os, fragile, fp);
	    ret += TeXCellPostamble(os, cell);
	    if (!IsLastCellInRow(cell)) { // not last cell in row
		os << "&" << endl;
		++ret;
	    }
	    ++cell;
	}
	os << "\\\\" << endl;
	ret += TeXBottomHLine(os, i);
	if (IsLongTabular()) {
	    if (i == endhead) {
		os << "\\endhead\n";
		++ret;
	    }
	    if (i == endfirsthead) {
		os << "\\endfirsthead\n";
		++ret;
	    }
	    if (i == endfoot) {
		os << "\\endfoot\n";
		++ret;
	    }
	    if (i == endlastfoot) {
		os << "\\endlastfoot\n";
		++ret;
	    }
	    if (row_info[i].newpage) {
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


InsetText * LyXTabular::GetCellInset(int cell) const
{
    return cell_info[row_of_cell(cell)][column_of_cell(cell)].inset;
}

void LyXTabular::Validate(LaTeXFeatures & features) const
{
    if (IsLongTabular())
        features.longtable = true;
    if (NeedRotating())
	features.rotating = true;
    for(int cell = 0; cell < numberofcells; ++cell)
	GetCellInset(cell)->Validate(features);
}
