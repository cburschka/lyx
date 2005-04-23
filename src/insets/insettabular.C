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
#include "coordcache.h"
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

#include "support/convert.h"

#include "frontends/Alert.h"
#include "frontends/font_metrics.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"
#include "frontends/nullpainter.h"

#include <sstream>
#include <iostream>
#include <limits>

using lyx::graphics::PreviewLoader;

using lyx::support::ltrim;

using boost::shared_ptr;

using std::auto_ptr;
using std::endl;
using std::max;
using std::string;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::swap;
using std::vector;


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
	{ LyXTabular::UNSET_LTHEAD, "unset-lthead" },
	{ LyXTabular::SET_LTFIRSTHEAD, "set-ltfirsthead" },
	{ LyXTabular::UNSET_LTFIRSTHEAD, "unset-ltfirsthead" },
	{ LyXTabular::SET_LTFOOT, "set-ltfoot" },
	{ LyXTabular::UNSET_LTFOOT, "unset-ltfoot" },
	{ LyXTabular::SET_LTLASTFOOT, "set-ltlastfoot" },
	{ LyXTabular::UNSET_LTLASTFOOT, "unset-ltlastfoot" },
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


InsetTabular::InsetTabular(Buffer const & buf, row_type rows,
                           col_type columns)
	: tabular(buf.params(), max(rows, row_type(1)),
	  max(columns, col_type(1))), buffer_(&buf), cursorx_(0)
{}


InsetTabular::InsetTabular(InsetTabular const & tab)
	: UpdatableInset(tab), tabular(tab.tabular),
		buffer_(tab.buffer_), cursorx_(0)
{}


InsetTabular::~InsetTabular()
{
	InsetTabularMailer(*this).hideDialog();
}


auto_ptr<InsetBase> InsetTabular::doClone() const
{
	return auto_ptr<InsetBase>(new InsetTabular(*this));
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


void InsetTabular::read(Buffer const & buf, LyXLex & lex)
{
	bool const old_format = (lex.getString() == "\\LyXTable");

	tabular.read(buf, lex);

	if (old_format)
		return;

	lex.next();
	string token = lex.getString();
	while (lex.isOK() && (token != "\\end_inset")) {
		lex.next();
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

	row_type i = 0;
	for (idx_type cell = 0; i < tabular.rows(); ++i) {
		int maxAsc = 0;
		int maxDesc = 0;
		for (col_type j = 0; j < tabular.columns(); ++j) {
			if (tabular.isPartOfMultiColumn(i, j))
				continue;
			Dimension dim;
			MetricsInfo m = mi;
			LyXLength p_width = tabular.column_info[j].p_width;
			if (!p_width.zero()) {
				m.base.textwidth = p_width.inPixels(mi.base.textwidth);
			}
			tabular.getCellInset(cell)->metrics(m, dim);
			maxAsc  = max(maxAsc, dim.asc);
			maxDesc = max(maxDesc, dim.des);
			tabular.setWidthOfCell(cell, dim.wid);
			++cell;
		}
		tabular.setAscentOfRow(i, maxAsc + ADD_TO_HEIGHT);
		tabular.setDescentOfRow(i, maxDesc + ADD_TO_HEIGHT);
	}

	dim.asc = tabular.getAscentOfRow(0);
	dim.des = tabular.getHeightOfTabular() - dim.asc;
	dim.wid = tabular.getWidthOfTabular() + 2 * ADD_TO_TABULAR_WIDTH;
	dim_ = dim;
}


void InsetTabular::draw(PainterInfo & pi, int x, int y) const
{
	setPosCache(pi, x, y);

	//lyxerr << "InsetTabular::draw: " << x << " " << y << endl;
	BufferView * bv = pi.base.bv;

	static NullPainter nop;
	static PainterInfo nullpi(bv, nop);

	resetPos(bv->cursor());

	x += scroll();
	x += ADD_TO_TABULAR_WIDTH;

	idx_type idx = 0;
	first_visible_cell = LyXTabular::npos;
	for (row_type i = 0; i < tabular.rows(); ++i) {
		int nx = x;
		int const a = tabular.getAscentOfRow(i);
		int const d = tabular.getDescentOfRow(i);
		idx = tabular.getCellNumber(i, 0);
		for (col_type j = 0; j < tabular.columns(); ++j) {
			if (tabular.isPartOfMultiColumn(i, j))
				continue;
			if (first_visible_cell == LyXTabular::npos)
				first_visible_cell = idx;

			int const cx = nx + tabular.getBeginningOfTextInCell(idx);
			if (nx + tabular.getWidthOfColumn(idx) < 0
			    || nx > bv->workWidth()
			    || y + d < 0
			    || y - a > bv->workHeight()) {
				cell(idx)->draw(nullpi, cx, y);
				drawCellLines(nop, nx, y, i, idx);
			} else {
				cell(idx)->draw(pi, cx, y);
				drawCellLines(pi.pain, nx, y, i, idx);
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

	LCursor & cur = pi.base.bv->cursor();
	if (!cur.selection())
		return;
	if (!ptr_cmp(&cur.inset(), this))
		return;

	resetPos(cur);

	x += scroll();
	x += ADD_TO_TABULAR_WIDTH;

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
							      LColor::selection);
				xx += w;

			}
			y += h;
		}

	} else {
		cur.text()->drawSelection(pi, x + getCellXPos(cur.idx()) + tabular.getBeginningOfTextInCell(cur.idx()), 0 /*this value is ignored */);
	}
}


void InsetTabular::drawCellLines(Painter & pain, int x, int y,
				 row_type row, idx_type cell) const
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


string const InsetTabular::editMessage() const
{
	return _("Opened table");
}


void InsetTabular::edit(LCursor & cur, bool left)
{
	lyxerr << "InsetTabular::edit: " << this << endl;
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
	// this accesses the position cache before it is initialized
	//resetPos(cur);
	//cur.bv().fitCursor();
}


void InsetTabular::doDispatch(LCursor & cur, FuncRequest & cmd)
{
	lyxerr << "# InsetTabular::dispatch: cmd: " << cmd << endl;
	//lyxerr << "  cur:\n" << cur << endl;
	CursorSlice sl = cur.top();
	LCursor & bvcur = cur.bv().cursor();

	switch (cmd.action) {

	case LFUN_MOUSE_PRESS:
		lyxerr << "# InsetTabular::MousePress\n" << cur.bv().cursor() << endl;

		if (cmd.button() == mouse_button::button1) {
			cur.selection() = false;
			setCursorFromCoordinates(cur, cmd.x, cmd.y);
			cur.resetAnchor();
			bvcur = cur;
			break;
		}

		if (cmd.button() == mouse_button::button2) {
			// FIXME: pasting multiple cells (with insettabular's own
			// LFUN_PASTESELECTION still does not work! (jspitzm)
			cmd = FuncRequest(LFUN_PASTESELECTION, "paragraph");
			cell(cur.idx())->dispatch(cur, cmd);
			break;
		}

		// we'll pop up the table dialog on release
		if (cmd.button() == mouse_button::button3)
			break;
		break;

	case LFUN_MOUSE_MOTION:
		lyxerr << "# InsetTabular::MouseMotion\n" << bvcur << endl;
		if (cmd.button() == mouse_button::button1) {
			// only accept motions to places not deeper nested than the real anchor
			if (bvcur.anchor_.hasPart(cur)) {
				setCursorFromCoordinates(cur, cmd.x, cmd.y);
				bvcur.setCursor(cur);
				bvcur.selection() = true;
			} else
				cur.undispatched();
		}
		break;

	case LFUN_MOUSE_RELEASE:
		lyxerr << "# InsetTabular::MouseRelease\n" << bvcur << endl;
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

	case LFUN_SCROLL_INSET:
		if (cmd.argument.empty())
			break;
		if (cmd.argument.find('.') != cmd.argument.npos)
			scroll(cur.bv(), static_cast<float>(convert<double>(cmd.argument)));
		else
			scroll(cur.bv(), convert<int>(cmd.argument));
		break;

	case LFUN_RIGHTSEL:
	case LFUN_RIGHT:
		cell(cur.idx())->dispatch(cur, cmd);
		cur.dispatched(); // override the cell's decision
		if (sl == cur.top())
			isRightToLeft(cur) ? movePrevCell(cur) : moveNextCell(cur);
		if (sl == cur.top()) {
			cmd = FuncRequest(LFUN_FINISHED_RIGHT);
			cur.undispatched();
			resetPos(cur);
		}
		break;

	case LFUN_LEFTSEL:
	case LFUN_LEFT:
		cell(cur.idx())->dispatch(cur, cmd);
		cur.dispatched(); // override the cell's decision
		if (sl == cur.top())
			isRightToLeft(cur) ? moveNextCell(cur) : movePrevCell(cur);
		if (sl == cur.top()) {
			cmd = FuncRequest(LFUN_FINISHED_LEFT);
			cur.undispatched();
			resetPos(cur);
		}
		break;

	case LFUN_DOWNSEL:
	case LFUN_DOWN:
		cell(cur.idx())->dispatch(cur, cmd);
		cur.dispatched(); // override the cell's decision
		if (sl == cur.top())
			if (tabular.row_of_cell(cur.idx()) != tabular.rows() - 1) {
				cur.idx() = tabular.getCellBelow(cur.idx());
				cur.pit() = 0;
				cur.pos() = 0;
				resetPos(cur);
			}
		if (sl == cur.top()) {
			cmd = FuncRequest(LFUN_FINISHED_DOWN);
			cur.undispatched();
			resetPos(cur);
		}
		break;

	case LFUN_UPSEL:
	case LFUN_UP:
		cell(cur.idx())->dispatch(cur, cmd);
		cur.dispatched(); // override the cell's decision
		if (sl == cur.top())
			if (tabular.row_of_cell(cur.idx()) != 0) {
				cur.idx() = tabular.getCellAbove(cur.idx());
				cur.pit() = cur.lastpit();
				cur.pos() = cur.lastpos();
				resetPos(cur);
			}
		if (sl == cur.top()) {
			cmd = FuncRequest(LFUN_FINISHED_UP);
			cur.undispatched();
			resetPos(cur);
		}
		break;

//	case LFUN_NEXT: {
//		//if (hasSelection())
//		//	cur.selection() = false;
//		col_type const col = tabular.column_of_cell(cur.idx());
//		int const t =	cur.bv().top_y() + cur.bv().painter().paperHeight();
//		if (t < yo() + tabular.getHeightOfTabular()) {
//			cur.bv().scrollDocView(t);
//			cur.idx() = tabular.getCellBelow(first_visible_cell) + col;
//		} else {
//			cur.idx() = tabular.getFirstCellInRow(tabular.rows() - 1) + col;
//		}
//		cur.par() = 0;
//		cur.pos() = 0;
//		resetPos(cur);
//		break;
//	}
//
//	case LFUN_PRIOR: {
//		//if (hasSelection())
//		//	cur.selection() = false;
//		col_type const col = tabular.column_of_cell(cur.idx());
//		int const t =	cur.bv().top_y() + cur.bv().painter().paperHeight();
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
//		resetPos(cur);
//		break;
//	}

	case LFUN_LAYOUT_TABULAR:
		InsetTabularMailer(*this).showDialog(&cur.bv());
		break;

	case LFUN_INSET_DIALOG_UPDATE:
		InsetTabularMailer(*this).updateDialog(&cur.bv());
		break;

	case LFUN_TABULAR_FEATURE:
		if (!tabularFeatures(cur, cmd.argument))
			cur.undispatched();
		break;

	// insert file functions
	case LFUN_FILE_INSERT_ASCII_PARA:
	case LFUN_FILE_INSERT_ASCII: {
		string const tmpstr = getContentsOfAsciiFile(&cur.bv(), cmd.argument, false);
		if (!tmpstr.empty() && !insertAsciiString(cur.bv(), tmpstr, false))
			cur.undispatched();
		break;
	}

	case LFUN_CUT:
		if (tablemode(cur)) {
			if (copySelection(cur)) {
				recordUndo(cur, Undo::DELETE);
				cutSelection(cur);
			}
		}
		else
			cell(cur.idx())->dispatch(cur, cmd);
		break;

	case LFUN_BACKSPACE:
	case LFUN_DELETE:
		recordUndo(cur, Undo::DELETE);
		if (tablemode(cur))
			cutSelection(cur);
		else
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

	case LFUN_PASTESELECTION: {
		string const clip = cur.bv().getClipboard();
		if (clip.empty())
			break;
		if (clip.find('\t') != string::npos) {
			col_type cols = 1;
			row_type rows = 1;
			col_type maxCols = 1;
			size_t len = clip.length();
			for (size_t p = 0; p < len; ++p) {
				p = clip.find_first_of("\t\n", p);
				if (p == string::npos)
					break;
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
			}
			maxCols = max(cols, maxCols);

			paste_tabular.reset(
				new LyXTabular(cur.buffer().params(), rows, maxCols));

			string::size_type op = 0;
			idx_type cell = 0;
			idx_type const cells =
				paste_tabular->getNumberOfCells();
			cols = 0;
			LyXFont font;
			for (size_t p = 0; cell < cells && p < len; ++p) {
				p = clip.find_first_of("\t\n", p);
				if (p == string::npos || p >= len)
					break;
				switch (clip[p]) {
				case '\t':
					paste_tabular->getCellInset(cell)->
						setText(clip.substr(op, p - op), font);
					++cols;
					++cell;
					break;
				case '\n':
					paste_tabular->getCellInset(cell)->
						setText(clip.substr(op, p - op), font);
					while (cols++ < maxCols)
						++cell;
					cols = 0;
					break;
				}
				op = p + 1;
			}
			// check for the last cell if there is no trailing '\n'
			if (cell < cells && op < len)
				paste_tabular->getCellInset(cell)->
					setText(clip.substr(op, len - op), font);
		} else if (!insertAsciiString(cur.bv(), clip, true)) {
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
			pasteSelection(cur);
			break;
		}
		cell(cur.idx())->dispatch(cur, cmd);
		break;

	case LFUN_EMPH:
	case LFUN_BOLD:
	case LFUN_ROMAN:
	case LFUN_NOUN:
	case LFUN_ITAL:
	case LFUN_FRAK:
	case LFUN_CODE:
	case LFUN_SANS:
	case LFUN_FREEFONT_APPLY:
	case LFUN_FREEFONT_UPDATE:
	case LFUN_FONT_SIZE:
	case LFUN_UNDERLINE:
	case LFUN_LANGUAGE:
	case LFUN_CAPITALIZE_WORD:
	case LFUN_UPCASE_WORD:
	case LFUN_LOWCASE_WORD:
	case LFUN_TRANSPOSE_CHARS:
		if (tablemode(cur)) {
			row_type rs, re;
			col_type cs, ce;
			getSelection(cur, rs, re, cs, ce);
			for (row_type i = rs; i <= re; ++i)
				for (col_type j = cs; j <= ce; ++j) {
					// cursor follows cell:
					cur.idx() = tabular.getCellNumber(i, j);
					// select this cell only:
					cur.pos() = 0;
				        cur.resetAnchor();
					cur.pos() = cur.top().lastpos();
				        cur.setCursor(cur);
				        cur.setSelection();
					cell(cur.idx())->dispatch(cur, cmd);
				}
			// Restore original selection
			cur.idx() = tabular.getCellNumber(rs, cs);
			cur.pos() = 0;
			cur.resetAnchor();
			cur.idx() = tabular.getCellNumber(re, ce);
			cur.pos() = cur.top().lastpos();
			cur.setCursor(cur);
			cur.setSelection();
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

	InsetTabularMailer(*this).updateDialog(&cur.bv());
}


// function sets an object as defined in func_status.h:
// states OK, Unknown, Disabled, On, Off.
bool InsetTabular::getStatus(LCursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
	case LFUN_TABULAR_FEATURE: {
		int action = LyXTabular::LAST_ACTION;
		int i = 0;
		for (; tabularFeature[i].action != LyXTabular::LAST_ACTION; ++i) {
			string const tmp = tabularFeature[i].feature;
			if (tmp == cmd.argument.substr(0, tmp.length())) {
				action = tabularFeature[i].action;
				break;
			}
		}
		if (action == LyXTabular::LAST_ACTION) {
			status.clear();
			status.unknown(true);
			return true;
		}

		string const argument
			= ltrim(cmd.argument.substr(tabularFeature[i].feature.length()));

		row_type sel_row_start = 0;
		row_type sel_row_end = 0;
		col_type dummy;
		LyXTabular::ltType dummyltt;
		bool flag = true;

		getSelection(cur, sel_row_start, sel_row_end, dummy, dummy);

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
			return true;

		case LyXTabular::MULTICOLUMN:
			status.setOnOff(tabular.isMultiColumn(cur.idx()));
			break;

		case LyXTabular::M_TOGGLE_LINE_TOP:
			flag = false;
		case LyXTabular::TOGGLE_LINE_TOP:
			status.setOnOff(tabular.topLine(cur.idx(), flag));
			break;

		case LyXTabular::M_TOGGLE_LINE_BOTTOM:
			flag = false;
		case LyXTabular::TOGGLE_LINE_BOTTOM:
			status.setOnOff(tabular.bottomLine(cur.idx(), flag));
			break;

		case LyXTabular::M_TOGGLE_LINE_LEFT:
			flag = false;
		case LyXTabular::TOGGLE_LINE_LEFT:
			status.setOnOff(tabular.leftLine(cur.idx(), flag));
			break;

		case LyXTabular::M_TOGGLE_LINE_RIGHT:
			flag = false;
		case LyXTabular::TOGGLE_LINE_RIGHT:
			status.setOnOff(tabular.rightLine(cur.idx(), flag));
			break;

		case LyXTabular::M_ALIGN_LEFT:
			flag = false;
		case LyXTabular::ALIGN_LEFT:
			status.setOnOff(tabular.getAlignment(cur.idx(), flag) == LYX_ALIGN_LEFT);
			break;

		case LyXTabular::M_ALIGN_RIGHT:
			flag = false;
		case LyXTabular::ALIGN_RIGHT:
			status.setOnOff(tabular.getAlignment(cur.idx(), flag) == LYX_ALIGN_RIGHT);
			break;

		case LyXTabular::M_ALIGN_CENTER:
			flag = false;
		case LyXTabular::ALIGN_CENTER:
			status.setOnOff(tabular.getAlignment(cur.idx(), flag) == LYX_ALIGN_CENTER);
			break;

		case LyXTabular::ALIGN_BLOCK:
			status.enabled(!tabular.getPWidth(cur.idx()).zero());
			status.setOnOff(tabular.getAlignment(cur.idx(), flag) == LYX_ALIGN_BLOCK);
			break;

		case LyXTabular::M_VALIGN_TOP:
			flag = false;
		case LyXTabular::VALIGN_TOP:
			status.setOnOff(
				tabular.getVAlignment(cur.idx(), flag) == LyXTabular::LYX_VALIGN_TOP);
			break;

		case LyXTabular::M_VALIGN_BOTTOM:
			flag = false;
		case LyXTabular::VALIGN_BOTTOM:
			status.setOnOff(
				tabular.getVAlignment(cur.idx(), flag) == LyXTabular::LYX_VALIGN_BOTTOM);
			break;

		case LyXTabular::M_VALIGN_MIDDLE:
			flag = false;
		case LyXTabular::VALIGN_MIDDLE:
			status.setOnOff(
				tabular.getVAlignment(cur.idx(), flag) == LyXTabular::LYX_VALIGN_MIDDLE);
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
			status.setOnOff(tabular.getRotateCell(cur.idx()));
			break;

		case LyXTabular::UNSET_ROTATE_CELL:
			status.setOnOff(!tabular.getRotateCell(cur.idx()));
			break;

		case LyXTabular::SET_USEBOX:
			status.setOnOff(convert<int>(argument) == tabular.getUsebox(cur.idx()));
			break;

		case LyXTabular::SET_LTFIRSTHEAD:
			status.setOnOff(tabular.getRowOfLTHead(sel_row_start, dummyltt));
			break;

		case LyXTabular::UNSET_LTFIRSTHEAD:
			status.setOnOff(!tabular.getRowOfLTHead(sel_row_start, dummyltt));
			break;

		case LyXTabular::SET_LTHEAD:
			status.setOnOff(tabular.getRowOfLTHead(sel_row_start, dummyltt));
			break;

		case LyXTabular::UNSET_LTHEAD:
			status.setOnOff(!tabular.getRowOfLTHead(sel_row_start, dummyltt));
			break;

		case LyXTabular::SET_LTFOOT:
			status.setOnOff(tabular.getRowOfLTFoot(sel_row_start, dummyltt));
			break;

		case LyXTabular::UNSET_LTFOOT:
			status.setOnOff(!tabular.getRowOfLTFoot(sel_row_start, dummyltt));
			break;

		case LyXTabular::SET_LTLASTFOOT:
			status.setOnOff(tabular.getRowOfLTFoot(sel_row_start, dummyltt));
			break;

		case LyXTabular::UNSET_LTLASTFOOT:
			status.setOnOff(!tabular.getRowOfLTFoot(sel_row_start, dummyltt));
			break;

		case LyXTabular::SET_LTNEWPAGE:
			status.setOnOff(tabular.getLTNewPage(sel_row_start));
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
	case LFUN_INSERT_CHARSTYLE:
	case LFUN_INSET_FLOAT:
	case LFUN_INSET_WIDE_FLOAT:
	case LFUN_INSET_FOOTNOTE:
	case LFUN_INSET_MARGINAL:
	case LFUN_INSERT_MATH:
	case LFUN_MATH_MODE:
	case LFUN_MATH_MUTATE:
	case LFUN_MATH_DISPLAY:
	case LFUN_INSERT_NOTE:
	case LFUN_INSET_OPTARG:
	case LFUN_INSERT_BOX:
	case LFUN_INSERT_BRANCH:
	case LFUN_INSET_WRAP:
	case LFUN_INSET_ERT: {
		if (tablemode(cur)) {
			status.enabled(false);
			return true;
		}
	}

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


int InsetTabular::latex(Buffer const & buf, ostream & os,
			OutputParams const & runparams) const
{
	return tabular.latex(buf, os, runparams);
}


int InsetTabular::plaintext(Buffer const & buf, ostream & os,
			OutputParams const & runparams) const
{
	int const dp = runparams.linelen ? runparams.depth : 0;
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
	InsetBase * master = 0;

#ifdef WITH_WARNINGS
#warning Why not pass a proper DocIterator here?
#endif
#if 0
	// if the table is inside a float it doesn't need the informaltable
	// wrapper. Search for it.
	for (master = owner(); master; master = master->owner())
		if (master->lyxCode() == InsetBase::FLOAT_CODE)
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


void InsetTabular::getCursorPos(CursorSlice const & sl, int & x, int & y) const
{
	cell(sl.idx())->getCursorPos(sl, x, y);

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
	x += scroll();
}


namespace  {


// Manhattan distance to nearest corner
int dist(InsetOld const & inset, int x, int y)
{
	int xx = 0;
	int yy = 0;
	Point o = theCoords.getInsets().xy(&inset);
	int const xo = o.x_;
	int const yo = o.y_;

	if (x < xo)
		xx = xo - x;
	else if (x > xo + inset.width())
		xx = x - xo - inset.width();

	if (y < yo - inset.ascent())
		yy = yo - inset.ascent() - y;
	else if (y > yo + inset.descent())
		yy = y - yo - inset.descent();

	lyxerr << " xo_=" << xo << "  yo_=" << yo
	       << " width_=" << inset.width() << " ascent=" << inset.ascent()
	       << " descent=" << inset.descent()
	       << " dist=" << xx + yy << endl;
	return xx + yy;
}


} //namespace anon


InsetBase * InsetTabular::editXY(LCursor & cur, int x, int y) const
{
	//lyxerr << "InsetTabular::editXY: " << this << endl;
	cur.selection() = false;
	cur.push(const_cast<InsetTabular&>(*this));
	cur.idx() = getNearestCell(x, y);
	resetPos(cur);
	return cell(cur.idx())->text_.editXY(cur, x, y);
	//int xx = cursorx_ - xo() + tabular.getBeginningOfTextInCell(cur.idx());
}


void InsetTabular::setCursorFromCoordinates(LCursor & cur, int x, int y) const
{
	//lyxerr << "# InsetTabular::setCursorFromCoordinates()\n" << cur << endl;
	cur.idx() = getNearestCell(x, y);
	resetPos(cur);
	return cell(cur.idx())->text_.setCursorFromCoordinates(cur, x, y);
}


InsetTabular::idx_type InsetTabular::getNearestCell(int x, int y) const
{
	lyxerr << "# InsetTabular::getNearestCell()  x=" << x << " y=" << y << endl;
	idx_type idx_min = 0;
	int dist_min = std::numeric_limits<int>::max();
	for (idx_type i = 0; i < nargs(); ++i) {
		if (theCoords.getInsets().has(tabular.getCellInset(i).get())) {
			int d = dist(*tabular.getCellInset(i), x, y);
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
	int lx = tabular.getWidthOfColumn(cell);
	for (; c < cell; ++c)
		lx += tabular.getWidthOfColumn(c);

	return lx - tabular.getWidthOfColumn(cell);
}


void InsetTabular::resetPos(LCursor & cur) const
{
	BufferView & bv = cur.bv();
//	col_type const actcol = tabular.column_of_cell(cur.idx());
//	int const offset = ADD_TO_TABULAR_WIDTH + 2;
//	int const new_x = getCellXPos(cur.idx()) + offset;
//	int const old_x = cursorx_;
//	int const col_width = tabular.getWidthOfColumn(cur.idx());
//	cursorx_ = new_x;
//    cursor.x(getCellXPos(cur.idx()) + offset);
//	if (actcol < tabular.columns() - 1 && scroll(false) &&
//		tabular.getWidthOfTabular() < bv.workWidth()-20)
//	{
//		scroll(bv, 0.0F);
//	} else if (cursorx_ - offset > 20 &&
//		   cursorx_ - offset + col_width > bv.workWidth() - 20) {
//		scroll(bv, - col_width - 20);
//	} else if (cursorx_ - offset < 20) {
//		scroll(bv, 20 - cursorx_ + offset);
//	} else if (scroll() && xo() > 20 &&
//		   xo() + tabular.getWidthOfTabular() > bv.workWidth() - 20) {
//		scroll(bv, old_x - cursorx_);
//	}

	if (&cur.inset() != this) {
		scroll(bv, 0.0F);
	} else {
		int const X1 = 0;
		int const X2 = bv.workWidth();
		int const offset = ADD_TO_TABULAR_WIDTH + 2;
		int const x1 = xo() + scroll() + getCellXPos(cur.idx()) + offset;
		int const x2 = x1 + tabular.getWidthOfColumn(cur.idx());

		if (x1 < X1)
			scroll(bv, X1 + 20 - x1);
		else if (x2 > X2)
			scroll(bv, X2 - 20 - x2);
	}

	cur.needsUpdate();

	InsetTabularMailer(*this).updateDialog(&bv);
}


void InsetTabular::moveNextCell(LCursor & cur)
{
	lyxerr << "InsetTabular::moveNextCell 1 cur: " << cur.top() << endl;
	if (isRightToLeft(cur)) {
		lyxerr << "InsetTabular::moveNextCell A cur: " << endl;
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
		lyxerr << "InsetTabular::moveNextCell B cur: " << endl;
		if (tabular.isLastCell(cur.idx()))
			return;
		++cur.idx();
	}
	cur.pit() = 0;
	cur.pos() = 0;
	lyxerr << "InsetTabular::moveNextCell 2 cur: " << cur.top() << endl;
	resetPos(cur);
}


void InsetTabular::movePrevCell(LCursor & cur)
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
	cur.pit() = cur.lastpit();
	cur.pos() = cur.lastpos();
	resetPos(cur);
}


bool InsetTabular::tabularFeatures(LCursor & cur, string const & what)
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
	col_type sel_col_start;
	col_type sel_col_end;
	row_type sel_row_start;
	row_type sel_row_end;
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

	recordUndo(cur, Undo::ATOMIC);

	getSelection(cur, sel_row_start, sel_row_end, sel_col_start, sel_col_end);
	row_type const row = tabular.row_of_cell(cur.idx());
	col_type const column = tabular.column_of_cell(cur.idx());
	bool flag = true;
	LyXTabular::ltType ltt;

	switch (feature) {

	case LyXTabular::SET_PWIDTH: {
		LyXLength const len(value);
		tabular.setColumnPWidth(cur.idx(), len);
		if (len.zero()
		    && tabular.getAlignment(cur.idx(), true) == LYX_ALIGN_BLOCK)
			tabularFeatures(cur, LyXTabular::ALIGN_CENTER, string());
		else if (!len.zero()
			 && tabular.getAlignment(cur.idx(), true) != LYX_ALIGN_BLOCK)
			tabularFeatures(cur, LyXTabular::ALIGN_BLOCK, string());
		break;
	}

	case LyXTabular::SET_MPWIDTH:
		tabular.setMColumnPWidth(cur.idx(), LyXLength(value));
		break;

	case LyXTabular::SET_SPECIAL_COLUMN:
	case LyXTabular::SET_SPECIAL_MULTI:
		tabular.setAlignSpecial(cur.idx(),value,feature);
		break;

	case LyXTabular::APPEND_ROW:
		// append the row into the tabular
		tabular.appendRow(bv.buffer()->params(), cur.idx());
		break;

	case LyXTabular::APPEND_COLUMN:
		// append the column into the tabular
		tabular.appendColumn(bv.buffer()->params(), cur.idx());
		break;

	case LyXTabular::DELETE_ROW:
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			tabular.deleteRow(sel_row_start);
		if (sel_row_start >= tabular.rows())
			--sel_row_start;
		cur.idx() = tabular.getCellNumber(sel_row_start, column);
		cur.pit() = 0;
		cur.pos() = 0;
		cur.selection() = false;
		break;

	case LyXTabular::DELETE_COLUMN:
		for (col_type i = sel_col_start; i <= sel_col_end; ++i)
			tabular.deleteColumn(sel_col_start);
		if (sel_col_start >= tabular.columns())
			--sel_col_start;
		cur.idx() = tabular.getCellNumber(row, sel_col_start);
		cur.pit() = 0;
		cur.pos() = 0;
		cur.selection() = false;
		break;

	case LyXTabular::M_TOGGLE_LINE_TOP:
		flag = false;
	case LyXTabular::TOGGLE_LINE_TOP: {
		bool lineSet = !tabular.topLine(cur.idx(), flag);
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setTopLine(
					tabular.getCellNumber(i, j),
					lineSet, flag);
		break;
	}

	case LyXTabular::M_TOGGLE_LINE_BOTTOM:
		flag = false;
	case LyXTabular::TOGGLE_LINE_BOTTOM: {
		bool lineSet = !tabular.bottomLine(cur.idx(), flag);
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setBottomLine(
					tabular.getCellNumber(i, j),
					lineSet,
					flag);
		break;
	}

	case LyXTabular::M_TOGGLE_LINE_LEFT:
		flag = false;
	case LyXTabular::TOGGLE_LINE_LEFT: {
		bool lineSet = !tabular.leftLine(cur.idx(), flag);
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setLeftLine(
					tabular.getCellNumber(i,j),
					lineSet,
					flag);
		break;
	}

	case LyXTabular::M_TOGGLE_LINE_RIGHT:
		flag = false;
	case LyXTabular::TOGGLE_LINE_RIGHT: {
		bool lineSet = !tabular.rightLine(cur.idx(), flag);
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
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
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
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
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
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

	case LyXTabular::SET_ALL_LINES:
		setLines = true;
	case LyXTabular::UNSET_ALL_LINES:
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
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
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setRotateCell(
					tabular.getCellNumber(i, j), true);
		break;

	case LyXTabular::UNSET_ROTATE_CELL:
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setRotateCell(
					tabular.getCellNumber(i, j), false);
		break;

	case LyXTabular::SET_USEBOX: {
		LyXTabular::BoxType val = LyXTabular::BoxType(convert<int>(value));
		if (val == tabular.getUsebox(cur.idx()))
			val = LyXTabular::BOX_NONE;
		for (row_type i = sel_row_start; i <= sel_row_end; ++i)
			for (col_type j = sel_col_start; j <= sel_col_end; ++j)
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

	InsetTabularMailer(*this).updateDialog(&bv);
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


void InsetTabular::getLabelList(Buffer const & buffer,
				vector<string> & list) const
{
	tabular.getLabelList(buffer, list);
}


bool InsetTabular::copySelection(LCursor & cur)
{
	if (!cur.selection())
		return false;

	row_type rs, re;
	col_type cs, ce;
	getSelection(cur, rs, re, cs, ce);

	paste_tabular.reset(new LyXTabular(tabular));

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

	ostringstream os;
	OutputParams const runparams;
	paste_tabular->plaintext(cur.buffer(), os, runparams, 0, true, '\t');
	cur.bv().stuffClipboard(os.str());
	return true;
}


bool InsetTabular::pasteSelection(LCursor & cur)
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
			shared_ptr<InsetText> inset = tabular.getCellInset(r2, c2);
			inset = paste_tabular->getCellInset(r1, c1);
			inset->markNew();
		}
	}
	return true;
}


void InsetTabular::cutSelection(LCursor & cur)
{
	if (!cur.selection())
		return;

	bool const track = cur.buffer().params().tracking_changes;
	row_type rs, re;
	col_type cs, ce;
	getSelection(cur, rs, re, cs, ce);
	for (row_type i = rs; i <= re; ++i)
		for (col_type j = cs; j <= ce; ++j)
			cell(tabular.getCellNumber(i, j))->clear(track);

	// cursor position might be invalid now
	cur.pos() = cur.lastpos();
	cur.clearSelection();
}


bool InsetTabular::isRightToLeft(LCursor & cur) const
{
	BOOST_ASSERT(cur.depth() > 1);
	Paragraph const & parentpar = cur[cur.depth() - 2].paragraph();
	LCursor::pos_type const parentpos = cur[cur.depth() - 2].pos();
	return parentpar.getFontSettings(cur.bv().buffer()->params(),
					 parentpos).language()->RightToLeft();
}


void InsetTabular::getSelection(LCursor & cur,
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


size_t InsetTabular::nargs() const
{
	return tabular.getNumberOfCells();
}


LyXText * InsetTabular::getText(int idx) const
{
	return size_t(idx) < nargs() ? cell(idx)->getText(0) : 0;
}


void InsetTabular::markErased()
{
	for (idx_type idx = 0; idx < nargs(); ++idx)
		cell(idx)->markErased();
}


bool InsetTabular::forceDefaultParagraphs(InsetBase const *) const
{
#if 0
	idx_type const cell = tabular.getCellFromInset(in);
	// FIXME: getCellFromInset() returns now always a valid cell, so
	// the stuff below can be deleted, and instead we have:
	return tabular.getPWidth(cell).zero();

	if (cell != npos)
		return tabular.getPWidth(cell).zero();

	// this is a workaround for a crash (New, Insert->Tabular,
	// Insert->FootNote)
	if (!owner())
		return false;

	// well we didn't obviously find it so maybe our owner knows more
	BOOST_ASSERT(owner());
	return owner()->forceDefaultParagraphs(in);
#endif
	return false;
}


bool InsetTabular::insertAsciiString(BufferView & bv, string const & buf,
				     bool usePaste)
{
	if (buf.length() <= 0)
		return true;

	col_type cols = 1;
	row_type rows = 1;
	col_type maxCols = 1;
	string::size_type const len = buf.length();
	string::size_type p = 0;

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
	idx_type cell = 0;
	col_type ocol = 0;
	row_type row = 0;
	if (usePaste) {
		paste_tabular.reset(
			new LyXTabular(bv.buffer()->params(), rows, maxCols));
		loctab = paste_tabular.get();
		cols = 0;
	} else {
		loctab = &tabular;
		cell = bv.cursor().idx();
		ocol = tabular.column_of_cell(cell);
		row = tabular.row_of_cell(cell);
	}

	string::size_type op = 0;
	idx_type const cells = loctab->getNumberOfCells();
	p = 0;
	cols = ocol;
	rows = loctab->rows();
	col_type const columns = loctab->columns();

	while (cell < cells && p < len && row < rows &&
	       (p = buf.find_first_of("\t\n", p)) != string::npos)
	{
		if (p >= len)
			break;
		switch (buf[p]) {
		case '\t':
			// we can only set this if we are not too far right
			if (cols < columns) {
				shared_ptr<InsetText> inset = loctab->getCellInset(cell);
				Paragraph & par = inset->text_.getPar(0);
				LyXFont const font = inset->text_.getFont(par, 0);
				inset->setText(buf.substr(op, p - op), font);
				++cols;
				++cell;
			}
			break;
		case '\n':
			// we can only set this if we are not too far right
			if (cols < columns) {
				shared_ptr<InsetText> inset = tabular.getCellInset(cell);
				Paragraph & par = inset->text_.getPar(0);
				LyXFont const font = inset->text_.getFont(par, 0);
				inset->setText(buf.substr(op, p - op), font);
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
		LyXFont const font = inset->text_.getFont(par, 0);
		inset->setText(buf.substr(op, len - op), font);
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


bool InsetTabular::tablemode(LCursor & cur) const
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
	LyXLex lex(0,0);
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
