/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	  Copyright (C) 1995 Matthias Ettrich
 *        Copyright (C) 1995-1998 The LyX Team.
 *
 * ====================================================== 
 */

#include <config.h>

#include <cstdlib>
#include "table.h"
#include "vspace.h"
#include "layout.h"
#include "support/lstrings.h"

#ifdef __GNUG__
#pragma implementation
#endif

extern void addNewlineAndDepth(string &file, int const depth); // Jug 990923

#define WIDTH_OF_LINE 5

/* konstruktor */
LyXTable::LyXTable(int rows_arg, int columns_arg)
{
	Init(rows_arg, columns_arg);
}


LyXTable::LyXTable(LyXLex &lex)
{
	FILE *file = lex.getFile();
	Read(file);
}


LyXTable::~LyXTable() {
	int i;
	delete[] rowofcell;
	delete[] columnofcell;
	delete[] column_info;
        delete[] row_info;
	for (i = 0; i<rows; i++) {
		delete[] cell_info[i]; // verify that this shoudn't be freed with delete
	}
	delete[] cell_info;
}


LyXTable * LyXTable::Clone()
{
    LyXTable *result = new LyXTable(rows, columns);
    int row, column;;

    for (row = 0; row<rows; row++){
        for (column = 0; column<columns;column++){
            result->cell_info[row][column] = cell_info[row][column];
        }
    }

    for (row = 0; row<rows; row++){
        result->row_info[row] = row_info[row];
    }

    for (column = 0; column<columns; column++){
        result->column_info[column].left_line = column_info[column].left_line;
        result->column_info[column].right_line = column_info[column].right_line;
        result->column_info[column].alignment = column_info[column].alignment;
        result->column_info[column].p_width = column_info[column].p_width;
        result->column_info[column].align_special = column_info[column].align_special;
    }
  
    result->SetLongTable(is_long_table);
    result->rotate = rotate;
    result->Reinit();
    return result;
}


/* activates all lines and sets all widths to 0 */ 
void LyXTable::Init(int rows_arg, int columns_arg)
{
    int i, j, cellno;
    rows = rows_arg;
    columns = columns_arg;
    column_info = new columnstruct[columns];
    row_info = new rowstruct[rows];
    cell_info = new cellstruct*[rows];

    cellno = 0;
    for (i = 0; i<rows;i++) {
        cell_info[i] = new cellstruct[columns];
        row_info[i].top_line = true;
        row_info[i].bottom_line = false;
        row_info[i].is_cont_row = false;
        row_info[i].newpage = false;
        for (j = 0; j<columns; j++) {
            cell_info[i][j].cellno = cellno++;
            cell_info[i][j].width_of_cell = 0;
            cell_info[i][j].multicolumn = LyXTable::CELL_NORMAL;
            cell_info[i][j].alignment = LYX_ALIGN_CENTER;
            cell_info[i][j].top_line = row_info[i].top_line;
            cell_info[i][j].bottom_line = row_info[i].bottom_line;
            cell_info[i][j].has_cont_row = false;
            cell_info[i][j].rotate = false;
            cell_info[i][j].linebreaks = false;
        }
    }
    row_info[i-1].bottom_line = true;
    row_info[0].bottom_line = true;

    for (i = 0; i<columns;i++) {
        column_info[i].left_line = true;
        column_info[i].right_line = false;
        column_info[i].alignment = LYX_ALIGN_CENTER;
        // set width_of_column to zero before it is used in
        // calculate_width_of_column() (thornley)
        column_info[i].width_of_column = 0;
        calculate_width_of_column(i);
    }
    column_info[i-1].right_line = true;
   
    calculate_width_of_table();

    rowofcell = 0;
    columnofcell = 0;
    set_row_column_number_info();
    is_long_table = false;
    rotate = 0;
    endhead = -1;
    endfirsthead = -1;
    endfoot = -1;
    endlastfoot = -1;
}


void LyXTable::AppendRow(int cell)
{
    int row = row_of_cell(cell);
    rowstruct *row_info2 = new rowstruct[rows+1];
    cellstruct** cell_info2 = new cellstruct*[rows+1];
    int i;

    for (i = 0; i <= row; i++) {
        cell_info2[i] = cell_info[i];
        row_info2[i] = row_info[i];
    }
    for (i = rows-1; i >= row; i--) {
        cell_info2[i+1] = cell_info[i];
        row_info2[i+1] = row_info[i];
    }
    for (i = row;row_info[i].is_cont_row;i--);
    if (((row+1)>= rows) || !row_info[row+1].is_cont_row)
        row_info2[row+1].is_cont_row = false;
    row_info2[row+1].top_line = row_info[i].top_line;
    cell_info2[row+1] = new cellstruct[columns];
    for (i = 0; i<columns; i++) {
        cell_info2[row+1][i].width_of_cell = 0;
        cell_info2[row+1][i] = cell_info2[row][i];
    }
   
    delete[] cell_info;
    cell_info = cell_info2;
    delete[] row_info;
    row_info = row_info2;
   
    rows++;
   
    Reinit();
}


void LyXTable::DeleteRow(int cell)
{
	int row = row_of_cell(cell);
        while(!row_info[row].is_cont_row && RowHasContRow(cell))
            DeleteRow(cell_info[row+1][0].cellno);
        rowstruct *row_info2 = new rowstruct[rows-1];
	cellstruct** cell_info2 = new cellstruct*[rows-1];
	int i;

	delete[] cell_info[row];
	for (i = 0; i < row; i++) {
		cell_info2[i] = cell_info[i];
                row_info2[i] = row_info[i];
	}
        if (row_info[i].is_cont_row)
            row_info2[i-1].bottom_line = row_info[i].bottom_line;
	for (i = row; i < rows - 1; i++) {
		cell_info2[i] = cell_info[i+1];
                row_info2[i] = row_info[i+1];
	}
   

	delete[] cell_info;
	cell_info = cell_info2;
        delete[] row_info;
        row_info = row_info2;
   
	rows--;

	Reinit();
}


void LyXTable::AppendColumn(int cell)
{
    int i, j;
    columnstruct *column_info2 = new columnstruct[columns+1];
    int column = right_column_of_cell(cell);
   
    for (i = 0; i<= column; i++){
        column_info2[i] = column_info[i];
    }
    for (i = columns-1; i>= column; i--){
        column_info2[i+1] = column_info[i];
    }
    
    delete[] column_info;
    column_info = column_info2;
    
    for (i = 0; i<rows;i++){
        cellstruct* tmp = cell_info[i];
        cell_info[i] = new cellstruct[columns+1];
        for (j = 0; j<= column; j++){
            cell_info[i][j] = tmp[j];
        }
        for (j = column; j<columns; j++){
            cell_info[i][j+1] = tmp[j];
        }
        // care about multicolumns
        if (cell_info[i][column+1].multicolumn
            == LyXTable::CELL_BEGIN_OF_MULTICOLUMN){
            cell_info[i][column+1].multicolumn = 
                LyXTable::CELL_PART_OF_MULTICOLUMN;
        }
        if (column + 1 == columns
            || cell_info[i][column+2].multicolumn
            != LyXTable::CELL_PART_OF_MULTICOLUMN){
            cell_info[i][column+1].multicolumn = 
                LyXTable::CELL_NORMAL;
        }
        
        delete[] tmp;
    }
    
    columns++;
    Reinit();
}


void LyXTable::Reinit()
{   
	int i, j;

	for (i = 0; i<rows;i++) {
		for (j = 0; j<columns; j++) {
			cell_info[i][j].width_of_cell = 0;
                        if ((i+1 < rows) && !row_info[i+1].is_cont_row)
                            cell_info[i][j].has_cont_row = false;
		}
	}
  
	for (i = 0; i<columns;i++) {
		calculate_width_of_column(i);
	}
	calculate_width_of_table();

	set_row_column_number_info();
}


void LyXTable::set_row_column_number_info()
{
	int row = 0;
	int c = 0;
	int column = 0;
	numberofcells = -1;
	for (row = 0; row<rows; ++row) {
		for (column = 0; column<columns; ++column) {
			if (cell_info[row][column].multicolumn
			    != LyXTable::CELL_PART_OF_MULTICOLUMN)
				++numberofcells;
                        cell_info[row][column].cellno = numberofcells;
		}
	}
	++numberofcells; // because this is one more than as we start from 0
	row = 0;
	column = 0;

	if (rowofcell)
		delete [] rowofcell;
	rowofcell = new int[numberofcells];
	if (columnofcell)
		delete [] columnofcell;
	columnofcell = new int[numberofcells];
  
	while (c < numberofcells && row < rows && column < columns) {
		rowofcell[c] = row;
		columnofcell[c] = column;
		c++;
		do{
			column++;
		} while (column < columns &&
			 cell_info[row][column].multicolumn
			 == LyXTable::CELL_PART_OF_MULTICOLUMN);
		if (column == columns){
			column = 0;
			row++;
		}
	}
}


void LyXTable::DeleteColumn(int cell)
{
	int column1 = column_of_cell(cell);
	int column2 = right_column_of_cell(cell);
	int column;
   
	if (column1 == 0 && column2 == columns - 1)
		return;
   
	for (column = column1; column <= column2;column++){
		delete_column(column1);
	}
	Reinit();
}


int LyXTable::GetNumberOfCells()
{
	return numberofcells;
}


int LyXTable::NumberOfCellsInRow(int cell)
{
	int row = row_of_cell(cell);
	int i = 0;
	int result = 0;
	for (i = 0; i<columns; i++){
		if (cell_info[row][i].multicolumn != LyXTable::CELL_PART_OF_MULTICOLUMN)
			result++;
	}
	return result;
}


int LyXTable::AppendCellAfterCell(int append_cell, int question_cell)
{
	return (right_column_of_cell(append_cell) == 
		right_column_of_cell(question_cell));
}


int LyXTable::DeleteCellIfColumnIsDeleted(int cell, int delete_column_cell)
{
    if (column_of_cell(delete_column_cell) == 0 && 
        right_column_of_cell(delete_column_cell) == columns - 1)
        return 0;
    else
        return
            (column_of_cell(cell) >= column_of_cell(delete_column_cell) &&
             column_of_cell(cell) <= right_column_of_cell(delete_column_cell));
}


/* returns 1 if there is a topline, returns 0 if not */ 
bool LyXTable::TopLine(int cell)
{
    int row = row_of_cell(cell);
    
    if (IsContRow(cell))
        return TopLine(cell_info[row-1][column_of_cell(cell)].cellno);
    if (IsMultiColumn(cell))
        return cellinfo_of_cell(cell)->top_line;
    return row_info[row].top_line;
}


bool LyXTable::BottomLine(int cell)
{
    //no bottom line underneath non-existent cells if you please
    if(cell >= numberofcells)
	return false;

    int row = row_of_cell(cell);
    
    if (RowHasContRow(cell))
        return BottomLine(cell_info[row+1][column_of_cell(cell)].cellno);
    if (IsMultiColumn(cell))
        return cellinfo_of_cell(cell)->bottom_line;
    return row_info[row_of_cell(cell)].bottom_line;
}


bool LyXTable::LeftLine(int cell)
{
	return column_info[column_of_cell(cell)].left_line;
}

bool LyXTable::RightLine(int cell)
{
	return column_info[right_column_of_cell(cell)].right_line;
}


bool LyXTable::TopAlreadyDrawed(int cell)
{
	if (AdditionalHeight(cell))
		return false;
	int row = row_of_cell(cell);
	if (row > 0){
		int column = column_of_cell(cell);
		while (column
		       && cell_info[row-1][column].multicolumn
		       == LyXTable::CELL_PART_OF_MULTICOLUMN)
			column--;
		if (cell_info[row-1][column].multicolumn
		    == LyXTable::CELL_NORMAL)
			return row_info[row-1].bottom_line;
		else
			return cell_info[row-1][column].bottom_line;
	}
	return false;
}


bool LyXTable::VeryLastRow(int cell)
{
	return (row_of_cell(cell) == rows-1);
}


int LyXTable::AdditionalHeight(int cell)
{
	int row = row_of_cell(cell);
	int top = 1;
	int bottom = 1;
	int column;
	if (row){
		for (column = 0;column < columns-1 && bottom;column++){
			switch (cell_info[row-1][column].multicolumn){
			case LyXTable::CELL_BEGIN_OF_MULTICOLUMN:
				bottom = cell_info[row-1][column].bottom_line;
				break;
			case LyXTable::CELL_NORMAL:
				bottom = row_info[row-1].bottom_line;
			}
		}
		for (column = 0;column < columns-1 && top;column++){
			switch (cell_info[row][column].multicolumn){
			case LyXTable::CELL_BEGIN_OF_MULTICOLUMN:
				top = cell_info[row][column].top_line;
				break;
			case LyXTable::CELL_NORMAL:
				top = row_info[row].top_line;
			}
		}
		if (top && bottom)
			return WIDTH_OF_LINE;
	}
	return 0;
}


int LyXTable::AdditionalWidth(int cell)
{
	// internally already set in SetWidthOfCell
	// used to get it back in text.C
	int col = right_column_of_cell(cell);
	if (col < columns - 1 && column_info[col].right_line &&
            column_info[col+1].left_line)
		return WIDTH_OF_LINE;
	else
		return 0;
}


// returns the maximum over all rows 
int LyXTable::WidthOfColumn(int cell)
{
	int column1 = column_of_cell(cell);
	int column2 = right_column_of_cell(cell);
	int i;
	int result = 0;
	for (i = column1; i<= column2;i++){
		result += column_info[i].width_of_column;
	}
	return result;
}


int LyXTable::WidthOfTable()
{
	return width_of_table;
}

/* returns 1 if a complete update is necessary, otherwise 0 */ 
bool LyXTable::SetWidthOfMulticolCell(int cell, int new_width)
{
    if (!IsMultiColumn(cell))
        return false;
    
    int row = row_of_cell(cell);
    int column1 = column_of_cell(cell);
    int column2 = right_column_of_cell(cell);
    int i;
    int width = 0;

    // first set columns to 0 so we can calculate the right width
    for (i = column1; i<= column2;i++) {
        cell_info[row][i].width_of_cell = 0;
    }
    // set the width to MAX_WIDTH until width > 0
    width = (new_width + 2*WIDTH_OF_LINE);
    for (i = column1; (i<column2) && (width > 0);i++){
        cell_info[row][i].width_of_cell = column_info[i].width_of_column;
        width -= column_info[i].width_of_column;
    }
    if (i == column2) {
        cell_info[row][i].width_of_cell = width;
    }
    return true;
}

void LyXTable::recalculateMulticolCells(int cell, int new_width)
{
    int
        row = row_of_cell(cell),
        column1 = column_of_cell(cell),
        column2 = right_column_of_cell(cell),
        i;

    // first set columns to 0 so we can calculate the right width
    for (i = column1; i<= column2;++i)
        cell_info[row][i].width_of_cell = 0;
    for(i = cell+1;(i<numberofcells) && (!IsMultiColumn(i));++i)
        ;
    if (i < numberofcells)
        recalculateMulticolCells(i, GetWidthOfCell(i)-(2*WIDTH_OF_LINE));
    SetWidthOfMulticolCell(cell, new_width);
}

/* returns 1 if a complete update is necessary, otherwise 0 */ 
bool LyXTable::SetWidthOfCell(int cell, int new_width)
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
        if (column_info[column1].right_line && (column1 < columns-1) &&
            column_info[column1+1].left_line) // additional width
            cell_info[row][column1].width_of_cell += WIDTH_OF_LINE;
        tmp = calculate_width_of_column_NMC(column1);
    }
    if (tmp) {
        int i;
        for(i = 0; i<columns;++i)
            calculate_width_of_column_NMC(i);
        for(i = 0; (i<numberofcells) && !IsMultiColumn(i); ++i)
            ;
        if (i<numberofcells)
            recalculateMulticolCells(i, GetWidthOfCell(i)-(2*WIDTH_OF_LINE));
        for(i = 0; i<columns;++i)
            calculate_width_of_column(i);
        calculate_width_of_table();
        return true;
    }
    return false;
}


bool LyXTable::SetAlignment(int cell, char align)
{
    if (!IsMultiColumn(cell))
        column_info[column_of_cell(cell)].alignment = align;
    cellinfo_of_cell(cell)->alignment = align;
    return true;
}

bool LyXTable::SetPWidth(int cell, string width)
{
    int fvcell = FirstVirtualCell(cell);

    if (IsMultiColumn(fvcell)) {
//        if (column_info[column_of_cell(cell)].p_width.empty())
//            column_info[column_of_cell(cell)].p_width = width;
        cellinfo_of_cell(fvcell)->p_width = width;
    } else {
        column_info[column_of_cell(fvcell)].p_width = width;
	if (!width.empty()) // do this only if there is a width
		SetAlignment(cell, LYX_ALIGN_LEFT);
    }
    return true;
}

bool LyXTable::SetAlignSpecial(int cell, string special, int what)
{
    if (what == SET_SPECIAL_MULTI)
        cellinfo_of_cell(cell)->align_special = special;
    else
        column_info[column_of_cell(cell)].align_special = special;
    return true;
}

bool LyXTable::SetAllLines(int cell, bool line)
{
    SetTopLine(cell, line);
    SetBottomLine(cell, line);
    SetRightLine(cell, line);
    SetLeftLine(cell, line);
    return true;
}

bool LyXTable::SetTopLine(int cell, bool line)
{
    int row = row_of_cell(cell);

    if (IsContRow(cell))
        SetTopLine(cell_info[row-1][column_of_cell(cell)].cellno, line);
    else if (!IsMultiColumn(cell))
        row_info[row].top_line = line;
    else
        cellinfo_of_cell(cell)->top_line = line;
    return true;
}


bool LyXTable::SetBottomLine(int cell, bool line)
{
    int row = row_of_cell(cell);

    if (RowHasContRow(cell))
        SetBottomLine(cell_info[row+1][column_of_cell(cell)].cellno, line);
    else if (!IsMultiColumn(cell))
        row_info[row_of_cell(cell)].bottom_line = line;
    else
        cellinfo_of_cell(cell)->bottom_line = line;
    return true;
}


bool LyXTable::SetLeftLine(int cell, bool line)
{
	column_info[column_of_cell(cell)].left_line = line;
	return true;
}


bool LyXTable::SetRightLine(int cell, bool line)
{
	column_info[right_column_of_cell(cell)].right_line = line;
	return true;
}


char LyXTable::GetAlignment(int cell)
{
	if (IsMultiColumn(cell))
		return cellinfo_of_cell(cell)->alignment;
	else
		return column_info[column_of_cell(cell)].alignment;
}

string LyXTable::GetPWidth(int cell)
{
	int fvcell = FirstVirtualCell(cell);
	
	if (IsMultiColumn(fvcell)) // && !cellinfo_of_cell(cell)->p_width.empty())
		return cellinfo_of_cell(fvcell)->p_width;
	return column_info[column_of_cell(fvcell)].p_width;
}

string LyXTable::GetAlignSpecial(int cell, int what)
{
    if (what == SET_SPECIAL_MULTI)
        return cellinfo_of_cell(cell)->align_special;
    return column_info[column_of_cell(cell)].align_special;
}

int LyXTable::GetWidthOfCell(int cell)
{
	int row = row_of_cell(cell);
	int column1 = column_of_cell(cell);
	int column2 = right_column_of_cell(cell);
	int i;
	int result = 0;
	for (i = column1; i<= column2;i++){
		result += cell_info[row][i].width_of_cell;
	}
  
	result += AdditionalWidth(cell);
  
	return result;
}


int LyXTable::GetBeginningOfTextInCell(int cell)
{
	int x = 0;
   
	switch (GetAlignment(cell)){
	case LYX_ALIGN_CENTER:
		x += (WidthOfColumn(cell) - GetWidthOfCell(cell)) / 2;
		break;
	case LYX_ALIGN_RIGHT:
		x += WidthOfColumn(cell) - GetWidthOfCell(cell) + AdditionalWidth(cell);
		break;
	default: /* LYX_ALIGN_LEFT: nothing :-) */ 
		break;
	}

	// the LaTeX Way :-(
	x += WIDTH_OF_LINE;
	return x;
}


bool LyXTable::IsFirstCell(int cell)
{
	return (column_of_cell(cell) == 0);
}

bool LyXTable::IsLastCell(int cell)
{
	return (right_column_of_cell(cell) == (columns-1));
}


bool LyXTable::calculate_width_of_column(int column)
{
	int i, max;
	int old_column_width = column_info[column].width_of_column;
	max = 0;
	for (i = 0; i<rows; i++) {
		if (cell_info[i][column].width_of_cell > max) {
			max = cell_info[i][column].width_of_cell;
		}
	}
	column_info[column].width_of_column = max;
	return (column_info[column].width_of_column != old_column_width);
}

bool LyXTable::calculate_width_of_column_NMC(int column)
{
    int i, max;
    int old_column_width = column_info[column].width_of_column;
    max = 0;
    for (i = 0; i<rows; ++i) {
        if (!IsMultiColumn(GetCellNumber(column, i)) &&
            (cell_info[i][column].width_of_cell > max)) {
            max = cell_info[i][column].width_of_cell;
        }
    }
    column_info[column].width_of_column = max;
    return (column_info[column].width_of_column != old_column_width);
}

void LyXTable::calculate_width_of_table()
{
	width_of_table = 0;
	for (int i = 0; i < columns; ++i) {
		width_of_table += column_info[i].width_of_column;
	}
}


int LyXTable::row_of_cell(int cell) 
{
    if (cell >= numberofcells)
        return rows-1;
    else if (cell < 0)
        return 0;
    return rowofcell[cell];
}


int LyXTable::column_of_cell(int cell)
{
    if (cell >= numberofcells)
        return columns-1;
    else if (cell < 0)
        return 0;
    return columnofcell[cell];
}


int LyXTable::right_column_of_cell(int cell) 
{
	int row = row_of_cell(cell);
	int column = column_of_cell(cell);
	while (column < columns - 1 &&
	       cell_info[row][column+1].multicolumn == LyXTable::CELL_PART_OF_MULTICOLUMN)
		column++;
	return column;
}


void LyXTable::Write(FILE* file)
{
    int i, j;
    fprintf(file, "multicol5\n");
    fprintf(file, "%d %d %d %d %d %d %d %d\n", rows, columns, is_long_table,
            rotate, endhead, endfirsthead, endfoot, endlastfoot);
    for (i = 0; i<rows; i++){
        fprintf(file, "%d %d %d %d\n", row_info[i].top_line,
                row_info[i].bottom_line, row_info[i].is_cont_row,
                row_info[i].newpage);
    }
    for (i = 0; i<columns; i++){
        fprintf(file, "%d %d %d \"%s\" \"%s\"\n",
                column_info[i].alignment, column_info[i].left_line,
                column_info[i].right_line,
                VSpace(column_info[i].p_width).asLyXCommand().c_str(),
                column_info[i].align_special.c_str());
    }

    for (i = 0; i<rows;i++){
        for (j = 0;j<columns;j++){
            fprintf(file, "%d %d %d %d %d %d %d \"%s\" \"%s\"\n",
                    cell_info[i][j].multicolumn,
                    cell_info[i][j].alignment,
                    cell_info[i][j].top_line,
                    cell_info[i][j].bottom_line,
                    cell_info[i][j].has_cont_row,
                    cell_info[i][j].rotate,
                    cell_info[i][j].linebreaks,
                    cell_info[i][j].align_special.c_str(),
                    cell_info[i][j].p_width.c_str());
        }
    }
}

void LyXTable::Read(FILE* file)
{
    int version;
    int i, j;
    int rows_arg = 0;
    int columns_arg = 0;
    int is_long_table_arg = false;
    int rotate_arg = false;
    string s;
    int a = 0;
    int b = 0;
    int c = 0;
    int d = 0;
    int e = 0;
    int f = 0;
    int g = 0;
    int h = 0;
    char vtmp[100], stmp[100], atmp[100];

    fscanf(file, "%s\n", vtmp);
    s = vtmp;
    if (s.length() > 8)
        version = atoi(s.c_str()+8);
    else
        version = 1;
#ifdef WITH_WARNINGS
#warning Insert a error message window here that this format is not supported anymore
#endif
    if (version < 5) {
	fprintf(stderr, "Tabular format < 5 is not supported anymore\n"
		"Get an older version of LyX (< 1.1.x) for conversion!\n");
	return;
    }
    a = b = c = d = -1;
    fgets(vtmp, sizeof(vtmp), file);
    sscanf(vtmp, "%d %d %d %d %d %d %d %d\n", &rows_arg, &columns_arg,
	   &is_long_table_arg, &rotate_arg, &a, &b, &c, &d);
    Init(rows_arg, columns_arg);
    SetLongTable(is_long_table_arg);
    SetRotateTable(rotate_arg);
    endhead = a;
    endfirsthead = b;
    endfoot = c;
    endlastfoot = d;
    for (i = 0; i<rows; i++){
        a = b = c = d = e = f = g = h = 0;
        fgets(vtmp, sizeof(vtmp), file);
        sscanf(vtmp, "%d %d %d %d\n",
               &a, &b, &c, &d);
        row_info[i].top_line = a;
        row_info[i].bottom_line = b;
        row_info[i].is_cont_row = c;
        row_info[i].newpage = d;
    }
    for (i = 0; i<columns; i++){
        *stmp = 0;
        *atmp = 0;
        fgets(vtmp, sizeof(vtmp), file);
        sscanf(vtmp, "%d %d %d %s %s", &a, &b, &c, stmp, atmp);
        column_info[i].alignment = (char) a;
        column_info[i].left_line = b;
        column_info[i].right_line = c;
        if (*stmp == '"') { /* strip quotes if they exists */
            *stmp = 0;
            *atmp = 0;
            // there are quotes so I have to reread the string correctly
            // this is only because the old format did not have "
            // this means also that atmp is ONLY set here!!!
            if (stmp[1] == '"')
                sscanf(vtmp, "%*d %*d %*d %*s \"%[^\"]\"", atmp);
            else // otherwise after the first empty "" read is aborded
                sscanf(vtmp, "%*d %*d %*d \"%[^\"]\" \"%[^\"]\"", stmp, atmp);
            column_info[i].p_width = stmp;
            column_info[i].align_special = atmp;
        } else if (*stmp)
            column_info[i].p_width = stmp;
    }
    for (i = 0; i<rows;i++){
	for (j = 0;j<columns;j++){
	    *stmp = 0;
	    *atmp = 0;
	    a = b = c = d = e = f = g = 0;
	    fgets(vtmp, sizeof(vtmp), file);
	    sscanf(vtmp, "%d %d %d %d %d %d %d %s %s\n",
		   &a, &b, &c, &d, &e, &f, &g, stmp, atmp);
	    cell_info[i][j].multicolumn = (char) a;
	    cell_info[i][j].alignment = (char) b;
	    cell_info[i][j].top_line = (char) c;
	    cell_info[i][j].bottom_line = (char) d;
	    cell_info[i][j].has_cont_row = (bool) e;
	    cell_info[i][j].rotate = (bool) f;
	    cell_info[i][j].linebreaks = (bool) g;
	    // this is only to see if I have an empty string first
	    // this clause should be always TRUE!!!
	    if (*stmp == '"') {
		*stmp = 0;
		*atmp = 0;
		if (stmp[1] == '"')
		    sscanf(vtmp, "%*d %*d %*d %*d %*d %*d %*d %*s \"%[^\"]\"",
			   atmp);
		else // otherwise after the first empty "" read is aborded
		    sscanf(vtmp, "%*d %*d %*d %*d %*d %*d %*d \"%[^\"]\" \"%[^\"]\"",
			   stmp, atmp);
		cell_info[i][j].align_special = stmp;
		cell_info[i][j].p_width = atmp;
	    } else if (*stmp)
		cell_info[i][j].align_special = stmp;
	}
    }
    set_row_column_number_info();
}


// cell <0 will tex the preamble
// returns the number of printed newlines
int LyXTable::TexEndOfCell(string & file, int cell)
{
    int i;
    int ret = 0;
    int tmp; // tmp2;
    int fcell, nvcell;
    if (ShouldBeVeryLastCell(cell)) {
        // the very end at the very beginning
        if (Linebreaks(cell))
            file += "\\smallskip{}}";
        if (IsMultiColumn(cell))
            file += '}';
        if (RotateCell(cell)) {
            file += "\n\\end{sideways}";
            ret++;
        }
        file += "\\\\\n";
        ret++;
    
        tmp = 0;
        fcell = cell; 
        while (!IsFirstCell(fcell))fcell--;
        for (i = 0; i < NumberOfCellsInRow(fcell); i++){
            if (BottomLine(fcell+i))
                tmp++;
        }
        if (tmp == NumberOfCellsInRow(fcell)){
            file += "\\hline ";
        } else {
            tmp = 0;
            for (i = 0; i < NumberOfCellsInRow(fcell); i++){
                if (BottomLine(fcell+i)){
		    file += "\\cline{";
		    file += tostr(column_of_cell(fcell+i)+1);
		    file += '-';
		    file += tostr(right_column_of_cell(fcell+i)+1);
		    file += "} ";
                    tmp = 1;
                }
            }
        }
        if (tmp){
            file += '\n';
            ret++;
        }
        if (is_long_table)
            file += "\\end{longtable}";
        else
            file += "\\end{tabular}";
        if (rotate) {
            file += "\n\\end{sideways}";
            ret++;
        }
    } else {
        nvcell = NextVirtualCell(cell+1);
        if (cell < 0){
            // preamble
            if (rotate) {
                file += "\\begin{sideways}\n";
                ret++;
            }
            if (is_long_table)
                file += "\\begin{longtable}{";
            else
                file += "\\begin{tabular}{";
            for (i = 0; i<columns;i++){
                if (column_info[i].left_line)
		    file += '|';
                if (!column_info[i].align_special.empty()) {
                    file += column_info[i].align_special.c_str();
                } else if (!column_info[i].p_width.empty()) {
                    file += "p{";
                    file += column_info[i].p_width;
                    file += '}';
                } else {
                    switch (column_info[i].alignment) {
                      case LYX_ALIGN_LEFT:
                          file += 'l';
                          break;
                      case LYX_ALIGN_RIGHT:
                          file += 'r';
                          break;
                      default:
                          file += 'c';
                          break;
                    }
                }
                if (column_info[i].right_line)
                    file += '|';
            }
            file += "}\n";
            ret++;
            tmp = 0;
            if (GetNumberOfCells()){
                fcell = 0;
                for (i = 0; i < NumberOfCellsInRow(fcell); i++){
                    if (TopLine(fcell+i))
                        tmp++;
                }
                if (tmp == NumberOfCellsInRow(fcell)){
                    file += "\\hline ";
                } else {
                    tmp = 0;
                    for (i = 0; i < NumberOfCellsInRow(fcell); i++){
                        if (TopLine(fcell+i)){
			    file += "\\cline{";
			    file += tostr(column_of_cell(fcell+i)+1);
			    file += '-';
			    file += tostr(right_column_of_cell(fcell+i)+1);
			    file += "} ";
                            tmp = 1;
                        }
                    }
                }
                if (tmp){
                    file += '\n';
                    ret++;
                }
            }
            if (RotateCell(0)) {
                file += "\\begin{sideways}\n";
                ret++;
            }
        } else {
            // usual cells
            if (Linebreaks(cell))
                file += "\\smallskip{}}";
            if (IsMultiColumn(cell)){
                file += '}';
            }
            if (RotateCell(cell)) {
                file += "\n\\end{sideways}";
                ret++;
            }
            if (IsLastCell(cell)) {
                int row = row_of_cell(cell);
                string hline1, hline2;
                bool print_hline = true;
                bool pr_top_hline, flag1, flag2;
                flag1 = IsLongTable() &&
                    ((row == endhead) || (row == endfirsthead) ||
                     (row == endfoot) || (row == endlastfoot));
                row++;
                flag2 = IsLongTable() &&
                    ((row <= endhead) || (row <= endfirsthead) ||
                     (row <= endfoot) || (row <= endlastfoot));
                row--;
                // print the bottom hline only if (otherwise it is doubled):
                // - is no LongTable
                // - there IS a first-header
                // - the next row is no special header/footer
                //   & this row is no special header/footer
                // - the next row is a special header/footer
                //   & this row is a special header/footer
                pr_top_hline = (flag1 && flag2) || (!flag1 && !flag2) ||
                    (endfirsthead == endhead);
                file += "\\\\\n";
                ret++;
                tmp = 0;
                fcell = cell;
                while (!IsFirstCell(fcell))
                    fcell--;
                for (i = 0; i < NumberOfCellsInRow(cell); i++){
                    if (BottomLine(fcell+i))
                        tmp++;
                }
                if (tmp == NumberOfCellsInRow(cell)){
                    file += "\\hline ";
                    hline1 = "\\hline ";
                } else {
                    tmp = 0;
                    for (i = 0; i < NumberOfCellsInRow(fcell); i++){
                        if (BottomLine(fcell+i)){
                            file += "\\cline{";
                            file += tostr(column_of_cell(fcell+i)+1);
                            file += '-';
                            file += tostr(right_column_of_cell(fcell+i)+1);
                            file += "} ";
                            hline1 += "\\cline{";
                            hline1 += tostr(column_of_cell(fcell+i)+1);
                            hline1 += '-';
                            hline1 += tostr(right_column_of_cell(fcell+i)+1);
                            hline1 += "} ";
                            tmp = 1;
                        }
                    }
                }
                if (tmp){
                    file += '\n';
                    ret++;
                }
                if (IsLongTable() && (row == endfoot)) {
                    file += "\\endfoot\n";
                    ret++;
                    print_hline = false; // no double line below footer
                }
                if (IsLongTable() && (row == endlastfoot)) {
                    file += "\\endlastfoot\n";
                    ret++;
                    print_hline = false; // no double line below footer
                }
                if (IsLongTable() && row_info[row].newpage) {
                    file += "\\newpage\n";
                    ret++;
                    print_hline = false; // no line below a \\newpage-command
                }
                tmp = 0;
                if (nvcell < numberofcells && (cell < GetNumberOfCells()-1) &&
                    !ShouldBeVeryLastCell(cell)) {
                    fcell = nvcell;
                    for (i = 0; i < NumberOfCellsInRow(fcell); i++){
                        if (TopLine(fcell+i))
                            tmp++;
                    }
                    if (tmp == NumberOfCellsInRow(fcell)){
                        if (print_hline)
                            file += "\\hline ";
                        hline2 = "\\hline ";
                    }
                    else {
                        tmp = 0;
                        for (i = 0; i < NumberOfCellsInRow(fcell); i++){
                            if (TopLine(fcell+i)){
                                if (print_hline) {
				    file += "\\cline{";
				    file += tostr(column_of_cell(fcell+i)+1);
				    file += '-';
				    file += tostr(right_column_of_cell(fcell+i)+1);
				    file += "} ";
				}
                                hline2 += "\\cline{";
                                hline2 += tostr(column_of_cell(fcell+i)+1);
                                hline2 += '-';
                                hline2 += tostr(right_column_of_cell(fcell+i)+1);
                                hline2 += "} ";
                                tmp = 1;
                            }
                        }
                    }
                    if (tmp && print_hline){
                        file += '\n';
                        ret++;
                    }
                }
                // the order here is important as if one defines two
                // or more things in one line only the first entry is
                // displayed the other are set to an empty-row. This
                // is important if I have a footer and want that the
                // lastfooter is NOT displayed!!!
                bool sflag2 = (row == endhead) || (row == endfirsthead) ||
                    (row == endfoot) || (row == endlastfoot);
                row--;
//                sflag2 = IsLongTable() && (row >= 0) &&
//                    (sflag2 || (row == endhead) || (row == endfirsthead));
                row += 2;
                bool sflag1 = IsLongTable() && (row != endhead) &&
                    (row != endfirsthead) &&
                    ((row == endfoot) || (row == endlastfoot));
                row--;
                if (IsLongTable() && (row == endhead)) {
		    file += "\\endhead\n";
                    ret++;
                }
                if (IsLongTable() && (row == endfirsthead)) {
                    file += "\\endfirsthead\n";
                    ret++;
                }
                if (sflag1) { // add the \hline for next foot row
                    if (!hline1.empty()) {
                        file += hline1 + '\n';
                        ret++;
                    }
                }
                // add the \hline for the first row
                if (pr_top_hline && sflag2) {
                    if (!hline2.empty()) {
                        file += hline2 + '\n';
                        ret++;
                    }
                }
                if (nvcell < numberofcells && RotateCell(nvcell)) {
                    file += "\\begin{sideways}\n";
                    ret++;
                }
            } else {
                file += "&\n";
                ret++;
                if (nvcell < numberofcells && RotateCell(nvcell)) {
                    file += "\\begin{sideways}\n";
                    ret++;
                }
            }
        }
        if (nvcell < numberofcells && IsMultiColumn(nvcell)) {
            file += "\\multicolumn{";
	    file += tostr(cells_in_multicolumn(nvcell));
	    file += "}{";
            if (!cellinfo_of_cell(cell+1)->align_special.empty()) {
                file += cellinfo_of_cell(cell+1)->align_special;
                file += "}{";
            } else {
                if (LeftLine(nvcell))
                    file += '|';
                if (!GetPWidth(nvcell).empty()) {
                    file += "p{";
                    file += GetPWidth(nvcell);
                    file += '}';
                } else {
                    switch (GetAlignment(nvcell)) {
                      case LYX_ALIGN_LEFT: file += 'l'; break;
                      case LYX_ALIGN_RIGHT: file += 'r'; break;
                      default:  file += 'c'; break;
                    }
                }
                if (RightLine(nvcell))
                    file += '|';
                //if (column_of_cell(cell+2)!= 0 && LeftLine(cell+2))
                if (((nvcell+1) < numberofcells) &&
                    (NextVirtualCell(nvcell+1) < numberofcells) &&
                    (column_of_cell(NextVirtualCell(nvcell+1))!= 0) &&
                    LeftLine(NextVirtualCell(nvcell+1)))
                    file += '|';
                file += "}{";
            }
        }
        if (nvcell < numberofcells && Linebreaks(nvcell)) {
//            !column_info[column_of_cell(nvcell)].p_width.empty()) {
            file += "\\parbox{";
	    file += GetPWidth(nvcell);
	    file += "}{\\smallskip{}";
	}
    }
    return ret;
}


// cell <0 will tex the preamble
// returns the number of printed newlines
int LyXTable::RoffEndOfCell(FILE* file, int cell)
{
    int i, j;
    int ret = 0;

    if (cell == GetNumberOfCells() - 1){
        // the very end at the very beginning
        if (CellHasContRow(cell) >= 0) {
            fprintf(file, "\nT}");
            ret++;
        }
        fprintf(file, "\n");
        ret++;
        if (row_info[row_of_cell(cell)].bottom_line) {
            fprintf(file, "_\n");
            ret++;
        }
        fprintf(file, ".TE\n.pl 1c");
    } else {  
        if (cell < 0){
            int fcell = 0;
            // preamble
            fprintf(file, "\n.pl 500c\n.TS\n");
            for (j = 0; j<rows; j++) {
                for (i = 0; i<columns;i++, fcell++) {
                    if (column_info[i].left_line)
                        fprintf(file, " | ");
                    if (cell_info[j][i].multicolumn == CELL_PART_OF_MULTICOLUMN)
                        fprintf(file, "s");
                    else {
                        switch (column_info[i].alignment) {
                          case LYX_ALIGN_LEFT:
                              fprintf(file, "l");
                              break;
                          case LYX_ALIGN_RIGHT:
                              fprintf(file, "r");
                              break;
                          default:
                              fprintf(file, "c");
                              break;
                        }
                    }
                    if (!column_info[i].p_width.empty())
                        fprintf(file, "w(%s)", column_info[i].p_width.c_str());
                    if (column_info[i].right_line)
                        fprintf(file, " | ");
                }
                if ((j+1) < rows) {
                    fprintf(file, "\n");
                    ret++;
                }
            }
            fprintf(file, ".\n");
            ret++;
            if (row_info[0].top_line) {
                fprintf(file, "_\n");
                ret++;
            }
            if (CellHasContRow(0) >= 0) {
                fprintf(file, "T{\n");
                ret++;
            }
        } else {
            // usual cells
            if (CellHasContRow(cell) >= 0) {
                fprintf(file, "\nT}");
                ret++;
            }
            if (right_column_of_cell(cell) == columns -1){
                fprintf(file, "\n");
                ret++;
                int row = row_of_cell(cell);
                if (row_info[row++].bottom_line) {
                    fprintf(file, "_\n");
                    ret++;
                }
                if ((row < rows) && row_info[row].top_line) {
                    fprintf(file, "_\n");
                    ret++;
                }
            } else
                fprintf(file, "\t");
            if ((cell < GetNumberOfCells() - 1) &&
                (CellHasContRow(cell+1) >= 0)) {
                fprintf(file, "T{\n");
                ret++;
            }
        }
    }
    return ret;
}

char const *LyXTable::getDocBookAlign(int cell, bool isColumn)
{
    int i;
    if (isColumn)
       i = cell;
    else
       i = column_of_cell(cell);
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
int LyXTable::DocBookEndOfCell(string & file, int cell, int &depth)
{
    int i;
    int ret = 0;
    //int tmp; // tmp2; // unused
    int nvcell; // fcell; // unused
    if (ShouldBeVeryLastCell(cell)) {
       addNewlineAndDepth(file,--depth);
        file += "</ENTRY>";
       addNewlineAndDepth(file,--depth);
        file += "</ROW>";
       addNewlineAndDepth(file,--depth);
        file += "</TBODY>";
       addNewlineAndDepth(file,--depth);
        if (is_long_table)
            file += "</TGROUP>";
        else
            file += "</TGROUP>";
       addNewlineAndDepth(file,--depth);
        ret += 4;
    } else {
        nvcell = NextVirtualCell(cell+1);
        if (cell < 0) {
            // preamble
            if (is_long_table)
                file += "<TGROUP ";
            else
                file += "<TGROUP ";
            file += "COLS='";
            file += tostr(columns);
            file += "' COLSEP='1' ROWSEP='1'>";
           addNewlineAndDepth(file,++depth);
            ret++;
            for (i = 0; i < columns; ++i) {
                file += "<COLSPEC ALIGN='";
               file += getDocBookAlign(i, true);
               file += "' COLNAME='col";
                file += tostr(i+1);
                file += "' COLNUM='";
                file += tostr(i+1);
               file += "' COLSEP='";
               if (i == (columns-1)) {
                    file += '1';
               } else {
                   if (column_info[i].right_line ||
                       column_info[i+1].left_line)
                       file += '1';
                   else
                       file += '0';
               }
               file += "'>";
               addNewlineAndDepth(file, depth);
                ret++;
#ifdef NOT_HANDLED_YET_AS_I_DONT_KNOW_HOW
                if (column_info[i].left_line)
                    file += '|';
#endif
            }
            file += "<TBODY>";
           addNewlineAndDepth(file,++depth);
            file += "<ROW>";
           addNewlineAndDepth(file,++depth);
            file += "<ENTRY ALIGN='";
            file += getDocBookAlign(0);
           file += "'";
           if (IsMultiColumn(0)) {
               file += " NAMEST='col1' NAMEEND='col";
               file += tostr(cells_in_multicolumn(0));
               file += "'";
           }
            file += ">";
           addNewlineAndDepth(file,++depth);
            ret += 3;
        } else {
            if (IsLastCell(cell)) {
               addNewlineAndDepth(file,--depth);
                file += "</ENTRY>";
               addNewlineAndDepth(file,--depth);
                file += "</ROW>";
               addNewlineAndDepth(file, depth);
               file += "<ROW>";
               addNewlineAndDepth(file,++depth);
                file += "<ENTRY ALIGN='";
                file += getDocBookAlign(cell+1);
                file += "' VALIGN='middle'";
               if (IsMultiColumn(cell+1)) {
                   file += " NAMEST='col";
                   file += tostr(column_of_cell(cell+1) + 1);
                   file += "' NAMEEND='col";
                   file += tostr(column_of_cell(cell+1) +
                       cells_in_multicolumn(cell+1));
                   file += "'";
               }
               file += ">";
               addNewlineAndDepth(file,++depth);
                ret += 4;
            } else {
               addNewlineAndDepth(file,--depth);
                file += "</ENTRY>";
               addNewlineAndDepth(file, depth);
                file += "<ENTRY ALIGN='";
                file += getDocBookAlign(cell+1);
                file += "' VALIGN='middle'";
               if (IsMultiColumn(cell+1)) {
                   file += " NAMEST='col";
                   file += tostr(column_of_cell(cell+1) + 1);
                   file += "' NAMEEND='col";
                   file += tostr(column_of_cell(cell+1) +
                       cells_in_multicolumn(cell+1));
                   file += "'";
               }
               file += ">";
               addNewlineAndDepth(file,++depth);
                ret += 3;
            }
        }
    }
    return ret;
}


bool LyXTable::IsMultiColumn(int cell)
{
    int fvcell = FirstVirtualCell(cell);

    return (cellinfo_of_cell(fvcell)->multicolumn != LyXTable::CELL_NORMAL);
}


LyXTable::cellstruct* LyXTable::cellinfo_of_cell(int cell)
{
    int row = row_of_cell(cell);
    int column = column_of_cell(cell);
    return  &cell_info[row][column];
}
   

void LyXTable::SetMultiColumn(int cell, int number)
{
    int fvcell = FirstVirtualCell(cell);
    int new_width = cellinfo_of_cell(fvcell)->width_of_cell;
    
    cellinfo_of_cell(fvcell)->multicolumn = LyXTable::CELL_BEGIN_OF_MULTICOLUMN;
    cellinfo_of_cell(fvcell)->alignment = column_info[column_of_cell(fvcell)].alignment;
    cellinfo_of_cell(fvcell)->top_line = row_info[row_of_cell(fvcell)].top_line;
    cellinfo_of_cell(fvcell)->bottom_line = row_info[row_of_cell(fvcell)].bottom_line;
    for (number--;number>0;number--){
        cellinfo_of_cell(fvcell+number)->multicolumn = 
            LyXTable::CELL_PART_OF_MULTICOLUMN;
        new_width += cellinfo_of_cell(fvcell+number)->width_of_cell;
    }
    set_row_column_number_info();
    SetWidthOfCell(fvcell, new_width);
}


int LyXTable::cells_in_multicolumn(int cell)
{
    int row = row_of_cell(cell);
    int column = column_of_cell(cell);
    int result = 1;
    column++;
    while (column < columns && cell_info[row][column].multicolumn
           == LyXTable::CELL_PART_OF_MULTICOLUMN){
        result++;
        column++;
    }
    return result;
}


int  LyXTable::UnsetMultiColumn(int cell)
{
    int fvcell = FirstVirtualCell(cell);
    int row = row_of_cell(fvcell);
    int column = column_of_cell(fvcell);
    
    int result = 0;
    
    if (cell_info[row][column].multicolumn
        == LyXTable::CELL_BEGIN_OF_MULTICOLUMN){
        cell_info[row][column].multicolumn = LyXTable::CELL_NORMAL;
        column++;
        while (column < columns &&
               cell_info[row][column].multicolumn
               == LyXTable::CELL_PART_OF_MULTICOLUMN){
            cell_info[row][column].multicolumn = 
                LyXTable::CELL_NORMAL;
            column++;
            result++;
        }
    }
    set_row_column_number_info();
    return result;
}


void LyXTable::delete_column(int column)
{
    int i, j;
    columnstruct *column_info2 = new columnstruct[columns-1];
   
    for (i = 0; i<column; i++){
        column_info2[i] = column_info[i];
    }
    for (i = column; i<columns-1; i++){
        column_info2[i] = column_info[i+1];
    }
   
    delete[] column_info;
    column_info = column_info2;

    for (i = 0; i<rows;i++){
        cellstruct* tmp = cell_info[i];
        cell_info[i] = new cellstruct[columns-1];
        for (j = 0; j<column; j++){
            cell_info[i][j] = tmp[j];
        }
        for (j = column; j<columns-1; j++){
            cell_info[i][j] = tmp[j+1];
        }
        delete[] tmp;
    }

    columns--;
    Reinit();
}

void LyXTable::SetLongTable(int what)
{
    is_long_table = what;
}

bool LyXTable::IsLongTable()
{
    return is_long_table;
}

void LyXTable::SetRotateTable(int what)
{
    rotate = what;
}

bool LyXTable::RotateTable()
{
    return rotate;
}

void LyXTable::SetRotateCell(int cell, int what)
{
    cellinfo_of_cell(cell)->rotate = what;
}

bool LyXTable::RotateCell(int cell)
{
    return cellinfo_of_cell(cell)->rotate;
}

bool LyXTable::NeedRotating()
{
    if (rotate)
        return true;
    for (int i = 0; i<rows;i++){
        for (int j = 0;j<columns;j++){
            if (cell_info[i][j].rotate)
                return true;
        }
    }
    return false;
}

void LyXTable::AppendContRow(int cell)
{
    int row = row_of_cell(cell)+1;

    if (!RowHasContRow(cell) || (CellHasContRow(cell)>= 0))
        AppendRow(cell);
    row_info[row].is_cont_row = true;
    row_info[row].top_line = false;
    cell_info[row-1][column_of_cell(cell)].has_cont_row = true;
    Reinit();
}

bool LyXTable::IsContRow(int cell)
{
    return row_info[row_of_cell(cell)].is_cont_row;
}

int LyXTable::CellHasContRow(int cell)
{
    int row = row_of_cell(cell);

    if (VeryLastRow(cell))
        return -1;
    if (cell_info[row][column_of_cell(cell)].has_cont_row)
        return cell_info[row+1][column_of_cell(cell)].cellno;
    return -1;
}

bool LyXTable::RowHasContRow(int cell)
{
    int row = row_of_cell(cell) + 1;

    if (row < rows)
        return row_info[row].is_cont_row;
    return false;
}

int LyXTable::FirstVirtualCell(int cell)
{
    if (!IsContRow(cell))
        return cell;
    int row = row_of_cell(cell);
    int column = column_of_cell(cell);
    for(;(row>0) && IsContRow(cell_info[row][column].cellno); row--)
        ;
    return cell_info[row][column].cellno;
}


int LyXTable::NextVirtualCell(int cell)
{
    if (!IsContRow(cell))
        return cell;
    int row = row_of_cell(cell);
    for(;(row < rows - 1) && IsContRow(cell_info[row][0].cellno); ++row)
	;
    // what if(row >= rows) ?
    return cell_info[row][0].cellno;
}


bool LyXTable::ShouldBeVeryLastCell(int cell)
// "very last cell" ..of what? the row? the table?
// "Cell" in this context appears to not count `virtual' cells
{
    int fcell = cell + 1;

    if (cell == GetNumberOfCells() - 1)
        return true; // not really sure if I should return false here
    if (!IsContRow(fcell))
        return false;
    while((fcell < GetNumberOfCells() - 1) && IsContRow(fcell))
        fcell++;
    if (fcell < GetNumberOfCells() - 1)
        return false;
    return true;
}

bool LyXTable::ShouldBeVeryLastRow(int cell)
{
    if (CellHasContRow(cell)>= 0)
        return false;
    int row = row_of_cell(cell)+1;
    int column = column_of_cell(cell);
    while((row < rows) && IsContRow(cell_info[row][column].cellno))
        row++;
    if (row < rows)
        return false; // found another valid row
    // I do not have any valid row after the actual
    return true;
}

int LyXTable::GetCellAbove(int cell)
{
    int row = row_of_cell(cell);
    
    if (row > 0)
        return cell_info[row-1][column_of_cell(cell)].cellno;
    return cell;
}

int LyXTable::GetCellNumber(int column, int row)
{
    if (column >= columns)
        column = columns - 1;
    else if (column < 0)
        column = 0;
    if (row >= rows)
        row = rows - 1;
    else if (row < 0)
        row = 0;
    
    return cell_info[row][column].cellno;
}

void LyXTable::SetLinebreaks(int cell, bool what)
{
    cellinfo_of_cell(FirstVirtualCell(cell))->linebreaks = what;
}

bool LyXTable::Linebreaks(int cell)
{
    int fvcell = FirstVirtualCell(cell);

    if (column_info[column_of_cell(fvcell)].p_width.empty() &&
        !(IsMultiColumn(fvcell) && !cellinfo_of_cell(fvcell)->p_width.empty()))
        return false;
    return cellinfo_of_cell(fvcell)->linebreaks;
}

void LyXTable::SetLTHead(int cell, bool first)
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

bool LyXTable::RowOfLTHead(int cell)
{
    if ((endhead+1) > rows)
        endhead = -1;
    return (row_of_cell(cell) == endhead);
}

bool LyXTable::RowOfLTFirstHead(int cell)
{
    if ((endfirsthead+1) > rows)
        endfirsthead = -1;
    return (row_of_cell(cell) == endfirsthead);
}

void LyXTable::SetLTFoot(int cell, bool last)
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

bool LyXTable::RowOfLTFoot(int cell)
{
    if ((endfoot+1) > rows) {
        endfoot = -1;
        return false;
    }
    return (row_of_cell(cell) == endfoot);
}

bool LyXTable::RowOfLTLastFoot(int cell)
{
    if ((endlastfoot+1) > rows)
        endlastfoot = -1;
    return (row_of_cell(cell) == endlastfoot);
}

void LyXTable::SetLTNewPage(int cell, bool what)
{
    row_info[row_of_cell(cell)].newpage = what;
}

bool LyXTable::LTNewPage(int cell)
{
    return row_info[row_of_cell(cell)].newpage;
}
