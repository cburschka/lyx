/**
 * \file InsetTabular.cpp
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

#include "InsetTabular.h"

#include "buffer_funcs.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "CoordCache.h"
#include "Counters.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "LyXFunc.h"
#include "LyXRC.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "paragraph_funcs.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "TextClass.h"
#include "TextMetrics.h"

#include "frontends/alert.h"
#include "frontends/Clipboard.h"
#include "frontends/Painter.h"
#include "frontends/Selection.h"


#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/FileName.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <boost/scoped_ptr.hpp>

#include <sstream>
#include <iostream>
#include <limits>
#include <cstring>

using namespace std;
using namespace lyx::support;

using boost::shared_ptr;
using boost::dynamic_pointer_cast;

namespace lyx {

using cap::dirtyTabularStack;
using cap::tabularStackDirty;

using graphics::PreviewLoader;

using frontend::Painter;
using frontend::Clipboard;

namespace Alert = frontend::Alert;


namespace {

int const ADD_TO_HEIGHT = 2; // in cell
int const ADD_TO_TABULAR_WIDTH = 6; // horiz space before and after the table
int const default_line_space = 10; // ?
int const WIDTH_OF_LINE = 5; // space between double lines


///
boost::scoped_ptr<Tabular> paste_tabular;


struct TabularFeature {
	Tabular::Feature action;
	string feature;
};


TabularFeature tabularFeature[] =
{
	{ Tabular::APPEND_ROW, "append-row" },
	{ Tabular::APPEND_COLUMN, "append-column" },
	{ Tabular::DELETE_ROW, "delete-row" },
	{ Tabular::DELETE_COLUMN, "delete-column" },
	{ Tabular::COPY_ROW, "copy-row" },
	{ Tabular::COPY_COLUMN, "copy-column" },
	{ Tabular::TOGGLE_LINE_TOP, "toggle-line-top" },
	{ Tabular::TOGGLE_LINE_BOTTOM, "toggle-line-bottom" },
	{ Tabular::TOGGLE_LINE_LEFT, "toggle-line-left" },
	{ Tabular::TOGGLE_LINE_RIGHT, "toggle-line-right" },
	{ Tabular::ALIGN_LEFT, "align-left" },
	{ Tabular::ALIGN_RIGHT, "align-right" },
	{ Tabular::ALIGN_CENTER, "align-center" },
	{ Tabular::ALIGN_BLOCK, "align-block" },
	{ Tabular::VALIGN_TOP, "valign-top" },
	{ Tabular::VALIGN_BOTTOM, "valign-bottom" },
	{ Tabular::VALIGN_MIDDLE, "valign-middle" },
	{ Tabular::M_ALIGN_LEFT, "m-align-left" },
	{ Tabular::M_ALIGN_RIGHT, "m-align-right" },
	{ Tabular::M_ALIGN_CENTER, "m-align-center" },
	{ Tabular::M_VALIGN_TOP, "m-valign-top" },
	{ Tabular::M_VALIGN_BOTTOM, "m-valign-bottom" },
	{ Tabular::M_VALIGN_MIDDLE, "m-valign-middle" },
	{ Tabular::MULTICOLUMN, "multicolumn" },
	{ Tabular::SET_ALL_LINES, "set-all-lines" },
	{ Tabular::UNSET_ALL_LINES, "unset-all-lines" },
	{ Tabular::SET_LONGTABULAR, "set-longtabular" },
	{ Tabular::UNSET_LONGTABULAR, "unset-longtabular" },
	{ Tabular::SET_PWIDTH, "set-pwidth" },
	{ Tabular::SET_MPWIDTH, "set-mpwidth" },
	{ Tabular::SET_ROTATE_TABULAR, "set-rotate-tabular" },
	{ Tabular::UNSET_ROTATE_TABULAR, "unset-rotate-tabular" },
	{ Tabular::TOGGLE_ROTATE_TABULAR, "toggle-rotate-tabular" },
	{ Tabular::SET_ROTATE_CELL, "set-rotate-cell" },
	{ Tabular::UNSET_ROTATE_CELL, "unset-rotate-cell" },
	{ Tabular::TOGGLE_ROTATE_CELL, "toggle-rotate-cell" },
	{ Tabular::SET_USEBOX, "set-usebox" },
	{ Tabular::SET_LTHEAD, "set-lthead" },
	{ Tabular::UNSET_LTHEAD, "unset-lthead" },
	{ Tabular::SET_LTFIRSTHEAD, "set-ltfirsthead" },
	{ Tabular::UNSET_LTFIRSTHEAD, "unset-ltfirsthead" },
	{ Tabular::SET_LTFOOT, "set-ltfoot" },
	{ Tabular::UNSET_LTFOOT, "unset-ltfoot" },
	{ Tabular::SET_LTLASTFOOT, "set-ltlastfoot" },
	{ Tabular::UNSET_LTLASTFOOT, "unset-ltlastfoot" },
	{ Tabular::SET_LTNEWPAGE, "set-ltnewpage" },
	{ Tabular::SET_SPECIAL_COLUMN, "set-special-column" },
	{ Tabular::SET_SPECIAL_MULTI, "set-special-multi" },
	{ Tabular::SET_BOOKTABS, "set-booktabs" },
	{ Tabular::UNSET_BOOKTABS, "unset-booktabs" },
	{ Tabular::SET_TOP_SPACE, "set-top-space" },
	{ Tabular::SET_BOTTOM_SPACE, "set-bottom-space" },
	{ Tabular::SET_INTERLINE_SPACE, "set-interline-space" },
	{ Tabular::SET_BORDER_LINES, "set-border-lines" },
	{ Tabular::LAST_ACTION, "" }
};


class FeatureEqual : public unary_function<TabularFeature, bool> {
public:
	FeatureEqual(Tabular::Feature feature)
		: feature_(feature) {}
	bool operator()(TabularFeature const & tf) const {
		return tf.action == feature_;
	}
private:
	Tabular::Feature feature_;
};


template <class T>
string const write_attribute(string const & name, T const & t)
{
	string const s = tostr(t);
	return s.empty() ? s : " " + name + "=\"" + s + "\"";
}

template <>
string const write_attribute(string const & name, string const & t)
{
	return t.empty() ? t : " " + name + "=\"" + t + "\"";
}


template <>
string const write_attribute(string const & name, docstring const & t)
{
	return t.empty() ? string() : " " + name + "=\"" + to_utf8(t) + "\"";
}


template <>
string const write_attribute(string const & name, bool const & b)
{
	// we write only true attribute values so we remove a bit of the
	// file format bloat for tabulars.
	return b ? write_attribute(name, convert<string>(b)) : string();
}


template <>
string const write_attribute(string const & name, int const & i)
{
	// we write only true attribute values so we remove a bit of the
	// file format bloat for tabulars.
	return i ? write_attribute(name, convert<string>(i)) : string();
}


template <>
string const write_attribute(string const & name, Tabular::idx_type const & i)
{
	// we write only true attribute values so we remove a bit of the
	// file format bloat for tabulars.
	return i ? write_attribute(name, convert<string>(i)) : string();
}


template <>
string const write_attribute(string const & name, Length const & value)
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


string const tostr(Tabular::VAlignment const & num)
{
	switch (num) {
	case Tabular::LYX_VALIGN_TOP:
		return "top";
	case Tabular::LYX_VALIGN_MIDDLE:
		return "middle";
	case Tabular::LYX_VALIGN_BOTTOM:
		return "bottom";
	}
	return string();
}


string const tostr(Tabular::BoxType const & num)
{
	switch (num) {
	case Tabular::BOX_NONE:
		return "none";
	case Tabular::BOX_PARBOX:
		return "parbox";
	case Tabular::BOX_MINIPAGE:
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


bool string2type(string const str, Tabular::VAlignment & num)
{
	if (str == "top")
		num = Tabular::LYX_VALIGN_TOP;
	else if (str == "middle" )
		num = Tabular::LYX_VALIGN_MIDDLE;
	else if (str == "bottom")
		num = Tabular::LYX_VALIGN_BOTTOM;
	else
		return false;
	return true;
}


bool string2type(string const str, Tabular::BoxType & num)
{
	if (str == "none")
		num = Tabular::BOX_NONE;
	else if (str == "parbox")
		num = Tabular::BOX_PARBOX;
	else if (str == "minipage")
		num = Tabular::BOX_MINIPAGE;
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


bool getTokenValue(string const & str, char const * token, string & ret)
{
	ret.erase();
	size_t token_length = strlen(token);
	size_t pos = str.find(token);

	if (pos == string::npos || pos + token_length + 1 >= str.length()
		|| str[pos + token_length] != '=')
		return false;
	pos += token_length + 1;
	char ch = str[pos];
	if (ch != '"' && ch != '\'') { // only read till next space
		ret += ch;
		ch = ' ';
	}
	while (pos < str.length() - 1 && str[++pos] != ch)
		ret += str[pos];

	return true;
}


bool getTokenValue(string const & str, char const * token, docstring & ret)
{
	string tmp;
	bool const success = getTokenValue(str, token, tmp);
	ret = from_utf8(tmp);
	return success;
}


bool getTokenValue(string const & str, char const * token, int & num)
{
	string tmp;
	num = 0;
	if (!getTokenValue(str, token, tmp))
		return false;
	num = convert<int>(tmp);
	return true;
}


bool getTokenValue(string const & str, char const * token, LyXAlignment & num)
{
	string tmp;
	return getTokenValue(str, token, tmp) && string2type(tmp, num);
}


bool getTokenValue(string const & str, char const * token,
				   Tabular::VAlignment & num)
{
	string tmp;
	return getTokenValue(str, token, tmp) && string2type(tmp, num);
}


bool getTokenValue(string const & str, char const * token,
				   Tabular::BoxType & num)
{
	string tmp;
	return getTokenValue(str, token, tmp) && string2type(tmp, num);
}


bool getTokenValue(string const & str, char const * token, bool & flag)
{
	// set the flag always to false as this should be the default for bools
	// not in the file-format.
	flag = false;
	string tmp;
	return getTokenValue(str, token, tmp) && string2type(tmp, flag);
}


bool getTokenValue(string const & str, char const * token, Length & len)
{
	// set the length to be zero() as default as this it should be if not
	// in the file format.
	len = Length();
	string tmp;
	return getTokenValue(str, token, tmp) && isValidLength(tmp, &len);
}


bool getTokenValue(string const & str, char const * token, Length & len, bool & flag)
{
	len = Length();
	flag = false;
	string tmp;
	if (!getTokenValue(str, token, tmp))
		return false;
	if (tmp == "default") {
		flag = true;
		return  true;
	}
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


string const featureAsString(Tabular::Feature feature)
{
	TabularFeature * end = tabularFeature +
		sizeof(tabularFeature) / sizeof(TabularFeature);
	TabularFeature * it = find_if(tabularFeature, end,
					   FeatureEqual(feature));
	return (it == end) ? string() : it->feature;
}



/////////////////////////////////////////////////////////////////////
//
// Tabular
//
/////////////////////////////////////////////////////////////////////


Tabular::CellData::CellData(Buffer const & buf, Tabular const & table)
	: cellno(0),
	  width(0),
	  multicolumn(Tabular::CELL_NORMAL),
	  alignment(LYX_ALIGN_CENTER),
	  valignment(LYX_VALIGN_TOP),
	  top_line(false),
	  bottom_line(false),
	  left_line(false),
	  right_line(false),
	  usebox(BOX_NONE),
	  rotate(false),
	  inset(new InsetTableCell(buf, this, &table))
{
	inset->setBuffer(const_cast<Buffer &>(buf));
	inset->paragraphs().back().setLayout(buf.params().documentClass().emptyLayout());
}


Tabular::CellData::CellData(CellData const & cs)
	: cellno(cs.cellno),
	  width(cs.width),
	  multicolumn(cs.multicolumn),
	  alignment(cs.alignment),
	  valignment(cs.valignment),
	  top_line(cs.top_line),
	  bottom_line(cs.bottom_line),
	  left_line(cs.left_line),
	  right_line(cs.right_line),
	  usebox(cs.usebox),
	  rotate(cs.rotate),
	  align_special(cs.align_special),
	  p_width(cs.p_width),
	  inset(dynamic_cast<InsetTableCell *>(cs.inset->clone()))
{
	inset->setCellData(this);
}


Tabular::CellData & Tabular::CellData::operator=(CellData cs)
{
	swap(cs);
	return *this;
}


void Tabular::CellData::swap(CellData & rhs)
{
	std::swap(cellno, rhs.cellno);
	std::swap(width, rhs.width);
	std::swap(multicolumn, rhs.multicolumn);
	std::swap(alignment, rhs.alignment);
	std::swap(valignment, rhs.valignment);
	std::swap(top_line, rhs.top_line);
	std::swap(bottom_line, rhs.bottom_line);
	std::swap(left_line, rhs.left_line);
	std::swap(right_line, rhs.right_line);
	std::swap(usebox, rhs.usebox);
	std::swap(rotate, rhs.rotate);
	std::swap(align_special, rhs.align_special);
	p_width.swap(rhs.p_width);
	inset.swap(rhs.inset);
}


Tabular::RowData::RowData()
	: ascent(0),
	  descent(0),
	  top_space_default(false),
	  bottom_space_default(false),
	  interline_space_default(false),
	  endhead(false),
	  endfirsthead(false),
	  endfoot(false),
	  endlastfoot(false),
	  newpage(false)
{}


Tabular::ColumnData::ColumnData()
	: alignment(LYX_ALIGN_CENTER),
	  valignment(LYX_VALIGN_TOP),
	  width(0)
{
}


Tabular::ltType::ltType()
	: topDL(false),
	  bottomDL(false),
	  empty(false)
{}


Tabular::Tabular()
{
	// unusable now!
}


Tabular::Tabular(Buffer const & buffer, row_type rows_arg, col_type columns_arg)
{
	init(buffer, rows_arg, columns_arg);
}


// activates all lines and sets all widths to 0
void Tabular::init(Buffer const & buf, row_type rows_arg,
		      col_type columns_arg)
{
	buffer_ = &buf;
	row_info = row_vector(rows_arg);
	column_info = column_vector(columns_arg);
	cell_info = cell_vvector(rows_arg, cell_vector(columns_arg, CellData(buf, *this)));
	row_info.reserve(10);
	column_info.reserve(10);
	cell_info.reserve(100);
	updateIndexes();
	is_long_tabular = false;
	rotate = false;
	use_booktabs = false;
	// set silly default lines
	for (row_type i = 0; i < rowCount(); ++i)
		for (col_type j = 0; j < columnCount(); ++j) {
			cell_info[i][j].top_line = true;
			cell_info[i][j].left_line = true;
			cell_info[i][j].bottom_line = i == 0 || i == rowCount() - 1;
			cell_info[i][j].right_line = j == columnCount() - 1;
		}
}


void Tabular::appendRow(idx_type const cell)
{
	BufferParams const & bp = buffer().params();
	row_type const row = cellRow(cell);

	row_vector::iterator rit = row_info.begin() + row;
	row_info.insert(rit, RowData());
	// now set the values of the row before
	row_info[row] = row_info[row + 1];

	row_type const nrows = rowCount();
	col_type const ncols = columnCount();

	cell_vvector old(nrows - 1);
	for (row_type i = 0; i < nrows - 1; ++i)
		swap(cell_info[i], old[i]);

	cell_info = cell_vvector(nrows, cell_vector(ncols, CellData(buffer(), *this)));

	for (row_type i = 0; i <= row; ++i)
		swap(cell_info[i], old[i]);
	for (row_type i = row + 2; i < nrows; ++i)
		swap(cell_info[i], old[i - 1]);

	if (bp.trackChanges)
		for (col_type j = 0; j < ncols; ++j)
			cell_info[row + 1][j].inset->setChange(Change(Change::INSERTED));

	updateIndexes();
}


void Tabular::deleteRow(row_type const row)
{
	// Not allowed to delete last row
	if (rowCount() == 1)
		return;

	row_info.erase(row_info.begin() + row);
	cell_info.erase(cell_info.begin() + row);
	updateIndexes();
}


void Tabular::copyRow(row_type const row)
{
	row_info.insert(row_info.begin() + row, row_info[row]);
	cell_info.insert(cell_info.begin() + row, cell_info[row]);

	if (buffer().params().trackChanges)
		for (col_type j = 0; j < columnCount(); ++j)
			cell_info[row + 1][j].inset->setChange(Change(Change::INSERTED));

	updateIndexes();
}


void Tabular::appendColumn(idx_type const cell)
{
	col_type const column = cellColumn(cell);
	column_vector::iterator cit = column_info.begin() + column + 1;
	column_info.insert(cit, ColumnData());
	col_type const ncols = columnCount();
	// set the column values of the column before
	column_info[column + 1] = column_info[column];

	for (row_type i = 0; i < rowCount(); ++i) {
		cell_info[i].insert(cell_info[i].begin() + column + 1, CellData(buffer(), *this));
		col_type c = column + 2;
		while (c < ncols 
			&& cell_info[i][c].multicolumn == CELL_PART_OF_MULTICOLUMN) {
			cell_info[i][c].multicolumn = CELL_NORMAL;
			++c;
		}
	}
	//++column;
	for (row_type i = 0; i < rowCount(); ++i) {
		cell_info[i][column + 1].inset->clear();
		if (buffer().params().trackChanges)
			cell_info[i][column + 1].inset->setChange(Change(Change::INSERTED));
	}
	updateIndexes();
}


void Tabular::deleteColumn(col_type const column)
{
	// Not allowed to delete last column
	if (columnCount() == 1)
		return;

	column_info.erase(column_info.begin() + column);
	for (row_type i = 0; i < rowCount(); ++i) {
		// Care about multicolumn cells
		if (column + 1 < columnCount() &&
		    cell_info[i][column].multicolumn == CELL_BEGIN_OF_MULTICOLUMN &&
		    cell_info[i][column + 1].multicolumn == CELL_PART_OF_MULTICOLUMN) {
			cell_info[i][column + 1].multicolumn = CELL_BEGIN_OF_MULTICOLUMN;
		}
		cell_info[i].erase(cell_info[i].begin() + column);
	}
	updateIndexes();
}


void Tabular::copyColumn(col_type const column)
{
	BufferParams const & bp = buffer().params();
	column_info.insert(column_info.begin() + column, column_info[column]);

	for (row_type i = 0; i < rowCount(); ++i)
		cell_info[i].insert(cell_info[i].begin() + column, cell_info[i][column]);

	if (bp.trackChanges)
		for (row_type i = 0; i < rowCount(); ++i)
			cell_info[i][column + 1].inset->setChange(Change(Change::INSERTED));
	updateIndexes();
}


void Tabular::updateIndexes()
{
	col_type ncols = columnCount();
	row_type nrows = rowCount();
	numberofcells = 0;
	for (row_type row = 0; row < nrows; ++row)
		for (col_type column = 0; column < ncols; ++column) {
			if (!isPartOfMultiColumn(row, column))
				++numberofcells;
			cell_info[row][column].cellno = numberofcells - 1;
		}

	rowofcell.resize(numberofcells);
	columnofcell.resize(numberofcells);
	idx_type i = 0;
	for (row_type row = 0; row < nrows; ++row)
		for (col_type column = 0; column < ncols; ++column) {
			if (isPartOfMultiColumn(row, column))
				continue;
			rowofcell[i] = row;
			columnofcell[i] = column;
			++i;
		}
}


Tabular::idx_type Tabular::cellCount() const
{
	return numberofcells;
}


Tabular::idx_type Tabular::numberOfCellsInRow(idx_type const cell) const
{
	row_type const row = cellRow(cell);
	idx_type result = 0;
	for (col_type i = 0; i < columnCount(); ++i)
		if (cell_info[row][i].multicolumn != Tabular::CELL_PART_OF_MULTICOLUMN)
			++result;
	return result;
}


bool Tabular::topLine(idx_type const cell) const
{
	return cellinfo_of_cell(cell).top_line;
}


bool Tabular::bottomLine(idx_type const cell) const
{
	return cellinfo_of_cell(cell).bottom_line;
}


bool Tabular::leftLine(idx_type cell) const
{
	if (use_booktabs)
		return false;
	return cellinfo_of_cell(cell).left_line;
}


bool Tabular::rightLine(idx_type cell) const
{
	if (use_booktabs)
		return false;
	return cellinfo_of_cell(cell).right_line;
}


bool Tabular::topAlreadyDrawn(idx_type cell) const
{
	row_type row = cellRow(cell);
	if (row == 0)
		return false;
	idx_type i = cellIndex(row - 1, cellColumn(cell));
	return !rowBottomLine(row - 1) && bottomLine(i);
}


bool Tabular::leftAlreadyDrawn(idx_type cell) const
{
	col_type col = cellColumn(cell);
	if (col == 0)
		return false;
	idx_type i = cellIndex(cellRow(cell), col - 1);
	return !columnRightLine(col - 1) && rightLine(i);
}


bool Tabular::isLastRow(idx_type cell) const
{
	return cellRow(cell) == rowCount() - 1;
}


int Tabular::getAdditionalHeight(row_type row) const
{
	if (!row || row >= rowCount())
		return 0;

	int const interline_space = row_info[row - 1].interline_space_default ?
		default_line_space :
		row_info[row - 1].interline_space.inPixels(width());
	if (rowTopLine(row) && rowBottomLine(row - 1))
		return interline_space + WIDTH_OF_LINE;
	return interline_space;
}


int Tabular::getAdditionalWidth(idx_type cell) const
{
	// internally already set in setCellWidth
	// used to get it back in text.cpp
	col_type c = cellColumn(cell);
	if (c < columnCount() - 1 
		&& columnRightLine(c) && columnLeftLine(c + 1)
		&& cellinfo_of_cell(cell).multicolumn == CELL_NORMAL)
		return WIDTH_OF_LINE;
	return 0;
}


int Tabular::columnWidth(idx_type cell) const
{
	int w = 0;
	col_type const span = columnSpan(cell);
	col_type const col = cellColumn(cell);
	for(col_type c = col; c < col + span ; ++c)
		w += column_info[c].width;
	return w;
}


bool Tabular::updateColumnWidths()
{
	col_type const ncols = columnCount();
	row_type const nrows = rowCount();
	bool update = false;
	// for each col get max of single col cells
	for(col_type c = 0; c < ncols; ++c) {
		int new_width = 0;
		for(row_type r = 0; r < nrows; ++r) {
			idx_type const i = cellIndex(r, c);
			if (columnSpan(i) == 1)
				new_width = max(new_width, cellinfo_of_cell(i).width);
		}

		if (column_info[c].width != new_width) {
			column_info[c].width = new_width;
			update = true;
		}
	}
	// update col widths to fit merged cells
	for(col_type c = 0; c < ncols; ++c)
		for(row_type r = 0; r < nrows; ++r) {
			idx_type const i = cellIndex(r, c);
			int const span = columnSpan(i);
			if (span == 1 || c > cellColumn(i))
				continue;

			int old_width = 0;
			for(col_type j = c; j < c + span ; ++j)
				old_width += column_info[j].width;

			if (cellinfo_of_cell(i).width > old_width) {
				column_info[c + span - 1].width += cellinfo_of_cell(i).width - old_width;
				update = true;
			}
		}

	return update;
}


int Tabular::width() const
{
	col_type const ncols = columnCount();
	int width = 0;
	for (col_type i = 0; i < ncols; ++i)
		width += column_info[i].width;
	return width;
}


void Tabular::setCellWidth(idx_type cell, int new_width)
{
	col_type const col = cellColumn(cell);
	int add_width = 0;
	if (col < columnCount() - 1 && columnRightLine(col) &&
		columnLeftLine(col + 1)) {
		add_width = WIDTH_OF_LINE;
	}
	cellinfo_of_cell(cell).width = new_width + 2 * WIDTH_OF_LINE + add_width;
	return;
}


void Tabular::setAlignment(idx_type cell, LyXAlignment align,
			      bool onlycolumn)
{
	if (!isMultiColumn(cell) || onlycolumn)
		column_info[cellColumn(cell)].alignment = align;
	if (!onlycolumn)
		cellinfo_of_cell(cell).alignment = align;
}


void Tabular::setVAlignment(idx_type cell, VAlignment align,
			       bool onlycolumn)
{
	if (!isMultiColumn(cell) || onlycolumn)
		column_info[cellColumn(cell)].valignment = align;
	if (!onlycolumn)
		cellinfo_of_cell(cell).valignment = align;
}


namespace {

/**
 * Allow line and paragraph breaks for fixed width cells or disallow them,
 * merge cell paragraphs and reset layout to standard for variable width
 * cells.
 */
void toggleFixedWidth(Cursor & cur, InsetTableCell * inset, bool fixedWidth)
{
	inset->setAutoBreakRows(fixedWidth);
	if (fixedWidth)
		return;

	// merge all paragraphs to one
	BufferParams const & bp = cur.bv().buffer().params();
	while (inset->paragraphs().size() > 1)
		mergeParagraph(bp, inset->paragraphs(), 0);

	// reset layout
	cur.push(*inset);
	// undo information has already been recorded
	inset->getText(0)->setLayout(cur.bv().buffer(), 0, cur.lastpit() + 1,
			bp.documentClass().emptyLayoutName());
	cur.pop();
}

}


void Tabular::setColumnPWidth(Cursor & cur, idx_type cell,
		Length const & width)
{
	col_type const j = cellColumn(cell);

	column_info[j].p_width = width;
	for (row_type i = 0; i < rowCount(); ++i) {
		idx_type const cell = cellIndex(i, j);
		// because of multicolumns
		toggleFixedWidth(cur, getCellInset(cell).get(),
				 !getPWidth(cell).zero());
	}
	// cur paragraph can become invalid after paragraphs were merged
	if (cur.pit() > cur.lastpit())
		cur.pit() = cur.lastpit();
	// cur position can become invalid after newlines were removed
	if (cur.pos() > cur.lastpos())
		cur.pos() = cur.lastpos();
}


bool Tabular::setMColumnPWidth(Cursor & cur, idx_type cell,
		Length const & width)
{
	if (!isMultiColumn(cell))
		return false;

	cellinfo_of_cell(cell).p_width = width;
	toggleFixedWidth(cur, getCellInset(cell).get(), !width.zero());
	// cur paragraph can become invalid after paragraphs were merged
	if (cur.pit() > cur.lastpit())
		cur.pit() = cur.lastpit();
	// cur position can become invalid after newlines were removed
	if (cur.pos() > cur.lastpos())
		cur.pos() = cur.lastpos();
	return true;
}


void Tabular::setAlignSpecial(idx_type cell, docstring const & special,
				 Tabular::Feature what)
{
	if (what == SET_SPECIAL_MULTI)
		cellinfo_of_cell(cell).align_special = special;
	else
		column_info[cellColumn(cell)].align_special = special;
}


void Tabular::setAllLines(idx_type cell, bool line)
{
	setTopLine(cell, line);
	setBottomLine(cell, line);
	setRightLine(cell, line);
	setLeftLine(cell, line);
}


void Tabular::setTopLine(idx_type i, bool line)
{
	cellinfo_of_cell(i).top_line = line;
}


void Tabular::setBottomLine(idx_type i, bool line)
{
	cellinfo_of_cell(i).bottom_line = line;
}


void Tabular::setLeftLine(idx_type cell, bool line)
{
	cellinfo_of_cell(cell).left_line = line;
}


void Tabular::setRightLine(idx_type cell, bool line)
{
	cellinfo_of_cell(cell).right_line = line;
}

bool Tabular::rowTopLine(row_type r) const
{
	idx_type i0 = getFirstCellInRow(r);
	idx_type i1 = getLastCellInRow(r);
	bool all_rows_set = true;
	for (idx_type j = i0; all_rows_set && j <= i1; ++j)
		all_rows_set = cellinfo_of_cell(j).top_line;
	return all_rows_set;
}


bool Tabular::rowBottomLine(row_type r) const
{
	idx_type i0 = getFirstCellInRow(r);
	idx_type i1 = getLastCellInRow(r);
	bool all_rows_set = true;
	for (idx_type j = i0; all_rows_set && j <= i1; ++j)
		all_rows_set = cellinfo_of_cell(j).bottom_line;
	return all_rows_set;
}


bool Tabular::columnLeftLine(col_type c) const
{
	bool all_cols_set = true;
	row_type nrows = rowCount();
	for (row_type r = 0; all_cols_set && r < nrows; ++r) {
		idx_type i = cellIndex(r, c);
		if (columnSpan(i) == 1)
			all_cols_set = cellinfo_of_cell(i).left_line;
	}
	return all_cols_set;
}


bool Tabular::columnRightLine(col_type c) const
{
	bool all_cols_set = true;
	row_type nrows = rowCount();
	for (row_type r = 0; all_cols_set && r < nrows; ++r) {
		idx_type i = cellIndex(r, c);
		if (c == cellColumn(i) + columnSpan(i) - 1)
			all_cols_set = cellinfo_of_cell(i).right_line;
	}
	return all_cols_set;
}


LyXAlignment Tabular::getAlignment(idx_type cell, bool onlycolumn) const
{
	if (!onlycolumn && isMultiColumn(cell))
		return cellinfo_of_cell(cell).alignment;
	return column_info[cellColumn(cell)].alignment;
}


Tabular::VAlignment
Tabular::getVAlignment(idx_type cell, bool onlycolumn) const
{
	if (!onlycolumn && isMultiColumn(cell))
		return cellinfo_of_cell(cell).valignment;
	return column_info[cellColumn(cell)].valignment;
}


Length const Tabular::getPWidth(idx_type cell) const
{
	if (isMultiColumn(cell))
		return cellinfo_of_cell(cell).p_width;
	return column_info[cellColumn(cell)].p_width;
}


Length const Tabular::getColumnPWidth(idx_type cell) const
{
	return column_info[cellColumn(cell)].p_width;
}


Length const Tabular::getMColumnPWidth(idx_type cell) const
{
	if (isMultiColumn(cell))
		return cellinfo_of_cell(cell).p_width;
	return Length();
}


docstring const Tabular::getAlignSpecial(idx_type cell, int what) const
{
	if (what == SET_SPECIAL_MULTI)
		return cellinfo_of_cell(cell).align_special;
	return column_info[cellColumn(cell)].align_special;
}


int Tabular::cellWidth(idx_type cell) const
{
	return cellinfo_of_cell(cell).width;
}


int Tabular::getBeginningOfTextInCell(idx_type cell) const
{
	int x = 0;

	switch (getAlignment(cell)) {
	case LYX_ALIGN_CENTER:
		x += (columnWidth(cell) - cellWidth(cell)) / 2;
		break;
	case LYX_ALIGN_RIGHT:
		x += columnWidth(cell) - cellWidth(cell);
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


bool Tabular::isFirstCellInRow(idx_type cell) const
{
	return cellColumn(cell) == 0;
}


Tabular::idx_type Tabular::getFirstCellInRow(row_type row) const
{
	if (row > rowCount() - 1)
		row = rowCount() - 1;
	return cell_info[row][0].cellno;
}


bool Tabular::isLastCellInRow(idx_type cell) const
{
	return cellRightColumn(cell) == columnCount() - 1;
}


Tabular::idx_type Tabular::getLastCellInRow(row_type row) const
{
	if (row > rowCount() - 1)
		row = rowCount() - 1;
	return cell_info[row][columnCount() - 1].cellno;
}


Tabular::row_type Tabular::cellRow(idx_type cell) const
{
	if (cell >= cellCount())
		return rowCount() - 1;
	if (cell == npos)
		return 0;
	return rowofcell[cell];
}


Tabular::col_type Tabular::cellColumn(idx_type cell) const
{
	if (cell >= cellCount())
		return columnCount() - 1;
	if (cell == npos)
		return 0;
	return columnofcell[cell];
}


Tabular::col_type Tabular::cellRightColumn(idx_type cell) const
{
	row_type const row = cellRow(cell);
	col_type column = cellColumn(cell);
	while (column < columnCount() - 1 &&
		   cell_info[row][column + 1].multicolumn == CELL_PART_OF_MULTICOLUMN)
		++column;
	return column;
}


void Tabular::write(ostream & os) const
{
	// header line
	os << "<lyxtabular"
	   << write_attribute("version", 3)
	   << write_attribute("rows", rowCount())
	   << write_attribute("columns", columnCount())
	   << ">\n";
	// global longtable options
	os << "<features"
	   << write_attribute("rotate", rotate)
	   << write_attribute("booktabs", use_booktabs)
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
	for (col_type j = 0; j < columnCount(); ++j) {
		os << "<column"
		   << write_attribute("alignment", column_info[j].alignment)
		   << write_attribute("valignment", column_info[j].valignment)
		   << write_attribute("width", column_info[j].p_width.asString())
		   << write_attribute("special", column_info[j].align_special)
		   << ">\n";
	}
	for (row_type i = 0; i < rowCount(); ++i) {
		static const string def("default");
		os << "<row";
		if (row_info[i].top_space_default)
			os << write_attribute("topspace", def);
		else
			os << write_attribute("topspace", row_info[i].top_space);
		if (row_info[i].bottom_space_default)
			os << write_attribute("bottomspace", def);
		else
			os << write_attribute("bottomspace", row_info[i].bottom_space);
		if (row_info[i].interline_space_default)
			os << write_attribute("interlinespace", def);
		else
			os << write_attribute("interlinespace", row_info[i].interline_space);
		os << write_attribute("endhead", row_info[i].endhead)
		   << write_attribute("endfirsthead", row_info[i].endfirsthead)
		   << write_attribute("endfoot", row_info[i].endfoot)
		   << write_attribute("endlastfoot", row_info[i].endlastfoot)
		   << write_attribute("newpage", row_info[i].newpage)
		   << ">\n";
		for (col_type j = 0; j < columnCount(); ++j) {
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
			cell_info[i][j].inset->write(os);
			os << "\n\\end_inset\n"
			   << "</cell>\n";
		}
		os << "</row>\n";
	}
	os << "</lyxtabular>\n";
}


void Tabular::read(Lexer & lex)
{
	string line;
	istream & is = lex.getStream();

	l_getline(is, line);
	if (!prefixIs(line, "<lyxtabular ") && !prefixIs(line, "<Tabular ")) {
		BOOST_ASSERT(false);
		return;
	}

	int version;
	if (!getTokenValue(line, "version", version))
		return;
	BOOST_ASSERT(version >= 2);

	int rows_arg;
	if (!getTokenValue(line, "rows", rows_arg))
		return;
	int columns_arg;
	if (!getTokenValue(line, "columns", columns_arg))
		return;
	init(buffer(), rows_arg, columns_arg);
	l_getline(is, line);
	if (!prefixIs(line, "<features")) {
		lyxerr << "Wrong tabular format (expected <features ...> got"
		       << line << ')' << endl;
		return;
	}
	getTokenValue(line, "rotate", rotate);
	getTokenValue(line, "booktabs", use_booktabs);
	getTokenValue(line, "islongtable", is_long_tabular);
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

	for (col_type j = 0; j < columnCount(); ++j) {
		l_getline(is,line);
		if (!prefixIs(line,"<column")) {
			lyxerr << "Wrong tabular format (expected <column ...> got"
			       << line << ')' << endl;
			return;
		}
		getTokenValue(line, "alignment", column_info[j].alignment);
		getTokenValue(line, "valignment", column_info[j].valignment);
		getTokenValue(line, "width", column_info[j].p_width);
		getTokenValue(line, "special", column_info[j].align_special);
	}

	for (row_type i = 0; i < rowCount(); ++i) {
		l_getline(is, line);
		if (!prefixIs(line, "<row")) {
			lyxerr << "Wrong tabular format (expected <row ...> got"
			       << line << ')' << endl;
			return;
		}
		getTokenValue(line, "topspace", row_info[i].top_space,
			      row_info[i].top_space_default);
		getTokenValue(line, "bottomspace", row_info[i].bottom_space,
			      row_info[i].bottom_space_default);
		getTokenValue(line, "interlinespace", row_info[i].interline_space,
			      row_info[i].interline_space_default);
		getTokenValue(line, "endfirsthead", row_info[i].endfirsthead);
		getTokenValue(line, "endhead", row_info[i].endhead);
		getTokenValue(line, "endfoot", row_info[i].endfoot);
		getTokenValue(line, "endlastfoot", row_info[i].endlastfoot);
		getTokenValue(line, "newpage", row_info[i].newpage);
		for (col_type j = 0; j < columnCount(); ++j) {
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
				cell_info[i][j].inset->read(lex);
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
	updateIndexes();
}


bool Tabular::isMultiColumn(idx_type cell) const
{
	return cellinfo_of_cell(cell).multicolumn != CELL_NORMAL;
}


bool Tabular::isMultiColumnReal(idx_type cell) const
{
	return cellColumn(cell) != cellRightColumn(cell) &&
			cellinfo_of_cell(cell).multicolumn != CELL_NORMAL;
}


Tabular::CellData & Tabular::cellinfo_of_cell(idx_type cell) const
{
	return cell_info[cellRow(cell)][cellColumn(cell)];
}


void Tabular::setMultiColumn(idx_type cell, idx_type number)
{
	CellData & cs = cellinfo_of_cell(cell);
	cs.multicolumn = CELL_BEGIN_OF_MULTICOLUMN;
	cs.alignment = column_info[cellColumn(cell)].alignment;
	for (idx_type i = 1; i < number; ++i) {
		CellData & cs1 = cellinfo_of_cell(cell + i);
		cs1.multicolumn = CELL_PART_OF_MULTICOLUMN;
		cs.inset->appendParagraphs(cs1.inset->paragraphs());
		cs1.inset->clear();
	}
	setRightLine(cell, rightLine(cell + number - 1));
	updateIndexes();
}


Tabular::idx_type Tabular::columnSpan(idx_type cell) const
{
	row_type const row = cellRow(cell);
	col_type const ncols = columnCount();
	idx_type result = 1;
	col_type column = cellColumn(cell) + 1;
	while (column < ncols && isPartOfMultiColumn(row, column)) {
		++result;
		++column;
	}
	return result;
}


Tabular::idx_type Tabular::unsetMultiColumn(idx_type cell)
{
	row_type const row = cellRow(cell);
	col_type column = cellColumn(cell);

	idx_type result = 0;

	if (cell_info[row][column].multicolumn == CELL_BEGIN_OF_MULTICOLUMN) {
		cell_info[row][column].multicolumn = CELL_NORMAL;
		++column;
		while (column < columnCount() &&
			   cell_info[row][column].multicolumn == CELL_PART_OF_MULTICOLUMN)
		{
			cell_info[row][column].multicolumn = CELL_NORMAL;
			++column;
			++result;
		}
	}
	updateIndexes();
	return result;
}


void Tabular::setBookTabs(bool what)
{
	use_booktabs = what;
}


bool Tabular::useBookTabs() const
{
	return use_booktabs;
}


void Tabular::setLongTabular(bool what)
{
	is_long_tabular = what;
}


bool Tabular::isLongTabular() const
{
	return is_long_tabular;
}


void Tabular::setRotateTabular(bool flag)
{
	rotate = flag;
}


bool Tabular::getRotateTabular() const
{
	return rotate;
}


void Tabular::setRotateCell(idx_type cell, bool flag)
{
	cellinfo_of_cell(cell).rotate = flag;
}


bool Tabular::getRotateCell(idx_type cell) const
{
	return cellinfo_of_cell(cell).rotate;
}


bool Tabular::needRotating() const
{
	if (rotate)
		return true;
	for (row_type i = 0; i < rowCount(); ++i)
		for (col_type j = 0; j < columnCount(); ++j)
			if (cell_info[i][j].rotate)
				return true;
	return false;
}


bool Tabular::isLastCell(idx_type cell) const
{
	if (cell + 1 < cellCount())
		return false;
	return true;
}


Tabular::idx_type Tabular::getCellAbove(idx_type cell) const
{
	if (cellRow(cell) > 0)
		return cell_info[cellRow(cell)-1][cellColumn(cell)].cellno;
	return cell;
}


Tabular::idx_type Tabular::getCellBelow(idx_type cell) const
{
	if (cellRow(cell) + 1 < rowCount())
		return cell_info[cellRow(cell)+1][cellColumn(cell)].cellno;
	return cell;
}


Tabular::idx_type Tabular::cellIndex(row_type row,
					       col_type column) const
{
	BOOST_ASSERT(column != npos && column < columnCount() &&
		     row    != npos && row    < rowCount());
	return cell_info[row][column].cellno;
}


void Tabular::setUsebox(idx_type cell, BoxType type)
{
	cellinfo_of_cell(cell).usebox = type;
}


Tabular::BoxType Tabular::getUsebox(idx_type cell) const
{
	if (column_info[cellColumn(cell)].p_width.zero() &&
		!(isMultiColumn(cell) && !cellinfo_of_cell(cell).p_width.zero()))
		return BOX_NONE;
	if (cellinfo_of_cell(cell).usebox > 1)
		return cellinfo_of_cell(cell).usebox;
	return useParbox(cell);
}


///
//  This are functions used for the longtable support
///
void Tabular::setLTHead(row_type row, bool flag, ltType const & hd,
			   bool first)
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


bool Tabular::getRowOfLTHead(row_type row, ltType & hd) const
{
	hd = endhead;
	hd.set = haveLTHead();
	return row_info[row].endhead;
}


bool Tabular::getRowOfLTFirstHead(row_type row, ltType & hd) const
{
	hd = endfirsthead;
	hd.set = haveLTFirstHead();
	return row_info[row].endfirsthead;
}


void Tabular::setLTFoot(row_type row, bool flag, ltType const & fd,
			   bool last)
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


bool Tabular::getRowOfLTFoot(row_type row, ltType & fd) const
{
	fd = endfoot;
	fd.set = haveLTFoot();
	return row_info[row].endfoot;
}


bool Tabular::getRowOfLTLastFoot(row_type row, ltType & fd) const
{
	fd = endlastfoot;
	fd.set = haveLTLastFoot();
	return row_info[row].endlastfoot;
}


void Tabular::setLTNewPage(row_type row, bool what)
{
	row_info[row].newpage = what;
}


bool Tabular::getLTNewPage(row_type row) const
{
	return row_info[row].newpage;
}


bool Tabular::haveLTHead() const
{
	for (row_type i = 0; i < rowCount(); ++i)
		if (row_info[i].endhead)
			return true;
	return false;
}


bool Tabular::haveLTFirstHead() const
{
	if (endfirsthead.empty)
		return false;
	for (row_type i = 0; i < rowCount(); ++i)
		if (row_info[i].endfirsthead)
			return true;
	return false;
}


bool Tabular::haveLTFoot() const
{
	for (row_type i = 0; i < rowCount(); ++i)
		if (row_info[i].endfoot)
			return true;
	return false;
}


bool Tabular::haveLTLastFoot() const
{
	if (endlastfoot.empty)
		return false;
	for (row_type i = 0; i < rowCount(); ++i)
		if (row_info[i].endlastfoot)
			return true;
	return false;
}


// end longtable support functions

void Tabular::setRowAscent(row_type row, int height)
{
	if (row >= rowCount() || row_info[row].ascent == height)
		return;
	row_info[row].ascent = height;
}


void Tabular::setRowDescent(row_type row, int height)
{
	if (row >= rowCount() || row_info[row].descent == height)
		return;
	row_info[row].descent = height;
}


int Tabular::rowAscent(row_type row) const
{
	if (row >= rowCount())
		return 0;
	return row_info[row].ascent;
}


int Tabular::rowDescent(row_type row) const
{
	BOOST_ASSERT(row < rowCount());
	return row_info[row].descent;
}


int Tabular::height() const
{
	int height = 0;
	for (row_type row = 0; row < rowCount(); ++row)
		height += rowAscent(row) + rowDescent(row) +
			getAdditionalHeight(row);
	return height;
}


bool Tabular::isPartOfMultiColumn(row_type row, col_type column) const
{
	BOOST_ASSERT(row < rowCount());
	BOOST_ASSERT(column < columnCount());
	return cell_info[row][column].multicolumn == CELL_PART_OF_MULTICOLUMN;
}


int Tabular::TeXTopHLine(odocstream & os, row_type row) const
{
	// FIXME: assert or return 0 as in TeXBottomHLine()?
	BOOST_ASSERT(row != npos);
	BOOST_ASSERT(row < rowCount());

	idx_type const fcell = getFirstCellInRow(row);
	idx_type const n = numberOfCellsInRow(fcell) + fcell;
	idx_type tmp = 0;

	for (idx_type i = fcell; i < n; ++i) {
		if (topLine(i))
			++tmp;
	}
	if (use_booktabs && row == 0) {
		if (topLine(fcell))
			os << "\\toprule ";
	} else if (tmp == n - fcell) {
		os << (use_booktabs ? "\\midrule " : "\\hline ");
	} else if (tmp) {
		for (idx_type i = fcell; i < n; ++i) {
			if (topLine(i)) {
				os << (use_booktabs ? "\\cmidrule{" : "\\cline{")
				   << cellColumn(i) + 1
				   << '-'
				   << cellRightColumn(i) + 1
				   << "} ";
			}
		}
	} else {
		return 0;
	}
	os << "\n";
	return 1;
}


int Tabular::TeXBottomHLine(odocstream & os, row_type row) const
{
	// FIXME: return 0 or assert as in TeXTopHLine()?
	if (row == npos || row >= rowCount())
		return 0;

	idx_type const fcell = getFirstCellInRow(row);
	idx_type const n = numberOfCellsInRow(fcell) + fcell;
	idx_type tmp = 0;

	for (idx_type i = fcell; i < n; ++i) {
		if (bottomLine(i))
			++tmp;
	}
	if (use_booktabs && row == rowCount() - 1) {
		if (bottomLine(fcell))
			os << "\\bottomrule";
	} else if (tmp == n - fcell) {
		os << (use_booktabs ? "\\midrule" : "\\hline");
	} else if (tmp) {
		for (idx_type i = fcell; i < n; ++i) {
			if (bottomLine(i)) {
				os << (use_booktabs ? "\\cmidrule{" : "\\cline{")
				   << cellColumn(i) + 1
				   << '-'
				   << cellRightColumn(i) + 1
				   << "} ";
			}
		}
	} else {
		return 0;
	}
	os << "\n";
	return 1;
}


int Tabular::TeXCellPreamble(odocstream & os, idx_type cell) const
{
	int ret = 0;

	if (getRotateCell(cell)) {
		os << "\\begin{sideways}\n";
		++ret;
	}
	Tabular::VAlignment valign =  getVAlignment(cell, !isMultiColumn(cell));
	LyXAlignment align = getAlignment(cell, !isMultiColumn(cell));
	col_type c = cellColumn(cell);
	if (isMultiColumn(cell)
		|| (leftLine(cell) && !columnLeftLine(c))
		|| (rightLine(cell) && !columnRightLine(c))) {
		os << "\\multicolumn{" << columnSpan(cell) << "}{";
		if (leftLine(cell))
			os << '|';
		if (!cellinfo_of_cell(cell).align_special.empty()) {
			os << cellinfo_of_cell(cell).align_special;
		} else {
			if (!getPWidth(cell).zero()) {
				switch (valign) {
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
				   << from_ascii(getPWidth(cell).asLatexString())
				   << '}';
			} else {
				switch (align) {
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
			} // end if else !getPWidth
		} // end if else !cellinfo_of_cell
		if (rightLine(cell))
			os << '|';
		os << "}{";
		}
	if (getUsebox(cell) == BOX_PARBOX) {
		os << "\\parbox[";
		switch (valign) {
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
		os << "]{" << from_ascii(getPWidth(cell).asLatexString())
		   << "}{";
	} else if (getUsebox(cell) == BOX_MINIPAGE) {
		os << "\\begin{minipage}[";
		switch (valign) {
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
		os << "]{" << from_ascii(getPWidth(cell).asLatexString())
		   << "}\n";
		++ret;
	}
	return ret;
}


int Tabular::TeXCellPostamble(odocstream & os, idx_type cell) const
{
	int ret = 0;

	// usual cells
	if (getUsebox(cell) == BOX_PARBOX)
		os << '}';
	else if (getUsebox(cell) == BOX_MINIPAGE) {
		os << "%\n\\end{minipage}";
		ret += 2;
	}
	col_type c = cellColumn(cell);
	if (isMultiColumn(cell)
		|| (leftLine(cell) && !columnLeftLine(c))
		|| (rightLine(cell) && !columnRightLine(c))) {
		os << '}';
	}
	if (getRotateCell(cell)) {
		os << "%\n\\end{sideways}";
		++ret;
	}
	return ret;
}


int Tabular::TeXLongtableHeaderFooter(odocstream & os,
					 OutputParams const & runparams) const
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
		for (row_type i = 0; i < rowCount(); ++i) {
			if (row_info[i].endhead) {
				ret += TeXRow(os, i, runparams);
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
		for (row_type i = 0; i < rowCount(); ++i) {
			if (row_info[i].endfirsthead) {
				ret += TeXRow(os, i, runparams);
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
		for (row_type i = 0; i < rowCount(); ++i) {
			if (row_info[i].endfoot) {
				ret += TeXRow(os, i, runparams);
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
		for (row_type i = 0; i < rowCount(); ++i) {
			if (row_info[i].endlastfoot) {
				ret += TeXRow(os, i, runparams);
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


bool Tabular::isValidRow(row_type row) const
{
	if (!is_long_tabular)
		return true;
	return !row_info[row].endhead && !row_info[row].endfirsthead &&
			!row_info[row].endfoot && !row_info[row].endlastfoot;
}


int Tabular::TeXRow(odocstream & os, row_type i,
		       OutputParams const & runparams) const
{
	idx_type cell = cellIndex(i, 0);
	int ret = TeXTopHLine(os, i);
	if (row_info[i].top_space_default) {
		if (use_booktabs)
			os << "\\addlinespace\n";
		else
			os << "\\noalign{\\vskip\\doublerulesep}\n";
		++ret;
	} else if(!row_info[i].top_space.zero()) {
		if (use_booktabs)
			os << "\\addlinespace["
			   << from_ascii(row_info[i].top_space.asLatexString())
			   << "]\n";
		else {
			os << "\\noalign{\\vskip"
			   << from_ascii(row_info[i].top_space.asLatexString())
			   << "}\n";
		}
		++ret;
	}

	for (col_type j = 0; j < columnCount(); ++j) {
		if (isPartOfMultiColumn(i, j))
			continue;
		ret += TeXCellPreamble(os, cell);
		shared_ptr<InsetTableCell> inset = getCellInset(cell);

		Paragraph const & par = inset->paragraphs().front();
		bool rtl = par.isRTL(buffer().params())
			&& !par.empty()
			&& getPWidth(cell).zero();

		if (rtl) {
			if (par.getParLanguage(buffer().params())->lang() ==
			"farsi")
				os << "\\textFR{";
			else if (par.getParLanguage(buffer().params())->lang() == "arabic_arabi")
				os << "\\textAR{";
			// currently, remaning RTL languages are arabic_arabtex and hebrew
			else
				os << "\\R{";
		}
		ret += inset->latex(os, runparams);
		if (rtl)
			os << '}';

		ret += TeXCellPostamble(os, cell);
		if (!isLastCellInRow(cell)) { // not last cell in row
			os << " & ";
		}
		++cell;
	}
	os << "\\tabularnewline";
	if (row_info[i].bottom_space_default) {
		if (use_booktabs)
			os << "\\addlinespace";
		else
			os << "[\\doublerulesep]";
	} else if (!row_info[i].bottom_space.zero()) {
		if (use_booktabs)
			os << "\\addlinespace";
		os << '['
		   << from_ascii(row_info[i].bottom_space.asLatexString())
		   << ']';
	}
	os << '\n';
	++ret;
	ret += TeXBottomHLine(os, i);
	if (row_info[i].interline_space_default) {
		if (use_booktabs)
			os << "\\addlinespace\n";
		else
			os << "\\noalign{\\vskip\\doublerulesep}\n";
		++ret;
	} else if (!row_info[i].interline_space.zero()) {
		if (use_booktabs)
			os << "\\addlinespace["
			   << from_ascii(row_info[i].interline_space.asLatexString())
			   << "]\n";
		else
			os << "\\noalign{\\vskip"
			   << from_ascii(row_info[i].interline_space.asLatexString())
			   << "}\n";
		++ret;
	}
	return ret;
}


int Tabular::latex(odocstream & os, OutputParams const & runparams) const
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

	for (col_type i = 0; i < columnCount(); ++i) {
		if (!use_booktabs && columnLeftLine(i))
			os << '|';
		if (!column_info[i].align_special.empty()) {
			os << column_info[i].align_special;
		} else {
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
				   << from_ascii(column_info[i].p_width.asLatexString())
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
			} // end if else !column_info[i].p_width
		} // end if else !column_info[i].align_special
		if (!use_booktabs && columnRightLine(i))
			os << '|';
	}
	os << "}\n";
	++ret;

	ret += TeXLongtableHeaderFooter(os, runparams);

	//+---------------------------------------------------------------------
	//+                      the single row and columns (cells)            +
	//+---------------------------------------------------------------------

	for (row_type i = 0; i < rowCount(); ++i) {
		if (isValidRow(i)) {
			ret += TeXRow(os, i, runparams);
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


int Tabular::docbookRow(odocstream & os, row_type row,
			   OutputParams const & runparams) const
{
	int ret = 0;
	idx_type cell = getFirstCellInRow(row);

	os << "<row>\n";
	for (col_type j = 0; j < columnCount(); ++j) {
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
			os << "nameend=\"col" << j + columnSpan(cell) - 1<< '"';
		}

		os << '>';
		ret += getCellInset(cell)->docbook(os, runparams);
		os << "</entry>\n";
		++cell;
	}
	os << "</row>\n";
	return ret;
}


int Tabular::docbook(odocstream & os, OutputParams const & runparams) const
{
	int ret = 0;

	//+---------------------------------------------------------------------
	//+                      first the opening preamble                    +
	//+---------------------------------------------------------------------

	os << "<tgroup cols=\"" << columnCount()
	   << "\" colsep=\"1\" rowsep=\"1\">\n";

	for (col_type i = 0; i < columnCount(); ++i) {
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
		os << '"';
		if (runparams.flavor == OutputParams::XML)
			os << '/';
		os << ">\n";
		++ret;
	}

	//+---------------------------------------------------------------------
	//+                      Long Tabular case                             +
	//+---------------------------------------------------------------------

	// output header info
	if (haveLTHead() || haveLTFirstHead()) {
		os << "<thead>\n";
		++ret;
		for (row_type i = 0; i < rowCount(); ++i) {
			if (row_info[i].endhead || row_info[i].endfirsthead) {
				ret += docbookRow(os, i, runparams);
			}
		}
		os << "</thead>\n";
		++ret;
	}
	// output footer info
	if (haveLTFoot() || haveLTLastFoot()) {
		os << "<tfoot>\n";
		++ret;
		for (row_type i = 0; i < rowCount(); ++i) {
			if (row_info[i].endfoot || row_info[i].endlastfoot) {
				ret += docbookRow(os, i, runparams);
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
	for (row_type i = 0; i < rowCount(); ++i) {
		if (isValidRow(i)) {
			ret += docbookRow(os, i, runparams);
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


bool Tabular::plaintextTopHLine(odocstream & os, row_type row,
				   vector<unsigned int> const & clen) const
{
	idx_type const fcell = getFirstCellInRow(row);
	idx_type const n = numberOfCellsInRow(fcell) + fcell;
	idx_type tmp = 0;

	for (idx_type i = fcell; i < n; ++i) {
		if (topLine(i)) {
			++tmp;
			break;
		}
	}
	if (!tmp)
		return false;

	char_type ch;
	for (idx_type i = fcell; i < n; ++i) {
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
		col_type column = cellColumn(i);
		int len = clen[column];
		while (column < columnCount() - 1
		       && isPartOfMultiColumn(row, ++column))
			len += clen[column] + 4;
		os << docstring(len, ch);
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
	return true;
}


bool Tabular::plaintextBottomHLine(odocstream & os, row_type row,
				      vector<unsigned int> const & clen) const
{
	idx_type const fcell = getFirstCellInRow(row);
	idx_type const n = numberOfCellsInRow(fcell) + fcell;
	idx_type tmp = 0;

	for (idx_type i = fcell; i < n; ++i) {
		if (bottomLine(i)) {
			++tmp;
			break;
		}
	}
	if (!tmp)
		return false;

	char_type ch;
	for (idx_type i = fcell; i < n; ++i) {
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
		col_type column = cellColumn(i);
		int len = clen[column];
		while (column < columnCount() -1
		       && isPartOfMultiColumn(row, ++column))
			len += clen[column] + 4;
		os << docstring(len, ch);
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
	return true;
}


void Tabular::plaintextPrintCell(odocstream & os,
			       OutputParams const & runparams,
			       idx_type cell, row_type row, col_type column,
			       vector<unsigned int> const & clen,
			       bool onlydata) const
{
	odocstringstream sstr;
	getCellInset(cell)->plaintext(sstr, runparams);

	if (onlydata) {
		os << sstr.str();
		return;
	}

	if (leftLine(cell))
		os << "| ";
	else
		os << "  ";

	unsigned int len1 = sstr.str().length();
	unsigned int len2 = clen[column];
	while (column < columnCount() -1
	       && isPartOfMultiColumn(row, ++column))
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

	os << docstring(len1, ' ') << sstr.str()
	   << docstring(len2, ' ');

	if (rightLine(cell))
		os << " |";
	else
		os << "  ";
}


void Tabular::plaintext(odocstream & os,
			   OutputParams const & runparams, int const depth,
			   bool onlydata, char_type delim) const
{
	// first calculate the width of the single columns
	vector<unsigned int> clen(columnCount());

	if (!onlydata) {
		// first all non (real) multicolumn cells!
		for (col_type j = 0; j < columnCount(); ++j) {
			clen[j] = 0;
			for (row_type i = 0; i < rowCount(); ++i) {
				idx_type cell = cellIndex(i, j);
				if (isMultiColumnReal(cell))
					continue;
				odocstringstream sstr;
				getCellInset(cell)->plaintext(sstr, runparams);
				if (clen[j] < sstr.str().length())
					clen[j] = sstr.str().length();
			}
		}
		// then all (real) multicolumn cells!
		for (col_type j = 0; j < columnCount(); ++j) {
			for (row_type i = 0; i < rowCount(); ++i) {
				idx_type cell = cellIndex(i, j);
				if (!isMultiColumnReal(cell) || isPartOfMultiColumn(i, j))
					continue;
				odocstringstream sstr;
				getCellInset(cell)->plaintext(sstr, runparams);
				int len = int(sstr.str().length());
				idx_type const n = columnSpan(cell);
				for (col_type k = j; len > 0 && k < j + n - 1; ++k)
					len -= clen[k];
				if (len > int(clen[j + n - 1]))
					clen[j + n - 1] = len;
			}
		}
	}
	idx_type cell = 0;
	for (row_type i = 0; i < rowCount(); ++i) {
		if (!onlydata && plaintextTopHLine(os, i, clen))
			os << docstring(depth * 2, ' ');
		for (col_type j = 0; j < columnCount(); ++j) {
			if (isPartOfMultiColumn(i, j))
				continue;
			if (onlydata && j > 0)
				// we don't use operator<< for single UCS4 character.
				// see explanation in docstream.h
				os.put(delim);
			plaintextPrintCell(os, runparams, cell, i, j, clen, onlydata);
			++cell;
		}
		os << endl;
		if (!onlydata) {
			os << docstring(depth * 2, ' ');
			if (plaintextBottomHLine(os, i, clen))
				os << docstring(depth * 2, ' ');
		}
	}
}


shared_ptr<InsetTableCell> Tabular::getCellInset(idx_type cell) const
{
	return cell_info[cellRow(cell)][cellColumn(cell)].inset;
}


shared_ptr<InsetTableCell> Tabular::getCellInset(row_type row,
					       col_type column) const
{
	return cell_info[row][column].inset;
}


void Tabular::setCellInset(row_type row, col_type column,
			      shared_ptr<InsetTableCell> ins) const
{
	CellData & cd = cell_info[row][column];
	cd.inset = ins;
	// reset the InsetTableCell's pointers
	ins->setCellData(&cd);
	ins->setTabular(this);
}


Tabular::idx_type
Tabular::getCellFromInset(Inset const * inset) const
{
	// is this inset part of the tabular?
	if (!inset) {
		lyxerr << "Error: this is not a cell of the tabular!" << endl;
		BOOST_ASSERT(false);
	}

	for (idx_type cell = 0, n = cellCount(); cell < n; ++cell)
		if (getCellInset(cell).get() == inset) {
			LYXERR(Debug::INSETTEXT, "Tabular::getCellFromInset: "
				<< "cell=" << cell);
			return cell;
		}

	// We should have found a cell at this point
	lyxerr << "Tabular::getCellFromInset: Cell of inset "
		<< inset << " not found!" << endl;
	BOOST_ASSERT(false);
	// shut up compiler
	return 0;
}


void Tabular::validate(LaTeXFeatures & features) const
{
	features.require("NeedTabularnewline");
	if (useBookTabs())
		features.require("booktabs");
	if (isLongTabular())
		features.require("longtable");
	if (needRotating())
		features.require("rotating");
	for (idx_type cell = 0; cell < cellCount(); ++cell) {
		if (getVAlignment(cell) != LYX_VALIGN_TOP ||
		     (!getPWidth(cell).zero() && !isMultiColumn(cell)))
			features.require("array");
		getCellInset(cell)->validate(features);
	}
}


Tabular::BoxType Tabular::useParbox(idx_type cell) const
{
	ParagraphList const & parlist = getCellInset(cell)->paragraphs();
	ParagraphList::const_iterator cit = parlist.begin();
	ParagraphList::const_iterator end = parlist.end();

	for (; cit != end; ++cit)
		for (int i = 0; i < cit->size(); ++i)
			if (cit->isNewline(i))
				return BOX_PARBOX;

	return BOX_NONE;
}


/////////////////////////////////////////////////////////////////////
//
// InsetTableCell
//
/////////////////////////////////////////////////////////////////////

InsetTableCell::InsetTableCell(Buffer const & buf,
	Tabular::CellData const * cell, Tabular const * table)
	: InsetText(buf), cell_data_(cell), table_(table)
{}


bool InsetTableCell::forceEmptyLayout(idx_type) const
{
	BOOST_ASSERT(table_);
	BOOST_ASSERT(cell_data_);
	return table_->getPWidth(cell_data_->cellno).zero();
}

bool InsetTableCell::allowParagraphCustomization(idx_type) const
{
	BOOST_ASSERT(table_);
	BOOST_ASSERT(cell_data_);
	return !table_->getPWidth(cell_data_->cellno).zero();
}

bool InsetTableCell::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	bool enabled;
	switch (cmd.action) {
	case LFUN_LAYOUT:
		enabled = !forceEmptyLayout();
		break;
	case LFUN_LAYOUT_PARAGRAPH:
		enabled = allowParagraphCustomization();
		break;
	default:
		return InsetText::getStatus(cur, cmd, status);
	}
	status.enabled(enabled);
	return true;
}

/////////////////////////////////////////////////////////////////////
//
// InsetTabular
//
/////////////////////////////////////////////////////////////////////

InsetTabular::InsetTabular(Buffer const & buf, row_type rows,
			   col_type columns)
	: tabular(buf, max(rows, row_type(1)), max(columns, col_type(1))), scx_(0)
{
	setBuffer(const_cast<Buffer &>(buf)); // FIXME: remove later
}


InsetTabular::InsetTabular(InsetTabular const & tab)
	: Inset(tab), tabular(tab.tabular),  scx_(0)
{
	setBuffer(const_cast<Buffer &>(tab.buffer())); // FIXME: remove later
}


InsetTabular::~InsetTabular()
{
	InsetTabularMailer(*this).hideDialog();
}


bool InsetTabular::insetAllowed(InsetCode code) const
{
	if (code == MATHMACRO_CODE)
		return false;

	return true;
}


void InsetTabular::write(ostream & os) const
{
	os << "Tabular" << endl;
	tabular.write(os);
}


void InsetTabular::read(Lexer & lex)
{
	bool const old_format = (lex.getString() == "\\LyXTable");

	tabular.read(lex);

	if (old_format)
		return;

	lex.next();
	string token = lex.getString();
	while (lex && token != "\\end_inset") {
		lex.next();
		token = lex.getString();
	}
	if (!lex) {
		lex.printError("Missing \\end_inset at this point. "
			       "Read: `$$Token'");
	}
}


int InsetTabular::rowFromY(Cursor & cur, int y) const
{
	// top y coordinate of tabular
	int h = yo(cur.bv()) - tabular.rowAscent(0);
	size_t nrows = tabular.rowCount();
	row_type r = 0;
	for (;r < nrows && y > h; ++r) {
		h += tabular.rowAscent(r);
		h += tabular.rowDescent(r);
		h += tabular.getAdditionalHeight(r);
	}
	return r - 1;
}


int InsetTabular::columnFromX(Cursor & cur, int x) const
{
	// left x coordinate of tabular
	int w = xo(cur.bv()) + ADD_TO_TABULAR_WIDTH;
	size_t ncols = tabular.columnCount();
	col_type c = 0;
	for (;c < ncols && x > w; ++c) {
		w += tabular.columnWidth(c);
	}
	return c - 1;
}


void InsetTabular::metrics(MetricsInfo & mi, Dimension & dim) const
{
	//lyxerr << "InsetTabular::metrics: " << mi.base.bv << " width: " <<
	//	mi.base.textwidth << "\n";
	if (!mi.base.bv) {
		lyxerr << "InsetTabular::metrics: need bv" << endl;
		BOOST_ASSERT(false);
	}

	row_type i = 0;
	for (idx_type cell = 0; i < tabular.rowCount(); ++i) {
		int maxAsc = 0;
		int maxDesc = 0;
		for (col_type j = 0; j < tabular.columnCount(); ++j) {
			if (tabular.isPartOfMultiColumn(i, j))
				// Multicolumn cell, but not first one
				continue;
			Dimension dim;
			MetricsInfo m = mi;
			Length p_width;
			if (tabular.cell_info[i][j].multicolumn ==
				Tabular::CELL_BEGIN_OF_MULTICOLUMN)
				p_width = tabular.cellinfo_of_cell(cell).p_width;
			else
				p_width = tabular.column_info[j].p_width;
			if (!p_width.zero())
				m.base.textwidth = p_width.inPixels(mi.base.textwidth);
			tabular.getCellInset(cell)->metrics(m, dim);
			if (!p_width.zero())
				dim.wid = m.base.textwidth;
			tabular.setCellWidth(cell, dim.wid);
			if (p_width.zero()) {
				m.base.textwidth = dim.wid + 2 * ADD_TO_TABULAR_WIDTH;
				// FIXME there must be a way to get rid of
				// the second metrics call
				tabular.getCellInset(cell)->metrics(m, dim);
			}
			maxAsc  = max(maxAsc, dim.asc);
			maxDesc = max(maxDesc, dim.des);
			++cell;
		}
		int const top_space = tabular.row_info[i].top_space_default ?
			default_line_space :
			tabular.row_info[i].top_space.inPixels(mi.base.textwidth);
		tabular.setRowAscent(i, maxAsc + ADD_TO_HEIGHT + top_space);
		int const bottom_space = tabular.row_info[i].bottom_space_default ?
			default_line_space :
			tabular.row_info[i].bottom_space.inPixels(mi.base.textwidth);
		tabular.setRowDescent(i, maxDesc + ADD_TO_HEIGHT + bottom_space);
	}
	tabular.updateColumnWidths();
	dim.asc = tabular.rowAscent(0);
	dim.des = tabular.height() - dim.asc;
	dim.wid = tabular.width() + 2 * ADD_TO_TABULAR_WIDTH;
}


void InsetTabular::draw(PainterInfo & pi, int x, int y) const
{
	//lyxerr << "InsetTabular::draw: " << x << " " << y << endl;
	BufferView * bv = pi.base.bv;

	// FIXME: As the full backrgound is painted in drawSelection(),
	// we have no choice but to do a full repaint for the Text cells.
	pi.full_repaint = true;

	resetPos(bv->cursor());

	x += scx_;
	x += ADD_TO_TABULAR_WIDTH;

	bool const original_drawing_state = pi.pain.isDrawingEnabled();

	idx_type idx = 0;
	first_visible_cell = Tabular::npos;
	for (row_type i = 0; i < tabular.rowCount(); ++i) {
		int nx = x;
		int const a = tabular.rowAscent(i);
		int const d = tabular.rowDescent(i);
		idx = tabular.cellIndex(i, 0);
		for (col_type j = 0; j < tabular.columnCount(); ++j) {
			if (tabular.isPartOfMultiColumn(i, j))
				continue;
			if (first_visible_cell == Tabular::npos)
				first_visible_cell = idx;

			int const cx = nx + tabular.getBeginningOfTextInCell(idx);
			// Cache the Inset position.
			bv->coordCache().insets().add(cell(idx).get(), cx, y);
			if (nx + tabular.columnWidth(idx) < 0
			    || nx > bv->workWidth()
			    || y + d < 0
			    || y - a > bv->workHeight()) {
				pi.pain.setDrawingEnabled(false);
				cell(idx)->draw(pi, cx, y);
				drawCellLines(pi.pain, nx, y, i, idx, pi.erased_);
				pi.pain.setDrawingEnabled(original_drawing_state);
			} else {
				cell(idx)->draw(pi, cx, y);
				drawCellLines(pi.pain, nx, y, i, idx, pi.erased_);
			}
			nx += tabular.columnWidth(idx);
			++idx;
		}

		if (i + 1 < tabular.rowCount())
			y += d + tabular.rowAscent(i + 1) +
				tabular.getAdditionalHeight(i + 1);
	}
}


void InsetTabular::drawSelection(PainterInfo & pi, int x, int y) const
{
	Cursor & cur = pi.base.bv->cursor();

	x += scx_ + ADD_TO_TABULAR_WIDTH;

	// FIXME: it is wrong to completely paint the background
	// if we want to do single row painting.

	// Paint background of current tabular
	int const w = tabular.width();
	int const h = tabular.height();
	int yy = y - tabular.rowAscent(0);
	pi.pain.fillRectangle(x, yy, w, h, backgroundColor());

	if (!cur.selection())
		return;
	if (&cur.inset() != this)
		return;

	//resetPos(cur);


	if (tablemode(cur)) {
		row_type rs, re;
		col_type cs, ce;
		getSelection(cur, rs, re, cs, ce);
		y -= tabular.rowAscent(0);
		for (row_type j = 0; j < tabular.rowCount(); ++j) {
			int const a = tabular.rowAscent(j);
			int const h = a + tabular.rowDescent(j);
			int xx = x;
			y += tabular.getAdditionalHeight(j);
			for (col_type i = 0; i < tabular.columnCount(); ++i) {
				if (tabular.isPartOfMultiColumn(j, i))
					continue;
				idx_type const cell =
					tabular.cellIndex(j, i);
				int const w = tabular.columnWidth(cell);
				if (i >= cs && i <= ce && j >= rs && j <= re)
					pi.pain.fillRectangle(xx, y, w, h,
							      Color_selection);
				xx += w;
			}
			y += h;
		}

	} else {
		x += getCellXPos(cur.idx());
		x += tabular.getBeginningOfTextInCell(cur.idx());
		cell(cur.idx())->drawSelection(pi, x, 0 /* ignored */);
	}
}


void InsetTabular::drawCellLines(Painter & pain, int x, int y,
				 row_type row, idx_type cell, bool erased) const
{
	int x2 = x + tabular.columnWidth(cell);
	bool on_off = false;
	ColorCode col = Color_tabularline;
	ColorCode onoffcol = Color_tabularonoffline;

	if (erased) {
		col = Color_deletedtext;
		onoffcol = Color_deletedtext;
	}

	if (!tabular.topAlreadyDrawn(cell)) {
		on_off = !tabular.topLine(cell);
		pain.line(x, y - tabular.rowAscent(row),
			  x2, y -  tabular.rowAscent(row),
			  on_off ? onoffcol : col,
			  on_off ? Painter::line_onoffdash : Painter::line_solid);
	}
	on_off = !tabular.bottomLine(cell);
	pain.line(x, y + tabular.rowDescent(row),
		  x2, y + tabular.rowDescent(row),
		  on_off ? onoffcol : col,
		  on_off ? Painter::line_onoffdash : Painter::line_solid);
	if (!tabular.leftAlreadyDrawn(cell)) {
		on_off = !tabular.leftLine(cell);
		pain.line(x, y -  tabular.rowAscent(row),
			  x, y +  tabular.rowDescent(row),
			  on_off ? onoffcol : col,
			  on_off ? Painter::line_onoffdash : Painter::line_solid);
	}
	on_off = !tabular.rightLine(cell);
	pain.line(x2 - tabular.getAdditionalWidth(cell),
		  y -  tabular.rowAscent(row),
		  x2 - tabular.getAdditionalWidth(cell),
		  y +  tabular.rowDescent(row),
		  on_off ? onoffcol : col,
		  on_off ? Painter::line_onoffdash : Painter::line_solid);
}


docstring InsetTabular::editMessage() const
{
	return _("Opened table");
}


void InsetTabular::edit(Cursor & cur, bool front, EntryDirection)
{
	//lyxerr << "InsetTabular::edit: " << this << endl;
	cur.finishUndo();
	cur.selection() = false;
	cur.push(*this);
	if (front) {
		if (isRightToLeft(cur))
			cur.idx() = tabular.getLastCellInRow(0);
		else
			cur.idx() = 0;
		cur.pit() = 0;
		cur.pos() = 0;
	} else {
		if (isRightToLeft(cur))
			cur.idx() = tabular.getFirstCellInRow(tabular.rowCount() - 1);
		else
			cur.idx() = tabular.cellCount() - 1;
		cur.pit() = 0;
		cur.pos() = cur.lastpos(); // FIXME crude guess
	}
	// FIXME: this accesses the position cache before it is initialized
	//resetPos(cur);
	//cur.bv().fitCursor();
}


void InsetTabular::updateLabels(ParIterator const & it)
{
	// In a longtable, tell captions what the current float is
	Counters & cnts = buffer().params().documentClass().counters();
	string const saveflt = cnts.current_float();
	if (tabular.isLongTabular())
		cnts.current_float("table");

	ParIterator it2 = it;
	it2.forwardPos();
	size_t const end = it2.nargs();
	for ( ; it2.idx() < end; it2.top().forwardIdx())
		lyx::updateLabels(buffer(), it2);

	//reset afterwards
	if (tabular.isLongTabular())
		cnts.current_float(saveflt);
}


void InsetTabular::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	LYXERR(Debug::DEBUG, "# InsetTabular::doDispatch: cmd: " << cmd
			     << "\n  cur:" << cur);
	CursorSlice sl = cur.top();
	Cursor & bvcur = cur.bv().cursor();

	switch (cmd.action) {

	case LFUN_MOUSE_PRESS: {
		//lyxerr << "# InsetTabular::MousePress\n" << cur.bv().cursor() << endl;

		// select row
		if (cmd.x < xo(cur.bv()) + ADD_TO_TABULAR_WIDTH
			|| cmd.x > xo(cur.bv()) + tabular.width()) {
			row_type r = rowFromY(cur, cmd.y);
			cur.idx() = tabular.getFirstCellInRow(r);
			cur.pos() = 0;
			cur.resetAnchor();
			cur.idx() = tabular.getLastCellInRow(r);
			cur.pos() = cur.lastpos();
			cur.selection() = true;
			bvcur = cur; 
			break;
		}
		// select column
		int const y0 = yo(cur.bv()) - tabular.rowAscent(0);
		if (cmd.y < y0 + ADD_TO_TABULAR_WIDTH 
			|| cmd.y > y0 + tabular.height()) {
			col_type c = columnFromX(cur, cmd.x);
			cur.idx() = tabular.cellIndex(0, c);
			cur.pos() = 0;
			cur.resetAnchor();
			cur.idx() = tabular.cellIndex(tabular.rowCount() - 1, c);
			cur.pos() = cur.lastpos();
			cur.selection() = true;
			bvcur = cur; 
			break;
		}
		// do not reset cursor/selection if we have selected
		// some cells (bug 2715).
		if (cmd.button() == mouse_button::button3
		    && &bvcur.selBegin().inset() == this 
		    && tablemode(bvcur)) 
			;
		else
			// Let InsetTableCell do it
			cell(cur.idx())->dispatch(cur, cmd);
		break;
	}
	case LFUN_MOUSE_MOTION:
		//lyxerr << "# InsetTabular::MouseMotion\n" << bvcur << endl;
		if (cmd.button() == mouse_button::button1) {
			// only accept motions to places not deeper nested than the real anchor
			if (!bvcur.anchor_.hasPart(cur)) {
				cur.undispatched();
				break;
			}
			// select (additional) row
			if (cmd.x < xo(cur.bv()) + ADD_TO_TABULAR_WIDTH
				|| cmd.x > xo(cur.bv()) + tabular.width()) {
				row_type r = rowFromY(cur, cmd.y);
				cur.idx() = tabular.getLastCellInRow(r);
				bvcur.setCursor(cur);
				bvcur.selection() = true;
				break;
			}
			// select (additional) column
			int const y0 = yo(cur.bv()) - tabular.rowAscent(0);
			if (cmd.y < y0 + ADD_TO_TABULAR_WIDTH 
				|| cmd.y > y0 + tabular.height()) {
				col_type c = columnFromX(cur, cmd.x);
				cur.idx() = tabular.cellIndex(tabular.rowCount() - 1, c);
				bvcur.setCursor(cur);
				bvcur.selection() = true;
				break;
			}
			// only update if selection changes
			if (bvcur.idx() == cur.idx() &&
				!(bvcur.anchor_.idx() == cur.idx() && bvcur.pos() != cur.pos()))
				cur.noUpdate();
			setCursorFromCoordinates(cur, cmd.x, cmd.y);
			bvcur.setCursor(cur);
			bvcur.selection() = true;
		}
		break;

	case LFUN_CELL_BACKWARD:
		movePrevCell(cur);
		cur.selection() = false;
		break;

	case LFUN_CELL_FORWARD:
		moveNextCell(cur);
		cur.selection() = false;
		break;
	case LFUN_CHAR_FORWARD_SELECT:
	case LFUN_CHAR_FORWARD:
		cell(cur.idx())->dispatch(cur, cmd);
		if (!cur.result().dispatched()) {
			moveNextCell(cur);
			if (sl == cur.top())
				cmd = FuncRequest(LFUN_FINISHED_FORWARD);
			else
				cur.dispatched();
		}
		break;

	case LFUN_CHAR_BACKWARD_SELECT:
	case LFUN_CHAR_BACKWARD:
		cell(cur.idx())->dispatch(cur, cmd);
		if (!cur.result().dispatched()) {
			movePrevCell(cur);
			if (sl == cur.top())
				cmd = FuncRequest(LFUN_FINISHED_BACKWARD);
			else
				cur.dispatched();
		}
		break;

	case LFUN_CHAR_RIGHT_SELECT:
	case LFUN_CHAR_RIGHT:
		//FIXME: for visual cursor, really move right
		if (isRightToLeft(cur))
			lyx::dispatch(FuncRequest(
				cmd.action == LFUN_CHAR_RIGHT_SELECT ?
					LFUN_CHAR_BACKWARD_SELECT : LFUN_CHAR_BACKWARD));
		else
			lyx::dispatch(FuncRequest(
				cmd.action == LFUN_CHAR_RIGHT_SELECT ?
					LFUN_CHAR_FORWARD_SELECT : LFUN_CHAR_FORWARD));
		break;

	case LFUN_CHAR_LEFT_SELECT:
	case LFUN_CHAR_LEFT:
		//FIXME: for visual cursor, really move left
		if (isRightToLeft(cur))
			lyx::dispatch(FuncRequest(
				cmd.action == LFUN_CHAR_LEFT_SELECT ?
					LFUN_CHAR_FORWARD_SELECT : LFUN_CHAR_FORWARD));
		else
			lyx::dispatch(FuncRequest(
				cmd.action == LFUN_CHAR_LEFT_SELECT ?
					LFUN_CHAR_BACKWARD_SELECT : LFUN_CHAR_BACKWARD));
		break;

	case LFUN_DOWN_SELECT:
	case LFUN_DOWN:
		cell(cur.idx())->dispatch(cur, cmd);
		cur.dispatched(); // override the cell's decision
		if (sl == cur.top())
			// if our Text didn't do anything to the cursor
			// then we try to put the cursor into the cell below
			// setting also the right targetX.
			if (tabular.cellRow(cur.idx()) != tabular.rowCount() - 1) {
				cur.idx() = tabular.getCellBelow(cur.idx());
				cur.pit() = 0;
				TextMetrics const & tm =
					cur.bv().textMetrics(cell(cur.idx())->getText(0));
				cur.pos() = tm.x2pos(cur.pit(), 0, cur.targetX());
			}
		if (sl == cur.top()) {
			// we trick it to go to forward after leaving the
			// tabular.
			cmd = FuncRequest(LFUN_FINISHED_FORWARD);
			cur.undispatched();
		}
		break;

	case LFUN_UP_SELECT:
	case LFUN_UP:
		cell(cur.idx())->dispatch(cur, cmd);
		cur.dispatched(); // override the cell's decision
		if (sl == cur.top())
			// if our Text didn't do anything to the cursor
			// then we try to put the cursor into the cell above
			// setting also the right targetX.
			if (tabular.cellRow(cur.idx()) != 0) {
				cur.idx() = tabular.getCellAbove(cur.idx());
				cur.pit() = cur.lastpit();
				Text const * text = cell(cur.idx())->getText(0);
				TextMetrics const & tm = cur.bv().textMetrics(text);
				ParagraphMetrics const & pm =
					tm.parMetrics(cur.lastpit());
				cur.pos() = tm.x2pos(cur.pit(), pm.rows().size()-1, cur.targetX());
			}
		if (sl == cur.top()) {
			cmd = FuncRequest(LFUN_UP);
			cur.undispatched();
		}
		break;

//	case LFUN_SCREEN_DOWN: {
//		//if (hasSelection())
//		//	cur.selection() = false;
//		col_type const col = tabular.cellColumn(cur.idx());
//		int const t =	cur.bv().top_y() + cur.bv().height();
//		if (t < yo() + tabular.getHeightOfTabular()) {
//			cur.bv().scrollDocView(t);
//			cur.idx() = tabular.getCellBelow(first_visible_cell) + col;
//		} else {
//			cur.idx() = tabular.getFirstCellInRow(tabular.rows() - 1) + col;
//		}
//		cur.par() = 0;
//		cur.pos() = 0;
//		break;
//	}
//
//	case LFUN_SCREEN_UP: {
//		//if (hasSelection())
//		//	cur.selection() = false;
//		col_type const col = tabular.cellColumn(cur.idx());
//		int const t =	cur.bv().top_y() + cur.bv().height();
//		if (yo() < 0) {
//			cur.bv().scrollDocView(t);
//			if (yo() > 0)
//				cur.idx() = col;
//			else
//				cur.idx() = tabular.getCellBelow(first_visible_cell) + col;
//		} else {
//			cur.idx() = col;
//		}
//		cur.par() = cur.lastpar();
//		cur.pos() = cur.lastpos();
//		break;
//	}

	case LFUN_LAYOUT_TABULAR:
		InsetTabularMailer(*this).showDialog(&cur.bv());
		break;

	case LFUN_INSET_DIALOG_UPDATE:
		InsetTabularMailer(*this).updateDialog(&cur.bv());
		break;

	case LFUN_TABULAR_FEATURE:
		if (!tabularFeatures(cur, to_utf8(cmd.argument())))
			cur.undispatched();
		break;

	// insert file functions
	case LFUN_FILE_INSERT_PLAINTEXT_PARA:
	case LFUN_FILE_INSERT_PLAINTEXT: {
		// FIXME UNICODE
		docstring const tmpstr = cur.bv().contentsOfPlaintextFile(
			FileName(to_utf8(cmd.argument())));
		if (tmpstr.empty())
			break;
		cur.recordUndoInset(INSERT_UNDO);
		if (insertPlaintextString(cur.bv(), tmpstr, false)) {
			// content has been replaced,
			// so cursor might be invalid
			cur.pos() = cur.lastpos();
			cur.pit() = cur.lastpit();
			bvcur.setCursor(cur);
		} else
			cur.undispatched();
		break;
	}

	case LFUN_CUT:
		if (tablemode(cur)) {
			if (copySelection(cur)) {
				cur.recordUndoInset(DELETE_UNDO);
				cutSelection(cur);
			}
		}
		else
			cell(cur.idx())->dispatch(cur, cmd);
		break;

	case LFUN_CHAR_DELETE_BACKWARD:
	case LFUN_CHAR_DELETE_FORWARD:
		if (tablemode(cur)) {
			cur.recordUndoInset(DELETE_UNDO);
			cutSelection(cur);
		}
		else
			cell(cur.idx())->dispatch(cur, cmd);
		break;

	case LFUN_COPY:
		if (!cur.selection())
			break;
		if (tablemode(cur)) {
			cur.finishUndo();
			copySelection(cur);
		} else
			cell(cur.idx())->dispatch(cur, cmd);
		break;

	case LFUN_CLIPBOARD_PASTE:
	case LFUN_PRIMARY_SELECTION_PASTE: {
		docstring const clip = (cmd.action == LFUN_CLIPBOARD_PASTE) ?
			theClipboard().getAsText() :
			theSelection().get();
		if (clip.empty())
			break;
		// pass to InsertPlaintextString, but
		// only if we have multi-cell content
		if (clip.find_first_of(from_ascii("\t\n")) != docstring::npos) {
			cur.recordUndoInset(INSERT_UNDO);
			if (insertPlaintextString(cur.bv(), clip, false)) {
				// content has been replaced,
				// so cursor might be invalid
				cur.pos() = cur.lastpos();
				cur.pit() = cur.lastpit();
				bvcur.setCursor(cur);
				break;
			}
		}
		// Let the cell handle normal text
		cell(cur.idx())->dispatch(cur, cmd);
		break;
	}

	case LFUN_PASTE:
		if (tabularStackDirty() && theClipboard().isInternal() ||
		    !theClipboard().hasInternal() && theClipboard().hasLyXContents()) {
			cur.recordUndoInset(INSERT_UNDO);
			pasteClipboard(cur);
			break;
		}
		cell(cur.idx())->dispatch(cur, cmd);
		break;

	case LFUN_FONT_EMPH:
	case LFUN_FONT_BOLD:
	case LFUN_FONT_ROMAN:
	case LFUN_FONT_NOUN:
	case LFUN_FONT_ITAL:
	case LFUN_FONT_FRAK:
	case LFUN_FONT_TYPEWRITER:
	case LFUN_FONT_SANS:
	case LFUN_FONT_FREE_APPLY:
	case LFUN_FONT_FREE_UPDATE:
	case LFUN_FONT_SIZE:
	case LFUN_FONT_UNDERLINE:
	case LFUN_LANGUAGE:
	case LFUN_WORD_CAPITALIZE:
	case LFUN_WORD_UPCASE:
	case LFUN_WORD_LOWCASE:
	case LFUN_CHARS_TRANSPOSE:
		if (tablemode(cur)) {
			row_type rs, re;
			col_type cs, ce;
			getSelection(cur, rs, re, cs, ce);
			Cursor tmpcur = cur;
			for (row_type i = rs; i <= re; ++i) {
				for (col_type j = cs; j <= ce; ++j) {
					// cursor follows cell:
					tmpcur.idx() = tabular.cellIndex(i, j);
					// select this cell only:
					tmpcur.pit() = 0;
					tmpcur.pos() = 0;
					tmpcur.resetAnchor();
					tmpcur.pit() = tmpcur.lastpit();
					tmpcur.pos() = tmpcur.top().lastpos();
					tmpcur.setCursor(tmpcur);
					tmpcur.setSelection();
					cell(tmpcur.idx())->dispatch(tmpcur, cmd);
				}
			}
			break;
		} else {
			cell(cur.idx())->dispatch(cur, cmd);
			break;
		}
	default:
		// we try to handle this event in the insets dispatch function.
		cell(cur.idx())->dispatch(cur, cmd);
		break;
	}
}


// function sets an object as defined in func_status.h:
// states OK, Unknown, Disabled, On, Off.
bool InsetTabular::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
	case LFUN_TABULAR_FEATURE: {
		int action = Tabular::LAST_ACTION;
		int i = 0;
		for (; tabularFeature[i].action != Tabular::LAST_ACTION; ++i) {
			string const tmp = tabularFeature[i].feature;
			if (tmp == to_utf8(cmd.argument()).substr(0, tmp.length())) {
				action = tabularFeature[i].action;
				break;
			}
		}
		if (action == Tabular::LAST_ACTION) {
			status.clear();
			status.unknown(true);
			return true;
		}

		string const argument
			= ltrim(to_utf8(cmd.argument()).substr(tabularFeature[i].feature.length()));

		row_type sel_row_start = 0;
		row_type sel_row_end = 0;
		col_type sel_col_start = 0;
		col_type sel_col_end = 0;
		Tabular::ltType dummyltt;
		bool flag = true;

		getSelection(cur, sel_row_start, sel_row_end, sel_col_start, sel_col_end);

		switch (action) {
		case Tabular::SET_PWIDTH:
		case Tabular::SET_MPWIDTH:
		case Tabular::SET_SPECIAL_COLUMN:
		case Tabular::SET_SPECIAL_MULTI:
		case Tabular::APPEND_ROW:
		case Tabular::APPEND_COLUMN:
		case Tabular::DELETE_ROW:
		case Tabular::DELETE_COLUMN:
		case Tabular::COPY_ROW:
		case Tabular::COPY_COLUMN:
		case Tabular::SET_ALL_LINES:
		case Tabular::UNSET_ALL_LINES:
		case Tabular::SET_TOP_SPACE:
		case Tabular::SET_BOTTOM_SPACE:
		case Tabular::SET_INTERLINE_SPACE:
		case Tabular::SET_BORDER_LINES:
			status.clear();
			return true;

		case Tabular::MULTICOLUMN:
			status.setOnOff(tabular.isMultiColumn(cur.idx()));
			break;

		case Tabular::TOGGLE_LINE_TOP:
			status.setOnOff(tabular.topLine(cur.idx()));
			break;

		case Tabular::TOGGLE_LINE_BOTTOM:
			status.setOnOff(tabular.bottomLine(cur.idx()));
			break;

		case Tabular::TOGGLE_LINE_LEFT:
			status.setOnOff(tabular.leftLine(cur.idx()));
			break;

		case Tabular::TOGGLE_LINE_RIGHT:
			status.setOnOff(tabular.rightLine(cur.idx()));
			break;

		case Tabular::M_ALIGN_LEFT:
			flag = false;
		case Tabular::ALIGN_LEFT:
			status.setOnOff(tabular.getAlignment(cur.idx(), flag) == LYX_ALIGN_LEFT);
			break;

		case Tabular::M_ALIGN_RIGHT:
			flag = false;
		case Tabular::ALIGN_RIGHT:
			status.setOnOff(tabular.getAlignment(cur.idx(), flag) == LYX_ALIGN_RIGHT);
			break;

		case Tabular::M_ALIGN_CENTER:
			flag = false;
		case Tabular::ALIGN_CENTER:
			status.setOnOff(tabular.getAlignment(cur.idx(), flag) == LYX_ALIGN_CENTER);
			break;

		case Tabular::ALIGN_BLOCK:
			status.enabled(!tabular.getPWidth(cur.idx()).zero());
			status.setOnOff(tabular.getAlignment(cur.idx(), flag) == LYX_ALIGN_BLOCK);
			break;

		case Tabular::M_VALIGN_TOP:
			flag = false;
		case Tabular::VALIGN_TOP:
			status.setOnOff(
				tabular.getVAlignment(cur.idx(), flag) == Tabular::LYX_VALIGN_TOP);
			break;

		case Tabular::M_VALIGN_BOTTOM:
			flag = false;
		case Tabular::VALIGN_BOTTOM:
			status.setOnOff(
				tabular.getVAlignment(cur.idx(), flag) == Tabular::LYX_VALIGN_BOTTOM);
			break;

		case Tabular::M_VALIGN_MIDDLE:
			flag = false;
		case Tabular::VALIGN_MIDDLE:
			status.setOnOff(
				tabular.getVAlignment(cur.idx(), flag) == Tabular::LYX_VALIGN_MIDDLE);
			break;

		case Tabular::SET_LONGTABULAR:
			status.setOnOff(tabular.isLongTabular());
			break;

		case Tabular::UNSET_LONGTABULAR:
			status.setOnOff(!tabular.isLongTabular());
			break;

		case Tabular::TOGGLE_ROTATE_TABULAR:
		case Tabular::SET_ROTATE_TABULAR:
			status.setOnOff(tabular.getRotateTabular());
			break;

		case Tabular::UNSET_ROTATE_TABULAR:
			status.setOnOff(!tabular.getRotateTabular());
			break;

		case Tabular::TOGGLE_ROTATE_CELL:
		case Tabular::SET_ROTATE_CELL:
			status.setOnOff(!oneCellHasRotationState(false,
				sel_row_start, sel_row_end, sel_col_start, sel_col_end));
			break;

		case Tabular::UNSET_ROTATE_CELL:
			status.setOnOff(!oneCellHasRotationState(true,
				sel_row_start, sel_row_end, sel_col_start, sel_col_end));
			break;

		case Tabular::SET_USEBOX:
			status.setOnOff(convert<int>(argument) == tabular.getUsebox(cur.idx()));
			break;

		case Tabular::SET_LTFIRSTHEAD:
			status.setOnOff(tabular.getRowOfLTHead(sel_row_start, dummyltt));
			break;

		case Tabular::UNSET_LTFIRSTHEAD:
			status.setOnOff(!tabular.getRowOfLTHead(sel_row_start, dummyltt));
			break;

		case Tabular::SET_LTHEAD:
			status.setOnOff(tabular.getRowOfLTHead(sel_row_start, dummyltt));
			break;

		case Tabular::UNSET_LTHEAD:
			status.setOnOff(!tabular.getRowOfLTHead(sel_row_start, dummyltt));
			break;

		case Tabular::SET_LTFOOT:
			status.setOnOff(tabular.getRowOfLTFoot(sel_row_start, dummyltt));
			break;

		case Tabular::UNSET_LTFOOT:
			status.setOnOff(!tabular.getRowOfLTFoot(sel_row_start, dummyltt));
			break;

		case Tabular::SET_LTLASTFOOT:
			status.setOnOff(tabular.getRowOfLTFoot(sel_row_start, dummyltt));
			break;

		case Tabular::UNSET_LTLASTFOOT:
			status.setOnOff(!tabular.getRowOfLTFoot(sel_row_start, dummyltt));
			break;

		case Tabular::SET_LTNEWPAGE:
			status.setOnOff(tabular.getLTNewPage(sel_row_start));
			break;

		case Tabular::SET_BOOKTABS:
			status.setOnOff(tabular.useBookTabs());
			break;

		case Tabular::UNSET_BOOKTABS:
			status.setOnOff(!tabular.useBookTabs());
			break;

		default:
			status.clear();
			status.enabled(false);
			break;
		}
		return true;
	}

	// These are only enabled inside tabular
	case LFUN_CELL_BACKWARD:
	case LFUN_CELL_FORWARD:
		status.enabled(true);
		return true;

	// disable these with multiple cells selected
	case LFUN_INSET_INSERT:
	case LFUN_TABULAR_INSERT:
	case LFUN_FLEX_INSERT:
	case LFUN_FLOAT_INSERT:
	case LFUN_FLOAT_WIDE_INSERT:
	case LFUN_FOOTNOTE_INSERT:
	case LFUN_MARGINALNOTE_INSERT:
	case LFUN_MATH_INSERT:
	case LFUN_MATH_MODE:
	case LFUN_MATH_MUTATE:
	case LFUN_MATH_DISPLAY:
	case LFUN_NOTE_INSERT:
	case LFUN_OPTIONAL_INSERT:
	case LFUN_BOX_INSERT:
	case LFUN_BRANCH_INSERT:
	case LFUN_WRAP_INSERT:
	case LFUN_ERT_INSERT: {
		if (tablemode(cur)) {
			status.enabled(false);
			return true;
		} else
			return cell(cur.idx())->getStatus(cur, cmd, status);
	}

	// disable in non-fixed-width cells
	case LFUN_NEW_LINE:
	case LFUN_BREAK_PARAGRAPH:
	case LFUN_BREAK_PARAGRAPH_SKIP: {
		if (tabular.getPWidth(cur.idx()).zero()) {
			status.enabled(false);
			return true;
		} else
			return cell(cur.idx())->getStatus(cur, cmd, status);
	}

	case LFUN_PASTE:
		if (tabularStackDirty() && theClipboard().isInternal()) {
			status.enabled(true);
			return true;
		} else
			return cell(cur.idx())->getStatus(cur, cmd, status);

	case LFUN_INSET_MODIFY:
		if (insetCode(cmd.getArg(0)) == TABULAR_CODE) {
			status.enabled(true);
			return true;
		}
		// Fall through

	default:
		// we try to handle this event in the insets dispatch function.
		return cell(cur.idx())->getStatus(cur, cmd, status);
	}
}


int InsetTabular::latex(odocstream & os, OutputParams const & runparams) const
{
	return tabular.latex(os, runparams);
}


int InsetTabular::plaintext(odocstream & os, OutputParams const & runparams) const
{
	os << '\n'; // output table on a new line
	int const dp = runparams.linelen > 0 ? runparams.depth : 0;
	tabular.plaintext(os, runparams, dp, false, 0);
	return PLAINTEXT_NEWLINE;
}


int InsetTabular::docbook(odocstream & os, OutputParams const & runparams) const
{
	int ret = 0;
	Inset * master = 0;

	// FIXME: Why not pass a proper DocIterator here?
#if 0
	// if the table is inside a float it doesn't need the informaltable
	// wrapper. Search for it.
	for (master = owner(); master; master = master->owner())
		if (master->lyxCode() == FLOAT_CODE)
			break;
#endif

	if (!master) {
		os << "<informaltable>";
		++ret;
	}
	ret += tabular.docbook(os, runparams);
	if (!master) {
		os << "</informaltable>";
		++ret;
	}
	return ret;
}


void InsetTabular::validate(LaTeXFeatures & features) const
{
	tabular.validate(features);
}


shared_ptr<InsetTableCell const> InsetTabular::cell(idx_type idx) const
{
	return tabular.getCellInset(idx);
}


shared_ptr<InsetTableCell> InsetTabular::cell(idx_type idx)
{
	return tabular.getCellInset(idx);
}


void InsetTabular::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const
{
	cell(sl.idx())->cursorPos(bv, sl, boundary, x, y);

	// y offset	correction
	int const row = tabular.cellRow(sl.idx());
	for (int i = 0;	i <= row; ++i) {
		if (i != 0) {
			y += tabular.rowAscent(i);
			y += tabular.getAdditionalHeight(i);
		}
		if (i != row)
			y += tabular.rowDescent(i);
	}

	// x offset correction
	int const col = tabular.cellColumn(sl.idx());
	int idx = tabular.cellIndex(row, 0);
	for (int j = 0; j < col; ++j) {
		if (tabular.isPartOfMultiColumn(row, j))
			continue;
		x += tabular.columnWidth(idx);
		++idx;
	}
	x += tabular.getBeginningOfTextInCell(idx);
	x += ADD_TO_TABULAR_WIDTH;
	x += scx_;
}


int InsetTabular::dist(BufferView & bv, idx_type const cell, int x, int y) const
{
	int xx = 0;
	int yy = 0;
	Inset const & inset = *tabular.getCellInset(cell);
	Point o = bv.coordCache().getInsets().xy(&inset);
	int const xbeg = o.x_ - tabular.getBeginningOfTextInCell(cell);
	int const xend = xbeg + tabular.columnWidth(cell);
	row_type const row = tabular.cellRow(cell);
	int const ybeg = o.y_ - tabular.rowAscent(row) -
			 tabular.getAdditionalHeight(row);
	int const yend = o.y_ + tabular.rowDescent(row);

	if (x < xbeg)
		xx = xbeg - x;
	else if (x > xend)
		xx = x - xend;

	if (y < ybeg)
		yy = ybeg - y;
	else if (y > yend)
		yy = y - yend;

	//lyxerr << " xbeg=" << xbeg << "  xend=" << xend
	//       << " ybeg=" << ybeg << " yend=" << yend
	//       << " xx=" << xx << " yy=" << yy
	//       << " dist=" << xx + yy << endl;
	return xx + yy;
}


Inset * InsetTabular::editXY(Cursor & cur, int x, int y)
{
	//lyxerr << "InsetTabular::editXY: " << this << endl;
	cur.selection() = false;
	cur.push(*this);
	cur.idx() = getNearestCell(cur.bv(), x, y);
	resetPos(cur);
	return cur.bv().textMetrics(&cell(cur.idx())->text_).editXY(cur, x, y);
}


void InsetTabular::setCursorFromCoordinates(Cursor & cur, int x, int y) const
{
	cur.idx() = getNearestCell(cur.bv(), x, y);
	cur.bv().textMetrics(&cell(cur.idx())->text_).setCursorFromCoordinates(cur, x, y);
}


InsetTabular::idx_type InsetTabular::getNearestCell(BufferView & bv, int x, int y) const
{
	idx_type idx_min = 0;
	int dist_min = numeric_limits<int>::max();
	for (idx_type i = 0, n = nargs(); i != n; ++i) {
		if (bv.coordCache().getInsets().has(tabular.getCellInset(i).get())) {
			int const d = dist(bv, i, x, y);
			if (d < dist_min) {
				dist_min = d;
				idx_min = i;
			}
		}
	}
	return idx_min;
}


int InsetTabular::getCellXPos(idx_type const cell) const
{
	idx_type c = cell;

	for (; !tabular.isFirstCellInRow(c); --c)
		;
	int lx = 0;
	for (; c < cell; ++c)
		lx += tabular.columnWidth(c);

	return lx;
}


void InsetTabular::resetPos(Cursor & cur) const
{
	BufferView & bv = cur.bv();
	int const maxwidth = bv.workWidth();

	if (&cur.inset() != this) {
		scx_ = 0;
	} else {
		int const X1 = 0;
		int const X2 = maxwidth;
		int const offset = ADD_TO_TABULAR_WIDTH + 2;
		int const x1 = xo(cur.bv()) + getCellXPos(cur.idx()) + offset;
		int const x2 = x1 + tabular.columnWidth(cur.idx());

		if (x1 < X1)
			scx_ = X1 + 20 - x1;
		else if (x2 > X2)
			scx_ = X2 - 20 - x2;
		else
			scx_ = 0;
	}

	cur.updateFlags(Update::Force | Update::FitCursor);
}


void InsetTabular::moveNextCell(Cursor & cur)
{
	if (isRightToLeft(cur)) {
		if (tabular.isFirstCellInRow(cur.idx())) {
			row_type const row = tabular.cellRow(cur.idx());
			if (row == tabular.rowCount() - 1)
				return;
			cur.idx() = tabular.getCellBelow(tabular.getLastCellInRow(row));
		} else {
			if (cur.idx() == 0)
				return;
			--cur.idx();
		}
	} else {
		if (tabular.isLastCell(cur.idx()))
			return;
		++cur.idx();
	}
	cur.pit() = 0;
	cur.pos() = 0;
	resetPos(cur);
}


void InsetTabular::movePrevCell(Cursor & cur)
{
	if (isRightToLeft(cur)) {
		if (tabular.isLastCellInRow(cur.idx())) {
			row_type const row = tabular.cellRow(cur.idx());
			if (row == 0)
				return;
			cur.idx() = tabular.getFirstCellInRow(row);
			cur.idx() = tabular.getCellAbove(cur.idx());
		} else {
			if (tabular.isLastCell(cur.idx()))
				return;
			++cur.idx();
		}
	} else {
		if (cur.idx() == 0) // first cell
			return;
		--cur.idx();
	}
	cur.pit() = cur.lastpit();
	cur.pos() = cur.lastpos();

	// FIXME: this accesses the position cache before it is initialized
	//resetPos(cur);
}


bool InsetTabular::tabularFeatures(Cursor & cur, string const & what)
{
	Tabular::Feature action = Tabular::LAST_ACTION;

	int i = 0;
	for (; tabularFeature[i].action != Tabular::LAST_ACTION; ++i) {
		string const tmp = tabularFeature[i].feature;

		if (tmp == what.substr(0, tmp.length())) {
			//if (!compare(tabularFeatures[i].feature.c_str(), what.c_str(),
			//tabularFeatures[i].feature.length()))
			action = tabularFeature[i].action;
			break;
		}
	}
	if (action == Tabular::LAST_ACTION)
		return false;

	string const val =
		ltrim(what.substr(tabularFeature[i].feature.length()));
	tabularFeatures(cur, action, val);
	return true;
}


static void checkLongtableSpecial(Tabular::ltType & ltt,
			  string const & special, bool & flag)
{
	if (special == "dl_above") {
		ltt.topDL = flag;
		ltt.set = false;
	} else if (special == "dl_below") {
		ltt.bottomDL = flag;
		ltt.set = false;
	} else if (special == "empty") {
		ltt.empty = flag;
		ltt.set = false;
	} else if (flag) {
		ltt.empty = false;
		ltt.set = true;
	}
}

bool InsetTabular::oneCellHasRotationState(bool rotated,
		row_type row_start, row_type row_end,
		col_type col_start, col_type col_end) const {

	for (row_type i = row_start; i <= row_end; ++i) {
		for (col_type j = col_start; j <= col_end; ++j) {
			if (tabular.getRotateCell(tabular.cellIndex(i, j))
				== rotated) {
				return true;
			}
		}
	}
	return false;
}

void InsetTabular::tabularFeatures(Cursor & cur,
	Tabular::Feature feature, string const & value)
{
	col_type sel_col_start;
	col_type sel_col_end;
	row_type sel_row_start;
	row_type sel_row_end;
	bool setLines = false;
	LyXAlignment setAlign = LYX_ALIGN_LEFT;
	Tabular::VAlignment setVAlign = Tabular::LYX_VALIGN_TOP;

	switch (feature) {

	case Tabular::M_ALIGN_LEFT:
	case Tabular::ALIGN_LEFT:
		setAlign = LYX_ALIGN_LEFT;
		break;

	case Tabular::M_ALIGN_RIGHT:
	case Tabular::ALIGN_RIGHT:
		setAlign = LYX_ALIGN_RIGHT;
		break;

	case Tabular::M_ALIGN_CENTER:
	case Tabular::ALIGN_CENTER:
		setAlign = LYX_ALIGN_CENTER;
		break;

	case Tabular::ALIGN_BLOCK:
		setAlign = LYX_ALIGN_BLOCK;
		break;

	case Tabular::M_VALIGN_TOP:
	case Tabular::VALIGN_TOP:
		setVAlign = Tabular::LYX_VALIGN_TOP;
		break;

	case Tabular::M_VALIGN_BOTTOM:
	case Tabular::VALIGN_BOTTOM:
		setVAlign = Tabular::LYX_VALIGN_BOTTOM;
		break;

	case Tabular::M_VALIGN_MIDDLE:
	case Tabular::VALIGN_MIDDLE:
		setVAlign = Tabular::LYX_VALIGN_MIDDLE;
		break;

	default:
		break;
	}

	cur.recordUndoInset(ATOMIC_UNDO);

	getSelection(cur, sel_row_start, sel_row_end, sel_col_start, sel_col_end);
	row_type const row = tabular.cellRow(cur.idx());
	col_type const column = tabular.cellColumn(cur.idx());
	bool flag = true;
	Tabular::ltType ltt;

	switch (feature) {

	case Tabular::SET_PWIDTH: {
		Length const len(value);
		tabular.setColumnPWidth(cur, cur.idx(), len);
		if (len.zero()
		    && tabular.getAlignment(cur.idx(), true) == LYX_ALIGN_BLOCK)
			tabularFeatures(cur, Tabular::ALIGN_CENTER, string());
		break;
	}

	case Tabular::SET_MPWIDTH:
		tabular.setMColumnPWidth(cur, cur.idx(), Length(value));
		break;

	case Tabular::SET_SPECIAL_COLUMN:
	case Tabular::SET_SPECIAL_MULTI:
		tabular.setAlignSpecial(cur.idx(), from_utf8(value), feature);
		break;

	case Tabular::APPEND_ROW:
		// append the row into the tabular
		tabular.appendRow(cur.idx());
		break;

	case Tabular::APPEND_COLUMN:
		// append the column into the tabular
		tabular.appendColumn(cur.idx());
		cur.idx() = tabular.cellIndex(row, column);
		break;

	case Tabular::DELETE_ROW:
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			tabular.deleteRow(sel_row_start);
		if (sel_row_start >= tabular.rowCount())
			--sel_row_start;
		cur.idx() = tabular.cellIndex(sel_row_start, column);
		cur.pit() = 0;
		cur.pos() = 0;
		cur.selection() = false;
		break;

	case Tabular::DELETE_COLUMN:
		for (col_type i = sel_col_start; i <= sel_col_end; ++i)
			tabular.deleteColumn(sel_col_start);
		if (sel_col_start >= tabular.columnCount())
			--sel_col_start;
		cur.idx() = tabular.cellIndex(row, sel_col_start);
		cur.pit() = 0;
		cur.pos() = 0;
		cur.selection() = false;
		break;

	case Tabular::COPY_ROW:
		tabular.copyRow(row);
		break;

	case Tabular::COPY_COLUMN:
		tabular.copyColumn(column);
		cur.idx() = tabular.cellIndex(row, column);
		break;

	case Tabular::TOGGLE_LINE_TOP: {
		bool lineSet = !tabular.topLine(cur.idx());
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setTopLine(tabular.cellIndex(i, j), lineSet);
		break;
	}

	case Tabular::TOGGLE_LINE_BOTTOM: {
		bool lineSet = !tabular.bottomLine(cur.idx());
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setBottomLine(tabular.cellIndex(i, j), lineSet);
		break;
	}

	case Tabular::TOGGLE_LINE_LEFT: {
		bool lineSet = !tabular.leftLine(cur.idx());
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setLeftLine(tabular.cellIndex(i, j), lineSet);
		break;
	}

	case Tabular::TOGGLE_LINE_RIGHT: {
		bool lineSet = !tabular.rightLine(cur.idx());
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setRightLine(tabular.cellIndex(i, j), lineSet);
		break;
	}

	case Tabular::M_ALIGN_LEFT:
	case Tabular::M_ALIGN_RIGHT:
	case Tabular::M_ALIGN_CENTER:
		flag = false;
	case Tabular::ALIGN_LEFT:
	case Tabular::ALIGN_RIGHT:
	case Tabular::ALIGN_CENTER:
	case Tabular::ALIGN_BLOCK:
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setAlignment(tabular.cellIndex(i, j), setAlign, flag);
		break;

	case Tabular::M_VALIGN_TOP:
	case Tabular::M_VALIGN_BOTTOM:
	case Tabular::M_VALIGN_MIDDLE:
		flag = false;
	case Tabular::VALIGN_TOP:
	case Tabular::VALIGN_BOTTOM:
	case Tabular::VALIGN_MIDDLE:
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setVAlignment(tabular.cellIndex(i, j), setVAlign, flag);
		break;

	case Tabular::MULTICOLUMN: {
		if (sel_row_start != sel_row_end) {
			// FIXME: Need I say it ? This is horrible.
			// FIXME UNICODE
			Alert::error(_("Error setting multicolumn"),
				     _("You cannot set multicolumn vertically."));
			return;
		}
		if (!cur.selection()) {
			// just multicol for one single cell
			// check whether we are completely in a multicol
			if (tabular.isMultiColumn(cur.idx()))
				tabular.unsetMultiColumn(cur.idx());
			else
				tabular.setMultiColumn(cur.idx(), 1);
			break;
		}
		// we have a selection so this means we just add all this
		// cells to form a multicolumn cell
		idx_type const s_start = cur.selBegin().idx();
		idx_type const s_end = cur.selEnd().idx();
		tabular.setMultiColumn(s_start, s_end - s_start + 1);
		cur.idx() = s_start;
		cur.pit() = 0;
		cur.pos() = 0;
		cur.selection() = false;
		break;
	}

	case Tabular::SET_ALL_LINES:
		setLines = true;
	case Tabular::UNSET_ALL_LINES:
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setAllLines(
					tabular.cellIndex(i,j), setLines);
		break;

	case Tabular::SET_BORDER_LINES:
		for (row_type i = sel_row_start; i <= sel_row_end; ++i) {
			tabular.setLeftLine(tabular.cellIndex(i, sel_col_start), true);
			tabular.setRightLine(tabular.cellIndex(i, sel_col_end), true);
		}
		for (col_type j = sel_col_start; j <= sel_col_end; ++j) {
			tabular.setTopLine(tabular.cellIndex(sel_row_start, j), true);
			tabular.setBottomLine(tabular.cellIndex(sel_row_end, j), true);
		}
		break;

	case Tabular::SET_LONGTABULAR:
		tabular.setLongTabular(true);
		break;

	case Tabular::UNSET_LONGTABULAR:
		tabular.setLongTabular(false);
		break;

	case Tabular::SET_ROTATE_TABULAR:
		tabular.setRotateTabular(true);
		break;

	case Tabular::UNSET_ROTATE_TABULAR:
		tabular.setRotateTabular(false);
		break;

	case Tabular::TOGGLE_ROTATE_TABULAR:
		tabular.setRotateTabular(!tabular.getRotateTabular());
		break;

	case Tabular::SET_ROTATE_CELL:
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setRotateCell(
					tabular.cellIndex(i, j), true);
		break;

	case Tabular::UNSET_ROTATE_CELL:
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setRotateCell(tabular.cellIndex(i, j), false);
		break;

	case Tabular::TOGGLE_ROTATE_CELL:
		{
		bool oneNotRotated = oneCellHasRotationState(false,
			sel_row_start, sel_row_end, sel_col_start, sel_col_end);

		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setRotateCell(tabular.cellIndex(i, j),
									  oneNotRotated);
		}
		break;

	case Tabular::SET_USEBOX: {
		Tabular::BoxType val = Tabular::BoxType(convert<int>(value));
		if (val == tabular.getUsebox(cur.idx()))
			val = Tabular::BOX_NONE;
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setUsebox(tabular.cellIndex(i, j), val);
		break;
	}

	case Tabular::UNSET_LTFIRSTHEAD:
		flag = false;
	case Tabular::SET_LTFIRSTHEAD:
		tabular.getRowOfLTFirstHead(row, ltt);
		checkLongtableSpecial(ltt, value, flag);
		tabular.setLTHead(row, flag, ltt, true);
		break;

	case Tabular::UNSET_LTHEAD:
		flag = false;
	case Tabular::SET_LTHEAD:
		tabular.getRowOfLTHead(row, ltt);
		checkLongtableSpecial(ltt, value, flag);
		tabular.setLTHead(row, flag, ltt, false);
		break;

	case Tabular::UNSET_LTFOOT:
		flag = false;
	case Tabular::SET_LTFOOT:
		tabular.getRowOfLTFoot(row, ltt);
		checkLongtableSpecial(ltt, value, flag);
		tabular.setLTFoot(row, flag, ltt, false);
		break;

	case Tabular::UNSET_LTLASTFOOT:
		flag = false;
	case Tabular::SET_LTLASTFOOT:
		tabular.getRowOfLTLastFoot(row, ltt);
		checkLongtableSpecial(ltt, value, flag);
		tabular.setLTFoot(row, flag, ltt, true);
		break;

	case Tabular::SET_LTNEWPAGE:
		tabular.setLTNewPage(row, !tabular.getLTNewPage(row));
		break;

	case Tabular::SET_BOOKTABS:
		tabular.setBookTabs(true);
		break;

	case Tabular::UNSET_BOOKTABS:
		tabular.setBookTabs(false);
		break;

	case Tabular::SET_TOP_SPACE: {
		Length len;
		if (value == "default")
			for (row_type i = sel_row_start; i <= sel_row_end; ++i)
				tabular.row_info[i].top_space_default = true;
		else if (isValidLength(value, &len))
			for (row_type i = sel_row_start; i <= sel_row_end; ++i) {
				tabular.row_info[i].top_space_default = false;
				tabular.row_info[i].top_space = len;
			}
		else
			for (row_type i = sel_row_start; i <= sel_row_end; ++i) {
				tabular.row_info[i].top_space_default = false;
				tabular.row_info[i].top_space = len;
			}
		break;
	}

	case Tabular::SET_BOTTOM_SPACE: {
		Length len;
		if (value == "default")
			for (row_type i = sel_row_start; i <= sel_row_end; ++i)
				tabular.row_info[i].bottom_space_default = true;
		else if (isValidLength(value, &len))
			for (row_type i = sel_row_start; i <= sel_row_end; ++i) {
				tabular.row_info[i].bottom_space_default = false;
				tabular.row_info[i].bottom_space = len;
			}
		else
			for (row_type i = sel_row_start; i <= sel_row_end; ++i) {
				tabular.row_info[i].bottom_space_default = false;
				tabular.row_info[i].bottom_space = len;
			}
		break;
	}

	case Tabular::SET_INTERLINE_SPACE: {
		Length len;
		if (value == "default")
			for (row_type i = sel_row_start; i <= sel_row_end; ++i)
				tabular.row_info[i].interline_space_default = true;
		else if (isValidLength(value, &len))
			for (row_type i = sel_row_start; i <= sel_row_end; ++i) {
				tabular.row_info[i].interline_space_default = false;
				tabular.row_info[i].interline_space = len;
			}
		else
			for (row_type i = sel_row_start; i <= sel_row_end; ++i) {
				tabular.row_info[i].interline_space_default = false;
				tabular.row_info[i].interline_space = len;
			}
		break;
	}

	// dummy stuff just to avoid warnings
	case Tabular::LAST_ACTION:
		break;
	}
}


bool InsetTabular::showInsetDialog(BufferView * bv) const
{
	InsetTabularMailer(*this).showDialog(bv);
	return true;
}


void InsetTabular::openLayoutDialog(BufferView * bv) const
{
	InsetTabularMailer(*this).showDialog(bv);
}


bool InsetTabular::copySelection(Cursor & cur)
{
	if (!cur.selection())
		return false;

	row_type rs, re;
	col_type cs, ce;
	getSelection(cur, rs, re, cs, ce);

	paste_tabular.reset(new Tabular(tabular));

	for (row_type i = 0; i < rs; ++i)
		paste_tabular->deleteRow(0);

	row_type const rows = re - rs + 1;
	while (paste_tabular->rowCount() > rows)
		paste_tabular->deleteRow(rows);

	for (col_type i = 0; i < cs; ++i)
		paste_tabular->deleteColumn(0);

	col_type const columns = ce - cs + 1;
	while (paste_tabular->columnCount() > columns)
		paste_tabular->deleteColumn(columns);

	// We clear all the InsetTableCell pointers, since they
	// might now become invalid and there is no point in having
	// them point to temporary things in paste_tabular.
	for (row_type i = 0; i < paste_tabular->rowCount(); ++i)
		for (col_type j = 0; j < paste_tabular->columnCount(); ++j) {
			paste_tabular->getCellInset(i,j)->setCellData(0);
			paste_tabular->getCellInset(i,j)->setTabular(0);
		}

	odocstringstream os;
	OutputParams const runparams(0);
	paste_tabular->plaintext(os, runparams, 0, true, '\t');
	// Needed for the "Edit->Paste recent" menu and the system clipboard.
	cap::copySelection(cur, os.str());

	// mark tabular stack dirty
	// FIXME: this is a workaround for bug 1919. Should be removed for 1.5,
	// when we (hopefully) have a one-for-all paste mechanism.
	// This must be called after cap::copySelection.
	dirtyTabularStack(true);

	return true;
}


bool InsetTabular::pasteClipboard(Cursor & cur)
{
	if (!paste_tabular)
		return false;
	col_type const actcol = tabular.cellColumn(cur.idx());
	row_type const actrow = tabular.cellRow(cur.idx());
	for (row_type r1 = 0, r2 = actrow;
	     r1 < paste_tabular->rowCount() && r2 < tabular.rowCount();
	     ++r1, ++r2) {
		for (col_type c1 = 0, c2 = actcol;
		    c1 < paste_tabular->columnCount() && c2 < tabular.columnCount();
		    ++c1, ++c2) {
			if (paste_tabular->isPartOfMultiColumn(r1, c1) &&
			    tabular.isPartOfMultiColumn(r2, c2))
				continue;
			if (paste_tabular->isPartOfMultiColumn(r1, c1)) {
				--c2;
				continue;
			}
			if (tabular.isPartOfMultiColumn(r2, c2)) {
				--c1;
				continue;
			}
			shared_ptr<InsetTableCell> inset(
				new InsetTableCell(*paste_tabular->getCellInset(r1, c1)));
			// note that setCellInset will call InsetTableCell::setCellData()
			// and InsetTableCell::setTabular()
			tabular.setCellInset(r2, c2, inset);
			// FIXME: change tracking (MG)
			inset->setChange(Change(cur.buffer().params().trackChanges ?
						Change::INSERTED : Change::UNCHANGED));
			cur.pos() = 0;
		}
	}
	return true;
}


void InsetTabular::cutSelection(Cursor & cur)
{
	if (!cur.selection())
		return;

	row_type rs, re;
	col_type cs, ce;
	getSelection(cur, rs, re, cs, ce);
	for (row_type i = rs; i <= re; ++i) {
		for (col_type j = cs; j <= ce; ++j) {
			shared_ptr<InsetTableCell> t
				= cell(tabular.cellIndex(i, j));
			if (cur.buffer().params().trackChanges)
				// FIXME: Change tracking (MG)
				t->setChange(Change(Change::DELETED));
			else
				t->clear();
		}
	}

	// cursor position might be invalid now
	if (cur.pit() > cur.lastpit())
		cur.pit() = cur.lastpit();
	if (cur.pos() > cur.lastpos())
		cur.pos() = cur.lastpos();
	cur.clearSelection();
}


bool InsetTabular::isRightToLeft(Cursor & cur) const
{
	BOOST_ASSERT(cur.depth() > 1);
	Paragraph const & parentpar = cur[cur.depth() - 2].paragraph();
	pos_type const parentpos = cur[cur.depth() - 2].pos();
	return parentpar.getFontSettings(cur.bv().buffer().params(),
					 parentpos).language()->rightToLeft();
}


void InsetTabular::getSelection(Cursor & cur,
	row_type & rs, row_type & re, col_type & cs, col_type & ce) const
{
	CursorSlice const & beg = cur.selBegin();
	CursorSlice const & end = cur.selEnd();
	cs = tabular.cellColumn(beg.idx());
	ce = tabular.cellColumn(end.idx());
	if (cs > ce) {
		ce = cs;
		cs = tabular.cellColumn(end.idx());
	} else {
		ce = tabular.cellRightColumn(end.idx());
	}

	rs = tabular.cellRow(beg.idx());
	re = tabular.cellRow(end.idx());
	if (rs > re)
		swap(rs, re);
}


Text * InsetTabular::getText(int idx) const
{
	return size_t(idx) < nargs() ? cell(idx)->getText(0) : 0;
}


void InsetTabular::setChange(Change const & change)
{
	for (idx_type idx = 0; idx < nargs(); ++idx)
		cell(idx)->setChange(change);
}


void InsetTabular::acceptChanges(BufferParams const & bparams)
{
	for (idx_type idx = 0; idx < nargs(); ++idx)
		cell(idx)->acceptChanges(bparams);
}


void InsetTabular::rejectChanges(BufferParams const & bparams)
{
	for (idx_type idx = 0; idx < nargs(); ++idx)
		cell(idx)->rejectChanges(bparams);
}


bool InsetTabular::allowParagraphCustomization(idx_type cell) const
{
	return tabular.getPWidth(cell).zero();
}


bool InsetTabular::forceEmptyLayout(idx_type cell) const
{
	return !tabular.getPWidth(cell).zero();
}


bool InsetTabular::insertPlaintextString(BufferView & bv, docstring const & buf,
				     bool usePaste)
{
	if (buf.length() <= 0)
		return true;

	col_type cols = 1;
	row_type rows = 1;
	col_type maxCols = 1;
	size_t const len = buf.length();
	size_t p = 0;

	while (p < len &&
	       (p = buf.find_first_of(from_ascii("\t\n"), p)) != docstring::npos) {
		switch (buf[p]) {
		case '\t':
			++cols;
			break;
		case '\n':
			if (p + 1 < len)
				++rows;
			maxCols = max(cols, maxCols);
			cols = 1;
			break;
		}
		++p;
	}
	maxCols = max(cols, maxCols);
	Tabular * loctab;
	idx_type cell = 0;
	col_type ocol = 0;
	row_type row = 0;
	if (usePaste) {
		paste_tabular.reset(new Tabular(buffer(), rows, maxCols));
		loctab = paste_tabular.get();
		cols = 0;
		dirtyTabularStack(true);
	} else {
		loctab = &tabular;
		cell = bv.cursor().idx();
		ocol = tabular.cellColumn(cell);
		row = tabular.cellRow(cell);
	}

	size_t op = 0;
	idx_type const cells = loctab->cellCount();
	p = 0;
	cols = ocol;
	rows = loctab->rowCount();
	col_type const columns = loctab->columnCount();

	while (cell < cells && p < len && row < rows &&
	       (p = buf.find_first_of(from_ascii("\t\n"), p)) != docstring::npos)
	{
		if (p >= len)
			break;
		switch (buf[p]) {
		case '\t':
			// we can only set this if we are not too far right
			if (cols < columns) {
				shared_ptr<InsetTableCell> inset = loctab->getCellInset(cell);
				Font const font = bv.textMetrics(&inset->text_).
					displayFont(0, 0);
				inset->setText(buf.substr(op, p - op), font,
					       buffer().params().trackChanges);
				++cols;
				++cell;
			}
			break;
		case '\n':
			// we can only set this if we are not too far right
			if (cols < columns) {
				shared_ptr<InsetTableCell> inset = tabular.getCellInset(cell);
				Font const font = bv.textMetrics(&inset->text_).
					displayFont(0, 0);
				inset->setText(buf.substr(op, p - op), font,
					       buffer().params().trackChanges);
			}
			cols = ocol;
			++row;
			if (row < rows)
				cell = loctab->cellIndex(row, cols);
			break;
		}
		++p;
		op = p;
	}
	// check for the last cell if there is no trailing '\n'
	if (cell < cells && op < len) {
		shared_ptr<InsetTableCell> inset = loctab->getCellInset(cell);
		Font const font = bv.textMetrics(&inset->text_).displayFont(0, 0);
		inset->setText(buf.substr(op, len - op), font,
			buffer().params().trackChanges);
	}
	return true;
}


void InsetTabular::addPreview(PreviewLoader & loader) const
{
	row_type const rows = tabular.rowCount();
	col_type const columns = tabular.columnCount();
	for (row_type i = 0; i < rows; ++i) {
		for (col_type j = 0; j < columns; ++j)
			tabular.getCellInset(i, j)->addPreview(loader);
	}
}


bool InsetTabular::tablemode(Cursor & cur) const
{
	return cur.selection() && cur.selBegin().idx() != cur.selEnd().idx();
}


bool InsetTabular::completionSupported(Cursor const & cur) const
{
	Cursor const & bvCur = cur.bv().cursor();
	if (&bvCur.inset() != this)
		return false;
	return cur.text()->completionSupported(cur);
}


bool InsetTabular::inlineCompletionSupported(Cursor const & cur) const
{
	return completionSupported(cur);
}


bool InsetTabular::automaticInlineCompletion() const
{
	return lyxrc.completion_inline_text;
}


bool InsetTabular::automaticPopupCompletion() const
{
	return lyxrc.completion_popup_text;
}


bool InsetTabular::showCompletionCursor() const
{
	return lyxrc.completion_cursor_text;
}


CompletionList const * InsetTabular::createCompletionList(Cursor const & cur) const
{
	return completionSupported(cur) ? cur.text()->createCompletionList(cur) : 0;
}


docstring InsetTabular::completionPrefix(Cursor const & cur) const
{
	if (!completionSupported(cur))
		return docstring();
	return cur.text()->completionPrefix(cur);
}


bool InsetTabular::insertCompletion(Cursor & cur, docstring const & s, bool finished)
{
	if (!completionSupported(cur))
		return false;

	return cur.text()->insertCompletion(cur, s, finished);
}


void InsetTabular::completionPosAndDim(Cursor const & cur, int & x, int & y, 
				    Dimension & dim) const
{
	TextMetrics const & tm = cur.bv().textMetrics(cur.text());
	tm.completionPosAndDim(cur, x, y, dim);
}


string const InsetTabularMailer::name_("tabular");

InsetTabularMailer::InsetTabularMailer(InsetTabular const & inset)
	: inset_(const_cast<InsetTabular &>(inset))
{}


string const InsetTabularMailer::inset2string(Buffer const &) const
{
	return params2string(inset_);
}


void InsetTabularMailer::string2params(string const & in, InsetTabular & inset)
{
	istringstream data(in);
	Lexer lex(0,0);
	lex.setStream(data);

	if (in.empty())
		return;

	string token;
	lex >> token;
	if (!lex || token != name_)
		return print_mailer_error("InsetTabularMailer", in, 1,
					  name_);

	// This is part of the inset proper that is usually swallowed
	// by Buffer::readInset
	lex >> token;
	if (!lex || token != "Tabular")
		return print_mailer_error("InsetTabularMailer", in, 2, "Tabular");

	inset.read(lex);
}


string const InsetTabularMailer::params2string(InsetTabular const & inset)
{
	ostringstream data;
	data << name_ << ' ';
	inset.write(data);
	data << "\\end_inset\n";
	return data.str();
}


} // namespace lyx
