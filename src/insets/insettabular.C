/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 *======================================================
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
#include "LaTeXFeatures.h"
#include "Painter.h"
#include "font.h"
#include "lyxtext.h"
#include "lyx_gui_misc.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "insets/insettext.h"

extern void MenuLayoutTabular(bool, InsetTabular *);
extern bool UpdateLayoutTabular(bool, InsetTabular *);
extern void TabularOptClose();

const int ADD_TO_HEIGHT = 2;
const int ADD_TO_TABULAR_WIDTH = 2;

using std::ostream;
using std::ifstream;
using std::max;
using std::endl;

#define cellstart(p) ((p % 2) == 0)

InsetTabular::InsetTabular(Buffer * buf, int rows, int columns)
{
    if (rows <= 0)
        rows = 1;
    if (columns <= 0)
        columns = 1;
    buffer = buf; // set this first!!!
    tabular = new LyXTabular(this, rows,columns);
    // for now make it always display as display() inset
    // just for test!!!
    the_locking_inset = 0;
    locked = no_selection = cursor_visible = false;
    cursor.x_fix = -1;
    oldcell = -1;
    actcell = cursor.pos = sel_pos_start = sel_pos_end = 0;
    init_inset = true;
}


InsetTabular::InsetTabular(InsetTabular const & tab, Buffer * buf)
{
    buffer = buf; // set this first
    tabular = new LyXTabular(this, *(tab.tabular));
    the_locking_inset = 0;
    locked = no_selection = cursor_visible = false;
    cursor.x_fix = -1;
    oldcell = -1;
    actcell = cursor.pos = sel_pos_start = sel_pos_end = 0;
    init_inset = true;
}


InsetTabular::~InsetTabular()
{
    delete tabular;
}


Inset * InsetTabular::Clone() const
{
    InsetTabular * t = new InsetTabular(*this, buffer);
    delete t->tabular;
    t->tabular = tabular->Clone(t);
    return t;
}


void InsetTabular::Write(ostream & os) const
{
    os << " Tabular" << endl;
    tabular->Write(os);
}


void InsetTabular::Read(LyXLex & lex)
{
    bool old_format = (lex.GetString() == "\\LyXTable");
    string token;

    if (tabular)
	delete tabular;
    tabular = new LyXTabular(this, lex);

    init_inset = true;

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


int InsetTabular::ascent(Painter & pain, LyXFont const & font) const
{
    if (init_inset) {
	calculate_width_of_cells(pain, font);
	init_inset = false;
    }
    return tabular->GetAscentOfRow(0);
}


int InsetTabular::descent(Painter & pain, LyXFont const & font) const
{
    if (init_inset) {
	calculate_width_of_cells(pain, font);
	init_inset = false;
    }
    return tabular->GetHeightOfTabular() - tabular->GetAscentOfRow(0);
}


int InsetTabular::width(Painter & pain, LyXFont const & font) const
{
    if (init_inset) {
	calculate_width_of_cells(pain, font);
	init_inset = false;
    }
    return tabular->GetWidthOfTabular() + (2 * ADD_TO_TABULAR_WIDTH);
}


void InsetTabular::draw(Painter & pain, const LyXFont & font, int baseline,
			float & x) const
{
    int i, j, cell=0;
    int nx;
    float cx;
    bool reinit = false;

    UpdatableInset::draw(pain,font,baseline,x);
    if (init_inset || (top_x != int(x)) || (top_baseline != baseline)) {
	int ox = top_x;
	init_inset = false;
	top_x = int(x);
	top_baseline = baseline;
	if (ox != top_x)
	    recomputeTextInsets(pain, font);
	calculate_width_of_cells(pain, font);
	resetPos(pain);
	reinit = true;
    }
    x += ADD_TO_TABULAR_WIDTH;
    for(i=0;i<tabular->rows();++i) {
	nx = int(x);
        for(j=0;j<tabular->columns();++j) {
	    if (tabular->IsPartOfMultiColumn(i,j))
		continue;
	    cx = nx + tabular->GetBeginningOfTextInCell(cell);
	    if (hasSelection())
		DrawCellSelection(pain, nx, baseline, i, cell);
	    tabular->GetCellInset(cell)->draw(pain, font, baseline, cx);
	    DrawCellLines(pain, nx, baseline, i, cell);
	    nx += tabular->GetWidthOfColumn(cell);
	    ++cell;
	}
        baseline += tabular->GetDescentOfRow(i) + tabular->GetAscentOfRow(i+1)
            + tabular->GetAdditionalHeight(cell+1);
    }
    x += width(pain, font);
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
		  on_off ? LColor::tabularonoffline:LColor::tabularline,
		  on_off ? Painter::line_onoffdash:Painter::line_solid);
    }
    on_off = !tabular->BottomLine(cell);
    pain.line(x,baseline +  tabular->GetDescentOfRow(row),
	      x2, baseline +  tabular->GetDescentOfRow(row),
	      on_off ? LColor::tabularonoffline:LColor::tabularline,
	      on_off ? Painter::line_onoffdash:Painter::line_solid);
    if (!tabular->LeftAlreadyDrawed(cell)) {
	on_off = !tabular->LeftLine(cell);
	pain.line(x, baseline -  tabular->GetAscentOfRow(row),
		  x, baseline +  tabular->GetDescentOfRow(row),
		  on_off ? LColor::tabularonoffline:LColor::tabularline,
		  on_off ? Painter::line_onoffdash:Painter::line_solid);
    }
    on_off = !tabular->RightLine(cell);
    pain.line(x2 - tabular->GetAdditionalWidth(cell),
              baseline -  tabular->GetAscentOfRow(row),
              x2 - tabular->GetAdditionalWidth(cell),
              baseline +  tabular->GetDescentOfRow(row),
	      on_off ? LColor::tabularonoffline:LColor::tabularline,
              on_off ? Painter::line_onoffdash:Painter::line_solid);
}


void InsetTabular::DrawCellSelection(Painter & pain, int x, int baseline,
				     int row, int cell) const
{
    int start, end;
    if (sel_cell_start > sel_cell_end) {
	start = sel_cell_end;
	end = sel_cell_start;
    } else {
	start = sel_cell_start;
	end = sel_cell_end;
    }
    int c1 = tabular->column_of_cell(cell);
    int cs = tabular->column_of_cell(start);
    int ce;
    if (tabular->IsLastCellInRow(end))
	ce = tabular->columns() - 1;
    else
	ce = tabular->column_of_cell(end+1) - 1; // because of multic.
    int rs = tabular->row_of_cell(start);
    int re = tabular->row_of_cell(end);

    if ((c1 >= cs) && (c1 <= ce) && (row >= rs) && (row <= re)) {
	int w = tabular->GetWidthOfColumn(cell);
	int h = tabular->GetAscentOfRow(row) + tabular->GetDescentOfRow(row);
	pain.fillRectangle(x, baseline - tabular->GetAscentOfRow(row),
			   w, h, LColor::selection);
    }
}


char const * InsetTabular::EditMessage() const
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
    inset_pos = inset_x = inset_y = 0;
    setPos(bv->painter(), x, y);
    sel_pos_start = sel_pos_end = cursor.pos;
    sel_cell_start = sel_cell_end = actcell;
    bv->text->FinishUndo();
    if (InsetHit(bv, x, y)) {
	ActivateCellInset(bv, x, y, button);
    }
    UpdateLocal(bv, true);
//    bv->getOwner()->getPopups().updateFormTabular();
}


void InsetTabular::InsetUnlock(BufferView * bv)
{
    TabularOptClose();
    if (the_locking_inset) {
	the_locking_inset->InsetUnlock(bv);
	the_locking_inset = 0;
    }
    HideInsetCursor(bv);
    if (hasSelection()) {
	sel_pos_start = sel_pos_end = cursor.pos;
	sel_cell_start = sel_cell_end = actcell;
	UpdateLocal(bv, false);
    }
    no_selection = false;
    oldcell = -1;
    locked = false;
}

void InsetTabular::UpdateLocal(BufferView * bv, bool flag)
{
    if (flag)
	calculate_width_of_cells(bv->painter(), LyXFont(LyXFont::ALL_SANE));
    bv->updateInset(this, flag);
    if (flag)
	resetPos(bv->painter());
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
	resetPos(bv->painter());
	inset_x = cursor.x - top_x + tabular->GetBeginningOfTextInCell(actcell);
	inset_y = cursor.y;
	inset_pos = cursor.pos;
	return true;
    } else if (the_locking_inset && (the_locking_inset == inset)) {
	if (cursor.pos == inset_pos) {
	    lyxerr[Debug::INSETS] << "OK" << endl;
	    resetPos(bv->painter());
	    inset_x = cursor.x - top_x + tabular->GetBeginningOfTextInCell(actcell);
	    inset_y = cursor.y;
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
        return true;
    }
    if (the_locking_inset->UnlockInsetInInset(bv, inset, lr)) {
	if ((inset->LyxCode() == TABULAR_CODE) &&
	    !the_locking_inset->GetFirstLockingInsetOfType(TABULAR_CODE))
	{
	    UpdateLayoutTabular(true, const_cast<InsetTabular *>(this));
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
    if (the_locking_inset != inset)
	return the_locking_inset->UpdateInsetInInset(bv, inset);
    UpdateLocal(bv);
    return true;
}


int InsetTabular::InsetInInsetY()
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
	MenuLayoutTabular(true, this);
	return;
    }
    if (the_locking_inset) {
        the_locking_inset->InsetButtonRelease(bv, x-inset_x, y-inset_y,button);
        return;
    }
    no_selection = false;
}


void InsetTabular::InsetButtonPress(BufferView * bv, int x, int y, int button)
{
    if (hasSelection()) {
	sel_pos_start = sel_pos_end = sel_cell_start = sel_cell_end = 0;
	UpdateLocal(bv, false);
    }
    no_selection = false;

    int ocell = actcell;

    setPos(bv->painter(), x, y);
    sel_pos_start = sel_pos_end = cursor.pos;
    sel_cell_start = sel_cell_end = actcell;

    bool inset_hit = InsetHit(bv, x, y);

    if ((ocell == actcell) && the_locking_inset && inset_hit) {
        the_locking_inset->InsetButtonPress(bv, x-inset_x, y-inset_y, button);
        return;
    } else if (the_locking_inset) {
	the_locking_inset->InsetUnlock(bv);
    }
    the_locking_inset = 0;
    if (inset_hit && bv->the_locking_inset) {
	ActivateCellInset(bv, x, y, button);
	the_locking_inset->InsetButtonPress(bv, x-inset_x, y-inset_y, button);
    }
    
#if 0
    if (button == 3)
        bview->getOwner()->getPopups().showFormTabular();
    else if (ocell != actcell)
        bview->getOwner()->getPopups().updateFormTabular();
#endif
}


void InsetTabular::InsetMotionNotify(BufferView * bv, int x, int y, int button)
{
    if (the_locking_inset) {
        the_locking_inset->InsetMotionNotify(bv, x - inset_x,
					     y - inset_y, button);
        return;
    }
    if (!no_selection) {
	    // int ocell = actcell,
	    int old = sel_pos_end;

	setPos(bv->painter(), x, y);
	sel_pos_end = cursor.pos;
	sel_cell_end = actcell;
	if (old != sel_pos_end)
	    UpdateLocal(bv, false);
#if 0
	if (ocell != actcell)
	    bview->getOwner()->getPopups().updateFormTabular();
#endif
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

	resetPos(bv->painter());
	return result;
    }
    result=DISPATCHED;

    if ((action < 0) && arg.empty())
        return FINISHED;

    if ((action != LFUN_DOWN) && (action != LFUN_UP) &&
        (action != LFUN_DOWNSEL) && (action != LFUN_UPSEL))
        cursor.x_fix = -1;
    if (the_locking_inset) {
        result=the_locking_inset->LocalDispatch(bv, action, arg);
	if (result == DISPATCHED_NOUPDATE)
	    return result;
	else if (result == DISPATCHED) {
            bool upd = SetCellDimensions(bv->painter(), actcell, actrow);
	    the_locking_inset->ToggleInsetCursor(bv);
	    UpdateLocal(bv, upd);
	    the_locking_inset->ToggleInsetCursor(bv);
            return result;
        } else if (result == FINISHED) {
	    if ((action == LFUN_RIGHT) || (action == -1)) {
		cursor.pos = inset_pos + 1;
		resetPos(bv->painter());
	    }
	    the_locking_inset=0;
	    result = DISPATCHED;
	    return result;
	}
    }

    bool hs = hasSelection();
    HideInsetCursor(bv);
    switch (action) {
	// Normal chars not handled here
    case -1:
	break;
	// --- Cursor Movements ---------------------------------------------
    case LFUN_RIGHTSEL:
	moveRight(bv, false);
	sel_pos_end = cursor.pos;
	if (!cellstart(cursor.pos)) {
	    if (sel_cell_start >= actcell)
		sel_cell_end = actcell+1;
	    else
		sel_cell_end = actcell;
	}
	UpdateLocal(bv, false);
	break;
    case LFUN_RIGHT:
	result = moveRight(bv);
	sel_pos_start = sel_pos_end = cursor.pos;
	sel_cell_start = sel_cell_end = actcell;
	if (hs)
	    UpdateLocal(bv, false);
	break;
    case LFUN_LEFTSEL:
	moveLeft(bv, false);
	sel_pos_end = cursor.pos;
	if (cellstart(cursor.pos)) {
	    if (sel_cell_start >= actcell)
		sel_cell_end = actcell;
	    else
		sel_cell_end = actcell-1;
	}
	UpdateLocal(bv, false);
	break;
    case LFUN_LEFT:
	result = moveLeft(bv);
	sel_pos_start = sel_pos_end = cursor.pos;
	sel_cell_start = sel_cell_end = actcell;
	if (hs)
	    UpdateLocal(bv, false);
	break;
    case LFUN_DOWNSEL:
	moveDown(bv);
	sel_pos_end = cursor.pos;
	sel_cell_end = actcell;
	UpdateLocal(bv, false);
	break;
    case LFUN_DOWN:
	result= moveDown(bv);
	sel_pos_start = sel_pos_end = cursor.pos;
	sel_cell_start = sel_cell_end = actcell;
	if (hs)
	    UpdateLocal(bv, false);
	break;
    case LFUN_UPSEL:
	moveUp(bv);
	sel_pos_end = cursor.pos;
	sel_cell_end = actcell;
	UpdateLocal(bv, false);
	break;
    case LFUN_UP:
	result= moveUp(bv);
	sel_pos_start = sel_pos_end = cursor.pos;
	sel_cell_start = sel_cell_end = actcell;
	if (hs)
	    UpdateLocal(bv, false);
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
	    the_locking_inset->InsetUnlock(bv);
	}
	the_locking_inset = 0;
	if (action == LFUN_TAB)
	    moveNextCell(bv);
	else
	    movePrevCell(bv);
	sel_pos_start = sel_pos_end = cursor.pos;
	sel_cell_start = sel_cell_end = actcell;
	if (hs)
	    UpdateLocal(bv, false);
	break;
    case LFUN_LAYOUT_TABLE:
    {
	int flag = (arg == "true");
	MenuLayoutTabular(flag, this);
    }
    break;
    default:
	result = UNDISPATCHED;
	break;
    }
    if (result!=FINISHED) {
	if (!the_locking_inset) {
#if 0	    
	    if (ocell != actcell)
		bview->getOwner()->getPopups().updateFormTabular();
#endif
	    ShowInsetCursor(bv);
	}
    } else
        bv->unlockInset(this);
    return result;
}


int InsetTabular::Latex(ostream & os, bool fragile, bool fp) const
{
    return tabular->Latex(os, fragile, fp);
}


int InsetTabular::Ascii(ostream &) const
{
    return 0;
}

int InsetTabular::Linuxdoc(ostream &) const
{
    return 0;
}


int InsetTabular::DocBook(ostream &) const
{
    return 0;
}


void InsetTabular::Validate(LaTeXFeatures & features) const
{
    tabular->Validate(features);
}


void InsetTabular::calculate_width_of_cells(Painter & pain,
					    LyXFont const & font) const
{
    int cell = -1;
    int maxAsc, maxDesc;
    InsetText * inset;
    
    for(int i = 0; i < tabular->rows(); ++i) {
	maxAsc = maxDesc = 0;
	for(int j= 0; j < tabular->columns(); ++j) {
	    if (tabular->IsPartOfMultiColumn(i,j))
		continue;
	    ++cell;
	    inset = tabular->GetCellInset(cell);
	    maxAsc = max(maxAsc, inset->ascent(pain, font));
	    maxDesc = max(maxDesc, inset->descent(pain, font));
	    tabular->SetWidthOfCell(cell, inset->width(pain, font));
	}
	tabular->SetAscentOfRow(i, maxAsc + ADD_TO_HEIGHT);
	tabular->SetDescentOfRow(i, maxDesc + ADD_TO_HEIGHT);
    }
}


void InsetTabular::GetCursorPos(int & x, int & y) const
{
    x = cursor.x-top_x;
    y = cursor.y;
}


void InsetTabular::ToggleInsetCursor(BufferView * bv)
{
    if (the_locking_inset) {
        the_locking_inset->ToggleInsetCursor(bv);
	return;
    }

    LyXFont font; // = the_locking_inset->GetFont(par, cursor.pos);

    int asc = lyxfont::maxAscent(font);
    int desc = lyxfont::maxDescent(font);
  
    if (cursor_visible)
        bv->hideLockedInsetCursor();
    else
        bv->showLockedInsetCursor(cursor.x, cursor.y, asc, desc);
    cursor_visible = !cursor_visible;
}


void InsetTabular::ShowInsetCursor(BufferView * bv)
{
    if (!cursor_visible) {
	LyXFont font; // = GetFont(par, cursor.pos);
    
	int asc = lyxfont::maxAscent(font);
	int desc = lyxfont::maxDescent(font);
	bv->fitLockedInsetCursor(cursor.x, cursor.y, asc, desc);
	bv->showLockedInsetCursor(cursor.x, cursor.y, asc, desc);
	cursor_visible = true;
    }
}


void InsetTabular::HideInsetCursor(BufferView * bv)
{
    if (cursor_visible)
        ToggleInsetCursor(bv);
}


void InsetTabular::setPos(Painter & pain, int x, int y) const
{
    cursor.y = cursor.pos = actcell = actrow = actcol = 0;
    int ly = tabular->GetDescentOfRow(actrow);

    // first search the right row
    while((ly < y) && (actrow < tabular->rows())) {
        cursor.y += tabular->GetDescentOfRow(actrow) +
	    tabular->GetAscentOfRow(actrow+1) +
            tabular->GetAdditionalHeight(tabular->GetCellNumber(actrow+1,
								actcol));
        ++actrow;
        ly = cursor.y + tabular->GetDescentOfRow(actrow);
    }
    actcell = tabular->GetCellNumber(actrow, actcol);

    // now search the right column
    int lx = tabular->GetWidthOfColumn(actcell) -
	tabular->GetAdditionalWidth(actcell);
    for(; !tabular->IsLastCellInRow(actcell) && (lx < x);
	++actcell,lx += tabular->GetWidthOfColumn(actcell) +
	    tabular->GetAdditionalWidth(actcell-1));
    cursor.pos = ((actcell+1) * 2) - 1;
    resetPos(pain);
    if ((lx - (tabular->GetWidthOfColumn(actcell)/2)) < x) {
	cursor.x = lx + top_x - 2;
    } else {
	--cursor.pos;
	cursor.x = lx - tabular->GetWidthOfColumn(actcell) + top_x + 2;
    }
    resetPos(pain);
}

int InsetTabular::getCellXPos(int cell) const
{
    int c;

    for(c=cell;!tabular->IsFirstCellInRow(c);--c)
	;
    int lx = tabular->GetWidthOfColumn(cell);
    for(; (c < cell); ++c) {
	lx += tabular->GetWidthOfColumn(c);
    }
    return (lx - tabular->GetWidthOfColumn(cell) + top_x +
	    ADD_TO_TABULAR_WIDTH);
}

void InsetTabular::resetPos(Painter & pain) const
{
    if (!locked)
	return;
    actcol = tabular->column_of_cell(actcell);

    int cell = 0;
    actrow = cursor.y = 0;
    for(; (cell<actcell) && !tabular->IsLastRow(cell); ++cell) {
	if (tabular->IsLastCellInRow(cell)) {
	    cursor.y += tabular->GetDescentOfRow(actrow) +
		tabular->GetAscentOfRow(actrow+1) +
		tabular->GetAdditionalHeight(cell+1);
	    ++actrow;
	}
    }
    cursor.x = getCellXPos(actcell) + 2;
    if (cursor.pos % 2) {
	LyXFont font(LyXFont::ALL_SANE);
	cursor.x += tabular->GetCellInset(actcell)->width(pain,font) +
		tabular->GetBeginningOfTextInCell(actcell);
    }
    if ((!the_locking_inset ||
	 !the_locking_inset->GetFirstLockingInsetOfType(TABULAR_CODE)) &&
	(actcell != oldcell)) {
	UpdateLayoutTabular(true, const_cast<InsetTabular *>(this));
	oldcell = actcell;
    }
}


bool InsetTabular::SetCellDimensions(Painter & pain, int cell, int row)
{
    InsetText * inset = tabular->GetCellInset(cell);
    LyXFont font(LyXFont::ALL_SANE);
    int asc = inset->ascent(pain, font) + ADD_TO_HEIGHT;
    int desc = inset->descent(pain, font) + ADD_TO_HEIGHT;
    int maxAsc = tabular->GetAscentOfRow(row);
    int maxDesc = tabular->GetDescentOfRow(row);
    bool ret = tabular->SetWidthOfCell(cell, inset->width(pain, font));

    if (maxAsc < asc) {
	ret = true;
	tabular->SetAscentOfRow(row, asc);
    }
    if (maxDesc < desc) {
	ret = true;
	tabular->SetDescentOfRow(row, desc);
    }
    return ret;
}


UpdatableInset::RESULT InsetTabular::moveRight(BufferView * bv, bool lock)
{
    if (cursor.pos % 2) { // behind the inset
	++actcell;
	if (actcell >= tabular->GetNumberOfCells())
	    return FINISHED;
	++cursor.pos;
    } else if (lock) {
	if (ActivateCellInset(bv))
	    return DISPATCHED;
    } else {              // before the inset
	++cursor.pos;
    }
    resetPos(bv->painter());
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT InsetTabular::moveLeft(BufferView * bv, bool lock)
{
    if (!cursor.pos)
	return FINISHED;
    --cursor.pos;
    if (cursor.pos % 2) { // behind the inset
	--actcell;
    } else if (lock) {       // behind the inset
	if (ActivateCellInset(bv, 0, 0, 0, true))
	    return DISPATCHED;
    }
    resetPos(bv->painter());
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT InsetTabular::moveUp(BufferView * bv)
{
    int ocell = actcell;
    actcell = tabular->GetCellAbove(actcell);
    if (actcell == ocell) // we moved out of the inset
	return FINISHED;
    resetPos(bv->painter());
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT InsetTabular::moveDown(BufferView * bv)
{
    int ocell = actcell;
    actcell = tabular->GetCellBelow(actcell);
    if (actcell == ocell) // we moved out of the inset
	return FINISHED;
    resetPos(bv->painter());
    return DISPATCHED_NOUPDATE;
}


bool InsetTabular::moveNextCell(BufferView * bv)
{
    if (tabular->IsLastCell(actcell))
	return false;
    ++actcell;
    ++cursor.pos;
    if (!cellstart(cursor.pos))
	++cursor.pos;
    resetPos(bv->painter());
    return true;
}


bool InsetTabular::movePrevCell(BufferView * bv)
{
    if (!actcell) // first cell
	return false;
    --actcell;
    --cursor.pos;
    if (cellstart(cursor.pos))
	--cursor.pos;
    resetPos(bv->painter());
    return true;
}


bool InsetTabular::Delete()
{
    return true;
}


void InsetTabular::SetFont(BufferView *, LyXFont const &, bool)
{
}


void InsetTabular::TabularFeatures(BufferView * bv, int feature, string val)
{
    int
	i,
	sel_start,
	sel_end,
        setLines = 0,
        setAlign = LYX_ALIGN_LEFT,
        lineSet;
    bool
        what;

    switch (feature) {
      case LyXTabular::ALIGN_LEFT:
          setAlign=LYX_ALIGN_LEFT;
          break;
      case LyXTabular::ALIGN_RIGHT:
          setAlign=LYX_ALIGN_RIGHT;
          break;
      case LyXTabular::ALIGN_CENTER:
          setAlign=LYX_ALIGN_CENTER;
          break;
      default:
          break;
    }
    if (hasSelection()) {
	if (sel_cell_start > sel_cell_end) {
	    sel_start = sel_cell_end;
	    sel_end = sel_cell_start;
	} else {
	    sel_start = sel_cell_start;
	    sel_end = sel_cell_end;
	}
    } else
	sel_start = sel_end = actcell;
    switch (feature) {
      case LyXTabular::SET_PWIDTH:
      {
	  bool update = (tabular->GetPWidth(actcell) != val);
          tabular->SetPWidth(actcell,val);
	  if (update)
	      UpdateLocal(bv, true);
      }
      break;
      case LyXTabular::SET_SPECIAL_COLUMN:
      case LyXTabular::SET_SPECIAL_MULTI:
          tabular->SetAlignSpecial(actcell,val,feature);
          break;
      case LyXTabular::APPEND_ROW:
      {
          // append the row into the tabular
          tabular->AppendRow(actcell);
          UpdateLocal(bv, true);
          break;
      }
      case LyXTabular::APPEND_COLUMN:
      { 
          // append the column into the tabular
          tabular->AppendColumn(actcell);
          UpdateLocal(bv, true);
          break;
      }
      case LyXTabular::DELETE_ROW:
          RemoveTabularRow();
          UpdateLocal(bv, true);
          break;
      case LyXTabular::DELETE_COLUMN:
      {
          /* delete the column from the tabular */ 
          tabular->DeleteColumn(actcell);
          UpdateLocal(bv, true);
          break;
      }
      case LyXTabular::TOGGLE_LINE_TOP:
          lineSet = !tabular->TopLine(actcell);
	  for(i=sel_start; i<=sel_end; ++i)
	      tabular->SetTopLine(i,lineSet);
          UpdateLocal(bv, true);
          break;
    
      case LyXTabular::TOGGLE_LINE_BOTTOM:
          lineSet = !tabular->BottomLine(actcell); 
	  for(i=sel_start; i<=sel_end; ++i)
	      tabular->SetBottomLine(i,lineSet);
          UpdateLocal(bv, true);
          break;
		
      case LyXTabular::TOGGLE_LINE_LEFT:
          lineSet = !tabular->LeftLine(actcell);
	  for(i=sel_start; i<=sel_end; ++i)
	      tabular->SetLeftLine(i,lineSet);
          UpdateLocal(bv, true);
          break;

      case LyXTabular::TOGGLE_LINE_RIGHT:
          lineSet = !tabular->RightLine(actcell);
	  for(i=sel_start; i<=sel_end; ++i)
	      tabular->SetRightLine(i,lineSet);
          UpdateLocal(bv, true);
          break;
      case LyXTabular::ALIGN_LEFT:
      case LyXTabular::ALIGN_RIGHT:
      case LyXTabular::ALIGN_CENTER:
	  for(i=sel_start; i<=sel_end; ++i)
	      tabular->SetAlignment(i,setAlign);
          UpdateLocal(bv, true);
          break;
      case LyXTabular::MULTICOLUMN:
      {
	  if (tabular->row_of_cell(sel_start) !=
	      tabular->row_of_cell(sel_end)) {
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
		  UpdateLocal(bv, true);
	      } else {
		  tabular->SetMultiColumn(actcell, 1);
		  UpdateLocal(bv, false);
	      }
	      return;
	  }
	  // we have a selection so this means we just add all this
	  // cells to form a multicolumn cell
	  int
	      s_start, s_end;

	  if (sel_start > sel_end) {
	      s_start = sel_end;
	      s_end = sel_start;
	  } else {
	      s_start = sel_start;
	      s_end = sel_end;
	  }
	  tabular->SetMultiColumn(s_start, s_end);
	  cursor.pos = s_start;
	  sel_cell_end = sel_cell_start;
	  UpdateLocal(bv, true);
          break;
      }
      case LyXTabular::SET_ALL_LINES:
          setLines = 1;
      case LyXTabular::UNSET_ALL_LINES:
	  for(i=sel_start; i<=sel_end; ++i)
	      tabular->SetAllLines(i, setLines);
          UpdateLocal(bv, true);
          break;
      case LyXTabular::SET_LONGTABULAR:
          tabular->SetLongTabular(true);
	  UpdateLocal(bv, true); // because this toggles displayed
          break;
      case LyXTabular::UNSET_LONGTABULAR:
          tabular->SetLongTabular(false);
	  UpdateLocal(bv, true); // because this toggles displayed
          break;
      case LyXTabular::SET_ROTATE_TABULAR:
          tabular->SetRotateTabular(true);
          break;
      case LyXTabular::UNSET_ROTATE_TABULAR:
          tabular->SetRotateTabular(false);
          break;
      case LyXTabular::SET_ROTATE_CELL:
	  for(i=sel_start; i<=sel_end; ++i)
	      tabular->SetRotateCell(i,true);
          break;
      case LyXTabular::UNSET_ROTATE_CELL:
	  for(i=sel_start; i<=sel_end; ++i)
	      tabular->SetRotateCell(i,false);
          break;
      case LyXTabular::SET_LINEBREAKS:
          what = !tabular->GetLinebreaks(actcell);
	  for(i=sel_start; i<=sel_end; ++i)
	      tabular->SetLinebreaks(i,what);
          break;
      case LyXTabular::SET_LTFIRSTHEAD:
          tabular->SetLTHead(actcell,true);
          break;
      case LyXTabular::SET_LTHEAD:
          tabular->SetLTHead(actcell,false);
          break;
      case LyXTabular::SET_LTFOOT:
          tabular->SetLTFoot(actcell,false);
          break;
      case LyXTabular::SET_LTLASTFOOT:
          tabular->SetLTFoot(actcell,true);
          break;
      case LyXTabular::SET_LTNEWPAGE:
          what = !tabular->GetLTNewPage(actcell);
          tabular->SetLTNewPage(actcell,what);
          break;
    }
}

void InsetTabular::RemoveTabularRow()
{
}

bool InsetTabular::ActivateCellInset(BufferView * bv, int x, int y, int button,
				     bool behind)
{
    // the cursor.pos has to be before the inset so if it isn't now just
    // reset the curor pos first!
    if (cursor.pos % 2) { // behind the inset
	--cursor.pos;
	resetPos(bv->painter());
    }
    UpdatableInset * inset =
	static_cast<UpdatableInset*>(tabular->GetCellInset(actcell));
    LyXFont font(LyXFont::ALL_SANE);
    if (behind) {
	x = inset->x() + inset->width(bv->painter(), font);
	y = inset->descent(bv->painter(), font);
    }
    inset_x = cursor.x - top_x + tabular->GetBeginningOfTextInCell(actcell);
    inset_y = cursor.y;
    inset->Edit(bv, x-inset_x, y-inset_y, button);
    if (!the_locking_inset)
	return false;
    UpdateLocal(bv, true);
    return true;
}

bool InsetTabular::InsetHit(BufferView * bv, int x, int ) const
{
    InsetText * inset = tabular->GetCellInset(actcell);
    int x1 = x + top_x;

    if (cursor.pos % 2) { // behind the inset
	return (((x+top_x) < cursor.x) &&
		((x+top_x) > (cursor.x - inset->width(bv->painter(),
						      LyXFont(LyXFont::ALL_SANE)))));
    } else {
	int x2 = cursor.x + tabular->GetBeginningOfTextInCell(actcell);
	return ((x1 > x2) &&
		(x1 < (x2 + inset->width(bv->painter(),
					 LyXFont(LyXFont::ALL_SANE)))));
    }
}

// This returns paperWidth() if the cell-width is unlimited or the width
// in pixels if we have a pwidth for this cell.
int InsetTabular::GetMaxWidthOfCell(Painter & pain, int cell) const
{
    string w;
	
    if ((w=tabular->GetPWidth(cell)).empty())
	return pain.paperWidth();
    return VSpace(w).inPixels( 0, 0);
}

int InsetTabular::getMaxWidth(Painter & pain,
			      UpdatableInset const * inset) const
{
    int cell;
    int n = tabular->GetNumberOfCells();
    for(cell=0; cell < n; ++cell) {
	if (tabular->GetCellInset(cell) == inset)
	    break;
    }
    if (cell >= n)
	return pain.paperWidth();
    int w = GetMaxWidthOfCell(pain, cell);
    // this because text insets remove the xpos from the maxwidth because
    // otherwise the would not break good!!!
//    w += getCellXPos(cell) + tabular->GetBeginningOfTextInCell(cell);
//    w += inset->x();
    return w;
}

void InsetTabular::recomputeTextInsets(Painter & pain, const LyXFont & font) const
{
    InsetText * inset;
    int cx, cell;

    cx = top_x;
    for(int j= 0; j < tabular->columns(); ++j) {
	for(int i = 0; i < tabular->rows(); ++i) {
	    if (tabular->IsPartOfMultiColumn(i,j))
		continue;
	    cell = tabular->GetCellNumber(i,j);
	    inset = tabular->GetCellInset(cell);
	    inset->computeTextRows(pain);
	    tabular->SetWidthOfCell(cell, inset->width(pain, font));
	}
	cell = tabular->GetCellNumber(0, j);
	cx += tabular->GetWidthOfColumn(cell);
    }
}
