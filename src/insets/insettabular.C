/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright (C) 2000 The LyX Team.
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
#include "insets/insettext.h"

const int ADD_TO_HEIGHT = 2;
const int ADD_TO_TABLE_WIDTH = 2;

using std::ostream;
using std::ifstream;

InsetTabular::InsetTabular(Buffer * buf, int rows=1, int columns=1)
{
    if (rows <= 0)
        rows = 1;
    if (columns <= 0)
        columns = 1;
    tabular = new LyXTable(rows,columns,buf);
    the_locking_inset = 0;
    buffer = buf;
    cursor_visible = false;
    old_x = -1;
    sel_pos_start = sel_pos_end = 0;
    no_selection = false;
    init = true;
}

InsetTabular::InsetTabular(InsetTabular const & tab, Buffer * buf)
{
    tabular = new LyXTable(*(tab.tabular), buf);
    the_locking_inset = 0;
    buffer = buf;
    cursor_visible = false;
    old_x = -1;
    sel_pos_start = sel_pos_end = 0;
    no_selection = false;
    init = true;
}

InsetTabular::~InsetTabular()
{
    if (tabular)
        delete tabular;
}


InsetTabular * InsetTabular::Clone() const
{
    InsetTabular * t = new InsetTabular(*this, buffer);
    return t;
}


void InsetTabular::Write(ostream & os) const
{
    os << "\\begin_inset Tabular\n";
    tabular->Write(os,false);
    os << "\\end_inset\n";
}


void InsetTabular::Read(LyXLex & lex)
{
    if (tabular)
	delete tabular;

//    bool old_format = (lex.GetString() == "\\LyXTable");
    tabular = new LyXTable(lex, buffer);
    init = true;
}


int InsetTabular::ascent(Painter & pain, LyXFont const & font) const
{
    if (init) {
	calculate_width_of_cells(pain, font);
	init = false;
    }
    return tabular->AscentOfRow(0);
}


int InsetTabular::descent(Painter & pain, LyXFont const & font) const
{
    if (init) {
	calculate_width_of_cells(pain, font);
	init = false;
    }
    return tabular->HeightOfTable() - tabular->AscentOfRow(0);
}


int InsetTabular::width(Painter & pain, LyXFont const & font) const
{
    if (init) {
	calculate_width_of_cells(pain, font);
	init = false;
    }
    return tabular->WidthOfTable() + (2 * ADD_TO_TABLE_WIDTH);
}


void InsetTabular::draw(Painter & pain, const LyXFont & font, int,
			float &) const
{
    calculate_width_of_cells(pain, font);
}


const char * InsetTabular::EditMessage() const
{
    return _("Opened Tabular Inset");
}


void InsetTabular::Edit(BufferView * bv, int x, int y, unsigned int button)
{
    UpdatableInset::Edit(bv, x, y, button);

    bv->lockInset(this);
    the_locking_inset = 0;
    inset_pos = inset_x = inset_y = 0;
    no_selection = true;
    setPos(x,y);
    sel_pos_start = sel_pos_end = actpos;
    sel_cell_start = sel_cell_end = actcell;
//    bv->getOwner()->getPopups().updateFormTable();
}


void InsetTabular::InsetUnlock(BufferView * bv)
{
    if (the_locking_inset)
	the_locking_inset->InsetUnlock(bv);
    HideInsetCursor(bv);
    the_locking_inset = 0;
    if (hasCharSelection()) {
	sel_pos_start = sel_pos_end = actpos;
	UpdateLocal(bv, false);
    } else
	sel_pos_start = sel_pos_end = actpos;
    no_selection = false;
}

bool InsetTabular::LockInsetInInset(UpdatableInset *)
{
    return true;
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
	    moveRight(false);
	return true;
    }
    return the_locking_inset->UnlockInsetInInset(bv, inset, lr);
}


void InsetTabular::UpdateLocal(BufferView * bv, bool flag)
{
    resetPos();
    bv->updateInset(this, flag);
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

void InsetTabular::InsetButtonRelease(BufferView *bv, int x, int y, int button)
{
    if (the_locking_inset) {
        the_locking_inset->InsetButtonRelease(bv, x-inset_x,y-inset_y,button);
        return;
    }
    no_selection = false;
}

void InsetTabular::InsetButtonPress(BufferView *bv, int x, int y, int)
{
    if (hasCharSelection()) {
	sel_pos_start = sel_pos_end = 0;
	UpdateLocal(bv, false);
    }
    no_selection = false;
    setPos(x,y,false);
    if (the_locking_inset) {
#if 0
	UpdatableInset
	    * inset=0;
	if (par->GetChar(actpos)==LYX_META_INSET)
	    inset=(UpdatableInset*)par->GetInset(actpos);
	if (the_locking_inset == inset) {
	    the_locking_inset->InsetButtonPress(bv,x-inset_x,y-inset_y,button);
	    return;
	} else if (inset) {
	    // otherwise unlock the_locking_inset and lock the new inset
	    inset_x = cx-top_x;
	    inset_y = cy;
	    inset_pos = actpos;
	    the_locking_inset->InsetUnlock(bv);
	    the_locking_inset = inset;
	    the_locking_inset->Edit(bv, x - inset_x, y - inset_y, button);
	    return;
	}
#endif
	// otherwise only unlock the_locking_inset
	the_locking_inset->InsetUnlock(bv);
    }
#if 0
    int
	oldcell = actcell;
#endif
    setPos(x,y);
    the_locking_inset = 0;
    sel_pos_start = sel_pos_end = actpos;
    sel_cell_start = sel_cell_end = actcell;
#if 0
    if (button == 3)
        bview->getOwner()->getPopups().showFormTable();
    else if (oldcell != actcell)
        bview->getOwner()->getPopups().updateFormTable();
#endif
}


void InsetTabular::InsetMotionNotify(BufferView * bv, int x, int y, int button)
{
    if (the_locking_inset) {
        the_locking_inset->InsetMotionNotify(bv, x-inset_x,y-inset_y,button);
        return;
    }
    if (!no_selection) {
	int
//	    oldcell = actcell,
	    old = sel_pos_end;

	setPos(x,y);
	sel_pos_end = actpos;
	sel_cell_end = actcell;
	if (old != sel_pos_end)
	    UpdateLocal(bv, false);
#if 0
	if (oldcell != actcell)
	    bview->getOwner()->getPopups().updateFormTable();
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


UpdatableInset::RESULT InsetTabular::LocalDispatch(BufferView *bv, int action,
						   string const & arg)
{
    no_selection = false;
    if (UpdatableInset::LocalDispatch(bv, action, arg)) {
	resetPos();
	return DISPATCHED;
    }

    UpdatableInset::RESULT 
        result=DISPATCHED;
    if ((action < 0) && arg.empty())
        return FINISHED;

    if ((action != LFUN_DOWN) && (action != LFUN_UP) &&
        (action != LFUN_DOWNSEL) && (action != LFUN_UPSEL))
        old_x = -1;
    if (the_locking_inset) {
        result=the_locking_inset->LocalDispatch(bv, action, arg);
	if (result == DISPATCHED) {
            setWidthOfCell(actpos,actcell,actrow);
	    the_locking_inset->ToggleInsetCursor(bv);
	    UpdateLocal(bv, false);
	    the_locking_inset->ToggleInsetCursor(bv);
            return result;
        } else if (result == FINISHED) {
	    if ((action == LFUN_RIGHT) || (action == -1)) {
		actpos = inset_pos + 1;
		resetPos();
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
          moveRight(false);
	  sel_pos_end = actpos;
	  UpdateLocal(bv, false);
	  break;
      case LFUN_RIGHT:
          result= DISPATCH_RESULT(moveRight());
	  if (hasCharSelection()) {
	      sel_pos_start = sel_pos_end = actpos;
	      UpdateLocal(bv, false);
	  } else
	      sel_pos_start = sel_pos_end = actpos;
          break;
      case LFUN_LEFTSEL:
          moveLeft(false);
	  sel_pos_end = actpos;
	  UpdateLocal(bv, false);
	  break;
      case LFUN_LEFT:
          result= DISPATCH_RESULT(moveLeft());
	  if (hasCharSelection()) {
	      sel_pos_start = sel_pos_end = actpos;
	      UpdateLocal(bv, false);
	  } else
	      sel_pos_start = sel_pos_end = actpos;
          break;
      case LFUN_DOWNSEL:
          moveDown(false);
	  sel_pos_end = actpos;
	  UpdateLocal(bv, false);
	  break;
      case LFUN_DOWN:
          result= DISPATCH_RESULT(moveDown());
	  if (hasCharSelection()) {
	      sel_pos_start = sel_pos_end = actpos;
	      UpdateLocal(bv, false);
	  } else
	      sel_pos_start = sel_pos_end = actpos;
          break;
      case LFUN_UPSEL:
          moveUp(false);
	  sel_pos_end = actpos;
	  UpdateLocal(bv, false);
	  break;
      case LFUN_UP:
          result= DISPATCH_RESULT(moveUp());
	  if (hasCharSelection()) {
	      sel_pos_start = sel_pos_end = actpos;
	      UpdateLocal(bv, false);
	  } else
	      sel_pos_start = sel_pos_end = actpos;
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
	      sel_pos_start = sel_pos_end = actpos;
	      UpdateLocal(bv, false);
	  }
	  sel_pos_start = sel_pos_end = actpos;
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
		bview->getOwner()->getPopups().updateFormTable();
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
    if (tabular->IsLongTable())
        features.longtable = true;
}


void InsetTabular::calculate_width_of_cells(Painter & pain, LyXFont const & font) const
{
    int
        cell=-1,
        i,j,
        maxAsc, maxDesc;
    InsetText
	*inset;
    
    for(i=0; i < tabular->rows; ++i) {
        maxAsc = maxDesc = 0;
        for(j=0; j < tabular->columns; ++j) {
            if (tabular->IsPartOfMultiColumn(i,j))
                continue;
            ++cell;
	    inset = tabular->GetCellInset(cell);
	    maxAsc = max(maxAsc, inset->ascent(pain, font));
	    maxDesc = max(maxDesc, inset->descent(pain, font));
            tabular->SetWidthOfCell(cell, inset->width(pain, font));
        }
        tabular->SetAscentOfRow(i,maxAsc+ADD_TO_HEIGHT);
        tabular->SetDescentOfRow(i,maxDesc+ADD_TO_HEIGHT);
    }
}


void InsetTabular::DrawCellLines(Painter & pain, int x, int baseline,
                                 int row, int cell)
{
    // Juergen, have you thought about drawing the on-off lines in a
    // different color (gray of some kind), especially since those
    // lines will not be there on the hardcopy anyway. (Lgb)
    int
        x_old = x;
    bool
        on_off;

    x_old -= tabular->WidthOfColumn(cell);
    on_off = !tabular->TopLine(cell);
    if ((!on_off || !tabular->TopAlreadyDrawed(cell)) &&
        !tabular->IsContRow(cell))
        pain.line(x_old, baseline - tabular->AscentOfRow(row),
                  x, baseline -  tabular->AscentOfRow(row),
                  LColor::tableline,
                  on_off ? Painter::line_onoffdash:Painter::line_solid);
    on_off = !tabular->BottomLine(cell);
    if ((!on_off && !tabular->RowHasContRow(cell)) || 
        tabular->VeryLastRow(cell))
        pain.line(x_old ,baseline +  tabular->DescentOfRow(row),
                  x, baseline +  tabular->DescentOfRow(row),
                  LColor::tableline,
                  on_off ? Painter::line_onoffdash:Painter::line_solid);
    on_off = !tabular->LeftLine(cell);
    pain.line(x_old, baseline -  tabular->AscentOfRow(row),
              x_old, baseline +  tabular->DescentOfRow(row),
              LColor::tableline,
              on_off ? Painter::line_onoffdash:Painter::line_solid);
    on_off = !tabular->RightLine(cell);
    pain.line(x - tabular->AdditionalWidth(cell),
              baseline -  tabular->AscentOfRow(row),
              x - tabular->AdditionalWidth(cell),
              baseline +  tabular->DescentOfRow(row),
              LColor::tableline,
              on_off ? Painter::line_onoffdash:Painter::line_solid);
}


void InsetTabular::GetCursorPos(int & x, int & y)
{
    x = cx-top_x;
    y = cy;
}


void InsetTabular::ToggleInsetCursor(BufferView * bv)
{
    if (the_locking_inset) {
        the_locking_inset->ToggleInsetCursor(bv);
	return;
    }
    int
	x=0,y=0,
        asc,desc;

    LyXFont
        font; // = the_locking_inset->GetFont(par, actpos);

    asc = lyxfont::maxAscent(font);
    desc = lyxfont::maxDescent(font);
  
    if (cursor_visible)
        bv->hideLockedInsetCursor();
    else
        bv->showLockedInsetCursor(cx+x, cy+y, asc, desc);
    cursor_visible = !cursor_visible;
}


void InsetTabular::ShowInsetCursor(BufferView * bv)
{
    if (!cursor_visible) {
	int
	    asc,desc;
	LyXFont
	    font; // = GetFont(par, actpos);
    
	asc = lyxfont::maxAscent(font);
	desc = lyxfont::maxDescent(font);
	bv->fitLockedInsetCursor(cx, cy, asc, desc);
	bv->showLockedInsetCursor(cx, cy, asc, desc);
	cursor_visible = true;
    }
}


void InsetTabular::HideInsetCursor(BufferView * bv)
{
    if (cursor_visible)
        ToggleInsetCursor(bv);
}


void InsetTabular::setPos(int, int, bool)
{
}

void InsetTabular::setWidthOfCell(int, int, int)
{
}

bool InsetTabular::moveRight(bool)
{
    return false;
}

bool InsetTabular::moveLeft(bool)
{
    return false;
}

bool InsetTabular::moveUp(bool)
{
    return false;
}

bool InsetTabular::moveDown(bool)
{
    return false;
}

bool InsetTabular::moveNextCell()
{
    return false;
}

bool InsetTabular::movePrevCell()
{
    return false;
}

void InsetTabular::resetPos()
{
}

bool InsetTabular::Delete()
{
    return true;
}

void  InsetTabular::SetFont(LyXFont const &)
{
}

void InsetTabular::TableFeatures(int, string)
{
#if 0
    int
	i,
	selection_start,
	selection_end,
        setLines = 0,
        setAlign = LYX_ALIGN_LEFT,
        lineSet;
    bool
        what;

    switch (feature) {
      case LyXTable::ALIGN_LEFT:
          setAlign=LYX_ALIGN_LEFT;
          break;
      case LyXTable::ALIGN_RIGHT:
          setAlign=LYX_ALIGN_RIGHT;
          break;
      case LyXTable::ALIGN_CENTER:
          setAlign=LYX_ALIGN_CENTER;
          break;
      default:
          break;
    }
    if (hasCellSelection()) {
	if (sel_cell_start > sel_cell_end) {
	    selection_start = sel_cell_end;
	    selection_end = sel_cell_start;
	} else {
	    selection_start = sel_cell_start;
	    selection_end = sel_cell_end;
	}
    } else
	selection_start = selection_end = actcell;
    switch (feature) {
      case LyXTable::SET_PWIDTH:
          tabular->SetPWidth(actcell,val);
          break;
      case LyXTable::SET_SPECIAL_COLUMN:
      case LyXTable::SET_SPECIAL_MULTI:
          tabular->SetAlignSpecial(actcell,val,feature);
          break;
      case LyXTable::APPEND_ROW:
      {
	  int
              pos = actpos,
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
		
          // append the row into the table
          tabular->AppendRow(cell_org);
          calculate_width_of_cells();
          UpdateLocal();
          return;
      }
      case LyXTable::APPEND_CONT_ROW:
      {
	  int
              pos = actpos,
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
          // append the row into the table
          tabular->AppendContRow(cell_org);
          calculate_width_of_cells();
          UpdateLocal();
          return;
      }
      case LyXTable::APPEND_COLUMN:
      { 
	  int
              pos = 0,
              cell_org = actcell,
              cell = 0;
          do {
              if (pos && (par->IsNewline(pos-1))) {
                  if (tabular->AppendCellAfterCell(cell_org, cell)) {
                      par->InsertChar(pos, LYX_META_NEWLINE);
                      if (pos <= actpos)
                          ++actpos;
                      ++pos;
                  }
                  ++cell;
              }
              ++pos;
          } while (pos <= par->last());
          // remember that the very last cell doesn't end with a newline.
          // This saves one byte memory per table ;-)
          if (tabular->AppendCellAfterCell(cell_org, cell))
              par->InsertChar(par->last(), LYX_META_NEWLINE);
          // append the column into the table
          tabular->AppendColumn(cell_org);
          calculate_width_of_cells();
          UpdateLocal();
          return;
      }
      case LyXTable::DELETE_ROW:
          RemoveTableRow();
          calculate_width_of_cells();
          UpdateLocal();
          return;
      case LyXTable::DELETE_COLUMN:
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
                                               // at the end of a table
                      --pos; // because of ++pos below
                  }
                  ++cell;
              }
              ++pos;
          } while (pos <= par->last());
          /* delete the column from the table */ 
          tabular->DeleteColumn(actcell);
          calculate_width_of_cells();
          UpdateLocal();
          return;
      }
      case LyXTable::TOGGLE_LINE_TOP:
          lineSet = !tabular->TopLine(actcell);
	  for(i=selection_start; i<=selection_end; ++i)
	      tabular->SetTopLine(i,lineSet);
          calculate_width_of_cells();
          UpdateLocal();
          return;
    
      case LyXTable::TOGGLE_LINE_BOTTOM:
          lineSet = !tabular->BottomLine(actcell); 
	  for(i=selection_start; i<=selection_end; ++i)
	      tabular->SetBottomLine(i,lineSet);
          calculate_width_of_cells();
          UpdateLocal();
          return;
		
      case LyXTable::TOGGLE_LINE_LEFT:
          lineSet = !tabular->LeftLine(actcell);
	  for(i=selection_start; i<=selection_end; ++i)
	      tabular->SetLeftLine(i,lineSet);
          calculate_width_of_cells();
          UpdateLocal();
          return;

      case LyXTable::TOGGLE_LINE_RIGHT:
          lineSet = !tabular->RightLine(actcell);
	  for(i=selection_start; i<=selection_end; ++i)
	      tabular->SetRightLine(i,lineSet);
          calculate_width_of_cells();
          UpdateLocal();
          return;
      case LyXTable::ALIGN_LEFT:
      case LyXTable::ALIGN_RIGHT:
      case LyXTable::ALIGN_CENTER:
	  for(i=selection_start; i<=selection_end; ++i)
	      tabular->SetAlignment(i,setAlign);
          UpdateLocal();
          return;
      case LyXTable::MULTICOLUMN:
      {
	  if (tabular->row_of_cell(selection_start) !=
	      tabular->row_of_cell(selection_end)) {
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
		      pos = actpos;
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
	  tabular->SetMultiColumn(selection_start,number);
	  actpos = s_start;
	  sel_cell_end = sel_cell_start;
	  calculate_width_of_cells();
	  UpdateLocal();
          return;
      }
      case LyXTable::SET_ALL_LINES:
          setLines = 1;
      case LyXTable::UNSET_ALL_LINES:
	  for(i=selection_start; i<=selection_end; ++i)
	      tabular->SetAllLines(i, setLines);
          calculate_width_of_cells();
          UpdateLocal();
          return;
      case LyXTable::SET_LONGTABLE:
          tabular->SetLongTable(true);
	  UpdateLocal(); // because this toggles displayed
          return;
      case LyXTable::UNSET_LONGTABLE:
          tabular->SetLongTable(false);
	  UpdateLocal(); // because this toggles displayed
          return;
      case LyXTable::SET_ROTATE_TABLE:
          tabular->SetRotateTable(true);
          return;
      case LyXTable::UNSET_ROTATE_TABLE:
          tabular->SetRotateTable(false);
          return;
      case LyXTable::SET_ROTATE_CELL:
	  for(i=selection_start; i<=selection_end; ++i)
	      tabular->SetRotateCell(i,true);
          return;
      case LyXTable::UNSET_ROTATE_CELL:
	  for(i=selection_start; i<=selection_end; ++i)
	      tabular->SetRotateCell(i,false);
          return;
      case LyXTable::SET_LINEBREAKS:
          what = !tabular->Linebreaks(tabular->FirstVirtualCell(actcell));
	  for(i=selection_start; i<=selection_end; ++i)
	      tabular->SetLinebreaks(i,what);
          return;
      case LyXTable::SET_LTFIRSTHEAD:
          tabular->SetLTHead(actcell,true);
          return;
      case LyXTable::SET_LTHEAD:
          tabular->SetLTHead(actcell,false);
          return;
      case LyXTable::SET_LTFOOT:
          tabular->SetLTFoot(actcell,false);
          return;
      case LyXTable::SET_LTLASTFOOT:
          tabular->SetLTFoot(actcell,true);
          return;
      case LyXTable::SET_LTNEWPAGE:
          what = !tabular->LTNewPage(actcell);
          tabular->SetLTNewPage(actcell,what);
          return;
    }
#endif
}

void InsetTabular::RemoveTableRow()
{
}
