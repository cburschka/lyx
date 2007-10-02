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
#include "callback.h" // added for Dispatch functions
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
#include "LyX.h"
#include "lyxfind.h"
#include "LyXFunc.h"
#include "Layout.h"
#include "LyXRC.h"
#include "MetricsInfo.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "Session.h"
#include "TexRow.h"
#include "Text.h"
#include "TextClass.h"
#include "Undo.h"
#include "VSpace.h"
#include "WordLangTuple.h"

#include "insets/InsetBibtex.h"
#include "insets/InsetCommand.h" // ChangeRefs
#include "insets/InsetRef.h"
#include "insets/InsetText.h"

#include "frontends/alert.h"
#include "frontends/Delegates.h"
#include "frontends/FileDialog.h"
#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"
#include "frontends/Selection.h"

#include "graphics/Previews.h"

#include "support/convert.h"
#include "support/FileFilterList.h"
#include "support/filetools.h"
#include "support/Package.h"
#include "support/types.h"

#include <boost/bind.hpp>
#include <boost/current_function.hpp>
#include <boost/next_prior.hpp>

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
T * getInsetByCode(Cursor const & cur, Inset::Code code)
{
	DocIterator it = cur;
	Inset * inset = it.nextInset();
	if (inset && inset->lyxCode() == code)
		return static_cast<T*>(inset);
	return 0;
}


bool findInset(DocIterator & dit, vector<Inset_code> const & codes,
	bool same_content);

bool findNextInset(DocIterator & dit, vector<Inset_code> const & codes,
	string const & contents)
{
	DocIterator tmpdit = dit;

	while (tmpdit) {
		Inset const * inset = tmpdit.nextInset();
		if (inset
		    && find(codes.begin(), codes.end(), inset->lyxCode()) != codes.end()
		    && (contents.empty() ||
		    static_cast<InsetCommand const *>(inset)->getContents() == contents)) {
			dit = tmpdit;
			return true;
		}
		tmpdit.forwardInset();
	}

	return false;
}


/// Looks for next inset with one of the the given code
bool findInset(DocIterator & dit, vector<Inset_code> const & codes,
	bool same_content)
{
	string contents;
	DocIterator tmpdit = dit;
	tmpdit.forwardInset();
	if (!tmpdit)
		return false;

	if (same_content) {
		Inset const * inset = tmpdit.nextInset();
		if (inset
		    && find(codes.begin(), codes.end(), inset->lyxCode()) != codes.end()) {
			contents = static_cast<InsetCommand const *>(inset)->getContents();
		}
	}

	if (!findNextInset(tmpdit, codes, contents)) {
		if (dit.depth() != 1 || dit.pit() != 0 || dit.pos() != 0) {
			tmpdit  = doc_iterator_begin(tmpdit.bottom().inset());
			if (!findNextInset(tmpdit, codes, contents))
				return false;
		} else
			return false;
	}

	dit = tmpdit;
	return true;
}


/// Looks for next inset with the given code
void findInset(DocIterator & dit, Inset_code code, bool same_content)
{
	findInset(dit, vector<Inset_code>(1, code), same_content);
}


/// Moves cursor to the next inset with one of the given codes.
void gotoInset(BufferView * bv, vector<Inset_code> const & codes,
	       bool same_content)
{
	Cursor tmpcur = bv->cursor();
	if (!findInset(tmpcur, codes, same_content)) {
		bv->cursor().message(_("No more insets"));
		return;
	}

	tmpcur.clearSelection();
	bv->setCursor(tmpcur);
}


/// Moves cursor to the next inset with given code.
void gotoInset(BufferView * bv, Inset_code code, bool same_content)
{
	gotoInset(bv, vector<Inset_code>(1, code), same_content);
}



/// the type of outline operation
enum OutlineOp {
	OutlineUp, // Move this header with text down
	OutlineDown,   // Move this header with text up
	OutlineIn, // Make this header deeper
	OutlineOut // Make this header shallower
};


void outline(OutlineOp mode, Cursor & cur)
{
	Buffer & buf = cur.buffer();
	pit_type & pit = cur.pit();
	ParagraphList & pars = buf.text().paragraphs();
	ParagraphList::iterator bgn = pars.begin();
	// The first paragraph of the area to be copied:
	ParagraphList::iterator start = boost::next(bgn, pit);
	// The final paragraph of area to be copied:
	ParagraphList::iterator finish = start;
	ParagraphList::iterator end = pars.end();

	TextClass::const_iterator lit =
		buf.params().getTextClass().begin();
	TextClass::const_iterator const lend =
		buf.params().getTextClass().end();

	int const thistoclevel = start->layout()->toclevel;
	int toclevel;
	switch (mode) {
		case OutlineUp: {
			// Move out (down) from this section header
			if (finish != end)
				++finish;
			// Seek the one (on same level) below
			for (; finish != end; ++finish) {
				toclevel = finish->layout()->toclevel;
				if (toclevel != Layout::NOT_IN_TOC
				    && toclevel <= thistoclevel) {
					break;
				}
			}
			ParagraphList::iterator dest = start;
			// Move out (up) from this header
			if (dest == bgn)
				break;
			// Search previous same-level header above
			do {
				--dest;
				toclevel = dest->layout()->toclevel;
			} while(dest != bgn
				&& (toclevel == Layout::NOT_IN_TOC
				    || toclevel > thistoclevel));
			// Not found; do nothing
			if (toclevel == Layout::NOT_IN_TOC || toclevel > thistoclevel)
				break;
			pit_type const newpit = std::distance(bgn, dest);
			pit_type const len = std::distance(start, finish);
			pit_type const deletepit = pit + len;
			recordUndo(cur, Undo::ATOMIC, newpit, deletepit - 1);
			pars.insert(dest, start, finish);
			start = boost::next(pars.begin(), deletepit);
			pit = newpit;
			pars.erase(start, finish);
			break;
		}
		case OutlineDown: {
			// Go down out of current header:
			if (finish != end)
				++finish;
			// Find next same-level header:
			for (; finish != end; ++finish) {
				toclevel = finish->layout()->toclevel;
				if (toclevel != Layout::NOT_IN_TOC && toclevel <= thistoclevel)
					break;
			}
			ParagraphList::iterator dest = finish;
			// Go one down from *this* header:
			if (dest != end)
				++dest;
			else
				break;
			// Go further down to find header to insert in front of:
			for (; dest != end; ++dest) {
				toclevel = dest->layout()->toclevel;
				if (toclevel != Layout::NOT_IN_TOC && toclevel <= thistoclevel)
					break;
			}
			// One such was found:
			pit_type newpit = std::distance(bgn, dest);
			pit_type const len = std::distance(start, finish);
			recordUndo(cur, Undo::ATOMIC, pit, newpit - 1);
			pars.insert(dest, start, finish);
			start = boost::next(bgn, pit);
			pit = newpit - len;
			pars.erase(start, finish);
			break;
		}
		case OutlineIn:
			recordUndo(cur);
			for (; lit != lend; ++lit) {
				if ((*lit)->toclevel == thistoclevel + 1 &&
				    start->layout()->labeltype == (*lit)->labeltype) {
					start->layout((*lit));
					break;
				}
			}
			break;
		case OutlineOut:
			recordUndo(cur);
			for (; lit != lend; ++lit) {
				if ((*lit)->toclevel == thistoclevel - 1 &&
				    start->layout()->labeltype == (*lit)->labeltype) {
					start->layout((*lit));
					break;
				}
			}
			break;
		default:
			break;
	}
}

} // anon namespace


/////////////////////////////////////////////////////////////////////
//
// BufferView
//
/////////////////////////////////////////////////////////////////////


BufferView::BufferView(Buffer & buf)
	: width_(0), height_(0), buffer_(buf), wh_(0),
	  cursor_(*this),
	  multiparsel_cache_(false), anchor_ref_(0), offset_ref_(0),
	  need_centering_(false), intl_(new Intl), last_inset_(0),
	  gui_(0)
{
	xsel_cache_.set = false;
	intl_->initKeyMapper(lyxrc.use_kbmap);

	cursor_.push(buffer_.inset());
	cursor_.resetAnchor();
	cursor_.setCurrentFont();

	if (graphics::Previews::status() != LyXRC::PREVIEW_OFF)
		graphics::Previews::get().generateBufferPreviews(buffer_);
}


BufferView::~BufferView()
{
	// current buffer is going to be switched-off, save cursor pos
	// Ideally, the whole cursor stack should be saved, but session
	// currently can only handle bottom (whole document) level pit and pos.
	// That is to say, if a cursor is in a nested inset, it will be
	// restore to the left of the top level inset.
	LyX::ref().session().lastFilePos().save(
		support::FileName(buffer_.fileName()),
		boost::tie(cursor_.bottom().pit(), cursor_.bottom().pos()) );
}


Buffer & BufferView::buffer()
{
	return buffer_;
}


Buffer const & BufferView::buffer() const
{
	return buffer_;
}


bool BufferView::fitCursor()
{
	if (cursorStatus(cursor_) == CUR_INSIDE) {
		frontend::FontMetrics const & fm =
			theFontMetrics(cursor_.getFont());
		int const asc = fm.maxAscent();
		int const des = fm.maxDescent();
		Point const p = getPos(cursor_, cursor_.boundary());
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
		<< "]  buffer: " << &buffer_ << endl;

	// Update macro store
	if (!(cursor().inMathed() && cursor().inMacroMode()))
		buffer_.buildMacros();

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

	if (flags & Update::FitCursor) {
		//FIXME: updateMetrics() does not update paragraph position
		// This is done at draw() time. So we need a redraw!
		buffer_.changed();
		if (fitCursor())
			updateMetrics(false);
		else
			// The screen has already been updated thanks to the
			// 'buffer_.changed()' call three line above. So no need
			// to redraw again.
			return false;
	}

	// tell the frontend to update the screen.
	return true;
}


void BufferView::updateScrollbar()
{
	Text & t = buffer_.text();
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

	Text & t = buffer_.text();
	TextMetrics & tm = text_metrics_[&t];

	float const bar = value / float(wh_ * t.paragraphs().size());

	anchor_ref_ = int(bar * t.paragraphs().size());
	if (anchor_ref_ >  int(t.paragraphs().size()) - 1)
		anchor_ref_ = int(t.paragraphs().size()) - 1;

	tm.redoParagraph(anchor_ref_);
	int const h = tm.parMetrics(anchor_ref_).height();
	offset_ref_ = int((bar * t.paragraphs().size() - anchor_ref_) * h);
	updateMetrics(false);
	buffer_.changed();
}


void BufferView::setCursorFromScrollbar()
{
	TextMetrics & tm = text_metrics_[&buffer_.text()];

	int const height = 2 * defaultRowHeight();
	int const first = height;
	int const last = height_ - height;
	Cursor & cur = cursor_;

	switch (cursorStatus(cur)) {
	case CUR_ABOVE:
		// We reset the cursor because cursorStatus() does not
		// work when the cursor is within mathed.
		cur.reset(buffer_.inset());
		tm.setCursorFromCoordinates(cur, 0, first);
		cur.clearSelection();
		break;
	case CUR_BELOW:
		// We reset the cursor because cursorStatus() does not
		// work when the cursor is within mathed.
		cur.reset(buffer_.inset());
		tm.setCursorFromCoordinates(cur, 0, last);
		cur.clearSelection();
		break;
	case CUR_INSIDE:
		int const y = getPos(cur, cur.boundary()).y_;
		int const newy = min(last, max(y, first));
		if (y != newy) {
			cur.reset(buffer_.inset());
			tm.setCursorFromCoordinates(cur, 0, newy);
		}
	}
}


Change const BufferView::getCurrentChange() const
{
	if (!cursor_.selection())
		return Change(Change::UNCHANGED);

	DocIterator dit = cursor_.selectionBegin();
	return dit.paragraph().lookupChange(dit.pos());
}


// this could be used elsewhere as well?
// FIXME: This does not work within mathed!
CursorStatus BufferView::cursorStatus(DocIterator const & dit) const
{
	Point const p = getPos(dit, dit.boundary());
	if (p.y_ < 0)
		return CUR_ABOVE;
	if (p.y_ > workHeight())
		return CUR_BELOW;
	return CUR_INSIDE;
}


void BufferView::saveBookmark(unsigned int idx)
{
	// tenatively save bookmark, id and pos will be used to
	// acturately locate a bookmark in a 'live' lyx session.
	// pit and pos will be updated with bottom level pit/pos
	// when lyx exits.
	LyX::ref().session().bookmarks().save(
		FileName(buffer_.fileName()),
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


bool BufferView::moveToPosition(pit_type bottom_pit, pos_type bottom_pos,
	int top_id, pos_type top_pos)
{
	bool success = false;
	DocIterator doc_it;

	cursor_.clearSelection();

	// if a valid par_id is given, try it first
	// This is the case for a 'live' bookmark when unique paragraph ID
	// is used to track bookmarks.
	if (top_id > 0) {
		ParIterator par = buffer_.getParFromID(top_id);
		if (par != buffer_.par_iterator_end()) {
			doc_it = makeDocIterator(par, min(par->size(), top_pos));
			// Some slices of the iterator may not be
			// reachable (e.g. closed collapsable inset)
			// so the dociterator may need to be
			// shortened. Otherwise, setCursor may crash
			// lyx when the cursor can not be set to these
			// insets.
			size_t const n = doc_it.depth();
			for (size_t i = 0; i < n; ++i)
				if (doc_it[i].inset().editable() != Inset::HIGHLY_EDITABLE) {
					doc_it.resize(i);
					break;
				}
			success = true;
		}
	}

	// if top_id == 0, or searching through top_id failed
	// This is the case for a 'restored' bookmark when only bottom
	// (document level) pit was saved. Because of this, bookmark
	// restoration is inaccurate. If a bookmark was within an inset,
	// it will be restored to the left of the outmost inset that contains
	// the bookmark.
	if (static_cast<size_t>(bottom_pit) < buffer_.paragraphs().size()) {
		doc_it = doc_iterator_begin(buffer_.inset());
		doc_it.pit() = bottom_pit;
		doc_it.pos() = min(bottom_pos, doc_it.paragraph().size());
		success = true;
	}

	if (success) {
		// Note: only bottom (document) level pit is set.
		setCursor(doc_it);
		// set the current font.
		cursor_.setCurrentFont();
		// center the screen on this new position.
		center();
	}

	return success;
}


void BufferView::translateAndInsert(char_type c, Text * t, Cursor & cur)
{
	if (lyxrc.rtl_support) {
		if (cursor_.real_current_font.isRightToLeft()) {
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
	int y = coordOffset(cursor_, cursor_.boundary()).y_;
	offset_ref_ = y + pm.ascent() - height_ / 2;

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
		flag.enabled(!buffer_.undostack().empty());
		break;
	case LFUN_REDO:
		flag.enabled(!buffer_.redostack().empty());
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
		flag.enabled(!cmd.argument().empty()
		    || getInsetByCode<InsetRef>(cur, Inset::REF_CODE));
		break;
	}

	case LFUN_CHANGES_TRACK:
		flag.enabled(true);
		flag.setOnOff(buffer_.params().trackChanges);
		break;

	case LFUN_CHANGES_OUTPUT:
		flag.enabled(true);
		flag.setOnOff(buffer_.params().outputChanges);
		break;

	case LFUN_CHANGES_MERGE:
	case LFUN_CHANGE_NEXT:
	case LFUN_ALL_CHANGES_ACCEPT:
	case LFUN_ALL_CHANGES_REJECT:
		// TODO: context-sensitive enabling of LFUNs
		// In principle, these command should only be enabled if there
		// is a change in the document. However, without proper
		// optimizations, this will inevitably result in poor performance.
		flag.enabled(true);
		break;

	case LFUN_BUFFER_TOGGLE_COMPRESSION: {
		flag.setOnOff(buffer_.params().compressed);
		break;
	}

	case LFUN_SCREEN_UP:
	case LFUN_SCREEN_DOWN:
		flag.enabled(true);
		break;

	// FIXME: LFUN_SCREEN_DOWN_SELECT should be removed from
	// everywhere else before this can enabled:
	case LFUN_SCREEN_UP_SELECT:
	case LFUN_SCREEN_DOWN_SELECT:
		flag.enabled(false);
		break;

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
		for (Buffer * b = &buffer_; i == 0 || b != &buffer_;
			b = theBufferList().next(b)) {

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

				if (b == &buffer_) {
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

	case LFUN_OUTLINE_UP:
		outline(OutlineUp, cursor_);
		cursor_.text()->setCursor(cursor_, cursor_.pit(), 0);
		updateLabels(buffer_);
		break;
	case LFUN_OUTLINE_DOWN:
		outline(OutlineDown, cursor_);
		cursor_.text()->setCursor(cursor_, cursor_.pit(), 0);
		updateLabels(buffer_);
		break;
	case LFUN_OUTLINE_IN:
		outline(OutlineIn, cursor_);
		updateLabels(buffer_);
		break;
	case LFUN_OUTLINE_OUT:
		outline(OutlineOut, cursor_);
		updateLabels(buffer_);
		break;

	case LFUN_NOTE_NEXT:
		gotoInset(this, Inset::NOTE_CODE, false);
		break;

	case LFUN_REFERENCE_NEXT: {
		vector<Inset_code> tmp;
		tmp.push_back(Inset::LABEL_CODE);
		tmp.push_back(Inset::REF_CODE);
		gotoInset(this, tmp, true);
		break;
	}

	case LFUN_CHANGES_TRACK:
		buffer_.params().trackChanges = !buffer_.params().trackChanges;
		break;

	case LFUN_CHANGES_OUTPUT:
		buffer_.params().outputChanges = !buffer_.params().outputChanges;
		if (buffer_.params().outputChanges) {
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
		cursor_.reset(buffer_.inset());
		cursor_.selHandle(true);
		buffer_.text().cursorBottom(cursor_);
		// accept everything in a single step to support atomic undo
		buffer_.text().acceptOrRejectChanges(cursor_, Text::ACCEPT);
		break;

	case LFUN_ALL_CHANGES_REJECT:
		// select complete document
		cursor_.reset(buffer_.inset());
		cursor_.selHandle(true);
		buffer_.text().cursorBottom(cursor_);
		// reject everything in a single step to support atomic undo
		// Note: reject does not work recursively; the user may have to repeat the operation
		buffer_.text().acceptOrRejectChanges(cursor_, Text::REJECT);
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
		findInset(tmpcur, Inset::BIBTEX_CODE, false);
		InsetBibtex * inset = getInsetByCode<InsetBibtex>(tmpcur,
						Inset::BIBTEX_CODE);
		if (inset) {
			if (inset->addDatabase(to_utf8(cmd.argument())))
				buffer_.updateBibfilesCache();
		}
		break;
	}

	case LFUN_BIBTEX_DATABASE_DEL: {
		Cursor tmpcur = cursor_;
		findInset(tmpcur, Inset::BIBTEX_CODE, false);
		InsetBibtex * inset = getInsetByCode<InsetBibtex>(tmpcur,
						Inset::BIBTEX_CODE);
		if (inset) {
			if (inset->delDatabase(to_utf8(cmd.argument())))
				buffer_.updateBibfilesCache();
		}
		break;
	}

	case LFUN_WORDS_COUNT: {
		DocIterator from, to;
		if (cur.selection()) {
			from = cur.selectionBegin();
			to = cur.selectionEnd();
		} else {
			from = doc_iterator_begin(buffer_.inset());
			to = doc_iterator_end(buffer_.inset());
		}
		int const count = countWords(from, to);
		docstring message;
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
		buffer_.params().compressed = !buffer_.params().compressed;
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

	case LFUN_SCREEN_UP:
	case LFUN_SCREEN_DOWN: {
		Point p = getPos(cur, cur.boundary());
		if (p.y_ < 0 || p.y_ > height_) {
			// The cursor is off-screen so recenter before proceeding.
			center();
			updateMetrics(false);
			//FIXME: updateMetrics() does not update paragraph position
			// This is done at draw() time. So we need a redraw!
			buffer_.changed();
			p = getPos(cur, cur.boundary());
		}
		scroll(cmd.action == LFUN_SCREEN_UP? - height_ : height_);
		cur.reset(buffer_.inset());
		text_metrics_[&buffer_.text()].editXY(cur, p.x_, p.y_);
		//FIXME: what to do with cur.x_target()?
		finishUndo();
		// The metrics are already up to date. see scroll()
		updateFlags = Update::None;
		break;
	}

	case LFUN_SCREEN_UP_SELECT:
	case LFUN_SCREEN_DOWN_SELECT: {
		cur.selHandle(true);
		size_t initial_depth = cur.depth();
		Point const p = getPos(cur, cur.boundary());
		scroll(cmd.action == LFUN_SCREEN_UP_SELECT? - height_ : height_);
		// FIXME: We need to verify if the cursor stayed within an inset...
		//cur.reset(buffer_.inset());
		text_metrics_[&buffer_.text()].editXY(cur, p.x_, p.y_);
		finishUndo();
		while (cur.depth() > initial_depth) {
			cur.forwardInset();
		}
		// FIXME: we need to do a redraw again because of the selection
		buffer_.changed();
		updateFlags = Update::Force | Update::FitCursor;
		break;
	}

	default:
		updateFlags = Update::None;
	}

	return updateFlags;
}


docstring const BufferView::requestSelection()
{
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
	cursor_.clearSelection();
	// Clear the selection buffer. Otherwise a subsequent
	// middle-mouse-button paste would use the selection buffer,
	// not the more current external selection.
	cap::clearSelection();
	xsel_cache_.set = false;
	// The buffer did not really change, but this causes the
	// redraw we need because we cleared the selection above.
	buffer_.changed();
}


void BufferView::resize(int width, int height)
{
	// Update from work area
	width_ = width;
	height_ = height;

	updateMetrics(false);
}


Inset const * BufferView::getCoveringInset(Text const & text, int x, int y)
{
	TextMetrics & tm = text_metrics_[&text];
	Inset * inset = tm.checkInsetHit(x, y);
	if (!inset)
		return 0;

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

	return inset;
}


bool BufferView::workAreaDispatch(FuncRequest const & cmd0)
{
	//lyxerr << BOOST_CURRENT_FUNCTION << "[ cmd0 " << cmd0 << "]" << endl;

	// This is only called for mouse related events including
	// LFUN_FILE_OPEN generated by drag-and-drop.
	FuncRequest cmd = cmd0;

	Cursor cur(*this);
	cur.push(buffer_.inset());
	cur.selection() = cursor_.selection();

	// Either the inset under the cursor or the
	// surrounding Text will handle this event.

	// make sure we stay within the screen...
	cmd.y = min(max(cmd.y, -1), height_);

	if (cmd.action == LFUN_MOUSE_MOTION && cmd.button() == mouse_button::none) {

		// Get inset under mouse, if there is one.
		Inset const * covering_inset =
			getCoveringInset(buffer_.text(), cmd.x, cmd.y);
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
		if (!need_redraw)
			return false;

		// if last metrics update was in singlepar mode, WorkArea::redraw() will
		// not expose the button for redraw. We adjust here the metrics dimension
		// to enable a full redraw in any case as this is not costly.
		TextMetrics & tm = text_metrics_[&buffer_.text()];
		std::pair<pit_type, ParagraphMetrics const *> firstpm = tm.first();
		std::pair<pit_type, ParagraphMetrics const *> lastpm = tm.last();
		int y1 = firstpm.second->position() - firstpm.second->ascent();
		int y2 = lastpm.second->position() + lastpm.second->descent();
		metrics_info_ = ViewMetricsInfo(firstpm.first, lastpm.first, y1, y2,
			FullScreenUpdate, buffer_.text().paragraphs().size());
		// Reinitialize anchor to first pit.
		anchor_ref_ = firstpm.first;
		offset_ref_ = -y1;
		LYXERR(Debug::PAINTING)
			<< "Mouse hover detected at: (" << cmd.x << ", " << cmd.y << ")"
			<< "\nTriggering redraw: y1: " << y1 << " y2: " << y2
			<< " pit1: " << firstpm.first << " pit2: " << lastpm.first << endl;

		// This event (moving without mouse click) is not passed further.
		// This should be changed if it is further utilized.
		return true;
	}

	// Build temporary cursor.
	Inset * inset = text_metrics_[&buffer_.text()].editXY(cur, cmd.x, cmd.y);

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


void BufferView::scroll(int y)
{
	if (y > 0)
		scrollDown(y);
	else if (y < 0)
		scrollUp(-y);
}


void BufferView::scrollDown(int offset)
{
	Text * text = &buffer_.text();
	TextMetrics & tm = text_metrics_[text];
	int ymax = height_ + offset;
	while (true) {
		std::pair<pit_type, ParagraphMetrics const *> last = tm.last();
		int bottom_pos = last.second->position() + last.second->descent();
		if (last.first + 1 == int(text->paragraphs().size())) {
			if (bottom_pos <= height_)
				return;
			offset = min(offset, bottom_pos - height_);
			break;
		}
		if (bottom_pos > ymax)
			break;
		tm.newParMetricsDown();
	}
	offset_ref_ += offset;
	updateMetrics(false);
	buffer_.changed();
}


void BufferView::scrollUp(int offset)
{
	Text * text = &buffer_.text();
	TextMetrics & tm = text_metrics_[text];
	int ymin = - offset;
	while (true) {
		std::pair<pit_type, ParagraphMetrics const *> first = tm.first();
		int top_pos = first.second->position() - first.second->ascent();
		if (first.first == 0) {
			if (top_pos >= 0)
				return;
			offset = min(offset, - top_pos);
			break;
		}
		if (top_pos < ymin)
			break;
		tm.newParMetricsUp();
	}
	offset_ref_ -= offset;
	updateMetrics(false);
	buffer_.changed();
}


void BufferView::setCursorFromRow(int row)
{
	int tmpid = -1;
	int tmppos = -1;

	buffer_.texrow().getIdFromRow(row, tmpid, tmppos);

	cursor_.reset(buffer_.inset());
	if (tmpid == -1)
		buffer_.text().setCursor(cursor_, 0, 0);
	else
		buffer_.text().setCursor(cursor_, buffer_.getParFromID(tmpid).pit(), tmppos);
}


void BufferView::gotoLabel(docstring const & label)
{
	for (InsetIterator it = inset_iterator_begin(buffer_.inset()); it; ++it) {
		vector<docstring> labels;
		it->getLabelList(buffer_, labels);
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

	updateLabels(buffer_);

	updateMetrics(false);
	buffer_.changed();
	return true;
}


bool BufferView::mouseSetCursor(Cursor & cur)
{
	BOOST_ASSERT(&cur.bv() == this);

	// this event will clear selection so we save selection for
	// persistent selection
	cap::saveSelection(cursor());

	// Has the cursor just left the inset?
	bool badcursor = false;
	bool leftinset = (&cursor_.inset() != &cur.inset());
	if (leftinset)
		badcursor = notifyCursorLeaves(cursor_, cur);

	// do the dEPM magic if needed
	// FIXME: (1) move this to InsetText::notifyCursorLeaves?
	// FIXME: (2) if we had a working InsetText::notifyCursorLeaves,
	// the leftinset bool would not be necessary (badcursor instead).
	bool update = leftinset;
	if (!badcursor && cursor_.inTexted())
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


bool BufferView::singleParUpdate()
{
	Text & buftext = buffer_.text();
	pit_type const bottom_pit = cursor_.bottom().pit();
	TextMetrics & tm = textMetrics(&buftext);
	int old_height = tm.parMetrics(bottom_pit).height();

	// In Single Paragraph mode, rebreak only
	// the (main text, not inset!) paragraph containing the cursor.
	// (if this paragraph contains insets etc., rebreaking will
	// recursively descend)
	tm.redoParagraph(bottom_pit);
	ParagraphMetrics const & pm = tm.parMetrics(bottom_pit);		
	if (pm.height() != old_height)
		// Paragraph height has changed so we cannot proceed to
		// the singlePar optimisation.
		return false;

	int y1 = pm.position() - pm.ascent();
	int y2 = pm.position() + pm.descent();
	metrics_info_ = ViewMetricsInfo(bottom_pit, bottom_pit, y1, y2,
		SingleParUpdate, buftext.paragraphs().size());
	LYXERR(Debug::PAINTING)
		<< BOOST_CURRENT_FUNCTION
		<< "\ny1: " << y1
		<< " y2: " << y2
		<< " pit: " << bottom_pit
		<< " singlepar: 1"
		<< endl;
	return true;
}


void BufferView::updateMetrics(bool singlepar)
{
	if (singlepar && singleParUpdate())
		// No need to update the full screen metrics.
		return;

	Text & buftext = buffer_.text();
	pit_type const npit = int(buftext.paragraphs().size());

	if (anchor_ref_ > int(npit - 1)) {
		anchor_ref_ = int(npit - 1);
		offset_ref_ = 0;
	}

	// Clear out the position cache in case of full screen redraw,
	coord_cache_.clear();

	// Clear out paragraph metrics to avoid having invalid metrics
	// in the cache from paragraphs not relayouted below
	// The complete text metrics will be redone.
	text_metrics_.clear();

	TextMetrics & tm = textMetrics(&buftext);

	pit_type const pit = anchor_ref_;
	int pit1 = pit;
	int pit2 = pit;

	// Rebreak anchor paragraph.
	tm.redoParagraph(pit);

	// Take care of anchor offset if case a recentering is needed.
	updateOffsetRef();

	int y0 = tm.parMetrics(pit).ascent() - offset_ref_;

	// Redo paragraphs above anchor if necessary.
	int y1 = y0;
	while (y1 > 0 && pit1 > 0) {
		y1 -= tm.parMetrics(pit1).ascent();
		--pit1;
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
		tm.redoParagraph(pit2);
		y2 += tm.parMetrics(pit2).ascent();
	}

	// Take care of descent of last line
	y2 += tm.parMetrics(pit2).descent();

	LYXERR(Debug::PAINTING)
		<< BOOST_CURRENT_FUNCTION
		<< "\n y1: " << y1
		<< " y2: " << y2
		<< " pit1: " << pit1
		<< " pit2: " << pit2
		<< " npit: " << npit
		<< " singlepar: 0"
		<< endl;

	metrics_info_ = ViewMetricsInfo(pit1, pit2, y1, y2,
		FullScreenUpdate, npit);

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
		string const trypath = buffer_.filePath();
		// If directory is writeable, use this as default.
		if (isDirWriteable(FileName(trypath)))
			initpath = trypath;

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
		ErrorList & el = buffer_.errorList("Parse");
		// Copy the inserted document error list into the current buffer one.
		el = buf.errorList("Parse");
		recordUndo(cursor_);
		cap::pasteParagraphList(cursor_, buf.paragraphs(),
					     buf.params().getTextClassPtr(), el);
		res = _("Document %1$s inserted.");
	} else
		res = _("Could not insert document %1$s");

	// emit message signal.
	message(bformat(res, disp_fn));
	buffer_.errors("Parse");
	updateMetrics(false);
}


Point BufferView::coordOffset(DocIterator const & dit, bool boundary) const
{
	int x = 0;
	int y = 0;
	int lastw = 0;

	// Addup contribution of nested insets, from inside to outside,
 	// keeping the outer paragraph for a special handling below
	for (size_t i = dit.depth() - 1; i >= 1; --i) {
		CursorSlice const & sl = dit[i];
		int xx = 0;
		int yy = 0;
		
		// get relative position inside sl.inset()
		sl.inset().cursorPos(*this, sl, boundary && (i + 1 == dit.depth()), xx, yy);
		
		// Make relative position inside of the edited inset relative to sl.inset()
		x += xx;
		y += yy;
		
		// In case of an RTL inset, the edited inset will be positioned to the left
		// of xx:yy
		if (sl.text()) {
			bool boundary_i = boundary && i + 1 == dit.depth();
			bool rtl = textMetrics(sl.text()).isRTL(sl, boundary_i);
			if (rtl)
				x -= lastw;
		}

		// remember width for the case that sl.inset() is positioned in an RTL inset
		if (i && dit[i - 1].text()) {
			// If this Inset is inside a Text Inset, retrieve the Dimension
			// from the containing text instead of using Inset::dimension() which
			// might not be implemented.
			// FIXME (Abdel 23/09/2007): this is a bit messy because of the
			// elimination of Inset::dim_ cache. This coordOffset() method needs
			// to be rewritten in light of the new design.
			Dimension const & dim = parMetrics(dit[i - 1].text(),
				dit[i - 1].pit()).insetDimension(&sl.inset());
			lastw = dim.wid;
		} else {
			Dimension const dim = sl.inset().dimension(*this);
			lastw = dim.wid;
		}
		
		//lyxerr << "Cursor::getPos, i: "
		// << i << " x: " << xx << " y: " << y << endl;
	}

	// Add contribution of initial rows of outermost paragraph
	CursorSlice const & sl = dit[0];
	TextMetrics const & tm = textMetrics(sl.text());
	ParagraphMetrics const & pm = tm.parMetrics(sl.pit());
	BOOST_ASSERT(!pm.rows().empty());
	y -= pm.rows()[0].ascent();
#if 1
	// FIXME: document this mess
	size_t rend;
	if (sl.pos() > 0 && dit.depth() == 1) {
		int pos = sl.pos();
		if (pos && boundary)
			--pos;
//		lyxerr << "coordOffset: boundary:" << boundary << " depth:" << dit.depth() << " pos:" << pos << " sl.pos:" << sl.pos() << std::endl;
		rend = pm.pos2row(pos);
	} else
		rend = pm.pos2row(sl.pos());
#else
	size_t rend = pm.pos2row(sl.pos());
#endif
	for (size_t rit = 0; rit != rend; ++rit)
		y += pm.rows()[rit].height();
	y += pm.rows()[rend].ascent();
	
	TextMetrics const & bottom_tm = textMetrics(dit.bottom().text());
	
	// Make relative position from the nested inset now bufferview absolute.
	int xx = bottom_tm.cursorX(dit.bottom(), boundary && dit.depth() == 1);
	x += xx;
	
	// In the RTL case place the nested inset at the left of the cursor in 
	// the outer paragraph
	bool boundary_1 = boundary && 1 == dit.depth();
	bool rtl = bottom_tm.isRTL(dit.bottom(), boundary_1);
	if (rtl)
		x -= lastw;
	
	return Point(x, y);
}


Point BufferView::getPos(DocIterator const & dit, bool boundary) const
{
	CursorSlice const & bot = dit.bottom();
	TextMetrics const & tm = textMetrics(bot.text());
	if (!tm.has(bot.pit()))
		return Point(-1, -1);

	Point p = coordOffset(dit, boundary); // offset from outer paragraph
	p.y_ += tm.parMetrics(bot.pit()).position();
	return p;
}


void BufferView::draw(frontend::Painter & pain)
{
	PainterInfo pi(this, pain);
	// Should the whole screen, including insets, be refreshed?
	// FIXME: We should also distinguish DecorationUpdate to avoid text
	// drawing if possible. This is not possible to do easily right now
	// because of the single backing pixmap.
	pi.full_repaint = metrics_info_.update_strategy != SingleParUpdate;

	if (pi.full_repaint)
		// Clear background (if not delegated to rows)
		pain.fillRectangle(0, metrics_info_.y1, width_,
			metrics_info_.y2 - metrics_info_.y1,
			buffer_.inset().backgroundColor());

	LYXERR(Debug::PAINTING) << "\t\t*** START DRAWING ***" << endl;
	Text & text = buffer_.text();
	TextMetrics const & tm = text_metrics_[&text];
	int y = metrics_info_.y1 + tm.parMetrics(metrics_info_.p1).ascent();
	if (!pi.full_repaint)
		tm.drawParagraph(pi, metrics_info_.p1, 0, y);
	else
		tm.draw(pi, 0, y);
	LYXERR(Debug::PAINTING) << "\n\t\t*** END DRAWING  ***" << endl;

	// and grey out above (should not happen later)
//	lyxerr << "par ascent: " << text.getPar(metrics_info_.p1).ascent() << endl;
	if (metrics_info_.y1 > 0
		&& metrics_info_.update_strategy == FullScreenUpdate)
		pain.fillRectangle(0, 0, width_, metrics_info_.y1, Color::bottomarea);

	// and possibly grey out below
//	lyxerr << "par descent: " << text.getPar(metrics_info_.p1).ascent() << endl;
	if (metrics_info_.y2 < height_
		&& metrics_info_.update_strategy == FullScreenUpdate)
		pain.fillRectangle(0, metrics_info_.y2, width_,
			height_ - metrics_info_.y2, Color::bottomarea);
}


void BufferView::message(docstring const & msg)
{
	if (gui_)
		gui_->message(msg);
}


void BufferView::showDialog(std::string const & name)
{
	if (gui_)
		gui_->showDialog(name);
}


void BufferView::showDialogWithData(std::string const & name,
	std::string const & data)
{
	if (gui_)
		gui_->showDialogWithData(name, data);
}


void BufferView::showInsetDialog(std::string const & name,
	std::string const & data, Inset * inset)
{
	if (gui_)
		gui_->showInsetDialog(name, data, inset);
}


void BufferView::updateDialog(std::string const & name, std::string const & data)
{
	if (gui_)
		gui_->updateDialog(name, data);
}


void BufferView::setGuiDelegate(frontend::GuiBufferViewDelegate * gui)
{
	gui_ = gui;
}


} // namespace lyx
