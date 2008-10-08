/**
 * \file InsetTabular.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
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

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "CoordCache.h"
#include "debug.h"
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Color.h"
#include "callback.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "Undo.h"

#include "support/convert.h"
#include "support/lstrings.h"

#include "frontends/alert.h"
#include "frontends/Clipboard.h"
#include "frontends/Painter.h"
#include "frontends/Selection.h"

#include <sstream>
#include <iostream>
#include <limits>

using std::abs;
using std::auto_ptr;
using std::endl;
using std::getline;
using std::istream;
using std::istringstream;
using std::max;
using std::ostream;
using std::ostringstream;
using std::string;
using std::swap;
using std::vector;

#ifndef CXX_GLOBAL_CSTD
using std::strlen;
#endif

using boost::shared_ptr;
using boost::dynamic_pointer_cast;

namespace lyx {

using support::prefixIs;
using support::ltrim;
using support::rtrim;
using support::suffixIs;

using cap::dirtyTabularStack;
using cap::tabularStackDirty;

using graphics::PreviewLoader;

using frontend::Painter;
using frontend::Clipboard;

namespace Alert = frontend::Alert;


namespace {

int const ADD_TO_HEIGHT = 2;
int const ADD_TO_TABULAR_WIDTH = 2;
int const default_line_space = 10;
int const WIDTH_OF_LINE = 5;


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
	{ Tabular::M_TOGGLE_LINE_TOP, "m-toggle-line-top" },
	{ Tabular::M_TOGGLE_LINE_BOTTOM, "m-toggle-line-bottom" },
	{ Tabular::M_TOGGLE_LINE_LEFT, "m-toggle-line-left" },
	{ Tabular::M_TOGGLE_LINE_RIGHT, "m-toggle-line-right" },
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
	{ Tabular::LAST_ACTION, "" }
};


class FeatureEqual : public std::unary_function<TabularFeature, bool> {
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
	string::size_type pos = str.find(token);

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
	// set the lenght to be zero() as default as this it should be if not
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
	TabularFeature * it = std::find_if(tabularFeature, end,
					   FeatureEqual(feature));
	return (it == end) ? string() : it->feature;
}



/////////////////////////////////////////////////////////////////////
//
// Tabular
//
/////////////////////////////////////////////////////////////////////


Tabular::cellstruct::cellstruct(BufferParams const & bp)
	: cellno(0),
	  width_of_cell(0),
	  multicolumn(Tabular::CELL_NORMAL),
	  alignment(LYX_ALIGN_CENTER),
	  valignment(LYX_VALIGN_TOP),
	  top_line(true),
	  bottom_line(false),
	  left_line(true),
	  right_line(false),
	  usebox(BOX_NONE),
	  rotate(false),
	  inset(new InsetText(bp))
{}


Tabular::cellstruct::cellstruct(cellstruct const & cs)
	: cellno(cs.cellno),
	  width_of_cell(cs.width_of_cell),
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
	  inset(dynamic_cast<InsetText*>(cs.inset->clone().release()))
{}


Tabular::cellstruct &
Tabular::cellstruct::operator=(cellstruct cs)
{
	swap(cs);
	return *this;
}


void
Tabular::cellstruct::swap(cellstruct & rhs)
{
	std::swap(cellno, rhs.cellno);
	std::swap(width_of_cell, rhs.width_of_cell);
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


Tabular::rowstruct::rowstruct()
	: ascent_of_row(0),
	  descent_of_row(0),
	  top_line(true),
	  bottom_line(false),
	  top_space_default(false),
	  bottom_space_default(false),
	  interline_space_default(false),
	  endhead(false),
	  endfirsthead(false),
	  endfoot(false),
	  endlastfoot(false),
	  newpage(false)
{}


Tabular::columnstruct::columnstruct()
	: alignment(LYX_ALIGN_CENTER),
	  valignment(LYX_VALIGN_TOP),
	  left_line(true),
	  right_line(false),
	  width_of_column(0)
{
}


Tabular::ltType::ltType()
	: topDL(false),
	  bottomDL(false),
	  empty(false)
{}


Tabular::Tabular(BufferParams const & bp, row_type rows_arg,
		       col_type columns_arg)
{
	init(bp, rows_arg, columns_arg);
}


// activates all lines and sets all widths to 0
void Tabular::init(BufferParams const & bp, row_type rows_arg,
		      col_type columns_arg)
{
	rows_    = rows_arg;
	columns_ = columns_arg;
	row_info = row_vector(rows_);
	column_info = column_vector(columns_);
	cell_info = cell_vvector(rows_, cell_vector(columns_, cellstruct(bp)));
	row_info.reserve(10);
	column_info.reserve(10);
	cell_info.reserve(100);
	fixCellNums();
	for (row_type i = 0; i < rows_; ++i)
		cell_info[i].back().right_line = true;
	row_info.back().bottom_line = true;
	row_info.front().bottom_line = true;
	column_info.back().right_line = true;
	is_long_tabular = false;
	rotate = false;
	use_booktabs = false;
}


void Tabular::fixCellNums()
{
	idx_type cellno = 0;
	for (row_type i = 0; i < rows_; ++i) {
		for (col_type j = 0; j < columns_; ++j) {
			// When debugging it can be nice to set
			// this to true.
			cell_info[i][j].inset->setDrawFrame(false);
			cell_info[i][j].cellno = cellno++;
		}
		cell_info[i].back().right_line = true;
	}

	set_row_column_number_info();
}


void Tabular::appendRow(BufferParams const & bp, idx_type const cell)
{
	++rows_;

	row_type const row = row_of_cell(cell);

	row_vector::iterator rit = row_info.begin() + row;
	row_info.insert(rit, rowstruct());
	// now set the values of the row before
	row_info[row] = row_info[row + 1];

	cell_vvector old(rows_ - 1);
	for (row_type i = 0; i < rows_ - 1; ++i)
		swap(cell_info[i], old[i]);

	cell_info = cell_vvector(rows_, cell_vector(columns_, cellstruct(bp)));

	for (row_type i = 0; i <= row; ++i)
		swap(cell_info[i], old[i]);
	for (row_type i = row + 2; i < rows_; ++i)
		swap(cell_info[i], old[i - 1]);

	if (bp.trackChanges)
		for (col_type j = 0; j < columns_; ++j)
			cell_info[row + 1][j].inset->setChange(Change(Change::INSERTED));

	set_row_column_number_info();
}


void Tabular::deleteRow(row_type const row)
{
	// Not allowed to delete last row
	if (rows_ == 1)
		return;

	row_info.erase(row_info.begin() + row);
	cell_info.erase(cell_info.begin() + row);
	--rows_;
	fixCellNums();
}


void Tabular::copyRow(BufferParams const & bp, row_type const row)
{
	++rows_;

	row_info.insert(row_info.begin() + row, row_info[row]);
	cell_info.insert(cell_info.begin() + row, cell_info[row]);

	if (bp.trackChanges)
		for (col_type j = 0; j < columns_; ++j)
			cell_info[row + 1][j].inset->setChange(Change(Change::INSERTED));

	set_row_column_number_info();
}


void Tabular::appendColumn(BufferParams const & bp, idx_type const cell)
{
	++columns_;

	col_type const column = column_of_cell(cell);
	column_vector::iterator cit = column_info.begin() + column + 1;
	column_info.insert(cit, columnstruct());
	// set the column values of the column before
	column_info[column + 1] = column_info[column];

	for (row_type i = 0; i < rows_; ++i) {
		cell_info[i].insert(cell_info[i].begin() + column + 1, cellstruct(bp));

		// care about multicolumns
		if (cell_info[i][column + 1].multicolumn == CELL_BEGIN_OF_MULTICOLUMN)
			cell_info[i][column + 1].multicolumn = CELL_PART_OF_MULTICOLUMN;

		if (column + 2 >= columns_
		    || cell_info[i][column + 2].multicolumn != CELL_PART_OF_MULTICOLUMN)
			cell_info[i][column + 1].multicolumn = Tabular::CELL_NORMAL;
	}
	//++column;
	for (row_type i = 0; i < rows_; ++i) {
		cell_info[i][column + 1].inset->clear();
		if (bp.trackChanges)
			cell_info[i][column + 1].inset->setChange(Change(Change::INSERTED));
	}
	fixCellNums();
}


void Tabular::deleteColumn(col_type const column)
{
	// Not allowed to delete last column
	if (columns_ == 1)
		return;

	column_info.erase(column_info.begin() + column);
	for (row_type i = 0; i < rows_; ++i) {
		// Care about multicolumn cells
		if (column + 1 < columns_ &&
		    cell_info[i][column].multicolumn == CELL_BEGIN_OF_MULTICOLUMN &&
		    cell_info[i][column + 1].multicolumn == CELL_PART_OF_MULTICOLUMN) {
			cell_info[i][column + 1].multicolumn = CELL_BEGIN_OF_MULTICOLUMN;
		}
		cell_info[i].erase(cell_info[i].begin() + column);
	}
	--columns_;
	fixCellNums();
}


void Tabular::copyColumn(BufferParams const & bp, col_type const column)
{
	++columns_;

	column_info.insert(column_info.begin() + column, column_info[column]);

	for (row_type i = 0; i < rows_; ++i)
		cell_info[i].insert(cell_info[i].begin() + column, cell_info[i][column]);

	if (bp.trackChanges)
		for (row_type i = 0; i < rows_; ++i)
			cell_info[i][column + 1].inset->setChange(Change(Change::INSERTED));
	fixCellNums();
}


void Tabular::set_row_column_number_info()
{
	numberofcells = 0;
	for (row_type row = 0; row < rows_; ++row) {
		for (col_type column = 0; column < columns_; ++column) {

			// If on its left is either the edge, or a normal cell,
			// then this cannot be a non-begin multicol cell.
			// Clean up as well as we can:
			if (cell_info[row][column].multicolumn
				   == CELL_PART_OF_MULTICOLUMN) {
				if (column == 0 || 
				    cell_info[row][column - 1]
					.multicolumn == CELL_NORMAL)
					cell_info[row][column].multicolumn = 
						CELL_NORMAL;
			}
			// Count only non-multicol cells plus begin multicol
			// cells, ignore non-begin multicol cells:
			if (cell_info[row][column].multicolumn
				!= Tabular::CELL_PART_OF_MULTICOLUMN)
				++numberofcells;
			// Shouldn't happen after above fix - MV
			BOOST_ASSERT(numberofcells != 0);
			cell_info[row][column].cellno =
				numberofcells - 1;
		}
	}

	rowofcell.resize(numberofcells);
	columnofcell.resize(numberofcells);

	row_type row = 0;
	col_type column = 0;
	for (idx_type c = 0;
		 c < numberofcells && row < rows_ && column < columns_;) {
		rowofcell[c] = row;
		columnofcell[c] = column;
		++c;
		do {
			++column;
		} while (column < columns_ &&
				 cell_info[row][column].multicolumn
				 == Tabular::CELL_PART_OF_MULTICOLUMN);

		if (column == columns_) {
			column = 0;
			++row;
		}
	}

	for (row_type row = 0; row < rows_; ++row) {
		for (col_type column = 0; column < columns_; ++column) {
			if (isPartOfMultiColumn(row,column))
				continue;
			cell_info[row][column].inset->setAutoBreakRows(
				!getPWidth(getCellNumber(row, column)).zero());
		}
	}
}


Tabular::idx_type Tabular::getNumberOfCells() const
{
	return numberofcells;
}


Tabular::idx_type Tabular::numberOfCellsInRow(idx_type const cell) const
{
	row_type const row = row_of_cell(cell);
	idx_type result = 0;
	for (col_type i = 0; i < columns_; ++i)
		if (cell_info[row][i].multicolumn != Tabular::CELL_PART_OF_MULTICOLUMN)
			++result;
	return result;
}


bool Tabular::topLine(idx_type const cell, bool const wholerow) const
{
	if (!wholerow && isMultiColumn(cell) &&
	    !(use_booktabs && row_of_cell(cell) == 0))
		return cellinfo_of_cell(cell).top_line;
	return row_info[row_of_cell(cell)].top_line;
}


bool Tabular::bottomLine(idx_type const cell, bool wholerow) const
{
	if (!wholerow && isMultiColumn(cell) &&
	    !(use_booktabs && isLastRow(cell)))
		return cellinfo_of_cell(cell).bottom_line;
	return row_info[row_of_cell(cell)].bottom_line;
}


bool Tabular::leftLine(idx_type cell, bool wholecolumn) const
{
	if (use_booktabs)
		return false;
	if (!wholecolumn && isMultiColumn(cell) &&
		(isFirstCellInRow(cell) || isMultiColumn(cell-1)))
		return cellinfo_of_cell(cell).left_line;
	return column_info[column_of_cell(cell)].left_line;
}


bool Tabular::rightLine(idx_type cell, bool wholecolumn) const
{
	if (use_booktabs)
		return false;
	if (!wholecolumn && isMultiColumn(cell) &&
		(isLastCellInRow(cell) || isMultiColumn(cell + 1)))
		return cellinfo_of_cell(cell).right_line;
	return column_info[right_column_of_cell(cell)].right_line;
}


bool Tabular::topAlreadyDrawn(idx_type cell) const
{
	row_type row = row_of_cell(cell);
	if (row > 0 && !getAdditionalHeight(row)) {
		col_type column = column_of_cell(cell);
		--row;
		while (column
			   && cell_info[row][column].multicolumn
			   == Tabular::CELL_PART_OF_MULTICOLUMN)
			--column;
		if (cell_info[row][column].multicolumn == Tabular::CELL_NORMAL)
			return row_info[row].bottom_line;
		else
			return cell_info[row][column].bottom_line;
	}
	return false;
}


bool Tabular::leftAlreadyDrawn(idx_type cell) const
{
	col_type column = column_of_cell(cell);
	if (column > 0) {
		row_type row = row_of_cell(cell);
		while (--column &&
			   (cell_info[row][column].multicolumn ==
				Tabular::CELL_PART_OF_MULTICOLUMN));
		if (getAdditionalWidth(cell_info[row][column].cellno))
			return false;
		return rightLine(cell_info[row][column].cellno);
	}
	return false;
}


bool Tabular::isLastRow(idx_type cell) const
{
	return row_of_cell(cell) == rows_ - 1;
}


int Tabular::getAdditionalHeight(row_type row) const
{
	if (!row || row >= rows_)
		return 0;

	bool top = true;
	bool bottom = true;

	for (col_type column = 0; column < columns_ && bottom; ++column) {
		switch (cell_info[row - 1][column].multicolumn) {
		case Tabular::CELL_BEGIN_OF_MULTICOLUMN:
			bottom = cell_info[row - 1][column].bottom_line;
			break;
		case Tabular::CELL_NORMAL:
			bottom = row_info[row - 1].bottom_line;
		}
	}
	for (col_type column = 0; column < columns_ && top; ++column) {
		switch (cell_info[row][column].multicolumn) {
		case Tabular::CELL_BEGIN_OF_MULTICOLUMN:
			top = cell_info[row][column].top_line;
			break;
		case Tabular::CELL_NORMAL:
			top = row_info[row].top_line;
		}
	}
	int const interline_space = row_info[row - 1].interline_space_default ?
		default_line_space :
		row_info[row - 1].interline_space.inPixels(width_of_tabular);
	if (top && bottom)
		return interline_space + WIDTH_OF_LINE;
	return interline_space;
}


int Tabular::getAdditionalWidth(idx_type cell) const
{
	// internally already set in setWidthOfCell
	// used to get it back in text.cpp
	col_type const col = right_column_of_cell(cell);
	row_type const row = row_of_cell(cell);
	if (col < columns_ - 1 && rightLine(cell) &&
		leftLine(cell_info[row][col+1].cellno)) // column_info[col+1].left_line)
	{
		return WIDTH_OF_LINE;
	}
	return 0;
}


// returns the maximum over all rows
int Tabular::getWidthOfColumn(idx_type cell) const
{
	col_type const column1 = column_of_cell(cell);
	col_type const column2 = right_column_of_cell(cell);
	int result = 0;
	for (col_type i = column1; i <= column2; ++i)
		result += column_info[i].width_of_column;
	return result;
}


int Tabular::getWidthOfTabular() const
{
	return width_of_tabular;
}


// returns true if a complete update is necessary, otherwise false
bool Tabular::setWidthOfMulticolCell(idx_type cell, int new_width)
{
	if (!isMultiColumn(cell))
		return false;

	row_type const row = row_of_cell(cell);
	col_type const column1 = column_of_cell(cell);
	col_type const column2 = right_column_of_cell(cell);
	int const old_val = cell_info[row][column2].width_of_cell;

	// first set columns to 0 so we can calculate the right width
	for (col_type i = column1; i <= column2; ++i) {
		cell_info[row][i].width_of_cell = 0;
	}
	// set the width to MAX_WIDTH until width > 0
	int width = new_width + 2 * WIDTH_OF_LINE;
	col_type i = column1;
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


void Tabular::recalculateMulticolumnsOfColumn(col_type column)
{
	// the last column does not have to be recalculated because all
	// multicolumns will have here there last multicolumn cell which
	// always will have the whole rest of the width of the cell.
	if (columns_ < 2 || column > (columns_ - 2))
		return;
	for (row_type row = 0; row < rows_; ++row) {
		int mc = cell_info[row][column].multicolumn;
		int nmc = cell_info[row][column+1].multicolumn;
		// we only have to update multicolumns which do not have this
		// column as their last column!
		if (mc == CELL_BEGIN_OF_MULTICOLUMN ||
			  (mc == CELL_PART_OF_MULTICOLUMN &&
			   nmc == CELL_PART_OF_MULTICOLUMN))
		{
			idx_type const cellno = cell_info[row][column].cellno;
			setWidthOfMulticolCell(cellno,
					       getWidthOfCell(cellno) - 2 * WIDTH_OF_LINE);
		}
	}
}


void Tabular::setWidthOfCell(idx_type cell, int new_width)
{
	row_type const row = row_of_cell(cell);
	col_type const column1 = column_of_cell(cell);
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

	if (isMultiColumnReal(cell)) {
		tmp = setWidthOfMulticolCell(cell, new_width);
	} else {
		width = new_width + 2 * WIDTH_OF_LINE + add_width;
		cell_info[row][column1].width_of_cell = width;
		tmp = calculate_width_of_column_NMC(column1);
		if (tmp)
			recalculateMulticolumnsOfColumn(column1);
	}
	if (tmp) {
		for (col_type i = 0; i < columns_; ++i)
			calculate_width_of_column(i);
		calculate_width_of_tabular();
	}
}


void Tabular::setAlignment(idx_type cell, LyXAlignment align,
			      bool onlycolumn)
{
	if (!isMultiColumn(cell) || onlycolumn)
		column_info[column_of_cell(cell)].alignment = align;
	if (!onlycolumn)
		cellinfo_of_cell(cell).alignment = align;
}


void Tabular::setVAlignment(idx_type cell, VAlignment align,
			       bool onlycolumn)
{
	if (!isMultiColumn(cell) || onlycolumn)
		column_info[column_of_cell(cell)].valignment = align;
	if (!onlycolumn)
		cellinfo_of_cell(cell).valignment = align;
}


namespace {

/**
 * Allow line and paragraph breaks for fixed width cells or disallow them,
 * merge cell paragraphs and reset layout to standard for variable width
 * cells.
 */
void toggleFixedWidth(Cursor & cur, InsetText * inset, bool fixedWidth)
{
	inset->setAutoBreakRows(fixedWidth);
	if (fixedWidth)
		return;

	// merge all paragraphs to one
	BufferParams const & bp = cur.bv().buffer()->params();
	while (inset->paragraphs().size() > 1)
		mergeParagraph(bp, inset->paragraphs(), 0);

	// reset layout
	cur.push(*inset);
	// undo information has already been recorded
	inset->getText(0)->setLayout(*cur.bv().buffer(), 0, cur.lastpit() + 1,
			bp.getTextClass().defaultLayoutName());
	cur.pop();
}

}


void Tabular::setColumnPWidth(Cursor & cur, idx_type cell,
		Length const & width)
{
	col_type const j = column_of_cell(cell);

	column_info[j].p_width = width;
	for (row_type i = 0; i < rows_; ++i) {
		idx_type const cell = getCellNumber(i, j);
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
		column_info[column_of_cell(cell)].align_special = special;
}


void Tabular::setAllLines(idx_type cell, bool line)
{
	setTopLine(cell, line);
	setBottomLine(cell, line);
	setRightLine(cell, line);
	setLeftLine(cell, line);
}


void Tabular::setTopLine(idx_type cell, bool line, bool wholerow)
{
	row_type const row = row_of_cell(cell);
	if (wholerow || !isMultiColumn(cell))
		row_info[row].top_line = line;
	else
		cellinfo_of_cell(cell).top_line = line;
}


void Tabular::setBottomLine(idx_type cell, bool line, bool wholerow)
{
	if (wholerow || !isMultiColumn(cell))
		row_info[row_of_cell(cell)].bottom_line = line;
	else
		cellinfo_of_cell(cell).bottom_line = line;
}


void Tabular::setLeftLine(idx_type cell, bool line, bool wholecolumn)
{
	if (wholecolumn || !isMultiColumn(cell))
		column_info[column_of_cell(cell)].left_line = line;
	else
		cellinfo_of_cell(cell).left_line = line;
}


void Tabular::setRightLine(idx_type cell, bool line, bool wholecolumn)
{
	if (wholecolumn || !isMultiColumn(cell))
		column_info[right_column_of_cell(cell)].right_line = line;
	else
		cellinfo_of_cell(cell).right_line = line;
}


LyXAlignment Tabular::getAlignment(idx_type cell, bool onlycolumn) const
{
	if (!onlycolumn && isMultiColumn(cell))
		return cellinfo_of_cell(cell).alignment;
	return column_info[column_of_cell(cell)].alignment;
}


Tabular::VAlignment
Tabular::getVAlignment(idx_type cell, bool onlycolumn) const
{
	if (!onlycolumn && isMultiColumn(cell))
		return cellinfo_of_cell(cell).valignment;
	return column_info[column_of_cell(cell)].valignment;
}


Length const Tabular::getPWidth(idx_type cell) const
{
	if (isMultiColumn(cell))
		return cellinfo_of_cell(cell).p_width;
	return column_info[column_of_cell(cell)].p_width;
}


Length const Tabular::getColumnPWidth(idx_type cell) const
{
	return column_info[column_of_cell(cell)].p_width;
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
	return column_info[column_of_cell(cell)].align_special;
}


int Tabular::getWidthOfCell(idx_type cell) const
{
	row_type const row = row_of_cell(cell);
	col_type const column1 = column_of_cell(cell);
	col_type const column2 = right_column_of_cell(cell);
	int result = 0;
	for (col_type i = column1; i <= column2; ++i)
		result += cell_info[row][i].width_of_cell;
	return result;
}


int Tabular::getBeginningOfTextInCell(idx_type cell) const
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


bool Tabular::isFirstCellInRow(idx_type cell) const
{
	return column_of_cell(cell) == 0;
}


Tabular::idx_type Tabular::getFirstCellInRow(row_type row) const
{
	if (row > rows_ - 1)
		row = rows_ - 1;
	return cell_info[row][0].cellno;
}


bool Tabular::isLastCellInRow(idx_type cell) const
{
	return right_column_of_cell(cell) == columns_ - 1;
}


Tabular::idx_type Tabular::getLastCellInRow(row_type row) const
{
	if (row > rows_ - 1)
		row = rows_ - 1;
	return cell_info[row][columns_-1].cellno;
}


void Tabular::calculate_width_of_column(col_type column)
{
	int maximum = 0;
	for (row_type i = 0; i < rows_; ++i)
		maximum = max(cell_info[i][column].width_of_cell, maximum);
	column_info[column].width_of_column = maximum;
}


//
// Calculate the columns regarding ONLY the normal cells and if this
// column is inside a multicolumn cell then use it only if its the last
// column of this multicolumn cell as this gives an added width to the
// column, all the rest should be adapted!
//
bool Tabular::calculate_width_of_column_NMC(col_type column)
{
	int const old_column_width = column_info[column].width_of_column;
	int max = 0;
	for (row_type i = 0; i < rows_; ++i) {
		idx_type cell = getCellNumber(i, column);
		bool ismulti = isMultiColumnReal(cell);
		if ((!ismulti || column == right_column_of_cell(cell)) &&
			cell_info[i][column].width_of_cell > max)
		{
			max = cell_info[i][column].width_of_cell;
		}
	}
	column_info[column].width_of_column = max;
	return column_info[column].width_of_column != old_column_width;
}


void Tabular::calculate_width_of_tabular()
{
	width_of_tabular = 0;
	for (col_type i = 0; i < columns_; ++i)
		width_of_tabular += column_info[i].width_of_column;
}


Tabular::row_type Tabular::row_of_cell(idx_type cell) const
{
	if (cell >= numberofcells)
		return rows_ - 1;
	if (cell == npos)
		return 0;
	return rowofcell[cell];
}


Tabular::col_type Tabular::column_of_cell(idx_type cell) const
{
	if (cell >= numberofcells)
		return columns_ - 1;
	if (cell == npos)
		return 0;
	return columnofcell[cell];
}


Tabular::col_type Tabular::right_column_of_cell(idx_type cell) const
{
	row_type const row = row_of_cell(cell);
	col_type column = column_of_cell(cell);
	while (column < columns_ - 1 &&
		   cell_info[row][column + 1].multicolumn == Tabular::CELL_PART_OF_MULTICOLUMN)
		++column;
	return column;
}


void Tabular::write(Buffer const & buf, ostream & os) const
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
	for (col_type j = 0; j < columns_; ++j) {
		os << "<column"
		   << write_attribute("alignment", column_info[j].alignment)
		   << write_attribute("valignment", column_info[j].valignment)
		   << write_attribute("leftline", column_info[j].left_line)
		   << write_attribute("rightline", column_info[j].right_line)
		   << write_attribute("width", column_info[j].p_width.asString())
		   << write_attribute("special", column_info[j].align_special)
		   << ">\n";
	}
	for (row_type i = 0; i < rows_; ++i) {
		os << "<row"
		   << write_attribute("topline", row_info[i].top_line)
		   << write_attribute("bottomline", row_info[i].bottom_line);
		static const string def("default");
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
		for (col_type j = 0; j < columns_; ++j) {
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
			cell_info[i][j].inset->write(buf, os);
			os << "\n\\end_inset\n"
			   << "</cell>\n";
		}
		os << "</row>\n";
	}
	os << "</lyxtabular>\n";
}


void Tabular::read(Buffer const & buf, Lexer & lex)
{
	string line;
	istream & is = lex.getStream();

	l_getline(is, line);
	if (!prefixIs(line, "<lyxtabular ")
		&& !prefixIs(line, "<Tabular ")) {
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
	init(buf.params(), rows_arg, columns_arg);
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

	for (col_type j = 0; j < columns_; ++j) {
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

	for (row_type i = 0; i < rows_; ++i) {
		l_getline(is, line);
		if (!prefixIs(line, "<row")) {
			lyxerr << "Wrong tabular format (expected <row ...> got"
			       << line << ')' << endl;
			return;
		}
		getTokenValue(line, "topline", row_info[i].top_line);
		getTokenValue(line, "bottomline", row_info[i].bottom_line);
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
		for (col_type j = 0; j < columns_; ++j) {
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
				cell_info[i][j].inset->read(buf, lex);
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


bool Tabular::isMultiColumn(idx_type cell) const
{
	return cellinfo_of_cell(cell).multicolumn != Tabular::CELL_NORMAL;
}


bool Tabular::isMultiColumnReal(idx_type cell) const
{
	return column_of_cell(cell) != right_column_of_cell(cell) &&
			cellinfo_of_cell(cell).multicolumn != Tabular::CELL_NORMAL;
}


Tabular::cellstruct & Tabular::cellinfo_of_cell(idx_type cell) const
{
	return cell_info[row_of_cell(cell)][column_of_cell(cell)];
}


void Tabular::setMultiColumn(Buffer * buffer, idx_type cell,
				idx_type number)
{
	cellstruct & cs = cellinfo_of_cell(cell);
	cs.multicolumn = CELL_BEGIN_OF_MULTICOLUMN;
	cs.alignment = column_info[column_of_cell(cell)].alignment;
	cs.top_line = row_info[row_of_cell(cell)].top_line;
	cs.bottom_line = row_info[row_of_cell(cell)].bottom_line;
	cs.left_line = column_info[column_of_cell(cell)].left_line;
	cs.right_line = column_info[column_of_cell(cell+number-1)].right_line;
	for (idx_type i = 1; i < number; ++i) {
		cellstruct & cs1 = cellinfo_of_cell(cell + i);
		cs1.multicolumn = CELL_PART_OF_MULTICOLUMN;
		cs.inset->appendParagraphs(buffer, cs1.inset->paragraphs());
		cs1.inset->clear();
	}
	set_row_column_number_info();
}


Tabular::idx_type Tabular::cells_in_multicolumn(idx_type cell) const
{
	row_type const row = row_of_cell(cell);
	col_type column = column_of_cell(cell);
	idx_type result = 1;
	++column;
	while (column < columns_ &&
		   cell_info[row][column].multicolumn == CELL_PART_OF_MULTICOLUMN)
	{
		++result;
		++column;
	}
	return result;
}


Tabular::idx_type Tabular::unsetMultiColumn(idx_type cell)
{
	row_type const row = row_of_cell(cell);
	col_type column = column_of_cell(cell);

	idx_type result = 0;

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
	for (row_type i = 0; i < rows_; ++i)
		for (col_type j = 0; j < columns_; ++j)
			if (cell_info[i][j].rotate)
				return true;
	return false;
}


bool Tabular::isLastCell(idx_type cell) const
{
	if (cell + 1 < numberofcells)
		return false;
	return true;
}


Tabular::idx_type Tabular::getCellAbove(idx_type cell) const
{
	if (row_of_cell(cell) > 0)
		return cell_info[row_of_cell(cell)-1][column_of_cell(cell)].cellno;
	return cell;
}


Tabular::idx_type Tabular::getCellBelow(idx_type cell) const
{
	if (row_of_cell(cell) + 1 < rows_)
		return cell_info[row_of_cell(cell)+1][column_of_cell(cell)].cellno;
	return cell;
}


Tabular::idx_type Tabular::getLastCellAbove(idx_type cell) const
{
	if (row_of_cell(cell) == 0)
		return cell;
	if (!isMultiColumn(cell))
		return getCellAbove(cell);
	return cell_info[row_of_cell(cell) - 1][right_column_of_cell(cell)].cellno;
}


Tabular::idx_type Tabular::getLastCellBelow(idx_type cell) const
{
	if (row_of_cell(cell) + 1 >= rows_)
		return cell;
	if (!isMultiColumn(cell))
		return getCellBelow(cell);
	return cell_info[row_of_cell(cell) + 1][right_column_of_cell(cell)].cellno;
}


Tabular::idx_type Tabular::getCellNumber(row_type row,
					       col_type column) const
{
	BOOST_ASSERT(column != npos && column < columns_ &&
		     row    != npos && row    < rows_);
	return cell_info[row][column].cellno;
}


void Tabular::setUsebox(idx_type cell, BoxType type)
{
	cellinfo_of_cell(cell).usebox = type;
}


Tabular::BoxType Tabular::getUsebox(idx_type cell) const
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
	for (row_type i = 0; i < rows_; ++i)
		if (row_info[i].endhead)
			return true;
	return false;
}


bool Tabular::haveLTFirstHead() const
{
	if (endfirsthead.empty)
		return false;
	for (row_type i = 0; i < rows_; ++i)
		if (row_info[i].endfirsthead)
			return true;
	return false;
}


bool Tabular::haveLTFoot() const
{
	for (row_type i = 0; i < rows_; ++i)
		if (row_info[i].endfoot)
			return true;
	return false;
}


bool Tabular::haveLTLastFoot() const
{
	if (endlastfoot.empty)
		return false;
	for (row_type i = 0; i < rows_; ++i)
		if (row_info[i].endlastfoot)
			return true;
	return false;
}


// end longtable support functions

void Tabular::setAscentOfRow(row_type row, int height)
{
	if (row >= rows_ || row_info[row].ascent_of_row == height)
		return;
	row_info[row].ascent_of_row = height;
}


void Tabular::setDescentOfRow(row_type row, int height)
{
	if (row >= rows_ || row_info[row].descent_of_row == height)
		return;
	row_info[row].descent_of_row = height;
}


int Tabular::getAscentOfRow(row_type row) const
{
	if (row >= rows_)
		return 0;
	return row_info[row].ascent_of_row;
}


int Tabular::getDescentOfRow(row_type row) const
{
	BOOST_ASSERT(row < rows_);
	return row_info[row].descent_of_row;
}


int Tabular::getHeightOfTabular() const
{
	int height = 0;
	for (row_type row = 0; row < rows_; ++row)
		height += getAscentOfRow(row) + getDescentOfRow(row) +
			getAdditionalHeight(row);
	return height;
}


bool Tabular::isPartOfMultiColumn(row_type row, col_type column) const
{
	BOOST_ASSERT(row < rows_);
	BOOST_ASSERT(column < columns_);
	return cell_info[row][column].multicolumn == CELL_PART_OF_MULTICOLUMN;
}


int Tabular::TeXTopHLine(odocstream & os, row_type row) const
{
	// FIXME: assert or return 0 as in TeXBottomHLine()?
	BOOST_ASSERT(row != npos);
	BOOST_ASSERT(row < rows_);

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


int Tabular::TeXBottomHLine(odocstream & os, row_type row) const
{
	// FIXME: return 0 or assert as in TeXTopHLine()?
	if (row == npos || row >= rows_)
		return 0;

	idx_type const fcell = getFirstCellInRow(row);
	idx_type const n = numberOfCellsInRow(fcell) + fcell;
	idx_type tmp = 0;

	for (idx_type i = fcell; i < n; ++i) {
		if (bottomLine(i))
			++tmp;
	}
	if (use_booktabs && row == rows_ - 1) {
		if (bottomLine(fcell))
			os << "\\bottomrule ";
	} else if (tmp == n - fcell) {
		os << (use_booktabs ? "\\midrule" : "\\hline");
	} else if (tmp) {
		for (idx_type i = fcell; i < n; ++i) {
			if (bottomLine(i)) {
				os << (use_booktabs ? "\\cmidrule{" : "\\cline{")
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


int Tabular::TeXCellPreamble(odocstream & os, idx_type cell) const
{
	int ret = 0;

	if (getRotateCell(cell)) {
		os << "\\begin{sideways}\n";
		++ret;
	}
	if (isMultiColumn(cell)) {
		os << "\\multicolumn{" << cells_in_multicolumn(cell) << "}{";
		if (leftLine(cell) &&
			(isFirstCellInRow(cell) ||
			 (!isMultiColumn(cell - 1) && !leftLine(cell, true) &&
			  !rightLine(cell - 1, true))))
			os << '|';
		if (!cellinfo_of_cell(cell).align_special.empty()) {
			os << cellinfo_of_cell(cell).align_special;
		} else {
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
				   << from_ascii(getPWidth(cell).asLatexString())
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
			} // end if else !getPWidth
		} // end if else !cellinfo_of_cell
		if (rightLine(cell))
			os << '|';
		if (((cell + 1) < numberofcells) && !isFirstCellInRow(cell+1) &&
			leftLine(cell+1))
			os << '|';
		os << "}{";
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
		os << "]{" << from_ascii(getPWidth(cell).asLatexString())
		   << "}{";
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
	if (isMultiColumn(cell)) {
		os << '}';
	}
	if (getRotateCell(cell)) {
		os << "%\n\\end{sideways}";
		++ret;
	}
	return ret;
}


int Tabular::TeXLongtableHeaderFooter(odocstream & os, Buffer const & buf,
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
		for (row_type i = 0; i < rows_; ++i) {
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
		for (row_type i = 0; i < rows_; ++i) {
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
		for (row_type i = 0; i < rows_; ++i) {
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
		for (row_type i = 0; i < rows_; ++i) {
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


bool Tabular::isValidRow(row_type row) const
{
	if (!is_long_tabular)
		return true;
	return !row_info[row].endhead && !row_info[row].endfirsthead &&
			!row_info[row].endfoot && !row_info[row].endlastfoot;
}


int Tabular::TeXRow(odocstream & os, row_type i, Buffer const & buf,
		       OutputParams const & runparams) const
{
	idx_type cell = getCellNumber(i, 0);
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

	for (col_type j = 0; j < columns_; ++j) {
		if (isPartOfMultiColumn(i, j))
			continue;
		ret += TeXCellPreamble(os, cell);
		shared_ptr<InsetText> inset = getCellInset(cell);

		Paragraph const & par = inset->paragraphs().front();
		bool rtl = par.isRightToLeftPar(buf.params())
			&& !par.empty()
			&& getPWidth(cell).zero();

		if (rtl) {
			if (par.getParLanguage(buf.params())->lang() ==
			"farsi")
				os << "\\textFR{";
			else if (par.getParLanguage(buf.params())->lang() == "arabic_arabi")
				os << "\\textAR{";
			// currently, remaning RTL languages are arabic_arabtex and hebrew
			else
				os << "\\R{";
		}
		ret += inset->latex(buf, os, runparams);
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


int Tabular::latex(Buffer const & buf, odocstream & os,
		      OutputParams const & runparams) const
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
	for (col_type i = 0; i < columns_; ++i) {
		if (!use_booktabs && column_info[i].left_line)
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
		if (!use_booktabs && column_info[i].right_line)
			os << '|';
	}
	os << "}\n";
	++ret;

	ret += TeXLongtableHeaderFooter(os, buf, runparams);

	//+---------------------------------------------------------------------
	//+                      the single row and columns (cells)            +
	//+---------------------------------------------------------------------

	for (row_type i = 0; i < rows_; ++i) {
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


int Tabular::docbookRow(Buffer const & buf, odocstream & os, row_type row,
			   OutputParams const & runparams) const
{
	int ret = 0;
	idx_type cell = getFirstCellInRow(row);

	os << "<row>\n";
	for (col_type j = 0; j < columns_; ++j) {
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
		ret += getCellInset(cell)->docbook(buf, os, runparams);
		os << "</entry>\n";
		++cell;
	}
	os << "</row>\n";
	return ret;
}


int Tabular::docbook(Buffer const & buf, odocstream & os,
			OutputParams const & runparams) const
{
	int ret = 0;

	//+---------------------------------------------------------------------
	//+                      first the opening preamble                    +
	//+---------------------------------------------------------------------

	os << "<tgroup cols=\"" << columns_
	   << "\" colsep=\"1\" rowsep=\"1\">\n";

	for (col_type i = 0; i < columns_; ++i) {
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
		for (row_type i = 0; i < rows_; ++i) {
			if (row_info[i].endhead || row_info[i].endfirsthead) {
				ret += docbookRow(buf, os, i, runparams);
			}
		}
		os << "</thead>\n";
		++ret;
	}
	// output footer info
	if (haveLTFoot() || haveLTLastFoot()) {
		os << "<tfoot>\n";
		++ret;
		for (row_type i = 0; i < rows_; ++i) {
			if (row_info[i].endfoot || row_info[i].endlastfoot) {
				ret += docbookRow(buf, os, i, runparams);
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
	for (row_type i = 0; i < rows_; ++i) {
		if (isValidRow(i)) {
			ret += docbookRow(buf, os, i, runparams);
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
		col_type column = column_of_cell(i);
		int len = clen[column];
		while (column < columns_ - 1
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
		col_type column = column_of_cell(i);
		int len = clen[column];
		while (column < columns_ -1
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


void Tabular::plaintextPrintCell(Buffer const & buf, odocstream & os,
			       OutputParams const & runparams,
			       idx_type cell, row_type row, col_type column,
			       vector<unsigned int> const & clen,
			       bool onlydata) const
{
	odocstringstream sstr;
	getCellInset(cell)->plaintext(buf, sstr, runparams);

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
	while (column < columns_ -1
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


void Tabular::plaintext(Buffer const & buf, odocstream & os,
			   OutputParams const & runparams, int const depth,
			   bool onlydata, char_type delim) const
{
	// first calculate the width of the single columns
	vector<unsigned int> clen(columns_);

	if (!onlydata) {
		// first all non (real) multicolumn cells!
		for (col_type j = 0; j < columns_; ++j) {
			clen[j] = 0;
			for (row_type i = 0; i < rows_; ++i) {
				idx_type cell = getCellNumber(i, j);
				if (isMultiColumnReal(cell))
					continue;
				odocstringstream sstr;
				getCellInset(cell)->plaintext(buf, sstr, runparams);
				if (clen[j] < sstr.str().length())
					clen[j] = sstr.str().length();
			}
		}
		// then all (real) multicolumn cells!
		for (col_type j = 0; j < columns_; ++j) {
			for (row_type i = 0; i < rows_; ++i) {
				idx_type cell = getCellNumber(i, j);
				if (!isMultiColumnReal(cell) || isPartOfMultiColumn(i, j))
					continue;
				odocstringstream sstr;
				getCellInset(cell)->plaintext(buf, sstr, runparams);
				int len = int(sstr.str().length());
				idx_type const n = cells_in_multicolumn(cell);
				for (col_type k = j; len > 0 && k < j + n - 1; ++k)
					len -= clen[k];
				if (len > int(clen[j + n - 1]))
					clen[j + n - 1] = len;
			}
		}
	}
	idx_type cell = 0;
	for (row_type i = 0; i < rows_; ++i) {
		if (!onlydata && plaintextTopHLine(os, i, clen))
			os << docstring(depth * 2, ' ');
		for (col_type j = 0; j < columns_; ++j) {
			if (isPartOfMultiColumn(i, j))
				continue;
			if (onlydata && j > 0)
				// we don't use operator<< for single UCS4 character
				// see explanation in docstream.h
				os.put(delim);
			plaintextPrintCell(buf, os, runparams,
					   cell, i, j, clen, onlydata);
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


shared_ptr<InsetText> Tabular::getCellInset(idx_type cell) const
{
	return cell_info[row_of_cell(cell)][column_of_cell(cell)].inset;
}


shared_ptr<InsetText> Tabular::getCellInset(row_type row,
					       col_type column) const
{
	return cell_info[row][column].inset;
}


void Tabular::setCellInset(row_type row, col_type column,
			      shared_ptr<InsetText> ins) const
{
	cell_info[row][column].inset = ins;
}


Tabular::idx_type
Tabular::getCellFromInset(Inset const * inset) const
{
	// is this inset part of the tabular?
	if (!inset) {
		lyxerr << "Error: this is not a cell of the tabular!" << endl;
		BOOST_ASSERT(false);
	}

	for (idx_type cell = 0, n = getNumberOfCells(); cell < n; ++cell)
		if (getCellInset(cell).get() == inset) {
			LYXERR(Debug::INSETTEXT) << "Tabular::getCellFromInset: "
				<< "cell=" << cell << endl;
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
	for (idx_type cell = 0; cell < numberofcells; ++cell) {
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
// InsetTabular
//
/////////////////////////////////////////////////////////////////////

InsetTabular::InsetTabular(Buffer const & buf, row_type rows,
			   col_type columns)
	: tabular(buf.params(), max(rows, row_type(1)),
	  max(columns, col_type(1))), buffer_(&buf), scx_(0)
{}


InsetTabular::InsetTabular(InsetTabular const & tab)
	: Inset(tab), tabular(tab.tabular),
		buffer_(tab.buffer_), scx_(0)
{}


InsetTabular::~InsetTabular()
{
	InsetTabularMailer(*this).hideDialog();
}


auto_ptr<Inset> InsetTabular::doClone() const
{
	return auto_ptr<Inset>(new InsetTabular(*this));
}


Buffer const & InsetTabular::buffer() const
{
	return *buffer_;
}


void InsetTabular::buffer(Buffer const * b)
{
	buffer_ = b;
}


void InsetTabular::write(Buffer const & buf, ostream & os) const
{
	os << "Tabular" << endl;
	tabular.write(buf, os);
}


void InsetTabular::read(Buffer const & buf, Lexer & lex)
{
	bool const old_format = (lex.getString() == "\\LyXTable");

	tabular.read(buf, lex);

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


bool InsetTabular::metrics(MetricsInfo & mi, Dimension & dim) const
{
	//lyxerr << "InsetTabular::metrics: " << mi.base.bv << " width: " <<
	//	mi.base.textwidth << "\n";
	if (!mi.base.bv) {
		lyxerr << "InsetTabular::metrics: need bv" << endl;
		BOOST_ASSERT(false);
	}

	row_type i = 0;
	for (idx_type cell = 0; i < tabular.rows(); ++i) {
		int maxAsc = 0;
		int maxDesc = 0;
		for (col_type j = 0; j < tabular.columns(); ++j) {
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
			tabular.setWidthOfCell(cell, dim.wid);
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
		tabular.setAscentOfRow(i, maxAsc + ADD_TO_HEIGHT + top_space);
		int const bottom_space = tabular.row_info[i].bottom_space_default ?
			default_line_space :
			tabular.row_info[i].bottom_space.inPixels(mi.base.textwidth);
		tabular.setDescentOfRow(i, maxDesc + ADD_TO_HEIGHT + bottom_space);
	}

	dim.asc = tabular.getAscentOfRow(0);
	dim.des = tabular.getHeightOfTabular() - dim.asc;
	dim.wid = tabular.getWidthOfTabular() + 2 * ADD_TO_TABULAR_WIDTH;
	bool const changed = dim_ != dim;
	dim_ = dim;
	return changed;
}


void InsetTabular::draw(PainterInfo & pi, int x, int y) const
{
	setPosCache(pi, x, y);

	//lyxerr << "InsetTabular::draw: " << x << " " << y << endl;
	BufferView * bv = pi.base.bv;

	resetPos(bv->cursor());

	x += scx_;
	x += ADD_TO_TABULAR_WIDTH;

	idx_type idx = 0;
	first_visible_cell = Tabular::npos;
	for (row_type i = 0; i < tabular.rows(); ++i) {
		int nx = x;
		int const a = tabular.getAscentOfRow(i);
		int const d = tabular.getDescentOfRow(i);
		idx = tabular.getCellNumber(i, 0);
		for (col_type j = 0; j < tabular.columns(); ++j) {
			if (tabular.isPartOfMultiColumn(i, j))
				continue;
			if (first_visible_cell == Tabular::npos)
				first_visible_cell = idx;

			int const cx = nx + tabular.getBeginningOfTextInCell(idx);
			if (nx + tabular.getWidthOfColumn(idx) < 0
			    || nx > bv->workWidth()
			    || y + d < 0
			    || y - a > bv->workHeight()) {
				pi.pain.setDrawingEnabled(false);
				cell(idx)->draw(pi, cx, y);
				drawCellLines(pi.pain, nx, y, i, idx, pi.erased_);
				pi.pain.setDrawingEnabled(true);
			} else {
				cell(idx)->draw(pi, cx, y);
				drawCellLines(pi.pain, nx, y, i, idx, pi.erased_);
			}
			nx += tabular.getWidthOfColumn(idx);
			++idx;
		}

		if (i + 1 < tabular.rows())
			y += d + tabular.getAscentOfRow(i + 1) +
				tabular.getAdditionalHeight(i + 1);
	}
}


void InsetTabular::drawSelection(PainterInfo & pi, int x, int y) const
{
	setPosCache(pi, x, y);

	Cursor & cur = pi.base.bv->cursor();

	x += scx_ + ADD_TO_TABULAR_WIDTH;

	// Paint background of current tabular
	int const w = tabular.getWidthOfTabular();
	int const h = tabular.getHeightOfTabular();
	int yy = y - tabular.getAscentOfRow(0);
	pi.pain.fillRectangle(x, yy, w, h, backgroundColor());

	if (!cur.selection())
		return;
	if (!ptr_cmp(&cur.inset(), this))
		return;

	//resetPos(cur);


	if (tablemode(cur)) {
		row_type rs, re;
		col_type cs, ce;
		getSelection(cur, rs, re, cs, ce);
		y -= tabular.getAscentOfRow(0);
		for (row_type j = 0; j < tabular.rows(); ++j) {
			int const a = tabular.getAscentOfRow(j);
			int const h = a + tabular.getDescentOfRow(j);
			int xx = x;
			y += tabular.getAdditionalHeight(j);
			for (col_type i = 0; i < tabular.columns(); ++i) {
				if (tabular.isPartOfMultiColumn(j, i))
					continue;
				idx_type const cell =
					tabular.getCellNumber(j, i);
				int const w = tabular.getWidthOfColumn(cell);
				if (i >= cs && i <= ce && j >= rs && j <= re)
					pi.pain.fillRectangle(xx, y, w, h,
							      Color::selection);
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
	int x2 = x + tabular.getWidthOfColumn(cell);
	bool on_off = false;
	Color::color col = Color::tabularline;
	Color::color onoffcol = Color::tabularonoffline;

	if (erased) {
		col = Color::deletedtext;
		onoffcol = Color::deletedtext;
	}

	if (!tabular.topAlreadyDrawn(cell)) {
		on_off = !tabular.topLine(cell);
		pain.line(x, y - tabular.getAscentOfRow(row),
			  x2, y -  tabular.getAscentOfRow(row),
			  on_off ? onoffcol : col,
			  on_off ? Painter::line_onoffdash : Painter::line_solid);
	}
	on_off = !tabular.bottomLine(cell);
	pain.line(x, y + tabular.getDescentOfRow(row),
		  x2, y + tabular.getDescentOfRow(row),
		  on_off ? onoffcol : col,
		  on_off ? Painter::line_onoffdash : Painter::line_solid);
	if (!tabular.leftAlreadyDrawn(cell)) {
		on_off = !tabular.leftLine(cell);
		pain.line(x, y -  tabular.getAscentOfRow(row),
			  x, y +  tabular.getDescentOfRow(row),
			  on_off ? onoffcol : col,
			  on_off ? Painter::line_onoffdash : Painter::line_solid);
	}
	on_off = !tabular.rightLine(cell);
	pain.line(x2 - tabular.getAdditionalWidth(cell),
		  y -  tabular.getAscentOfRow(row),
		  x2 - tabular.getAdditionalWidth(cell),
		  y +  tabular.getDescentOfRow(row),
		  on_off ? onoffcol : col,
		  on_off ? Painter::line_onoffdash : Painter::line_solid);
}


docstring const InsetTabular::editMessage() const
{
	return _("Opened table");
}


void InsetTabular::edit(Cursor & cur, bool left)
{
	//lyxerr << "InsetTabular::edit: " << this << endl;
	finishUndo();
	cur.selection() = false;
	cur.push(*this);
	if (left) {
		if (isRightToLeft(cur))
			cur.idx() = tabular.getLastCellInRow(0);
		else
			cur.idx() = 0;
		cur.pit() = 0;
		cur.pos() = 0;
	} else {
		if (isRightToLeft(cur))
			cur.idx() = tabular.getFirstCellInRow(tabular.rows() - 1);
		else
			cur.idx() = tabular.getNumberOfCells() - 1;
		cur.pit() = 0;
		cur.pos() = cur.lastpos(); // FIXME crude guess
	}
	// FIXME: this accesses the position cache before it is initialized
	//resetPos(cur);
	//cur.bv().fitCursor();
}


void InsetTabular::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	LYXERR(Debug::DEBUG) << "# InsetTabular::doDispatch: cmd: " << cmd
			     << "\n  cur:" << cur << endl;
	CursorSlice sl = cur.top();
	Cursor & bvcur = cur.bv().cursor();

	switch (cmd.action) {

	case LFUN_MOUSE_PRESS:
		//lyxerr << "# InsetTabular::MousePress\n" << cur.bv().cursor() << endl;

		// do not reset cursor/selection if we have selected
		// some cells (bug 2715).
		if (cmd.button() == mouse_button::button3
		    && &bvcur.selBegin().inset() == this 
		    && tablemode(bvcur)) 
			;
		else
			// Let InsetText do it
			cell(cur.idx())->dispatch(cur, cmd);
		break;
	case LFUN_MOUSE_MOTION:
		//lyxerr << "# InsetTabular::MouseMotion\n" << bvcur << endl;
		if (cmd.button() == mouse_button::button1) {
			// only accept motions to places not deeper nested than the real anchor
			if (bvcur.anchor_.hasPart(cur)) {
				// only update if selection changes
				if (bvcur.idx() == cur.idx() &&
					!(bvcur.anchor_.idx() == cur.idx() && bvcur.pos() != cur.pos()))
					cur.noUpdate();
				setCursorFromCoordinates(cur, cmd.x, cmd.y);
				bvcur.setCursor(cur);
				bvcur.selection() = true;
	  			if (tablemode(bvcur)) {
	 				bvcur.pit() = bvcur.lastpit();
	 				bvcur.pos() = bvcur.lastpos();
	  			}
			} else
				cur.undispatched();
		}
		break;

	case LFUN_MOUSE_RELEASE:
		//lyxerr << "# InsetTabular::MouseRelease\n" << bvcur << endl;
		if (cmd.button() == mouse_button::button3)
			InsetTabularMailer(*this).showDialog(&cur.bv());
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
			isRightToLeft(cur) ? movePrevCell(cur) : moveNextCell(cur);
			if (sl == cur.top())
				cmd = FuncRequest(LFUN_FINISHED_RIGHT);
			else
				cur.dispatched();
		}
		break;

	case LFUN_CHAR_BACKWARD_SELECT:
	case LFUN_CHAR_BACKWARD:
		cell(cur.idx())->dispatch(cur, cmd);
		if (!cur.result().dispatched()) {
			isRightToLeft(cur) ? moveNextCell(cur) : movePrevCell(cur);
			if (sl == cur.top())
				cmd = FuncRequest(LFUN_FINISHED_LEFT);
			else
				cur.dispatched();
		}
		break;

	case LFUN_DOWN_SELECT:
	case LFUN_DOWN:
		cell(cur.idx())->dispatch(cur, cmd);
		cur.dispatched(); // override the cell's decision
		if (sl == cur.top())
			// if our Text didn't do anything to the cursor
			// then we try to put the cursor into the cell below
			// setting also the right targetX.
			if (tabular.row_of_cell(cur.idx()) != tabular.rows() - 1) {
				cur.idx() = tabular.getCellBelow(cur.idx());
				cur.pit() = 0;
				TextMetrics const & tm =
					cur.bv().textMetrics(cell(cur.idx())->getText(0));
				cur.pos() = tm.x2pos(cur.pit(), 0, cur.targetX());
			}
		if (sl == cur.top()) {
			// we trick it to go to the RIGHT after leaving the
			// tabular.
			cmd = FuncRequest(LFUN_FINISHED_RIGHT);
			cur.undispatched();
		}
		if (tablemode(cur)) {
			cur.pit() = cur.lastpit();
			cur.pos() = cur.lastpos();
			return;
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
			if (tabular.row_of_cell(cur.idx()) != 0) {
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
		if (tablemode(cur)) {
			cur.pit() = cur.lastpit();
			cur.pos() = cur.lastpos();
			return;
		}
		break;

//	case LFUN_SCREEN_DOWN: {
//		//if (hasSelection())
//		//	cur.selection() = false;
//		col_type const col = tabular.column_of_cell(cur.idx());
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
//		col_type const col = tabular.column_of_cell(cur.idx());
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
		docstring const tmpstr =
			getContentsOfPlaintextFile(&cur.bv(), to_utf8(cmd.argument()), false);
		if (tmpstr.empty())
			break;
		recordUndoInset(cur, Undo::INSERT);
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
				recordUndoInset(cur, Undo::DELETE);
				cutSelection(cur);
			}
		}
		else
			cell(cur.idx())->dispatch(cur, cmd);
		break;

	case LFUN_CHAR_DELETE_BACKWARD:
	case LFUN_CHAR_DELETE_FORWARD:
		if (tablemode(cur)) {
			recordUndoInset(cur, Undo::DELETE);
			cutSelection(cur);
		}
		else
			cell(cur.idx())->dispatch(cur, cmd);
		break;

	case LFUN_SELF_INSERT:
		if (tablemode(cur)) {
			recordUndoInset(cur, Undo::DELETE);
			cutSelection(cur);
		}
		cell(cur.idx())->dispatch(cur, cmd);
		break;

	case LFUN_COPY:
		if (!cur.selection())
			break;
		if (tablemode(cur)) {
			finishUndo();
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
			recordUndoInset(cur, Undo::INSERT);
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
		if (tabularStackDirty() && (theClipboard().isInternal() ||
		    !theClipboard().hasInternal() && theClipboard().hasLyXContents())) {
			recordUndoInset(cur, Undo::INSERT);
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
	case LFUN_FONT_CODE:
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
					tmpcur.idx() = tabular.getCellNumber(i, j);
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
			status.clear();
			return true;

		case Tabular::MULTICOLUMN:
			status.setOnOff(tabular.isMultiColumn(cur.idx()));
			break;

		case Tabular::M_TOGGLE_LINE_TOP:
			flag = false;
		case Tabular::TOGGLE_LINE_TOP:
			status.setOnOff(tabular.topLine(cur.idx(), flag));
			break;

		case Tabular::M_TOGGLE_LINE_BOTTOM:
			flag = false;
		case Tabular::TOGGLE_LINE_BOTTOM:
			status.setOnOff(tabular.bottomLine(cur.idx(), flag));
			break;

		case Tabular::M_TOGGLE_LINE_LEFT:
			flag = false;
		case Tabular::TOGGLE_LINE_LEFT:
			status.setOnOff(tabular.leftLine(cur.idx(), flag));
			break;

		case Tabular::M_TOGGLE_LINE_RIGHT:
			flag = false;
		case Tabular::TOGGLE_LINE_RIGHT:
			status.setOnOff(tabular.rightLine(cur.idx(), flag));
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
	case LFUN_CHARSTYLE_INSERT:
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
	case LFUN_BREAK_LINE:
	case LFUN_BREAK_PARAGRAPH:
	case LFUN_BREAK_PARAGRAPH_KEEP_LAYOUT:
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
		if (translate(cmd.getArg(0)) == TABULAR_CODE) {
			status.enabled(true);
			return true;
		}
		// Fall through

	default:
		// we try to handle this event in the insets dispatch function.
		return cell(cur.idx())->getStatus(cur, cmd, status);
	}
}


int InsetTabular::latex(Buffer const & buf, odocstream & os,
			OutputParams const & runparams) const
{
	return tabular.latex(buf, os, runparams);
}


int InsetTabular::plaintext(Buffer const & buf, odocstream & os,
			    OutputParams const & runparams) const
{
	os << '\n'; // output table on a new line
	int const dp = runparams.linelen > 0 ? runparams.depth : 0;
	tabular.plaintext(buf, os, runparams, dp, false, 0);
	return PLAINTEXT_NEWLINE;
}


int InsetTabular::docbook(Buffer const & buf, odocstream & os,
			  OutputParams const & runparams) const
{
	int ret = 0;
	Inset * master = 0;

#ifdef WITH_WARNINGS
#warning Why not pass a proper DocIterator here?
#endif
#if 0
	// if the table is inside a float it doesn't need the informaltable
	// wrapper. Search for it.
	for (master = owner(); master; master = master->owner())
		if (master->lyxCode() == Inset::FLOAT_CODE)
			break;
#endif

	if (!master) {
		os << "<informaltable>";
		++ret;
	}
	ret += tabular.docbook(buf, os, runparams);
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


shared_ptr<InsetText const> InsetTabular::cell(idx_type idx) const
{
	return tabular.getCellInset(idx);
}


shared_ptr<InsetText> InsetTabular::cell(idx_type idx)
{
	return tabular.getCellInset(idx);
}


void InsetTabular::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const
{
	cell(sl.idx())->cursorPos(bv, sl, boundary, x, y);

	// y offset	correction
	int const row = tabular.row_of_cell(sl.idx());
	for (int i = 0;	i <= row; ++i) {
		if (i != 0) {
			y += tabular.getAscentOfRow(i);
			y += tabular.getAdditionalHeight(i);
		}
		if (i != row)
			y += tabular.getDescentOfRow(i);
	}

	// x offset correction
	int const col = tabular.column_of_cell(sl.idx());
	int idx = tabular.getCellNumber(row, 0);
	for (int j = 0; j < col; ++j) {
		if (tabular.isPartOfMultiColumn(row, j))
			continue;
		x += tabular.getWidthOfColumn(idx);
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
	int const xend = xbeg + tabular.getWidthOfColumn(cell);
	row_type const row = tabular.row_of_cell(cell);
	int const ybeg = o.y_ - tabular.getAscentOfRow(row) -
			 tabular.getAdditionalHeight(row);
	int const yend = o.y_ + tabular.getDescentOfRow(row);

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
	return cell(cur.idx())->text_.editXY(cur, x, y);
}


void InsetTabular::setCursorFromCoordinates(Cursor & cur, int x, int y) const
{
	cur.idx() = getNearestCell(cur.bv(), x, y);
	cell(cur.idx())->text_.setCursorFromCoordinates(cur, x, y);
}


InsetTabular::idx_type InsetTabular::getNearestCell(BufferView & bv, int x, int y) const
{
	idx_type idx_min = 0;
	int dist_min = std::numeric_limits<int>::max();
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
		lx += tabular.getWidthOfColumn(c);

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
		int const x2 = x1 + tabular.getWidthOfColumn(cur.idx());

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
			row_type const row = tabular.row_of_cell(cur.idx());
			if (row == tabular.rows() - 1)
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
 
 	cur.boundary(false);
 
 	if (tablemode(cur)) {
 		cur.pit() = cur.lastpit();
 		cur.pos() = cur.lastpos();
 		resetPos(cur);
 		return;
 	}

	cur.pit() = 0;
	cur.pos() = 0;
	resetPos(cur);
}


void InsetTabular::movePrevCell(Cursor & cur)
{
	if (isRightToLeft(cur)) {
		if (tabular.isLastCellInRow(cur.idx())) {
			row_type const row = tabular.row_of_cell(cur.idx());
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

	if (tablemode(cur)) {
		cur.pit() = cur.lastpit();
		cur.pos() = cur.lastpos();
		resetPos(cur);
		return;
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
			if (tabular.getRotateCell(tabular.getCellNumber(i, j))
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
	BufferView & bv = cur.bv();
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

	recordUndoInset(cur, Undo::ATOMIC);

	getSelection(cur, sel_row_start, sel_row_end, sel_col_start, sel_col_end);
	row_type const row = tabular.row_of_cell(cur.idx());
	col_type const column = tabular.column_of_cell(cur.idx());
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
		tabular.appendRow(bv.buffer()->params(), cur.idx());
		break;

	case Tabular::APPEND_COLUMN:
		// append the column into the tabular
		tabular.appendColumn(bv.buffer()->params(), cur.idx());
		cur.idx() = tabular.getCellNumber(row, column);
		break;

	case Tabular::DELETE_ROW:
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			tabular.deleteRow(sel_row_start);
		if (sel_row_start >= tabular.rows())
			--sel_row_start;
		cur.idx() = tabular.getCellNumber(sel_row_start, column);
		cur.pit() = 0;
		cur.pos() = 0;
		cur.selection() = false;
		break;

	case Tabular::DELETE_COLUMN:
		for (col_type i = sel_col_start; i <= sel_col_end; ++i)
			tabular.deleteColumn(sel_col_start);
		if (sel_col_start >= tabular.columns())
			--sel_col_start;
		cur.idx() = tabular.getCellNumber(row, sel_col_start);
		cur.pit() = 0;
		cur.pos() = 0;
		cur.selection() = false;
		break;

	case Tabular::COPY_ROW:
		tabular.copyRow(bv.buffer()->params(), row);
		break;

	case Tabular::COPY_COLUMN:
		tabular.copyColumn(bv.buffer()->params(), column);
		cur.idx() = tabular.getCellNumber(row, column);
		break;

	case Tabular::M_TOGGLE_LINE_TOP:
		flag = false;
	case Tabular::TOGGLE_LINE_TOP: {
		bool lineSet = !tabular.topLine(cur.idx(), flag);
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setTopLine(
					tabular.getCellNumber(i, j),
					lineSet, flag);
		break;
	}

	case Tabular::M_TOGGLE_LINE_BOTTOM:
		flag = false;
	case Tabular::TOGGLE_LINE_BOTTOM: {
		bool lineSet = !tabular.bottomLine(cur.idx(), flag);
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setBottomLine(
					tabular.getCellNumber(i, j),
					lineSet,
					flag);
		break;
	}

	case Tabular::M_TOGGLE_LINE_LEFT:
		flag = false;
	case Tabular::TOGGLE_LINE_LEFT: {
		bool lineSet = !tabular.leftLine(cur.idx(), flag);
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setLeftLine(
					tabular.getCellNumber(i,j),
					lineSet,
					flag);
		break;
	}

	case Tabular::M_TOGGLE_LINE_RIGHT:
		flag = false;
	case Tabular::TOGGLE_LINE_RIGHT: {
		bool lineSet = !tabular.rightLine(cur.idx(), flag);
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setRightLine(
					tabular.getCellNumber(i,j),
					lineSet,
					flag);
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
				tabular.setAlignment(
					tabular.getCellNumber(i, j),
					setAlign,
					flag);
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
				tabular.setVAlignment(
					tabular.getCellNumber(i, j),
					setVAlign, flag);
		break;

	case Tabular::MULTICOLUMN: {
		if (sel_row_start != sel_row_end) {
#ifdef WITH_WARNINGS
#warning Need I say it ? This is horrible.
#endif
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
				tabular.setMultiColumn(bv.buffer(), cur.idx(), 1);
			break;
		}
		// we have a selection so this means we just add all this
		// cells to form a multicolumn cell
		idx_type const s_start = cur.selBegin().idx();
		idx_type const s_end = cur.selEnd().idx();
		tabular.setMultiColumn(bv.buffer(), s_start, s_end - s_start + 1);
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
					tabular.getCellNumber(i,j), setLines);
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
					tabular.getCellNumber(i, j), true);
		break;

	case Tabular::UNSET_ROTATE_CELL:
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setRotateCell(
					tabular.getCellNumber(i, j), false);
		break;

	case Tabular::TOGGLE_ROTATE_CELL:
		{
		bool oneNotRotated = oneCellHasRotationState(false,
			sel_row_start, sel_row_end, sel_col_start, sel_col_end);

		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setRotateCell(tabular.getCellNumber(i, j),
									  oneNotRotated);
		}
		break;

	case Tabular::SET_USEBOX: {
		Tabular::BoxType val = Tabular::BoxType(convert<int>(value));
		if (val == tabular.getUsebox(cur.idx()))
			val = Tabular::BOX_NONE;
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setUsebox(tabular.getCellNumber(i, j), val);
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
	while (paste_tabular->rows() > rows)
		paste_tabular->deleteRow(rows);

	paste_tabular->setTopLine(0, true, true);
	paste_tabular->setBottomLine(paste_tabular->getFirstCellInRow(rows - 1),
				     true, true);

	for (col_type i = 0; i < cs; ++i)
		paste_tabular->deleteColumn(0);

	col_type const columns = ce - cs + 1;
	while (paste_tabular->columns() > columns)
		paste_tabular->deleteColumn(columns);

	paste_tabular->setLeftLine(0, true, true);
	paste_tabular->setRightLine(paste_tabular->getLastCellInRow(0),
				    true, true);

	odocstringstream os;
	OutputParams const runparams(0);
	paste_tabular->plaintext(cur.buffer(), os, runparams, 0, true, '\t');
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
	col_type const actcol = tabular.column_of_cell(cur.idx());
	row_type const actrow = tabular.row_of_cell(cur.idx());
	for (row_type r1 = 0, r2 = actrow;
	     r1 < paste_tabular->rows() && r2 < tabular.rows();
	     ++r1, ++r2) {
		for (col_type c1 = 0, c2 = actcol;
		    c1 < paste_tabular->columns() && c2 < tabular.columns();
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
			shared_ptr<InsetText> inset(
				new InsetText(*paste_tabular->getCellInset(r1, c1)));
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
			shared_ptr<InsetText> t
				= cell(tabular.getCellNumber(i, j));
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
	return parentpar.getFontSettings(cur.bv().buffer()->params(),
					 parentpos).language()->rightToLeft();
}


void InsetTabular::getSelection(Cursor & cur,
	row_type & rs, row_type & re, col_type & cs, col_type & ce) const
{
	CursorSlice const & beg = cur.selBegin();
	CursorSlice const & end = cur.selEnd();
	cs = tabular.column_of_cell(beg.idx());
	ce = tabular.column_of_cell(end.idx());
	if (cs > ce) {
		ce = cs;
		cs = tabular.column_of_cell(end.idx());
	} else {
		ce = tabular.right_column_of_cell(end.idx());
	}

	rs = tabular.row_of_cell(beg.idx());
	re = tabular.row_of_cell(end.idx());
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


bool InsetTabular::forceDefaultParagraphs(idx_type cell) const
{
	return tabular.getPWidth(cell).zero();
}


bool InsetTabular::insertPlaintextString(BufferView & bv, docstring const & buf,
				     bool usePaste)
{
	if (buf.length() <= 0)
		return true;

	Buffer const & buffer = *bv.buffer();

	col_type cols = 1;
	row_type rows = 1;
	col_type maxCols = 1;
	docstring::size_type const len = buf.length();
	docstring::size_type p = 0;

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
		paste_tabular.reset(
			new Tabular(buffer.params(), rows, maxCols));
		loctab = paste_tabular.get();
		cols = 0;
		dirtyTabularStack(true);
	} else {
		loctab = &tabular;
		cell = bv.cursor().idx();
		ocol = tabular.column_of_cell(cell);
		row = tabular.row_of_cell(cell);
	}

	docstring::size_type op = 0;
	idx_type const cells = loctab->getNumberOfCells();
	p = 0;
	cols = ocol;
	rows = loctab->rows();
	col_type const columns = loctab->columns();

	while (cell < cells && p < len && row < rows &&
	       (p = buf.find_first_of(from_ascii("\t\n"), p)) != docstring::npos)
	{
		if (p >= len)
			break;
		switch (buf[p]) {
		case '\t':
			// we can only set this if we are not too far right
			if (cols < columns) {
				shared_ptr<InsetText> inset = loctab->getCellInset(cell);
				Paragraph & par = inset->text_.getPar(0);
				Font const font = inset->text_.getFont(buffer, par, 0);
				inset->setText(buf.substr(op, p - op), font,
					       buffer.params().trackChanges);
				++cols;
				++cell;
			}
			break;
		case '\n':
			// we can only set this if we are not too far right
			if (cols < columns) {
				shared_ptr<InsetText> inset = tabular.getCellInset(cell);
				Paragraph & par = inset->text_.getPar(0);
				Font const font = inset->text_.getFont(buffer, par, 0);
				inset->setText(buf.substr(op, p - op), font,
					       buffer.params().trackChanges);
			}
			cols = ocol;
			++row;
			if (row < rows)
				cell = loctab->getCellNumber(row, cols);
			break;
		}
		++p;
		op = p;
	}
	// check for the last cell if there is no trailing '\n'
	if (cell < cells && op < len) {
		shared_ptr<InsetText> inset = loctab->getCellInset(cell);
		Paragraph & par = inset->text_.getPar(0);
		Font const font = inset->text_.getFont(buffer, par, 0);
		inset->setText(buf.substr(op, len - op), font,
			buffer.params().trackChanges);
	}
	return true;
}


void InsetTabular::addPreview(PreviewLoader & loader) const
{
	row_type const rows = tabular.rows();
	col_type const columns = tabular.columns();
	for (row_type i = 0; i < rows; ++i) {
		for (col_type j = 0; j < columns; ++j)
			tabular.getCellInset(i, j)->addPreview(loader);
	}
}


bool InsetTabular::tablemode(Cursor & cur) const
{
	return cur.selection() && cur.selBegin().idx() != cur.selEnd().idx();
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
		return print_mailer_error("InsetTabularMailer", in, 2,
					  "Tabular");

	Buffer const & buffer = inset.buffer();
	inset.read(buffer, lex);
}


string const InsetTabularMailer::params2string(InsetTabular const & inset)
{
	ostringstream data;
	data << name_ << ' ';
	inset.write(inset.buffer(), data);
	data << "\\end_inset\n";
	return data.str();
}


} // namespace lyx
