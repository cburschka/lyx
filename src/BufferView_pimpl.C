/**
 * \file BufferView_pimpl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Alfredo Braunstein
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 * \author Dekel Tsur
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BufferView_pimpl.h"
#include "buffer.h"
#include "buffer_funcs.h"
#include "bufferlist.h"
#include "bufferparams.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "factory.h"
#include "FloatList.h"
#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "intl.h"
#include "insetiterator.h"
#include "lyx_cb.h" // added for Dispatch functions
#include "lyx_main.h"
#include "lyxfind.h"
#include "lyxfunc.h"
#include "lyxtext.h"
#include "lyxrc.h"
#include "lastfiles.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "pariterator.h"
#include "rowpainter.h"
#include "undo.h"
#include "vspace.h"

#include "insets/insetref.h"
#include "insets/insettext.h"

#include "frontends/Alert.h"
#include "frontends/Dialogs.h"
#include "frontends/FileDialog.h"
#include "frontends/LyXView.h"
#include "frontends/LyXScreenFactory.h"
#include "frontends/screen.h"
#include "frontends/WorkArea.h"
#include "frontends/WorkAreaFactory.h"

#include "graphics/Previews.h"

#include "support/filetools.h"
#include "support/forkedcontr.h"
#include "support/globbing.h"
#include "support/path_defines.h"
#include "support/tostr.h"
#include "support/types.h"

#include <boost/bind.hpp>

using lyx::pos_type;

using lyx::support::AddPath;
using lyx::support::bformat;
using lyx::support::FileFilterList;
using lyx::support::FileSearch;
using lyx::support::ForkedcallsController;
using lyx::support::IsDirWriteable;
using lyx::support::MakeDisplayPath;
using lyx::support::strToUnsignedInt;
using lyx::support::system_lyxdir;

using std::endl;
using std::istringstream;
using std::make_pair;
using std::min;
using std::string;


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


BufferView::Pimpl::Pimpl(BufferView & bv, LyXView * owner,
			 int width, int height)
	: bv_(&bv), owner_(owner), buffer_(0), cursor_timeout(400),
	  using_xterm_cursor(false), cursor_(bv)
{
	xsel_cache_.set = false;

	workarea_.reset(WorkAreaFactory::create(*owner_, width, height));
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

	errorConnection_ =
		buf.error.connect(
			boost::bind(&BufferView::Pimpl::addError, this, _1));

	messageConnection_ =
		buf.message.connect(
			boost::bind(&LyXView::message, owner_, _1));

	busyConnection_ =
		buf.busy.connect(
			boost::bind(&LyXView::busy, owner_, _1));

	titleConnection_ =
		buf.updateTitles.connect(
			boost::bind(&LyXView::updateWindowTitle, owner_));

	timerConnection_ =
		buf.resetAutosaveTimers.connect(
			boost::bind(&LyXView::resetAutosaveTimer, owner_));

	readonlyConnection_ =
		buf.readonly.connect(
			boost::bind(&BufferView::Pimpl::showReadonly, this, _1));

	closingConnection_ =
		buf.closing.connect(
			boost::bind(&BufferView::Pimpl::setBuffer, this, (Buffer *)0));
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


void BufferView::Pimpl::newFile(string const & filename, string const & tname,
	bool isNamed)
{
	setBuffer(::newFile(filename, tname, isNamed));
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
			setBuffer(bufferlist.getBuffer(s));
			return true;
		}
		// FIXME: should be LFUN_REVERT
		if (!bufferlist.close(bufferlist.getBuffer(s), false))
			return false;
		// Fall through to new load. (Asger)
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

	setBuffer(b);
	bv_->showErrorList(_("Parse"));

	if (tolastfiles)
		LyX::ref().lastfiles().newFile(b->fileName());

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


void BufferView::Pimpl::top_y(int y)
{
	top_y_ = y;
}


int BufferView::Pimpl::top_y() const
{
	return top_y_;
}


void BufferView::Pimpl::setBuffer(Buffer * b)
{
	lyxerr[Debug::INFO] << "Setting buffer in BufferView ("
			    << b << ')' << endl;
	if (buffer_)
		disconnectBuffer();

	// set current buffer
	buffer_ = b;

	// reset old cursor
	top_y_ = 0;
	cursor_ = LCursor(*bv_);

	// if we're quitting lyx, don't bother updating stuff
	if (quitting)
		return;

	if (buffer_) {
		lyxerr[Debug::INFO] << "Buffer addr: " << buffer_ << endl;
		connectBuffer(*buffer_);

		cursor_.push(buffer_->inset());
		cursor_.resetAnchor();
		buffer_->text().init(bv_);
		buffer_->text().setCurrentFont(cursor_);

		// If we don't have a text object for this, we make one
		//if (bv_->text() == 0)
		//	resizeCurrentBuffer();

		// Buffer-dependent dialogs should be updated or
		// hidden. This should go here because some dialogs (eg ToC)
		// require bv_->text.
		owner_->getDialogs().updateBufferDependent(true);
	} else {
		lyxerr[Debug::INFO] << "  No Buffer!" << endl;
		// we are closing the buffer, use the first buffer as current
		buffer_ = bufferlist.first();
		owner_->getDialogs().hideBufferDependent();
	}

	update();
	updateScrollbar();
	owner_->updateMenubar();
	owner_->updateToolbars();
	owner_->updateLayoutChoice();
	owner_->updateWindowTitle();

	// This is done after the layout combox has been populated
	if (buffer_)
		owner_->setLayout(cursor_.paragraph().layout()->name());

	if (buffer_ && lyx::graphics::Previews::status() != LyXRC::PREVIEW_OFF)
		lyx::graphics::Previews::get().generateBufferPreviews(*buffer_);
}


bool BufferView::Pimpl::fitCursor()
{
	// to get the correct y cursor info
	lyxerr[Debug::DEBUG] << "BufferView::fitCursor" << std::endl;
	lyx::par_type const pit = bv_->cursor().bottom().par();
	bv_->text()->redoParagraph(pit);
	refreshPar(*bv_, *bv_->text(), pit);

	if (!screen().fitCursor(bv_))
		return false;
	updateScrollbar();
	return true;
}


void BufferView::Pimpl::redoCurrentBuffer()
{
	lyxerr[Debug::DEBUG] << "BufferView::redoCurrentBuffer" << endl;
	if (buffer_ && bv_->text()) {
		resizeCurrentBuffer();
		updateScrollbar();
		owner_->updateLayoutChoice();
	}
}


void BufferView::Pimpl::resizeCurrentBuffer()
{
	lyxerr[Debug::DEBUG] << "resizeCurrentBuffer" << endl;
	owner_->busy(true);
	owner_->message(_("Formatting document..."));

	LyXText * text = bv_->text();
	if (!text)
		return;

	text->init(bv_);
	update();
	fitCursor();

	switchKeyMap();
	owner_->busy(false);

	// reset the "Formatting..." message
	owner_->clearMessage();

	updateScrollbar();
}


void BufferView::Pimpl::updateScrollbar()
{
	if (!bv_->text()) {
		lyxerr[Debug::DEBUG] << "no text in updateScrollbar" << endl;
		workarea().setScrollbarParams(0, 0, 0);
		return;
	}

	LyXText const & t = *bv_->text();

	lyxerr[Debug::GUI]
		<< "Updating scrollbar: height: " << t.height()
		<< " top_y: " << top_y()
		<< " default height " << defaultRowHeight() << endl;

	workarea().setScrollbarParams(t.height(), top_y(), defaultRowHeight());
}


void BufferView::Pimpl::scrollDocView(int value)
{
	lyxerr[Debug::GUI] << "scrollDocView of " << value << endl;

	if (!buffer_)
		return;

	screen().hideCursor();

	top_y(value);
	screen().redraw(*bv_);

	if (!lyxrc.cursor_follows_scrollbar)
		return;

	int const height = defaultRowHeight();
	int const first = top_y() + height;
	int const last = top_y() + workarea().workHeight() - height;

	bv_->cursor().reset(bv_->buffer()->inset());
	LyXText * text = bv_->text();
	int y = text->cursorY(bv_->cursor().front());
	if (y < first)
		y = first;
	if (y > last)
		y = last;
	text->setCursorFromCoordinates(bv_->cursor(), 0, y);

	owner_->updateLayoutChoice();
}


void BufferView::Pimpl::scroll(int lines)
{
	if (!buffer_)
		return;

	LyXText const * t = bv_->text();
	int const line_height = defaultRowHeight();

	// The new absolute coordinate
	int new_top_y = top_y() + lines * line_height;

	// Restrict to a valid value
	new_top_y = std::min(t->height() - 4 * line_height, new_top_y);
	new_top_y = std::max(0, new_top_y);

	scrollDocView(new_top_y);

	// Update the scrollbar.
	workarea().setScrollbarParams(t->height(), top_y(), defaultRowHeight());
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

	LCursor & cur = bv_->cursor();

	if (!cur.selection()) {
		xsel_cache_.set = false;
		return;
	}

	if (!xsel_cache_.set ||
	    cur.back() != xsel_cache_.cursor ||
	    cur.anchor_.back() != xsel_cache_.anchor)
	{
		xsel_cache_.cursor = cur.back();
		xsel_cache_.anchor = cur.anchor_.back();
		xsel_cache_.set = cur.selection();
		sel = cur.selectionAsString(false);
		if (!sel.empty())
			workarea().putClipboard(sel);
	}
}


void BufferView::Pimpl::selectionLost()
{
	if (available()) {
		screen().hideCursor();
		bv_->cursor().clearSelection();
		xsel_cache_.set = false;
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

	if (buffer_ && widthChange) {
		// The visible LyXView need a resize
		resizeCurrentBuffer();
	}

	if (widthChange || heightChange)
		update();

	// always make sure that the scrollbar is sane.
	updateScrollbar();
	owner_->updateLayoutChoice();
}


void BufferView::Pimpl::update()
{
	//lyxerr << "BufferView::Pimpl::update(), buffer: " << buffer_ << endl;
	// fix cursor coordinate cache in case something went wrong

	// check needed to survive LyX startup
	if (buffer_) {
		// update macro store
		buffer_->buildMacros();

		// update all 'visible' paragraphs
		lyx::par_type beg, end;
		getParsInRange(buffer_->paragraphs(),
			       top_y(), top_y() + workarea().workHeight(),
			       beg, end);
		bv_->text()->redoParagraphs(beg, end);

		// and the scrollbar
		updateScrollbar();
	}
	screen().redraw(*bv_);
	bv_->owner()->view_state_changed();
}


// Callback for cursor timer
void BufferView::Pimpl::cursorToggle()
{
	if (buffer_) {
		screen().toggleCursor(*bv_);

		// Use this opportunity to deal with any child processes that
		// have finished but are waiting to communicate this fact
		// to the rest of LyX.
		ForkedcallsController & fcc = ForkedcallsController::get();
		if (fcc.processesCompleted())
			fcc.handleCompletedProcesses();
	}

	cursor_timeout.restart();
}


bool BufferView::Pimpl::available() const
{
	return buffer_ && bv_->text();
}


Change const BufferView::Pimpl::getCurrentChange()
{
	if (!bv_->buffer()->params().tracking_changes)
		return Change(Change::UNCHANGED);

	LyXText * text = bv_->getLyXText();
	LCursor & cur = bv_->cursor();

	if (!cur.selection())
		return Change(Change::UNCHANGED);

	return text->getPar(cur.selBegin().par()).
			lookupChangeFull(cur.selBegin().pos());
}


void BufferView::Pimpl::savePosition(unsigned int i)
{
	if (i >= saved_positions_num)
		return;
	BOOST_ASSERT(bv_->cursor().inTexted());
	saved_positions[i] = Position(buffer_->fileName(),
				      bv_->cursor().paragraph().id(),
				      bv_->cursor().pos());
	if (i > 0)
		owner_->message(bformat(_("Saved bookmark %1$s"), tostr(i)));
}


void BufferView::Pimpl::restorePosition(unsigned int i)
{
	if (i >= saved_positions_num)
		return;

	string const fname = saved_positions[i].filename;

	bv_->cursor().clearSelection();

	if (fname != buffer_->fileName()) {
		Buffer * b = 0;
		if (bufferlist.exists(fname))
			b = bufferlist.getBuffer(fname);
		else {
			b = bufferlist.newBuffer(fname);
			::loadLyXFile(b, fname); // don't ask, just load it
		}
		if (b)
			setBuffer(b);
	}

	ParIterator par = buffer_->getParFromID(saved_positions[i].par_id);
	if (par == buffer_->par_iterator_end())
		return;

	bv_->text()->setCursor(bv_->cursor(), par.pit(),
		min(par->size(), saved_positions[i].par_pos));

	if (i > 0)
		owner_->message(bformat(_("Moved to bookmark %1$s"), tostr(i)));
}


bool BufferView::Pimpl::isSavedPosition(unsigned int i)
{
	return i < saved_positions_num && !saved_positions[i].filename.empty();
}


void BufferView::Pimpl::switchKeyMap()
{
	if (!lyxrc.rtl_support)
		return;

	Intl & intl = owner_->getIntl();
	if (bv_->getLyXText()->real_current_font.isRightToLeft()) {
		if (intl.keymap == Intl::PRIMARY)
			intl.KeyMapSec();
	} else {
		if (intl.keymap == Intl::SECONDARY)
			intl.KeyMapPrim();
	}
}


void BufferView::Pimpl::center()
{
	LyXText * text = bv_->text();

	bv_->cursor().clearSelection();
	int const half_height = workarea().workHeight() / 2;
	int new_y = text->cursorY(bv_->cursor().front()) - half_height;
	if (new_y < 0)
		new_y = 0;

	// FIXME: look at this comment again ...
	// This updates top_y() but means the fitCursor() call
	// from the update(FITCUR) doesn't realise that we might
	// have moved (e.g. from GOTOPARAGRAPH), so doesn't cause
	// the scrollbar to be updated as it should, so we have
	// to do it manually. Any operation that does a center()
	// and also might have moved top_y() must make sure to call
	// updateScrollbar() currently. Never mind that this is a
	// pretty obfuscated way of updating text->top_y()
	top_y(new_y);
}


void BufferView::Pimpl::stuffClipboard(string const & stuff) const
{
	workarea().putClipboard(stuff);
}


InsetBase * BufferView::Pimpl::getInsetByCode(InsetBase::Code code)
{
#ifdef WITH_WARNINGS
#warning Does not work for mathed. (Still true?)
#endif
	// Ok, this is a little bit too brute force but it
	// should work for now. Better infrastructure is coming. (Lgb)
	LCursor & cur = bv_->cursor();
	DocIterator it = cur;
	for (it.forwardInset(); it; it.forwardInset()) {
		BOOST_ASSERT(it.nextInset());
		if (it.nextInset()->lyxCode() == code)
				return it.nextInset();
	}
	return 0;
}


void BufferView::Pimpl::MenuInsertLyXFile(string const & filenm)
{
	string filename = filenm;

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
				     FileFilterList(_("LyX Documents (*.lyx)")),
				     string());

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
	Buffer * buf = bv_->buffer();
	bool const tracking = buf->params().tracking_changes;

	if (!tracking) {
		ParIterator const end = buf->par_iterator_end();
		for (ParIterator it = buf->par_iterator_begin(); it != end; ++it)
			it->trackChanges();
		buf->params().tracking_changes = true;

		// we cannot allow undos beyond the freeze point
		buf->undostack().clear();
	} else {
		update();
		bv_->text()->setCursor(bv_->cursor(), 0, 0);
#ifdef WITH_WARNINGS
#warning changes FIXME
#endif
		bool found = lyx::find::findNextChange(bv_);
		if (found) {
			owner_->getDialogs().show("changes");
			return;
		}

		ParIterator const end = buf->par_iterator_end();
		for (ParIterator it = buf->par_iterator_begin(); it != end; ++it)
			it->untrackChanges();
		buf->params().tracking_changes = false;
	}

	buf->redostack().clear();
}


bool BufferView::Pimpl::workAreaDispatch(FuncRequest const & cmd0)
{
	//lyxerr << "BufferView::Pimpl::workAreaDispatch: request: "
	//  << cmd << std::endl;
	// this is only called for mouse related events including
	// LFUN_FILE_OPEN generated by drag-and-drop.
	FuncRequest cmd = cmd0;

	// handle drag&drop
	if (cmd.action == LFUN_FILE_OPEN) {
		owner_->dispatch(cmd);
		return true;
	}

	cmd.y += bv_->top_y();
	if (!bv_->buffer())
		return false;

	LCursor cur(*bv_);
	cur.push(bv_->buffer()->inset());
	cur.selection() = bv_->cursor().selection();

	// Doesn't go through lyxfunc, so we need to update
	// the layout choice etc. ourselves

	// e.g. Qt mouse press when no buffer
	if (!available())
		return false;

	screen().hideCursor();

	// Either the inset under the cursor or the
	// surrounding LyXText will handle this event.

	// Build temporary cursor.
	InsetBase * inset = bv_->text()->editXY(cur, cmd.x, cmd.y);
	lyxerr << "hit inset at tip: " << inset << endl;
	lyxerr << "created temp cursor:\n" << cur << endl;

	// Put anchor at the same position.
	cur.resetAnchor();

	// Try to dispatch to an non-editable inset near this position
	// via the temp cursor. If the inset wishes to change the real
	// cursor it has to do so explicitly by using
	//  cur.bv().cursor() = cur;  (or similar)'
	if (inset)
		inset->dispatch(cur, cmd);

	// Now dispatch to the temporary cursor. If the real cursor should
	// be modified, the inset's dispatch has to do so explicitly.
	if (!cur.result().dispatched())
		cur.dispatch(cmd);

	if (cur.result().dispatched()) {
		// Redraw if requested or necessary.
		if (fitCursor() || cur.result().update())
			update();
	}

	// see workAreaKeyPress
	cursor_timeout.restart();
	screen().showCursor(*bv_);

	// skip these when selecting
	if (cmd.action != LFUN_MOUSE_MOTION) {
		owner_->updateLayoutChoice();
		owner_->updateToolbars();
	}

	// slight hack: this is only called currently when we
	// clicked somewhere, so we force through the display
	// of the new status here.
	owner_->clearMessage();
	return true;
}


FuncStatus BufferView::Pimpl::getStatus(FuncRequest const & cmd)
{
	Buffer * buf = bv_->buffer();

	FuncStatus flag;

	switch (cmd.action) {

	case LFUN_UNDO:
		flag.enabled(!buf->undostack().empty());
		break;
	case LFUN_REDO:
		flag.enabled(!buf->redostack().empty());
		break;
	case LFUN_FILE_INSERT:
	case LFUN_FILE_INSERT_ASCII_PARA:
	case LFUN_FILE_INSERT_ASCII:
	case LFUN_FONT_STATE:
	case LFUN_INSERT_LABEL:
	case LFUN_BOOKMARK_SAVE:
	case LFUN_REF_GOTO:
	case LFUN_WORD_FIND:
	case LFUN_WORD_REPLACE:
	case LFUN_MARK_OFF:
	case LFUN_MARK_ON:
	case LFUN_SETMARK:
	case LFUN_CENTER:
	case LFUN_BEGINNINGBUF:
	case LFUN_ENDBUF:
	case LFUN_BEGINNINGBUFSEL:
	case LFUN_ENDBUFSEL:
		flag.enabled(true);
		break;
	case LFUN_BOOKMARK_GOTO:
		flag.enabled(bv_->isSavedPosition(strToUnsignedInt(cmd.argument)));
		break;
	case LFUN_TRACK_CHANGES:
		flag.enabled(true);
		flag.setOnOff(buf->params().tracking_changes);
		break;

	case LFUN_MERGE_CHANGES:
	case LFUN_ACCEPT_CHANGE: // what about these two
	case LFUN_REJECT_CHANGE: // what about these two
	case LFUN_ACCEPT_ALL_CHANGES:
	case LFUN_REJECT_ALL_CHANGES:
		flag.enabled(buf && buf->params().tracking_changes);
		break;
	default:
		flag.enabled(false);
	}

	return flag;
}



bool BufferView::Pimpl::dispatch(FuncRequest const & cmd)
{
	//lyxerr << "BufferView::Pimpl::dispatch  cmd: " << cmd << std::endl;
	// Make sure that the cached BufferView is correct.
	lyxerr[Debug::ACTION] << "BufferView::Pimpl::Dispatch:"
		<< " action[" << cmd.action << ']'
		<< " arg[" << cmd.argument << ']'
		<< " x[" << cmd.x << ']'
		<< " y[" << cmd.y << ']'
		<< " button[" << cmd.button() << ']'
		<< endl;

	LCursor & cur = bv_->cursor();

	switch (cmd.action) {

	case LFUN_UNDO:
		if (available()) {
			cur.message(_("Undo"));
			cur.clearSelection();
			if (!textUndo(*bv_))
				cur.message(_("No further undo information"));
			update();
			switchKeyMap();
		}
		break;

	case LFUN_REDO:
		if (available()) {
			cur.message(_("Redo"));
			cur.clearSelection();
			if (!textRedo(*bv_))
				cur.message(_("No further redo information"));
			update();
			switchKeyMap();
		}
		break;

	case LFUN_FILE_INSERT:
		MenuInsertLyXFile(cmd.argument);
		break;

	case LFUN_FILE_INSERT_ASCII_PARA:
		InsertAsciiFile(bv_, cmd.argument, true);
		break;

	case LFUN_FILE_INSERT_ASCII:
		InsertAsciiFile(bv_, cmd.argument, false);
		break;

	case LFUN_FONT_STATE:
		cur.message(cur.currentState());
		break;

	case LFUN_BOOKMARK_SAVE:
		savePosition(strToUnsignedInt(cmd.argument));
		break;

	case LFUN_BOOKMARK_GOTO:
		restorePosition(strToUnsignedInt(cmd.argument));
		break;

	case LFUN_REF_GOTO: {
		string label = cmd.argument;
		if (label.empty()) {
			InsetRef * inset =
				static_cast<InsetRef*>(getInsetByCode(InsetBase::REF_CODE));
			if (inset) {
				label = inset->getContents();
				savePosition(0);
			}
		}

		if (!label.empty())
			bv_->gotoLabel(label);
		break;
	}

	case LFUN_TRACK_CHANGES:
		trackChanges();
		break;

	case LFUN_MERGE_CHANGES:
		owner_->getDialogs().show("changes");
		break;

	case LFUN_ACCEPT_ALL_CHANGES: {
		bv_->cursor().reset(bv_->buffer()->inset());
#ifdef WITH_WARNINGS
#warning FIXME changes
#endif
		while (lyx::find::findNextChange(bv_))
			bv_->getLyXText()->acceptChange(bv_->cursor());
		update();
		break;
	}

	case LFUN_REJECT_ALL_CHANGES: {
		bv_->cursor().reset(bv_->buffer()->inset());
#ifdef WITH_WARNINGS
#warning FIXME changes
#endif
		while (lyx::find::findNextChange(bv_))
			bv_->getLyXText()->rejectChange(bv_->cursor());
		break;
	}

	case LFUN_WORD_FIND:
		lyx::find::find(bv_, cmd);
		break;

	case LFUN_WORD_REPLACE:
		lyx::find::replace(bv_, cmd);
		break;

	case LFUN_MARK_OFF:
		cur.clearSelection();
		cur.resetAnchor();
		cur.message(N_("Mark off"));
		break;

	case LFUN_MARK_ON:
		cur.clearSelection();
		cur.mark() = true;
		cur.resetAnchor();
		cur.message(N_("Mark on"));
		break;

	case LFUN_SETMARK:
		cur.clearSelection();
		if (cur.mark()) {
			cur.mark() = false;
			cur.message(N_("Mark removed"));
		} else {
			cur.mark() = true;
			cur.message(N_("Mark set"));
		}
		cur.resetAnchor();
		break;

	case LFUN_CENTER:
		bv_->center();
		break;

	case LFUN_BEGINNINGBUFSEL:
		bv_->cursor().reset(bv_->buffer()->inset());
		if (!cur.selection())
			cur.resetAnchor();
		bv_->text()->cursorTop(cur);
		finishUndo();
		break;

	case LFUN_ENDBUFSEL:
		bv_->cursor().reset(bv_->buffer()->inset());
		if (!cur.selection())
			cur.resetAnchor();
		bv_->text()->cursorBottom(cur);
		finishUndo();
		break;

	default:
		return false;
	}

	return true;
}
