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
#include "LColor.h"
#include "support/textutils.h"
#include "support/LAssert.h"
#include "lyxrow.h"
#include "lyxrc.h"
#include "intl.h"
#include "trans_mgr.h"
#include "lyxscreen.h"
#include "WorkArea.h"
#include "gettext.h"
#include "lyxfunc.h"

using std::ostream;
using std::ifstream;
using std::endl;
using std::min;
using std::max;

extern unsigned char getCurrentTextClass(Buffer *);
extern bool math_insert_greek(BufferView *, char);
extern int greek_kb_flag;

InsetText::InsetText()
{
	par = new LyXParagraph;
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
	interline_space = 1;
	no_selection = false;
	need_update = INIT;
	drawTextXOffset = 0;
	drawTextYOffset = 0;
	autoBreakRows = false;
	drawFrame_ = NEVER;
	xpos = 0.0;
	if (ins) {
		SetParagraphData(ins->par);
		autoBreakRows = ins->autoBreakRows;
		drawFrame_ = ins->drawFrame_;
	}
	par->SetInsetOwner(this);
	frame_color = LColor::insetframe;
	locked = false;
	old_par = 0;
	last_drawn_width = -1;
	frame_is_visible = false;
}


InsetText::~InsetText()
{
	// delete all instances of LyXText before deleting the paragraps used
	// by it.
	for (Cache::iterator cit = cache.begin(); cit != cache.end(); ++cit) {
		delete (*cit).second;
		(*cit).second = 0;
	}

	while (par) {
		LyXParagraph * tmp = par->next();
		delete par;
		par = tmp;
	}
}


void InsetText::clear()
{
	// delete all instances of LyXText before deleting the paragraps used
	// by it.
	for (Cache::iterator cit = cache.begin(); cit != cache.end(); ++cit) {
		delete (*cit).second;
		(*cit).second = 0;
	}

	while (par) {
		LyXParagraph * tmp = par->next();
		delete par;
		par = tmp;
	}
	par = new LyXParagraph;
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
	LyXFont font(LyXFont::ALL_INHERIT);

#if 0
	// delete all instances of LyXText before deleting the paragraps used
	// by it.
	for (Cache::iterator cit = cache.begin(); cit != cache.end(); ++cit) {
		delete (*cit).second;
		(*cit).second = 0;
	}

	while (par) {
		LyXParagraph * tmp = par->next();
		delete par;
		par = tmp;
	}

	par = new LyXParagraph;
#else
	clear();
#endif
	
	while (lex.IsOK()) {
		lex.nextToken();
		token = lex.GetString();
		if (token.empty())
			continue;
		if (token == "\\end_inset")
			break;
		if (const_cast<Buffer*>(buf)->
			parseSingleLyXformat2Token(lex, par, return_par,
						   token, pos, depth, font)) {
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
		return_par = return_par->next();
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
	insetWidth = max(textWidth(bv),
			 (int)TEXT(bv)->width + (2 * TEXT_TO_INSET_OFFSET));
	return insetWidth;
}


int InsetText::textWidth(BufferView * bv) const
{
	int const w = getMaxWidth(bv, this);
	return w;
}


void InsetText::draw(BufferView * bv, LyXFont const & f,
                     int baseline, float & x, bool cleared) const
{
	if (nodraw())
		return;

	Painter & pain = bv->painter();

	// no draw is necessary !!!
	if ((drawFrame_ == LOCKED) && !locked && !par->size()) {
		top_x = int(x);
		top_baseline = baseline;
		x += width(bv, f);
		if (!cleared && (need_update & CLEAR_FRAME))
			clearFrame(pain, cleared);
		need_update = NONE;
		return;
	}

	xpos = x;
#if 0
	UpdatableInset::draw(bv, f, baseline, x, cleared);
#else
	if (!owner())
		x += static_cast<float>(scroll());
#endif
	// update insetWidth and insetHeight with dummy calls
	(void)ascent(bv, f);
	(void)descent(bv, f);
	(void)width(bv, f);

	// if top_x differs we have a rule down and we don't have to clear anything
	if (!cleared && (top_x == int(x)) &&
		((need_update&(INIT|FULL)) || (top_baseline!=baseline) ||
		 (last_drawn_width!=insetWidth)))
	{
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
		need_update |= INIT;
		top_x = int(x);
		bv->text->status = LyXText::CHANGED_IN_DRAW;
		return;
	}

//	lyxerr << "InsetText::draw[" << this << "](" << need_update << ":" << int(x) << ":" << top_x << ")\n";

	if (cleared || (last_drawn_width != insetWidth)) {
		need_update |= FULL;
		last_drawn_width = insetWidth;
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
		drawFrame(pain, cleared);
		x += width(bv, f);
		need_update = NONE;
		return;
	}
	x += TEXT_TO_INSET_OFFSET;

#ifdef WITH_WARNINGS
#warning Jürgen, why is this a block of its own? (Lgb)
#warning because you told me to define variables only in local context (Jug)!
#warning then make it a function/method of its own. (Lgb)
#endif
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
	if (cleared) {
		int yf = y_offset;
		y = 0;
		while ((row != 0) && (yf < ph)) {
			TEXT(bv)->GetVisibleRow(bv, y+y_offset, int(x), row,
						y+first, cleared);
		y += row->height();
		yf += row->height();
		row = row->next();
		}
	} else if (!locked) {
		if (need_update & CURSOR) {
			bv->screen()->ToggleSelection(TEXT(bv), bv, true, y_offset,int(x));
			TEXT(bv)->ClearSelection(bv);
			TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
		}
		bv->screen()->Update(TEXT(bv), bv, y_offset, int(x));
	} else {
		locked = false;
		if (need_update & SELECTION)
			bv->screen()->ToggleToggle(TEXT(bv), bv, y_offset, int(x));
		else if (need_update & CURSOR) {
			bv->screen()->ToggleSelection(TEXT(bv), bv, true, y_offset,int(x));
			TEXT(bv)->ClearSelection(bv);
			TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
		}
		bv->screen()->Update(TEXT(bv), bv, y_offset, int(x));
		locked = true;
	}
	}
	TEXT(bv)->refresh_y = 0;
	TEXT(bv)->status = LyXText::UNCHANGED;
	if ((need_update != CURSOR_PAR) &&
		((drawFrame_ == ALWAYS) || ((drawFrame_ == LOCKED) && locked)))
		drawFrame(pain, cleared);
	else if (need_update & CLEAR_FRAME)
		clearFrame(pain, cleared);
	x += width(bv, f) - TEXT_TO_INSET_OFFSET;
	if (bv->text->status==LyXText::CHANGED_IN_DRAW) {
		need_update |= INIT;
	} else if (need_update != INIT)
		need_update = NONE;
}


void InsetText::drawFrame(Painter & pain, bool cleared) const
{
	if (!frame_is_visible || cleared) {
		pain.rectangle(top_x + 1, top_baseline - insetAscent + 1,
		               insetWidth - 1, insetAscent + insetDescent - 1,
		               frame_color);
		frame_is_visible = true;
	}
}


void InsetText::clearFrame(Painter & pain, bool cleared) const
{
	if (frame_is_visible) {
		if (!cleared) {
			pain.rectangle(top_x + 1, top_baseline - insetAscent + 1,
			               insetWidth - 1, insetAscent + insetDescent - 1,
			               LColor::background);
		}
		frame_is_visible = false;
	}
}


void InsetText::update(BufferView * bv, LyXFont const & font, bool reinit)
{
	if (reinit) {
		need_update |= INIT;
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
#if 0
	if (need_update == INIT) {
		resizeLyXText(bv);
		need_update |= FULL;
	}
#endif
	int oldw = insetWidth;
	insetWidth = TEXT(bv)->width + (2 * TEXT_TO_INSET_OFFSET);
	if (oldw != insetWidth) {
		resizeLyXText(bv);
		need_update |= FULL;
//		update(bv, font, reinit);
		return;
	}
	if ((need_update&CURSOR_PAR) && (TEXT(bv)->status==LyXText::UNCHANGED) &&
		the_locking_inset)
	{
		TEXT(bv)->UpdateInset(bv, the_locking_inset);
	}
	
	if (TEXT(bv)->status == LyXText::NEED_MORE_REFRESH)
		need_update |= FULL;
#if 0
	int y_temp = 0;
	Row * row = TEXT(bv)->GetRowNearY(y_temp);
	insetAscent = row->ascent_of_text() + TEXT_TO_INSET_OFFSET;
	insetDescent = TEXT(bv)->height - row->ascent_of_text() +
		TEXT_TO_INSET_OFFSET;
#endif
}


void InsetText::SetUpdateStatus(BufferView * bv, int what) const
{
	need_update |= what;
	if (TEXT(bv)->status == LyXText::NEED_MORE_REFRESH)
		need_update |= FULL;
	else if (TEXT(bv)->status == LyXText::NEED_VERY_LITTLE_REFRESH)
		need_update |= CURSOR_PAR;

	// this to not draw a selection when we redraw all of it!
	if ((need_update & (INIT|FULL)) && (need_update & CURSOR))
		TEXT(bv)->ClearSelection(bv);
}


void InsetText::UpdateLocal(BufferView * bv, int what, bool mark_dirty)
{
	TEXT(bv)->FullRebreak(bv);
	SetUpdateStatus(bv, what);
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
	inset_boundary = false;
	inset_par = 0;
	old_par = 0;
	int tmp_y = (y < 0)?0:y;
	if (!checkAndActivateInset(bv, x, tmp_y, button))
		TEXT(bv)->SetCursorFromCoordinates(bv, x - drawTextXOffset,
						   y + insetAscent);
	TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
	bv->text->FinishUndo();
	ShowInsetCursor(bv);
	UpdateLocal(bv, CURSOR, false);

	// If the inset is empty set the language of the current font to the
	// language to the surronding text.
	if (par->size() == 0 && !par->next()) {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(bv->getParentLanguage(this));
		SetFont(bv, font, false);
	}
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
	UpdateLocal(bv, CLEAR_FRAME|CURSOR, false);
	if (owner())
		bv->owner()->setLayout(owner()->getLyXText(bv)
				       ->cursor.par()->GetLayout());
	else
		bv->owner()->setLayout(bv->text->cursor.par()->GetLayout());
}


bool InsetText::LockInsetInInset(BufferView * bv, UpdatableInset * inset)
{
	lyxerr[Debug::INSETS] << "InsetText::LockInsetInInset("
			      << inset << "): ";
	if (!inset)
		return false;
	if (inset == cpar(bv)->GetInset(cpos(bv))) {
		lyxerr[Debug::INSETS] << "OK" << endl;
		the_locking_inset = inset;
		inset_x = cx(bv) - top_x + drawTextXOffset;
		inset_y = cy(bv) + drawTextYOffset;
		inset_pos = cpos(bv);
		inset_par = cpar(bv);
		inset_boundary = cboundary(bv);
#if 0
		TEXT(bv)->ClearSelection(bv);
		TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
		TEXT(bv)->UpdateInset(bv, the_locking_inset);
#else
		UpdateLocal(bv, CURSOR, false);
#endif
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
		if (scroll())
			scroll(bv, 0.0F);
		else
			UpdateLocal(bv, CURSOR, false);
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
		SetUpdateStatus(bv, CURSOR_PAR);
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
	no_selection = true;

	int tmp_x = x - drawTextXOffset;
	int tmp_y = y + insetAscent - TEXT(bv)->first;
	Inset * inset = bv->checkInsetHit(TEXT(bv), tmp_x, tmp_y, button);

	HideInsetCursor(bv);
	if (the_locking_inset) {
		if (the_locking_inset == inset) {
			the_locking_inset->InsetButtonPress(bv,x-inset_x,y-inset_y,button);
			no_selection = false;
			return;
		} else if (inset) {
			// otherwise unlock the_locking_inset and lock the new inset
			the_locking_inset->InsetUnlock(bv);
			inset_x = cx(bv) - top_x + drawTextXOffset;
			inset_y = cy(bv) + drawTextYOffset;
			the_locking_inset = static_cast<UpdatableInset*>(inset);
			inset->InsetButtonPress(bv, x - inset_x, y - inset_y, button);
			inset->Edit(bv, x - inset_x, y - inset_y, button);
			if (the_locking_inset)
				UpdateLocal(bv, CURSOR, false);
			no_selection = false;
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
			inset_boundary = cboundary(bv);
			the_locking_inset = uinset;
			uinset->InsetButtonPress(bv, x - inset_x, y - inset_y,
						 button);
			uinset->Edit(bv, x - inset_x, y - inset_y, 0);
			if (the_locking_inset)
				UpdateLocal(bv, CURSOR, false);
			no_selection = false;
			return;
		}
	}
	if (!inset) { // && (button == 2)) {
		bool paste_internally = false;
		if ((button == 2) && TEXT(bv)->selection) {
			LocalDispatch(bv, LFUN_COPY, "");
			paste_internally = true;
		}
		TEXT(bv)->SetCursorFromCoordinates(bv, x-drawTextXOffset,
						   y + insetAscent);
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
				LocalDispatch(bv, LFUN_PASTESELECTION,
					      "paragraph");
		}
	}
	ShowInsetCursor(bv);
	no_selection = false;
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
				inset->InsetButtonRelease(bv,
							  x - inset_x,
							  y - inset_y, button);
			} else {
				inset_x = cx(bv) - top_x + drawTextXOffset;
				inset_y = cy(bv) + drawTextYOffset;
				inset->InsetButtonRelease(bv,
							  x - inset_x,
							  y - inset_y, button);
				inset->Edit(bv,
					    x - inset_x, y - inset_y, button);
			}
			UpdateLocal(bv, CURSOR_PAR, false);
		}
	}
	no_selection = false;
}


void InsetText::InsetMotionNotify(BufferView * bv, int x, int y, int state)
{
	if (no_selection)
		return;
	if (the_locking_inset) {
		the_locking_inset->InsetMotionNotify(bv, x - inset_x,
						     y - inset_y,state);
		return;
	}
	HideInsetCursor(bv);
	TEXT(bv)->SetCursorFromCoordinates(bv, x - drawTextXOffset,
					   y + insetAscent);
	TEXT(bv)->SetSelection(bv);
	if (TEXT(bv)->toggle_cursor.par()!=TEXT(bv)->toggle_end_cursor.par() ||
		TEXT(bv)->toggle_cursor.pos()!=TEXT(bv)->toggle_end_cursor.pos())
		UpdateLocal(bv, SELECTION, false);
	ShowInsetCursor(bv);
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
			 kb_action action, string const & arg)
{
	no_selection = false;
	UpdatableInset::RESULT
		result= UpdatableInset::LocalDispatch(bv, action, arg);
	if (result != UNDISPATCHED) {
		return DISPATCHED;
	}

	result = DISPATCHED;
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
			bool dispatched = false;
			switch (action) {
			case LFUN_UNKNOWN_ACTION:
			case LFUN_BREAKPARAGRAPH:
			case LFUN_BREAKLINE:
				moveRightIntern(bv, false, false);
				break;
			case LFUN_RIGHT:
				if (!TEXT(bv)->cursor.par()->isRightToLeftPar(bv->buffer()->params))
					moveRightIntern(bv, false, false);
				dispatched = true;
				break;
			case LFUN_LEFT:
				if (TEXT(bv)->cursor.par()->isRightToLeftPar(bv->buffer()->params))
					moveRightIntern(bv, false, false);
				dispatched = true;
				break;
			default:
				break;
			}
			the_locking_inset = 0;
			if (dispatched)
				return DISPATCHED;
		}
	}
	HideInsetCursor(bv);
	switch (action) {
	// Normal chars
	case LFUN_UNKNOWN_ACTION:
		if (bv->buffer()->isReadonly()) {
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
					  bv->text->cursor.par()->previous(),
					  bv->text->cursor.par()->next());
			bv->setState();
			if (lyxrc.auto_region_delete) {
				if (TEXT(bv)->selection){
					TEXT(bv)->CutSelection(bv, false);
				}
			}
			TEXT(bv)->ClearSelection(bv);
			for (string::size_type i = 0; i < arg.length(); ++i) {
				if (greek_kb_flag) {
					if (!math_insert_greek(bv, arg[i])) {
						bv->owner()->getIntl()->getTrans().TranslateAndInsert(arg[i], TEXT(bv));
					} else if (!the_locking_inset) {
						(void)moveRight(bv, false);
					}
				} else {
					bv->owner()->getIntl()->getTrans().TranslateAndInsert(arg[i], TEXT(bv));
				}
			}
		}
		TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
		UpdateLocal(bv, CURSOR_PAR, true);
		result=DISPATCHED_NOUPDATE;
		break;
		// --- Cursor Movements -----------------------------------
	case LFUN_RIGHTSEL:
		bv->text->FinishUndo();
		moveRight(bv, false, true);
		TEXT(bv)->SetSelection(bv);
		UpdateLocal(bv, SELECTION, false);
		break;
	case LFUN_RIGHT:
		result = moveRight(bv);
		bv->text->FinishUndo();
		UpdateLocal(bv, CURSOR, false);
		break;
	case LFUN_LEFTSEL:
		bv->text->FinishUndo();
		moveLeft(bv, false, true);
		TEXT(bv)->SetSelection(bv);
		UpdateLocal(bv, SELECTION, false);
		break;
	case LFUN_LEFT:
		bv->text->FinishUndo();
		result = moveLeft(bv);
		UpdateLocal(bv, CURSOR, false);
		break;
	case LFUN_DOWNSEL:
		bv->text->FinishUndo();
		moveDown(bv);
		TEXT(bv)->SetSelection(bv);
		UpdateLocal(bv, SELECTION, false);
		break;
	case LFUN_DOWN:
		bv->text->FinishUndo();
		result = moveDown(bv);
		UpdateLocal(bv, CURSOR, false);
		break;
	case LFUN_UPSEL:
		bv->text->FinishUndo();
		moveUp(bv);
		TEXT(bv)->SetSelection(bv);
		UpdateLocal(bv, SELECTION, false);
		break;
	case LFUN_UP:
		bv->text->FinishUndo();
		result = moveUp(bv);
		UpdateLocal(bv, CURSOR, false);
		break;
	case LFUN_HOME:
		bv->text->FinishUndo();
		TEXT(bv)->CursorHome(bv);
		UpdateLocal(bv, CURSOR, false);
		break;
	case LFUN_END:
		TEXT(bv)->CursorEnd(bv);
		UpdateLocal(bv, CURSOR, false);
		break;
	case LFUN_BACKSPACE:
		bv->text->SetUndo(bv->buffer(), Undo::DELETE,
				  bv->text->cursor.par()->previous(),
				  bv->text->cursor.par()->next());
		if (TEXT(bv)->selection)
			TEXT(bv)->CutSelection(bv);
		else
			TEXT(bv)->Backspace(bv);
		UpdateLocal(bv, CURSOR_PAR, true);
		break;
	case LFUN_DELETE:
		bv->text->SetUndo(bv->buffer(), Undo::DELETE,
				  bv->text->cursor.par()->previous(),
				  bv->text->cursor.par()->next());
		if (TEXT(bv)->selection)
			TEXT(bv)->CutSelection(bv);
		else
			TEXT(bv)->Delete(bv);
		UpdateLocal(bv, CURSOR_PAR, true);
		break;
	case LFUN_CUT:
		bv->text->SetUndo(bv->buffer(), Undo::DELETE,
				  bv->text->cursor.par()->previous(),
				  bv->text->cursor.par()->next());
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
		string clip(bv->getClipboard());
	
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
				  bv->text->cursor.par()->previous(),
				  bv->text->cursor.par()->next());
		TEXT(bv)->PasteSelection(bv);
		UpdateLocal(bv, CURSOR_PAR, true);
		break;
	case LFUN_BREAKPARAGRAPH:
		if (!autoBreakRows)
			return DISPATCHED;
		TEXT(bv)->BreakParagraph(bv, 0);
		UpdateLocal(bv, FULL, true);
		break;
	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
		if (!autoBreakRows)
			return DISPATCHED;
		TEXT(bv)->BreakParagraph(bv, 1);
		UpdateLocal(bv, FULL, true);
		break;
	case LFUN_BREAKLINE:
		if (!autoBreakRows)
			return DISPATCHED;
		bv->text->SetUndo(bv->buffer(), Undo::INSERT,
				  bv->text->cursor.par()->previous(),
				  bv->text->cursor.par()->next());
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
				string const msg = string(N_("Layout ")) + arg + N_(" not known");
				bv->owner()->getLyXFunc()->Dispatch(LFUN_MESSAGE, msg);
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
		Spacing::Space cur_spacing = par->params.spacing().getSpace();
		float cur_value = 1.0;
		if (cur_spacing == Spacing::Other) {
			cur_value = par->params.spacing().getValue();
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
			par->params.spacing(Spacing(new_spacing, new_value));
			//TEXT(bv)->RedoParagraph(owner->view());
			UpdateLocal(bv, CURSOR_PAR, true);
			//bv->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		}
	}
	break;
	
	default:
		if (!bv->Dispatch(action, arg))
			result = UNDISPATCHED;
		break;
	}

	/// If the action has deleted all text in the inset, we need to change the
	// language to the language to the surronding text.
	if (par->size() == 0 && !par->next()) {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(bv->getParentLanguage(this));
		SetFont(bv, font, false);
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
		p = p->next();
	}
	return lines;
}


int InsetText::DocBook(Buffer const * buf, ostream & os) const
{
	LyXParagraph * p = par;
	unsigned int lines = 0;
	int desc=0;
	
	string tmp;
	while (p) {
		buf->SimpleDocBookOnePar(os,tmp,p,desc,0);
		p = p->next();
	}
	
	return lines;
}


void InsetText::Validate(LaTeXFeatures & features) const
{
	LyXParagraph * p = par;
	while(p) {
		p->validate(features);
		p = p->next();
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

	LyXFont const font(TEXT(bv)->GetFont(bv->buffer(), cpar(bv), cpos(bv)));

	int const asc = lyxfont::maxAscent(font);
	int const desc = lyxfont::maxDescent(font);
  
	if (isCursorVisible())
		bv->hideLockedInsetCursor();
	else
		bv->showLockedInsetCursor(cx(bv), cy(bv), asc, desc);
	toggleCursorVisible();
}


void InsetText::ShowInsetCursor(BufferView * bv, bool show)
{
	if (the_locking_inset) {
		the_locking_inset->ShowInsetCursor(bv);
		return;
	}
	if (!isCursorVisible()) {
		LyXFont const font =
			TEXT(bv)->GetFont(bv->buffer(), cpar(bv), cpos(bv));
	
		int const asc = lyxfont::maxAscent(font);
		int const desc = lyxfont::maxDescent(font);

		bv->fitLockedInsetCursor(cx(bv), cy(bv), asc, desc);
		if (show)
			bv->showLockedInsetCursor(cx(bv), cy(bv), asc, desc);
		setCursorVisible(true);
	}
}


void InsetText::HideInsetCursor(BufferView * bv)
{
	if (isCursorVisible()) {
		bv->hideLockedInsetCursor();
		setCursorVisible(false);
	}
	if (the_locking_inset)
		the_locking_inset->HideInsetCursor(bv);
}


UpdatableInset::RESULT
InsetText::moveRight(BufferView * bv, bool activate_inset, bool selecting)
{
	if (TEXT(bv)->cursor.par()->isRightToLeftPar(bv->buffer()->params))
		return moveLeftIntern(bv, false, activate_inset, selecting);
	else
		return moveRightIntern(bv, false, activate_inset, selecting);
}

UpdatableInset::RESULT
InsetText::moveLeft(BufferView * bv, bool activate_inset, bool selecting)
{
	if (TEXT(bv)->cursor.par()->isRightToLeftPar(bv->buffer()->params))
		return moveRightIntern(bv, true, activate_inset, selecting);
	else
		return moveLeftIntern(bv, true, activate_inset, selecting);
}


UpdatableInset::RESULT
InsetText::moveRightIntern(BufferView * bv, bool behind, 
			   bool activate_inset, bool selecting)
{
	if (!cpar(bv)->next() && (cpos(bv) >= cpar(bv)->size()))
		return FINISHED;
	if (activate_inset && checkAndActivateInset(bv, behind))
		return DISPATCHED;
	TEXT(bv)->CursorRight(bv);
	if (!selecting)
		TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
	return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT
InsetText::moveLeftIntern(BufferView * bv, bool behind,
			  bool activate_inset, bool selecting)
{
	if (!cpar(bv)->previous() && (cpos(bv) <= 0))
		return FINISHED;
	TEXT(bv)->CursorLeft(bv);
	if (!selecting)
		TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
	if (activate_inset && checkAndActivateInset(bv, behind))
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
			  bv->text->cursor.par()->previous(),
			  bv->text->cursor.par()->next());
	inset->setOwner(this);
	HideInsetCursor(bv);
	TEXT(bv)->InsertInset(bv, inset);
#if 0
	if ((cpar(bv)->GetChar(cpos(bv)) != LyXParagraph::META_INSET) ||
		(cpar(bv)->GetInset(cpos(bv)) != inset))
		TEXT(bv)->CursorLeft(bv);
#endif
	bv->fitCursor(TEXT(bv));
	UpdateLocal(bv, CURSOR_PAR|CURSOR, true);
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


bool InsetText::ShowInsetDialog(BufferView * bv) const
{
	if (the_locking_inset)
		return the_locking_inset->ShowInsetDialog(bv);
	return false;
}


std::vector<string> const InsetText::getLabelList() const 
{
	std::vector<string> label_list;

	LyXParagraph * tpar = par;
	while (tpar) {
		LyXParagraph::inset_iterator beg = tpar->inset_iterator_begin();
		LyXParagraph::inset_iterator end = tpar->inset_iterator_end();
		for (; beg != end; ++beg) {
			std::vector<string> const l = (*beg)->getLabelList();
			label_list.insert(label_list.end(), l.begin(), l.end());
		}
		tpar = tpar->next();
	}
	return label_list;
}


void InsetText::SetFont(BufferView * bv, LyXFont const & font, bool toggleall)
{
	if (TEXT(bv)->selection) {
		bv->text->SetUndo(bv->buffer(), Undo::EDIT,
				  bv->text->cursor.par()->previous(),
				  bv->text->cursor.par()->next());
	}
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
			y = font.isRightToLeft() ? 0 : inset->descent(bv, font);
		} else {
			x = 0;
			y = font.isRightToLeft() ? inset->descent(bv, font) : 0;
		}
		//inset_x = cx(bv) - top_x + drawTextXOffset;
		//inset_y = cy(bv) + drawTextYOffset;
		inset->Edit(bv, x, y, 0);
		if (!the_locking_inset)
			return false;
		UpdateLocal(bv, CURSOR, false);
		return true;
	}
	return false;
}


bool InsetText::checkAndActivateInset(BufferView * bv, int x, int y,
				      int button)
{
	int dummyx, dummyy;

	dummyx = x = x - drawTextXOffset;
	dummyy = y + insetAscent;
	Inset * inset = bv->checkInsetHit(TEXT(bv), dummyx, dummyy, button);

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
		UpdateLocal(bv, CURSOR, false);
		return true;
	}
	return false;
}


int InsetText::getMaxWidth(BufferView * bv, UpdatableInset const * inset) const
{
	int w = UpdatableInset::getMaxWidth(bv, inset);
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


void InsetText::SetParagraphData(LyXParagraph * p)
{
	// delete all instances of LyXText before deleting the paragraps used
	// by it.
	for (Cache::iterator cit = cache.begin(); cit != cache.end(); ++cit){
		delete (*cit).second;
		(*cit).second = 0;
	}

	while (par) {
		LyXParagraph * tmp = par->next();
		delete par;
		par = tmp;
	}

#if 0
	par = p->Clone();
#else
	par = new LyXParagraph(*p);
#endif
	par->SetInsetOwner(this);
	LyXParagraph * np = par;
	while (p->next()) {
		p = p->next();
#if 0
		np->next(p->Clone());
#else
		np->next(new LyXParagraph(*p));
#endif
		np->next()->previous(np);
		np = np->next();
		np->SetInsetOwner(this);
	}

	need_update = INIT;
}


void InsetText::SetText(string const & data)
{
	clear();
	LyXFont font(LyXFont::ALL_SANE);
	for (unsigned int i=0; i < data.length(); ++i)
		par->InsertChar(i, data[i], font);
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
	if (how != drawFrame_) {
		drawFrame_ = how;
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


int InsetText::cx(BufferView * bv) const
{
	LyXText * text = TEXT(bv);
	int x = text->cursor.x() + top_x + TEXT_TO_INSET_OFFSET;
	if (the_locking_inset) {
		LyXFont font = text->GetFont(bv->buffer(),
					     text->cursor.par(),
					     text->cursor.pos());
		if (font.isVisibleRightToLeft())
			x -= the_locking_inset->width(bv, font);
	}
	return x;
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

bool InsetText::cboundary(BufferView * bv) const
{
	return TEXT(bv)->cursor.boundary();
}


Row * InsetText::crow(BufferView * bv) const
{
	return TEXT(bv)->cursor.row();
}


LyXText * InsetText::getLyXText(BufferView const * lbv,
				bool const recursive) const
{
	// Super UGLY! (Lgb)
	BufferView * bv = const_cast<BufferView *>(lbv);
	
	if ((cache.find(bv) != cache.end()) && cache[bv]) {
		if (recursive && the_locking_inset)
			return the_locking_inset->getLyXText(bv);
		return cache[bv];
	}
	LyXText * lt = new LyXText(const_cast<InsetText *>(this));
	lt->init(bv);
	cache[bv] = lt;
	if (the_locking_inset) {
		lt->SetCursor(bv, inset_par, inset_pos, true, inset_boundary);
		if (recursive)
			return the_locking_inset->getLyXText(bv);
	}
	return lt;
}


void InsetText::deleteLyXText(BufferView * bv, bool recursive) const
{
	if ((cache.find(bv) == cache.end()) || !cache[bv])
		return;
	delete cache[bv];
	cache.erase(bv);
	if (recursive) {
		/// then remove all LyXText in text-insets
		LyXParagraph * p = par;
		for (; p; p = p->next()) {
			p->deleteInsetsLyXText(bv);
		}
	}
}


void InsetText::resizeLyXText(BufferView * bv) const
{
	if (!par->next() && !par->size()) // resize not neccessary!
		return;
	if ((cache.find(bv) == cache.end()) || !cache[bv])
		return;

	LyXParagraph * lpar = 0;
	LyXParagraph * selstartpar = 0;
	LyXParagraph * selendpar = 0;
	LyXParagraph::size_type pos = 0;
	LyXParagraph::size_type selstartpos = 0;
	LyXParagraph::size_type selendpos = 0;
	bool boundary = false;
	bool selstartboundary = false;
	bool selendboundary = false;
	int selection = 0;
	int mark_set = 0;

//    ProhibitInput(bv);

	if (locked) {
		lpar = TEXT(bv)->cursor.par();
		pos = TEXT(bv)->cursor.pos();
		boundary = TEXT(bv)->cursor.boundary();
		selstartpar = TEXT(bv)->sel_start_cursor.par();
		selstartpos = TEXT(bv)->sel_start_cursor.pos();
		selstartboundary = TEXT(bv)->sel_start_cursor.boundary();
		selendpar = TEXT(bv)->sel_end_cursor.par();
		selendpos = TEXT(bv)->sel_end_cursor.pos();
		selendboundary = TEXT(bv)->sel_end_cursor.boundary();
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
			TEXT(bv)->SetCursor(bv, selstartpar, selstartpos,
					    true, selstartboundary);
			TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
			TEXT(bv)->SetCursor(bv, selendpar, selendpos,
					    true, selendboundary);
			TEXT(bv)->SetSelection(bv);
			TEXT(bv)->SetCursor(bv, lpar, pos);
		} else {
			TEXT(bv)->SetCursor(bv, lpar, pos, true, boundary);
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
		for (LyXParagraph * p = par; p; p = p->next()) {
			p->resizeInsetsLyXText(bv);
		}
	}
	need_update = FULL;
}


void InsetText::removeNewlines()
{
	for (LyXParagraph * p = par; p; p = p->next()) {
		for (int i = 0; i < p->size(); ++i) {
			if (p->GetChar(i) == LyXParagraph::META_NEWLINE)
				p->Erase(i);
		}
	}
}

bool InsetText::nodraw() const
{
	if (the_locking_inset)
		return the_locking_inset->nodraw();
	return UpdatableInset::nodraw();
}

int InsetText::scroll(bool recursive) const
{
	int sx = UpdatableInset::scroll(false);

	if (recursive && the_locking_inset)
		sx += the_locking_inset->scroll(recursive);

	return sx;
}

bool InsetText::doClearArea() const
{
	return !locked || (need_update & (FULL|INIT));
}
/* Emacs:
 * Local variables:
 * tab-width: 4
 * End:
 * vi:set tabstop=4:
 */
