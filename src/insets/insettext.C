/**
 * \file insettext.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "insettext.h"

#include "buffer.h"
#include "BufferView.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"
#include "intl.h"
#include "LaTeXFeatures.h"
#include "LColor.h"
#include "lyxfont.h"
#include "lyxcursor.h"
#include "lyxfind.h"
#include "lyxlex.h"
#include "lyxrow.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "paragraph.h"
#include "ParagraphParameters.h"
#include "trans_mgr.h"
#include "undo_funcs.h"
#include "WordLangTuple.h"
#include "paragraph_funcs.h"
#include "sgml.h"
#include "rowpainter.h"
#include "insetnewline.h"
#include "Lsstream.h"

#include "frontends/Alert.h"
#include "frontends/Dialogs.h"
#include "frontends/font_metrics.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"
#include "frontends/screen.h"

#include "support/textutils.h"
#include "support/LAssert.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h" // lyx::count

#include <boost/bind.hpp>

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
using std::for_each;

using lyx::pos_type;
using lyx::textclass_type;

// These functions should probably go into bufferview_funcs somehow (Jug)

void InsetText::saveLyXTextState(LyXText * t) const
{
	// check if my paragraphs are still valid
	ParagraphList::iterator it = paragraphs.begin();
	ParagraphList::iterator end = paragraphs.end();
	for (; it != end; ++it) {
		if (&*it == t->cursor.par())
			break;
	}

	if (it != end && t->cursor.pos() <= it->size()) {
		sstate.lpar = &*t->cursor.par();
		sstate.pos = t->cursor.pos();
		sstate.boundary = t->cursor.boundary();
		sstate.selstartpar = &*t->selection.start.par();
		sstate.selstartpos = t->selection.start.pos();
		sstate.selstartboundary = t->selection.start.boundary();
		sstate.selendpar = &*t->selection.end.par();
		sstate.selendpos = t->selection.end.pos();
		sstate.selendboundary = t->selection.end.boundary();
		sstate.selection = t->selection.set();
		sstate.mark_set = t->selection.mark();
	} else {
		sstate.lpar = 0;
	}
}


void InsetText::restoreLyXTextState(LyXText * t) const
{
	if (!sstate.lpar)
		return;

	t->selection.set(true);
	/* at this point just to avoid the DEPM when setting the cursor */
	t->selection.mark(sstate.mark_set);
	if (sstate.selection) {
		t->setCursor(sstate.selstartpar, sstate.selstartpos,
			     true, sstate.selstartboundary);
		t->selection.cursor = t->cursor;
		t->setCursor(sstate.selendpar, sstate.selendpos,
			     true, sstate.selendboundary);
		t->setSelection();
		t->setCursor(sstate.lpar, sstate.pos);
	} else {
		t->setCursor(sstate.lpar, sstate.pos, true, sstate.boundary);
		t->selection.cursor = t->cursor;
		t->selection.set(false);
	}
}


InsetText::InnerCache::InnerCache(boost::shared_ptr<LyXText> t)
{
	text = t;
	remove = false;
}


InsetText::InsetText(BufferParams const & bp)
	: UpdatableInset(), lt(0), in_update(false), do_resize(0),
	  do_reinit(false)
{
	paragraphs.push_back(new Paragraph);
	paragraphs.begin()->layout(bp.getLyXTextClass().defaultLayout());
	if (bp.tracking_changes)
		paragraphs.begin()->trackChanges();
	init();
}


InsetText::InsetText(InsetText const & in, bool same_id)
	: UpdatableInset(in, same_id), lt(0), in_update(false), do_resize(0),
	  do_reinit(false)
{
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
		setParagraphData(ins->paragraphs, same_id);
		autoBreakRows = ins->autoBreakRows;
		drawFrame_ = ins->drawFrame_;
		frame_color = ins->frame_color;
		if (same_id)
			id_ = ins->id_;
	} else {
		for_each(paragraphs.begin(), paragraphs.end(),
			 boost::bind(&Paragraph::setInsetOwner, _1, this));

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
	no_selection = true;
	need_update = FULL;
	drawTextXOffset = 0;
	drawTextYOffset = 0;
	locked = false;
	old_par = 0;
	last_drawn_width = -1;
	cached_bview = 0;
	sstate.lpar = 0;
	in_insetAllowed = false;
}


InsetText::~InsetText()
{
	paragraphs.clear();
}


void InsetText::clear(bool just_mark_erased)
{
	if (just_mark_erased) {
		ParagraphList::iterator it = paragraphs.begin();
		ParagraphList::iterator end = paragraphs.end();
		for (; it != end; ++it) {
			it->markErased();
		}
		need_update = FULL;
		return;
	}

	// This is a gross hack...
	LyXLayout_ptr old_layout = paragraphs.begin()->layout();

	paragraphs.clear();
	paragraphs.push_back(new Paragraph);
	paragraphs.begin()->setInsetOwner(this);
	paragraphs.begin()->layout(old_layout);

	reinitLyXText();
	need_update = INIT;
}


Inset * InsetText::clone(Buffer const &, bool same_id) const
{
	return new InsetText(*this, same_id);
}


void InsetText::write(Buffer const * buf, ostream & os) const
{
	os << "Text\n";
	writeParagraphData(buf, os);
}


void InsetText::writeParagraphData(Buffer const * buf, ostream & os) const
{
	ParagraphList::iterator it = paragraphs.begin();
	ParagraphList::iterator end = paragraphs.end();
	Paragraph::depth_type dth = 0;
	for (; it != end; ++it) {
		it->write(buf, os, buf->params, dth);
	}
}


void InsetText::read(Buffer const * buf, LyXLex & lex)
{
	string token;
	Paragraph::depth_type depth = 0;

	clear(false);

	if (buf->params.tracking_changes)
		paragraphs.begin()->trackChanges();

	// delete the initial paragraph
	paragraphs.clear();
	ParagraphList::iterator pit = paragraphs.begin();

	while (lex.isOK()) {
		lex.nextToken();
		token = lex.getString();
		if (token.empty())
			continue;
		if (token == "\\end_inset") {
			break;
		}

		if (token == "\\the_end") {
			lex.printError("\\the_end read in inset! Error in document!");
			return;
		}

		// FIXME: ugly.
		const_cast<Buffer*>(buf)->readParagraph(lex, token, paragraphs, pit, depth);
	}

	pit = paragraphs.begin();
	ParagraphList::iterator const end = paragraphs.end();
	for (; pit != end; ++pit)
		pit->setInsetOwner(this);

	if (token != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point. "
					   "Read: `$$Token'");
	}
	need_update = FULL;
}


int InsetText::ascent(BufferView * bv, LyXFont const &) const
{
	insetAscent = getLyXText(bv)->rows().begin()->ascent_of_text() +
		TEXT_TO_INSET_OFFSET;
	return insetAscent;
}


int InsetText::descent(BufferView * bv, LyXFont const &) const
{
	LyXText * llt = getLyXText(bv);
	insetDescent = llt->height - llt->rows().begin()->ascent_of_text() +
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
		     int baseline, float & x) const
{
	if (nodraw())
		return;

	// update our idea of where we are. Clearly, we should
	// not have to know this information.
	if (top_x != int(x))
		top_x = int(x);

	int const start_x = int(x);

	Painter & pain = bv->painter();

	// call these methods so that insetWidth, insetAscent and
	// insetDescent have the right values.
	width(bv, f);
	ascent(bv, f);
	descent(bv, f);

	// repaint the background if needed
	if (backgroundColor() != LColor::background)
		clearInset(bv, start_x + TEXT_TO_INSET_OFFSET, baseline);

	// no draw is necessary !!!
	if ((drawFrame_ == LOCKED) && !locked && paragraphs.begin()->empty()) {
		top_baseline = baseline;
		x += width(bv, f);
		need_update = NONE;
		return;
	}

	if (!owner())
		x += static_cast<float>(scroll());

	top_baseline = baseline;
	top_y = baseline - insetAscent;

	if (last_drawn_width != insetWidth) {
		need_update |= FULL;
		last_drawn_width = insetWidth;
	}

	if (the_locking_inset && (cpar(bv) == inset_par)
		&& (cpos(bv) == inset_pos)) {
		inset_x = cix(bv) - int(x) + drawTextXOffset;
		inset_y = ciy(bv) + drawTextYOffset;
	}

	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	x += TEXT_TO_INSET_OFFSET;

	RowList::iterator rowit = lt->rows().begin();
	RowList::iterator end = lt->rows().end();

	int y_offset = baseline - rowit->ascent_of_text();
	int ph = pain.paperHeight();
	int first = 0;
	int y = y_offset;
	while ((rowit != end) && ((y + rowit->height()) <= 0)) {
		y += rowit->height();
		first += rowit->height();
		++rowit;
	}
	if (y_offset < 0) {
		lt->top_y(-y_offset);
		first = y;
		y_offset = 0;
	} else {
		lt->top_y(first);
		first = 0;
	}

	int yf = y_offset + first;
	y = 0;

	bv->hideCursor();

	while ((rowit != end) && (yf < ph)) {
		RowPainter rp(*bv, *lt, rowit);
		rp.paint(y + y_offset + first, int(x), y + lt->top_y());
		y += rowit->height();
		yf += rowit->height();
		++rowit;
	}

	lt->clearPaint();

	if ((drawFrame_ == ALWAYS) || (drawFrame_ == LOCKED && locked)) {
		drawFrame(pain, int(start_x));
	}

	x += insetWidth - TEXT_TO_INSET_OFFSET;

	if (need_update != INIT) {
		need_update = NONE;
	}
	if (clear)
		lt = 0;
}


void InsetText::drawFrame(Painter & pain, int x) const
{
	static int const ttoD2 = TEXT_TO_INSET_OFFSET / 2;
	frame_x = x + ttoD2;
	frame_y = top_baseline - insetAscent + ttoD2;
	frame_w = insetWidth - TEXT_TO_INSET_OFFSET;
	frame_h = insetAscent + insetDescent - TEXT_TO_INSET_OFFSET;
	pain.rectangle(frame_x, frame_y, frame_w, frame_h,
		       frame_color);
}


void InsetText::update(BufferView * bv, bool reinit)
{
	if (in_update) {
		if (reinit && owner()) {
			reinitLyXText();
			owner()->update(bv, true);
		}
		return;
	}
	in_update = true;

	if (reinit || need_update == INIT) {
		need_update = FULL;
		// we should put this call where we set need_update to INIT!
		reinitLyXText();
		if (owner())
			owner()->update(bv, true);
		in_update = false;

		int nw = getMaxWidth(bv, this);
		if (nw > 0 && old_max_width != nw) {
			need_update |= INIT;
			old_max_width = nw;
		}
		return;
	}

	if (!autoBreakRows &&
	    boost::next(paragraphs.begin()) != paragraphs.end())
		collapseParagraphs(bv);

	if (the_locking_inset) {
		inset_x = cix(bv) - top_x + drawTextXOffset;
		inset_y = ciy(bv) + drawTextYOffset;
		the_locking_inset->update(bv, reinit);
	}

	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	if ((need_update & CURSOR_PAR) && (lt->refreshStatus() == LyXText::REFRESH_NONE) &&
		the_locking_inset) {
		lt->updateInset(the_locking_inset);
	}

	if (lt->refreshStatus() == LyXText::REFRESH_AREA)
		need_update |= FULL;
	if (clear)
		lt = 0;

	in_update = false;

	int nw = getMaxWidth(bv, this);
	if (nw > 0 && old_max_width != nw) {
		need_update |= INIT;
		old_max_width = nw;
	}
}


void InsetText::setUpdateStatus(BufferView * bv, int what) const
{
	// this does nothing dangerous so use only a localized buffer
	LyXText * llt = getLyXText(bv);

	need_update |= what;
	// we have to redraw us full if our LyXText REFRESH_AREA or
	// if we don't break row so that we only have one row to update!
	if ((llt->refreshStatus() == LyXText::REFRESH_AREA) ||
	    (!autoBreakRows &&
	     (llt->refreshStatus() == LyXText::REFRESH_ROW)))
	{
		need_update |= FULL;
	} else if (llt->refreshStatus() == LyXText::REFRESH_ROW) {
		need_update |= CURSOR_PAR;
	}

	// this to not draw a selection when we redraw all of it!
	if (need_update & CURSOR && !(need_update & SELECTION)) {
		if (llt->selection.set())
			need_update = FULL;
		llt->clearSelection();
	}
}


void InsetText::updateLocal(BufferView * bv, int what, bool mark_dirty)
{
#if 0
	if (!autoBreakRows &&
	    boost::next(paragraphs.begin()) != paragraphs.end())
		collapseParagraphs(bv);
#else
	if (!autoBreakRows && paragraphs.size() > 1)
		collapseParagraphs(bv);
#endif

	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	lt->fullRebreak();
	setUpdateStatus(bv, what);
	bool flag = mark_dirty ||
		(((need_update != CURSOR) && (need_update != NONE)) ||
		 (lt->refreshStatus() != LyXText::REFRESH_NONE) || lt->selection.set());
	if (!lt->selection.set())
		lt->selection.cursor = lt->cursor;

	bv->fitCursor();

	if (flag) {
		lt->postPaint(0);
		bv->updateInset(const_cast<InsetText *>(this));
	}

	if (clear)
		lt = 0;

	if (need_update == CURSOR)
		need_update = NONE;
	bv->owner()->view_state_changed();
	bv->owner()->updateMenubar();
	bv->owner()->updateToolbar();
	if (old_par != cpar(bv)) {
		bv->owner()->setLayout(cpar(bv)->layout()->name());
		old_par = cpar(bv);
	}
}


string const InsetText::editMessage() const
{
	return _("Opened Text Inset");
}


void InsetText::edit(BufferView * bv, int x, int y, mouse_button::state button)
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
	// we put here -1 and not button as now the button in the
	// edit call should not be needed we will fix this in 1.3.x
	// cycle hopefully (Jug 20020509)
	// FIXME: GUII I've changed this to none: probably WRONG
	if (!checkAndActivateInset(bv, x, tmp_y, mouse_button::none)) {
		lt->setCursorFromCoordinates(x - drawTextXOffset,
					    y + insetAscent);
		lt->cursor.x_fix(lt->cursor.x());
	}
	lt->clearSelection();
	finishUndo();
	// If the inset is empty set the language of the current font to the
	// language to the surronding text (if different).
	if (paragraphs.begin()->empty() &&
	    boost::next(paragraphs.begin()) == paragraphs.end()&&
		bv->getParentLanguage(this) != lt->current_font.language())
	{
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(bv->getParentLanguage(this));
		setFont(bv, font, false);
	}
	if (clear)
		lt = 0;

	int code = CURSOR;
	if (drawFrame_ == LOCKED)
		code = CURSOR|DRAW_FRAME;
	updateLocal(bv, code, false);

	// Tell the paragraph dialog that we've entered an insettext.
	bv->dispatch(FuncRequest(LFUN_PARAGRAPH_UPDATE));
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
		lt->setCursor(paragraphs.begin(), 0);
	else {
		ParagraphList::iterator it = paragraphs.begin();
		ParagraphList::iterator end = paragraphs.end();
		while (boost::next(it) != end)
			++it;
//		int const pos = (p->size() ? p->size()-1 : p->size());
		lt->setCursor(it, it->size());
	}
	lt->clearSelection();
	finishUndo();
	// If the inset is empty set the language of the current font to the
	// language to the surronding text (if different).
	if (paragraphs.begin()->empty() &&
	    boost::next(paragraphs.begin()) == paragraphs.end() &&
		bv->getParentLanguage(this) != lt->current_font.language()) {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(bv->getParentLanguage(this));
		setFont(bv, font, false);
	}
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
		updateLocal(bv, CURSOR_PAR, false);
	}
	no_selection = true;
	locked = false;
	int code = NONE;
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
				       ->cursor.par()->layout()->name());
	} else
		bv->owner()->setLayout(bv->text->cursor.par()->layout()->name());
	// hack for deleteEmptyParMech
	if (!paragraphs.begin()->empty()) {
		lt->setCursor(paragraphs.begin(), 0);
	} else if (boost::next(paragraphs.begin()) != paragraphs.end()) {
		lt->setCursor(boost::next(paragraphs.begin()), 0);
	}
	if (clear)
		lt = 0;
#if 0
	updateLocal(bv, code, false);
#else
	if (code != NONE)
		setUpdateStatus(bv, code);
#endif
}


void InsetText::lockInset(BufferView * bv)
{
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
	lt->setCursor(paragraphs.begin(), 0);
	lt->clearSelection();
	finishUndo();
	// If the inset is empty set the language of the current font to the
	// language to the surronding text (if different).
	if (paragraphs.begin()->empty() &&
	    boost::next(paragraphs.begin()) == paragraphs.end() &&
		bv->getParentLanguage(this) != lt->current_font.language()) {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(bv->getParentLanguage(this));
		setFont(bv, font, false);
	}
	if (clear)
		lt = 0;
	int code = CURSOR;
	if (drawFrame_ == LOCKED)
		code = CURSOR|DRAW_FRAME;
	setUpdateStatus(bv, code);
}


void InsetText::lockInset(BufferView * bv, UpdatableInset * inset)
{
	the_locking_inset = inset;
	inset_x = cix(bv) - top_x + drawTextXOffset;
	inset_y = ciy(bv) + drawTextYOffset;
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
		ParagraphList::iterator pit = paragraphs.begin();
		ParagraphList::iterator pend = paragraphs.end();

		int const id = inset->id();
		for (; pit != pend; ++pit) {
			InsetList::iterator it =
				pit->insetlist.begin();
			InsetList::iterator const end =
				pit->insetlist.end();
			for (; it != end; ++it) {
				if (it.getInset() == inset) {
					getLyXText(bv)->setCursorIntern(pit, it.getPos());
					lockInset(bv, inset);
					return true;
				}
				if (it.getInset()->getInsetFromID(id)) {
					getLyXText(bv)->setCursorIntern(pit, it.getPos());
					it.getInset()->edit(bv);
					return the_locking_inset->lockInsetInInset(bv, inset);
				}
			}
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
			inset_x = cix(bv) - top_x + drawTextXOffset;
			inset_y = ciy(bv) + drawTextYOffset;
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
		getLyXText(bv)->updateInset(inset);
		the_locking_inset = 0;
		if (lr)
			moveRightIntern(bv, true, false);
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
	if (!autoBreakRows &&
	    boost::next(paragraphs.begin()) != paragraphs.end())
		collapseParagraphs(bv);
	if (inset == this)
		return true;
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	if (inset->owner() != this) {
		int ustat = CURSOR_PAR;
		bool found = false;
		UpdatableInset * tl_inset = the_locking_inset;
		if (tl_inset)
			found = tl_inset->updateInsetInInset(bv, inset);
		if (!found) {
			tl_inset = static_cast<UpdatableInset *>(inset);
			while(tl_inset->owner() && tl_inset->owner() != this)
				tl_inset = static_cast<UpdatableInset *>(tl_inset->owner());
			if (!tl_inset->owner())
				return false;
			found = tl_inset->updateInsetInInset(bv, inset);
			ustat = FULL;
		}
		if (found)
			lt->updateInset(tl_inset);
		if (clear)
			lt = 0;
		if (found)
			setUpdateStatus(bv, ustat);
		return found;
	}
	bool found = lt->updateInset(inset);
	if (clear)
		lt = 0;
	if (found) {
		setUpdateStatus(bv, CURSOR_PAR);
		if (the_locking_inset &&
		    cpar(bv) == inset_par && cpos(bv) == inset_pos)
		{
			inset_x = cix(bv) - top_x + drawTextXOffset;
			inset_y = ciy(bv) + drawTextYOffset;
		}
	}
	return found;
}


void InsetText::lfunMousePress(FuncRequest const & cmd)
{
	no_selection = true;

	// use this to check mouse motion for selection!
	mouse_x = cmd.x;
	mouse_y = cmd.y;

	BufferView * bv = cmd.view();
	FuncRequest cmd1 = cmd;
	cmd1.x -= inset_x;
	cmd1.y -= inset_y;
	if (!locked)
		lockInset(bv);

	int tmp_x = cmd.x - drawTextXOffset;
	int tmp_y = cmd.y + insetAscent - getLyXText(bv)->top_y();
	Inset * inset = getLyXText(bv)->checkInsetHit(tmp_x, tmp_y);

	if (the_locking_inset) {
		if (the_locking_inset == inset) {
			the_locking_inset->localDispatch(cmd1);
			return;
		}
		// otherwise only unlock the_locking_inset
		the_locking_inset->insetUnlock(bv);
		the_locking_inset = 0;
	}
	if (!inset)
		no_selection = false;

	if (bv->theLockingInset()) {
		if (isHighlyEditableInset(inset)) {
			// We just have to lock the inset before calling a
			// PressEvent on it!
			UpdatableInset * uinset = static_cast<UpdatableInset*>(inset);
			if (!bv->lockInset(uinset)) {
				lyxerr[Debug::INSETS] << "Cannot lock inset" << endl;
			}
			inset->localDispatch(cmd1);
			if (the_locking_inset)
				updateLocal(bv, CURSOR, false);
			return;
		}
	}
	if (!inset) {
		bool paste_internally = false;
		if (cmd.button() == mouse_button::button2 && getLyXText(bv)->selection.set()) {
			localDispatch(FuncRequest(bv, LFUN_COPY));
			paste_internally = true;
		}
		bool clear = false;
		if (!lt) {
			lt = getLyXText(bv);
			clear = true;
		}
		int old_top_y = lt->top_y();

		lt->setCursorFromCoordinates(cmd.x - drawTextXOffset,
					     cmd.y + insetAscent);
		// set the selection cursor!
		lt->selection.cursor = lt->cursor;
		lt->cursor.x_fix(lt->cursor.x());

		if (lt->selection.set()) {
			lt->clearSelection();
			updateLocal(bv, FULL, false);
		} else {
			lt->clearSelection();
			updateLocal(bv, CURSOR, false);
		}

		if (clear)
			lt = 0;

		bv->owner()->setLayout(cpar(bv)->layout()->name());

		// we moved the view we cannot do mouse selection in this case!
		if (getLyXText(bv)->top_y() != old_top_y)
			no_selection = true;
		old_par = cpar(bv);
		// Insert primary selection with middle mouse
		// if there is a local selection in the current buffer,
		// insert this
		if (cmd.button() == mouse_button::button2) {
			if (paste_internally)
				localDispatch(FuncRequest(bv, LFUN_PASTE));
			else
				localDispatch(FuncRequest(bv, LFUN_PASTESELECTION, "paragraph"));
		}
	} else {
		getLyXText(bv)->clearSelection();
	}
}


bool InsetText::lfunMouseRelease(FuncRequest const & cmd)
{
	BufferView * bv = cmd.view();
	FuncRequest cmd1 = cmd;
	cmd1.x -= inset_x;
	cmd1.y -= inset_y;

	no_selection = true;
	if (the_locking_inset)
		return the_locking_inset->localDispatch(cmd1);

	int tmp_x = cmd.x - drawTextXOffset;
	int tmp_y = cmd.y + insetAscent - getLyXText(bv)->top_y();
	Inset * inset = getLyXText(bv)->checkInsetHit(tmp_x, tmp_y);
	bool ret = false;
	if (inset) {
// This code should probably be removed now. Simple insets
// (!highlyEditable) can actually take the localDispatch,
// and turn it into edit() if necessary. But we still
// need to deal properly with the whole relative vs.
// absolute mouse co-ords thing in a realiable, sensible way
#if 0
		if (isHighlyEditableInset(inset))
			ret = inset->localDispatch(cmd1);
		else {
			inset_x = cix(bv) - top_x + drawTextXOffset;
			inset_y = ciy(bv) + drawTextYOffset;
			cmd1.x = cmd.x - inset_x;
			cmd1.y = cmd.x - inset_y;
			inset->edit(bv, cmd1.x, cmd1.y, cmd.button());
			ret = true;
		}
#endif
		ret = inset->localDispatch(cmd1);
		updateLocal(bv, CURSOR_PAR, false);

	}
	return ret;
}


void InsetText::lfunMouseMotion(FuncRequest const & cmd)
{
	FuncRequest cmd1 = cmd;
	cmd1.x -= inset_x;
	cmd1.y -= inset_y;

	if (the_locking_inset) {
		the_locking_inset->localDispatch(cmd1);
		return;
	}

	if (no_selection || (mouse_x == cmd.x && mouse_y == cmd.y))
		return;

	BufferView * bv = cmd.view();
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	LyXCursor cur = lt->cursor;
	lt->setCursorFromCoordinates
		(cmd.x - drawTextXOffset, cmd.y + insetAscent);
	lt->cursor.x_fix(lt->cursor.x());
	if (cur == lt->cursor) {
		if (clear)
			lt = 0;
		return;
	}
	lt->setSelection();
	bool flag = (lt->toggle_cursor.par() != lt->toggle_end_cursor.par() ||
				 lt->toggle_cursor.pos() != lt->toggle_end_cursor.pos());
	if (clear)
		lt = 0;
	if (flag) {
		updateLocal(bv, SELECTION, false);
	}
}


Inset::RESULT InsetText::localDispatch(FuncRequest const & ev)
{
	BufferView * bv = ev.view();
	switch (ev.action) {
		case LFUN_MOUSE_PRESS:
			lfunMousePress(ev);
			return DISPATCHED;
		case LFUN_MOUSE_MOTION:
			lfunMouseMotion(ev);
			return DISPATCHED;
		case LFUN_MOUSE_RELEASE:
			return lfunMouseRelease(ev) ? DISPATCHED : UNDISPATCHED;
		default:
			break;
	}

	bool was_empty = (paragraphs.begin()->empty() &&
			  boost::next(paragraphs.begin()) == paragraphs.end());
	no_selection = false;
	RESULT result = UpdatableInset::localDispatch(ev);
	if (result != UNDISPATCHED)
		return DISPATCHED;

	result = DISPATCHED;
	if (ev.action < 0 && ev.argument.empty())
		return FINISHED;

	if (the_locking_inset) {
		result = the_locking_inset->localDispatch(ev);
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
			updateLocal(bv, CURSOR, false);
			// make sure status gets reset immediately
			bv->owner()->clearMessage();
			return result;
		}
	}
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	int updwhat = 0;
	int updflag = false;

	// what type of update to do on a cursor movement
	int cursor_update = CURSOR;

	if (lt->selection.set())
		cursor_update = SELECTION;

	switch (ev.action) {

	// Normal chars
	case LFUN_SELFINSERT:
		if (bv->buffer()->isReadonly()) {
//	    setErrorMessage(N_("Document is read only"));
			break;
		}
		if (!ev.argument.empty()) {
			/* Automatically delete the currently selected
			 * text and replace it with what is being
			 * typed in now. Depends on lyxrc settings
			 * "auto_region_delete", which defaults to
			 * true (on). */
#if 0
			// This should not be needed here and is also WRONG!
			setUndo(bv, Undo::INSERT,
				lt->cursor.par(), boost::next(lt->cursor.par()));
#endif
			bv->switchKeyMap();
			if (lyxrc.auto_region_delete) {
				if (lt->selection.set()) {
					lt->cutSelection(false, false);
				}
			}
			lt->clearSelection();
			for (string::size_type i = 0; i < ev.argument.length(); ++i) {
				bv->owner()->getIntl().getTransManager().
					TranslateAndInsert(ev.argument[i], lt);
			}
		}
		lt->selection.cursor = lt->cursor;
		updwhat = CURSOR | CURSOR_PAR;
		updflag = true;
		result = DISPATCHED_NOUPDATE;
		break;

	// cursor movements that need special handling

	case LFUN_RIGHT:
		result = moveRight(bv);
		finishUndo();
		updwhat = cursor_update;
		break;
	case LFUN_LEFT:
		finishUndo();
		result = moveLeft(bv);
		updwhat = cursor_update;
		break;
	case LFUN_DOWN:
		finishUndo();
		result = moveDown(bv);
		updwhat = cursor_update;
		break;
	case LFUN_UP:
		finishUndo();
		result = moveUp(bv);
		updwhat = cursor_update;
		break;

	case LFUN_PRIOR:
		if (crow(bv) == lt->rows().begin())
			result = FINISHED_UP;
		else {
			lt->cursorPrevious();
			lt->clearSelection();
			result = DISPATCHED_NOUPDATE;
		}
		updwhat = cursor_update;
		break;

	case LFUN_NEXT:
		if (boost::next(crow(bv)) == lt->rows().end())
			result = FINISHED_DOWN;
		else {
			lt->cursorNext();
			lt->clearSelection();
			result = DISPATCHED_NOUPDATE;
		}
		updwhat = cursor_update;
		break;

	case LFUN_BACKSPACE: {
		if (lt->selection.set())
			lt->cutSelection(true, false);
		else
			lt->backspace();
		updwhat = CURSOR_PAR;
		updflag = true;
		break;
	}

	case LFUN_DELETE: {
		if (lt->selection.set()) {
			lt->cutSelection(true, false);
		} else {
			lt->Delete();
		}
		updwhat = CURSOR_PAR;
		updflag = true;
		break;
	}

	case LFUN_CUT: {
		lt->cutSelection(bv);
		updwhat = CURSOR_PAR;
		updflag = true;
		break;
	}

	case LFUN_COPY:
		finishUndo();
		lt->copySelection();
		updwhat = CURSOR_PAR;
		break;

	case LFUN_PASTESELECTION:
	{
		string const clip(bv->getClipboard());

		if (clip.empty())
			break;
		if (ev.argument == "paragraph") {
			lt->insertStringAsParagraphs(clip);
		} else {
			lt->insertStringAsLines(clip);
		}
		// bug 393
		lt->clearSelection();

		updwhat = CURSOR_PAR;
		updflag = true;
		break;
	}

	case LFUN_PASTE: {
		if (!autoBreakRows) {
			if (CutAndPaste::nrOfParagraphs() > 1) {
#ifdef WITH_WARNINGS
#warning FIXME horrendously bad UI
#endif
				Alert::error(_("Paste failed"), _("Cannot include more than one paragraph."));
				break;
			}
		}

		lt->pasteSelection();
		// bug 393
		lt->clearSelection();
		updwhat = CURSOR_PAR;
		updflag = true;
		break;
	}

	case LFUN_BREAKPARAGRAPH:
		if (!autoBreakRows) {
			result = DISPATCHED;
			break;
		}
		lt->breakParagraph(paragraphs, 0);
		updwhat = CURSOR | FULL;
		updflag = true;
		break;

	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
		if (!autoBreakRows) {
			result = DISPATCHED;
			break;
		}
		lt->breakParagraph(paragraphs, 1);
		updwhat = CURSOR | FULL;
		updflag = true;
		break;

	case LFUN_BREAKLINE: {
		if (!autoBreakRows) {
			result = DISPATCHED;
			break;
		}

		lt->insertInset(new InsetNewline);
		updwhat = CURSOR | CURSOR_PAR;
		updflag = true;
		break;
	}

	case LFUN_LAYOUT:
		// do not set layouts on non breakable textinsets
		if (autoBreakRows) {
			string cur_layout = cpar(bv)->layout()->name();

			// Derive layout number from given argument (string)
			// and current buffer's textclass (number). */
			LyXTextClass const & tclass =
				bv->buffer()->params.getLyXTextClass();
			string layout = ev.argument;
			bool hasLayout = tclass.hasLayout(layout);

			// If the entry is obsolete, use the new one instead.
			if (hasLayout) {
				string const & obs =
					tclass[layout]->obsoleted_by();
				if (!obs.empty())
					layout = obs;
			}

			// see if we found the layout number:
			if (!hasLayout) {
				FuncRequest lf(LFUN_MESSAGE, N_("Layout ") + ev.argument + N_(" not known"));
				bv->owner()->dispatch(lf);
				break;
			}

			if (cur_layout != layout) {
				cur_layout = layout;
				lt->setLayout(layout);
				bv->owner()->setLayout(cpar(bv)->layout()->name());
				updwhat = CURSOR_PAR;
				updflag = true;
			}
		} else {
			// reset the layout box
			bv->owner()->setLayout(cpar(bv)->layout()->name());
		}
		break;
	case LFUN_PARAGRAPH_SPACING:
		// This one is absolutely not working. When fiddling with this
		// it also seems to me that the paragraphs inside the insettext
		// inherit bufferparams/paragraphparams in a strange way. (Lgb)
		// FIXME: how old is this comment ? ...
	{
		ParagraphList::iterator pit = lt->cursor.par();
		Spacing::Space cur_spacing = pit->params().spacing().getSpace();
		float cur_value = 1.0;
		if (cur_spacing == Spacing::Other) {
			cur_value = pit->params().spacing().getValue();
		}

		istringstream istr(ev.argument.c_str());
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
				   << ev.argument << endl;
		}
		if (cur_spacing != new_spacing || cur_value != new_value) {
			pit->params().spacing(Spacing(new_spacing, new_value));
			updwhat = CURSOR_PAR;
			updflag = true;
		}
	}
	break;

	// These need to do repaints but don't require
	// special handling otherwise. A *lot* of the
	// above could probably be done similarly ...

	case LFUN_HOME:
	case LFUN_END:
	case LFUN_WORDLEFT:
	case LFUN_WORDRIGHT:
	// these two are really unhandled ...
	case LFUN_ENDBUF:
	case LFUN_BEGINNINGBUF:
		updwhat = cursor_update;
		if (!bv->dispatch(ev))
			result = UNDISPATCHED;
		break;

	case LFUN_RIGHTSEL:
	case LFUN_UPSEL:
	case LFUN_DOWNSEL:
	case LFUN_LEFTSEL:
	case LFUN_HOMESEL:
	case LFUN_ENDSEL:
	case LFUN_WORDLEFTSEL:
	case LFUN_WORDRIGHTSEL:
		updwhat = SELECTION;

		// fallthrough

	default:
		if (!bv->dispatch(ev))
			result = UNDISPATCHED;
		break;
	}

	if (clear)
		lt = 0;
	if (updwhat > 0)
		updateLocal(bv, updwhat, updflag);
	/// If the action has deleted all text in the inset, we need to change the
	// language to the language of the surronding text.
	if (!was_empty && paragraphs.begin()->empty() &&
	    boost::next(paragraphs.begin()) == paragraphs.end()) {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(bv->getParentLanguage(this));
		setFont(bv, font, false);
	}

	if (result >= FINISHED)
		bv->unlockInset(this);

	if (result == DISPATCHED_NOUPDATE && (need_update & FULL))
		result = DISPATCHED;
	return result;
}


int InsetText::latex(Buffer const * buf, ostream & os,
		     bool moving_arg, bool) const
{
	TexRow texrow;
	latexParagraphs(buf, paragraphs,
			paragraphs.begin(), paragraphs.end(),
			os, texrow, moving_arg);
	return texrow.rows();
}


int InsetText::ascii(Buffer const * buf, ostream & os, int linelen) const
{
	unsigned int lines = 0;

	ParagraphList::iterator beg = paragraphs.begin();
	ParagraphList::iterator end = paragraphs.end();
	ParagraphList::iterator it = beg;
	for (; it != end; ++it) {
		string const tmp = buf->asciiParagraph(*it, linelen, it == beg);
		lines += lyx::count(tmp.begin(), tmp.end(), '\n');
		os << tmp;
	}
	return lines;
}


int InsetText::docbook(Buffer const * buf, ostream & os, bool mixcont) const
{
	unsigned int lines = 0;

	vector<string> environment_stack(10);
	vector<string> environment_inner(10);

	int const command_depth = 0;
	string item_name;

	Paragraph::depth_type depth = 0; // paragraph depth

	ParagraphList::iterator pit = paragraphs.begin();
	ParagraphList::iterator pend = paragraphs.end();

	for (; pit != pend; ++pit) {
		string sgmlparam;
		int desc_on = 0; // description mode

		LyXLayout_ptr const & style = pit->layout();

		// environment tag closing
		for (; depth > pit->params().depth(); --depth) {
			if (environment_inner[depth] != "!-- --") {
				item_name = "listitem";
				lines += sgml::closeTag(os, command_depth + depth, mixcont, item_name);
				if (environment_inner[depth] == "varlistentry")
					lines += sgml::closeTag(os, depth+command_depth, mixcont, environment_inner[depth]);
			}
			lines += sgml::closeTag(os, depth + command_depth, mixcont, environment_stack[depth]);
			environment_stack[depth].erase();
			environment_inner[depth].erase();
		}

		if (depth == pit->params().depth()
		   && environment_stack[depth] != style->latexname()
		   && !environment_stack[depth].empty()) {
			if (environment_inner[depth] != "!-- --") {
				item_name= "listitem";
				lines += sgml::closeTag(os, command_depth+depth, mixcont, item_name);
				if (environment_inner[depth] == "varlistentry")
					lines += sgml::closeTag(os, depth + command_depth, mixcont, environment_inner[depth]);
			}

			lines += sgml::closeTag(os, depth + command_depth, mixcont, environment_stack[depth]);

			environment_stack[depth].erase();
			environment_inner[depth].erase();
		}

		// Write opening SGML tags.
		switch (style->latextype) {
		case LATEX_PARAGRAPH:
			lines += sgml::openTag(os, depth + command_depth, mixcont, style->latexname());
			break;

		case LATEX_COMMAND:
			buf->sgmlError(&*pit, 0,  _("Error: LatexType Command not allowed here.\n"));
			return -1;
			break;

		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
			if (depth < pit->params().depth()) {
				depth = pit->params().depth();
				environment_stack[depth].erase();
			}

			if (environment_stack[depth] != style->latexname()) {
				if (environment_stack.size() == depth + 1) {
					environment_stack.push_back("!-- --");
					environment_inner.push_back("!-- --");
				}
				environment_stack[depth] = style->latexname();
				environment_inner[depth] = "!-- --";
				lines += sgml::openTag(os, depth + command_depth, mixcont, environment_stack[depth]);
			} else {
				if (environment_inner[depth] != "!-- --") {
					item_name= "listitem";
					lines += sgml::closeTag(os, command_depth + depth, mixcont, item_name);
					if (environment_inner[depth] == "varlistentry")
						lines += sgml::closeTag(os, depth + command_depth, mixcont, environment_inner[depth]);
				}
			}

			if (style->latextype == LATEX_ENVIRONMENT) {
				if (!style->latexparam().empty()) {
					if (style->latexparam() == "CDATA")
						os << "<![CDATA[";
					else
					  lines += sgml::openTag(os, depth + command_depth, mixcont, style->latexparam());
				}
				break;
			}

			desc_on = (style->labeltype == LABEL_MANUAL);

			environment_inner[depth] = desc_on ? "varlistentry" : "listitem";
			lines += sgml::openTag(os, depth + 1 + command_depth, mixcont, environment_inner[depth]);

			item_name = desc_on ? "term" : "para";
			lines += sgml::openTag(os, depth + 1 + command_depth, mixcont, item_name);

			break;
		default:
			lines += sgml::openTag(os, depth + command_depth, mixcont, style->latexname());
			break;
		}

		buf->simpleDocBookOnePar(os, pit, desc_on, depth + 1 + command_depth);

		string end_tag;
		// write closing SGML tags
		switch (style->latextype) {
		case LATEX_ENVIRONMENT:
			if (!style->latexparam().empty()) {
				if (style->latexparam() == "CDATA")
					os << "]]>";
				else
					lines += sgml::closeTag(os, depth + command_depth, mixcont, style->latexparam());
			}
			break;
		case LATEX_ITEM_ENVIRONMENT:
			if (desc_on == 1) break;
			end_tag= "para";
			lines += sgml::closeTag(os, depth + 1 + command_depth, mixcont, end_tag);
			break;
		case LATEX_PARAGRAPH:
			lines += sgml::closeTag(os, depth + command_depth, mixcont, style->latexname());
			break;
		default:
			lines += sgml::closeTag(os, depth + command_depth, mixcont, style->latexname());
			break;
		}
	}

	// Close open tags
	for (int d = depth; d >= 0; --d) {
		if (!environment_stack[depth].empty()) {
			if (environment_inner[depth] != "!-- --") {
				item_name = "listitem";
				lines += sgml::closeTag(os, command_depth + depth, mixcont, item_name);
			       if (environment_inner[depth] == "varlistentry")
				       lines += sgml::closeTag(os, depth + command_depth, mixcont, environment_inner[depth]);
			}

			lines += sgml::closeTag(os, depth + command_depth, mixcont, environment_stack[depth]);
		}
	}

	return lines;
}


void InsetText::validate(LaTeXFeatures & features) const
{
	for_each(paragraphs.begin(), paragraphs.end(),
		 boost::bind(&Paragraph::validate, _1, boost::ref(features)));
}


void InsetText::getCursor(BufferView & bv, int & x, int & y) const
{
	if (the_locking_inset) {
		the_locking_inset->getCursor(bv, x, y);
		return;
	}
	x = cx(&bv);
	y = cy(&bv) + InsetText::y();
}


void InsetText::getCursorPos(BufferView * bv, int & x, int & y) const
{
	if (the_locking_inset) {
		the_locking_inset->getCursorPos(bv, x, y);
		return;
	}
	x = cx(bv) - top_x - TEXT_TO_INSET_OFFSET;
	y = cy(bv) - TEXT_TO_INSET_OFFSET;
}


int InsetText::insetInInsetY() const
{
	if (!the_locking_inset)
		return 0;

	return (inset_y + the_locking_inset->insetInInsetY());
}


void InsetText::fitInsetCursor(BufferView * bv) const
{
	if (the_locking_inset) {
		the_locking_inset->fitInsetCursor(bv);
		return;
	}
	LyXFont const font =
		getLyXText(bv)->getFont(bv->buffer(), cpar(bv), cpos(bv));

	int const asc = font_metrics::maxAscent(font);
	int const desc = font_metrics::maxDescent(font);

	if (bv->fitLockedInsetCursor(cx(bv), cy(bv), asc, desc))
		need_update |= FULL;
}


Inset::RESULT
InsetText::moveRight(BufferView * bv, bool activate_inset, bool selecting)
{
	if (getLyXText(bv)->cursor.par()->isRightToLeftPar(bv->buffer()->params))
		return moveLeftIntern(bv, false, activate_inset, selecting);
	else
		return moveRightIntern(bv, true, activate_inset, selecting);
}


Inset::RESULT
InsetText::moveLeft(BufferView * bv, bool activate_inset, bool selecting)
{
	if (getLyXText(bv)->cursor.par()->isRightToLeftPar(bv->buffer()->params))
		return moveRightIntern(bv, true, activate_inset, selecting);
	else
		return moveLeftIntern(bv, false, activate_inset, selecting);
}


Inset::RESULT
InsetText::moveRightIntern(BufferView * bv, bool front,
			   bool activate_inset, bool selecting)
{
	LyXText * text = getLyXText(bv);

	ParagraphList::iterator c_par = cpar(bv);

	if (boost::next(c_par) == paragraphs.end() &&
	    (cpos(bv) >= c_par->size()))
		return FINISHED_RIGHT;
	if (activate_inset && checkAndActivateInset(bv, front))
		return DISPATCHED;
	text->cursorRight(bv);
	if (!selecting)
		text->clearSelection();
	return DISPATCHED_NOUPDATE;
}


Inset::RESULT
InsetText::moveLeftIntern(BufferView * bv, bool front,
			  bool activate_inset, bool selecting)
{
	LyXText * text = getLyXText(bv);

	if (cpar(bv) == paragraphs.begin() && (cpos(bv) <= 0))
		return FINISHED;
	text->cursorLeft(bv);
	if (!selecting)
		text->clearSelection();
	if (activate_inset && checkAndActivateInset(bv, front))
		return DISPATCHED;
	return DISPATCHED_NOUPDATE;
}


Inset::RESULT InsetText::moveUp(BufferView * bv)
{
	if (crow(bv) == getLyXText(bv)->rows().begin())
		return FINISHED_UP;
	getLyXText(bv)->cursorUp(bv);
	getLyXText(bv)->clearSelection();
	return DISPATCHED_NOUPDATE;
}


Inset::RESULT InsetText::moveDown(BufferView * bv)
{
	if (boost::next(crow(bv)) == getLyXText(bv)->rows().end())
		return FINISHED_DOWN;
	getLyXText(bv)->cursorDown(bv);
	getLyXText(bv)->clearSelection();
	return DISPATCHED_NOUPDATE;
}


bool InsetText::insertInset(BufferView * bv, Inset * inset)
{
	if (the_locking_inset) {
		if (the_locking_inset->insetAllowed(inset))
			return the_locking_inset->insertInset(bv, inset);
		return false;
	}
	inset->setOwner(this);
	getLyXText(bv)->insertInset(inset);
	bv->fitCursor();
	updateLocal(bv, CURSOR_PAR|CURSOR, true);
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

	ParagraphList::iterator pit = paragraphs.begin();
	ParagraphList::iterator pend = paragraphs.end();
	for (; pit != pend; ++pit) {
		InsetList::iterator beg = pit->insetlist.begin();
		InsetList::iterator end = pit->insetlist.end();
		for (; beg != end; ++beg) {
			vector<string> const l = beg.getInset()->getLabelList();
			label_list.insert(label_list.end(), l.begin(), l.end());
		}
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
	if ((boost::next(paragraphs.begin()) == paragraphs.end() &&
	     paragraphs.begin()->empty()) || cpar(bv)->empty()) {
		getLyXText(bv)->setFont(font, toggleall);
		return;
	}
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	if (lt->selection.set()) {
		setUndo(bv, Undo::EDIT, lt->cursor.par(), boost::next(lt->cursor.par()));
	}
	if (selectall)
		selectAll(bv);
	lt->toggleFree(font, toggleall);
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


bool InsetText::checkAndActivateInset(BufferView * bv, bool front)
{
	if (cpar(bv)->isInset(cpos(bv))) {
		Inset * inset =
			static_cast<UpdatableInset*>(cpar(bv)->getInset(cpos(bv)));
		if (!isHighlyEditableInset(inset))
			return false;
		inset->edit(bv, front);
		if (!the_locking_inset)
			return false;
		updateLocal(bv, CURSOR, false);
		return true;
	}
	return false;
}


bool InsetText::checkAndActivateInset(BufferView * bv, int x, int y,
				      mouse_button::state button)
{
	x -= drawTextXOffset;
	int dummyx = x;
	int dummyy = y + insetAscent;
	Inset * inset = getLyXText(bv)->checkInsetHit(dummyx, dummyy);
	// we only do the edit() call if the inset was hit by the mouse
	// or if it is a highly editable inset. So we should call this
	// function from our own edit with button < 0.
	// FIXME: GUII jbl. I've changed this to ::none for now which is probably
	// WRONG
	if (button == mouse_button::none && !isHighlyEditableInset(inset))
		return false;

	if (inset) {
		if (x < 0)
			x = insetWidth;
		if (y < 0)
			y = insetDescent;
		inset_x = cix(bv) - top_x + drawTextXOffset;
		inset_y = ciy(bv) + drawTextYOffset;
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


void InsetText::setParagraphData(ParagraphList const & plist, bool same_id)
{
	// we have to unlock any locked inset otherwise we're in troubles
	the_locking_inset = 0;

	paragraphs.clear();
	ParagraphList::iterator pit = plist.begin();
	ParagraphList::iterator pend = plist.end();
	for (; pit != pend; ++pit) {
		Paragraph * new_par = new Paragraph(*pit, same_id);
		new_par->setInsetOwner(this);
		paragraphs.push_back(new_par);
	}

	reinitLyXText();
	need_update = INIT;
}


void InsetText::markNew(bool track_changes)
{
	ParagraphList::iterator pit = paragraphs.begin();
	ParagraphList::iterator pend = paragraphs.end();
	for (; pit != pend; ++pit) {
		if (track_changes) {
			pit->trackChanges();
		} else {
			// no-op when not tracking
			pit->cleanChanges();
		}
	}
}


void InsetText::setText(string const & data, LyXFont const & font)
{
	clear(false);
	for (unsigned int i = 0; i < data.length(); ++i)
		paragraphs.begin()->insertChar(i, data[i], font);
	reinitLyXText();
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


int InsetText::cix(BufferView * bv) const
{
	// we do nothing dangerous so we use a local cache
	LyXText * llt = getLyXText(bv);
	int x = llt->cursor.ix() + top_x + TEXT_TO_INSET_OFFSET;
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


int InsetText::ciy(BufferView * bv) const
{
	LyXFont font;
	return getLyXText(bv)->cursor.iy() - ascent(bv, font) + TEXT_TO_INSET_OFFSET;
}


pos_type InsetText::cpos(BufferView * bv) const
{
	return getLyXText(bv)->cursor.pos();
}


ParagraphList::iterator InsetText::cpar(BufferView * bv) const
{
	return getLyXText(bv)->cursor.par();
}


bool InsetText::cboundary(BufferView * bv) const
{
	return getLyXText(bv)->cursor.boundary();
}


RowList::iterator InsetText::crow(BufferView * bv) const
{
	return getLyXText(bv)->cursor.row();
}


LyXText * InsetText::getLyXText(BufferView const * lbv,
				bool const recursive) const
{
	if (cached_bview == lbv) {
		if (recursive && the_locking_inset)
			return the_locking_inset->getLyXText(lbv, true);
		LyXText * lt = cached_text.get();
		lyx::Assert(lt && lt->rows().begin()->par() == paragraphs.begin());
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
			restoreLyXTextState(it->second.text.get());
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
	cached_text.reset(new LyXText(bv, const_cast<InsetText *>(this)));
	cached_text->init(bv);
	restoreLyXTextState(cached_text.get());

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
		for_each(paragraphs.begin(), paragraphs.end(),
			 boost::bind(&Paragraph::deleteInsetsLyXText, _1, bv));
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
	if (boost::next(paragraphs.begin()) == paragraphs.end() &&
	    paragraphs.begin()->empty()) { // no data, resize not neccessary!
		// we have to do this as a fixed width may have changed!
		LyXText * t = getLyXText(bv);
		saveLyXTextState(t);
		t->init(bv, true);
		restoreLyXTextState(t);
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

	for_each(paragraphs.begin(), paragraphs.end(),
		 boost::bind(&Paragraph::resizeInsetsLyXText, _1, bv));

	t->init(bv, true);
	restoreLyXTextState(t);
	if (the_locking_inset) {
		inset_x = cix(bv) - top_x + drawTextXOffset;
		inset_y = ciy(bv) + drawTextYOffset;
	}

	t->top_y(bv->screen().topCursorVisible(t->cursor, t->top_y()));
	if (!owner()) {
		const_cast<InsetText*>(this)->updateLocal(bv, FULL, false);
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
	for (Cache::iterator it = cache.begin(); it != cache.end(); ++it) {
		lyx::Assert(it->second.text.get());

		LyXText * t = it->second.text.get();
		BufferView * bv = it->first;

		saveLyXTextState(t);

		for_each(paragraphs.begin(), paragraphs.end(),
			 boost::bind(&Paragraph::resizeInsetsLyXText, _1, bv));

		t->init(bv, true);
		restoreLyXTextState(t);
		if (the_locking_inset) {
			inset_x = cix(bv) - top_x + drawTextXOffset;
			inset_y = ciy(bv) + drawTextYOffset;
		}
		t->top_y(bv->screen().topCursorVisible(t->cursor, t->top_y()));
		if (!owner()) {
			const_cast<InsetText*>(this)->updateLocal(bv, FULL, false);
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

	ParagraphList::iterator it = paragraphs.begin();
	ParagraphList::iterator end = paragraphs.end();
	for (; it != end; ++it) {
		for (int i = 0; i < it->size(); ++i) {
			if (it->isNewline(i)) {
				changed = true;
				it->erase(i);
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


void InsetText::selectAll(BufferView * bv)
{
	getLyXText(bv)->cursorTop();
	getLyXText(bv)->selection.cursor = getLyXText(bv)->cursor;
	getLyXText(bv)->cursorBottom();
	getLyXText(bv)->setSelection();
}


void InsetText::clearSelection(BufferView * bv)
{
	getLyXText(bv)->clearSelection();
}


void InsetText::clearInset(BufferView * bv, int start_x, int baseline) const
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
	pain.fillRectangle(start_x + 1, ty + 1, w - 3, h - 1, backgroundColor());
	need_update = FULL;
}


ParagraphList * InsetText::getParagraphs(int i) const
{
	return (i == 0) ? const_cast<ParagraphList*>(&paragraphs) : 0;
}


LyXCursor const & InsetText::cursor(BufferView * bv) const
{
	if (the_locking_inset)
		return the_locking_inset->cursor(bv);
	return getLyXText(bv)->cursor;
}


void InsetText::paragraph(Paragraph * p)
{
	// GENERAL COMMENT: We don't have to free the old paragraphs as the
	// caller of this function has to take care of it. This IS important
	// as we could have to insert a paragraph before this one and just
	// link the actual to a new ones next and set it with this function
	// and are done!
	paragraphs.set(p);
	// set ourself as owner for all the paragraphs inserted!
	for_each(paragraphs.begin(), paragraphs.end(),
		 boost::bind(&Paragraph::setInsetOwner, _1, this));

	reinitLyXText();
	// redraw myself when asked for
	need_update = INIT;
}


Inset * InsetText::getInsetFromID(int id_arg) const
{
	if (id_arg == id())
		return const_cast<InsetText *>(this);

	ParagraphList::iterator pit = paragraphs.begin();
	ParagraphList::iterator pend = paragraphs.end();
	for (; pit != pend; ++pit) {
		InsetList::iterator it = pit->insetlist.begin();
		InsetList::iterator end = pit->insetlist.end();
		for (; it != end; ++it) {
			if (it.getInset()->id() == id_arg)
				return it.getInset();
			Inset * in = it.getInset()->getInsetFromID(id_arg);
			if (in)
				return in;
		}
	}
	return 0;
}


WordLangTuple const
InsetText::selectNextWordToSpellcheck(BufferView * bv,
				      float & value) const
{
	bool clear = false;
	WordLangTuple word;

	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	if (the_locking_inset) {
		word = the_locking_inset->selectNextWordToSpellcheck(bv, value);
		if (!word.word().empty()) {
			value += cy(bv);
			if (clear)
				lt = 0;
			return word;
		}
		// we have to go on checking so move cursor to the next char
		lt->cursor.pos(lt->cursor.pos() + 1);
	}
	word = lt->selectNextWordToSpellcheck(value);
	if (word.word().empty())
		bv->unlockInset(const_cast<InsetText *>(this));
	else
		value = cy(bv);
	if (clear)
		lt = 0;
	return word;
}


void InsetText::selectSelectedWord(BufferView * bv)
{
	if (the_locking_inset) {
		the_locking_inset->selectSelectedWord(bv);
		return;
	}
	getLyXText(bv)->selectSelectedWord();
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

	RowList::iterator rowit = lt->rows().begin();
	RowList::iterator end = lt->rows().end();
	int y_offset = top_baseline - rowit->ascent_of_text();
	int y = y_offset;
	while ((rowit != end) && ((y + rowit->height()) <= 0)) {
		y += rowit->height();
		++rowit;
	}
	if (y_offset < 0)
		y_offset = y;

	if (need_update & SELECTION)
		need_update = NONE;
	bv->screen().toggleSelection(lt, bv, kill_selection, y_offset, x);
	if (clear)
		lt = 0;
}


bool InsetText::nextChange(BufferView * bv, lyx::pos_type & length)
{
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	if (the_locking_inset) {
		if (the_locking_inset->nextChange(bv, length))
			return true;
		lt->cursorRight(true);
	}
	lyxfind::SearchResult result =
		lyxfind::findNextChange(bv, lt, length);

	if (result == lyxfind::SR_FOUND) {
		LyXCursor cur = lt->cursor;
		bv->unlockInset(bv->theLockingInset());
		if (bv->lockInset(this))
			locked = true;
		lt->cursor = cur;
		lt->setSelectionRange(length);
		updateLocal(bv, SELECTION, false);
	}
	if (clear)
		lt = 0;
	return result != lyxfind::SR_NOT_FOUND;
}


bool InsetText::searchForward(BufferView * bv, string const & str,
			      bool cs, bool mw)
{
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	if (the_locking_inset) {
		if (the_locking_inset->searchForward(bv, str, cs, mw))
			return true;
		lt->cursorRight(true);
	}
	lyxfind::SearchResult result =
		lyxfind::LyXFind(bv, lt, str, true, cs, mw);

	if (result == lyxfind::SR_FOUND) {
		LyXCursor cur = lt->cursor;
		bv->unlockInset(bv->theLockingInset());
		if (bv->lockInset(this))
			locked = true;
		lt->cursor = cur;
		lt->setSelectionRange(str.length());
		updateLocal(bv, SELECTION, false);
	}
	if (clear)
		lt = 0;
	return (result != lyxfind::SR_NOT_FOUND);
}

bool InsetText::searchBackward(BufferView * bv, string const & str,
			       bool cs, bool mw)
{
	if (the_locking_inset) {
		if (the_locking_inset->searchBackward(bv, str, cs, mw))
			return true;
	}
	bool clear = false;
	if (!lt) {
		lt = getLyXText(bv);
		clear = true;
	}
	if (!locked) {
		ParagraphList::iterator pit = paragraphs.begin();
		ParagraphList::iterator pend = paragraphs.end();

		while (boost::next(pit) != pend)
			++pit;

		lt->setCursor(pit, pit->size());
	}
	lyxfind::SearchResult result =
		lyxfind::LyXFind(bv, lt, str, false, cs, mw);

	if (result == lyxfind::SR_FOUND) {
		LyXCursor cur = lt->cursor;
		bv->unlockInset(bv->theLockingInset());
		if (bv->lockInset(this))
			locked = true;
		lt->cursor = cur;
		lt->setSelectionRange(str.length());
		updateLocal(bv, SELECTION, false);
	}
	if (clear)
		lt = 0;
	return (result != lyxfind::SR_NOT_FOUND);
}


bool InsetText::checkInsertChar(LyXFont & font)
{
	if (owner())
		return owner()->checkInsertChar(font);
	return true;
}


void InsetText::collapseParagraphs(BufferView * bv)
{
	LyXText * llt = getLyXText(bv);

	while (boost::next(paragraphs.begin()) != paragraphs.end()) {
		if (!paragraphs.begin()->empty() &&
		    !boost::next(paragraphs.begin())->empty() &&
			!paragraphs.begin()->isSeparator(paragraphs.begin()->size() - 1))
		{
			paragraphs.begin()->insertChar(paragraphs.begin()->size(), ' ');
		}
		if (llt->selection.set()) {
			if (llt->selection.start.par() == boost::next(paragraphs.begin())) {
				llt->selection.start.par(paragraphs.begin());
				llt->selection.start.pos(
					llt->selection.start.pos() + paragraphs.begin()->size());
			}
			if (llt->selection.end.par() == boost::next(paragraphs.begin())) {
				llt->selection.end.par(paragraphs.begin());
				llt->selection.end.pos(
					llt->selection.end.pos() + paragraphs.begin()->size());
			}
		}
		mergeParagraph(bv->buffer()->params, paragraphs, paragraphs.begin());
	}
	reinitLyXText();
}


void InsetText::getDrawFont(LyXFont & font) const
{
	if (!owner())
		return;
	owner()->getDrawFont(font);
}


void InsetText::appendParagraphs(Buffer * buffer,
				 ParagraphList & plist)
{
#warning FIXME Check if Changes stuff needs changing here. (Lgb)
// And it probably does. You have to take a look at this John. (Lgb)
#warning John, have a look here. (Lgb)
	BufferParams const & bparams = buffer->params;
#if 0
	Paragraph * buf;
	Paragraph * tmpbuf = newpar;
	Paragraph * lastbuffer = buf = new Paragraph(*tmpbuf, false);
	if (bparams.tracking_changes)
		buf->cleanChanges();

	while (tmpbuf->next()) {
		tmpbuf = tmpbuf->next();
		lastbuffer->next(new Paragraph(*tmpbuf, false));
		lastbuffer->next()->previous(lastbuffer);
		lastbuffer = lastbuffer->next();
		if (bparams.tracking_changes)
			lastbuffer->cleanChanges();
	}

	lastbuffer = &*(paragraphs.begin());
	while (lastbuffer->next())
		lastbuffer = lastbuffer->next();
	if (!newpar->empty() && !lastbuffer->empty() &&
		!lastbuffer->isSeparator(lastbuffer->size() - 1))
	{
		lastbuffer->insertChar(lastbuffer->size(), ' ');
	}

	// make the buf exactly the same layout than our last paragraph
	buf->makeSameLayout(lastbuffer);

	// paste it!
	lastbuffer->next(buf);
	buf->previous(lastbuffer);
	mergeParagraph(buffer->params, paragraphs, lastbuffer);
#else
	ParagraphList::iterator pit = plist.begin();
	ParagraphList::iterator ins = paragraphs.insert(paragraphs.end(),
							new Paragraph(*pit, false));
	++pit;
	mergeParagraph(buffer->params, paragraphs, boost::prior(ins));

	ParagraphList::iterator pend = plist.end();
	for (; pit != pend; ++pit) {
		paragraphs.push_back(new Paragraph(*pit, false));
	}

#endif

	reinitLyXText();
}


void InsetText::addPreview(grfx::PreviewLoader & loader) const
{
	ParagraphList::iterator pit = paragraphs.begin();
	ParagraphList::iterator pend = paragraphs.end();

	for (; pit != pend; ++pit) {
		InsetList::iterator it  = pit->insetlist.begin();
		InsetList::iterator end = pit->insetlist.end();
		for (; it != end; ++it) {
			it.getInset()->addPreview(loader);
		}
	}
}
