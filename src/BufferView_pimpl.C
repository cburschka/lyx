/**
 * \file BufferView_pimpl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Alfredo Braustein
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
#include "gettext.h"
#include "intl.h"
#include "iterators.h"
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
#include "undo.h"
#include "vspace.h"

#include "insets/insetfloatlist.h"
#include "insets/insetref.h"

#include "frontends/Alert.h"
#include "frontends/Dialogs.h"
#include "frontends/FileDialog.h"
#include "frontends/LyXView.h"
#include "frontends/LyXScreenFactory.h"
#include "frontends/screen.h"
#include "frontends/WorkArea.h"
#include "frontends/WorkAreaFactory.h"

#include "graphics/Previews.h"

#include "mathed/formulabase.h"

#include "support/filetools.h"
#include "support/path_defines.h"
#include "support/tostr.h"

#include <boost/bind.hpp>

using bv_funcs::currentState;

using lyx::pos_type;

using lyx::support::AddPath;
using lyx::support::bformat;
using lyx::support::FileSearch;
using lyx::support::IsDirWriteable;
using lyx::support::MakeDisplayPath;
using lyx::support::strToUnsignedInt;
using lyx::support::system_lyxdir;

using std::endl;
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


BufferView::Pimpl::Pimpl(BufferView * bv, LyXView * owner,
	     int xpos, int ypos, int width, int height)
	: bv_(bv), owner_(owner), buffer_(0), cursor_timeout(400),
	  using_xterm_cursor(false), cursor_(bv)
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


void BufferView::Pimpl::buffer(Buffer * b)
{
	lyxerr[Debug::INFO] << "Setting buffer in BufferView ("
			    << b << ')' << endl;
	if (buffer_) {
		disconnectBuffer();
		//delete bv_->text();
		//bv_->setText(0);
	}

	// set current buffer
	buffer_ = b;
	buffer_->text().init(bv_);
	buffer_->text().textwidth_ = workarea().workWidth();
	buffer_->text().fullRebreak();

	top_y_ = 0;

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
		if (bv_->text() == 0)
			resizeCurrentBuffer();

		// FIXME: needed when ?
		fitCursor();

		// Buffer-dependent dialogs should be updated or
		// hidden. This should go here because some dialogs (eg ToC)
		// require bv_->text.
		owner_->getDialogs().updateBufferDependent(true);
	} else {
		lyxerr[Debug::INFO] << "  No Buffer!" << endl;
		owner_->getDialogs().hideBufferDependent();
	}

	update();
	updateScrollbar();
	owner_->updateMenubar();
	owner_->updateToolbar();
	owner_->updateLayoutChoice();
	owner_->updateWindowTitle();

	// Don't forget to update the Layout
	if (buffer_)
		owner_->setLayout(bv_->text()->cursorPar()->layout()->name());

	if (lyx::graphics::Previews::activated() && buffer_)
		lyx::graphics::Previews::get().generateBufferPreviews(*buffer_);
}


bool BufferView::Pimpl::fitCursor()
{
	lyxerr << "BufferView::Pimpl::fitCursor." << endl;
	if (screen().fitCursor(bv_)) {
		updateScrollbar();
		return true;
	}
	return false;
}


void BufferView::Pimpl::redoCurrentBuffer()
{
	lyxerr[Debug::INFO] << "BufferView::redoCurrentBuffer" << endl;
	if (buffer_ && bv_->text()) {
		resizeCurrentBuffer();
		updateScrollbar();
		owner_->updateLayoutChoice();
	}
}


void BufferView::Pimpl::resizeCurrentBuffer()
{
	lyxerr[Debug::INFO] << "resizeCurrentBuffer" << endl;

	int par = -1;
	int selstartpar = -1;
	int selendpar = -1;

	pos_type pos = 0;
	pos_type selstartpos = 0;
	pos_type selendpos = 0;
	bool selection = false;
	bool mark_set  = false;

	owner_->busy(true);

	owner_->message(_("Formatting document..."));

	lyxerr << "### resizeCurrentBuffer: text" << bv_->text() << endl;
	if (!bv_->text())
		return;

	//if (bv_->text()) {
		par = bv_->text()->cursor.par();
		pos = bv_->text()->cursor.pos();
		selstartpar = bv_->text()->selection.start.par();
		selstartpos = bv_->text()->selection.start.pos();
		selendpar = bv_->text()->selection.end.par();
		selendpos = bv_->text()->selection.end.pos();
		selection = bv_->text()->selection.set();
		mark_set = bv_->text()->selection.mark();
		bv_->text()->fullRebreak();
		update();
	//} else {
	//	bv_->setText(new LyXText(bv_, 0, false, bv_->buffer()->paragraphs()));
	//	bv_->text()->init(bv_);
	//}

	if (par != -1) {
		bv_->text()->selection.set(true);
		// At this point just to avoid the Delete-Empty-Paragraph-
		// Mechanism when setting the cursor.
		bv_->text()->selection.mark(mark_set);
		if (selection) {
			bv_->text()->setCursor(selstartpar, selstartpos);
			bv_->text()->selection.cursor = bv_->text()->cursor;
			bv_->text()->setCursor(selendpar, selendpos);
			bv_->text()->setSelection();
			bv_->text()->setCursor(par, pos);
		} else {
			bv_->text()->setCursor(par, pos);
			bv_->text()->selection.cursor = bv_->text()->cursor;
			bv_->text()->selection.set(false);
		}
	}

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
		lyxerr[Debug::GUI] << "no text in updateScrollbar" << endl;
		workarea().setScrollbarParams(0, 0, 0);
		return;
	}

	LyXText const & t = *bv_->text();

	lyxerr[Debug::GUI] << "Updating scrollbar: h " << t.height << ", top_y() "
		<< top_y() << ", default height " << defaultRowHeight() << endl;

	workarea().setScrollbarParams(t.height, top_y(), defaultRowHeight());
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

	LyXText * text = bv_->text();
	if (text->cursor.y() < first)
		text->setCursorFromCoordinates(0, first);
	else if (text->cursor.y() > last)
		text->setCursorFromCoordinates(0, last);

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
	new_top_y = std::min(t->height - 4 * line_height, new_top_y);
	new_top_y = std::max(0, new_top_y);

	scrollDocView(new_top_y);

	// Update the scrollbar.
	workarea().setScrollbarParams(t->height, top_y(), defaultRowHeight());
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
		(!bv_->text()->xsel_cache.set() ||
		 text->selection.start != bv_->text()->xsel_cache.start ||
		 text->selection.end != bv_->text()->xsel_cache.end))
	{
		bv_->text()->xsel_cache = text->selection;
		sel = text->selectionAsString(*bv_->buffer(), false);
	} else if (!text->selection.set()) {
		sel = string();
		bv_->text()->xsel_cache.set(false);
	}
	if (!sel.empty()) {
		workarea().putClipboard(sel);
	}
}


void BufferView::Pimpl::selectionLost()
{
	if (available()) {
		screen().hideCursor();
		bv_->getLyXText()->clearSelection();
		bv_->text()->xsel_cache.set(false);
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
	//lyxerr << "BufferView::update()" << endl;
	// fix cursor coordinate cache in case something went wrong

	// check needed to survive LyX startup
	if (bv_->getLyXText()) {
		// update all 'visible' paragraphs
		ParagraphList::iterator beg;
		ParagraphList::iterator end;
		getParsInRange(buffer_->paragraphs(),
			       top_y(), top_y() + workarea().workHeight(),
			       beg, end);
		bv_->text()->redoParagraphs(beg, end);
		bv_->getLyXText()->redoCursor();
		updateScrollbar();
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
	return buffer_ && bv_->text();
}


Change const BufferView::Pimpl::getCurrentChange()
{
	if (!bv_->buffer()->params().tracking_changes)
		return Change(Change::UNCHANGED);

	LyXText * text = bv_->getLyXText();

	if (!text->selection.set())
		return Change(Change::UNCHANGED);

	return text->getPar(text->selection.start)
		->lookupChangeFull(text->selection.start.pos());
}


void BufferView::Pimpl::savePosition(unsigned int i)
{
	if (i >= saved_positions_num)
		return;
	saved_positions[i] = Position(buffer_->fileName(),
				      bv_->text()->cursorPar()->id(),
				      bv_->text()->cursor.pos());
	if (i > 0)
		owner_->message(bformat(_("Saved bookmark %1$s"), tostr(i)));
}


void BufferView::Pimpl::restorePosition(unsigned int i)
{
	if (i >= saved_positions_num)
		return;

	string const fname = saved_positions[i].filename;

	bv_->text()->clearSelection();

	if (fname != buffer_->fileName()) {
		Buffer * b = 0;
		if (bufferlist.exists(fname))
			b = bufferlist.getBuffer(fname);
		else {
			b = bufferlist.newBuffer(fname);
			::loadLyXFile(b, fname); // don't ask, just load it
		}
		if (b)
			buffer(b);
	}

	ParIterator par = buffer_->getParFromID(saved_positions[i].par_id);
	if (par == buffer_->par_iterator_end())
		return;

	bv_->text()->setCursor(par.pit(),
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

	text->clearSelection();
	int const half_height = workarea().workHeight() / 2;
	int new_y = std::max(0, text->cursor.y() - half_height);

	// FIXME: look at this comment again ...
	// This updates top_y() but means the fitCursor() call
	// from the update(FITCUR) doesn't realise that we might
	// have moved (e.g. from GOTOPARAGRAPH), so doesn't cause
	// the scrollbar to be updated as it should, so we have
	// to do it manually. Any operation that does a center()
	// and also might have moved top_y() must make sure to call
	// updateScrollbar() currently. Never mind that this is a
	// pretty obfuscated way of updating t->top_y()
	top_y(new_y);
}


void BufferView::Pimpl::stuffClipboard(string const & stuff) const
{
	workarea().putClipboard(stuff);
}


InsetOld * BufferView::Pimpl::getInsetByCode(InsetOld::Code code)
{
#if 0
	LyXCursor cursor = bv_->getLyXText()->cursor;
	Buffer::inset_iterator it =
		find_if(Buffer::inset_iterator(
			cursorPar(), cursor.pos()),
			buffer_->inset_iterator_end(),
			lyx::compare_memfun(&Inset::lyxCode, code));
	return it != buffer_->inset_iterator_end() ? (*it) : 0;
#else
	// Ok, this is a little bit too brute force but it
	// should work for now. Better infrastructure is coming. (Lgb)

	Buffer * b = bv_->buffer();
	LyXText * text =  bv_->getLyXText();

	Buffer::inset_iterator beg = b->inset_iterator_begin();
	Buffer::inset_iterator end = b->inset_iterator_end();

	bool cursorPar_seen = false;

	for (; beg != end; ++beg) {
		if (beg.getPar() == text->cursorPar()) {
			cursorPar_seen = true;
		}
		if (cursorPar_seen) {
			if (beg.getPar() == text->cursorPar()
			    && beg.getPos() >= text->cursor.pos()) {
				break;
			} else if (beg.getPar() != text->cursorPar()) {
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
	bool const tracking(buf->params().tracking_changes);

	if (!tracking) {
		ParIterator const end = buf->par_iterator_end();
		for (ParIterator it = buf->par_iterator_begin(); it != end; ++it)
			it->trackChanges();
		buf->params().tracking_changes = true;

		// we cannot allow undos beyond the freeze point
		buf->undostack().clear();
	} else {
		update();
		bv_->text()->setCursor(0, 0);
#warning changes FIXME
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

#warning remove me
LCursor theTempCursor(0);

namespace {

	InsetOld * insetFromCoords(BufferView * bv, int x, int y)
	{
		LyXText * text = bv->text();
		InsetOld * inset = 0;
		theTempCursor = LCursor(bv);
		while (true) {
			InsetOld * inset_hit = text->checkInsetHit(x, y);
			if (!inset_hit) {
				lyxerr << "no further inset hit" << endl;
				break;
			}
			inset = inset_hit;
			if (!inset_hit->descendable()) {
				lyxerr << "not descendable" << endl;
				break;
			}
			text = inset_hit->getText(0);
			lyxerr << "Hit inset: " << inset << " at x: " << x
				<< " text: " << text << " y: " << y << endl;
			theTempCursor.push(static_cast<UpdatableInset*>(inset));
		}
		lyxerr << "theTempCursor: " << theTempCursor << endl;
		return inset;
	}

}


bool BufferView::Pimpl::workAreaDispatch(FuncRequest const & cmd)
{
	switch (cmd.action) {
	case LFUN_MOUSE_MOTION: {
		FuncRequest cmd1(cmd, bv_);
		UpdatableInset * inset = bv_->cursor().innerInset();
		DispatchResult res;
		if (inset) {
			cmd1.x -= inset->x();
			cmd1.y -= inset->y();
			res = inset->dispatch(cmd1);
		} else {
			cmd1.y += bv_->top_y();
			res = bv_->cursor().innerText()->dispatch(cmd1);
		}

		if (bv_->fitCursor() || res.update()) {
			bv_->update();
			bv_->cursor().updatePos();
		}

		return true;
	}

	case LFUN_MOUSE_PRESS:
	case LFUN_MOUSE_RELEASE:
	case LFUN_MOUSE_DOUBLE:
	case LFUN_MOUSE_TRIPLE: {
		// We pass those directly to the Bufferview, since
		// otherwise selection handling breaks down

		// Doesn't go through lyxfunc, so we need to update
		// the layout choice etc. ourselves

		// e.g. Qt mouse press when no buffer
		if (!available())
			return false;

		screen().hideCursor();

		// either the inset under the cursor or the surrounding LyXText will
		// handle this event.

		// built temporary path to inset
		InsetOld * inset = insetFromCoords(bv_, cmd.x, cmd.y);
		FuncRequest cmd1(cmd, bv_);
		DispatchResult res;

		// try to dispatch to that inset
		if (inset) {
			FuncRequest cmd2 = cmd1;
			lyxerr << "dispatching action " << cmd2.action
			       << " to inset " << inset << endl;
			cmd2.x -= inset->x();
			cmd2.y -= inset->y();
			res = inset->dispatch(cmd2);
			if (res.update()) {
				bv_->update();
				bv_->cursor().updatePos();
			}
			res.update(false);
			switch (res.val()) {
				case FINISHED:
				case FINISHED_RIGHT:
				case FINISHED_UP:
				case FINISHED_DOWN:
					theTempCursor.pop();
					bv_->cursor() = theTempCursor;
					bv_->cursor().innerText()->setCursorFromCoordinates(cmd.x, top_y() + cmd.y);
					if (bv_->fitCursor())
						bv_->update();
					return true;
				default:
					lyxerr << "not dispatched by inner inset val: " << res.val() << endl;
					break;
			}
		}

		// otherwise set cursor to surrounding LyXText
		if (!res.dispatched()) {
			lyxerr << "temp cursor is: " << theTempCursor << endl;
			lyxerr << "dispatching " << cmd1
			       << " to surrounding LyXText "
			       << theTempCursor.innerText() << endl;
			bv_->cursor() = theTempCursor;
			cmd1.y += bv_->top_y();
			res = bv_->cursor().innerText()->dispatch(cmd1);
			if (bv_->fitCursor() || res.update())
				bv_->update();

			//return DispatchResult(true, true);
		}
		// see workAreaKeyPress
		cursor_timeout.restart();
		screen().showCursor(*bv_);

		// skip these when selecting
		if (cmd.action != LFUN_MOUSE_MOTION) {
			owner_->updateLayoutChoice();
			owner_->updateToolbar();
		}

		// slight hack: this is only called currently when we
		// clicked somewhere, so we force through the display
		// of the new status here.
		owner_->clearMessage();
		return true;
	}

	default:
		owner_->dispatch(cmd);
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

	LyXTextClass const & tclass = buffer_->params().getLyXTextClass();

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
		break;
	}

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
#warning is this code ever called?
		// Remove if not triggered. Mail lyx-devel if triggered.
		// This code was replaced by code in text3.C.
		BOOST_ASSERT(false);
		string const name = ev.getArg(0);

		InsetBase * inset = owner_->getDialogs().getOpenInset(name);
		if (inset) {
			// This works both for 'original' and 'mathed' insets.
			// Note that the localDispatch performs update also.
			FuncRequest fr(bv_, LFUN_INSET_MODIFY, ev.argument);
			inset->dispatch(fr);
		} else {
			FuncRequest fr(bv_, LFUN_INSET_INSERT, ev.argument);
			dispatch(fr);
		}
	}
	break;

	case LFUN_INSET_INSERT: {
		// Same as above.
		BOOST_ASSERT(false);
		InsetOld * inset = createInset(ev);
		if (!inset || !insertInset(inset))
			delete inset;
		break;
	}

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
		string data;
		params2string(*bv_->getLyXText()->cursorPar(), data);
		data = "show\n" + data;
		bv_->owner()->getDialogs().show("paragraph", data);
		break;
	}

	case LFUN_PARAGRAPH_UPDATE:
		updateParagraphDialog();
		break;

	case LFUN_PARAGRAPH_APPLY:
		setParagraphParams(*bv_, ev.argument);
		break;

	case LFUN_THESAURUS_ENTRY: {
		string arg = ev.argument;

		if (arg.empty()) {
			arg = bv_->getLyXText()->selectionAsString(*buffer_,
								   false);

			// FIXME
			if (arg.size() > 100 || arg.empty()) {
				// Get word or selection
				bv_->getLyXText()->selectWordWhenUnderCursor(lyx::WHOLE_WORD);
				arg = bv_->getLyXText()->selectionAsString(*buffer_, false);
				// FIXME: where is getLyXText()->unselect(bv_) ?
			}
		}

		bv_->owner()->getDialogs().show("thesaurus", arg);
		break;
	}

	case LFUN_TRACK_CHANGES:
		trackChanges();
		break;

	case LFUN_MERGE_CHANGES:
		owner_->getDialogs().show("changes");
		break;

	case LFUN_ACCEPT_ALL_CHANGES: {
		bv_->text()->setCursor(0, 0);
#warning FIXME changes
		while (lyx::find::findNextChange(bv_))
			bv_->getLyXText()->acceptChange();
		update();
		break;
	}

	case LFUN_REJECT_ALL_CHANGES: {
		bv_->text()->setCursor(0, 0);
#warning FIXME changes
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
		return bv_->getLyXText()->dispatch(FuncRequest(ev, bv_)).dispatched();
	} // end of switch

	return true;
}


bool BufferView::Pimpl::insertInset(InsetOld * inset, string const & lout)
{
#ifdef LOCK
	// if we are in a locking inset we should try to insert the
	// inset there otherwise this is a illegal function now
	if (bv_->theLockingInset()) {
		if (bv_->theLockingInset()->insetAllowed(inset))
			return bv_->theLockingInset()->insertInset(bv_, inset);
		return false;
	}
#endif

	// not quite sure if we want this...
	bv_->text()->recUndo(bv_->text()->cursor.par());
	freezeUndo();

	bv_->text()->clearSelection();
	if (!lout.empty()) {
		bv_->text()->breakParagraph(bv_->buffer()->paragraphs());

		if (!bv_->text()->cursorPar()->empty()) {
			bv_->text()->cursorLeft(bv_);
			bv_->text()->breakParagraph(bv_->buffer()->paragraphs());
		}

		string lres = lout;
		LyXTextClass const & tclass = buffer_->params().getLyXTextClass();
		bool hasLayout = tclass.hasLayout(lres);

		bv_->text()->setLayout(hasLayout ? lres : tclass.defaultLayoutName());
		bv_->text()->setParagraph(Spacing(), LYX_ALIGN_LAYOUT, string(), 0);
	}
	bv_->cursor().innerText()->insertInset(inset);
	unFreezeUndo();
	return true;
}


bool BufferView::Pimpl::ChangeInsets(InsetOld::Code code,
				     string const & from, string const & to)
{
	bool need_update = false;
	LyXCursor cursor = bv_->text()->cursor;
	LyXCursor tmpcursor = cursor;
	cursor.par(tmpcursor.par());
	cursor.pos(tmpcursor.pos());

	ParIterator end = bv_->buffer()->par_iterator_end();
	for (ParIterator it = bv_->buffer()->par_iterator_begin();
	     it != end; ++it) {
		bool changed_inset = false;
		for (InsetList::iterator it2 = it->insetlist.begin();
		     it2 != it->insetlist.end(); ++it2) {
			if (it2->inset->lyxCode() == code) {
				InsetCommand * inset = static_cast<InsetCommand *>(it2->inset);
				if (inset->getContents() == from) {
					inset->setContents(to);
					changed_inset = true;
				}
			}
		}
		if (changed_inset) {
			need_update = true;

			// FIXME

			// The test it.size()==1 was needed to prevent crashes.
			// How to set the cursor correctly when it.size()>1 ??
			if (it.size() == 1) {
				bv_->text()->setCursorIntern(bv_->text()->parOffset(it.pit()), 0);
				bv_->text()->redoParagraph(bv_->text()->cursorPar());
			}
		}
	}
	bv_->text()->setCursorIntern(cursor.par(), cursor.pos());
	return need_update;
}


void BufferView::Pimpl::updateParagraphDialog()
{
	if (!bv_->owner()->getDialogs().visible("paragraph"))
		return;
	Paragraph const & par = *bv_->getLyXText()->cursorPar();
	string data;
	params2string(par, data);

	// Will the paragraph accept changes from the dialog?
	InsetOld * const inset = par.inInset();
	bool const accept =
		!(inset && inset->forceDefaultParagraphs(inset));

	data = "update " + tostr(accept) + '\n' + data;
	bv_->owner()->getDialogs().update("paragraph", data);
}
