/**
 * \file tabular.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 * \author José Matos
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "tabular.h"

#include "buffer.h"
#include "debug.h"
#include "LaTeXFeatures.h"
#include "lyxlex.h"
#include "paragraph.h"

#include "insets/insettabular.h"

#include "support/LAssert.h"
#include "support/lstrings.h"
#include "support/tostr.h"

#include "support/std_sstream.h"

using namespace lyx::support;

using std::abs;
using std::endl;
using std::getline;
using std::max;

using std::istream;
using std::ostream;
using std::ostringstream;
using std::vector;

#ifndef CXX_GLOBAL_CSTD
using std::strlen;
#endif

namespace {

int const WIDTH_OF_LINE = 5;

template <class T>
string const write_attribute(string const & name, T const & t)
{
	string const s = tostr(t);
	return s.empty() ? s : " " + name + "=\"" + s + "\"";
}

string const write_attribute(string const & name, string const & t)
{
	return t.empty() ? t : " " + name + "=\"" + t + "\"";
}


string const write_attribute(string const & name, bool const & b)
{
	// we write only true attribute values so we remove a bit of the
	// file format bloat for tabulars.
	return b ? write_attribute(name, tostr(b)) : string();
}


string const write_attribute(string const & name, int const & i)
{
	// we write only true attribute values so we remove a bit of the
	// file format bloat for tabulars.
	return i ? write_attribute(name, tostr(i)) : string();
}


string const write_attribute(string const & name, LyXLength const & value)
{
	// we write only the value if we really have one same reson as above.
	return value.zero() ? string() : write_attribute(name, value.asString());
}


string const tostr(LyXAlignment const & num)
{
	switch (num) {
	case LYX_ALIGN_NONE:
		return "none";
	case LYX_ALIGN_BLOCK:
		return "block";
	case LYX_ALIGN_LEFT:
		return "left";
	case LYX_ALIGN_CENTER:
		return "center";
	case LYX_ALIGN_RIGHT:
		return "right";
	case LYX_ALIGN_LAYOUT:
		return "layout";
	case LYX_ALIGN_SPECIAL:
		return "special";
	}
	return string();
}


string const tostr(LyXTabular::VAlignment const & num)
{
	switch (num) {
	case LyXTabular::LYX_VALIGN_TOP:
		return "top";
	case LyXTabular::LYX_VALIGN_MIDDLE:
		return "middle";
	case LyXTabular::LYX_VALIGN_BOTTOM:
		return "bottom";
	}
	return string();
}


string const tostr(LyXTabular::BoxType const & num)
{
	switch (num) {
	case LyXTabular::BOX_NONE:
		return "none";
	case LyXTabular::BOX_PARBOX:
		return "parbox";
	case LyXTabular::BOX_MINIPAGE:
		return "minipage";
	}
	return string();
}

// I would have liked a fromstr template a lot better. (Lgb)
bool string2type(string const str, LyXAlignment & num)
{
	if (str == "none")
		num = LYX_ALIGN_NONE;
	else if (str == "block")
		num = LYX_ALIGN_BLOCK;
	else if (str == "left")
		num = LYX_ALIGN_LEFT;
	else if (str == "center")
		num = LYX_ALIGN_CENTER;
	else if (str == "right")
		num = LYX_ALIGN_RIGHT;
	else
		return false;
	return true;
}


bool string2type(string const str, LyXTabular::VAlignment & num)
{
	if (str == "top")
		num = LyXTabular::LYX_VALIGN_TOP;
	else if (str == "middle" )
		num = LyXTabular::LYX_VALIGN_MIDDLE;
	else if (str == "bottom")
		num = LyXTabular::LYX_VALIGN_BOTTOM;
	else
		return false;
	return true;
}


bool string2type(string const str, LyXTabular::BoxType & num)
{
	if (str == "none")
		num = LyXTabular::BOX_NONE;
	else if (str == "parbox")
		num = LyXTabular::BOX_PARBOX;
	else if (str == "minipage")
		num = LyXTabular::BOX_MINIPAGE;
	else
		return false;
	return true;
}


bool string2type(string const str, bool & num)
{
	if (str == "true")
		num = true;
	else if (str == "false")
		num = false;
	else
		return false;
	return true;
}


bool getTokenValue(string const & str, const char * token, string & ret)
{
	ret.erase();
	size_t token_length = strlen(token);
	string::size_type pos = str.find(token);

	if (pos == string::npos || pos + token_length + 1 >= str.length()
		|| str[pos + token_length] != '=')
		return false;
	pos += token_length + 1;
	char ch = str[pos];
	if ((ch != '"') && (ch != '\'')) { // only read till next space
		ret += ch;
		ch = ' ';
	}
	while ((pos < str.length() - 1) && (str[++pos] != ch))
		ret += str[pos];

	return true;
}


bool getTokenValue(string const & str, const char * token, int & num)
{
	string tmp;
	num = 0;
	if (!getTokenValue(str, token, tmp))
		return false;
	num = strToInt(tmp);
	return true;
}


bool getTokenValue(string const & str, const char * token, LyXAlignment & num)
{
	string tmp;
	if (!getTokenValue(str, token, tmp))
		return false;
	return string2type(tmp, num);
}


bool getTokenValue(string const & str, const char * token,
				   LyXTabular::VAlignment & num)
{
	string tmp;
	if (!getTokenValue(str, token, tmp))
		return false;
	return string2type(tmp, num);
}


bool getTokenValue(string const & str, const char * token,
				   LyXTabular::BoxType & num)
{
	string tmp;
	if (!getTokenValue(str, token, tmp))
		return false;
	return string2type(tmp, num);
}


bool getTokenValue(string const & str, const char * token, bool & flag)
{
	// set the flag always to false as this should be the default for bools
	// not in the file-format.
	flag = false;
	string tmp;
	if (!getTokenValue(str, token, tmp))
		return false;
	return string2type(tmp, flag);
}


bool getTokenValue(string const & str, const char * token, LyXLength & len)
{
	// set the lenght to be zero() as default as this it should be if not
	// in the file format.
	len = LyXLength();
	string tmp;
	if (!getTokenValue(str, token, tmp))
		return false;
	return isValidLength(tmp, &len);
}


void l_getline(istream & is, string & str)
{
	str.erase();
	while (str.empty()) {
		getline(is, str);
		if (!str.empty() && str[str.length() - 1] == '\r')
			str.erase(str.length() - 1);
	}
}

} // namespace

/// Define a few methods for the inner structs

LyXTabular::cellstruct::cellstruct(BufferParams const & bg)
	: inset(bg)
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


LyXTabular::ltType::ltType()
{
	topDL = false;
	bottomDL = false;
	empty = false;
}


LyXTabular::LyXTabular(BufferParams const & bp,
		       InsetTabular * inset, int rows_arg, int columns_arg)
{
	owner_ = inset;
	cur_cell = -1;
	init(bp, rows_arg, columns_arg);
}


LyXTabular::LyXTabular(BufferParams const & bp,
		       InsetTabular * inset, LyXTabular const & lt)
{
	owner_ = inset;
	cur_cell = -1;
	init(bp, lt.rows_, lt.columns_, &lt);
}


LyXTabular::LyXTabular(Buffer const & buf, InsetTabular * inset, LyXLex & lex)
{
	owner_ = inset;
	cur_cell = -1;
	read(buf, lex);
}


// activates all lines and sets all widths to 0
void LyXTabular::init(BufferParams const & bp,
		      int rows_arg, int columns_arg, LyXTabular const * lt)
{
	rows_ = rows_arg;
	columns_ = columns_arg;
	row_info = row_vector(rows_, rowstruct());
	column_info = column_vector(columns_, columnstruct());
	cell_info = cell_vvector(rows_, cell_vector(columns_, cellstruct(bp)));

	if (lt) {
		operator=(*lt);
		return;
	}

	int cellno = 0;
	for (int i = 0; i < rows_; ++i) {
		for (int j = 0; j < columns_; ++j) {
			cell_info[i][j].inset.setOwner(owner_);
			cell_info[i][j].inset.setDrawFrame(InsetText::LOCKED);
			cell_info[i][j].cellno = cellno++;
		}
		cell_info[i].back().right_line = true;
	}
	row_info.back().bottom_line = true;
	row_info.front().bottom_line = true;

	for (int i = 0; i < columns_; ++i)
		calculate_width_of_column(i);

	column_info.back().right_line = true;

	calculate_width_of_tabular();

	rowofcell.clear();
	columnofcell.clear();
	set_row_column_number_info();
	is_long_tabular = false;
	rotate = false;
}


void LyXTabular::appendRow(BufferParams const & bp, int cell)
{
	++rows_;

	int row = row_of_cell(cell);

	row_vector::iterator rit = row_info.begin() + row;
	row_info.insert(rit, rowstruct());
	// now set the values of the row before
	row_info[row] = row_info[row + 1];

#if 0
	cell_vvector::iterator cit = cell_info.begin() + row;
	cell_info.insert(cit, vector<cellstruct>(columns_, cellstruct(bp)));
#else
	cell_vvector c_info = cell_vvector(rows_, cell_vector(columns_,
							      cellstruct(bp)));

	for (int i = 0; i <= row; ++i)
		for (int j = 0; j < columns_; ++j)
			c_info[i][j] = cell_info[i][j];

	for (int i = row + 1; i < rows_; ++i)
		for (int j = 0; j < columns_; ++j)
			c_info[i][j] = cell_info[i-1][j];

	cell_info = c_info;
	++row;
	for (int j = 0; j < columns_; ++j) {
		cell_info[row][j].inset.clear(false);
		if (bp.tracking_changes)
			cell_info[row][j].inset.markNew(true);
	}
#endif
	Reinit();
}


void LyXTabular::deleteRow(int row)
{
	// Not allowed to delete last row
	if (rows_ == 1)
		return;

	row_info.erase(row_info.begin() + row);
	cell_info.erase(cell_info.begin() + row);
	--rows_;
	Reinit();
}


void LyXTabular::appendColumn(BufferParams const & bp, int cell)
{
	++columns_;

	cell_vvector c_info = cell_vvector(rows_, cell_vector(columns_,
							      cellstruct(bp)));
	int const column = column_of_cell(cell);
	column_vector::iterator cit = column_info.begin() + column + 1;
	column_info.insert(cit, columnstruct());
	// set the column values of the column before
	column_info[column + 1] = column_info[column];

	for (int i = 0; i < rows_; ++i) {
		for (int j = 0; j <= column; ++j)
			c_info[i][j] = cell_info[i][j];

		for (int j = column + 1; j < columns_; ++j)
			c_info[i][j] = cell_info[i][j - 1];

		// care about multicolumns
		if (c_info[i][column + 1].multicolumn == CELL_BEGIN_OF_MULTICOLUMN)
			c_info[i][column + 1].multicolumn = CELL_PART_OF_MULTICOLUMN;

		if (column + 2 >= columns_
		    || c_info[i][column + 2].multicolumn != CELL_PART_OF_MULTICOLUMN)
			c_info[i][column + 1].multicolumn = LyXTabular::CELL_NORMAL;
	}
	cell_info = c_info;
	//++column;
	for (int i = 0; i < rows_; ++i) {
		cell_info[i][column + 1].inset.clear(false);
		if (bp.tracking_changes)
			cell_info[i][column + 1].inset.markNew(true);
	}
	Reinit();
}


void LyXTabular::deleteColumn(int column)
{
	// Not allowed to delete last column
	if (columns_ == 1)
		return;

	column_info.erase(column_info.begin() + column);
	for (int i = 0; i < rows_; ++i)
		cell_info[i].erase(cell_info[i].begin() + column);
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

	for (int i = 0; i < columns_; ++i)
		calculate_width_of_column(i);

	calculate_width_of_tabular();

	set_row_column_number_info();
}


void LyXTabular::set_row_column_number_info(bool oldformat)
{
	numberofcells = -1;
	for (int row = 0; row < rows_; ++row) {
		for (int column = 0; column < columns_; ++column) {
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
			if (isPartOfMultiColumn(row,column))
				continue;
			// now set the right line of multicolumns right for oldformat read
			if (oldformat &&
				cell_info[row][column].multicolumn == CELL_BEGIN_OF_MULTICOLUMN)
			{
				int cn = cells_in_multicolumn(cell_info[row][column].cellno);
				cell_info[row][column].right_line =
					cell_info[row][column+cn-1].right_line;
			}
			cell_info[row][column].inset.setAutoBreakRows(
				!getPWidth(getCellNumber(row, column)).zero());
		}
	}
}


int LyXTabular::getNumberOfCells() const
{
	return numberofcells;
}


int LyXTabular::numberOfCellsInRow(int cell) const
{
	int const row = row_of_cell(cell);
	int result = 0;
	for (int i = 0; i < columns_; ++i)
		if (cell_info[row][i].multicolumn != LyXTabular::CELL_PART_OF_MULTICOLUMN)
			++result;
	return result;
}


// returns 1 if there is a topline, returns 0 if not
bool LyXTabular::topLine(int cell, bool onlycolumn) const
{
	if (!onlycolumn && isMultiColumn(cell))
		return cellinfo_of_cell(cell).top_line;
	return row_info[row_of_cell(cell)].top_line;
}


bool LyXTabular::bottomLine(int cell, bool onlycolumn) const
{
	if (!onlycolumn && isMultiColumn(cell))
		return cellinfo_of_cell(cell).bottom_line;
	return row_info[row_of_cell(cell)].bottom_line;
}


bool LyXTabular::leftLine(int cell, bool onlycolumn) const
{
	if (!onlycolumn && isMultiColumn(cell) &&
		(isFirstCellInRow(cell) || isMultiColumn(cell-1)))
	{
		if (cellinfo_of_cell(cell).align_special.empty())
			return cellinfo_of_cell(cell).left_line;
		return prefixIs(ltrim(cellinfo_of_cell(cell).align_special), "|");
	}
	if (column_info[column_of_cell(cell)].align_special.empty())
		return column_info[column_of_cell(cell)].left_line;
	return prefixIs(ltrim(column_info[column_of_cell(cell)].align_special), "|");
}


bool LyXTabular::rightLine(int cell, bool onlycolumn) const
{
	if (!onlycolumn && isMultiColumn(cell) &&
		(isLastCellInRow(cell) || isMultiColumn(cell+1)))
	{
		if (cellinfo_of_cell(cell).align_special.empty())
			return cellinfo_of_cell(cell).right_line;
		return suffixIs(rtrim(cellinfo_of_cell(cell).align_special), "|");
	}
	if (column_info[column_of_cell(cell)].align_special.empty())
		return column_info[right_column_of_cell(cell)].right_line;
	return suffixIs(rtrim(column_info[column_of_cell(cell)].align_special), "|");
}


bool LyXTabular::topAlreadyDrawn(int cell) const
{
	int row = row_of_cell(cell);
	if (row > 0 && !getAdditionalHeight(row)) {
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


bool LyXTabular::leftAlreadyDrawn(int cell) const
{
	int column = column_of_cell(cell);
	if (column > 0) {
		int row = row_of_cell(cell);
		while (--column &&
			   (cell_info[row][column].multicolumn ==
				LyXTabular::CELL_PART_OF_MULTICOLUMN));
		if (getAdditionalWidth(cell_info[row][column].cellno))
			return false;
		return rightLine(cell_info[row][column].cellno);
	}
	return false;
}


bool LyXTabular::isLastRow(int cell) const
{
	return (row_of_cell(cell) == rows_ - 1);
}


int LyXTabular::getAdditionalHeight(int row) const
{
	if (!row || row >= rows_)
		return 0;

	bool top = true;
	bool bottom = true;

	for (int column = 0; column < columns_ && bottom; ++column) {
		switch (cell_info[row - 1][column].multicolumn) {
		case LyXTabular::CELL_BEGIN_OF_MULTICOLUMN:
			bottom = cell_info[row - 1][column].bottom_line;
			break;
		case LyXTabular::CELL_NORMAL:
			bottom = row_info[row - 1].bottom_line;
		}
	}
	for (int column = 0; column < columns_ && top; ++column) {
		switch (cell_info[row][column].multicolumn) {
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


int LyXTabular::getAdditionalWidth(int cell) const
{
	// internally already set in setWidthOfCell
	// used to get it back in text.C
	int const col = right_column_of_cell(cell);
	int const row = row_of_cell(cell);
	if (col < columns_ - 1 && rightLine(cell) &&
		leftLine(cell_info[row][col+1].cellno)) // column_info[col+1].left_line)
	{
		return WIDTH_OF_LINE;
	}
	return 0;
}


// returns the maximum over all rows
int LyXTabular::getWidthOfColumn(int cell) const
{
	int const column1 = column_of_cell(cell);
	int const column2 = right_column_of_cell(cell);
	int result = 0;
	for (int i = column1; i <= column2; ++i)
		result += column_info[i].width_of_column;
	return result;
}


int LyXTabular::getWidthOfTabular() const
{
	return width_of_tabular;
}


// returns true if a complete update is necessary, otherwise false
bool LyXTabular::setWidthOfMulticolCell(int cell, int new_width)
{
	if (!isMultiColumn(cell))
		return false;

	int const row = row_of_cell(cell);
	int const column1 = column_of_cell(cell);
	int const column2 = right_column_of_cell(cell);
	int const old_val = cell_info[row][column2].width_of_cell;

	// first set columns to 0 so we can calculate the right width
	for (int i = column1; i <= column2; ++i) {
		cell_info[row][i].width_of_cell = 0;
	}
	// set the width to MAX_WIDTH until width > 0
	int width = new_width + 2 * WIDTH_OF_LINE;
	int i = column1;
	for (; i < column2 && width > column_info[i].width_of_column; ++i) {
		cell_info[row][i].width_of_cell = column_info[i].width_of_column;
		width -= column_info[i].width_of_column;
	}
	if (width > 0) {
		cell_info[row][i].width_of_cell = width;
	}
	if (old_val != cell_info[row][column2].width_of_cell) {
		// in this case we have to recalculate all multicolumn cells which
		// have this column as one of theirs but not as last one
		calculate_width_of_column_NMC(i);
		recalculateMulticolumnsOfColumn(i);
		calculate_width_of_column(i);
	}
	return true;
}


void LyXTabular::recalculateMulticolumnsOfColumn(int column)
{
	// the last column does not have to be recalculated because all
	// multicolumns will have here there last multicolumn cell which
	// always will have the whole rest of the width of the cell.
	if (column > (columns_ - 2))
		return;
	for(int row = 0; row < rows_; ++row) {
		int mc = cell_info[row][column].multicolumn;
		int nmc = cell_info[row][column+1].multicolumn;
		// we only have to update multicolumns which do not have this
		// column as their last column!
		if (mc == CELL_BEGIN_OF_MULTICOLUMN ||
			  (mc == CELL_PART_OF_MULTICOLUMN &&
			   nmc == CELL_PART_OF_MULTICOLUMN))
		{
			int const cellno = cell_info[row][column].cellno;
			setWidthOfMulticolCell(cellno,
					       getWidthOfCell(cellno) - 2 * WIDTH_OF_LINE);
		}
	}
}


// returns 1 if a complete update is necessary, otherwise 0
void LyXTabular::setWidthOfCell(int cell, int new_width)
{
	int const row = row_of_cell(cell);
	int const column1 = column_of_cell(cell);
	bool tmp = false;
	int width = 0;
	int add_width = 0;

	if (rightLine(cell_info[row][column1].cellno, true) &&
		column1 < columns_ - 1 &&
		leftLine(cell_info[row][column1+1].cellno, true))
	{
		add_width = WIDTH_OF_LINE;
	}

	if (getWidthOfCell(cell) == new_width + 2 * WIDTH_OF_LINE + add_width)
		return;

	if (isMultiColumn(cell, true)) {
		tmp = setWidthOfMulticolCell(cell, new_width);
	} else {
		width = new_width + 2 * WIDTH_OF_LINE + add_width;
		cell_info[row][column1].width_of_cell = width;
		tmp = calculate_width_of_column_NMC(column1);
		if (tmp)
			recalculateMulticolumnsOfColumn(column1);
	}
	if (tmp) {
		for (int i = 0; i < columns_; ++i)
			calculate_width_of_column(i);
		calculate_width_of_tabular();
	}
}


void LyXTabular::setAlignment(int cell, LyXAlignment align, bool onlycolumn)
{
	if (!isMultiColumn(cell) || onlycolumn)
		column_info[column_of_cell(cell)].alignment = align;
	if (!onlycolumn)
		cellinfo_of_cell(cell).alignment = align;
}


void LyXTabular::setVAlignment(int cell, VAlignment align, bool onlycolumn)
{
	if (!isMultiColumn(cell) || onlycolumn)
		column_info[column_of_cell(cell)].valignment = align;
	if (!onlycolumn)
		cellinfo_of_cell(cell).valignment = align;
}


void LyXTabular::setColumnPWidth(int cell, LyXLength const & width)
{
	int const j = column_of_cell(cell);

	column_info[j].p_width = width;
	for (int i = 0; i < rows_; ++i) {
		int const cell = getCellNumber(i, j);
		// because of multicolumns
		getCellInset(cell).setAutoBreakRows(!getPWidth(cell).zero());
	}
}


bool LyXTabular::setMColumnPWidth(int cell, LyXLength const & width)
{
	bool const flag = !width.zero();

	cellinfo_of_cell(cell).p_width = width;
	if (isMultiColumn(cell)) {
		getCellInset(cell).setAutoBreakRows(flag);
		return true;
	}
	return false;
}


void LyXTabular::setAlignSpecial(int cell, string const & special,
				 LyXTabular::Feature what)
{
	if (what == SET_SPECIAL_MULTI)
		cellinfo_of_cell(cell).align_special = special;
	else
		column_info[column_of_cell(cell)].align_special = special;
}


void LyXTabular::setAllLines(int cell, bool line)
{
	setTopLine(cell, line);
	setBottomLine(cell, line);
	setRightLine(cell, line);
	setLeftLine(cell, line);
}


void LyXTabular::setTopLine(int cell, bool line, bool onlycolumn)
{
	int const row = row_of_cell(cell);
	if (onlycolumn || !isMultiColumn(cell))
		row_info[row].top_line = line;
	else
		cellinfo_of_cell(cell).top_line = line;
}


void LyXTabular::setBottomLine(int cell, bool line, bool onlycolumn)
{
	if (onlycolumn || !isMultiColumn(cell))
		row_info[row_of_cell(cell)].bottom_line = line;
	else
		cellinfo_of_cell(cell).bottom_line = line;
}


void LyXTabular::setLeftLine(int cell, bool line, bool onlycolumn)
{
	if (onlycolumn || !isMultiColumn(cell))
		column_info[column_of_cell(cell)].left_line = line;
	else
		cellinfo_of_cell(cell).left_line = line;
}


void LyXTabular::setRightLine(int cell, bool line, bool onlycolumn)
{
	if (onlycolumn || !isMultiColumn(cell))
		column_info[right_column_of_cell(cell)].right_line = line;
	else
		cellinfo_of_cell(cell).right_line = line;
}


LyXAlignment LyXTabular::getAlignment(int cell, bool onlycolumn) const
{
	if (!onlycolumn && isMultiColumn(cell))
		return cellinfo_of_cell(cell).alignment;
	return column_info[column_of_cell(cell)].alignment;
}


LyXTabular::VAlignment
LyXTabular::getVAlignment(int cell, bool onlycolumn) const
{
	if (!onlycolumn && isMultiColumn(cell))
		return cellinfo_of_cell(cell).valignment;
	return column_info[column_of_cell(cell)].valignment;
}


LyXLength const LyXTabular::getPWidth(int cell) const
{
	if (isMultiColumn(cell))
		return cellinfo_of_cell(cell).p_width;
	return column_info[column_of_cell(cell)].p_width;
}


LyXLength const LyXTabular::getColumnPWidth(int cell) const
{
	return column_info[column_of_cell(cell)].p_width;
}


LyXLength const LyXTabular::getMColumnPWidth(int cell) const
{
	if (isMultiColumn(cell))
		return cellinfo_of_cell(cell).p_width;
	return LyXLength();
}


string const LyXTabular::getAlignSpecial(int cell, int what) const
{
	if (what == SET_SPECIAL_MULTI)
		return cellinfo_of_cell(cell).align_special;
	return column_info[column_of_cell(cell)].align_special;
}


int LyXTabular::getWidthOfCell(int cell) const
{
	int const row = row_of_cell(cell);
	int const column1 = column_of_cell(cell);
	int const column2 = right_column_of_cell(cell);
	int result = 0;
	for (int i = column1; i <= column2; ++i)
		result += cell_info[row][i].width_of_cell;
	return result;
}


int LyXTabular::getBeginningOfTextInCell(int cell) const
{
	int x = 0;

	switch (getAlignment(cell)) {
	case LYX_ALIGN_CENTER:
		x += (getWidthOfColumn(cell) - getWidthOfCell(cell)) / 2;
		break;
	case LYX_ALIGN_RIGHT:
		x += getWidthOfColumn(cell) - getWidthOfCell(cell);
		// + getAdditionalWidth(cell);
		break;
	default:
		// LYX_ALIGN_LEFT: nothing :-)
		break;
	}

	// the LaTeX Way :-(
	x += WIDTH_OF_LINE;
	return x;
}


bool LyXTabular::isFirstCellInRow(int cell) const
{
	return column_of_cell(cell) == 0;
}


int LyXTabular::getFirstCellInRow(int row) const
{
	if (row > rows_ - 1)
		row = rows_ - 1;
	return cell_info[row][0].cellno;
}


bool LyXTabular::isLastCellInRow(int cell) const
{
	return right_column_of_cell(cell) == columns_ - 1;
}


int LyXTabular::getLastCellInRow(int row) const
{
	if (row > rows_ - 1)
		row = rows_ - 1;
	return cell_info[row][columns_-1].cellno;
}


void LyXTabular::calculate_width_of_column(int column)
{
	int maximum = 0;
	for (int i = 0; i < rows_; ++i)
		maximum = max(cell_info[i][column].width_of_cell, maximum);
	column_info[column].width_of_column = maximum;
}


//
// Calculate the columns regarding ONLY the normal cells and if this
// column is inside a multicolumn cell then use it only if its the last
// column of this multicolumn cell as this gives an added width to the
// column, all the rest should be adapted!
//
bool LyXTabular::calculate_width_of_column_NMC(int column)
{
	int const old_column_width = column_info[column].width_of_column;
	int max = 0;
	for (int i = 0; i < rows_; ++i) {
		int cell = getCellNumber(i, column);
		bool ismulti = isMultiColumn(cell, true);
		if ((!ismulti || column == right_column_of_cell(cell)) &&
			cell_info[i][column].width_of_cell > max)
		{
			max = cell_info[i][column].width_of_cell;
		}
	}
	column_info[column].width_of_column = max;
	return column_info[column].width_of_column != old_column_width;
}


void LyXTabular::calculate_width_of_tabular()
{
	width_of_tabular = 0;
	for (int i = 0; i < columns_; ++i)
		width_of_tabular += column_info[i].width_of_column;
}


int LyXTabular::row_of_cell(int cell) const
{
	if (cell >= numberofcells)
		return rows_ - 1;
	if (cell < 0)
		return 0;
	return rowofcell[cell];
}


int LyXTabular::column_of_cell(int cell) const
{
	if (cell >= numberofcells)
		return columns_ - 1;
	if (cell < 0)
		return 0;
	return columnofcell[cell];
}


int LyXTabular::right_column_of_cell(int cell) const
{
	int const row = row_of_cell(cell);
	int column = column_of_cell(cell);
	while (column < columns_ - 1 &&
		   cell_info[row][column + 1].multicolumn == LyXTabular::CELL_PART_OF_MULTICOLUMN)
		++column;
	return column;
}


void LyXTabular::write(Buffer const & buf, ostream & os) const
{
	// header line
	os << "<lyxtabular"
	   << write_attribute("version", 3)
	   << write_attribute("rows", rows_)
	   << write_attribute("columns", columns_)
	   << ">\n";
	// global longtable options
	os << "<features"
	   << write_attribute("rotate", rotate)
	   << write_attribute("islongtable", is_long_tabular)
	   << write_attribute("firstHeadTopDL", endfirsthead.topDL)
	   << write_attribute("firstHeadBottomDL", endfirsthead.bottomDL)
	   << write_attribute("firstHeadEmpty", endfirsthead.empty)
	   << write_attribute("headTopDL", endhead.topDL)
	   << write_attribute("headBottomDL", endhead.bottomDL)
	   << write_attribute("footTopDL", endfoot.topDL)
	   << write_attribute("footBottomDL", endfoot.bottomDL)
	   << write_attribute("lastFootTopDL", endlastfoot.topDL)
	   << write_attribute("lastFootBottomDL", endlastfoot.bottomDL)
	   << write_attribute("lastFootEmpty", endlastfoot.empty)
	   << ">\n";
	for (int j = 0; j < columns_; ++j) {
		os << "<column"
		   << write_attribute("alignment", column_info[j].alignment)
		   << write_attribute("valignment", column_info[j].valignment)
		   << write_attribute("leftline", column_info[j].left_line)
		   << write_attribute("rightline", column_info[j].right_line)
		   << write_attribute("width", column_info[j].p_width.asString())
		   << write_attribute("special", column_info[j].align_special)
		   << ">\n";
	}
	for (int i = 0; i < rows_; ++i) {
		os << "<row"
		   << write_attribute("topline", row_info[i].top_line)
		   << write_attribute("bottomline", row_info[i].bottom_line)
		   << write_attribute("endhead", row_info[i].endhead)
		   << write_attribute("endfirsthead", row_info[i].endfirsthead)
		   << write_attribute("endfoot", row_info[i].endfoot)
		   << write_attribute("endlastfoot", row_info[i].endlastfoot)
		   << write_attribute("newpage", row_info[i].newpage)
		   << ">\n";
		for (int j = 0; j < columns_; ++j) {
			os << "<cell"
			   << write_attribute("multicolumn", cell_info[i][j].multicolumn)
			   << write_attribute("alignment", cell_info[i][j].alignment)
			   << write_attribute("valignment", cell_info[i][j].valignment)
			   << write_attribute("topline", cell_info[i][j].top_line)
			   << write_attribute("bottomline", cell_info[i][j].bottom_line)
			   << write_attribute("leftline", cell_info[i][j].left_line)
			   << write_attribute("rightline", cell_info[i][j].right_line)
			   << write_attribute("rotate", cell_info[i][j].rotate)
			   << write_attribute("usebox", cell_info[i][j].usebox)
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


void LyXTabular::setHeaderFooterRows(int hr, int fhr, int fr, int lfr)
{
	// set header info
	while (hr > 0) {
		row_info[--hr].endhead = true;
	}
	// set firstheader info
	if (fhr && fhr < rows_) {
		if (row_info[fhr].endhead) {
			while (fhr > 0) {
				row_info[--fhr].endfirsthead = true;
				row_info[fhr].endhead = false;
			}
		} else if (row_info[fhr - 1].endhead) {
			endfirsthead.empty = true;
		} else {
			while (fhr > 0 && !row_info[--fhr].endhead) {
				row_info[fhr].endfirsthead = true;
			}
		}
	}
	// set footer info
	if (fr && fr < rows_) {
		if (row_info[fr].endhead && row_info[fr-1].endhead) {
			while (fr > 0 && !row_info[--fr].endhead) {
				row_info[fr].endfoot = true;
				row_info[fr].endhead = false;
			}
		} else if (row_info[fr].endfirsthead && row_info[fr-1].endfirsthead) {
			while (fr > 0 && !row_info[--fr].endfirsthead) {
				row_info[fr].endfoot = true;
				row_info[fr].endfirsthead = false;
			}
		} else if (!row_info[fr - 1].endhead && !row_info[fr - 1].endfirsthead) {
			while (fr > 0 && !row_info[--fr].endhead &&
				  !row_info[fr].endfirsthead)
			{
				row_info[fr].endfoot = true;
			}
		}
	}
	// set lastfooter info
	if (lfr && lfr < rows_) {
		if (row_info[lfr].endhead && row_info[lfr - 1].endhead) {
			while (lfr > 0 && !row_info[--lfr].endhead) {
				row_info[lfr].endlastfoot = true;
				row_info[lfr].endhead = false;
			}
		} else if (row_info[lfr].endfirsthead &&
				   row_info[lfr - 1].endfirsthead)
		{
			while (lfr > 0 && !row_info[--lfr].endfirsthead) {
				row_info[lfr].endlastfoot = true;
				row_info[lfr].endfirsthead = false;
			}
		} else if (row_info[lfr].endfoot
			   && row_info[lfr - 1].endfoot) {
			while (lfr > 0 && !row_info[--lfr].endfoot) {
				row_info[lfr].endlastfoot = true;
				row_info[lfr].endfoot = false;
			}
		} else if (!row_info[fr - 1].endhead
			   && !row_info[fr - 1].endfirsthead &&
				   !row_info[fr - 1].endfoot)
		{
			while (lfr > 0 &&
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


void LyXTabular::read(Buffer const & buf, LyXLex & lex)
{
	string line;
	istream & is = lex.getStream();

	l_getline(is, line);
	if (!prefixIs(line, "<lyxtabular ")
		&& !prefixIs(line, "<LyXTabular ")) {
		Assert(false);
		return;
	}

	int version;
	if (!getTokenValue(line, "version", version))
		return;
	Assert(version >= 2);

	int rows_arg;
	if (!getTokenValue(line, "rows", rows_arg))
		return;
	int columns_arg;
	if (!getTokenValue(line, "columns", columns_arg))
		return;
	init(buf.params, rows_arg, columns_arg);
	l_getline(is, line);
	if (!prefixIs(line, "<features")) {
		lyxerr << "Wrong tabular format (expected <features ...> got"
		       << line << ')' << endl;
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
			lyxerr << "Wrong tabular format (expected <column ...> got"
			       << line << ')' << endl;
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
			lyxerr << "Wrong tabular format (expected <row ...> got"
			       << line << ')' << endl;
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
				lyxerr << "Wrong tabular format (expected <cell ...> got"
				       << line << ')' << endl;
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
				lyxerr << "Wrong tabular format (expected </cell> got"
				       << line << ')' << endl;
				return;
			}
		}
		l_getline(is, line);
		if (!prefixIs(line, "</row>")) {
			lyxerr << "Wrong tabular format (expected </row> got"
			       << line << ')' << endl;
			return;
		}
	}
	while (!prefixIs(line, "</lyxtabular>")) {
		l_getline(is, line);
	}
	set_row_column_number_info();
}


bool LyXTabular::isMultiColumn(int cell, bool real) const
{
	return (!real || column_of_cell(cell) != right_column_of_cell(cell)) &&
			cellinfo_of_cell(cell).multicolumn != LyXTabular::CELL_NORMAL;
}


LyXTabular::cellstruct & LyXTabular::cellinfo_of_cell(int cell) const
{
	return cell_info[row_of_cell(cell)][column_of_cell(cell)];
}


void LyXTabular::setMultiColumn(Buffer * buffer, int cell, int number)
{
	cellstruct & cs = cellinfo_of_cell(cell);
	cs.multicolumn = CELL_BEGIN_OF_MULTICOLUMN;
	cs.alignment = column_info[column_of_cell(cell)].alignment;
	cs.top_line = row_info[row_of_cell(cell)].top_line;
	cs.bottom_line = row_info[row_of_cell(cell)].bottom_line;
	cs.right_line = column_info[column_of_cell(cell+number-1)].right_line;
	for (int i = 1; i < number; ++i) {
		cellstruct & cs1 = cellinfo_of_cell(cell + i);
		cs1.multicolumn = CELL_PART_OF_MULTICOLUMN;
		cs.inset.appendParagraphs(buffer, cs1.inset.paragraphs);
		cs1.inset.clear(false);
	}
	set_row_column_number_info();
}


int LyXTabular::cells_in_multicolumn(int cell) const
{
	int const row = row_of_cell(cell);
	int column = column_of_cell(cell);
	int result = 1;
	++column;
	while (column < columns_ &&
		   cell_info[row][column].multicolumn == CELL_PART_OF_MULTICOLUMN)
	{
		++result;
		++column;
	}
	return result;
}


int LyXTabular::unsetMultiColumn(int cell)
{
	int const row = row_of_cell(cell);
	int column = column_of_cell(cell);

	int result = 0;

	if (cell_info[row][column].multicolumn == CELL_BEGIN_OF_MULTICOLUMN) {
		cell_info[row][column].multicolumn = CELL_NORMAL;
		++column;
		while (column < columns_ &&
			   cell_info[row][column].multicolumn == CELL_PART_OF_MULTICOLUMN)
		{
			cell_info[row][column].multicolumn = CELL_NORMAL;
			++column;
			++result;
		}
	}
	set_row_column_number_info();
	return result;
}


void LyXTabular::setLongTabular(bool what)
{
	is_long_tabular = what;
}


bool LyXTabular::isLongTabular() const
{
	return is_long_tabular;
}


void LyXTabular::setRotateTabular(bool flag)
{
	rotate = flag;
}


bool LyXTabular::getRotateTabular() const
{
	return rotate;
}


void LyXTabular::setRotateCell(int cell, bool flag)
{
	cellinfo_of_cell(cell).rotate = flag;
}


bool LyXTabular::getRotateCell(int cell) const
{
	return cellinfo_of_cell(cell).rotate;
}


bool LyXTabular::needRotating() const
{
	if (rotate)
		return true;
	for (int i = 0; i < rows_; ++i)
		for (int j = 0; j < columns_; ++j)
			if (cell_info[i][j].rotate)
				return true;
	return false;
}


bool LyXTabular::isLastCell(int cell) const
{
	if (cell + 1 < numberofcells)
		return false;
	return true;
}


int LyXTabular::getCellAbove(int cell) const
{
	if (row_of_cell(cell) > 0)
		return cell_info[row_of_cell(cell)-1][column_of_cell(cell)].cellno;
	return cell;
}


int LyXTabular::getCellBelow(int cell) const
{
	if (row_of_cell(cell) + 1 < rows_)
		return cell_info[row_of_cell(cell)+1][column_of_cell(cell)].cellno;
	return cell;
}


int LyXTabular::getLastCellAbove(int cell) const
{
	if (row_of_cell(cell) <= 0)
		return cell;
	if (!isMultiColumn(cell))
		return getCellAbove(cell);
	return cell_info[row_of_cell(cell) - 1][right_column_of_cell(cell)].cellno;
}


int LyXTabular::getLastCellBelow(int cell) const
{
	if (row_of_cell(cell) + 1 >= rows_)
		return cell;
	if (!isMultiColumn(cell))
		return getCellBelow(cell);
	return cell_info[row_of_cell(cell) + 1][right_column_of_cell(cell)].cellno;
}


int LyXTabular::getCellNumber(int row, int column) const
{
	Assert(column >= 0 || column < columns_ || row >= 0 || row < rows_);
	return cell_info[row][column].cellno;
}


void LyXTabular::setUsebox(int cell, BoxType type)
{
	cellinfo_of_cell(cell).usebox = type;
}


LyXTabular::BoxType LyXTabular::getUsebox(int cell) const
{
	if (column_info[column_of_cell(cell)].p_width.zero() &&
		!(isMultiColumn(cell) && !cellinfo_of_cell(cell).p_width.zero()))
		return BOX_NONE;
	if (cellinfo_of_cell(cell).usebox > 1)
		return cellinfo_of_cell(cell).usebox;
	return useParbox(cell);
}


///
//  This are functions used for the longtable support
///
void LyXTabular::setLTHead(int row, bool flag, ltType const & hd, bool first)
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


bool LyXTabular::getRowOfLTHead(int row, ltType & hd) const
{
	hd = endhead;
	hd.set = haveLTHead();
	return row_info[row].endhead;
}


bool LyXTabular::getRowOfLTFirstHead(int row, ltType & hd) const
{
	hd = endfirsthead;
	hd.set = haveLTFirstHead();
	return row_info[row].endfirsthead;
}


void LyXTabular::setLTFoot(int row, bool flag, ltType const & fd, bool last)
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


bool LyXTabular::getRowOfLTFoot(int row, ltType & fd) const
{
	fd = endfoot;
	fd.set = haveLTFoot();
	return row_info[row].endfoot;
}


bool LyXTabular::getRowOfLTLastFoot(int row, ltType & fd) const
{
	fd = endlastfoot;
	fd.set = haveLTLastFoot();
	return row_info[row].endlastfoot;
}


void LyXTabular::setLTNewPage(int row, bool what)
{
	row_info[row].newpage = what;
}


bool LyXTabular::getLTNewPage(int row) const
{
	return row_info[row].newpage;
}


bool LyXTabular::haveLTHead() const
{
	for (int i = 0; i < rows_; ++i)
		if (row_info[i].endhead)
			return true;
	return false;
}


bool LyXTabular::haveLTFirstHead() const
{
	if (endfirsthead.empty)
		return false;
	for (int i = 0; i < rows_; ++i)
		if (row_info[i].endfirsthead)
			return true;
	return false;
}


bool LyXTabular::haveLTFoot() const
{
	for (int i = 0; i < rows_; ++i)
		if (row_info[i].endfoot)
			return true;
	return false;
}


bool LyXTabular::haveLTLastFoot() const
{
	if (endlastfoot.empty)
		return false;
	for (int i = 0; i < rows_; ++i)
		if (row_info[i].endlastfoot)
			return true;
	return false;
}


// end longtable support functions

void LyXTabular::setAscentOfRow(int row, int height)
{
	if (row >= rows_ || row_info[row].ascent_of_row == height)
		return;
	row_info[row].ascent_of_row = height;
}


void LyXTabular::setDescentOfRow(int row, int height)
{
	if (row >= rows_ || row_info[row].descent_of_row == height)
		return;
	row_info[row].descent_of_row = height;
}


int LyXTabular::getAscentOfRow(int row) const
{
	if (row >= rows_)
		return 0;
	return row_info[row].ascent_of_row;
}


int LyXTabular::getDescentOfRow(int row) const
{
	if (row >= rows_)
		return 0;
	return row_info[row].descent_of_row;
}


int LyXTabular::getHeightOfTabular() const
{
	int height = 0;
	for (int row = 0; row < rows_; ++row)
		height += getAscentOfRow(row) + getDescentOfRow(row) +
			getAdditionalHeight(row);
	return height;
}


bool LyXTabular::isPartOfMultiColumn(int row, int column) const
{
	if (row >= rows_ || column >= columns_)
		return false;
	return cell_info[row][column].multicolumn == CELL_PART_OF_MULTICOLUMN;
}


int LyXTabular::TeXTopHLine(ostream & os, int row) const
{
	if (row < 0 || row >= rows_)
		return 0;

	int const fcell = getFirstCellInRow(row);
	int const n = numberOfCellsInRow(fcell) + fcell;
	int tmp = 0;

	for (int i = fcell; i < n; ++i) {
		if (topLine(i))
			++tmp;
	}
	if (tmp == n - fcell) {
		os << "\\hline ";
	} else if (tmp) {
		for (int i = fcell; i < n; ++i) {
			if (topLine(i)) {
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
	if (row < 0 || row >= rows_)
		return 0;

	int const fcell = getFirstCellInRow(row);
	int const n = numberOfCellsInRow(fcell) + fcell;
	int tmp = 0;

	for (int i = fcell; i < n; ++i) {
		if (bottomLine(i))
			++tmp;
	}
	if (tmp == n - fcell) {
		os << "\\hline";
	} else if (tmp) {
		for (int i = fcell; i < n; ++i) {
			if (bottomLine(i)) {
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

	if (getRotateCell(cell)) {
		os << "\\begin{sideways}\n";
		++ret;
	}
	if (isMultiColumn(cell)) {
		os << "\\multicolumn{" << cells_in_multicolumn(cell) << "}{";
		if (!cellinfo_of_cell(cell).align_special.empty()) {
			os << cellinfo_of_cell(cell).align_special << "}{";
		} else {
			if (leftLine(cell) &&
				(isFirstCellInRow(cell) ||
				 (!isMultiColumn(cell - 1) && !leftLine(cell, true) &&
				  !rightLine(cell - 1, true))))
			{
				os << '|';
			}
			if (!getPWidth(cell).zero()) {
				switch (getVAlignment(cell)) {
				case LYX_VALIGN_TOP:
					os << 'p';
					break;
				case LYX_VALIGN_MIDDLE:
					os << 'm';
					break;
				case LYX_VALIGN_BOTTOM:
					os << 'b';
					break;
				}
				os << '{'
				   << getPWidth(cell).asLatexString()
				   << '}';
			} else {
				switch (getAlignment(cell)) {
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
			if (rightLine(cell))
				os << '|';
			if (((cell + 1) < numberofcells) && !isFirstCellInRow(cell+1) &&
				leftLine(cell+1))
				os << '|';
			os << "}{";
		}
	}
	if (getUsebox(cell) == BOX_PARBOX) {
		os << "\\parbox[";
		switch (getVAlignment(cell)) {
		case LYX_VALIGN_TOP:
			os << 't';
			break;
		case LYX_VALIGN_MIDDLE:
			os << 'c';
			break;
		case LYX_VALIGN_BOTTOM:
			os << 'b';
			break;
		}
		os << "]{" << getPWidth(cell).asLatexString() << "}{";
	} else if (getUsebox(cell) == BOX_MINIPAGE) {
		os << "\\begin{minipage}[";
		switch (getVAlignment(cell)) {
		case LYX_VALIGN_TOP:
			os << 't';
			break;
		case LYX_VALIGN_MIDDLE:
			os << 'm';
			break;
		case LYX_VALIGN_BOTTOM:
			os << 'b';
			break;
		}
		os << "]{" << getPWidth(cell).asLatexString() << "}\n";
		++ret;
	}
	return ret;
}


int LyXTabular::TeXCellPostamble(ostream & os, int cell) const
{
	int ret = 0;

	// usual cells
	if (getUsebox(cell) == BOX_PARBOX)
		os << '}';
	else if (getUsebox(cell) == BOX_MINIPAGE) {
		os << "%\n\\end{minipage}";
		ret += 2;
	}
	if (isMultiColumn(cell)) {
		os << '}';
	}
	if (getRotateCell(cell)) {
		os << "%\n\\end{sideways}";
		++ret;
	}
	return ret;
}


int LyXTabular::TeXLongtableHeaderFooter(ostream & os, Buffer const & buf,
					 LatexRunParams const & runparams) const
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
				ret += TeXRow(os, i, buf, runparams);
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
				ret += TeXRow(os, i, buf, runparams);
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
				ret += TeXRow(os, i, buf, runparams);
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
				ret += TeXRow(os, i, buf, runparams);
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


int LyXTabular::TeXRow(ostream & os, int const i, Buffer const & buf,
		       LatexRunParams const & runparams) const
{
	int ret = 0;
	int cell = getCellNumber(i, 0);

	ret += TeXTopHLine(os, i);
	for (int j = 0; j < columns_; ++j) {
		if (isPartOfMultiColumn(i, j))
			continue;
		ret += TeXCellPreamble(os, cell);
		InsetText & inset = getCellInset(cell);

		bool rtl = inset.paragraphs.begin()->isRightToLeftPar(buf.params) &&
			!inset.paragraphs.begin()->empty() && getPWidth(cell).zero();

		if (rtl)
			os << "\\R{";
		ret += inset.latex(buf, os, runparams);
		if (rtl)
			os << '}';

		ret += TeXCellPostamble(os, cell);
		if (!isLastCellInRow(cell)) { // not last cell in row
			os << "&\n";
			++ret;
		}
		++cell;
	}
	os << "\\tabularnewline\n";
	++ret;
	ret += TeXBottomHLine(os, i);
	return ret;
}


int LyXTabular::latex(Buffer const & buf, ostream & os,
		      LatexRunParams const & runparams) const
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
				switch (column_info[i].alignment) {
				case LYX_ALIGN_LEFT:
					os << ">{\\raggedright}";
					break;
				case LYX_ALIGN_RIGHT:
					os << ">{\\raggedleft}";
					break;
				case LYX_ALIGN_CENTER:
					os << ">{\\centering}";
					break;
				case LYX_ALIGN_NONE:
				case LYX_ALIGN_BLOCK:
				case LYX_ALIGN_LAYOUT:
				case LYX_ALIGN_SPECIAL:
					break;
				}

				switch (column_info[i].valignment) {
				case LYX_VALIGN_TOP:
					os << 'p';
					break;
				case LYX_VALIGN_MIDDLE:
					os << 'm';
					break;
				case LYX_VALIGN_BOTTOM:
					os << 'b';
					break;
			}
				os << '{'
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

	ret += TeXLongtableHeaderFooter(os, buf, runparams);

	//+---------------------------------------------------------------------
	//+                      the single row and columns (cells)            +
	//+---------------------------------------------------------------------

	for (int i = 0; i < rows_; ++i) {
		if (isValidRow(i)) {
			ret += TeXRow(os, i, buf, runparams);
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


int LyXTabular::linuxdoc(Buffer const & buf, ostream & os) const
{
	os << "<tabular ca=\"";
	for (int i = 0; i < columns_; ++i) {
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
	os << "\">\n";
	int cell = 0;
	int ret = 0;
	for (int i = 0; i < rows_; ++i) {
		for (int j = 0; j < columns_; ++j) {
			if (isPartOfMultiColumn(i, j))
				continue;
			InsetText & inset = getCellInset(cell);

			ret += inset.linuxdoc(buf, os);

			if (isLastCellInRow(cell)) {
				os << "@\n";
				++ret;
			} else {
				os << "|";
			}
			++cell;
		}
	}
	os << "</tabular>\n";
	return ret;
}


int LyXTabular::docbookRow(Buffer const & buf, ostream & os, int row) const
{
	int ret = 0;
	int cell = getFirstCellInRow(row);

	os << "<row>\n";
	for (int j = 0; j < columns_; ++j) {
		if (isPartOfMultiColumn(row, j))
			continue;

		os << "<entry align=\"";
		switch (getAlignment(cell)) {
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
		switch (getVAlignment(cell)) {
		case LYX_VALIGN_TOP:
			os << "top";
			break;
		case LYX_VALIGN_BOTTOM:
			os << "bottom";
			break;
		case LYX_VALIGN_MIDDLE:
			os << "middle";
		}
		os << '"';

		if (isMultiColumn(cell)) {
			os << " namest=\"col" << j << "\" ";
			os << "nameend=\"col" << j + cells_in_multicolumn(cell) - 1<< '"';
		}

		os << '>';
		ret += getCellInset(cell).docbook(buf, os, true);
		os << "</entry>\n";
		++cell;
	}
	os << "</row>\n";
	return ret;
}


int LyXTabular::docbook(Buffer const & buf, ostream & os,
			bool /*mixcont*/) const
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

	// output header info
	if (haveLTHead() || haveLTFirstHead()) {
		os << "<thead>\n";
		++ret;
		for (int i = 0; i < rows_; ++i) {
			if (row_info[i].endhead || row_info[i].endfirsthead) {
				ret += docbookRow(buf, os, i);
			}
		}
		os << "</thead>\n";
		++ret;
	}
	// output footer info
	if (haveLTFoot() || haveLTLastFoot()) {
		os << "<tfoot>\n";
		++ret;
		for (int i = 0; i < rows_; ++i) {
			if (row_info[i].endfoot || row_info[i].endlastfoot) {
				ret += docbookRow(buf, os, i);
			}
		}
		os << "</tfoot>\n";
		++ret;
	}

	//+---------------------------------------------------------------------
	//+                      the single row and columns (cells)            +
	//+---------------------------------------------------------------------

	os << "<tbody>\n";
	++ret;
	for (int i = 0; i < rows_; ++i) {
		if (isValidRow(i)) {
			ret += docbookRow(buf, os, i);
		}
	}
	os << "</tbody>\n";
	++ret;
	//+---------------------------------------------------------------------
	//+                      the closing of the tabular                    +
	//+---------------------------------------------------------------------

	os << "</tgroup>";
	++ret;

	return ret;
}


int LyXTabular::asciiTopHLine(ostream & os, int row,
			      vector<unsigned int> const & clen) const
{
	int const fcell = getFirstCellInRow(row);
	int const n = numberOfCellsInRow(fcell) + fcell;
	int tmp = 0;

	for (int i = fcell; i < n; ++i) {
		if (topLine(i)) {
			++tmp;
			break;
		}
	}
	if (!tmp)
		return 0;

	unsigned char ch;
	for (int i = fcell; i < n; ++i) {
		if (topLine(i)) {
			if (leftLine(i))
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
		while (isPartOfMultiColumn(row, ++column))
			len += clen[column] + 4;
		os << string(len, ch);
		if (topLine(i)) {
			if (rightLine(i))
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
	int const fcell = getFirstCellInRow(row);
	int const n = numberOfCellsInRow(fcell) + fcell;
	int tmp = 0;

	for (int i = fcell; i < n; ++i) {
		if (bottomLine(i)) {
			++tmp;
			break;
		}
	}
	if (!tmp)
		return 0;

	unsigned char ch;
	for (int i = fcell; i < n; ++i) {
		if (bottomLine(i)) {
			if (leftLine(i))
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
		while (isPartOfMultiColumn(row, ++column))
			len += clen[column] + 4;
		os << string(len, ch);
		if (bottomLine(i)) {
			if (rightLine(i))
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


int LyXTabular::asciiPrintCell(Buffer const & buf, ostream & os,
			       int cell, int row, int column,
			       vector<unsigned int> const & clen,
			       bool onlydata) const
{
	ostringstream sstr;
	int ret = getCellInset(cell).ascii(buf, sstr, 0);

	if (onlydata) {
		os << sstr.str();
		return ret;
	}

	if (leftLine(cell))
		os << "| ";
	else
		os << "  ";

	unsigned int len1 = sstr.str().length();
	unsigned int len2 = clen[column];
	while (isPartOfMultiColumn(row, ++column))
		len2 += clen[column] + 4;
	len2 -= len1;

	switch (getAlignment(cell)) {
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

	os << string(len1, ' ') << sstr.str() << string(len2, ' ');

	if (rightLine(cell))
		os << " |";
	else
		os << "  ";

	return ret;
}


int LyXTabular::ascii(Buffer const & buf, ostream & os, int const depth,
					  bool onlydata, unsigned char delim) const
{
	int ret = 0;

	// first calculate the width of the single columns
	vector<unsigned int> clen(columns_);

	if (!onlydata) {
		// first all non (real) multicolumn cells!
		for (int j = 0; j < columns_; ++j) {
			clen[j] = 0;
			for (int i = 0; i < rows_; ++i) {
				int cell = getCellNumber(i, j);
				if (isMultiColumn(cell, true))
					continue;
				ostringstream sstr;
				getCellInset(cell).ascii(buf, sstr, 0);
				if (clen[j] < sstr.str().length())
					clen[j] = sstr.str().length();
			}
		}
		// then all (real) multicolumn cells!
		for (int j = 0; j < columns_; ++j) {
			for (int i = 0; i < rows_; ++i) {
				int cell = getCellNumber(i, j);
				if (!isMultiColumn(cell, true) || isPartOfMultiColumn(i, j))
					continue;
				ostringstream sstr;
				getCellInset(cell).ascii(buf, sstr, 0);
				int len = int(sstr.str().length());
				int const n = cells_in_multicolumn(cell);
				for (int k = j; len > 0 && k < j + n - 1; ++k)
					len -= clen[k];
				if (len > int(clen[j + n - 1]))
					clen[j + n - 1] = len;
			}
		}
	}
	int cell = 0;
	for (int i = 0; i < rows_; ++i) {
		if (!onlydata && asciiTopHLine(os, i, clen))
			for (int j = 0; j < depth; ++j)
				os << "  ";
		for (int j = 0; j < columns_; ++j) {
			if (isPartOfMultiColumn(i, j))
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
			if (asciiBottomHLine(os, i, clen))
				for (int j = 0; j < depth; ++j)
					os << "  ";
		}
	}
	return ret;
}


InsetText & LyXTabular::getCellInset(int cell) const
{
	cur_cell = cell;
	return cell_info[row_of_cell(cell)][column_of_cell(cell)].inset;
}


InsetText & LyXTabular::getCellInset(int row, int column) const
{
	cur_cell = getCellNumber(row, column);
	return cell_info[row][column].inset;
}


int LyXTabular::getCellFromInset(InsetOld const * inset, int maybe_cell) const
{
	// is this inset part of the tabular?
	if (!inset || inset->owner() != owner_) {
		lyxerr << "Error: this is not a cell of the tabular!" << endl;
		return -1;
	}

	const int save_cur_cell = cur_cell;
	int cell = cur_cell;
	if (&getCellInset(cell) != inset) {
		cell = maybe_cell;
		if (cell == -1 || &getCellInset(cell) != inset)
			cell = -1;
	}

	if (cell == -1) {
		for (cell = getNumberOfCells(); cell >= 0; --cell)
			if (&getCellInset(cell) == inset)
				break;

		lyxerr[Debug::INSETTEXT]
			 << "LyXTabular::getCellFromInset: "
				    << "cell=" << cell
				    << ", cur_cell=" << save_cur_cell
				    << ", maybe_cell=" << maybe_cell
				    << endl;
		// We should have found a cell at this point
		if (cell == -1) {
			lyxerr << "LyXTabular::getCellFromInset: "
			       << "Cell not found!" << endl;
		}
	}

	return cell;
}


void LyXTabular::validate(LaTeXFeatures & features) const
{
	features.require("NeedTabularnewline");
	if (isLongTabular())
		features.require("longtable");
	if (needRotating())
		features.require("rotating");
	for (int cell = 0; cell < numberofcells; ++cell) {
		if (getVAlignment(cell) != LYX_VALIGN_TOP ||
		     (!getPWidth(cell).zero() && !isMultiColumn(cell)))
			features.require("array");
		getCellInset(cell).validate(features);
	}
}


void LyXTabular::getLabelList(std::vector<string> & list) const
{
	for (int i = 0; i < rows_; ++i)
		for (int j = 0; j < columns_; ++j)
			getCellInset(i, j).getLabelList(list);
}


LyXTabular::BoxType LyXTabular::useParbox(int cell) const
{
	ParagraphList const & parlist = getCellInset(cell).paragraphs;
	ParagraphList::const_iterator cit = parlist.begin();
	ParagraphList::const_iterator end = parlist.end();

	for (; cit != end; ++cit)
		for (int i = 0; i < cit->size(); ++i)
			if (cit->isNewline(i))
				return BOX_PARBOX;

	return BOX_NONE;
}
