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
 * \author Uwe Stöhr
 * \author Edwin Leuven
 * \author Scott Kostyshak
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
#include "InsetList.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "LyX.h"
#include "LyXRC.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "output_xhtml.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "TextClass.h"
#include "TextMetrics.h"

#include "frontends/Application.h"
#include "frontends/alert.h"
#include "frontends/Clipboard.h"
#include "frontends/Painter.h"
#include "frontends/Selection.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/FileName.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

#include <boost/scoped_ptr.hpp>

#include <cstring>
#include <iostream>
#include <limits>
#include <sstream>

using namespace std;
using namespace lyx::support;



namespace lyx {

using cap::dirtyTabularStack;
using cap::tabularStackDirty;

using graphics::PreviewLoader;

using frontend::Painter;
using frontend::Clipboard;

namespace Alert = frontend::Alert;


namespace {

int const ADD_TO_HEIGHT = 2; // in cell
int const ADD_TO_TABULAR_WIDTH = 6; // horizontal space before and after the table
int const default_line_space = 10; // ?
int const WIDTH_OF_LINE = 5; // space between double lines


///
boost::scoped_ptr<Tabular> paste_tabular;


struct TabularFeature {
	Tabular::Feature action;
	string feature;
	bool need_value;
};


TabularFeature tabularFeature[] =
{
	// the SET/UNSET actions are used by the table dialog,
	// the TOGGLE actions by the table toolbar buttons
	{ Tabular::APPEND_ROW, "append-row", false },
	{ Tabular::APPEND_COLUMN, "append-column", false },
	{ Tabular::DELETE_ROW, "delete-row", false },
	{ Tabular::DELETE_COLUMN, "delete-column", false },
	{ Tabular::COPY_ROW, "copy-row", false },
	{ Tabular::COPY_COLUMN, "copy-column", false },
	{ Tabular::MOVE_COLUMN_RIGHT, "move-column-right", false },
	{ Tabular::MOVE_COLUMN_LEFT, "move-column-left", false },
	{ Tabular::MOVE_ROW_DOWN, "move-row-down", false },
	{ Tabular::MOVE_ROW_UP, "move-row-up", false },
	{ Tabular::SET_LINE_TOP, "set-line-top", true },
	{ Tabular::SET_LINE_BOTTOM, "set-line-bottom", true },
	{ Tabular::SET_LINE_LEFT, "set-line-left", true },
	{ Tabular::SET_LINE_RIGHT, "set-line-right", true },
	{ Tabular::TOGGLE_LINE_TOP, "toggle-line-top", false },
	{ Tabular::TOGGLE_LINE_BOTTOM, "toggle-line-bottom", false },
	{ Tabular::TOGGLE_LINE_LEFT, "toggle-line-left", false },
	{ Tabular::TOGGLE_LINE_RIGHT, "toggle-line-right", false },
	{ Tabular::ALIGN_LEFT, "align-left", false },
	{ Tabular::ALIGN_RIGHT, "align-right", false },
	{ Tabular::ALIGN_CENTER, "align-center", false },
	{ Tabular::ALIGN_BLOCK, "align-block", false },
	{ Tabular::ALIGN_DECIMAL, "align-decimal", false },
	{ Tabular::VALIGN_TOP, "valign-top", false },
	{ Tabular::VALIGN_BOTTOM, "valign-bottom", false },
	{ Tabular::VALIGN_MIDDLE, "valign-middle", false },
	{ Tabular::M_ALIGN_LEFT, "m-align-left", false },
	{ Tabular::M_ALIGN_RIGHT, "m-align-right", false },
	{ Tabular::M_ALIGN_CENTER, "m-align-center", false },
	{ Tabular::M_VALIGN_TOP, "m-valign-top", false },
	{ Tabular::M_VALIGN_BOTTOM, "m-valign-bottom", false },
	{ Tabular::M_VALIGN_MIDDLE, "m-valign-middle", false },
	{ Tabular::MULTICOLUMN, "multicolumn", false },
	{ Tabular::SET_MULTICOLUMN, "set-multicolumn", false },
	{ Tabular::UNSET_MULTICOLUMN, "unset-multicolumn", false },
	{ Tabular::MULTIROW, "multirow", false },
	{ Tabular::SET_MULTIROW, "set-multirow", false },
	{ Tabular::UNSET_MULTIROW, "unset-multirow", false },
	{ Tabular::SET_MROFFSET, "set-mroffset", true },
	{ Tabular::SET_ALL_LINES, "set-all-lines", false },
	{ Tabular::UNSET_ALL_LINES, "unset-all-lines", false },
	{ Tabular::SET_LONGTABULAR, "set-longtabular", false },
	{ Tabular::UNSET_LONGTABULAR, "unset-longtabular", false },
	{ Tabular::SET_PWIDTH, "set-pwidth", true },
	{ Tabular::SET_MPWIDTH, "set-mpwidth", true },
	{ Tabular::SET_ROTATE_TABULAR, "set-rotate-tabular", true },
	{ Tabular::UNSET_ROTATE_TABULAR, "unset-rotate-tabular", true },
	{ Tabular::TOGGLE_ROTATE_TABULAR, "toggle-rotate-tabular", true },
	{ Tabular::SET_ROTATE_CELL, "set-rotate-cell", true },
	{ Tabular::UNSET_ROTATE_CELL, "unset-rotate-cell", true },
	{ Tabular::TOGGLE_ROTATE_CELL, "toggle-rotate-cell", true },
	{ Tabular::SET_USEBOX, "set-usebox", true },
	{ Tabular::SET_LTHEAD, "set-lthead", true },
	{ Tabular::UNSET_LTHEAD, "unset-lthead", true },
	{ Tabular::SET_LTFIRSTHEAD, "set-ltfirsthead", true },
	{ Tabular::UNSET_LTFIRSTHEAD, "unset-ltfirsthead", true },
	{ Tabular::SET_LTFOOT, "set-ltfoot", true },
	{ Tabular::UNSET_LTFOOT, "unset-ltfoot", true },
	{ Tabular::SET_LTLASTFOOT, "set-ltlastfoot", true },
	{ Tabular::UNSET_LTLASTFOOT, "unset-ltlastfoot", true },
	{ Tabular::SET_LTNEWPAGE, "set-ltnewpage", false },
	{ Tabular::UNSET_LTNEWPAGE, "unset-ltnewpage", false },
	{ Tabular::TOGGLE_LTCAPTION, "toggle-ltcaption", false },
	{ Tabular::SET_LTCAPTION, "set-ltcaption", false },
	{ Tabular::UNSET_LTCAPTION, "unset-ltcaption", false },
	{ Tabular::SET_SPECIAL_COLUMN, "set-special-column", true },
	{ Tabular::SET_SPECIAL_MULTICOLUMN, "set-special-multicolumn", true },
	{ Tabular::SET_BOOKTABS, "set-booktabs", false },
	{ Tabular::UNSET_BOOKTABS, "unset-booktabs", false },
	{ Tabular::SET_TOP_SPACE, "set-top-space", true },
	{ Tabular::SET_BOTTOM_SPACE, "set-bottom-space", true },
	{ Tabular::SET_INTERLINE_SPACE, "set-interline-space", true },
	{ Tabular::SET_BORDER_LINES, "set-border-lines", false },
	{ Tabular::TABULAR_VALIGN_TOP, "tabular-valign-top", false},
	{ Tabular::TABULAR_VALIGN_MIDDLE, "tabular-valign-middle", false},
	{ Tabular::TABULAR_VALIGN_BOTTOM, "tabular-valign-bottom", false},
	{ Tabular::LONGTABULAR_ALIGN_LEFT, "longtabular-align-left", false },
	{ Tabular::LONGTABULAR_ALIGN_CENTER, "longtabular-align-center", false },
	{ Tabular::LONGTABULAR_ALIGN_RIGHT, "longtabular-align-right", false },
	{ Tabular::SET_DECIMAL_POINT, "set-decimal-point", true },
	{ Tabular::SET_TABULAR_WIDTH, "set-tabular-width", true },
	{ Tabular::LAST_ACTION, "", false }
};


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
	case LYX_ALIGN_DECIMAL:
		return "decimal";
	}
	return string();
}


string const tostr(Tabular::HAlignment const & num)
{
	switch (num) {
	case Tabular::LYX_LONGTABULAR_ALIGN_LEFT:
		return "left";
	case Tabular::LYX_LONGTABULAR_ALIGN_CENTER:
		return "center";
	case Tabular::LYX_LONGTABULAR_ALIGN_RIGHT:
		return "right";
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
	else if (str == "decimal")
		num = LYX_ALIGN_DECIMAL;
	else
		return false;
	return true;
}


bool string2type(string const str, Tabular::HAlignment & num)
{
	if (str == "left")
		num = Tabular::LYX_LONGTABULAR_ALIGN_LEFT;
	else if (str == "center" )
		num = Tabular::LYX_LONGTABULAR_ALIGN_CENTER;
	else if (str == "right")
		num = Tabular::LYX_LONGTABULAR_ALIGN_RIGHT;
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
				   Tabular::HAlignment & num)
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

} // namespace


string const featureAsString(Tabular::Feature action)
{
	for (size_t i = 0; i != Tabular::LAST_ACTION; ++i) {
		if (tabularFeature[i].action == action)
			return tabularFeature[i].feature;
	}
	return string();
}


DocIterator separatorPos(InsetTableCell * cell, docstring const & align_d)
{
	DocIterator dit = doc_iterator_begin(&(cell->buffer()), cell);
	for (; dit; dit.forwardChar())
		if (dit.inTexted() && dit.depth() == 1
			&& dit.paragraph().find(align_d, false, false, dit.pos()))
			break;

	return dit;
}


InsetTableCell splitCell(InsetTableCell & head, docstring const align_d, bool & hassep)
{
	InsetTableCell tail = InsetTableCell(head);
	DocIterator const dit = separatorPos(&head, align_d);
	hassep = dit;
	if (hassep) {
		pit_type const psize = head.paragraphs().front().size();
		head.paragraphs().front().eraseChars(dit.pos(), psize, false);
		tail.paragraphs().front().eraseChars(0, 
			dit.pos() < psize ? dit.pos() + 1 : psize, false);
	}

	return tail;
}


/////////////////////////////////////////////////////////////////////
//
// Tabular
//
/////////////////////////////////////////////////////////////////////


Tabular::CellData::CellData(Buffer * buf)
	: cellno(0),
	  width(0),
	  multicolumn(Tabular::CELL_NORMAL),
	  multirow(Tabular::CELL_NORMAL),
	  alignment(LYX_ALIGN_CENTER),
	  valignment(LYX_VALIGN_TOP),
	  decimal_hoffset(0),
	  decimal_width(0),
	  voffset(0),
	  top_line(false),
	  bottom_line(false),
	  left_line(false),
	  right_line(false),
	  usebox(BOX_NONE),
	  rotate(0),
	  inset(new InsetTableCell(buf))
{
	inset->setBuffer(*buf);
}


Tabular::CellData::CellData(CellData const & cs)
	: cellno(cs.cellno),
	  width(cs.width),
	  multicolumn(cs.multicolumn),
	  multirow(cs.multirow),
	  mroffset(cs.mroffset),
	  alignment(cs.alignment),
	  valignment(cs.valignment),
	  decimal_hoffset(cs.decimal_hoffset),
	  decimal_width(cs.decimal_width),
	  voffset(cs.voffset),
	  top_line(cs.top_line),
	  bottom_line(cs.bottom_line),
	  left_line(cs.left_line),
	  right_line(cs.right_line),
	  usebox(cs.usebox),
	  rotate(cs.rotate),
	  align_special(cs.align_special),
	  p_width(cs.p_width),
	  inset(static_cast<InsetTableCell *>(cs.inset->clone()))
{
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
	std::swap(multirow, rhs.multirow);
	std::swap(mroffset, rhs.mroffset);
	std::swap(alignment, rhs.alignment);
	std::swap(valignment, rhs.valignment);
	std::swap(decimal_hoffset, rhs.decimal_hoffset);
	std::swap(decimal_width, rhs.decimal_width);
	std::swap(voffset, rhs.voffset);
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
	  newpage(false),
	  caption(false)
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


Tabular::Tabular(Buffer * buffer, row_type rows_arg, col_type columns_arg)
{
	init(buffer, rows_arg, columns_arg);
}


void Tabular::setBuffer(Buffer & buffer)
{
	buffer_ = &buffer;
	for (row_type i = 0; i < nrows(); ++i)
		for (col_type j = 0; j < ncols(); ++j)
			cell_info[i][j].inset->setBuffer(*buffer_);
}


// activates all lines and sets all widths to 0
void Tabular::init(Buffer * buf, row_type rows_arg,
		      col_type columns_arg)
{
	buffer_ = buf;
	row_info = row_vector(rows_arg);
	column_info = column_vector(columns_arg);
	cell_info = cell_vvector(rows_arg, cell_vector(columns_arg, CellData(buf)));
	row_info.reserve(10);
	column_info.reserve(10);
	cell_info.reserve(100);
	updateIndexes();
	is_long_tabular = false;
	tabular_valignment = LYX_VALIGN_MIDDLE;
	tabular_width = Length();
	longtabular_alignment = LYX_LONGTABULAR_ALIGN_CENTER;
	rotate = 0;
	use_booktabs = false;
	// set silly default lines
	for (row_type r = 0; r < nrows(); ++r)
		for (col_type c = 0; c < ncols(); ++c) {
			cell_info[r][c].inset->setBuffer(*buffer_);
			cell_info[r][c].top_line = true;
			cell_info[r][c].left_line = true;
			cell_info[r][c].bottom_line = r == 0 || r == nrows() - 1;
			cell_info[r][c].right_line = c == ncols() - 1;
		}
}


void Tabular::deleteRow(row_type const row)
{
	// Not allowed to delete last row
	if (nrows() == 1)
		return;

	for (col_type c = 0; c < ncols(); ++c) {
		// Care about multirow cells
		if (row + 1 < nrows() &&
		    cell_info[row][c].multirow == CELL_BEGIN_OF_MULTIROW &&
		    cell_info[row + 1][c].multirow == CELL_PART_OF_MULTIROW) {
				cell_info[row + 1][c].multirow = CELL_BEGIN_OF_MULTIROW;
		}
	}
	row_info.erase(row_info.begin() + row);
	cell_info.erase(cell_info.begin() + row);
	updateIndexes();
}


void Tabular::copyRow(row_type const row)
{
	insertRow(row, true);
}


void Tabular::appendRow(row_type row)
{
	insertRow(row, false);
}


void Tabular::insertRow(row_type const row, bool copy)
{
	row_info.insert(row_info.begin() + row + 1, RowData(row_info[row]));
	cell_info.insert(cell_info.begin() + row + 1, 
		cell_vector(0, CellData(buffer_)));
	
	for (col_type c = 0; c < ncols(); ++c) {
		cell_info[row + 1].insert(cell_info[row + 1].begin() + c,
			copy ? CellData(cell_info[row][c]) : CellData(buffer_));
		if (buffer().params().trackChanges)
			cell_info[row + 1][c].inset->setChange(Change(Change::INSERTED));
		if (cell_info[row][c].multirow == CELL_BEGIN_OF_MULTIROW)
			cell_info[row + 1][c].multirow = CELL_PART_OF_MULTIROW;
	}
	
	updateIndexes();
	for (col_type c = 0; c < ncols(); ++c) {
		if (isPartOfMultiRow(row, c))
			continue;
		// inherit line settings
		idx_type const i = cellIndex(row + 1, c);
		idx_type const j = cellIndex(row, c);
		setLeftLine(i, leftLine(j));
		setRightLine(i, rightLine(j));
		setTopLine(i, topLine(j));
		if (topLine(j) && bottomLine(j)) {
			setBottomLine(i, true);
			setBottomLine(j, false);
		}
		// mark track changes
		if (buffer().params().trackChanges)
			cellInfo(i).inset->setChange(Change(Change::INSERTED));
	}
}


void Tabular::moveColumn(col_type col, ColDirection direction)
{
	if (direction == Tabular::LEFT)
		col = col - 1;

	std::swap(column_info[col], column_info[col + 1]);

	for (row_type r = 0; r < nrows(); ++r) {
		std::swap(cell_info[r][col], cell_info[r][col + 1]);
		std::swap(cell_info[r][col].left_line, cell_info[r][col + 1].left_line);
		std::swap(cell_info[r][col].right_line, cell_info[r][col + 1].right_line);

		// FIXME track changes is broken for tabular features (#8469)
		idx_type const i = cellIndex(r, col);
		idx_type const j = cellIndex(r, col + 1);
		if (buffer().params().trackChanges) {
			cellInfo(i).inset->setChange(Change(Change::INSERTED));
			cellInfo(j).inset->setChange(Change(Change::INSERTED));
		}
	}
	updateIndexes();
}


void Tabular::moveRow(row_type row, RowDirection direction)
{
	if (direction == Tabular::UP)
		row = row - 1;

	std::swap(row_info[row], row_info[row + 1]);

	for (col_type c = 0; c < ncols(); ++c) {
		std::swap(cell_info[row][c], cell_info[row + 1][c]);
		std::swap(cell_info[row][c].top_line, cell_info[row + 1][c].top_line);
		std::swap(cell_info[row][c].bottom_line, cell_info[row + 1][c].bottom_line);

		// FIXME track changes is broken for tabular features (#8469)
		idx_type const i = cellIndex(row, c);
		idx_type const j = cellIndex(row + 1, c);
		if (buffer().params().trackChanges) {
			cellInfo(i).inset->setChange(Change(Change::INSERTED));
			cellInfo(j).inset->setChange(Change(Change::INSERTED));
		}
	}
	updateIndexes();
}


void Tabular::deleteColumn(col_type const col)
{
	// Not allowed to delete last column
	if (ncols() == 1)
		return;

	for (row_type r = 0; r < nrows(); ++r) {
		// Care about multicolumn cells
		if (col + 1 < ncols() &&
		    cell_info[r][col].multicolumn == CELL_BEGIN_OF_MULTICOLUMN &&
		    cell_info[r][col + 1].multicolumn == CELL_PART_OF_MULTICOLUMN) {
				cell_info[r][col + 1].multicolumn = CELL_BEGIN_OF_MULTICOLUMN;
		}
		cell_info[r].erase(cell_info[r].begin() + col);
	}
	column_info.erase(column_info.begin() + col);
	updateIndexes();
}


void Tabular::copyColumn(col_type const col)
{
	insertColumn(col, true);
}


void Tabular::appendColumn(col_type col)
{	
	insertColumn(col, false);
}


void Tabular::insertColumn(col_type const col, bool copy)
{
	BufferParams const & bp = buffer().params();
	column_info.insert(column_info.begin() + col + 1, ColumnData(column_info[col]));

	for (row_type r = 0; r < nrows(); ++r) {
		cell_info[r].insert(cell_info[r].begin() + col + 1,
			copy ? CellData(cell_info[r][col]) : CellData(buffer_));
		if (bp.trackChanges)
			cell_info[r][col + 1].inset->setChange(Change(Change::INSERTED));
		if (cell_info[r][col].multicolumn == CELL_BEGIN_OF_MULTICOLUMN)
			cell_info[r][col + 1].multicolumn = CELL_PART_OF_MULTICOLUMN;
	}
	updateIndexes();
	for (row_type r = 0; r < nrows(); ++r) {
		// inherit line settings
		idx_type const i = cellIndex(r, col + 1);
		idx_type const j = cellIndex(r, col);
		setBottomLine(i, bottomLine(j));
		setTopLine(i, topLine(j));
		setLeftLine(i, leftLine(j));
		if (rightLine(j) && rightLine(j)) {
			setRightLine(i, true);
			setRightLine(j, false);
		}
		if (buffer().params().trackChanges)
			cellInfo(i).inset->setChange(Change(Change::INSERTED));
	}
}


void Tabular::updateIndexes()
{
	setBuffer(buffer());
	numberofcells = 0;
	// reset cell number
	for (row_type row = 0; row < nrows(); ++row)
		for (col_type column = 0; column < ncols(); ++column) {
			if (!isPartOfMultiColumn(row, column)
				&& !isPartOfMultiRow(row, column))
				++numberofcells;
			if (isPartOfMultiRow(row, column))
				cell_info[row][column].cellno = cell_info[row - 1][column].cellno;
			else
				cell_info[row][column].cellno = numberofcells - 1;
		}

	rowofcell.resize(numberofcells);
	columnofcell.resize(numberofcells);
	idx_type i = 0;
	// reset column and row of cells and update their width and alignment
	for (row_type row = 0; row < nrows(); ++row)
		for (col_type column = 0; column < ncols(); ++column) {
			if (isPartOfMultiColumn(row, column))
				continue;
			// columnofcell needs to be called before setting width and aligment
			// multirow cells inherit the width from the column width
			if (!isPartOfMultiRow(row, column)) {
				columnofcell[i] = column;
				rowofcell[i] = row;
			}
			setFixedWidth(row, column);
			if (isPartOfMultiRow(row, column))
				continue;
			cell_info[row][column].inset->setContentAlignment(
				getAlignment(cellIndex(row, column)));
			++i;
		}
}


Tabular::idx_type Tabular::numberOfCellsInRow(row_type const row) const
{
	idx_type result = 0;
	for (col_type c = 0; c < ncols(); ++c)
		if (cell_info[row][c].multicolumn != Tabular::CELL_PART_OF_MULTICOLUMN)
			++result;
	return result;
}


bool Tabular::topLine(idx_type const cell) const
{
	return cellInfo(cell).top_line;
}


bool Tabular::bottomLine(idx_type const cell) const
{
	return cellInfo(cell).bottom_line;
}


bool Tabular::leftLine(idx_type cell) const
{
	if (use_booktabs)
		return false;
	return cellInfo(cell).left_line;
}


bool Tabular::rightLine(idx_type cell) const
{
	if (use_booktabs)
		return false;
	return cellInfo(cell).right_line;
}


int Tabular::interRowSpace(row_type row) const
{
	if (!row || row >= nrows())
		return 0;

	int const interline_space = row_info[row - 1].interline_space_default ?
		default_line_space :
		row_info[row - 1].interline_space.inPixels(width());
	if (rowTopLine(row) && rowBottomLine(row - 1))
		return interline_space + WIDTH_OF_LINE;
	return interline_space;
}


int Tabular::interColumnSpace(idx_type cell) const
{
	col_type const nextcol = cellColumn(cell) + columnSpan(cell);
	if (rightLine(cell) && nextcol < ncols()
		&& leftLine(cellIndex(cellRow(cell), nextcol)))
		return WIDTH_OF_LINE;
	return 0;
}


int Tabular::cellWidth(idx_type cell) const
{
	int w = 0;
	col_type const span = columnSpan(cell);
	col_type const col = cellColumn(cell);
	for(col_type c = col; c < col + span ; ++c)
		w += column_info[c].width;
	return w;
}


int Tabular::cellHeight(idx_type cell) const
{
	row_type const span = rowSpan(cell);
	row_type const row = cellRow(cell);
	int h = 0;
	for(row_type r = row; r < row + span ; ++r) {
		h += rowAscent(r) + rowDescent(r);
		if (r != row + span - 1)
			h += interRowSpace(r + 1);
	}

	return h;
}


bool Tabular::updateColumnWidths()
{
	vector<int> max_dwidth(ncols(), 0);
	for(col_type c = 0; c < ncols(); ++c)
		for(row_type r = 0; r < nrows(); ++r) {
			idx_type const i = cellIndex(r, c);
			if (getAlignment(i) == LYX_ALIGN_DECIMAL)
				max_dwidth[c] = max(max_dwidth[c], cell_info[r][c].decimal_width);
		}

	bool update = false;
	// for each col get max of single col cells
	for(col_type c = 0; c < ncols(); ++c) {
		int new_width = 0;
		for(row_type r = 0; r < nrows(); ++r) {
			idx_type const i = cellIndex(r, c);
			if (columnSpan(i) == 1) {
				if (getAlignment(i) == LYX_ALIGN_DECIMAL
					&& cell_info[r][c].decimal_width!=0)
					new_width = max(new_width, cellInfo(i).width 
				                + max_dwidth[c] - cellInfo(i).decimal_width);
				else
					new_width = max(new_width, cellInfo(i).width);
			}
		}

		if (column_info[c].width != new_width) {
			column_info[c].width = new_width;
			update = true;
		}
	}
	// update col widths to fit merged cells
	for(col_type c = 0; c < ncols(); ++c)
		for(row_type r = 0; r < nrows(); ++r) {
			idx_type const i = cellIndex(r, c);
			int const span = columnSpan(i);
			if (span == 1 || c > cellColumn(i))
				continue;

			int old_width = 0;
			for(col_type j = c; j < c + span ; ++j)
				old_width += column_info[j].width;

			if (cellInfo(i).width > old_width) {
				column_info[c + span - 1].width += cellInfo(i).width - old_width;
				update = true;
			}
		}

	return update;
}


int Tabular::width() const
{
	int width = 0;
	for (col_type c = 0; c < ncols(); ++c)
		width += column_info[c].width;
	return width;
}


void Tabular::setAlignment(idx_type cell, LyXAlignment align,
			      bool has_width)
{
	col_type const col = cellColumn(cell);
	// set alignment for the whole row if we are not in a multicolumn cell,
	// exclude possible multicolumn cells in the row
	if (!isMultiColumn(cell)) {
		for (row_type r = 0; r < nrows(); ++r) {
			// only if the column has no width the multirow inherits the
			// alignment of the column, otherwise it is left aligned
			if (!(isMultiRow(cellIndex(r, col)) && has_width)
				&& !isMultiColumn(cellIndex(r, col))) {
				cell_info[r][col].alignment = align;
				cell_info[r][col].inset->setContentAlignment(align);
			}
			if ((isMultiRow(cellIndex(r, col)) && has_width)
				&& !isMultiColumn(cellIndex(r, col))) {
				cell_info[r][col].alignment = LYX_ALIGN_LEFT;
				cell_info[r][col].inset->setContentAlignment(LYX_ALIGN_LEFT);
			}
		}
		column_info[col].alignment = align;
		docstring & dpoint = column_info[col].decimal_point;
		if (align == LYX_ALIGN_DECIMAL && dpoint.empty())
			dpoint = from_utf8(lyxrc.default_decimal_point);
	} else {
		cellInfo(cell).alignment = align; 
		cellInset(cell).get()->setContentAlignment(align); 
	}
}


void Tabular::setVAlignment(idx_type cell, VAlignment align,
			       bool onlycolumn)
{
	if (!isMultiColumn(cell) || onlycolumn)
		column_info[cellColumn(cell)].valignment = align;
	if (!onlycolumn)
		cellInfo(cell).valignment = align;
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
	inset->toggleFixedWidth(fixedWidth);
	if (fixedWidth)
		return;

	// merge all paragraphs to one
	BufferParams const & bp = cur.bv().buffer().params();
	while (inset->paragraphs().size() > 1)
		mergeParagraph(bp, inset->paragraphs(), 0);

	// reset layout
	cur.push(*inset);
	// undo information has already been recorded
	inset->getText(0)->setLayout(0, cur.lastpit() + 1,
			bp.documentClass().plainLayoutName());
	cur.pop();
}

}


void Tabular::setColumnPWidth(Cursor & cur, idx_type cell,
		Length const & width)
{
	col_type const c = cellColumn(cell);

	column_info[c].p_width = width;
	// reset the vertical alignment to top if the fixed width
	// is removed or zero because only fixed width columns can
	// have a vertical alignment
	if (column_info[c].p_width.zero())
		column_info[c].valignment = LYX_VALIGN_TOP;
	for (row_type r = 0; r < nrows(); ++r) {
		idx_type const cell = cellIndex(r, c);
		// because of multicolumns
		toggleFixedWidth(cur, cellInset(cell).get(),
				 !getPWidth(cell).zero());
		if (isMultiRow(cell))
			setAlignment(cell, LYX_ALIGN_LEFT, false);
	}
	// cur paragraph can become invalid after paragraphs were merged
	if (cur.pit() > cur.lastpit())
		cur.pit() = cur.lastpit();
	// cur position can become invalid after newlines were removed
	if (cur.pos() > cur.lastpos())
		cur.pos() = cur.lastpos();
}


bool Tabular::setFixedWidth(row_type r, col_type c)
{
	bool const multicol = cell_info[r][c].multicolumn != CELL_NORMAL;
	bool const fixed_width = (!column_info[c].p_width.zero() && !multicol)
	      || (multicol && !cell_info[r][c].p_width.zero());
	cell_info[r][c].inset->toggleFixedWidth(fixed_width);
	return fixed_width;
}


bool Tabular::setMColumnPWidth(Cursor & cur, idx_type cell,
		Length const & width)
{
	if (!isMultiColumn(cell))
		return false;

	cellInfo(cell).p_width = width;
	toggleFixedWidth(cur, cellInset(cell).get(), !width.zero());
	// cur paragraph can become invalid after paragraphs were merged
	if (cur.pit() > cur.lastpit())
		cur.pit() = cur.lastpit();
	// cur position can become invalid after newlines were removed
	if (cur.pos() > cur.lastpos())
		cur.pos() = cur.lastpos();
	return true;
}


bool Tabular::setMROffset(Cursor &, idx_type cell, Length const & mroffset)
{
	cellInfo(cell).mroffset = mroffset;
	return true;
}


void Tabular::setAlignSpecial(idx_type cell, docstring const & special,
				 Tabular::Feature what)
{
	if (what == SET_SPECIAL_MULTICOLUMN)
		cellInfo(cell).align_special = special;
	else
		column_info[cellColumn(cell)].align_special = special;
}


void Tabular::setTopLine(idx_type i, bool line)
{
	cellInfo(i).top_line = line;
}


void Tabular::setBottomLine(idx_type i, bool line)
{
	cellInfo(i).bottom_line = line;
}


void Tabular::setLeftLine(idx_type cell, bool line)
{
	cellInfo(cell).left_line = line;
}


void Tabular::setRightLine(idx_type cell, bool line)
{
	cellInfo(cell).right_line = line;
}

bool Tabular::rowTopLine(row_type r) const
{
	bool all_rows_set = true;
	for (col_type c = 0; all_rows_set && c < ncols(); ++c)
		all_rows_set = cellInfo(cellIndex(r, c)).top_line;
	return all_rows_set;
}


bool Tabular::rowBottomLine(row_type r) const
{
	bool all_rows_set = true;
	for (col_type c = 0; all_rows_set && c < ncols(); ++c)
		all_rows_set = cellInfo(cellIndex(r, c)).bottom_line;
	return all_rows_set;
}


bool Tabular::columnLeftLine(col_type c) const
{
	if (use_booktabs)
		return false;

	int nrows_left = 0;
	int total = 0;
	for (row_type r = 0; r < nrows(); ++r) {
		idx_type const i = cellIndex(r, c);
		if (c == cellColumn(i)) {
			++total;
			bool right = c > 0 && cellInfo(cellIndex(r, c - 1)).right_line;
			if (cellInfo(i).left_line || right)
				++nrows_left;
		}
	}
	return 2 * nrows_left >= total;
}


bool Tabular::columnRightLine(col_type c) const
{
	if (use_booktabs)
		return false;

	int nrows_right = 0;
	int total = 0;
	for (row_type r = 0; r < nrows(); ++r) {
		idx_type i = cellIndex(r, c);
		if (c == cellColumn(i) + columnSpan(i) - 1) {
			++total;
			bool left = (c + 1 < ncols() 
				&& cellInfo(cellIndex(r, c + 1)).left_line)
				|| c + 1 == ncols();
			if (cellInfo(i).right_line && left)
				++nrows_right;
		}
	}
	return 2 * nrows_right >= total;
}


LyXAlignment Tabular::getAlignment(idx_type cell, bool onlycolumn) const
{
	if (!onlycolumn && (isMultiColumn(cell) || isMultiRow(cell)))
		return cellInfo(cell).alignment;
	
	return column_info[cellColumn(cell)].alignment;
}


Tabular::VAlignment
Tabular::getVAlignment(idx_type cell, bool onlycolumn) const
{
	if (!onlycolumn && (isMultiColumn(cell) || isMultiRow(cell)))
		return cellInfo(cell).valignment;
	return column_info[cellColumn(cell)].valignment;
}


Length const Tabular::getPWidth(idx_type cell) const
{
	if (isMultiColumn(cell))
		return cellInfo(cell).p_width;
	return column_info[cellColumn(cell)].p_width;
}


Length const Tabular::getMROffset(idx_type cell) const
{
	return cellInfo(cell).mroffset;
}


int Tabular::textHOffset(idx_type cell) const
{
	// the LaTeX Way :-(
	int x = WIDTH_OF_LINE;

	int const w = cellWidth(cell) - cellInfo(cell).width;

	switch (getAlignment(cell)) {
	case LYX_ALIGN_CENTER:
		x += w / 2;
		break;
	case LYX_ALIGN_RIGHT:
		x += w;
		break;
	case LYX_ALIGN_DECIMAL: {
		// we center when no decimal point
		if (cellInfo(cell).decimal_width == 0) {
			x += w / 2;
			break;
		}
		col_type const c = cellColumn(cell);
		int max_dhoffset = 0;
		for(row_type r = 0; r < row_info.size() ; ++r) {
			idx_type const i = cellIndex(r, c);
			if (getAlignment(i) == LYX_ALIGN_DECIMAL
				&& cellInfo(i).decimal_width != 0)
				max_dhoffset = max(max_dhoffset, cellInfo(i).decimal_hoffset);
		}
		x += max_dhoffset - cellInfo(cell).decimal_hoffset;
	}
	default:
		// LYX_ALIGN_LEFT: nothing :-)
		break;
	}

	return x;
}


int Tabular::textVOffset(idx_type cell) const
{
	int voffset = cellInfo(cell).voffset;
	if (isMultiRow(cell)) {
		row_type const row = cellRow(cell);
		voffset += (cellHeight(cell) - rowAscent(row) - rowDescent(row))/2; 
	}
	return voffset;
}


Tabular::idx_type Tabular::getFirstCellInRow(row_type row) const
{
	col_type c = 0;
	idx_type const numcells = numberOfCellsInRow(row);
	// we check against numcells to make sure we do not crash if all the
	// cells are multirow (bug #7535), but in that case our return value
	// is really invalid, i.e., it is NOT the first cell in the row. but
	// i do not know what to do here. (rgh)
	while (c < numcells - 1
	       && cell_info[row][c].multirow == CELL_PART_OF_MULTIROW)
		++c;
	return cell_info[row][c].cellno;
}


Tabular::idx_type Tabular::getLastCellInRow(row_type row) const
{
	col_type c = ncols() - 1;
	// of course we check against 0 so we don't crash. but we have the same
	// problem as in the previous routine: if all the cells are part of a
	// multirow or part of a multi column, then our return value is invalid.
	while (c > 0
	       && (cell_info[row][c].multirow == CELL_PART_OF_MULTIROW
		   || cell_info[row][c].multicolumn == CELL_PART_OF_MULTICOLUMN))
		--c;
	return cell_info[row][c].cellno;
}


Tabular::row_type Tabular::cellRow(idx_type cell) const
{
	if (cell >= numberofcells)
		return nrows() - 1;
	if (cell == npos)
		return 0;
	return rowofcell[cell];
}


Tabular::col_type Tabular::cellColumn(idx_type cell) const
{
	if (cell >= numberofcells)
		return ncols() - 1;
	if (cell == npos)
		return 0;
	return columnofcell[cell];
}


void Tabular::write(ostream & os) const
{
	// header line
	os << "<lyxtabular"
	   << write_attribute("version", 3)
	   << write_attribute("rows", nrows())
	   << write_attribute("columns", ncols())
	   << ">\n";
	// global longtable options
	os << "<features"
	   << write_attribute("rotate", convert<string>(rotate))
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
	   << write_attribute("lastFootEmpty", endlastfoot.empty);
	// longtables cannot be aligned vertically
	if (!is_long_tabular) {
	   os << write_attribute("tabularvalignment", tabular_valignment);
	   os << write_attribute("tabularwidth", tabular_width);
	}
	if (is_long_tabular)
	   os << write_attribute("longtabularalignment",
	                         longtabular_alignment);
	os << ">\n";
	for (col_type c = 0; c < ncols(); ++c) {
		os << "<column"
		   << write_attribute("alignment", column_info[c].alignment);
		if (column_info[c].alignment == LYX_ALIGN_DECIMAL)
		   os << write_attribute("decimal_point", column_info[c].decimal_point);
		os << write_attribute("valignment", column_info[c].valignment)
		   << write_attribute("width", column_info[c].p_width.asString())
		   << write_attribute("special", column_info[c].align_special)
		   << ">\n";
	}
	for (row_type r = 0; r < nrows(); ++r) {
		static const string def("default");
		os << "<row";
		if (row_info[r].top_space_default)
			os << write_attribute("topspace", def);
		else
			os << write_attribute("topspace", row_info[r].top_space);
		if (row_info[r].bottom_space_default)
			os << write_attribute("bottomspace", def);
		else
			os << write_attribute("bottomspace", row_info[r].bottom_space);
		if (row_info[r].interline_space_default)
			os << write_attribute("interlinespace", def);
		else
			os << write_attribute("interlinespace", row_info[r].interline_space);
		os << write_attribute("endhead", row_info[r].endhead)
		   << write_attribute("endfirsthead", row_info[r].endfirsthead)
		   << write_attribute("endfoot", row_info[r].endfoot)
		   << write_attribute("endlastfoot", row_info[r].endlastfoot)
		   << write_attribute("newpage", row_info[r].newpage)
		   << write_attribute("caption", row_info[r].caption)
		   << ">\n";
		for (col_type c = 0; c < ncols(); ++c) {
			os << "<cell"
			   << write_attribute("multicolumn", cell_info[r][c].multicolumn)
			   << write_attribute("multirow", cell_info[r][c].multirow)
			   << write_attribute("mroffset", cell_info[r][c].mroffset)
			   << write_attribute("alignment", cell_info[r][c].alignment)
			   << write_attribute("valignment", cell_info[r][c].valignment)
			   << write_attribute("topline", cell_info[r][c].top_line)
			   << write_attribute("bottomline", cell_info[r][c].bottom_line)
			   << write_attribute("leftline", cell_info[r][c].left_line)
			   << write_attribute("rightline", cell_info[r][c].right_line)
			   << write_attribute("rotate", cell_info[r][c].rotate)
			   << write_attribute("usebox", cell_info[r][c].usebox)
			   << write_attribute("width", cell_info[r][c].p_width)
			   << write_attribute("special", cell_info[r][c].align_special)
			   << ">\n";
			os << "\\begin_inset ";
			cell_info[r][c].inset->write(os);
			os << "\n\\end_inset\n"
			   << "</cell>\n";
			// FIXME This can be removed again once the mystery
			// crash has been resolved.
			os << flush;
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
		LASSERT(false, return);
	}

	int version;
	if (!getTokenValue(line, "version", version))
		return;
	LATTEST(version >= 2);

	int rows_arg;
	if (!getTokenValue(line, "rows", rows_arg))
		return;
	int columns_arg;
	if (!getTokenValue(line, "columns", columns_arg))
		return;
	init(buffer_, rows_arg, columns_arg);
	l_getline(is, line);
	if (!prefixIs(line, "<features")) {
		lyxerr << "Wrong tabular format (expected <features ...> got"
		       << line << ')' << endl;
		return;
	}
	getTokenValue(line, "rotate", rotate);
	getTokenValue(line, "booktabs", use_booktabs);
	getTokenValue(line, "islongtable", is_long_tabular);
	getTokenValue(line, "tabularvalignment", tabular_valignment);
	getTokenValue(line, "tabularwidth", tabular_width);
	getTokenValue(line, "longtabularalignment", longtabular_alignment);
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

	for (col_type c = 0; c < ncols(); ++c) {
		l_getline(is,line);
		if (!prefixIs(line,"<column")) {
			lyxerr << "Wrong tabular format (expected <column ...> got"
			       << line << ')' << endl;
			return;
		}
		getTokenValue(line, "alignment", column_info[c].alignment);
		getTokenValue(line, "decimal_point", column_info[c].decimal_point);
		getTokenValue(line, "valignment", column_info[c].valignment);
		getTokenValue(line, "width", column_info[c].p_width);
		getTokenValue(line, "special", column_info[c].align_special);
	}

	for (row_type i = 0; i < nrows(); ++i) {
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
		getTokenValue(line, "caption", row_info[i].caption);
		for (col_type j = 0; j < ncols(); ++j) {
			l_getline(is, line);
			if (!prefixIs(line, "<cell")) {
				lyxerr << "Wrong tabular format (expected <cell ...> got"
				       << line << ')' << endl;
				return;
			}
			getTokenValue(line, "multicolumn", cell_info[i][j].multicolumn);
			getTokenValue(line, "multirow", cell_info[i][j].multirow);
			getTokenValue(line, "mroffset", cell_info[i][j].mroffset);
			getTokenValue(line, "alignment", cell_info[i][j].alignment);
			getTokenValue(line, "valignment", cell_info[i][j].valignment);
			getTokenValue(line, "topline", cell_info[i][j].top_line);
			getTokenValue(line, "bottomline", cell_info[i][j].bottom_line);
			getTokenValue(line, "leftline", cell_info[i][j].left_line);
			getTokenValue(line, "rightline", cell_info[i][j].right_line);
			getTokenValue(line, "rotate", cell_info[i][j].rotate);
			getTokenValue(line, "usebox", cell_info[i][j].usebox);
			getTokenValue(line, "width", cell_info[i][j].p_width);
			setFixedWidth(i,j);
			getTokenValue(line, "special", cell_info[i][j].align_special);
			l_getline(is, line);
			if (prefixIs(line, "\\begin_inset")) {
				cell_info[i][j].inset->setBuffer(*buffer_);
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
	return (cellInfo(cell).multicolumn == CELL_BEGIN_OF_MULTICOLUMN 
		|| cellInfo(cell).multicolumn == CELL_PART_OF_MULTICOLUMN);
}


bool Tabular::hasMultiColumn(col_type c) const
{
	for (row_type r = 0; r < nrows(); ++r) {
		if (isMultiColumn(cellIndex(r, c)))
			return true;
	}
	return false;
}


Tabular::CellData & Tabular::cellInfo(idx_type cell) const
{
	return cell_info[cellRow(cell)][cellColumn(cell)];
}


Tabular::idx_type Tabular::setMultiColumn(idx_type cell, idx_type number,
					  bool const right_border)
{
	idx_type const col = cellColumn(cell);
	idx_type const row = cellRow(cell);
	for (idx_type i = 0; i < number; ++i)
		unsetMultiRow(cellIndex(row, col + i));

	// unsetting of multirow may have invalidated cell index
	cell = cellIndex(row, col);
	CellData & cs = cellInfo(cell);
	cs.multicolumn = CELL_BEGIN_OF_MULTICOLUMN;
	if (column_info[col].alignment != LYX_ALIGN_DECIMAL)
		cs.alignment = column_info[col].alignment;
	setRightLine(cell, right_border);

	idx_type lastcell = cellIndex(row, col + number - 1);
	for (idx_type i = 1; i < lastcell - cell + 1; ++i) {
		CellData & cs1 = cellInfo(cell + i);
		cs1.multicolumn = CELL_PART_OF_MULTICOLUMN;
		cs.inset->appendParagraphs(cs1.inset->paragraphs());
		cs1.inset->clear();
	}
	updateIndexes();
	return cell;
}


bool Tabular::isMultiRow(idx_type cell) const
{
	return (cellInfo(cell).multirow == CELL_BEGIN_OF_MULTIROW
		|| cellInfo(cell).multirow == CELL_PART_OF_MULTIROW);
}

bool Tabular::hasMultiRow(row_type r) const
{
	for (col_type c = 0; c < ncols(); ++c) {
		if (isMultiRow(cellIndex(r, c)))
			return true;
	}
	return false;
}

Tabular::idx_type Tabular::setMultiRow(idx_type cell, idx_type number,
				       bool const bottom_border,
				       LyXAlignment const halign)
{
	idx_type const col = cellColumn(cell);
	idx_type const row = cellRow(cell);
	for (idx_type i = 0; i < number; ++i)
		unsetMultiColumn(cellIndex(row + i, col));

	// unsetting of multirow may have invalidated cell index
	cell = cellIndex(row, col);
	CellData & cs = cellInfo(cell);
	cs.multirow = CELL_BEGIN_OF_MULTIROW;
	cs.valignment = LYX_VALIGN_MIDDLE;
	// the horizontal alignment of multirow cells can only
	// be changed for the whole table row,
	// support changing this only for the multirow cell can be done via
	// \multirowsetup
	if (getPWidth(cell).zero())
		cs.alignment = halign;
	else
		cs.alignment = LYX_ALIGN_LEFT;

	// set the bottom line of the last selected cell
	setBottomLine(cell, bottom_border);

	for (idx_type i = 1; i < number; ++i) {
		CellData & cs1 = cell_info[row + i][col];
		cs1.multirow = CELL_PART_OF_MULTIROW;
		cs.inset->appendParagraphs(cs1.inset->paragraphs());
		cs1.inset->clear();
	}
	updateIndexes();
	return cell;
}


Tabular::idx_type Tabular::columnSpan(idx_type cell) const
{
	row_type const row = cellRow(cell);
	col_type const col = cellColumn(cell);
	int span = 1;
	while (col + span < ncols() && isPartOfMultiColumn(row, col + span))
		++span;

	return span;
}


Tabular::idx_type Tabular::rowSpan(idx_type cell) const
{
	col_type const column = cellColumn(cell);
	col_type row = cellRow(cell) + 1;
	while (row < nrows() && isPartOfMultiRow(row, column))
		++row;
	
	return row - cellRow(cell);
}


void Tabular::unsetMultiColumn(idx_type cell)
{
	if (!isMultiColumn(cell))
		return;

	row_type const row = cellRow(cell);
	col_type const col = cellColumn(cell);
	row_type const span = columnSpan(cell);
	for (col_type c = 0; c < span; ++c) {
		// in the table dialog the lines are set in every case
		// when unsetting a multicolumn this leads to an additional right
		// line for every cell that was part of the former multicolumn cell,
		// except if the cell is in the last column
		// therefore remove this line
		if (cell_info[row][col + c].multicolumn == CELL_BEGIN_OF_MULTICOLUMN
			&& (col + c) < (col + span - 1))
			cell_info[row][col + c].right_line = false;
		cell_info[row][col + c].multicolumn = CELL_NORMAL;
	}
	updateIndexes();
}


void Tabular::unsetMultiRow(idx_type cell)
{
	if (!isMultiRow(cell))
		return;

	cellInfo(cell).valignment = LYX_VALIGN_TOP;
	cellInfo(cell).alignment = LYX_ALIGN_CENTER;
	row_type const row = cellRow(cell);
	col_type const col = cellColumn(cell);
	row_type const span = rowSpan(cell);
	for (row_type r = 0; r < span; ++r)
		cell_info[row + r][col].multirow = CELL_NORMAL;
	updateIndexes();
}


void Tabular::setRotateCell(idx_type cell, int value)
{
	cellInfo(cell).rotate = value;
}


int Tabular::getRotateCell(idx_type cell) const
{
	return cellInfo(cell).rotate;
}


bool Tabular::needRotating() const
{
	if (rotate)
		return true;
	for (row_type r = 0; r < nrows(); ++r)
		for (col_type c = 0; c < ncols(); ++c)
			if (cell_info[r][c].rotate != 0)
				return true;
	return false;
}


bool Tabular::isLastCell(idx_type cell) const
{
	if (cell + 1 < numberofcells)
		return false;
	return true;
}


Tabular::idx_type Tabular::cellAbove(idx_type cell) const
{
	if (cellRow(cell) == 0)
		return cell;
	
	col_type const col = cellColumn(cell);
	row_type r = cellRow(cell) - 1;
	while (r > 0 && cell_info[r][col].multirow == CELL_PART_OF_MULTIROW)
		--r;

	return cell_info[r][col].cellno;
}


Tabular::idx_type Tabular::cellBelow(idx_type cell) const
{
	row_type const nextrow = cellRow(cell) + rowSpan(cell);
	if (nextrow < nrows())
		return cell_info[nextrow][cellColumn(cell)].cellno;
	return cell;
}


Tabular::idx_type Tabular::cellIndex(row_type row, col_type column) const
{
	LASSERT(column != npos && column < ncols(), column = 0);
	LASSERT(row != npos && row < nrows(), row = 0);
	return cell_info[row][column].cellno;
}


void Tabular::setUsebox(idx_type cell, BoxType type)
{
	cellInfo(cell).usebox = type;
}


// FIXME: Remove this routine because we cannot insert \parboxes when the user
// adds line breaks, see bug 4886.
Tabular::BoxType Tabular::getUsebox(idx_type cell) const
{
	if ((!column_info[cellColumn(cell)].p_width.zero() && !isMultiColumn(cell)) ||
		(isMultiColumn(cell) && !cellInfo(cell).p_width.zero()))
		return BOX_NONE;
	if (cellInfo(cell).usebox > 1)
		return cellInfo(cell).usebox;
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


bool Tabular::haveLTHead(bool withcaptions) const
{
	if (!is_long_tabular)
		return false;
	for (row_type i = 0; i < nrows(); ++i)
		if (row_info[i].endhead &&
		    (withcaptions || !row_info[i].caption))
			return true;
	return false;
}


bool Tabular::haveLTFirstHead(bool withcaptions) const
{
	if (!is_long_tabular || endfirsthead.empty)
		return false;
	for (row_type r = 0; r < nrows(); ++r)
		if (row_info[r].endfirsthead &&
		    (withcaptions || !row_info[r].caption))
			return true;
	return false;
}


bool Tabular::haveLTFoot(bool withcaptions) const
{
	if (!is_long_tabular)
		return false;
	for (row_type r = 0; r < nrows(); ++r)
		if (row_info[r].endfoot &&
		    (withcaptions || !row_info[r].caption))
			return true;
	return false;
}


bool Tabular::haveLTLastFoot(bool withcaptions) const
{
	if (!is_long_tabular || endlastfoot.empty)
		return false;
	for (row_type r = 0; r < nrows(); ++r)
		if (row_info[r].endlastfoot &&
		    (withcaptions || !row_info[r].caption))
			return true;
	return false;
}


Tabular::idx_type Tabular::setLTCaption(row_type row, bool what)
{
	idx_type i = getFirstCellInRow(row);
	if (what) {
		setMultiColumn(i, numberOfCellsInRow(row), false);
		setTopLine(i, false);
		setBottomLine(i, false);
		setLeftLine(i, false);
		setRightLine(i, false);
		if (!row_info[row].endfirsthead && !row_info[row].endhead &&
		    !row_info[row].endfoot && !row_info[row].endlastfoot) {
			setLTHead(row, true, endfirsthead, true);
			row_info[row].endfirsthead = true;
		}
	} else {
		unsetMultiColumn(i);
		// When unsetting a caption row, also all existing
		// captions in this row must be dissolved.
	}
	row_info[row].caption = what;
	return i;
}


bool Tabular::ltCaption(row_type row) const
{
	return row_info[row].caption;
}


bool Tabular::haveLTCaption(CaptionType captiontype) const
{
	if (!is_long_tabular)
		return false;
	for (row_type r = 0; r < nrows(); ++r) {
		if (row_info[r].caption) {
			switch (captiontype) {
			case CAPTION_FIRSTHEAD:
				if (row_info[r].endfirsthead)
					return true;
				break;
			case CAPTION_HEAD:
				if (row_info[r].endhead)
					return true;
				break;
			case CAPTION_FOOT:
				if (row_info[r].endfoot)
					return true;
				break;
			case CAPTION_LASTFOOT:
				if (row_info[r].endlastfoot)
					return true;
				break;
			case CAPTION_ANY:
				return true;
			}
		}
	}
	return false;
}


// end longtable support functions

void Tabular::setRowAscent(row_type row, int height)
{
	if (row >= nrows() || row_info[row].ascent == height)
		return;
	row_info[row].ascent = height;
}


void Tabular::setRowDescent(row_type row, int height)
{
	if (row >= nrows() || row_info[row].descent == height)
		return;
	row_info[row].descent = height;
}


int Tabular::rowAscent(row_type row) const
{
	LASSERT(row < nrows(), row = 0);
	return row_info[row].ascent;
}


int Tabular::rowDescent(row_type row) const
{
	LASSERT(row < nrows(), row = 0);
	return row_info[row].descent;
}


int Tabular::height() const
{
	int height = 0;
	for (row_type row = 0; row < nrows(); ++row)
		height += rowAscent(row) + rowDescent(row) +
			interRowSpace(row);
	return height;
}


bool Tabular::isPartOfMultiColumn(row_type row, col_type column) const
{
	LASSERT(row < nrows(), return false);
	LASSERT(column < ncols(), return false);
	return cell_info[row][column].multicolumn == CELL_PART_OF_MULTICOLUMN;
}


bool Tabular::isPartOfMultiRow(row_type row, col_type column) const
{
	LASSERT(row < nrows(), return false);
	LASSERT(column < ncols(), return false);
	return cell_info[row][column].multirow == CELL_PART_OF_MULTIROW;
}


void Tabular::TeXTopHLine(otexstream & os, row_type row, string const lang) const
{
	// we only output complete row lines and the 1st row here, the rest
	// is done in Tabular::TeXBottomHLine(...)

	// get for each column the topline (if any)
	vector<bool> topline;
	col_type nset = 0;
	for (col_type c = 0; c < ncols(); ++c) {
		topline.push_back(topLine(cellIndex(row, c)));
		// If cell is part of a multirow and not the first cell of the
		// multirow, no line must be drawn.
		if (row != 0)
			if (isMultiRow(cellIndex(row, c))
			    && cell_info[row][c].multirow != CELL_BEGIN_OF_MULTIROW)
				topline[c] = false;
		if (topline[c])
			++nset;
	}

	// do nothing if empty first row, or incomplete row line after
	if ((row == 0 && nset == 0) || (row > 0 && nset != ncols()))
		return;

	// only output complete row lines and the 1st row's clines
	if (nset == ncols()) {
		if (use_booktabs) {
			os << (row == 0 ? "\\toprule " : "\\midrule ");
		} else {
			os << "\\hline ";
		}
	} else if (row == 0) {
		for (col_type c = 0; c < ncols(); ++c) {
			if (topline[c]) {
				col_type offset = 0;
				for (col_type j = 0 ; j < c; ++j)
					if (column_info[j].alignment == LYX_ALIGN_DECIMAL)
						++offset;
				
				//babel makes the "-" character an active one, so we have to suppress this here
				//see http://groups.google.com/group/comp.text.tex/browse_thread/thread/af769424a4a0f289#
				if (lang == "slovak" || lang == "czech")
					os << "\\expandafter" << (use_booktabs ? "\\cmidrule" : "\\cline") 
					<< "\\expandafter{\\expandafter" << c + 1 + offset << "\\string-";
				else
					os << (use_booktabs ? "\\cmidrule{" : "\\cline{") << c + 1 + offset << '-';
				
				col_type cstart = c;
				for ( ; c < ncols() && topline[c]; ++c) {}
				
				for (col_type j = cstart ; j < c ; ++j)
					if (column_info[j].alignment == LYX_ALIGN_DECIMAL)
						++offset;
				
				os << c + offset << "} ";
			}
		}
	}
	os << "\n";
}


void Tabular::TeXBottomHLine(otexstream & os, row_type row, string const lang) const
{
	// we output bottomlines of row r and the toplines of row r+1
	// if the latter do not span the whole tabular

	// get the bottomlines of row r, and toplines in next row
	bool lastrow = row == nrows() - 1;
	vector<bool> bottomline, topline;
	bool nextrowset = true;
	for (col_type c = 0; c < ncols(); ++c) {
		bottomline.push_back(bottomLine(cellIndex(row, c)));
		topline.push_back(!lastrow && topLine(cellIndex(row + 1, c)));
		// If cell is part of a multirow and not the last cell of the
		// multirow, no line must be drawn.
		if (!lastrow)
			if (isMultiRow(cellIndex(row, c))
			    && isMultiRow(cellIndex(row + 1, c))
			    && cell_info[row + 1][c].multirow != CELL_BEGIN_OF_MULTIROW) {
				bottomline[c] = false;
				topline[c] = false;
				}
		nextrowset &= topline[c];
	}

	// combine this row's bottom lines and next row's toplines if necessary
	col_type nset = 0;
	for (col_type c = 0; c < ncols(); ++c) {
		if (!nextrowset)
			bottomline[c] = bottomline[c] || topline[c];
		if (bottomline[c])
			++nset;
	}

	// do nothing if empty, OR incomplete row line with a topline in next row
	if (nset == 0 || (nextrowset && nset != ncols()))
		return;

	if (nset == ncols()) {
		if (use_booktabs)
			os << (lastrow ? "\\bottomrule" : "\\midrule");
		else
			os << "\\hline ";
	} else {
		for (col_type c = 0; c < ncols(); ++c) {
			if (bottomline[c]) {
				col_type offset = 0;
				for (col_type j = 0 ; j < c; ++j)
					if (column_info[j].alignment == LYX_ALIGN_DECIMAL)
						++offset;
				
				//babel makes the "-" character an active one, so we have to suppress this here
				//see http://groups.google.com/group/comp.text.tex/browse_thread/thread/af769424a4a0f289#
				if (lang == "slovak" || lang == "czech")
					os << "\\expandafter" << (use_booktabs ? "\\cmidrule" : "\\cline")
					<< "\\expandafter{\\expandafter" << c + 1 + offset << "\\string-";
				else
					os << (use_booktabs ? "\\cmidrule{" : "\\cline{") << c + 1 + offset << '-';
				
				col_type cstart = c;
				for ( ; c < ncols() && bottomline[c]; ++c) {}
				
				for (col_type j = cstart ; j < c ; ++j)
					if (column_info[j].alignment == LYX_ALIGN_DECIMAL)
						++offset;
				
				os << c + offset << "} ";
			}
		}
	}
	os << "\n";
}


void Tabular::TeXCellPreamble(otexstream & os, idx_type cell,
			      bool & ismulticol, bool & ismultirow) const
{
	row_type const r = cellRow(cell);
	if (is_long_tabular && row_info[r].caption)
		return;

	Tabular::VAlignment valign =  getVAlignment(cell, !isMultiColumn(cell));
	LyXAlignment align = getAlignment(cell, !isMultiColumn(cell));
	// figure out how to set the lines
	// we always set double lines to the right of the cell
	col_type const c = cellColumn(cell);
	col_type const nextcol = c + columnSpan(cell);
	bool colright = columnRightLine(c);
	bool colleft = columnLeftLine(c);
	bool nextcolleft = nextcol < ncols() && columnLeftLine(nextcol);
	bool nextcellleft = nextcol < ncols() 
		&& leftLine(cellIndex(r, nextcol));
	bool coldouble = colright && nextcolleft;
	bool celldouble = rightLine(cell) && nextcellleft;

	ismulticol = isMultiColumn(cell)
		|| (c == 0 && colleft != leftLine(cell))
		|| ((colright || nextcolleft) && !rightLine(cell) && !nextcellleft)
		|| (!colright && !nextcolleft && (rightLine(cell) || nextcellleft))
		|| (coldouble != celldouble);

	// we center in multicol when no decimal point
	if (column_info[c].alignment == LYX_ALIGN_DECIMAL) {
		docstring const align_d = column_info[c].decimal_point;
		DocIterator const dit = separatorPos(cellInset(cell).get(), align_d);
		ismulticol |= !dit;
	}

	// up counter by 1 for each decimally aligned col since they use 2 latex cols
	int latexcolspan = columnSpan(cell);
	for(col_type col = c; col < c + columnSpan(cell); ++col)
		if (column_info[col].alignment == LYX_ALIGN_DECIMAL)
			++latexcolspan;

	if (ismulticol) {
		os << "\\multicolumn{" << latexcolspan << "}{";
		if (c ==0 && leftLine(cell))
			os << '|';
		if (!cellInfo(cell).align_special.empty()) {
			os << cellInfo(cell).align_special;
		} else {
			if (!getPWidth(cell).zero()) {
				switch (align) {
				case LYX_ALIGN_LEFT:
					os << ">{\\raggedright}";
					break;
				case LYX_ALIGN_RIGHT:
					os << ">{\\raggedleft}";
					break;
				case LYX_ALIGN_CENTER:
					os << ">{\\centering}";
					break;
				default:
					break;
				}
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
		if (rightLine(cell) || nextcellleft)
			os << '|';
		if (celldouble)
			// add extra vertical line if we want a double one
			os << '|';
		os << "}{";
	} // end if ismulticol

	// we only need code for the first multirow cell
	ismultirow = isMultiRow(cell);
	if (ismultirow) {
		os << "\\multirow{" << rowSpan(cell) << "}{";
		if (!getPWidth(cell).zero())
			os << from_ascii(getPWidth(cell).asLatexString());
		else
			// we need to set a default value
			os << "*";
		os << "}";
		if (!getMROffset(cell).zero())
			os << "[" << from_ascii(getMROffset(cell).asLatexString()) << "]";
		os << "{";
	} // end if ismultirow

	if (getRotateCell(cell) != 0)
		os << "\\begin{turn}{" << convert<string>(getRotateCell(cell)) << "}\n";

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
	}
}


void Tabular::TeXCellPostamble(otexstream & os, idx_type cell,
			       bool ismulticol, bool ismultirow) const
{
	row_type const r = cellRow(cell);
	if (is_long_tabular && row_info[r].caption)
		return;

	// usual cells
	if (getUsebox(cell) == BOX_PARBOX)
		os << '}';
	else if (getUsebox(cell) == BOX_MINIPAGE)
		os << breakln << "\\end{minipage}";
	if (getRotateCell(cell) != 0)
		os << breakln << "\\end{turn}";
	if (ismultirow)
		os << '}';
	if (ismulticol)
		os << '}';
}


void Tabular::TeXLongtableHeaderFooter(otexstream & os,
					OutputParams const & runparams) const
{
	if (!is_long_tabular)
		return;

	// caption handling
	// output caption which is in no header or footer
	if (haveLTCaption()) {
		for (row_type r = 0; r < nrows(); ++r) {
			if (row_info[r].caption &&
			    !row_info[r].endfirsthead && !row_info[r].endhead &&
			    !row_info[r].endfoot && !row_info[r].endlastfoot)
				TeXRow(os, r, runparams);
		}
	}
	// output first header info
	if (haveLTFirstHead()) {
		if (endfirsthead.topDL)
			os << "\\hline\n";
		for (row_type r = 0; r < nrows(); ++r) {
			if (row_info[r].endfirsthead)
				TeXRow(os, r, runparams);
		}
		if (endfirsthead.bottomDL)
			os << "\\hline\n";
		os << "\\endfirsthead\n";
	}
	// output header info
	if (haveLTHead()) {
		if (endfirsthead.empty && !haveLTFirstHead())
			os << "\\endfirsthead\n";
		if (endhead.topDL)
			os << "\\hline\n";
		for (row_type r = 0; r < nrows(); ++r) {
			if (row_info[r].endhead)
				TeXRow(os, r, runparams);
		}
		if (endhead.bottomDL)
			os << "\\hline\n";
		os << "\\endhead\n";
	}
	// output footer info
	if (haveLTFoot()) {
		if (endfoot.topDL)
			os << "\\hline\n";
		for (row_type r = 0; r < nrows(); ++r) {
			if (row_info[r].endfoot)
				TeXRow(os, r, runparams);
		}
		if (endfoot.bottomDL)
			os << "\\hline\n";
		os << "\\endfoot\n";
		if (endlastfoot.empty && !haveLTLastFoot())
			os << "\\endlastfoot\n";
	}
	// output lastfooter info
	if (haveLTLastFoot()) {
		if (endlastfoot.topDL)
			os << "\\hline\n";
		for (row_type r = 0; r < nrows(); ++r) {
			if (row_info[r].endlastfoot)
				TeXRow(os, r, runparams);
		}
		if (endlastfoot.bottomDL)
			os << "\\hline\n";
		os << "\\endlastfoot\n";
	}
}


bool Tabular::isValidRow(row_type row) const
{
	if (!is_long_tabular)
		return true;
	return !row_info[row].endhead && !row_info[row].endfirsthead
		&& !row_info[row].endfoot && !row_info[row].endlastfoot
		&& !row_info[row].caption;
}


void Tabular::TeXRow(otexstream & os, row_type row,
		     OutputParams const & runparams) const
{
	idx_type cell = cellIndex(row, 0);
	shared_ptr<InsetTableCell> inset = cellInset(cell);
	Paragraph const & par = inset->paragraphs().front();
	string const lang = par.getParLanguage(buffer().params())->lang();

	//output the top line
	TeXTopHLine(os, row, lang);

	if (row_info[row].top_space_default) {
		if (use_booktabs)
			os << "\\addlinespace\n";
		else
			os << "\\noalign{\\vskip\\doublerulesep}\n";
	} else if(!row_info[row].top_space.zero()) {
		if (use_booktabs)
			os << "\\addlinespace["
			   << from_ascii(row_info[row].top_space.asLatexString())
			   << "]\n";
		else {
			os << "\\noalign{\\vskip"
			   << from_ascii(row_info[row].top_space.asLatexString())
			   << "}\n";
		}
	}
	bool ismulticol = false;
	bool ismultirow = false;
	for (col_type c = 0; c < ncols(); ++c) {
		if (isPartOfMultiColumn(row, c))
			continue;

		cell = cellIndex(row, c);

		if (isPartOfMultiRow(row, c)
		    && column_info[c].alignment != LYX_ALIGN_DECIMAL) {
			if (cell != getLastCellInRow(row))
				os << " & "; 
			continue;
		}

		TeXCellPreamble(os, cell, ismulticol, ismultirow);
		shared_ptr<InsetTableCell> inset = cellInset(cell);

		Paragraph const & par = inset->paragraphs().front();
		bool rtl = par.isRTL(buffer().params())
			&& !par.empty()
			&& getPWidth(cell).zero()
			&& !runparams.use_polyglossia;

		if (rtl) {
			string const lang =
				par.getParLanguage(buffer().params())->lang();
			if (lang == "farsi")
				os << "\\textFR{";
			else if (lang == "arabic_arabi")
				os << "\\textAR{";
			// currently, remaining RTL languages are
			// arabic_arabtex and hebrew
			else
				os << "\\R{";
		}
		// pass to the OutputParams that we are in a cell and
		// which alignment we have set.
		// InsetNewline needs this context information.
		OutputParams newrp(runparams);
		newrp.inTableCell = (getAlignment(cell) == LYX_ALIGN_BLOCK)
				    ? OutputParams::PLAIN
				    : OutputParams::ALIGNED;

		if (getAlignment(cell) == LYX_ALIGN_DECIMAL) {
			// copy cell and split in 2
			InsetTableCell head = InsetTableCell(*cellInset(cell).get());
			head.setBuffer(buffer());
			DocIterator dit = cellInset(cell)->getText(0)->macrocontextPosition();
			dit.pop_back();
			dit.push_back(CursorSlice(head));
			head.setMacrocontextPositionRecursive(dit);
			bool hassep = false;
			InsetTableCell tail = splitCell(head, column_info[c].decimal_point, hassep);
			head.latex(os, newrp);
			if (hassep) {
				os << '&';
				tail.setBuffer(head.buffer());
				dit.pop_back();
				dit.push_back(CursorSlice(tail));
				tail.setMacrocontextPositionRecursive(dit);
				tail.latex(os, newrp);
			}
		} else if (!isPartOfMultiRow(row, c)) {
			if (!runparams.nice)
				os.texrow().start(par.id(), 0);
			inset->latex(os, newrp);
		}

		runparams.encoding = newrp.encoding;
		if (rtl)
			os << '}';

		TeXCellPostamble(os, cell, ismulticol, ismultirow);
		if (cell != getLastCellInRow(row)) { // not last cell in row
			if (runparams.nice)
				os << " & ";
			else
				os << " &\n";
		}
	}
	os << "\\tabularnewline";
	if (row_info[row].bottom_space_default) {
		if (use_booktabs)
			os << "\\addlinespace";
		else
			os << "[\\doublerulesep]";
	} else if (!row_info[row].bottom_space.zero()) {
		if (use_booktabs)
			os << "\\addlinespace";
		os << '['
		   << from_ascii(row_info[row].bottom_space.asLatexString())
		   << ']';
	}
	os << '\n';

	//output the bottom line
	TeXBottomHLine(os, row, lang);

	if (row_info[row].interline_space_default) {
		if (use_booktabs)
			os << "\\addlinespace\n";
		else
			os << "\\noalign{\\vskip\\doublerulesep}\n";
	} else if (!row_info[row].interline_space.zero()) {
		if (use_booktabs)
			os << "\\addlinespace["
			   << from_ascii(row_info[row].interline_space.asLatexString())
			   << "]\n";
		else
			os << "\\noalign{\\vskip"
			   << from_ascii(row_info[row].interline_space.asLatexString())
			   << "}\n";
	}
}


void Tabular::latex(otexstream & os, OutputParams const & runparams) const
{
	bool const is_tabular_star = !tabular_width.zero();

	//+---------------------------------------------------------------------
	//+                      first the opening preamble                    +
	//+---------------------------------------------------------------------

	os << safebreakln;
	if (runparams.lastid != -1)
		os.texrow().start(runparams.lastid, runparams.lastpos);

	if (rotate != 0)
		os << "\\begin{turn}{" << convert<string>(rotate) << "}\n";

	if (is_long_tabular) {
		os << "\\begin{longtable}";
		switch (longtabular_alignment) {
		case LYX_LONGTABULAR_ALIGN_LEFT:
			os << "[l]";
			break;
		case LYX_LONGTABULAR_ALIGN_CENTER:
			break;
		case LYX_LONGTABULAR_ALIGN_RIGHT:
			os << "[r]";
			break;
		}
	} else {
		if (is_tabular_star)
			os << "\\begin{tabular*}{" << from_ascii(tabular_width.asLatexString()) << "}";
		else
			os << "\\begin{tabular}";
		switch (tabular_valignment) {
		case LYX_VALIGN_TOP:
			os << "[t]";
			break;
		case LYX_VALIGN_MIDDLE:
			break;
		case LYX_VALIGN_BOTTOM:
			os << "[b]";
			break;
		}
	}
	
	os << "{";

	if (is_tabular_star)
		os << "@{\\extracolsep{\\fill}}";

	for (col_type c = 0; c < ncols(); ++c) {
		if (columnLeftLine(c))
			os << '|';
		if (!column_info[c].align_special.empty()) {
			os << column_info[c].align_special;
		} else {
			if (!column_info[c].p_width.zero()) {
				switch (column_info[c].alignment) {
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
				case LYX_ALIGN_DECIMAL:
					break;
				}

				switch (column_info[c].valignment) {
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
				   << from_ascii(column_info[c].p_width.asLatexString())
				   << '}';
			} else {
				switch (column_info[c].alignment) {
				case LYX_ALIGN_LEFT:
					os << 'l';
					break;
				case LYX_ALIGN_RIGHT:
					os << 'r';
					break;
				case LYX_ALIGN_DECIMAL:
					os << "r@{\\extracolsep{0pt}" << column_info[c].decimal_point << "}l";
					break;
				default:
					os << 'c';
					break;
				}
			} // end if else !column_info[i].p_width
		} // end if else !column_info[i].align_special
		if (columnRightLine(c))
			os << '|';
	}
	os << "}\n";

	TeXLongtableHeaderFooter(os, runparams);

	//+---------------------------------------------------------------------
	//+                      the single row and columns (cells)            +
	//+---------------------------------------------------------------------

	for (row_type r = 0; r < nrows(); ++r) {
		if (isValidRow(r)) {
			TeXRow(os, r, runparams);
			if (is_long_tabular && row_info[r].newpage)
				os << "\\newpage\n";
		}
	}

	//+---------------------------------------------------------------------
	//+                      the closing of the tabular                    +
	//+---------------------------------------------------------------------

	if (is_long_tabular)
		os << "\\end{longtable}";
	else {
		if (is_tabular_star)
			os << "\\end{tabular*}";
		else
			os << "\\end{tabular}";
	}

	if (rotate != 0)
		os << breakln << "\\end{turn}";
}


int Tabular::docbookRow(odocstream & os, row_type row,
			   OutputParams const & runparams) const
{
	int ret = 0;
	idx_type cell = getFirstCellInRow(row);

	os << "<row>\n";
	for (col_type c = 0; c < ncols(); ++c) {
		if (isPartOfMultiColumn(row, c))
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
			os << " namest=\"col" << c << "\" ";
			os << "nameend=\"col" << c + columnSpan(cell) - 1 << '"';
		}

		os << '>';
		ret += cellInset(cell)->docbook(os, runparams);
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

	os << "<tgroup cols=\"" << ncols()
	   << "\" colsep=\"1\" rowsep=\"1\">\n";

	for (col_type c = 0; c < ncols(); ++c) {
		os << "<colspec colname=\"col" << c << "\" align=\"";
		switch (column_info[c].alignment) {
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

	// output caption info
	// The caption flag wins over head/foot
	if (haveLTCaption()) {
		os << "<caption>\n";
		++ret;
		for (row_type r = 0; r < nrows(); ++r) {
			if (row_info[r].caption) {
				ret += docbookRow(os, r, runparams);
			}
		}
		os << "</caption>\n";
		++ret;
	}
	// output header info
	if (haveLTHead(false) || haveLTFirstHead(false)) {
		os << "<thead>\n";
		++ret;
		for (row_type r = 0; r < nrows(); ++r) {
			if ((row_info[r].endhead || row_info[r].endfirsthead) &&
			    !row_info[r].caption) {
				ret += docbookRow(os, r, runparams);
			}
		}
		os << "</thead>\n";
		++ret;
	}
	// output footer info
	if (haveLTFoot(false) || haveLTLastFoot(false)) {
		os << "<tfoot>\n";
		++ret;
		for (row_type r = 0; r < nrows(); ++r) {
			if ((row_info[r].endfoot || row_info[r].endlastfoot) &&
			    !row_info[r].caption) {
				ret += docbookRow(os, r, runparams);
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
	for (row_type r = 0; r < nrows(); ++r) {
		if (isValidRow(r)) {
			ret += docbookRow(os, r, runparams);
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


docstring Tabular::xhtmlRow(XHTMLStream & xs, row_type row,
			   OutputParams const & runparams, bool header) const
{
	docstring ret;
	string const celltag = header ? "th" : "td";
	idx_type cell = getFirstCellInRow(row);

	xs << html::StartTag("tr");
	for (col_type c = 0; c < ncols(); ++c) {
		if (isPartOfMultiColumn(row, c) || isPartOfMultiRow(row, c))
			continue;

		stringstream attr;
		attr << "align='";
		switch (getAlignment(cell)) {
		case LYX_ALIGN_LEFT:
			attr << "left";
			break;
		case LYX_ALIGN_RIGHT:
			attr << "right";
			break;
		default:
			attr << "center";
			break;
		}
		attr << "'";
		attr << " valign='";
		switch (getVAlignment(cell)) {
		case LYX_VALIGN_TOP:
			attr << "top";
			break;
		case LYX_VALIGN_BOTTOM:
			attr << "bottom";
			break;
		case LYX_VALIGN_MIDDLE:
			attr << "middle";
		}
		attr << "'";

		if (isMultiColumn(cell))
			attr << " colspan='" << columnSpan(cell) << "'";
		else if (isMultiRow(cell))
			attr << " rowspan='" << rowSpan(cell) << "'";

		xs << html::StartTag(celltag, attr.str()) << html::CR();
		ret += cellInset(cell)->xhtml(xs, runparams);
		xs << html::EndTag(celltag) << html::CR();
		++cell;
	}
	xs << html::EndTag("tr");
	return ret;
}


docstring Tabular::xhtml(XHTMLStream & xs, OutputParams const & runparams) const
{
	docstring ret;

	if (is_long_tabular) {
		// we'll wrap it in a div, so as to deal with alignment
		string align;
		switch (longtabular_alignment) {
		case LYX_LONGTABULAR_ALIGN_LEFT:
			align = "left";
			break;
		case LYX_LONGTABULAR_ALIGN_CENTER:
			align = "center";
			break;
		case LYX_LONGTABULAR_ALIGN_RIGHT:
			align = "right";
			break;
		}
		xs << html::StartTag("div", "class='longtable' style='text-align: " + align + ";'")
		   << html::CR();
		// The caption flag wins over head/foot
		if (haveLTCaption()) {
			xs << html::StartTag("div", "class='longtable-caption' style='text-align: " + align + ";'")
			   << html::CR();
			for (row_type r = 0; r < nrows(); ++r)
				if (row_info[r].caption)
					ret += xhtmlRow(xs, r, runparams);
			xs << html::EndTag("div") << html::CR();
		}
	}

	xs << html::StartTag("table") << html::CR();

	// output header info
	bool const havefirsthead = haveLTFirstHead(false);
	// if we have a first head, then we are going to ignore the
	// headers for the additional pages, since there aren't any
	// in XHTML. this test accomplishes that.
	bool const havehead = !havefirsthead && haveLTHead(false);
	if (havehead || havefirsthead) {
		xs << html::StartTag("thead") << html::CR();
		for (row_type r = 0; r < nrows(); ++r) {
			if (((havefirsthead && row_info[r].endfirsthead) ||
			     (havehead && row_info[r].endhead)) &&
			    !row_info[r].caption) {
				ret += xhtmlRow(xs, r, runparams, true);
			}
		}
		xs << html::EndTag("thead") << html::CR();
	}
	// output footer info
	bool const havelastfoot = haveLTLastFoot(false);
	// as before.
	bool const havefoot = !havelastfoot && haveLTFoot(false);
	if (havefoot || havelastfoot) {
		xs << html::StartTag("tfoot") << html::CR();
		for (row_type r = 0; r < nrows(); ++r) {
			if (((havelastfoot && row_info[r].endlastfoot) ||
			     (havefoot && row_info[r].endfoot)) &&
			    !row_info[r].caption) {
				ret += xhtmlRow(xs, r, runparams);
			}
		}
		xs << html::EndTag("tfoot") << html::CR();
	}

	xs << html::StartTag("tbody") << html::CR();
	for (row_type r = 0; r < nrows(); ++r) {
		if (isValidRow(r)) {
			ret += xhtmlRow(xs, r, runparams);
		}
	}
	xs << html::EndTag("tbody")
	   << html::CR()
	   << html::EndTag("table")
	   << html::CR();
	if (is_long_tabular)
		xs << html::EndTag("div") << html::CR();
	return ret;
}


bool Tabular::plaintextTopHLine(odocstringstream & os, row_type row,
				   vector<unsigned int> const & clen) const
{
	idx_type const fcell = getFirstCellInRow(row);
	idx_type const n = numberOfCellsInRow(row) + fcell;
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
		while (column < ncols() - 1
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


bool Tabular::plaintextBottomHLine(odocstringstream & os, row_type row,
				      vector<unsigned int> const & clen) const
{
	idx_type const fcell = getFirstCellInRow(row);
	idx_type const n = numberOfCellsInRow(row) + fcell;
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
		while (column < ncols() - 1 && isPartOfMultiColumn(row, ++column))
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


void Tabular::plaintextPrintCell(odocstringstream & os,
			       OutputParams const & runparams,
			       idx_type cell, row_type row, col_type column,
			       vector<unsigned int> const & clen,
			       bool onlydata, size_t max_length) const
{
	odocstringstream sstr;
	cellInset(cell)->plaintext(sstr, runparams, max_length);

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
	while (column < ncols() - 1 && isPartOfMultiColumn(row, ++column))
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


void Tabular::plaintext(odocstringstream & os,
			   OutputParams const & runparams, int const depth,
			   bool onlydata, char_type delim, size_t max_length) const
{
	// first calculate the width of the single columns
	vector<unsigned int> clen(ncols());

	if (!onlydata) {
		// first all non multicolumn cells!
		for (col_type c = 0; c < ncols(); ++c) {
			clen[c] = 0;
			for (row_type r = 0; r < nrows(); ++r) {
				idx_type cell = cellIndex(r, c);
				if (isMultiColumn(cell))
					continue;
				odocstringstream sstr;
				cellInset(cell)->plaintext(sstr, runparams, max_length);
				if (clen[c] < sstr.str().length())
					clen[c] = sstr.str().length();
			}
		}
		// then all multicolumn cells!
		for (col_type c = 0; c < ncols(); ++c) {
			for (row_type r = 0; r < nrows(); ++r) {
				idx_type cell = cellIndex(r, c);
				if (cell_info[r][c].multicolumn != CELL_BEGIN_OF_MULTICOLUMN)
					continue;
				odocstringstream sstr;
				cellInset(cell)->plaintext(sstr, runparams, max_length);
				int len = int(sstr.str().length());
				idx_type const n = columnSpan(cell);
				for (col_type k = c; len > 0 && k < c + n - 1; ++k)
					len -= clen[k];
				if (len > int(clen[c + n - 1]))
					clen[c + n - 1] = len;
			}
		}
	}
	idx_type cell = 0;
	for (row_type r = 0; r < nrows(); ++r) {
		if (!onlydata && plaintextTopHLine(os, r, clen))
			os << docstring(depth * 2, ' ');
		for (col_type c = 0; c < ncols(); ++c) {
			if (isPartOfMultiColumn(r, c) || isPartOfMultiRow(r,c))
				continue;
			if (onlydata && c > 0)
				// we don't use operator<< for single UCS4 character.
				// see explanation in docstream.h
				os.put(delim);
			plaintextPrintCell(os, runparams, cell, r, c, clen, onlydata, max_length);
			++cell;
			if (os.str().size() > max_length)
				break;
		}
		os << endl;
		if (!onlydata) {
			os << docstring(depth * 2, ' ');
			if (plaintextBottomHLine(os, r, clen))
				os << docstring(depth * 2, ' ');
		}
		if (os.str().size() > max_length)
			break;
	}
}


shared_ptr<InsetTableCell> Tabular::cellInset(idx_type cell) const
{
	return cell_info[cellRow(cell)][cellColumn(cell)].inset;
}


shared_ptr<InsetTableCell> Tabular::cellInset(row_type row,
					       col_type column) const
{
	return cell_info[row][column].inset;
}


void Tabular::setCellInset(row_type row, col_type column,
			      shared_ptr<InsetTableCell> ins) const
{
	CellData & cd = cell_info[row][column];
	cd.inset = ins;
}


void Tabular::validate(LaTeXFeatures & features) const
{
	features.require("NeedTabularnewline");
	if (use_booktabs)
		features.require("booktabs");
	if (is_long_tabular)
		features.require("longtable");
	if (needRotating())
		features.require("rotating");
	for (idx_type cell = 0; cell < numberofcells; ++cell) {
		if (isMultiRow(cell))
			features.require("multirow");
		if (getVAlignment(cell) != LYX_VALIGN_TOP
		    || !getPWidth(cell).zero())
			features.require("array");
		cellInset(cell)->validate(features);
	}
}


Tabular::BoxType Tabular::useParbox(idx_type cell) const
{
	ParagraphList const & parlist = cellInset(cell)->paragraphs();
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

InsetTableCell::InsetTableCell(Buffer * buf)
	: InsetText(buf, InsetText::PlainLayout), isFixedWidth(false),
	  contentAlign(LYX_ALIGN_CENTER)
{}


bool InsetTableCell::forcePlainLayout(idx_type) const
{
	return !isFixedWidth;
}


bool InsetTableCell::allowParagraphCustomization(idx_type) const
{
	return isFixedWidth;
}


bool InsetTableCell::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	bool enabled = true;
	switch (cmd.action()) {
	case LFUN_LAYOUT:
		enabled = !forcePlainLayout();
		break;
	case LFUN_LAYOUT_PARAGRAPH:
		enabled = allowParagraphCustomization();
		break;

	case LFUN_MATH_DISPLAY:
		if (!hasFixedWidth()) {
			enabled = false;
			break;
		} //fall-through
	default:
		return InsetText::getStatus(cur, cmd, status);
	}
	status.setEnabled(enabled);
	return true;
}

docstring InsetTableCell::asString(bool intoInsets) 
{
	docstring retval;
	if (paragraphs().empty())
		return retval;
	ParagraphList::const_iterator it = paragraphs().begin();
	ParagraphList::const_iterator en = paragraphs().end();
	bool first = true;
	for (; it != en; ++it) {
		if (!first)
			retval += "\n";
		else
			first = false;
		retval += it->asString(intoInsets ? AS_STR_INSETS : AS_STR_NONE);
	}
	return retval;
}


void InsetTableCell::addToToc(DocIterator const & di, bool output_active) const
{
	InsetText::iterateForToc(di, output_active);
}


docstring InsetTableCell::xhtml(XHTMLStream & xs, OutputParams const & rp) const
{
	if (!isFixedWidth)
		return InsetText::insetAsXHTML(xs, rp, InsetText::JustText);
	return InsetText::xhtml(xs, rp);
}



/////////////////////////////////////////////////////////////////////
//
// InsetTabular
//
/////////////////////////////////////////////////////////////////////

InsetTabular::InsetTabular(Buffer * buf, row_type rows,
			   col_type columns)
	: Inset(buf), tabular(buf, max(rows, row_type(1)), max(columns, col_type(1))), scx_(0), 
	rowselect_(false), colselect_(false)
{
}


InsetTabular::InsetTabular(InsetTabular const & tab)
	: Inset(tab), tabular(tab.tabular),  scx_(0)
{
}


InsetTabular::~InsetTabular()
{
	hideDialogs("tabular", this);
}


void InsetTabular::setBuffer(Buffer & buf)
{
	tabular.setBuffer(buf);
	Inset::setBuffer(buf);
}


bool InsetTabular::insetAllowed(InsetCode code) const
{
	switch (code) {
	case FLOAT_CODE:
	case MARGIN_CODE:
	case MATHMACRO_CODE:
	case WRAP_CODE:
		return false;

	case CAPTION_CODE:
		return tabular.is_long_tabular;

	default:
		return true;
	}
}


bool InsetTabular::allowsCaptionVariation(std::string const & newtype) const
{
	return tabular.is_long_tabular &&
		(newtype == "Standard" || newtype == "LongTableNoNumber");
}


void InsetTabular::write(ostream & os) const
{
	os << "Tabular" << endl;
	tabular.write(os);
}


string InsetTabular::contextMenu(BufferView const &, int, int) const
{
	// FIXME: depending on the selection state,
	// we could offer a different menu.
	return cell(0)->contextMenuName() + ";" + contextMenuName();
}


string InsetTabular::contextMenuName() const
{
	return "context-tabular";
}


void InsetTabular::read(Lexer & lex)
{
	//bool const old_format = (lex.getString() == "\\LyXTable");

	tabular.read(lex);

	//if (old_format)
	//	return;

	lex.next();
	string token = lex.getString();
	while (lex && token != "\\end_inset") {
		lex.next();
		token = lex.getString();
	}
	if (!lex)
		lex.printError("Missing \\end_inset at this point. ");
}


int InsetTabular::rowFromY(Cursor & cur, int y) const
{
	// top y coordinate of tabular
	int h = yo(cur.bv()) - tabular.rowAscent(0) + offset_valign_;
	row_type r = 0;
	for (; r < tabular.nrows() && y > h; ++r)
		h += tabular.rowAscent(r) + tabular.rowDescent(r)
			+ tabular.interRowSpace(r);

	return r - 1;
}


int InsetTabular::columnFromX(Cursor & cur, int x) const
{
	// left x coordinate of tabular
	int w = xo(cur.bv()) + ADD_TO_TABULAR_WIDTH;
	col_type c = 0;
	for (; c < tabular.ncols() && x > w; ++c)
		w += tabular.cellWidth(c);
	return c - 1;
}


void InsetTabular::metrics(MetricsInfo & mi, Dimension & dim) const
{
	//lyxerr << "InsetTabular::metrics: " << mi.base.bv << " width: " <<
	//	mi.base.textwidth << "\n";
	LBUFERR(mi.base.bv);

	for (row_type r = 0; r < tabular.nrows(); ++r) {
		int maxasc = 0;
		int maxdes = 0;
		for (col_type c = 0; c < tabular.ncols(); ++c) {
			if (tabular.isPartOfMultiColumn(r, c)
				|| tabular.isPartOfMultiRow(r, c))
				// multicolumn or multirow cell, but not first one
				continue;
			idx_type const cell = tabular.cellIndex(r, c);
			Dimension dim;
			MetricsInfo m = mi;
			Length const p_width = tabular.getPWidth(cell);
			if (!p_width.zero())
				m.base.textwidth = p_width.inPixels(mi.base.textwidth);
			tabular.cellInset(cell)->metrics(m, dim);
			if (!p_width.zero())
				dim.wid = m.base.textwidth;
			tabular.cellInfo(cell).width = dim.wid + 2 * WIDTH_OF_LINE 
				+ tabular.interColumnSpace(cell);

			// FIXME(?): do we need a second metrics call?
			TextMetrics const & tm = 
				mi.base.bv->textMetrics(tabular.cellInset(cell)->getText(0));

			// determine horizontal offset because of decimal align (if necessary)
			int decimal_width = 0;
			if (tabular.getAlignment(cell) == LYX_ALIGN_DECIMAL) {
				InsetTableCell tail = InsetTableCell(*tabular.cellInset(cell).get());
				tail.setBuffer(tabular.buffer());
				// we need to set macrocontext position everywhere
				// otherwise we crash with nested insets (e.g. footnotes)
				// after decimal point
				DocIterator dit = tabular.cellInset(cell)->getText(0)->macrocontextPosition();
				dit.pop_back();
				dit.push_back(CursorSlice(tail));
				tail.setMacrocontextPositionRecursive(dit);

				// remove text leading decimal point
				docstring const align_d = tabular.column_info[c].decimal_point;
				dit = separatorPos(&tail, align_d);

				pit_type const psize = tail.paragraphs().front().size();
				if (dit) {
					tail.paragraphs().front().eraseChars(0,
						dit.pos() < psize ? dit.pos() + 1 : psize, false);
					Dimension dim1;
					tail.metrics(m, dim1);
					decimal_width = dim1.width();
				}
			}
			tabular.cell_info[r][c].decimal_hoffset = tm.width() - decimal_width;
			tabular.cell_info[r][c].decimal_width = decimal_width;

			// with LYX_VALIGN_BOTTOM the descent is relative to the last par
			// = descent of text in last par + TEXT_TO_INSET_OFFSET:
			int const lastpardes = tm.last().second->descent()
				+ TEXT_TO_INSET_OFFSET;
			int offset = 0;
			switch (tabular.getVAlignment(cell)) { 
				case Tabular::LYX_VALIGN_TOP:
					break; 
				case Tabular::LYX_VALIGN_MIDDLE:
					offset = -(dim.des - lastpardes)/2; 
					break; 
				case Tabular::LYX_VALIGN_BOTTOM:
					offset = -(dim.des - lastpardes); 
					break;
			}
			tabular.cell_info[r][c].voffset = offset;
			maxasc = max(maxasc, dim.asc - offset);
			maxdes = max(maxdes, dim.des + offset);
		}
		int const top_space = tabular.row_info[r].top_space_default ?
			default_line_space :
			tabular.row_info[r].top_space.inPixels(mi.base.textwidth);
		tabular.setRowAscent(r, maxasc + ADD_TO_HEIGHT + top_space);
		int const bottom_space = tabular.row_info[r].bottom_space_default ?
			default_line_space :
			tabular.row_info[r].bottom_space.inPixels(mi.base.textwidth);
		tabular.setRowDescent(r, maxdes + ADD_TO_HEIGHT + bottom_space);
	}

	// for top-alignment the first horizontal table line must be exactly at
	// the position of the base line of the surrounding text line
	// for bottom alignment, the same is for the last table line
	switch (tabular.tabular_valignment) {
	case Tabular::LYX_VALIGN_BOTTOM:
		offset_valign_ = tabular.rowAscent(0) - tabular.height();
		break;
	case Tabular::LYX_VALIGN_MIDDLE:
		offset_valign_ = (- tabular.height()) / 2 + tabular.rowAscent(0);
		break;
	case Tabular::LYX_VALIGN_TOP:
		offset_valign_ = tabular.rowAscent(0);
		break;
	}

	tabular.updateColumnWidths();
	dim.asc = tabular.rowAscent(0) - offset_valign_;	
	dim.des = tabular.height() - dim.asc;
	dim.wid = tabular.width() + 2 * ADD_TO_TABULAR_WIDTH;
}


bool InsetTabular::isCellSelected(Cursor & cur, row_type row, col_type col) 
	const
{
	if (&cur.inset() == this && cur.selection()) {
		if (cur.selIsMultiCell()) {
			row_type rs, re;
			col_type cs, ce;
			getSelection(cur, rs, re, cs, ce);
			
			idx_type const cell = tabular.cellIndex(row, col);
			col_type const cspan = tabular.columnSpan(cell);
			row_type const rspan = tabular.rowSpan(cell);
			if (col + cspan - 1 >= cs && col <= ce 
				&& row + rspan - 1 >= rs && row <= re)
				return true;
		} else 
			if (col == tabular.cellColumn(cur.idx()) 
				&& row == tabular.cellRow(cur.idx())) {
			CursorSlice const & beg = cur.selBegin();
			CursorSlice const & end = cur.selEnd();

			if ((end.lastpos() > 0 || end.lastpit() > 0)
				  && end.pos() == end.lastpos() && beg.pos() == 0
				  && end.pit() == end.lastpit() && beg.pit() == 0)
				return true;
		}
	}
	return false;
}


void InsetTabular::draw(PainterInfo & pi, int x, int y) const
{
	x += scx_ + ADD_TO_TABULAR_WIDTH;

	BufferView * bv = pi.base.bv;
	Cursor & cur = pi.base.bv->cursor();
	resetPos(cur);

	// FIXME: As the full background is painted in drawBackground(),
	// we have no choice but to do a full repaint for the Text cells.
	pi.full_repaint = true;

	bool const original_selection_state = pi.selected;

	idx_type idx = 0;
	first_visible_cell = Tabular::npos;

	int yy = y + offset_valign_;
	for (row_type r = 0; r < tabular.nrows(); ++r) {
		int nx = x;
		for (col_type c = 0; c < tabular.ncols(); ++c) {
			if (tabular.isPartOfMultiColumn(r, c))
				continue;
			
			idx = tabular.cellIndex(r, c);
			
			if (tabular.isPartOfMultiRow(r, c)) {
				nx += tabular.cellWidth(idx);
				continue;
			}

			if (first_visible_cell == Tabular::npos)
				first_visible_cell = idx;

			pi.selected |= isCellSelected(cur, r, c);
			int const cx = nx + tabular.textHOffset(idx);
			int const cy = yy + tabular.textVOffset(idx);
			// Cache the Inset position.
			bv->coordCache().insets().add(cell(idx).get(), cx, cy);
			cell(idx)->draw(pi, cx, cy);
			drawCellLines(pi, nx, yy, r, idx);
			nx += tabular.cellWidth(idx);
			pi.selected = original_selection_state;
		}

		if (r + 1 < tabular.nrows())
			yy += tabular.rowDescent(r) + tabular.rowAscent(r + 1) 
				+ tabular.interRowSpace(r + 1);
	}
}


void InsetTabular::drawBackground(PainterInfo & pi, int x, int y) const
{
	x += scx_ + ADD_TO_TABULAR_WIDTH;
	y += offset_valign_ - tabular.rowAscent(0);
	pi.pain.fillRectangle(x, y, tabular.width(), tabular.height(),
		pi.backgroundColor(this));
}


void InsetTabular::drawSelection(PainterInfo & pi, int x, int y) const
{
	Cursor & cur = pi.base.bv->cursor();
	resetPos(cur);

	x += scx_ + ADD_TO_TABULAR_WIDTH;

	if (!cur.selection())
		return;
	if (&cur.inset() != this)
		return;

	//resetPos(cur);

	bool const full_cell_selected = isCellSelected(cur,
		tabular.cellRow(cur.idx()), tabular.cellColumn(cur.idx()));

	if (cur.selIsMultiCell() || full_cell_selected) {
		for (row_type r = 0; r < tabular.nrows(); ++r) {
			int xx = x;
			for (col_type c = 0; c < tabular.ncols(); ++c) {
				if (tabular.isPartOfMultiColumn(r, c))
					continue;

				idx_type const cell = tabular.cellIndex(r, c);

				if (tabular.isPartOfMultiRow(r, c)) {
					xx += tabular.cellWidth(cell);
					continue;
				}
				int const w = tabular.cellWidth(cell);
				int const h = tabular.cellHeight(cell);
				int const yy = y - tabular.rowAscent(r) + offset_valign_;
				if (isCellSelected(cur, r, c))
					pi.pain.fillRectangle(xx, yy, w, h, Color_selection);
				xx += w;
			}
			if (r + 1 < tabular.nrows())
				y += tabular.rowDescent(r) + tabular.rowAscent(r + 1)
				     + tabular.interRowSpace(r + 1);
		}

	} 
	// FIXME: This code has no effect because InsetTableCell does not handle
	// drawSelection other than the trivial implementation in Inset.
	//else {
	//	x += cellXPos(cur.idx());
	//	x += tabular.textHOffset(cur.idx());
	//	cell(cur.idx())->drawSelection(pi, x, 0 /* ignored */);
	//}
}


void InsetTabular::drawCellLines(PainterInfo & pi, int x, int y,
				 row_type row, idx_type cell) const
{
	y -= tabular.rowAscent(row);
	int const w = tabular.cellWidth(cell);
	int const h = tabular.cellHeight(cell);
	Color const linecolor = pi.textColor(Color_tabularline);
	Color const gridcolor = pi.textColor(Color_tabularonoffline);

	// Top
	bool drawline = tabular.topLine(cell)
		|| (row > 0 && tabular.bottomLine(tabular.cellAbove(cell)));
	pi.pain.line(x, y, x + w, y,
		drawline ? linecolor : gridcolor,
		drawline ? Painter::line_solid : Painter::line_onoffdash);

	// Bottom
	drawline = tabular.bottomLine(cell);
	pi.pain.line(x, y + h, x + w, y + h,
		drawline ? linecolor : gridcolor,
		drawline ? Painter::line_solid : Painter::line_onoffdash);

	// Left
	col_type const col = tabular.cellColumn(cell);
	drawline = tabular.leftLine(cell)
		|| (col > 0 && tabular.rightLine(tabular.cellIndex(row, col - 1)));
	pi.pain.line(x, y, x, y + h,
		drawline ? linecolor : gridcolor,
		drawline ? Painter::line_solid : Painter::line_onoffdash);

	// Right
	x -= tabular.interColumnSpace(cell);
	col_type next_cell_col = col + 1;
	while (next_cell_col < tabular.ncols() 
		&& tabular.isMultiColumn(tabular.cellIndex(row, next_cell_col)))
		next_cell_col++;
	drawline = tabular.rightLine(cell)
		   || (next_cell_col < tabular.ncols()
		       && tabular.leftLine(tabular.cellIndex(row, next_cell_col)));
	pi.pain.line(x + w, y, x + w, y + h,
		drawline ? linecolor : gridcolor,
		drawline ? Painter::line_solid : Painter::line_onoffdash);
}


void InsetTabular::edit(Cursor & cur, bool front, EntryDirection)
{
	//lyxerr << "InsetTabular::edit: " << this << endl;
	cur.finishUndo();
	//cur.setSelection(false);
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
			cur.idx() = tabular.getFirstCellInRow(tabular.nrows() - 1);
		else
			cur.idx() = tabular.numberofcells - 1;
		cur.pit() = 0;
		cur.pos() = cur.lastpos(); // FIXME crude guess
	}
	cur.setCurrentFont();
	// FIXME: this accesses the position cache before it is initialized
	//resetPos(cur);
	//cur.bv().fitCursor();
}


void InsetTabular::updateBuffer(ParIterator const & it, UpdateType utype)
{
	// In a longtable, tell captions what the current float is
	Counters & cnts = buffer().masterBuffer()->params().documentClass().counters();
	string const saveflt = cnts.current_float();
	if (tabular.is_long_tabular) {
		cnts.current_float("table");
		// in longtables, we only step the counter once
		cnts.step(from_ascii("table"), utype);
		cnts.isLongtable(true);
	}

	ParIterator it2 = it;
	it2.forwardPos();
	size_t const end = it2.nargs();
	for ( ; it2.idx() < end; it2.top().forwardIdx())
		buffer().updateBuffer(it2, utype);

	//reset afterwards
	if (tabular.is_long_tabular) {
		cnts.current_float(saveflt);
		cnts.isLongtable(false);
	}
}


void InsetTabular::addToToc(DocIterator const & cpit, bool output_active) const
{
	DocIterator dit = cpit;
	dit.forwardPos();
	size_t const end = dit.nargs();
	for ( ; dit.idx() < end; dit.top().forwardIdx())
		cell(dit.idx())->addToToc(dit, output_active);
}


void InsetTabular::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	LYXERR(Debug::DEBUG, "# InsetTabular::doDispatch: cmd: " << cmd
			     << "\n  cur:" << cur);
	CursorSlice sl = cur.top();
	Cursor & bvcur = cur.bv().cursor();

	FuncCode const act = cmd.action();
	
	switch (act) {

	case LFUN_MOUSE_PRESS: {
		//lyxerr << "# InsetTabular::MousePress\n" << cur.bv().cursor() << endl;
		// select row
		if (cmd.x() < xo(cur.bv()) + ADD_TO_TABULAR_WIDTH
			|| cmd.x() > xo(cur.bv()) + tabular.width()) {
			row_type r = rowFromY(cur, cmd.y());
			cur.idx() = tabular.getFirstCellInRow(r);
			cur.pos() = 0;
			cur.resetAnchor();
			cur.idx() = tabular.getLastCellInRow(r);
			cur.pos() = cur.lastpos();
			cur.setSelection(true);
			bvcur = cur; 
			rowselect_ = true;
			break;
		}
		// select column
		int const y0 = yo(cur.bv()) - tabular.rowAscent(0) + offset_valign_;
		if (cmd.y() < y0 + ADD_TO_TABULAR_WIDTH 
			|| cmd.y() > y0 + tabular.height()) {
			col_type c = columnFromX(cur, cmd.x());
			cur.idx() = tabular.cellIndex(0, c);
			cur.pos() = 0;
			cur.resetAnchor();
			cur.idx() = tabular.cellIndex(tabular.nrows() - 1, c);
			cur.pos() = cur.lastpos();
			cur.setSelection(true);
			bvcur = cur; 
			colselect_ = true;
			break;
		}
		// do not reset cursor/selection if we have selected
		// some cells (bug 2715).
		if (cmd.button() == mouse_button::button3
		    && &bvcur.selBegin().inset() == this 
		    && bvcur.selIsMultiCell()) 
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
			if (!bvcur.realAnchor().hasPart(cur)) {
				cur.undispatched();
				break;
			}
			// select (additional) row
			if (rowselect_) {
				row_type r = rowFromY(cur, cmd.y());
				cur.idx() = tabular.getLastCellInRow(r);
				// we need to reset the cursor's pit and pos now, as the old ones
				// may no longer be valid.
				cur.pit() = 0;
				cur.pos() = 0;
				bvcur.setCursor(cur);
				bvcur.setSelection(true);
				break;
			}
			// select (additional) column
			if (colselect_) {
				col_type c = columnFromX(cur, cmd.x());
				cur.idx() = tabular.cellIndex(tabular.nrows() - 1, c);
				// we need to reset the cursor's pit and pos now, as the old ones
				// may no longer be valid.
				cur.pit() = 0;
				cur.pos() = 0;
				bvcur.setCursor(cur);
				bvcur.setSelection(true);
				break;
			}
			// only update if selection changes
			if (bvcur.idx() == cur.idx() &&
				!(bvcur.realAnchor().idx() == cur.idx() && bvcur.pos() != cur.pos()))
				cur.noScreenUpdate();
			setCursorFromCoordinates(cur, cmd.x(), cmd.y());
			bvcur.setCursor(cur);
			bvcur.setSelection(true);
			// if this is a multicell selection, we just set the cursor to
			// the beginning of the cell's text.
			if (bvcur.selIsMultiCell()) {
				bvcur.pit() = bvcur.lastpit();
				bvcur.pos() = bvcur.lastpos();
			}
		}
		break;

	case LFUN_MOUSE_RELEASE:
		rowselect_ = false;
		colselect_ = false;
		break;

	case LFUN_CELL_BACKWARD:
		movePrevCell(cur);
		cur.setSelection(false);
		break;

	case LFUN_CELL_FORWARD:
		moveNextCell(cur);
		cur.setSelection(false);
		break;

	case LFUN_CHAR_FORWARD_SELECT:
	case LFUN_CHAR_FORWARD:
	case LFUN_CHAR_BACKWARD_SELECT:
	case LFUN_CHAR_BACKWARD:
	case LFUN_CHAR_RIGHT_SELECT:
	case LFUN_CHAR_RIGHT:
	case LFUN_CHAR_LEFT_SELECT:
	case LFUN_CHAR_LEFT: 
	case LFUN_WORD_FORWARD:
	case LFUN_WORD_FORWARD_SELECT:
	case LFUN_WORD_BACKWARD:
	case LFUN_WORD_BACKWARD_SELECT:
	case LFUN_WORD_RIGHT:
	case LFUN_WORD_RIGHT_SELECT:
	case LFUN_WORD_LEFT:
	case LFUN_WORD_LEFT_SELECT: {
		// determine whether we move to next or previous cell, where to enter 
		// the new cell from, and which command to "finish" (i.e., exit the
		// inset) with:
		bool next_cell;
		EntryDirection entry_from = ENTRY_DIRECTION_IGNORE;
		FuncCode finish_lfun;

		if (act == LFUN_CHAR_FORWARD 
				|| act == LFUN_CHAR_FORWARD_SELECT
				|| act == LFUN_WORD_FORWARD
				|| act == LFUN_WORD_FORWARD_SELECT) {
			next_cell = true;
			finish_lfun = LFUN_FINISHED_FORWARD;
		}
		else if (act == LFUN_CHAR_BACKWARD
				|| act == LFUN_CHAR_BACKWARD_SELECT
				|| act == LFUN_WORD_BACKWARD
				|| act == LFUN_WORD_BACKWARD_SELECT) {
			next_cell = false;
			finish_lfun = LFUN_FINISHED_BACKWARD;
		}
		// LEFT or RIGHT commands --- the interpretation will depend on the 
		// table's direction.
		else {
			bool const right = act == LFUN_CHAR_RIGHT
				|| act == LFUN_CHAR_RIGHT_SELECT
				|| act == LFUN_WORD_RIGHT
				|| act == LFUN_WORD_RIGHT_SELECT;
			next_cell = isRightToLeft(cur) != right;
			
			if (lyxrc.visual_cursor)
				entry_from = right ? ENTRY_DIRECTION_LEFT:ENTRY_DIRECTION_RIGHT;

			finish_lfun = right ? LFUN_FINISHED_RIGHT : LFUN_FINISHED_LEFT;
		}

		bool const select =	act == LFUN_CHAR_FORWARD_SELECT 
		    || act == LFUN_CHAR_BACKWARD_SELECT
		    || act == LFUN_CHAR_RIGHT_SELECT
		    || act == LFUN_CHAR_LEFT_SELECT
			|| act == LFUN_WORD_FORWARD_SELECT
			|| act == LFUN_WORD_RIGHT_SELECT
			|| act == LFUN_WORD_BACKWARD_SELECT
			|| act == LFUN_WORD_LEFT_SELECT;

		// If we have a multicell selection or we're 
		// not doing some LFUN_*_SELECT thing anyway...
		if (!cur.selIsMultiCell() || !select) {
			col_type const c = tabular.cellColumn(cur.idx());
			row_type const r = tabular.cellRow(cur.idx());
			// Are we trying to select the whole cell and is the whole cell 
			// not yet selected?
			bool const select_whole = select && !isCellSelected(cur, r, c) &&
				((next_cell && cur.pit() == cur.lastpit() 
				&& cur.pos() == cur.lastpos())
				|| (!next_cell && cur.pit() == 0 && cur.pos() == 0));

			bool const empty_cell = cur.lastpos() == 0 && cur.lastpit() == 0;

			// ...try to dispatch to the cell's inset.
			cell(cur.idx())->dispatch(cur, cmd);

			// When we already have a selection we want to select the whole cell
			// before going to the next cell.
			if (select_whole && !empty_cell){
				getText(cur.idx())->selectAll(cur);
				cur.dispatched();
				cur.screenUpdateFlags(Update::Force | Update::FitCursor);
				break;
			}

			// FIXME: When we support the selection of an empty cell, remove 
			// the !empty_cell from this condition. For now we jump to the next
			// cell if the current cell is empty.
			if (cur.result().dispatched() && !empty_cell)
				break;
		}

		// move to next/prev cell, as appropriate
		// note that we will always do this if we're selecting and we have
		// a multicell selection
		LYXERR(Debug::RTL, "entering " << (next_cell ? "next" : "previous")
			<< " cell from: " << int(entry_from));
		if (next_cell)
			moveNextCell(cur, entry_from);
		else
			movePrevCell(cur, entry_from);
		// if we're exiting the table, call the appropriate FINISHED lfun
		if (sl == cur.top()) {
			cmd = FuncRequest(finish_lfun);
			cur.undispatched();
		} else
			cur.dispatched();

		cur.screenUpdateFlags(Update::Force | Update::FitCursor);
		break;

	}

	case LFUN_DOWN_SELECT:
	case LFUN_DOWN:
		if (!(cur.selection() && cur.selIsMultiCell()))
			cell(cur.idx())->dispatch(cur, cmd);
		
		cur.dispatched(); // override the cell's decision
		if (sl == cur.top()) {
			// if our Text didn't do anything to the cursor
			// then we try to put the cursor into the cell below
			// setting also the right targetX.
			cur.selHandle(act == LFUN_DOWN_SELECT);
			if (tabular.cellRow(cur.idx()) != tabular.nrows() - 1) {
				int const xtarget = cur.targetX();
				// WARNING: Once cur.idx() has been reset, the cursor is in
				// an inconsistent state until pos() has been set. Be careful
				// what you do with it!
				cur.idx() = tabular.cellBelow(cur.idx());
				cur.pit() = 0;
				TextMetrics const & tm =
					cur.bv().textMetrics(cell(cur.idx())->getText(0));
				cur.pos() = tm.x2pos(cur.pit(), 0, xtarget);
				cur.setCurrentFont();
			}
		}
		if (sl == cur.top()) {
			// we trick it to go to forward after leaving the
			// tabular.
			cmd = FuncRequest(LFUN_FINISHED_FORWARD);
			cur.undispatched();
		}
		if (cur.selIsMultiCell()) {
			cur.pit() = cur.lastpit();
			cur.pos() = cur.lastpos();
			cur.setCurrentFont();
			cur.screenUpdateFlags(Update::Force | Update::FitCursor);
			return;
		}
		cur.screenUpdateFlags(Update::Force | Update::FitCursor);
		break;

	case LFUN_UP_SELECT:
	case LFUN_UP:
		if (!(cur.selection() && cur.selIsMultiCell()))
			cell(cur.idx())->dispatch(cur, cmd);
		cur.dispatched(); // override the cell's decision
		if (sl == cur.top()) {
			// if our Text didn't do anything to the cursor
			// then we try to put the cursor into the cell above
			// setting also the right targetX.
			cur.selHandle(act == LFUN_UP_SELECT);
			if (tabular.cellRow(cur.idx()) != 0) {
				int const xtarget = cur.targetX();
				// WARNING: Once cur.idx() has been reset, the cursor is in
				// an inconsistent state until pos() has been set. Be careful
				// what you do with it!
				cur.idx() = tabular.cellAbove(cur.idx());
				cur.pit() = cur.lastpit();
				Text const * text = cell(cur.idx())->getText(0);
				TextMetrics const & tm = cur.bv().textMetrics(text);
				ParagraphMetrics const & pm =
					tm.parMetrics(cur.lastpit());
				cur.pos() = tm.x2pos(cur.pit(), pm.rows().size()-1, xtarget);
				cur.setCurrentFont();
			}
		}
		if (sl == cur.top()) {
			cmd = FuncRequest(LFUN_UP);
			cur.undispatched();
		}
		if (cur.selIsMultiCell()) {
			cur.pit() = 0;
			cur.pos() = cur.lastpos();
			cur.setCurrentFont();
			cur.screenUpdateFlags(Update::Force | Update::FitCursor);
			return;
		}
		cur.screenUpdateFlags(Update::Force | Update::FitCursor);
		break;

//	case LFUN_SCREEN_DOWN: {
//		//if (hasSelection())
//		//	cur.selection() = false;
//		col_type const col = tabular.cellColumn(cur.idx());
//		int const t =	cur.bv().top_y() + cur.bv().height();
//		if (t < yo() + tabular.getHeightOfTabular()) {
//			cur.bv().scrollDocView(t, true);
//			cur.idx() = tabular.cellBelow(first_visible_cell) + col;
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
//			cur.bv().scrollDocView(t, true);
//			if (yo() > 0)
//				cur.idx() = col;
//			else
//				cur.idx() = tabular.cellBelow(first_visible_cell) + col;
//		} else {
//			cur.idx() = col;
//		}
//		cur.par() = cur.lastpar();
//		cur.pos() = cur.lastpos();
//		break;
//	}

	case LFUN_LAYOUT_TABULAR:
		cur.bv().showDialog("tabular");
		break;

	case LFUN_INSET_MODIFY: {
		string arg;
		if (cmd.getArg(1) == "from-dialog")
			arg = cmd.getArg(0) + to_utf8(cmd.argument().substr(19));
		else
			arg = to_utf8(cmd.argument());
		if (!tabularFeatures(cur, arg))
			cur.undispatched();
		break;
	}

	// insert file functions
	case LFUN_FILE_INSERT_PLAINTEXT_PARA:
	case LFUN_FILE_INSERT_PLAINTEXT:
		// FIXME UNICODE
		if (FileName::isAbsolute(to_utf8(cmd.argument()))) {
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
		}
		break;

	case LFUN_CUT:
		if (cur.selIsMultiCell()) {
			if (copySelection(cur)) {
				cur.recordUndoInset(DELETE_UNDO);
				cutSelection(cur);
			}
		} else
			cell(cur.idx())->dispatch(cur, cmd);
		break;

	case LFUN_SELF_INSERT:
		if (cur.selIsMultiCell()) {
			cur.recordUndoInset(DELETE_UNDO);
			cutSelection(cur);
			BufferView * bv = &cur.bv();
			docstring::const_iterator cit = cmd.argument().begin();
			docstring::const_iterator const end = cmd.argument().end();
			for (; cit != end; ++cit)
				bv->translateAndInsert(*cit, getText(cur.idx()), cur);
	
			cur.resetAnchor();
			bv->bookmarkEditPosition();
		} else
			cell(cur.idx())->dispatch(cur, cmd);
		break;

	case LFUN_CHAR_DELETE_BACKWARD:
	case LFUN_CHAR_DELETE_FORWARD:
		if (cur.selIsMultiCell()) {
			cur.recordUndoInset(DELETE_UNDO);
			cutSelection(cur);
		} else
			cell(cur.idx())->dispatch(cur, cmd);
		break;

	case LFUN_COPY:
		if (!cur.selection())
			break;
		if (cur.selIsMultiCell()) {
			cur.finishUndo();
			copySelection(cur);
		} else
			cell(cur.idx())->dispatch(cur, cmd);
		break;

	case LFUN_CLIPBOARD_PASTE:
	case LFUN_PRIMARY_SELECTION_PASTE: {
		docstring const clip = (act == LFUN_CLIPBOARD_PASTE) ?
			theClipboard().getAsText(Clipboard::PlainTextType) :
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
		if (!tabularStackDirty()) {
			if (!cur.selIsMultiCell())
				cell(cur.idx())->dispatch(cur, cmd);
			break;
		}
		if (theClipboard().isInternal()) {
			cur.recordUndoInset(INSERT_UNDO);
			pasteClipboard(cur);
		}
		break;

	case LFUN_FONT_EMPH:
	case LFUN_FONT_BOLD:
	case LFUN_FONT_BOLDSYMBOL:
	case LFUN_FONT_ROMAN:
	case LFUN_FONT_NOUN:
	case LFUN_FONT_ITAL:
	case LFUN_FONT_FRAK:
	case LFUN_FONT_TYPEWRITER:
	case LFUN_FONT_SANS:
	case LFUN_TEXTSTYLE_APPLY:
	case LFUN_TEXTSTYLE_UPDATE:
	case LFUN_FONT_SIZE:
	case LFUN_FONT_UNDERLINE:
	case LFUN_FONT_STRIKEOUT:
	case LFUN_FONT_UNDERUNDERLINE:
	case LFUN_FONT_UNDERWAVE:
	case LFUN_LANGUAGE:
	case LFUN_PARAGRAPH_PARAMS_APPLY:
	case LFUN_PARAGRAPH_PARAMS:
	case LFUN_WORD_CAPITALIZE:
	case LFUN_WORD_UPCASE:
	case LFUN_WORD_LOWCASE:
	case LFUN_CHARS_TRANSPOSE:
		if (cur.selIsMultiCell()) {
			row_type rs, re;
			col_type cs, ce;
			getSelection(cur, rs, re, cs, ce);
			Cursor tmpcur = cur;
			for (row_type r = rs; r <= re; ++r) {
				for (col_type c = cs; c <= ce; ++c) {
					// cursor follows cell:
					tmpcur.idx() = tabular.cellIndex(r, c);
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

	case LFUN_INSET_SETTINGS:
		// relay this lfun to Inset, not to the cell.
		Inset::doDispatch(cur, cmd);
		break;

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
	switch (cmd.action()) {
	case LFUN_INSET_MODIFY: {
		if (&cur.inset() != this || cmd.getArg(0) != "tabular") 
			break;

		// FIXME: We only check for the very first argument...
		string const s = cmd.getArg(1);
		// We always enable the lfun if it is coming from the dialog
		// because the dialog makes sure all the settings are valid,
		// even though the first argument might not be valid now.
		if (s == "from-dialog") {
			status.setEnabled(true);
			return true;
		}

		int action = Tabular::LAST_ACTION;
		int i = 0;
		for (; tabularFeature[i].action != Tabular::LAST_ACTION; ++i) {
			if (tabularFeature[i].feature == s) {
				action = tabularFeature[i].action;
				break;
			}
		}
		if (action == Tabular::LAST_ACTION) {
			status.clear();
			status.setUnknown(true);
			return true;
		}

		string const argument = cmd.getLongArg(2);

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
		case Tabular::SET_SPECIAL_MULTICOLUMN:
		case Tabular::APPEND_ROW:
		case Tabular::APPEND_COLUMN:
		case Tabular::DELETE_ROW:
		case Tabular::DELETE_COLUMN:
		case Tabular::COPY_ROW:
		case Tabular::COPY_COLUMN:
		case Tabular::SET_TOP_SPACE:
		case Tabular::SET_BOTTOM_SPACE:
		case Tabular::SET_INTERLINE_SPACE:
			status.clear();
			return true;

		case Tabular::SET_TABULAR_WIDTH:
			status.setEnabled(!tabular.rotate &&  !tabular.is_long_tabular
				&& tabular.tabular_valignment == Tabular::LYX_VALIGN_MIDDLE);
			break;

		case Tabular::MOVE_COLUMN_RIGHT:
		case Tabular::MOVE_COLUMN_LEFT:
		case Tabular::MOVE_ROW_DOWN:
		case Tabular::MOVE_ROW_UP: {
			if (cur.selection()) {
				status.message(_("Selections not supported."));
				status.setEnabled(false);
				break;
			}

			if ((action == Tabular::MOVE_COLUMN_RIGHT &&
				tabular.ncols() == tabular.cellColumn(cur.idx()) + 1) ||
			    (action == Tabular::MOVE_COLUMN_LEFT &&
				tabular.cellColumn(cur.idx()) == 0) ||
			    (action == Tabular::MOVE_ROW_DOWN &&
				tabular.nrows() == tabular.cellRow(cur.idx()) + 1) ||
			    (action == Tabular::MOVE_ROW_UP &&
				tabular.cellRow(cur.idx()) == 0)) {
					status.setEnabled(false);
					break;
			}

			if (action == Tabular::MOVE_COLUMN_RIGHT ||
			    action == Tabular::MOVE_COLUMN_LEFT) {
				if (tabular.hasMultiColumn(tabular.cellColumn(cur.idx())) ||
				    tabular.hasMultiColumn(tabular.cellColumn(cur.idx()) +
					(action == Tabular::MOVE_COLUMN_RIGHT ? 1 : -1))) {
					status.message(_("Multi-column in current or"
							 " destination column."));
					status.setEnabled(false);
					break;
				}
			}

			if (action == Tabular::MOVE_ROW_DOWN ||
			    action == Tabular::MOVE_ROW_UP) {
				if (tabular.hasMultiRow(tabular.cellRow(cur.idx())) ||
				    tabular.hasMultiRow(tabular.cellRow(cur.idx()) +
					(action == Tabular::MOVE_ROW_DOWN ? 1 : -1))) {
					status.message(_("Multi-row in current or"
							 " destination row."));
					status.setEnabled(false);
					break;
				}
			}

			status.setEnabled(true);
			break;
		}

		case Tabular::SET_DECIMAL_POINT:
			status.setEnabled(
				tabular.getAlignment(cur.idx()) == LYX_ALIGN_DECIMAL);
			break;

		case Tabular::SET_MULTICOLUMN:
		case Tabular::UNSET_MULTICOLUMN:
		case Tabular::MULTICOLUMN:
			// If a row is set as longtable caption, it must not be allowed
			// to unset that this row is a multicolumn.
			status.setEnabled(sel_row_start == sel_row_end
				&& !tabular.ltCaption(tabular.cellRow(cur.idx())));
			status.setOnOff(tabular.isMultiColumn(cur.idx()));
			break;

		case Tabular::SET_MULTIROW:
		case Tabular::UNSET_MULTIROW:
		case Tabular::MULTIROW:
			// If a row is set as longtable caption, it must not be allowed
			// to unset that this row is a multirow.
			status.setEnabled(sel_col_start == sel_col_end
				&& !tabular.ltCaption(tabular.cellRow(cur.idx())));
			status.setOnOff(tabular.isMultiRow(cur.idx()));
			break;

		case Tabular::SET_ALL_LINES:
		case Tabular::UNSET_ALL_LINES:
		case Tabular::SET_BORDER_LINES:
			status.setEnabled(!tabular.ltCaption(tabular.cellRow(cur.idx())));
			break;

		case Tabular::SET_LINE_TOP:
		case Tabular::SET_LINE_BOTTOM:
		case Tabular::SET_LINE_LEFT:
		case Tabular::SET_LINE_RIGHT:
			status.setEnabled(!tabular.ltCaption(tabular.cellRow(cur.idx())));
			break;

		case Tabular::TOGGLE_LINE_TOP:
			status.setEnabled(!tabular.ltCaption(tabular.cellRow(cur.idx())));
			status.setOnOff(tabular.topLine(cur.idx()));
			break;

		case Tabular::TOGGLE_LINE_BOTTOM:
			status.setEnabled(!tabular.ltCaption(tabular.cellRow(cur.idx())));
			status.setOnOff(tabular.bottomLine(cur.idx()));
			break;

		case Tabular::TOGGLE_LINE_LEFT:
			status.setEnabled(!tabular.ltCaption(tabular.cellRow(cur.idx())));
			status.setOnOff(tabular.leftLine(cur.idx()));
			break;

		case Tabular::TOGGLE_LINE_RIGHT:
			status.setEnabled(!tabular.ltCaption(tabular.cellRow(cur.idx())));
			status.setOnOff(tabular.rightLine(cur.idx()));
			break;

		// multirow cells only inherit the alignment of the column if the column has
		// no width, otherwise they are left-aligned
		// therefore allow always left but right and center only if there is no width
		case Tabular::M_ALIGN_LEFT:
			flag = false;
		case Tabular::ALIGN_LEFT:
			status.setOnOff(tabular.getAlignment(cur.idx(), flag) == LYX_ALIGN_LEFT);
			break;

		case Tabular::M_ALIGN_RIGHT:
			flag = false;
		case Tabular::ALIGN_RIGHT:
			status.setEnabled(!(tabular.isMultiRow(cur.idx()) 
				&& !tabular.getPWidth(cur.idx()).zero()));
			status.setOnOff(tabular.getAlignment(cur.idx(), flag) == LYX_ALIGN_RIGHT);
			break;

		case Tabular::M_ALIGN_CENTER:
			flag = false;
		case Tabular::ALIGN_CENTER:
			status.setEnabled(!(tabular.isMultiRow(cur.idx()) 
				&& !tabular.getPWidth(cur.idx()).zero()));
			status.setOnOff(tabular.getAlignment(cur.idx(), flag) == LYX_ALIGN_CENTER);
			break;

		case Tabular::ALIGN_BLOCK:
			status.setEnabled(!tabular.getPWidth(cur.idx()).zero()
				&& !tabular.isMultiRow(cur.idx()));
			status.setOnOff(tabular.getAlignment(cur.idx(), flag) == LYX_ALIGN_BLOCK);
			break;

		case Tabular::ALIGN_DECIMAL:
			status.setEnabled(!tabular.isMultiRow(cur.idx()) 
				&& !tabular.isMultiColumn(cur.idx()));
			status.setOnOff(tabular.getAlignment(cur.idx(), true) == LYX_ALIGN_DECIMAL);
			break;

		case Tabular::M_VALIGN_TOP:
			flag = false;
		case Tabular::VALIGN_TOP:
			status.setEnabled(!tabular.getPWidth(cur.idx()).zero()
				&& !tabular.isMultiRow(cur.idx()));
			status.setOnOff(
				tabular.getVAlignment(cur.idx(), flag) == Tabular::LYX_VALIGN_TOP);
			break;

		case Tabular::M_VALIGN_BOTTOM:
			flag = false;
		case Tabular::VALIGN_BOTTOM:
			status.setEnabled(!tabular.getPWidth(cur.idx()).zero()
				&& !tabular.isMultiRow(cur.idx()));
			status.setOnOff(
				tabular.getVAlignment(cur.idx(), flag) == Tabular::LYX_VALIGN_BOTTOM);
			break;

		case Tabular::M_VALIGN_MIDDLE:
			flag = false;
		case Tabular::VALIGN_MIDDLE:
			status.setEnabled(!tabular.getPWidth(cur.idx()).zero()
				&& !tabular.isMultiRow(cur.idx()));
			status.setOnOff(
				tabular.getVAlignment(cur.idx(), flag) == Tabular::LYX_VALIGN_MIDDLE);
			break;

		case Tabular::SET_LONGTABULAR:
			// setting as longtable is not allowed when table is inside a float
			if (cur.innerInsetOfType(FLOAT_CODE) != 0
				|| cur.innerInsetOfType(WRAP_CODE) != 0)
				status.setEnabled(false);
			else
				status.setEnabled(true);
			status.setOnOff(tabular.is_long_tabular);
			break;

		case Tabular::UNSET_LONGTABULAR:
			status.setOnOff(!tabular.is_long_tabular);
			break;

		case Tabular::TOGGLE_ROTATE_TABULAR:
		case Tabular::SET_ROTATE_TABULAR:
			status.setOnOff(tabular.rotate != 0);
			break;

		case Tabular::TABULAR_VALIGN_TOP:
			status.setEnabled(tabular.tabular_width.zero());
			status.setOnOff(tabular.tabular_valignment 
				== Tabular::LYX_VALIGN_TOP);
			break;
		case Tabular::TABULAR_VALIGN_MIDDLE:
			status.setEnabled(tabular.tabular_width.zero());
			status.setOnOff(tabular.tabular_valignment 
				== Tabular::LYX_VALIGN_MIDDLE);
			break;
		case Tabular::TABULAR_VALIGN_BOTTOM:
			status.setEnabled(tabular.tabular_width.zero());
			status.setOnOff(tabular.tabular_valignment 
				== Tabular::LYX_VALIGN_BOTTOM);
			break;

		case Tabular::LONGTABULAR_ALIGN_LEFT:
			status.setOnOff(tabular.longtabular_alignment 
				== Tabular::LYX_LONGTABULAR_ALIGN_LEFT);
			break;
		case Tabular::LONGTABULAR_ALIGN_CENTER:
			status.setOnOff(tabular.longtabular_alignment 
				== Tabular::LYX_LONGTABULAR_ALIGN_CENTER);
			break;
		case Tabular::LONGTABULAR_ALIGN_RIGHT:
			status.setOnOff(tabular.longtabular_alignment 
				== Tabular::LYX_LONGTABULAR_ALIGN_RIGHT);
			break;

		case Tabular::UNSET_ROTATE_TABULAR:
			status.setOnOff(tabular.rotate == 0);
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

		// every row can only be one thing:
		// either a footer or header
		case Tabular::SET_LTFIRSTHEAD:
			status.setEnabled(sel_row_start == sel_row_end);
			status.setOnOff(tabular.getRowOfLTFirstHead(sel_row_start, dummyltt));
			break;

		case Tabular::UNSET_LTFIRSTHEAD:
			status.setEnabled(sel_row_start == sel_row_end);
			status.setOnOff(!tabular.getRowOfLTFirstHead(sel_row_start, dummyltt));
			break;

		case Tabular::SET_LTHEAD:
			status.setEnabled(sel_row_start == sel_row_end);
			status.setOnOff(tabular.getRowOfLTHead(sel_row_start, dummyltt));
			break;

		case Tabular::UNSET_LTHEAD:
			status.setEnabled(sel_row_start == sel_row_end);
			status.setOnOff(!tabular.getRowOfLTHead(sel_row_start, dummyltt));
			break;

		case Tabular::SET_LTFOOT:
			status.setEnabled(sel_row_start == sel_row_end);
			status.setOnOff(tabular.getRowOfLTFoot(sel_row_start, dummyltt));
			break;

		case Tabular::UNSET_LTFOOT:
			status.setEnabled(sel_row_start == sel_row_end);
			status.setOnOff(!tabular.getRowOfLTFoot(sel_row_start, dummyltt));
			break;

		case Tabular::SET_LTLASTFOOT:
			status.setEnabled(sel_row_start == sel_row_end);
			status.setOnOff(tabular.getRowOfLTLastFoot(sel_row_start, dummyltt));
			break;

		case Tabular::UNSET_LTLASTFOOT:
			status.setEnabled(sel_row_start == sel_row_end);
			status.setOnOff(!tabular.getRowOfLTLastFoot(sel_row_start, dummyltt));
			break;

		case Tabular::SET_LTNEWPAGE:
			status.setOnOff(tabular.getLTNewPage(sel_row_start));
			break;
		case Tabular::UNSET_LTNEWPAGE:
			status.setOnOff(!tabular.getLTNewPage(sel_row_start));
			break;

		// only one row in head/firsthead/foot/lasthead can be the caption
		// and a multirow cannot be set as caption
		case Tabular::SET_LTCAPTION:
			status.setEnabled(sel_row_start == sel_row_end
				&& (!tabular.getRowOfLTFirstHead(sel_row_start, dummyltt)
				 || !tabular.haveLTCaption(Tabular::CAPTION_FIRSTHEAD))
				&& (!tabular.getRowOfLTHead(sel_row_start, dummyltt)
				 || !tabular.haveLTCaption(Tabular::CAPTION_HEAD))
				&& (!tabular.getRowOfLTFoot(sel_row_start, dummyltt)
				 || !tabular.haveLTCaption(Tabular::CAPTION_FOOT))
				&& (!tabular.getRowOfLTLastFoot(sel_row_start, dummyltt)
				 || !tabular.haveLTCaption(Tabular::CAPTION_LASTFOOT))
				&& !tabular.isMultiRow(sel_row_start));
			status.setOnOff(tabular.ltCaption(sel_row_start));
			break;

		case Tabular::UNSET_LTCAPTION:
			status.setEnabled(sel_row_start == sel_row_end && tabular.ltCaption(sel_row_start));
			break;

		case Tabular::TOGGLE_LTCAPTION:
			status.setEnabled(sel_row_start == sel_row_end && (tabular.ltCaption(sel_row_start)
				|| ((!tabular.getRowOfLTFirstHead(sel_row_start, dummyltt)
				  || !tabular.haveLTCaption(Tabular::CAPTION_FIRSTHEAD))
				 && (!tabular.getRowOfLTHead(sel_row_start, dummyltt)
				  || !tabular.haveLTCaption(Tabular::CAPTION_HEAD))
				 && (!tabular.getRowOfLTFoot(sel_row_start, dummyltt)
				  || !tabular.haveLTCaption(Tabular::CAPTION_FOOT))
				 && (!tabular.getRowOfLTLastFoot(sel_row_start, dummyltt)
				  || !tabular.haveLTCaption(Tabular::CAPTION_LASTFOOT)))));
			status.setOnOff(tabular.ltCaption(sel_row_start));
			break;

		case Tabular::SET_BOOKTABS:
			status.setOnOff(tabular.use_booktabs);
			break;

		case Tabular::UNSET_BOOKTABS:
			status.setOnOff(!tabular.use_booktabs);
			break;

		default:
			status.clear();
			status.setEnabled(false);
			break;
		}
		return true;
	}

	case LFUN_CAPTION_INSERT: {
		// caption is only allowed in caption cell of longtable
		if (!tabular.ltCaption(tabular.cellRow(cur.idx()))) {
			status.setEnabled(false);
			return true;
		}
		// only standard caption is allowed
		string arg = cmd.getArg(0);
		if (!arg.empty() && arg != "Standard") {
			status.setEnabled(false);
			return true;
		}
		// check if there is already a caption
		bool have_caption = false;
		InsetTableCell itc = InsetTableCell(*tabular.cellInset(cur.idx()).get());
		ParagraphList::const_iterator pit = itc.paragraphs().begin();
		ParagraphList::const_iterator pend = itc.paragraphs().end();
		for (; pit != pend; ++pit) {
			InsetList::const_iterator it  = pit->insetList().begin();
			InsetList::const_iterator end = pit->insetList().end();
			for (; it != end; ++it) {
				if (it->inset->lyxCode() == CAPTION_CODE) {
					have_caption = true;
					break;
				}
			}
		}
		status.setEnabled(!have_caption);
		return true;
	}

	// These are only enabled inside tabular
	case LFUN_CELL_BACKWARD:
	case LFUN_CELL_FORWARD:
		status.setEnabled(true);
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
	case LFUN_ARGUMENT_INSERT:
	case LFUN_BOX_INSERT:
	case LFUN_BRANCH_INSERT:
	case LFUN_PHANTOM_INSERT:
	case LFUN_WRAP_INSERT:
	case LFUN_PREVIEW_INSERT:
	case LFUN_ERT_INSERT: {
		if (cur.selIsMultiCell()) {
			status.setEnabled(false);
			return true;
		} else
			return cell(cur.idx())->getStatus(cur, cmd, status);
	}

	// disable in non-fixed-width cells
	case LFUN_PARAGRAPH_BREAK:
		// multirow does not allow paragraph breaks
		if (tabular.isMultiRow(cur.idx())) {
			status.setEnabled(false);
			return true;
		}
	case LFUN_NEWLINE_INSERT: {
		if (tabular.getPWidth(cur.idx()).zero()) {
			status.setEnabled(false);
			return true;
		} else
			return cell(cur.idx())->getStatus(cur, cmd, status);
	}

	case LFUN_NEWPAGE_INSERT:
		status.setEnabled(false);
		return true;

	case LFUN_PASTE:
		if (tabularStackDirty() && theClipboard().isInternal()) {
			if (cur.selIsMultiCell()) {
				row_type rs, re;
				col_type cs, ce;
				getSelection(cur, rs, re, cs, ce);
				if (paste_tabular && paste_tabular->ncols() == ce - cs + 1
					  && paste_tabular->nrows() == re - rs + 1)
					status.setEnabled(true);	
				else {
					status.setEnabled(false);
					status.message(_("Selection size should match clipboard content."));
				}
			} else
				status.setEnabled(true);
			return true;
		}
		return cell(cur.idx())->getStatus(cur, cmd, status);

	case LFUN_INSET_SETTINGS:
		// relay this lfun to Inset, not to the cell.
		return Inset::getStatus(cur, cmd, status);

	default:
		// we try to handle this event in the insets dispatch function.
		return cell(cur.idx())->getStatus(cur, cmd, status);
	}
	return false;
}


Inset::DisplayType InsetTabular::display() const
{
		if (tabular.is_long_tabular) {
			switch (tabular.longtabular_alignment) {
			case Tabular::LYX_LONGTABULAR_ALIGN_LEFT:
				return AlignLeft;
			case Tabular::LYX_LONGTABULAR_ALIGN_CENTER:
				return AlignCenter;
			case Tabular::LYX_LONGTABULAR_ALIGN_RIGHT:
				return AlignRight;
			default:
				return AlignCenter;
			}
		} else
			return Inline;
}


void InsetTabular::latex(otexstream & os, OutputParams const & runparams) const
{
	tabular.latex(os, runparams);
}


int InsetTabular::plaintext(odocstringstream & os,
        OutputParams const & runparams, size_t max_length) const
{
	os << '\n'; // output table on a new line
	int const dp = runparams.linelen > 0 ? runparams.depth : 0;
	tabular.plaintext(os, runparams, dp, false, 0, max_length);
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


docstring InsetTabular::xhtml(XHTMLStream & xs, OutputParams const & rp) const
{
	return tabular.xhtml(xs, rp);
}


void InsetTabular::validate(LaTeXFeatures & features) const
{
	tabular.validate(features);
	// FIXME XHTML
	// It'd be better to be able to get this from an InsetLayout, but at present
	// InsetLayouts do not seem really to work for things that aren't InsetTexts.
	if (features.runparams().flavor == OutputParams::HTML)
		features.addCSSSnippet(
			"table { border: 1px solid black; display: inline-block; }\n"
			"td { border: 1px solid black; padding: 0.5ex; }");
}


shared_ptr<InsetTableCell const> InsetTabular::cell(idx_type idx) const
{
	return tabular.cellInset(idx);
}


shared_ptr<InsetTableCell> InsetTabular::cell(idx_type idx)
{
	return tabular.cellInset(idx);
}


void InsetTabular::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const
{
	cell(sl.idx())->cursorPos(bv, sl, boundary, x, y);

	// y offset	correction
	y += cellYPos(sl.idx());
	y += tabular.textVOffset(sl.idx());
	y += offset_valign_;

	// x offset correction
	x += cellXPos(sl.idx());
	x += tabular.textHOffset(sl.idx());
	x += ADD_TO_TABULAR_WIDTH;
	x += scx_;
}


int InsetTabular::dist(BufferView & bv, idx_type const cell, int x, int y) const
{
	int xx = 0;
	int yy = 0;
	Inset const & inset = *tabular.cellInset(cell);
	Point o = bv.coordCache().getInsets().xy(&inset);
	int const xbeg = o.x_ - tabular.textHOffset(cell);
	int const xend = xbeg + tabular.cellWidth(cell);
	row_type const row = tabular.cellRow(cell);
	int const ybeg = o.y_ - tabular.rowAscent(row)
		- tabular.interRowSpace(row) - tabular.textVOffset(cell);
	int const yend = ybeg + tabular.cellHeight(cell);

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
	cur.setSelection(false);
	cur.push(*this);
	cur.idx() = getNearestCell(cur.bv(), x, y);
	resetPos(cur);
	return cur.bv().textMetrics(&cell(cur.idx())->text()).editXY(cur, x, y);
}


void InsetTabular::setCursorFromCoordinates(Cursor & cur, int x, int y) const
{
	cur.idx() = getNearestCell(cur.bv(), x, y);
	cur.bv().textMetrics(&cell(cur.idx())->text()).setCursorFromCoordinates(cur, x, y);
}


InsetTabular::idx_type InsetTabular::getNearestCell(BufferView & bv, int x, int y) const
{
	idx_type idx_min = 0;
	int dist_min = numeric_limits<int>::max();
	for (idx_type i = 0, n = nargs(); i != n; ++i) {
		if (bv.coordCache().getInsets().has(tabular.cellInset(i).get())) {
			int const d = dist(bv, i, x, y);
			if (d < dist_min) {
				dist_min = d;
				idx_min = i;
			}
		}
	}
	return idx_min;
}


int InsetTabular::cellYPos(idx_type const cell) const
{
	row_type row = tabular.cellRow(cell);
	int ly = 0;
	for (row_type r = 0; r < row; ++r)
		ly += tabular.rowDescent(r) + tabular.rowAscent(r + 1) 
			+ tabular.interRowSpace(r + 1);
	return ly;
}


int InsetTabular::cellXPos(idx_type const cell) const
{
	col_type col = tabular.cellColumn(cell);
	int lx = 0;
	for (col_type c = 0; c < col; ++c)
		lx += tabular.column_info[c].width;
	return lx;
}


void InsetTabular::resetPos(Cursor & cur) const
{
	BufferView & bv = cur.bv();
	int const maxwidth = bv.workWidth();

	int const scx_old = scx_;
	int const i = cur.find(this);
	if (i == -1) {
		scx_ = 0;
	} else {
		int const X1 = 0;
		int const X2 = maxwidth;
		int const offset = ADD_TO_TABULAR_WIDTH + 2;
		int const x1 = xo(cur.bv()) + cellXPos(cur[i].idx()) + offset;
		int const x2 = x1 + tabular.cellWidth(cur[i].idx());

		if (x1 < X1)
			scx_ = X1 + 20 - x1;
		else if (x2 > X2)
			scx_ = X2 - 20 - x2;
		else
			scx_ = 0;
	}

	// only update if offset changed
	if (scx_ != scx_old)
		cur.screenUpdateFlags(Update::Force | Update::FitCursor);
}


void InsetTabular::moveNextCell(Cursor & cur, EntryDirection entry_from)
{
	row_type const row = tabular.cellRow(cur.idx());
	col_type const col = tabular.cellColumn(cur.idx());

	if (isRightToLeft(cur)) {
		if (tabular.cellColumn(cur.idx()) == 0) {
			if (row == tabular.nrows() - 1)
				return;
			cur.idx() = tabular.cellBelow(tabular.getLastCellInRow(row));
		} else {
			if (cur.idx() == 0)
				return;
			if (col == 0)
				cur.idx() = tabular.getLastCellInRow(row - 1);
			else
				cur.idx() = tabular.cellIndex(row, col - 1);
		}
	} else {
		if (tabular.isLastCell(cur.idx()))
			return;
		if (cur.idx() == tabular.getLastCellInRow(row))
			cur.idx() = tabular.cellIndex(row + 1, 0);
		else {
			col_type const colnextcell = col + tabular.columnSpan(cur.idx());
			cur.idx() = tabular.cellIndex(row, colnextcell);
		}
	}

	cur.boundary(false);

	if (cur.selIsMultiCell()) {
		cur.pit() = cur.lastpit();
		cur.pos() = cur.lastpos();
		resetPos(cur);
		return;
	}

	cur.pit() = 0;
	cur.pos() = 0;

	// in visual mode, place cursor at extreme left or right
	
	switch(entry_from) {

	case ENTRY_DIRECTION_RIGHT:
		cur.posVisToRowExtremity(false /* !left */);
		break;
	case ENTRY_DIRECTION_LEFT:
		cur.posVisToRowExtremity(true /* left */);
		break;
	case ENTRY_DIRECTION_IGNORE:
		// nothing to do in this case
		break;

	}
	cur.setCurrentFont();
	resetPos(cur);
}


void InsetTabular::movePrevCell(Cursor & cur, EntryDirection entry_from)
{
	row_type const row = tabular.cellRow(cur.idx());
	col_type const col = tabular.cellColumn(cur.idx());

	if (isRightToLeft(cur)) {
		if (cur.idx() == tabular.getLastCellInRow(row)) {
			if (row == 0)
				return;
			cur.idx() = tabular.getFirstCellInRow(row);
			cur.idx() = tabular.cellAbove(cur.idx());
		} else {
			if (tabular.isLastCell(cur.idx()))
				return;
			if (cur.idx() == tabular.getLastCellInRow(row))
				cur.idx() = tabular.cellIndex(row + 1, 0);
			else
				cur.idx() = tabular.cellIndex(row, col + 1);
		}
	} else {
		if (cur.idx() == 0) // first cell
			return;
		if (col == 0)
			cur.idx() = tabular.getLastCellInRow(row - 1);
		else
			cur.idx() = tabular.cellIndex(row, col - 1);
	}

	if (cur.selIsMultiCell()) {
		cur.pit() = cur.lastpit();
		cur.pos() = cur.lastpos();
		resetPos(cur);
		return;
	}

	cur.pit() = cur.lastpit();
	cur.pos() = cur.lastpos();

	// in visual mode, place cursor at extreme left or right
	
	switch(entry_from) {

	case ENTRY_DIRECTION_RIGHT:
		cur.posVisToRowExtremity(false /* !left */);
		break;
	case ENTRY_DIRECTION_LEFT:
		cur.posVisToRowExtremity(true /* left */);
		break;
	case ENTRY_DIRECTION_IGNORE:
		// nothing to do in this case
		break;

	}
	cur.setCurrentFont();
	resetPos(cur);
}


bool InsetTabular::tabularFeatures(Cursor & cur, string const & argument)
{
	istringstream is(argument);
	string s;
	is >> s;
	if (insetCode(s) != TABULAR_CODE)
		return false;

	// Safe guard.
	size_t safe_guard = 0;
	for (;;) {
		if (is.eof())
			break;
		safe_guard++;
		if (safe_guard > 1000) {
			LYXERR0("parameter max count reached!");
			break;
		}
		is >> s;
		Tabular::Feature action = Tabular::LAST_ACTION;

		size_t i = 0;
		for (; tabularFeature[i].action != Tabular::LAST_ACTION; ++i) {
			if (s != tabularFeature[i].feature)
				continue;
			action = tabularFeature[i].action;
			break;
		}
		if (action == Tabular::LAST_ACTION) {
			LYXERR0("Feature not found " << s);
			continue;
		}
		string val;
		if (tabularFeature[i].need_value)
			is >> val;
		LYXERR(Debug::DEBUG, "Feature: " << s << "\t\tvalue: " << val);
		tabularFeatures(cur, action, val);
	}
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
		col_type col_start, col_type col_end) const 
{
	for (row_type r = row_start; r <= row_end; ++r)
		for (col_type c = col_start; c <= col_end; ++c)
			if (rotated) {
				if (tabular.getRotateCell(tabular.cellIndex(r, c)) != 0)
					return true;
			} else {
				if (tabular.getRotateCell(tabular.cellIndex(r, c)) == 0)
					return true;
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

	case Tabular::ALIGN_DECIMAL:
		if (tabular.column_info[tabular.cellColumn(cur.idx())].alignment == LYX_ALIGN_DECIMAL)
			setAlign = LYX_ALIGN_CENTER;
		else
			setAlign = LYX_ALIGN_DECIMAL;
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

	cur.recordUndoInset(ATOMIC_UNDO, this);

	getSelection(cur, sel_row_start, sel_row_end, sel_col_start, sel_col_end);
	row_type const row = tabular.cellRow(cur.idx());
	col_type const column = tabular.cellColumn(cur.idx());
	bool flag = true;
	Tabular::ltType ltt;

	switch (feature) {

	case Tabular::SET_TABULAR_WIDTH:
		tabular.setTabularWidth(Length(value));
		break;

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

	case Tabular::SET_MROFFSET:
		tabular.setMROffset(cur, cur.idx(), Length(value));
		break;

	case Tabular::SET_SPECIAL_COLUMN:
	case Tabular::SET_SPECIAL_MULTICOLUMN:
		if (value == "none")
			tabular.setAlignSpecial(cur.idx(), docstring(), feature);
		else
			tabular.setAlignSpecial(cur.idx(), from_utf8(value), feature);
		break;

	case Tabular::APPEND_ROW:
		// append the row into the tabular
		tabular.appendRow(row);
		break;

	case Tabular::APPEND_COLUMN:
		// append the column into the tabular
		tabular.appendColumn(column);
		cur.idx() = tabular.cellIndex(row, column);
		break;

	case Tabular::DELETE_ROW:
		if (sel_row_end == tabular.nrows() - 1 && sel_row_start != 0) {
			for (col_type c = 0; c < tabular.ncols(); c++)
				tabular.setBottomLine(tabular.cellIndex(sel_row_start - 1, c),
					tabular.bottomLine(tabular.cellIndex(sel_row_end, c)));
		}

		for (row_type r = sel_row_start; r <= sel_row_end; ++r)
			tabular.deleteRow(sel_row_start);
		if (sel_row_start >= tabular.nrows())
			--sel_row_start;
		cur.idx() = tabular.cellIndex(sel_row_start, column);
		cur.pit() = 0;
		cur.pos() = 0;
		cur.setSelection(false);
		break;

	case Tabular::DELETE_COLUMN:
		if (sel_col_end == tabular.ncols() - 1 && sel_col_start != 0) {
			for (row_type r = 0; r < tabular.nrows(); r++)
				tabular.setRightLine(tabular.cellIndex(r, sel_col_start - 1),
					tabular.rightLine(tabular.cellIndex(r, sel_col_end)));
		}

		if (sel_col_start == 0 && sel_col_end != tabular.ncols() - 1) {
			for (row_type r = 0; r < tabular.nrows(); r++)
				tabular.setLeftLine(tabular.cellIndex(r, sel_col_end + 1),
					tabular.leftLine(tabular.cellIndex(r, 0)));
		}

		for (col_type c = sel_col_start; c <= sel_col_end; ++c)
			tabular.deleteColumn(sel_col_start);
		if (sel_col_start >= tabular.ncols())
			--sel_col_start;
		cur.idx() = tabular.cellIndex(row, sel_col_start);
		cur.pit() = 0;
		cur.pos() = 0;
		cur.setSelection(false);
		break;

	case Tabular::COPY_ROW:
		tabular.copyRow(row);
		break;

	case Tabular::COPY_COLUMN:
		tabular.copyColumn(column);
		cur.idx() = tabular.cellIndex(row, column);
		break;

	case Tabular::MOVE_COLUMN_RIGHT:
		tabular.moveColumn(column, Tabular::RIGHT);
		cur.idx() = tabular.cellIndex(row, column + 1);
		break;

	case Tabular::MOVE_COLUMN_LEFT:
		tabular.moveColumn(column, Tabular::LEFT);
		cur.idx() = tabular.cellIndex(row, column - 1);
		break;

	case Tabular::MOVE_ROW_DOWN:
		tabular.moveRow(row, Tabular::DOWN);
		cur.idx() = tabular.cellIndex(row + 1, column);
		break;

	case Tabular::MOVE_ROW_UP:
		tabular.moveRow(row, Tabular::UP);
		cur.idx() = tabular.cellIndex(row - 1, column);
		break;

	case Tabular::SET_LINE_TOP:
	case Tabular::TOGGLE_LINE_TOP: {
		bool lineSet = (feature == Tabular::SET_LINE_TOP)
			       ? (value == "true") : !tabular.topLine(cur.idx());
		for (row_type r = sel_row_start; r <= sel_row_end; ++r)
			for (col_type c = sel_col_start; c <= sel_col_end; ++c)
				tabular.setTopLine(tabular.cellIndex(r, c), lineSet);
		break;
	}

	case Tabular::SET_LINE_BOTTOM:
	case Tabular::TOGGLE_LINE_BOTTOM: {
		bool lineSet = (feature == Tabular::SET_LINE_BOTTOM)
			       ? (value == "true") : !tabular.bottomLine(cur.idx());
		for (row_type r = sel_row_start; r <= sel_row_end; ++r)
			for (col_type c = sel_col_start; c <= sel_col_end; ++c)
				tabular.setBottomLine(tabular.cellIndex(r, c), lineSet);
		break;
	}

	case Tabular::SET_LINE_LEFT:
	case Tabular::TOGGLE_LINE_LEFT: {
		bool lineSet = (feature == Tabular::SET_LINE_LEFT)
			       ? (value == "true") : !tabular.leftLine(cur.idx());
		for (row_type r = sel_row_start; r <= sel_row_end; ++r)
			for (col_type c = sel_col_start; c <= sel_col_end; ++c)
				tabular.setLeftLine(tabular.cellIndex(r, c), lineSet);
		break;
	}

	case Tabular::SET_LINE_RIGHT:
	case Tabular::TOGGLE_LINE_RIGHT: {
		bool lineSet = (feature == Tabular::SET_LINE_RIGHT)
			       ? (value == "true") : !tabular.rightLine(cur.idx());
		for (row_type r = sel_row_start; r <= sel_row_end; ++r)
			for (col_type c = sel_col_start; c <= sel_col_end; ++c)
				tabular.setRightLine(tabular.cellIndex(r, c), lineSet);
		break;
	}

	case Tabular::M_ALIGN_LEFT:
	case Tabular::M_ALIGN_RIGHT:
	case Tabular::M_ALIGN_CENTER:
	case Tabular::ALIGN_LEFT:
	case Tabular::ALIGN_RIGHT:
	case Tabular::ALIGN_CENTER:
	case Tabular::ALIGN_BLOCK:
	case Tabular::ALIGN_DECIMAL:
		for (row_type r = sel_row_start; r <= sel_row_end; ++r)
			for (col_type c = sel_col_start; c <= sel_col_end; ++c)
				tabular.setAlignment(tabular.cellIndex(r, c), setAlign,
				!tabular.getPWidth(c).zero());
		break;

	case Tabular::M_VALIGN_TOP:
	case Tabular::M_VALIGN_BOTTOM:
	case Tabular::M_VALIGN_MIDDLE:
		flag = false;
	case Tabular::VALIGN_TOP:
	case Tabular::VALIGN_BOTTOM:
	case Tabular::VALIGN_MIDDLE:
		for (row_type r = sel_row_start; r <= sel_row_end; ++r)
			for (col_type c = sel_col_start; c <= sel_col_end; ++c)
				tabular.setVAlignment(tabular.cellIndex(r, c), setVAlign, flag);
		break;

	case Tabular::SET_MULTICOLUMN: {
		if (!cur.selection()) {
			// just multicol for one single cell
			// check whether we are completely in a multicol
			if (!tabular.isMultiColumn(cur.idx()))
				tabular.setMultiColumn(cur.idx(), 1,
					tabular.rightLine(cur.idx()));
			break;
		}
		// we have a selection so this means we just add all these
		// cells to form a multicolumn cell
		idx_type const s_start = cur.selBegin().idx();
		row_type const col_start = tabular.cellColumn(s_start);
		row_type const col_end = tabular.cellColumn(cur.selEnd().idx());
		cur.idx() = tabular.setMultiColumn(s_start, col_end - col_start + 1,
						   tabular.rightLine(cur.selEnd().idx()));
		cur.pit() = 0;
		cur.pos() = 0;
		cur.setSelection(false);
		break;
	}

	case Tabular::UNSET_MULTICOLUMN: {
		if (!cur.selection()) {
			if (tabular.isMultiColumn(cur.idx()))
				tabular.unsetMultiColumn(cur.idx());
		}
		break;
	}

	case Tabular::MULTICOLUMN: {
		if (!cur.selection()) {
			if (tabular.isMultiColumn(cur.idx()))
				tabularFeatures(cur, Tabular::UNSET_MULTICOLUMN);
			else
				tabularFeatures(cur, Tabular::SET_MULTICOLUMN);
			break;
		}
		bool merge = false;
		for (col_type c = sel_col_start; c <= sel_col_end; ++c) {
			row_type const r = sel_row_start;
			if (!tabular.isMultiColumn(tabular.cellIndex(r, c))
			    || (r > sel_row_start && !tabular.isPartOfMultiColumn(r, c)))
				merge = true;
		}
		// If the selection contains at least one singlecol cell
		// or multiple multicol cells,
		// we assume the user will merge is to a single multicol
		if (merge)
			tabularFeatures(cur, Tabular::SET_MULTICOLUMN);
		else
			tabularFeatures(cur, Tabular::UNSET_MULTICOLUMN);
		break;
	}

	case Tabular::SET_MULTIROW: {
		if (!cur.selection()) {
			// just multirow for one single cell
			// check whether we are completely in a multirow
			if (!tabular.isMultiRow(cur.idx()))
				tabular.setMultiRow(cur.idx(), 1,
						    tabular.bottomLine(cur.idx()),
						    tabular.getAlignment(cur.idx()));
			break;
		}
		// we have a selection so this means we just add all this
		// cells to form a multirow cell
		idx_type const s_start = cur.selBegin().idx();
		row_type const row_start = tabular.cellRow(s_start);
		row_type const row_end = tabular.cellRow(cur.selEnd().idx());
		cur.idx() = tabular.setMultiRow(s_start, row_end - row_start + 1,
						tabular.bottomLine(cur.selEnd().idx()),
						tabular.getAlignment(cur.selEnd().idx()));
		cur.pit() = 0;
		cur.pos() = 0;
		cur.setSelection(false);
		break;
	}

	case Tabular::UNSET_MULTIROW: {
		if (!cur.selection()) {
			if (tabular.isMultiRow(cur.idx()))
				tabular.unsetMultiRow(cur.idx());
		}
		break;
	}

	case Tabular::MULTIROW: {
		if (!cur.selection()) {
			if (tabular.isMultiRow(cur.idx()))
				tabularFeatures(cur, Tabular::UNSET_MULTIROW);
			else
				tabularFeatures(cur, Tabular::SET_MULTIROW);
			break;
		}
		bool merge = false;
		for (row_type r = sel_row_start; r <= sel_row_end; ++r) {
			col_type const c = sel_col_start;
			if (!tabular.isMultiRow(tabular.cellIndex(r, c))
			    || (r > sel_row_start && !tabular.isPartOfMultiRow(r, c)))
				merge = true;
		}
		// If the selection contains at least one singlerow cell
		// or multiple multirow cells,
		// we assume the user will merge is to a single multirow
		if (merge)
			tabularFeatures(cur, Tabular::SET_MULTIROW);
		else
			tabularFeatures(cur, Tabular::UNSET_MULTIROW);
		break;
	}

	case Tabular::SET_ALL_LINES:
		setLines = true;
	case Tabular::UNSET_ALL_LINES:
		for (row_type r = sel_row_start; r <= sel_row_end; ++r)
			for (col_type c = sel_col_start; c <= sel_col_end; ++c) {
				idx_type const cell = tabular.cellIndex(r, c);
				tabular.setTopLine(cell, setLines);
				tabular.setBottomLine(cell, setLines);
				tabular.setRightLine(cell, setLines);
				tabular.setLeftLine(cell, setLines);
			}
		break;

	case Tabular::SET_BORDER_LINES:
		for (row_type r = sel_row_start; r <= sel_row_end; ++r) {
			tabular.setLeftLine(tabular.cellIndex(r, sel_col_start), true);
			tabular.setRightLine(tabular.cellIndex(r, sel_col_end), true);
		}
		for (col_type c = sel_col_start; c <= sel_col_end; ++c) {
			tabular.setTopLine(tabular.cellIndex(sel_row_start, c), true);
			tabular.setBottomLine(tabular.cellIndex(sel_row_end, c), true);
		}
		break;

	case Tabular::SET_LONGTABULAR:
		tabular.is_long_tabular = true;
		break;

	case Tabular::UNSET_LONGTABULAR:
		for (row_type r = 0; r < tabular.nrows(); ++r) {
			if (tabular.ltCaption(r)) {
				cur.idx() = tabular.cellIndex(r, 0);
				cur.pit() = 0;
				cur.pos() = 0;
				tabularFeatures(cur, Tabular::TOGGLE_LTCAPTION);
			}
		}
		tabular.is_long_tabular = false;
		break;

	case Tabular::SET_ROTATE_TABULAR:
		tabular.rotate = convert<int>(value);
		break;

	case Tabular::UNSET_ROTATE_TABULAR:
		tabular.rotate = 0;
		break;

	case Tabular::TOGGLE_ROTATE_TABULAR:
		// when pressing the rotate button we default to 90° rotation
		tabular.rotate != 0 ? tabular.rotate = 0 : tabular.rotate = 90;
		break;

	case Tabular::TABULAR_VALIGN_TOP:
		tabular.tabular_valignment = Tabular::LYX_VALIGN_TOP;
		break;

	case Tabular::TABULAR_VALIGN_MIDDLE:
		tabular.tabular_valignment = Tabular::LYX_VALIGN_MIDDLE;
		break;

	case Tabular::TABULAR_VALIGN_BOTTOM:
		tabular.tabular_valignment = Tabular::LYX_VALIGN_BOTTOM;
		break;

	case Tabular::LONGTABULAR_ALIGN_LEFT:
		tabular.longtabular_alignment = Tabular::LYX_LONGTABULAR_ALIGN_LEFT;
		break;

	case Tabular::LONGTABULAR_ALIGN_CENTER:
		tabular.longtabular_alignment = Tabular::LYX_LONGTABULAR_ALIGN_CENTER;
		break;

	case Tabular::LONGTABULAR_ALIGN_RIGHT:
		tabular.longtabular_alignment = Tabular::LYX_LONGTABULAR_ALIGN_RIGHT;
		break;

		

	case Tabular::SET_ROTATE_CELL:
		for (row_type r = sel_row_start; r <= sel_row_end; ++r)
			for (col_type c = sel_col_start; c <= sel_col_end; ++c)
				tabular.setRotateCell(tabular.cellIndex(r, c), convert<int>(value));
		break;

	case Tabular::UNSET_ROTATE_CELL:
		for (row_type r = sel_row_start; r <= sel_row_end; ++r)
			for (col_type c = sel_col_start; c <= sel_col_end; ++c)
				tabular.setRotateCell(tabular.cellIndex(r, c), 0);
		break;

	case Tabular::TOGGLE_ROTATE_CELL:
		{
		bool oneNotRotated = oneCellHasRotationState(false,
			sel_row_start, sel_row_end, sel_col_start, sel_col_end);

		for (row_type r = sel_row_start; r <= sel_row_end; ++r)
			for (col_type c = sel_col_start; c <= sel_col_end; ++c) {
				// when pressing the rotate cell button we default to 90° rotation
				if (oneNotRotated)
					tabular.setRotateCell(tabular.cellIndex(r, c), 90);
				else
					tabular.setRotateCell(tabular.cellIndex(r, c), 0);
			}
		}
		break;

	case Tabular::SET_USEBOX: {
		Tabular::BoxType val = Tabular::BoxType(convert<int>(value));
		if (val == tabular.getUsebox(cur.idx()))
			val = Tabular::BOX_NONE;
		for (row_type r = sel_row_start; r <= sel_row_end; ++r)
			for (col_type c = sel_col_start; c <= sel_col_end; ++c)
				tabular.setUsebox(tabular.cellIndex(r, c), val);
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

	case Tabular::UNSET_LTNEWPAGE:
		flag = false;
	case Tabular::SET_LTNEWPAGE:
		tabular.setLTNewPage(row, flag);
		break;

	case Tabular::SET_LTCAPTION: {
		if (tabular.ltCaption(row))
			break;
		cur.idx() = tabular.setLTCaption(row, true);
		cur.pit() = 0;
		cur.pos() = 0;
		cur.setSelection(false);
		// If a row is set as caption, then also insert
		// a caption. Otherwise the LaTeX output is broken.
		lyx::dispatch(FuncRequest(LFUN_INSET_SELECT_ALL));
		lyx::dispatch(FuncRequest(LFUN_CAPTION_INSERT));
		break;
	}
	
	case Tabular::UNSET_LTCAPTION: {
		if (!tabular.ltCaption(row))
			break;
		cur.idx() = tabular.setLTCaption(row, false);
		cur.pit() = 0;
		cur.pos() = 0;
		cur.setSelection(false);
		FuncRequest fr(LFUN_INSET_DISSOLVE, "caption");
		if (lyx::getStatus(fr).enabled())
			lyx::dispatch(fr);
		break;
	}

	case Tabular::TOGGLE_LTCAPTION: {
		if (tabular.ltCaption(row))
			tabularFeatures(cur, Tabular::UNSET_LTCAPTION);
		else
			tabularFeatures(cur, Tabular::SET_LTCAPTION);
		break;
	}

	case Tabular::SET_BOOKTABS:
		tabular.use_booktabs = true;
		break;

	case Tabular::UNSET_BOOKTABS:
		tabular.use_booktabs = false;
		break;

	case Tabular::SET_TOP_SPACE: {
		Length len;
		if (value == "default")
			for (row_type r = sel_row_start; r <= sel_row_end; ++r)
				tabular.row_info[r].top_space_default = true;
		else if (value == "none")
			for (row_type r = sel_row_start; r <= sel_row_end; ++r) {
				tabular.row_info[r].top_space_default = false;
				tabular.row_info[r].top_space = len;
			}
		else if (isValidLength(value, &len))
			for (row_type r = sel_row_start; r <= sel_row_end; ++r) {
				tabular.row_info[r].top_space_default = false;
				tabular.row_info[r].top_space = len;
			}
		break;
	}

	case Tabular::SET_BOTTOM_SPACE: {
		Length len;
		if (value == "default")
			for (row_type r = sel_row_start; r <= sel_row_end; ++r)
				tabular.row_info[r].bottom_space_default = true;
		else if (value == "none")
			for (row_type r = sel_row_start; r <= sel_row_end; ++r) {
				tabular.row_info[r].bottom_space_default = false;
				tabular.row_info[r].bottom_space = len;
			}
		else if (isValidLength(value, &len))
			for (row_type r = sel_row_start; r <= sel_row_end; ++r) {
				tabular.row_info[r].bottom_space_default = false;
				tabular.row_info[r].bottom_space = len;
			}
		break;
	}

	case Tabular::SET_INTERLINE_SPACE: {
		Length len;
		if (value == "default")
			for (row_type r = sel_row_start; r <= sel_row_end; ++r)
				tabular.row_info[r].interline_space_default = true;
		else if (value == "none")
			for (row_type r = sel_row_start; r <= sel_row_end; ++r) {
				tabular.row_info[r].interline_space_default = false;
				tabular.row_info[r].interline_space = len;
			}
		else if (isValidLength(value, &len))
			for (row_type r = sel_row_start; r <= sel_row_end; ++r) {
				tabular.row_info[r].interline_space_default = false;
				tabular.row_info[r].interline_space = len;
			}
		break;
	}

	case Tabular::SET_DECIMAL_POINT:
		for (col_type c = sel_col_start; c <= sel_col_end; ++c)
			tabular.column_info[c].decimal_point = from_utf8(value);
		break;

	// dummy stuff just to avoid warnings
	case Tabular::LAST_ACTION:
		break;
	}
}


bool InsetTabular::copySelection(Cursor & cur)
{
	if (!cur.selection())
		return false;

	row_type rs, re;
	col_type cs, ce;
	getSelection(cur, rs, re, cs, ce);

	paste_tabular.reset(new Tabular(tabular));

	for (row_type r = 0; r < rs; ++r)
		paste_tabular->deleteRow(0);

	row_type const rows = re - rs + 1;
	while (paste_tabular->nrows() > rows)
		paste_tabular->deleteRow(rows);

	for (col_type c = 0; c < cs; ++c)
		paste_tabular->deleteColumn(0);

	col_type const columns = ce - cs + 1;
	while (paste_tabular->ncols() > columns)
		paste_tabular->deleteColumn(columns);

	paste_tabular->setBuffer(tabular.buffer());

	odocstringstream os;
	OutputParams const runparams(0);
	paste_tabular->plaintext(os, runparams, 0, true, '\t', INT_MAX);
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
	col_type actcol = tabular.cellColumn(cur.idx());
	row_type actrow = tabular.cellRow(cur.idx());

	if (cur.selIsMultiCell()) {
		row_type re;
		col_type ce;
		getSelection(cur, actrow, re, actcol, ce);
	}

	for (row_type r1 = 0, r2 = actrow;
	     r1 < paste_tabular->nrows() && r2 < tabular.nrows();
	     ++r1, ++r2) {
		for (col_type c1 = 0, c2 = actcol;
		    c1 < paste_tabular->ncols() && c2 < tabular.ncols();
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
				new InsetTableCell(*paste_tabular->cellInset(r1, c1)));
			tabular.setCellInset(r2, c2, inset);
			// FIXME?: why do we need to do this explicitly? (EL)
			tabular.cellInset(r2, c2)->setBuffer(tabular.buffer());

			// FIXME: change tracking (MG)
			inset->setChange(Change(buffer().params().trackChanges ?
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
	for (row_type r = rs; r <= re; ++r) {
		for (col_type c = cs; c <= ce; ++c) {
			shared_ptr<InsetTableCell> t
				= cell(tabular.cellIndex(r, c));
			if (buffer().params().trackChanges)
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
	// LASSERT: It might be better to abandon this Buffer.
	LASSERT(cur.depth() > 1, return false);
	Paragraph const & parentpar = cur[cur.depth() - 2].paragraph();
	pos_type const parentpos = cur[cur.depth() - 2].pos();
	return parentpar.getFontSettings(buffer().params(),
					 parentpos).language()->rightToLeft();
}


docstring InsetTabular::asString(idx_type stidx, idx_type enidx, 
                                 bool intoInsets)
{
	LASSERT(stidx <= enidx, return docstring());
	docstring retval;
	col_type const col1 = tabular.cellColumn(stidx);
	col_type const col2 = tabular.cellColumn(enidx);
	row_type const row1 = tabular.cellRow(stidx);
	row_type const row2 = tabular.cellRow(enidx);
	bool first = true;
	for (col_type col = col1; col <= col2; col++)
		for (row_type row = row1; row <= row2; row++) {
			if (!first)
				retval += "\n";
			else
				first = false;
			retval += tabular.cellInset(row, col)->asString(intoInsets);
		}
	return retval;
}


void InsetTabular::getSelection(Cursor & cur,
	row_type & rs, row_type & re, col_type & cs, col_type & ce) const
{
	CursorSlice const & beg = cur.selBegin();
	CursorSlice const & end = cur.selEnd();
	cs = tabular.cellColumn(beg.idx());
	ce = tabular.cellColumn(end.idx());
	if (cs > ce)
		swap(cs, ce);

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


void InsetTabular::acceptChanges()
{
	for (idx_type idx = 0; idx < nargs(); ++idx)
		cell(idx)->acceptChanges();
}


void InsetTabular::rejectChanges()
{
	for (idx_type idx = 0; idx < nargs(); ++idx)
		cell(idx)->rejectChanges();
}


bool InsetTabular::allowParagraphCustomization(idx_type cell) const
{
	return tabular.getPWidth(cell).zero();
}


bool InsetTabular::forcePlainLayout(idx_type cell) const
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
		paste_tabular.reset(new Tabular(buffer_, rows, maxCols));
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
	idx_type const cells = loctab->numberofcells;
	p = 0;
	cols = ocol;
	rows = loctab->nrows();
	col_type const columns = loctab->ncols();

	while (cell < cells && p < len && row < rows &&
	       (p = buf.find_first_of(from_ascii("\t\n"), p)) != docstring::npos)
	{
		if (p >= len)
			break;
		switch (buf[p]) {
		case '\t':
			// we can only set this if we are not too far right
			if (cols < columns) {
				shared_ptr<InsetTableCell> inset = loctab->cellInset(cell);
				Font const font = bv.textMetrics(&inset->text()).
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
				shared_ptr<InsetTableCell> inset = tabular.cellInset(cell);
				Font const font = bv.textMetrics(&inset->text()).
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
		shared_ptr<InsetTableCell> inset = loctab->cellInset(cell);
		Font const font = bv.textMetrics(&inset->text()).displayFont(0, 0);
		inset->setText(buf.substr(op, len - op), font,
			buffer().params().trackChanges);
	}
	return true;
}


void InsetTabular::addPreview(DocIterator const & inset_pos,
	PreviewLoader & loader) const
{
	DocIterator cell_pos = inset_pos;

	cell_pos.push_back(CursorSlice(*const_cast<InsetTabular *>(this)));
	for (row_type r = 0; r < tabular.nrows(); ++r) {
		for (col_type c = 0; c < tabular.ncols(); ++c) {
			cell_pos.top().idx() = tabular.cellIndex(r, c);
			tabular.cellInset(r, c)->addPreview(cell_pos, loader);
		}
	}
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


void InsetTabular::string2params(string const & in, InsetTabular & inset)
{
	istringstream data(in);
	Lexer lex;
	lex.setStream(data);

	if (in.empty())
		return;

	string token;
	lex >> token;
	if (!lex || token != "tabular") {
		LYXERR0("Expected arg 1 to be \"tabular\" in " << in);
		return;
	}

	// This is part of the inset proper that is usually swallowed
	// by Buffer::readInset
	lex >> token;
	if (!lex || token != "Tabular") {
		LYXERR0("Expected arg 2 to be \"Tabular\" in " << in);
		return;
	}

	inset.read(lex);
}


string InsetTabular::params2string(InsetTabular const & inset)
{
	ostringstream data;
	data << "tabular" << ' ';
	inset.write(data);
	data << "\\end_inset\n";
	return data.str();
}


void InsetTabular::setLayoutForHiddenCells(DocumentClass const & dc)
{
	for (Tabular::col_type c = 0; c < tabular.ncols(); ++c) {
		for (Tabular::row_type r = 0; r < tabular.nrows(); ++r) {
			if (!tabular.isPartOfMultiColumn(r,c) &&
			    !tabular.isPartOfMultiRow(r,c))
				continue;

			ParagraphList & parlist = tabular.cellInset(r,c)->paragraphs();
			ParagraphList::iterator it = parlist.begin();
			ParagraphList::iterator const en = parlist.end();
			for (; it != en; ++it)
					it->setLayout(dc.plainLayout());
		}
	}
}


} // namespace lyx
