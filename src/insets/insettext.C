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
#include "lyxtextclasslist.h"
#include "LaTeXFeatures.h"
#include "Painter.h"
#include "lyxtext.h"
#include "lyxcursor.h"
#include "CutAndPaste.h"
#include "font.h"
#include "LColor.h"
#include "lyxrow.h"
#include "lyxrc.h"
#include "intl.h"
#include "trans_mgr.h"
#include "lyxscreen.h"
#include "WorkArea.h"
#include "gettext.h"
#include "lyxfunc.h"
#include "ParagraphParameters.h"
#include "undo_funcs.h"
#include "lyxfind.h"

#include "frontends/Alert.h"

#include "support/textutils.h"
#include "support/LAssert.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h" // lyx::count

#include <fstream>
#include <algorithm>
#include <cstdlib>
//#include <csignal>

using std::ostream;
using std::ifstream;
using std::endl;
using std::min;
using std::max;
using std::make_pair;
using std::vector;
using std::pair;

using lyx::pos_type;
using lyx::layout_type;
using lyx::textclass_type;

extern unsigned char getCurrentTextClass(Buffer *);
extern bool math_insert_greek(BufferView *, char);
extern int greek_kb_flag;


// These functions should probably go into bufferview_funcs somehow (Jug)

void InsetText::saveLyXTextState(LyXText * t) const
{
	// check if my paragraphs are still valid
	Paragraph * p = par;
	while (p) {
		if (p == t->cursor.par())
			break;
		p = p->next();
	}
	
	if (p && t->cursor.pos() <= p->size()) {
		sstate.lpar = t->cursor.par();
		sstate.pos = t->cursor.pos();
		sstate.boundary = t->cursor.boundary();
		sstate.selstartpar = t->selection.start.par();
		sstate.selstartpos = t->selection.start.pos();
		sstate.selstartboundary = t->selection.start.boundary();
		sstate.selendpar = t->selection.end.par();
		sstate.selendpos = t->selection.end.pos();
		sstate.selendboundary = t->selection.end.boundary();
		sstate.selection = t->selection.set();
		sstate.mark_set = t->selection.mark();
		sstate.refresh = t->refresh_row != 0;
	} else {
		sstate.lpar = 0;
	}
}

void InsetText::restoreLyXTextState(BufferView * bv, LyXText * t) const
{
	if (sstate.lpar) {
		t->selection.set(true);
		/* at this point just to avoid the Delete-Empty-Paragraph
		 * Mechanism when setting the cursor */
		t->selection.mark(sstate.mark_set);
		if (sstate.selection) {
			t->setCursor(bv, sstate.selstartpar, sstate.selstartpos,
			             true, sstate.selstartboundary);
			t->selection.cursor = t->cursor;
			t->setCursor(bv, sstate.selendpar, sstate.selendpos,
			             true, sstate.selendboundary);
			t->setSelection(bv);
			t->setCursor(bv, sstate.lpar, sstate.pos);
		} else {
			t->setCursor(bv, sstate.lpar, sstate.pos, true, sstate.boundary);
			t->selection.cursor = t->cursor;
			t->selection.set(false);
		}
		if (sstate.refresh) {
		}
	}
}


InsetText::InnerCache::InnerCache(boost::shared_ptr<LyXText> t)
{
	text = t;
	remove = false;
}


InsetText::InsetText()
	: UpdatableInset(), lt(0), in_update(false), do_resize(0),
	  do_reinit(false)
{
	par = new Paragraph;
	init();
}


InsetText::InsetText(InsetText const & in, bool same_id)
	: UpdatableInset(in, same_id), lt(0), in_update(false), do_resize(0),
	  do_reinit(false)
{
	par = 0;
	init(&in, same_id);
}


InsetText & InsetText::operator=(InsetText const & it)
{
	init(&it);
	return * this;
}


void InsetText::init(InsetText const * ins, bool same_id)
{
	if (ins) {
		setParagraphData(ins->par, same_id);
		autoBreakRows = ins->autoBreakRows;
		drawFrame_ = ins->drawFrame_;
		frame_color = ins->frame_color;
		if (same_id)
			id_ = ins->id_;
	} else {
		Paragraph * p = par;
		while (p) {
			p->setInsetOwner(this);
			p = p->next();
		}
		the_locking_inset = 0;
		drawFrame_ = NEVER;
		frame_color = LColor::insetframe;
		autoBreakRows = false;
	}
	top_y = 0;
	insetAscent = 0;
	insetDescent = 0;
	insetWidth = 0;
	old_max_width = 0;
	no_selection = false;
	need_update = FULL;
	drawTextXOffset = 0;
	drawTextYOffset = 0;
	xpos = 0.0;
	locked = false;
	old_par = 0;
	last_drawn_width = -1;
	frame_is_visible = false;
	cached_bview = 0;
	sstate.lpar = 0;
	in_insetAllowed = false;
}


InsetText::~InsetText()
{
	cached_bview = 0;

	// NOTE
	
	while (par) {
		Paragraph * tmp = par->next();
		delete par;
		par = tmp;
	}
}


void InsetText::clear()
{
	while (par) {
		Paragraph * tmp = par->next();
		delete par;
		par = tmp;
	}
	par = new Paragraph;
	reinitLyXText();
	need_update = INIT;
}


Inset * InsetText::clone(Buffer const &, bool same_id) const
{
	return  new InsetText(*this, same_id);
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
	
	while (lex.isOK()) {
		lex.nextToken();
		token = lex.getString();
		if (token.empty())
			continue;
		if (token == "\\end_inset") {
#ifndef NO_COMPABILITY
			const_cast<Buffer*>(buf)->insertErtContents(par, pos, false);
#endif
			break;
		}
		
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
	while (return_par) {
		return_par->setInsetOwner(this);
		return_par = return_par->next();
	}
	
	if (token != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point. "
					   "Read: `$$Token'");
	}
	need_update = FULL;
}


int InsetText::ascent(BufferView * bv, LyXFont const &) const
{
	insetAscent = getLyXText(bv)->firstRow()->ascent_of_text() +
		TEXT_TO_INSET_OFFSET;
	return insetAscent;
}


int InsetText::descent(BufferView * bv, LyXFont const &) const
{
	LyXText * llt = getLyXText(bv);
	insetDescent = llt->height - llt->firstRow()->ascent_of_text() +
		TEXT_TO_INSET_OFFSET;
	return insetDescent;
}


int InsetText::width(BufferView * bv, LyXFont const &) const
{
	insetWidth = max(textWidth(bv), (int)getLyXText(bv)->width) +
		(2 * TEXT_TO_INSET_OFFSET);
	insetWidth = max(insetWidth, 10);
	return insetWidth;
}


int InsetText::textWidth(BufferView * bv, bool fordraw) const
{
	int w;
	if (!autoBreakRows) {
		w = -1;
	} else {
		w = getMaxWidth(bv, this);
	}
	if (fordraw) {
		return max(w - (2 * TEXT_TO_INSET_OFFSET),
		           (int)getLyXText(bv)->width);
	} else if (w < 0) {
	    return -1;
	}
	return w - (2 * TEXT_TO_INSET_OFFSET);
}


void InsetText::draw(BufferView * bv, LyXFont const & f,
                     int baseline, float & x, bool cleared) const
{
	if (nodraw())
		return;

	Painter & pain = bv->painter();
	
	// this is the first thing we have to ask because if the x pos
	// changed we have to do a complete rebreak of the text as we
	// may have few space to draw in. Well we should check on this too
	int old_x = top_x;
	if (top_x != int(x)) {
		top_x = int(x);
		topx_set = true;
		int nw = getMaxWidth(bv, this);
		if (nw > 0 && old_max_width != nw) {
			need_update = INIT;
			old_max_width = nw;
			bv->text->status(bv, LyXText::CHANGED_IN_DRAW);
			return;
		}
	}

	// call these methods so that insetWidth, insetAscent and
	// insetDescent have the right values. 
	width(bv, f);
	ascent(bv, f);
	descent(bv, f);

	// repaint the background if needed
	if (cleared && backgroundColor() != LColor::background) {
		clearInset(bv, baseline, cleared);
	}

	// no draw is necessary !!!
	if ((drawFrame_ == LOCKED) && !locked && !par->size()) {
		top_baseline = baseline;
		x += width(bv, f);
		if (need_update & CLEAR_FRAME)
			clearFrame(pain, cleared);
		need_update = NONE;
		return;
	}

	xpos = x;
	if (!owner())
		x += static_cast<float>(scroll());

	// if top_x differs we did it already
	if (!cleared && (old_x == int(x))
	    && ((need_update&(INIT|FULL)) || (top_baseline != baseline)
		||(last_drawn_width != insetWidth)))
	{
		// Condition necessary to eliminate bug 59 attachment 37
		if (baseline > 0)
			clearInset(bv, baseline, cleared);
	}

	if (cleared)
		frame_is_visible = false;

	if (!cleared && (need_update == NONE)) {
		if (locked)
			drawFrame(pain, cleared);
		return;
	}

	top_baseline = baseline;
	top_y = baseline - insetAscent;

	if (last_drawn_width != insetWidth) {
		if (!cleared) 
			clearInset(bv, baseline, cleared);
		need_update |= FULL;
		last_drawn_width = insetWidth;
	}

	if (the_locking_inset && (cpar(bv) == inset_par)
		&& (cpos(bv) == inset_pos)) {
		inset_x = cx(bv) - top_x + drawTextXOffset;
		inset_y = cy(bv) + drawTextYOffset;
	}
	if (!cleared && (need_update == CURSOR)
	    && !getLyXText(bv)->selection.set()) {
		drawFrame(pain, cleared);
		x += insetWidth; 
		need_update = NONE;
		return;
	}
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	x += TEXT_TO_INSET_OFFSET;

	Row * row = lt->firstRow();
	int y_offset = baseline - row->ascent_of_text();
	int ph = pain.paperHeight();
	int first = 0;
	int y = y_offset;
	while ((row != 0) && ((y+row->height()) <= 0)) {
		y += row->height();
		first += row->height();
		row = row->next();
	}
	if (y_offset < 0)
		y_offset = y;
	lt->first = first;
	if (cleared || (need_update&(INIT|FULL))) {
		int yf = y_offset;
		y = 0;
		while ((row != 0) && (yf < ph)) {
			lt->getVisibleRow(bv, y+y_offset, int(x), row,
			                        y+first, cleared);
			y += row->height();
			yf += row->height();
			row = row->next();
		}
	} else if (!locked) {
		if (need_update & CURSOR) {
			bv->screen()->toggleSelection(lt, bv, true, y_offset,int(x));
			lt->clearSelection();
			lt->selection.cursor = lt->cursor;
		}
		bv->screen()->update(lt, bv, y_offset, int(x));
	} else {
		locked = false;
		if (need_update & SELECTION) {
			bv->screen()->toggleToggle(lt, bv, y_offset, int(x));
		} else if (need_update & CURSOR) {
			bv->screen()->toggleSelection(lt, bv, true, y_offset,int(x));
			lt->clearSelection();
			lt->selection.cursor = lt->cursor;
		}
		bv->screen()->update(lt, bv, y_offset, int(x));
		locked = true;
	}

	lt->refresh_y = 0;
	lt->status(bv, LyXText::UNCHANGED);
	if ((need_update != CURSOR_PAR) &&
	    ((drawFrame_ == ALWAYS) || ((drawFrame_ == LOCKED) && locked))) {
		drawFrame(pain, cleared);
	} else if (need_update & CLEAR_FRAME) {
		clearFrame(pain, cleared);
	}
	
	x += insetWidth - TEXT_TO_INSET_OFFSET;
	
	if (bv->text->status() == LyXText::CHANGED_IN_DRAW) {
		need_update |= FULL;
	} else if (need_update != INIT) {
		need_update = NONE;
	}
	if (clear)
		lt = 0;
}


void InsetText::drawFrame(Painter & pain, bool cleared) const
{
	static int const ttoD2 = TEXT_TO_INSET_OFFSET / 2;
	if (!frame_is_visible || cleared) {
		frame_x = top_x + ttoD2;
		frame_y = top_baseline - insetAscent + ttoD2;
		frame_w = insetWidth - TEXT_TO_INSET_OFFSET;
		frame_h = insetAscent + insetDescent - TEXT_TO_INSET_OFFSET;
		pain.rectangle(frame_x, frame_y, frame_w, frame_h,
		               frame_color);
		frame_is_visible = true;
	}
}


void InsetText::clearFrame(Painter & pain, bool cleared) const
{
	if (frame_is_visible) {
		if (!cleared) {
			pain.rectangle(frame_x, frame_y, frame_w, frame_h,
			               backgroundColor());
		}
		frame_is_visible = false;
	}
}


void InsetText::update(BufferView * bv, LyXFont const & font, bool reinit)
{
	if (in_update) {
		if (reinit && owner()) {
			reinitLyXText();
			owner()->update(bv, font, true);
		}
		return;
	}
	in_update = true;
	if (reinit || need_update == INIT) {
		need_update = FULL;
		// we should put this call where we set need_update to INIT!
		reinitLyXText();
		if (owner())
			owner()->update(bv, font, true);
		in_update = false;
		return;
	}
	if (the_locking_inset) {
		inset_x = cx(bv) - top_x + drawTextXOffset;
		inset_y = cy(bv) + drawTextYOffset;
		the_locking_inset->update(bv, font, reinit);
	}

	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	if ((need_update & CURSOR_PAR) && (lt->status() == LyXText::UNCHANGED) &&
		the_locking_inset)
	{
		lt->updateInset(bv, the_locking_inset);
	}
	if (lt->status() == LyXText::NEED_MORE_REFRESH)
		need_update |= FULL;
	if (clear)
		lt = 0;
	in_update = false;
}


void InsetText::setUpdateStatus(BufferView * bv, int what) const
{
	// this does nothing dangerous so use only a localized buffer
	LyXText * llt = getLyXText(bv);
	
	need_update |= what;
	// we have to redraw us full if our LyXText NEEDS_MORE_REFRES or
	// if we don't break row so that we only have one row to update!
	if ((llt->status() == LyXText::NEED_MORE_REFRESH) ||
	    (!autoBreakRows &&
	     (llt->status() == LyXText::NEED_VERY_LITTLE_REFRESH)))
	{
		need_update |= FULL;
	} else if (llt->status() == LyXText::NEED_VERY_LITTLE_REFRESH) {
		need_update |= CURSOR_PAR;
	}

	// this to not draw a selection when we redraw all of it!
	if (need_update & CURSOR && !(need_update & SELECTION)) {
		if (llt->selection.set())
			need_update = FULL;
		llt->clearSelection();
	}
}


void InsetText::updateLocal(BufferView * bv, int what, bool mark_dirty) const
{
	if (!autoBreakRows && par->next())
		collapseParagraphs(bv->buffer()->params);
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	lt->fullRebreak(bv);
	setUpdateStatus(bv, what);
	bool flag = (((need_update != CURSOR) && (need_update != NONE)) ||
	             (lt->status() != LyXText::UNCHANGED) || lt->selection.set());
	if (clear)
		lt = 0;
	if (flag)
		bv->updateInset(const_cast<InsetText *>(this), mark_dirty);
	else
		bv->fitCursor();
	
	if (need_update == CURSOR)
		need_update = NONE;
	bv->owner()->showState();
	bv->owner()->updateMenubar();
	bv->owner()->updateToolbar();
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
	int tmp_y = (y < 0) ? 0 : y;
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	if (!checkAndActivateInset(bv, x, tmp_y, button))
		lt->setCursorFromCoordinates(bv, x - drawTextXOffset,
		                            y + insetAscent);
	lt->clearSelection();
	finishUndo();
	// If the inset is empty set the language of the current font to the
	// language to the surronding text (if different).
	if (par->size() == 0 && !par->next() &&
		bv->getParentLanguage(this) != lt->current_font.language())
	{
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(bv->getParentLanguage(this));
		setFont(bv, font, false);
	}
	showInsetCursor(bv);
	if (clear)
		lt = 0;
	
	int code = CURSOR;
	if (drawFrame_ == LOCKED)
		code = CURSOR|DRAW_FRAME;
	updateLocal(bv, code, false);
}


void InsetText::edit(BufferView * bv, bool front)
{
	UpdatableInset::edit(bv, front);
	
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
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	if (front)
		lt->setCursor(bv, par, 0);
	else {
		Paragraph * p = par;
		while (p->next())
			p = p->next();
//		int const pos = (p->size() ? p->size()-1 : p->size());
		lt->setCursor(bv, p, p->size());
	}
	lt->clearSelection();
	finishUndo();
	// If the inset is empty set the language of the current font to the
	// language to the surronding text (if different).
	if (par->size() == 0 && !par->next() &&
		bv->getParentLanguage(this) != lt->current_font.language()) {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(bv->getParentLanguage(this));
		setFont(bv, font, false);
	}
	showInsetCursor(bv);
	if (clear)
		lt = 0;
	int code = CURSOR;
	if (drawFrame_ == LOCKED)
		code = CURSOR|DRAW_FRAME;
	updateLocal(bv, code, false);
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
	int code;
	if (drawFrame_ == LOCKED)
		code = CURSOR|CLEAR_FRAME;
	else 
		code = CURSOR;
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	if (lt->selection.set()) {
		lt->clearSelection();
		code = FULL;
	} else if (owner()) {
		bv->owner()->setLayout(owner()->getLyXText(bv)
		                       ->cursor.par()->getLayout());
	} else
		bv->owner()->setLayout(bv->text->cursor.par()->getLayout());
	// hack for deleteEmptyParMech
	lt->setCursor(bv, par, 0);
	if (clear)
		lt = 0;
	updateLocal(bv, code, false);
}

void InsetText::lockInset(BufferView * bv, UpdatableInset * inset)
{
	the_locking_inset = inset;
	inset_x = cx(bv) - top_x + drawTextXOffset;
	inset_y = cy(bv) + drawTextYOffset;
	inset_pos = cpos(bv);
	inset_par = cpar(bv);
	inset_boundary = cboundary(bv);
	updateLocal(bv, CURSOR, false);
}


bool InsetText::lockInsetInInset(BufferView * bv, UpdatableInset * inset)
{
	lyxerr[Debug::INSETS] << "InsetText::LockInsetInInset("
			      << inset << "): ";
	if (!inset)
		return false;
	if (!the_locking_inset) {
		Paragraph * p = par;
		int const id = inset->id();
		while(p) {
			Paragraph::inset_iterator it =
				p->inset_iterator_begin();
			Paragraph::inset_iterator const end =
				p->inset_iterator_end();
			for (; it != end; ++it) {
				if ((*it) == inset) {
					getLyXText(bv)->setCursorIntern(bv, p, it.getPos());
					lockInset(bv, inset);
					return true;
				}
				if ((*it)->getInsetFromID(id)) {
					getLyXText(bv)->setCursorIntern(bv, p, it.getPos());
					(*it)->edit(bv);
					return the_locking_inset->lockInsetInInset(bv, inset);
				}
			}
			p = p->next();
		}
		return false;
	}
	if (inset == cpar(bv)->getInset(cpos(bv))) {
		lyxerr[Debug::INSETS] << "OK" << endl;
		lockInset(bv, inset);
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
		getLyXText(bv)->updateInset(bv, inset);
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
	if (!autoBreakRows && par->next())
		collapseParagraphs(bv->buffer()->params);
	if (inset == this)
		return true;
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	if (!the_locking_inset) {
		bool found = lt->updateInset(bv, inset);
		if (clear)
			lt = 0;
		if (found)
			setUpdateStatus(bv, NONE);
		return found;
	}
	if (the_locking_inset != inset) {
		bool found = the_locking_inset->updateInsetInInset(bv, inset);
		if (clear)
			lt = 0;
		if (found)
			setUpdateStatus(bv, CURSOR_PAR);
		return found;
	}
	bool found = lt->updateInset(bv, inset);
	if (clear)
		lt = 0;
	if (found) {
		setUpdateStatus(bv, CURSOR_PAR);
		if (cpar(bv) == inset_par && cpos(bv) == inset_pos) {
			inset_x = cx(bv) - top_x + drawTextXOffset;
			inset_y = cy(bv) + drawTextYOffset;
		}
	}
	return found;
}


void InsetText::insetButtonPress(BufferView * bv, int x, int y, int button)
{
	no_selection = true;

	// use this to check mouse motion for selection!
	mouse_x = x;
	mouse_y = y;

	int tmp_x = x - drawTextXOffset;
	int tmp_y = y + insetAscent - getLyXText(bv)->first;
	Inset * inset = bv->checkInsetHit(getLyXText(bv), tmp_x, tmp_y);

	hideInsetCursor(bv);
	if (the_locking_inset) {
		if (the_locking_inset == inset) {
			the_locking_inset->insetButtonPress(bv,
							    x - inset_x,
							    y - inset_y,
							    button);
			no_selection = false;
			return;
		} else if (inset) {
			// otherwise unlock the_locking_inset and lock the new inset
			the_locking_inset->insetUnlock(bv);
			inset_x = cx(bv) - top_x + drawTextXOffset;
			inset_y = cy(bv) + drawTextYOffset;
			the_locking_inset = 0;
			inset->insetButtonPress(bv, x - inset_x,
						y - inset_y, button);
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
		if (isHighlyEditableInset(inset)) {
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
		if ((button == 2) && getLyXText(bv)->selection.set()) {
			localDispatch(bv, LFUN_COPY, "");
			paste_internally = true;
		}
		bool clear = false;
		if (!lt) {
			lt = getLyXText(bv);
			clear = true;
		}

		lt->setCursorFromCoordinates(bv, x - drawTextXOffset,
		                             y + insetAscent);
		// set the selection cursor!
		lt->selection.cursor = lt->cursor;
		lt->cursor.x_fix(lt->cursor.x());

		if (lt->selection.set()) {
			lt->clearSelection();
			if (clear)
				lt = 0;
			updateLocal(bv, FULL, false);
		} else {
			lt->clearSelection();
			if (clear)
				lt = 0;
			updateLocal(bv, CURSOR, false);
		}
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
	} else {
		getLyXText(bv)->clearSelection();
	}
	showInsetCursor(bv);
	no_selection = false;
}


bool InsetText::insetButtonRelease(BufferView * bv, int x, int y, int button)
{
	if (the_locking_inset) {
		return the_locking_inset->insetButtonRelease(bv,
		                                             x - inset_x, y - inset_y,
		                                             button);
	}
	int tmp_x = x - drawTextXOffset;
	int tmp_y = y + insetAscent - getLyXText(bv)->first;
	Inset * inset = bv->checkInsetHit(getLyXText(bv), tmp_x, tmp_y);
	bool ret = false;
	if (inset) {
		if (isHighlyEditableInset(inset)) {
			ret = inset->insetButtonRelease(bv, x - inset_x,
			                                y - inset_y, button);
		} else {
			inset_x = cx(bv) - top_x + drawTextXOffset;
			inset_y = cy(bv) + drawTextYOffset;
			ret = inset->insetButtonRelease(bv, x - inset_x,
			                                y - inset_y, button);
			inset->edit(bv, x - inset_x,
			            y - inset_y, button);
		}
		updateLocal(bv, CURSOR_PAR, false);
	}
	return ret;
}


void InsetText::insetMotionNotify(BufferView * bv, int x, int y, int state)
{
	if (no_selection || ((mouse_x == x) && (mouse_y == y)))
		return;
	if (the_locking_inset) {
		the_locking_inset->insetMotionNotify(bv, x - inset_x,
		                                     y - inset_y,state);
		return;
	}
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	hideInsetCursor(bv);
	LyXCursor cur = lt->cursor;
	lt->setCursorFromCoordinates(bv, x - drawTextXOffset, y + insetAscent);
	if (cur == lt->cursor) {
		if (clear)
			lt = 0;
		return;
	}
	lt->setSelection(bv);
	bool flag = (lt->toggle_cursor.par() != lt->toggle_end_cursor.par() ||
				 lt->toggle_cursor.pos() != lt->toggle_end_cursor.pos());
	if (clear)
		lt = 0;
	if (flag) {
		updateLocal(bv, SELECTION, false);
	}
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
	bool was_empty = par->size() == 0 && !par->next();
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
		} else if (result >= FINISHED) {
			switch (result) {
			case FINISHED_RIGHT:
				moveRightIntern(bv, false, false);
				result = DISPATCHED;
				break;
			case FINISHED_UP:
				if ((result = moveUp(bv)) >= FINISHED) {
					updateLocal(bv, CURSOR, false);
					bv->unlockInset(this);
				}
				break;
			case FINISHED_DOWN:
				if ((result = moveDown(bv)) >= FINISHED) {
					updateLocal(bv, CURSOR, false);
					bv->unlockInset(this);
				}
				break;
			default:
				result = DISPATCHED;
				break;
			}
			the_locking_inset = 0;
#ifdef WITH_WARNINGS
#warning I changed this to always return Dispatched maybe it is wrong (20011001 Jug)
#endif
			return result;
		}
	}
	hideInsetCursor(bv);
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	int updwhat = 0;
	int updflag = false;
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

			setUndo(bv, Undo::INSERT,
			        lt->cursor.par(), lt->cursor.par()->next());
			bv->setState();
			if (lyxrc.auto_region_delete) {
				if (lt->selection.set()) {
					lt->cutSelection(bv, false);
				}
			}
			lt->clearSelection();
			for (string::size_type i = 0; i < arg.length(); ++i) {
				bv->owner()->getIntl()->getTrans().TranslateAndInsert(arg[i], lt);
			}
		}
		lt->selection.cursor = lt->cursor;
		updwhat = CURSOR_PAR;
		updflag = true;
		result = DISPATCHED_NOUPDATE;
		break;
		// --- Cursor Movements -----------------------------------
	case LFUN_RIGHTSEL:
		finishUndo();
		moveRight(bv, false, true);
		lt->setSelection(bv);
		updwhat = SELECTION;
		break;
	case LFUN_RIGHT:
		result = moveRight(bv);
		finishUndo();
		updwhat = CURSOR;
		break;
	case LFUN_LEFTSEL:
		finishUndo();
		moveLeft(bv, false, true);
		lt->setSelection(bv);
		updwhat = SELECTION;
		break;
	case LFUN_LEFT:
		finishUndo();
		result = moveLeft(bv);
		updwhat = CURSOR;
		break;
	case LFUN_DOWNSEL:
		finishUndo();
		moveDown(bv);
		lt->setSelection(bv);
		updwhat = SELECTION;
		break;
	case LFUN_DOWN:
		finishUndo();
		result = moveDown(bv);
		updwhat = CURSOR;
		break;
	case LFUN_UPSEL:
		finishUndo();
		moveUp(bv);
		lt->setSelection(bv);
		updwhat = SELECTION;
		break;
	case LFUN_UP:
		finishUndo();
		result = moveUp(bv);
		updwhat = CURSOR;
		break;
	case LFUN_HOME:
		finishUndo();
		lt->cursorHome(bv);
		updwhat = CURSOR;
		break;
	case LFUN_END:
		lt->cursorEnd(bv);
		updwhat = CURSOR;
		break;
	case LFUN_BACKSPACE: {
		setUndo(bv, Undo::DELETE,
		        lt->cursor.par(), lt->cursor.par()->next());
		if (lt->selection.set())
			lt->cutSelection(bv);
		else
			lt->backspace(bv);
		updwhat = CURSOR_PAR;
		updflag = true;
	}
	break;
	
	case LFUN_DELETE: {
		setUndo(bv, Undo::DELETE,
		        lt->cursor.par(), lt->cursor.par()->next());
		if (lt->selection.set()) {
			lt->cutSelection(bv);
		} else {
			lt->Delete(bv);
		}
		updwhat = CURSOR_PAR;
		updflag = true;
	}
	break;
	
	case LFUN_CUT: {
		setUndo(bv, Undo::DELETE,
		        lt->cursor.par(), lt->cursor.par()->next());
		lt->cutSelection(bv);
		updwhat = CURSOR_PAR;
		updflag = true;
	}
	break;

	case LFUN_COPY:
		finishUndo();
		lt->copySelection(bv);
		updwhat = CURSOR_PAR;
		break;
	case LFUN_PASTESELECTION:
	{
		string const clip(bv->getClipboard());
	
		if (clip.empty())
			break;
		if (arg == "paragraph") {
			lt->insertStringAsParagraphs(bv, clip);
		} else {
			lt->insertStringAsLines(bv, clip);
		}
		updwhat = CURSOR_PAR;
		updflag = true;
		break;
	}
	case LFUN_PASTE: {
		if (!autoBreakRows) {

			if (CutAndPaste::nrOfParagraphs() > 1) {
				Alert::alert(_("Impossible operation"),
						   _("Cannot include more than one paragraph!"),
						   _("Sorry."));
				break;
			}
		}
		setUndo(bv, Undo::INSERT,
		        lt->cursor.par(), lt->cursor.par()->next());
		lt->pasteSelection(bv);
		updwhat = CURSOR_PAR;
		updflag = true;
	}
	break;

	case LFUN_BREAKPARAGRAPH:
		if (!autoBreakRows) {
			result = DISPATCHED;
			break;
		}
		lt->breakParagraph(bv, 0);
		updwhat = FULL;
		updflag = true;
		break;
	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
		if (!autoBreakRows) {
			result = DISPATCHED;
			break;
		}
		lt->breakParagraph(bv, 1);
		updwhat = FULL;
		updflag = true;
		break;

	case LFUN_BREAKLINE: {
		if (!autoBreakRows) {
			result = DISPATCHED;
			break;
		}
		setUndo(bv, Undo::INSERT,
		        lt->cursor.par(), lt->cursor.par()->next());
		lt->insertChar(bv, Paragraph::META_NEWLINE);
		updwhat = CURSOR_PAR;
		updflag = true;
	}
	break;

	case LFUN_LAYOUT:
		// do not set layouts on non breakable textinsets
		if (autoBreakRows) {
			layout_type cur_layout = cpar(bv)->layout;
	  
			// Derive layout number from given argument (string)
			// and current buffer's textclass (number). */    
			textclass_type tclass = bv->buffer()->params.textclass;
			pair<bool, layout_type> layout = 
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
				bv->owner()->getLyXFunc()->dispatch(LFUN_MESSAGE, msg);
				break;
			}

			if (cur_layout != layout.second) {
				cur_layout = layout.second;
				lt->setLayout(bv, layout.second);
				bv->owner()->setLayout(cpar(bv)->getLayout());
				updwhat = CURSOR_PAR;
				updflag = true;
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
		Paragraph * par = lt->cursor.par();
		Spacing::Space cur_spacing = par->params().spacing().getSpace();
		float cur_value = 1.0;
		if (cur_spacing == Spacing::Other) {
			cur_value = par->params().spacing().getValue();
		}
				
		istringstream istr(arg.c_str());
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
			updwhat = CURSOR_PAR;
			updflag = true;
		}
	}
	break;
	
	default:
		if (!bv->Dispatch(action, arg))
			result = UNDISPATCHED;
		break;
	}

	if (clear)
		lt = 0;
	if (updwhat > 0)
		updateLocal(bv, updwhat, updflag);
	/// If the action has deleted all text in the inset, we need to change the
	// language to the language of the surronding text.
	if (!was_empty && par->size() == 0 && !par->next()) {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(bv->getParentLanguage(this));
		setFont(bv, font, false);
	}

	if (result < FINISHED) {
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
		string const tmp = buf->asciiParagraph(p, linelen, p->previous()==0);
		lines += lyx::count(tmp.begin(), tmp.end(), '\n');
		os << tmp;
		p = p->next();
	}
	return lines;
}


int InsetText::docbook(Buffer const * buf, ostream & os) const
{
	Paragraph * p = par;
	unsigned int lines = 0;

	vector<string> environment_stack(10);
	vector<string> environment_inner(10);
	
	int const command_depth = 0;
	string item_name;
	
	Paragraph::depth_type depth = 0; // paragraph depth

	while (p) {
		string sgmlparam;
		int desc_on = 0; // description mode

		LyXLayout const & style =
			textclasslist.Style(buf->params.textclass,
					    p->layout);

		// environment tag closing
		for (; depth > p->params().depth(); --depth) {
			if (environment_inner[depth] != "!-- --") {
				item_name = "listitem";
				buf->sgmlCloseTag(os, command_depth + depth,
					     item_name);
				if (environment_inner[depth] == "varlistentry")
					buf->sgmlCloseTag(os, depth+command_depth,
						     environment_inner[depth]);
			}
			buf->sgmlCloseTag(os, depth + command_depth,
				     environment_stack[depth]);
			environment_stack[depth].erase();
			environment_inner[depth].erase();
		}

		if (depth == p->params().depth()
		   && environment_stack[depth] != style.latexname()
		   && !environment_stack[depth].empty()) {
			if (environment_inner[depth] != "!-- --") {
				item_name= "listitem";
				buf->sgmlCloseTag(os, command_depth+depth,
						  item_name);
				if (environment_inner[depth] == "varlistentry")
					buf->sgmlCloseTag(os,
							  depth + command_depth,
							  environment_inner[depth]);
			}
			
			buf->sgmlCloseTag(os, depth + command_depth,
					  environment_stack[depth]);
			
			environment_stack[depth].erase();
			environment_inner[depth].erase();
                }

		// Write opening SGML tags.
		switch (style.latextype) {
		case LATEX_PARAGRAPH:
			buf->sgmlOpenTag(os, depth + command_depth,
					 style.latexname());
			break;

		case LATEX_COMMAND:
			buf->sgmlError(p, 0,
				       _("Error : LatexType Command not allowed here.\n"));
			return -1;
			break;

		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
			if (depth < p->params().depth()) {
				depth = p->params().depth();
				environment_stack[depth].erase();
			}

			if (environment_stack[depth] != style.latexname()) {
				if (environment_stack.size() == depth + 1) {
					environment_stack.push_back("!-- --");
					environment_inner.push_back("!-- --");
				}
				environment_stack[depth] = style.latexname();
				environment_inner[depth] = "!-- --";
				buf->sgmlOpenTag(os, depth + command_depth,
						 environment_stack[depth]);
			} else {
				if (environment_inner[depth] != "!-- --") {
					item_name= "listitem";
					buf->sgmlCloseTag(os,
							  command_depth + depth,
							  item_name);
					if (environment_inner[depth] == "varlistentry")
						buf->sgmlCloseTag(os,
								  depth + command_depth,
								  environment_inner[depth]);
				}
			}
			
			if (style.latextype == LATEX_ENVIRONMENT) {
				if (!style.latexparam().empty()) {
					if (style.latexparam() == "CDATA")
						os << "<![CDATA[";
					else
						buf->sgmlOpenTag(os, depth + command_depth,
								 style.latexparam());
				}
				break;
			}

			desc_on = (style.labeltype == LABEL_MANUAL);

			if (desc_on)
				environment_inner[depth]= "varlistentry";
			else
				environment_inner[depth]= "listitem";

			buf->sgmlOpenTag(os, depth + 1 + command_depth,
					 environment_inner[depth]);

			if (desc_on) {
				item_name= "term";
				buf->sgmlOpenTag(os, depth + 1 + command_depth,
						 item_name);
			} else {
				item_name= "para";
				buf->sgmlOpenTag(os, depth + 1 + command_depth,
						 item_name);
			}
			break;
		default:
			buf->sgmlOpenTag(os, depth + command_depth,
					 style.latexname());
			break;
		}

		buf->simpleDocBookOnePar(os, p, desc_on,
					 depth + 1 + command_depth);
		p = p->next();

		string end_tag;
		// write closing SGML tags
		switch (style.latextype) {
		case LATEX_ENVIRONMENT:
			if (!style.latexparam().empty()) {
				if (style.latexparam() == "CDATA")
					os << "]]>";
				else
					buf->sgmlCloseTag(os, depth + command_depth,
							  style.latexparam());
			}
			break;
		case LATEX_ITEM_ENVIRONMENT:
			if (desc_on == 1) break;
			end_tag= "para";
			buf->sgmlCloseTag(os, depth + 1 + command_depth, end_tag);
			break;
		case LATEX_PARAGRAPH:
			buf->sgmlCloseTag(os, depth + command_depth, style.latexname());
			break;
		default:
			buf->sgmlCloseTag(os, depth + command_depth, style.latexname());
			break;
		}
	}

	// Close open tags
	for (int d = depth; d >= 0; --d) {
		if (!environment_stack[depth].empty()) {
			if (environment_inner[depth] != "!-- --") {
				item_name = "listitem";
				buf->sgmlCloseTag(os, command_depth + depth,
						  item_name);
                               if (environment_inner[depth] == "varlistentry")
				       buf->sgmlCloseTag(os, depth + command_depth,
							 environment_inner[depth]);
			}
			
			buf->sgmlCloseTag(os, depth + command_depth,
					  environment_stack[depth]);
		}
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
	if (the_locking_inset) {
		the_locking_inset->getCursorPos(bv, x, y);
		return;
	}
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

	LyXFont const font(getLyXText(bv)->getFont(bv->buffer(), cpar(bv), cpos(bv)));

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
		the_locking_inset->showInsetCursor(bv, show);
		return;
	}
	if (!isCursorVisible()) {
		LyXFont const font =
			getLyXText(bv)->getFont(bv->buffer(), cpar(bv), cpos(bv));
	
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


void InsetText::fitInsetCursor(BufferView * bv) const
{
	if (the_locking_inset) {
		the_locking_inset->fitInsetCursor(bv);
		return;
	}
	LyXFont const font =
		getLyXText(bv)->getFont(bv->buffer(), cpar(bv), cpos(bv));
	
	int const asc = lyxfont::maxAscent(font);
	int const desc = lyxfont::maxDescent(font);

	bv->fitLockedInsetCursor(cx(bv), cy(bv), asc, desc);
}


UpdatableInset::RESULT
InsetText::moveRight(BufferView * bv, bool activate_inset, bool selecting)
{
	if (getLyXText(bv)->cursor.par()->isRightToLeftPar(bv->buffer()->params))
		return moveLeftIntern(bv, false, activate_inset, selecting);
	else
		return moveRightIntern(bv, false, activate_inset, selecting);
}


UpdatableInset::RESULT
InsetText::moveLeft(BufferView * bv, bool activate_inset, bool selecting)
{
	if (getLyXText(bv)->cursor.par()->isRightToLeftPar(bv->buffer()->params))
		return moveRightIntern(bv, true, activate_inset, selecting);
	else
		return moveLeftIntern(bv, true, activate_inset, selecting);
}


UpdatableInset::RESULT
InsetText::moveRightIntern(BufferView * bv, bool behind, 
                           bool activate_inset, bool selecting)
{
	if (!cpar(bv)->next() && (cpos(bv) >= cpar(bv)->size()))
		return FINISHED_RIGHT;
	if (activate_inset && checkAndActivateInset(bv, behind))
		return DISPATCHED;
	getLyXText(bv)->cursorRight(bv);
	if (!selecting)
		getLyXText(bv)->selection.cursor = getLyXText(bv)->cursor;
	return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT
InsetText::moveLeftIntern(BufferView * bv, bool behind,
                          bool activate_inset, bool selecting)
{
	if (!cpar(bv)->previous() && (cpos(bv) <= 0))
		return FINISHED;
	getLyXText(bv)->cursorLeft(bv);
	if (!selecting)
		getLyXText(bv)->selection.cursor = getLyXText(bv)->cursor;
	if (activate_inset && checkAndActivateInset(bv, behind))
		return DISPATCHED;
	return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT
InsetText::moveUp(BufferView * bv)
{
	if (!crow(bv)->previous())
		return FINISHED_UP;
	getLyXText(bv)->cursorUp(bv);
	return DISPATCHED_NOUPDATE;
}


UpdatableInset::RESULT
InsetText::moveDown(BufferView * bv)
{
	if (!crow(bv)->next())
		return FINISHED_DOWN;
	getLyXText(bv)->cursorDown(bv);
	return DISPATCHED_NOUPDATE;
}


bool InsetText::insertInset(BufferView * bv, Inset * inset)
{
	if (the_locking_inset) {
		if (the_locking_inset->insetAllowed(inset))
			return the_locking_inset->insertInset(bv, inset);
		return false;
	}
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	setUndo(bv, Undo::FINISH, lt->cursor.par(), lt->cursor.par()->next());
	freezeUndo();
	inset->setOwner(this);
	hideInsetCursor(bv);
	lt->insertInset(bv, inset);
	bv->fitCursor();
	if (clear)
		lt = 0;
	updateLocal(bv, CURSOR_PAR|CURSOR, true);
	unFreezeUndo();
	return true;
}


bool InsetText::insetAllowed(Inset::Code code) const
{
	// in_insetAllowed is a really gross hack,
	// to allow us to call the owner's insetAllowed
	// without stack overflow, which can happen
	// when the owner uses InsetCollapsable::insetAllowed()
	bool ret = true;
	if (in_insetAllowed)
		return ret;
	in_insetAllowed = true;
	if (the_locking_inset)
		ret = the_locking_inset->insetAllowed(code);
	else if (owner())
		ret = owner()->insetAllowed(code);
	in_insetAllowed = false;
	return ret;
}


UpdatableInset * InsetText::getLockingInset() const
{
	return the_locking_inset ? the_locking_inset->getLockingInset() :
		const_cast<InsetText *>(this);
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


vector<string> const InsetText::getLabelList() const 
{
	vector<string> label_list;

	Paragraph * tpar = par;
	while (tpar) {
		Paragraph::inset_iterator beg = tpar->inset_iterator_begin();
		Paragraph::inset_iterator end = tpar->inset_iterator_end();
		for (; beg != end; ++beg) {
			vector<string> const l = (*beg)->getLabelList();
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
	if ((!par->next() && !par->size()) || !cpar(bv)->size()) {
		getLyXText(bv)->setFont(bv, font, toggleall);
		return;
	}
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	if (lt->selection.set()) {
		setUndo(bv, Undo::EDIT, lt->cursor.par(), lt->cursor.par()->next());
	}
	if (selectall)
		selectAll(bv);
	lt->toggleFree(bv, font, toggleall);
	if (selectall)
		lt->clearSelection();
	bv->fitCursor();
	bool flag = (selectall || lt->selection.set());
	if (clear)
		lt = 0;
	if (flag)
		updateLocal(bv, FULL, true);
	else
		updateLocal(bv, CURSOR_PAR, true);
}


bool InsetText::checkAndActivateInset(BufferView * bv, bool behind)
{
	if (cpar(bv)->isInset(cpos(bv))) {
		unsigned int x;
		unsigned int y;
		Inset * inset =
			static_cast<UpdatableInset*>(cpar(bv)->getInset(cpos(bv)));
		if (!isHighlyEditableInset(inset))
			return false;
		LyXFont const font =
			getLyXText(bv)->getFont(bv->buffer(), cpar(bv), cpos(bv));
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
	Inset * inset = bv->checkInsetHit(getLyXText(bv), dummyx, dummyy);

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
#if 0
	int w = UpdatableInset::getMaxWidth(bv, inset);
	if (w < 0) {
		return -1;
	}
	if (owner()) {
		w = w - top_x + owner()->x();
		return w;
	}
	w -= (2 * TEXT_TO_INSET_OFFSET);
	return w - top_x;
#else
	return UpdatableInset::getMaxWidth(bv, inset);
#endif
}


void InsetText::setParagraphData(Paragraph * p, bool same_id)
{
	// we have to unlock any locked inset otherwise we're in troubles
	the_locking_inset = 0;
	while (par) {
		Paragraph * tmp = par->next();
		delete par;
		par = tmp;
	}

	par = new Paragraph(*p, same_id);
	par->setInsetOwner(this);
	Paragraph * np = par;
	while (p->next()) {
		p = p->next();
		np->next(new Paragraph(*p, same_id));
		np->next()->previous(np);
		np = np->next();
		np->setInsetOwner(this);
	}
	reinitLyXText();
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
		if (!flag)
			removeNewlines();
		need_update = INIT;
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
	// we do nothing dangerous so we use a local cache
	LyXText * llt = getLyXText(bv);
	int x = llt->cursor.x() + top_x + TEXT_TO_INSET_OFFSET;
	if (the_locking_inset) {
		LyXFont font = llt->getFont(bv->buffer(), llt->cursor.par(),
		                            llt->cursor.pos());
		if (font.isVisibleRightToLeft())
			x -= the_locking_inset->width(bv, font);
	}
	return x;
}


int InsetText::cy(BufferView * bv) const
{
	LyXFont font;
	return getLyXText(bv)->cursor.y() - ascent(bv, font) + TEXT_TO_INSET_OFFSET;
}


pos_type InsetText::cpos(BufferView * bv) const
{
	return getLyXText(bv)->cursor.pos();
}


Paragraph * InsetText::cpar(BufferView * bv) const
{
	return getLyXText(bv)->cursor.par();
}


bool InsetText::cboundary(BufferView * bv) const
{
	return getLyXText(bv)->cursor.boundary();
}


Row * InsetText::crow(BufferView * bv) const
{
	return getLyXText(bv)->cursor.row();
}


LyXText * InsetText::getLyXText(BufferView const * lbv,
                                bool const recursive) const
{
	if (!recursive && (cached_bview == lbv)) {
		LyXText * lt = cached_text.get();
		lyx::Assert(lt && lt->firstRow()->par() == par);
		return lt;
	}
	
	// Super UGLY! (Lgb)
	BufferView * bv = const_cast<BufferView *>(lbv);
	
	cached_bview = bv;
	Cache::iterator it = cache.find(bv);

	if (it != cache.end()) {
		if (do_reinit) {
			reinitLyXText();
		} else if (do_resize) {
			resizeLyXText(do_resize);
		} else {
			if (lt || !it->second.remove) {
				lyx::Assert(it->second.text.get());
				cached_text = it->second.text;
				if (recursive && the_locking_inset) {
					return the_locking_inset->getLyXText(bv, true);
				}
				return cached_text.get();
			} else if (it->second.remove) {
				if (locked) {
					saveLyXTextState(it->second.text.get());
				} else {
					sstate.lpar = 0;
				}
			}
			//
			// when we have to reinit the existing LyXText!
			//
			it->second.text->init(bv);
			restoreLyXTextState(bv, it->second.text.get());
			it->second.remove = false;
		}
		cached_text = it->second.text;
		if (the_locking_inset && recursive) {
			return the_locking_inset->getLyXText(bv);
		}
		return cached_text.get();
	}
	///
	// we are here only if we don't have a BufferView * in the cache!!!
	///
	cached_text.reset(new LyXText(const_cast<InsetText *>(this)));
	cached_text->init(bv);
	restoreLyXTextState(bv, cached_text.get());

	cache.insert(make_pair(bv, cached_text));
	
	if (the_locking_inset && recursive) {
		return the_locking_inset->getLyXText(bv);
	}
	return cached_text.get();
}


void InsetText::deleteLyXText(BufferView * bv, bool recursive) const
{
	cached_bview = 0;

	Cache::iterator it = cache.find(bv);
	
	if (it == cache.end()) {
		return;
	}

	lyx::Assert(it->second.text.get());

	it->second.remove = true;
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
	if (lt) {
		// we cannot resize this because we are in use!
		// so do this on the next possible getLyXText()
		do_resize = bv;
		return;
	}
	do_resize = 0;
//	lyxerr << "InsetText::resizeLyXText\n";
	if (!par->next() && !par->size()) { // no data, resize not neccessary!
		// we have to do this as a fixed width may have changed!
		LyXText * t = getLyXText(bv);
		saveLyXTextState(t);
		t->init(bv, true);
		restoreLyXTextState(bv, t);
		return;
	}
	// one endless line, resize normally not necessary
	if (!force && getMaxWidth(bv, this) < 0)
		return;

	Cache::iterator it = cache.find(bv);
	if (it == cache.end()) {
		return;
	}
	lyx::Assert(it->second.text.get());

	LyXText * t = it->second.text.get();
	saveLyXTextState(t);
	for (Paragraph * p = par; p; p = p->next()) {
		p->resizeInsetsLyXText(bv);
	}
	t->init(bv, true);
	restoreLyXTextState(bv, t);
	if (the_locking_inset) {
		inset_x = cx(bv) - top_x + drawTextXOffset;
		inset_y = cy(bv) + drawTextYOffset;
	}

	if (bv->screen()) {
		t->first = bv->screen()->topCursorVisible(t);
	}
	if (!owner()) {
		updateLocal(bv, FULL, false);
		// this will scroll the screen such that the cursor becomes visible 
		bv->updateScrollbar();
	} else {
		need_update |= FULL;
	}
}


void InsetText::reinitLyXText() const
{
	if (lt) {
		// we cannot resize this because we are in use!
		// so do this on the next possible getLyXText()
		do_reinit = true;
		return;
	}
	do_reinit = false;
	do_resize = 0;
//	lyxerr << "InsetText::reinitLyXText\n";
	for(Cache::iterator it = cache.begin(); it != cache.end(); ++it) {
		lyx::Assert(it->second.text.get());

		LyXText * t = it->second.text.get();
		BufferView * bv = it->first;

		saveLyXTextState(t);
		for (Paragraph * p = par; p; p = p->next()) {
			p->resizeInsetsLyXText(bv);
		}
		t->init(bv, true);
		restoreLyXTextState(bv, t);
		if (the_locking_inset) {
			inset_x = cx(bv) - top_x + drawTextXOffset;
			inset_y = cy(bv) + drawTextYOffset;
		}
		if (bv->screen()) {
			t->first = bv->screen()->topCursorVisible(t);
		}
		if (!owner()) {
			updateLocal(bv, FULL, false);
			// this will scroll the screen such that the cursor becomes visible 
			bv->updateScrollbar();
		} else {
			need_update = FULL;
		}
	}
}


void InsetText::removeNewlines()
{
	bool changed = false;
	
	for (Paragraph * p = par; p; p = p->next()) {
		for (int i = 0; i < p->size(); ++i) {
			if (p->getChar(i) == Paragraph::META_NEWLINE) {
				changed = true;
				p->erase(i);
			}
		}
	}
	if (changed)
		reinitLyXText();
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
	getLyXText(bv)->cursorTop(bv);
	getLyXText(bv)->selection.cursor = getLyXText(bv)->cursor;
	getLyXText(bv)->cursorBottom(bv);
	getLyXText(bv)->setSelection(bv);
}


void InsetText::clearSelection(BufferView * bv)
{
	getLyXText(bv)->clearSelection();
}


void InsetText::clearInset(BufferView * bv, int baseline, bool & cleared) const
{
	Painter & pain = bv->painter();
	int w = insetWidth;
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
//	w -= TEXT_TO_INSET_OFFSET;
	pain.fillRectangle(top_x, ty, w+1, h+1, backgroundColor());
	cleared = true;
	need_update = FULL;
	frame_is_visible = false;
}


Paragraph * InsetText::getParFromID(int id) const
{
#if 0
	Paragraph * result = par;
	Paragraph * ires = 0;
	while (result && result->id() != id) {
		if ((ires = result->getParFromID(id)))
			return ires;
		result = result->next();
	}
	return result;
#else
	Paragraph * tmp = par;
	while (tmp) {
		if (tmp->id() == id) {
			return tmp;
		}
		Paragraph * tmp2 = tmp->getParFromID(id);
		if (tmp2 != 0) {
			return tmp2;
		}
		tmp = tmp->next();
	}
	return 0;
#endif
}


Paragraph * InsetText::firstParagraph() const
{
	Paragraph * result;
	if (the_locking_inset)
		if ((result = the_locking_inset->firstParagraph()))
			return result;
	return par;
}


Paragraph * InsetText::getFirstParagraph(int i) const
{
	return (i == 0) ? par : 0;
}


LyXCursor const & InsetText::cursor(BufferView * bv) const
{
		if (the_locking_inset)
				return the_locking_inset->cursor(bv);
		return getLyXText(bv)->cursor;
}


Paragraph * InsetText::paragraph() const
{
	return par;
}


void InsetText::paragraph(Paragraph * p)
{
	// GENERAL COMMENT: We don't have to free the old paragraphs as the
	// caller of this function has to take care of it. This IS important
	// as we could have to insert a paragraph before this one and just
	// link the actual to a new ones next and set it with this function
	// and are done!
	par = p;
	// set ourself as owner for all the paragraphs inserted!
	Paragraph * np = par;
	while (np) {
		np->setInsetOwner(this);
		np = np->next();
	}
	reinitLyXText();
	// redraw myself when asked for
	need_update = INIT;
}


Inset * InsetText::getInsetFromID(int id_arg) const
{
	if (id_arg == id())
		return const_cast<InsetText *>(this);

	Paragraph * lp = par;

	while (lp) {
		for (Paragraph::inset_iterator it = lp->inset_iterator_begin(),
			 en = lp->inset_iterator_end();
			 it != en; ++it)
		{
			if ((*it)->id() == id_arg)
				return *it;
			Inset * in = (*it)->getInsetFromID(id_arg);
			if (in)
				return in;
		}
		lp = lp->next();
	}
	return 0;
}


string const InsetText::selectNextWordToSpellcheck(BufferView * bv, float & value) const
{
	bool clear = false;
	string str;

	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	if (the_locking_inset) {
		str = the_locking_inset->selectNextWordToSpellcheck(bv, value);
		if (!str.empty()) {
			value += cy(bv);
			if (clear)
				lt = 0;
			return str;
		}
		// we have to go on checking so move cusor to the next char
		lt->cursor.pos(lt->cursor.pos() + 1);
	}
	str = lt->selectNextWordToSpellcheck(bv, value);
	if (str.empty())
		bv->unlockInset(const_cast<InsetText *>(this));
	else
		value = cy(bv);
	if (clear)
		lt = 0;
	return str;
}


void InsetText::selectSelectedWord(BufferView * bv)
{
	if (the_locking_inset) {
		the_locking_inset->selectSelectedWord(bv);
		return;
	}
	getLyXText(bv)->selectSelectedWord(bv);
	updateLocal(bv, SELECTION, false);
}


void InsetText::toggleSelection(BufferView * bv, bool kill_selection)
{
	if (the_locking_inset) {
		the_locking_inset->toggleSelection(bv, kill_selection);
	}
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}

	int x = top_x + TEXT_TO_INSET_OFFSET;

	Row * row = lt->firstRow();
	int y_offset = top_baseline - row->ascent_of_text();
	int y = y_offset;
	while ((row != 0) && ((y+row->height()) <= 0)) {
		y += row->height();
		row = row->next();
	}
	if (y_offset < 0)
		y_offset = y;
	
	if (need_update & SELECTION)
		need_update = NONE;
	bv->screen()->toggleSelection(lt, bv, kill_selection, y_offset, x);
	if (clear)
		lt = 0;
}


bool InsetText::searchForward(BufferView * bv, string const & str,
                              bool cs, bool mw)
{
	if (the_locking_inset) {
		if (the_locking_inset->searchForward(bv, str, cs, mw))
			return true;
		bool clear = false;
		if (!lt) {
			lt = getLyXText(bv);
			clear = true;
		}
		Paragraph * lpar = lt->cursor.par();
		pos_type pos = lt->cursor.pos();
		if (pos < lpar->size() - 1)
			++pos;
		else {
			pos = 0;
			lpar = lpar->next();
		}
		if (!lpar) {
			if (clear)
				lt = 0;
			// we have to unlock ourself in this function by default!
			bv->unlockInset(const_cast<InsetText *>(this));
			return false;
		}
		lt->setCursor(bv, lpar, pos);
		if (clear)
			lt = 0;
	}
	if (LyXFind(bv, str, true, true, cs , mw)) {
		return true;
	}
	// we have to unlock ourself in this function by default!
	bv->unlockInset(const_cast<InsetText *>(this));
	return false;
}

bool InsetText::searchBackward(BufferView * bv, string const & str,
                               bool cs, bool mw)
{
	if (the_locking_inset)
		if (the_locking_inset->searchBackward(bv, str, cs, mw))
			return true;
	if (LyXFind(bv, str, false, true, cs, mw)) {
		return true;
	}
	// we have to unlock ourself in this function by default!
	bv->unlockInset(const_cast<InsetText *>(this));
	return false;
}


bool InsetText::checkInsertChar(LyXFont & font)
{
	if (owner())
		return owner()->checkInsertChar(font);
	return true;
}


void InsetText::collapseParagraphs(BufferParams const & bparams) const
{
	while(par->next()) {
		if (!par->isSeparator(par->size()-1))
			par->insertChar(par->size()-1, ' ');
		par->pasteParagraph(bparams);
	}
	reinitLyXText();
}
