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
	  buffer_(&buf), cursorx_(0)
{
	tabular.setOwner(this);
}


InsetTabular::InsetTabular(InsetTabular const & tab)
	: UpdatableInset(tab), tabular(tab.tabular),
		buffer_(tab.buffer_), cursorx_(0)
{
	tabular.setOwner(this);
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

	dim.asc = tabular.getAscentOfRow(0);
	dim.des = tabular.getHeightOfTabular() - tabular.getAscentOfRow(0) + 1;
	dim.wid = tabular.getWidthOfTabular() + 2 * ADD_TO_TABULAR_WIDTH;
	dim_ = dim;
}


void InsetTabular::draw(PainterInfo & pi, int x, int y) const
{
	//lyxerr << "InsetTabular::draw: " << x << " " << y << endl;

	BufferView * bv = pi.base.bv;
	setPosCache(pi, x, y);

	if (!owner())
		x += scroll();

	x += ADD_TO_TABULAR_WIDTH;

	int idx = 0;
	first_visible_cell = -1;
	for (int i = 0; i < tabular.rows(); ++i) {
		int nx = x;
		idx = tabular.getCellNumber(i, 0);
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
				first_visible_cell = idx;
			if (bv->cursor().selection())
				drawCellSelection(pi, nx, y, i, j, idx);

			int const cx = nx + tabular.getBeginningOfTextInCell(idx);
			cell(idx).draw(pi, cx, y);
			drawCellLines(pi.pain, nx, y, i, idx);
			nx += tabular.getWidthOfColumn(idx);
			++idx;
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


void InsetTabular::drawCellSelection(PainterInfo & pi, int x, int y,
				     int row, int column, int cell) const
{
	LCursor & cur = pi.base.bv->cursor();
	BOOST_ASSERT(cur.selection());
	if (tablemode(cur)) {
		int rs, re, cs, ce;
		getSelection(cur, rs, re, cs, ce);
		if (column >= cs && column <= ce && row >= rs && row <= re) {
			int w = tabular.getWidthOfColumn(cell);
			int h = tabular.getAscentOfRow(row) + tabular.getDescentOfRow(row)-1;
			pi.pain.fillRectangle(x, y - tabular.getAscentOfRow(row) + 1,
						 w, h, LColor::selection);
		}
	}
}


string const InsetTabular::editMessage() const
{
	return _("Opened table");
}


void InsetTabular::edit(LCursor & cur, bool left)
{
	lyxerr << "InsetTabular::edit: " << this << endl;
	finishUndo();
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
	cur.selection() = false;
	resetPos(cur);
	cur.bv().fitCursor();
	cur.push(*this);
	cur.idx() = cell;
}


InsetBase * InsetTabular::editXY(LCursor & cur, int x, int y)
{
	//lyxerr << "InsetTabular::editXY: " << this << endl;
	cur.selection() = false;
	cur.push(*this);
	return setPos(cur, x, y);
	//int xx = cursorx_ - xo_ + tabular.getBeginningOfTextInCell(actcell);
}


void InsetTabular::priv_dispatch(LCursor & cur, FuncRequest & cmd)
{
	lyxerr << "# InsetTabular::dispatch: cmd: " << cmd << endl;
	//lyxerr << "  cur:\n" << cur << endl;
	CursorSlice sl = cur.top();

	switch (cmd.action) {

	case LFUN_MOUSE_PRESS:
		// we'll pop up the table dialog on release
		if (cmd.button() == mouse_button::button3)
			break;
		cur.selection() = false;
		setPos(cur, cmd.x, cmd.y);
		cur.resetAnchor();
		cur.bv().cursor().setCursor(cur, false);
		//if (cmd.button() == mouse_button::button2)
		//	dispatch(cur, FuncRequest(LFUN_PASTESELECTION, "paragraph"));
		//lyxerr << "# InsetTabular::MousePress\n" << cur.bv().cursor() << endl;
		break;

	case LFUN_MOUSE_MOTION:
		if (cmd.button() != mouse_button::button1)
			break;
		setPos(cur, cmd.x, cmd.y);
		cur.bv().cursor().setCursor(cur, true);
		//lyxerr << "# InsetTabular::MouseMotion\n" << cur.bv().cursor() << endl;
		break;

	case LFUN_MOUSE_RELEASE:
		//lyxerr << "# InsetTabular::MouseRelease\n" << cur.bv().cursor() << endl;
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
			scroll(cur.bv(), static_cast<float>(strToDbl(cmd.argument)));
		else
			scroll(cur.bv(), strToInt(cmd.argument));
		cur.update();
		break;

	case LFUN_RIGHTSEL:
	case LFUN_RIGHT:
		cell(cur.idx()).dispatch(cur, cmd);
		if (sl == cur.top())
			isRightToLeft(cur) ? movePrevCell(cur) : moveNextCell(cur);
		if (sl == cur.top()) {
			cmd = FuncRequest(LFUN_FINISHED_RIGHT);
			cur.undispatched();
		}
		break;

	case LFUN_LEFTSEL: 
	case LFUN_LEFT:
		cell(cur.idx()).dispatch(cur, cmd);
		if (sl == cur.top())
			isRightToLeft(cur) ? moveNextCell(cur) : movePrevCell(cur);
		if (sl == cur.top()) {
			cmd = FuncRequest(LFUN_FINISHED_LEFT);
			cur.undispatched();
		}
		break;

	case LFUN_DOWNSEL:
	case LFUN_DOWN:
		cell(cur.idx()).dispatch(cur, cmd);
		if (sl == cur.top())
			if (tabular.row_of_cell(cur.idx()) != tabular.rows() - 1) {
				cur.idx() = tabular.getCellBelow(cur.idx());
				cur.par() = 0;
				cur.pos() = 0;
				resetPos(cur);
			}
		if (sl == cur.top()) {
			cmd = FuncRequest(LFUN_FINISHED_DOWN);
			cur.undispatched();
		}
		break;

	case LFUN_UPSEL:
	case LFUN_UP:
		cell(cur.idx()).dispatch(cur, cmd);
		if (sl == cur.top())
			if (tabular.row_of_cell(cur.idx()) != 0) {
				cur.idx() = tabular.getCellAbove(cur.idx());
				cur.par() = cur.lastpar();
				cur.pos() = cur.lastpos();
				resetPos(cur);
			}
		if (sl == cur.top()) {
			cmd = FuncRequest(LFUN_FINISHED_UP);
			cur.undispatched();
		}
		break;

	case LFUN_NEXT: {
		//if (hasSelection())
		//	cur.selection() = false;
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
		//if (hasSelection())
		//	cur.selection() = false;
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
		string tmpstr = getContentsOfAsciiFile(&cur.bv(), cmd.argument, false);
		if (!tmpstr.empty() && !insertAsciiString(cur.bv(), tmpstr, false))
			cur.undispatched();
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
		if (copySelection(cur)) {
			recordUndo(cur, Undo::DELETE);
			cutSelection(cur);
		}
		break;

	case LFUN_BACKSPACE:
	case LFUN_DELETE:
		recordUndo(cur, Undo::DELETE);
		if (tablemode(cur))
			cutSelection(cur);
		else
			cell(cur.idx()).dispatch(cur, cmd);
		break;

	case LFUN_COPY:
		if (!cur.selection())
			break;
		finishUndo();
		copySelection(cur);
		break;

	case LFUN_PASTESELECTION: {
		string const clip = cur.bv().getClipboard();
		if (clip.empty())
			break;
		if (clip.find('\t') != string::npos) {
			int cols = 1;
			int rows = 1;
			int maxCols = 1;
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
				new LyXTabular(cur.bv().buffer()->params(), rows, maxCols));

			string::size_type op = 0;
			int cell = 0;
			int cells = paste_tabular->getNumberOfCells();
			cols = 0;
			LyXFont font;
			for (size_t p = 0; cell < cells && p < len; ++p) {
				p = clip.find_first_of("\t\n", p);
				if (p == string::npos || p >= len)
					break;
				switch (clip[p]) {
				case '\t':
					paste_tabular->getCellInset(cell).
						setText(clip.substr(op, p - op), font);
					++cols;
					++cell;
					break;
				case '\n':
					paste_tabular->getCellInset(cell).
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
				paste_tabular->getCellInset(cell).
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
		cell(cur.idx()).dispatch(cur, cmd);
		break;

	default:
		// we try to handle this event in the insets dispatch function.
		cell(cur.idx()).dispatch(cur, cmd);
		break;
	}

	InsetTabularMailer(*this).updateDialog(&cur.bv());
}


bool InsetTabular::getStatus(LCursor & cur, FuncRequest const & cmd,
	FuncStatus & flag) const
{
	switch (cmd.action) {
	case LFUN_TABULAR_FEATURE:
#if 0
		if (cur.inMathed()) {
			// FIXME: check temporarily disabled
			// valign code
			char align = mathcursor::valign();
			if (align == '\0') {
				enable = false;
				break;
			}
			if (cmd.argument.empty()) {
				flag.clear();
				break;
			}
			if (!contains("tcb", cmd.argument[0])) {
				enable = false;
				break;
			}
			flag.setOnOff(cmd.argument[0] == align);
		} else {
			enable = false;

			char const align = mathcursor::halign();
			if (align == '\0') {
				enable = false;
				break;
			}
			if (cmd.argument.empty()) {
				flag.clear();
				break;
			}
			if (!contains("lcr", cmd.argument[0])) {
				enable = false;
				break;
			}
			flag.setOnOff(cmd.argument[0] == align);

			disable = !mathcursor::halign();
			break;
		}

			FuncStatus ret;
			//ret.disabled(true);
			InsetTabular * tab = static_cast<InsetTabular *>
				(cur.innerInsetOfType(InsetBase::TABULAR_CODE));
			if (tab) {
				ret = tab->getStatus(cmd.argument);
				flag |= ret;
				enable = true;
			} else {
				enable = false;
			}
		} else {
			static InsetTabular inset(*buf, 1, 1);
			enable = false;
			FuncStatus ret = inset.getStatus(cmd.argument);
			if (ret.onoff(true) || ret.onoff(false))
				flag.setOnOff(false);
		}
#endif
		return true;

	default:
		// we try to handle this event in the insets dispatch function.
		return cell(cur.idx()).getStatus(cur, cmd, flag);
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


InsetText const & InsetTabular::cell(int idx) const
{
	return tabular.getCellInset(idx);
}


InsetText & InsetTabular::cell(int idx)
{
	return tabular.getCellInset(idx);
}


void InsetTabular::getCursorPos(CursorSlice const & cur, int & x, int & y) const
{
	cell(cur.idx()).getCursorPos(cur, x, y);
}


InsetBase * InsetTabular::setPos(LCursor & cur, int x, int y) const
{
	int idx_min = 0;
	int dist_min = 1000000;
	for (idx_type i = 0; i < nargs(); ++i) {
		int d = getText(i)->dist(x, y);
		if (d < dist_min) {
			dist_min = d;
			idx_min = i;
		}
	}
	cur.idx() = idx_min;
	InsetBase * inset = cell(cur.idx()).text_.editXY(cur, x, y);
	//lyxerr << "# InsetTabular::setPos()\n" << cur << endl;
	return inset;
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


void InsetTabular::resetPos(LCursor & cur) const
{
	BufferView & bv = cur.bv();
	int actcell = cur.idx();
	int actcol = tabular.column_of_cell(actcell);

	int const offset = ADD_TO_TABULAR_WIDTH + 2;
	int new_x = getCellXPos(actcell) + offset;
	int old_x = cursorx_;
	cursorx_ = new_x;
//    cursor.x(getCellXPos(actcell) + offset);
	if (actcol < tabular.columns() - 1 && scroll(false) &&
		tabular.getWidthOfTabular() < bv.workWidth()-20)
	{
		scroll(bv, 0.0F);
	} else if (cursorx_ - offset > 20 &&
		   cursorx_ - offset + tabular.getWidthOfColumn(actcell)
		   > bv.workWidth() - 20) {
		scroll(bv, - tabular.getWidthOfColumn(actcell) - 20);
	} else if (cursorx_ - offset < 20) {
		scroll(bv, 20 - cursorx_ + offset);
	} else if (scroll() && xo_ > 20 &&
		   xo_ + tabular.getWidthOfTabular() > bv.workWidth() - 20) {
		scroll(bv, old_x - cursorx_);
	}

	InsetTabularMailer(*this).updateDialog(&bv);
}


void InsetTabular::moveNextCell(LCursor & cur)
{
	lyxerr << "InsetTabular::moveNextCell 1 cur: " << cur << endl;
	if (isRightToLeft(cur)) {
		if (tabular.isFirstCellInRow(cur.idx())) {
			int row = tabular.row_of_cell(cur.idx());
			if (row == tabular.rows() - 1)
				return;
			cur.idx() = tabular.getLastCellInRow(row);
			cur.idx() = tabular.getCellBelow(cur.idx());
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
	cur.par() = 0;
	cur.pos() = 0;
	lyxerr << "InsetTabular::moveNextCell 2 cur: " << cur << endl;
	resetPos(cur);
}


void InsetTabular::movePrevCell(LCursor & cur)
{
	if (isRightToLeft(cur)) {
		if (tabular.isLastCellInRow(cur.idx())) {
			int row = tabular.row_of_cell(cur.idx());
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
	cur.par() = 0;
	cur.pos() = 0;
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

	recordUndo(cur, Undo::ATOMIC);

	getSelection(cur, sel_row_start, sel_row_end, sel_col_start, sel_col_end);
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
		cur.selection() = false;
		break;

	case LyXTabular::DELETE_COLUMN:
		for (int i = sel_col_start; i <= sel_col_end; ++i)
			tabular.deleteColumn(sel_col_start);
		if (sel_col_start >= tabular.columns())
			--sel_col_start;
		actcell = tabular.getCellNumber(row, sel_col_start);
		cur.selection() = false;
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
#if 0
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
#endif
		cur.selection() = false;
		break;
	}

	case LyXTabular::SET_ALL_LINES:
		setLines = true;
	case LyXTabular::UNSET_ALL_LINES:
#if 0
		for (int i = sel_row_start; i <= sel_row_end; ++i)
			for (int j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setAllLines(
					tabular.getCellNumber(i,j), setLines);
#endif
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


//
// function returns an object as defined in func_status.h:
// states OK, Unknown, Disabled, On, Off.
//
FuncStatus InsetTabular::getStatus(BufferView & bv,
	string const & what, int actcell) const
{
	FuncStatus status;
	int action = LyXTabular::LAST_ACTION;	
	LCursor & cur = bv.cursor();

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

	int sel_row_start = 0;
	int sel_row_end = 0;
	int dummy;
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
		status.enabled(!tabular.getPWidth(actcell).zero());
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
		status.enabled(false);
		break;
	}
	return status;
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

	int rs, re, cs, ce;
	getSelection(cur, rs, re, cs, ce);

	paste_tabular.reset(new LyXTabular(tabular));
	paste_tabular->setOwner(this);

	for (int i = 0; i < rs; ++i)
		paste_tabular->deleteRow(0);

	int const rows = re - rs + 1;
	while (paste_tabular->rows() > rows)
		paste_tabular->deleteRow(rows);

	paste_tabular->setTopLine(0, true, true);
	paste_tabular->setBottomLine(paste_tabular->getFirstCellInRow(rows - 1),
				     true, true);

	for (int i = 0; i < cs; ++i)
		paste_tabular->deleteColumn(0);

	int const columns = ce - cs + 1;
	while (paste_tabular->columns() > columns)
		paste_tabular->deleteColumn(columns);

	paste_tabular->setLeftLine(0, true, true);
	paste_tabular->setRightLine(paste_tabular->getLastCellInRow(0),
				    true, true);

	ostringstream os;
	OutputParams const runparams;
	paste_tabular->plaintext(*cur.bv().buffer(), os, runparams, 0, true, '\t');
	cur.bv().stuffClipboard(os.str());
	return true;
}


bool InsetTabular::pasteSelection(LCursor & cur)
{
	if (!paste_tabular)
		return false;
	int actcell = cur.idx();
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


void InsetTabular::cutSelection(LCursor & cur)
{
	if (!cur.selection())
		return;

	bool const track = cur.bv().buffer()->params().tracking_changes;
	int rs, re, cs, ce;
	getSelection(cur, rs, re, cs, ce); 
	for (int i = rs; i <= re; ++i)
		for (int j = cs; j <= ce; ++j)
			cell(tabular.getCellNumber(i, j)).clear(track);
}


bool InsetTabular::isRightToLeft(LCursor & cur)
{
	return cur.bv().getParentLanguage(this)->RightToLeft();
}


void InsetTabular::getSelection(LCursor & cur,
	int & rs, int & re, int & cs, int & ce) const
{
	CursorSlice & beg = cur.selBegin();
	CursorSlice & end = cur.selEnd();
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
	return size_t(idx) < nargs() ? cell(idx).getText(0) : 0;
}


void InsetTabular::markErased()
{
	for (idx_type idx = 0; idx < nargs(); ++idx)
		cell(idx).markErased();
}


bool InsetTabular::forceDefaultParagraphs(InsetBase const *) const
{
#if 0
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
#endif
	return false;
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


bool InsetTabular::tablemode(LCursor & cur) const
{
	return cur.selBegin().idx() != cur.selEnd().idx();
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
	
