/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#include <fstream>
#include <algorithm>

#include <cstdlib>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insettabular.h"

#include "buffer.h"
#include "commandtags.h"
#include "debug.h"
#include "LaTeXFeatures.h"
#include "Painter.h"
#include "font.h"
#include "lyxtext.h"
#include "lyx_gui_misc.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "insets/insettext.h"
#include "frontends/Dialogs.h"
#include "debug.h"
#include "lyxfunc.h"

const int ADD_TO_HEIGHT = 2;
const int ADD_TO_TABULAR_WIDTH = 2;
///
static LyXTabular * paste_tabular = 0;
bool InsetTabular::hasPasteBuffer() const
{
    return (paste_tabular != 0);
}

using std::ostream;
using std::ifstream;
using std::max;
using std::endl;
using std::swap;

    
struct tabular_features {
    LyXTabular::Feature action;
    string feature;
};

//static tabular_features * tabularFeatures = 0;

static tabular_features tabularFeatures[] =
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
    { LyXTabular::VALIGN_TOP, "valign-top" },
    { LyXTabular::VALIGN_BOTTOM, "valign-bottom" },
    { LyXTabular::VALIGN_CENTER, "valign-center" },
    { LyXTabular::M_TOGGLE_LINE_TOP, "m-toggle-line-top" },
    { LyXTabular::M_TOGGLE_LINE_BOTTOM, "m-toggle-line-bottom" },
    { LyXTabular::M_TOGGLE_LINE_LEFT, "m-toggle-line-left" },
    { LyXTabular::M_TOGGLE_LINE_RIGHT, "m-toggle-line-right" },
    { LyXTabular::M_ALIGN_LEFT, "m-align-left" },
    { LyXTabular::M_ALIGN_RIGHT, "m-align-right" },
    { LyXTabular::M_ALIGN_CENTER, "m-align-center" },
    { LyXTabular::M_VALIGN_TOP, "m-valign-top" },
    { LyXTabular::M_VALIGN_BOTTOM, "m-valign-bottom" },
    { LyXTabular::M_VALIGN_CENTER, "m-valign-center" },
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


static inline
bool cellstart(LyXParagraph::size_type p) 
{
	return ((p % 2) == 0);
}


InsetTabular::InsetTabular(Buffer const & buf, int rows, int columns)
	: buffer(&buf)
{
    if (rows <= 0)
        rows = 1;
    if (columns <= 0)
        columns = 1;
    tabular = new LyXTabular(this, rows,columns);
    // for now make it always display as display() inset
    // just for test!!!
    the_locking_inset = 0;
    locked = no_selection = cursor_visible = false;
    cursor.x_fix(-1);
    oldcell = -1;
    actrow = actcell = 0;
    cursor.pos(0);
    clearSelection();
    need_update = INIT;
}


InsetTabular::InsetTabular(InsetTabular const & tab, Buffer const & buf)
	: buffer(&buf)
{
    tabular = new LyXTabular(this, *(tab.tabular));
    the_locking_inset = 0;
    locked = no_selection = cursor_visible = false;
    cursor.x_fix(-1);
    oldcell = -1;
    actrow = actcell = 0;
    cursor.pos(0);
    sel_pos_start = sel_pos_end = sel_cell_start = sel_cell_end = 0;
    need_update = INIT;
}


InsetTabular::~InsetTabular()
{
    delete tabular;
    hideDialog();
}


Inset * InsetTabular::Clone(Buffer const & buf) const
{
    InsetTabular * t = new InsetTabular(*this, buf);
    delete t->tabular;
    t->tabular = tabular->Clone(t);
    return t;
}


void InsetTabular::Write(Buffer const * buf, ostream & os) const
{
    os << " Tabular" << endl;
    tabular->Write(buf, os);
}


void InsetTabular::Read(Buffer const * buf, LyXLex & lex)
{
    bool old_format = (lex.GetString() == "\\LyXTable");
    string token;

    if (tabular)
	delete tabular;
    tabular = new LyXTabular(buf, this, lex);

    need_update = INIT;

    if (old_format)
	return;

    lex.nextToken();
    token = lex.GetString();
    while (lex.IsOK() && (token != "\\end_inset")) {
	lex.nextToken();
	token = lex.GetString();
    }
    if (token != "\\end_inset") {
        lex.printError("Missing \\end_inset at this point. "
                       "Read: `$$Token'");
    }
}


int InsetTabular::ascent(BufferView *, LyXFont const &) const
{
    return tabular->GetAscentOfRow(0);
}


int InsetTabular::descent(BufferView *, LyXFont const &) const
{
    return tabular->GetHeightOfTabular() - tabular->GetAscentOfRow(0) + 1;
}


int InsetTabular::width(BufferView *, LyXFont const &) const
{
    return tabular->GetWidthOfTabular() + (2 * ADD_TO_TABULAR_WIDTH);
}


void InsetTabular::draw(BufferView * bv, LyXFont const & font, int baseline,
			float & x, bool cleared) const
{
    Painter & pain = bv->painter();
    int i, j;
    int nx;
    float cx;

    UpdatableInset::draw(bv, font, baseline, x, cleared);
    if (!cleared && ((need_update == INIT) || (need_update == FULL) ||
		     (top_x != int(x)) || (top_baseline != baseline))) {
	int h = ascent(bv, font) + descent(bv, font);
	int tx = display() || !owner() ? 0 : top_x;
	int w =  tx ? width(bv, font) : pain.paperWidth();
	int ty = baseline - ascent(bv, font);
	
	if (ty < 0)
	    ty = 0;
	if ((ty + h) > pain.paperHeight())
	    h = pain.paperHeight();
	if ((top_x + w) > pain.paperWidth())
	    w = pain.paperWidth();
	pain.fillRectangle(tx, ty, w, h);
	need_update = FULL;
	cleared = true;
    }
    top_x = int(x);
    top_baseline = baseline;
    if (bv->text->status == LyXText::CHANGED_IN_DRAW)
	return;
    bool dodraw;
    x += ADD_TO_TABULAR_WIDTH;
    if (cleared) {
	int cell = 0;
	for (i = 0; i < tabular->rows(); ++i) {
	    nx = int(x);
	    dodraw = ((baseline + tabular->GetDescentOfRow(i)) > 0) &&
		    (baseline - tabular->GetAscentOfRow(i))<pain.paperHeight();
	    for (j = 0; j < tabular->columns(); ++j) {
		if (tabular->IsPartOfMultiColumn(i, j))
		    continue;
		cx = nx + tabular->GetBeginningOfTextInCell(cell);
		if (dodraw) {
		    if (hasSelection())
			DrawCellSelection(pain, nx, baseline, i, j, cell);
		    tabular->GetCellInset(cell)->draw(bv, font, baseline, cx,
						      cleared);
		    DrawCellLines(pain, nx, baseline, i, cell);
		}
		nx += tabular->GetWidthOfColumn(cell);
		++cell;
	    }
	    baseline += tabular->GetDescentOfRow(i) +
		tabular->GetAscentOfRow(i + 1) +
		tabular->GetAdditionalHeight(cell);
	}
    } else if (need_update == CELL) {
	int cell = 0;
	nx = int(x);
	if (the_locking_inset) {
	    Inset * inset = tabular->GetCellInset(cell);
	    for (i = 0; (inset != the_locking_inset) && (i < tabular->rows());
		 ++i)
	    {
		for (j = 0; (inset != the_locking_inset) && (j < tabular->columns()); ++j)
		{
		    if (tabular->IsPartOfMultiColumn(i, j))
			continue;
		    nx += tabular->GetWidthOfColumn(cell);
		    ++cell;
		    inset = tabular->GetCellInset(cell);
		}
		if (tabular->row_of_cell(cell) > i) {
		    nx = int(x);
		    baseline += tabular->GetDescentOfRow(i) +
			tabular->GetAscentOfRow(i + 1) +
			tabular->GetAdditionalHeight(cell);
		}
	    }
	} else {
	    for (i = 0; (cell < actcell) && (i < tabular->rows()); ++i) {
		for (j = 0; (cell < actcell) && (j < tabular->columns()); ++j) {
		    if (tabular->IsPartOfMultiColumn(i, j))
			continue;
		    nx += tabular->GetWidthOfColumn(cell);
		    ++cell;
		}
		if (tabular->row_of_cell(cell) > i) {
		    nx = int(x);
		    baseline += tabular->GetDescentOfRow(i) +
			tabular->GetAscentOfRow(i + 1) +
			tabular->GetAdditionalHeight(cell);
		}
	    }
	}
	i = tabular->row_of_cell(cell);
	if (the_locking_inset != tabular->GetCellInset(cell)) {
	    lyxerr[Debug::INSETS] << "ERROR this shouldn't happen\n";
	    return;
	}
#if 0
	LyXText::text_status st = bv->text->status;
	do {
	    cx = nx + tabular->GetBeginningOfTextInCell(cell);
	    bv->text->status = st;
	    if (need_update == CELL) {
		// clear before the inset
		pain.fillRectangle(
		    nx + 1,
		    baseline - tabular->GetAscentOfRow(i) + 1,
		    int(cx - nx - 1),
		    tabular->GetAscentOfRow(i) +
		    tabular->GetDescentOfRow(i) - 1);
		// clear behind the inset
		pain.fillRectangle(
		    int(cx + the_locking_inset->width(bv,font) + 1),
		    baseline - tabular->GetAscentOfRow(i) + 1,
		    tabular->GetWidthOfColumn(cell) -
		    tabular->GetBeginningOfTextInCell(cell) -
		    the_locking_inset->width(bv,font) -
		    tabular->GetAdditionalWidth(cell) - 1,
		    tabular->GetAscentOfRow(i) +
		    tabular->GetDescentOfRow(i) - 1);
	    }
	    tabular->GetCellInset(cell)->draw(bv,font,baseline, cx, false);
	} while(bv->text->status == LyXText::CHANGED_IN_DRAW);
#else
	cx = nx + tabular->GetBeginningOfTextInCell(cell);
	if (need_update == CELL) {
	    // clear before the inset
	    pain.fillRectangle(
		nx + 1,
		baseline - tabular->GetAscentOfRow(i) + 1,
		int(cx - nx - 1),
		tabular->GetAscentOfRow(i) +
		tabular->GetDescentOfRow(i) - 1);
	    // clear behind the inset
	    pain.fillRectangle(
		int(cx + the_locking_inset->width(bv,font) + 1),
		baseline - tabular->GetAscentOfRow(i) + 1,
		tabular->GetWidthOfColumn(cell) -
		tabular->GetBeginningOfTextInCell(cell) -
		the_locking_inset->width(bv,font) -
		tabular->GetAdditionalWidth(cell) - 1,
		tabular->GetAscentOfRow(i) +
		tabular->GetDescentOfRow(i) - 1);
	}
	tabular->GetCellInset(cell)->draw(bv,font,baseline, cx, false);
	if (bv->text->status == LyXText::CHANGED_IN_DRAW)
	    return;
#endif
    }
    x -= ADD_TO_TABULAR_WIDTH;
    x += width(bv, font);
    if (bv->text->status == LyXText::CHANGED_IN_DRAW)
	need_update = INIT;
    else
	need_update = NONE;
}


void InsetTabular::DrawCellLines(Painter & pain, int x, int baseline,
                                 int row, int cell) const
{
    int  x2 = x + tabular->GetWidthOfColumn(cell);
    bool on_off;

    if (!tabular->TopAlreadyDrawed(cell)) {
	on_off = !tabular->TopLine(cell);
	pain.line(x, baseline - tabular->GetAscentOfRow(row),
		  x2, baseline -  tabular->GetAscentOfRow(row),
		  on_off ? LColor::tabularonoffline : LColor::tabularline,
		  on_off ? Painter::line_onoffdash : Painter::line_solid);
    }
    on_off = !tabular->BottomLine(cell);
    pain.line(x,baseline +  tabular->GetDescentOfRow(row),
	      x2, baseline +  tabular->GetDescentOfRow(row),
	      on_off ? LColor::tabularonoffline : LColor::tabularline,
	      on_off ? Painter::line_onoffdash : Painter::line_solid);
    if (!tabular->LeftAlreadyDrawed(cell)) {
	on_off = !tabular->LeftLine(cell);
	pain.line(x, baseline -  tabular->GetAscentOfRow(row),
		  x, baseline +  tabular->GetDescentOfRow(row),
		  on_off ? LColor::tabularonoffline : LColor::tabularline,
		  on_off ? Painter::line_onoffdash : Painter::line_solid);
    }
    on_off = !tabular->RightLine(cell);
    pain.line(x2 - tabular->GetAdditionalWidth(cell),
              baseline -  tabular->GetAscentOfRow(row),
              x2 - tabular->GetAdditionalWidth(cell),
              baseline +  tabular->GetDescentOfRow(row),
	      on_off ? LColor::tabularonoffline : LColor::tabularline,
              on_off ? Painter::line_onoffdash : Painter::line_solid);
}


void InsetTabular::DrawCellSelection(Painter & pain, int x, int baseline,
				     int row, int column, int cell) const
{
    int cs = tabular->column_of_cell(sel_cell_start);
    int ce = tabular->column_of_cell(sel_cell_end);
    if (cs > ce) {
	ce = cs;
	cs = tabular->column_of_cell(sel_cell_end);
    } else {
	ce = tabular->right_column_of_cell(sel_cell_end);
    }

    int rs = tabular->row_of_cell(sel_cell_start);
    int re = tabular->row_of_cell(sel_cell_end);
    if (rs > re) swap(rs, re);

    if ((column >= cs) && (column <= ce) && (row >= rs) && (row <= re)) {
	int w = tabular->GetWidthOfColumn(cell);
	int h = tabular->GetAscentOfRow(row) + tabular->GetDescentOfRow(row);
	pain.fillRectangle(x, baseline - tabular->GetAscentOfRow(row),
			   w, h, LColor::selection);
    }
}


void InsetTabular::update(BufferView * bv, LyXFont const & font, bool reinit)
{
    if (reinit) {
	need_update = INIT;
	calculate_dimensions_of_cells(bv, font, true);
	if (owner())
	    owner()->update(bv, font, true);
	return;
    }
    if (the_locking_inset) {
	the_locking_inset->update(bv, font, reinit);
//	resetPos(bv);
//	inset_x = cursor.x() - top_x + tabular->GetBeginningOfTextInCell(actcell);
//	inset_y = cursor.y();
    }
    switch (need_update) {
    case INIT:
    case FULL:
    case CELL:
	if (calculate_dimensions_of_cells(bv, font, false))
	    need_update = INIT;
	break;
    case SELECTION:
	need_update = INIT;
	break;
    default:
	break;
    }
}


string const InsetTabular::EditMessage() const
{
    return _("Opened Tabular Inset");
}


void InsetTabular::Edit(BufferView * bv, int x, int y, unsigned int button)
{
    UpdatableInset::Edit(bv, x, y, button);

    if (!bv->lockInset(this)) {
	lyxerr[Debug::INSETS] << "InsetTabular::Cannot lock inset" << endl;
	return;
    }
    locked = true;
    the_locking_inset = 0;
    inset_pos = 0;
    inset_x = 0;
    inset_y = 0;
    setPos(bv, x, y);
    sel_pos_start = sel_pos_end = cursor.pos();
    sel_cell_start = sel_cell_end = actcell;
    bv->text->FinishUndo();
    if (InsetHit(bv, x, y)) {
	ActivateCellInset(bv, x, y, button);
    }
//    UpdateLocal(bv, NONE, false);
//    bv->getOwner()->getPopups().updateFormTabular();
}


void InsetTabular::InsetUnlock(BufferView * bv)
{
    if (the_locking_inset) {
	the_locking_inset->InsetUnlock(bv);
	the_locking_inset = 0;
    }
    HideInsetCursor(bv);
    no_selection = false;
    oldcell = -1;
    locked = false;
    if (scroll() || hasSelection()) {
	if (scroll()) {
	    scroll(bv, 0.0F);
	} else {
	    sel_pos_start = sel_pos_end = 0;
	    sel_cell_start = sel_cell_end = 0;
	}
	UpdateLocal(bv, FULL, false);
    }
}


void InsetTabular::UpdateLocal(BufferView * bv, UpdateCodes what,
			       bool mark_dirty) const
{
    if (need_update < what) // only set this if it has greater update
	need_update = what;
    // Dirty Cast! (Lgb)
    if (need_update != NONE) {
	bv->updateInset(const_cast<InsetTabular *>(this), mark_dirty);
	if (locked) // && (what != NONE))
	    resetPos(bv);
    }
}


bool InsetTabular::LockInsetInInset(BufferView * bv, UpdatableInset * inset)
{
    lyxerr[Debug::INSETS] << "InsetTabular::LockInsetInInset(" <<inset<< "): ";
    if (!inset)
	return false;
    oldcell = -1;
    if (inset == tabular->GetCellInset(actcell)) {
	lyxerr[Debug::INSETS] << "OK" << endl;
	the_locking_inset = tabular->GetCellInset(actcell);
	resetPos(bv);
	inset_x = cursor.x() - top_x + tabular->GetBeginningOfTextInCell(actcell);
	inset_y = cursor.y();
	inset_pos = cursor.pos();
	return true;
    } else if (the_locking_inset && (the_locking_inset == inset)) {
	if (cursor.pos() == inset_pos) {
	    lyxerr[Debug::INSETS] << "OK" << endl;
	    resetPos(bv);
	    inset_x = cursor.x() - top_x + tabular->GetBeginningOfTextInCell(actcell);
	    inset_y = cursor.y();
	} else {
	    lyxerr[Debug::INSETS] << "cursor.pos != inset_pos" << endl;
	}
    } else if (the_locking_inset) {
	lyxerr[Debug::INSETS] << "MAYBE" << endl;
	return the_locking_inset->LockInsetInInset(bv, inset);
    }
    lyxerr[Debug::INSETS] << "NOT OK" << endl;
    return false;
}


bool InsetTabular::UnlockInsetInInset(BufferView * bv, UpdatableInset * inset,
				   bool lr)
{
    if (!the_locking_inset)
        return false;
    if (the_locking_inset == inset) {
        the_locking_inset->InsetUnlock(bv);
        the_locking_inset = 0;
        if (lr)
            moveRight(bv, false);
	ShowInsetCursor(bv, false);
	UpdateLocal(bv, CELL, false);
        return true;
    }
    if (the_locking_inset->UnlockInsetInInset(bv, inset, lr)) {
	if (inset->LyxCode() == TABULAR_CODE &&
	    !the_locking_inset->GetFirstLockingInsetOfType(TABULAR_CODE))
	{
		bv->owner()->getDialogs()->updateTabular(this);
		oldcell = actcell;
	}
	return true;
    }
    return false;
}


bool InsetTabular::UpdateInsetInInset(BufferView * bv, Inset * inset)
{
    if (!the_locking_inset)
	return false;
    if (the_locking_inset != inset) {
	if (!the_locking_inset->UpdateInsetInInset(bv, inset))
	    return false;
    }
    UpdateLocal(bv, CELL, false);
    return true;
}


unsigned int InsetTabular::InsetInInsetY()
{
    if (!the_locking_inset)
	return 0;

    return (inset_y + the_locking_inset->InsetInInsetY());
}


UpdatableInset * InsetTabular::GetLockingInset()
{
    return the_locking_inset ? the_locking_inset->GetLockingInset() : this;
}


UpdatableInset * InsetTabular::GetFirstLockingInsetOfType(Inset::Code c)
{
    if (c == LyxCode())
	return this;
    if (the_locking_inset)
	return the_locking_inset->GetFirstLockingInsetOfType(c);
    return 0;
}


bool InsetTabular::InsertInset(BufferView * bv, Inset * inset)
{
    if (the_locking_inset)
	return the_locking_inset->InsertInset(bv, inset);
    return false;
}


void InsetTabular::InsetButtonPress(BufferView * bv, int x, int y, int button)
{
    if (hasSelection() && (button != 3)) {
	clearSelection();
	UpdateLocal(bv, SELECTION, false);
    }
    no_selection = false;

    int const ocell = actcell;
    int const orow = actrow;

    HideInsetCursor(bv);
    setPos(bv, x, y);
    if (actrow != orow)
	UpdateLocal(bv, NONE, false);
    sel_pos_start = sel_pos_end = cursor.pos();
    sel_cell_start = sel_cell_end = actcell;

    bool const inset_hit = InsetHit(bv, x, y);

    if ((ocell == actcell) && the_locking_inset && inset_hit) {
	cursor.pos(0); // always before the inset!
	resetPos(bv);
        the_locking_inset->InsetButtonPress(bv,
					    x - inset_x, y - inset_y, button);
        return;
    } else if (the_locking_inset) {
	the_locking_inset->InsetUnlock(bv);
    }
    the_locking_inset = 0;
    if (inset_hit && bv->theLockingInset()) {
	if (ActivateCellInset(bv, x, y, button))
	    the_locking_inset->InsetButtonPress(bv, x - inset_x,
						y - inset_y, button);
	return;
    }
    ShowInsetCursor(bv);
}


void InsetTabular::InsetButtonRelease(BufferView * bv,
				      int x, int y, int button)
{
    if (button == 3) {
	if (the_locking_inset) {
	    UpdatableInset * i;
	    if ((i=the_locking_inset->GetFirstLockingInsetOfType(TABULAR_CODE))) {
		i->InsetButtonRelease(bv, x, y, button);
		return;
	    }
	}
	bv->owner()->getDialogs()->showTabular(this);
	return;
    }
    if (the_locking_inset) {
        the_locking_inset->InsetButtonRelease(bv, x-inset_x, y-inset_y,button);
        return;
    }
    no_selection = false;
}


void InsetTabular::InsetMotionNotify(BufferView * bv, int x, int y, int button)
{
    if (the_locking_inset) {
        the_locking_inset->InsetMotionNotify(bv, x - inset_x,
					     y - inset_y, button);
        return;
    }
    if (!no_selection) {
	HideInsetCursor(bv);
	LyXParagraph::size_type const old_pos = sel_pos_end;
	int const old_cell = actcell;

	setPos(bv, x, y);
	sel_pos_end = cursor.pos();
	sel_cell_end = actcell;
	if ((sel_cell_end != old_cell) || (old_pos != sel_pos_end))
	    UpdateLocal(bv, SELECTION, false);
	ShowInsetCursor(bv);
    }
    no_selection = false;
}


void InsetTabular::InsetKeyPress(XKeyEvent * xke)
{
    if (the_locking_inset) {
        the_locking_inset->InsetKeyPress(xke);
        return;
    }
}


UpdatableInset::RESULT InsetTabular::LocalDispatch(BufferView * bv, int action,
						   string const & arg)
{
    UpdatableInset::RESULT 
        result;

    no_selection = false;
    if (((result=UpdatableInset::LocalDispatch(bv, action, arg)) == DISPATCHED)
	|| (result == DISPATCHED_NOUPDATE)) {

	resetPos(bv);
	return result;
    }

    if ((action < 0) && arg.empty())
        return FINISHED;

    if ((action != LFUN_DOWN) && (action != LFUN_UP) &&
        (action != LFUN_DOWNSEL) && (action != LFUN_UPSEL))
        cursor.x_fix(-1);
    if (the_locking_inset) {
        result=the_locking_inset->LocalDispatch(bv, action, arg);
	if (result == DISPATCHED_NOUPDATE)
	    return result;
	else if (result == DISPATCHED) {
	    the_locking_inset->ToggleInsetCursor(bv);
	    UpdateLocal(bv, CELL, false);
	    the_locking_inset->ToggleInsetCursor(bv);
            return result;
        } else if (result == FINISHED) {
	    if ((action == LFUN_RIGHT) || (action == -1)) {
		cursor.pos(inset_pos + 1);
		resetPos(bv);
	    }
	    sel_pos_start = sel_pos_end = cursor.pos();
	    sel_cell_start = sel_cell_end = actcell;
	    the_locking_inset=0;
	    ShowInsetCursor(bv);
	    result = DISPATCHED;
	    return result;
	}
    }

    bool hs = hasSelection();
    HideInsetCursor(bv);
    result=DISPATCHED;
    switch (action) {
	// --- Cursor Movements ---------------------------------------------
    case LFUN_RIGHTSEL:
	if (tabular->IsLastCellInRow(actcell) && !cellstart(cursor.pos()))
	    break;
	moveRight(bv, false);
#if 0
	if (!cellstart(cursor.pos())) {
	    sel_pos_end = cursor.pos();
	    if (tabular->right_column_of_cell(sel_cell_start) >
		tabular->right_column_of_cell(actcell))
		sel_cell_end = actcell+1;
	    else
		sel_cell_end = actcell;
	}
#else
	sel_pos_end = cursor.pos();
	sel_cell_end = actcell;
#endif
	UpdateLocal(bv, SELECTION, false);
	break;
    case LFUN_RIGHT:
	result = moveRight(bv);
	sel_pos_start = sel_pos_end = cursor.pos();
	sel_cell_start = sel_cell_end = actcell;
	if (hs)
	    UpdateLocal(bv, SELECTION, false);
	break;
    case LFUN_LEFTSEL:
	if (tabular->IsFirstCellInRow(actcell) && cellstart(cursor.pos()))
	    break;
	moveLeft(bv, false);
#if 0
	if (cellstart(cursor.pos())) {
	    sel_pos_end = cursor.pos();
	    if (tabular->column_of_cell(sel_cell_start) >=
		tabular->column_of_cell(actcell))
		sel_cell_end = actcell;
	    else
		sel_cell_end = actcell-1;
	}
#else
	sel_pos_end = cursor.pos();
	sel_cell_end = actcell;
#endif
	UpdateLocal(bv, SELECTION, false);
	break;
    case LFUN_LEFT:
	result = moveLeft(bv);
	sel_pos_start = sel_pos_end = cursor.pos();
	sel_cell_start = sel_cell_end = actcell;
	if (hs)
	    UpdateLocal(bv, SELECTION, false);
	break;
    case LFUN_DOWNSEL:
    {
	int const ocell = actcell;
	moveDown(bv);
	sel_pos_end = cursor.pos();
	if ((ocell == sel_cell_end) ||
	    (tabular->column_of_cell(ocell)>tabular->column_of_cell(actcell)))
	    sel_cell_end = tabular->GetCellBelow(sel_cell_end);
	else
	    sel_cell_end = tabular->GetLastCellBelow(sel_cell_end);
	UpdateLocal(bv, SELECTION, false);
    }
    break;
    case LFUN_DOWN:
	result = moveDown(bv);
	sel_pos_start = sel_pos_end = cursor.pos();
	sel_cell_start = sel_cell_end = actcell;
	if (hs)
	    UpdateLocal(bv, SELECTION, false);
	break;
    case LFUN_UPSEL:
    {
	int const ocell = actcell;
	moveUp(bv);
	sel_pos_end = cursor.pos();
	if ((ocell == sel_cell_end) ||
	    (tabular->column_of_cell(ocell)>tabular->column_of_cell(actcell)))
	    sel_cell_end = tabular->GetCellAbove(sel_cell_end);
	else
	    sel_cell_end = tabular->GetLastCellAbove(sel_cell_end);
	UpdateLocal(bv, SELECTION, false);
    }
    break;
    case LFUN_UP:
	result = moveUp(bv);
	sel_pos_start = sel_pos_end = cursor.pos();
	sel_cell_start = sel_cell_end = actcell;
	if (hs)
	    UpdateLocal(bv, SELECTION, false);
	break;
    case LFUN_BACKSPACE:
	break;
    case LFUN_DELETE:
	break;
    case LFUN_HOME:
	break;
    case LFUN_END:
	break;
    case LFUN_SHIFT_TAB:
    case LFUN_TAB:
	if (the_locking_inset) {
	    UnlockInsetInInset(bv, the_locking_inset);
	    the_locking_inset = 0;
	}
	if (action == LFUN_TAB)
	    moveNextCell(bv);
	else
	    movePrevCell(bv);
	sel_pos_start = sel_pos_end = cursor.pos();
	sel_cell_start = sel_cell_end = actcell;
	if (hs)
	    UpdateLocal(bv, SELECTION, false);
	break;
    case LFUN_LAYOUT_TABULAR:
    {
	bv->owner()->getDialogs()->showTabular(this);
    }
    break;
    case LFUN_TABULAR_FEATURE:
	if (!TabularFeatures(bv, arg))
	    result = UNDISPATCHED;
	break;
    case LFUN_CUT:
	if (!copySelection(bv))
	    break;
	bv->text->SetUndo(bv->buffer(), Undo::DELETE,
#ifndef NEW_INSETS
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next
#else
	  bv->text->cursor.par()->previous,
	  bv->text->cursor.par()->next
#endif
		);
	cutSelection();
	UpdateLocal(bv, INIT, true);
	break;
    case LFUN_COPY:
	if (!hasSelection())
	    break;
	bv->text->FinishUndo();
	copySelection(bv);
	break;
    case LFUN_PASTE:
	if (!hasPasteBuffer())
	    break;
	bv->text->SetUndo(bv->buffer(), Undo::INSERT,
#ifndef NEW_INSETS
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next
#else
	  bv->text->cursor.par()->previous,
	  bv->text->cursor.par()->next
#endif
		);
	pasteSelection(bv);
	UpdateLocal(bv, INIT, true);
	break;
    default:
	// we try to activate the actual inset and put this event down to
	// the insets dispatch function.
	result = UNDISPATCHED;
	if (the_locking_inset)
	    break;
	if (ActivateCellInset(bv, 0, 0, 0, !cellstart(cursor.pos()))) {
	    result=the_locking_inset->LocalDispatch(bv, action, arg);
	    if (result == DISPATCHED_NOUPDATE)
		return result;
	    else if (result == DISPATCHED) {
		the_locking_inset->ToggleInsetCursor(bv);
		UpdateLocal(bv, CELL, false);
		the_locking_inset->ToggleInsetCursor(bv);
		return result;
	    } else {
		result = UNDISPATCHED;
	    }
	}
	break;
    }
    if (result!=FINISHED) {
	if (!the_locking_inset) {
	    ShowInsetCursor(bv);
	}
    } else
        bv->unlockInset(this);
    return result;
}


int InsetTabular::Latex(Buffer const * buf, ostream & os,
			bool fragile, bool fp) const
{
    return tabular->Latex(buf, os, fragile, fp);
}


int InsetTabular::Ascii(Buffer const * buf, ostream & os, int) const
{
    // This should be changed to a real ascii export
    return tabular->Ascii(buf, os);
}


int InsetTabular::Linuxdoc(Buffer const *, ostream &) const
{
    return 0;
}


int InsetTabular::DocBook(Buffer const * buf, ostream & os) const
{
    return tabular->DocBook(buf,os);
}


void InsetTabular::Validate(LaTeXFeatures & features) const
{
    tabular->Validate(features);
}


bool InsetTabular::calculate_dimensions_of_cells(BufferView * bv,
						 LyXFont const & font,
						 bool reinit) const
{
    int cell = -1;
    int maxAsc, maxDesc;
    InsetText * inset;
    bool changed = false;
    
    // if we have a locking_inset we should have to check only this cell for
    // change so I'll try this to have a boost, but who knows ;)
    if ((need_update != INIT) &&
	(the_locking_inset == tabular->GetCellInset(actcell))) {
	maxAsc = the_locking_inset->ascent(bv, font);
	maxDesc = the_locking_inset->descent(bv, font);
	changed = tabular->SetWidthOfCell(actcell, the_locking_inset->width(bv, font));
	changed = tabular->SetAscentOfRow(actrow, maxAsc + ADD_TO_HEIGHT) || changed;
	changed = tabular->SetDescentOfRow(actrow, maxDesc + ADD_TO_HEIGHT) || changed;
	return changed;
    }
    for (int i = 0; i < tabular->rows(); ++i) {
	maxAsc = maxDesc = 0;
	for (int j= 0; j < tabular->columns(); ++j) {
	    if (tabular->IsPartOfMultiColumn(i,j))
		continue;
	    ++cell;
	    inset = tabular->GetCellInset(cell);
	    if (!reinit)
		inset->update(bv, font, false);
	    maxAsc = max(maxAsc, inset->ascent(bv, font));
	    maxDesc = max(maxDesc, inset->descent(bv, font));
	    changed = tabular->SetWidthOfCell(cell, inset->width(bv, font)) || changed;
	}
	changed = tabular->SetAscentOfRow(i, maxAsc + ADD_TO_HEIGHT) || changed;
	changed = tabular->SetDescentOfRow(i, maxDesc + ADD_TO_HEIGHT) || changed;
    }
    return changed;
}


void InsetTabular::GetCursorPos(BufferView *,
				int & x, int & y) const
{
    x = cursor.x() - top_x;
    y = cursor.y();
}


void InsetTabular::ToggleInsetCursor(BufferView * bv)
{
    if (the_locking_inset) {
        the_locking_inset->ToggleInsetCursor(bv);
	return;
    }

    LyXFont font; // = the_locking_inset->GetFont(par, cursor.pos);

    int const asc = lyxfont::maxAscent(font);
    int const desc = lyxfont::maxDescent(font);
  
    if (cursor_visible)
        bv->hideLockedInsetCursor();
    else
        bv->showLockedInsetCursor(cursor.x(), cursor.y(), asc, desc);
    cursor_visible = !cursor_visible;
}


void InsetTabular::ShowInsetCursor(BufferView * bv, bool show)
{
    if (!cursor_visible) {
	LyXFont font; // = GetFont(par, cursor.pos);
    
	int const asc = lyxfont::maxAscent(font);
	int const desc = lyxfont::maxDescent(font);
	bv->fitLockedInsetCursor(cursor.x(), cursor.y(), asc, desc);
	if (show)
	    bv->showLockedInsetCursor(cursor.x(), cursor.y(), asc, desc);
	cursor_visible = true;
    }
}


void InsetTabular::HideInsetCursor(BufferView * bv)
{
    if (cursor_visible) {
        bv->hideLockedInsetCursor();
	cursor_visible = false;
    }
//    if (cursor_visible)
//        ToggleInsetCursor(bv);
}


void InsetTabular::setPos(BufferView * bv, int x, int y) const
{
    cursor.y(0);
    cursor.pos(0);
	
    actcell = actrow = actcol = 0;
    int ly = tabular->GetDescentOfRow(actrow);

    // first search the right row
    while((ly < y) && (actrow < tabular->rows())) {
        cursor.y(cursor.y() + tabular->GetDescentOfRow(actrow) +
	    tabular->GetAscentOfRow(actrow + 1) +
            tabular->GetAdditionalHeight(tabular->GetCellNumber(actrow + 1,
								actcol)));
        ++actrow;
        ly = cursor.y() + tabular->GetDescentOfRow(actrow);
    }
    actcell = tabular->GetCellNumber(actrow, actcol);

    // now search the right column
    int lx = tabular->GetWidthOfColumn(actcell) -
	tabular->GetAdditionalWidth(actcell);
#if 0
#warning Jürgen, can you rewrite this to _not_ use the sequencing operator. (Lgb)
    for (; !tabular->IsLastCellInRow(actcell) && (lx < x);
	++actcell,lx += tabular->GetWidthOfColumn(actcell) +
	    tabular->GetAdditionalWidth(actcell - 1));
#else
    // Jürgen, you should check that this is correct. (Lgb)
    for (; !tabular->IsLastCellInRow(actcell) && lx < x; ++actcell) {
	    lx += tabular->GetWidthOfColumn(actcell + 1)
		    + tabular->GetAdditionalWidth(actcell);
    }
    
#endif
    cursor.pos(0);
    resetPos(bv);
    if ((lx - (tabular->GetWidthOfColumn(actcell) / 2)) < x) {
	cursor.x(lx + top_x - 2);
	cursor.pos(1);
    } else {
	cursor.x(lx - tabular->GetWidthOfColumn(actcell) + top_x + 2);
    }
    resetPos(bv);
}


int InsetTabular::getCellXPos(int cell) const
{
    int c = cell;

    for (; !tabular->IsFirstCellInRow(c); --c)
	;
    int lx = tabular->GetWidthOfColumn(cell);
    for (; c < cell; ++c) {
	lx += tabular->GetWidthOfColumn(c);
    }
    return (lx - tabular->GetWidthOfColumn(cell) + top_x);
}


void InsetTabular::resetPos(BufferView * bv) const
{
    if (!locked)
	return;
    actcol = tabular->column_of_cell(actcell);

    int cell = 0;
    actrow = 0;
    cursor.y(0);
    for (; (cell < actcell) && !tabular->IsLastRow(cell); ++cell) {
	if (tabular->IsLastCellInRow(cell)) {
	    cursor.y(cursor.y() + tabular->GetDescentOfRow(actrow) +
		tabular->GetAscentOfRow(actrow + 1) +
		tabular->GetAdditionalHeight(cell + 1));
	    ++actrow;
	}
    }
    static int const offset = ADD_TO_TABULAR_WIDTH + 2;
    int new_x = getCellXPos(actcell);
    new_x += offset;
    cursor.x(new_x);
//    cursor.x(getCellXPos(actcell) + offset);
    if (((cursor.x() - offset) > 20) &&
	((cursor.x()-offset+tabular->GetWidthOfColumn(actcell)) >
	 (bv->workWidth()-20)))
    {
	scroll(bv, -tabular->GetWidthOfColumn(actcell)-20);
	UpdateLocal(bv, FULL, false);
    } else if ((cursor.x() - offset) < 20) {
	scroll(bv, 20 - cursor.x() + offset);
	UpdateLocal(bv, FULL, false);
    } else if (!cellstart(cursor.pos())) {
	LyXFont font(LyXFont::ALL_SANE);
	cursor.x(cursor.x() + tabular->GetCellInset(actcell)->width(bv,font) +
		tabular->GetBeginningOfTextInCell(actcell));
    }
    if ((!the_locking_inset ||
	 !the_locking_inset->GetFirstLockingInsetOfType(TABULAR_CODE)) &&
	(actcell != oldcell)) {
	    InsetTabular * inset = const_cast<InsetTabular *>(this);
	    bv->owner()->getDialogs()->updateTabular(inset);
	    oldcell = actcell;
    }
}


UpdatableInset::RESULT InsetTabular::moveRight(BufferView * bv, bool lock)
{
    if (!cellstart(cursor.pos())) {
	if (tabular->IsLastCell(actcell))
	    return FINISHED;
	++actcell;
	cursor.pos(0);
    } else if (lock) {
	if (ActivateCellInset(bv))
	    return DISPATCHED;
    } else {              // before the inset
	cursor.pos(1);
    }
    resetPos(bv);
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT InsetTabular::moveLeft(BufferView * bv, bool lock)
{
    if (cellstart(cursor.pos()) && !actcell)
	return FINISHED;
    if (cellstart(cursor.pos())) {
	--actcell;
	cursor.pos(1);
    } else if (lock) {       // behind the inset
	cursor.pos(0);
	if (ActivateCellInset(bv, 0, 0, 0, true))
	    return DISPATCHED;
    } else {
	cursor.pos(0);
    }
    resetPos(bv);
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT InsetTabular::moveUp(BufferView * bv)
{
    int const ocell = actcell;
    actcell = tabular->GetCellAbove(actcell);
    if (actcell == ocell) // we moved out of the inset
	return FINISHED;
    resetPos(bv);
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT InsetTabular::moveDown(BufferView * bv)
{
    int const ocell = actcell;
    actcell = tabular->GetCellBelow(actcell);
    if (actcell == ocell) // we moved out of the inset
	return FINISHED;
    resetPos(bv);
    return DISPATCHED_NOUPDATE;
}


bool InsetTabular::moveNextCell(BufferView * bv)
{
    if (tabular->IsLastCell(actcell))
	return false;
    ++actcell;
    resetPos(bv);
    return true;
}


bool InsetTabular::movePrevCell(BufferView * bv)
{
    if (!actcell) // first cell
	return false;
    --actcell;
    resetPos(bv);
    return true;
}


bool InsetTabular::Delete()
{
    return true;
}


void InsetTabular::SetFont(BufferView * bv, LyXFont const & font, bool tall)
{
    if (the_locking_inset)
	the_locking_inset->SetFont(bv, font, tall);
}


bool InsetTabular::TabularFeatures(BufferView * bv, string const & what)
{
    LyXTabular::Feature action = LyXTabular::LAST_ACTION;

    int i = 0;
    for (; tabularFeatures[i].action != LyXTabular::LAST_ACTION; ++i) {
	string const tmp = tabularFeatures[i].feature;
	    
	if (tmp == what.substr(0, tmp.length())) {
	//if (!strncmp(tabularFeatures[i].feature.c_str(), what.c_str(),
	      //tabularFeatures[i].feature.length())) {
	    action = tabularFeatures[i].action;
	    break;
	}
    }
    if (action == LyXTabular::LAST_ACTION)
	return false;

    string const val =
	    frontStrip(what.substr(tabularFeatures[i].feature.length()));
    TabularFeatures(bv, action, val);
    return true;
}


void InsetTabular::TabularFeatures(BufferView * bv,
				   LyXTabular::Feature feature,
				   string const & value)
{
    int i;
    int j;
    int sel_col_start;
    int sel_col_end;
    int sel_row_start;
    int sel_row_end;
    int setLines = 0;
    LyXAlignment setAlign = LYX_ALIGN_LEFT;
    LyXTabular::VAlignment setVAlign = LyXTabular::LYX_VALIGN_TOP;
    int lineSet;
    bool what;

    switch (feature) {
      case LyXTabular::M_ALIGN_LEFT:
      case LyXTabular::ALIGN_LEFT:
          setAlign=LYX_ALIGN_LEFT;
          break;
      case LyXTabular::M_ALIGN_RIGHT:
      case LyXTabular::ALIGN_RIGHT:
          setAlign=LYX_ALIGN_RIGHT;
          break;
      case LyXTabular::M_ALIGN_CENTER:
      case LyXTabular::ALIGN_CENTER:
          setAlign=LYX_ALIGN_CENTER;
          break;
      case LyXTabular::M_VALIGN_TOP:
      case LyXTabular::VALIGN_TOP:
          setVAlign=LyXTabular::LYX_VALIGN_TOP;
          break;
      case LyXTabular::M_VALIGN_BOTTOM:
      case LyXTabular::VALIGN_BOTTOM:
          setVAlign=LyXTabular::LYX_VALIGN_BOTTOM;
          break;
      case LyXTabular::M_VALIGN_CENTER:
      case LyXTabular::VALIGN_CENTER:
          setVAlign=LyXTabular::LYX_VALIGN_CENTER;
          break;
      default:
          break;
    }
    if (hasSelection()) {
	sel_col_start = tabular->column_of_cell(sel_cell_start);
	sel_col_end = tabular->column_of_cell(sel_cell_end);
	if (sel_col_start > sel_col_end) {
	    sel_col_end = sel_col_start;
	    sel_col_start = tabular->column_of_cell(sel_cell_end);
	} else {
	    sel_col_end = tabular->right_column_of_cell(sel_cell_end);
	}
	
	sel_row_start = tabular->row_of_cell(sel_cell_start);
	sel_row_end = tabular->row_of_cell(sel_cell_end);
	if (sel_row_start > sel_row_end) {
		//int tmp = sel_row_start;
		//sel_row_start = sel_row_end;
		//sel_row_end = tmp;
	    swap(sel_row_start, sel_row_end);
	}
    } else {
	sel_col_start = sel_col_end = tabular->column_of_cell(actcell);
	sel_row_start = sel_row_end = tabular->row_of_cell(actcell);
    }
    bv->text->SetUndo(bv->buffer(), Undo::FINISH,
#ifndef NEW_INSETS
	      bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	      bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next
#else
	      bv->text->cursor.par()->previous,
	      bv->text->cursor.par()->next
#endif
	    );

    int row = tabular->row_of_cell(actcell);
    int column = tabular->column_of_cell(actcell);
    bool flag = true;
    
    switch (feature) {
    case LyXTabular::SET_PWIDTH:
    {
	bool const update = (tabular->GetColumnPWidth(actcell) != value);
	tabular->SetColumnPWidth(actcell,value);
	if (update) {
	    for (int i=0; i < tabular->rows(); ++i) {
		tabular->GetCellInset(tabular->GetCellNumber(i, column))->
		    resizeLyXText(bv);
	    }
	    UpdateLocal(bv, INIT, true);
	}
    }
    break;
    case LyXTabular::SET_MPWIDTH:
    {
	bool const update = (tabular->GetPWidth(actcell) != value);
	tabular->SetMColumnPWidth(actcell,value);
	if (update) {
	    for (int i=0; i < tabular->rows(); ++i) {
		tabular->GetCellInset(tabular->GetCellNumber(i, column))->
		    resizeLyXText(bv);
	    }
	    UpdateLocal(bv, INIT, true);
	}
    }
    break;
    case LyXTabular::SET_SPECIAL_COLUMN:
    case LyXTabular::SET_SPECIAL_MULTI:
	tabular->SetAlignSpecial(actcell,value,feature);
	break;
    case LyXTabular::APPEND_ROW:
	// append the row into the tabular
	UnlockInsetInInset(bv, the_locking_inset);
	tabular->AppendRow(actcell);
	UpdateLocal(bv, INIT, true);
	break;
    case LyXTabular::APPEND_COLUMN:
	// append the column into the tabular
	tabular->AppendColumn(actcell);
	actcell = tabular->GetCellNumber(row, column);
	UpdateLocal(bv, INIT, true);
	break;
    case LyXTabular::DELETE_ROW:
	tabular->DeleteRow(tabular->row_of_cell(actcell));
	if ((row+1) > tabular->rows())
	    --row;
	actcell = tabular->GetCellNumber(row, column);
	clearSelection();
	UpdateLocal(bv, INIT, true);
	break;
    case LyXTabular::DELETE_COLUMN:
	tabular->DeleteColumn(tabular->column_of_cell(actcell));
	if ((column+1) > tabular->columns())
	    --column;
	actcell = tabular->GetCellNumber(row, column);
	clearSelection();
	UpdateLocal(bv, INIT, true);
	break;
    case LyXTabular::M_TOGGLE_LINE_TOP:
	flag = false;
    case LyXTabular::TOGGLE_LINE_TOP:
	lineSet = !tabular->TopLine(actcell, flag);
	for (i=sel_row_start; i<=sel_row_end; ++i)
	    for (j=sel_col_start; j<=sel_col_end; ++j)
		tabular->SetTopLine(tabular->GetCellNumber(i,j),lineSet, flag);
	UpdateLocal(bv, INIT, true);
	break;
    
    case LyXTabular::M_TOGGLE_LINE_BOTTOM:
	flag = false;
    case LyXTabular::TOGGLE_LINE_BOTTOM:
	lineSet = !tabular->BottomLine(actcell, flag); 
	for (i=sel_row_start; i<=sel_row_end; ++i)
	    for (j=sel_col_start; j<=sel_col_end; ++j)
		tabular->SetBottomLine(tabular->GetCellNumber(i,j),lineSet,
				       flag);
	UpdateLocal(bv, INIT, true);
	break;
		
    case LyXTabular::M_TOGGLE_LINE_LEFT:
	flag = false;
    case LyXTabular::TOGGLE_LINE_LEFT:
	lineSet = !tabular->LeftLine(actcell, flag);
	for (i=sel_row_start; i<=sel_row_end; ++i)
	    for (j=sel_col_start; j<=sel_col_end; ++j)
		tabular->SetLeftLine(tabular->GetCellNumber(i,j),lineSet,
				     flag);
	UpdateLocal(bv, INIT, true);
	break;

    case LyXTabular::M_TOGGLE_LINE_RIGHT:
	flag = false;
    case LyXTabular::TOGGLE_LINE_RIGHT:
	lineSet = !tabular->RightLine(actcell, flag);
	for (i=sel_row_start; i<=sel_row_end; ++i)
	    for (j=sel_col_start; j<=sel_col_end; ++j)
		tabular->SetRightLine(tabular->GetCellNumber(i,j),lineSet,
				      flag);
	UpdateLocal(bv, INIT, true);
	break;
    case LyXTabular::M_ALIGN_LEFT:
    case LyXTabular::M_ALIGN_RIGHT:
    case LyXTabular::M_ALIGN_CENTER:
	flag = false;
    case LyXTabular::ALIGN_LEFT:
    case LyXTabular::ALIGN_RIGHT:
    case LyXTabular::ALIGN_CENTER:
	for (i = sel_row_start; i <= sel_row_end; ++i)
	    for (j = sel_col_start; j <= sel_col_end; ++j)
		tabular->SetAlignment(tabular->GetCellNumber(i, j), setAlign,
				      flag);
	if (hasSelection())
	    UpdateLocal(bv, INIT, true);
	else
	    UpdateLocal(bv, CELL, true);
	break;
    case LyXTabular::M_VALIGN_TOP:
    case LyXTabular::M_VALIGN_BOTTOM:
    case LyXTabular::M_VALIGN_CENTER:
	flag = false;
    case LyXTabular::VALIGN_TOP:
    case LyXTabular::VALIGN_BOTTOM:
    case LyXTabular::VALIGN_CENTER:
	for (i = sel_row_start; i <= sel_row_end; ++i)
	    for (j = sel_col_start; j <= sel_col_end; ++j)
		tabular->SetVAlignment(tabular->GetCellNumber(i, j),
				       setVAlign, flag);
	if (hasSelection())
	    UpdateLocal(bv, INIT, true);
	else
	    UpdateLocal(bv, CELL, true);
	break;
    case LyXTabular::MULTICOLUMN:
    {
	if (sel_row_start != sel_row_end) {
	    WriteAlert(_("Impossible Operation!"), 
		       _("Multicolumns can only be horizontally."), 
		       _("Sorry."));
	    return;
	}
	// just multicol for one Single Cell
	if (!hasSelection()) {
	    // check wether we are completly in a multicol
	    if (tabular->IsMultiColumn(actcell)) {
		tabular->UnsetMultiColumn(actcell);
		UpdateLocal(bv, INIT, true);
	    } else {
		tabular->SetMultiColumn(actcell, 1);
		UpdateLocal(bv, CELL, true);
	    }
	    return;
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
	tabular->SetMultiColumn(s_start, s_end - s_start + 1);
	actcell = s_start;
	cursor.pos(0);
	sel_cell_end = sel_cell_start;
	sel_pos_end = sel_pos_start;
	UpdateLocal(bv, INIT, true);
	break;
    }
    case LyXTabular::SET_ALL_LINES:
	setLines = 1;
    case LyXTabular::UNSET_ALL_LINES:
	for (i=sel_row_start; i<=sel_row_end; ++i)
	    for (j=sel_col_start; j<=sel_col_end; ++j)
		tabular->SetAllLines(tabular->GetCellNumber(i,j), setLines);
	UpdateLocal(bv, INIT, true);
	break;
    case LyXTabular::SET_LONGTABULAR:
	tabular->SetLongTabular(true);
	UpdateLocal(bv, INIT, true); // because this toggles displayed
	break;
    case LyXTabular::UNSET_LONGTABULAR:
	tabular->SetLongTabular(false);
	UpdateLocal(bv, INIT, true); // because this toggles displayed
	break;
    case LyXTabular::SET_ROTATE_TABULAR:
	tabular->SetRotateTabular(true);
	break;
    case LyXTabular::UNSET_ROTATE_TABULAR:
	tabular->SetRotateTabular(false);
	break;
    case LyXTabular::SET_ROTATE_CELL:
	for (i=sel_row_start; i<=sel_row_end; ++i)
	    for (j=sel_col_start; j<=sel_col_end; ++j)
		tabular->SetRotateCell(tabular->GetCellNumber(i,j),true);
	break;
    case LyXTabular::UNSET_ROTATE_CELL:
	for (i = sel_row_start; i <= sel_row_end; ++i)
	    for (j = sel_col_start; j <= sel_col_end; ++j)
		tabular->SetRotateCell(tabular->GetCellNumber(i, j), false);
	break;
    case LyXTabular::SET_USEBOX:
    {
	LyXTabular::BoxType val = LyXTabular::BoxType(strToInt(value));
	if (val == tabular->GetUsebox(actcell))
	    val = LyXTabular::BOX_NONE;
	for (i = sel_row_start; i <= sel_row_end; ++i)
	    for (j = sel_col_start; j <= sel_col_end; ++j)
		tabular->SetUsebox(tabular->GetCellNumber(i, j), val);
	break;
    }
    case LyXTabular::SET_LTFIRSTHEAD:
	tabular->SetLTHead(actcell, true);
	break;
    case LyXTabular::SET_LTHEAD:
	tabular->SetLTHead(actcell, false);
	break;
    case LyXTabular::SET_LTFOOT:
	tabular->SetLTFoot(actcell, false);
	break;
    case LyXTabular::SET_LTLASTFOOT:
	tabular->SetLTFoot(actcell, true);
	break;
    case LyXTabular::SET_LTNEWPAGE:
	what = !tabular->GetLTNewPage(actcell);
	tabular->SetLTNewPage(actcell, what);
	break;
    // dummy stuff just to avoid warnings
    case LyXTabular::LAST_ACTION:
	break;
    }
}


bool InsetTabular::ActivateCellInset(BufferView * bv, int x, int y, int button,
				     bool behind)
{
    // the cursor.pos has to be before the inset so if it isn't now just
    // reset the curor pos first!
    if (!cellstart(cursor.pos())) {
	cursor.pos(0);
	resetPos(bv);
    }
    UpdatableInset * inset =
	static_cast<UpdatableInset*>(tabular->GetCellInset(actcell));
    LyXFont font(LyXFont::ALL_SANE);
    if (behind) {
	x = inset->x() + inset->width(bv, font);
	y = inset->descent(bv, font);
    }
    inset_x = cursor.x() - top_x + tabular->GetBeginningOfTextInCell(actcell);
    inset_y = cursor.y();
    inset->Edit(bv, x - inset_x, y - inset_y, button);
    if (!the_locking_inset)
	return false;
    UpdateLocal(bv, CELL, false);
    return (the_locking_inset != 0);
}


bool InsetTabular::InsetHit(BufferView * bv, int x, int ) const
{
    InsetText * inset = tabular->GetCellInset(actcell);
    int const x1 = x + top_x;

    if (!cellstart(cursor.pos())) {
	return ((x + top_x) < cursor.x() &&
		(x + top_x) > (cursor.x() - inset->width(bv,
						      LyXFont(LyXFont::ALL_SANE))));
    } else {
	int const x2 = cursor.x() + tabular->GetBeginningOfTextInCell(actcell);
	return (x1 > x2 &&
		x1 < (x2 + inset->width(bv, LyXFont(LyXFont::ALL_SANE))));
    }
}


// This returns paperWidth() if the cell-width is unlimited or the width
// in pixels if we have a pwidth for this cell.
int InsetTabular::GetMaxWidthOfCell(Painter &, int cell) const
{
    string const s = tabular->GetPWidth(cell);

    if (s.empty())
	return -1;
    return VSpace(s).inPixels(0, 0);
}


int InsetTabular::getMaxWidth(Painter & pain,
			      UpdatableInset const * inset) const
{
    int const n = tabular->GetNumberOfCells();
    int cell = 0;
    for (; cell < n; ++cell) {
	if (tabular->GetCellInset(cell) == inset)
	    break;
    }
    if (cell >= n)
	return -1;
    int w = GetMaxWidthOfCell(pain, cell);
    if (w > 0)
	// because the inset then subtracts it's top_x and owner->x()
	w += (inset->x() - top_x);
    return w;
}


void InsetTabular::resizeLyXText(BufferView *) const
{
    need_update = FULL;
}


LyXText * InsetTabular::getLyXText(BufferView const * bv) const
{
    if (the_locking_inset)
	return the_locking_inset->getLyXText(bv);
    return Inset::getLyXText(bv);
}


void InsetTabular::OpenLayoutDialog(BufferView * bv) const
{
    if (the_locking_inset) {
	InsetTabular * i = static_cast<InsetTabular *>
	    (the_locking_inset->GetFirstLockingInsetOfType(TABULAR_CODE));
	if (i) {
	    i->OpenLayoutDialog(bv);
	    return;
	}
    }
    bv->owner()->getDialogs()->showTabular(const_cast<InsetTabular *>(this));
}

//
// functions returns:
// 0 ... disabled
// 1 ... enabled
// 2 ... toggled on
// 3 ... toggled off
//
LyXFunc::func_status InsetTabular::getStatus(string const & what) const
{
    int action = LyXTabular::LAST_ACTION;
    LyXFunc::func_status status = LyXFunc::OK;
    
    int i = 0;
    for (; tabularFeatures[i].action != LyXTabular::LAST_ACTION; ++i) {
	string const tmp = tabularFeatures[i].feature;
	if (tmp == what.substr(0, tmp.length())) {		    
	//if (!strncmp(tabularFeatures[i].feature.c_str(), what.c_str(),
	//   tabularFeatures[i].feature.length())) {
	    action = tabularFeatures[i].action;
	    break;
	}
    }
    if (action == LyXTabular::LAST_ACTION)
	return LyXFunc::Unknown;

    string const argument = frontStrip(what.substr(tabularFeatures[i].feature.length()));

    int sel_row_start, sel_row_end;
    int dummy;
    bool flag = true;

    if (hasSelection()) {
	sel_row_start = tabular->row_of_cell(sel_cell_start);
	sel_row_end = tabular->row_of_cell(sel_cell_end);
	if (sel_row_start > sel_row_end) {
		//int tmp = sel_row_start;
		//sel_row_start = sel_row_end;
		//sel_row_end = tmp;
	    swap(sel_row_start, sel_row_end);
	}
    } else {
	sel_row_start = sel_row_end = tabular->row_of_cell(actcell);
    }

    switch (action) {
    case LyXTabular::SET_PWIDTH:
    case LyXTabular::SET_MPWIDTH:
    case LyXTabular::SET_SPECIAL_COLUMN:
    case LyXTabular::SET_SPECIAL_MULTI:
	status |= LyXFunc::Disabled;
	return status;

    case LyXTabular::APPEND_ROW:
    case LyXTabular::APPEND_COLUMN:
    case LyXTabular::DELETE_ROW:
    case LyXTabular::DELETE_COLUMN:
    case LyXTabular::SET_ALL_LINES:
    case LyXTabular::UNSET_ALL_LINES:
	status |= LyXFunc::OK;
	return status;

    case LyXTabular::MULTICOLUMN:
	if (tabular->IsMultiColumn(actcell))
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::M_TOGGLE_LINE_TOP:
	flag = false;
    case LyXTabular::TOGGLE_LINE_TOP:
	if (tabular->TopLine(actcell, flag))
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::M_TOGGLE_LINE_BOTTOM:
	flag = false;
    case LyXTabular::TOGGLE_LINE_BOTTOM:
	if (tabular->BottomLine(actcell, flag))
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::M_TOGGLE_LINE_LEFT:
	flag = false;
    case LyXTabular::TOGGLE_LINE_LEFT:
	if (tabular->LeftLine(actcell, flag))
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::M_TOGGLE_LINE_RIGHT:
	flag = false;
    case LyXTabular::TOGGLE_LINE_RIGHT:
	if (tabular->RightLine(actcell, flag))
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::M_ALIGN_LEFT:
	flag = false;
    case LyXTabular::ALIGN_LEFT:
	if (tabular->GetAlignment(actcell, flag) == LYX_ALIGN_LEFT)
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::M_ALIGN_RIGHT:
	flag = false;
    case LyXTabular::ALIGN_RIGHT:
	if (tabular->GetAlignment(actcell, flag) == LYX_ALIGN_RIGHT)
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::M_ALIGN_CENTER:
	flag = false;
    case LyXTabular::ALIGN_CENTER:
	if (tabular->GetAlignment(actcell, flag) == LYX_ALIGN_CENTER)
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::M_VALIGN_TOP:
	flag = false;
    case LyXTabular::VALIGN_TOP:
	if (tabular->GetVAlignment(actcell, flag) == LyXTabular::LYX_VALIGN_TOP)
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::M_VALIGN_BOTTOM:
	flag = false;
    case LyXTabular::VALIGN_BOTTOM:
	if (tabular->GetVAlignment(actcell, flag) == LyXTabular::LYX_VALIGN_BOTTOM)
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::M_VALIGN_CENTER:
	flag = false;
    case LyXTabular::VALIGN_CENTER:
	if (tabular->GetVAlignment(actcell, flag) == LyXTabular::LYX_VALIGN_CENTER)
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::SET_LONGTABULAR:
	if (tabular->IsLongTabular())
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::UNSET_LONGTABULAR:
	if (!tabular->IsLongTabular())
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::SET_ROTATE_TABULAR:
	if (tabular->GetRotateTabular())
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::UNSET_ROTATE_TABULAR:
	if (!tabular->GetRotateTabular())
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::SET_ROTATE_CELL:
	if (tabular->GetRotateCell(actcell))
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::UNSET_ROTATE_CELL:
	if (!tabular->GetRotateCell(actcell))
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::SET_USEBOX:
	if (strToInt(argument) == tabular->GetUsebox(actcell))
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::SET_LTFIRSTHEAD:
	if (tabular->GetRowOfLTHead(actcell, dummy))
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::SET_LTHEAD:
	if (tabular->GetRowOfLTHead(actcell, dummy))
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::SET_LTFOOT:
	if (tabular->GetRowOfLTFoot(actcell, dummy))
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::SET_LTLASTFOOT:
	if (tabular->GetRowOfLTFoot(actcell, dummy))
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    case LyXTabular::SET_LTNEWPAGE:
	if (tabular->GetLTNewPage(actcell))
	    status |= LyXFunc::ToggleOn;
	else
	    status |= LyXFunc::ToggleOff;
	break;
    default:
	status = LyXFunc::Disabled;
	break;
    }
    return status;
}


bool InsetTabular::copySelection(BufferView * bv)
{
    if (!hasSelection())
	return false;
    delete paste_tabular;

    int sel_col_start, sel_col_end;
    int sel_row_start, sel_row_end;

    sel_col_start = tabular->column_of_cell(sel_cell_start);
    sel_col_end = tabular->column_of_cell(sel_cell_end);
    if (sel_col_start > sel_col_end) {
	sel_col_start = sel_col_end;
	sel_col_end = tabular->right_column_of_cell(sel_cell_start);
    } else {
	sel_col_end = tabular->right_column_of_cell(sel_cell_end);
    }
    int columns = sel_col_end - sel_col_start + 1;

    sel_row_start = tabular->row_of_cell(sel_cell_start);
    sel_row_end = tabular->row_of_cell(sel_cell_end);
    if (sel_row_start > sel_row_end) {
	    //int tmp tmp = sel_row_start;
	    //sel_row_start = sel_row_end;
	    //sel_row_end = tmp;
	swap(sel_row_start, sel_row_end);
    }
    int rows = sel_row_end - sel_row_start + 1;

    paste_tabular = new LyXTabular(this, *tabular); // rows, columns);
    int i;
    for (i=0; i < sel_row_start; ++i)
	paste_tabular->DeleteRow(0);
    while(paste_tabular->rows() > rows)
	paste_tabular->DeleteRow(rows);
    paste_tabular->SetTopLine(0, true, true);
    paste_tabular->SetBottomLine(paste_tabular->GetFirstCellInRow(rows-1),
				 true, true);
    for (i=0; i < sel_col_start; ++i)
	paste_tabular->DeleteColumn(0);
    while(paste_tabular->columns() > columns)
	paste_tabular->DeleteColumn(columns);
    paste_tabular->SetLeftLine(0, true, true);
    paste_tabular->SetRightLine(paste_tabular->GetLastCellInRow(0),true, true);

    ostringstream sstr;
    paste_tabular->Ascii(bv->buffer(), sstr);
    bv->stuffClipboard(sstr.str().c_str());
    return true;
}


bool InsetTabular::pasteSelection(BufferView * bv)
{
    if (!paste_tabular)
	return false;
    for (int j=0, i=actcell; j<paste_tabular->GetNumberOfCells(); ++j,++i) {
	while (paste_tabular->row_of_cell(j) > tabular->row_of_cell(i)-actrow)
	    ++i;
	if (tabular->GetNumberOfCells() <= i)
	    break;
	while (paste_tabular->row_of_cell(j) < tabular->row_of_cell(i)-actrow)
	    ++j;
	if (paste_tabular->GetNumberOfCells() <= j)
	    break;
	*(tabular->GetCellInset(i)) = *(paste_tabular->GetCellInset(j));
	tabular->GetCellInset(i)->setOwner(this);
	tabular->GetCellInset(i)->deleteLyXText(bv);
    }
    return true;
}


bool InsetTabular::cutSelection()
{
    if (!hasSelection())
	return false;

    int sel_col_start, sel_col_end;
    int sel_row_start, sel_row_end;

    sel_col_start = tabular->column_of_cell(sel_cell_start);
    sel_col_end = tabular->column_of_cell(sel_cell_end);
    if (sel_col_start > sel_col_end) {
	sel_col_start = sel_col_end;
	sel_col_end = tabular->right_column_of_cell(sel_cell_start);
    } else {
	sel_col_end = tabular->right_column_of_cell(sel_cell_end);
    }
    sel_row_start = tabular->row_of_cell(sel_cell_start);
    sel_row_end = tabular->row_of_cell(sel_cell_end);
    if (sel_row_start > sel_row_end) {
	    //int tmp = sel_row_start;
	    //sel_row_start = sel_row_end;
	    //sel_row_end = tmp;
	swap(sel_row_start, sel_row_end);
    }
    if (sel_cell_start > sel_cell_end) {
	    //int tmp = sel_cell_start;
	    //sel_cell_start = sel_cell_end;
	    //sel_cell_end = tmp;
	swap(sel_cell_start, sel_cell_end);
    }
    for (int i = sel_row_start; i <= sel_row_end; ++i) {
	for (int j = sel_col_start; j <= sel_col_end; ++j) {
	    tabular->GetCellInset(tabular->GetCellNumber(i, j))->clear();
	}
    }
    return true;
}

