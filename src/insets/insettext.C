/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1998-2001 The LyX Team.
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
#include "paragraph.h"
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
#include "ParagraphParameters.h"

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
	par = new Paragraph;
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
	old_max_width = 0;
	no_selection = false;
	need_update = INIT;
	drawTextXOffset = 0;
	drawTextYOffset = 0;
	autoBreakRows = false;
	drawFrame_ = NEVER;
	xpos = 0.0;
	if (ins) {
		setParagraphData(ins->par);
		autoBreakRows = ins->autoBreakRows;
		drawFrame_ = ins->drawFrame_;
	}
	par->setInsetOwner(this);
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
		Paragraph * tmp = par->next();
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
		Paragraph * tmp = par->next();
		delete par;
		par = tmp;
	}
	par = new Paragraph;
}


Inset * InsetText::clone(Buffer const &) const
{
	InsetText * t = new InsetText(*this);
	return t;
}


void InsetText::write(Buffer const * buf, ostream & os) const
{
	os << "Text\n";
	writeParagraphData(buf, os);
}


void InsetText::writeParagraphData(Buffer const * buf, ostream & os) const
{
	par->writeFile(buf, os, buf->params, 0);
}


void InsetText::read(Buffer const * buf, LyXLex & lex)
{
	string token;
	int pos = 0;
	Paragraph * return_par = 0;
	Paragraph::depth_type depth = 0; 
	LyXFont font(LyXFont::ALL_INHERIT);

	clear();
	
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
		return_par->setInsetOwner(this);
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
	Row * row = TEXT(bv)->getRowNearY(y_temp);
	insetAscent = row->ascent_of_text() + TEXT_TO_INSET_OFFSET;
	return insetAscent;
}


int InsetText::descent(BufferView * bv, LyXFont const &) const
{
	int y_temp = 0;
	Row * row = TEXT(bv)->getRowNearY(y_temp);
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
		else if (cleared)
			frame_is_visible = false;
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
#if 0
	// update insetWidth and insetHeight with dummy calls
	(void)ascent(bv, f);
	(void)descent(bv, f);
	(void)width(bv, f);
#endif

	// if top_x differs we have a rule down and we don't have to clear anything
	if (!cleared && (top_x == int(x)) &&
		((need_update&(INIT|FULL)) || (top_baseline!=baseline) ||
		 (last_drawn_width!=insetWidth)))
	{
		clearInset(pain, baseline, cleared);
	}
	if (cleared)
		frame_is_visible = false;

	if (!cleared && (need_update == NONE))
		return;

	if (top_x != int(x)) {
		if ((getMaxWidth(bv, this) > 0) &&
			(TEXT(bv)->width != old_max_width))
		{
			resizeLyXText(bv);
			need_update |= FULL;
			old_max_width = TEXT(bv)->width;
			bv->text->status = LyXText::CHANGED_IN_DRAW;
		}
		top_x = int(x);
#if 1
		clearInset(pain, baseline, cleared);
#else
		return;
#endif
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
	if (!cleared && (need_update == CURSOR) && !TEXT(bv)->selection.set()) {
		drawFrame(pain, cleared);
		x += last_width; // was width(bv, f);
		need_update = NONE;
		return;
	}
	x += TEXT_TO_INSET_OFFSET;

	int y = 0;
	Row * row = TEXT(bv)->getRowNearY(y);
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
			TEXT(bv)->getVisibleRow(bv, y+y_offset, int(x), row,
			                        y+first, cleared);
			y += row->height();
			yf += row->height();
			row = row->next();
		}
	} else if (!locked) {
		if (need_update & CURSOR) {
			bv->screen()->ToggleSelection(TEXT(bv), bv, true, y_offset,int(x));
			TEXT(bv)->clearSelection(bv);
			TEXT(bv)->selection.cursor = TEXT(bv)->cursor;
		}
		bv->screen()->Update(TEXT(bv), bv, y_offset, int(x));
	} else {
		locked = false;
		if (need_update & SELECTION)
			bv->screen()->ToggleToggle(TEXT(bv), bv, y_offset, int(x));
		else if (need_update & CURSOR) {
			bv->screen()->ToggleSelection(TEXT(bv), bv, true, y_offset,int(x));
			TEXT(bv)->clearSelection(bv);
			TEXT(bv)->selection.cursor = TEXT(bv)->cursor;
		}
		bv->screen()->Update(TEXT(bv), bv, y_offset, int(x));
		locked = true;
	}

	TEXT(bv)->refresh_y = 0;
	TEXT(bv)->status = LyXText::UNCHANGED;
	if ((need_update != CURSOR_PAR) &&
		((drawFrame_ == ALWAYS) || ((drawFrame_ == LOCKED) && locked)))
		drawFrame(pain, cleared);
	else if (need_update & CLEAR_FRAME)
		clearFrame(pain, cleared);
	x += last_width /* was width(bv, f) */ - TEXT_TO_INSET_OFFSET;
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
#if 0
	int ww = TEXT(bv)->width;
	TEXT(bv)->BreakParagraph(bv);
	if (ww != TEXT(bv)->width)
		reinit = true;
#endif
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
		TEXT(bv)->updateInset(bv, the_locking_inset);
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


void InsetText::setUpdateStatus(BufferView * bv, int what) const
{
	need_update |= what;
	if (TEXT(bv)->status == LyXText::NEED_MORE_REFRESH)
		need_update |= FULL;
	else if (TEXT(bv)->status == LyXText::NEED_VERY_LITTLE_REFRESH)
		need_update |= CURSOR_PAR;

	// this to not draw a selection when we redraw all of it!
	if ((need_update & (INIT|FULL)) && (need_update & CURSOR))
		TEXT(bv)->clearSelection(bv);
}


void InsetText::updateLocal(BufferView * bv, int what, bool mark_dirty)
{
	TEXT(bv)->fullRebreak(bv);
	setUpdateStatus(bv, what);
	if ((need_update != CURSOR) || (TEXT(bv)->status != LyXText::UNCHANGED) ||
		TEXT(bv)->selection.set())
		bv->updateInset(this, mark_dirty);
	bv->owner()->showState();
	if (old_par != cpar(bv)) {
		bv->owner()->setLayout(cpar(bv)->getLayout());
		old_par = cpar(bv);
	}
}


string const InsetText::editMessage() const
{
	return _("Opened Text Inset");
}


void InsetText::edit(BufferView * bv, int x, int y, unsigned int button)
{
//    par->SetInsetOwner(this);
	UpdatableInset::edit(bv, x, y, button);

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
		TEXT(bv)->setCursorFromCoordinates(bv, x - drawTextXOffset,
						   y + insetAscent);
	TEXT(bv)->selection.cursor = TEXT(bv)->cursor;
	bv->text->finishUndo();
	showInsetCursor(bv);
	updateLocal(bv, CURSOR, false);

	// If the inset is empty set the language of the current font to the
	// language to the surronding text (if different).
	if (par->size() == 0 && !par->next() &&
		bv->getParentLanguage(this) != TEXT(bv)->current_font.language())
	{
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(bv->getParentLanguage(this));
		setFont(bv, font, false);
	}
}


void InsetText::insetUnlock(BufferView * bv)
{
	if (the_locking_inset) {
		the_locking_inset->insetUnlock(bv);
		the_locking_inset = 0;
	}
	hideInsetCursor(bv);
	no_selection = false;
	locked = false;
	int code = CURSOR|CLEAR_FRAME;
	if (TEXT(bv)->selection.set()) {
		TEXT(bv)->clearSelection(bv);
		code = FULL;
	} else if (owner())
		bv->owner()->setLayout(owner()->getLyXText(bv)
		                       ->cursor.par()->getLayout());
	else
		bv->owner()->setLayout(bv->text->cursor.par()->getLayout());
	updateLocal(bv, code, false);
}


bool InsetText::lockInsetInInset(BufferView * bv, UpdatableInset * inset)
{
	lyxerr[Debug::INSETS] << "InsetText::LockInsetInInset("
			      << inset << "): ";
	if (!inset)
		return false;
	if (inset == cpar(bv)->getInset(cpos(bv))) {
		lyxerr[Debug::INSETS] << "OK" << endl;
		the_locking_inset = inset;
		inset_x = cx(bv) - top_x + drawTextXOffset;
		inset_y = cy(bv) + drawTextYOffset;
		inset_pos = cpos(bv);
		inset_par = cpar(bv);
		inset_boundary = cboundary(bv);
#if 0
		TEXT(bv)->clearSelection(bv);
		TEXT(bv)->sel_cursor = TEXT(bv)->cursor;
		TEXT(bv)->UpdateInset(bv, the_locking_inset);
#else
		updateLocal(bv, CURSOR, false);
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
		return the_locking_inset->lockInsetInInset(bv, inset);
	}
	lyxerr[Debug::INSETS] << "NOT OK" << endl;
	return false;
}


bool InsetText::unlockInsetInInset(BufferView * bv, UpdatableInset * inset,
				   bool lr)
{
	if (!the_locking_inset)
		return false;
	if (the_locking_inset == inset) {
		the_locking_inset->insetUnlock(bv);
		TEXT(bv)->updateInset(bv, inset);
		the_locking_inset = 0;
		if (lr)
			moveRight(bv, false);
		old_par = 0; // force layout setting
		if (scroll())
			scroll(bv, 0.0F);
		else
			updateLocal(bv, CURSOR, false);
		return true;
	}
	return the_locking_inset->unlockInsetInInset(bv, inset, lr);
}


bool InsetText::updateInsetInInset(BufferView * bv, Inset * inset)
{
	if (!the_locking_inset)
		return false;
	if (the_locking_inset != inset) {
		TEXT(bv)->updateInset(bv, the_locking_inset);
		setUpdateStatus(bv, CURSOR_PAR);
		return the_locking_inset->updateInsetInInset(bv, inset);
	}
//    updateLocal(bv, FULL, false);
	if (TEXT(bv)->updateInset(bv, inset))
		updateLocal(bv, CURSOR_PAR, false);
	if (cpar(bv) == inset_par && cpos(bv) == inset_pos) {
		inset_x = cx(bv) - top_x + drawTextXOffset;
		inset_y = cy(bv) + drawTextYOffset;
	}
	return true;
}


void InsetText::insetButtonPress(BufferView * bv, int x, int y, int button)
{
	no_selection = true;

	int tmp_x = x - drawTextXOffset;
	int tmp_y = y + insetAscent - TEXT(bv)->first;
	Inset * inset = bv->checkInsetHit(TEXT(bv), tmp_x, tmp_y, button);

	hideInsetCursor(bv);
	if (the_locking_inset) {
		if (the_locking_inset == inset) {
			the_locking_inset->insetButtonPress(bv,x-inset_x,y-inset_y,button);
			no_selection = false;
			return;
		} else if (inset) {
			// otherwise unlock the_locking_inset and lock the new inset
			the_locking_inset->insetUnlock(bv);
			inset_x = cx(bv) - top_x + drawTextXOffset;
			inset_y = cy(bv) + drawTextYOffset;
			the_locking_inset = static_cast<UpdatableInset*>(inset);
			inset->insetButtonPress(bv, x - inset_x, y - inset_y, button);
			inset->edit(bv, x - inset_x, y - inset_y, button);
			if (the_locking_inset)
				updateLocal(bv, CURSOR, false);
			no_selection = false;
			return;
		}
		// otherwise only unlock the_locking_inset
		the_locking_inset->insetUnlock(bv);
		the_locking_inset = 0;
	}
	if (bv->theLockingInset()) {
		if (inset && inset->editable() == Inset::HIGHLY_EDITABLE) {
			UpdatableInset * uinset = static_cast<UpdatableInset*>(inset);
			inset_x = cx(bv) - top_x + drawTextXOffset;
			inset_y = cy(bv) + drawTextYOffset;
			inset_pos = cpos(bv);
			inset_par = cpar(bv);
			inset_boundary = cboundary(bv);
			the_locking_inset = uinset;
			uinset->insetButtonPress(bv, x - inset_x, y - inset_y,
						 button);
			uinset->edit(bv, x - inset_x, y - inset_y, 0);
			if (the_locking_inset)
				updateLocal(bv, CURSOR, false);
			no_selection = false;
			return;
		}
	}
	if (!inset) { // && (button == 2)) {
		bool paste_internally = false;
		if ((button == 2) && TEXT(bv)->selection.set()) {
			localDispatch(bv, LFUN_COPY, "");
			paste_internally = true;
		}
		TEXT(bv)->setCursorFromCoordinates(bv, x-drawTextXOffset,
						   y + insetAscent);
		TEXT(bv)->selection.cursor = TEXT(bv)->cursor;
		updateLocal(bv, CURSOR, false);
		bv->owner()->setLayout(cpar(bv)->getLayout());
		old_par = cpar(bv);
		// Insert primary selection with middle mouse
		// if there is a local selection in the current buffer,
		// insert this
		if (button == 2) {
			if (paste_internally)
				localDispatch(bv, LFUN_PASTE, "");
			else
				localDispatch(bv, LFUN_PASTESELECTION,
					      "paragraph");
		}
	}
	showInsetCursor(bv);
	no_selection = false;
}


void InsetText::insetButtonRelease(BufferView * bv, int x, int y, int button)
{
	UpdatableInset * inset = 0;

	if (the_locking_inset) {
		the_locking_inset->insetButtonRelease(bv,
						      x - inset_x, y - inset_y,
						      button);
	} else {
		if (cpar(bv)->getChar(cpos(bv)) == Paragraph::META_INSET) {
			inset = static_cast<UpdatableInset*>(cpar(bv)->getInset(cpos(bv)));
			if (inset->editable() == Inset::HIGHLY_EDITABLE) {
				inset->insetButtonRelease(bv,
							  x - inset_x,
							  y - inset_y, button);
			} else {
				inset_x = cx(bv) - top_x + drawTextXOffset;
				inset_y = cy(bv) + drawTextYOffset;
				inset->insetButtonRelease(bv,
							  x - inset_x,
							  y - inset_y, button);
				inset->edit(bv,
					    x - inset_x, y - inset_y, button);
			}
			updateLocal(bv, CURSOR_PAR, false);
		}
	}
	no_selection = false;
}


void InsetText::insetMotionNotify(BufferView * bv, int x, int y, int state)
{
	if (no_selection)
		return;
	if (the_locking_inset) {
		the_locking_inset->insetMotionNotify(bv, x - inset_x,
						     y - inset_y,state);
		return;
	}
	hideInsetCursor(bv);
	TEXT(bv)->setCursorFromCoordinates(bv, x - drawTextXOffset,
					   y + insetAscent);
	TEXT(bv)->setSelection(bv);
	if (TEXT(bv)->toggle_cursor.par()!=TEXT(bv)->toggle_end_cursor.par() ||
		TEXT(bv)->toggle_cursor.pos()!=TEXT(bv)->toggle_end_cursor.pos())
		updateLocal(bv, SELECTION, false);
	showInsetCursor(bv);
}


void InsetText::insetKeyPress(XKeyEvent * xke)
{
	if (the_locking_inset) {
		the_locking_inset->insetKeyPress(xke);
		return;
	}
}


UpdatableInset::RESULT
InsetText::localDispatch(BufferView * bv,
                         kb_action action, string const & arg)
{
	no_selection = false;
	UpdatableInset::RESULT
		result= UpdatableInset::localDispatch(bv, action, arg);
	if (result != UNDISPATCHED) {
		return DISPATCHED;
	}

	result = DISPATCHED;
	if ((action < 0) && arg.empty())
		return FINISHED;

	if (the_locking_inset) {
		result = the_locking_inset->localDispatch(bv, action, arg);
		if (result == DISPATCHED_NOUPDATE)
			return result;
		else if (result == DISPATCHED) {
			updateLocal(bv, CURSOR_PAR, false);
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
	hideInsetCursor(bv);
	switch (action) {
	// Normal chars
	case LFUN_SELFINSERT:
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

			bv->text->setUndo(bv->buffer(), Undo::INSERT,
					  bv->text->cursor.par()->previous(),
					  bv->text->cursor.par()->next());
			bv->setState();
			if (lyxrc.auto_region_delete) {
				if (TEXT(bv)->selection.set()) {
					TEXT(bv)->cutSelection(bv, false);
				}
			}
			TEXT(bv)->clearSelection(bv);
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
		TEXT(bv)->selection.cursor = TEXT(bv)->cursor;
		updateLocal(bv, CURSOR_PAR, true);
		result=DISPATCHED_NOUPDATE;
		break;
		// --- Cursor Movements -----------------------------------
	case LFUN_RIGHTSEL:
		bv->text->finishUndo();
		moveRight(bv, false, true);
		TEXT(bv)->setSelection(bv);
		updateLocal(bv, SELECTION, false);
		break;
	case LFUN_RIGHT:
		result = moveRight(bv);
		bv->text->finishUndo();
		updateLocal(bv, CURSOR, false);
		break;
	case LFUN_LEFTSEL:
		bv->text->finishUndo();
		moveLeft(bv, false, true);
		TEXT(bv)->setSelection(bv);
		updateLocal(bv, SELECTION, false);
		break;
	case LFUN_LEFT:
		bv->text->finishUndo();
		result = moveLeft(bv);
		updateLocal(bv, CURSOR, false);
		break;
	case LFUN_DOWNSEL:
		bv->text->finishUndo();
		moveDown(bv);
		TEXT(bv)->setSelection(bv);
		updateLocal(bv, SELECTION, false);
		break;
	case LFUN_DOWN:
		bv->text->finishUndo();
		result = moveDown(bv);
		updateLocal(bv, CURSOR, false);
		break;
	case LFUN_UPSEL:
		bv->text->finishUndo();
		moveUp(bv);
		TEXT(bv)->setSelection(bv);
		updateLocal(bv, SELECTION, false);
		break;
	case LFUN_UP:
		bv->text->finishUndo();
		result = moveUp(bv);
		updateLocal(bv, CURSOR, false);
		break;
	case LFUN_HOME:
		bv->text->finishUndo();
		TEXT(bv)->cursorHome(bv);
		updateLocal(bv, CURSOR, false);
		break;
	case LFUN_END:
		TEXT(bv)->cursorEnd(bv);
		updateLocal(bv, CURSOR, false);
		break;
	case LFUN_BACKSPACE:
		bv->text->setUndo(bv->buffer(), Undo::DELETE,
				  bv->text->cursor.par()->previous(),
				  bv->text->cursor.par()->next());
		if (TEXT(bv)->selection.set())
			TEXT(bv)->cutSelection(bv);
		else
			TEXT(bv)->backspace(bv);
		updateLocal(bv, CURSOR_PAR, true);
		break;
	case LFUN_DELETE:
		bv->text->setUndo(bv->buffer(), Undo::DELETE,
				  bv->text->cursor.par()->previous(),
				  bv->text->cursor.par()->next());
		if (TEXT(bv)->selection.set())
			TEXT(bv)->cutSelection(bv);
		else
			TEXT(bv)->Delete(bv);
		updateLocal(bv, CURSOR_PAR, true);
		break;
	case LFUN_CUT:
		bv->text->setUndo(bv->buffer(), Undo::DELETE,
				  bv->text->cursor.par()->previous(),
				  bv->text->cursor.par()->next());
		TEXT(bv)->cutSelection(bv);
		updateLocal(bv, CURSOR_PAR, true);
		break;
	case LFUN_COPY:
		bv->text->finishUndo();
		TEXT(bv)->copySelection(bv);
		updateLocal(bv, CURSOR_PAR, false);
		break;
	case LFUN_PASTESELECTION:
	{
		string const clip(bv->getClipboard());
	
		if (clip.empty())
			break;
		if (arg == "paragraph") {
			TEXT(bv)->insertStringAsParagraphs(bv, clip);
		} else {
			TEXT(bv)->insertStringAsLines(bv, clip);
		}
		updateLocal(bv, CURSOR_PAR, true);
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
		bv->text->setUndo(bv->buffer(), Undo::INSERT,
				  bv->text->cursor.par()->previous(),
				  bv->text->cursor.par()->next());
		TEXT(bv)->pasteSelection(bv);
		updateLocal(bv, CURSOR_PAR, true);
		break;
	case LFUN_BREAKPARAGRAPH:
		if (!autoBreakRows)
			return DISPATCHED;
		TEXT(bv)->breakParagraph(bv, 0);
		updateLocal(bv, FULL, true);
		break;
	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
		if (!autoBreakRows)
			return DISPATCHED;
		TEXT(bv)->breakParagraph(bv, 1);
		updateLocal(bv, FULL, true);
		break;
	case LFUN_BREAKLINE:
		if (!autoBreakRows)
			return DISPATCHED;
		bv->text->setUndo(bv->buffer(), Undo::INSERT,
				  bv->text->cursor.par()->previous(),
				  bv->text->cursor.par()->next());
		TEXT(bv)->insertChar(bv, Paragraph::META_NEWLINE);
		updateLocal(bv, CURSOR_PAR, true);
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
				TEXT(bv)->setLayout(bv, layout.second);
				bv->owner()->setLayout(cpar(bv)->getLayout());
				updateLocal(bv, CURSOR_PAR, true);
			}
		} else {
			// reset the layout box
			bv->owner()->setLayout(cpar(bv)->getLayout());
		}
		break;
	case LFUN_PARAGRAPH_SPACING:
		// This one is absolutely not working. When fiddling with this
		// it also seems to me that the paragraphs inside the insettext
		// inherit bufferparams/paragraphparams in a strange way. (Lgb)
	{
		Paragraph * par = TEXT(bv)->cursor.par();
		Spacing::Space cur_spacing = par->params().spacing().getSpace();
		float cur_value = 1.0;
		if (cur_spacing == Spacing::Other) {
			cur_value = par->params().spacing().getValue();
		}
				
		std::istringstream istr(arg);
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
			par->params().spacing(Spacing(new_spacing, new_value));
			//TEXT(bv)->RedoParagraph(owner->view());
			updateLocal(bv, CURSOR_PAR, true);
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
		setFont(bv, font, false);
	}

	if (result != FINISHED) {
		showInsetCursor(bv);
	} else
		bv->unlockInset(this);
	return result;
}


int InsetText::latex(Buffer const * buf, ostream & os, bool, bool) const
{
	TexRow texrow;
	buf->latexParagraphs(os, par, 0, texrow);
	return texrow.rows();
}


int InsetText::ascii(Buffer const * buf, ostream & os, int linelen) const
{
	Paragraph * p = par;
	unsigned int lines = 0;
	
	while (p) {
		string const tmp = buf->asciiParagraph(p, linelen);
		lines += countChar(tmp, '\n');
		os << tmp;
		p = p->next();
	}
	return lines;
}


int InsetText::docBook(Buffer const * buf, ostream & os) const
{
	Paragraph * p = par;
	unsigned int lines = 0;
	int desc = 0;
	
	string tmp;
	while (p) {
		buf->simpleDocBookOnePar(os, tmp, p, desc, 0);
		p = p->next();
	}
	
	return lines;
}


void InsetText::validate(LaTeXFeatures & features) const
{
	Paragraph * p = par;
	while (p) {
		p->validate(features);
		p = p->next();
	}
}


int InsetText::beginningOfMainBody(Buffer const * buf, Paragraph * p) const
{
	if (textclasslist.Style(buf->params.textclass,
				p->getLayout()).labeltype != LABEL_MANUAL)
		return 0;
	else
		return p->beginningOfMainBody();
}


void InsetText::getCursorPos(BufferView * bv,
			     int & x, int & y) const
{
	x = cx(bv);
	y = cy(bv);
}


unsigned int InsetText::insetInInsetY()
{
	if (!the_locking_inset)
		return 0;

	return (inset_y + the_locking_inset->insetInInsetY());
}


void InsetText::toggleInsetCursor(BufferView * bv)
{
	if (the_locking_inset) {
		the_locking_inset->toggleInsetCursor(bv);
		return;
	}

	LyXFont const font(TEXT(bv)->getFont(bv->buffer(), cpar(bv), cpos(bv)));

	int const asc = lyxfont::maxAscent(font);
	int const desc = lyxfont::maxDescent(font);
  
	if (isCursorVisible())
		bv->hideLockedInsetCursor();
	else
		bv->showLockedInsetCursor(cx(bv), cy(bv), asc, desc);
	toggleCursorVisible();
}


void InsetText::showInsetCursor(BufferView * bv, bool show)
{
	if (the_locking_inset) {
		the_locking_inset->showInsetCursor(bv);
		return;
	}
	if (!isCursorVisible()) {
		LyXFont const font =
			TEXT(bv)->getFont(bv->buffer(), cpar(bv), cpos(bv));
	
		int const asc = lyxfont::maxAscent(font);
		int const desc = lyxfont::maxDescent(font);

		bv->fitLockedInsetCursor(cx(bv), cy(bv), asc, desc);
		if (show)
			bv->showLockedInsetCursor(cx(bv), cy(bv), asc, desc);
		setCursorVisible(true);
	}
}


void InsetText::hideInsetCursor(BufferView * bv)
{
	if (isCursorVisible()) {
		bv->hideLockedInsetCursor();
		setCursorVisible(false);
	}
	if (the_locking_inset)
		the_locking_inset->hideInsetCursor(bv);
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
	TEXT(bv)->cursorRight(bv);
	if (!selecting)
		TEXT(bv)->selection.cursor = TEXT(bv)->cursor;
	return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT
InsetText::moveLeftIntern(BufferView * bv, bool behind,
			  bool activate_inset, bool selecting)
{
	if (!cpar(bv)->previous() && (cpos(bv) <= 0))
		return FINISHED;
	TEXT(bv)->cursorLeft(bv);
	if (!selecting)
		TEXT(bv)->selection.cursor = TEXT(bv)->cursor;
	if (activate_inset && checkAndActivateInset(bv, behind))
		return DISPATCHED;
	return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT
InsetText::moveUp(BufferView * bv)
{
	if (!crow(bv)->previous())
		return FINISHED;
	TEXT(bv)->cursorUp(bv);
	return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT
InsetText::moveDown(BufferView * bv)
{
	if (!crow(bv)->next())
		return FINISHED;
	TEXT(bv)->cursorDown(bv);
	return DISPATCHED_NOUPDATE;
}


bool InsetText::insertInset(BufferView * bv, Inset * inset)
{
	if (the_locking_inset) {
		if (the_locking_inset->insertInsetAllowed(inset))
			return the_locking_inset->insertInset(bv, inset);
		return false;
	}
	bv->text->setUndo(bv->buffer(), Undo::INSERT,
			  bv->text->cursor.par()->previous(),
			  bv->text->cursor.par()->next());
	inset->setOwner(this);
	hideInsetCursor(bv);
	TEXT(bv)->insertInset(bv, inset);
#if 0
	if ((cpar(bv)->GetChar(cpos(bv)) != Paragraph::META_INSET) ||
		(cpar(bv)->GetInset(cpos(bv)) != inset))
		TEXT(bv)->CursorLeft(bv);
#endif
	bv->fitCursor(TEXT(bv));
	updateLocal(bv, CURSOR_PAR|CURSOR, true);
	showInsetCursor(bv);
	return true;
}


UpdatableInset * InsetText::getLockingInset()
{
	return the_locking_inset ? the_locking_inset->getLockingInset() : this;
}


UpdatableInset * InsetText::getFirstLockingInsetOfType(Inset::Code c)
{
	if (c == lyxCode())
		return this;
	if (the_locking_inset)
		return the_locking_inset->getFirstLockingInsetOfType(c);
	return 0;
}


bool InsetText::showInsetDialog(BufferView * bv) const
{
	if (the_locking_inset)
		return the_locking_inset->showInsetDialog(bv);
	return false;
}


std::vector<string> const InsetText::getLabelList() const 
{
	std::vector<string> label_list;

	Paragraph * tpar = par;
	while (tpar) {
		Paragraph::inset_iterator beg = tpar->inset_iterator_begin();
		Paragraph::inset_iterator end = tpar->inset_iterator_end();
		for (; beg != end; ++beg) {
			std::vector<string> const l = (*beg)->getLabelList();
			label_list.insert(label_list.end(), l.begin(), l.end());
		}
		tpar = tpar->next();
	}
	return label_list;
}


void InsetText::setFont(BufferView * bv, LyXFont const & font, bool toggleall,
                        bool selectall)
{
	if (the_locking_inset) {
		the_locking_inset->setFont(bv, font, toggleall, selectall);
		return;
	}
	if (TEXT(bv)->selection.set()) {
		bv->text->setUndo(bv->buffer(), Undo::EDIT,
				  bv->text->cursor.par()->previous(),
				  bv->text->cursor.par()->next());
	}
	if (selectall)
		selectAll(bv);
	TEXT(bv)->setFont(bv, font, toggleall);
	if (selectall)
		TEXT(bv)->clearSelection(bv);
	bv->fitCursor(TEXT(bv));
	if (selectall || TEXT(bv)->selection.set())
		updateLocal(bv, FULL, true);
	else
		updateLocal(bv, CURSOR_PAR, true);
}


bool InsetText::checkAndActivateInset(BufferView * bv, bool behind)
{
	if (cpar(bv)->getChar(cpos(bv)) == Paragraph::META_INSET) {
		unsigned int x;
		unsigned int y;
		Inset * inset =
			static_cast<UpdatableInset*>(cpar(bv)->getInset(cpos(bv)));
		if (!inset || inset->editable() != Inset::HIGHLY_EDITABLE)
			return false;
		LyXFont const font =
			TEXT(bv)->getFont(bv->buffer(), cpar(bv), cpos(bv));
		if (behind) {
			x = inset->width(bv, font);
			y = font.isRightToLeft() ? 0 : inset->descent(bv, font);
		} else {
			x = 0;
			y = font.isRightToLeft() ? inset->descent(bv, font) : 0;
		}
		//inset_x = cx(bv) - top_x + drawTextXOffset;
		//inset_y = cy(bv) + drawTextYOffset;
		inset->edit(bv, x, y, 0);
		if (!the_locking_inset)
			return false;
		updateLocal(bv, CURSOR, false);
		return true;
	}
	return false;
}


bool InsetText::checkAndActivateInset(BufferView * bv, int x, int y,
				      int button)
{
	x -= drawTextXOffset;
	int dummyx = x;
	int dummyy = y + insetAscent;
	Inset * inset = bv->checkInsetHit(TEXT(bv), dummyx, dummyy, button);

	if (inset) {
		if (x < 0)
			x = insetWidth;
		if (y < 0)
			y = insetDescent;
		inset_x = cx(bv) - top_x + drawTextXOffset;
		inset_y = cy(bv) + drawTextYOffset;
		inset->edit(bv, x - inset_x, y - inset_y, button);
		if (!the_locking_inset)
			return false;
		updateLocal(bv, CURSOR, false);
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


void InsetText::setParagraphData(Paragraph * p)
{
	// delete all instances of LyXText before deleting the paragraps used
	// by it.
	for (Cache::iterator cit = cache.begin(); cit != cache.end(); ++cit){
		delete (*cit).second;
		(*cit).second = 0;
	}

	while (par) {
		Paragraph * tmp = par->next();
		delete par;
		par = tmp;
	}

	par = new Paragraph(*p);
	par->setInsetOwner(this);
	Paragraph * np = par;
	while (p->next()) {
		p = p->next();
		np->next(new Paragraph(*p));
		np->next()->previous(np);
		np = np->next();
		np->setInsetOwner(this);
	}

	need_update = INIT;
}


void InsetText::setText(string const & data)
{
	clear();
	LyXFont font(LyXFont::ALL_SANE);
	for (unsigned int i=0; i < data.length(); ++i)
		par->insertChar(i, data[i], font);
}


void InsetText::setAutoBreakRows(bool flag)
{
	if (flag != autoBreakRows) {
		autoBreakRows = flag;
		need_update = FULL;
		if (!flag)
			removeNewlines();
	}
}


void InsetText::setDrawFrame(BufferView * bv, DrawFrame how)
{
	if (how != drawFrame_) {
		drawFrame_ = how;
		if (bv)
			updateLocal(bv, DRAW_FRAME, false);
	}
}


void InsetText::setFrameColor(BufferView * bv, LColor::color col)
{
	if (frame_color != col) {
		frame_color = col;
		if (bv)
			updateLocal(bv, DRAW_FRAME, false);
	}
}


int InsetText::cx(BufferView * bv) const
{
	LyXText * text = TEXT(bv);
	int x = text->cursor.x() + top_x + TEXT_TO_INSET_OFFSET;
	if (the_locking_inset) {
		LyXFont font = text->getFont(bv->buffer(),
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


Paragraph::size_type InsetText::cpos(BufferView * bv) const
{
	return TEXT(bv)->cursor.pos();
}


Paragraph * InsetText::cpar(BufferView * bv) const
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
		lt->setCursor(bv, inset_par, inset_pos, true, inset_boundary);
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
		Paragraph * p = par;
		for (; p; p = p->next()) {
			p->deleteInsetsLyXText(bv);
		}
	}
}


void InsetText::resizeLyXText(BufferView * bv, bool force) const
{
	if (!par->next() && !par->size()) // no data, resize not neccessary!
		return;
	// one endless line, resize normally not necessary
	if (!force && getMaxWidth(bv, this) < 0)
		return;
	if ((cache.find(bv) == cache.end()) || !cache[bv])
		return;

	Paragraph * lpar = 0;
	Paragraph * selstartpar = 0;
	Paragraph * selendpar = 0;
	Paragraph::size_type pos = 0;
	Paragraph::size_type selstartpos = 0;
	Paragraph::size_type selendpos = 0;
	bool boundary = false;
	bool selstartboundary = false;
	bool selendboundary = false;
	bool selection = false;
	bool mark_set = false;

//    ProhibitInput(bv);

	if (locked) {
		lpar = TEXT(bv)->cursor.par();
		pos = TEXT(bv)->cursor.pos();
		boundary = TEXT(bv)->cursor.boundary();
		selstartpar = TEXT(bv)->selection.start.par();
		selstartpos = TEXT(bv)->selection.start.pos();
		selstartboundary = TEXT(bv)->selection.start.boundary();
		selendpar = TEXT(bv)->selection.end.par();
		selendpos = TEXT(bv)->selection.end.pos();
		selendboundary = TEXT(bv)->selection.end.boundary();
		selection = TEXT(bv)->selection.set();
		mark_set = TEXT(bv)->selection.mark();
	}
	deleteLyXText(bv, (the_locking_inset == 0) || force);

	if (lpar) {
		TEXT(bv)->selection.set(true);
		/* at this point just to avoid the Delete-Empty-Paragraph
		 * Mechanism when setting the cursor */
		TEXT(bv)->selection.mark(mark_set);
		if (selection) {
			TEXT(bv)->setCursor(bv, selstartpar, selstartpos,
					    true, selstartboundary);
			TEXT(bv)->selection.cursor = TEXT(bv)->cursor;
			TEXT(bv)->setCursor(bv, selendpar, selendpos,
					    true, selendboundary);
			TEXT(bv)->setSelection(bv);
			TEXT(bv)->setCursor(bv, lpar, pos);
		} else {
			TEXT(bv)->setCursor(bv, lpar, pos, true, boundary);
			TEXT(bv)->selection.cursor = TEXT(bv)->cursor;
			TEXT(bv)->selection.set(false);
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
		for (Paragraph * p = par; p; p = p->next()) {
			p->resizeInsetsLyXText(bv);
		}
	}
	need_update = FULL;
}


void InsetText::removeNewlines()
{
	for (Paragraph * p = par; p; p = p->next()) {
		for (int i = 0; i < p->size(); ++i) {
			if (p->getChar(i) == Paragraph::META_NEWLINE)
				p->erase(i);
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


void InsetText::selectAll(BufferView * bv)
{
	TEXT(bv)->cursorTop(bv);
	TEXT(bv)->selection.cursor = TEXT(bv)->cursor;
	TEXT(bv)->cursorBottom(bv);
	TEXT(bv)->setSelection(bv);
}


void InsetText::clearSelection(BufferView * bv)
{
	TEXT(bv)->clearSelection(bv);
}


void InsetText::clearInset(Painter & pain, int baseline, bool & cleared) const
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
/* Emacs:
 * Local variables:
 * tab-width: 4
 * End:
 * vi:set tabstop=4:
 */
