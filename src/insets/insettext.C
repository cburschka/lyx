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
#include "lyxscreen.h"

using std::ostream;
using std::ifstream;
using std::endl;
using std::min;
using std::max;

extern unsigned char getCurrentTextClass(Buffer *);

// Jürgen, we don't like macros, even small ones like this. (Lgb)
//#define TEXT(a) getLyXText(a)
// I created a inline function in insettext.h instead. (Lgb)

InsetText::InsetText()
{
    par = new LyXParagraph();
    init();
}


InsetText::InsetText(InsetText const & ins)
	: UpdatableInset()
{
    par = 0;
    init(&ins);
    autoBreakRows = ins.autoBreakRows;
}


InsetText & InsetText::operator=(InsetText const & it)
{
    init(&it);
    autoBreakRows = it.autoBreakRows;
    return * this;
}

void InsetText::init(InsetText const * ins)
{
    top_y = last_width = last_height = 0;
    insetAscent = insetDescent = insetWidth = 0;
    the_locking_inset = 0;
    cursor_visible = false;
    interline_space = 1;
    no_selection = false;
    need_update = INIT;
    drawTextXOffset = drawTextYOffset = 0;
    autoBreakRows = drawLockedFrame = false;
    xpos = 0.0;
    if (ins) {
	SetParagraphData(ins->par);
	autoBreakRows = ins->autoBreakRows;
	drawLockedFrame = ins->drawLockedFrame;
    }
    par->SetInsetOwner(this);
    frame_color = LColor::insetframe;
    locked = false;
    old_par = 0;
}


InsetText::~InsetText()
{
    LyXParagraph * p;
    p = par->next;
    delete par;
    while(p) {
	par = p;
	p = p->next;
	delete par;
    }
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
	string token;
    int pos = 0;
    LyXParagraph * return_par = 0;
    char depth = 0; // signed or unsigned?
    LyXParagraph::footnote_flag footnoteflag = LyXParagraph::NO_FOOTNOTE;
    LyXParagraph::footnote_kind footnotekind = LyXParagraph::FOOTNOTE;
    LyXFont font(LyXFont::ALL_INHERIT);

    LyXParagraph * p = par->next;
    delete par;
    while(p) {
	par = p;
	p = p->next;
	delete par;
    }
    par = new LyXParagraph;
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
    while(return_par) {
	return_par->SetInsetOwner(this);
	return_par = return_par->next;
    }
    
    if (token != "\\end_inset") {
        lex.printError("Missing \\end_inset at this point. "
                       "Read: `$$Token'");
    }
    need_update = INIT;
}


int InsetText::ascent(Painter &, LyXFont const &) const
{
    return insetAscent;
}


int InsetText::descent(Painter &, LyXFont const &) const
{
    return insetDescent;
}


int InsetText::width(Painter &, LyXFont const &) const
{
    return insetWidth;
}


int InsetText::textWidth(Painter & pain) const
{
    return getMaxTextWidth(pain, this) - drawTextXOffset;
}


void InsetText::draw(BufferView * bv, LyXFont const & f,
		     int baseline, float & x, bool cleared) const
{
    Painter & pain = bv->painter();

    if (!cleared && ((need_update==FULL) || (top_x!=int(x)) ||
		     (top_baseline!=baseline))) {
	int w =  insetWidth;
	int h = insetAscent + insetDescent;
	int ty = baseline - insetAscent;
	
	if (ty < 0)
	    ty = 0;
	if ((ty + h) > pain.paperHeight())
	    h = pain.paperHeight();
	if ((top_x + drawTextXOffset + w) > pain.paperWidth())
	    w = pain.paperWidth();
	pain.fillRectangle(top_x+drawTextXOffset, ty, w, h);
	cleared = true;
	need_update = FULL;
    }

    if (!cleared && (need_update == NONE))
	return;

    xpos = x;
    UpdatableInset::draw(bv, f, baseline, x, cleared);
 
    top_baseline = baseline;
    top_x = int(x);
    top_y = baseline - ascent(pain, f);
    last_width = width(pain, f);
    last_height = ascent(pain, f) + descent(pain, f);

    if (the_locking_inset && (cpos(bv) == inset_pos)) {
	inset_x = cx(bv) - top_x + drawTextXOffset;
	inset_y = cy(bv) + drawTextYOffset;
    }
    if (!cleared && (need_update == CURSOR)) {
	x += width(pain, f);
	need_update = NONE;
	return;
    }
    x += TEXT_TO_INSET_OFFSET; // place for border
    long int y = 0;
    Row * row = TEXT(bv)->GetRowNearY(y);
    y += baseline - row->ascent_of_text() + 1;
    if (cleared || !locked || (need_update == FULL)) {
	while (row != 0) {
	    TEXT(bv)->GetVisibleRow(bv, y, x, row, y);
	    y += row->height();
	    row = row->next();
	}
    } else if (need_update == SELECTION) {
	bv->screen()->ToggleToggle(getLyXText(bv), y, x);
    } else {
	locked = false;
	bv->screen()->Update(TEXT(bv), y, x);
	locked = true;
    }
    TEXT(bv)->refresh_y = 0;
    TEXT(bv)->status = LyXText::UNCHANGED;
    if (drawLockedFrame && locked) {
	    pain.rectangle(top_x, baseline - ascent(pain, f), width(pain, f),
			   ascent(pain,f) + descent(pain, f), frame_color);
    } else {
	    pain.rectangle(top_x, baseline - ascent(pain, f), width(pain, f),
			   ascent(pain,f) + descent(pain, f),
			   LColor::background);
    }
    x += width(pain, f) - TEXT_TO_INSET_OFFSET;
    need_update = NONE;
}


void InsetText::update(BufferView * bv, LyXFont const & font, bool dodraw)
{
    if (the_locking_inset)
	the_locking_inset->update(bv, font, dodraw);
    if (need_update == INIT) {
	deleteLyXText(bv);
	need_update = FULL;
    }
    if (dodraw && (need_update != CURSOR))
	    need_update = FULL;

    TEXT(bv)->FullRebreak(bv);

    long int y_temp = 0;
    Row * row = TEXT(bv)->GetRowNearY(y_temp);
    insetAscent = row->ascent_of_text() + TEXT_TO_INSET_OFFSET;
    insetDescent = TEXT(bv)->height - row->ascent_of_text() +
	TEXT_TO_INSET_OFFSET;
    insetWidth = max(textWidth(bv->painter()),
		     static_cast<int>(TEXT(bv)->width)) +
	(2 * TEXT_TO_INSET_OFFSET);
}


char const * InsetText::EditMessage() const
{
    return _("Opened Text Inset");
}


void InsetText::Edit(BufferView * bv, int x, int y, unsigned int button)
{
//    par->SetInsetOwner(this);
    UpdatableInset::Edit(bv, x, y, button);

    if (!bv->lockInset(this)) {
	lyxerr[Debug::INSETS] << "Cannot lock inset" << endl;
	return;
    }
    locked = true;
    the_locking_inset = 0;
    inset_pos = inset_x = inset_y = 0;
    TEXT(bv)->SetCursorFromCoordinates(bv, x-drawTextXOffset,
				       y+TEXT(bv)->first-drawTextYOffset+
				       insetAscent);
    checkAndActivateInset(bv, x, y, button);
    TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
    bv->text->FinishUndo();
    UpdateLocal(bv, FULL, false);
}


void InsetText::InsetUnlock(BufferView * bv)
{
    if (the_locking_inset) {
	the_locking_inset->InsetUnlock(bv);
	the_locking_inset = 0;
    }
    HideInsetCursor(bv);
    no_selection = false;
    locked = false;
    TEXT(bv)->selection = 0;
    UpdateLocal(bv, CURSOR_PAR, false);
    bv->owner()->getToolbar()->combox->select(bv->text->cursor.par()->GetLayout()+1);
}


bool InsetText::LockInsetInInset(BufferView * bv, UpdatableInset * inset)
{
    lyxerr[Debug::INSETS] << "InsetText::LockInsetInInset(" << inset << "): ";
    if (!inset)
	return false;
    if (inset == cpar(bv)->GetInset(cpos(bv))) {
	lyxerr[Debug::INSETS] << "OK" << endl;
	the_locking_inset = inset;
//	resetPos(bv->painter());
	inset_x = cx(bv) - top_x + drawTextXOffset;
	inset_y = cy(bv) + drawTextYOffset;
	inset_pos = cpos(bv);
	return true;
    } else if (the_locking_inset && (the_locking_inset == inset)) {
	if (cpos(bv) == inset_pos) {
	    lyxerr[Debug::INSETS] << "OK" << endl;
//	    resetPos(bv->painter());
	    inset_x = cx(bv) - top_x + drawTextXOffset;
	    inset_y = cy(bv) + drawTextYOffset;
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
	TEXT(bv)->UpdateInset(bv, inset);
        the_locking_inset = 0;
        if (lr)
            moveRight(bv, false);
	UpdateLocal(bv, FULL, false);
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
//    UpdateLocal(bv, FULL, false);
    TEXT(bv)->UpdateInset(bv, inset);
    if (cpos(bv) == inset_pos) {
	inset_x = cx(bv) - top_x + drawTextXOffset;
	inset_y = cy(bv) + drawTextYOffset;
    }
    return true;
}


void InsetText::InsetButtonPress(BufferView * bv, int x, int y, int button)
{
    if (TEXT(bv)->selection) {
	TEXT(bv)->selection = 0;
	UpdateLocal(bv, FULL, false);
    }
    no_selection = false;
    TEXT(bv)->SetCursorFromCoordinates(bv, x-drawTextXOffset,
				       y+TEXT(bv)->first-drawTextYOffset+
				       insetAscent);
    if (the_locking_inset) {
	UpdatableInset * inset = 0;
	if (cpar(bv)->GetChar(cpos(bv)) == LyXParagraph::META_INSET)
	    inset = static_cast<UpdatableInset*>(cpar(bv)->GetInset(cpos(bv)));
	if (the_locking_inset == inset) {
	    the_locking_inset->InsetButtonPress(bv,x-inset_x,y-inset_y,button);
	    return;
	} else if (inset) {
	    // otherwise unlock the_locking_inset and lock the new inset
	    the_locking_inset->InsetUnlock(bv);
	    inset_x = cx(bv) - top_x + drawTextXOffset;
	    inset_y = cy(bv) + drawTextYOffset;
	    inset->InsetButtonPress(bv, x - inset_x, y - inset_y, button);
	    inset->Edit(bv, x - inset_x, y - inset_y, button);
	    UpdateLocal(bv, FULL, false);
	    return;
	}
	// otherwise only unlock the_locking_inset
	the_locking_inset->InsetUnlock(bv);
	the_locking_inset = 0;
    }
    if (bv->the_locking_inset) {
	if ((cpar(bv)->GetChar(cpos(bv)) == LyXParagraph::META_INSET) &&
	    cpar(bv)->GetInset(cpos(bv)) &&
	    (cpar(bv)->GetInset(cpos(bv))->Editable() == Inset::HIGHLY_EDITABLE)) {
	    UpdatableInset * inset =
		static_cast<UpdatableInset*>(cpar(bv)->GetInset(cpos(bv)));
	    inset_x = cx(bv) - top_x + drawTextXOffset;
	    inset_y = cy(bv) + drawTextYOffset;
	    inset->InsetButtonPress(bv, x - inset_x, y - inset_y, button);
	    inset->Edit(bv, x - inset_x, y - inset_y, 0);
	    UpdateLocal(bv, FULL, false);
	}
    }
//    selection_start_cursor = selection_end_cursor = cursor;
}


void InsetText::InsetButtonRelease(BufferView * bv, int x, int y, int button)
{
    UpdatableInset * inset = 0;

    if (the_locking_inset) {
	    the_locking_inset->InsetButtonRelease(bv, x-inset_x, y-inset_y,button);
    } else {
	if (cpar(bv)->GetChar(cpos(bv)) == LyXParagraph::META_INSET) {
    	    inset = static_cast<UpdatableInset*>(cpar(bv)->GetInset(cpos(bv)));
	    if (inset->Editable() == Inset::HIGHLY_EDITABLE) {
		inset->InsetButtonRelease(bv, x - inset_x, y - inset_y,button);
	    } else {
		inset_x = cx(bv) - top_x + drawTextXOffset;
		inset_y = cy(bv) + drawTextYOffset;
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
#warning REDO this (Jug)
    if (!no_selection) {
//	LyXCursor old = selection_end_cursor;
	HideInsetCursor(bv);
//	setPos(bv->painter(), x, y);
//	selection_end_cursor = cursor;
//	if (old != selection_end_cursor)
//	    UpdateLocal(bv, false, false);
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
//	resetPos(bv->painter());
	return DISPATCHED;
    }

    result=DISPATCHED;
    if ((action < 0) && arg.empty())
        return FINISHED;

    if (the_locking_inset) {
        result = the_locking_inset->LocalDispatch(bv, action, arg);
	if (result == DISPATCHED_NOUPDATE)
	    return result;
	else if (result == DISPATCHED) {
	    UpdateLocal(bv, CURSOR_PAR, false);
            return result;
        } else if (result == FINISHED) {
	    switch(action) {
	    case -1:
	    case LFUN_RIGHT:
		moveRight(bv, false);
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
		if (TEXT(bv)->selection){
		    TEXT(bv)->CutSelection(bv, false);
		}
	    }
	    TEXT(bv)->ClearSelection();
	    for (string::size_type i = 0; i < arg.length(); ++i) {
		bv->owner()->getIntl()->getTrans()->TranslateAndInsert(arg[i], TEXT(bv));
	    }
	}
	UpdateLocal(bv, CURSOR_PAR, true);
	break;
        // --- Cursor Movements ---------------------------------------------
    case LFUN_RIGHTSEL:
	bv->text->FinishUndo();
	moveRight(bv, false);
	TEXT(bv)->SetSelection();
	UpdateLocal(bv, SELECTION, false);
	break;
    case LFUN_RIGHT:
	bv->text->FinishUndo();
	result = moveRight(bv);
	TEXT(bv)->selection = 0;
	TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
	UpdateLocal(bv, CURSOR, false);
	break;
    case LFUN_LEFTSEL:
	bv->text->FinishUndo();
	moveLeft(bv, false);
	TEXT(bv)->SetSelection();
	UpdateLocal(bv, SELECTION, false);
	break;
    case LFUN_LEFT:
	bv->text->FinishUndo();
	result= moveLeft(bv);
	TEXT(bv)->selection = 0;
	TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
	UpdateLocal(bv, CURSOR, false);
	break;
    case LFUN_DOWNSEL:
	bv->text->FinishUndo();
	moveDown(bv);
	TEXT(bv)->SetSelection();
	UpdateLocal(bv, SELECTION, false);
	break;
    case LFUN_DOWN:
	bv->text->FinishUndo();
	result = moveDown(bv);
	TEXT(bv)->selection = 0;
	TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
	UpdateLocal(bv, CURSOR, false);
	break;
    case LFUN_UPSEL:
	bv->text->FinishUndo();
	moveUp(bv);
	TEXT(bv)->SetSelection();
	UpdateLocal(bv, SELECTION, false);
	break;
    case LFUN_UP:
	bv->text->FinishUndo();
	result = moveUp(bv);
	TEXT(bv)->selection = 0;
	TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
	UpdateLocal(bv, CURSOR, false);
	break;
    case LFUN_HOME:
	bv->text->FinishUndo();
	TEXT(bv)->CursorHome(bv);
	TEXT(bv)->selection = 0;
	TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
	UpdateLocal(bv, CURSOR, false);
	break;
    case LFUN_END:
	TEXT(bv)->CursorEnd(bv);
	TEXT(bv)->selection = 0;
	TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
	UpdateLocal(bv, CURSOR, false);
	break;
    case LFUN_BACKSPACE:
	bv->text->SetUndo(bv->buffer(), Undo::DELETE, 
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next);
	TEXT(bv)->Backspace(bv);
	UpdateLocal(bv, CURSOR_PAR, true);
	break;
    case LFUN_DELETE:
	bv->text->SetUndo(bv->buffer(), Undo::DELETE, 
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next);
	TEXT(bv)->Delete(bv);
	UpdateLocal(bv, CURSOR_PAR, true);
	break;
    case LFUN_CUT:
	bv->text->SetUndo(bv->buffer(), Undo::DELETE, 
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next);
	TEXT(bv)->CutSelection(bv);
	UpdateLocal(bv, CURSOR_PAR, true);
	break;
    case LFUN_COPY:
	bv->text->FinishUndo();
	TEXT(bv)->CopySelection(bv);
	UpdateLocal(bv, CURSOR_PAR, false);
	break;
    case LFUN_PASTE:
	bv->text->SetUndo(bv->buffer(), Undo::INSERT, 
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next);
	TEXT(bv)->PasteSelection(bv);
	UpdateLocal(bv, CURSOR_PAR, true);
	break;
    case LFUN_BREAKPARAGRAPH:
	if (!autoBreakRows)
	    return DISPATCHED;
	TEXT(bv)->BreakParagraph(bv, 0);
	UpdateLocal(bv, FULL, true);
	break;
    case LFUN_BREAKLINE:
	if (!autoBreakRows)
	    return DISPATCHED;
	bv->text->SetUndo(bv->buffer(), Undo::INSERT, 
	    bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	    bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next);
	TEXT(bv)->InsertChar(bv, LyXParagraph::META_NEWLINE);
	UpdateLocal(bv, CURSOR_PAR, true);
	break;
    case LFUN_LAYOUT:
    {
      static LyXTextClass::size_type cur_layout = cpar(bv)->layout;
      
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
	    TEXT(bv)->SetLayout(bv, layout.second);
	    bv->owner()->getToolbar()->combox->select(cpar(bv)->GetLayout()+1);
	    UpdateLocal(bv, CURSOR_PAR, true);
	}
    }
    break;
    case LFUN_PARAGRAPH_SPACING:
	    // This one is absolutely not working. When fiddling with this
	    // it also seems to me that the paragraphs inside the insettext
	    // inherit bufferparams/paragraphparams in a strange way. (Lgb)
    {
	    LyXParagraph * par = TEXT(bv)->cursor.par();
	    Spacing::Space cur_spacing = par->spacing.getSpace();
	    float cur_value = 1.0;
	    if (cur_spacing == Spacing::Other) {
		    cur_value = par->spacing.getValue();
	    }
	    		
#ifdef HAVE_SSTREAM
	    std::istringstream istr(arg);
#else
	    istrstream istr(arg.c_str());
#endif
	    string tmp;
	    istr >> tmp;
	    Spacing::Space new_spacing = cur_spacing;
	    float new_value = cur_value;
	    if (tmp.empty()) {
		    lyxerr << "Missing argument to `paragraph-spacing'"
			   << endl;
	    } else if (tmp == "single") {
		    new_spacing = Spacing::Single;
	    } else if (tmp == "onehalf") {
		    new_spacing = Spacing::Onehalf;
	    } else if (tmp == "double") {
		    new_spacing = Spacing::Double;
	    } else if (tmp == "other") {
		    new_spacing = Spacing::Other;
		    float tmpval = 0.0;
		    istr >> tmpval;
		    lyxerr << "new_value = " << tmpval << endl;
		    if (tmpval != 0.0)
			    new_value = tmpval;
	    } else if (tmp == "default") {
		    new_spacing = Spacing::Default;
	    } else {
		    lyxerr << _("Unknown spacing argument: ")
			   << arg << endl;
	    }
	    if (cur_spacing != new_spacing || cur_value != new_value) {
		    par->spacing.set(new_spacing, new_value);
		    //TEXT(bv)->RedoParagraph(owner->view());
		    UpdateLocal(bv, CURSOR_PAR, true);
		    //bv->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
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
    LyXParagraph * p = par;
    while(p) {
	p->validate(features);
	p = p->next;
    }
}


int InsetText::BeginningOfMainBody(Buffer const * buf, LyXParagraph * p) const
{
    if (textclasslist.Style(buf->params.textclass,
			    p->GetLayout()).labeltype != LABEL_MANUAL)
	return 0;
    else
        return p->BeginningOfMainBody();
}


void InsetText::GetCursorPos(BufferView * bv, int & x, int & y) const
{
    x = cx(bv);
    y = cy(bv);
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

    LyXFont font = TEXT(bv)->GetFont(bv->buffer(), cpar(bv), cpos(bv));

    int asc = lyxfont::maxAscent(font);
    int desc = lyxfont::maxDescent(font);
  
    if (cursor_visible)
        bv->hideLockedInsetCursor();
    else
        bv->showLockedInsetCursor(cx(bv), cy(bv),
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
	LyXFont font = TEXT(bv)->GetFont(bv->buffer(), cpar(bv), cpos(bv));
	
	int asc = lyxfont::maxAscent(font);
	int desc = lyxfont::maxDescent(font);

	bv->fitLockedInsetCursor(cx(bv), cy(bv), asc, desc);
	bv->showLockedInsetCursor(cx(bv), cy(bv), asc, desc);
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


UpdatableInset::RESULT
InsetText::moveRight(BufferView * bv, bool activate_inset)
{
    if (!cpar(bv)->next && (cpos(bv) >= cpar(bv)->Last()))
	return FINISHED;
    if (activate_inset && checkAndActivateInset(bv)) {
	return DISPATCHED;
    }
    TEXT(bv)->CursorRight(bv);
//    real_current_font = current_font = GetFont(bv->buffer(), cpar(bv), cpos(bv));
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT
InsetText::moveLeft(BufferView * bv, bool activate_inset)
{
    if (!cpar(bv)->previous && (cpos(bv) <= 0))
	return FINISHED;
    TEXT(bv)->CursorLeft(bv);
    if (activate_inset)
	if (checkAndActivateInset(bv, -1, -1))
	    return DISPATCHED;
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT
InsetText::moveUp(BufferView * bv)
{
    if (!crow(bv)->previous())
	return FINISHED;
    TEXT(bv)->CursorUp(bv);
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT
InsetText::moveDown(BufferView * bv)
{
    if (!crow(bv)->next())
	return FINISHED;
    TEXT(bv)->CursorDown(bv);
    return DISPATCHED_NOUPDATE;
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
#ifdef NEW_WAY
    cpar(bv)->InsertInset(cpos(bv), inset);
#else
    cpar(bv)->InsertChar(cpos(bv), LyXParagraph::META_INSET);
    cpar(bv)->InsertInset(cpos(bv), inset);
#endif
    TEXT(bv)->selection = 0;
    UpdateLocal(bv, CURSOR_PAR, true);
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
    TEXT(bv)->SetFont(bv, font, toggleall);
}


void InsetText::UpdateLocal(BufferView * bv, UpdateCodes what, bool mark_dirty)
{
    if (TEXT(bv)->status == LyXText::NEED_MORE_REFRESH)
	need_update = FULL;
    else
	need_update = what;
    if ((need_update != CURSOR) || (TEXT(bv)->status != LyXText::UNCHANGED))
	    bv->updateInset(this, mark_dirty);
    if (old_par != cpar(bv)) {
	    bv->owner()->getToolbar()->combox->select(cpar(bv)->GetLayout()+1);
	    old_par = cpar(bv);
    }
}


bool InsetText::checkAndActivateInset(BufferView * bv, int x, int y,
				      int button)
{
    if (cpar(bv)->GetChar(cpos(bv)) == LyXParagraph::META_INSET) {
	UpdatableInset * inset =
	    static_cast<UpdatableInset*>(cpar(bv)->GetInset(cpos(bv)));
	LyXFont font = TEXT(bv)->GetFont(bv->buffer(), cpar(bv), cpos(bv));
	if (x < 0)
	    x = inset->width(bv->painter(), font);
	if (y < 0)
	    y = inset->descent(bv->painter(), font);
	inset_x = cx(bv) - top_x + drawTextXOffset;
	inset_y = cy(bv) + drawTextYOffset;
	inset->Edit(bv, x - inset_x, y - inset_y, button);
	if (!the_locking_inset)
	    return false;
	UpdateLocal(bv, CURSOR_PAR, false);
	return true;
    }
    return false;
}


int InsetText::getMaxTextWidth(Painter & pain,
			       UpdatableInset const * inset) const
{
    return getMaxWidth(pain, inset) - (2 * TEXT_TO_INSET_OFFSET);
}


void InsetText::SetParagraphData(LyXParagraph *p)
{
    LyXParagraph * np;

    if (par) {
	np = par->next;
	delete par;
	while(np) {
	    par = np;
	    np = np->next;
	    delete par;
	}
    }
    par = p->Clone();
    par->SetInsetOwner(this);
    np = par;
    while(p->next) {
	p = p->next;
	np->next = p->Clone();
	np->next->previous = np;
	np = np->next;
	np->SetInsetOwner(this);
    }
    need_update = INIT;
}


void InsetText::SetAutoBreakRows(bool flag)
{
    if (flag != autoBreakRows) {
	autoBreakRows = flag;
	need_update = FULL;
    }
}


void InsetText::SetDrawLockedFrame(bool flag)
{
    if (flag != drawLockedFrame)
	drawLockedFrame = flag;
}


void InsetText::SetFrameColor(LColor::color col)
{
    if (frame_color != col)
	frame_color = col;
}


LyXFont InsetText::GetDrawFont(BufferView * bv, LyXParagraph * p, int pos) const
{
    return TEXT(bv)->GetFont(bv->buffer(), p, pos);
}


int InsetText::cx(BufferView * bv) const
{
    return TEXT(bv)->cursor.x() + top_x + 1;
}


int InsetText::cy(BufferView * bv) const
{
    long int y_dummy = 0;
    Row * tmprow = TEXT(bv)->GetRowNearY(y_dummy);
    return TEXT(bv)->cursor.y() - tmprow->baseline();
}


int InsetText::cpos(BufferView * bv) const
{
    return TEXT(bv)->cursor.pos();
}


LyXParagraph * InsetText::cpar(BufferView * bv) const
{
    return TEXT(bv)->cursor.par();
}


Row * InsetText::crow(BufferView * bv) const
{
    return TEXT(bv)->cursor.row();
}


LyXText * InsetText::getLyXText(BufferView * bv) const
{
    if (cache.find(bv) != cache.end())
	return cache[bv];
    LyXText * lt = new LyXText(const_cast<InsetText *>(this));
    lt->init(bv);
    cache[bv] = lt;
    return lt;
}


void InsetText::deleteLyXText(BufferView * bv)
{
    cache.erase(bv);
}
