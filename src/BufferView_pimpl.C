/**
 * \file BufferView_pimpl.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author various
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "BufferView_pimpl.h"
#include "frontends/WorkArea.h"
#include "frontends/screen.h"
#include "frontends/LyXScreenFactory.h"
#include "frontends/WorkAreaFactory.h"
#include "frontends/Dialogs.h"
#include "frontends/Alert.h"
#include "frontends/FileDialog.h"
#include "frontends/mouse_state.h"
#include "lyxtext.h"
#include "lyxrow.h"
#include "paragraph.h"
#include "frontends/LyXView.h"
#include "commandtags.h"
#include "lyxfunc.h"
#include "debug.h"
#include "bufferview_funcs.h"
#include "TextCache.h"
#include "bufferlist.h"
#include "lyxrc.h"
#include "intl.h"
// added for Dispatch functions
#include "lyx_cb.h"
#include "lyx_main.h"
#include "FloatList.h"
#include "gettext.h"
#include "ParagraphParameters.h"
#include "undo_funcs.h"
#include "funcrequest.h"

#include "insets/insetbib.h"
#include "insets/insettext.h"
#include "insets/inseturl.h"
#include "insets/insetlatexaccent.h"
#include "insets/insettoc.h"
#include "insets/insetindex.h"
#include "insets/insetref.h"
#include "insets/insetinclude.h"
#include "insets/insetcite.h"
#include "insets/insetgraphics.h"
#include "insets/insetmarginal.h"
#include "insets/insetfloatlist.h"

#include "mathed/formulabase.h"

#include "graphics/Previews.h"

#include "support/LAssert.h"
#include "support/lstrings.h"
#include "support/filetools.h"

#include <boost/bind.hpp>
#include <boost/signals/connection.hpp>

#include <unistd.h>
#include <sys/wait.h>


using std::vector;
using std::find_if;
using std::find;
using std::pair;
using std::endl;
using std::make_pair;
using std::min;

using lyx::pos_type;

extern BufferList bufferlist;


namespace {

unsigned int const saved_positions_num = 20;

// All the below connection objects are needed because of a bug in some
// versions of GCC (<=2.96 are on the suspects list.) By having and assigning
// to these connections we avoid a segfault upon startup, and also at exit.
// (Lgb)

boost::signals::connection dispatchcon;
boost::signals::connection timecon;
boost::signals::connection doccon;
boost::signals::connection resizecon;
boost::signals::connection kpresscon;
boost::signals::connection selectioncon;
boost::signals::connection lostcon;


} // anon namespace


BufferView::Pimpl::Pimpl(BufferView * bv, LyXView * owner,
	     int xpos, int ypos, int width, int height)
	: bv_(bv), owner_(owner), buffer_(0), cursor_timeout(400),
	  using_xterm_cursor(false)
{
	workarea_.reset(WorkAreaFactory::create(xpos, ypos, width, height));
	screen_.reset(LyXScreenFactory::create(workarea()));

	// Setup the signals
	doccon = workarea().scrollDocView
		.connect(boost::bind(&BufferView::Pimpl::scrollDocView, this, _1));
	resizecon = workarea().workAreaResize
		.connect(boost::bind(&BufferView::Pimpl::workAreaResize, this));
	dispatchcon = workarea().dispatch
		.connect(boost::bind(&BufferView::Pimpl::dispatch, this, _1));
	kpresscon = workarea().workAreaKeyPress
		.connect(boost::bind(&BufferView::Pimpl::workAreaKeyPress, this, _1, _2));
	selectioncon = workarea().selectionRequested
		.connect(boost::bind(&BufferView::Pimpl::selectionRequested, this));
	lostcon = workarea().selectionLost
		.connect(boost::bind(&BufferView::Pimpl::selectionLost, this));

	timecon = cursor_timeout.timeout
		.connect(boost::bind(&BufferView::Pimpl::cursorToggle, this));
	cursor_timeout.start();
	saved_positions.resize(saved_positions_num);
}


WorkArea & BufferView::Pimpl::workarea() const
{
	return *workarea_.get();
}


LyXScreen & BufferView::Pimpl::screen() const
{
	return *screen_.get();
}


Painter & BufferView::Pimpl::painter() const
{
	return workarea().getPainter();
}


void BufferView::Pimpl::buffer(Buffer * b)
{
	lyxerr[Debug::INFO] << "Setting buffer in BufferView ("
			    << b << ")" << endl;
	if (buffer_) {
		buffer_->delUser(bv_);

		// Put the old text into the TextCache, but
		// only if the buffer is still loaded.
		// Also set the owner of the test to 0
		//		bv_->text->owner(0);
		textcache.add(buffer_, workarea().workWidth(), bv_->text);
		if (lyxerr.debugging())
			textcache.show(lyxerr, "BufferView::buffer");

		bv_->text = 0;
	}

	// set current buffer
	buffer_ = b;

	if (bufferlist.getState() == BufferList::CLOSING) return;

	// if we are closing the buffer, use the first buffer as current
	if (!buffer_) {
		buffer_ = bufferlist.first();
	}

	if (buffer_) {
		lyxerr[Debug::INFO] << "Buffer addr: " << buffer_ << endl;
		buffer_->addUser(bv_);

		// If we don't have a text object for this, we make one
		if (bv_->text == 0) {
			resizeCurrentBuffer();
		}

		// FIXME: needed when ?
		bv_->text->first_y =
			screen().topCursorVisible(bv_->text->cursor, bv_->text->first_y);

		// Similarly, buffer-dependent dialogs should be updated or
		// hidden. This should go here because some dialogs (eg ToC)
		// require bv_->text.
		owner_->getDialogs().updateBufferDependent(true);
	} else {
		lyxerr[Debug::INFO] << "  No Buffer!" << endl;
		owner_->getDialogs().hideBufferDependent();

		// Also remove all remaining text's from the testcache.
		// (there should not be any!) (if there is any it is a
		// bug!)
		if (lyxerr.debugging())
			textcache.show(lyxerr, "buffer delete all");
		textcache.clear();
	}

	repaint();
	updateScrollbar();
	owner_->updateMenubar();
	owner_->updateToolbar();
	owner_->updateLayoutChoice();
	owner_->updateWindowTitle();

	if (grfx::Previews::activated() && buffer_)
		grfx::Previews::get().generateBufferPreviews(*buffer_);
}


bool BufferView::Pimpl::fitCursor()
{
	bool ret;

	if (bv_->theLockingInset()) {
		bv_->theLockingInset()->fitInsetCursor(bv_);
		ret = true;
	} else {
		ret = screen().fitCursor(bv_->text, bv_);
	}

	bv_->owner()->getDialogs().updateParagraph();
	if (ret)
		updateScrollbar();
	return ret;
}


void BufferView::Pimpl::redoCurrentBuffer()
{
	lyxerr[Debug::INFO] << "BufferView::redoCurrentBuffer" << endl;
	if (buffer_ && bv_->text) {
		resizeCurrentBuffer();
		updateScrollbar();
		owner_->updateLayoutChoice();
		repaint();
	}
}


int BufferView::Pimpl::resizeCurrentBuffer()
{
	lyxerr[Debug::INFO] << "resizeCurrentBuffer" << endl;

	Paragraph * par = 0;
	Paragraph * selstartpar = 0;
	Paragraph * selendpar = 0;
	UpdatableInset * the_locking_inset = 0;

	pos_type pos = 0;
	pos_type selstartpos = 0;
	pos_type selendpos = 0;
	bool selection = false;
	bool mark_set  = false;

	owner_->prohibitInput();

	owner_->message(_("Formatting document..."));

	if (bv_->text) {
		par = bv_->text->cursor.par();
		pos = bv_->text->cursor.pos();
		selstartpar = bv_->text->selection.start.par();
		selstartpos = bv_->text->selection.start.pos();
		selendpar = bv_->text->selection.end.par();
		selendpos = bv_->text->selection.end.pos();
		selection = bv_->text->selection.set();
		mark_set = bv_->text->selection.mark();
		the_locking_inset = bv_->theLockingInset();
		buffer_->resizeInsets(bv_);
		// I don't think the delete and new are necessary here we just could
		// call only init! (Jug 20020419)
		delete bv_->text;
		bv_->text = new LyXText(bv_);
		bv_->text->init(bv_);
	} else {
		// See if we have a text in TextCache that fits
		// the new buffer_ with the correct width.
		bv_->text = textcache.findFit(buffer_, workarea().workWidth());
		if (bv_->text) {
			if (lyxerr.debugging()) {
				lyxerr << "Found a LyXText that fits:\n";
				textcache.show(lyxerr, make_pair(buffer_, make_pair(workarea().workWidth(), bv_->text)));
			}
			// Set the owner of the newly found text
			//	bv_->text->owner(bv_);
			if (lyxerr.debugging())
				textcache.show(lyxerr, "resizeCurrentBuffer");
		} else {
			bv_->text = new LyXText(bv_);
			bv_->text->init(bv_);
			//buffer_->resizeInsets(bv_);
		}
	}

	if (par) {
		bv_->text->selection.set(true);
		// At this point just to avoid the Delete-Empty-Paragraph-
		// Mechanism when setting the cursor.
		bv_->text->selection.mark(mark_set);
		if (selection) {
			bv_->text->setCursor(bv_, selstartpar, selstartpos);
			bv_->text->selection.cursor = bv_->text->cursor;
			bv_->text->setCursor(bv_, selendpar, selendpos);
			bv_->text->setSelection(bv_);
			bv_->text->setCursor(bv_, par, pos);
		} else {
			bv_->text->setCursor(bv_, par, pos);
			bv_->text->selection.cursor = bv_->text->cursor;
			bv_->text->selection.set(false);
		}
		// remake the inset locking
		bv_->theLockingInset(the_locking_inset);
	}

	bv_->text->first_y = screen().topCursorVisible(bv_->text->cursor, bv_->text->first_y);

	switchKeyMap();
	owner_->allowInput();

	updateScrollbar();

	return 0;
}


void BufferView::Pimpl::repaint()
{
	// Regenerate the screen.
	screen().redraw(bv_->text, bv_);
}


void BufferView::Pimpl::updateScrollbar()
{
	if (!bv_->text) {
		lyxerr[Debug::GUI] << "no text in updateScrollbar" << endl;
		workarea().setScrollbarParams(0, 0, 0);
		return;
	}

	LyXText const & t = *bv_->text;

	lyxerr[Debug::GUI] << "Updating scrollbar: h " << t.height << ", first_y "
		<< t.first_y << ", default height " << t.defaultHeight() << endl;

	workarea().setScrollbarParams(t.height, t.first_y, t.defaultHeight());
}


void BufferView::Pimpl::scrollDocView(int value)
{
	lyxerr[Debug::GUI] << "scrollDocView of " << value << endl;

	if (!buffer_) return;

	screen().draw(bv_->text, bv_, value);

	if (!lyxrc.cursor_follows_scrollbar) {
		return;
	}

	LyXText * vbt = bv_->text;

	int const height = vbt->defaultHeight();
	int const first = static_cast<int>((bv_->text->first_y + height));
	int const last = static_cast<int>((bv_->text->first_y + workarea().workHeight() - height));

	if (vbt->cursor.y() < first)
		vbt->setCursorFromCoordinates(bv_, 0, first);
	else if (vbt->cursor.y() > last)
		vbt->setCursorFromCoordinates(bv_, 0, last);
}


int BufferView::Pimpl::scroll(long time)
{
	if (!buffer_)
		return 0;

	LyXText const * t = bv_->text;

	double const diff = t->defaultHeight()
		+ double(time) * double(time) * 0.125;

	scrollDocView(int(diff));
	workarea().setScrollbarParams(t->height, t->first_y, t->defaultHeight());
	return 0;
}


void BufferView::Pimpl::workAreaKeyPress(LyXKeySymPtr key,
					 key_modifier::state state)
{
	bv_->owner()->getLyXFunc().processKeySym(key, state);
}


void BufferView::Pimpl::selectionRequested()
{
	static string sel;

	if (!available())
		return;

	LyXText * text = bv_->getLyXText();

	if (text->selection.set() &&
		(!bv_->text->xsel_cache.set() ||
		 text->selection.start != bv_->text->xsel_cache.start ||
		 text->selection.end != bv_->text->xsel_cache.end))
	{
		bv_->text->xsel_cache = text->selection;
		sel = text->selectionAsString(bv_->buffer(), false);
	} else if (!text->selection.set()) {
		sel = string();
		bv_->text->xsel_cache.set(false);
	}
	if (!sel.empty()) {
		workarea().putClipboard(sel);
	}
}


void BufferView::Pimpl::selectionLost()
{
	if (available()) {
		hideCursor();
		toggleSelection();
		bv_->getLyXText()->clearSelection();
		showCursor();
		bv_->text->xsel_cache.set(false);
	}
}


void BufferView::Pimpl::workAreaResize()
{
	static int work_area_width;
	static int work_area_height;

	bool const widthChange = workarea().workWidth() != work_area_width;
	bool const heightChange = workarea().workHeight() != work_area_height;

	// update from work area
	work_area_width = workarea().workWidth();
	work_area_height = workarea().workHeight();

	if (buffer_ != 0) {
		if (widthChange) {
			// The visible LyXView need a resize
			resizeCurrentBuffer();

			// Remove all texts from the textcache
			// This is not _really_ what we want to do. What
			// we really want to do is to delete in textcache
			// that does not have a BufferView with matching
			// width, but as long as we have only one BufferView
			// deleting all gives the same result.
			if (lyxerr.debugging())
				textcache.show(lyxerr, "Expose delete all");
			textcache.clear();
			// FIXME: this is already done in resizeCurrentBuffer() ??
			buffer_->resizeInsets(bv_);
		} else if (heightChange) {
			// fitCursor() ensures we don't jump back
			// to the start of the document on vertical
			// resize
			fitCursor();
		}
	}

	if (widthChange || heightChange) {
		repaint();
	}

	// always make sure that the scrollbar is sane.
	updateScrollbar();
	owner_->updateLayoutChoice();
	return;
}


void BufferView::Pimpl::update()
{
	if (!bv_->theLockingInset() || !bv_->theLockingInset()->nodraw()) {
		LyXText::text_status st = bv_->text->status();
		screen().update(bv_->text, bv_);
		bool fitc = false;
		while (bv_->text->status() == LyXText::CHANGED_IN_DRAW) {
			bv_->text->fullRebreak(bv_);
			st = LyXText::NEED_MORE_REFRESH;
			bv_->text->setCursor(bv_, bv_->text->cursor.par(),
					     bv_->text->cursor.pos());
			if (bv_->text->selection.set()) {
				bv_->text->setCursor(bv_, bv_->text->selection.start,
						     bv_->text->selection.start.par(),
						     bv_->text->selection.start.pos());
				bv_->text->setCursor(bv_, bv_->text->selection.end,
						     bv_->text->selection.end.par(),
						     bv_->text->selection.end.pos());
			}
			fitc = true;
			bv_->text->status(bv_, st);
			screen().update(bv_->text, bv_);
		}
		// do this here instead of in the screen::update because of
		// the above loop!
		bv_->text->status(bv_, LyXText::UNCHANGED);
		if (fitc)
			fitCursor();
	}
}

// Values used when calling update:
// -3 - update
// -2 - update, move sel_cursor if selection, fitcursor
// -1 - update, move sel_cursor if selection, fitcursor, mark dirty
//  0 - update, move sel_cursor if selection, fitcursor
//  1 - update, move sel_cursor if selection, fitcursor, mark dirty
//  3 - update, move sel_cursor if selection
//
// update -
// a simple redraw of the parts that need refresh
//
// move sel_cursor if selection -
// the text's sel_cursor is moved if there is selection is progress
//
// fitcursor -
// fitCursor() is called and the scrollbar updated
//
// mark dirty -
// the buffer is marked dirty.
//
// enum {
//       UPDATE = 0,
//       SELECT = 1,
//       FITCUR = 2,
//       CHANGE = 4
// };
//
// UPDATE_ONLY = UPDATE;
// UPDATE_SELECT = UPDATE | SELECT;
// UPDATE_SELECT_MOVE = UPDATE | SELECT | FITCUR;
// UPDATE_SELECT_MOVE_AFTER_CHANGE = UPDATE | SELECT | FITCUR | CHANGE;
//
// update(-3) -> update(0)         -> update(0) -> update(UPDATE)
// update(-2) -> update(1 + 2)     -> update(3) -> update(SELECT|FITCUR)
// update(-1) -> update(1 + 2 + 4) -> update(7) -> update(SELECT|FITCUR|CHANGE)
// update(1)  -> update(1 + 2 + 4) -> update(7) -> update(SELECT|FITCUR|CHANGE)
// update(3)  -> update(1)         -> update(1) -> update(SELECT)

void BufferView::Pimpl::update(LyXText * text, BufferView::UpdateCodes f)
{
	owner_->updateLayoutChoice();

	if (!text->selection.set() && (f & SELECT)) {
		text->selection.cursor = text->cursor;
	}

	text->fullRebreak(bv_);

	if (text->inset_owner) {
		text->inset_owner->setUpdateStatus(bv_, InsetText::NONE);
		updateInset(text->inset_owner, false);
	} else {
		update();
	}

	if ((f & FITCUR)) {
		fitCursor();
	}

	if ((f & CHANGE)) {
		buffer_->markDirty();
	}
}


// Callback for cursor timer
void BufferView::Pimpl::cursorToggle()
{
	if (!buffer_) {
		cursor_timeout.restart();
		return;
	}

	/* FIXME */
	extern void reapSpellchecker(void);
	reapSpellchecker();

	if (!bv_->theLockingInset()) {
		screen().cursorToggle(bv_);
	} else {
		bv_->theLockingInset()->toggleInsetCursor(bv_);
	}

	cursor_timeout.restart();
}


bool BufferView::Pimpl::available() const
{
	if (buffer_ && bv_->text)
		return true;
	return false;
}


void BufferView::Pimpl::beforeChange(LyXText * text)
{
	toggleSelection();
	text->clearSelection();
}


void BufferView::Pimpl::savePosition(unsigned int i)
{
	if (i >= saved_positions_num)
		return;
	saved_positions[i] = Position(buffer_->fileName(),
				      bv_->text->cursor.par()->id(),
				      bv_->text->cursor.pos());
	if (i > 0) {
		ostringstream str;
		str << _("Saved bookmark") << ' ' << i;
		owner_->message(str.str().c_str());
	}
}


void BufferView::Pimpl::restorePosition(unsigned int i)
{
	if (i >= saved_positions_num)
		return;

	string const fname = saved_positions[i].filename;

	beforeChange(bv_->text);

	if (fname != buffer_->fileName()) {
		Buffer * b = bufferlist.exists(fname) ?
			bufferlist.getBuffer(fname) :
			bufferlist.loadLyXFile(fname); // don't ask, just load it
		if (b != 0) buffer(b);
	}

	Paragraph * par = buffer_->getParFromID(saved_positions[i].par_id);
	if (!par)
		return;

	bv_->text->setCursor(bv_, par,
			     min(par->size(), saved_positions[i].par_pos));

	update(bv_->text, BufferView::SELECT | BufferView::FITCUR);
	if (i > 0) {
		ostringstream str;
		str << _("Moved to bookmark") << ' ' << i;
		owner_->message(str.str().c_str());
	}
}


bool BufferView::Pimpl::isSavedPosition(unsigned int i)
{
	if (i >= saved_positions_num)
		return false;

	return !saved_positions[i].filename.empty();
}


void BufferView::Pimpl::switchKeyMap()
{
	if (!lyxrc.rtl_support)
		return;

	LyXText * text = bv_->getLyXText();
	if (text->real_current_font.isRightToLeft()
	    && !(bv_->theLockingInset()
		 && bv_->theLockingInset()->lyxCode() == Inset::ERT_CODE))
	{
		if (owner_->getIntl().keymap == Intl::PRIMARY)
			owner_->getIntl().KeyMapSec();
	} else {
		if (owner_->getIntl().keymap == Intl::SECONDARY)
			owner_->getIntl().KeyMapPrim();
	}
}


void BufferView::Pimpl::insetUnlock()
{
	if (bv_->theLockingInset()) {
		bv_->theLockingInset()->insetUnlock(bv_);
		bv_->theLockingInset(0);
		finishUndo();
	}
}


void BufferView::Pimpl::showCursor()
{
	if (bv_->theLockingInset())
		bv_->theLockingInset()->showInsetCursor(bv_);
	else
		screen().showCursor(bv_->text, bv_);
}


void BufferView::Pimpl::hideCursor()
{
	if (!bv_->theLockingInset())
		screen().hideCursor();
}


void BufferView::Pimpl::toggleSelection(bool b)
{
	if (bv_->theLockingInset())
		bv_->theLockingInset()->toggleSelection(bv_, b);
	screen().toggleSelection(bv_->text, bv_, b);
}


void BufferView::Pimpl::toggleToggle()
{
	screen().toggleToggle(bv_->text, bv_);
}


void BufferView::Pimpl::center()
{
	LyXText * t = bv_->text;

	beforeChange(t);
	int const half_height = workarea().workHeight() / 2;
	int new_y = 0;

	if (t->cursor.y() > half_height) {
		new_y = t->cursor.y() - half_height;
	}

	// FIXME: can we do this w/o calling screen directly ?
	// This updates first_y but means the fitCursor() call
	// from the update(FITCUR) doesn't realise that we might
	// have moved (e.g. from GOTOPARAGRAPH), so doesn't cause
	// the scrollbar to be updated as it should, so we have
	// to do it manually. Any operation that does a center()
	// and also might have moved first_y must make sure to call
	// updateScrollbar() currently. Never mind that this is a
	// pretty obfuscated way of updating t->first_y
	screen().draw(t, bv_, new_y);

	update(t, BufferView::SELECT | BufferView::FITCUR);
}


void BufferView::Pimpl::stuffClipboard(string const & stuff) const
{
	workarea().putClipboard(stuff);
}


/*
 * Dispatch functions for actions which can be valid for BufferView->text
 * and/or InsetText->text!!!
 */


Inset * BufferView::Pimpl::getInsetByCode(Inset::Code code)
{
#if 0
	LyXCursor cursor = bv_->getLyXText()->cursor;
	Buffer::inset_iterator it =
		find_if(Buffer::inset_iterator(
			cursor.par(), cursor.pos()),
			buffer_->inset_iterator_end(),
			lyx::compare_memfun(&Inset::lyxCode, code));
	return it != buffer_->inset_iterator_end() ? (*it) : 0;
#else
	// Ok, this is a little bit too brute force but it
	// should work for now. Better infrastructure is comming. (Lgb)

	Buffer * b = bv_->buffer();
	LyXCursor cursor = bv_->getLyXText()->cursor;

	Buffer::inset_iterator beg = b->inset_iterator_begin();
	Buffer::inset_iterator end = b->inset_iterator_end();

	bool cursor_par_seen = false;

	for (; beg != end; ++beg) {
		if (beg.getPar() == cursor.par()) {
			cursor_par_seen = true;
		}
		if (cursor_par_seen) {
			if (beg.getPar() == cursor.par()
			    && beg.getPos() >= cursor.pos()) {
				break;
			} else if (beg.getPar() != cursor.par()) {
				break;
			}
		}

	}
	if (beg != end) {
		// Now find the first inset that matches code.
		for (; beg != end; ++beg) {
			if (beg->lyxCode() == code) {
				return &(*beg);
			}
		}
	}
	return 0;
#endif
}


void BufferView::Pimpl::MenuInsertLyXFile(string const & filen)
{
	string filename = filen;

	if (filename.empty()) {
		// Launch a file browser
		string initpath = lyxrc.document_path;

		if (available()) {
			string const trypath = owner_->buffer()->filePath();
			// If directory is writeable, use this as default.
			if (IsDirWriteable(trypath))
				initpath = trypath;
		}

		FileDialog fileDlg(bv_->owner(),
				   _("Select LyX document to insert"),
			LFUN_FILE_INSERT,
			make_pair(string(_("Documents|#o#O")),
				  string(lyxrc.document_path)),
			make_pair(string(_("Examples|#E#e")),
				  string(AddPath(system_lyxdir, "examples"))));

		FileDialog::Result result =
			fileDlg.Select(initpath,
				       _("*.lyx| LyX Documents (*.lyx)"));

		if (result.first == FileDialog::Later)
			return;

		filename = result.second;

		// check selected filename
		if (filename.empty()) {
			owner_->message(_("Canceled."));
			return;
		}
	}

	// get absolute path of file and add ".lyx" to the filename if
	// necessary
	filename = FileSearch(string(), filename, "lyx");

	string const disp_fn(MakeDisplayPath(filename));

	ostringstream s1;
	s1 << _("Inserting document") << ' '
	   << disp_fn << " ...";
	owner_->message(s1.str().c_str());
	bool const res = bv_->insertLyXFile(filename);
	if (res) {
		ostringstream str;
		str << _("Document") << ' ' << disp_fn
		    << ' ' << _("inserted.");
		owner_->message(str.str().c_str());
	} else {
		ostringstream str;
		str << _("Could not insert document") << ' '
		    << disp_fn;
		owner_->message(str.str().c_str());
	}
}


bool BufferView::Pimpl::dispatch(FuncRequest const & ev)
{
	lyxerr[Debug::ACTION] << "BufferView::Pimpl::Dispatch:"
		<< " action[" << ev.action <<"]"
		<< " arg[" << ev.argument << "]"
		<< " x[" << ev.x << "]"
		<< " y[" << ev.y << "]"
		<< " button[" << ev.button() << "]"
		<< endl;

	LyXTextClass const & tclass = buffer_->params.getLyXTextClass();

	switch (ev.action) {

	case LFUN_SCROLL_INSET:
		// this is not handled here as this function is only active
		// if we have a locking_inset and that one is (or contains)
		// a tabular-inset
		break;

	case LFUN_INSET_GRAPHICS:
	{
		Inset * new_inset = new InsetGraphics;
		if (!insertInset(new_inset)) {
			delete new_inset;
		} else {
			// this is need because you don't use a inset->Edit()
			updateInset(new_inset, true);
			new_inset->edit(bv_);
		}
		break;
	}

	case LFUN_LAYOUT_COPY:
		bv_->copyEnvironment();
		break;

	case LFUN_LAYOUT_PASTE:
		bv_->pasteEnvironment();
		switchKeyMap();
		break;

	case LFUN_DEPTH_MIN:
		changeDepth(bv_, bv_->getLyXText(), -1);
		break;

	case LFUN_DEPTH_PLUS:
		changeDepth(bv_, bv_->getLyXText(), 1);
		break;

	case LFUN_FREE:
		owner_->getDialogs().setUserFreeFont();
		break;

	case LFUN_FILE_INSERT:
		MenuInsertLyXFile(ev.argument);
		break;

	case LFUN_FILE_INSERT_ASCII_PARA:
		InsertAsciiFile(bv_, ev.argument, true);
		break;

	case LFUN_FILE_INSERT_ASCII:
		InsertAsciiFile(bv_, ev.argument, false);
		break;

	case LFUN_LANGUAGE:
		lang(bv_, ev.argument);
		switchKeyMap();
		owner_->view_state_changed();
		break;

	case LFUN_EMPH:
		emph(bv_);
		owner_->view_state_changed();
		break;

	case LFUN_BOLD:
		bold(bv_);
		owner_->view_state_changed();
		break;

	case LFUN_NOUN:
		noun(bv_);
		owner_->view_state_changed();
		break;

	case LFUN_CODE:
		code(bv_);
		owner_->view_state_changed();
		break;

	case LFUN_SANS:
		sans(bv_);
		owner_->view_state_changed();
		break;

	case LFUN_ROMAN:
		roman(bv_);
		owner_->view_state_changed();
		break;

	case LFUN_DEFAULT:
		styleReset(bv_);
		owner_->view_state_changed();
		break;

	case LFUN_UNDERLINE:
		underline(bv_);
		owner_->view_state_changed();
		break;

	case LFUN_FONT_SIZE:
		fontSize(bv_, ev.argument);
		owner_->view_state_changed();
		break;

	case LFUN_FONT_STATE:
		owner_->getLyXFunc().setMessage(currentState(bv_));
		break;

	case LFUN_INSERT_LABEL:
		MenuInsertLabel(bv_, ev.argument);
		break;

	case LFUN_REF_INSERT:
		if (ev.argument.empty()) {
			InsetCommandParams p("ref");
			owner_->getDialogs().createRef(p.getAsString());
		} else {
			InsetCommandParams p;
			p.setFromString(ev.argument);

			InsetRef * inset = new InsetRef(p, *buffer_);
			if (!insertInset(inset))
				delete inset;
			else
				updateInset(inset, true);
		}
		break;

	case LFUN_BOOKMARK_SAVE:
		savePosition(strToUnsignedInt(ev.argument));
		break;

	case LFUN_BOOKMARK_GOTO:
		restorePosition(strToUnsignedInt(ev.argument));
		break;

	case LFUN_REF_GOTO:
	{
		string label = ev.argument;
		if (label.empty()) {
			InsetRef * inset =
				static_cast<InsetRef*>(getInsetByCode(Inset::REF_CODE));
			if (inset) {
				label = inset->getContents();
				savePosition(0);
			}
		}

		if (!label.empty()) {
			//bv_->savePosition(0);
			if (!bv_->gotoLabel(label))
				Alert::alert(_("Error"),
					   _("Couldn't find this label"),
					   _("in current document."));
		}
	}
	break;

	case LFUN_HTMLURL:
	case LFUN_URL:
	{
		InsetCommandParams p;
		if (ev.action == LFUN_HTMLURL)
			p.setCmdName("htmlurl");
		else
			p.setCmdName("url");
		owner_->getDialogs().createUrl(p.getAsString());
	}
	break;

	case LFUN_INSERT_URL:
	{
		InsetCommandParams p;
		p.setFromString(ev.argument);

		InsetUrl * inset = new InsetUrl(p);
		if (!insertInset(inset))
			delete inset;
		else
			updateInset(inset, true);
	}
	break;


	// --- accented characters ---------------------------

	case LFUN_UMLAUT:
	case LFUN_CIRCUMFLEX:
	case LFUN_GRAVE:
	case LFUN_ACUTE:
	case LFUN_TILDE:
	case LFUN_CEDILLA:
	case LFUN_MACRON:
	case LFUN_DOT:
	case LFUN_UNDERDOT:
	case LFUN_UNDERBAR:
	case LFUN_CARON:
	case LFUN_SPECIAL_CARON:
	case LFUN_BREVE:
	case LFUN_TIE:
	case LFUN_HUNG_UMLAUT:
	case LFUN_CIRCLE:
	case LFUN_OGONEK:
		if (ev.argument.empty()) {
			// As always...
			owner_->getLyXFunc().handleKeyFunc(ev.action);
		} else {
			owner_->getLyXFunc().handleKeyFunc(ev.action);
			owner_->getIntl().getTransManager()
				.TranslateAndInsert(ev.argument[0], bv_->getLyXText());
			update(bv_->getLyXText(),
			       BufferView::SELECT
			       | BufferView::FITCUR
			       | BufferView::CHANGE);
		}
		break;

	case LFUN_MATH_MACRO:
	case LFUN_MATH_DELIM:
	case LFUN_INSERT_MATRIX:
	case LFUN_INSERT_MATH:
	case LFUN_MATH_IMPORT_SELECTION: // Imports LaTeX from the X selection
	case LFUN_MATH_DISPLAY:          // Open or create a displayed math inset
	case LFUN_MATH_MODE:             // Open or create an inlined math inset
	case LFUN_GREEK:                 // Insert a single greek letter
		mathDispatch(FuncRequest(bv_, ev.action, ev.argument));
		break;

	case LFUN_CITATION_INSERT:
	{
		InsetCommandParams p;
		p.setFromString(ev.argument);

		InsetCitation * inset = new InsetCitation(p);
		if (!insertInset(inset))
			delete inset;
		else
			updateInset(inset, true);
	}
	break;

	case LFUN_INSERT_BIBTEX:
	{
		// ale970405+lasgoutt970425
		// The argument can be up to two tokens separated
		// by a space. The first one is the bibstyle.
		string const db = token(ev.argument, ' ', 0);
		string bibstyle = token(ev.argument, ' ', 1);
		if (bibstyle.empty())
			bibstyle = "plain";

		InsetCommandParams p("BibTeX", db, bibstyle);
		InsetBibtex * inset = new InsetBibtex(p);

		if (insertInset(inset)) {
			if (ev.argument.empty())
				inset->edit(bv_);
		} else
			delete inset;
	}
	break;

	// BibTeX data bases
	case LFUN_BIBDB_ADD:
	{
		InsetBibtex * inset =
			static_cast<InsetBibtex*>(getInsetByCode(Inset::BIBTEX_CODE));
		if (inset) {
			inset->addDatabase(ev.argument);
		}
	}
	break;

	case LFUN_BIBDB_DEL:
	{
		InsetBibtex * inset =
			static_cast<InsetBibtex*>(getInsetByCode(Inset::BIBTEX_CODE));
		if (inset)
			inset->delDatabase(ev.argument);
	}
	break;

	case LFUN_BIBTEX_STYLE:
	{
		InsetBibtex * inset =
			static_cast<InsetBibtex*>(getInsetByCode(Inset::BIBTEX_CODE));
		if (inset) 
			inset->setOptions(ev.argument);
	}
	break;

	case LFUN_CHILD_INSERT:
	{
		InsetInclude::Params p;
		p.cparams.setFromString(ev.argument);
		p.masterFilename_ = buffer_->fileName();

		InsetInclude * inset = new InsetInclude(p);
		if (!insertInset(inset))
			delete inset;
		else {
			updateInset(inset, true);
			bv_->owner()->getDialogs().showInclude(inset);
		}
	}
	break;

	case LFUN_FLOAT_LIST:
		if (tclass.floats().typeExist(ev.argument)) {
			Inset * inset = new InsetFloatList(ev.argument);
			if (!insertInset(inset, tclass.defaultLayoutName()))
				delete inset;
		} else {
			lyxerr << "Non-existent float type: "
			       << ev.argument << endl;
		}
		break;

	case LFUN_THESAURUS_ENTRY:
	{
		string arg = ev.argument;

		if (arg.empty()) {
			arg = bv_->getLyXText()->selectionAsString(buffer_,
								   false);

			// FIXME
			if (arg.size() > 100 || arg.empty()) {
				// Get word or selection
				bv_->getLyXText()->selectWordWhenUnderCursor(bv_, LyXText::WHOLE_WORD);
				arg = bv_->getLyXText()->selectionAsString(buffer_, false);
				// FIXME: where is getLyXText()->unselect(bv_) ?
			}
		}

		bv_->owner()->getDialogs().showThesaurus(arg);
	}
		break;

	case LFUN_UNKNOWN_ACTION:
		ev.errorMessage(N_("Unknown function!"));
		break;

	default:
		return bv_->getLyXText()->dispatch(FuncRequest(ev, bv_));
	} // end of switch

	return true;
}


bool BufferView::Pimpl::insertInset(Inset * inset, string const & lout)
{
	// if we are in a locking inset we should try to insert the
	// inset there otherwise this is a illegal function now
	if (bv_->theLockingInset()) {
		if (bv_->theLockingInset()->insetAllowed(inset))
		    return bv_->theLockingInset()->insertInset(bv_, inset);
		return false;
	}

	// not quite sure if we want this...
	setCursorParUndo(bv_);
	freezeUndo();

	beforeChange(bv_->text);
	if (!lout.empty()) {
		update(bv_->text, BufferView::SELECT|BufferView::FITCUR);
		bv_->text->breakParagraph(bv_);
		update(bv_->text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);

		if (!bv_->text->cursor.par()->empty()) {
			bv_->text->cursorLeft(bv_);

			bv_->text->breakParagraph(bv_);
			update(bv_->text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		}

		string lres = lout;
		LyXTextClass const & tclass =
			buffer_->params.getLyXTextClass();
		bool hasLayout = tclass.hasLayout(lres);
		string lay = tclass.defaultLayoutName();

		if (hasLayout != false) {
			// layout found
			lay = lres;
		} else {
			// layout not fount using default
			lay = tclass.defaultLayoutName();
		}

		bv_->text->setLayout(bv_, lay);

		bv_->text->setParagraph(bv_, 0, 0,
				   0, 0,
				   VSpace(VSpace::NONE), VSpace(VSpace::NONE),
				   Spacing(),
				   LYX_ALIGN_LAYOUT,
				   string(),
				   0);
		update(bv_->text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
	}

	bv_->text->insertInset(bv_, inset);
	update(bv_->text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);

	unFreezeUndo();
	return true;
}


void BufferView::Pimpl::updateInset(Inset * inset, bool mark_dirty)
{
	if (!inset || !available())
		return;

	// first check for locking insets
	if (bv_->theLockingInset()) {
		if (bv_->theLockingInset() == inset) {
			if (bv_->text->updateInset(bv_, inset)) {
				update();
				if (mark_dirty) {
					buffer_->markDirty();
				}
				updateScrollbar();
				return;
			}
		} else if (bv_->theLockingInset()->updateInsetInInset(bv_, inset)) {
			if (bv_->text->updateInset(bv_,  bv_->theLockingInset())) {
				update();
				if (mark_dirty) {
					buffer_->markDirty();
				}
				updateScrollbar();
				return;
			}
		}
	}

	// then check if the inset is a top_level inset (has no owner)
	// if yes do the update as always otherwise we have to update the
	// toplevel inset where this inset is inside
	Inset * tl_inset = inset;
	while (tl_inset->owner())
		tl_inset = tl_inset->owner();
	hideCursor();
	if (tl_inset == inset) {
		update(bv_->text, BufferView::UPDATE);
		if (bv_->text->updateInset(bv_, inset)) {
			if (mark_dirty) {
				update(bv_->text,
				       BufferView::SELECT
				       | BufferView::FITCUR
				       | BufferView::CHANGE);
			} else {
				update(bv_->text, SELECT);
			}
			return;
		}
	} else if (static_cast<UpdatableInset *>(tl_inset)
			   ->updateInsetInInset(bv_, inset))
	{
		if (bv_->text->updateInset(bv_, tl_inset)) {
			update();
			updateScrollbar();
		}
	}
}
