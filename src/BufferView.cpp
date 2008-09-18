/**
 * \file BufferView.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author André Pönitz
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BufferView.h"

#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "bufferview_funcs.h"
#include "CoordCache.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "DispatchResult.h"
#include "ErrorList.h"
#include "factory.h"
#include "FloatList.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "Intl.h"
#include "InsetIterator.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "callback.h" // added for Dispatch functions
#include "LyX.h"
#include "lyxfind.h"
#include "LyXFunc.h"
#include "Layout.h"
#include "Text.h"
#include "TextClass.h"
#include "LyXRC.h"
#include "Session.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "TexRow.h"
#include "Undo.h"
#include "VSpace.h"
#include "WordLangTuple.h"
#include "MetricsInfo.h"

#include "insets/InsetBibtex.h"
#include "insets/InsetCommand.h" // ChangeRefs
#include "insets/InsetRef.h"
#include "insets/InsetText.h"

#include "frontends/alert.h"
#include "frontends/FileDialog.h"
#include "frontends/FontMetrics.h"
#include "frontends/Selection.h"

#include "graphics/Previews.h"

#include "support/convert.h"
#include "support/FileFilterList.h"
#include "support/filetools.h"
#include "support/Package.h"
#include "support/types.h"

#include <boost/bind.hpp>
#include <boost/current_function.hpp>

#include <functional>
#include <vector>

using std::distance;
using std::endl;
using std::istringstream;
using std::make_pair;
using std::min;
using std::max;
using std::mem_fun_ref;
using std::string;
using std::vector;


namespace lyx {

using support::addPath;
using support::bformat;
using support::FileFilterList;
using support::FileName;
using support::fileSearch;
using support::isDirWriteable;
using support::isFileReadable;
using support::makeDisplayPath;
using support::package;

namespace Alert = frontend::Alert;

namespace {

/// Return an inset of this class if it exists at the current cursor position
template <class T>
T * getInsetByCode(Cursor & cur, Inset::Code code)
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


BufferView::BufferView()
	: width_(0), height_(0), buffer_(0), wh_(0),
	  cursor_(*this),
	  multiparsel_cache_(false), anchor_ref_(0), offset_ref_(0),
	  need_centering_(false), intl_(new Intl), last_inset_(0)
{
	xsel_cache_.set = false;
	intl_->initKeyMapper(lyxrc.use_kbmap);
}


BufferView::~BufferView()
{
}


Buffer * BufferView::buffer() const
{
	return buffer_;
}


Buffer * BufferView::setBuffer(Buffer * b)
{
	LYXERR(Debug::INFO) << BOOST_CURRENT_FUNCTION
			    << "[ b = " << b << "]" << endl;

	if (buffer_) {
		// Save the current selection if any
		cap::saveSelection(cursor_);
		// Save the actual cursor position and anchor inside the
		// buffer so that it can be restored in case we rechange
		// to this buffer later on.
		buffer_->saveCursor(cursor_.selectionBegin(),
				    cursor_.selectionEnd());
		// update bookmark pit of the current buffer before switch
		for (size_t i = 0; i < LyX::ref().session().bookmarks().size(); ++i) {
			BookmarksSection::Bookmark const & bm = LyX::ref().session().bookmarks().bookmark(i);
			if (buffer()->fileName() != bm.filename.absFilename())
				continue;
			// if top_id or bottom_pit, bottom_pos has been changed, update bookmark
			// see http://bugzilla.lyx.org/show_bug.cgi?id=3092
			pit_type new_pit;
			pos_type new_pos;
			int new_id;
			boost::tie(new_pit, new_pos, new_id) = moveToPosition(bm.bottom_pit, bm.bottom_pos, bm.top_id, bm.top_pos);
			if (bm.bottom_pit != new_pit || bm.bottom_pos != new_pos || bm.top_id != new_id )
				const_cast<BookmarksSection::Bookmark &>(bm).updatePos(new_pit, new_pos, new_id);
		}
		// current buffer is going to be switched-off, save cursor pos
		// Ideally, the whole cursor stack should be saved, but session
		// currently can only handle bottom (whole document) level pit and pos.
		// That is to say, if a cursor is in a nested inset, it will be
		// restore to the left of the top level inset.
		LyX::ref().session().lastFilePos().save(FileName(buffer_->fileName()),
			boost::tie(cursor_.bottom().pit(), cursor_.bottom().pos()) );
	}

	// If we're quitting lyx, don't bother updating stuff
	if (quitting) {
		buffer_ = 0;
		return 0;
	}

	//FIXME Fix for bug 3440 is here.
	// If we are closing current buffer, switch to the first in
	// buffer list.
	if (!b) {
		LYXERR(Debug::INFO) << BOOST_CURRENT_FUNCTION
				    << " No Buffer!" << endl;
		// We are closing the buffer, use the first buffer as current
		//FIXME 3440
		// if (last_buffer_) buffer_ = last_buffer_;
		// also check that this is in theBufferList()?
		buffer_ = theBufferList().first();
	} else {
		//FIXME 3440
		// last_buffer = buffer_;
		// Set current buffer
		buffer_ = b;
	}

	// Reset old cursor
	cursor_ = Cursor(*this);
	anchor_ref_ = 0;
	offset_ref_ = 0;

	if (!buffer_)
		return 0;

	LYXERR(Debug::INFO) << BOOST_CURRENT_FUNCTION
					<< "Buffer addr: " << buffer_ << endl;
	cursor_.push(buffer_->inset());
	cursor_.resetAnchor();
	buffer_->text().setCurrentFont(cursor_);

	// Update the metrics now that we have a proper Cursor.
	updateMetrics(false);

	// FIXME: This code won't be needed once we switch to
	// "one Buffer" / "one BufferView".
	if (buffer_->getCursor().size() > 0 &&
			buffer_->getAnchor().size() > 0)
	{
		cursor_.setCursor(buffer_->getAnchor().asDocIterator(&(buffer_->inset())));
		cursor_.resetAnchor();
		cursor_.setCursor(buffer_->getCursor().asDocIterator(&(buffer_->inset())));
		cursor_.setSelection();
		// do not set selection to the new buffer because we
		// only paste recent selection.

		// Make sure that the restored cursor is not broken. This can happen for
		// example if this Buffer has been modified by another view.
		cursor_.fixIfBroken();

		if (fitCursor())
			// Update the metrics if the cursor new position was off screen.
			updateMetrics(false);
	}

	if (graphics::Previews::status() != LyXRC::PREVIEW_OFF)
		graphics::Previews::get().generateBufferPreviews(*buffer_);
	return buffer_;
}


void BufferView::resize()
{
	if (!buffer_)
		return;

	LYXERR(Debug::DEBUG) << BOOST_CURRENT_FUNCTION << endl;

	updateMetrics(false);
}


bool BufferView::fitCursor()
{
	if (bv_funcs::status(this, cursor_) == bv_funcs::CUR_INSIDE) {
		frontend::FontMetrics const & fm =
			theFontMetrics(cursor_.getFont());
		int const asc = fm.maxAscent();
		int const des = fm.maxDescent();
		Point const p = bv_funcs::getPos(*this, cursor_, cursor_.boundary());
		if (p.y_ - asc >= 0 && p.y_ + des < height_)
			return false;
	}
	center();
	return true;
}


bool BufferView::multiParSel()
{
	if (!cursor_.selection())
		return false;
	bool ret = multiparsel_cache_;
	multiparsel_cache_ = cursor_.selBegin().pit() != cursor_.selEnd().pit();
	// Either this, or previous selection spans paragraphs
	return ret || multiparsel_cache_;
}


bool BufferView::update(Update::flags flags)
{
	// last_inset_ points to the last visited inset. This pointer may become
	// invalid because of keyboard editing. Since all such operations
	// causes screen update(), I reset last_inset_ to avoid such a problem.
	last_inset_ = 0;
	// This is close to a hot-path.
	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< "[fitcursor = " << (flags & Update::FitCursor)
		<< ", forceupdate = " << (flags & Update::Force)
		<< ", singlepar = " << (flags & Update::SinglePar)
		<< "]  buffer: " << buffer_ << endl;

	// Check needed to survive LyX startup
	if (!buffer_)
		return false;

	LYXERR(Debug::WORKAREA) << "BufferView::update" << std::endl;

	// Update macro store
	if (!(cursor().inMathed() && cursor().inMacroMode()))
		buffer_->buildMacros();

	// Now do the first drawing step if needed. This consists on updating
	// the CoordCache in updateMetrics().
	// The second drawing step is done in WorkArea::redraw() if needed.

	// Case when no explicit update is requested.
	if (!flags) {
		// no need to redraw anything.
		metrics_info_.update_strategy = NoScreenUpdate;
		return false;
	}

	if (flags == Update::Decoration) {
		metrics_info_.update_strategy = DecorationUpdate;
		return true;
	}

	if (flags == Update::FitCursor
		|| flags == (Update::Decoration | Update::FitCursor)) {
		bool const fit_cursor = fitCursor();
		// tell the frontend to update the screen if needed.
		if (fit_cursor) {
			updateMetrics(false);
			return true;
		}
		if (flags & Update::Decoration) {
			metrics_info_.update_strategy = DecorationUpdate;
			return true;
		}
		// no screen update is needed.
		metrics_info_.update_strategy = NoScreenUpdate;
		return false;
	}

	bool full_metrics = flags & Update::Force;
	if (flags & Update::MultiParSel)
		full_metrics |= multiParSel();

	bool const single_par = !full_metrics;
	updateMetrics(single_par);

	if (flags & Update::FitCursor && fitCursor())
		updateMetrics(false);

	// tell the frontend to update the screen.
	return true;
}


void BufferView::updateScrollbar()
{
	if (!buffer_) {
		LYXERR(Debug::DEBUG) << BOOST_CURRENT_FUNCTION
				     << " no text in updateScrollbar" << endl;
		scrollbarParameters_.reset();
		return;
	}

	Text & t = buffer_->text();
	TextMetrics & tm = text_metrics_[&t];

	int const parsize = int(t.paragraphs().size() - 1);
	if (anchor_ref_ >  parsize)  {
		anchor_ref_ = parsize;
		offset_ref_ = 0;
	}

	LYXERR(Debug::GUI)
		<< BOOST_CURRENT_FUNCTION
		<< " Updating scrollbar: height: " << t.paragraphs().size()
		<< " curr par: " << cursor_.bottom().pit()
		<< " default height " << defaultRowHeight() << endl;

	// It would be better to fix the scrollbar to understand
	// values in [0..1] and divide everything by wh

	// estimated average paragraph height:
	if (wh_ == 0)
		wh_ = height_ / 4;

	int h = tm.parMetrics(anchor_ref_).height();

	// Normalize anchor/offset (MV):
	while (offset_ref_ > h && anchor_ref_ < parsize) {
		anchor_ref_++;
		offset_ref_ -= h;
		h = tm.parMetrics(anchor_ref_).height();
	}
	// Look at paragraph heights on-screen
	int sumh = 0;
	int nh = 0;
	for (pit_type pit = anchor_ref_; pit <= parsize; ++pit) {
		if (sumh > height_)
			break;
		int const h2 = tm.parMetrics(pit).height();
		sumh += h2;
		nh++;
	}

	BOOST_ASSERT(nh);
	int const hav = sumh / nh;
	// More realistic average paragraph height
	if (hav > wh_)
		wh_ = hav;

	BOOST_ASSERT(h);
	scrollbarParameters_.height = (parsize + 1) * wh_;
	scrollbarParameters_.position = anchor_ref_ * wh_ + int(offset_ref_ * wh_ / float(h));
	scrollbarParameters_.lineScrollHeight = int(wh_ * defaultRowHeight() / float(h));
}


ScrollbarParameters const & BufferView::scrollbarParameters() const
{
	return scrollbarParameters_;
}


void BufferView::scrollDocView(int value)
{
	LYXERR(Debug::GUI) << BOOST_CURRENT_FUNCTION
			   << "[ value = " << value << "]" << endl;

	if (!buffer_)
		return;

	Text & t = buffer_->text();
	TextMetrics & tm = text_metrics_[&t];

	float const bar = value / float(wh_ * t.paragraphs().size());

	anchor_ref_ = int(bar * t.paragraphs().size());
	if (anchor_ref_ >  int(t.paragraphs().size()) - 1)
		anchor_ref_ = int(t.paragraphs().size()) - 1;

	tm.redoParagraph(anchor_ref_);
	int const h = tm.parMetrics(anchor_ref_).height();
	offset_ref_ = int((bar * t.paragraphs().size() - anchor_ref_) * h);
	updateMetrics(false);
}


void BufferView::setCursorFromScrollbar()
{
	if (!buffer_)
		return;

	Text & t = buffer_->text();

	int const height = 2 * defaultRowHeight();
	int const first = height;
	int const last = height_ - height;
	int newy = 0;
	Cursor const & oldcur = cursor_;
	
	switch (bv_funcs::status(this, oldcur)) {
	case bv_funcs::CUR_ABOVE:
		newy = first;
		break;
	case bv_funcs::CUR_BELOW:
		newy = last;
		break;
	case bv_funcs::CUR_INSIDE:
		int const y = bv_funcs::getPos(*this, 
					       oldcur, oldcur.boundary()).y_;
		newy = min(last, max(y, first));
		if (y == newy)
			return;
	}
	// We reset the cursor because cursorStatus() does not
	// work when the cursor is within mathed.
	Cursor cur(*this);
	cur.reset(buffer_->inset());
	t.setCursorFromCoordinates(cur, 0, newy);
	mouseSetCursor(cur);
 }


Change const BufferView::getCurrentChange() const
{
	if (!cursor_.selection())
		return Change(Change::UNCHANGED);

	DocIterator dit = cursor_.selectionBegin();
	return dit.paragraph().lookupChange(dit.pos());
}


void BufferView::saveBookmark(unsigned int idx)
{
	// tenatively save bookmark, id and pos will be used to
	// acturately locate a bookmark in a 'live' lyx session.
	// pit and pos will be updated with bottom level pit/pos
	// when lyx exits.
	LyX::ref().session().bookmarks().save(
		FileName(buffer_->fileName()),
		cursor_.bottom().pit(),
		cursor_.bottom().pos(),
		cursor_.paragraph().id(),
		cursor_.pos(),
		idx
	);
	if (idx)
		// emit message signal.
		message(_("Save bookmark"));
}


boost::tuple<pit_type, pos_type, int> BufferView::moveToPosition(pit_type bottom_pit, pos_type bottom_pos,
	int top_id, pos_type top_pos)
{
	cursor_.clearSelection();

	// if a valid par_id is given, try it first
	// This is the case for a 'live' bookmark when unique paragraph ID
	// is used to track bookmarks.
	if (top_id > 0) {
		ParIterator par = buffer_->getParFromID(top_id);
		if (par != buffer_->par_iterator_end()) {
			DocIterator dit = makeDocIterator(par, min(par->size(), top_pos));
			// Some slices of the iterator may not be
			// reachable (e.g. closed collapsable inset)
			// so the dociterator may need to be
			// shortened. Otherwise, setCursor may crash
			// lyx when the cursor can not be set to these
			// insets.
			size_t const n = dit.depth();
			for (size_t i = 0; i < n; ++i)
				if (dit[i].inset().editable() != Inset::HIGHLY_EDITABLE) {
					dit.resize(i);
					break;
				}
			setCursor(dit);
			cursor_.text()->setCurrentFont(cursor_);
			// Note: return bottom (document) level pit.
			return boost::make_tuple(cursor_.bottom().pit(), cursor_.bottom().pos(), top_id);
		}
	}
	// if top_id == 0, or searching through top_id failed
	// This is the case for a 'restored' bookmark when only bottom
	// (document level) pit was saved. Because of this, bookmark
	// restoration is inaccurate. If a bookmark was within an inset,
	// it will be restored to the left of the outmost inset that contains
	// the bookmark.
	if (static_cast<size_t>(bottom_pit) < buffer_->paragraphs().size()) {
		DocIterator it = doc_iterator_begin(buffer_->inset());
		it.pit() = bottom_pit;
		it.pos() = min(bottom_pos, it.paragraph().size());
		setCursor(it);
		cursor_.text()->setCurrentFont(cursor_);
		return boost::make_tuple(it.pit(), it.pos(),
					 it.paragraph().id());
	}
	// both methods fail
	return boost::make_tuple(pit_type(0), pos_type(0), 0);
}


void BufferView::translateAndInsert(char_type c, Text * t, Cursor & cur)
{
	if (lyxrc.rtl_support) {
		if (cursor_.innerText()->real_current_font.isRightToLeft()) {
			if (intl_->keymap == Intl::PRIMARY)
				intl_->keyMapSec();
		} else {
			if (intl_->keymap == Intl::SECONDARY)
				intl_->keyMapPrim();
		}
	}
	
	intl_->getTransManager().translateAndInsert(c, t, cur);
}


int BufferView::workWidth() const
{
	return width_;
}


void BufferView::updateOffsetRef()
{
	// No need to update offset_ref_ in this case.
	if (!need_centering_)
		return;

	// We are not properly started yet, delay until resizing is
	// done.
	if (height_ == 0)
		return;

	CursorSlice & bot = cursor_.bottom();
	TextMetrics & tm = text_metrics_[bot.text()];
	ParagraphMetrics const & pm = tm.parMetrics(bot.pit());
	Point p = bv_funcs::coordOffset(*this, cursor_, cursor_.boundary());
	offset_ref_ = p.y_ + pm.ascent() - height_ / 2;

	need_centering_ = false;
}


void BufferView::center()
{
	anchor_ref_ = cursor_.bottom().pit();
	need_centering_ = true;
}


FuncStatus BufferView::getStatus(FuncRequest const & cmd)
{
	FuncStatus flag;

	Cursor & cur = cursor_;

	switch (cmd.action) {

	case LFUN_UNDO:
		flag.enabled(!buffer_->undostack().empty());
		break;
	case LFUN_REDO:
		flag.enabled(!buffer_->redostack().empty());
		break;
	case LFUN_FILE_INSERT:
	case LFUN_FILE_INSERT_PLAINTEXT_PARA:
	case LFUN_FILE_INSERT_PLAINTEXT:
	case LFUN_BOOKMARK_SAVE:
		// FIXME: Actually, these LFUNS should be moved to Text
		flag.enabled(cur.inTexted());
		break;
	case LFUN_FONT_STATE:
	case LFUN_LABEL_INSERT:
	case LFUN_PARAGRAPH_GOTO:
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
		flag.enabled(!cmd.argument().empty()
		    || getInsetByCode<InsetRef>(cur, Inset::REF_CODE));
		break;
	}

	case LFUN_CHANGES_TRACK:
		flag.enabled(true);
		flag.setOnOff(buffer_->params().trackChanges);
		break;

	case LFUN_CHANGES_OUTPUT:
		flag.enabled(buffer_);
		flag.setOnOff(buffer_->params().outputChanges);
		break;

	case LFUN_CHANGES_MERGE:
	case LFUN_CHANGE_NEXT:
	case LFUN_ALL_CHANGES_ACCEPT:
	case LFUN_ALL_CHANGES_REJECT:
		// TODO: context-sensitive enabling of LFUNs
		// In principle, these command should only be enabled if there
		// is a change in the document. However, without proper
		// optimizations, this will inevitably result in poor performance.
		flag.enabled(buffer_);
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


Update::flags BufferView::dispatch(FuncRequest const & cmd)
{
	//lyxerr << BOOST_CURRENT_FUNCTION
	//       << [ cmd = " << cmd << "]" << endl;

	// Make sure that the cached BufferView is correct.
	LYXERR(Debug::ACTION) << BOOST_CURRENT_FUNCTION
		<< " action[" << cmd.action << ']'
		<< " arg[" << to_utf8(cmd.argument()) << ']'
		<< " x[" << cmd.x << ']'
		<< " y[" << cmd.y << ']'
		<< " button[" << cmd.button() << ']'
		<< endl;

	// FIXME: this should not be possible.
	if (!buffer_)
		return Update::None;

	Cursor & cur = cursor_;
	// Default Update flags.
	Update::flags updateFlags = Update::Force | Update::FitCursor;

	switch (cmd.action) {

	case LFUN_UNDO:
		cur.message(_("Undo"));
		cur.clearSelection();
		if (!textUndo(*this)) {
			cur.message(_("No further undo information"));
			updateFlags = Update::None;
		}
		break;

	case LFUN_REDO:
		cur.message(_("Redo"));
		cur.clearSelection();
		if (!textRedo(*this)) {
			cur.message(_("No further redo information"));
			updateFlags = Update::None;
		}
		break;

	case LFUN_FILE_INSERT:
		// FIXME UNICODE
		menuInsertLyXFile(to_utf8(cmd.argument()));
		break;

	case LFUN_FILE_INSERT_PLAINTEXT_PARA:
		// FIXME UNICODE
		insertPlaintextFile(this, to_utf8(cmd.argument()), true);
		break;

	case LFUN_FILE_INSERT_PLAINTEXT:
		// FIXME UNICODE
		insertPlaintextFile(this, to_utf8(cmd.argument()), false);
		break;

	case LFUN_FONT_STATE:
		cur.message(cur.currentState());
		break;

	case LFUN_BOOKMARK_SAVE:
		saveBookmark(convert<unsigned int>(to_utf8(cmd.argument())));
		break;

	case LFUN_LABEL_GOTO: {
		docstring label = cmd.argument();
		if (label.empty()) {
			InsetRef * inset =
				getInsetByCode<InsetRef>(cursor_,
							 Inset::REF_CODE);
			if (inset) {
				label = inset->getParam("reference");
				// persistent=false: use temp_bookmark
				saveBookmark(0);
			}
		}

		if (!label.empty())
			gotoLabel(label);
		break;
	}

	case LFUN_PARAGRAPH_GOTO: {
		int const id = convert<int>(to_utf8(cmd.argument()));
		int i = 0;
		for (Buffer * b = buffer_; i == 0 || b != buffer_; b = theBufferList().next(b)) {
			ParIterator par = b->getParFromID(id);
			if (par == b->par_iterator_end()) {
				LYXERR(Debug::INFO)
					<< "No matching paragraph found! ["
					<< id << "]." << endl;
			} else {
				LYXERR(Debug::INFO)
					<< "Paragraph " << par->id()
					<< " found in buffer `"
					<< b->fileName() << "'." << endl;

				if (b == buffer_) {
					// Set the cursor
					setCursor(makeDocIterator(par, 0));
				} else {
					// Switch to other buffer view and resend cmd
					theLyXFunc().dispatch(FuncRequest(
						LFUN_BUFFER_SWITCH, b->fileName()));
					theLyXFunc().dispatch(cmd);
					updateFlags = Update::None;
				}
				break;
			}
			++i;
		}
		break;
	}

	case LFUN_NOTE_NEXT:
		bv_funcs::gotoInset(this, Inset::NOTE_CODE, false);
		break;

	case LFUN_REFERENCE_NEXT: {
		vector<Inset_code> tmp;
		tmp.push_back(Inset::LABEL_CODE);
		tmp.push_back(Inset::REF_CODE);
		bv_funcs::gotoInset(this, tmp, true);
		break;
	}

	case LFUN_CHANGES_TRACK:
		buffer_->params().trackChanges = !buffer_->params().trackChanges;
		break;

	case LFUN_CHANGES_OUTPUT:
		buffer_->params().outputChanges = !buffer_->params().outputChanges;
		if (buffer_->params().outputChanges) {
			bool dvipost    = LaTeXFeatures::isAvailable("dvipost");
			bool xcolorsoul = LaTeXFeatures::isAvailable("soul") &&
					  LaTeXFeatures::isAvailable("xcolor");

			if (!dvipost && !xcolorsoul) {
				Alert::warning(_("Changes not shown in LaTeX output"),
					       _("Changes will not be highlighted in LaTeX output, "
						 "because neither dvipost nor xcolor/soul are installed.\n"
						 "Please install these packages or redefine "
						 "\\lyxadded and \\lyxdeleted in the LaTeX preamble."));
			} else if (!xcolorsoul) {
				Alert::warning(_("Changes not shown in LaTeX output"),
					       _("Changes will not be highlighted in LaTeX output "
						 "when using pdflatex, because xcolor and soul are not installed.\n"
						 "Please install both packages or redefine "
						 "\\lyxadded and \\lyxdeleted in the LaTeX preamble."));
			}
		}
		break;

	case LFUN_CHANGE_NEXT:
		findNextChange(this);
		break;

	case LFUN_CHANGES_MERGE:
		if (findNextChange(this))
			showDialog("changes");
		break;

	case LFUN_ALL_CHANGES_ACCEPT:
		// select complete document
		cursor_.reset(buffer_->inset());
		cursor_.selHandle(true);
		buffer_->text().cursorBottom(cursor_);
		// accept everything in a single step to support atomic undo
		buffer_->text().acceptOrRejectChanges(cursor_, Text::ACCEPT);
		break;

	case LFUN_ALL_CHANGES_REJECT:
		// select complete document
		cursor_.reset(buffer_->inset());
		cursor_.selHandle(true);
		buffer_->text().cursorBottom(cursor_);
		// reject everything in a single step to support atomic undo
		// Note: reject does not work recursively; the user may have to repeat the operation
		buffer_->text().acceptOrRejectChanges(cursor_, Text::REJECT);
		break;

	case LFUN_WORD_FIND:
		find(this, cmd);
		break;

	case LFUN_WORD_REPLACE: {
		bool has_deleted = false;
		if (cur.selection()) {
			DocIterator beg = cur.selectionBegin();
			DocIterator end = cur.selectionEnd();
			if (beg.pit() == end.pit()) {
				for (pos_type p = beg.pos() ; p < end.pos() ; ++p) {
					if (cur.paragraph().isDeleted(p))
						has_deleted = true;
				}
			}
		}
		replace(this, cmd, has_deleted);
		break;
	}

	case LFUN_MARK_OFF:
		cur.clearSelection();
		cur.resetAnchor();
		cur.message(from_utf8(N_("Mark off")));
		break;

	case LFUN_MARK_ON:
		cur.clearSelection();
		cur.mark() = true;
		cur.resetAnchor();
		cur.message(from_utf8(N_("Mark on")));
		break;

	case LFUN_MARK_TOGGLE:
		cur.clearSelection();
		if (cur.mark()) {
			cur.mark() = false;
			cur.message(from_utf8(N_("Mark removed")));
		} else {
			cur.mark() = true;
			cur.message(from_utf8(N_("Mark set")));
		}
		cur.resetAnchor();
		break;

	case LFUN_SCREEN_RECENTER:
		center();
		break;

	case LFUN_BIBTEX_DATABASE_ADD: {
		Cursor tmpcur = cursor_;
		bv_funcs::findInset(tmpcur, Inset::BIBTEX_CODE, false);
		InsetBibtex * inset = getInsetByCode<InsetBibtex>(tmpcur,
						Inset::BIBTEX_CODE);
		if (inset) {
			if (inset->addDatabase(to_utf8(cmd.argument())))
				buffer_->updateBibfilesCache();
		}
		break;
	}

	case LFUN_BIBTEX_DATABASE_DEL: {
		Cursor tmpcur = cursor_;
		bv_funcs::findInset(tmpcur, Inset::BIBTEX_CODE, false);
		InsetBibtex * inset = getInsetByCode<InsetBibtex>(tmpcur,
						Inset::BIBTEX_CODE);
		if (inset) {
			if (inset->delDatabase(to_utf8(cmd.argument())))
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
		int const words = countWords(from, to);
		int const chars = countChars(from, to, false);
		int const chars_blanks = countChars(from, to, true);
		docstring message;
		if (cur.selection())
			message = _("Statistics for the selection:");
		else
			message = _("Statistics for the document:");
		message += "\n\n";
		if (words != 1)
			message += bformat(_("%1$d words"), words);
		else
			message += _("One word");
		message += "\n";
		if (chars_blanks != 1)
			message += bformat(_("%1$d characters (including blanks)"),
					  chars_blanks);
		else
			message += _("One character (including blanks)");
		message += "\n";
		if (chars != 1)
			message += bformat(_("%1$d characters (excluding blanks)"),
					  chars);
		else
			message += _("One character (excluding blanks)");

		Alert::information(_("Statistics"), message);
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
		Inset * inset = cur.nextInset();
		if (inset) {
			if (inset->isActive()) {
				Cursor tmpcur = cur;
				tmpcur.pushLeft(*inset);
				inset->dispatch(tmpcur, tmpcmd);
				if (tmpcur.result().dispatched()) {
					cur.dispatched();
				}
			} else if (inset->editable() == Inset::IS_EDITABLE) {
				inset->edit(cur, true);
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
		updateFlags = Update::None;
	}

	return updateFlags;
}


docstring const BufferView::requestSelection()
{
	if (!buffer_)
		return docstring();

	Cursor & cur = cursor_;

	if (!cur.selection()) {
		xsel_cache_.set = false;
		return docstring();
	}

	if (!xsel_cache_.set ||
	    cur.top() != xsel_cache_.cursor ||
	    cur.anchor_.top() != xsel_cache_.anchor)
	{
		xsel_cache_.cursor = cur.top();
		xsel_cache_.anchor = cur.anchor_.top();
		xsel_cache_.set = cur.selection();
		return cur.selectionAsString(false);
	}
	return docstring();
}


void BufferView::clearSelection()
{
	if (buffer_) {
		cursor_.clearSelection();
		// Clear the selection buffer. Otherwise a subsequent
		// middle-mouse-button paste would use the selection buffer,
		// not the more current external selection.
		cap::clearSelection();
		xsel_cache_.set = false;
		// The buffer did not really change, but this causes the
		// redraw we need because we cleared the selection above.
		buffer_->changed();
	}
}


void BufferView::workAreaResize(int width, int height)
{
	// Update from work area
	width_ = width;
	height_ = height;

	if (buffer_)
		resize();
}


Inset const * BufferView::getCoveringInset(Text const & text, int x, int y)
{
	pit_type pit = text.getPitNearY(*this, y);
	BOOST_ASSERT(pit != -1);
	Paragraph const & par = text.getPar(pit);

	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< ": x: " << x
		<< " y: " << y
		<< "  pit: " << pit
		<< endl;
	InsetList::const_iterator iit = par.insetlist.begin();
	InsetList::const_iterator iend = par.insetlist.end();
	for (; iit != iend; ++iit) {
		Inset * const inset = iit->inset;
		if (inset->covers(*this, x, y)) {
			if (!inset->descendable())
				// No need to go further down if the inset is not
				// descendable.
				return inset;

			size_t cell_number = inset->nargs();
			// Check all the inner cell.
			for (size_t i = 0; i != cell_number; ++i) {
				Text const * inner_text = inset->getText(i);
				if (inner_text) {
					// Try deeper.
					Inset const * inset_deeper =
						getCoveringInset(*inner_text, x, y);
					if (inset_deeper)
						return inset_deeper;
				}
			}

			LYXERR(Debug::DEBUG)
				<< BOOST_CURRENT_FUNCTION
				<< ": Hit inset: " << inset << endl;
			return inset;
		}
	}
	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< ": No inset hit. " << endl;
	return 0;
}


bool BufferView::workAreaDispatch(FuncRequest const & cmd0)
{
	//lyxerr << BOOST_CURRENT_FUNCTION << "[ cmd0 " << cmd0 << "]" << endl;

	// This is only called for mouse related events including
	// LFUN_FILE_OPEN generated by drag-and-drop.
	FuncRequest cmd = cmd0;

	// E.g. Qt mouse press when no buffer
	if (!buffer_)
		return false;

	Cursor cur(*this);
	cur.push(buffer_->inset());
	cur.selection() = cursor_.selection();

	// Either the inset under the cursor or the
	// surrounding Text will handle this event.

	// make sure we stay within the screen...
	cmd.y = min(max(cmd.y, -1), height_);

	if (cmd.action == LFUN_MOUSE_MOTION && cmd.button() == mouse_button::none) {

		// Get inset under mouse, if there is one.
		Inset const * covering_inset =
			getCoveringInset(buffer_->text(), cmd.x, cmd.y);
		if (covering_inset == last_inset_)
			// Same inset, no need to do anything...
			return false;

		bool need_redraw = false;
		// const_cast because of setMouseHover().
		Inset * inset = const_cast<Inset *>(covering_inset);
		if (last_inset_)
			// Remove the hint on the last hovered inset (if any).
			need_redraw |= last_inset_->setMouseHover(false);
		if (inset)
			// Highlighted the newly hovered inset (if any).
			need_redraw |= inset->setMouseHover(true);
		last_inset_ = inset;

		// if last metrics update was in singlepar mode, WorkArea::redraw() will
		// not expose the button for redraw. We adjust here the metrics dimension
		// to enable a full redraw.
		// FIXME: It is possible to redraw only the area around the button!
		if (need_redraw
			&& metrics_info_.update_strategy == SingleParUpdate) {
			// FIXME: It should be possible to redraw only the area around
			// the button by doing this:
			//
			//metrics_info_.singlepar = false;
			//metrics_info_.y1 = ymin of button;
			//metrics_info_.y2 = ymax of button;
			//
			// Unfortunately, rowpainter.cpp:paintText() does not distinguish
			// between background updates and text updates. So we use the hammer
			// solution for now. We could also avoid the updateMetrics() below
			// by using the first and last pit of the CoordCache. Have a look
			// at Text::getPitNearY() to see what I mean.
			//
			//metrics_info_.pit1 = first pit of CoordCache;
			//metrics_info_.pit2 = last pit of CoordCache;
			//metrics_info_.singlepar = false;
			//metrics_info_.y1 = 0;
			//metrics_info_.y2 = height_;
			//
			updateMetrics(false);
		}

		// This event (moving without mouse click) is not passed further.
		// This should be changed if it is further utilized.
		return need_redraw;
	}

	// Build temporary cursor.
	Inset * inset = buffer_->text().editXY(cur, cmd.x, cmd.y);

	// Put anchor at the same position.
	cur.resetAnchor();

	// Try to dispatch to an non-editable inset near this position
	// via the temp cursor. If the inset wishes to change the real
	// cursor it has to do so explicitly by using
	//  cur.bv().cursor() = cur;  (or similar)
	if (inset) {
		inset->dispatch(cur, cmd);
	}

	// Now dispatch to the temporary cursor. If the real cursor should
	// be modified, the inset's dispatch has to do so explicitly.
	if (!cur.result().dispatched())
		cur.dispatch(cmd);

	//Do we have a selection?
	theSelection().haveSelection(cursor().selection());

	// Redraw if requested and necessary.
	if (cur.result().dispatched() && cur.result().update())
		return update(cur.result().update());

	return false;
}


void BufferView::scroll(int /*lines*/)
{
//	if (!buffer_)
//		return;
//
//	Text const * t = &buffer_->text();
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
}


void BufferView::setCursorFromRow(int row)
{
	int tmpid = -1;
	int tmppos = -1;

	buffer_->texrow().getIdFromRow(row, tmpid, tmppos);

	cursor_.reset(buffer_->inset());
	if (tmpid == -1)
		buffer_->text().setCursor(cursor_, 0, 0);
	else
		buffer_->text().setCursor(cursor_, buffer_->getParFromID(tmpid).pit(), tmppos);
}


bool BufferView::setCursorFromInset(Inset const * inset)
{
	// are we already there?
	if (cursor().nextInset() == inset)
		return true;

	// Inset is not at cursor position. Find it in the document.
	Cursor cur(*this);
	cur.reset(buffer()->inset());
	do 
		cur.forwardInset();
	while (cur && cur.nextInset() != inset);

	if (cur) {
		setCursor(cur);
		return true;
	}
	return false;
}


void BufferView::gotoLabel(docstring const & label)
{
	for (InsetIterator it = inset_iterator_begin(buffer_->inset()); it; ++it) {
		vector<docstring> labels;
		it->getLabelList(*buffer_, labels);
		if (std::find(labels.begin(), labels.end(), label) != labels.end()) {
			setCursor(it);
			update();
			return;
		}
	}
}


TextMetrics const & BufferView::textMetrics(Text const * t) const
{
	return const_cast<BufferView *>(this)->textMetrics(t);
}


TextMetrics & BufferView::textMetrics(Text const * t)
{
	TextMetricsCache::iterator tmc_it  = text_metrics_.find(t);
	if (tmc_it == text_metrics_.end()) {
		tmc_it = text_metrics_.insert(
			make_pair(t, TextMetrics(this, const_cast<Text *>(t)))).first;
	}
	return tmc_it->second;
}


ParagraphMetrics const & BufferView::parMetrics(Text const * t,
		pit_type pit) const
{
	return textMetrics(t).parMetrics(pit);
}


int BufferView::workHeight() const
{
	return height_;
}


void BufferView::setCursor(DocIterator const & dit)
{
	size_t const n = dit.depth();
	for (size_t i = 0; i < n; ++i)
		dit[i].inset().edit(cursor_, true);

	cursor_.setCursor(dit);
	cursor_.selection() = false;
}


bool BufferView::checkDepm(Cursor & cur, Cursor & old)
{
	// Would be wrong to delete anything if we have a selection.
	if (cur.selection())
		return false;

	bool need_anchor_change = false;
	bool changed = cursor_.text()->deleteEmptyParagraphMechanism(cur, old,
		need_anchor_change);

	if (need_anchor_change)
		cur.resetAnchor();

	if (!changed)
		return false;

	updateLabels(*buffer_);

	updateMetrics(false);
	buffer_->changed();
	return true;
}


bool BufferView::mouseSetCursor(Cursor & cur, bool select)
{
	BOOST_ASSERT(&cur.bv() == this);

	if (!select)
		// this event will clear selection so we save selection for
		// persistent selection
		cap::saveSelection(cursor());

	// Has the cursor just left the inset?
	bool badcursor = false;
	bool leftinset = (&cursor_.inset() != &cur.inset());
	if (leftinset)
		badcursor = notifyCursorLeaves(cursor_, cur);

	// FIXME: shift-mouse selection doesn't work well across insets.
	bool do_selection = select && &cursor_.anchor().inset() == &cur.inset();

	// do the dEPM magic if needed
	// FIXME: (1) move this to InsetText::notifyCursorLeaves?
	// FIXME: (2) if we had a working InsetText::notifyCursorLeaves,
	// the leftinset bool would not be necessary (badcursor instead).
	bool update = leftinset;
	if (!do_selection && !badcursor && cursor_.inTexted())
		update |= checkDepm(cur, cursor_);

	// if the cursor was in an empty script inset and the new
	// position is in the nucleus of the inset, notifyCursorLeaves
	// will kill the script inset itself. So we check all the
	// elements of the cursor to make sure that they are correct.
	// For an example, see bug 2933:
	// http://bugzilla.lyx.org/show_bug.cgi?id=2933
	// The code below could maybe be moved to a DocIterator method.
	//lyxerr << "cur before " << cur <<std::endl;
	DocIterator dit(cur.inset());
	dit.push_back(cur.bottom());
	size_t i = 1;
	while (i < cur.depth() && dit.nextInset() == &cur[i].inset()) {
		dit.push_back(cur[i]);
		++i;
	}
	//lyxerr << "5 cur after" << dit <<std::endl;

	cursor_.setCursor(dit);
	cursor_.boundary(cur.boundary());
	if (do_selection)
		cursor_.setSelection();
	else
		cursor_.clearSelection();
	finishUndo();
	return update;
}


void BufferView::putSelectionAt(DocIterator const & cur,
				int length, bool backwards)
{
	cursor_.clearSelection();

	setCursor(cur);

	if (length) {
		if (backwards) {
			cursor_.pos() += length;
			cursor_.setSelection(cursor_, -length);
		} else
			cursor_.setSelection(cursor_, length);
	}
}


Cursor & BufferView::cursor()
{
	return cursor_;
}


Cursor const & BufferView::cursor() const
{
	return cursor_;
}


pit_type BufferView::anchor_ref() const
{
	return anchor_ref_;
}


ViewMetricsInfo const & BufferView::viewMetricsInfo()
{
	return metrics_info_;
}


// FIXME: We should split-up updateMetrics() for the singlepar case.
void BufferView::updateMetrics(bool singlepar)
{
	Text & buftext = buffer_->text();
	pit_type size = int(buftext.paragraphs().size());

	if (anchor_ref_ > int(buftext.paragraphs().size() - 1)) {
		anchor_ref_ = int(buftext.paragraphs().size() - 1);
		offset_ref_ = 0;
	}

	if (!singlepar) {
		// Clear out the position cache in case of full screen redraw,
		coord_cache_.clear();
	
		// Clear out paragraph metrics to avoid having invalid metrics
		// in the cache from paragraphs not relayouted below
		// The complete text metrics will be redone.
		text_metrics_.clear();
	}

	TextMetrics & tm = textMetrics(&buftext);

	// If the paragraph metrics has changed, we can not
	// use the singlepar optimisation.
	if (singlepar) {
		pit_type const bottom_pit = cursor_.bottom().pit();
		int old_height = tm.parMetrics(bottom_pit).height();
		// In Single Paragraph mode, rebreak only
		// the (main text, not inset!) paragraph containing the cursor.
		// (if this paragraph contains insets etc., rebreaking will
		// recursively descend)
		tm.redoParagraph(bottom_pit);
		// Paragraph height has changed so we cannot proceed to
		// the singlePar optimisation.
		if (tm.parMetrics(bottom_pit).height() != old_height)
			singlepar = false;
	}

	pit_type const pit = anchor_ref_;
	int pit1 = pit;
	int pit2 = pit;
	size_t const npit = buftext.paragraphs().size();

	// Rebreak anchor paragraph.
	if (!singlepar)
		tm.redoParagraph(pit);

	updateOffsetRef();

	int y0 = tm.parMetrics(pit).ascent() - offset_ref_;

	// Redo paragraphs above anchor if necessary.
	int y1 = y0;
	while (y1 > 0 && pit1 > 0) {
		y1 -= tm.parMetrics(pit1).ascent();
		--pit1;
		if (!singlepar)
			tm.redoParagraph(pit1);
		y1 -= tm.parMetrics(pit1).descent();
	}


	// Take care of ascent of first line
	y1 -= tm.parMetrics(pit1).ascent();

	// Normalize anchor for next time
	anchor_ref_ = pit1;
	offset_ref_ = -y1;

	// Grey at the beginning is ugly
	if (pit1 == 0 && y1 > 0) {
		y0 -= y1;
		y1 = 0;
		anchor_ref_ = 0;
	}

	// Redo paragraphs below the anchor if necessary.
	int y2 = y0;
	while (y2 < height_ && pit2 < int(npit) - 1) {
		y2 += tm.parMetrics(pit2).descent();
		++pit2;
		if (!singlepar)
			tm.redoParagraph(pit2);
		y2 += tm.parMetrics(pit2).ascent();
	}

	// Take care of descent of last line
	y2 += tm.parMetrics(pit2).descent();

	// The coordinates of all these paragraphs are correct, cache them
	int y = y1;
	CoordCache::InnerParPosCache & parPos = coord_cache_.parPos()[&buftext];
	for (pit_type pit = pit1; pit <= pit2; ++pit) {
		ParagraphMetrics const & pm = tm.parMetrics(pit);
		y += pm.ascent();
		parPos[pit] = Point(0, y);
		if (singlepar && pit == cursor_.bottom().pit()) {
			// In Single Paragraph mode, collect here the
			// y1 and y2 of the (one) paragraph the cursor is in
			y1 = y - pm.ascent();
			y2 = y + pm.descent();
		}
		y += pm.descent();
	}

	if (singlepar) {
		// collect cursor paragraph iter bounds
		pit1 = cursor_.bottom().pit();
		pit2 = cursor_.bottom().pit();
	}

	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< " y1: " << y1
		<< " y2: " << y2
		<< " pit1: " << pit1
		<< " pit2: " << pit2
		<< " npit: " << npit
		<< " singlepar: " << singlepar
		<< "size: " << size
		<< endl;

	metrics_info_ = ViewMetricsInfo(pit1, pit2, y1, y2,
		singlepar? SingleParUpdate: FullScreenUpdate, size);

	if (lyxerr.debugging(Debug::WORKAREA)) {
		LYXERR(Debug::WORKAREA) << "BufferView::updateMetrics" << endl;
		coord_cache_.dump();
	}
}


void BufferView::menuInsertLyXFile(string const & filenm)
{
	BOOST_ASSERT(cursor_.inTexted());
	string filename = filenm;

	if (filename.empty()) {
		// Launch a file browser
		// FIXME UNICODE
		string initpath = lyxrc.document_path;

		if (buffer_) {
			string const trypath = buffer_->filePath();
			// If directory is writeable, use this as default.
			if (isDirWriteable(FileName(trypath)))
				initpath = trypath;
		}

		// FIXME UNICODE
		FileDialog fileDlg(_("Select LyX document to insert"),
			LFUN_FILE_INSERT,
			make_pair(_("Documents|#o#O"), from_utf8(lyxrc.document_path)),
			make_pair(_("Examples|#E#e"),
				    from_utf8(addPath(package().system_support().absFilename(),
				    "examples"))));

		FileDialog::Result result =
			fileDlg.open(from_utf8(initpath),
				     FileFilterList(_("LyX Documents (*.lyx)")),
				     docstring());

		if (result.first == FileDialog::Later)
			return;

		// FIXME UNICODE
		filename = to_utf8(result.second);

		// check selected filename
		if (filename.empty()) {
			// emit message signal.
			message(_("Canceled."));
			return;
		}
	}

	// Get absolute path of file and add ".lyx"
	// to the filename if necessary
	filename = fileSearch(string(), filename, "lyx").absFilename();

	docstring const disp_fn = makeDisplayPath(filename);
	// emit message signal.
	message(bformat(_("Inserting document %1$s..."), disp_fn));

	docstring res;
	Buffer buf("", false);
	if (lyx::loadLyXFile(&buf, FileName(filename))) {
		ErrorList & el = buffer_->errorList("Parse");
		// Copy the inserted document error list into the current buffer one.
		el = buf.errorList("Parse");
		recordUndo(cursor_);
		cap::pasteParagraphList(cursor_, buf.paragraphs(),
					     buf.params().textclass, el);
		res = _("Document %1$s inserted.");
	} else
		res = _("Could not insert document %1$s");

	// emit message signal.
	message(bformat(res, disp_fn));
	buffer_->errors("Parse");
	resize();
}

} // namespace lyx
