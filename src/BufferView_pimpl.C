/**
 * \file BufferView_pimpl.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars Gullik Bjønnes
 * \author various
 */

#include <config.h>

#include "BufferView_pimpl.h"
#include "bufferlist.h"
#include "buffer.h"
#include "buffer_funcs.h"
#include "bufferview_funcs.h"
#include "lfuns.h"
#include "debug.h"
#include "factory.h"
#include "FloatList.h"
#include "funcrequest.h"
#include "gettext.h"
#include "intl.h"
#include "iterators.h"
#include "Lsstream.h"
#include "lyx_cb.h" // added for Dispatch functions
#include "lyx_main.h"
#include "lyxfind.h"
#include "lyxfunc.h"
#include "lyxtext.h"
#include "lyxrc.h"
#include "lastfiles.h"
#include "paragraph.h"
#include "ParagraphParameters.h"
#include "TextCache.h"
#include "undo_funcs.h"

#include "insets/insetfloatlist.h"
#include "insets/insetgraphics.h"
#include "insets/insetinclude.h"
#include "insets/insetref.h"
#include "insets/insettext.h"

#include "frontends/Alert.h"
#include "frontends/Dialogs.h"
#include "frontends/FileDialog.h"
#include "frontends/LyXView.h"
#include "frontends/LyXScreenFactory.h"
#include "frontends/mouse_state.h"
#include "frontends/screen.h"
#include "frontends/WorkArea.h"
#include "frontends/WorkAreaFactory.h"

#include "mathed/formulabase.h"

#include "graphics/Previews.h"

#include "support/LAssert.h"
#include "support/tostr.h"
#include "support/filetools.h"
#include "support/path_defines.h"

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
using namespace lyx::support;
using namespace bv_funcs;

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
		.connect(boost::bind(&BufferView::Pimpl::workAreaDispatch, this, _1));
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


void BufferView::Pimpl::addError(ErrorItem const & ei)
{
	errorlist_.push_back(ei);
}


void BufferView::Pimpl::showReadonly(bool)
{
	owner_->updateWindowTitle();
	owner_->getDialogs().updateBufferDependent(false);
}


void BufferView::Pimpl::connectBuffer(Buffer & buf)
{
	if (errorConnection_.connected())
		disconnectBuffer();

	errorConnection_ = buf.error.connect(boost::bind(&BufferView::Pimpl::addError, this, _1));
	messageConnection_ = buf.message.connect(boost::bind(&LyXView::message, owner_, _1));
	busyConnection_ = buf.busy.connect(boost::bind(&LyXView::busy, owner_, _1));
	titleConnection_ = buf.updateTitles.connect(boost::bind(&LyXView::updateWindowTitle, owner_));
	timerConnection_ = buf.resetAutosaveTimers.connect(boost::bind(&LyXView::resetAutosaveTimer, owner_));
	readonlyConnection_ = buf.readonly.connect(boost::bind(&BufferView::Pimpl::showReadonly, this, _1));
	closingConnection_ = buf.closing.connect(boost::bind(&BufferView::Pimpl::buffer, this, (Buffer *)0));
}


void BufferView::Pimpl::disconnectBuffer()
{
	errorConnection_.disconnect();
	messageConnection_.disconnect();
	busyConnection_.disconnect();
	titleConnection_.disconnect();
	timerConnection_.disconnect();
	readonlyConnection_.disconnect();
	closingConnection_.disconnect();
}


bool BufferView::Pimpl::newFile(string const & filename,
				string const & tname,
				bool isNamed)
{
	Buffer * b = ::newFile(filename, tname, isNamed);
	buffer(b);
	return true;
}


bool BufferView::Pimpl::loadLyXFile(string const & filename, bool tolastfiles)
{
	// get absolute path of file and add ".lyx" to the filename if
	// necessary
	string s = FileSearch(string(), filename, "lyx");

	bool const found = !s.empty();

	if (!found)
		s = filename;

	// file already open?
	if (bufferlist.exists(s)) {
		string const file = MakeDisplayPath(s, 20);
		string text = bformat(_("The document %1$s is already "
					"loaded.\n\nDo you want to revert "
					"to the saved version?"), file);
		int const ret = Alert::prompt(_("Revert to saved document?"),
			text, 0, 1,  _("&Revert"), _("&Switch to document"));

		if (ret != 0) {
			buffer(bufferlist.getBuffer(s));
			return true;
		} else {
			// FIXME: should be LFUN_REVERT
			if (!bufferlist.close(bufferlist.getBuffer(s), false))
				return false;
			// Fall through to new load. (Asger)
		}
	}

	Buffer * b;

	if (found) {
		b = bufferlist.newBuffer(s);
		connectBuffer(*b);
		if (!::loadLyXFile(b, s)) {
			bufferlist.release(b);
			return false;
		}
	} else {
		string text = bformat(_("The document %1$s does not yet "
					"exist.\n\nDo you want to create "
					"a new document?"), s);
		int const ret = Alert::prompt(_("Create new document?"),
			 text, 0, 1, _("&Create"), _("Cancel"));

		if (ret == 0)
			b = ::newFile(s, string(), true);
		else
			return false;
	}

	buffer(b);
	bv_->showErrorList(_("Parse"));

	if (tolastfiles)
		lastfiles->newFile(b->fileName());

	return true;
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
			    << b << ')' << endl;
	if (buffer_) {
		disconnectBuffer();
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

	// if we're quitting lyx, don't bother updating stuff
	if (quitting)
		return;

	// if we are closing the buffer, use the first buffer as current
	if (!buffer_)
		buffer_ = bufferlist.first();

	if (buffer_) {
		lyxerr[Debug::INFO] << "Buffer addr: " << buffer_ << endl;
		connectBuffer(*buffer_);

		// If we don't have a text object for this, we make one
		if (bv_->text == 0)
			resizeCurrentBuffer();

		// FIXME: needed when ?
		bv_->text->top_y(screen().topCursorVisible(bv_->text));

		// Buffer-dependent dialogs should be updated or
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

	update();
	updateScrollbar();
	owner_->updateMenubar();
	owner_->updateToolbar();
	owner_->updateLayoutChoice();
	owner_->updateWindowTitle();

	if (buffer_) {
		// Don't forget to update the Layout
		string const layoutname =
			bv_->text->cursor.par()->layout()->name();
		owner_->setLayout(layoutname);
	}

	if (lyx::graphics::Previews::activated() && buffer_)
		lyx::graphics::Previews::get().generateBufferPreviews(*buffer_);
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

	//dispatch(FuncRequest(LFUN_PARAGRAPH_UPDATE));

	// We need to always update, in case we did a
	// paste and we stayed anchored to a row, but
	// the actual height of the doc changed ...
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
		update();
	}
}


void BufferView::Pimpl::resizeCurrentBuffer()
{
	lyxerr[Debug::INFO] << "resizeCurrentBuffer" << endl;

	ParagraphList::iterator par;
	ParagraphList::iterator selstartpar;
	ParagraphList::iterator selendpar;
	UpdatableInset * the_locking_inset = 0;

	pos_type pos = 0;
	pos_type selstartpos = 0;
	pos_type selendpos = 0;
	bool selection = false;
	bool mark_set  = false;

	owner_->busy(true);

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
		bv_->text->fullRebreak();
		update();
	} else {
		lyxerr << "text not available!" << endl;
		// See if we have a text in TextCache that fits
		// the new buffer_ with the correct width.
		bv_->text = textcache.findFit(buffer_, workarea().workWidth());
		if (bv_->text) {
			lyxerr << "text in cache!" << endl;
			if (lyxerr.debugging()) {
				lyxerr << "Found a LyXText that fits:" << endl;
				textcache.show(lyxerr, make_pair(buffer_, make_pair(workarea().workWidth(), bv_->text)));
			}
			// Set the owner of the newly found text
			//	bv_->text->owner(bv_);
			if (lyxerr.debugging())
				textcache.show(lyxerr, "resizeCurrentBuffer");
		} else {
			lyxerr << "no text in cache!" << endl;
			bv_->text = new LyXText(bv_);
			bv_->text->init(bv_);
		}

		par = bv_->text->ownerParagraphs().end();
		selstartpar = bv_->text->ownerParagraphs().end();
		selendpar = bv_->text->ownerParagraphs().end();
	}

	if (par != bv_->text->ownerParagraphs().end()) {
		bv_->text->selection.set(true);
		// At this point just to avoid the Delete-Empty-Paragraph-
		// Mechanism when setting the cursor.
		bv_->text->selection.mark(mark_set);
		if (selection) {
			bv_->text->setCursor(selstartpar, selstartpos);
			bv_->text->selection.cursor = bv_->text->cursor;
			bv_->text->setCursor(selendpar, selendpos);
			bv_->text->setSelection();
			bv_->text->setCursor(par, pos);
		} else {
			bv_->text->setCursor(par, pos);
			bv_->text->selection.cursor = bv_->text->cursor;
			bv_->text->selection.set(false);
		}
		// remake the inset locking
		bv_->theLockingInset(the_locking_inset);
	}

	bv_->text->top_y(screen().topCursorVisible(bv_->text));

	switchKeyMap();
	owner_->busy(false);

	// reset the "Formatting..." message
	owner_->clearMessage();

	updateScrollbar();
}


void BufferView::Pimpl::updateScrollbar()
{
	if (!bv_->text) {
		lyxerr[Debug::GUI] << "no text in updateScrollbar" << endl;
		workarea().setScrollbarParams(0, 0, 0);
		return;
	}

	LyXText const & t = *bv_->text;

	lyxerr[Debug::GUI] << "Updating scrollbar: h " << t.height << ", top_y() "
		<< t.top_y() << ", default height " << defaultRowHeight() << endl;

	workarea().setScrollbarParams(t.height, t.top_y(), defaultRowHeight());
}


void BufferView::Pimpl::scrollDocView(int value)
{
	lyxerr[Debug::GUI] << "scrollDocView of " << value << endl;

	if (!buffer_)
		return;

	screen().hideCursor();

	screen().draw(bv_->text, bv_, value);

	if (!lyxrc.cursor_follows_scrollbar)
		return;

	LyXText * vbt = bv_->text;

	int const height = defaultRowHeight();
	int const first = static_cast<int>((bv_->text->top_y() + height));
	int const last = static_cast<int>((bv_->text->top_y() + workarea().workHeight() - height));

	if (vbt->cursor.y() < first)
		vbt->setCursorFromCoordinates(0, first);
	else if (vbt->cursor.y() > last)
		vbt->setCursorFromCoordinates(0, last);

	owner_->updateLayoutChoice();
}


void BufferView::Pimpl::scroll(int lines)
{
	if (!buffer_) {
		return;
	}

	LyXText const * t = bv_->text;
	int const line_height = defaultRowHeight();

	// The new absolute coordinate
	int new_top_y = t->top_y() + lines * line_height;

	// Restrict to a valid value
	new_top_y = std::min(t->height - 4 * line_height, new_top_y);
	new_top_y = std::max(0, new_top_y);

	scrollDocView(new_top_y);

	// Update the scrollbar.
	workarea().setScrollbarParams(t->height, t->top_y(), defaultRowHeight());
}


void BufferView::Pimpl::workAreaKeyPress(LyXKeySymPtr key,
					 key_modifier::state state)
{
	bv_->owner()->getLyXFunc().processKeySym(key, state);

	/* This is perhaps a bit of a hack. When we move
	 * around, or type, it's nice to be able to see
	 * the cursor immediately after the keypress. So
	 * we reset the toggle timeout and force the visibility
	 * of the cursor. Note we cannot do this inside
	 * dispatch() itself, because that's called recursively.
	 */
	if (available()) {
		cursor_timeout.restart();
		screen().showCursor(*bv_);
	}
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
		screen().hideCursor();
		toggleSelection();
		bv_->getLyXText()->clearSelection();
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
		}
	}

	if (widthChange || heightChange)
		update();

	// always make sure that the scrollbar is sane.
	updateScrollbar();
	owner_->updateLayoutChoice();
}


void BufferView::Pimpl::update()
{
	lyxerr << "BufferView::update()" << endl;
	// fix cursor coordinate cache in case something went wrong
	if (bv_->getLyXText()) {
		// check needed to survive LyX startup
		bv_->getLyXText()->redoCursor();
		fitCursor();
	}
	screen().redraw(*bv_);
}


// Callback for cursor timer
void BufferView::Pimpl::cursorToggle()
{
	if (!buffer_) {
		cursor_timeout.restart();
		return;
	}

	screen().toggleCursor(*bv_);

	cursor_timeout.restart();
}


bool BufferView::Pimpl::available() const
{
	if (buffer_ && bv_->text)
		return true;
	return false;
}


Change const BufferView::Pimpl::getCurrentChange()
{
	if (!bv_->buffer()->params.tracking_changes)
		return Change(Change::UNCHANGED);

	LyXText * t(bv_->getLyXText());

	if (!t->selection.set())
		return Change(Change::UNCHANGED);

	LyXCursor const & cur(t->selection.start);
	return cur.par()->lookupChangeFull(cur.pos());
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
	if (i > 0)
		owner_->message(bformat(_("Saved bookmark %1$s"), tostr(i)));
}


void BufferView::Pimpl::restorePosition(unsigned int i)
{
	if (i >= saved_positions_num)
		return;

	string const fname = saved_positions[i].filename;

	beforeChange(bv_->text);

	if (fname != buffer_->fileName()) {
		Buffer * b;
		if (bufferlist.exists(fname))
			b = bufferlist.getBuffer(fname);
		else {
			b = bufferlist.newBuffer(fname);
			::loadLyXFile(b, fname); // don't ask, just load it
		}
		if (b != 0)
			buffer(b);
	}

	ParIterator par = buffer_->getParFromID(saved_positions[i].par_id);
	if (par == buffer_->par_iterator_end())
		return;

	bv_->text->setCursor(par.pit(),
			     min(par->size(), saved_positions[i].par_pos));

	update();
	if (i > 0)
		owner_->message(bformat(_("Moved to bookmark %1$s"), tostr(i)));
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
		 && bv_->theLockingInset()->lyxCode() == InsetOld::ERT_CODE))
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


void BufferView::Pimpl::toggleSelection(bool b)
{
	screen().toggleSelection(bv_->text, bv_, b);
}


void BufferView::Pimpl::center()
{
	LyXText * t = bv_->text;

	beforeChange(t);
	int const half_height = workarea().workHeight() / 2;
	int new_y = 0;

	if (t->cursor.y() > half_height)
		new_y = t->cursor.y() - half_height;

	// FIXME: look at this comment again ...

	// FIXME: can we do this w/o calling screen directly ?
	// This updates top_y() but means the fitCursor() call
	// from the update(FITCUR) doesn't realise that we might
	// have moved (e.g. from GOTOPARAGRAPH), so doesn't cause
	// the scrollbar to be updated as it should, so we have
	// to do it manually. Any operation that does a center()
	// and also might have moved top_y() must make sure to call
	// updateScrollbar() currently. Never mind that this is a
	// pretty obfuscated way of updating t->top_y()
	screen().draw(t, bv_, new_y);

	update();
}


void BufferView::Pimpl::stuffClipboard(string const & stuff) const
{
	workarea().putClipboard(stuff);
}


/*
 * Dispatch functions for actions which can be valid for BufferView->text
 * and/or InsetText->text!!!
 */


InsetOld * BufferView::Pimpl::getInsetByCode(InsetOld::Code code)
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

		FileDialog fileDlg(_("Select LyX document to insert"),
			LFUN_FILE_INSERT,
			make_pair(string(_("Documents|#o#O")),
				  string(lyxrc.document_path)),
			make_pair(string(_("Examples|#E#e")),
				  string(AddPath(system_lyxdir(), "examples"))));

		FileDialog::Result result =
			fileDlg.open(initpath,
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

	string const disp_fn = MakeDisplayPath(filename);
	owner_->message(bformat(_("Inserting document %1$s..."), disp_fn));
	if (bv_->insertLyXFile(filename))
		owner_->message(bformat(_("Document %1$s inserted."),
					disp_fn));
	else
		owner_->message(bformat(_("Could not insert document %1$s"),
					disp_fn));
}


void BufferView::Pimpl::trackChanges()
{
	Buffer * buf(bv_->buffer());
	bool const tracking(buf->params.tracking_changes);

	if (!tracking) {
		ParIterator const end = buf->par_iterator_end();
		for (ParIterator it = buf->par_iterator_begin(); it != end; ++it)
			it->trackChanges();
		buf->params.tracking_changes = true;

		// we cannot allow undos beyond the freeze point
		buf->undostack.clear();
	} else {
		update();
		bv_->text->setCursor(buf->paragraphs.begin(), 0);
#warning changes FIXME
		//moveCursorUpdate(false);

		bool found = lyx::find::findNextChange(bv_);
		if (found) {
			owner_->getDialogs().show("changes");
			return;
		}

		ParIterator const end = buf->par_iterator_end();
		for (ParIterator it = buf->par_iterator_begin(); it != end; ++it)
			it->untrackChanges();
		buf->params.tracking_changes = false;
	}

	buf->redostack.clear();
}


bool BufferView::Pimpl::workAreaDispatch(FuncRequest const & ev)
{
	switch (ev.action) {
	case LFUN_MOUSE_PRESS:
	case LFUN_MOUSE_MOTION:
	case LFUN_MOUSE_RELEASE:
	case LFUN_MOUSE_DOUBLE:
	case LFUN_MOUSE_TRIPLE:
	{
		// We pass those directly to the Bufferview, since
		// otherwise selection handling breaks down

		// Doesn't go through lyxfunc, so we need to update
		// the layout choice etc. ourselves

		// e.g. Qt mouse press when no buffer
		if (!available())
			return false;

		screen().hideCursor();

		bool const res = dispatch(ev);
		
		// see workAreaKeyPress
		cursor_timeout.restart();
		screen().showCursor(*bv_);

		// FIXME: we should skip these when selecting
		owner_->updateLayoutChoice();
		owner_->updateToolbar();
		fitCursor();

		// slight hack: this is only called currently when we
		// clicked somewhere, so we force through the display
		// of the new status here.
		owner_->clearMessage();

		return res;
	}
	default:
		owner_->dispatch(ev);
		return true;
	}
}


bool BufferView::Pimpl::dispatch(FuncRequest const & ev_in)
{
	// Make sure that the cached BufferView is correct.
	FuncRequest ev = ev_in;
	ev.setView(bv_);

	lyxerr[Debug::ACTION] << "BufferView::Pimpl::Dispatch:"
		<< " action[" << ev.action << ']'
		<< " arg[" << ev.argument << ']'
		<< " x[" << ev.x << ']'
		<< " y[" << ev.y << ']'
		<< " button[" << ev.button() << ']'
		<< endl;

	LyXTextClass const & tclass = buffer_->params.getLyXTextClass();

	switch (ev.action) {

	case LFUN_SCROLL_INSET:
		// this is not handled here as this function is only active
		// if we have a locking_inset and that one is (or contains)
		// a tabular-inset
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

	case LFUN_INSERT_LABEL: {
		// Try and generate a valid label
		string const contents = ev.argument.empty() ?
			getPossibleLabel(*bv_) : ev.argument;
		InsetCommandParams icp("label", contents);
		string data = InsetCommandMailer::params2string("label", icp);
		owner_->getDialogs().show("label", data, 0);
	}
	break;

	case LFUN_BOOKMARK_SAVE:
		savePosition(strToUnsignedInt(ev.argument));
		break;

	case LFUN_BOOKMARK_GOTO:
		restorePosition(strToUnsignedInt(ev.argument));
		break;

	case LFUN_REF_GOTO: {
		string label = ev.argument;
		if (label.empty()) {
			InsetRef * inset =
				static_cast<InsetRef*>(getInsetByCode(InsetOld::REF_CODE));
			if (inset) {
				label = inset->getContents();
				savePosition(0);
			}
		}

		if (!label.empty())
			bv_->gotoLabel(label);
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
			update();
		}
		break;

	case LFUN_MATH_MACRO:
	case LFUN_MATH_DELIM:
	case LFUN_INSERT_MATRIX:
	case LFUN_INSERT_MATH:
	case LFUN_MATH_IMPORT_SELECTION: // Imports LaTeX from the X selection
	case LFUN_MATH_DISPLAY:          // Open or create a displayed math inset
	case LFUN_MATH_MODE:             // Open or create an inlined math inset
		mathDispatch(ev);
		break;

	case LFUN_INSET_APPLY: {
		string const name = ev.getArg(0);

		InsetBase * inset = owner_->getDialogs().getOpenInset(name);
		if (inset) {
			// This works both for 'original' and 'mathed' insets.
			// Note that the localDispatch performs updateInset
			// also.
			FuncRequest fr(bv_, LFUN_INSET_MODIFY, ev.argument);
			inset->localDispatch(fr);
		} else {
			FuncRequest fr(bv_, LFUN_INSET_INSERT, ev.argument);
			dispatch(fr);
		}
	}
	break;

	case LFUN_INSET_INSERT: {
		InsetOld * inset = createInset(ev);
		if (inset && insertInset(inset)) {
			updateInset();

			string const name = ev.getArg(0);
			if (name == "bibitem") {
				// We need to do a redraw because the maximum
				// InsetBibitem width could have changed
				bv_->fitCursor();
#warning check whether the update() is needed at all
				bv_->update();
			}
		} else {
			delete inset;
		}
	}
	break;

	case LFUN_FLOAT_LIST:
		if (tclass.floats().typeExist(ev.argument)) {
			InsetOld * inset = new InsetFloatList(ev.argument);
			if (!insertInset(inset, tclass.defaultLayoutName()))
				delete inset;
		} else {
			lyxerr << "Non-existent float type: "
			       << ev.argument << endl;
		}
		break;

	case LFUN_LAYOUT_PARAGRAPH: {
		Paragraph const * par = &*bv_->getLyXText()->cursor.par();
		if (!par)
			break;

		string data;
		params2string(*par, data);

		data = "show\n" + data;
		bv_->owner()->getDialogs().show("paragraph", data);
		break;
	}

	case LFUN_PARAGRAPH_UPDATE: {
		if (!bv_->owner()->getDialogs().visible("paragraph"))
			break;
		Paragraph const & par = *bv_->getLyXText()->cursor.par();

		string data;
		params2string(par, data);

		// Will the paragraph accept changes from the dialog?
		InsetOld * const inset = par.inInset();
		bool const accept =
			!(inset && inset->forceDefaultParagraphs(inset));

		data = "update " + tostr(accept) + '\n' + data;
		bv_->owner()->getDialogs().update("paragraph", data);
		break;
	}

	case LFUN_PARAGRAPH_APPLY:
		setParagraphParams(*bv_, ev.argument);
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
				bv_->getLyXText()->selectWordWhenUnderCursor(lyx::WHOLE_WORD);
				arg = bv_->getLyXText()->selectionAsString(buffer_, false);
				// FIXME: where is getLyXText()->unselect(bv_) ?
			}
		}

		bv_->owner()->getDialogs().show("thesaurus", arg);
	}
		break;

	case LFUN_TRACK_CHANGES:
		trackChanges();
		break;

	case LFUN_MERGE_CHANGES:
		owner_->getDialogs().show("changes");
		break;

	case LFUN_ACCEPT_ALL_CHANGES: {
		bv_->text->setCursor(bv_->buffer()->paragraphs.begin(), 0);
#warning FIXME changes
		//moveCursorUpdate(false);

		while (lyx::find::findNextChange(bv_))
			bv_->getLyXText()->acceptChange();

		update();
		break;
	}

	case LFUN_REJECT_ALL_CHANGES: {
		bv_->text->setCursor(bv_->buffer()->paragraphs.begin(), 0);
#warning FIXME changes
		//moveCursorUpdate(false);

		while (lyx::find::findNextChange(bv_))
			bv_->getLyXText()->rejectChange();

		update();
		break;
	}

	case LFUN_ACCEPT_CHANGE: {
		bv_->getLyXText()->acceptChange();
		update();
		break;
	}

	case LFUN_REJECT_CHANGE: {
		bv_->getLyXText()->rejectChange();
		update();
		break;
	}

	case LFUN_UNKNOWN_ACTION:
		ev.errorMessage(N_("Unknown function!"));
		break;

	default:
		return bv_->getLyXText()->dispatch(FuncRequest(ev, bv_));
	} // end of switch

	return true;
}


bool BufferView::Pimpl::insertInset(InsetOld * inset, string const & lout)
{
	// if we are in a locking inset we should try to insert the
	// inset there otherwise this is a illegal function now
	if (bv_->theLockingInset()) {
		if (bv_->theLockingInset()->insetAllowed(inset))
			return bv_->theLockingInset()->insertInset(bv_, inset);
		return false;
	}

	// not quite sure if we want this...
	recordUndo(bv_, Undo::ATOMIC);
	freezeUndo();

	beforeChange(bv_->text);
	if (!lout.empty()) {
		bv_->text->breakParagraph(bv_->buffer()->paragraphs);

		if (!bv_->text->cursor.par()->empty()) {
			bv_->text->cursorLeft(bv_);
			bv_->text->breakParagraph(bv_->buffer()->paragraphs);
		}

		string lres = lout;
		LyXTextClass const & tclass = buffer_->params.getLyXTextClass();
		bool hasLayout = tclass.hasLayout(lres);
		string lay = tclass.defaultLayoutName();

		if (hasLayout != false) {
			// layout found
			lay = lres;
		} else {
			// layout not fount using default
			lay = tclass.defaultLayoutName();
		}

		bv_->text->setLayout(lay);

		bv_->text->setParagraph(0, 0,
				   0, 0,
				   VSpace(VSpace::NONE), VSpace(VSpace::NONE),
				   Spacing(),
				   LYX_ALIGN_LAYOUT,
				   string(),
				   0);
	}

	bv_->text->insertInset(inset);
	update();

	unFreezeUndo();
	return true;
}


void BufferView::Pimpl::updateInset()
{
	if (!available())
		return;

	// this should not be needed, but it is...
	bv_->text->redoParagraph(bv_->text->cursor.par());

	update();
	updateScrollbar();
}
