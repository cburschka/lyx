// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1998-2000 The LyX Team.
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

#include "insettext.h"
#include "lyxparagraph.h"
#include "lyxlex.h"
#include "debug.h"
#include "lyxfont.h"
#include "commandtags.h"
#include "buffer.h"
#include "LyXView.h"
#include "BufferView.h"
#include "layout.h"
#include "LaTeXFeatures.h"
#include "Painter.h"
#include "lyx_gui_misc.h"
#include "lyxtext.h"
#include "lyxcursor.h"
#include "CutAndPaste.h"
#include "font.h"
#include "minibuffer.h"
#include "toolbar.h"
#include "LColor.h"
#include "support/textutils.h"
#include "support/LAssert.h"
#include "lyxrow.h"
#include "lyxrc.h"
#include "intl.h"
#include "trans_mgr.h"

using std::ostream;
using std::ifstream;
using std::endl;
using std::min;
using std::max;

extern unsigned char getCurrentTextClass(Buffer *);
#warning BAAAAAAAADDDDDDD current_view (but Lars wanted it :) !!!
extern BufferView * current_view;


InsetText::InsetText()
{
    par = new LyXParagraph();
    init();
    text = new LyXText(this);
}


InsetText::InsetText(InsetText const & ins) : UpdatableInset()
{
    par = 0;
    init(&ins);
    text = new LyXText(this);
    autoBreakRows = ins.autoBreakRows;
}


InsetText & InsetText::operator=(InsetText const & it)
{
    init(&it);
    text = new LyXText(this);
    autoBreakRows = it.autoBreakRows;
    return * this;
}

void InsetText::init(InsetText const * ins)
{
    the_locking_inset = 0;
    cursor_visible = false;
    cursor.x_fix(-1);
    interline_space = 1;
    no_selection = false;
    init_inset = true;
    maxAscent = maxDescent = insetWidth = 0;
    drawTextXOffset = drawTextYOffset = 0;
    autoBreakRows = drawLockedFrame = false;
    xpos = 0.0;
    if (ins) {
	SetParagraphData(ins->par);
	autoBreakRows = ins->autoBreakRows;
	drawLockedFrame = ins->drawLockedFrame;
    }
    par->SetInsetOwner(this);
    cursor.par(par);
    cursor.pos(0);
    selection_start_cursor = selection_end_cursor = cursor;
    frame_color = LColor::insetframe;
    locked = false;
    old_par = 0;
}


InsetText::~InsetText()
{
    delete par;
}


Inset * InsetText::Clone() const
{
    InsetText * t = new InsetText(*this);
    return t;
}


void InsetText::Write(Buffer const * buf, ostream & os) const
{
    os << "Text\n";
    WriteParagraphData(buf, os);
}


void InsetText::WriteParagraphData(Buffer const * buf, ostream & os) const
{
    par->writeFile(buf, os, buf->params, 0, 0);
}


void InsetText::Read(Buffer const * buf, LyXLex & lex)
{
    string token, tmptok;
    int pos = 0;
    LyXParagraph * return_par = 0;
    char depth = 0; // signed or unsigned?
    LyXParagraph::footnote_flag footnoteflag = LyXParagraph::NO_FOOTNOTE;
    LyXParagraph::footnote_kind footnotekind = LyXParagraph::FOOTNOTE;
    LyXFont font(LyXFont::ALL_INHERIT);

    delete par;
    par = new LyXParagraph;
    par->SetInsetOwner(this);
    
    while (lex.IsOK()) {
        lex.nextToken();
        token = lex.GetString();
        if (token.empty())
            continue;
	if (token == "\\end_inset")
	    break;
	if (const_cast<Buffer*>(buf)->parseSingleLyXformat2Token(lex, par, return_par,
					    token, pos, depth,
					    font, footnoteflag,
					    footnotekind)) {
	    // the_end read this should NEVER happen
	    lex.printError("\\the_end read in inset! Error in document!");
	    return;
	}
    }
    if (!return_par)
	    return_par = par;
    par = return_par;
    if (token != "\\end_inset") {
        lex.printError("Missing \\end_inset at this point. "
                       "Read: `$$Token'");
    }
    init_inset = true;
}


int InsetText::ascent(Painter &, LyXFont const &) const
{
    long int y_temp = 0;
    Row * row = text->GetRowNearY(y_temp);
    return row->ascent_of_text() + 2;
}


int InsetText::descent(Painter &, LyXFont const &) const
{
    long int y = 0;
    Row * row = text->GetRowNearY(y);
    return text->height - row->ascent_of_text() + 2;
}


int InsetText::width(Painter & pain, LyXFont const &) const
{
    return std::max(static_cast<long int>(getMaxTextWidth(pain, this)),
		    text->width);
}


void InsetText::draw(Painter & pain, LyXFont const & f,
		     int baseline, float & x) const
{
    xpos = x;
    UpdatableInset::draw(pain, f, baseline, x);
 
#if 0   
    if (init_inset) {
	text->init(current_view);
	computeTextRows(pain);
	init_inset = false;
    }
#endif
    if ((baseline != top_baseline) || (top_x != int(x))) {
	top_baseline = baseline;
	top_x = int(x);
	computeBaselines(baseline);
    }
    if (the_locking_inset && (text->cursor.pos() == inset_pos)) {
	resetPos(pain);
	inset_x = cx() - top_x + drawTextXOffset;
	inset_y = cy() + drawTextYOffset;
    }
    if (drawLockedFrame && locked) {
	pain.rectangle(int(x), baseline - ascent(pain, f), width(pain, f),
		       ascent(pain,f) + descent(pain, f), frame_color);
    }
    x += TEXT_TO_INSET_OFFSET; // place for border
#if 1
    long int y = 0;
    Row * row = text->GetRowNearY(y);
    y += baseline - row->ascent_of_text() + 1;
    while (row != 0) {
	text->GetVisibleRow(current_view, y, x, row, y);
	y += row->height();
	row = row->next();
    }
#else
    for(RowList::size_type r = 0; r < rows.size() - 1; ++r) {
        drawRowSelection(pain, rows[r].pos, rows[r + 1].pos, r, 
                         rows[r].baseline, x);
        drawRowText(pain, rows[r].pos, rows[r + 1].pos, rows[r].baseline, x);
    }
#endif
    x += insetWidth - TEXT_TO_INSET_OFFSET;
}


void InsetText::update(BufferView * bv, LyXFont const &) const
{
    if (init_inset) {
	text->init(bv);
	init_inset = false;
#if 0
	// Dump all rowinformation:
	long y_dummy = 0;
	Row * tmprow = text->GetRowNearY(y_dummy);
	lyxerr << "Width = " << text->width << endl;
	lyxerr << "Baseline Paragraph Pos Height Ascent Fill\n";
	while (tmprow) {
		lyxerr << tmprow->baseline() << '\t'
		       << tmprow->par() << '\t'
		       << tmprow->pos() << '\t'
		       << tmprow->height() << '\t'
		       << tmprow->ascent_of_text() << '\t'
		       << tmprow->fill() << '\n';
		tmprow = tmprow->next();
	}
	lyxerr.flush();
#endif
    }
}


void InsetText::drawRowSelection(Painter & pain, int startpos, int endpos,
                                 int row, int baseline, float x) const
{
    if (!hasSelection())
	return;

    int s_start, s_end;
    if (selection_start_cursor.pos() > selection_end_cursor.pos()) {
	s_start = selection_end_cursor.pos();
	s_end = selection_start_cursor.pos();
    } else {
	s_start = selection_start_cursor.pos();
	s_end = selection_end_cursor.pos();
    }
    if ((s_start > endpos) || (s_end < startpos))
	return;
    
    int esel_x;
    int ssel_x = esel_x = int(x);
    LyXFont font;
    int p = startpos;
    for(; p < endpos; ++p) {
	if (p == s_start)
	    ssel_x = int(x);
	if ((p >= s_start) && (p <= s_end))
	    esel_x = int(x);
	char ch = par->GetChar(p);
	font = GetDrawFont(current_view->buffer(), par, p);
	if (IsFloatChar(ch)) {
	    // skip for now
	} else if (ch == LyXParagraph::META_INSET) {
	    Inset const * tmpinset = par->GetInset(p);
	    x += tmpinset->width(pain, font);
	} else {
	    x += lyxfont::width(ch, font);
	}
    }
    if (p == s_start)
	ssel_x = int(x);
    if ((p >= s_start) && (p <= s_end))
	esel_x = int(x);
    if (ssel_x < esel_x) {
	pain.fillRectangle(int(ssel_x), baseline-rows[row].asc,
			   int(esel_x - ssel_x),
			   rows[row].asc + rows[row].desc,
			   LColor::selection);
    }
}


void InsetText::drawRowText(Painter & pain, int startpos, int endpos,
			    int baseline, float x) const
{
    Assert(endpos <= par->Last());

    for(int p = startpos; p < endpos; ++p) {
	char ch = par->GetChar(p);
	LyXFont font = GetDrawFont(current_view->buffer(), par, p);
	if (IsFloatChar(ch)) {
	    // skip for now
	} else if (par->IsNewline(p)) {
		// Draw end-of-line marker
		int wid = lyxfont::width('n', font);
		int asc = lyxfont::maxAscent(font);
		int y = baseline;
		int xp[3], yp[3];
		
		xp[0] = int(x + wid * 0.375);
		yp[0] = int(y - 0.875 * asc * 0.75);
		
		xp[1] = int(x);
		yp[1] = int(y - 0.500 * asc * 0.75);
		
		xp[2] = int(x + wid * 0.375);
		yp[2] = int(y - 0.125 * asc * 0.75);
		
		pain.lines(xp, yp, 3, LColor::eolmarker);
		
		xp[0] = int(x);
		yp[0] = int(y - 0.500 * asc * 0.75);
		
		xp[1] = int(x + wid);
		yp[1] = int(y - 0.500 * asc * 0.75);
		
		xp[2] = int(x + wid);
		yp[2] = int(y - asc * 0.75);
			
		pain.lines(xp, yp, 3, LColor::eolmarker);
		x += wid;
	} else if (ch == LyXParagraph::META_INSET) {
	    Inset * tmpinset = par->GetInset(p);
	    if (tmpinset) 
		tmpinset->draw(pain, font, baseline, x);
	} else {
	    pain.text(int(x), baseline, ch, font);
	    x += lyxfont::width(ch, font);
	}
    }
}


char const * InsetText::EditMessage() const
{
    return _("Opened Text Inset");
}


void InsetText::Edit(BufferView * bv, int x, int y, unsigned int button)
{
    par->SetInsetOwner(this);
    UpdatableInset::Edit(bv, x, y, button);

    if (!bv->lockInset(this)) {
	lyxerr[Debug::INSETS] << "Cannot lock inset" << endl;
	return;
    }
    locked = true;
    the_locking_inset = 0;
    inset_pos = inset_x = inset_y = 0;
    setPos(bv->painter(), x, y);
    checkAndActivateInset(bv, x, y, button);
    selection_start_cursor = selection_end_cursor = cursor;
    text->sel_cursor = text->cursor;
    bv->text->FinishUndo();
    UpdateLocal(bv, true, false);
}


void InsetText::InsetUnlock(BufferView * bv)
{
    if (the_locking_inset) {
	the_locking_inset->InsetUnlock(bv);
	the_locking_inset = 0;
    }
    HideInsetCursor(bv);
    lyxerr[Debug::INSETS] << "InsetText::InsetUnlock(" << this <<
	    ")" << endl;
    selection_start_cursor = selection_end_cursor = cursor;
    no_selection = false;
    locked = false;
    UpdateLocal(bv, true, false);
}


bool InsetText::LockInsetInInset(BufferView * bv, UpdatableInset * inset)
{
    lyxerr[Debug::INSETS] << "InsetText::LockInsetInInset(" << inset << "): ";
    if (!inset)
	return false;
    if (inset == par->GetInset(text->cursor.pos())) {
	lyxerr[Debug::INSETS] << "OK" << endl;
	the_locking_inset = inset;
	resetPos(bv->painter());
	inset_x = cx() - top_x + drawTextXOffset;
	inset_y = cy() + drawTextYOffset;
	inset_pos = text->cursor.pos();
	return true;
    } else if (the_locking_inset && (the_locking_inset == inset)) {
	if (text->cursor.pos() == inset_pos) {
	    lyxerr[Debug::INSETS] << "OK" << endl;
	    resetPos(bv->painter());
	    inset_x = cx() - top_x + drawTextXOffset;
	    inset_y = cy() + drawTextYOffset;
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


bool InsetText::UnlockInsetInInset(BufferView * bv, UpdatableInset * inset,
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


bool InsetText::UpdateInsetInInset(BufferView * bv, Inset * inset)
{
    if (!the_locking_inset)
        return false;
    if (the_locking_inset != inset)
        return the_locking_inset->UpdateInsetInInset(bv, inset);
    lyxerr[Debug::INSETS] << "InsetText::UpdateInsetInInset(" << inset <<
	    ")" << endl;
    UpdateLocal(bv, true, false);
    if (text->cursor.pos() == inset_pos) {
	inset_x = cx() - top_x + drawTextXOffset;
	inset_y = cy() + drawTextYOffset;
    }
    return true;
}


void InsetText::InsetButtonPress(BufferView * bv, int x, int y, int button)
{
    if (hasSelection()) {
	selection_start_cursor = selection_end_cursor = cursor;
	UpdateLocal(bv, false, false);
    }
    no_selection = false;
    setPos(bv->painter(), x, y);
    cursor.x_fix(-1);
    if (the_locking_inset) {
	UpdatableInset * inset = 0;
	if (par->GetChar(cursor.pos()) == LyXParagraph::META_INSET)
	    inset = static_cast<UpdatableInset*>(par->GetInset(cursor.pos()));
	if (the_locking_inset == inset) {
	    the_locking_inset->InsetButtonPress(bv,x-inset_x,y-inset_y,button);
	    return;
	} else if (inset) {
	    // otherwise unlock the_locking_inset and lock the new inset
	    the_locking_inset->InsetUnlock(bv);
	    inset_x = cursor.x() - top_x + drawTextXOffset;
	    inset_y = cursor.y() + drawTextYOffset;
	    inset->InsetButtonPress(bv, x - inset_x, y - inset_y, button);
	    inset->Edit(bv, x - inset_x, y - inset_y, button);
	    UpdateLocal(bv, true, false);
	    return;
	}
	// otherwise only unlock the_locking_inset
	the_locking_inset->InsetUnlock(bv);
	the_locking_inset = 0;
    }
    if (bv->the_locking_inset) {
	if ((par->GetChar(cursor.pos()) == LyXParagraph::META_INSET) &&
	    par->GetInset(cursor.pos()) &&
	    (par->GetInset(cursor.pos())->Editable() == Inset::HIGHLY_EDITABLE)) {
	    UpdatableInset * inset =
		static_cast<UpdatableInset*>(par->GetInset(cursor.pos()));
	    inset_x = cursor.x() - top_x + drawTextXOffset;
	    inset_y = cursor.y() + drawTextYOffset;
	    inset->InsetButtonPress(bv, x - inset_x, y - inset_y, button);
	    inset->Edit(bv, x - inset_x, y - inset_y, 0);
	    UpdateLocal(bv, true, false);
	}
    }
    selection_start_cursor = selection_end_cursor = cursor;
}


void InsetText::InsetButtonRelease(BufferView * bv, int x, int y, int button)
{
    UpdatableInset * inset = 0;

    if (the_locking_inset) {
	    the_locking_inset->InsetButtonRelease(bv, x-inset_x, y-inset_y,button);
    } else {
	if (par->GetChar(cursor.pos()) == LyXParagraph::META_INSET) {
    	    inset = static_cast<UpdatableInset*>(par->GetInset(cursor.pos()));
	    if (inset->Editable() == Inset::HIGHLY_EDITABLE) {
		inset->InsetButtonRelease(bv, x - inset_x, y - inset_y,button);
	    } else {
		inset_x = cursor.x() - top_x + drawTextXOffset;
		inset_y = cursor.y() + drawTextYOffset;
		inset->InsetButtonRelease(bv, x - inset_x, y - inset_y,button);
		inset->Edit(bv, x - inset_x, y - inset_y, button);
	    }
	}
    }
    no_selection = false;
}


void InsetText::InsetMotionNotify(BufferView * bv, int x, int y, int state)
{
    if (the_locking_inset) {
        the_locking_inset->InsetMotionNotify(bv, x - inset_x,
					     y - inset_y,state);
        return;
    }
    if (!no_selection) {
	LyXCursor old = selection_end_cursor;
	HideInsetCursor(bv);
	setPos(bv->painter(), x, y);
	selection_end_cursor = cursor;
	if (old != selection_end_cursor)
	    UpdateLocal(bv, false, false);
	ShowInsetCursor(bv);
    }
    no_selection = false;
}


void InsetText::InsetKeyPress(XKeyEvent * xke)
{
    if (the_locking_inset) {
        the_locking_inset->InsetKeyPress(xke);
        return;
    }
}


UpdatableInset::RESULT
InsetText::LocalDispatch(BufferView * bv,
			 int action, string const & arg)
{
    no_selection = false;
    UpdatableInset::RESULT
        result= UpdatableInset::LocalDispatch(bv, action, arg);
    if (result != UNDISPATCHED) {
	resetPos(bv->painter());
	return DISPATCHED;
    }

    result=DISPATCHED;
    if ((action < 0) && arg.empty())
        return FINISHED;

    if ((action != LFUN_DOWN) && (action != LFUN_UP) &&
        (action != LFUN_DOWNSEL) && (action != LFUN_UPSEL))
        cursor.x_fix(-1);
    if (the_locking_inset) {
        result = the_locking_inset->LocalDispatch(bv, action, arg);
	if (result == DISPATCHED_NOUPDATE)
	    return result;
	else if (result == DISPATCHED) {
	    the_locking_inset->ToggleInsetCursor(bv);
	    UpdateLocal(bv, false, false);
	    the_locking_inset->ToggleInsetCursor(bv);
            return result;
        } else if (result == FINISHED) {
	    switch(action) {
	    case -1:
	    case LFUN_RIGHT:
		cursor.pos(inset_pos + 1);
		resetPos(bv->painter());
		break;
	    case LFUN_DOWN:
		moveDown(bv);
		break;
	    }
	    the_locking_inset = 0;
	    return DISPATCHED;
	}
    }
    HideInsetCursor(bv);
    switch (action) {
	// Normal chars
    case -1:
	if (bv->buffer()->isReadonly()) {
	    LyXBell();
//	    setErrorMessage(N_("Document is read only"));
	    break;
	}
	if (!arg.empty()) {
	    /* Automatically delete the currently selected
	     * text and replace it with what is being
	     * typed in now. Depends on lyxrc settings
	     * "auto_region_delete", which defaults to
	     * true (on). */

	    bv->text->SetUndo(bv->buffer(), Undo::INSERT, 
			      bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
			      bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next);
	    if (lyxrc.auto_region_delete) {
		if (text->selection){
		    text->CutSelection(bv, false);
		}
	    }
	    text->ClearSelection();
	    for (string::size_type i = 0; i < arg.length(); ++i) {
		bv->owner()->getIntl()->getTrans()->TranslateAndInsert(arg[i], text);
	    }
	}
	UpdateLocal(bv, true, true);
	break;
        // --- Cursor Movements ---------------------------------------------
    case LFUN_RIGHTSEL:
	bv->text->FinishUndo();
	moveRight(bv, false);
	text->SetSelection();
	UpdateLocal(bv, false, false);
	break;
    case LFUN_RIGHT:
	bv->text->FinishUndo();
	result = moveRight(bv);
	text->sel_cursor = text->cursor;
	UpdateLocal(bv, false, false);
	break;
    case LFUN_LEFTSEL:
	bv->text->FinishUndo();
	moveLeft(bv, false);
	text->SetSelection();
	UpdateLocal(bv, false, false);
	break;
    case LFUN_LEFT:
	bv->text->FinishUndo();
	result= moveLeft(bv);
	text->sel_cursor = text->cursor;
	UpdateLocal(bv, false, false);
	break;
    case LFUN_DOWNSEL:
	bv->text->FinishUndo();
	moveDown(bv);
	selection_end_cursor = cursor;
	UpdateLocal(bv, false, false);
	break;
    case LFUN_DOWN:
	bv->text->FinishUndo();
	result = moveDown(bv);
	text->sel_cursor = text->cursor;
	UpdateLocal(bv, false, false);
	break;
    case LFUN_UPSEL:
	bv->text->FinishUndo();
	moveUp(bv);
	selection_end_cursor = cursor;
	UpdateLocal(bv, false, false);
	break;
    case LFUN_UP:
	bv->text->FinishUndo();
	result = moveUp(bv);
	text->sel_cursor = text->cursor;
	UpdateLocal(bv, false, false);
	break;
    case LFUN_HOME:
	bv->text->FinishUndo();
	text->CursorHome(bv);
	text->sel_cursor = text->cursor;
	UpdateLocal(bv, false, false);
	break;
    case LFUN_END:
	text->CursorEnd(bv);
	text->sel_cursor = text->cursor;
	UpdateLocal(bv, false, false);
	break;
    case LFUN_BACKSPACE:
	bv->text->SetUndo(bv->buffer(), Undo::DELETE, 
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next);
	text->Backspace(bv);
	UpdateLocal(bv, true, true);
	break;
    case LFUN_DELETE:
	bv->text->SetUndo(bv->buffer(), Undo::DELETE, 
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next);
	text->Delete(bv);
	UpdateLocal(bv, true, true);
	break;
    case LFUN_CUT:
	bv->text->SetUndo(bv->buffer(), Undo::DELETE, 
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next);
	text->CutSelection(bv);
	UpdateLocal(bv, true, true);
	break;
    case LFUN_COPY:
	bv->text->FinishUndo();
	text->CopySelection(bv);
	UpdateLocal(bv, true, false);
	break;
    case LFUN_PASTE:
	bv->text->SetUndo(bv->buffer(), Undo::INSERT, 
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next);
	text->PasteSelection(bv);
	UpdateLocal(bv, true, true);
	break;
    case LFUN_BREAKPARAGRAPH:
	if (!autoBreakRows)
	    return DISPATCHED;
	text->BreakParagraph(bv, 0);
	UpdateLocal(bv, true, true);
	break;
    case LFUN_BREAKLINE:
	if (!autoBreakRows)
	    return DISPATCHED;
	bv->text->SetUndo(bv->buffer(), Undo::INSERT, 
	    bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	    bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next);
	text->InsertChar(bv, LyXParagraph::META_NEWLINE);
	UpdateLocal(bv, true, true);
	break;
    case LFUN_LAYOUT:
    {
      static LyXTextClass::size_type cur_layout = par->layout;
      
	// Derive layout number from given argument (string)
	// and current buffer's textclass (number). */    
	LyXTextClassList::ClassList::size_type tclass =
	    bv->buffer()->params.textclass;
	std::pair <bool, LyXTextClass::size_type> layout = 
	    textclasslist.NumberOfLayout(tclass, arg);

	// If the entry is obsolete, use the new one instead.
	if (layout.first) {
	    string obs = textclasslist.Style(tclass,layout.second).
		obsoleted_by();
	    if (!obs.empty()) 
		layout = textclasslist.NumberOfLayout(tclass, obs);
	}

	// see if we found the layout number:
	if (!layout.first) {
	    string msg = string(N_("Layout ")) + arg + N_(" not known");

	    bv->owner()->getMiniBuffer()->Set(msg);
	    break;
	}

	if (cur_layout != layout.second) {
	    cur_layout = layout.second;
	    text->SetLayout(bv, layout.second);
	    bv->owner()->getToolbar()->combox->select(cursor.par()->GetLayout()+1);
	    UpdateLocal(bv, true, true);
	}
    }
    break;
    default:
	result = UNDISPATCHED;
	break;
    }
    if (result != FINISHED) {
	ShowInsetCursor(bv);
    } else
        bv->unlockInset(this);
    return result;
}


int InsetText::Latex(Buffer const * buf, ostream & os, bool, bool) const
{
    TexRow texrow;
    buf->latexParagraphs(os, par, 0, texrow);
    return texrow.rows();
}


void InsetText::Validate(LaTeXFeatures & features) const
{
    par->validate(features);
}


// Returns the width of a character at a certain spot
int InsetText::SingleWidth(Painter & pain, LyXParagraph * p, int pos) const
{
    LyXFont font = GetDrawFont(current_view->buffer(), p, pos);
    char c = p->GetChar(pos);

    if (IsPrintable(c)) {
        return lyxfont::width(c, font);
    } else if (c == LyXParagraph::META_INSET) {
        Inset const * tmpinset = p->GetInset(pos);
        if (tmpinset)
            return tmpinset->width(pain, font);
        else
            return 0;
    } else if (IsSeparatorChar(c))
        c = ' ';
    else if (IsNewlineChar(c))
        c = 'n';
    return lyxfont::width(c, font);
}


// Returns the width of a character at a certain spot
void InsetText::SingleHeight(Painter & pain, LyXParagraph * p,int pos,
			     int & asc, int & desc) const
{
    LyXFont font = GetDrawFont(current_view->buffer(), p, pos);
    char c = p->GetChar(pos);

    asc = desc = 0;
    if (c == LyXParagraph::META_INSET) {
        Inset const * tmpinset=p->GetInset(pos);
        if (tmpinset) {
	    asc = tmpinset->ascent(pain, font);
	    desc = tmpinset->descent(pain, font);
        }
    } else {
        asc = lyxfont::maxAscent(font);
        desc = lyxfont::maxDescent(font);
    }
    return;
}


// Gets the fully instantiated font at a given position in a paragraph
// Basically the same routine as LyXParagraph::getFont() in paragraph.C.
// The difference is that this one is used for displaying, and thus we
// are allowed to make cosmetic improvements. For instance make footnotes
// smaller. (Asger)
// If position is -1, we get the layout font of the paragraph.
// If position is -2, we get the font of the manual label of the paragraph.
LyXFont InsetText::GetFont(Buffer const * buf, LyXParagraph * p, int pos) const
{
    char par_depth = p->GetDepth();

    LyXLayout const & layout =
	    textclasslist.Style(buf->params.textclass, p->GetLayout());

    // We specialize the 95% common case:
    if (p->footnoteflag == LyXParagraph::NO_FOOTNOTE && !par_depth) {
        if (pos >= 0) {
            // 95% goes here
            if (layout.labeltype == LABEL_MANUAL
                && pos < BeginningOfMainBody(buf, p)) {
		// 1% goes here
                return p->GetFontSettings(buf->params,
					  pos).realize(layout.reslabelfont);
            } else
                return p->GetFontSettings(buf->params,
					  pos).realize(layout.resfont);
        } else {
            // 5% goes here.
            // process layoutfont for pos == -1 and labelfont for pos < -1
            if (pos == -1)
                return layout.resfont;
            else
                return layout.reslabelfont;
        }
    }
    // The uncommon case need not be optimized as much

    LyXFont layoutfont, tmpfont;

    if (pos >= 0){
        // 95% goes here
        if (pos < BeginningOfMainBody(buf, p)) {
            // 1% goes here
            layoutfont = layout.labelfont;
        } else {
            // 99% goes here
            layoutfont = layout.font;
        }
        tmpfont = p->GetFontSettings(buf->params, pos);
        tmpfont.realize(layoutfont);
    } else{
        // 5% goes here.
        // process layoutfont for pos == -1 and labelfont for pos < -1
        if (pos == -1)
            tmpfont = layout.font;
        else
            tmpfont = layout.labelfont;
    }
    
    // Resolve against environment font information
    //if (par->GetDepth()){ // already in while condition
    while (p && par_depth && !tmpfont.resolved()) {
        p = p->DepthHook(par_depth - 1);
        if (p) {
            tmpfont.realize(textclasslist.Style(buf->params.textclass,
                                                p->GetLayout()).font);
            par_depth = p->GetDepth();
        }
    }
    tmpfont.realize((textclasslist.TextClass(buf->params.textclass).
                    defaultfont()));
    return tmpfont;
}


// the font for drawing may be different from the real font
LyXFont InsetText::GetDrawFont(Buffer const * buf, LyXParagraph * p, int pos) const
{
    return text->GetFont(buf, p, pos);
}


int InsetText::BeginningOfMainBody(Buffer const * buf, LyXParagraph * p) const
{
    if (textclasslist.Style(buf->params.textclass,
			    p->GetLayout()).labeltype != LABEL_MANUAL)
	return 0;
    else
        return p->BeginningOfMainBody();
}


void InsetText::GetCursorPos(int & x, int & y) const
{
    x = cursor.x();
    y = cursor.y();
}


int InsetText::InsetInInsetY()
{
    if (!the_locking_inset)
	return 0;

    return (inset_y + the_locking_inset->InsetInInsetY());
}


void InsetText::ToggleInsetCursor(BufferView * bv)
{
    if (the_locking_inset) {
        the_locking_inset->ToggleInsetCursor(bv);
        return;
    }

    LyXFont font = GetDrawFont(bv->buffer(), cpar(), cpos());

    int asc = lyxfont::maxAscent(font);
    int desc = lyxfont::maxDescent(font);
  
    if (cursor_visible)
        bv->hideLockedInsetCursor();
    else
        bv->showLockedInsetCursor(cx(), cy(),
				  asc, desc);
    cursor_visible = !cursor_visible;
}


void InsetText::ShowInsetCursor(BufferView * bv)
{
    if (the_locking_inset) {
	the_locking_inset->ShowInsetCursor(bv);
	return;
    }
    if (!cursor_visible) {
	LyXFont font = GetDrawFont(bv->buffer(), cpar(), cpos());
	
	int asc = lyxfont::maxAscent(font);
	int desc = lyxfont::maxDescent(font);

	bv->fitLockedInsetCursor(cx(), cy(), asc, desc);
	bv->showLockedInsetCursor(cx(), cy(), asc, desc);
	cursor_visible = true;
    }
}


void InsetText::HideInsetCursor(BufferView * bv)
{
    if (cursor_visible) {
        bv->hideLockedInsetCursor();
	cursor_visible = false;
    }
    if (the_locking_inset)
	the_locking_inset->HideInsetCursor(bv);
}


void InsetText::setPos(Painter & pain, int x, int y) const
{
    x -= drawTextXOffset;
    y -= drawTextYOffset;
    // search right X-pos x==0 -> top_x
    cursor.pos(0);
    actrow = 0;
    cursor.y(top_baseline);
    y += cursor.y();
    for(unsigned int i = 1;
	(long(cursor.y() + rows[i - 1].desc) < y)
		&& (i < rows.size() - 1); ++i) {
	cursor.y(rows[i].baseline);
	cursor.pos(rows[i].pos);
	actrow = i;
    }
    cursor.y(cursor.y() - top_baseline);
    cursor.x(top_x + 2); // 2 = frame width
    x += cursor.x();

    int swh;
    int sw = swh = SingleWidth(pain, par, cursor.pos());
    if (par->GetChar(cursor.pos()) != LyXParagraph::META_INSET)
	swh /= 2;
    int checkpos = rows[actrow + 1].pos;
    if ((actrow + 2) < (int)rows.size())
	--checkpos;
    while ((cursor.pos() < checkpos) && ((cursor.x() + swh) < x)) {
	cursor.x(cursor.x() + sw);
	cursor.pos(cursor.pos() + 1);
	sw = swh = SingleWidth(pain, par, cursor.pos());
	if (par->GetChar(cursor.pos())!=LyXParagraph::META_INSET)
	    swh /= 2;
    }
}


void InsetText::resetPos(Painter & pain) const
{
    cursor.par(par);

    if (!rows.size())
	return;

    int old_pos = cursor.pos();

    cursor.y(top_baseline);
    actrow = 0;
    for(unsigned int i = 0;
	i < (rows.size() - 1) && rows[i].pos <= cursor.pos();
	++i) {
	cursor.y(rows[i].baseline);
	actrow = i;
    }
    cursor.y(cursor.y() - top_baseline);
    setPos(pain, 0, cursor.y());
    cursor.x(top_x + 2); // 2 = frame width
    while(cursor.pos() < old_pos) {
	cursor.x(cursor.x() + SingleWidth(pain, par,cursor.pos()));
	cursor.pos(cursor.pos() + 1);
    }
}


UpdatableInset::RESULT
InsetText::moveRight(BufferView * bv, bool activate_inset)
{
    if (!cpar()->next && (cpos() >= cpar()->Last()))
	return FINISHED;
    if (activate_inset && checkAndActivateInset(bv)) {
	return DISPATCHED;
    }
    text->CursorRight(bv);
//    real_current_font = current_font = GetFont(bv->buffer(), cpar(), cpos());
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT
InsetText::moveLeft(BufferView * bv, bool activate_inset)
{
    if (!cpar()->previous && (cpos() <= 0))
	return FINISHED;
    text->CursorLeft(bv);
    if (activate_inset)
	if (checkAndActivateInset(bv, -1, -1))
	    return DISPATCHED;
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT
InsetText::moveUp(BufferView * bv)
{
    if (!crow()->previous())
	return FINISHED;
    text->CursorUp(bv);
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT
InsetText::moveDown(BufferView * bv)
{
    if (!crow()->next())
	return FINISHED;
    text->CursorDown(bv);
    return DISPATCHED_NOUPDATE;
}


bool InsetText::Delete()
{
    if ((par->GetChar(cursor.pos())==LyXParagraph::META_INSET) &&
	!par->GetInset(cursor.pos())->Deletable()) {
	return false;
    }
    par->Erase(cursor.pos());
    return true;
}


bool InsetText::InsertInset(BufferView * bv, Inset * inset)
{
    if (the_locking_inset) {
	if (the_locking_inset->InsertInsetAllowed(inset))
	    return the_locking_inset->InsertInset(bv, inset);
	return false;
    }
    bv->text->SetUndo(bv->buffer(), Undo::INSERT, 
	      bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	      bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next);
    if (inset->Editable() == Inset::IS_EDITABLE) {
	UpdatableInset * i = static_cast<UpdatableInset *>(inset);
	i->setOwner(static_cast<UpdatableInset *>(this));
    }
    par->InsertChar(cursor.pos(), LyXParagraph::META_INSET);
    par->InsertInset(cursor.pos(), inset);
    if (hasSelection()) {
	selection_start_cursor = selection_end_cursor = cursor;
    } else {
	selection_start_cursor = selection_end_cursor = cursor;
    }
    UpdateLocal(bv, true, true);
    static_cast<UpdatableInset*>(inset)->Edit(bv, 0, 0, 0);
    return true;
}


UpdatableInset * InsetText::GetLockingInset()
{
    return the_locking_inset ? the_locking_inset->GetLockingInset() : this;
}


UpdatableInset * InsetText::GetFirstLockingInsetOfType(Inset::Code c)
{
    if (c == LyxCode())
	return this;
    if (the_locking_inset)
	return the_locking_inset->GetFirstLockingInsetOfType(c);
    return 0;
}


void InsetText::SetFont(BufferView * bv, LyXFont const & font, bool toggleall)
{
    text->SetFont(bv, font, toggleall);
}


void InsetText::computeTextRows(Painter & pain) const
{
    int p,
	nwp = 0,
	asc = 0,
	desc = 0,
	oasc = 0,
	odesc = 0,
	wordAscent,
	wordDescent;
    row_struct row;

    if (rows.size())
	    rows.clear();
    int width = wordAscent = wordDescent = 0;
    insetWidth = maxAscent = maxDescent = 0;
    row.asc      = 0;
    row.desc     = 0;
    row.pos      = 0;
    row.baseline = 0;
    rows.push_back(row);
    if (!autoBreakRows || (getMaxTextWidth(pain, this) < 0)) {
	for(p = 0; p < par->Last(); ++p) {
	    insetWidth += SingleWidth(pain, par, p);
	    SingleHeight(pain, par, p, asc, desc);
	    maxAscent = max(maxAscent, asc);
	    maxDescent = max(maxDescent, desc);
	}
	insetWidth += (2 * TEXT_TO_INSET_OFFSET);
	rows[0].asc = maxAscent;
	rows[0].desc = maxDescent;
	// alocate a dummy row for the endpos
	row.pos = par->Last();
	rows.push_back(row);
	return;
    }

    bool is_first_word_in_row = true;
    int cw, lastWordWidth = 0;
    int maxWidth = getMaxTextWidth(pain, this);
    // if we auto break rows than the insetwidth should be always the max
    // width as the display is stable it may get larger if we have a really
    // large word below and we draw it!!!
    insetWidth = maxWidth;

    for(p = 0; p < par->Last(); ++p) {
	if (par->IsNewline(p)) {
	    rows.back().asc = wordAscent;
	    rows.back().desc = wordDescent;
	    row.pos = p+1;
	    rows.push_back(row);
	    nwp = p+1;
	    width = lastWordWidth = 0;
	    oasc = odesc = wordAscent = wordDescent = 0;
	    is_first_word_in_row = true;
	    continue;
	}
	cw = SingleWidth(pain, par, p);
	Inset * inset = 0;
	if (par->GetChar(p) == LyXParagraph::META_INSET)
	    inset = par->GetInset(p);
	if (inset && inset->display()) {
	    inset->setOwner(const_cast<InsetText *>(this));
	    if (cw > insetWidth)
		insetWidth = cw;
	    if (!is_first_word_in_row || (p != nwp)) {
		oasc = max(oasc, wordAscent);
		odesc = max(odesc, wordDescent);
		rows.back().asc = oasc;
		rows.back().desc = odesc;
		row.pos = p;
		rows.push_back(row);
	    }
	    SingleHeight(pain, par, p, asc, desc);
	    rows.back().asc = asc;
	    rows.back().desc = desc;
	    row.pos = nwp = p + 1;
	    rows.push_back(row);
	    width = lastWordWidth = 0;
	    oasc = odesc = wordAscent = wordDescent = 0;
	    is_first_word_in_row = true;
	    continue;
	}
	SingleHeight(pain, par, p, asc, desc);
	width += cw;
	lastWordWidth += cw;
	if (width > maxWidth) {
	    if (is_first_word_in_row) {
		if (!(width-cw)) { // only this character in word
		    rows.back().asc = asc;
		    rows.back().desc = desc;
		    row.pos = p+1;
		    rows.push_back(row);
		    oasc = 0;
		    odesc = 0;
		    wordAscent = 0;
		    wordDescent = 0;
		    nwp = p + 1;
		    lastWordWidth = width = 0;
		} else {
		    rows.back().asc = wordAscent;
		    rows.back().desc = wordDescent;
		    row.pos = p;
		    rows.push_back(row);
		    oasc = 0;
		    odesc = 0;
		    wordAscent = asc;
		    wordDescent = desc;
		    lastWordWidth = width = cw;
		    nwp = p;
		}
	    } else {
		rows.back().asc = oasc;
		rows.back().desc = odesc;
		row.pos = nwp;
		rows.push_back(row);
		oasc = wordAscent;
		odesc = wordDescent;
		width = lastWordWidth;	
		wordAscent = max(wordAscent, asc);
		wordDescent = max(wordDescent, desc);
		is_first_word_in_row = true;
	    }
	} else {
	    wordAscent = max(wordAscent, asc);
	    wordDescent = max(wordDescent, desc);
	}
	if (par->IsSeparator(p) || inset) {
	    if (inset) {
		inset->setOwner(const_cast<InsetText *>(this));
		if (cw > maxWidth)
		    insetWidth = cw;
	    }
	    oasc = max(oasc, wordAscent);
	    odesc = max(odesc, wordDescent);
	    wordAscent = wordDescent = lastWordWidth = 0;
	    nwp = p + 1;
	    is_first_word_in_row = false;
	}
    }
    // if we have some data in the paragraph we have ascent/descent
    if (p) {
	// assign last row data
	rows.back().asc = max(oasc, wordAscent);
	rows.back().desc = max(odesc, wordDescent);
    }
    insetWidth += (2 * TEXT_TO_INSET_OFFSET);
    // alocate a dummy row for the endpos
    row.pos = par->Last();
    rows.push_back(row);
    // calculate maxAscent/Descent
    maxAscent = rows[0].asc;
    maxDescent = rows[0].desc;
    for (RowList::size_type i = 1; i < rows.size() - 1; ++i) {
	maxDescent += rows[i].asc + rows[i].desc + interline_space;
    }
}


void InsetText::computeBaselines(int baseline) const
{
    rows[0].baseline = baseline;
    for (unsigned int i = 1; i < rows.size() - 1; i++) {
	rows[i].baseline = rows[i - 1].baseline + rows[i - 1].desc + 
	    rows[i].asc + interline_space;
    }
}


void InsetText::UpdateLocal(BufferView * bv, bool what, bool mark_dirty)
{
    if (what) {
	text->FullRebreak(bv);
    }
    bv->updateInset(this, mark_dirty);
//    if (flag)
//	resetPos(bv->painter());
    if (old_par != cpar()) {
	    bv->owner()->getToolbar()->combox->select(cpar()->GetLayout()+1);
	    old_par = cpar();
    }
}


bool InsetText::cutSelection(Buffer const * buf)
{
    if (!hasSelection())
	return false;

    CutAndPaste cap;

    LyXParagraph * endpar = par;
    int start, end;
    if (selection_start_cursor.pos() > selection_end_cursor.pos()) {
	    start = selection_end_cursor.pos();
	    end = selection_start_cursor.pos();
    } else {
	    start = selection_start_cursor.pos();
	    end = selection_end_cursor.pos();
    }

    return cap.cutSelection(par, &endpar, start, end, buf->params.textclass);
}


bool InsetText::copySelection(Buffer const * buf)
{
    if (!hasSelection())
	return false;

    CutAndPaste cap;

    int start, end;
    if (selection_start_cursor.pos() > selection_end_cursor.pos()) {
	    start = selection_end_cursor.pos();
	    end = selection_start_cursor.pos();
    } else {
	    start = selection_start_cursor.pos();
	    end = selection_end_cursor.pos();
    }
    return cap.copySelection(par, par, start, end, buf->params.textclass);
}


bool InsetText::pasteSelection(Buffer const * buf)
{
    CutAndPaste cap;

    if (cap.nrOfParagraphs() > 1) {
	WriteAlert(_("Impossible operation"),
		   _("Cannot include more than one paragraph!"),
		   _("Sorry."));
	return false;
    }
    LyXParagraph * endpar;
    LyXParagraph * actpar = par;

    int pos = cursor.pos();
    bool ret = cap.pasteSelection(&actpar, &endpar, pos,buf->params.textclass);
    cursor.pos(pos);
    return ret;
}


bool InsetText::checkAndActivateInset(BufferView * bv, int x, int y,
				      int button)
{
    if (par->GetChar(cursor.pos()) == LyXParagraph::META_INSET) {
	UpdatableInset * inset =
	    static_cast<UpdatableInset*>(par->GetInset(cursor.pos()));
	LyXFont font = text->GetFont(bv->buffer(), cpar(), cpos());
	if (x < 0)
	    x = inset->width(bv->painter(), font);
	if (y < 0)
	    y = inset->descent(bv->painter(), font);
	inset_x = cursor.x() - top_x + drawTextXOffset;
	inset_y = cursor.y() + drawTextYOffset;
	inset->Edit(bv, x - inset_x, y - inset_y, button);
	if (!the_locking_inset)
	    return false;
	UpdateLocal(bv, true, false);
	return true;
    }
    return false;
}


int InsetText::getMaxTextWidth(Painter & pain, UpdatableInset const * inset) const
{
//    int w=getMaxWidth(pain, inset);
//    return (w - x);
    return getMaxWidth(pain, inset) - 4; // 2+2 width of eventual border
}

void InsetText::SetParagraphData(LyXParagraph *p)
{
    if (par)
	delete par;
    par = p->Clone();
    par->SetInsetOwner(this);
    init_inset = true;
}

void InsetText::SetAutoBreakRows(bool flag)
{
    if (flag != autoBreakRows) {
	autoBreakRows = flag;
	init_inset = true;
    }
}

void InsetText::SetDrawLockedFrame(bool flag)
{
    if (flag != drawLockedFrame) {
	drawLockedFrame = flag;
	init_inset = true;
    }
}

void InsetText::SetFrameColor(LColor::color col)
{
    if (frame_color != col) {
	frame_color = col;
	init_inset = true;
    }
}

int InsetText::cx() const
{
    return text->cursor.x() + top_x + 1;
}

int InsetText::cy() const
{
    long int y_dummy = 0;
    Row * tmprow = text->GetRowNearY(y_dummy);
    return text->cursor.y() - tmprow->baseline();
}

int InsetText::cpos() const
{
    return text->cursor.pos();
}

LyXParagraph * InsetText::cpar() const
{
    return text->cursor.par();
}

Row * InsetText::crow() const
{
    return text->cursor.row();
}
