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
#include "LColor.h"
#include "support/textutils.h"
#include "support/LAssert.h"
#include "lyxrow.h"
#include "lyxrc.h"
#include "intl.h"
#include "trans_mgr.h"
#include "lyxscreen.h"
#include "WorkArea.h"

using std::ostream;
using std::ifstream;
using std::endl;
using std::min;
using std::max;

extern unsigned char getCurrentTextClass(Buffer *);

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
    top_y = 0;
    last_width = 0;
    last_height = 0;
    insetAscent = 0;
    insetDescent = 0;
    insetWidth = 0;
    the_locking_inset = 0;
    cursor_visible = false;
    interline_space = 1;
    no_selection = false;
    need_update = INIT;
    drawTextXOffset = 0;
    drawTextYOffset = 0;
    autoBreakRows = false;
    drawFrame = NEVER;
    xpos = 0.0;
    if (ins) {
	SetParagraphData(ins->par);
	autoBreakRows = ins->autoBreakRows;
	drawFrame = ins->drawFrame;
    }
    par->SetInsetOwner(this);
    frame_color = LColor::insetframe;
    locked = false;
    old_par = 0;
}


InsetText::~InsetText()
{
    for(Cache::const_iterator cit = cache.begin(); cit != cache.end(); ++cit)
	delete (*cit).second;
//	deleteLyXText((*cit).first);
    LyXParagraph * p = par->next;
    delete par;
    while(p) {
	par = p;
	p = p->next;
	delete par;
    }
}


void InsetText::clear()
{
    LyXParagraph * p = par->next;
    delete par;
    while(p) {
	par = p;
	p = p->next;
	delete par;
    }
    par = new LyXParagraph();
}


Inset * InsetText::Clone(Buffer const &) const
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
#ifndef NEW_INSETS
    LyXParagraph::footnote_flag footnoteflag = LyXParagraph::NO_FOOTNOTE;
    LyXParagraph::footnote_kind footnotekind = LyXParagraph::FOOTNOTE;
#endif
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
	if (const_cast<Buffer*>(buf)->
	    parseSingleLyXformat2Token(lex, par, return_par,token, pos, depth,
				       font
#ifndef NEW_INSETS
				       , footnoteflag, footnotekind
#endif
				       ))
	{
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


int InsetText::ascent(BufferView * bv, LyXFont const &) const
{
    int y_temp = 0;
    Row * row = TEXT(bv)->GetRowNearY(y_temp);
    insetAscent = row->ascent_of_text() + TEXT_TO_INSET_OFFSET;
    return insetAscent;
}


int InsetText::descent(BufferView * bv, LyXFont const &) const
{
    int y_temp = 0;
    Row * row = TEXT(bv)->GetRowNearY(y_temp);
    insetDescent = TEXT(bv)->height - row->ascent_of_text() +
	TEXT_TO_INSET_OFFSET;
    return insetDescent;
}


int InsetText::width(BufferView * bv, LyXFont const &) const
{
    insetWidth = TEXT(bv)->width + (2 * TEXT_TO_INSET_OFFSET);
    return insetWidth;
}


int InsetText::textWidth(Painter & pain) const
{
    int w = getMaxWidth(pain, this);
    return w;
}


void InsetText::draw(BufferView * bv, LyXFont const & f,
		     int baseline, float & x, bool cleared) const
{
    Painter & pain = bv->painter();

    // no draw is necessary !!!
    if ((drawFrame == LOCKED) && !locked && !par->size()) {
	if (!cleared && (need_update == CLEAR_FRAME)) {
	    pain.rectangle(top_x + 1, baseline - insetAscent + 1,
			   width(bv, f) - 1,
			   insetAscent + insetDescent - 1,
			   LColor::background);
	}
	top_x = int(x);
	top_baseline = baseline;
	x += width(bv, f);
	need_update = NONE;
	return;
    }

    xpos = x;
    UpdatableInset::draw(bv, f, baseline, x, cleared);

    if (!cleared && ((need_update==FULL) || (top_x!=int(x)) ||
		     (top_baseline!=baseline))) {
	int w =  insetWidth;
	int h = insetAscent + insetDescent;
	int ty = baseline - insetAscent;
	
	if (ty < 0) {
	    h += ty;
	    ty = 0;
	}
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

    if (top_x != int(x)) {
	need_update = INIT;
	top_x = int(x);
	bv->text->status = LyXText::CHANGED_IN_DRAW;
	return;
    }

    top_baseline = baseline;
    top_y = baseline - ascent(bv, f);
    last_width = width(bv, f);
    last_height = ascent(bv, f) + descent(bv, f);

    if (the_locking_inset && (cpar(bv) == inset_par) && (cpos(bv) == inset_pos)) {
	inset_x = cx(bv) - top_x + drawTextXOffset;
	inset_y = cy(bv) + drawTextYOffset;
    }
    if (!cleared && (need_update == CURSOR) && !TEXT(bv)->selection) {
	x += width(bv, f);
	need_update = NONE;
	return;
    }
    x += TEXT_TO_INSET_OFFSET;
    {
	int y = 0;
	Row * row = TEXT(bv)->GetRowNearY(y);
	int y_offset = baseline - row->ascent_of_text();
	int ph = pain.paperHeight();
	int first = 0;
	y = y_offset;
	while ((row != 0) && ((y+row->height()) <= 0)) {
	    y += row->height();
	    first += row->height();
	    row = row->next();
	}
	if (y_offset < 0)
	    y_offset = y;
	TEXT(bv)->first = first;
	if (cleared || !locked || (need_update == FULL)) {
	    int yf = y_offset;
	    y = 0;
	    while ((row != 0) && (yf < ph)) {
		    TEXT(bv)->GetVisibleRow(bv, y+y_offset, int(x), row,
					    y+first, cleared);
		y += row->height();
		yf += row->height();
		row = row->next();
	    }
	} else if (need_update == SELECTION) {
	    bv->screen()->ToggleToggle(TEXT(bv), y_offset, int(x));
	} else {
	    locked = false;
	    if (need_update == CURSOR) {
		bv->screen()->ToggleSelection(TEXT(bv), true, y_offset,int(x));
		TEXT(bv)->ClearSelection();
		TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
	    }
	    bv->screen()->Update(TEXT(bv), y_offset, int(x));
	    locked = true;
	}
    }
    TEXT(bv)->refresh_y = 0;
    TEXT(bv)->status = LyXText::UNCHANGED;
    if ((need_update != CURSOR_PAR) &&
	((drawFrame == ALWAYS) || ((drawFrame == LOCKED) && locked)))
    {
	    pain.rectangle(top_x + 1, baseline - insetAscent + 1,
			   width(bv, f) - 1, insetAscent + insetDescent - 1,
			   frame_color);
    } else if (need_update == CLEAR_FRAME) {
	    pain.rectangle(top_x + 1, baseline - insetAscent + 1,
			   width(bv, f) - 1, insetAscent + insetDescent - 1,
			   LColor::background);
    }
    x += width(bv, f) - TEXT_TO_INSET_OFFSET;
    if (bv->text->status==LyXText::CHANGED_IN_DRAW)
	need_update = INIT;
    else if (need_update != INIT)
	need_update = NONE;
}


void InsetText::update(BufferView * bv, LyXFont const & font, bool reinit)
{
    if (reinit) {  // && (need_update != CURSOR)) {
	need_update = INIT;
	resizeLyXText(bv);
	if (owner())
	    owner()->update(bv, font, true);
	return;
    }
    if (the_locking_inset) {
	inset_x = cx(bv) - top_x + drawTextXOffset;
	inset_y = cy(bv) + drawTextYOffset;
	the_locking_inset->update(bv, font, reinit);
    }
    if (need_update == INIT) {
	resizeLyXText(bv);
	need_update = FULL;
//	if (!owner() && bv->text)
//	    bv->text->UpdateInset(bv, this);
    }
    int oldw = insetWidth;
#if 1
    insetWidth = TEXT(bv)->width + (2 * TEXT_TO_INSET_OFFSET);
    // max(textWidth(bv->painter()),
    // static_cast<int>(TEXT(bv)->width) + drawTextXOffset) +
    // (2 * TEXT_TO_INSET_OFFSET);
#else
    insetWidth = textWidth(bv->painter());
    if (insetWidth < 0)
	    insetWidth = static_cast<int>(TEXT(bv)->width);
#endif
    if (oldw != insetWidth) {
//	    printf("TW(%p): %d-%d-%d-%d\n",this,insetWidth, oldw,
//		   textWidth(bv->painter()),static_cast<int>(TEXT(bv)->width));
	resizeLyXText(bv);
	need_update = FULL;
#if 0
	if (owner()) {
	    owner()->update(bv, font, reinit);
	    return;
	} else {
	    update(bv, font, reinit);
	}
#else
#if 1
	update(bv, font, reinit);
#else
	UpdateLocal(bv, INIT, false);
#endif
#endif
	return;
    }
    if ((need_update==CURSOR_PAR) && (TEXT(bv)->status==LyXText::UNCHANGED) &&
	the_locking_inset)
    {
	TEXT(bv)->UpdateInset(bv, the_locking_inset);
    }

    if (TEXT(bv)->status == LyXText::NEED_MORE_REFRESH)
	need_update = FULL;

    int y_temp = 0;
    Row * row = TEXT(bv)->GetRowNearY(y_temp);
    insetAscent = row->ascent_of_text() + TEXT_TO_INSET_OFFSET;
    insetDescent = TEXT(bv)->height - row->ascent_of_text() +
	TEXT_TO_INSET_OFFSET;
}


void InsetText::UpdateLocal(BufferView * bv, UpdateCodes what, bool mark_dirty)
{
    TEXT(bv)->FullRebreak(bv);
    if (need_update != INIT) {
	if (TEXT(bv)->status == LyXText::NEED_MORE_REFRESH)
	    need_update = FULL;
	else if (!the_locking_inset || (what != CURSOR))
	    need_update = what;
    }
    if ((need_update != CURSOR) || (TEXT(bv)->status != LyXText::UNCHANGED) ||
	TEXT(bv)->selection)
	    bv->updateInset(this, mark_dirty);
    bv->owner()->showState();
    if (old_par != cpar(bv)) {
	    bv->owner()->setLayout(cpar(bv)->GetLayout());
	    old_par = cpar(bv);
    }
}


string const InsetText::EditMessage() const
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
    inset_par = 0;
    old_par = 0;
    if (!checkAndActivateInset(bv, x, y, button))
	TEXT(bv)->SetCursorFromCoordinates(bv, x-drawTextXOffset,
					   y+insetAscent);
    TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
    bv->text->FinishUndo();
    ShowInsetCursor(bv);
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
    UpdateLocal(bv, CLEAR_FRAME, false);
    if (owner())
	    bv->owner()->setLayout(owner()->getLyXText(bv)
				    ->cursor.par()->GetLayout());
    else
	    bv->owner()->setLayout(bv->text->cursor.par()->GetLayout());
}


bool InsetText::LockInsetInInset(BufferView * bv, UpdatableInset * inset)
{
    lyxerr[Debug::INSETS] << "InsetText::LockInsetInInset(" << inset << "): ";
    if (!inset)
	return false;
    if (inset == cpar(bv)->GetInset(cpos(bv))) {
	lyxerr[Debug::INSETS] << "OK" << endl;
	the_locking_inset = inset;
	inset_x = cx(bv) - top_x + drawTextXOffset;
	inset_y = cy(bv) + drawTextYOffset;
	inset_pos = cpos(bv);
	inset_par = cpar(bv);
	TEXT(bv)->UpdateInset(bv, the_locking_inset);
	return true;
    } else if (the_locking_inset && (the_locking_inset == inset)) {
	if (cpar(bv) == inset_par && cpos(bv) == inset_pos) {
	    lyxerr[Debug::INSETS] << "OK" << endl;
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
	old_par = 0; // force layout setting
	UpdateLocal(bv, CURSOR_PAR, false);
        return true;
    }
    return the_locking_inset->UnlockInsetInInset(bv, inset, lr);
}


bool InsetText::UpdateInsetInInset(BufferView * bv, Inset * inset)
{
    if (!the_locking_inset)
        return false;
    if (the_locking_inset != inset) {
	TEXT(bv)->UpdateInset(bv, the_locking_inset);
	need_update = CURSOR_PAR;
        return the_locking_inset->UpdateInsetInInset(bv, inset);
    }
//    UpdateLocal(bv, FULL, false);
    if (TEXT(bv)->UpdateInset(bv, inset))
	UpdateLocal(bv, CURSOR_PAR, false);
    if (cpar(bv) == inset_par && cpos(bv) == inset_pos) {
	inset_x = cx(bv) - top_x + drawTextXOffset;
	inset_y = cy(bv) + drawTextYOffset;
    }
    return true;
}


void InsetText::InsetButtonPress(BufferView * bv, int x, int y, int button)
{
    no_selection = false;

    int tmp_x = x - drawTextXOffset;
    int tmp_y = y + insetAscent;
    Inset * inset = bv->checkInsetHit(TEXT(bv), tmp_x, tmp_y, button);

    HideInsetCursor(bv);
    if (the_locking_inset) {
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
	    if (the_locking_inset) {
		UpdateLocal(bv, CURSOR_PAR, false);
	    }
	    return;
	}
	// otherwise only unlock the_locking_inset
	the_locking_inset->InsetUnlock(bv);
	the_locking_inset = 0;
    }
    if (bv->theLockingInset()) {
	if (inset && inset->Editable() == Inset::HIGHLY_EDITABLE) {
	    UpdatableInset * uinset = static_cast<UpdatableInset*>(inset);
	    inset_x = cx(bv) - top_x + drawTextXOffset;
	    inset_y = cy(bv) + drawTextYOffset;
	    inset_pos = cpos(bv);
	    inset_par = cpar(bv);
	    uinset->InsetButtonPress(bv, x - inset_x, y - inset_y, button);
	    uinset->Edit(bv, x - inset_x, y - inset_y, 0);
	    if (the_locking_inset) {
		UpdateLocal(bv, CURSOR_PAR, false);
	    }
	    return;
	}
    }
    if (!inset) {
	bool paste_internally = false;
	if ((button == 2) && TEXT(bv)->selection) {
	    LocalDispatch(bv, LFUN_COPY, "");
	    paste_internally = true;
	}
	TEXT(bv)->SetCursorFromCoordinates(bv, x-drawTextXOffset,
					   y+insetAscent);
	TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
	UpdateLocal(bv, CURSOR, false);
	bv->owner()->setLayout(cpar(bv)->GetLayout());
	old_par = cpar(bv);
	// Insert primary selection with middle mouse
	// if there is a local selection in the current buffer,
	// insert this
	if (button == 2) {
	    if (paste_internally)
		LocalDispatch(bv, LFUN_PASTE, "");
	    else
		LocalDispatch(bv, LFUN_PASTESELECTION, "paragraph");
	}
    }
    ShowInsetCursor(bv);
}


void InsetText::InsetButtonRelease(BufferView * bv, int x, int y, int button)
{
    UpdatableInset * inset = 0;

    if (the_locking_inset) {
	    the_locking_inset->InsetButtonRelease(bv,
						  x - inset_x, y - inset_y,
						  button);
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
	    UpdateLocal(bv, CURSOR_PAR, false);
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
	HideInsetCursor(bv);
	TEXT(bv)->SetCursorFromCoordinates(bv, x-drawTextXOffset,
					   y+insetAscent);
	TEXT(bv)->SetSelection();
	if (TEXT(bv)->toggle_cursor.par()!=TEXT(bv)->toggle_end_cursor.par() ||
	    TEXT(bv)->toggle_cursor.pos()!=TEXT(bv)->toggle_end_cursor.pos())
	    UpdateLocal(bv, SELECTION, false);
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
#ifndef NEW_INSETS
			      bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
			      bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next
#else
			      bv->text->cursor.par()->previous,
			      bv->text->cursor.par()->next
#endif
		    );
	    // if an empty paragraph set the language to the surronding
	    // paragraph language on insertion of the first character!
	    if (!par->Last() && !par->next) {
		LyXText * text = 0;
		if (owner()) {
		    Inset * inset = owner();
		    while(inset && inset->getLyXText(bv) == TEXT(bv))
			inset = inset->owner();
		    if (inset)
			text = inset->getLyXText(bv);
		}
		if (!text)
		    text = bv->text;
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(text->cursor.par()->getParLanguage(bv->buffer()->params));
		SetFont(bv, font, false);
	    }
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
	moveRight(bv, false, true);
	TEXT(bv)->SetSelection();
	UpdateLocal(bv, SELECTION, false);
	break;
    case LFUN_RIGHT:
	result = moveRight(bv);
	bv->text->FinishUndo();
	TEXT(bv)->ClearSelection();
	UpdateLocal(bv, CURSOR, false);
	break;
    case LFUN_LEFTSEL:
	bv->text->FinishUndo();
	moveLeft(bv, false, true);
	TEXT(bv)->SetSelection();
	UpdateLocal(bv, SELECTION, false);
	break;
    case LFUN_LEFT:
	bv->text->FinishUndo();
	result= moveLeft(bv);
	TEXT(bv)->ClearSelection();
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
	TEXT(bv)->ClearSelection();
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
	TEXT(bv)->ClearSelection();
	TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
	UpdateLocal(bv, CURSOR, false);
	break;
    case LFUN_HOME:
	bv->text->FinishUndo();
	TEXT(bv)->CursorHome(bv);
	TEXT(bv)->ClearSelection();
	TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
	UpdateLocal(bv, CURSOR, false);
	break;
    case LFUN_END:
	TEXT(bv)->CursorEnd(bv);
	TEXT(bv)->ClearSelection();
	TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
	UpdateLocal(bv, CURSOR, false);
	break;
    case LFUN_BACKSPACE:
	bv->text->SetUndo(bv->buffer(), Undo::DELETE,
#ifndef NEW_INSETS
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next
#else
	  bv->text->cursor.par()->previous,
	  bv->text->cursor.par()->next
#endif
		);
	if (TEXT(bv)->selection)
	    TEXT(bv)->CutSelection(bv);
	else
	    TEXT(bv)->Backspace(bv);
	UpdateLocal(bv, CURSOR_PAR, true);
	break;
    case LFUN_DELETE:
	bv->text->SetUndo(bv->buffer(), Undo::DELETE,
#ifndef NEW_INSETS
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next
#else
	  bv->text->cursor.par()->previous,
	  bv->text->cursor.par()->next
#endif
		);
	if (TEXT(bv)->selection)
	    TEXT(bv)->CutSelection(bv);
	else
	    TEXT(bv)->Delete(bv);
	UpdateLocal(bv, CURSOR_PAR, true);
	break;
    case LFUN_CUT:
	bv->text->SetUndo(bv->buffer(), Undo::DELETE,
#ifndef NEW_INSETS
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next
#else
	  bv->text->cursor.par()->previous,
	  bv->text->cursor.par()->next
#endif
		);
	TEXT(bv)->CutSelection(bv);
	UpdateLocal(bv, CURSOR_PAR, true);
	break;
    case LFUN_COPY:
	bv->text->FinishUndo();
	TEXT(bv)->CopySelection(bv);
	UpdateLocal(bv, CURSOR_PAR, false);
	break;
    case LFUN_PASTESELECTION:
    {
	string clip(bv->workarea()->getClipboard());
	
	if (clip.empty())
	    break;
	if (arg == "paragraph") {
		TEXT(bv)->InsertStringB(bv, clip);
	} else {
		TEXT(bv)->InsertStringA(bv, clip);
	}
	UpdateLocal(bv, CURSOR_PAR, true);
	break;
    }
    case LFUN_PASTE:
	if (!autoBreakRows) {
	    CutAndPaste cap;

	    if (cap.nrOfParagraphs() > 1) {
		WriteAlert(_("Impossible operation"),
			   _("Cannot include more than one paragraph!"),
			   _("Sorry."));
		break;
	    }
	}
	bv->text->SetUndo(bv->buffer(), Undo::INSERT,
#ifndef NEW_INSETS
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	  bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next
#else
	  bv->text->cursor.par()->previous,
	  bv->text->cursor.par()->next
#endif
		);
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
#ifndef NEW_INSETS
	    bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	    bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next
#else
	    bv->text->cursor.par()->previous,
	    bv->text->cursor.par()->next
#endif
		);
	TEXT(bv)->InsertChar(bv, LyXParagraph::META_NEWLINE);
	UpdateLocal(bv, CURSOR_PAR, true);
	break;
    case LFUN_LAYOUT:
	// do not set layouts on non breakable textinsets
	if (autoBreakRows) {
	    LyXTextClass::size_type cur_layout = cpar(bv)->layout;
      
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
		bv->owner()->setLayout(cpar(bv)->GetLayout());
		UpdateLocal(bv, CURSOR_PAR, true);
	    }
	} else {
	    // reset the layout box
	    bv->owner()->setLayout(cpar(bv)->GetLayout());
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
	    		
	    std::istringstream istr(arg.c_str());
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


int InsetText::Ascii(Buffer const * buf, ostream & os, int linelen) const
{
    LyXParagraph * p = par;
    unsigned int lines = 0;
    
    string tmp;
    while (p) {
	tmp = buf->asciiParagraph(p, linelen);
	lines += countChar(tmp, '\n');
	os << tmp;
	p = p->next;
    }
    return lines;
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


void InsetText::GetCursorPos(BufferView * bv,
			     int & x, int & y) const
{
    x = cx(bv);
    y = cy(bv);
}


unsigned int InsetText::InsetInInsetY()
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
        bv->showLockedInsetCursor(cx(bv), cy(bv), asc, desc);
    cursor_visible = !cursor_visible;
}


void InsetText::ShowInsetCursor(BufferView * bv, bool show)
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
	if (show)
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
InsetText::moveRight(BufferView * bv, bool activate_inset, bool selecting)
{
    if (!cpar(bv)->next && (cpos(bv) >= cpar(bv)->Last()))
	return FINISHED;
    if (activate_inset && checkAndActivateInset(bv, false))
	return DISPATCHED;
    TEXT(bv)->CursorRight(bv, selecting);
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT
InsetText::moveLeft(BufferView * bv, bool activate_inset, bool selecting)
{
    if (!cpar(bv)->previous && (cpos(bv) <= 0))
	return FINISHED;
    TEXT(bv)->CursorLeft(bv, selecting);
    if (activate_inset && checkAndActivateInset(bv, true))
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
#ifndef NEW_INSETS
	      bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	      bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next
#else
	      bv->text->cursor.par()->previous,
	      bv->text->cursor.par()->next
#endif
	    );
    if (inset->Editable() == Inset::IS_EDITABLE) {
	UpdatableInset * i = static_cast<UpdatableInset *>(inset);
	i->setOwner(static_cast<UpdatableInset *>(this));
    }
    HideInsetCursor(bv);
    TEXT(bv)->InsertInset(bv, inset);
    TEXT(bv)->selection = 0;
    bv->fitCursor(TEXT(bv));
    UpdateLocal(bv, CURSOR_PAR, true);
    static_cast<UpdatableInset*>(inset)->Edit(bv, 0, 0, 0);
    ShowInsetCursor(bv);
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
    bv->text->SetUndo(bv->buffer(), Undo::EDIT,
#ifndef NEW_INSETS
	      bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->previous,
	      bv->text->cursor.par()->ParFromPos(bv->text->cursor.pos())->next
#else
	      bv->text->cursor.par()->previous,
	      bv->text->cursor.par()->next
#endif
	    );
    TEXT(bv)->SetFont(bv, font, toggleall);
    bv->fitCursor(TEXT(bv));
    UpdateLocal(bv, CURSOR_PAR, true);
}


bool InsetText::checkAndActivateInset(BufferView * bv, bool behind)
{
    if (cpar(bv)->GetChar(cpos(bv)) == LyXParagraph::META_INSET) {
	unsigned int x;
	unsigned int y;
	Inset * inset =
	    static_cast<UpdatableInset*>(cpar(bv)->GetInset(cpos(bv)));
	if (!inset || inset->Editable() != Inset::HIGHLY_EDITABLE)
	    return false;
	LyXFont const font =
		TEXT(bv)->GetFont(bv->buffer(), cpar(bv), cpos(bv));
	if (behind) {
	    x = inset->width(bv, font);
	    y = inset->descent(bv, font);
	} else {
	    x = y = 0;
	}
	inset_x = cx(bv) - top_x + drawTextXOffset;
	inset_y = cy(bv) + drawTextYOffset;
	inset->Edit(bv, x - inset_x, y - inset_y, 0);
	if (!the_locking_inset)
	    return false;
	UpdateLocal(bv, CURSOR_PAR, false);
	return true;
    }
    return false;
}


bool InsetText::checkAndActivateInset(BufferView * bv, int x, int y,
				      int button)
{
    x = x - drawTextXOffset;
    y = y + insetAscent;
    Inset * inset = bv->checkInsetHit(TEXT(bv), x, y, button);

    if (inset) {
	if (x < 0)
	    x = insetWidth;
	if (y < 0)
	    y = insetDescent;
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


int InsetText::getMaxWidth(Painter & pain, UpdatableInset const * inset) const
{
    int w = UpdatableInset::getMaxWidth(pain, inset);
    if (w < 0) {
	return w;
    }
    if (owner()) {
	w = w - top_x + owner()->x();
	return w;
    }
    w -= (2 * TEXT_TO_INSET_OFFSET);
    return w - top_x;
//    return  w - (2*TEXT_TO_INSET_OFFSET);
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
	if (!flag)
	    removeNewlines();
    }
}


void InsetText::SetDrawFrame(BufferView * bv, DrawFrame how)
{
    if (how != drawFrame) {
	drawFrame = how;
	if (bv)
	    UpdateLocal(bv, DRAW_FRAME, false);
    }
}


void InsetText::SetFrameColor(BufferView * bv, LColor::color col)
{
    if (frame_color != col) {
	frame_color = col;
	if (bv)
	    UpdateLocal(bv, DRAW_FRAME, false);
    }
}

#if 0
LyXFont InsetText::GetDrawFont(BufferView * bv, LyXParagraph * p, int pos) const
{
    return TEXT(bv)->GetFont(bv->buffer(), p, pos);
}
#endif


int InsetText::cx(BufferView * bv) const
{
    return TEXT(bv)->cursor.x() + top_x + TEXT_TO_INSET_OFFSET;
}


int InsetText::cy(BufferView * bv) const
{
    LyXFont font;
    return TEXT(bv)->cursor.y() - ascent(bv, font) + TEXT_TO_INSET_OFFSET;
}


LyXParagraph::size_type InsetText::cpos(BufferView * bv) const
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
    if (the_locking_inset) {
	lt->SetCursor(bv, inset_par, inset_pos);
    }
    return lt;
}


void InsetText::deleteLyXText(BufferView * bv, bool recursive) const
{
    if (cache.find(bv) == cache.end())
	return;
    delete cache[bv];
    cache.erase(bv);
    if (recursive) {
	/// then remove all LyXText in text-insets
	LyXParagraph * p = par;
	for(;p;p = p->next) {
	    p->deleteInsetsLyXText(bv);
	}
    }
}


void InsetText::resizeLyXText(BufferView * bv) const
{
    if (!par->next && !par->size()) // resize not neccessary!
	return;
    if (cache.find(bv) == cache.end())
	return;

    LyXParagraph * lpar = 0;
    LyXParagraph * selstartpar = 0;
    LyXParagraph * selendpar = 0;
    LyXParagraph::size_type pos = 0;
    LyXParagraph::size_type selstartpos = 0;
    LyXParagraph::size_type selendpos = 0;
    int selection = 0;
    int mark_set = 0;

//    ProhibitInput(bv);

    if (locked) {
	lpar = TEXT(bv)->cursor.par();
	pos = TEXT(bv)->cursor.pos();
	selstartpar = TEXT(bv)->sel_start_cursor.par();
	selstartpos = TEXT(bv)->sel_start_cursor.pos();
	selendpar = TEXT(bv)->sel_end_cursor.par();
	selendpos = TEXT(bv)->sel_end_cursor.pos();
	selection = TEXT(bv)->selection;
	mark_set = TEXT(bv)->mark_set;
    }
    deleteLyXText(bv, (the_locking_inset == 0));

    if (lpar) {
	TEXT(bv)->selection = true;
	/* at this point just to avoid the Delete-Empty-Paragraph
	 * Mechanism when setting the cursor */
	TEXT(bv)->mark_set = mark_set;
	if (selection) {
	    TEXT(bv)->SetCursor(bv, selstartpar, selstartpos);
	    TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
	    TEXT(bv)->SetCursor(bv, selendpar, selendpos);
	    TEXT(bv)->SetSelection();
	    TEXT(bv)->SetCursor(bv, lpar, pos);
	} else {
	    TEXT(bv)->SetCursor(bv, lpar, pos);
	    TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
	    TEXT(bv)->selection = false;
	}
    }
    if (bv->screen())
	    TEXT(bv)->first = bv->screen()->TopCursorVisible(TEXT(bv));
    // this will scroll the screen such that the cursor becomes visible 
    bv->updateScrollbar();
//    AllowInput(bv);
    if (the_locking_inset) {
	/// then resize all LyXText in text-insets
	inset_x = cx(bv) - top_x + drawTextXOffset;
	inset_y = cy(bv) + drawTextYOffset;
	for(LyXParagraph * p = par; p; p = p->next) {
	    p->resizeInsetsLyXText(bv);
	}
    }
    need_update = FULL;
}


void InsetText::removeNewlines()
{
    for(LyXParagraph * p = par; p; p = p->next) {
	for(int i = 0; i < p->Last(); ++i) {
	    if (p->GetChar(i) == LyXParagraph::META_NEWLINE)
		p->Erase(i);
	}
    }
}
