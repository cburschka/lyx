/**
 * \file insettabular.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insettabular.h"

#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "language.h"
#include "LColor.h"
#include "lyx_cb.h"
#include "lyxlex.h"
#include "metricsinfo.h"
#include "outputparams.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "undo.h"

#include "frontends/Alert.h"
#include "frontends/font_metrics.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"

#include "support/std_sstream.h"

#include <iostream>

using lyx::graphics::PreviewLoader;

using lyx::support::ltrim;
using lyx::support::strToInt;
using lyx::support::strToDbl;

using std::endl;
using std::max;
using std::swap;
using std::string;
using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;


namespace {

int const ADD_TO_HEIGHT = 2;
int const ADD_TO_TABULAR_WIDTH = 2;

///
boost::scoped_ptr<LyXTabular> paste_tabular;


struct TabularFeature {
	LyXTabular::Feature action;
	string feature;
};


TabularFeature tabularFeature[] =
{
	{ LyXTabular::APPEND_ROW, "append-row" },
	{ LyXTabular::APPEND_COLUMN, "append-column" },
	{ LyXTabular::DELETE_ROW, "delete-row" },
	{ LyXTabular::DELETE_COLUMN, "delete-column" },
	{ LyXTabular::TOGGLE_LINE_TOP, "toggle-line-top" },
	{ LyXTabular::TOGGLE_LINE_BOTTOM, "toggle-line-bottom" },
	{ LyXTabular::TOGGLE_LINE_LEFT, "toggle-line-left" },
	{ LyXTabular::TOGGLE_LINE_RIGHT, "toggle-line-right" },
	{ LyXTabular::ALIGN_LEFT, "align-left" },
	{ LyXTabular::ALIGN_RIGHT, "align-right" },
	{ LyXTabular::ALIGN_CENTER, "align-center" },
	{ LyXTabular::ALIGN_BLOCK, "align-block" },
	{ LyXTabular::VALIGN_TOP, "valign-top" },
	{ LyXTabular::VALIGN_BOTTOM, "valign-bottom" },
	{ LyXTabular::VALIGN_MIDDLE, "valign-middle" },
	{ LyXTabular::M_TOGGLE_LINE_TOP, "m-toggle-line-top" },
	{ LyXTabular::M_TOGGLE_LINE_BOTTOM, "m-toggle-line-bottom" },
	{ LyXTabular::M_TOGGLE_LINE_LEFT, "m-toggle-line-left" },
	{ LyXTabular::M_TOGGLE_LINE_RIGHT, "m-toggle-line-right" },
	{ LyXTabular::M_ALIGN_LEFT, "m-align-left" },
	{ LyXTabular::M_ALIGN_RIGHT, "m-align-right" },
	{ LyXTabular::M_ALIGN_CENTER, "m-align-center" },
	{ LyXTabular::M_VALIGN_TOP, "m-valign-top" },
	{ LyXTabular::M_VALIGN_BOTTOM, "m-valign-bottom" },
	{ LyXTabular::M_VALIGN_MIDDLE, "m-valign-middle" },
	{ LyXTabular::MULTICOLUMN, "multicolumn" },
	{ LyXTabular::SET_ALL_LINES, "set-all-lines" },
	{ LyXTabular::UNSET_ALL_LINES, "unset-all-lines" },
	{ LyXTabular::SET_LONGTABULAR, "set-longtabular" },
	{ LyXTabular::UNSET_LONGTABULAR, "unset-longtabular" },
	{ LyXTabular::SET_PWIDTH, "set-pwidth" },
	{ LyXTabular::SET_MPWIDTH, "set-mpwidth" },
	{ LyXTabular::SET_ROTATE_TABULAR, "set-rotate-tabular" },
	{ LyXTabular::UNSET_ROTATE_TABULAR, "unset-rotate-tabular" },
	{ LyXTabular::SET_ROTATE_CELL, "set-rotate-cell" },
	{ LyXTabular::UNSET_ROTATE_CELL, "unset-rotate-cell" },
	{ LyXTabular::SET_USEBOX, "set-usebox" },
	{ LyXTabular::SET_LTHEAD, "set-lthead" },
	{ LyXTabular::SET_LTFIRSTHEAD, "set-ltfirsthead" },
	{ LyXTabular::SET_LTFOOT, "set-ltfoot" },
	{ LyXTabular::SET_LTLASTFOOT, "set-ltlastfoot" },
	{ LyXTabular::SET_LTNEWPAGE, "set-ltnewpage" },
	{ LyXTabular::SET_SPECIAL_COLUMN, "set-special-column" },
	{ LyXTabular::SET_SPECIAL_MULTI, "set-special-multi" },
	{ LyXTabular::LAST_ACTION, "" }
};


class FeatureEqual : public std::unary_function<TabularFeature, bool> {
public:
	FeatureEqual(LyXTabular::Feature feature)
		: feature_(feature) {}
	bool operator()(TabularFeature const & tf) const {
		return tf.action == feature_;
	}
private:
	LyXTabular::Feature feature_;
};

} // namespace anon


string const featureAsString(LyXTabular::Feature feature)
{
	TabularFeature * end = tabularFeature +
		sizeof(tabularFeature) / sizeof(TabularFeature);
	TabularFeature * it = std::find_if(tabularFeature, end,
					   FeatureEqual(feature));
	return (it == end) ? string() : it->feature;
}


bool InsetTabular::hasPasteBuffer() const
{
	return (paste_tabular.get() != 0);
}


InsetTabular::InsetTabular(Buffer const & buf, int rows, int columns)
	: tabular(buf.params(), max(rows, 1), max(columns, 1)),
	  buffer_(&buf), cursorx_(0), cursory_(0), tablemode(false)
{
	tabular.setOwner(this);
	clearSelection();
	in_reset_pos = 0;
}


InsetTabular::InsetTabular(InsetTabular const & tab)
	: UpdatableInset(tab), tabular(tab.tabular),
		buffer_(tab.buffer_), cursorx_(0), cursory_(0), tablemode(false)
{
	tabular.setOwner(this);
	clearSelection();
	in_reset_pos = 0;
}


InsetTabular::~InsetTabular()
{
	InsetTabularMailer(*this).hideDialog();
}


auto_ptr<InsetBase> InsetTabular::clone() const
{
	return auto_ptr<InsetBase>(new InsetTabular(*this));
}


Buffer const & InsetTabular::buffer() const
{
	return *buffer_;
}


void InsetTabular::buffer(Buffer * b)
{
	buffer_ = b;
}


void InsetTabular::write(Buffer const & buf, ostream & os) const
{
	os << "Tabular" << endl;
	tabular.write(buf, os);
}


void InsetTabular::read(Buffer const & buf, LyXLex & lex)
{
	bool const old_format = (lex.getString() == "\\LyXTable");

	tabular.read(buf, lex);

	if (old_format)
		return;

	lex.nextToken();
	string token = lex.getString();
	while (lex.isOK() && (token != "\\end_inset")) {
		lex.nextToken();
		token = lex.getString();
	}
	if (token != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point. "
			       "Read: `$$Token'");
	}
}


void InsetTabular::metrics(MetricsInfo & mi, Dimension & dim) const
{
	//lyxerr << "InsetTabular::metrics: " << mi.base.bv << " width: " <<
	//	mi.base.textwidth << "\n";
	if (!mi.base.bv) {
		lyxerr << "InsetTabular::metrics: need bv" << endl;
		BOOST_ASSERT(false);
	}

	calculate_dimensions_of_cells(mi);

	dim.asc = tabular.getAscentOfRow(0);
	dim.des = tabular.getHeightOfTabular() - tabular.getAscentOfRow(0) + 1;
	dim.wid = tabular.getWidthOfTabular() + 2 * ADD_TO_TABULAR_WIDTH;
	dim_ = dim;
}


void InsetTabular::draw(PainterInfo & pi, int x, int y) const
{
	//lyxerr << "InsetTabular::draw: " << x << " " << y << endl;

	BufferView * bv = pi.base.bv;

	if (!owner())
		x += scroll();

	xo_ = x;
	yo_ = y;
	x += ADD_TO_TABULAR_WIDTH;

	int cell = 0;
	first_visible_cell = -1;
	for (int i = 0; i < tabular.rows(); ++i) {
		int nx = x;
		cell = tabular.getCellNumber(i, 0);
		if (y + tabular.getDescentOfRow(i) <= 0 &&
			  y - tabular.getAscentOfRow(i) < pi.pain.paperHeight())
		{
			y += tabular.getDescentOfRow(i) +
					tabular.getAscentOfRow(i + 1) +
					tabular.getAdditionalHeight(i + 1);
			continue;
		}
		for (int j = 0; j < tabular.columns(); ++j) {
			if (nx > bv->workWidth())
				break;
			if (tabular.isPartOfMultiColumn(i, j))
				continue;
			if (first_visible_cell < 0)
				first_visible_cell = cell;
			if (hasSelection())
				drawCellSelection(pi.pain, nx, y, i, j, cell);

			int const cx = nx + tabular.getBeginningOfTextInCell(cell);
			tabular.getCellInset(cell).draw(pi, cx, y);
			drawCellLines(pi.pain, nx, y, i, cell);
			nx += tabular.getWidthOfColumn(cell);
			++cell;
		}

// Would be nice, but for some completely unfathomable reason,
// on a col resize to a new fixed width, even though the insettexts
// are resized, the cell isn't, but drawing all cells in a tall table
// has the desired effect somehow. Complete dark magic.
#if 0
		// avoiding drawing the rest of a long table is
		// a pretty big speedup
		if (y > bv->workHeight())
			break;
#endif

		y += tabular.getDescentOfRow(i) +
			tabular.getAscentOfRow(i + 1) +
			tabular.getAdditionalHeight(i + 1);
	}
}


void InsetTabular::drawCellLines(Painter & pain, int x, int y,
				 int row, int cell) const
{
	int x2 = x + tabular.getWidthOfColumn(cell);
	bool on_off = false;

	if (!tabular.topAlreadyDrawn(cell)) {
		on_off = !tabular.topLine(cell);
		pain.line(x, y - tabular.getAscentOfRow(row),
			  x2, y -  tabular.getAscentOfRow(row),
			  on_off ? LColor::tabularonoffline : LColor::tabularline,
			  on_off ? Painter::line_onoffdash : Painter::line_solid);
	}
	on_off = !tabular.bottomLine(cell);
	pain.line(x, y + tabular.getDescentOfRow(row),
		  x2, y + tabular.getDescentOfRow(row),
		  on_off ? LColor::tabularonoffline : LColor::tabularline,
		  on_off ? Painter::line_onoffdash : Painter::line_solid);
	if (!tabular.leftAlreadyDrawn(cell)) {
		on_off = !tabular.leftLine(cell);
		pain.line(x, y -  tabular.getAscentOfRow(row),
			  x, y +  tabular.getDescentOfRow(row),
			  on_off ? LColor::tabularonoffline : LColor::tabularline,
			  on_off ? Painter::line_onoffdash : Painter::line_solid);
	}
	on_off = !tabular.rightLine(cell);
	pain.line(x2 - tabular.getAdditionalWidth(cell),
		  y -  tabular.getAscentOfRow(row),
		  x2 - tabular.getAdditionalWidth(cell),
		  y +  tabular.getDescentOfRow(row),
		  on_off ? LColor::tabularonoffline : LColor::tabularline,
		  on_off ? Painter::line_onoffdash : Painter::line_solid);
}


void InsetTabular::drawCellSelection(Painter & pain, int x, int y,
				     int row, int column, int cell) const
{
	if (!tablemode)
		return;

	BOOST_ASSERT(hasSelection());
	int cs = tabular.column_of_cell(sel_cell_start);
	int ce = tabular.column_of_cell(sel_cell_end);
	if (cs > ce) {
		ce = cs;
		cs = tabular.column_of_cell(sel_cell_end);
	} else {
		ce = tabular.right_column_of_cell(sel_cell_end);
	}

	int rs = tabular.row_of_cell(sel_cell_start);
	int re = tabular.row_of_cell(sel_cell_end);
	if (rs > re)
		swap(rs, re);

	if (column >= cs && column <= ce && row >= rs && row <= re) {
		int w = tabular.getWidthOfColumn(cell);
		int h = tabular.getAscentOfRow(row) + tabular.getDescentOfRow(row)-1;
		pain.fillRectangle(x, y - tabular.getAscentOfRow(row) + 1,
				   w, h, LColor::selection);
	}
}


string const InsetTabular::editMessage() const
{
	return _("Opened table");
}


void InsetTabular::updateLocal(LCursor & cur) const
{
	cur.bv().update();
	resetPos(cur);
}


extern CursorBase theTempCursor;


void InsetTabular::lfunMousePress(LCursor & cur, FuncRequest const & cmd)
{
	if (hasSelection() && cmd.button() == mouse_button::button3)
		return;

	int cell = getCell(cmd.x + xo_, cmd.y + yo_);
	clearSelection();

	lyxerr << "# InsetTabular::lfunMousePress cell: " << cell << endl;
	if (cell == -1) {
		tablemode = true;
		cur.cursor_ = theTempCursor;
		cur.push(this);
		cur.idx() = cell;
	} else {
		tablemode = false;
		setPos(cur.bv(), cmd.x, cmd.y);
		cur.cursor_ = theTempCursor;
		cur.idx() = cell;
	}
	cur.resetAnchor();
	lyxerr << cur << endl;

	if (cmd.button() == mouse_button::button2)
		dispatch(cur, FuncRequest(LFUN_PASTESELECTION, "paragraph"));
}


void InsetTabular::lfunMouseMotion(LCursor & cur, FuncRequest const & cmd)
{
	int const actcell = getCell(cmd.x + xo_, cmd.y + yo_);
	lyxerr << "# InsetTabular::lfunMouseMotion cell: " << actcell << endl;

	setPos(cur.bv(), cmd.x, cmd.y);
	if (!hasSelection()) {
		setSelection(actcell, actcell);
		cur.setSelection();
	} else {
		cur.idx() = actcell;
		setSelection(sel_cell_start, actcell);
		tablemode = (sel_cell_start != actcell);
	}
}


void InsetTabular::lfunMouseRelease(LCursor & cur, FuncRequest const & cmd)
{
	int const actcell = getCell(cmd.x + xo_, cmd.y + yo_);
	lyxerr << "# InsetTabular::lfunMouseRelease cell: " << actcell << endl;
	if (cmd.button() == mouse_button::button3)
		InsetTabularMailer(*this).showDialog(&cur.bv());
}


void InsetTabular::edit(LCursor & cur, bool left)
{
	lyxerr << "InsetTabular::edit: " << this << endl;
	finishUndo();
	//tablemode = false;
	int cell;
	if (left) {
		if (isRightToLeft(cur))
			cell = tabular.getLastCellInRow(0);
		else
			cell = 0;
	} else {
		if (isRightToLeft(cur))
			cell = tabular.getFirstCellInRow(tabular.rows()-1);
		else
			cell = tabular.getNumberOfCells() - 1;
	}
	clearSelection();
	resetPos(cur);
	cur.bv().fitCursor();
	cur.push(this);
	cur.idx() = cell;
}


void InsetTabular::edit(LCursor & cur, int x, int y)
{
	lyxerr << "InsetTabular::edit: " << this << " first cell "
		<< &tabular.cell_info[0][0].inset << endl;

	finishUndo();
	setPos(cur.bv(), x, y);
	clearSelection();
	finishUndo();
	//int xx = cursorx_ - xo_ + tabular.getBeginningOfTextInCell(actcell);
	cur.push(this);
	//if (x > xx)
	//	activateCellInset(bv, cell, x - xx, y - cursory_);
}


DispatchResult
InsetTabular::priv_dispatch(LCursor & cur, FuncRequest const & cmd)
{
	lyxerr << "# InsetTabular::dispatch: " << cmd
		<< " tablemode: " << tablemode << endl;

	DispatchResult result(true, true);
	switch (cmd.action) {

	case LFUN_MOUSE_PRESS:
		lfunMousePress(cur, cmd);
		return DispatchResult(true, true);

	case LFUN_MOUSE_MOTION:
		lfunMouseMotion(cur, cmd);
		return DispatchResult(true, true);

	case LFUN_MOUSE_RELEASE:
		lfunMouseRelease(cur, cmd);
		return DispatchResult(true, true);

	default:
		break;
	}

	if (!tablemode) {
		
		int cell = cur.idx();
		lyxerr << "# InsetTabular::dispatch: A " << cur << endl;
		result = tabular.getCellInset(cell).dispatch(cur, cmd);

		switch (result.val()) {
		case FINISHED:
			if (movePrevCell(cur))
				result = DispatchResult(true, true);
			else
				result = DispatchResult(false, FINISHED);
			break;

		case FINISHED_RIGHT:
			if (moveNextCell(cur))
				result = DispatchResult(true, true);
			else
				result = DispatchResult(false, FINISHED_RIGHT);
			break;

		case FINISHED_UP:
			if (moveUpLock(cur))
				result = DispatchResult(true, true);
			else
				result = DispatchResult(false, FINISHED_UP);
			break;

		case FINISHED_DOWN:
			if (moveDownLock(cur))
				result = DispatchResult(true, true);
			else
				result = DispatchResult(false, FINISHED_UP);
			break;

		default:
			lyxerr << "# don't handle dispatch" << endl;
			break;
		}

		lyxerr << "# InsetTabular::dispatch: B " << cur << endl;
	} else {

		lyxerr << "# InsetTabular::dispatch 3: " << cmd << endl;
		switch (cmd.action) {

		case LFUN_CELL_BACKWARD:
		case LFUN_CELL_FORWARD:
			if (cmd.action == LFUN_CELL_FORWARD)
				moveNextCell(cur);
			else
				movePrevCell(cur);
			clearSelection();
			return result;

		case LFUN_SCROLL_INSET:
			if (!cmd.argument.empty()) {
				if (cmd.argument.find('.') != cmd.argument.npos)
					scroll(cur.bv(), static_cast<float>(strToDbl(cmd.argument)));
				else
					scroll(cur.bv(), strToInt(cmd.argument));
				cur.bv().update();
				return DispatchResult(true, true);
			}

		case LFUN_RIGHTSEL: {
			int const start = hasSelection() ? sel_cell_start : cur.idx();
			if (tabular.isLastCellInRow(cur.idx())) {
				setSelection(start, cur.idx());
				break;
			}

			int end = cur.idx();
			// if we are starting a selection, only select
			// the current cell at the beginning
			if (hasSelection()) {
				moveRight(cur);
				end = cur.idx();
			}
			setSelection(start, end);
			break;
		}

		case LFUN_RIGHT:
			if (!moveRightLock(cur))
				result = DispatchResult(false, FINISHED_RIGHT);
			clearSelection();
			break;

		case LFUN_LEFTSEL: {
			int const start = hasSelection() ? sel_cell_start : cur.idx();
			if (tabular.isFirstCellInRow(cur.idx())) {
				setSelection(start, cur.idx());
				break;
			}

			int end = cur.idx();
			// if we are starting a selection, only select
			// the current cell at the beginning
			if (hasSelection()) {
				moveLeft(cur);
				end = cur.idx();
			}
			setSelection(start, end);
			break;
		}

		case LFUN_LEFT:
			if (!moveLeftLock(cur))
				result = DispatchResult(false, FINISHED);
			clearSelection();
			break;

		case LFUN_DOWNSEL: {
			int const start = hasSelection() ? sel_cell_start : cur.idx();
			int const ocell = cur.idx();
			// if we are starting a selection, only select
			// the current cell at the beginning
			if (hasSelection()) {
				moveDown(cur);
				if (ocell == sel_cell_end ||
						tabular.column_of_cell(ocell) >
tabular.column_of_cell(cur.idx()))
					setSelection(start, tabular.getCellBelow(sel_cell_end));
				else
					setSelection(start, tabular.getLastCellBelow(sel_cell_end));
			} else {
				setSelection(start, start);
			}
			break;
		}

		case LFUN_DOWN:
			if (!moveDown(cur))
				result = DispatchResult(false, FINISHED_DOWN);
			clearSelection();
			break;

		case LFUN_UPSEL: {
			int const start = hasSelection() ? sel_cell_start : cur.idx();
			int const ocell = cur.idx();
			// if we are starting a selection, only select
			// the current cell at the beginning
			if (hasSelection()) {
				moveUp(cur);
				if (ocell == sel_cell_end ||
						tabular.column_of_cell(ocell) >
tabular.column_of_cell(cur.idx()))
					setSelection(start, tabular.getCellAbove(sel_cell_end));
				else
					setSelection(start, tabular.getLastCellAbove(sel_cell_end));
			} else {
				setSelection(start, start);
			}
			break;
		}

		case LFUN_UP:
			if (!moveUp(cur))
				result = DispatchResult(false, FINISHED_DOWN);
			clearSelection();
			break;

		case LFUN_NEXT: {
			if (hasSelection())
				clearSelection();
			int actcell = cur.idx();
			int actcol = tabular.column_of_cell(actcell);
			int column = actcol;
			if (cur.bv().top_y() + cur.bv().painter().paperHeight()
					< yo_ + tabular.getHeightOfTabular())
			{
				cur.bv().scrollDocView(
					cur.bv().top_y() + cur.bv().painter().paperHeight());
				cur.idx() = tabular.getCellBelow(first_visible_cell) + column;
			} else {
				cur.idx() = tabular.getFirstCellInRow(tabular.rows() - 1) + column;
			}
			resetPos(cur);
			break;
		}

		case LFUN_PRIOR: {
			if (hasSelection())
				clearSelection();
			int column = tabular.column_of_cell(cur.idx());
			if (yo_ < 0) {
				cur.bv().scrollDocView(	
					cur.bv().top_y() - cur.bv().painter().paperHeight());
				if (yo_ > 0)
					cur.idx() = column;
				else
					cur.idx() = tabular.getCellBelow(first_visible_cell) + column;
			} else {
				cur.idx() = column;
			}
			resetPos(cur);
			break;
		}

		// none of these make sense for insettabular,
		// but we must catch them to prevent any
		// selection from being confused
		case LFUN_PRIORSEL:
		case LFUN_NEXTSEL:
		case LFUN_WORDLEFT:
		case LFUN_WORDLEFTSEL:
		case LFUN_WORDRIGHT:
		case LFUN_WORDRIGHTSEL:
		case LFUN_WORDSEL:
		case LFUN_DOWN_PARAGRAPH:
		case LFUN_DOWN_PARAGRAPHSEL:
		case LFUN_UP_PARAGRAPH:
		case LFUN_UP_PARAGRAPHSEL:
		case LFUN_BACKSPACE:
		case LFUN_HOME:
		case LFUN_HOMESEL:
		case LFUN_END:
		case LFUN_ENDSEL:
		case LFUN_BEGINNINGBUF:
		case LFUN_BEGINNINGBUFSEL:
		case LFUN_ENDBUF:
		case LFUN_ENDBUFSEL:
			break;

		case LFUN_LAYOUT_TABULAR:
			InsetTabularMailer(*this).showDialog(&cur.bv());
			break;

		case LFUN_INSET_DIALOG_UPDATE:
			InsetTabularMailer(*this).updateDialog(&cur.bv());
			break;

		case LFUN_TABULAR_FEATURE:
			if (!tabularFeatures(cur, cmd.argument))
				result = DispatchResult(false);
			break;

		// insert file functions
		case LFUN_FILE_INSERT_ASCII_PARA:
		case LFUN_FILE_INSERT_ASCII: {
			string tmpstr = getContentsOfAsciiFile(&cur.bv(), cmd.argument, false);
			if (!tmpstr.empty() && !insertAsciiString(cur.bv(), tmpstr, false))
				result = DispatchResult(false);
			break;
		}

		case LFUN_LANGUAGE:
		case LFUN_EMPH:
		case LFUN_BOLD:
		case LFUN_NOUN:
		case LFUN_CODE:
		case LFUN_SANS:
		case LFUN_ROMAN:
		case LFUN_DEFAULT:
		case LFUN_UNDERLINE:
		case LFUN_FONT_SIZE:
			lyxerr << "font changes not re-implemented for tables after LOCK" << endl;
			break;

		case LFUN_CUT:
			if (copySelection(cur.bv())) {
				recordUndo(cur, Undo::DELETE);
				cutSelection(cur.bv().buffer()->params());
			}
			break;

		case LFUN_DELETE:
			recordUndo(cur, Undo::DELETE);
			cutSelection(cur.bv().buffer()->params());
			break;

		case LFUN_COPY:
			if (!hasSelection())
				break;
			finishUndo();
			copySelection(cur.bv());
			break;

		case LFUN_PASTESELECTION: {
			string const clip = cur.bv().getClipboard();
			if (clip.empty())
				break;
			if (clip.find('\t') != string::npos) {
				int cols = 1;
				int rows = 1;
				int maxCols = 1;
				string::size_type len = clip.length();
				string::size_type p = 0;

				while (p < len &&
							(p = clip.find_first_of("\t\n", p)) != string::npos) {
					switch (clip[p]) {
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

				paste_tabular.reset(
					new LyXTabular(cur.bv().buffer()->params(), rows, maxCols));

				string::size_type op = 0;
				int cell = 0;
				int cells = paste_tabular->getNumberOfCells();
				p = 0;
				cols = 0;
				LyXFont font;
				while (cell < cells && p < len &&
							(p = clip.find_first_of("\t\n", p)) != string::npos) {
					if (p >= len)
						break;
					switch (clip[p]) {
					case '\t':
						paste_tabular->getCellInset(cell).
							setText(clip.substr(op, p-op), font);
						++cols;
						++cell;
						break;
					case '\n':
						paste_tabular->getCellInset(cell).
							setText(clip.substr(op, p-op), font);
						while (cols++ < maxCols)
							++cell;
						cols = 0;
						break;
					}
					++p;
					op = p;
				}
				// check for the last cell if there is no trailing '\n'
				if (cell < cells && op < len)
					paste_tabular->getCellInset(cell).
						setText(clip.substr(op, len-op), font);
			} else if (!insertAsciiString(cur.bv(), clip, true))
			{
				// so that the clipboard is used and it goes on
				// to default
				// and executes LFUN_PASTESELECTION in insettext!
				paste_tabular.reset();
			}
			// fall through
		}

		case LFUN_PASTE:
			if (hasPasteBuffer()) {
				recordUndo(cur, Undo::INSERT);
				pasteSelection(cur.bv());
				break;
			}
			// fall through

		// ATTENTION: the function above has to be PASTE and PASTESELECTION!!!

		default:
			// handle font changing stuff on selection before we lock the inset
			// in the default part!
			result = DispatchResult(false);
			// we try to activate the actual inset and put this event down to
			// the insets dispatch function.
			break;
		}

		updateLocal(cur);
		InsetTabularMailer(*this).updateDialog(&cur.bv());
	}

	if (cmd.action == LFUN_INSET_TOGGLE) {
		tablemode = !tablemode;
		result = DispatchResult(true, true);
	}

	return result;
}


int InsetTabular::latex(Buffer const & buf, ostream & os,
			OutputParams const & runparams) const
{
	return tabular.latex(buf, os, runparams);
}


int InsetTabular::plaintext(Buffer const & buf, ostream & os,
			OutputParams const & runparams) const
{
	int dp = runparams.linelen ? ownerPar(buf, this).params().depth() : 0;
	return tabular.plaintext(buf, os, runparams, dp, false, 0);
}


int InsetTabular::linuxdoc(Buffer const & buf, ostream & os,
			   OutputParams const & runparams) const
{
	return tabular.linuxdoc(buf,os, runparams);
}


int InsetTabular::docbook(Buffer const & buf, ostream & os,
			  OutputParams const & runparams) const
{
	int ret = 0;
	InsetOld * master;

	// if the table is inside a float it doesn't need the informaltable
	// wrapper. Search for it.
	for (master = owner(); master; master = master->owner())
		if (master->lyxCode() == InsetOld::FLOAT_CODE)
			break;

	if (!master) {
		os << "<informaltable>";
		if (runparams.mixed_content)
			os << endl;
		++ret;
	}
	ret += tabular.docbook(buf, os, runparams);
	if (!master) {
		os << "</informaltable>";
		if (runparams.mixed_content)
			os << endl;
		++ret;
	}
	return ret;
}


void InsetTabular::validate(LaTeXFeatures & features) const
{
	tabular.validate(features);
}


void InsetTabular::calculate_dimensions_of_cells(MetricsInfo & mi) const
{
	for (int i = 0, cell = -1; i < tabular.rows(); ++i) {
		int maxAsc = 0;
		int maxDesc = 0;
		for (int j = 0; j < tabular.columns(); ++j) {
			if (tabular.isPartOfMultiColumn(i, j))
				continue;
			++cell;
			Dimension dim;
			MetricsInfo m = mi;
			m.base.textwidth =
				tabular.column_info[j].p_width.inPixels(mi.base.textwidth);
			tabular.getCellInset(cell).metrics(m, dim);
			maxAsc  = max(maxAsc, dim.asc);
			maxDesc = max(maxDesc, dim.des);
			tabular.setWidthOfCell(cell, dim.wid);
		}
		tabular.setAscentOfRow(i, maxAsc + ADD_TO_HEIGHT);
		tabular.setDescentOfRow(i, maxDesc + ADD_TO_HEIGHT);
	}
}


void InsetTabular::getCursorPos(int cell, int & x, int & y) const
{
	InsetText const & inset = tabular.getCellInset(cell);
	inset.getCursorPos(cell, x, y);
	x += inset.x() - xo_;
	y += inset.y() - yo_;
}


void InsetTabular::setPos(BufferView & bv, int x, int y) const
{
	int const cell = getCell(x + xo_, y + yo_);
	lyxerr << "# InsetTabular::setPos()  cell: " << cell << endl;
	InsetText const & inset = tabular.getCellInset(cell);
	inset.text_.setCursorFromCoordinates(x, y);
#if 0
	cursory_ = 0;
	int actcell = 0;
	int actrow = 0;
	int actcol = 0;
	int ly = tabular.getDescentOfRow(actrow);

	// first search the right row
	while (ly < y && actrow + 1 < tabular.rows()) {
		cursory_ += tabular.getDescentOfRow(actrow) +
				 tabular.getAscentOfRow(actrow + 1) +
				 tabular.getAdditionalHeight(actrow + 1);
		++actrow;
		ly = cursory_ + tabular.getDescentOfRow(actrow);
	}
	actcell = tabular.getCellNumber(actrow, actcol);

	// now search the right column
	int lx = tabular.getWidthOfColumn(actcell) -
		tabular.getAdditionalWidth(actcell);

	for (; !tabular.isLastCellInRow(actcell) && lx < x; ++actcell)
		lx += tabular.getWidthOfColumn(actcell + 1)
			+ tabular.getAdditionalWidth(actcell);

	cursorx_ = lx - tabular.getWidthOfColumn(actcell) + xo_ + 2;
#endif
	resetPos(bv.cursor());
}


int InsetTabular::getCellXPos(int cell) const
{
	int c = cell;

	for (; !tabular.isFirstCellInRow(c); --c)
		;
	int lx = tabular.getWidthOfColumn(cell);
	for (; c < cell; ++c)
		lx += tabular.getWidthOfColumn(c);

	return lx - tabular.getWidthOfColumn(cell) + xo_;
}


void InsetTabular::resetPos(LCursor &) const
{
#if 0
#ifdef WITH_WARNINGS
#warning This should be fixed in the right manner (20011128 Jug)
#endif
	// fast hack to fix infinite repaintings!
	if (in_reset_pos > 0)
		return;

	BufferView & bv = cur.bv();
	int cell = 0;
	int actcell = cur.cell();
	int actcol = tabular.column_of_cell(actcell);
	int actrow = 0;
	cursory_ = 0;
	for (; cell < actcell && !tabular.isLastRow(cell); ++cell) {
		if (tabular.isLastCellInRow(cell)) {
			cursory_ += tabular.getDescentOfRow(actrow) +
					 tabular.getAscentOfRow(actrow + 1) +
					 tabular.getAdditionalHeight(actrow + 1);
			++actrow;
		}
	}

	// we need this only from here on!!!
	++in_reset_pos;
	int const offset = ADD_TO_TABULAR_WIDTH + 2;
	int new_x = getCellXPos(actcell) + offset;
	int old_x = cursorx_;
	cursorx_ = new_x;
//    cursor.x(getCellXPos(actcell) + offset);
	if (actcol < tabular.columns() - 1 && scroll(false) &&
		tabular.getWidthOfTabular() < bv->workWidth()-20)
	{
		scroll(bv, 0.0F);
		updateLocal(cur);
	} else if (cursorx_ - offset > 20 &&
		   cursorx_ - offset + tabular.getWidthOfColumn(actcell)
		   > bv.workWidth() - 20) {
		scroll(&bv, - tabular.getWidthOfColumn(actcell) - 20);
		updateLocal(cur);
	} else if (cursorx_ - offset < 20) {
		scroll(&bv, 20 - cursorx_ + offset);
		updateLocal(cur);
	} else if (scroll() && xo_ > 20 &&
		   xo_ + tabular.getWidthOfTabular() > bv->workWidth() - 20) {
		scroll(&bv, old_x - cursorx_);
		updateLocal(cur);
	}
	InsetTabularMailer(*this).updateDialog(bv);
	in_reset_pos = 0;
#endif
}


bool InsetTabular::moveRight(LCursor & cur)
{
	bool moved = isRightToLeft(cur) ? movePrevCell(cur) : moveNextCell(cur);
	if (!moved)
		return false;
	resetPos(cur);
	return true;
}


bool InsetTabular::moveRightLock(LCursor & cur)
{
	bool moved = isRightToLeft(cur) ? movePrevCell(cur) : moveNextCell(cur);
	if (!moved)
		return false;
	activateCellInset(cur, cur.idx(), false);
	return true;
}


bool InsetTabular::moveLeft(LCursor & cur)
{
	bool moved = isRightToLeft(cur) ? moveNextCell(cur) : movePrevCell(cur);
	if (!moved)
		return false;
	resetPos(cur);
	return true;
}


bool InsetTabular::moveLeftLock(LCursor & cur)
{
	bool moved = isRightToLeft(cur) ? moveNextCell(cur) : movePrevCell(cur);
	if (!moved)
		return false;
	activateCellInset(cur, cur.idx(), true);
	return true;
}


bool InsetTabular::moveUp(LCursor & cur)
{
	if (tabular.row_of_cell(cur.idx()) == 0)
		return false;
	cur.idx() = tabular.getCellAbove(cur.idx());
	resetPos(cur);
	return true;
}


bool InsetTabular::moveUpLock(LCursor & cur)
{
	if (tabular.row_of_cell(cur.idx()) == 0)
		return false;
	cur.idx() = tabular.getCellAbove(cur.idx());
	resetPos(cur);
	activateCellInset(cur, cur.idx(), cur.x_target(), 0);
	return true;
}


bool InsetTabular::moveDown(LCursor & cur)
{
	if (tabular.row_of_cell(cur.idx()) == tabular.rows() - 1)
		return false;
	cur.idx() = tabular.getCellBelow(cur.idx());
	resetPos(cur);
	return true;
}


bool InsetTabular::moveDownLock(LCursor & cur)
{
	if (tabular.row_of_cell(cur.idx()) == tabular.rows() - 1)
		return false;
	cur.idx() = tabular.getCellBelow(cur.idx());
	resetPos(cur);
	activateCellInset(cur, cur.idx(), cur.x_target());
	return true;
}


bool InsetTabular::moveNextCell(LCursor & cur)
{
	lyxerr << "InsetTabular::moveNextCell 1 cur: " << cur << endl;
	if (isRightToLeft(cur)) {
		if (tabular.isFirstCellInRow(cur.idx())) {
			int row = tabular.row_of_cell(cur.idx());
			if (row == tabular.rows() - 1)
				return false;
			cur.idx() = tabular.getLastCellInRow(row);
			cur.idx() = tabular.getCellBelow(cur.idx());
		} else {
			if (cur.idx() == 0)
				return false;
			--cur.idx();
		}
	} else {
		if (tabular.isLastCell(cur.idx()))
			return false;
		++cur.idx();
	}
	cur.par() = 0;
	cur.pos() = 0;
	lyxerr << "InsetTabular::moveNextCell 2 cur: " << cur << endl;
	resetPos(cur);
	return true;
}


bool InsetTabular::movePrevCell(LCursor & cur)
{
	if (isRightToLeft(cur)) {
		if (tabular.isLastCellInRow(cur.idx())) {
			int row = tabular.row_of_cell(cur.idx());
			if (row == 0)
				return false;
			cur.idx() = tabular.getFirstCellInRow(row);
			cur.idx() = tabular.getCellAbove(cur.idx());
		} else {
			if (tabular.isLastCell(cur.idx()))
				return false;
			++cur.idx();
		}
	} else {
		if (cur.idx() == 0) // first cell
			return false;
		--cur.idx();
	}
	cur.par() = 0;
	cur.pos() = 0;
	resetPos(cur);
	return true;
}


bool InsetTabular::tabularFeatures(LCursor & cur, std::string const & what)
{
	LyXTabular::Feature action = LyXTabular::LAST_ACTION;

	int i = 0;
	for (; tabularFeature[i].action != LyXTabular::LAST_ACTION; ++i) {
		string const tmp = tabularFeature[i].feature;

		if (tmp == what.substr(0, tmp.length())) {
			//if (!compare(tabularFeatures[i].feature.c_str(), what.c_str(),
			//tabularFeatures[i].feature.length())) {
			action = tabularFeature[i].action;
			break;
		}
	}
	if (action == LyXTabular::LAST_ACTION)
		return false;

	string const val =
		ltrim(what.substr(tabularFeature[i].feature.length()));
	tabularFeatures(cur, action, val);
	return true;
}


namespace {

void checkLongtableSpecial(LyXTabular::ltType & ltt,
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

} // anon namespace


void InsetTabular::tabularFeatures(LCursor & cur,
	LyXTabular::Feature feature, string const & value)
{
	BufferView & bv = cur.bv();
	int actcell = cur.idx();
	int sel_col_start;
	int sel_col_end;
	int sel_row_start;
	int sel_row_end;
	bool setLines = false;
	LyXAlignment setAlign = LYX_ALIGN_LEFT;
	LyXTabular::VAlignment setVAlign = LyXTabular::LYX_VALIGN_TOP;

	switch (feature) {

	case LyXTabular::M_ALIGN_LEFT:
	case LyXTabular::ALIGN_LEFT:
		setAlign = LYX_ALIGN_LEFT;
		break;

	case LyXTabular::M_ALIGN_RIGHT:
	case LyXTabular::ALIGN_RIGHT:
		setAlign = LYX_ALIGN_RIGHT;
		break;

	case LyXTabular::M_ALIGN_CENTER:
	case LyXTabular::ALIGN_CENTER:
		setAlign = LYX_ALIGN_CENTER;
		break;

	case LyXTabular::ALIGN_BLOCK:
		setAlign = LYX_ALIGN_BLOCK;
		break;

	case LyXTabular::M_VALIGN_TOP:
	case LyXTabular::VALIGN_TOP:
		setVAlign = LyXTabular::LYX_VALIGN_TOP;
		break;

	case LyXTabular::M_VALIGN_BOTTOM:
	case LyXTabular::VALIGN_BOTTOM:
		setVAlign = LyXTabular::LYX_VALIGN_BOTTOM;
		break;

	case LyXTabular::M_VALIGN_MIDDLE:
	case LyXTabular::VALIGN_MIDDLE:
		setVAlign = LyXTabular::LYX_VALIGN_MIDDLE;
		break;

	default:
		break;
	}

	if (hasSelection()) {
		getSelection(actcell, sel_row_start, sel_row_end, sel_col_start, sel_col_end);
	} else {
		sel_col_start = sel_col_end = tabular.column_of_cell(actcell);
		sel_row_start = sel_row_end = tabular.row_of_cell(actcell);
	}
	recordUndo(cur, Undo::ATOMIC);

	int row =  tabular.row_of_cell(actcell);
	int column = tabular.column_of_cell(actcell);
	bool flag = true;
	LyXTabular::ltType ltt;

	switch (feature) {

	case LyXTabular::SET_PWIDTH: {
		LyXLength const len(value);
		tabular.setColumnPWidth(actcell, len);
		if (len.zero()
		    && tabular.getAlignment(actcell, true) == LYX_ALIGN_BLOCK)
			tabularFeatures(cur, LyXTabular::ALIGN_CENTER, string());
		else if (!len.zero()
			 && tabular.getAlignment(actcell, true) != LYX_ALIGN_BLOCK)
			tabularFeatures(cur, LyXTabular::ALIGN_BLOCK, string());
		break;
	}

	case LyXTabular::SET_MPWIDTH:
		tabular.setMColumnPWidth(actcell, LyXLength(value));
		break;

	case LyXTabular::SET_SPECIAL_COLUMN:
	case LyXTabular::SET_SPECIAL_MULTI:
		tabular.setAlignSpecial(actcell,value,feature);
		break;

	case LyXTabular::APPEND_ROW:
		// append the row into the tabular
		tabular.appendRow(bv.buffer()->params(), actcell);
		tabular.setOwner(this);
		break;

	case LyXTabular::APPEND_COLUMN:
		// append the column into the tabular
		tabular.appendColumn(bv.buffer()->params(), actcell);
		tabular.setOwner(this);
		actcell = tabular.getCellNumber(row, column);
		break;

	case LyXTabular::DELETE_ROW:
		for (int i = sel_row_start; i <= sel_row_end; ++i)
			tabular.deleteRow(sel_row_start);
		if (sel_row_start >= tabular.rows())
			--sel_row_start;
		actcell = tabular.getCellNumber(sel_row_start, column);
		clearSelection();
		break;

	case LyXTabular::DELETE_COLUMN:
		for (int i = sel_col_start; i <= sel_col_end; ++i)
			tabular.deleteColumn(sel_col_start);
		if (sel_col_start >= tabular.columns())
			--sel_col_start;
		actcell = tabular.getCellNumber(row, sel_col_start);
		clearSelection();
		break;

	case LyXTabular::M_TOGGLE_LINE_TOP:
		flag = false;
	case LyXTabular::TOGGLE_LINE_TOP: {
		bool lineSet = !tabular.topLine(actcell, flag);
		for (int i = sel_row_start; i <= sel_row_end; ++i)
			for (int j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setTopLine(
					tabular.getCellNumber(i, j),
					lineSet, flag);
		break;
	}

	case LyXTabular::M_TOGGLE_LINE_BOTTOM:
		flag = false;
	case LyXTabular::TOGGLE_LINE_BOTTOM: {
		bool lineSet = !tabular.bottomLine(actcell, flag);
		for (int i = sel_row_start; i <= sel_row_end; ++i)
			for (int j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setBottomLine(
					tabular.getCellNumber(i, j),
					lineSet,
					flag);
		break;
	}

	case LyXTabular::M_TOGGLE_LINE_LEFT:
		flag = false;
	case LyXTabular::TOGGLE_LINE_LEFT: {
		bool lineSet = !tabular.leftLine(actcell, flag);
		for (int i = sel_row_start; i <= sel_row_end; ++i)
			for (int j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setLeftLine(
					tabular.getCellNumber(i,j),
					lineSet,
					flag);
		break;
	}

	case LyXTabular::M_TOGGLE_LINE_RIGHT:
		flag = false;
	case LyXTabular::TOGGLE_LINE_RIGHT: {
		bool lineSet = !tabular.rightLine(actcell, flag);
		for (int i = sel_row_start; i <= sel_row_end; ++i)
			for (int j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setRightLine(
					tabular.getCellNumber(i,j),
					lineSet,
					flag);
		break;
	}

	case LyXTabular::M_ALIGN_LEFT:
	case LyXTabular::M_ALIGN_RIGHT:
	case LyXTabular::M_ALIGN_CENTER:
		flag = false;
	case LyXTabular::ALIGN_LEFT:
	case LyXTabular::ALIGN_RIGHT:
	case LyXTabular::ALIGN_CENTER:
	case LyXTabular::ALIGN_BLOCK:
		for (int i = sel_row_start; i <= sel_row_end; ++i)
			for (int j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setAlignment(
					tabular.getCellNumber(i, j),
					setAlign,
					flag);
		break;

	case LyXTabular::M_VALIGN_TOP:
	case LyXTabular::M_VALIGN_BOTTOM:
	case LyXTabular::M_VALIGN_MIDDLE:
		flag = false;
	case LyXTabular::VALIGN_TOP:
	case LyXTabular::VALIGN_BOTTOM:
	case LyXTabular::VALIGN_MIDDLE:
		for (int i = sel_row_start; i <= sel_row_end; ++i)
			for (int j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setVAlignment(
					tabular.getCellNumber(i, j),
					setVAlign, flag);
		break;

	case LyXTabular::MULTICOLUMN: {
		if (sel_row_start != sel_row_end) {
#ifdef WITH_WARNINGS
#warning Need I say it ? This is horrible.
#endif
			Alert::error(_("Error setting multicolumn"),
				   _("You cannot set multicolumn vertically."));
			return;
		}
		// just multicol for one Single Cell
		if (!hasSelection()) {
			// check wether we are completly in a multicol
			if (tabular.isMultiColumn(actcell))
				tabular.unsetMultiColumn(actcell);
			else
				tabular.setMultiColumn(bv.buffer(), actcell, 1);
			break;
		}
		// we have a selection so this means we just add all this
		// cells to form a multicolumn cell
		int s_start;
		int s_end;

		if (sel_cell_start > sel_cell_end) {
			s_start = sel_cell_end;
			s_end = sel_cell_start;
		} else {
			s_start = sel_cell_start;
			s_end = sel_cell_end;
		}
		tabular.setMultiColumn(bv.buffer(), s_start, s_end - s_start + 1);
		actcell = s_start;
		clearSelection();
		break;
	}

	case LyXTabular::SET_ALL_LINES:
		setLines = true;
	case LyXTabular::UNSET_ALL_LINES:
		for (int i = sel_row_start; i <= sel_row_end; ++i)
			for (int j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setAllLines(
					tabular.getCellNumber(i,j), setLines);
		break;

	case LyXTabular::SET_LONGTABULAR:
		tabular.setLongTabular(true);
		break;

	case LyXTabular::UNSET_LONGTABULAR:
		tabular.setLongTabular(false);
		break;

	case LyXTabular::SET_ROTATE_TABULAR:
		tabular.setRotateTabular(true);
		break;

	case LyXTabular::UNSET_ROTATE_TABULAR:
		tabular.setRotateTabular(false);
		break;

	case LyXTabular::SET_ROTATE_CELL:
		for (int i = sel_row_start; i <= sel_row_end; ++i)
			for (int j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setRotateCell(
					tabular.getCellNumber(i, j), true);
		break;

	case LyXTabular::UNSET_ROTATE_CELL:
		for (int i = sel_row_start; i <= sel_row_end; ++i)
			for (int j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setRotateCell(
					tabular.getCellNumber(i, j), false);
		break;

	case LyXTabular::SET_USEBOX: {
		LyXTabular::BoxType val = LyXTabular::BoxType(strToInt(value));
		if (val == tabular.getUsebox(actcell))
			val = LyXTabular::BOX_NONE;
		for (int i = sel_row_start; i <= sel_row_end; ++i)
			for (int j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setUsebox(tabular.getCellNumber(i, j), val);
		break;
	}

	case LyXTabular::UNSET_LTFIRSTHEAD:
		flag = false;
	case LyXTabular::SET_LTFIRSTHEAD:
		tabular.getRowOfLTFirstHead(row, ltt);
		checkLongtableSpecial(ltt, value, flag);
		tabular.setLTHead(row, flag, ltt, true);
		break;

	case LyXTabular::UNSET_LTHEAD:
		flag = false;
	case LyXTabular::SET_LTHEAD:
		tabular.getRowOfLTHead(row, ltt);
		checkLongtableSpecial(ltt, value, flag);
		tabular.setLTHead(row, flag, ltt, false);
		break;

	case LyXTabular::UNSET_LTFOOT:
		flag = false;
	case LyXTabular::SET_LTFOOT:
		tabular.getRowOfLTFoot(row, ltt);
		checkLongtableSpecial(ltt, value, flag);
		tabular.setLTFoot(row, flag, ltt, false);
		break;

	case LyXTabular::UNSET_LTLASTFOOT:
		flag = false;
	case LyXTabular::SET_LTLASTFOOT:
		tabular.getRowOfLTLastFoot(row, ltt);
		checkLongtableSpecial(ltt, value, flag);
		tabular.setLTFoot(row, flag, ltt, true);
		break;

	case LyXTabular::SET_LTNEWPAGE:
		tabular.setLTNewPage(row, !tabular.getLTNewPage(row));
		break;

	// dummy stuff just to avoid warnings
	case LyXTabular::LAST_ACTION:
		break;
	}

	updateLocal(cur);
	InsetTabularMailer(*this).updateDialog(&bv);
}


void InsetTabular::activateCellInset(LCursor & cur, int cell, int x, int y)
{
	tabular.getCellInset(cell).edit(cur, x, y);
	cur.idx() = cell;
	updateLocal(cur);
}


void InsetTabular::activateCellInset(LCursor & cur, int cell, bool behind)
{
	tabular.getCellInset(cell).edit(cur, behind);
	cur.idx() = cell;
	updateLocal(cur);
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


//
// function returns an object as defined in func_status.h:
// states OK, Unknown, Disabled, On, Off.
//
FuncStatus InsetTabular::getStatus(string const & what, int actcell) const
{
	FuncStatus status;
	int action = LyXTabular::LAST_ACTION;

	int i = 0;
	for (; tabularFeature[i].action != LyXTabular::LAST_ACTION; ++i) {
		string const tmp = tabularFeature[i].feature;
		if (tmp == what.substr(0, tmp.length())) {
			//if (!compare(tabularFeatures[i].feature.c_str(), what.c_str(),
			//   tabularFeatures[i].feature.length())) 
			action = tabularFeature[i].action;
			break;
		}
	}
	if (action == LyXTabular::LAST_ACTION) {
		status.clear();
		status.unknown(true);
		return status;
	}

	string const argument
		= ltrim(what.substr(tabularFeature[i].feature.length()));

	int sel_row_start;
	int sel_row_end;
	int dummy;
	LyXTabular::ltType dummyltt;
	bool flag = true;

	if (hasSelection())
		getSelection(actcell, sel_row_start, sel_row_end, dummy, dummy);
	else
		sel_row_start = sel_row_end = tabular.row_of_cell(actcell);

	switch (action) {
	case LyXTabular::SET_PWIDTH:
	case LyXTabular::SET_MPWIDTH:
	case LyXTabular::SET_SPECIAL_COLUMN:
	case LyXTabular::SET_SPECIAL_MULTI:
	case LyXTabular::APPEND_ROW:
	case LyXTabular::APPEND_COLUMN:
	case LyXTabular::DELETE_ROW:
	case LyXTabular::DELETE_COLUMN:
	case LyXTabular::SET_ALL_LINES:
	case LyXTabular::UNSET_ALL_LINES:
		status.clear();
		return status;

	case LyXTabular::MULTICOLUMN:
		status.setOnOff(tabular.isMultiColumn(actcell));
		break;

	case LyXTabular::M_TOGGLE_LINE_TOP:
		flag = false;
	case LyXTabular::TOGGLE_LINE_TOP:
		status.setOnOff(tabular.topLine(actcell, flag));
		break;

	case LyXTabular::M_TOGGLE_LINE_BOTTOM:
		flag = false;
	case LyXTabular::TOGGLE_LINE_BOTTOM:
		status.setOnOff(tabular.bottomLine(actcell, flag));
		break;

	case LyXTabular::M_TOGGLE_LINE_LEFT:
		flag = false;
	case LyXTabular::TOGGLE_LINE_LEFT:
		status.setOnOff(tabular.leftLine(actcell, flag));
		break;

	case LyXTabular::M_TOGGLE_LINE_RIGHT:
		flag = false;
	case LyXTabular::TOGGLE_LINE_RIGHT:
		status.setOnOff(tabular.rightLine(actcell, flag));
		break;

	case LyXTabular::M_ALIGN_LEFT:
		flag = false;
	case LyXTabular::ALIGN_LEFT:
		status.setOnOff(tabular.getAlignment(actcell, flag) == LYX_ALIGN_LEFT);
		break;

	case LyXTabular::M_ALIGN_RIGHT:
		flag = false;
	case LyXTabular::ALIGN_RIGHT:
		status.setOnOff(tabular.getAlignment(actcell, flag) == LYX_ALIGN_RIGHT);
		break;

	case LyXTabular::M_ALIGN_CENTER:
		flag = false;
	case LyXTabular::ALIGN_CENTER:
		status.setOnOff(tabular.getAlignment(actcell, flag) == LYX_ALIGN_CENTER);
		break;

	case LyXTabular::ALIGN_BLOCK:
		status.disabled(tabular.getPWidth(actcell).zero());
		status.setOnOff(tabular.getAlignment(actcell, flag) == LYX_ALIGN_BLOCK);
		break;

	case LyXTabular::M_VALIGN_TOP:
		flag = false;
	case LyXTabular::VALIGN_TOP:
		status.setOnOff(
			tabular.getVAlignment(actcell, flag) == LyXTabular::LYX_VALIGN_TOP);
		break;

	case LyXTabular::M_VALIGN_BOTTOM:
		flag = false;
	case LyXTabular::VALIGN_BOTTOM:
		status.setOnOff(
			tabular.getVAlignment(actcell, flag) == LyXTabular::LYX_VALIGN_BOTTOM);
		break;

	case LyXTabular::M_VALIGN_MIDDLE:
		flag = false;
	case LyXTabular::VALIGN_MIDDLE:
		status.setOnOff(
			tabular.getVAlignment(actcell, flag) == LyXTabular::LYX_VALIGN_MIDDLE);
		break;

	case LyXTabular::SET_LONGTABULAR:
		status.setOnOff(tabular.isLongTabular());
		break;

	case LyXTabular::UNSET_LONGTABULAR:
		status.setOnOff(!tabular.isLongTabular());
		break;

	case LyXTabular::SET_ROTATE_TABULAR:
		status.setOnOff(tabular.getRotateTabular());
		break;

	case LyXTabular::UNSET_ROTATE_TABULAR:
		status.setOnOff(!tabular.getRotateTabular());
		break;

	case LyXTabular::SET_ROTATE_CELL:
		status.setOnOff(tabular.getRotateCell(actcell));
		break;

	case LyXTabular::UNSET_ROTATE_CELL:
		status.setOnOff(!tabular.getRotateCell(actcell));
		break;

	case LyXTabular::SET_USEBOX:
		status.setOnOff(strToInt(argument) == tabular.getUsebox(actcell));
		break;

	case LyXTabular::SET_LTFIRSTHEAD:
		status.setOnOff(tabular.getRowOfLTHead(sel_row_start, dummyltt));
		break;

	case LyXTabular::SET_LTHEAD:
		status.setOnOff(tabular.getRowOfLTHead(sel_row_start, dummyltt));
		break;

	case LyXTabular::SET_LTFOOT:
		status.setOnOff(tabular.getRowOfLTFoot(sel_row_start, dummyltt));
		break;

	case LyXTabular::SET_LTLASTFOOT:
		status.setOnOff(tabular.getRowOfLTFoot(sel_row_start, dummyltt));
		break;

	case LyXTabular::SET_LTNEWPAGE:
		status.setOnOff(tabular.getLTNewPage(sel_row_start));
		break;

	default:
		status.clear();
		status.disabled(true);
		break;
	}
	return status;
}


void InsetTabular::getLabelList(Buffer const & buffer,
				std::vector<string> & list) const
{
	tabular.getLabelList(buffer, list);
}


bool InsetTabular::copySelection(BufferView & bv)
{
	if (!hasSelection())
		return false;

	int sel_col_start = tabular.column_of_cell(sel_cell_start);
	int sel_col_end = tabular.column_of_cell(sel_cell_end);
	if (sel_col_start > sel_col_end) {
		sel_col_start = sel_col_end;
		sel_col_end = tabular.right_column_of_cell(sel_cell_start);
	} else {
		sel_col_end = tabular.right_column_of_cell(sel_cell_end);
	}

	int sel_row_start = tabular.row_of_cell(sel_cell_start);
	int sel_row_end = tabular.row_of_cell(sel_cell_end);
	if (sel_row_start > sel_row_end)
		swap(sel_row_start, sel_row_end);

	paste_tabular.reset(new LyXTabular(tabular));
	paste_tabular->setOwner(this);

	for (int i = 0; i < sel_row_start; ++i)
		paste_tabular->deleteRow(0);

	int const rows = sel_row_end - sel_row_start + 1;
	while (paste_tabular->rows() > rows)
		paste_tabular->deleteRow(rows);

	paste_tabular->setTopLine(0, true, true);
	paste_tabular->setBottomLine(paste_tabular->getFirstCellInRow(rows - 1),
				     true, true);

	for (int i = 0; i < sel_col_start; ++i)
		paste_tabular->deleteColumn(0);

	int const columns = sel_col_end - sel_col_start + 1;
	while (paste_tabular->columns() > columns)
		paste_tabular->deleteColumn(columns);

	paste_tabular->setLeftLine(0, true, true);
	paste_tabular->setRightLine(paste_tabular->getLastCellInRow(0),
				    true, true);

	ostringstream os;
	OutputParams const runparams;
	paste_tabular->plaintext(*bv.buffer(), os, runparams,
				 ownerPar(*bv.buffer(), this).params().depth(), true, '\t');
	bv.stuffClipboard(os.str());
	return true;
}


bool InsetTabular::pasteSelection(BufferView & bv)
{
	if (!paste_tabular)
		return false;
	int actcell = bv.cursor().idx();
	int actcol = tabular.column_of_cell(actcell);
	int actrow = tabular.row_of_cell(actcell);
	for (int r1 = 0, r2 = actrow;
	     r1 < paste_tabular->rows() && r2 < tabular.rows();
	     ++r1, ++r2) {
		for (int c1 = 0, c2 = actcol;
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
			InsetText & inset = tabular.getCellInset(r2, c2);
			inset = paste_tabular->getCellInset(r1, c1);
			inset.setOwner(this);
			inset.markNew();
		}
	}
	return true;
}


bool InsetTabular::cutSelection(BufferParams const & bp)
{
	if (!hasSelection())
		return false;

	int sel_col_start = tabular.column_of_cell(sel_cell_start);
	int sel_col_end = tabular.column_of_cell(sel_cell_end);
	if (sel_col_start > sel_col_end) {
		sel_col_start = sel_col_end;
		sel_col_end = tabular.right_column_of_cell(sel_cell_start);
	} else {
		sel_col_end = tabular.right_column_of_cell(sel_cell_end);
	}

	int sel_row_start = tabular.row_of_cell(sel_cell_start);
	int sel_row_end = tabular.row_of_cell(sel_cell_end);

	if (sel_row_start > sel_row_end)
		swap(sel_row_start, sel_row_end);

	if (sel_cell_start > sel_cell_end)
		swap(sel_cell_start, sel_cell_end);

	for (int i = sel_row_start; i <= sel_row_end; ++i)
		for (int j = sel_col_start; j <= sel_col_end; ++j)
			tabular.getCellInset(tabular.getCellNumber(i, j))
				.clear(bp.tracking_changes);
	return true;
}


bool InsetTabular::isRightToLeft(LCursor & cur)
{
	return cur.bv().getParentLanguage(this)->RightToLeft();
}


void InsetTabular::getSelection(int actcell,
	int & srow, int & erow, int & scol, int & ecol) const
{
	int const start = hasSelection() ? sel_cell_start : actcell;
	int const end = hasSelection() ? sel_cell_end : actcell;

	srow = tabular.row_of_cell(start);
	erow = tabular.row_of_cell(end);
	if (srow > erow)
		swap(srow, erow);

	scol = tabular.column_of_cell(start);
	ecol = tabular.column_of_cell(end);
	if (scol > ecol)
		swap(scol, ecol);
	else
		ecol = tabular.right_column_of_cell(end);
}


int InsetTabular::numParagraphs() const
{
	return tabular.getNumberOfCells();
}


LyXText * InsetTabular::getText(int i) const
{
	return i < tabular.getNumberOfCells()
		? tabular.getCellInset(i).getText(0)
		: 0;
}


void InsetTabular::markErased()
{
	for (int cell = 0; cell < tabular.getNumberOfCells(); ++cell)
		tabular.getCellInset(cell).markErased();
}


bool InsetTabular::forceDefaultParagraphs(InsetOld const * in) const
{
	const int cell = tabular.getCellFromInset(in);

	if (cell != -1)
		return tabular.getPWidth(cell).zero();

	// this is a workaround for a crash (New, Insert->Tabular,
	// Insert->FootNote)
	if (!owner())
		return false;

	// well we didn't obviously find it so maybe our owner knows more
	BOOST_ASSERT(owner());
	return owner()->forceDefaultParagraphs(in);
}


bool InsetTabular::insertAsciiString(BufferView & bv, string const & buf,
				     bool usePaste)
{
	if (buf.length() <= 0)
		return true;

	int cols = 1;
	int rows = 1;
	int maxCols = 1;
	string::size_type len = buf.length();
	string::size_type p = 0;

	int actcell = bv.cursor().idx();
	int actcol = tabular.column_of_cell(actcell);
	int actrow = tabular.row_of_cell(actcell);

	while (p < len && (p = buf.find_first_of("\t\n", p)) != string::npos) {
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
	LyXTabular * loctab;
	int cell = 0;
	int ocol = 0;
	int row = 0;
	if (usePaste) {
		paste_tabular.reset(
			new LyXTabular(bv.buffer()->params(), rows, maxCols));
		paste_tabular->setOwner(this);
		loctab = paste_tabular.get();
		cols = 0;
	} else {
		loctab = &tabular;
		cell = actcell;
		ocol = actcol;
		row = actrow;
	}

	string::size_type op = 0;
	int cells = loctab->getNumberOfCells();
	p = 0;
	cols = ocol;
	rows = loctab->rows();
	int const columns = loctab->columns();

	while (cell < cells && p < len && row < rows &&
	       (p = buf.find_first_of("\t\n", p)) != string::npos)
	{
		if (p >= len)
			break;
		switch (buf[p]) {
		case '\t':
			// we can only set this if we are not too far right
			if (cols < columns) {
				InsetText & inset = loctab->getCellInset(cell);
				LyXFont const font = inset.text_.
					getFont(inset.paragraphs().begin(), 0);
				inset.setText(buf.substr(op, p - op), font);
				++cols;
				++cell;
			}
			break;
		case '\n':
			// we can only set this if we are not too far right
			if (cols < columns) {
				InsetText & inset = tabular.getCellInset(cell);
				LyXFont const font = inset.text_.
					getFont(inset.paragraphs().begin(), 0);
				inset.setText(buf.substr(op, p - op), font);
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
		InsetText & inset = loctab->getCellInset(cell);
		LyXFont const font = inset.text_.getFont(inset.paragraphs().begin(), 0);
		inset.setText(buf.substr(op, len - op), font);
	}
	return true;
}


void InsetTabular::addPreview(PreviewLoader & loader) const
{
	int const rows = tabular.rows();
	int const columns = tabular.columns();
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < columns; ++j)
			tabular.getCellInset(i, j).addPreview(loader);
	}
}



void InsetTabular::clearSelection() const
{
	sel_cell_start = 0;
	sel_cell_end = 0;
	has_selection = false;
}


void InsetTabular::setSelection(int start, int end) const
{
	sel_cell_start = start;
	sel_cell_end = end;
	has_selection = true;
}


string const InsetTabularMailer::name_("tabular");

InsetTabularMailer::InsetTabularMailer(InsetTabular const & inset)
	: inset_(const_cast<InsetTabular &>(inset))
{}


string const InsetTabularMailer::inset2string(Buffer const &) const
{
	return params2string(inset_);
}


int InsetTabularMailer::string2params(string const & in, InsetTabular & inset)
{
	istringstream data(in);
	LyXLex lex(0,0);
	lex.setStream(data);

#warning CHECK verify that this is a sane value to return.
	if (in.empty())
		return -1;

	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token != name_)
			return -1;
	}

	int cell = -1;
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token != "\\active_cell")
			return -1;
		lex.next();
		cell = lex.getInteger();
	}

	// This is part of the inset proper that is usually swallowed
	// by Buffer::readInset
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token != "Tabular")
			return -1;
	}

	if (!lex.isOK())
		return -1;

	Buffer const & buffer = inset.buffer();
	inset.read(buffer, lex);

	// We can't set the active cell, but we can tell the frontend
	// what it is.
	return cell;
}


string const InsetTabularMailer::params2string(InsetTabular const & inset)
{
	ostringstream data;
#warning wrong!
	//data << name_ << " \\active_cell " << inset.getActCell() << '\n';
	data << name_ << " \\active_cell " << 0 << '\n';
	inset.write(inset.buffer(), data);
	data << "\\end_inset\n";
	return data.str();
}
