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
#include "insets/insettext.h"

const int ADD_TO_HEIGHT = 2;
const int ADD_TO_TABULAR_WIDTH = 2;

using std::ostream;
using std::ifstream;
using std::max;
using std::endl;


InsetTabular::InsetTabular(Buffer * buf, int rows, int columns)
{
    if (rows <= 0)
        rows = 1;
    if (columns <= 0)
        columns = 1;
    tabular = new LyXTabular(rows,columns,buf);
    // for now make it always display as display() inset
    // just for test!!!
    tabular->SetLongTabular(true);
    the_locking_inset = 0;
    buffer = buf;
    cursor_visible = false;
    cursor.x_fix = -1;
    actcell = cursor.pos = sel_pos_start = sel_pos_end = 0;
    no_selection = false;
    init = true;
}


InsetTabular::InsetTabular(InsetTabular const & tab, Buffer * buf)
{
    tabular = new LyXTabular(*(tab.tabular), buf);
    the_locking_inset = 0;
    buffer = buf;
    cursor_visible = false;
    cursor.x_fix = -1;
    actcell = cursor.pos = sel_pos_start = sel_pos_end = 0;
    no_selection = false;
    init = true;
}


InsetTabular::~InsetTabular()
{
    delete tabular;
}


Inset * InsetTabular::Clone() const
{
    InsetTabular * t = new InsetTabular(*this, buffer);
    return t;
}


void InsetTabular::Write(ostream & os) const
{
    os << " Tabular" << endl;
    tabular->Write(os);
}


void InsetTabular::Read(LyXLex & lex)
{
//    bool old_format = (lex.GetString() == "\\LyXTable");
    string token;

    if (tabular)
	delete tabular;
    tabular = new LyXTabular(lex, buffer);

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
    tabular->SetLongTabular(true);
    init = true;
}


int InsetTabular::ascent(Painter & pain, LyXFont const & font) const
{
    if (init) {
	calculate_width_of_cells(pain, font);
	init = false;
    }
    return tabular->GetAscentOfRow(0);
}


int InsetTabular::descent(Painter & pain, LyXFont const & font) const
{
    if (init) {
	calculate_width_of_cells(pain, font);
	init = false;
    }
    return tabular->GetHeightOfTabular() - tabular->GetAscentOfRow(0);
}


int InsetTabular::width(Painter & pain, LyXFont const & font) const
{
    if (init) {
	calculate_width_of_cells(pain, font);
	init = false;
    }
    return tabular->GetWidthOfTabular() + (2 * ADD_TO_TABULAR_WIDTH);
}


void InsetTabular::draw(Painter & pain, const LyXFont & font, int baseline,
			float & x) const
{
    int i, j, cell=0;
    int nx;
    float cx;

    UpdatableInset::draw(pain,font,baseline,x);
    if ((top_x != int(x)) || (top_baseline != baseline)) {
	top_x = int(x);
	top_baseline = baseline;
	resetPos(pain);
    }
    for(i=0;i<tabular->rows();++i) {
	nx = int(x);
        for(j=0;j<tabular->columns();++j) {
	    cx = nx + tabular->GetBeginningOfTextInCell(cell);
	    tabular->GetCellInset(cell)->draw(pain, font, baseline, cx);
	    DrawCellLines(pain, nx, baseline, i, cell);
	    nx += tabular->GetWidthOfColumn(cell);
	    ++cell;
	}
        baseline += tabular->GetDescentOfRow(i) + tabular->GetAscentOfRow(i+1)
            + tabular->GetAdditionalHeight(cell+1);
    }
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
    the_locking_inset = 0;
    sel_pos_start = sel_pos_end = inset_pos = inset_x = inset_y = 0;
    setPos(bv->painter(), x, y);
    sel_pos_start = sel_pos_end = cursor.pos;
    bv->text->FinishUndo();
    if (InsetHit(bv, x, y)) {
	ActivateCellInset(bv, x, y, button);
    }
    UpdateLocal(bv, true);
//    bv->getOwner()->getPopups().updateFormTabular();
}


void InsetTabular::InsetUnlock(BufferView * bv)
{
    if (the_locking_inset) {
	the_locking_inset->InsetUnlock(bv);
	the_locking_inset = 0;
    }
    HideInsetCursor(bv);
    if (hasCharSelection()) {
	sel_pos_start = sel_pos_end = cursor.pos;
	UpdateLocal(bv, false);
    } else
	sel_pos_start = sel_pos_end = cursor.pos;
    no_selection = false;
}

void InsetTabular::UpdateLocal(BufferView * bv, bool flag)
{
    if (flag)
	calculate_width_of_cells(bv->painter(), LyXFont(LyXFont::ALL_SANE));
    bv->updateInset(this, flag);
}

bool InsetTabular::LockInsetInInset(BufferView * bv, UpdatableInset * inset)
{
    lyxerr[Debug::INSETS] << "InsetTabular::LockInsetInInset(" <<inset<< "): ";
    if (!inset)
	return false;
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
    return the_locking_inset->UnlockInsetInInset(bv, inset, lr);
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

void InsetTabular::InsetButtonRelease(BufferView * bv,
				      int x, int y, int button)
{
    if (the_locking_inset) {
        the_locking_inset->InsetButtonRelease(bv, x-inset_x, y-inset_y,button);
        return;
    }
    no_selection = false;
}


void InsetTabular::InsetButtonPress(BufferView * bv, int x, int y, int button)
{
    if (hasCharSelection()) {
	sel_pos_start = sel_pos_end = 0;
	UpdateLocal(bv, false);
    }
    no_selection = false;

    int oldcell = actcell;

    setPos(bv->painter(), x, y);

    bool inset_hit = InsetHit(bv, x, y);

    if ((oldcell == actcell) && the_locking_inset && inset_hit) {
        the_locking_inset->InsetButtonPress(bv, x-inset_x, y-inset_y, button);
        return;
    } else if (the_locking_inset) {
	the_locking_inset->InsetUnlock(bv);
    }
    the_locking_inset = 0;
    sel_pos_start = sel_pos_end = cursor.pos;
    sel_cell_start = sel_cell_end = actcell;
    if (inset_hit && bv->the_locking_inset) {
	ActivateCellInset(bv, x, y, button);
	the_locking_inset->InsetButtonPress(bv, x-inset_x, y-inset_y, button);
    }
    
#if 0
    if (button == 3)
        bview->getOwner()->getPopups().showFormTabular();
    else if (oldcell != actcell)
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
	    // int oldcell = actcell,
	    int old = sel_pos_end;

	setPos(bv->painter(), x, y);
	sel_pos_end = cursor.pos;
	sel_cell_end = actcell;
	if (old != sel_pos_end)
	    UpdateLocal(bv, false);
#if 0
	if (oldcell != actcell)
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

    HideInsetCursor(bv);
    switch (action) {
      // Normal chars not handled here
      case -1:
	  break;
      // --- Cursor Movements ---------------------------------------------
      case LFUN_RIGHTSEL:
          moveRight(bv, false);
	  sel_pos_end = cursor.pos;
	  UpdateLocal(bv, false);
	  break;
      case LFUN_RIGHT:
          result = moveRight(bv);
	  if (hasCharSelection()) {
	      sel_pos_start = sel_pos_end = cursor.pos;
	      UpdateLocal(bv, false);
	  } else
	      sel_pos_start = sel_pos_end = cursor.pos;
          break;
      case LFUN_LEFTSEL:
          moveLeft(bv, false);
	  sel_pos_end = cursor.pos;
	  UpdateLocal(bv, false);
	  break;
      case LFUN_LEFT:
          result = moveLeft(bv);
	  if (hasCharSelection()) {
	      sel_pos_start = sel_pos_end = cursor.pos;
	      UpdateLocal(bv, false);
	  } else
	      sel_pos_start = sel_pos_end = cursor.pos;
          break;
      case LFUN_DOWNSEL:
          moveDown();
	  sel_pos_end = cursor.pos;
	  UpdateLocal(bv, false);
	  break;
      case LFUN_DOWN:
          result= moveDown();
	  if (hasCharSelection()) {
	      sel_pos_start = sel_pos_end = cursor.pos;
	      UpdateLocal(bv, false);
	  } else
	      sel_pos_start = sel_pos_end = cursor.pos;
          break;
      case LFUN_UPSEL:
          moveUp();
	  sel_pos_end = cursor.pos;
	  UpdateLocal(bv, false);
	  break;
      case LFUN_UP:
          result= moveUp();
	  if (hasCharSelection()) {
	      sel_pos_start = sel_pos_end = cursor.pos;
	      UpdateLocal(bv, false);
	  } else
	      sel_pos_start = sel_pos_end = cursor.pos;
          break;
      case LFUN_BACKSPACE:
	  break;
      case LFUN_DELETE:
          break;
      case LFUN_HOME:
          break;
      case LFUN_END:
          break;
      case LFUN_TAB:
	  if (hasCharSelection()) {
	      sel_pos_start = sel_pos_end = cursor.pos;
	      UpdateLocal(bv, false);
	  }
	  sel_pos_start = sel_pos_end = cursor.pos;
          moveNextCell();
          break;
      default:
          result = UNDISPATCHED;
          break;
    }
    if (result!=FINISHED) {
	if (!the_locking_inset) {
#if 0	    
	    if (oldcell != actcell)
		bview->getOwner()->getPopups().updateFormTabular();
#endif
	    ShowInsetCursor(bv);
	}
    } else
        bv->unlockInset(this);
    return result;
}


int InsetTabular::Latex(ostream & os, bool, bool) const
{
    return tabular->Latex(os);
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
    if (tabular->IsLongTabular())
        features.longtable = true;
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
            tabular->GetAdditionalHeight(tabular->GetCellNumber(actcol,actrow+1));
        ++actrow;
        ly = cursor.y + tabular->GetDescentOfRow(actrow);
    }
    actcell = tabular->GetCellNumber(actcol, actrow);

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


void InsetTabular::resetPos(Painter & pain) const
{
    actrow = cursor.y = actcol = 0;

    int cell = 0;
    for(; (cell<actcell) && !tabular->IsLastRow(cell); ++cell) {
	if (tabular->IsLastCellInRow(cell)) {
	    cursor.y += tabular->GetDescentOfRow(actrow) +
		tabular->GetAscentOfRow(actrow+1) +
		tabular->GetAdditionalHeight(cell+1);
	    ++actrow;
	}
    }
    for(cell=actcell;!tabular->IsFirstCellInRow(cell);--cell)
	;
    int lx = tabular->GetWidthOfColumn(actcell);
    for(; (cell < actcell); ++cell) {
	lx += tabular->GetWidthOfColumn(cell);
	++actcol;
    }
    cursor.x = lx - tabular->GetWidthOfColumn(actcell) + top_x + 2;
    if (cursor.pos % 2) {
	LyXFont font(LyXFont::ALL_SANE);
	cursor.x += tabular->GetCellInset(actcell)->width(pain,font) +
		tabular->GetBeginningOfTextInCell(actcell);
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
	if (ActivateCellInset(bv, -1, -1))
	    return DISPATCHED;
    }
    resetPos(bv->painter());
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT InsetTabular::moveUp()
{
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT InsetTabular::moveDown()
{
    return DISPATCHED_NOUPDATE;
}


bool InsetTabular::moveNextCell()
{
    return false;
}


bool InsetTabular::movePrevCell()
{
    return false;
}


bool InsetTabular::Delete()
{
    return true;
}


void InsetTabular::SetFont(BufferView *, LyXFont const &, bool)
{
}


void InsetTabular::TabularFeatures(int, string)
{
#if 0
    int
	i,
	sel_pos_start,
	sel_pos_end,
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
    if (hasCellSelection()) {
	if (sel_cell_start > sel_cell_end) {
	    sel_pos_start = sel_cell_end;
	    sel_pos_end = sel_cell_start;
	} else {
	    sel_pos_start = sel_cell_start;
	    sel_pos_end = sel_cell_end;
	}
    } else
	sel_pos_start = sel_pos_end = actcell;
    switch (feature) {
      case LyXTabular::SET_PWIDTH:
          tabular->SetPWidth(actcell,val);
          break;
      case LyXTabular::SET_SPECIAL_COLUMN:
      case LyXTabular::SET_SPECIAL_MULTI:
          tabular->SetAlignSpecial(actcell,val,feature);
          break;
      case LyXTabular::APPEND_ROW:
      {
	  int
              pos = cursor.pos,
              cell_org = actcell,
              cell = actcell;

          // if there is a ContRow following this row I have to add
          // the row after the ContRow's
          if ((pos < par->last()) && tabular->RowHasContRow(cell_org)) {
              while((pos < par->last()) && !tabular->IsContRow(cell)) {
                  while (pos < par->last() && !par->IsNewline(pos))
                      ++pos;
                  if (pos < par->last())
                      ++pos;
                  ++cell;
              }
              while((pos < par->last()) && tabular->IsContRow(cell)) {
                  while (pos < par->last() && !par->IsNewline(pos))
                      ++pos;
                  if (pos < par->last())
                      ++pos;
                  ++cell;
              }
              cell_org = --cell;
              if (pos < par->last())
                  --pos;
          }
          while ((pos < par->last()) && 
                 ((cell == cell_org) || !tabular->IsFirstCell(cell))) {
              while ((pos < par->last()) && !par->IsNewline(pos))
                  ++pos;
              if (pos < par->last())
                  ++pos;
              ++cell;
          }
          // insert the new cells
          int number = tabular->NumberOfCellsInRow(cell_org);
          for (i=0; i<number; ++i)
              par->InsertChar(pos, LYX_META_NEWLINE);
		
          // append the row into the tabular
          tabular->AppendRow(cell_org);
          calculate_width_of_cells();
          UpdateLocal();
          return;
      }
      case LyXTabular::APPEND_CONT_ROW:
      {
	  int
              pos = cursor.pos,
              cell_org = actcell,
              cell = actcell;

          // if there is already a controw but not for this cell
          // the AppendContRow sets only the right values but does
          // not actually add a row
          if (tabular->RowHasContRow(cell_org) &&
              (tabular->CellHasContRow(cell_org) < 0)) {
              tabular->AppendContRow(cell_org);
              calculate_width_of_cells();
              UpdateLocal();
              return;
          }
          while ((pos < par->last()) &&
                 ((cell == cell_org) || !tabular->IsFirstCell(cell))) {
              while (pos < par->last() && !par->IsNewline(pos))
                  ++pos;
              if (pos < par->last())
                  ++pos;
              ++cell;
          }
          // insert the new cells
          int number = tabular->NumberOfCellsInRow(cell_org);
          for (i=0; i<number; ++i)
              par->InsertChar(pos, LYX_META_NEWLINE);
          // append the row into the tabular
          tabular->AppendContRow(cell_org);
          calculate_width_of_cells();
          UpdateLocal();
          return;
      }
      case LyXTabular::APPEND_COLUMN:
      { 
	  int
              pos = 0,
              cell_org = actcell,
              cell = 0;
          do {
              if (pos && (par->IsNewline(pos-1))) {
                  if (tabular->AppendCellAfterCell(cell_org, cell)) {
                      par->InsertChar(pos, LYX_META_NEWLINE);
                      if (pos <= cursor.pos)
                          ++cursor.pos;
                      ++pos;
                  }
                  ++cell;
              }
              ++pos;
          } while (pos <= par->last());
          // remember that the very last cell doesn't end with a newline.
          // This saves one byte memory per tabular ;-)
          if (tabular->AppendCellAfterCell(cell_org, cell))
              par->InsertChar(par->last(), LYX_META_NEWLINE);
          // append the column into the tabular
          tabular->AppendColumn(cell_org);
          calculate_width_of_cells();
          UpdateLocal();
          return;
      }
      case LyXTabular::DELETE_ROW:
          RemoveTabularRow();
          calculate_width_of_cells();
          UpdateLocal();
          return;
      case LyXTabular::DELETE_COLUMN:
      {
          int pos = 0;
          int cell = 0;
          do {
              if (!pos || (par->IsNewline(pos-1))){
                  if (tabular->DeleteCellIfColumnIsDeleted(cell, actcell)) {
                      // delete one cell
                      while (pos < par->last() && !par->IsNewline(pos))
                          par->Erase(pos);
                      if (pos < par->last())
                          par->Erase(pos);
                      else 
                          par->Erase(pos - 1); // the missing newline
                                               // at the end of a tabular
                      --pos; // because of ++pos below
                  }
                  ++cell;
              }
              ++pos;
          } while (pos <= par->last());
          /* delete the column from the tabular */ 
          tabular->DeleteColumn(actcell);
          calculate_width_of_cells();
          UpdateLocal();
          return;
      }
      case LyXTabular::TOGGLE_LINE_TOP:
          lineSet = !tabular->TopLine(actcell);
	  for(i=sel_pos_start; i<=sel_pos_end; ++i)
	      tabular->SetTopLine(i,lineSet);
          calculate_width_of_cells();
          UpdateLocal();
          return;
    
      case LyXTabular::TOGGLE_LINE_BOTTOM:
          lineSet = !tabular->BottomLine(actcell); 
	  for(i=sel_pos_start; i<=sel_pos_end; ++i)
	      tabular->SetBottomLine(i,lineSet);
          calculate_width_of_cells();
          UpdateLocal();
          return;
		
      case LyXTabular::TOGGLE_LINE_LEFT:
          lineSet = !tabular->LeftLine(actcell);
	  for(i=sel_pos_start; i<=sel_pos_end; ++i)
	      tabular->SetLeftLine(i,lineSet);
          calculate_width_of_cells();
          UpdateLocal();
          return;

      case LyXTabular::TOGGLE_LINE_RIGHT:
          lineSet = !tabular->RightLine(actcell);
	  for(i=sel_pos_start; i<=sel_pos_end; ++i)
	      tabular->SetRightLine(i,lineSet);
          calculate_width_of_cells();
          UpdateLocal();
          return;
      case LyXTabular::ALIGN_LEFT:
      case LyXTabular::ALIGN_RIGHT:
      case LyXTabular::ALIGN_CENTER:
	  for(i=sel_pos_start; i<=sel_pos_end; ++i)
	      tabular->SetAlignment(i,setAlign);
          UpdateLocal();
          return;
      case LyXTabular::MULTICOLUMN:
      {
	  if (tabular->row_of_cell(sel_pos_start) !=
	      tabular->row_of_cell(sel_pos_end)) {
	      WriteAlert(_("Impossible Operation!"), 
			 _("Multicolumns can only be horizontally."), 
			 _("Sorry."));
	      return;
	  }
	  // just multicol for one Single Cell
	  if (!hasCellSelection()) {
	      // check wether we are completly in a multicol
	      if (tabular->IsMultiColumn(actcell)) {
		  int
		      newlines,
		      pos = cursor.pos;
		  if ((newlines=tabular->UnsetMultiColumn(actcell))) {
		      while ((pos < par->last()) && !par->IsNewline(pos))
			  ++pos;
		      for (;newlines;--newlines)
			  par->InsertChar(pos, LYX_META_NEWLINE);
		  }
		  calculate_width_of_cells();
	      } else {
		  tabular->SetMultiColumn(actcell, 1);
	      }
	      UpdateLocal();
	      return;
	  }
	  // we have a selection so this means we just add all this
	  // cells to form a multicolumn cell
	  int
	      number = 1,
	      s_start, s_end;

	  if (sel_pos_start > sel_pos_end) {
	      s_start = sel_pos_end;
	      s_end = sel_pos_start;
	  } else {
	      s_start = sel_pos_start;
	      s_end = sel_pos_end;
	  }
	  for(i=s_start; i < s_end; ++i) {
	      if (par->IsNewline(i)) {
		  par->Erase(i);
		  // check for double-blanks
		  if ((i && !par->IsLineSeparator(i-1)) &&
		      (i < par->last()) && !par->IsLineSeparator(i))
		      par->InsertChar(i, ' ');
		  else
		      --i;
		  ++number;
	      }
	  }
	  tabular->SetMultiColumn(sel_pos_start,number);
	  cursor.pos = s_start;
	  sel_cell_end = sel_cell_start;
	  calculate_width_of_cells();
	  UpdateLocal();
          return;
      }
      case LyXTabular::SET_ALL_LINES:
          setLines = 1;
      case LyXTabular::UNSET_ALL_LINES:
	  for(i=sel_pos_start; i<=sel_pos_end; ++i)
	      tabular->SetAllLines(i, setLines);
          calculate_width_of_cells();
          UpdateLocal();
          return;
      case LyXTabular::SET_LONGTABULAR:
          tabular->SetLongTabular(true);
	  UpdateLocal(); // because this toggles displayed
          return;
      case LyXTabular::UNSET_LONGTABULAR:
          tabular->SetLongTabular(false);
	  UpdateLocal(); // because this toggles displayed
          return;
      case LyXTabular::SET_ROTATE_TABULAR:
          tabular->SetRotateTabular(true);
          return;
      case LyXTabular::UNSET_ROTATE_TABULAR:
          tabular->SetRotateTabular(false);
          return;
      case LyXTabular::SET_ROTATE_CELL:
	  for(i=sel_pos_start; i<=sel_pos_end; ++i)
	      tabular->SetRotateCell(i,true);
          return;
      case LyXTabular::UNSET_ROTATE_CELL:
	  for(i=sel_pos_start; i<=sel_pos_end; ++i)
	      tabular->SetRotateCell(i,false);
          return;
      case LyXTabular::SET_LINEBREAKS:
          what = !tabular->Linebreaks(tabular->FirstVirtualCell(actcell));
	  for(i=sel_pos_start; i<=sel_pos_end; ++i)
	      tabular->SetLinebreaks(i,what);
          return;
      case LyXTabular::SET_LTFIRSTHEAD:
          tabular->SetLTHead(actcell,true);
          return;
      case LyXTabular::SET_LTHEAD:
          tabular->SetLTHead(actcell,false);
          return;
      case LyXTabular::SET_LTFOOT:
          tabular->SetLTFoot(actcell,false);
          return;
      case LyXTabular::SET_LTLASTFOOT:
          tabular->SetLTFoot(actcell,true);
          return;
      case LyXTabular::SET_LTNEWPAGE:
          what = !tabular->LTNewPage(actcell);
          tabular->SetLTNewPage(actcell,what);
          return;
    }
#endif
}

void InsetTabular::RemoveTabularRow()
{
}

bool InsetTabular::ActivateCellInset(BufferView * bv, int x, int y, int button)
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
    if (x < 0)
	x = inset->width(bv->painter(), font) + top_x;
    if (y < 0)
	y = inset->descent(bv->painter(), font);
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
