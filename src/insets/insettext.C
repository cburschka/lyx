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
#include "dimension.h"
#include "funcrequest.h"
#include "gettext.h"
#include "errorlist.h"
#include "intl.h"
#include "LaTeXFeatures.h"
#include "LColor.h"
#include "Lsstream.h"
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
#include "metricsinfo.h"

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

using namespace lyx::support;

using lyx::pos_type;
using lyx::textclass_type;

namespace grfx = lyx::graphics;

// These functions should probably go into bufferview_funcs somehow (Jug)

void InsetText::saveLyXTextState(LyXText * t) const
{
	// check if my paragraphs are still valid
	ParagraphList::iterator it = const_cast<ParagraphList&>(paragraphs).begin();
	ParagraphList::iterator end = const_cast<ParagraphList&>(paragraphs).end();
	for (; it != end; ++it) {
		if (it == t->cursor.par())
			break;
	}

	if (it != end && t->cursor.pos() <= it->size())
		sstate = *t; // slicing intended
	else
		sstate.cursor.par(end);
}


void InsetText::restoreLyXTextState(LyXText * t) const
{
	if (sstate.cursor.par() == const_cast<ParagraphList&>(paragraphs).end())
		return;

	t->selection.set(true);
	// at this point just to avoid the DEPM when setting the cursor
	t->selection.mark(sstate.selection.mark());
	if (sstate.selection.set()) {
		t->setCursor(sstate.selection.start.par(),
			sstate.selection.start.pos(),
			true, sstate.selection.start.boundary());
		t->selection.cursor = t->cursor;
		t->setCursor(sstate.selection.end.par(), sstate.selection.end.pos(),
			true, sstate.selection.end.boundary());
		t->setSelection();
		t->setCursor(sstate.cursor.par(), sstate.cursor.pos());
	} else {
		t->setCursor(sstate.cursor.par(), sstate.cursor.pos(),
			true, sstate.cursor.boundary());
		t->selection.cursor = t->cursor;
		t->selection.set(false);
	}
}


InsetText::InsetText(BufferParams const & bp)
	: UpdatableInset(), in_update(false), text_(0, this)
{
	paragraphs.push_back(Paragraph());
	paragraphs.begin()->layout(bp.getLyXTextClass().defaultLayout());
	if (bp.tracking_changes)
		paragraphs.begin()->trackChanges();
	init(0);
}


InsetText::InsetText(InsetText const & in)
	: UpdatableInset(in), in_update(false), text_(0, this)
{
	init(&in);
}


InsetText & InsetText::operator=(InsetText const & it)
{
	init(&it);
	return *this;
}


void InsetText::init(InsetText const * ins)
{
	if (ins) {
		text_.bv_owner = ins->text_.bv_owner;
		setParagraphData(ins->paragraphs);
		autoBreakRows = ins->autoBreakRows;
		drawFrame_ = ins->drawFrame_;
		frame_color = ins->frame_color;
	} else {
		drawFrame_ = NEVER;
		frame_color = LColor::insetframe;
		autoBreakRows = false;
	}
	the_locking_inset = 0;
	for_each(paragraphs.begin(), paragraphs.end(),
		 boost::bind(&Paragraph::setInsetOwner, _1, this));
	top_y = 0;
	old_max_width = 0;
	no_selection = true;
	need_update = FULL;
	drawTextXOffset = 0;
	drawTextYOffset = 0;
	locked = false;
	old_par = paragraphs.end();
	last_drawn_width = -1;
	sstate.cursor.par(paragraphs.end());
	in_insetAllowed = false;
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
	paragraphs.push_back(Paragraph());
	paragraphs.begin()->setInsetOwner(this);
	paragraphs.begin()->layout(old_layout);

	reinitLyXText();
	need_update = INIT;
}


InsetBase * InsetText::clone() const
{
	return new InsetText(*this);
}


void InsetText::write(Buffer const * buf, ostream & os) const
{
	os << "Text\n";
	writeParagraphData(buf, os);
}


void InsetText::writeParagraphData(Buffer const * buf, ostream & os) const
{
	ParagraphList::const_iterator it = paragraphs.begin();
	ParagraphList::const_iterator end = paragraphs.end();
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


void InsetText::metrics(MetricsInfo & mi, Dimension & dim) const
{
	BufferView * bv = mi.base.bv;
	LyXText * text = getLyXText(bv);
	dim.asc = text->rows().begin()->ascent_of_text() + TEXT_TO_INSET_OFFSET;
	dim.des = text->height - dim.asc + TEXT_TO_INSET_OFFSET;
	dim.wid = max(textWidth(bv), int(text->width)) + 2 * TEXT_TO_INSET_OFFSET;
	dim.wid = max(dim.wid, 10);
	dim_ = dim;
}


int InsetText::textWidth(BufferView * bv, bool fordraw) const
{
	int w = autoBreakRows ? getMaxWidth(bv, this) : -1;

	if (fordraw)
		return max(w - 2 * TEXT_TO_INSET_OFFSET,
			   (int)getLyXText(bv)->width);

	if (w < 0)
		return -1;

	return w - 2 * TEXT_TO_INSET_OFFSET;
}


void InsetText::draw(PainterInfo & pi, int x, int baseline) const
{
	if (nodraw())
		return;

	// update our idea of where we are. Clearly, we should
	// not have to know this information.
	if (top_x != x)
		top_x = x;

	int const start_x = x;

	BufferView * bv = pi.base.bv;
	Painter & pain = pi.pain;

	// repaint the background if needed
	if (backgroundColor() != LColor::background)
		clearInset(bv, start_x + TEXT_TO_INSET_OFFSET, baseline);

	// no draw is necessary !!!
	if (drawFrame_ == LOCKED && !locked && paragraphs.begin()->empty()) {
		top_baseline = baseline;
		need_update = NONE;
		return;
	}

	if (!owner())
		x += scroll();

	top_baseline = baseline;
	top_y = baseline - dim_.asc;

	if (last_drawn_width != dim_.wid) {
		need_update |= FULL;
		last_drawn_width = dim_.wid;
	}

	if (the_locking_inset && (cpar(bv) == inset_par)
		&& (cpos(bv) == inset_pos)) {
		inset_x = cix(bv) - int(x) + drawTextXOffset;
		inset_y = ciy(bv) + drawTextYOffset;
	}

	x += TEXT_TO_INSET_OFFSET;

	RowList::iterator rowit = text_.rows().begin();
	RowList::iterator end = text_.rows().end();

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
		text_.top_y(-y_offset);
		first = y;
		y_offset = 0;
	} else {
		text_.top_y(first);
		first = 0;
	}

	int yf = y_offset + first;
	y = 0;

	bv->hideCursor();

	while ((rowit != end) && (yf < ph)) {
		RowPainter rp(*bv, text_, rowit);
		rp.paint(y + y_offset + first, int(x), y + text_.top_y());
		y += rowit->height();
		yf += rowit->height();
		++rowit;
	}

	text_.clearPaint();

	if ((drawFrame_ == ALWAYS) || (drawFrame_ == LOCKED && locked)) {
		drawFrame(pain, int(start_x));
	}

	x += dim_.wid - TEXT_TO_INSET_OFFSET;

	if (need_update != INIT) {
		need_update = NONE;
	}
}


void InsetText::drawFrame(Painter & pain, int x) const
{
	static int const ttoD2 = TEXT_TO_INSET_OFFSET / 2;
	frame_x = x + ttoD2;
	frame_y = top_baseline - dim_.asc + ttoD2;
	frame_w = dim_.wid - TEXT_TO_INSET_OFFSET;
	frame_h = dim_.asc + dim_.des - TEXT_TO_INSET_OFFSET;
	pain.rectangle(frame_x, frame_y, frame_w, frame_h,
		       frame_color);
}


void InsetText::update(BufferView * bv, bool reinit)
{
	if (bv)
		text_.bv_owner = const_cast<BufferView *>(bv);

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

	if (!autoBreakRows && paragraphs.size() > 1)
		collapseParagraphs(bv);

	if (the_locking_inset) {
		inset_x = cix(bv) - top_x + drawTextXOffset;
		inset_y = ciy(bv) + drawTextYOffset;
		the_locking_inset->update(bv, reinit);
	}

	if ((need_update & CURSOR_PAR) && (text_.refreshStatus() == LyXText::REFRESH_NONE) &&
		the_locking_inset) {
		text_.updateInset(the_locking_inset);
	}

	if (text_.refreshStatus() == LyXText::REFRESH_AREA)
		need_update |= FULL;

	in_update = false;

	int nw = getMaxWidth(bv, this);
	if (nw > 0 && old_max_width != nw) {
		need_update |= INIT;
		old_max_width = nw;
	}
}


void InsetText::setUpdateStatus(BufferView *, int what) const
{
	need_update |= what;
	// we have to redraw us full if our LyXText REFRESH_AREA or
	// if we don't break row so that we only have one row to update!
	if ((text_.refreshStatus() == LyXText::REFRESH_AREA) ||
	    (!autoBreakRows &&
	     (text_.refreshStatus() == LyXText::REFRESH_ROW)))
	{
		need_update |= FULL;
	} else if (text_.refreshStatus() == LyXText::REFRESH_ROW) {
		need_update |= CURSOR_PAR;
	}

	// this to not draw a selection when we redraw all of it!
	if (need_update & CURSOR && !(need_update & SELECTION)) {
		if (text_.selection.set())
			need_update = FULL;
		text_.clearSelection();
	}
}


void InsetText::updateLocal(BufferView * bv, int what, bool mark_dirty)
{
	if (!autoBreakRows && paragraphs.size() > 1)
		collapseParagraphs(bv);

	text_.partialRebreak();
	setUpdateStatus(bv, what);
	bool flag = mark_dirty ||
		(((need_update != CURSOR) && (need_update != NONE)) ||
		 (text_.refreshStatus() != LyXText::REFRESH_NONE) || text_.selection.set());
	if (!text_.selection.set())
		text_.selection.cursor = text_.cursor;

	bv->fitCursor();

	if (flag) {
		text_.postPaint(0);
		bv->updateInset(const_cast<InsetText *>(this));
	}

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

	if (text_.selection.set()) {
		text_.clearSelection();
		code = FULL;
	} else if (owner()) {
		bv->owner()->setLayout(owner()->getLyXText(bv)
				       ->cursor.par()->layout()->name());
	} else
		bv->owner()->setLayout(bv->text->cursor.par()->layout()->name());
	// hack for deleteEmptyParMech
	ParagraphList::iterator first_par = paragraphs.begin();
	if (!first_par->empty()) {
		text_.setCursor(first_par, 0);
	} else if (paragraphs.size() > 1) {
		text_.setCursor(boost::next(first_par), 0);
	}
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
	inset_par = paragraphs.end();
	old_par = paragraphs.end();
	text_.setCursor(paragraphs.begin(), 0);
	text_.clearSelection();
	finishUndo();
	// If the inset is empty set the language of the current font to the
	// language to the surronding text (if different).
	if (paragraphs.begin()->empty() && paragraphs.size() == 1 &&
		bv->getParentLanguage(this) != text_.current_font.language()) {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(bv->getParentLanguage(this));
		setFont(bv, font, false);
	}
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
				if (it->inset == inset) {
					getLyXText(bv)->setCursorIntern(pit, it->pos);
					lockInset(bv, inset);
					return true;
				}
				if (it->inset->getInsetFromID(id)) {
					getLyXText(bv)->setCursorIntern(pit, it->pos);
					it->inset->localDispatch(FuncRequest(bv, LFUN_INSET_EDIT));
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
		old_par = paragraphs.end(); // force layout setting
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
	if (!autoBreakRows && paragraphs.size() > 1)
		collapseParagraphs(bv);

	if (inset == this)
		return true;

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
			text_.updateInset(tl_inset);
		if (found)
			setUpdateStatus(bv, ustat);
		return found;
	}
	bool found = text_.updateInset(inset);
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
	int tmp_y = cmd.y + dim_.asc - getLyXText(bv)->top_y();
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
		int old_top_y = text_.top_y();

		text_.setCursorFromCoordinates(cmd.x - drawTextXOffset,
					     cmd.y + dim_.asc);
		// set the selection cursor!
		text_.selection.cursor = text_.cursor;
		text_.cursor.x_fix(text_.cursor.x());

		if (text_.selection.set()) {
			text_.clearSelection();
			updateLocal(bv, FULL, false);
		} else {
			text_.clearSelection();
			updateLocal(bv, CURSOR, false);
		}

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
	int tmp_y = cmd.y + dim_.asc - getLyXText(bv)->top_y();
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
	LyXCursor cur = text_.cursor;
	text_.setCursorFromCoordinates
		(cmd.x - drawTextXOffset, cmd.y + dim_.asc);
	text_.cursor.x_fix(text_.cursor.x());
	if (cur == text_.cursor)
		return;
	text_.setSelection();
	bool flag = (text_.toggle_cursor.par() != text_.toggle_end_cursor.par() ||
				 text_.toggle_cursor.pos() != text_.toggle_end_cursor.pos());
	if (flag) {
		updateLocal(bv, SELECTION, false);
	}
}


Inset::RESULT InsetText::localDispatch(FuncRequest const & cmd)
{
	BufferView * bv = cmd.view();
	if (bv)
		text_.bv_owner = bv;

	if (cmd.action == LFUN_INSET_EDIT) {
		UpdatableInset::localDispatch(cmd);

		if (!bv->lockInset(this)) {
			lyxerr[Debug::INSETS] << "Cannot lock inset" << endl;
			return DISPATCHED;
		}

		locked = true;
		the_locking_inset = 0;
		inset_pos = inset_x = inset_y = 0;
		inset_boundary = false;
		inset_par = paragraphs.end();
		old_par = paragraphs.end();


		if (cmd.argument.size()) {
			if (cmd.argument == "left")
				text_.setCursor(paragraphs.begin(), 0);
			else {
				ParagraphList::iterator it = paragraphs.begin();
				ParagraphList::iterator end = paragraphs.end();
				while (boost::next(it) != end)
					++it;
		//		int const pos = (p->size() ? p->size()-1 : p->size());
				text_.setCursor(it, it->size());
			}
		} else {
			int tmp_y = (cmd.y < 0) ? 0 : cmd.y;
			// we put here -1 and not button as now the button in the
			// edit call should not be needed we will fix this in 1.3.x
			// cycle hopefully (Jug 20020509)
			// FIXME: GUII I've changed this to none: probably WRONG
			if (!checkAndActivateInset(bv, cmd.x, tmp_y, mouse_button::none)) {
				text_.setCursorFromCoordinates(cmd.x - drawTextXOffset,
									cmd.y + dim_.asc);
				text_.cursor.x_fix(text_.cursor.x());
			}
		}

		text_.clearSelection();
		finishUndo();

		// If the inset is empty set the language of the current font to the
		// language to the surronding text (if different).
		if (paragraphs.begin()->empty() &&
		    paragraphs.size() == 1 &&
		    bv->getParentLanguage(this) != text_.current_font.language())
		{
			LyXFont font(LyXFont::ALL_IGNORE);
			font.setLanguage(bv->getParentLanguage(this));
			setFont(bv, font, false);
		}

		int code = CURSOR;
		if (drawFrame_ == LOCKED)
			code = CURSOR | DRAW_FRAME;

		updateLocal(bv, code, false);
		// Tell the paragraph dialog that we've entered an insettext.
		bv->dispatch(FuncRequest(LFUN_PARAGRAPH_UPDATE));
		return DISPATCHED;
	}


	switch (cmd.action) {
		case LFUN_MOUSE_PRESS:
			lfunMousePress(cmd);
			return DISPATCHED;
		case LFUN_MOUSE_MOTION:
			lfunMouseMotion(cmd);
			return DISPATCHED;
		case LFUN_MOUSE_RELEASE:
			return lfunMouseRelease(cmd) ? DISPATCHED : UNDISPATCHED;
		default:
			break;
	}

	bool was_empty = (paragraphs.begin()->empty() &&
			  paragraphs.size() == 1);

	no_selection = false;
	RESULT result = UpdatableInset::localDispatch(cmd);
	if (result != UNDISPATCHED)
		return DISPATCHED;

	result = DISPATCHED;
	if (cmd.action < 0 && cmd.argument.empty())
		return FINISHED;

	if (the_locking_inset) {
		result = the_locking_inset->localDispatch(cmd);
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
	int updwhat = 0;
	int updflag = false;

	// what type of update to do on a cursor movement
	int cursor_update = CURSOR;

	if (text_.selection.set())
		cursor_update = SELECTION;

	switch (cmd.action) {

	// Normal chars
	case LFUN_SELFINSERT:
		if (bv->buffer()->isReadonly()) {
//	    setErrorMessage(N_("Document is read only"));
			break;
		}
		if (!cmd.argument.empty()) {
			/* Automatically delete the currently selected
			 * text and replace it with what is being
			 * typed in now. Depends on lyxrc settings
			 * "auto_region_delete", which defaults to
			 * true (on). */
#if 0
			// This should not be needed here and is also WRONG!
			setUndo(bv, Undo::INSERT, text_.cursor.par());
#endif
			bv->switchKeyMap();
			if (lyxrc.auto_region_delete) {
				if (text_.selection.set()) {
					text_.cutSelection(false, false);
				}
			}
			text_.clearSelection();
			for (string::size_type i = 0; i < cmd.argument.length(); ++i) {
				bv->owner()->getIntl().getTransManager().
					TranslateAndInsert(cmd.argument[i], &text_);
			}
		}
		text_.selection.cursor = text_.cursor;
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
		if (crow(bv) == text_.rows().begin())
			result = FINISHED_UP;
		else {
			text_.cursorPrevious();
			text_.clearSelection();
			result = DISPATCHED_NOUPDATE;
		}
		updwhat = cursor_update;
		break;

	case LFUN_NEXT:
		if (boost::next(crow(bv)) == text_.rows().end())
			result = FINISHED_DOWN;
		else {
			text_.cursorNext();
			text_.clearSelection();
			result = DISPATCHED_NOUPDATE;
		}
		updwhat = cursor_update;
		break;

	case LFUN_BACKSPACE: {
		if (text_.selection.set())
			text_.cutSelection(true, false);
		else
			text_.backspace();
		updwhat = CURSOR_PAR;
		updflag = true;
		break;
	}

	case LFUN_DELETE: {
		if (text_.selection.set()) {
			text_.cutSelection(true, false);
		} else {
			text_.Delete();
		}
		updwhat = CURSOR_PAR;
		updflag = true;
		break;
	}

	case LFUN_CUT: {
		text_.cutSelection(true, true);
		updwhat = CURSOR_PAR;
		updflag = true;
		break;
	}

	case LFUN_COPY:
		finishUndo();
		text_.copySelection();
		updwhat = CURSOR_PAR;
		break;

	case LFUN_PASTESELECTION:
	{
		string const clip(bv->getClipboard());

		if (clip.empty())
			break;
		if (cmd.argument == "paragraph") {
			text_.insertStringAsParagraphs(clip);
		} else {
			text_.insertStringAsLines(clip);
		}
		// bug 393
		text_.clearSelection();

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

		size_t sel_index = 0;
		string const & arg = cmd.argument;
		if (isStrUnsignedInt(arg)) {
			size_t const paste_arg = strToUnsignedInt(arg);
#warning FIXME Check if the arg is in the domain of available selections.
			sel_index = paste_arg;
		}
		text_.pasteSelection(sel_index);
		// bug 393
		text_.clearSelection();
		updwhat = CURSOR_PAR;
		updflag = true;
		break;
	}

	case LFUN_BREAKPARAGRAPH:
		if (!autoBreakRows) {
			result = DISPATCHED;
			break;
		}
		text_.breakParagraph(paragraphs, 0);
		updwhat = CURSOR | FULL;
		updflag = true;
		break;

	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
		if (!autoBreakRows) {
			result = DISPATCHED;
			break;
		}
		text_.breakParagraph(paragraphs, 1);
		updwhat = CURSOR | FULL;
		updflag = true;
		break;

	case LFUN_BREAKLINE: {
		if (!autoBreakRows) {
			result = DISPATCHED;
			break;
		}

		text_.insertInset(new InsetNewline);
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
			string layout = cmd.argument;
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
				FuncRequest lf(LFUN_MESSAGE, N_("Layout ") + cmd.argument + N_(" not known"));
				bv->owner()->dispatch(lf);
				break;
			}

			if (cur_layout != layout) {
				cur_layout = layout;
				text_.setLayout(layout);
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
		ParagraphList::iterator pit = text_.cursor.par();
		Spacing::Space cur_spacing = pit->params().spacing().getSpace();
		float cur_value = 1.0;
		if (cur_spacing == Spacing::Other) {
			cur_value = pit->params().spacing().getValue();
		}

		istringstream istr(STRCONV(cmd.argument));
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
				   << cmd.argument << endl;
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
		if (!bv->dispatch(cmd))
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
		if (!bv->dispatch(cmd))
			result = UNDISPATCHED;
		break;
	}

	if (updwhat > 0)
		updateLocal(bv, updwhat, updflag);
	/// If the action has deleted all text in the inset, we need to change the
	// language to the language of the surronding text.
	if (!was_empty && paragraphs.begin()->empty() &&
	    paragraphs.size() == 1) {
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
		     LatexRunParams const & runparams) const
{
	TexRow texrow;
	latexParagraphs(buf, paragraphs, os, texrow, runparams);
	return texrow.rows();
}


int InsetText::ascii(Buffer const * buf, ostream & os, int linelen) const
{
	unsigned int lines = 0;

	ParagraphList::const_iterator beg = paragraphs.begin();
	ParagraphList::const_iterator end = paragraphs.end();
	ParagraphList::const_iterator it = beg;
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

	ParagraphList::iterator pit = const_cast<ParagraphList&>(paragraphs).begin();
	ParagraphList::iterator pend = const_cast<ParagraphList&>(paragraphs).end();

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
			buf->error(ErrorItem(_("Error"), _("LatexType Command not allowed here.\n"), pit->id(), 0, pit->size()));
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


void InsetText::getLabelList(std::vector<string> & list) const
{
	ParagraphList::const_iterator pit = paragraphs.begin();
	ParagraphList::const_iterator pend = paragraphs.end();
	for (; pit != pend; ++pit) {
		InsetList::const_iterator beg = pit->insetlist.begin();
		InsetList::const_iterator end = pit->insetlist.end();
		for (; beg != end; ++beg)
			beg->inset->getLabelList(list);
	}
}


void InsetText::setFont(BufferView * bv, LyXFont const & font, bool toggleall,
			bool selectall)
{
	if (the_locking_inset) {
		the_locking_inset->setFont(bv, font, toggleall, selectall);
		return;
	}

	if ((paragraphs.size() == 1 && paragraphs.begin()->empty())
	    || cpar(bv)->empty()) {
		getLyXText(bv)->setFont(font, toggleall);
		return;
	}


	if (text_.selection.set()) {
		setUndo(bv, Undo::EDIT, text_.cursor.par());
	}

	if (selectall)
		selectAll(bv);

	text_.toggleFree(font, toggleall);

	if (selectall)
		text_.clearSelection();

	bv->fitCursor();

	bool flag = (selectall || text_.selection.set());

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
		FuncRequest cmd(bv, LFUN_INSET_EDIT, front ? "left" : "right");
		inset->localDispatch(cmd);
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
	int dummyy = y + dim_.asc;
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
			x = dim_.wid;
		if (y < 0)
			y = dim_.des;
		inset_x = cix(bv) - top_x + drawTextXOffset;
		inset_y = ciy(bv) + drawTextYOffset;
		FuncRequest cmd(bv, LFUN_INSET_EDIT, x - inset_x, y - inset_y, button);
		inset->localDispatch(cmd);
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


void InsetText::setParagraphData(ParagraphList const & plist)
{
	// we have to unlock any locked inset otherwise we're in troubles
	the_locking_inset = 0;

	// But it it makes no difference that is a lot better.
#warning FIXME.
	// See if this can be simplified when std::list is in effect.
	paragraphs.clear();

	ParagraphList::const_iterator it = plist.begin();
	ParagraphList::const_iterator end = plist.end();
	for (; it != end; ++it) {
		paragraphs.push_back(*it);
		Paragraph & tmp = paragraphs.back();
		tmp.setInsetOwner(this);
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
	int x = text_.cursor.x() + top_x + TEXT_TO_INSET_OFFSET;
	if (the_locking_inset) {
		LyXFont font = text_.getFont(bv->buffer(), text_.cursor.par(),
					    text_.cursor.pos());
		if (font.isVisibleRightToLeft())
			x -= the_locking_inset->width(bv, font);
	}
	return x;
}


int InsetText::cix(BufferView * bv) const
{
	int x = text_.cursor.ix() + top_x + TEXT_TO_INSET_OFFSET;
	if (the_locking_inset) {
		LyXFont font = text_.getFont(bv->buffer(), text_.cursor.par(),
					    text_.cursor.pos());
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
	return getLyXText(bv)->cursorRow();
}


LyXText * InsetText::getLyXText(BufferView const * bv,
				bool const recursive) const
{
	setViewCache(bv);
	if (recursive && the_locking_inset)
		return the_locking_inset->getLyXText(bv, true);
	return &text_;
}


void InsetText::setViewCache(BufferView const * bv) const
{
	if (bv)
		text_.bv_owner = const_cast<BufferView *>(bv);
}


void InsetText::deleteLyXText(BufferView * bv, bool recursive) const
{
	if (recursive) {
		/// then remove all LyXText in text-insets
		for_each(const_cast<ParagraphList&>(paragraphs).begin(),
			 const_cast<ParagraphList&>(paragraphs).end(),
			 boost::bind(&Paragraph::deleteInsetsLyXText, _1, bv));
	}
}


void InsetText::resizeLyXText(BufferView * bv, bool force) const
{
	if (paragraphs.size() == 1 && paragraphs.begin()->empty()) {
		// no data, resize not neccessary!
		// we have to do this as a fixed width may have changed!
		saveLyXTextState(&text_);
		text_.init(bv, true);
		restoreLyXTextState(&text_);
		return;
	}

	if (!bv)
		return;

	Assert(bv);
	text_.bv_owner = bv;

	// one endless line, resize normally not necessary
	if (!force && getMaxWidth(bv, this) < 0)
		return;

	LyXText * t = &text_;
	saveLyXTextState(t);

	for_each(const_cast<ParagraphList&>(paragraphs).begin(),
		 const_cast<ParagraphList&>(paragraphs).end(),
		 boost::bind(&Paragraph::resizeInsetsLyXText, _1, bv));

	t->init(bv, true);
	restoreLyXTextState(t);

	if (the_locking_inset) {
		inset_x = cix(bv) - top_x + drawTextXOffset;
		inset_y = ciy(bv) + drawTextYOffset;
	}

	t->top_y(bv->screen().topCursorVisible(t));
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
	LyXText * t = &text_;
	BufferView * bv = text_.bv_owner;

	if (!bv)
		return;

	saveLyXTextState(t);

	for_each(const_cast<ParagraphList&>(paragraphs).begin(),
		 const_cast<ParagraphList&>(paragraphs).end(),
		 boost::bind(&Paragraph::resizeInsetsLyXText, _1, bv));

	t->init(bv, true);
	restoreLyXTextState(t);
	if (the_locking_inset) {
		inset_x = cix(bv) - top_x + drawTextXOffset;
		inset_y = ciy(bv) + drawTextYOffset;
	}
	t->top_y(bv->screen().topCursorVisible(t));
	if (!owner()) {
		const_cast<InsetText*>(this)->updateLocal(bv, FULL, false);
		// this will scroll the screen such that the cursor becomes visible
		bv->updateScrollbar();
	} else {
		need_update = FULL;
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
	int w = dim_.wid;
	int h = dim_.asc + dim_.des;
	int ty = baseline - dim_.asc;

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


Inset * InsetText::getInsetFromID(int id_arg) const
{
	if (id_arg == id())
		return const_cast<InsetText *>(this);

	ParagraphList::const_iterator pit = paragraphs.begin();
	ParagraphList::const_iterator pend = paragraphs.end();
	for (; pit != pend; ++pit) {
		InsetList::const_iterator it = pit->insetlist.begin();
		InsetList::const_iterator end = pit->insetlist.end();
		for (; it != end; ++it) {
			if (it->inset->id() == id_arg)
				return it->inset;
			Inset * in = it->inset->getInsetFromID(id_arg);
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
	WordLangTuple word;
	if (the_locking_inset) {
		word = the_locking_inset->selectNextWordToSpellcheck(bv, value);
		if (!word.word().empty()) {
			value += cy(bv);
			return word;
		}
		// we have to go on checking so move cursor to the next char
		text_.cursor.pos(text_.cursor.pos() + 1);
	}
	word = text_.selectNextWordToSpellcheck(value);
	if (word.word().empty())
		bv->unlockInset(const_cast<InsetText *>(this));
	else
		value = cy(bv);
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

	int x = top_x + TEXT_TO_INSET_OFFSET;

	RowList::iterator rowit = text_.rows().begin();
	RowList::iterator end = text_.rows().end();
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
	bv->screen().toggleSelection(&text_, bv, kill_selection, y_offset, x);
}


bool InsetText::nextChange(BufferView * bv, lyx::pos_type & length)
{
	if (the_locking_inset) {
		if (the_locking_inset->nextChange(bv, length))
			return true;
		text_.cursorRight(true);
	}
	lyxfind::SearchResult result =
		lyxfind::findNextChange(bv, &text_, length);

	if (result == lyxfind::SR_FOUND) {
		LyXCursor cur = text_.cursor;
		bv->unlockInset(bv->theLockingInset());
		if (bv->lockInset(this))
			locked = true;
		text_.cursor = cur;
		text_.setSelectionRange(length);
		updateLocal(bv, SELECTION, false);
	}
	return result != lyxfind::SR_NOT_FOUND;
}


bool InsetText::searchForward(BufferView * bv, string const & str,
			      bool cs, bool mw)
{
	if (the_locking_inset) {
		if (the_locking_inset->searchForward(bv, str, cs, mw))
			return true;
		text_.cursorRight(true);
	}
	lyxfind::SearchResult result =
		lyxfind::LyXFind(bv, &text_, str, true, cs, mw);

	if (result == lyxfind::SR_FOUND) {
		LyXCursor cur = text_.cursor;
		bv->unlockInset(bv->theLockingInset());
		if (bv->lockInset(this))
			locked = true;
		text_.cursor = cur;
		text_.setSelectionRange(str.length());
		updateLocal(bv, SELECTION, false);
	}
	return (result != lyxfind::SR_NOT_FOUND);
}

bool InsetText::searchBackward(BufferView * bv, string const & str,
			       bool cs, bool mw)
{
	if (the_locking_inset) {
		if (the_locking_inset->searchBackward(bv, str, cs, mw))
			return true;
	}
	if (!locked) {
		ParagraphList::iterator pit = paragraphs.begin();
		ParagraphList::iterator pend = paragraphs.end();

		while (boost::next(pit) != pend)
			++pit;

		text_.setCursor(pit, pit->size());
	}
	lyxfind::SearchResult result =
		lyxfind::LyXFind(bv, &text_, str, false, cs, mw);

	if (result == lyxfind::SR_FOUND) {
		LyXCursor cur = text_.cursor;
		bv->unlockInset(bv->theLockingInset());
		if (bv->lockInset(this))
			locked = true;
		text_.cursor = cur;
		text_.setSelectionRange(str.length());
		updateLocal(bv, SELECTION, false);
	}
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
	while (paragraphs.size() > 1) {
		ParagraphList::iterator first_par = paragraphs.begin();
		ParagraphList::iterator next_par = boost::next(first_par);
		size_t const first_par_size = first_par->size();

		if (!first_par->empty() &&
		    !next_par->empty() &&
		    !first_par->isSeparator(first_par_size - 1)) {
			first_par->insertChar(first_par_size, ' ');
		}

		if (text_.selection.set()) {
			if (text_.selection.start.par() == next_par) {
				text_.selection.start.par(first_par);
				text_.selection.start.pos(
					text_.selection.start.pos() + first_par_size);
			}
			if (text_.selection.end.par() == next_par) {
				text_.selection.end.par(first_par);
				text_.selection.end.pos(
					text_.selection.end.pos() + first_par_size);
			}
		}

		mergeParagraph(bv->buffer()->params, paragraphs, first_par);
	}
	reinitLyXText();
}


void InsetText::getDrawFont(LyXFont & font) const
{
	if (!owner())
		return;
	owner()->getDrawFont(font);
}


void InsetText::appendParagraphs(Buffer * buffer, ParagraphList & plist)
{
#warning FIXME Check if Changes stuff needs changing here. (Lgb)
// And it probably does. You have to take a look at this John. (Lgb)
#warning John, have a look here. (Lgb)
	ParagraphList::iterator pit = plist.begin();
	ParagraphList::iterator ins = paragraphs.insert(paragraphs.end(), *pit);
	++pit;
	mergeParagraph(buffer->params, paragraphs, boost::prior(ins));

	ParagraphList::iterator pend = plist.end();
	for (; pit != pend; ++pit) {
		paragraphs.push_back(*pit);
	}

	reinitLyXText();
}


void InsetText::addPreview(grfx::PreviewLoader & loader) const
{
	ParagraphList::const_iterator pit = paragraphs.begin();
	ParagraphList::const_iterator pend = paragraphs.end();

	for (; pit != pend; ++pit) {
		InsetList::const_iterator it  = pit->insetlist.begin();
		InsetList::const_iterator end = pit->insetlist.end();
		for (; it != end; ++it) {
			it->inset->addPreview(loader);
		}
	}
}
