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
#include "lyxlex.h"
#include "debug.h"
#include "lyxfont.h"
#include "lyxlex.h"
#include "commandtags.h"
#include "buffer.h"
#include "LyXView.h"
#include "BufferView.h"
#include "support/textutils.h"
#include "layout.h"
#include "insetlatexaccent.h"
#include "insetquotes.h"
#include "mathed/formulamacro.h"
#include "figinset.h"
#include "insetinfo.h"
#include "insetinclude.h"
#include "insetbib.h"
#include "insetcommand.h"
#include "insetindex.h"
#include "insetlabel.h"
#include "insetref.h"
//#include "insettabular.h"
#include "insetert.h"
#include "insetspecialchar.h"
#include "LaTeXFeatures.h"
#include "Painter.h"
#include "lyx_gui_misc.h"
#include "support/LAssert.h"
#include "lyxtext.h"
#include "lyxcursor.h"
#include "CutAndPaste.h"
#include "font.h"

using std::ostream;
using std::ifstream;
using std::endl;
using std::min;
using std::max;

extern unsigned char getCurrentTextClass(Buffer *);


InsetText::InsetText(Buffer * buf)
{
    par = new LyXParagraph();
    init(buf);
}


InsetText::InsetText(InsetText const & ins, Buffer * buf)
{
    par = 0;
    init(buf, &ins);
    autoBreakRows = ins.autoBreakRows;
}


void InsetText::init(Buffer * buf, InsetText const * ins)
{
    the_locking_inset = 0;
    buffer = buf;
    cursor_visible = false;
    cursor.x_fix = -1;
    selection_start = selection_end = 0;
    interline_space = 1;
    no_selection = false;
    init_inset = true;
    maxAscent = maxDescent = insetWidth = 0;
    drawTextXOffset = drawTextYOffset = 0;
    autoBreakRows = false;
    xpos = 0.0;
    if (ins) {
	if (par)
	    delete par;
	par = ins->par->Clone();
	autoBreakRows = ins->autoBreakRows;
    }
    par->SetInsetOwner(this);
    cursor.par = par;
    cursor.pos = 0;
}


InsetText::~InsetText()
{
    delete par;
}


Inset * InsetText::Clone() const
{
    InsetText * t = new InsetText(*this, buffer);
    return t;
}


void InsetText::Write(ostream & os) const
{
    os << "Text\n";
    WriteParagraphData(os);
}


void InsetText::WriteParagraphData(ostream & os) const
{
    par->writeFile(os, buffer->params, 0, 0);
}


void InsetText::Read(LyXLex & lex)
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
	if (buffer->parseSingleLyXformat2Token(lex, par, return_par,
					       token, pos, depth,
					       font, footnoteflag,
					       footnotekind)) {
	    // the_end read this should NEVER happen
	    lex.printError("\\the_end read in inset! Error in document!");
	    return;
	}
    }
    if (token != "\\end_inset") {
        lex.printError("Missing \\end_inset at this point. "
                       "Read: `$$Token'");
    }
    init_inset = true;
}


int InsetText::ascent(Painter & pain, LyXFont const & font) const
{
    if (init_inset) {
	computeTextRows(pain, xpos);
	resetPos(pain);
	init_inset = false;
    }
    if (maxAscent)
	return maxAscent;
    return lyxfont::maxAscent(font);
}


int InsetText::descent(Painter & pain, LyXFont const & font) const
{
    if (init_inset) {
	computeTextRows(pain, xpos);
	resetPos(pain);
	init_inset = false;
    }
    if (maxDescent)
	return maxDescent;
    return lyxfont::maxDescent(font);
}


int InsetText::width(Painter & pain, LyXFont const &) const
{
    if (init_inset) {
	computeTextRows(pain, xpos);
	resetPos(pain);
	init_inset = false;
    }
    return insetWidth;
}


void InsetText::draw(Painter & pain, LyXFont const & f,
		     int baseline, float & x) const
{
    xpos = x;
    UpdatableInset::draw(pain, f, baseline, x);
    
    if (init_inset || (baseline != top_baseline) || (top_x != int(x))) {
	top_baseline = baseline;
	if (init_inset || (top_x != int(x))) {
	    top_x = int(x);
	    computeTextRows(pain, x);
	    init_inset = false;
	}
	computeBaselines(baseline);
    }
    if (the_locking_inset && (cursor.pos == inset_pos)) {
	resetPos(pain);
	inset_x = cursor.x - top_x + drawTextXOffset;
	inset_y = cursor.y + drawTextYOffset;
    }
    for(RowList::size_type r = 0; r < rows.size() - 1; ++r) {
        drawRowSelection(pain, rows[r].pos, rows[r + 1].pos, r, 
                         rows[r].baseline, x);
        drawRowText(pain, rows[r].pos, rows[r + 1].pos, rows[r].baseline, x);
    }
    x += insetWidth;
}


void InsetText::drawRowSelection(Painter & pain, int startpos, int endpos,
                                 int row, int baseline, float x) const
{
    if (!hasSelection())
	return;

    int s_start, s_end;
    if (selection_start > selection_end) {
	s_start = selection_end;
	s_end = selection_start;
    } else {
	s_start = selection_start;
	s_end = selection_end;
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
	font = GetDrawFont(par,p);
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
	LyXFont font = GetDrawFont(par,p);
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
    the_locking_inset = 0;
    selection_start = selection_end = inset_pos = inset_x = inset_y = 0;
//    no_selection = true;
    setPos(bv->painter(), x, y);
    checkAndActivateInset(bv, x, y, button);
    selection_start = selection_end = cursor.pos;
    current_font = real_current_font = GetFont(par, cursor.pos);
    bv->text->FinishUndo();
    UpdateLocal(bv, true);
}


void InsetText::InsetUnlock(BufferView * bv)
{
    if (the_locking_inset) {
	the_locking_inset->InsetUnlock(bv);
	the_locking_inset = 0;
    }
    lyxerr[Debug::INSETS] << "InsetText::InsetUnlock(" << this <<
	    ")" << endl;
    if (hasSelection()) {
	selection_start = selection_end = cursor.pos;
	UpdateLocal(bv, false);
    }
    no_selection = false;
}


bool InsetText::LockInsetInInset(BufferView * bv, UpdatableInset * inset)
{
    lyxerr[Debug::INSETS] << "InsetText::LockInsetInInset(" << inset << "): ";
    if (!inset)
	return false;
    if (inset == par->GetInset(cursor.pos)) {
	lyxerr[Debug::INSETS] << "OK" << endl;
	the_locking_inset = inset;
	resetPos(bv->painter());
	inset_x = cursor.x - top_x + drawTextXOffset;
	inset_y = cursor.y + drawTextYOffset;
	inset_pos = cursor.pos;
	return true;
    } else if (the_locking_inset && (the_locking_inset == inset)) {
	if (cursor.pos == inset_pos) {
	    lyxerr[Debug::INSETS] << "OK" << endl;
	    resetPos(bv->painter());
	    inset_x = cursor.x - top_x + drawTextXOffset;
	    inset_y = cursor.y + drawTextYOffset;
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
    UpdateLocal(bv, true);
    if (cursor.pos == inset_pos) {
	inset_x = cursor.x - top_x + drawTextXOffset;
	inset_y = cursor.y + drawTextYOffset;
    }
    return true;
}


void InsetText::InsetButtonPress(BufferView * bv, int x, int y, int button)
{
    if (hasSelection()) {
	selection_start = selection_end = cursor.pos;
	UpdateLocal(bv, false);
    }
    no_selection = false;
    setPos(bv->painter(), x, y);
    cursor.x_fix = -1;
    if (the_locking_inset) {
	UpdatableInset
	    *inset = 0;
	if (par->GetChar(cursor.pos) == LyXParagraph::META_INSET)
	    inset = static_cast<UpdatableInset*>(par->GetInset(cursor.pos));
	if (the_locking_inset == inset) {
	    the_locking_inset->InsetButtonPress(bv,x-inset_x,y-inset_y,button);
	    return;
	} else if (inset) {
	    // otherwise unlock the_locking_inset and lock the new inset
	    the_locking_inset->InsetUnlock(bv);
	    inset_x = cursor.x - top_x + drawTextXOffset;
	    inset_y = cursor.y + drawTextYOffset;
	    inset->InsetButtonPress(bv, x-inset_x, y-inset_y, button);
	    inset->Edit(bv, x - inset_x, y - inset_y, button);
	    UpdateLocal(bv, true);
	    return;
	}
	// otherwise only unlock the_locking_inset
	the_locking_inset->InsetUnlock(bv);
	the_locking_inset = 0;
    }
    if (bv->the_locking_inset) {
	if ((par->GetChar(cursor.pos) == LyXParagraph::META_INSET) &&
	    par->GetInset(cursor.pos) &&
	    (par->GetInset(cursor.pos)->Editable() == Inset::HIGHLY_EDITABLE)) {
	    UpdatableInset *inset =
		static_cast<UpdatableInset*>(par->GetInset(cursor.pos));
	    inset_x = cursor.x - top_x + drawTextXOffset;
	    inset_y = cursor.y + drawTextYOffset;
	    inset->InsetButtonPress(bv, x-inset_x, y-inset_y, button);
	    inset->Edit(bv, x-inset_x, y-inset_y, 0);
	    UpdateLocal(bv, true);
	}
    }
    selection_start = selection_end = cursor.pos;
}


void InsetText::InsetButtonRelease(BufferView * bv, int x, int y, int button)
{
    UpdatableInset * inset = 0;

    if (the_locking_inset) {
	    the_locking_inset->InsetButtonRelease(bv, x-inset_x, y-inset_y,button);
    } else {
	if (par->GetChar(cursor.pos) == LyXParagraph::META_INSET) {
    	    inset = static_cast<UpdatableInset*>(par->GetInset(cursor.pos));
	    if (inset->Editable()==Inset::HIGHLY_EDITABLE) {
		inset->InsetButtonRelease(bv, x-inset_x, y-inset_y,button);
	    } else {
		inset_x = cursor.x - top_x + drawTextXOffset;
		inset_y = cursor.y + drawTextYOffset;
		inset->InsetButtonRelease(bv, x-inset_x, y-inset_y,button);
		inset->Edit(bv, x-inset_x, y-inset_y, button);
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
	int old = selection_end;
	HideInsetCursor(bv);
	setPos(bv->painter(), x, y);
	selection_end = cursor.pos;
	if (old != selection_end)
	    UpdateLocal(bv, false);
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
        cursor.x_fix = -1;
    if (the_locking_inset) {
        result = the_locking_inset->LocalDispatch(bv, action, arg);
	if (result == DISPATCHED_NOUPDATE)
	    return result;
	else if (result == DISPATCHED) {
	    the_locking_inset->ToggleInsetCursor(bv);
	    UpdateLocal(bv, false);
	    the_locking_inset->ToggleInsetCursor(bv);
            return result;
        } else if (result == FINISHED) {
	    switch(action) {
	    case -1:
	    case LFUN_RIGHT:
		cursor.pos = inset_pos + 1;
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
	bv->text->SetUndo(Undo::INSERT, 
	    bv->text->cursor.par->ParFromPos(bv->text->cursor.pos)->previous,
	    bv->text->cursor.par->ParFromPos(bv->text->cursor.pos)->next);
	cutSelection();
	cursor.pos = selection_start;
	par->InsertChar(cursor.pos,arg[0]);
	SetCharFont(cursor.pos,current_font);
	++cursor.pos;
	selection_start = selection_end = cursor.pos;
	UpdateLocal(bv, true);
	break;
        // --- Cursor Movements ---------------------------------------------
    case LFUN_RIGHTSEL:
	bv->text->FinishUndo();
	moveRight(bv, false);
	selection_end = cursor.pos;
	UpdateLocal(bv, false);
	break;
    case LFUN_RIGHT:
	bv->text->FinishUndo();
	result = moveRight(bv);
	if (hasSelection()) {
	    selection_start = selection_end = cursor.pos;
	    UpdateLocal(bv, false);
	} else {
	    selection_start = selection_end = cursor.pos;
	}
	break;
    case LFUN_LEFTSEL:
	bv->text->FinishUndo();
	moveLeft(bv, false);
	selection_end = cursor.pos;
	UpdateLocal(bv, false);
	break;
    case LFUN_LEFT:
	bv->text->FinishUndo();
	result= moveLeft(bv);
	if (hasSelection()) {
		selection_start = selection_end = cursor.pos;
		UpdateLocal(bv, false);
	} else {
		selection_start = selection_end = cursor.pos;
	}
	break;
    case LFUN_DOWNSEL:
	bv->text->FinishUndo();
	moveDown(bv);
	selection_end = cursor.pos;
	UpdateLocal(bv, false);
	break;
    case LFUN_DOWN:
	bv->text->FinishUndo();
	result = moveDown(bv);
	if (hasSelection()) {
	    selection_start = selection_end = cursor.pos;
	    UpdateLocal(bv, false);
	} else {
	    selection_start = selection_end = cursor.pos;
	}
	break;
    case LFUN_UPSEL:
	bv->text->FinishUndo();
	moveUp(bv);
	selection_end = cursor.pos;
	UpdateLocal(bv, false);
	break;
    case LFUN_UP:
	bv->text->FinishUndo();
	result = moveUp(bv);
	if (hasSelection()) {
	    selection_start = selection_end = cursor.pos;
	    UpdateLocal(bv, false);
	} else {
	    selection_start = selection_end = cursor.pos;
	}
	break;
    case LFUN_BACKSPACE:
	if (!cursor.pos) { // || par->IsNewline(cursor.pos-1)) {
	    if (hasSelection()) {
		selection_start = selection_end = cursor.pos;
		UpdateLocal(bv, false);
	    }
	    break;
	}
	moveLeft(bv);
    case LFUN_DELETE:
    {
 	bv->text->SetUndo(Undo::DELETE, 
	    bv->text->cursor.par->ParFromPos(bv->text->cursor.pos)->previous,
	    bv->text->cursor.par->ParFromPos(bv->text->cursor.pos)->next);
	bool ret = true;
	if (hasSelection()) {
	    LyXParagraph::size_type i = selection_start;
	    for (; i < selection_end; ++i) {
		par->Erase(selection_start);
	    }
	} else
	    ret = Delete();
	if (ret) { // we need update
	    selection_start = selection_end = cursor.pos;
	    UpdateLocal(bv, true);
	} else if (hasSelection()) {
	    selection_start = selection_end = cursor.pos;
	    UpdateLocal(bv, false);
	}
    }
    resetPos(bv->painter());
    break;
    case LFUN_CUT:
	bv->text->SetUndo(Undo::DELETE, 
	  bv->text->cursor.par->ParFromPos(bv->text->cursor.pos)->previous,
	  bv->text->cursor.par->ParFromPos(bv->text->cursor.pos)->next);

	if (cutSelection()) {
	    // we need update
	    cursor.pos = selection_end = selection_start;
	    UpdateLocal(bv, true);
	} else if (hasSelection()) {
	    selection_start = selection_end = cursor.pos;
	    UpdateLocal(bv, false);
	}
	resetPos(bv->painter());
	break;
    case LFUN_COPY:
	bv->text->FinishUndo();
	if (copySelection()) {
	    // we need update
	    selection_start = selection_end = cursor.pos;
	    UpdateLocal(bv, true);
	} else if (hasSelection()) {
	    selection_start = selection_end = cursor.pos;
	    UpdateLocal(bv, false);
	}
	break;
    case LFUN_PASTE:
    {
	bv->text->SetUndo(Undo::INSERT, 
	  bv->text->cursor.par->ParFromPos(bv->text->cursor.pos)->previous,
	  bv->text->cursor.par->ParFromPos(bv->text->cursor.pos)->next);
	if (pasteSelection()) {
	    selection_start = selection_end = cursor.pos;
	    UpdateLocal(bv, true);
	}
    }
    resetPos(bv->painter());
    break;
    case LFUN_HOME:
	bv->text->FinishUndo();
	for(; cursor.pos > rows[actrow].pos; --cursor.pos)
	    cursor.x -= SingleWidth(bv->painter(), par, cursor.pos);
	cursor.x -= SingleWidth(bv->painter(), par, cursor.pos);
	if (hasSelection()) {
	    selection_start = selection_end = cursor.pos;
	    UpdateLocal(bv, false);
	} else {
	    selection_start = selection_end = cursor.pos;
	}
	resetPos(bv->painter());
	break;
    case LFUN_END:
    {
	bv->text->FinishUndo();
	int checkpos = (int)rows[actrow + 1].pos;
	if ((actrow + 2) < (int)rows.size())
	    --checkpos;
	for(; cursor.pos < checkpos; ++cursor.pos)
	    cursor.x += SingleWidth(bv->painter(), par, cursor.pos);
	if (hasSelection()) {
	    selection_start = selection_end = cursor.pos;
	    UpdateLocal(bv, false);
	} else {
	    selection_start = selection_end = cursor.pos;
	}
    }
    resetPos(bv->painter());
    break;
    case LFUN_MATH_MODE:
	InsertInset(bv, new InsetFormula);
	return DISPATCHED;
    case LFUN_INSET_ERT:
	InsertInset(bv, new InsetERT(buffer));
	return DISPATCHED;
    case LFUN_BREAKPARAGRAPH:
    case LFUN_BREAKLINE:
	if (!autoBreakRows)
	    return DISPATCHED;
	bv->text->SetUndo(Undo::INSERT, 
	    bv->text->cursor.par->ParFromPos(bv->text->cursor.pos)->previous,
	    bv->text->cursor.par->ParFromPos(bv->text->cursor.pos)->next);
	par->InsertChar(cursor.pos,LyXParagraph::META_NEWLINE);
	SetCharFont(cursor.pos,current_font);
	UpdateLocal(bv, true);
	++cursor.pos;
	selection_start = selection_end = cursor.pos;
	resetPos(bv->painter());
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


int InsetText::Latex(ostream & os, bool /*fragile*/, bool /*fp*/) const
{
	TexRow texrow;
	int ret = par->SimpleTeXOnePar(os, texrow);
	return ret;
}


void InsetText::Validate(LaTeXFeatures & features) const
{
    par->validate(features);
}


// Returns the width of a character at a certain spot
int InsetText::SingleWidth(Painter & pain, LyXParagraph * par, int pos) const
{
    LyXFont font = GetDrawFont(par, pos);
    char c = par->GetChar(pos);

    if (IsPrintable(c)) {
        return lyxfont::width(c, font);
    } else if (c == LyXParagraph::META_INSET) {
        Inset const * tmpinset = par->GetInset(pos);
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
void InsetText::SingleHeight(Painter & pain, LyXParagraph * par,int pos,
			     int & asc, int & desc) const
{
    LyXFont font = GetDrawFont(par, pos);
    char c = par->GetChar(pos);

    asc = desc = 0;
    if (c == LyXParagraph::META_INSET) {
        Inset const * tmpinset=par->GetInset(pos);
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
LyXFont InsetText::GetFont(LyXParagraph * par, int pos) const
{
    char par_depth = par->GetDepth();

    LyXLayout const & layout =
	    textclasslist.Style(buffer->params.textclass, par->GetLayout());

    // We specialize the 95% common case:
    if (par->footnoteflag == LyXParagraph::NO_FOOTNOTE && !par_depth) {
        if (pos >= 0) {
            // 95% goes here
            if (layout.labeltype == LABEL_MANUAL
                && pos < BeginningOfMainBody(par)) {
		// 1% goes here
                return par->GetFontSettings(pos).realize(layout.reslabelfont);
            } else
                return par->GetFontSettings(pos).realize(layout.resfont);
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
        if (pos < BeginningOfMainBody(par)) {
            // 1% goes here
            layoutfont = layout.labelfont;
        } else {
            // 99% goes here
            layoutfont = layout.font;
        }
        tmpfont = par->GetFontSettings(pos);
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
    while (par && par_depth && !tmpfont.resolved()) {
        par = par->DepthHook(par_depth - 1);
        if (par) {
            tmpfont.realize(textclasslist.Style(buffer->params.textclass,
                                                par->GetLayout()).font);
            par_depth = par->GetDepth();
        }
    }
    tmpfont.realize((textclasslist.TextClass(buffer->params.textclass).
                    defaultfont()));
    return tmpfont;
}


// the font for drawing may be different from the real font
LyXFont InsetText::GetDrawFont(LyXParagraph * par, int pos) const
{
    return GetFont(par, pos);
}


int InsetText::BeginningOfMainBody(LyXParagraph * par) const
{
    if (textclasslist.Style(buffer->params.textclass,
                       par->GetLayout()).labeltype != LABEL_MANUAL)
        return 0;
    else
        return par->BeginningOfMainBody();
}


void InsetText::GetCursorPos(int & x, int & y) const
{
    x = cursor.x;
    y = cursor.y;
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

    LyXFont font = GetDrawFont(par, cursor.pos);

    int asc = lyxfont::maxAscent(font);
    int desc = lyxfont::maxDescent(font);
  
    if (cursor_visible)
        bv->hideLockedInsetCursor();
    else
        bv->showLockedInsetCursor(cursor.x, cursor.y, asc, desc);
    cursor_visible = !cursor_visible;
}


void InsetText::ShowInsetCursor(BufferView * bv)
{
    if (the_locking_inset) {
	the_locking_inset->ShowInsetCursor(bv);
	return;
    }
    if (!cursor_visible) {
	LyXFont font = GetDrawFont(par, cursor.pos);
	
	int asc = lyxfont::maxAscent(font);
	int desc = lyxfont::maxDescent(font);
	bv->fitLockedInsetCursor(cursor.x, cursor.y, asc, desc);
	bv->showLockedInsetCursor(cursor.x, cursor.y, asc, desc);
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
    cursor.pos = actrow = 0;
    cursor.y = top_baseline;
    y += cursor.y;
    for(unsigned int i = 1;
	((cursor.y + rows[i - 1].desc) < y) && (i < rows.size() - 1); ++i) {
	cursor.y = rows[i].baseline;
	cursor.pos = rows[i].pos;
	actrow = i;
    }
    cursor.y -= top_baseline;
    cursor.x = top_x;
    x += top_x;

    int swh;
    int sw = swh = SingleWidth(pain, par,cursor.pos);
    if (par->GetChar(cursor.pos)!=LyXParagraph::META_INSET)
	swh /= 2;
    int checkpos = rows[actrow + 1].pos;
    if ((actrow+2) < (int)rows.size())
	--checkpos;
    while ((cursor.pos < checkpos) && ((cursor.x + swh) < x)) {
	cursor.x += sw;
	++cursor.pos;
	sw = swh = SingleWidth(pain, par,cursor.pos);
	if (par->GetChar(cursor.pos)!=LyXParagraph::META_INSET)
	    swh /= 2;
    }
}


void InsetText::resetPos(Painter & pain) const
{
    if (!rows.size())
	return;

    int old_pos = cursor.pos;

    cursor.y = top_baseline;
    actrow = 0;
    for(unsigned int i = 0;
	(i < (rows.size()-1)) && (rows[i].pos <= cursor.pos);
	++i) {
	cursor.y = rows[i].baseline;
	actrow = i;
    }
    cursor.y -= top_baseline;
    setPos(pain, 0, cursor.y);
    cursor.x = top_x;
    while(cursor.pos < old_pos) {
	cursor.x += SingleWidth(pain, par,cursor.pos);
	++cursor.pos;
    }
}


UpdatableInset::RESULT
InsetText::moveRight(BufferView * bv, bool activate_inset)
{
    if (cursor.pos >= par->Last())
	return FINISHED;
    if (activate_inset && checkAndActivateInset(bv)) {
	return DISPATCHED;
    }
    ++cursor.pos;
    resetPos(bv->painter());
    real_current_font = current_font = GetFont(par, cursor.pos);
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT
InsetText::moveLeft(BufferView * bv, bool activate_inset)
{
    if (cursor.pos <= 0)
	return FINISHED;
    --cursor.pos;
    resetPos(bv->painter());
    if (activate_inset)
	if (checkAndActivateInset(bv, -1, -1))
	    return DISPATCHED;
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT
InsetText::moveUp(BufferView * bv)
{
    if (!actrow)
	return FINISHED;
    cursor.y = rows[actrow - 1].baseline - top_baseline;
    if (cursor.x_fix < 0)
	cursor.x_fix = cursor.x;
    setPos(bv->painter(), cursor.x_fix-top_x+drawTextXOffset, cursor.y);
    return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT
InsetText::moveDown(BufferView * bv)
{
    if (actrow >= int(rows.size() - 2))
	return FINISHED;
    cursor.y = rows[actrow + 1].baseline - top_baseline;
    if (cursor.x_fix < 0)
	cursor.x_fix = cursor.x;
    setPos(bv->painter(), cursor.x_fix-top_x+drawTextXOffset, cursor.y);
    return DISPATCHED_NOUPDATE;
}


bool InsetText::Delete()
{
    if ((par->GetChar(cursor.pos)==LyXParagraph::META_INSET) &&
	!par->GetInset(cursor.pos)->Deletable()) {
	return false;
    }
    par->Erase(cursor.pos);
    return true;
}


bool InsetText::InsertInset(BufferView * bv, Inset * inset)
{
    bv->text->SetUndo(Undo::INSERT, 
	      bv->text->cursor.par->ParFromPos(bv->text->cursor.pos)->previous,
	      bv->text->cursor.par->ParFromPos(bv->text->cursor.pos)->next);
    if (inset->Editable() == Inset::IS_EDITABLE) {
	UpdatableInset *i = (UpdatableInset *)inset;
	i->setOwner((UpdatableInset *)this);
    }
    par->InsertChar(cursor.pos, LyXParagraph::META_INSET);
    par->InsertInset(cursor.pos, inset);
    if (hasSelection()) {
	selection_start = selection_end = cursor.pos;
    } else {
	selection_start = selection_end = cursor.pos;
    }
    UpdateLocal(bv, true);
    static_cast<UpdatableInset*>(inset)->Edit(bv, 0, 0, 0);
    return true;
}


UpdatableInset * InsetText::GetLockingInset()
{
    return the_locking_inset ? the_locking_inset->GetLockingInset() : this;
}


void InsetText::SetFont(BufferView * bv, LyXFont const & font, bool toggleall)
{
    // if there is no selection just set the current_font
    if (!hasSelection()) {
	// Determine basis font
	LyXFont layoutfont;
	if (cursor.pos < BeginningOfMainBody(par))
	    layoutfont = GetFont(par, -2);
	else
	    layoutfont = GetFont(par, -1);
	
	// Update current font
	real_current_font.update(font, bv->buffer()->params.language_info,
				 toggleall);
	
	// Reduce to implicit settings
	current_font = real_current_font;
	current_font.reduce(layoutfont);
	// And resolve it completely
	real_current_font.realize(layoutfont);
	return;
    }
    
    int s_start, s_end;
    if (selection_start > selection_end) {
	s_start = selection_end;
	s_end = selection_start;
    } else {
	s_start = selection_start;
	s_end = selection_end;
    }
    LyXFont newfont;
    while(s_start < s_end) {
	newfont = GetFont(par,s_start);
	newfont.update(font, bv->buffer()->params.language_info, toggleall);
	SetCharFont(s_start, newfont);
	++s_start;
    }
    UpdateLocal(bv, true);
}


void InsetText::SetCharFont(int pos, LyXFont const & f)
{
    /* let the insets convert their font */
	LyXFont font(f);
	
    if (par->GetChar(pos) == LyXParagraph::META_INSET) {
	if (par->GetInset(pos))
	    font = par->GetInset(pos)->ConvertFont(font);
    }
    LyXLayout const & layout =
	    textclasslist.Style(buffer->params.textclass,par->GetLayout());

    // Get concrete layout font to reduce against
    LyXFont layoutfont;

    if (pos < BeginningOfMainBody(par))
	layoutfont = layout.labelfont;
    else
	layoutfont = layout.font;


    layoutfont.realize((textclasslist.TextClass(buffer->params.textclass).
		       defaultfont()));

    // Now, reduce font against full layout font
    font.reduce(layoutfont);

    par->SetFont(pos, font);
}


void InsetText::computeTextRows(Painter & pain, float x) const
{
    int p,
	nwp = 0,
	asc = 0,
	desc = 0,
	oasc = 0,
	odesc = 0,
	owidth = 0,
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
    if (!autoBreakRows) {
	for(p = 0; p < par->Last(); ++p) {
	    insetWidth += SingleWidth(pain, par, p);
	    SingleHeight(pain, par, p, asc, desc);
	    maxAscent = max(maxAscent, asc);
	    maxDescent = max(maxDescent, desc);
	}
	rows[0].asc = maxAscent;
	rows[0].desc = maxDescent;
	// alocate a dummy row for the endpos
	row.pos = par->Last();
	rows.push_back(row);
	return;
    }

    bool is_first_word_in_row = true;
    int cw, lastWordWidth = 0;
    int maxWidth = getMaxTextWidth(pain, this, x);

    for(p = 0; p < par->Last(); ++p) {
	cw = SingleWidth(pain, par, p);
	width += cw;
	lastWordWidth += cw;
	SingleHeight(pain, par, p, asc, desc);
	wordAscent = max(wordAscent, asc);
	wordDescent = max(wordDescent, desc);
	if (par->IsNewline(p)) {
	    if (!is_first_word_in_row && (width >= maxWidth)) {
		// we have to split also the row above
		rows.back().asc = oasc;
		rows.back().desc = odesc;
		row.pos = nwp;
		rows.push_back(row);
		oasc = wordAscent;
		odesc = wordDescent;
		insetWidth = max(insetWidth, owidth);
		width = lastWordWidth;
		lastWordWidth = 0;
	    }
	    rows.back().asc = wordAscent;
	    rows.back().desc = wordDescent;
	    row.pos = ++p;
	    rows.push_back(row);
	    SingleHeight(pain, par, p, oasc, odesc);
	    insetWidth = max(insetWidth, owidth);
	    width = 0;
	    is_first_word_in_row = true;
	    wordAscent = wordDescent = lastWordWidth = 0;
	    nwp = p;
	    continue;
	}
	Inset * inset = 0;
	if (((p + 1) < par->Last()) &&
	    (par->GetChar(p + 1)==LyXParagraph::META_INSET))
	    inset = par->GetInset(p + 1);
	if (inset) {
		inset->setOwner(const_cast<InsetText*>(this)); // is this safe?
	    if (inset->display()) {
		if (!is_first_word_in_row && (width >= maxWidth)) {
		    // we have to split also the row above
		    rows.back().asc = oasc;
		    rows.back().desc = odesc;
		    row.pos = nwp;
		    rows.push_back(row);
		    oasc = wordAscent;
		    odesc = wordDescent;
		    insetWidth = max(insetWidth, owidth);
		    width = lastWordWidth;
		    lastWordWidth = 0;
		} else {
		    oasc = max(oasc, wordAscent);
		    odesc = max(odesc, wordDescent);
		}
		rows.back().asc = oasc;
		rows.back().desc = odesc;
		row.pos = ++p;
		rows.push_back(row);
		SingleHeight(pain, par, p, asc, desc);
		rows.back().asc = asc;
		rows.back().desc = desc;
		row.pos = nwp = p + 1;
		rows.push_back(row);
		oasc = odesc = width = lastWordWidth = 0;
		is_first_word_in_row = true;
		wordAscent = wordDescent = 0;
		continue;
	    }
	} else if (par->IsSeparator(p)) {
	    if (width >= maxWidth) {
		if (is_first_word_in_row) {
		    rows.back().asc = wordAscent;
		    rows.back().desc = wordDescent;
		    row.pos = p + 1;
		    rows.push_back(row);
		    oasc = odesc = width = 0;
		} else {
		    rows.back().asc = oasc;
		    rows.back().desc = odesc;
		    row.pos = nwp;
		    rows.push_back(row);
		    oasc = wordAscent;
		    odesc = wordDescent;
		    insetWidth = max(insetWidth, owidth);
		    width = lastWordWidth;
		}
		wordAscent = wordDescent = lastWordWidth = 0;
		nwp = p + 1;
		continue;
	    }
	    owidth = width;
	    oasc = max(oasc, wordAscent);
	    odesc = max(odesc, wordDescent);
	    wordAscent = wordDescent = lastWordWidth = 0;
	    nwp = p + 1;
	    is_first_word_in_row = false;
	}
    }
    // if we have some data in the paragraph we have ascent/descent
    if (p) {
	if (width >= maxWidth) {
	    // assign upper row
	    rows.back().asc = oasc;
	    rows.back().desc = odesc;
	    // assign and allocate lower row
	    row.pos = nwp;
	    rows.push_back(row);
	    rows.back().asc = wordAscent;
	    rows.back().desc = wordDescent;
	    width -= lastWordWidth;
	} else {
	    // assign last row data
	    rows.back().asc = max(oasc, wordAscent);
	    rows.back().desc = max(odesc, wordDescent);
	}
    }
    insetWidth = max(insetWidth, width);
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


void InsetText::UpdateLocal(BufferView * bv, bool flag)
{
    if (flag) {
	computeTextRows(bv->painter(), xpos);
	computeBaselines(top_baseline);
    }
    bv->updateInset(this, flag);
    if (flag)
	resetPos(bv->painter());
}


bool InsetText::cutSelection()
{
    if (!hasSelection())
	return false;

    CutAndPaste cap;

    LyXParagraph * endpar = par;
    int start, end;
    if (selection_start > selection_end) {
	    start = selection_end;
	    end = selection_start;
    } else {
	    start = selection_start;
	    end = selection_end;
    }

    return cap.cutSelection(par, &endpar, start, end,buffer->params.textclass);
}


bool InsetText::copySelection()
{
    if (!hasSelection())
	return false;

    CutAndPaste cap;

    int start, end;
    if (selection_start > selection_end) {
	    start = selection_end;
	    end = selection_start;
    } else {
	    start = selection_start;
	    end = selection_end;
    }
    return cap.copySelection(par, par, start, end, buffer->params.textclass);
}


bool InsetText::pasteSelection()
{
    CutAndPaste cap;

    if (cap.nrOfParagraphs() > 1) {
	WriteAlert(_("Impossible operation"),
		   _("Cannot include more than one paragraph!"),
		   _("Sorry."));
	return false;
    }
    LyXParagraph *endpar;
    LyXParagraph *actpar = par;

    return cap.pasteSelection(&actpar, &endpar, cursor.pos,
			      buffer->params.textclass);
}


bool InsetText::checkAndActivateInset(BufferView * bv, int x, int y,
				      int button)
{
    if (par->GetChar(cursor.pos) == LyXParagraph::META_INSET) {
	UpdatableInset * inset =
	    static_cast<UpdatableInset*>(par->GetInset(cursor.pos));
	LyXFont font = GetFont(par, cursor.pos);
	if (x < 0)
	    x = inset->width(bv->painter(), font);
	if (y < 0)
	    y = inset->descent(bv->painter(), font);
	inset_x = cursor.x - top_x + drawTextXOffset;
	inset_y = cursor.y + drawTextYOffset;
	inset->Edit(bv, x-inset_x, y-inset_y, button);
	if (!the_locking_inset)
	    return false;
	UpdateLocal(bv, true);
	return true;
    }
    return false;
}


int InsetText::getMaxTextWidth(Painter & pain, UpdatableInset const * inset,
			       int x) const
{
    return getMaxWidth(pain, inset) - x;
}
