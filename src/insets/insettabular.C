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
#include "debug.h"
#include "dispatchresult.h"
#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "language.h"
#include "latexrunparams.h"
#include "LColor.h"
#include "lyx_cb.h"
#include "lyxlex.h"
#include "metricsinfo.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "undo.h"
#include "WordLangTuple.h"

#include "frontends/Alert.h"
#include "frontends/font_metrics.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"

#include "support/std_sstream.h"
#include <iostream>

using lyx::graphics::PreviewLoader;

using lyx::support::ltrim;
using lyx::support::strToInt;

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
LyXTabular * paste_tabular = 0;


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

struct FindFeature {
	FindFeature(LyXTabular::Feature feature) : feature_(feature) {}
	bool operator()(TabularFeature & tf)
	{
		return tf.action == feature_;
	}
private:
	LyXTabular::Feature feature_;
};

} // namespace anon


string const featureAsString(LyXTabular::Feature feature)
{
	TabularFeature * it  = tabularFeature;
	TabularFeature * end = it +
		sizeof(tabularFeature) / sizeof(TabularFeature);
	it = std::find_if(it, end, FindFeature(feature));
	return (it == end) ? string() : it->feature;
}


bool InsetTabular::hasPasteBuffer() const
{
	return (paste_tabular != 0);
}


InsetTabular::InsetTabular(Buffer const & buf, int rows, int columns)
	: tabular(buf.params(), max(rows, 1), max(columns, 1)),
	  buffer_(&buf), cursorx_(0), cursory_(0)
{
	tabular.setOwner(this);
	// for now make it always display as display() inset
	// just for test!!!
	the_locking_inset = 0;
	old_locking_inset = 0;
	locked = false;
	oldcell = -1;
	actrow = actcell = 0;
	clearSelection();
	in_reset_pos = 0;
	inset_x = 0;
	inset_y = 0;
}


InsetTabular::InsetTabular(InsetTabular const & tab)
	: UpdatableInset(tab), tabular(tab.tabular),
		buffer_(tab.buffer_), cursorx_(0), cursory_(0)
{
	tabular.setOwner(this);
	the_locking_inset = 0;
	old_locking_inset = 0;
	locked = false;
	oldcell = -1;
	actrow = actcell = 0;
	clearSelection();
	in_reset_pos = 0;
	inset_x = 0;
	inset_y = 0;
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


BufferView * InsetTabular::view() const
{
	BOOST_ASSERT(false);
	return 0;
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

#if 0
	UpdatableInset::draw(pi, x, y);
#else
	if (!owner())
		x += scroll();
#endif

	top_x = x;
	top_baseline = y;
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
			int cx = nx + tabular.getBeginningOfTextInCell(cell);
			if (first_visible_cell < 0)
				first_visible_cell = cell;
			if (hasSelection()) {
				drawCellSelection(pi.pain, nx, y, i, j, cell);
			}

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
	bool on_off;

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

	if ((column >= cs) && (column <= ce) && (row >= rs) && (row <= re)) {
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


void InsetTabular::insetUnlock(BufferView * bv)
{
	if (the_locking_inset) {
		the_locking_inset->insetUnlock(bv);
		updateLocal(bv);
		the_locking_inset = 0;
	}
	actcell = 0;
	oldcell = -1;
	locked = false;
	if (scroll(false) || hasSelection()) {
		clearSelection();
		if (scroll(false))
			scroll(bv, 0.0F);
		updateLocal(bv);
	}
}


void InsetTabular::updateLocal(BufferView * bv) const
{
	bv->updateInset(this);
	if (locked)
		resetPos(bv);
}


bool InsetTabular::lockInsetInInset(BufferView * bv, UpdatableInset * inset)
{
	lyxerr[Debug::INSETTEXT] << "InsetTabular::LockInsetInInset("
			      << inset << "): ";
	if (!inset)
		return false;

	oldcell = -1;

	if (inset == &tabular.getCellInset(actcell)) {
		lyxerr[Debug::INSETTEXT] << "OK" << endl;
		the_locking_inset = &tabular.getCellInset(actcell);
		resetPos(bv);
		return true;
	}

	if (!the_locking_inset) {
		int const n = tabular.getNumberOfCells();
		int const id = inset->id();
		for (int i = 0; i < n; ++i) {
			InsetText * in = &tabular.getCellInset(i);
			if (inset == in) {
				actcell = i;
				the_locking_inset = in;
				locked = true;
				resetPos(bv);
				return true;
			}
			if (in->getInsetFromID(id)) {
				actcell = i;
				in->dispatch(FuncRequest(bv, LFUN_INSET_EDIT));
				return the_locking_inset->lockInsetInInset(bv, inset);
			}
		}
		return false;
	}

	if (the_locking_inset && (the_locking_inset == inset)) {
		lyxerr[Debug::INSETTEXT] << "OK" << endl;
		resetPos(bv);
		return false;
	}

	if (the_locking_inset) {
		lyxerr[Debug::INSETTEXT] << "MAYBE" << endl;
		return the_locking_inset->lockInsetInInset(bv, inset);
	}

	lyxerr[Debug::INSETTEXT] << "NOT OK" << endl;
	return false;
}


bool InsetTabular::unlockInsetInInset(BufferView * bv, UpdatableInset * inset,
				      bool lr)
{
	if (!the_locking_inset)
		return false;
	if (the_locking_inset == inset) {
		the_locking_inset->insetUnlock(bv);
#ifdef WITH_WARNINGS
#warning fix scrolling when cellinset has requested a scroll (Jug)!!!
#endif
#if 0
		if (scroll(false))
			scroll(bv, 0.0F);
#endif
		updateLocal(bv);
		// this has to be here otherwise we don't redraw the cell!
		the_locking_inset = 0;
		return true;
	}
	if (the_locking_inset->unlockInsetInInset(bv, inset, lr)) {
		if (inset->lyxCode() == TABULAR_CODE &&
		    !the_locking_inset->getFirstLockingInsetOfType(TABULAR_CODE)) {
			InsetTabularMailer(*this).updateDialog(bv);
			oldcell = actcell;
		}
		return true;
	}
	return false;
}


int InsetTabular::insetInInsetY() const
{
	if (!the_locking_inset)
		return 0;
	return inset_y + the_locking_inset->insetInInsetY();
}


UpdatableInset * InsetTabular::getLockingInset() const
{
	return the_locking_inset ? the_locking_inset->getLockingInset() :
		const_cast<InsetTabular *>(this);
}


UpdatableInset * InsetTabular::getFirstLockingInsetOfType(InsetOld::Code c)
{
	if (c == lyxCode())
		return this;
	if (the_locking_inset)
		return the_locking_inset->getFirstLockingInsetOfType(c);
	return 0;
}


bool InsetTabular::insertInset(BufferView * bv, InsetOld * inset)
{
	if (the_locking_inset)
		return the_locking_inset->insertInset(bv, inset);
	return false;
}


void InsetTabular::lfunMousePress(FuncRequest const & cmd)
{
	if (hasSelection() && cmd.button() == mouse_button::button3)
		return;

	if (hasSelection()) {
		clearSelection();
		updateLocal(cmd.view());
	}

	int const ocell = actcell;
	BufferView * bv = cmd.view();

	if (!locked) {
		locked = true;
		the_locking_inset = 0;
		inset_x = 0;
		inset_y = 0;
	}
	setPos(bv, cmd.x, cmd.y);
	clearSelection();

	bool const inset_hit = insetHit(bv, cmd.x, cmd.y);

	if (ocell == actcell && the_locking_inset && inset_hit) {
		resetPos(bv);
		FuncRequest cmd1 = cmd;
		cmd1.x -= inset_x;
		cmd1.y -= inset_y;
		the_locking_inset->dispatch(cmd1);
		return;
	}

	if (the_locking_inset) {
		the_locking_inset->insetUnlock(bv);
		updateLocal(bv);
		the_locking_inset = 0;
	}

	if (cmd.button() == mouse_button::button2) {
		dispatch(FuncRequest(bv, LFUN_PASTESELECTION, "paragraph"));
		return;
	}

	if (inset_hit && bv->theLockingInset()) {
		if (!bv->lockInset(&tabular.getCellInset(actcell))) {
			lyxerr[Debug::INSETS] << "Cannot lock inset" << endl;
			return;
		}
		FuncRequest cmd1 = cmd;
		cmd1.x -= inset_x;
		cmd1.y -= inset_y;
		the_locking_inset->dispatch(cmd1);
	}
}


bool InsetTabular::lfunMouseRelease(FuncRequest const & cmd)
{
	DispatchResult ret(false);
	if (the_locking_inset) {
		FuncRequest cmd1 = cmd;
		cmd1.x -= inset_x;
		cmd1.y -= inset_y;
		ret = the_locking_inset->dispatch(cmd1);
	}
	if (cmd.button() == mouse_button::button3 && ret == DispatchResult(false)) {
		InsetTabularMailer(*this).showDialog(cmd.view());
		return true;
	}
	return ret.dispatched() || ret.val() > FINISHED;
}


void InsetTabular::lfunMouseMotion(FuncRequest const & cmd)
{
	if (the_locking_inset) {
		FuncRequest cmd1 = cmd;
		cmd1.x -= inset_x;
		cmd1.y -= inset_y;
		the_locking_inset->dispatch(cmd1);
		return;
	}

	BufferView * bv = cmd.view();
	int const old_cell = actcell;

	setPos(bv, cmd.x, cmd.y);
	if (!hasSelection()) {
		setSelection(actcell, actcell);
		updateLocal(bv);
	} else if (old_cell != actcell) {
		setSelection(sel_cell_start, actcell);
		updateLocal(bv);
	}
}


void InsetTabular::edit(BufferView * bv, int index)
{
	lyxerr << "InsetTabular::edit" << endl;
	if (!bv->lockInset(this)) {
		lyxerr << "InsetTabular::Cannot lock inset (2)" << endl;
		return;
	}

	locked = true;
	the_locking_inset = 0;
	inset_x = 0;
	inset_y = 0;
	actcell = index;
	clearSelection();
	resetPos(bv);
	bv->fitCursor();

	UpdatableInset & inset = tabular.getCellInset(actcell);
	inset.dispatch(FuncRequest(bv, LFUN_INSET_EDIT, "left"));
	if (the_locking_inset)
		updateLocal(bv);
}


DispatchResult
InsetTabular::priv_dispatch(FuncRequest const & cmd,
			    idx_type & idx, pos_type & pos)
{
	// We need to save the value of the_locking_inset as the call to
	// the_locking_inset->localDispatch might unlock it.
	old_locking_inset = the_locking_inset;
	DispatchResult result = UpdatableInset::priv_dispatch(cmd, idx, pos);
	BufferView * bv = cmd.view();

	if (cmd.action == LFUN_INSET_EDIT) {

		lyxerr << "InsetTabular::edit: " << this << " args: '"
			<< cmd.argument << "'  first cell: "
			<< &tabular.cell_info[0][0].inset << endl;

		if (!bv->lockInset(this)) {
			lyxerr << "InsetTabular::Cannot lock inset" << endl;
			return DispatchResult(true);
		}

		finishUndo();
		locked = true;
		the_locking_inset = 0;
		inset_x = 0;
		inset_y = 0;

		if (cmd.argument.size()) {
			if (cmd.argument == "left") {
				if (isRightToLeft(bv))
					actcell = tabular.getLastCellInRow(0);
				else
					actcell = 0;
			} else {
				if (isRightToLeft(bv))
					actcell = tabular.getFirstCellInRow(tabular.rows()-1);
				else
					actcell = tabular.getNumberOfCells() - 1;
			}
			clearSelection();
			resetPos(bv);
			bv->fitCursor();
		}

		else {
			setPos(bv, cmd.x, cmd.y);
			clearSelection();
			finishUndo();
			if (insetHit(bv, cmd.x, cmd.y) && cmd.button() != mouse_button::button3) {
				inset_x = cursorx_ - top_x + tabular.getBeginningOfTextInCell(actcell);
				inset_y = cursory_;
				activateCellInset(bv, cmd.x - inset_x, cmd.y - inset_y, cmd.button());
			}
		}
		return DispatchResult(true);
	}

	if (result.dispatched()) {
		resetPos(bv);
		return result;
	}

	if (cmd.action < 0 && cmd.argument.empty())
		return DispatchResult(false, FINISHED);

	bool hs = hasSelection();

	result = DispatchResult(true);
	// this one have priority over the locked InsetText, if we're not already
	// inside another tabular then that one get's priority!
	if (getFirstLockingInsetOfType(InsetOld::TABULAR_CODE) == this) {
		switch (cmd.action) {
		case LFUN_MOUSE_PRESS:
			lfunMousePress(cmd);
			return DispatchResult(true);

		case LFUN_MOUSE_MOTION:
			lfunMouseMotion(cmd);
			return DispatchResult(true);

		case LFUN_MOUSE_RELEASE:
			return DispatchResult(lfunMouseRelease(cmd));

		case LFUN_CELL_BACKWARD:
		case LFUN_CELL_FORWARD:
			unlockInsetInInset(bv, the_locking_inset);
			if (cmd.action == LFUN_CELL_FORWARD)
				moveNextCell(bv, old_locking_inset != 0);
			else
				movePrevCell(bv, old_locking_inset != 0);
			clearSelection();
			if (hs)
				updateLocal(bv);
			if (!the_locking_inset)
				return DispatchResult(true, NOUPDATE);
			return result;
		// this to avoid compiler warnings.
		default:
			break;
		}
	}

	kb_action action = cmd.action;
	string    arg    = cmd.argument;
	if (the_locking_inset) {
		result = the_locking_inset->dispatch(cmd);
		if (result.dispatched()) {
			if (result.val() == NOUPDATE) {
				int const sc = scroll();
				resetPos(bv);
				if (sc != scroll()) {
					// inset has been scrolled
					updateLocal(bv);
				}
			} else {
				updateLocal(bv);
			}
			return result;
		} else if (result.val() == FINISHED_UP) {
			action = LFUN_UP;
			// Make sure to reset status message after
			// exiting, e.g. math inset
			bv->owner()->clearMessage();
		} else if (result.val() == FINISHED_DOWN) {
			action = LFUN_DOWN;
			bv->owner()->clearMessage();
		} else if (result.val() == FINISHED_RIGHT) {
			action = LFUN_RIGHT;
			bv->owner()->clearMessage();
		} else if (result.val() == FINISHED) {
			bv->owner()->clearMessage();
		}
	}

	result = DispatchResult(true);
	switch (action) {
		// --- Cursor Movements ----------------------------------
	case LFUN_RIGHTSEL: {
		int const start = hasSelection() ? sel_cell_start : actcell;
		if (tabular.isLastCellInRow(actcell)) {
			setSelection(start, actcell);
			break;
		}

		int end = actcell;
		// if we are starting a selection, only select
		// the current cell at the beginning
		if (hasSelection()) {
			moveRight(bv, false);
			end = actcell;
		}
		setSelection(start, end);
		updateLocal(bv);
		break;
	}
	case LFUN_RIGHT:
		result = moveRight(bv);
		clearSelection();
		if (hs)
			updateLocal(bv);
		break;
	case LFUN_LEFTSEL: {
		int const start = hasSelection() ? sel_cell_start : actcell;
		if (tabular.isFirstCellInRow(actcell)) {
			setSelection(start, actcell);
			break;
		}

		int end = actcell;
		// if we are starting a selection, only select
		// the current cell at the beginning
		if (hasSelection()) {
			moveLeft(bv, false);
			end = actcell;
		}
		setSelection(start, end);
		updateLocal(bv);
		break;
	}
	case LFUN_LEFT:
		result = moveLeft(bv);
		clearSelection();
		if (hs)
			updateLocal(bv);
		break;
	case LFUN_DOWNSEL: {
		int const start = hasSelection() ? sel_cell_start : actcell;
		int const ocell = actcell;
		// if we are starting a selection, only select
		// the current cell at the beginning
		if (hasSelection()) {
			moveDown(bv, false);
			if (ocell == sel_cell_end ||
			    tabular.column_of_cell(ocell) > tabular.column_of_cell(actcell))
				setSelection(start, tabular.getCellBelow(sel_cell_end));
			else
				setSelection(start, tabular.getLastCellBelow(sel_cell_end));
		} else {
			setSelection(start, start);
		}
		updateLocal(bv);
	}
	break;
	case LFUN_DOWN:
		result = moveDown(bv, old_locking_inset != 0);
		clearSelection();
		if (hs)
			updateLocal(bv);
		break;
	case LFUN_UPSEL: {
		int const start = hasSelection() ? sel_cell_start : actcell;
		int const ocell = actcell;
		// if we are starting a selection, only select
		// the current cell at the beginning
		if (hasSelection()) {
			moveUp(bv, false);
			if ((ocell == sel_cell_end) ||
			    (tabular.column_of_cell(ocell)>tabular.column_of_cell(actcell)))
				setSelection(start, tabular.getCellAbove(sel_cell_end));
			else
				setSelection(start, tabular.getLastCellAbove(sel_cell_end));
		} else {
			setSelection(start, start);
		}
		updateLocal(bv);
	}
	break;
	case LFUN_UP:
		result = moveUp(bv, old_locking_inset != 0);
		clearSelection();
		if (hs)
			updateLocal(bv);
		break;
	case LFUN_NEXT: {
		if (hs)
			clearSelection();
		int column = actcol;
		unlockInsetInInset(bv, the_locking_inset);
		if (bv->top_y() + bv->painter().paperHeight() <
		    top_baseline + tabular.getHeightOfTabular())
			{
				bv->scrollDocView(bv->top_y() + bv->painter().paperHeight());
				actcell = tabular.getCellBelow(first_visible_cell) + column;
			} else {
				actcell = tabular.getFirstCellInRow(tabular.rows() - 1) + column;
			}
		resetPos(bv);
		updateLocal(bv);
		break;
	}
	case LFUN_PRIOR: {
		if (hs)
			clearSelection();
		int column = actcol;
		unlockInsetInInset(bv, the_locking_inset);
		if (top_baseline < 0) {
			bv->scrollDocView(bv->top_y() - bv->painter().paperHeight());
			if (top_baseline > 0)
				actcell = column;
			else
				actcell = tabular.getCellBelow(first_visible_cell) + column;
		} else {
			actcell = column;
		}
		resetPos(bv);
		updateLocal(bv);
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
		InsetTabularMailer(*this).showDialog(bv);
		break;
	case LFUN_INSET_DIALOG_UPDATE:
		InsetTabularMailer(*this).updateDialog(bv);
		break;
	case LFUN_TABULAR_FEATURE:
		if (!tabularFeatures(bv, arg))
			result = DispatchResult(false);
		break;
		// insert file functions
	case LFUN_FILE_INSERT_ASCII_PARA:
	case LFUN_FILE_INSERT_ASCII:
	{
		string tmpstr = getContentsOfAsciiFile(bv, arg, false);
		if (tmpstr.empty())
			break;
		if (insertAsciiString(bv, tmpstr, false))
			updateLocal(bv);
		else
			result = DispatchResult(false);
		break;
	}
	// cut and paste functions
	case LFUN_CUT:
		if (!copySelection(bv))
			break;
		// no break here!
	case LFUN_DELETE:
		recordUndo(bv, Undo::DELETE);
		cutSelection(bv->buffer()->params());
		updateLocal(bv);
		break;
	case LFUN_COPY:
		if (!hasSelection())
			break;
		finishUndo();
		copySelection(bv);
		break;
	case LFUN_PASTESELECTION:
	{
		string const clip = bv->getClipboard();
		if (clip.empty())
			break;
#if 0
		if (clip.find('\t') != string::npos) {
			int cols = 1;
			int rows = 1;
			int maxCols = 1;
			string::size_type len = clip.length();
			string::size_type p = 0;

			while (p < len &&
			      ((p = clip.find_first_of("\t\n", p)) != string::npos)) {
				switch (clip[p]) {
				case '\t':
					++cols;
					break;
				case '\n':
					if ((p+1) < len)
						++rows;
					maxCols = max(cols, maxCols);
					cols = 1;
					break;
				}
				++p;
			}
			maxCols = max(cols, maxCols);
			delete paste_tabular;
			paste_tabular = new LyXTabular(bv->buffer()->params(),
						       this, rows, maxCols);
			string::size_type op = 0;
			int cell = 0;
			int cells = paste_tabular->getNumberOfCells();
			p = cols = 0;
			while ((cell < cells) && (p < len) &&
			      (p = clip.find_first_of("\t\n", p)) != string::npos) {
				if (p >= len)
					break;
				switch (clip[p]) {
				case '\t':
					paste_tabular->getCellInset(cell)->setText(clip.substr(op, p-op));
					++cols;
					++cell;
					break;
				case '\n':
					paste_tabular->getCellInset(cell)->setText(clip.substr(op, p-op));
					while (cols++ < maxCols)
						++cell;
					cols = 0;
					break;
				}
				++p;
				op = p;
			}
			// check for the last cell if there is no trailing '\n'
			if ((cell < cells) && (op < len))
				paste_tabular->getCellInset(cell)->setText(clip.substr(op, len-op));
		} else
#else
		if (!insertAsciiString(bv, clip, true))
#endif
		{
			// so that the clipboard is used and it goes on
			// to default
			// and executes LFUN_PASTESELECTION in insettext!
			delete paste_tabular;
			paste_tabular = 0;
		}
	}
	case LFUN_PASTE:
		if (hasPasteBuffer()) {
			recordUndo(bv, Undo::INSERT);
			pasteSelection(bv);
			updateLocal(bv);
			break;
		}
		// ATTENTION: the function above has to be PASTE and PASTESELECTION!!!
	default:
		// handle font changing stuff on selection before we lock the inset
		// in the default part!
		result = DispatchResult(false);
		if (hs) {
			switch(action) {
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
				if (bv->dispatch(FuncRequest(bv, action, arg)))
					result = DispatchResult(true);
				break;
			default:
				break;
			}
		}
		// we try to activate the actual inset and put this event down to
		// the insets dispatch function.
		if (result.dispatched() || the_locking_inset)
			break;
		if (activateCellInset(bv)) {
			result = the_locking_inset->dispatch(FuncRequest(bv, action, arg));
			if (!result.dispatched()) {
				unlockInsetInInset(bv, the_locking_inset);
				// we need to update if this was requested before
				updateLocal(bv);
				return DispatchResult(false);
			}
			if (hs)
				clearSelection();
			updateLocal(bv);
			return result;
		}
		break;
	}

	if (result.val() >= FINISHED)
		bv->unlockInset(this);
	else if (!the_locking_inset && bv->fitCursor())
		updateLocal(bv);

	return result;
}


int InsetTabular::latex(Buffer const & buf, ostream & os,
			LatexRunParams const & runparams) const
{
	return tabular.latex(buf, os, runparams);
}


int InsetTabular::ascii(Buffer const & buf, ostream & os,
			LatexRunParams const & runparams) const
{
	if (runparams.linelen > 0)
		return tabular.ascii(buf, os, runparams,
				     ownerPar(buf, this).params().depth(),
				     false, 0);
	return tabular.ascii(buf, os, runparams, 0, false, 0);
}


int InsetTabular::linuxdoc(Buffer const & buf, ostream & os,
			   LatexRunParams const & runparams) const
{
	return tabular.linuxdoc(buf,os, runparams);
}


int InsetTabular::docbook(Buffer const & buf, ostream & os,
			  LatexRunParams const & runparams) const
{
	int ret = 0;
	InsetOld * master;

	// if the table is inside a float it doesn't need the informaltable
	// wrapper. Search for it.
	for (master = owner();
	     master && master->lyxCode() != InsetOld::FLOAT_CODE;
	     master = master->owner());

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
#if 1
	// if we have a locking_inset we should have to check only this cell for
	// change so I'll try this to have a boost, but who knows ;) (Jug?)
	// This is _really_ important (André)
	if (the_locking_inset == &tabular.getCellInset(actcell)) {
		int maxAsc = 0;
		int maxDesc = 0;
		for (int j = 0; j < tabular.columns(); ++j) {
			Dimension dim;
			MetricsInfo m = mi;
			m.base.textwidth =
				tabular.column_info[j].p_width.inPixels(mi.base.textwidth);
			tabular.getCellInset(actrow, j).metrics(m, dim);
			maxAsc  = max(dim.asc, maxAsc);
			maxDesc = max(dim.des, maxDesc);
		}
		tabular.setWidthOfCell(actcell, the_locking_inset->width());
		tabular.setAscentOfRow(actrow, maxAsc + ADD_TO_HEIGHT);
		tabular.setDescentOfRow(actrow, maxDesc + ADD_TO_HEIGHT);
		return;
	}
#endif

	int cell = -1;
	for (int i = 0; i < tabular.rows(); ++i) {
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


void InsetTabular::getCursor(BufferView & bv, int & x, int & y) const
{
	if (the_locking_inset) {
		the_locking_inset->getCursor(bv, x, y);
		return;
	}

	x = cursorx_;
	y = cursory_ + InsetTabular::y();

	// Fun stuff
	int desc = tabular.getDescentOfRow(actrow);
	y += desc;
	int ascdesc = tabular.getAscentOfRow(actrow) + desc;
	y -= ascdesc / 2;
	y += ADD_TO_HEIGHT * 2;
	y += TEXT_TO_INSET_OFFSET;
}


void InsetTabular::getCursorPos(BufferView * bv, int & x, int & y) const
{
	if (the_locking_inset) {
		the_locking_inset->getCursorPos(bv, x, y);
		return;
	}
	x = cursorx_ - top_x;
	y = cursory_;
}


void InsetTabular::fitInsetCursor(BufferView * bv) const
{
	if (the_locking_inset) {
		the_locking_inset->fitInsetCursor(bv);
		return;
	}

	LyXFont font;
	int const asc = font_metrics::maxAscent(font);
	int const desc = font_metrics::maxDescent(font);
	resetPos(bv);

	bv->fitLockedInsetCursor(cursorx_, cursory_, asc, desc);
}


void InsetTabular::setPos(BufferView * bv, int x, int y) const
{
	cursory_ = 0;
	actcell = actrow = actcol = 0;
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

	cursorx_ = lx - tabular.getWidthOfColumn(actcell) + top_x + 2;
	resetPos(bv);
}


int InsetTabular::getCellXPos(int cell) const
{
	int c = cell;

	for (; !tabular.isFirstCellInRow(c); --c)
		;
	int lx = tabular.getWidthOfColumn(cell);
	for (; c < cell; ++c)
		lx += tabular.getWidthOfColumn(c);

	return (lx - tabular.getWidthOfColumn(cell) + top_x);
}


void InsetTabular::resetPos(BufferView * bv) const
{
#ifdef WITH_WARNINGS
#warning This should be fixed in the right manner (20011128 Jug)
#endif
	// fast hack to fix infinite repaintings!
	if (in_reset_pos > 0)
		return;

	int cell = 0;
	actcol = tabular.column_of_cell(actcell);
	actrow = 0;
	cursory_ = 0;
	for (; cell < actcell && !tabular.isLastRow(cell); ++cell) {
		if (tabular.isLastCellInRow(cell)) {
			cursory_ += tabular.getDescentOfRow(actrow) +
					 tabular.getAscentOfRow(actrow + 1) +
					 tabular.getAdditionalHeight(actrow + 1);
			++actrow;
		}
	}
	if (!locked) {
		if (the_locking_inset)
			inset_y = cursory_;
		return;
	}
	// we need this only from here on!!!
	++in_reset_pos;
	int const offset = ADD_TO_TABULAR_WIDTH + 2;
	int new_x = getCellXPos(actcell);
	int old_x = cursorx_;
	new_x += offset;
	cursorx_ = new_x;
//    cursor.x(getCellXPos(actcell) + offset);
	if (actcol < tabular.columns() - 1 && scroll(false) &&
		tabular.getWidthOfTabular() < bv->workWidth()-20)
	{
		scroll(bv, 0.0F);
		updateLocal(bv);
	} else if (the_locking_inset &&
		 tabular.getWidthOfColumn(actcell) > bv->workWidth() - 20)
	{
		int xx = cursorx_ - offset + bv->text->getRealCursorX();
		if (xx > bv->workWidth()-20) {
			scroll(bv, -(xx - bv->workWidth() + 60));
			updateLocal(bv);
		} else if (xx < 20) {
			if (xx < 0)
				xx = -xx + 60;
			else
				xx = 60;
			scroll(bv, xx);
			updateLocal(bv);
		}
	} else if (cursorx_ - offset > 20 &&
		   cursorx_ - offset + tabular.getWidthOfColumn(actcell)
		   > bv->workWidth() - 20) {
		scroll(bv, -tabular.getWidthOfColumn(actcell) - 20);
		updateLocal(bv);
	} else if (cursorx_ - offset < 20) {
		scroll(bv, 20 - cursorx_ + offset);
		updateLocal(bv);
	} else if (scroll() && top_x > 20 &&
		   (top_x + tabular.getWidthOfTabular()) > bv->workWidth() - 20) {
		scroll(bv, old_x - cursorx_);
		updateLocal(bv);
	}
	if (the_locking_inset) {
		inset_x = cursorx_ - top_x + tabular.getBeginningOfTextInCell(actcell);
		inset_y = cursory_;
	}
	if ((!the_locking_inset ||
	     !the_locking_inset->getFirstLockingInsetOfType(TABULAR_CODE)) &&
	    actcell != oldcell) {
		InsetTabularMailer(*this).updateDialog(bv);
		oldcell = actcell;
	}
	in_reset_pos = 0;
}


DispatchResult InsetTabular::moveRight(BufferView * bv, bool lock)
{
	if (lock && !old_locking_inset) {
		if (activateCellInset(bv))
			return DispatchResult(true);
	} else {
		bool moved = isRightToLeft(bv)
			? movePrevCell(bv) : moveNextCell(bv);
		if (!moved)
			return DispatchResult(false, FINISHED_RIGHT);
		if (lock && activateCellInset(bv))
			return DispatchResult(true);
	}
	resetPos(bv);
	return DispatchResult(true, NOUPDATE);
}


DispatchResult InsetTabular::moveLeft(BufferView * bv, bool lock)
{
	bool moved = isRightToLeft(bv) ? moveNextCell(bv) : movePrevCell(bv);
	if (!moved)
		return DispatchResult(false, FINISHED);
	// behind the inset
	if (lock && activateCellInset(bv, 0, 0, mouse_button::none, true))
		return DispatchResult(true);
	resetPos(bv);
	return DispatchResult(true, NOUPDATE);
}


DispatchResult InsetTabular::moveUp(BufferView * bv, bool lock)
{
	int const ocell = actcell;
	actcell = tabular.getCellAbove(actcell);
	if (actcell == ocell) // we moved out of the inset
		return DispatchResult(false, FINISHED_UP);
	resetPos(bv);
	if (lock) {
		int x = 0;
		int y = 0;
		if (old_locking_inset) {
			old_locking_inset->getCursorPos(bv, x, y);
			x -= cursorx_ + tabular.getBeginningOfTextInCell(actcell);
		}
		if (activateCellInset(bv, x, 0))
			return DispatchResult(true);
	}
	return DispatchResult(true, NOUPDATE);
}


DispatchResult InsetTabular::moveDown(BufferView * bv, bool lock)
{
	int const ocell = actcell;
	actcell = tabular.getCellBelow(actcell);
	if (actcell == ocell) // we moved out of the inset
		return DispatchResult(false, FINISHED_DOWN);
	resetPos(bv);
	if (lock) {
		int x = 0;
		int y = 0;
		if (old_locking_inset) {
			old_locking_inset->getCursorPos(bv, x, y);
			x -= cursorx_ + tabular.getBeginningOfTextInCell(actcell);
		}
		if (activateCellInset(bv, x, 0))
			return DispatchResult(true);
	}
	return DispatchResult(true, NOUPDATE);
}


bool InsetTabular::moveNextCell(BufferView * bv, bool lock)
{
	if (isRightToLeft(bv)) {
		if (tabular.isFirstCellInRow(actcell)) {
			int row = tabular.row_of_cell(actcell);
			if (row == tabular.rows() - 1)
				return false;
			actcell = tabular.getLastCellInRow(row);
			actcell = tabular.getCellBelow(actcell);
		} else {
			if (!actcell)
				return false;
			--actcell;
		}
	} else {
		if (tabular.isLastCell(actcell))
			return false;
		++actcell;
	}
	if (lock) {
		bool rtl = tabular.getCellInset(actcell).paragraphs.begin()->
			isRightToLeftPar(bv->buffer()->params());
		activateCellInset(bv, 0, 0, mouse_button::none, !rtl);
	}
	resetPos(bv);
	return true;
}


bool InsetTabular::movePrevCell(BufferView * bv, bool lock)
{
	if (isRightToLeft(bv)) {
		if (tabular.isLastCellInRow(actcell)) {
			int row = tabular.row_of_cell(actcell);
			if (row == 0)
				return false;
			actcell = tabular.getFirstCellInRow(row);
			actcell = tabular.getCellAbove(actcell);
		} else {
			if (tabular.isLastCell(actcell))
				return false;
			++actcell;
		}
	} else {
		if (!actcell) // first cell
			return false;
		--actcell;
	}
	if (lock) {
		bool rtl = tabular.getCellInset(actcell).paragraphs.begin()->
			isRightToLeftPar(bv->buffer()->params());
		activateCellInset(bv, 0, 0, mouse_button::none, !rtl);
	}
	resetPos(bv);
	return true;
}


void InsetTabular::setFont(BufferView * bv, LyXFont const & font, bool tall,
			   bool selectall)
{
	if (selectall) {
		setSelection(0, tabular.getNumberOfCells() - 1);
	}
	if (hasSelection()) {
		recordUndo(bv, Undo::ATOMIC);
		bool const frozen = undo_frozen;
		if (!frozen)
			freezeUndo();
		// apply the fontchange on the whole selection
		int sel_row_start;
		int sel_row_end;
		int sel_col_start;
		int sel_col_end;
		getSelection(sel_row_start, sel_row_end, sel_col_start, sel_col_end);
		for (int i = sel_row_start; i <= sel_row_end; ++i)
			for (int j = sel_col_start; j <= sel_col_end; ++j)
				tabular.getCellInset(i, j).setFont(bv, font, tall, true);

		if (!frozen)
			unFreezeUndo();
		if (selectall)
			clearSelection();
		updateLocal(bv);
	}
	if (the_locking_inset)
		the_locking_inset->setFont(bv, font, tall);
}


bool InsetTabular::tabularFeatures(BufferView * bv, string const & what)
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
	tabularFeatures(bv, action, val);
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


void InsetTabular::tabularFeatures(BufferView * bv,
	LyXTabular::Feature feature, string const & value)
{
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
		getSelection(sel_row_start, sel_row_end, sel_col_start, sel_col_end);
	} else {
		sel_col_start = sel_col_end = tabular.column_of_cell(actcell);
		sel_row_start = sel_row_end = tabular.row_of_cell(actcell);
	}
	recordUndo(bv, Undo::ATOMIC);

	int row =  tabular.row_of_cell(actcell);
	int column = tabular.column_of_cell(actcell);
	bool flag = true;
	LyXTabular::ltType ltt;

	switch (feature) {

	case LyXTabular::SET_PWIDTH: {
		LyXLength const len(value);
		LyXLength const & oldlen = tabular.getColumnPWidth(actcell);

		tabular.setColumnPWidth(actcell, len);
		if (oldlen != len) {
			// We need this otherwise we won't resize
			// the insettext of the active cell (if any)
			// until later (see InsetText::do_resize)
			unlockInsetInInset(bv, the_locking_inset);
			bv->update();
		}

		if (len.zero()
		    && tabular.getAlignment(actcell, true) == LYX_ALIGN_BLOCK)
			tabularFeatures(bv, LyXTabular::ALIGN_CENTER, string());
		else if (!len.zero()
			 && tabular.getAlignment(actcell, true) != LYX_ALIGN_BLOCK)
			tabularFeatures(bv, LyXTabular::ALIGN_BLOCK, string());
		break;
	}

	case LyXTabular::SET_MPWIDTH:
	{
		LyXLength const len(value);
		LyXLength const & oldlen = tabular.getPWidth(actcell);
		tabular.setMColumnPWidth(actcell, len);
		if (oldlen != len) {
			// We need this otherwise we won't resize
			// the insettext of the active cell (if any)
			// until later (see InsetText::do_resize)
			unlockInsetInInset(bv, the_locking_inset);
			updateLocal(bv);
		}
	}
	break;

	case LyXTabular::SET_SPECIAL_COLUMN:
	case LyXTabular::SET_SPECIAL_MULTI:
		tabular.setAlignSpecial(actcell,value,feature);
		updateLocal(bv);
		break;

	case LyXTabular::APPEND_ROW:
		// append the row into the tabular
		unlockInsetInInset(bv, the_locking_inset);
		tabular.appendRow(bv->buffer()->params(), actcell);
		tabular.setOwner(this);
		updateLocal(bv);
		break;

	case LyXTabular::APPEND_COLUMN:
		// append the column into the tabular
		unlockInsetInInset(bv, the_locking_inset);
		tabular.appendColumn(bv->buffer()->params(), actcell);
		tabular.setOwner(this);
		actcell = tabular.getCellNumber(row, column);
		updateLocal(bv);
		break;

	case LyXTabular::DELETE_ROW:
		unlockInsetInInset(bv, the_locking_inset);
		for (int i = sel_row_start; i <= sel_row_end; ++i)
			tabular.deleteRow(sel_row_start);
		if (sel_row_start >= tabular.rows())
			--sel_row_start;
		actcell = tabular.getCellNumber(sel_row_start, column);
		clearSelection();
		updateLocal(bv);
		break;

	case LyXTabular::DELETE_COLUMN:
		unlockInsetInInset(bv, the_locking_inset);
		for (int i = sel_col_start; i <= sel_col_end; ++i)
			tabular.deleteColumn(sel_col_start);
		if (sel_col_start >= tabular.columns())
			--sel_col_start;
		actcell = tabular.getCellNumber(row, sel_col_start);
		clearSelection();
		updateLocal(bv);
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
		updateLocal(bv);
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
		updateLocal(bv);
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
		updateLocal(bv);
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
		updateLocal(bv);
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
		updateLocal(bv);
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
		updateLocal(bv);
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
				tabular.setMultiColumn(bv->buffer(), actcell, 1);
			updateLocal(bv);
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
		tabular.setMultiColumn(bv->buffer(), s_start, s_end - s_start + 1);
		actcell = s_start;
		clearSelection();
		updateLocal(bv);
		break;
	}

	case LyXTabular::SET_ALL_LINES:
		setLines = true;
	case LyXTabular::UNSET_ALL_LINES:
		for (int i = sel_row_start; i <= sel_row_end; ++i)
			for (int j = sel_col_start; j <= sel_col_end; ++j)
				tabular.setAllLines(
					tabular.getCellNumber(i,j), setLines);
		updateLocal(bv);
		break;

	case LyXTabular::SET_LONGTABULAR:
		tabular.setLongTabular(true);
		updateLocal(bv); // because this toggles displayed
		break;

	case LyXTabular::UNSET_LONGTABULAR:
		tabular.setLongTabular(false);
		updateLocal(bv); // because this toggles displayed
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

	InsetTabularMailer(*this).updateDialog(bv);
}


bool InsetTabular::activateCellInset(BufferView * bv, int x, int y,
	mouse_button::state button, bool behind)
{
	UpdatableInset & inset = tabular.getCellInset(actcell);
	if (behind) {
#warning metrics?
		x = inset.x() + inset.width();
		y = inset.descent();
	}
	//inset_x = cursorx_ - top_x + tabular.getBeginningOfTextInCell(actcell);
	//inset_y = cursory_;
	inset.dispatch(FuncRequest(bv, LFUN_INSET_EDIT, x,  y, button));
	if (!the_locking_inset)
		return false;
	updateLocal(bv);
	return the_locking_inset;
}


bool InsetTabular::insetHit(BufferView *, int x, int) const
{
	return x + top_x > cursorx_ + tabular.getBeginningOfTextInCell(actcell);
}


void InsetTabular::deleteLyXText(BufferView * /*bv*/, bool /*recursive*/) const
{
	//resizeLyXText(bv, recursive);
}


LyXText * InsetTabular::getLyXText(BufferView const * bv,
				   bool const recursive) const
{
	if (the_locking_inset)
		return the_locking_inset->getLyXText(bv, recursive);
	return InsetOld::getLyXText(bv, recursive);
}


bool InsetTabular::showInsetDialog(BufferView * bv) const
{
	if (!the_locking_inset || !the_locking_inset->showInsetDialog(bv))
		InsetTabularMailer(*this).showDialog(bv);
	return true;
}


void InsetTabular::openLayoutDialog(BufferView * bv) const
{
	if (the_locking_inset) {
		InsetTabular * inset = static_cast<InsetTabular *>
			(the_locking_inset->getFirstLockingInsetOfType(TABULAR_CODE));
		if (inset) {
			inset->openLayoutDialog(bv);
			return;
		}
	}
	InsetTabularMailer(*this).showDialog(bv);
}


//
// function returns an object as defined in func_status.h:
// states OK, Unknown, Disabled, On, Off.
//
FuncStatus InsetTabular::getStatus(string const & what) const
{
	int action = LyXTabular::LAST_ACTION;
	FuncStatus status;

	int i = 0;
	for (; tabularFeature[i].action != LyXTabular::LAST_ACTION; ++i) {
		string const tmp = tabularFeature[i].feature;
		if (tmp == what.substr(0, tmp.length())) {
			//if (!compare(tabularFeatures[i].feature.c_str(), what.c_str(),
			//   tabularFeatures[i].feature.length())) {
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
		getSelection(sel_row_start, sel_row_end, dummy, dummy);
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
		status.setOnOff(tabular.getVAlignment(actcell, flag) == LyXTabular::LYX_VALIGN_TOP);
		break;

	case LyXTabular::M_VALIGN_BOTTOM:
		flag = false;
	case LyXTabular::VALIGN_BOTTOM:
		status.setOnOff(tabular.getVAlignment(actcell, flag) == LyXTabular::LYX_VALIGN_BOTTOM);
		break;

	case LyXTabular::M_VALIGN_MIDDLE:
		flag = false;
	case LyXTabular::VALIGN_MIDDLE:
		status.setOnOff(tabular.getVAlignment(actcell, flag) == LyXTabular::LYX_VALIGN_MIDDLE);
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


bool InsetTabular::copySelection(BufferView * bv)
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

	delete paste_tabular;
	paste_tabular = new LyXTabular(tabular);
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
	LatexRunParams const runparams;	
	paste_tabular->ascii(*bv->buffer(), os, runparams,
			     ownerPar(*bv->buffer(), this).params().depth(), true, '\t');
	bv->stuffClipboard(os.str());
	return true;
}


bool InsetTabular::pasteSelection(BufferView * bv)
{
	if (!paste_tabular)
		return false;

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
			inset.deleteLyXText(bv);
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


bool InsetTabular::isRightToLeft(BufferView * bv)
{
	return bv->getParentLanguage(this)->RightToLeft();
}


int InsetTabular::scroll(bool recursive) const
{
	int sx = UpdatableInset::scroll(false);

	if (recursive && the_locking_inset)
		sx += the_locking_inset->scroll(recursive);

	return sx;
}


void InsetTabular::getSelection(int & srow, int & erow,
				int & scol, int & ecol) const
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


ParagraphList * InsetTabular::getParagraphs(int i) const
{
	return i < tabular.getNumberOfCells()
		? tabular.getCellInset(i).getParagraphs(0)
		: 0;
}


int InsetTabular::numParagraphs() const
{
	return tabular.getNumberOfCells();
}


LyXText * InsetTabular::getText(int i) const
{
	return i < tabular.getNumberOfCells()
		?  tabular.getCellInset(i).getText(0)
		: 0;
}


LyXCursor const & InsetTabular::cursor(BufferView * bv) const
{
	if (the_locking_inset)
		return the_locking_inset->cursor(bv);
	return InsetOld::cursor(bv);
}


InsetOld * InsetTabular::getInsetFromID(int id_arg) const
{
	if (id_arg == id())
		return const_cast<InsetTabular *>(this);

	for (int i = 0; i < tabular.rows(); ++i) {
		for (int j = 0; j < tabular.columns(); ++j) {
			InsetOld * inset = tabular.getCellInset(i, j).getInsetFromID(id_arg);
			if (inset)
				return inset;
		}
	}
	return 0;
}


WordLangTuple const
InsetTabular::selectNextWordToSpellcheck(BufferView * bv, float & value) const
{
	if (the_locking_inset) {
		WordLangTuple word =
			the_locking_inset->selectNextWordToSpellcheck(bv, value);
		if (!word.word().empty())
			return word;
		if (tabular.isLastCell(actcell)) {
			bv->unlockInset(const_cast<InsetTabular *>(this));
			return WordLangTuple();
		}
		++actcell;
	}
	// otherwise we have to lock the next inset and ask for it's selecttion
	tabular.getCellInset(actcell)
		.dispatch(FuncRequest(bv, LFUN_INSET_EDIT));
	WordLangTuple word = selectNextWordInt(bv, value);
	if (!word.word().empty())
		resetPos(bv);
	return word;
}


WordLangTuple InsetTabular::selectNextWordInt(BufferView * bv, float & value) const
{
	// when entering this function the inset should be ALWAYS locked!
	BOOST_ASSERT(the_locking_inset);

	WordLangTuple word =
		the_locking_inset->selectNextWordToSpellcheck(bv, value);
	if (!word.word().empty())
		return word;

	if (tabular.isLastCell(actcell)) {
		bv->unlockInset(const_cast<InsetTabular *>(this));
		return WordLangTuple();
	}

	// otherwise we have to lock the next inset and ask for it's selecttion
	++actcell;
	tabular.getCellInset(actcell)
		.dispatch(FuncRequest(bv, LFUN_INSET_EDIT));
	return selectNextWordInt(bv, value);
}


void InsetTabular::selectSelectedWord(BufferView * bv)
{
	if (the_locking_inset)
		the_locking_inset->selectSelectedWord(bv);
}


void InsetTabular::markErased()
{
	for (int cell = 0; cell < tabular.getNumberOfCells(); ++cell)
		tabular.getCellInset(cell).markErased();
}


bool InsetTabular::nextChange(BufferView * bv, lyx::pos_type & length)
{
	if (the_locking_inset) {
		if (the_locking_inset->nextChange(bv, length)) {
			updateLocal(bv);
			return true;
		}
		if (tabular.isLastCell(actcell))
			return false;
		++actcell;
	}
	InsetText & inset = tabular.getCellInset(actcell);
	if (inset.nextChange(bv, length)) {
		updateLocal(bv);
		return true;
	}
	while (!tabular.isLastCell(actcell)) {
		++actcell;
		InsetText & inset = tabular.getCellInset(actcell);
		if (inset.nextChange(bv, length)) {
			updateLocal(bv);
			return true;
		}
	}
	return false;
}


bool InsetTabular::searchForward(BufferView * bv, string const & str,
				 bool cs, bool mw)
{
	int cell = 0;
	if (the_locking_inset) {
		if (the_locking_inset->searchForward(bv, str, cs, mw)) {
			updateLocal(bv);
			return true;
		}
		if (tabular.isLastCell(actcell))
			return false;
		cell = actcell + 1;
	}
	InsetText & inset = tabular.getCellInset(cell);
	if (inset.searchForward(bv, str, cs, mw)) {
		updateLocal(bv);
		return true;
	}
	while (!tabular.isLastCell(cell)) {
		++cell;
		InsetText & inset = tabular.getCellInset(cell);
		if (inset.searchForward(bv, str, cs, mw)) {
			updateLocal(bv);
			return true;
		}
	}
	return false;
}


bool InsetTabular::searchBackward(BufferView * bv, string const & str,
			       bool cs, bool mw)
{
	int cell = tabular.getNumberOfCells();
	if (the_locking_inset) {
		if (the_locking_inset->searchBackward(bv, str, cs, mw)) {
			updateLocal(bv);
			return true;
		}
		cell = actcell;
	}

	while (cell) {
		--cell;
		InsetText & inset = tabular.getCellInset(cell);
		if (inset.searchBackward(bv, str, cs, mw)) {
			updateLocal(bv);
			return true;
		}
	}
	return false;
}


bool InsetTabular::insetAllowed(InsetOld::Code code) const
{
	if (the_locking_inset)
		return the_locking_inset->insetAllowed(code);
	// we return true here because if the inset is not locked someone
	// wants to insert something in one of our insettexts and we generally
	// allow to do so.
	return true;
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


bool InsetTabular::insertAsciiString(BufferView * bv, string const & buf,
				     bool usePaste)
{
	if (buf.length() <= 0)
		return true;

	int cols = 1;
	int rows = 1;
	int maxCols = 1;
	string::size_type len = buf.length();
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
	int cell = 0;
	int ocol = 0;
	int row = 0;
	if (usePaste) {
		delete paste_tabular;
		paste_tabular = new LyXTabular(bv->buffer()->params(),
					       rows, maxCols);
		paste_tabular->setOwner(this);
		loctab = paste_tabular;
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
				LyXFont const font = inset.getLyXText(bv)->
					getFont(inset.paragraphs.begin(), 0);
				inset.setText(buf.substr(op, p - op), font);
				++cols;
				++cell;
			}
			break;
		case '\n':
			// we can only set this if we are not too far right
			if (cols < columns) {
				InsetText & inset = tabular.getCellInset(cell);
				LyXFont const font = inset.getLyXText(bv)->
					getFont(inset.paragraphs.begin(), 0);
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
		LyXFont const font = inset.getLyXText(bv)->
			getFont(inset.paragraphs.begin(), 0);
		inset.setText(buf.substr(op, len - op), font);
	}

	return true;
}


void InsetTabular::addPreview(PreviewLoader & loader) const
{
	int const rows = tabular.rows();
	int const columns = tabular.columns();
	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < columns; ++j)
			tabular.getCellInset(i, j).addPreview(loader);
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
	Buffer const & buffer = inset.buffer();

	ostringstream data;
	data << name_ << " \\active_cell " << inset.getActCell() << '\n';
	inset.write(buffer, data);
	data << "\\end_inset\n";
	return data.str();
}
