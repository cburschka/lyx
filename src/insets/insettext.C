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
#warning BAAAAAAAADDDDDDD current_view (but Lars wanted it :) !!!
extern BufferView * current_view;


InsetText::InsetText()
{
    par = new LyXParagraph();
    init();
    text = new LyXText(this);
}


InsetText::InsetText(InsetText const & ins)
	: UpdatableInset()
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
    interline_space = 1;
    no_selection = false;
    init_inset = true;
    drawTextXOffset = drawTextYOffset = 0;
    autoBreakRows = drawLockedFrame = false;
    xpos = 0.0;
    if (ins) {
	SetParagraphData(ins->par);
	autoBreakRows = ins->autoBreakRows;
	drawLockedFrame = ins->drawLockedFrame;
    }
    par->SetInsetOwner(this);
//    selection_start_cursor = selection_end_cursor = cursor;
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
    string token, tmptok;
    int pos = 0;
    LyXParagraph * return_par = 0;
    char depth = 0; // signed or unsigned?
    LyXParagraph::footnote_flag footnoteflag = LyXParagraph::NO_FOOTNOTE;
    LyXParagraph::footnote_kind footnotekind = LyXParagraph::FOOTNOTE;
    LyXFont font(LyXFont::ALL_INHERIT);

    LyXParagraph * p;
    p = par->next;
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
 
    top_baseline = baseline;
    top_x = int(x);

    if (the_locking_inset && (cpos() == inset_pos)) {
	inset_x = cx() - top_x + drawTextXOffset;
	inset_y = cy() + drawTextYOffset;
    }
    x += TEXT_TO_INSET_OFFSET; // place for border
    long int y = 0;
    Row * row = text->GetRowNearY(y);
    y += baseline - row->ascent_of_text() + 1;
    while (row != 0) {
	text->GetVisibleRow(current_view, y, x, row, y);
	y += row->height();
	row = row->next();
    }
    x += width(pain, f) - TEXT_TO_INSET_OFFSET;
    if (drawLockedFrame && locked) {
	pain.rectangle(top_x, baseline - ascent(pain, f), width(pain, f),
		       ascent(pain,f) + descent(pain, f), frame_color);
    }
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
//    setPos(bv->painter(), x, y);
    checkAndActivateInset(bv, x, y, button);
//    selection_start_cursor = selection_end_cursor = cursor;
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
//    selection_start_cursor = selection_end_cursor = cursor;
    no_selection = false;
    locked = false;
    UpdateLocal(bv, true, false);
}


bool InsetText::LockInsetInInset(BufferView * bv, UpdatableInset * inset)
{
    lyxerr[Debug::INSETS] << "InsetText::LockInsetInInset(" << inset << "): ";
    if (!inset)
	return false;
    if (inset == par->GetInset(cpos())) {
	lyxerr[Debug::INSETS] << "OK" << endl;
	the_locking_inset = inset;
//	resetPos(bv->painter());
	inset_x = cx() - top_x + drawTextXOffset;
	inset_y = cy() + drawTextYOffset;
	inset_pos = cpos();
	return true;
    } else if (the_locking_inset && (the_locking_inset == inset)) {
	if (cpos() == inset_pos) {
	    lyxerr[Debug::INSETS] << "OK" << endl;
//	    resetPos(bv->painter());
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
    if (cpos() == inset_pos) {
	inset_x = cx() - top_x + drawTextXOffset;
	inset_y = cy() + drawTextYOffset;
    }
    return true;
}


void InsetText::InsetButtonPress(BufferView * bv, int x, int y, int button)
{
    if (text->selection) {
	text->selection = 0;
	UpdateLocal(bv, false, false);
    }
    no_selection = false;
//    setPos(bv->painter(), x, y);
//    cursor.x_fix(-1);
    text->SetCursorFromCoordinates(bv, x, y+bv->screen()->first);
    if (the_locking_inset) {
	UpdatableInset * inset = 0;
	if (par->GetChar(cpos()) == LyXParagraph::META_INSET)
	    inset = static_cast<UpdatableInset*>(par->GetInset(cpos()));
	if (the_locking_inset == inset) {
	    the_locking_inset->InsetButtonPress(bv,x-inset_x,y-inset_y,button);
	    return;
	} else if (inset) {
	    // otherwise unlock the_locking_inset and lock the new inset
	    the_locking_inset->InsetUnlock(bv);
	    inset_x = cx() - top_x + drawTextXOffset;
	    inset_y = cy() + drawTextYOffset;
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
	if ((par->GetChar(cpos()) == LyXParagraph::META_INSET) &&
	    par->GetInset(cpos()) &&
	    (par->GetInset(cpos())->Editable() == Inset::HIGHLY_EDITABLE)) {
	    UpdatableInset * inset =
		static_cast<UpdatableInset*>(par->GetInset(cpos()));
	    inset_x = cx() - top_x + drawTextXOffset;
	    inset_y = cy() + drawTextYOffset;
	    inset->InsetButtonPress(bv, x - inset_x, y - inset_y, button);
	    inset->Edit(bv, x - inset_x, y - inset_y, 0);
	    UpdateLocal(bv, true, false);
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
	if (par->GetChar(cpos()) == LyXParagraph::META_INSET) {
    	    inset = static_cast<UpdatableInset*>(par->GetInset(cpos()));
	    if (inset->Editable() == Inset::HIGHLY_EDITABLE) {
		inset->InsetButtonRelease(bv, x - inset_x, y - inset_y,button);
	    } else {
		inset_x = cx() - top_x + drawTextXOffset;
		inset_y = cy() + drawTextYOffset;
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
	    the_locking_inset->ToggleInsetCursor(bv);
	    UpdateLocal(bv, false, false);
	    the_locking_inset->ToggleInsetCursor(bv);
            return result;
        } else if (result == FINISHED) {
	    switch(action) {
	    case -1:
	    case LFUN_RIGHT:
		moveRight(bv);
//		text->cursor.pos(inset_pos + 1);
//		resetPos(bv->painter());
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
	text->selection = 0;
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
	text->selection = 0;
	text->sel_cursor = text->cursor;
	UpdateLocal(bv, false, false);
	break;
    case LFUN_DOWNSEL:
	bv->text->FinishUndo();
	moveDown(bv);
	text->SetSelection();
	UpdateLocal(bv, false, false);
	break;
    case LFUN_DOWN:
	bv->text->FinishUndo();
	result = moveDown(bv);
	text->selection = 0;
	text->sel_cursor = text->cursor;
	UpdateLocal(bv, false, false);
	break;
    case LFUN_UPSEL:
	bv->text->FinishUndo();
	moveUp(bv);
	text->SetSelection();
	UpdateLocal(bv, false, false);
	break;
    case LFUN_UP:
	bv->text->FinishUndo();
	result = moveUp(bv);
	text->selection = 0;
	text->sel_cursor = text->cursor;
	UpdateLocal(bv, false, false);
	break;
    case LFUN_HOME:
	bv->text->FinishUndo();
	text->CursorHome(bv);
	text->selection = 0;
	text->sel_cursor = text->cursor;
	UpdateLocal(bv, false, false);
	break;
    case LFUN_END:
	text->CursorEnd(bv);
	text->selection = 0;
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
	    bv->owner()->getToolbar()->combox->select(cpar()->GetLayout()+1);
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
    x = cx();
    y = cy();
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

    LyXFont font = text->GetFont(bv->buffer(), cpar(), cpos());

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
	LyXFont font = text->GetFont(bv->buffer(), cpar(), cpos());
	
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
    if ((par->GetChar(cpos())==LyXParagraph::META_INSET) &&
	!par->GetInset(cpos())->Deletable()) {
	return false;
    }
    par->Erase(cpos());
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
    par->InsertChar(cpos(), LyXParagraph::META_INSET);
    par->InsertInset(cpos(), inset);
    text->selection = 0;
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


bool InsetText::checkAndActivateInset(BufferView * bv, int x, int y,
				      int button)
{
    if (par->GetChar(cpos()) == LyXParagraph::META_INSET) {
	UpdatableInset * inset =
	    static_cast<UpdatableInset*>(par->GetInset(cpos()));
	LyXFont font = text->GetFont(bv->buffer(), cpar(), cpos());
	if (x < 0)
	    x = inset->width(bv->painter(), font);
	if (y < 0)
	    y = inset->descent(bv->painter(), font);
	inset_x = cx() - top_x + drawTextXOffset;
	inset_y = cy() + drawTextYOffset;
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
    return getMaxWidth(pain, inset) - 4; // 2+2 width of eventual border
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
    }
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

LyXFont InsetText::GetDrawFont(Buffer const * buf, LyXParagraph * par, int pos) const
{
    return text->GetFont(buf, par, pos);
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
