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
 * \author Abdelrazak Younes
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
#include "CutAndPaste.h"
#include "debug.h"
#include "dispatchresult.h"
#include "errorlist.h"
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
#include "session.h"
#include "metricsinfo.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "pariterator.h"
#include "toc.h"
#include "undo.h"
#include "vspace.h"

#include "insets/insetbibtex.h"
#include "insets/insetref.h"
#include "insets/insettext.h"

#include "frontends/Alert.h"
#include "frontends/Dialogs.h"
#include "frontends/FileDialog.h"
#include "frontends/font_metrics.h"
#include "frontends/Gui.h"
#include "frontends/LyXView.h"
#include "frontends/Selection.h"

#include "graphics/Previews.h"

#include "support/convert.h"
#include "support/filefilterlist.h"
#include "support/filetools.h"
#include "support/package.h"
#include "support/types.h"

#include <boost/bind.hpp>
#include <boost/current_function.hpp>

#include <functional>
#include <vector>

using lyx::frontend::Clipboard;
using lyx::frontend::Gui;

using lyx::pos_type;

using lyx::support::addPath;
using lyx::support::bformat;
using lyx::support::FileFilterList;
using lyx::support::fileSearch;
using lyx::support::isDirWriteable;
using lyx::support::makeDisplayPath;
using lyx::support::makeAbsPath;
using lyx::support::package;

using std::endl;
using std::istringstream;
using std::make_pair;
using std::min;
using std::max;
using std::string;
using std::mem_fun_ref;
using std::vector;

extern BufferList bufferlist;


namespace {

unsigned int const saved_positions_num = 20;


/// Return an inset of this class if it exists at the current cursor position
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


BufferView::Pimpl::Pimpl(BufferView & bv, LyXView * owner)
	: bv_(&bv), owner_(owner), buffer_(0), wh_(0),
	  cursor_(bv),
	  multiparsel_cache_(false), anchor_ref_(0), offset_ref_(0)
{
	xsel_cache_.set = false;

	saved_positions.resize(saved_positions_num);
	// load saved bookmarks
	lyx::Session::BookmarkList & bmList = LyX::ref().session().loadBookmarks();
	for (lyx::Session::BookmarkList::iterator bm = bmList.begin();
		bm != bmList.end(); ++bm)
		if (bm->get<0>() < saved_positions_num)
			saved_positions[bm->get<0>()] = Position( bm->get<1>(), bm->get<2>(), bm->get<3>() );
	// and then clear them
	bmList.clear();
}


bool BufferView::Pimpl::loadLyXFile(string const & filename, bool tolastfiles)
{
	// Get absolute path of file and add ".lyx"
	// to the filename if necessary
	string s = fileSearch(string(), filename, "lyx");

	bool const found = !s.empty();

	if (!found)
		s = filename;

	// File already open?
	if (bufferlist.exists(s)) {
		string const file = makeDisplayPath(s, 20);
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

	Buffer * b = 0;

	if (found) {
		b = bufferlist.newBuffer(s);
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

		if (ret == 0) {
			b = newFile(s, string(), true);
			if (!b)
				return false;
		} else
			return false;
	}

	setBuffer(b);
	// Send the "errors" signal in case of parsing errors
	b->errors("Parse");

	// scroll to the position when the file was last closed
	if (lyxrc.use_lastfilepos) {
		lyx::pit_type pit;
		lyx::pos_type pos;
		boost::tie(pit, pos) = LyX::ref().session().loadFilePosition(s);
		// I am not sure how to separate the following part to a function
		// so I will leave this to Lars.
		//
		// check pit since the document may be externally changed.
		if ( static_cast<size_t>(pit) < b->paragraphs().size() ) {
			ParIterator it = b->par_iterator_begin();
			ParIterator const end = b->par_iterator_end();
			for (; it != end; ++it)
				if (it.pit() == pit) {
					// restored pos may be bigger than it->size
					bv_->setCursor(makeDocIterator(it, min(pos, it->size())));
					bv_->update(Update::FitCursor);
					break;
				}
		}
	}

	if (tolastfiles)
		LyX::ref().session().addLastFile(b->fileName());

	return true;
}


int BufferView::Pimpl::width() const
{
	return width_;
}


int BufferView::Pimpl::height() const
{
	return height_;
}


void BufferView::Pimpl::setBuffer(Buffer * b)
{
	lyxerr[Debug::INFO] << BOOST_CURRENT_FUNCTION
			    << "[ b = " << b << "]" << endl;

	if (buffer_) {
		// Save the actual cursor position and anchor inside the
		// buffer so that it can be restored in case we rechange
		// to this buffer later on.
		buffer_->saveCursor(cursor_.selectionBegin(),
				    cursor_.selectionEnd());
		// current buffer is going to be switched-off, save cursor pos
		LyX::ref().session().saveFilePosition(buffer_->fileName(),
			boost::tie(cursor_.pit(), cursor_.pos()) );
	}

	// If we're quitting lyx, don't bother updating stuff
	if (quitting) {
		buffer_ = 0;
		return;
	}

	// If we are closing current buffer, switch to the first in
	// buffer list.
	if (!b) {
		lyxerr[Debug::INFO] << BOOST_CURRENT_FUNCTION
				    << " No Buffer!" << endl;
		// We are closing the buffer, use the first buffer as current
		buffer_ = bufferlist.first();
	} else {
		// Set current buffer
		buffer_ = b;
	}

	// Reset old cursor
	cursor_ = LCursor(*bv_);
	anchor_ref_ = 0;
	offset_ref_ = 0;

	if (buffer_) {
		lyxerr[Debug::INFO] << BOOST_CURRENT_FUNCTION
				    << "Buffer addr: " << buffer_ << endl;
		cursor_.push(buffer_->inset());
		cursor_.resetAnchor();
		buffer_->text().init(bv_);
		buffer_->text().setCurrentFont(cursor_);
		if (buffer_->getCursor().size() > 0 &&
		    buffer_->getAnchor().size() > 0)
		{
			cursor_.setCursor(buffer_->getAnchor().asDocIterator(&(buffer_->inset())));
			cursor_.resetAnchor();
			cursor_.setCursor(buffer_->getCursor().asDocIterator(&(buffer_->inset())));
			cursor_.setSelection();
		}
	}

	update();

	if (buffer_ && lyx::graphics::Previews::status() != LyXRC::PREVIEW_OFF)
		lyx::graphics::Previews::get().generateBufferPreviews(*buffer_);
}


void BufferView::Pimpl::resizeCurrentBuffer()
{
	lyxerr[Debug::DEBUG] << BOOST_CURRENT_FUNCTION << endl;

	LyXText * text = bv_->text();
	if (!text)
		return;

	text->init(bv_);
	update();
	switchKeyMap();
}


void BufferView::Pimpl::updateScrollbar()
{
	if (!bv_->text()) {
		lyxerr[Debug::DEBUG] << BOOST_CURRENT_FUNCTION
				     << " no text in updateScrollbar" << endl;
		scrollbarParameters_.reset();
		return;
	}

	LyXText & t = *bv_->text();
	int const parsize = int(t.paragraphs().size() - 1);
	if (anchor_ref_ >  parsize)  {
		anchor_ref_ = parsize;
		offset_ref_ = 0;
	}

	lyxerr[Debug::GUI]
		<< BOOST_CURRENT_FUNCTION
		<< " Updating scrollbar: height: " << t.paragraphs().size()
		<< " curr par: " << cursor_.bottom().pit()
		<< " default height " << defaultRowHeight() << endl;

	// It would be better to fix the scrollbar to understand
	// values in [0..1] and divide everything by wh

	// estimated average paragraph height:
	if (wh_ == 0)
		wh_ = height_ / 4;
	int h = t.getPar(anchor_ref_).height();

	// Normalize anchor/offset (MV):
	while (offset_ref_ > h && anchor_ref_ < parsize) {
		anchor_ref_++;
		offset_ref_ -= h;
		h = t.getPar(anchor_ref_).height();
	}
	// Look at paragraph heights on-screen
	int sumh = 0;
	int nh = 0;
	for (lyx::pit_type pit = anchor_ref_; pit <= parsize; ++pit) {
		if (sumh > height_)
			break;
		int const h2 = t.getPar(pit).height();
		sumh += h2;
		nh++;
	}
	int const hav = sumh / nh;
	// More realistic average paragraph height
	if (hav > wh_)
		wh_ = hav;

	scrollbarParameters_.height = (parsize + 1) * wh_;
	scrollbarParameters_.position = anchor_ref_ * wh_ + int(offset_ref_ * wh_ / float(h));
	scrollbarParameters_.lineScrollHeight = int(wh_ * defaultRowHeight() / float(h));
}


ScrollbarParameters const & BufferView::Pimpl::scrollbarParameters() const
{
	return scrollbarParameters_;
}


void BufferView::Pimpl::scrollDocView(int value)
{
	lyxerr[Debug::GUI] << BOOST_CURRENT_FUNCTION
			   << "[ value = " << value << "]" << endl;

	if (!buffer_)
		return;

	LyXText & t = *bv_->text();

	float const bar = value / float(wh_ * t.paragraphs().size());

	anchor_ref_ = int(bar * t.paragraphs().size());
	if (anchor_ref_ >  int(t.paragraphs().size()) - 1)
		anchor_ref_ = int(t.paragraphs().size()) - 1;
	t.redoParagraph(anchor_ref_);
	int const h = t.getPar(anchor_ref_).height();
	offset_ref_ = int((bar * t.paragraphs().size() - anchor_ref_) * h);
}


void BufferView::Pimpl::setCursorFromScrollbar()
{
	LyXText & t = *bv_->text();

	int const height = 2 * defaultRowHeight();
	int const first = height;
	int const last = height_ - height;
	LCursor & cur = cursor_;

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
		int const y = bv_funcs::getPos(cur, cur.boundary()).y_;
		int const newy = min(last, max(y, first));
		if (y != newy) {
			cur.reset(buffer_->inset());
			t.setCursorFromCoordinates(cur, 0, newy);
		}
	}
}


void BufferView::Pimpl::scroll(int /*lines*/)
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
//	workArea_->setScrollbarParams(t->height(), top_y(), defaultRowHeight());}
}


void BufferView::Pimpl::selectionRequested()
{
	static string sel;

	if (!available())
		return;

	LCursor & cur = cursor_;

	if (!cur.selection()) {
		xsel_cache_.set = false;
		return;
	}

	if (!xsel_cache_.set ||
	    cur.top() != xsel_cache_.cursor ||
	    cur.anchor_.top() != xsel_cache_.anchor)
	{
		xsel_cache_.cursor = cur.top();
		xsel_cache_.anchor = cur.anchor_.top();
		xsel_cache_.set = cur.selection();
		sel = cur.selectionAsString(false);
		if (!sel.empty())
			owner_->gui().selection().put(sel);
	}
}


void BufferView::Pimpl::selectionLost()
{
	if (available()) {
		cursor_.clearSelection();
		xsel_cache_.set = false;
	}
}


void BufferView::Pimpl::workAreaResize(int width, int height)
{
	bool const widthChange = width != width_;
	bool const heightChange = height != height_;

	// Update from work area
	width_ = width;
	height_ = height;

	if (buffer_ && widthChange) {
		// The WorkArea content needs a resize
		resizeCurrentBuffer();
	}

	if (widthChange || heightChange)
		update();
}


bool BufferView::Pimpl::fitCursor()
{
	if (bv_funcs::status(bv_, cursor_) == bv_funcs::CUR_INSIDE) {
		LyXFont const font = cursor_.getFont();
		int const asc = font_metrics::maxAscent(font);
		int const des = font_metrics::maxDescent(font);
		Point const p = bv_funcs::getPos(cursor_, cursor_.boundary());
		if (p.y_ - asc >= 0 && p.y_ + des < height_)
			return false;
	}
	center();
	return true;
}


bool BufferView::Pimpl::multiParSel()
{
	if (!cursor_.selection())
		return false;
	bool ret = multiparsel_cache_;
	multiparsel_cache_ = cursor_.selBegin().pit() != cursor_.selEnd().pit();
	// Either this, or previous selection spans paragraphs
	return ret || multiparsel_cache_;
}


ViewMetricsInfo const & BufferView::Pimpl::viewMetricsInfo()
{
	return metrics_info_;
}


bool BufferView::Pimpl::update(Update::flags flags)
{
	// This is close to a hot-path.
	if (lyxerr.debugging(Debug::DEBUG)) {
		lyxerr[Debug::DEBUG]
			<< BOOST_CURRENT_FUNCTION
			<< "[fitcursor = " << (flags & Update::FitCursor)
			<< ", forceupdate = " << (flags & Update::Force)
			<< ", singlepar = " << (flags & Update::SinglePar)
			<< "]  buffer: " << buffer_ << endl;
	}

	// Check needed to survive LyX startup
	if (!buffer_)
		return false;

	// Update macro store
	buffer_->buildMacros();

	// First drawing step
	updateMetrics(flags & Update::SinglePar);

	// The second drawing step is done in WorkArea::redraw() if needed.
	bool const need_second_step =
		(flags & (Update::Force | Update::FitCursor | Update::MultiParSel))
		&& (fitCursor() || multiParSel());

	return need_second_step;
}


bool BufferView::Pimpl::available() const
{
	return buffer_ && bv_->text();
}


Change const BufferView::Pimpl::getCurrentChange()
{
	if (!buffer_->params().tracking_changes)
		return Change(Change::UNCHANGED);

	LyXText * text = bv_->getLyXText();
	LCursor & cur = cursor_;

	if (!cur.selection())
		return Change(Change::UNCHANGED);

	return text->getPar(cur.selBegin().pit()).
			lookupChange(cur.selBegin().pos());
}


void BufferView::Pimpl::savePosition(unsigned int i)
{
	if (i >= saved_positions_num)
		return;
	BOOST_ASSERT(cursor_.inTexted());
	saved_positions[i] = Position(buffer_->fileName(),
				      cursor_.paragraph().id(),
				      cursor_.pos());
	if (i > 0)
		// emit message signal.
		bv_->message(bformat(_("Saved bookmark %1$d"), i));
}


void BufferView::Pimpl::restorePosition(unsigned int i)
{
	if (i >= saved_positions_num)
		return;

	string const fname = saved_positions[i].filename;

	cursor_.clearSelection();

	if (fname != buffer_->fileName()) {
		Buffer * b = 0;
		if (bufferlist.exists(fname))
			b = bufferlist.getBuffer(fname);
		else {
			b = bufferlist.newBuffer(fname);
			// Don't ask, just load it
			::loadLyXFile(b, fname);
		}
		if (b)
			setBuffer(b);
	}

	ParIterator par = buffer_->getParFromID(saved_positions[i].par_id);
	if (par == buffer_->par_iterator_end())
		return;

	bv_->setCursor(makeDocIterator(par, min(par->size(), saved_positions[i].par_pos)));

	if (i > 0)
		// emit message signal.
		bv_->message(bformat(_("Moved to bookmark %1$d"), i));
}


bool BufferView::Pimpl::isSavedPosition(unsigned int i)
{
	return i < saved_positions_num && !saved_positions[i].filename.empty();
}


void BufferView::Pimpl::saveSavedPositions()
{
	// save bookmarks. It is better to use the pit interface
	// but I do not know how to effectively convert between
	// par_id and pit.
	for (unsigned int i=1; i < saved_positions_num; ++i) {
		if ( isSavedPosition(i) )
			LyX::ref().session().saveBookmark( boost::tie(
				i,
				saved_positions[i].filename,
				saved_positions[i].par_id,
				saved_positions[i].par_pos) );
	}
}


void BufferView::Pimpl::switchKeyMap()
{
	if (!lyxrc.rtl_support)
		return;

	Intl & intl = owner_->getIntl();
	if (bv_->getLyXText()->real_current_font.isRightToLeft()) {
		if (intl.keymap == Intl::PRIMARY)
			intl.keyMapSec();
	} else {
		if (intl.keymap == Intl::SECONDARY)
			intl.keyMapPrim();
	}
}


void BufferView::Pimpl::center()
{
	CursorSlice & bot = cursor_.bottom();
	lyx::pit_type const pit = bot.pit();
	bot.text()->redoParagraph(pit);
	Paragraph const & par = bot.text()->paragraphs()[pit];
	anchor_ref_ = pit;
	offset_ref_ = bv_funcs::coordOffset(cursor_, cursor_.boundary()).y_
		+ par.ascent() - height_ / 2;
}


void BufferView::Pimpl::menuInsertLyXFile(string const & filenm)
{
	BOOST_ASSERT(cursor_.inTexted());
	string filename = filenm;

	if (filename.empty()) {
		// Launch a file browser
		string initpath = lyxrc.document_path;

		if (available()) {
			string const trypath = buffer_->filePath();
			// If directory is writeable, use this as default.
			if (isDirWriteable(trypath))
				initpath = trypath;
		}

		FileDialog fileDlg(_("Select LyX document to insert"),
			LFUN_FILE_INSERT,
			make_pair(string(_("Documents|#o#O")),
				  string(lyxrc.document_path)),
			make_pair(string(_("Examples|#E#e")),
				  string(addPath(package().system_support(), "examples"))));

		FileDialog::Result result =
			fileDlg.open(initpath,
				     FileFilterList(_("LyX Documents (*.lyx)")),
				     string());

		if (result.first == FileDialog::Later)
			return;

		filename = result.second;

		// check selected filename
		if (filename.empty()) {
			// emit message signal.
			bv_->message(_("Canceled."));
			return;
		}
	}

	// Get absolute path of file and add ".lyx"
	// to the filename if necessary
	filename = fileSearch(string(), filename, "lyx");

	string const disp_fn = makeDisplayPath(filename);
	// emit message signal.
	bv_->message(bformat(_("Inserting document %1$s..."), disp_fn));

	string res;
	Buffer buf("", false);
	if (::loadLyXFile(&buf, makeAbsPath(filename))) {
		ErrorList & el = buffer_->errorList("Parse");
		// Copy the inserted document error list into the current buffer one.
		el = buf.errorList("Parse");
		lyx::cap::pasteParagraphList(cursor_, buf.paragraphs(),
					     buf.params().textclass, el);
		res = _("Document %1$s inserted.");
	} else
		res = _("Could not insert document %1$s");

	// emit message signal.
	bv_->message(bformat(res, disp_fn));
	buffer_->errors("Parse");
	resizeCurrentBuffer();
}


void BufferView::Pimpl::trackChanges()
{
	bool const tracking = buffer_->params().tracking_changes;

	if (!tracking) {
		for_each(buffer_->par_iterator_begin(),
			 buffer_->par_iterator_end(),
			 bind(&Paragraph::trackChanges, _1, Change::UNCHANGED));
		buffer_->params().tracking_changes = true;

		// We cannot allow undos beyond the freeze point
		buffer_->undostack().clear();
	} else {
		cursor_.setCursor(doc_iterator_begin(buffer_->inset()));
		if (lyx::find::findNextChange(bv_)) {
			owner_->getDialogs().show("changes");
			return;
		}

		for_each(buffer_->par_iterator_begin(),
			 buffer_->par_iterator_end(),
			 mem_fun_ref(&Paragraph::untrackChanges));

		buffer_->params().tracking_changes = false;
	}

	buffer_->redostack().clear();
}


bool BufferView::Pimpl::workAreaDispatch(FuncRequest const & cmd0)
{
	//lyxerr << BOOST_CURRENT_FUNCTION << "[ cmd0 " << cmd0 << "]" << endl;

	// This is only called for mouse related events including
	// LFUN_FILE_OPEN generated by drag-and-drop.
	FuncRequest cmd = cmd0;

	if (!buffer_)
		return false;

	LCursor cur(*bv_);
	cur.push(buffer_->inset());
	cur.selection() = cursor_.selection();

	// Doesn't go through lyxfunc, so we need to update
	// the layout choice etc. ourselves

	// E.g. Qt mouse press when no buffer
	if (!available())
		return false;

	// Either the inset under the cursor or the
	// surrounding LyXText will handle this event.

	// Build temporary cursor.
	cmd.y = min(max(cmd.y, -1), height_);
	InsetBase * inset = bv_->text()->editXY(cur, cmd.x, cmd.y);
	//lyxerr << BOOST_CURRENT_FUNCTION
	//       << " * hit inset at tip: " << inset << endl;
	//lyxerr << BOOST_CURRENT_FUNCTION
	//       << " * created temp cursor:" << cur << endl;

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
		if (cur.result().update())
			update(Update::FitCursor | Update::Force);
		else
			update(Update::FitCursor | Update::MultiParSel);
	}

	return true;
}


FuncStatus BufferView::Pimpl::getStatus(FuncRequest const & cmd)
{
	FuncStatus flag;

	switch (cmd.action) {

	case LFUN_UNDO:
		flag.enabled(!buffer_->undostack().empty());
		break;
	case LFUN_REDO:
		flag.enabled(!buffer_->redostack().empty());
		break;
	case LFUN_FILE_INSERT:
	case LFUN_FILE_INSERT_ASCII_PARA:
	case LFUN_FILE_INSERT_ASCII:
	case LFUN_BOOKMARK_SAVE:
		// FIXME: Actually, these LFUNS should be moved to LyXText
		flag.enabled(cursor_.inTexted());
		break;
	case LFUN_FONT_STATE:
	case LFUN_LABEL_INSERT:
	case LFUN_PARAGRAPH_GOTO:
	// FIXME handle non-trivially
	case LFUN_OUTLINE_UP:
	case LFUN_OUTLINE_DOWN:
	case LFUN_OUTLINE_IN:
	case LFUN_OUTLINE_OUT:
	case LFUN_NOTE_NEXT:
	case LFUN_REFERENCE_NEXT:
	case LFUN_WORD_FIND:
	case LFUN_WORD_REPLACE:
	case LFUN_MARK_OFF:
	case LFUN_MARK_ON:
	case LFUN_MARK_TOGGLE:
	case LFUN_SCREEN_RECENTER:
	case LFUN_BIBTEX_DATABASE_ADD:
	case LFUN_BIBTEX_DATABASE_DEL:
	case LFUN_WORDS_COUNT:
	case LFUN_NEXT_INSET_TOGGLE:
		flag.enabled(true);
		break;

	case LFUN_LABEL_GOTO: {
		flag.enabled(!cmd.argument.empty()
		    || getInsetByCode<InsetRef>(cursor_, InsetBase::REF_CODE));
		break;
	}

	case LFUN_BOOKMARK_GOTO:
		flag.enabled(isSavedPosition(convert<unsigned int>(cmd.argument)));
		break;
	case LFUN_CHANGES_TRACK:
		flag.enabled(true);
		flag.setOnOff(buffer_->params().tracking_changes);
		break;

	case LFUN_CHANGES_OUTPUT: {
		OutputParams runparams;
		LaTeXFeatures features(*buffer_, buffer_->params(), runparams);
		flag.enabled(buffer_ && buffer_->params().tracking_changes
			&& features.isAvailable("dvipost"));
		flag.setOnOff(buffer_->params().output_changes);
		break;
	}

	case LFUN_CHANGES_MERGE:
	case LFUN_CHANGE_ACCEPT: // what about these two
	case LFUN_CHANGE_REJECT: // what about these two
	case LFUN_ALL_CHANGES_ACCEPT:
	case LFUN_ALL_CHANGES_REJECT:
		flag.enabled(buffer_ && buffer_->params().tracking_changes);
		break;

	case LFUN_BUFFER_TOGGLE_COMPRESSION: {
		flag.setOnOff(buffer_->params().compressed);
		break;
	}

	default:
		flag.enabled(false);
	}

	return flag;
}



bool BufferView::Pimpl::dispatch(FuncRequest const & cmd)
{
	//lyxerr << BOOST_CURRENT_FUNCTION
	//       << [ cmd = " << cmd << "]" << endl;

	// Make sure that the cached BufferView is correct.
	lyxerr[Debug::ACTION] << BOOST_CURRENT_FUNCTION
		<< " action[" << cmd.action << ']'
		<< " arg[" << cmd.argument << ']'
		<< " x[" << cmd.x << ']'
		<< " y[" << cmd.y << ']'
		<< " button[" << cmd.button() << ']'
		<< endl;

	LCursor & cur = cursor_;

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
		menuInsertLyXFile(cmd.argument);
		break;

	case LFUN_FILE_INSERT_ASCII_PARA:
		insertAsciiFile(bv_, cmd.argument, true);
		break;

	case LFUN_FILE_INSERT_ASCII:
		insertAsciiFile(bv_, cmd.argument, false);
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

	case LFUN_LABEL_GOTO: {
		string label = cmd.argument;
		if (label.empty()) {
			InsetRef * inset =
				getInsetByCode<InsetRef>(cursor_,
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

	case LFUN_PARAGRAPH_GOTO: {
		int const id = convert<int>(cmd.argument);
		ParIterator par = buffer_->getParFromID(id);
		if (par == buffer_->par_iterator_end()) {
			lyxerr[Debug::INFO] << "No matching paragraph found! ["
					    << id << ']' << endl;
			break;
		} else {
			lyxerr[Debug::INFO] << "Paragraph " << par->id()
					    << " found." << endl;
		}

		// Set the cursor
		bv_->setCursor(makeDocIterator(par, 0));

		update();
		switchKeyMap();
		break;
	}

	case LFUN_OUTLINE_UP:
		lyx::toc::outline(lyx::toc::Up, cursor_);
		cursor_.text()->setCursor(cursor_, cursor_.pit(), 0);
		updateLabels(*buffer_);
		break;
	case LFUN_OUTLINE_DOWN:
		lyx::toc::outline(lyx::toc::Down, cursor_);
		cursor_.text()->setCursor(cursor_, cursor_.pit(), 0);
		updateLabels(*buffer_);
		break;
	case LFUN_OUTLINE_IN:
		lyx::toc::outline(lyx::toc::In, cursor_);
		updateLabels(*buffer_);
		break;
	case LFUN_OUTLINE_OUT:
		lyx::toc::outline(lyx::toc::Out, cursor_);
		updateLabels(*buffer_);
		break;

	case LFUN_NOTE_NEXT:
		bv_funcs::gotoInset(bv_, InsetBase::NOTE_CODE, false);
		break;

	case LFUN_REFERENCE_NEXT: {
		vector<InsetBase_code> tmp;
		tmp.push_back(InsetBase::LABEL_CODE);
		tmp.push_back(InsetBase::REF_CODE);
		bv_funcs::gotoInset(bv_, tmp, true);
		break;
	}

	case LFUN_CHANGES_TRACK:
		trackChanges();
		break;

	case LFUN_CHANGES_OUTPUT: {
		bool const state = buffer_->params().output_changes;
		buffer_->params().output_changes = !state;
		break;
	}

	case LFUN_CHANGES_MERGE:
		if (lyx::find::findNextChange(bv_))
			owner_->getDialogs().show("changes");
		break;

	case LFUN_ALL_CHANGES_ACCEPT: {
		cursor_.reset(buffer_->inset());
#ifdef WITH_WARNINGS
#warning FIXME changes
#endif
		while (lyx::find::findNextChange(bv_))
			bv_->getLyXText()->acceptChange(cursor_);
		update();
		break;
	}

	case LFUN_ALL_CHANGES_REJECT: {
		cursor_.reset(buffer_->inset());
#ifdef WITH_WARNINGS
#warning FIXME changes
#endif
		while (lyx::find::findNextChange(bv_))
			bv_->getLyXText()->rejectChange(cursor_);
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

	case LFUN_MARK_TOGGLE:
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

	case LFUN_SCREEN_RECENTER:
		center();
		break;

	case LFUN_BIBTEX_DATABASE_ADD: {
		LCursor tmpcur = cursor_;
		bv_funcs::findInset(tmpcur, InsetBase::BIBTEX_CODE, false);
		InsetBibtex * inset = getInsetByCode<InsetBibtex>(tmpcur,
						InsetBase::BIBTEX_CODE);
		if (inset) {
			if (inset->addDatabase(cmd.argument))
				buffer_->updateBibfilesCache();
		}
		break;
	}

	case LFUN_BIBTEX_DATABASE_DEL: {
		LCursor tmpcur = cursor_;
		bv_funcs::findInset(tmpcur, InsetBase::BIBTEX_CODE, false);
		InsetBibtex * inset = getInsetByCode<InsetBibtex>(tmpcur,
						InsetBase::BIBTEX_CODE);
		if (inset) {
			if (inset->delDatabase(cmd.argument))
				buffer_->updateBibfilesCache();
		}
		break;
	}

	case LFUN_WORDS_COUNT: {
		DocIterator from, to;
		if (cur.selection()) {
			from = cur.selectionBegin();
			to = cur.selectionEnd();
		} else {
			from = doc_iterator_begin(buffer_->inset());
			to = doc_iterator_end(buffer_->inset());
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

	case LFUN_BUFFER_TOGGLE_COMPRESSION:
		// turn compression on/off
		buffer_->params().compressed = !buffer_->params().compressed;
		break;

	case LFUN_NEXT_INSET_TOGGLE: {
		// this is the real function we want to invoke
		FuncRequest tmpcmd = FuncRequest(LFUN_INSET_TOGGLE, cmd.origin);
		// if there is an inset at cursor, see whether it
		// wants to toggle.
		InsetBase * inset = cur.nextInset();
		if (inset && inset->isActive()) {
			LCursor tmpcur = cur;
			tmpcur.pushLeft(*inset);
			inset->dispatch(tmpcur, tmpcmd);
			if (tmpcur.result().dispatched()) {
				cur.dispatched();
			}
		}
		// if it did not work, try the underlying inset.
		if (!cur.result().dispatched())
			cur.dispatch(tmpcmd);

		if (cur.result().dispatched()) 
			cur.clearSelection();
		
		break;
	}

	default:
		return false;
	}

	return true;
}


void BufferView::Pimpl::updateMetrics(bool singlepar)
{
	// Remove old position cache
	theCoords.clear();
	BufferView & bv = *bv_;
	LyXText * const text = bv.text();
	lyx::pit_type size = int(text->paragraphs().size());

	if (anchor_ref_ > int(text->paragraphs().size() - 1)) {
		anchor_ref_ = int(text->paragraphs().size() - 1);
		offset_ref_ = 0;
	}

	lyx::pit_type const pit = anchor_ref_;
	int pit1 = pit;
	int pit2 = pit;
	size_t const npit = text->paragraphs().size();

	// Rebreak anchor paragraph. In Single Paragraph mode, rebreak only
	// the (main text, not inset!) paragraph containing the cursor.
	// (if this paragraph contains insets etc., rebreaking will
	// recursively descend)
	if (!singlepar || pit == cursor_.bottom().pit())
		text->redoParagraph(pit);
	int y0 = text->getPar(pit).ascent() - offset_ref_;

	// Redo paragraphs above anchor if necessary; again, in Single Par
	// mode, only if we encounter the (main text) one having the cursor.
	int y1 = y0;
	while (y1 > 0 && pit1 > 0) {
		y1 -= text->getPar(pit1).ascent();
		--pit1;
		if (!singlepar || pit1 == cursor_.bottom().pit())
			text->redoParagraph(pit1);
		y1 -= text->getPar(pit1).descent();
	}


	// Take care of ascent of first line
	y1 -= text->getPar(pit1).ascent();

	// Normalize anchor for next time
	anchor_ref_ = pit1;
	offset_ref_ = -y1;

	// Grey at the beginning is ugly
	if (pit1 == 0 && y1 > 0) {
		y0 -= y1;
		y1 = 0;
		anchor_ref_ = 0;
	}

	// Redo paragraphs below the anchor if necessary. Single par mode:
	// only the one containing the cursor if encountered.
	int y2 = y0;
	while (y2 < bv.workHeight() && pit2 < int(npit) - 1) {
		y2 += text->getPar(pit2).descent();
		++pit2;
		if (!singlepar || pit2 == cursor_.bottom().pit())
			text->redoParagraph(pit2);
		y2 += text->getPar(pit2).ascent();
	}

	// Take care of descent of last line
	y2 += text->getPar(pit2).descent();

	// The coordinates of all these paragraphs are correct, cache them
	int y = y1;
	CoordCache::InnerParPosCache & parPos = theCoords.parPos()[text];
	for (lyx::pit_type pit = pit1; pit <= pit2; ++pit) {
		Paragraph const & par = text->getPar(pit);
		y += par.ascent();
		parPos[pit] = Point(0, y);
		if (singlepar && pit == cursor_.bottom().pit()) {
			// In Single Paragraph mode, collect here the
			// y1 and y2 of the (one) paragraph the cursor is in
			y1 = y - par.ascent();
			y2 = y + par.descent();
		}
		y += par.descent();
	}

	if (singlepar) {
		// collect cursor paragraph iter bounds
		pit1 = cursor_.bottom().pit();
		pit2 = cursor_.bottom().pit();
	}

	lyxerr[Debug::DEBUG]
		<< BOOST_CURRENT_FUNCTION
		<< " y1: " << y1
		<< " y2: " << y2
		<< " pit1: " << pit1
		<< " pit2: " << pit2
		<< " npit: " << npit
		<< " singlepar: " << singlepar
		<< "size: " << size
		<< endl;

	metrics_info_ = ViewMetricsInfo(pit1, pit2, y1, y2, singlepar, size);
}
