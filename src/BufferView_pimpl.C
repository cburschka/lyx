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
#include "coordcache.h"
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
#include "LaTeXFeatures.h"
#include "lyx_cb.h" // added for Dispatch functions
#include "lyx_main.h"
#include "lyxfind.h"
#include "lyxfunc.h"
#include "lyxtext.h"
#include "lyxrc.h"
#include "lastfiles.h"
#include "metricsinfo.h"
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
#include "frontends/font_metrics.h"
#include "frontends/LyXView.h"
#include "frontends/LyXScreenFactory.h"
#include "frontends/screen.h"
#include "frontends/WorkArea.h"
#include "frontends/WorkAreaFactory.h"

#include "graphics/Previews.h"

#include "support/convert.h"
#include "support/filefilterlist.h"
#include "support/filetools.h"
#include "support/forkedcontr.h"
#include "support/package.h"
#include "support/types.h"

#include <boost/bind.hpp>

#include <functional>

using lyx::pos_type;

using lyx::support::AddPath;
using lyx::support::bformat;
using lyx::support::FileFilterList;
using lyx::support::FileSearch;
using lyx::support::ForkedcallsController;
using lyx::support::IsDirWriteable;
using lyx::support::MakeDisplayPath;
using lyx::support::MakeAbsPath;
using lyx::support::package;

using std::endl;
using std::istringstream;
using std::make_pair;
using std::min;
using std::max;
using std::string;
using std::mem_fun_ref;


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


/// Get next inset of this class from current cursor position
template <class T>
T * getInsetByCode(LCursor & cur, InsetBase::Code code)
{
	T * inset = 0;
	DocIterator it = cur;
	if (it.nextInset() &&
	    it.nextInset()->lyxCode() == code) {
		inset = static_cast<T*>(it.nextInset());
	}
	return inset;
}


} // anon namespace


BufferView::Pimpl::Pimpl(BufferView & bv, LyXView * owner,
			 int width, int height)
	: bv_(&bv), owner_(owner), buffer_(0), cursor_timeout(400),
	  using_xterm_cursor(false), cursor_(bv) ,
	  anchor_ref_(0), offset_ref_(0)
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


void BufferView::Pimpl::setBuffer(Buffer * b)
{
	lyxerr[Debug::INFO] << "Setting buffer in BufferView ("
			    << b << ')' << endl;
	if (buffer_)
		disconnectBuffer();

	// if we are closing current buffer, switch to the first in
	// buffer list.
	if (!b) {
		lyxerr[Debug::INFO] << "  No Buffer!" << endl;
		// we are closing the buffer, use the first buffer as current
		buffer_ = bufferlist.first();
		owner_->getDialogs().hideBufferDependent();
	} else {
		// set current buffer
		buffer_ = b;
	}

	// reset old cursor
	cursor_ = LCursor(*bv_);
	anchor_ref_ = 0;
	offset_ref_ = 0;


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

		// Buffer-dependent dialogs should be updated or
		// hidden. This should go here because some dialogs (eg ToC)
		// require bv_->text.
		owner_->getDialogs().updateBufferDependent(true);
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

	LyXText & t = *bv_->text();
	if (anchor_ref_ >  int(t.paragraphs().size()) - 1) {
		anchor_ref_ = int(t.paragraphs().size()) - 1;
		offset_ref_ = 0;
	}

	lyxerr[Debug::GUI]
		<< "Updating scrollbar: height: " << t.paragraphs().size()
		<< " curr par: " << bv_->cursor().bottom().pit()
		<< " default height " << defaultRowHeight() << endl;

	//it would be better to fix the scrollbar to understand
	//values in [0..1] and divide everything by wh
	int const wh = workarea().workHeight() / 4;
	int const h = t.getPar(anchor_ref_).height();
	workarea().setScrollbarParams(t.paragraphs().size() * wh, anchor_ref_ * wh + int(offset_ref_ * wh / float(h)), int (wh * defaultRowHeight() / float(h)));
//	workarea().setScrollbarParams(t.paragraphs().size(), anchor_ref_, 1);
}


void BufferView::Pimpl::scrollDocView(int value)
{
	lyxerr[Debug::GUI] << "scrollDocView of " << value << endl;

	if (!buffer_)
		return;

	screen().hideCursor();

	int const wh = workarea().workHeight() / 4;

	LyXText & t = *bv_->text();

	float const bar = value / float(wh * t.paragraphs().size());

	anchor_ref_ = int(bar * t.paragraphs().size());
	t.redoParagraph(anchor_ref_);
	int const h = t.getPar(anchor_ref_).height();
	offset_ref_ = int((bar * t.paragraphs().size() - anchor_ref_) * h);
	update();

	if (!lyxrc.cursor_follows_scrollbar)
		return;

	int const height = 2 * defaultRowHeight();
	int const first = height;
	int const last = workarea().workHeight() - height;
	LCursor & cur = bv_->cursor();

	bv_funcs::CurStatus st = bv_funcs::status(bv_, cur);

	switch (st) {
	case bv_funcs::CUR_ABOVE:
		t.setCursorFromCoordinates(cur, 0, first);
		cur.clearSelection();
		break;
	case bv_funcs::CUR_BELOW:
		t.setCursorFromCoordinates(cur, 0, last);
		cur.clearSelection();
		break;
	case bv_funcs::CUR_INSIDE:
		int const y = bv_funcs::getPos(cur).y_;
		int const newy = min(last, max(y, first));
		if (y != newy) {
			cur.reset(bv_->buffer()->inset());
			t.setCursorFromCoordinates(cur, 0, newy);
		}
	}
	owner_->updateLayoutChoice();
}


void BufferView::Pimpl::scroll(int lines)
{
//	if (!buffer_)
//		return;
//
//	LyXText const * t = bv_->text();
//	int const line_height = defaultRowHeight();
//
//	// The new absolute coordinate
//	int new_top_y = top_y() + lines * line_height;
//
//	// Restrict to a valid value
//	new_top_y = std::min(t->height() - 4 * line_height, new_top_y);
//	new_top_y = std::max(0, new_top_y);
//
//	scrollDocView(new_top_y);
//
//	// Update the scrollbar.
//	workarea().setScrollbarParams(t->height(), top_y(), defaultRowHeight());
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


bool BufferView::Pimpl::fitCursor()
{
	if (bv_funcs::status(bv_, bv_->cursor()) == bv_funcs::CUR_INSIDE) {
		LyXFont const font = bv_->cursor().getFont();
		int const asc = font_metrics::maxAscent(font);
		int const des = font_metrics::maxDescent(font);
		Point p = bv_funcs::getPos(bv_->cursor());
		if (p.y_ - asc >= 0 && p.y_ + des < bv_->workHeight())
			return false;
	}
	bv_->center();
	return true;
}


void BufferView::Pimpl::update(bool fitcursor, bool forceupdate)
{
	lyxerr << "BufferView::Pimpl::update(fc=" << fitcursor << ", fu="
	       << forceupdate << ")  buffer: " << buffer_ << endl;

	// check needed to survive LyX startup
	if (buffer_) {
		// update macro store
		buffer_->buildMacros();
		// first drawing step

		CoordCache backup;
		std::swap(theCoords, backup);
		theCoords.startUpdating();
		//
		ViewMetricsInfo vi = metrics();

		if (fitcursor && fitCursor()) {
			forceupdate = true;
			vi = metrics();
		}
		if (forceupdate) {
			// second drawing step
			screen().redraw(*bv_, vi);
			theCoords.doneUpdating();
		} else {
			// Abort updating of the coord cache - just restore the old one
			std::swap(theCoords, backup);
		}
	} else
		screen().greyOut();

	// and the scrollbar
	updateScrollbar();
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

	return text->getPar(cur.selBegin().pit()).
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
		owner_->message(bformat(_("Saved bookmark %1$d"), i));
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
		owner_->message(bformat(_("Moved to bookmark %1$d"), i));
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
	CursorSlice & bot = bv_->cursor().bottom();
	lyx::pit_type const pit = bot.pit();
	bot.text()->redoParagraph(pit);
	Paragraph const & par = bot.text()->paragraphs()[pit];
	anchor_ref_ = pit;
	offset_ref_ = bv_funcs::coordOffset(bv_->cursor()).y_ + par.ascent()
		- workarea().workHeight() / 2;
}


void BufferView::Pimpl::stuffClipboard(string const & content) const
{
	workarea().putClipboard(content);
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
				  string(AddPath(package().system_support(), "examples"))));

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

	bv_->cursor().clearSelection();
	bv_->getLyXText()->breakParagraph(bv_->cursor());

	BOOST_ASSERT(bv_->cursor().inTexted());

	string const fname = MakeAbsPath(filename);
	bool const res = bv_->buffer()->readFile(fname, bv_->cursor().pit());
	bv_->resize();

	string s = res ? _("Document %1$s inserted.")
	               : _("Could not insert document %1$s");
	owner_->message(bformat(s, disp_fn));
}


void BufferView::Pimpl::trackChanges()
{
	Buffer * buf = bv_->buffer();
	bool const tracking = buf->params().tracking_changes;

	if (!tracking) {
		for_each(buf->par_iterator_begin(),
			 buf->par_iterator_end(),
			 bind(&Paragraph::trackChanges, _1, Change::UNCHANGED));
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

		for_each(buf->par_iterator_begin(),
			 buf->par_iterator_end(),
			 mem_fun_ref(&Paragraph::untrackChanges));

		buf->params().tracking_changes = false;
	}

	buf->redostack().clear();
}


bool BufferView::Pimpl::workAreaDispatch(FuncRequest const & cmd0)
{
	lyxerr << "BufferView::Pimpl::workAreaDispatch: request: "
	  << cmd0 << std::endl;
	// this is only called for mouse related events including
	// LFUN_FILE_OPEN generated by drag-and-drop.
	FuncRequest cmd = cmd0;

	// handle drag&drop
	if (cmd.action == LFUN_FILE_OPEN) {
		owner_->dispatch(cmd);
		return true;
	}

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
	cmd.y = min(max(cmd.y,-1), bv_->workHeight());
	InsetBase * inset = bv_->text()->editXY(cur, cmd.x, cmd.y);
	lyxerr << " * hit inset at tip: " << inset << endl;
	lyxerr << " * created temp cursor:" << cur << endl;

	// Put anchor at the same position.
	cur.resetAnchor();

	// Try to dispatch to an non-editable inset near this position
	// via the temp cursor. If the inset wishes to change the real
	// cursor it has to do so explicitly by using
	//  cur.bv().cursor() = cur;  (or similar)
	if (inset)
		inset->dispatch(cur, cmd);

	// Now dispatch to the temporary cursor. If the real cursor should
	// be modified, the inset's dispatch has to do so explicitly.
	if (!cur.result().dispatched())
		cur.dispatch(cmd);

	if (cur.result().dispatched()) {
		// Redraw if requested or necessary.
		update(cur.result().update(), cur.result().update());
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
	case LFUN_WORDS_COUNT:
		flag.enabled(true);
		break;

	case LFUN_BOOKMARK_GOTO:
		flag.enabled(bv_->isSavedPosition(convert<unsigned int>(cmd.argument)));
		break;
	case LFUN_TRACK_CHANGES:
		flag.enabled(true);
		flag.setOnOff(buf->params().tracking_changes);
		break;

	case LFUN_OUTPUT_CHANGES: {
		LaTeXFeatures features(*buf, buf->params(), false);
		flag.enabled(buf && buf->params().tracking_changes
			&& features.isAvailable("dvipost"));
		flag.setOnOff(buf->params().output_changes);
		break;
	}

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
		savePosition(convert<unsigned int>(cmd.argument));
		break;

	case LFUN_BOOKMARK_GOTO:
		restorePosition(convert<unsigned int>(cmd.argument));
		break;

	case LFUN_REF_GOTO: {
		string label = cmd.argument;
		if (label.empty()) {
			InsetRef * inset =
				getInsetByCode<InsetRef>(bv_->cursor(),
							 InsetBase::REF_CODE);
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

	case LFUN_OUTPUT_CHANGES: {
		Buffer * buf = bv_->buffer();
		bool const state = buf->params().output_changes;
		buf->params().output_changes = !state;
		break;
	}

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

	case LFUN_WORDS_COUNT: {
		DocIterator from, to;
		if (cur.selection()) {
			from = cur.selectionBegin();
			to = cur.selectionEnd();
		} else {
			from = doc_iterator_begin(bv_->buffer()->inset());
			to = doc_iterator_end(bv_->buffer()->inset());
		}
		int const count = countWords(from, to);
		string message;
		if (count != 1) {
			if (cur.selection())
				message = bformat(_("%1$d words in selection."),
					  count);
				else
					message = bformat(_("%1$d words in document."),
							  count);
		}
		else {
			if (cur.selection())
				message = _("One word in selection.");
			else
				message = _("One word in document.");
		}

		Alert::information(_("Count words"), message);
	}
		break;
	default:
		return false;
	}

	return true;
}


ViewMetricsInfo BufferView::Pimpl::metrics()
{
	// remove old position cache
	theCoords.clear();
	BufferView & bv = *bv_;
	LyXText * const text = bv.text();
	if (anchor_ref_ > int(text->paragraphs().size() - 1)) {
		anchor_ref_ = int(text->paragraphs().size() - 1);
		offset_ref_ = 0;
	}

	lyx::pit_type const pit = anchor_ref_;
	int pit1 = pit;
	int pit2 = pit;
	size_t npit = text->paragraphs().size();
	lyxerr << "npit: " << npit << " pit1: " << pit1
		<< " pit2: " << pit2 << endl;

	// rebreak anchor par
	text->redoParagraph(pit);
	int y0 = text->getPar(pit1).ascent() - offset_ref_;

	// redo paragraphs above cursor if necessary
	int y1 = y0;
	while (y1 > 0 && pit1 > 0) {
		y1 -= text->getPar(pit1).ascent();
		--pit1;
		text->redoParagraph(pit1);
		y1 -= text->getPar(pit1).descent();
	}


	// take care of ascent of first line
	y1 -= text->getPar(pit1).ascent();

	//normalize anchor for next time
	anchor_ref_ = pit1;
	offset_ref_ = -y1;

	// grey at the beginning is ugly
	if (pit1 == 0 && y1 > 0) {
		y0 -= y1;
		y1 = 0;
		anchor_ref_ = 0;
	}

	// redo paragraphs below cursor if necessary
	int y2 = y0;
	while (y2 < bv.workHeight() && pit2 < int(npit) - 1) {
		y2 += text->getPar(pit2).descent();
		++pit2;
		text->redoParagraph(pit2);
		y2 += text->getPar(pit2).ascent();
	}

	// take care of descent of last line
	y2 += text->getPar(pit2).descent();

	// the coordinates of all these paragraphs are correct, cache them
	int y = y1;
	for (lyx::pit_type pit = pit1; pit <= pit2; ++pit) {
		y += text->getPar(pit).ascent();
		theCoords.parPos()[text][pit] = Point(0, y);
		y += text->getPar(pit).descent();
	}

	lyxerr << "bv:metrics:  y1: " << y1 << " y2: " << y2 << endl;
	return ViewMetricsInfo(pit1, pit2, y1, y2);
}
