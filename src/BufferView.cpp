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

#include "BranchList.h"
#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "DispatchResult.h"
#include "ErrorList.h"
#include "factory.h"
#include "FloatList.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Intl.h"
#include "InsetIterator.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "LayoutFile.h"
#include "Length.h"
#include "Lexer.h"
#include "LyX.h"
#include "LyXAction.h"
#include "lyxfind.h"
#include "Layout.h"
#include "LyXRC.h"
#include "MetricsInfo.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "RowPainter.h"
#include "Session.h"
#include "Text.h"
#include "TextClass.h"
#include "TextMetrics.h"
#include "TexRow.h"
#include "TocBackend.h"
#include "WordLangTuple.h"

#include "insets/InsetBibtex.h"
#include "insets/InsetCitation.h"
#include "insets/InsetCommand.h" // ChangeRefs
#include "insets/InsetExternal.h"
#include "insets/InsetGraphics.h"
#include "insets/InsetNote.h"
#include "insets/InsetRef.h"
#include "insets/InsetText.h"

#include "mathed/MathData.h"

#include "frontends/alert.h"
#include "frontends/Application.h"
#include "frontends/Delegates.h"
#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"
#include "frontends/Selection.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/ExceptionMessage.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/Package.h"
#include "support/types.h"

#include <cerrno>
#include <fstream>
#include <functional>
#include <iterator>
#include <sstream>
#include <vector>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = frontend::Alert;

namespace {

/// Return an inset of this class if it exists at the current cursor position
template <class T>
T * getInsetByCode(Cursor const & cur, InsetCode code)
{
	DocIterator it = cur;
	Inset * inset = it.nextInset();
	if (inset && inset->lyxCode() == code)
		return static_cast<T*>(inset);
	return 0;
}


/// Note that comparing contents can only be used for InsetCommand
bool findNextInset(DocIterator & dit, vector<InsetCode> const & codes,
	docstring const & contents)
{
	DocIterator tmpdit = dit;

	while (tmpdit) {
		Inset const * inset = tmpdit.nextInset();
		if (inset) {
			bool const valid_code = std::find(codes.begin(), codes.end(),
				inset->lyxCode()) != codes.end();
			InsetCommand const * ic = inset->asInsetCommand();
			bool const same_or_no_contents =  contents.empty()
				|| (ic && (ic->getFirstNonOptParam() == contents));

			if (valid_code && same_or_no_contents) {
				dit = tmpdit;
				return true;
			}
		}
		tmpdit.forwardInset();
	}

	return false;
}


/// Looks for next inset with one of the given codes.
/// Note that same_content can only be used for InsetCommand
bool findInset(DocIterator & dit, vector<InsetCode> const & codes,
	bool same_content)
{
	docstring contents;
	DocIterator tmpdit = dit;
	tmpdit.forwardInset();
	if (!tmpdit)
		return false;

	Inset const * inset = tmpdit.nextInset();
	if (same_content && inset) {
		InsetCommand const * ic = inset->asInsetCommand();
		if (ic) {
			bool const valid_code = std::find(codes.begin(), codes.end(),
				ic->lyxCode()) != codes.end();
			if (valid_code)
				contents = ic->getFirstNonOptParam();
		}
	}

	if (!findNextInset(tmpdit, codes, contents)) {
		if (dit.depth() != 1 || dit.pit() != 0 || dit.pos() != 0) {
			Inset * inset = &tmpdit.bottom().inset();
			tmpdit = doc_iterator_begin(&inset->buffer(), inset);
			if (!findNextInset(tmpdit, codes, contents))
				return false;
		} else {
			return false;
		}
	}

	dit = tmpdit;
	return true;
}


/// Looks for next inset with the given code
void findInset(DocIterator & dit, InsetCode code, bool same_content)
{
	findInset(dit, vector<InsetCode>(1, code), same_content);
}


/// Moves cursor to the next inset with one of the given codes.
void gotoInset(BufferView * bv, vector<InsetCode> const & codes,
	       bool same_content)
{
	Cursor tmpcur = bv->cursor();
	if (!findInset(tmpcur, codes, same_content)) {
		bv->cursor().message(_("No more insets"));
		return;
	}

	tmpcur.clearSelection();
	bv->setCursor(tmpcur);
	bv->showCursor();
}


/// Moves cursor to the next inset with given code.
void gotoInset(BufferView * bv, InsetCode code, bool same_content)
{
	gotoInset(bv, vector<InsetCode>(1, code), same_content);
}


/// A map from a Text to the associated text metrics
typedef map<Text const *, TextMetrics> TextMetricsCache;

enum ScreenUpdateStrategy {
	NoScreenUpdate,
	SingleParUpdate,
	FullScreenUpdate,
	DecorationUpdate
};

} // anon namespace


/////////////////////////////////////////////////////////////////////
//
// BufferView
//
/////////////////////////////////////////////////////////////////////

struct BufferView::Private
{
	Private(BufferView & bv) : update_strategy_(NoScreenUpdate),
		wh_(0), cursor_(bv),
		anchor_pit_(0), anchor_ypos_(0),
		inlineCompletionUniqueChars_(0),
		last_inset_(0), clickable_inset_(false),
		mouse_position_cache_(),
		bookmark_edit_position_(-1), gui_(0),
		horiz_scroll_offset_(0)
	{
		xsel_cache_.set = false;
	}

	///
	ScrollbarParameters scrollbarParameters_;
	///
	ScreenUpdateStrategy update_strategy_;
	///
	CoordCache coord_cache_;

	/// Estimated average par height for scrollbar.
	int wh_;
	/// this is used to handle XSelection events in the right manner.
	struct {
		CursorSlice cursor;
		CursorSlice anchor;
		bool set;
	} xsel_cache_;
	///
	Cursor cursor_;
	///
	pit_type anchor_pit_;
	///
	int anchor_ypos_;
	///
	vector<int> par_height_;

	///
	DocIterator inlineCompletionPos_;
	///
	docstring inlineCompletion_;
	///
	size_t inlineCompletionUniqueChars_;

	/// keyboard mapping object.
	Intl intl_;

	/// last visited inset.
	/** kept to send setMouseHover(false).
	  * Not owned, so don't delete.
	  */
	Inset const * last_inset_;
	/// are we hovering something that we can click
	bool clickable_inset_;

	/// position of the mouse at the time of the last mouse move
	/// This is used to update the hovering status of inset in
	/// cases where the buffer is scrolled, but the mouse didn't move.
	Point mouse_position_cache_;

	// cache for id of the paragraph which was edited the last time
	int bookmark_edit_position_;

	mutable TextMetricsCache text_metrics_;

	/// Whom to notify.
	/** Not owned, so don't delete.
	  */
	frontend::GuiBufferViewDelegate * gui_;

	/// Cache for Find Next
	FuncRequest search_request_cache_;

	///
	map<string, Inset *> edited_insets_;

	/// When the row where the cursor lies is scrolled, this
	/// contains the scroll offset
	int horiz_scroll_offset_;
	/// a slice pointing to the start of the row where the cursor
	/// is (at last draw time)
	CursorSlice current_row_slice_;
	/// a slice pointing to the start of the row where cursor was
	/// at previous draw event
	CursorSlice last_row_slice_;
};


BufferView::BufferView(Buffer & buf)
	: width_(0), height_(0), full_screen_(false), buffer_(buf),
      d(new Private(*this))
{
	d->xsel_cache_.set = false;
	d->intl_.initKeyMapper(lyxrc.use_kbmap);

	d->cursor_.setBuffer(&buf);
	d->cursor_.push(buffer_.inset());
	d->cursor_.resetAnchor();
	d->cursor_.setCurrentFont();

	buffer_.updatePreviews();
}


BufferView::~BufferView()
{
	// current buffer is going to be switched-off, save cursor pos
	// Ideally, the whole cursor stack should be saved, but session
	// currently can only handle bottom (whole document) level pit and pos.
	// That is to say, if a cursor is in a nested inset, it will be
	// restore to the left of the top level inset.
	LastFilePosSection::FilePos fp;
	fp.pit = d->cursor_.bottom().pit();
	fp.pos = d->cursor_.bottom().pos();
	theSession().lastFilePos().save(buffer_.fileName(), fp);

	if (d->last_inset_)
		d->last_inset_->setMouseHover(this, false);

	delete d;
}


int BufferView::rightMargin() const
{
	// The value used to be hardcoded to 10, which is 2.5mm at 100dpi
	int const default_margin = Length(2.5, Length::MM).inPixels(0);
	// The additional test for the case the outliner is opened.
	if (!full_screen_ || !lyxrc.full_screen_limit
	    || width_ < lyxrc.full_screen_width + 2 * default_margin)
		return default_margin;

	return (width_ - lyxrc.full_screen_width) / 2;
}


int BufferView::leftMargin() const
{
	return rightMargin();
}


bool BufferView::isTopScreen() const
{
	return d->scrollbarParameters_.position == d->scrollbarParameters_.min;
}


bool BufferView::isBottomScreen() const
{
	return d->scrollbarParameters_.position == d->scrollbarParameters_.max;
}


Intl & BufferView::getIntl()
{
	return d->intl_;
}


Intl const & BufferView::getIntl() const
{
	return d->intl_;
}


CoordCache & BufferView::coordCache()
{
	return d->coord_cache_;
}


CoordCache const & BufferView::coordCache() const
{
	return d->coord_cache_;
}


Buffer & BufferView::buffer()
{
	return buffer_;
}


Buffer const & BufferView::buffer() const
{
	return buffer_;
}


bool BufferView::needsFitCursor() const
{
	if (cursorStatus(d->cursor_) == CUR_INSIDE) {
		frontend::FontMetrics const & fm =
			theFontMetrics(d->cursor_.getFont().fontInfo());
		int const asc = fm.maxAscent();
		int const des = fm.maxDescent();
		Point const p = getPos(d->cursor_);
		if (p.y_ - asc >= 0 && p.y_ + des < height_)
			return false;
	}
	return true;
}


void BufferView::processUpdateFlags(Update::flags flags)
{
	// This is close to a hot-path.
	LYXERR(Debug::PAINTING, "BufferView::processUpdateFlags()"
		<< "[fitcursor = " << (flags & Update::FitCursor)
		<< ", forceupdate = " << (flags & Update::Force)
		<< ", singlepar = " << (flags & Update::SinglePar)
		<< "]  buffer: " << &buffer_);

	// FIXME Does this really need doing here? It's done in updateBuffer, and
	// if the Buffer doesn't need updating, then do the macros?
	buffer_.updateMacros();

	// Now do the first drawing step if needed. This consists on updating
	// the CoordCache in updateMetrics().
	// The second drawing step is done in WorkArea::redraw() if needed.
	// FIXME: is this still true now that Buffer::changed() is used all over?

	// Case when no explicit update is requested.
	if (!flags) {
		// no need to redraw anything.
		d->update_strategy_ = NoScreenUpdate;
		return;
	}

	if (flags == Update::Decoration) {
		d->update_strategy_ = DecorationUpdate;
		buffer_.changed(false);
		return;
	}

	if (flags == Update::FitCursor
		|| flags == (Update::Decoration | Update::FitCursor)) {
		// tell the frontend to update the screen if needed.
		if (needsFitCursor()) {
			showCursor();
			return;
		}
		if (flags & Update::Decoration) {
			d->update_strategy_ = DecorationUpdate;
			buffer_.changed(false);
			return;
		}
		// no screen update is needed in principle, but this
		// could change if cursor row needs horizontal scrolling.
		d->update_strategy_ = NoScreenUpdate;
		buffer_.changed(false);
		return;
	}

	bool const full_metrics = flags & Update::Force || !singleParUpdate();

	if (full_metrics)
		// We have to update the full screen metrics.
		updateMetrics();

	if (!(flags & Update::FitCursor)) {
		// Nothing to do anymore. Trigger a redraw and return
		buffer_.changed(false);
		return;
	}

	// updateMetrics() does not update paragraph position
	// This is done at draw() time. So we need a redraw!
	buffer_.changed(false);

	if (needsFitCursor()) {
		// The cursor is off screen so ensure it is visible.
		// refresh it:
		showCursor();
	}

	updateHoveredInset();
}


void BufferView::updateScrollbar()
{
	if (height_ == 0 && width_ == 0)
		return;

	// We prefer fixed size line scrolling.
	d->scrollbarParameters_.single_step = defaultRowHeight();
	// We prefer full screen page scrolling.
	d->scrollbarParameters_.page_step = height_;

	Text & t = buffer_.text();
	TextMetrics & tm = d->text_metrics_[&t];

	LYXERR(Debug::GUI, " Updating scrollbar: height: "
		<< t.paragraphs().size()
		<< " curr par: " << d->cursor_.bottom().pit()
		<< " default height " << defaultRowHeight());

	size_t const parsize = t.paragraphs().size();
	if (d->par_height_.size() != parsize) {
		d->par_height_.clear();
		// FIXME: We assume a default paragraph height of 2 rows. This
		// should probably be pondered with the screen width.
		d->par_height_.resize(parsize, defaultRowHeight() * 2);
	}

	// Look at paragraph heights on-screen
	pair<pit_type, ParagraphMetrics const *> first = tm.first();
	pair<pit_type, ParagraphMetrics const *> last = tm.last();
	for (pit_type pit = first.first; pit <= last.first; ++pit) {
		d->par_height_[pit] = tm.parMetrics(pit).height();
		LYXERR(Debug::SCROLLING, "storing height for pit " << pit << " : "
			<< d->par_height_[pit]);
	}

	int top_pos = first.second->position() - first.second->ascent();
	int bottom_pos = last.second->position() + last.second->descent();
	bool first_visible = first.first == 0 && top_pos >= 0;
	bool last_visible = last.first + 1 == int(parsize) && bottom_pos <= height_;
	if (first_visible && last_visible) {
		d->scrollbarParameters_.min = 0;
		d->scrollbarParameters_.max = 0;
		return;
	}

	d->scrollbarParameters_.min = top_pos;
	for (size_t i = 0; i != size_t(first.first); ++i)
		d->scrollbarParameters_.min -= d->par_height_[i];
	d->scrollbarParameters_.max = bottom_pos;
	for (size_t i = last.first + 1; i != parsize; ++i)
		d->scrollbarParameters_.max += d->par_height_[i];

	d->scrollbarParameters_.position = 0;
	// The reference is the top position so we remove one page.
	if (lyxrc.scroll_below_document)
		d->scrollbarParameters_.max -= minVisiblePart();
	else
		d->scrollbarParameters_.max -= d->scrollbarParameters_.page_step;
}


ScrollbarParameters const & BufferView::scrollbarParameters() const
{
	return d->scrollbarParameters_;
}


docstring BufferView::toolTip(int x, int y) const
{
	// Get inset under mouse, if there is one.
	Inset const * covering_inset = getCoveringInset(buffer_.text(), x, y);
	if (!covering_inset)
		// No inset, no tooltip...
		return docstring();
	return covering_inset->toolTip(*this, x, y);
}


string BufferView::contextMenu(int x, int y) const
{
	//If there is a selection, return the containing inset menu
	if (d->cursor_.selection())
		return d->cursor_.inset().contextMenu(*this, x, y);

	// Get inset under mouse, if there is one.
	Inset const * covering_inset = getCoveringInset(buffer_.text(), x, y);
	if (covering_inset)
		return covering_inset->contextMenu(*this, x, y);

	return buffer_.inset().contextMenu(*this, x, y);
}


void BufferView::scrollDocView(int value, bool update)
{
	int const offset = value - d->scrollbarParameters_.position;

	// No scrolling at all? No need to redraw anything
	if (offset == 0)
		return;

	// If the offset is less than 2 screen height, prefer to scroll instead.
	if (abs(offset) <= 2 * height_) {
		d->anchor_ypos_ -= offset;
		buffer_.changed(true);
		updateHoveredInset();
		return;
	}

	// cut off at the top
	if (value <= d->scrollbarParameters_.min) {
		DocIterator dit = doc_iterator_begin(&buffer_);
		showCursor(dit, false, update);
		LYXERR(Debug::SCROLLING, "scroll to top");
		return;
	}

	// cut off at the bottom
	if (value >= d->scrollbarParameters_.max) {
		DocIterator dit = doc_iterator_end(&buffer_);
		dit.backwardPos();
		showCursor(dit, false, update);
		LYXERR(Debug::SCROLLING, "scroll to bottom");
		return;
	}

	// find paragraph at target position
	int par_pos = d->scrollbarParameters_.min;
	pit_type i = 0;
	for (; i != int(d->par_height_.size()); ++i) {
		par_pos += d->par_height_[i];
		if (par_pos >= value)
			break;
	}

	if (par_pos < value) {
		// It seems we didn't find the correct pit so stay on the safe side and
		// scroll to bottom.
		LYXERR0("scrolling position not found!");
		scrollDocView(d->scrollbarParameters_.max, update);
		return;
	}

	DocIterator dit = doc_iterator_begin(&buffer_);
	dit.pit() = i;
	LYXERR(Debug::SCROLLING, "value = " << value << " -> scroll to pit " << i);
	showCursor(dit, false, update);
}


// FIXME: this method is not working well.
void BufferView::setCursorFromScrollbar()
{
	TextMetrics & tm = d->text_metrics_[&buffer_.text()];

	int const height = 2 * defaultRowHeight();
	int const first = height;
	int const last = height_ - height;
	int newy = 0;
	Cursor const & oldcur = d->cursor_;

	switch (cursorStatus(oldcur)) {
	case CUR_ABOVE:
		newy = first;
		break;
	case CUR_BELOW:
		newy = last;
		break;
	case CUR_INSIDE:
		int const y = getPos(oldcur).y_;
		newy = min(last, max(y, first));
		if (y == newy)
			return;
	}
	// We reset the cursor because cursorStatus() does not
	// work when the cursor is within mathed.
	Cursor cur(*this);
	cur.reset();
	tm.setCursorFromCoordinates(cur, 0, newy);

	// update the bufferview cursor and notify insets
	// FIXME: Care about the d->cursor_ flags to redraw if needed
	Cursor old = d->cursor_;
	mouseSetCursor(cur);
	// the DEPM call in mouseSetCursor() might have destroyed the
	// paragraph the cursor is in.
	bool badcursor = old.fixIfBroken();
	badcursor |= notifyCursorLeavesOrEnters(old, d->cursor_);
	if (badcursor)
		d->cursor_.fixIfBroken();
}


Change const BufferView::getCurrentChange() const
{
	if (!d->cursor_.selection())
		return Change(Change::UNCHANGED);

	DocIterator dit = d->cursor_.selectionBegin();
	// The selected content might have been changed (see #7685)
	dit = dit.getInnerText();
	return dit.paragraph().lookupChange(dit.pos());
}


// this could be used elsewhere as well?
// FIXME: This does not work within mathed!
CursorStatus BufferView::cursorStatus(DocIterator const & dit) const
{
	Point const p = getPos(dit);
	if (p.y_ < 0)
		return CUR_ABOVE;
	if (p.y_ > workHeight())
		return CUR_BELOW;
	return CUR_INSIDE;
}


void BufferView::bookmarkEditPosition()
{
	// Don't eat cpu time for each keystroke
	if (d->cursor_.paragraph().id() == d->bookmark_edit_position_)
		return;
	saveBookmark(0);
	d->bookmark_edit_position_ = d->cursor_.paragraph().id();
}


void BufferView::saveBookmark(unsigned int idx)
{
	// tentatively save bookmark, id and pos will be used to
	// acturately locate a bookmark in a 'live' lyx session.
	// pit and pos will be updated with bottom level pit/pos
	// when lyx exits.
	if (!buffer_.isInternal()) {
		theSession().bookmarks().save(
			buffer_.fileName(),
			d->cursor_.bottom().pit(),
			d->cursor_.bottom().pos(),
			d->cursor_.paragraph().id(),
			d->cursor_.pos(),
			idx
			);
		if (idx)
			// emit message signal.
			message(_("Save bookmark"));
	}
}


bool BufferView::moveToPosition(pit_type bottom_pit, pos_type bottom_pos,
	int top_id, pos_type top_pos)
{
	bool success = false;
	DocIterator dit;

	d->cursor_.clearSelection();

	// if a valid par_id is given, try it first
	// This is the case for a 'live' bookmark when unique paragraph ID
	// is used to track bookmarks.
	if (top_id > 0) {
		dit = buffer_.getParFromID(top_id);
		if (!dit.atEnd()) {
			dit.pos() = min(dit.paragraph().size(), top_pos);
			// Some slices of the iterator may not be
			// reachable (e.g. closed collapsable inset)
			// so the dociterator may need to be
			// shortened. Otherwise, setCursor may crash
			// lyx when the cursor can not be set to these
			// insets.
			size_t const n = dit.depth();
			for (size_t i = 0; i < n; ++i)
				if (!dit[i].inset().editable()) {
					dit.resize(i);
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
	if (bottom_pit < int(buffer_.paragraphs().size())) {
		dit = doc_iterator_begin(&buffer_);

		dit.pit() = bottom_pit;
		dit.pos() = min(bottom_pos, dit.paragraph().size());
		success = true;
	}

	if (success) {
		// Note: only bottom (document) level pit is set.
		setCursor(dit);
		// set the current font.
		d->cursor_.setCurrentFont();
		// Do not forget to reset the anchor (see #9912)
		d->cursor_.resetAnchor();
		// To center the screen on this new position we need the
		// paragraph position which is computed at draw() time.
		// So we need a redraw!
		buffer_.changed(false);
		if (needsFitCursor())
			showCursor();
	}

	return success;
}


void BufferView::translateAndInsert(char_type c, Text * t, Cursor & cur)
{
	if (d->cursor_.real_current_font.isRightToLeft()) {
		if (d->intl_.keymap == Intl::PRIMARY)
			d->intl_.keyMapSec();
	} else {
		if (d->intl_.keymap == Intl::SECONDARY)
			d->intl_.keyMapPrim();
	}

	d->intl_.getTransManager().translateAndInsert(c, t, cur);
}


int BufferView::workWidth() const
{
	return width_;
}


void BufferView::recenter()
{
	showCursor(d->cursor_, true, true);
}


void BufferView::showCursor()
{
	showCursor(d->cursor_, false, true);
}


void BufferView::showCursor(DocIterator const & dit,
	bool recenter, bool update)
{
	if (scrollToCursor(dit, recenter) && update) {
		buffer_.changed(true);
		updateHoveredInset();
	}
}


void BufferView::scrollToCursor()
{
	if (scrollToCursor(d->cursor_, false)) {
		buffer_.changed(true);
		updateHoveredInset();
	}
}


bool BufferView::scrollToCursor(DocIterator const & dit, bool recenter)
{
	// We are not properly started yet, delay until resizing is
	// done.
	if (height_ == 0)
		return false;

	LYXERR(Debug::SCROLLING, "recentering!");

	CursorSlice const & bot = dit.bottom();
	TextMetrics & tm = d->text_metrics_[bot.text()];

	pos_type const max_pit = pos_type(bot.text()->paragraphs().size() - 1);
	int bot_pit = bot.pit();
	if (bot_pit > max_pit) {
		// FIXME: Why does this happen?
		LYXERR0("bottom pit is greater that max pit: "
			<< bot_pit << " > " << max_pit);
		bot_pit = max_pit;
	}

	if (bot_pit == tm.first().first - 1)
		tm.newParMetricsUp();
	else if (bot_pit == tm.last().first + 1)
		tm.newParMetricsDown();

	if (tm.contains(bot_pit)) {
		ParagraphMetrics const & pm = tm.parMetrics(bot_pit);
		LBUFERR(!pm.rows().empty());
		// FIXME: smooth scrolling doesn't work in mathed.
		CursorSlice const & cs = dit.innerTextSlice();
		int offset = coordOffset(dit).y_;
		int ypos = pm.position() + offset;
		Dimension const & row_dim =
			pm.getRow(cs.pos(), dit.boundary()).dimension();
		int scrolled = 0;
		if (recenter)
			scrolled = scroll(ypos - height_/2);

		// We try to visualize the whole row, if the row height is larger than
		// the screen height, we scroll to a heuristic value of height_ / 4.
		// FIXME: This heuristic value should be replaced by a recursive search
		// for a row in the inset that can be visualized completely.
		else if (row_dim.height() > height_) {
			if (ypos < defaultRowHeight())
				scrolled = scroll(ypos - height_ / 4);
			else if (ypos > height_ - defaultRowHeight())
				scrolled = scroll(ypos - 3 * height_ / 4);
		}

		// If the top part of the row falls of the screen, we scroll
		// up to align the top of the row with the top of the screen.
		else if (ypos - row_dim.ascent() < 0 && ypos < height_) {
			int ynew = row_dim.ascent();
			scrolled = scrollUp(ynew - ypos);
		}

		// If the bottom of the row falls of the screen, we scroll down.
		else if (ypos + row_dim.descent() > height_ && ypos > 0) {
			int ynew = height_ - row_dim.descent();
			scrolled = scrollDown(ypos - ynew);
		}

		// else, nothing to do, the cursor is already visible so we just return.
		return scrolled != 0;
	}

	// fix inline completion position
	if (d->inlineCompletionPos_.fixIfBroken())
		d->inlineCompletionPos_ = DocIterator();

	tm.redoParagraph(bot_pit);
	ParagraphMetrics const & pm = tm.parMetrics(bot_pit);
	int offset = coordOffset(dit).y_;

	d->anchor_pit_ = bot_pit;
	CursorSlice const & cs = dit.innerTextSlice();
	Dimension const & row_dim =
		pm.getRow(cs.pos(), dit.boundary()).dimension();

	if (recenter)
		d->anchor_ypos_ = height_/2;
	else if (d->anchor_pit_ == 0)
		d->anchor_ypos_ = offset + pm.ascent();
	else if (d->anchor_pit_ == max_pit)
		d->anchor_ypos_ = height_ - offset - row_dim.descent();
	else if (offset > height_)
		d->anchor_ypos_ = height_ - offset - defaultRowHeight();
	else
		d->anchor_ypos_ = defaultRowHeight() * 2;

	return true;
}


void BufferView::makeDocumentClass()
{
	DocumentClassConstPtr olddc = buffer_.params().documentClassPtr();
	buffer_.params().makeDocumentClass();
	updateDocumentClass(olddc);
}


void BufferView::updateDocumentClass(DocumentClassConstPtr olddc)
{
	message(_("Converting document to new document class..."));

	StableDocIterator backcur(d->cursor_);
	ErrorList & el = buffer_.errorList("Class Switch");
	cap::switchBetweenClasses(
			olddc, buffer_.params().documentClassPtr(),
			static_cast<InsetText &>(buffer_.inset()), el);

	setCursor(backcur.asDocIterator(&buffer_));

	buffer_.errors("Class Switch");
}


/** Return the change status at cursor position, taking into account the
 * status at each level of the document iterator (a table in a deleted
 * footnote is deleted).
 * When \param outer is true, the top slice is not looked at.
 */
static Change::Type lookupChangeType(DocIterator const & dit, bool outer = false)
{
	size_t const depth = dit.depth() - (outer ? 1 : 0);

	for (size_t i = 0 ; i < depth ; ++i) {
		CursorSlice const & slice = dit[i];
		if (!slice.inset().inMathed()
		    && slice.pos() < slice.paragraph().size()) {
			Change::Type const ch = slice.paragraph().lookupChange(slice.pos()).type;
			if (ch != Change::UNCHANGED)
				return ch;
		}
	}
	return Change::UNCHANGED;
}


bool BufferView::getStatus(FuncRequest const & cmd, FuncStatus & flag)
{
	FuncCode const act = cmd.action();

	// Can we use a readonly buffer?
	if (buffer_.isReadonly()
	    && !lyxaction.funcHasFlag(act, LyXAction::ReadOnly)
	    && !lyxaction.funcHasFlag(act, LyXAction::NoBuffer)) {
		flag.message(from_utf8(N_("Document is read-only")));
		flag.setEnabled(false);
		return true;
	}

	// Are we in a DELETED change-tracking region?
	if (lookupChangeType(d->cursor_, true) == Change::DELETED
	    && !lyxaction.funcHasFlag(act, LyXAction::ReadOnly)
	    && !lyxaction.funcHasFlag(act, LyXAction::NoBuffer)) {
		flag.message(from_utf8(N_("This portion of the document is deleted.")));
		flag.setEnabled(false);
		return true;
	}

	Cursor & cur = d->cursor_;

	if (cur.getStatus(cmd, flag))
		return true;

	switch (act) {

	// FIXME: This is a bit problematic because we don't check if this is
	// a document BufferView or not for these LFUNs. We probably have to
	// dispatch both to currentBufferView() and, if that fails,
	// to documentBufferView(); same as we do now for current Buffer and
	// document Buffer. Ideally those LFUN should go to Buffer as they
	// operate on the full Buffer and the cursor is only needed either for
	// an Undo record or to restore a cursor position. But we don't know
	// how to do that inside Buffer of course.
	case LFUN_BUFFER_PARAMS_APPLY:
	case LFUN_LAYOUT_MODULES_CLEAR:
	case LFUN_LAYOUT_MODULE_ADD:
	case LFUN_LAYOUT_RELOAD:
	case LFUN_TEXTCLASS_APPLY:
	case LFUN_TEXTCLASS_LOAD:
		flag.setEnabled(!buffer_.isReadonly());
		break;

	case LFUN_UNDO:
		// We do not use the LyXAction flag for readonly because Undo sets the
		// buffer clean/dirty status by itself.
		flag.setEnabled(!buffer_.isReadonly() && buffer_.undo().hasUndoStack());
		break;
	case LFUN_REDO:
		// We do not use the LyXAction flag for readonly because Redo sets the
		// buffer clean/dirty status by itself.
		flag.setEnabled(!buffer_.isReadonly() && buffer_.undo().hasRedoStack());
		break;
	case LFUN_FILE_INSERT_PLAINTEXT_PARA:
	case LFUN_FILE_INSERT_PLAINTEXT: {
		docstring const fname = cmd.argument();
		if (!FileName::isAbsolute(to_utf8(fname))) {
			flag.message(_("Absolute filename expected."));
			return false;
		}
		flag.setEnabled(cur.inTexted());
		break;
	}
	case LFUN_FILE_INSERT:
	case LFUN_BOOKMARK_SAVE:
		// FIXME: Actually, these LFUNS should be moved to Text
		flag.setEnabled(cur.inTexted());
		break;

	case LFUN_FONT_STATE:
	case LFUN_LABEL_INSERT:
	case LFUN_INFO_INSERT:
	case LFUN_PARAGRAPH_GOTO:
	case LFUN_NOTE_NEXT:
	case LFUN_REFERENCE_NEXT:
	case LFUN_WORD_FIND:
	case LFUN_WORD_FIND_FORWARD:
	case LFUN_WORD_FIND_BACKWARD:
	case LFUN_WORD_REPLACE:
	case LFUN_MARK_OFF:
	case LFUN_MARK_ON:
	case LFUN_MARK_TOGGLE:
	case LFUN_SCREEN_RECENTER:
	case LFUN_SCREEN_SHOW_CURSOR:
	case LFUN_BIBTEX_DATABASE_ADD:
	case LFUN_BIBTEX_DATABASE_DEL:
	case LFUN_STATISTICS:
	case LFUN_KEYMAP_OFF:
	case LFUN_KEYMAP_PRIMARY:
	case LFUN_KEYMAP_SECONDARY:
	case LFUN_KEYMAP_TOGGLE:
	case LFUN_INSET_SELECT_ALL:
		flag.setEnabled(true);
		break;

	case LFUN_WORD_FINDADV: {
		FindAndReplaceOptions opt;
		istringstream iss(to_utf8(cmd.argument()));
		iss >> opt;
		flag.setEnabled(opt.repl_buf_name.empty()
				|| !buffer_.isReadonly());
		break;
	}

	case LFUN_LABEL_GOTO:
		flag.setEnabled(!cmd.argument().empty()
		    || getInsetByCode<InsetRef>(cur, REF_CODE));
		break;

	case LFUN_CHANGES_MERGE:
	case LFUN_CHANGE_NEXT:
	case LFUN_CHANGE_PREVIOUS:
	case LFUN_ALL_CHANGES_ACCEPT:
	case LFUN_ALL_CHANGES_REJECT:
		flag.setEnabled(buffer_.areChangesPresent());
		break;

	case LFUN_SCREEN_UP:
	case LFUN_SCREEN_DOWN:
	case LFUN_SCROLL:
	case LFUN_SCREEN_UP_SELECT:
	case LFUN_SCREEN_DOWN_SELECT:
	case LFUN_INSET_FORALL:
		flag.setEnabled(true);
		break;

	case LFUN_LAYOUT_TABULAR:
		flag.setEnabled(cur.innerInsetOfType(TABULAR_CODE));
		break;

	case LFUN_LAYOUT:
		flag.setEnabled(!cur.inset().forcePlainLayout(cur.idx()));
		break;

	case LFUN_LAYOUT_PARAGRAPH:
		flag.setEnabled(cur.inset().allowParagraphCustomization(cur.idx()));
		break;

	case LFUN_BRANCH_ADD_INSERT:
		flag.setEnabled(!(cur.inTexted() && cur.paragraph().isPassThru()));
		break;

	case LFUN_DIALOG_SHOW_NEW_INSET:
		// FIXME: this is wrong, but I do not understand the
		// intent (JMarc)
		if (cur.inset().lyxCode() == CAPTION_CODE)
			return cur.inset().getStatus(cur, cmd, flag);
		// FIXME we should consider passthru paragraphs too.
		flag.setEnabled(!(cur.inTexted() && cur.paragraph().isPassThru()));
		break;

	case LFUN_CITATION_INSERT: {
		FuncRequest fr(LFUN_INSET_INSERT, "citation");
		// FIXME: This could turn in a recursive hell.
		// Shouldn't we use Buffer::getStatus() instead?
		flag.setEnabled(lyx::getStatus(fr).enabled());
		break;
	}
	case LFUN_INSET_APPLY: {
		string const name = cmd.getArg(0);
		Inset * inset = editedInset(name);
		if (inset) {
			FuncRequest fr(LFUN_INSET_MODIFY, cmd.argument());
			if (!inset->getStatus(cur, fr, flag)) {
				// Every inset is supposed to handle this
				LASSERT(false, break);
			}
		} else {
			FuncRequest fr(LFUN_INSET_INSERT, cmd.argument());
			flag = lyx::getStatus(fr);
		}
		break;
	}

	default:
		return false;
	}

	return true;
}


Inset * BufferView::editedInset(string const & name) const
{
	map<string, Inset *>::const_iterator it = d->edited_insets_.find(name);
	return it == d->edited_insets_.end() ? 0 : it->second;
}


void BufferView::editInset(string const & name, Inset * inset)
{
	d->edited_insets_[name] = inset;
}


void BufferView::dispatch(FuncRequest const & cmd, DispatchResult & dr)
{
	LYXERR(Debug::ACTION, "BufferView::dispatch: cmd: " << cmd);

	string const argument = to_utf8(cmd.argument());
	Cursor & cur = d->cursor_;
	Cursor old = cur;

	// Don't dispatch function that does not apply to internal buffers.
	if (buffer_.isInternal()
	    && lyxaction.funcHasFlag(cmd.action(), LyXAction::NoInternal))
		return;

	// We'll set this back to false if need be.
	bool dispatched = true;
	buffer_.undo().beginUndoGroup();

	FuncCode const act = cmd.action();
	switch (act) {

	case LFUN_BUFFER_PARAMS_APPLY: {
		DocumentClassConstPtr olddc = buffer_.params().documentClassPtr();
		cur.recordUndoBufferParams();
		istringstream ss(to_utf8(cmd.argument()));
		Lexer lex;
		lex.setStream(ss);
		int const unknown_tokens = buffer_.readHeader(lex);
		if (unknown_tokens != 0) {
			LYXERR0("Warning in LFUN_BUFFER_PARAMS_APPLY!\n"
						<< unknown_tokens << " unknown token"
						<< (unknown_tokens == 1 ? "" : "s"));
		}
		updateDocumentClass(olddc);

		// We are most certainly here because of a change in the document
		// It is then better to make sure that all dialogs are in sync with
		// current document settings.
		dr.screenUpdate(Update::Force | Update::FitCursor);
		dr.forceBufferUpdate();
		break;
	}

	case LFUN_LAYOUT_MODULES_CLEAR: {
		// FIXME: this modifies the document in cap::switchBetweenClasses
		//  without calling recordUndo. Fix this before using
		//  recordUndoBufferParams().
		cur.recordUndoFullBuffer();
		buffer_.params().clearLayoutModules();
		makeDocumentClass();
		dr.screenUpdate(Update::Force);
		dr.forceBufferUpdate();
		break;
	}

	case LFUN_LAYOUT_MODULE_ADD: {
		BufferParams const & params = buffer_.params();
		if (!params.layoutModuleCanBeAdded(argument)) {
			LYXERR0("Module `" << argument <<
				"' cannot be added due to failed requirements or "
				"conflicts with installed modules.");
			break;
		}
		// FIXME: this modifies the document in cap::switchBetweenClasses
		//  without calling recordUndo. Fix this before using
		//  recordUndoBufferParams().
		cur.recordUndoFullBuffer();
		buffer_.params().addLayoutModule(argument);
		makeDocumentClass();
		dr.screenUpdate(Update::Force);
		dr.forceBufferUpdate();
		break;
	}

	case LFUN_TEXTCLASS_APPLY: {
		// since this shortcircuits, the second call is made only if
		// the first fails
		bool const success =
			LayoutFileList::get().load(argument, buffer_.temppath()) ||
			LayoutFileList::get().load(argument, buffer_.filePath());
		if (!success) {
			docstring s = bformat(_("The document class `%1$s' "
						 "could not be loaded."), from_utf8(argument));
			frontend::Alert::error(_("Could not load class"), s);
			break;
		}

		LayoutFile const * old_layout = buffer_.params().baseClass();
		LayoutFile const * new_layout = &(LayoutFileList::get()[argument]);

		if (old_layout == new_layout)
			// nothing to do
			break;

		// Save the old, possibly modular, layout for use in conversion.
		// FIXME: this modifies the document in cap::switchBetweenClasses
		//  without calling recordUndo. Fix this before using
		//  recordUndoBufferParams().
		cur.recordUndoFullBuffer();
		buffer_.params().setBaseClass(argument);
		makeDocumentClass();
		dr.screenUpdate(Update::Force);
		dr.forceBufferUpdate();
		break;
	}

	case LFUN_TEXTCLASS_LOAD: {
		// since this shortcircuits, the second call is made only if
		// the first fails
		bool const success =
			LayoutFileList::get().load(argument, buffer_.temppath()) ||
			LayoutFileList::get().load(argument, buffer_.filePath());
		if (!success) {
			docstring s = bformat(_("The document class `%1$s' "
						 "could not be loaded."), from_utf8(argument));
			frontend::Alert::error(_("Could not load class"), s);
		}
		break;
	}

	case LFUN_LAYOUT_RELOAD: {
		LayoutFileIndex bc = buffer_.params().baseClassID();
		LayoutFileList::get().reset(bc);
		buffer_.params().setBaseClass(bc);
		makeDocumentClass();
		dr.screenUpdate(Update::Force);
		dr.forceBufferUpdate();
		break;
	}

	case LFUN_UNDO:
		dr.setMessage(_("Undo"));
		cur.clearSelection();
		if (!cur.textUndo())
			dr.setMessage(_("No further undo information"));
		else
			dr.screenUpdate(Update::Force | Update::FitCursor);
		dr.forceBufferUpdate();
		break;

	case LFUN_REDO:
		dr.setMessage(_("Redo"));
		cur.clearSelection();
		if (!cur.textRedo())
			dr.setMessage(_("No further redo information"));
		else
			dr.screenUpdate(Update::Force | Update::FitCursor);
		dr.forceBufferUpdate();
		break;

	case LFUN_FONT_STATE:
		dr.setMessage(cur.currentState());
		break;

	case LFUN_BOOKMARK_SAVE:
		saveBookmark(convert<unsigned int>(to_utf8(cmd.argument())));
		break;

	case LFUN_LABEL_GOTO: {
		docstring label = cmd.argument();
		if (label.empty()) {
			InsetRef * inset =
				getInsetByCode<InsetRef>(cur, REF_CODE);
			if (inset) {
				label = inset->getParam("reference");
				// persistent=false: use temp_bookmark
				saveBookmark(0);
			}
		}
		if (!label.empty()) {
			gotoLabel(label);
			// at the moment, this is redundant, since gotoLabel will
			// eventually call LFUN_PARAGRAPH_GOTO, but it seems best
			// to have it here.
			dr.screenUpdate(Update::Force | Update::FitCursor);
		}
		break;
	}

	case LFUN_PARAGRAPH_GOTO: {
		int const id = convert<int>(cmd.getArg(0));
		int const pos = convert<int>(cmd.getArg(1));
		int i = 0;
		for (Buffer * b = &buffer_; i == 0 || b != &buffer_;
			b = theBufferList().next(b)) {

			Cursor cur(*this);
			cur.setCursor(b->getParFromID(id));
			if (cur.atEnd()) {
				LYXERR(Debug::INFO, "No matching paragraph found! [" << id << "].");
				++i;
				continue;
			}
			LYXERR(Debug::INFO, "Paragraph " << cur.paragraph().id()
				<< " found in buffer `"
				<< b->absFileName() << "'.");

			if (b == &buffer_) {
				// Set the cursor
				cur.pos() = pos;
				mouseSetCursor(cur);
				dr.screenUpdate(Update::Force | Update::FitCursor);
			} else {
				// Switch to other buffer view and resend cmd
				lyx::dispatch(FuncRequest(
					LFUN_BUFFER_SWITCH, b->absFileName()));
				lyx::dispatch(cmd);
			}
			break;
		}
		break;
	}

	case LFUN_NOTE_NEXT:
		gotoInset(this, NOTE_CODE, false);
		break;

	case LFUN_REFERENCE_NEXT: {
		vector<InsetCode> tmp;
		tmp.push_back(LABEL_CODE);
		tmp.push_back(REF_CODE);
		gotoInset(this, tmp, true);
		break;
	}

	case LFUN_CHANGE_NEXT:
		findNextChange(this);
		// FIXME: Move this LFUN to Buffer so that we don't have to do this:
		dr.screenUpdate(Update::Force | Update::FitCursor);
		break;

	case LFUN_CHANGE_PREVIOUS:
		findPreviousChange(this);
		// FIXME: Move this LFUN to Buffer so that we don't have to do this:
		dr.screenUpdate(Update::Force | Update::FitCursor);
		break;

	case LFUN_CHANGES_MERGE:
		if (findNextChange(this) || findPreviousChange(this)) {
			dr.screenUpdate(Update::Force | Update::FitCursor);
			dr.forceBufferUpdate();
			showDialog("changes");
		}
		break;

	case LFUN_ALL_CHANGES_ACCEPT:
		// select complete document
		cur.reset();
		cur.selHandle(true);
		buffer_.text().cursorBottom(cur);
		// accept everything in a single step to support atomic undo
		buffer_.text().acceptOrRejectChanges(cur, Text::ACCEPT);
		cur.resetAnchor();
		// FIXME: Move this LFUN to Buffer so that we don't have to do this:
		dr.screenUpdate(Update::Force | Update::FitCursor);
		dr.forceBufferUpdate();
		break;

	case LFUN_ALL_CHANGES_REJECT:
		// select complete document
		cur.reset();
		cur.selHandle(true);
		buffer_.text().cursorBottom(cur);
		// reject everything in a single step to support atomic undo
		// Note: reject does not work recursively; the user may have to repeat the operation
		buffer_.text().acceptOrRejectChanges(cur, Text::REJECT);
		cur.resetAnchor();
		// FIXME: Move this LFUN to Buffer so that we don't have to do this:
		dr.screenUpdate(Update::Force | Update::FitCursor);
		dr.forceBufferUpdate();
		break;

	case LFUN_WORD_FIND_FORWARD:
	case LFUN_WORD_FIND_BACKWARD: {
		// FIXME THREAD
		// Would it maybe be better if this variable were view specific anyway?
		static docstring last_search;
		docstring searched_string;

		if (!cmd.argument().empty()) {
			last_search = cmd.argument();
			searched_string = cmd.argument();
		} else {
			searched_string = last_search;
		}

		if (searched_string.empty())
			break;

		bool const fw = act == LFUN_WORD_FIND_FORWARD;
		docstring const data =
			find2string(searched_string, true, false, fw);
		bool found = lyxfind(this, FuncRequest(LFUN_WORD_FIND, data));
		if (found) {
			dr.screenUpdate(Update::Force | Update::FitCursor);
			cur.dispatched();
			dispatched = true;
		} else {
			cur.undispatched();
			dispatched = false;
		}
		break;
	}

	case LFUN_WORD_FIND: {
		FuncRequest req = cmd;
		if (cmd.argument().empty() && !d->search_request_cache_.argument().empty())
			req = d->search_request_cache_;
		if (req.argument().empty()) {
			lyx::dispatch(FuncRequest(LFUN_DIALOG_SHOW, "findreplace"));
			break;
		}
		if (lyxfind(this, req)) {
			dr.screenUpdate(Update::Force | Update::FitCursor);
			cur.dispatched();
			dispatched = true;
		} else {
			cur.undispatched();
			dispatched = false;
		}
		d->search_request_cache_ = req;
		break;
	}

	case LFUN_WORD_REPLACE: {
		bool has_deleted = false;
		if (cur.selection()) {
			DocIterator beg = cur.selectionBegin();
			DocIterator end = cur.selectionEnd();
			if (beg.pit() == end.pit()) {
				for (pos_type p = beg.pos() ; p < end.pos() ; ++p) {
					if (!cur.inMathed() && cur.paragraph().isDeleted(p)) {
						has_deleted = true;
						break;
					}
				}
			}
		}
		if (lyxreplace(this, cmd, has_deleted)) {
			dr.forceBufferUpdate();
			dr.screenUpdate(Update::Force | Update::FitCursor);
			cur.dispatched();
			dispatched = true;
		} else {
			cur.undispatched();
			dispatched = false;
		}
		break;
	}

	case LFUN_WORD_FINDADV: {
		FindAndReplaceOptions opt;
		istringstream iss(to_utf8(cmd.argument()));
		iss >> opt;
		if (findAdv(this, opt)) {
			dr.screenUpdate(Update::Force | Update::FitCursor);
			cur.dispatched();
			dispatched = true;
		} else {
			cur.undispatched();
			dispatched = false;
		}
		break;
	}

	case LFUN_MARK_OFF:
		cur.clearSelection();
		dr.setMessage(from_utf8(N_("Mark off")));
		break;

	case LFUN_MARK_ON:
		cur.clearSelection();
		cur.setMark(true);
		dr.setMessage(from_utf8(N_("Mark on")));
		break;

	case LFUN_MARK_TOGGLE:
		cur.selection(false);
		if (cur.mark()) {
			cur.setMark(false);
			dr.setMessage(from_utf8(N_("Mark removed")));
		} else {
			cur.setMark(true);
			dr.setMessage(from_utf8(N_("Mark set")));
		}
		cur.resetAnchor();
		break;

	case LFUN_SCREEN_SHOW_CURSOR:
		showCursor();
		break;

	case LFUN_SCREEN_RECENTER:
		recenter();
		break;

	case LFUN_BIBTEX_DATABASE_ADD: {
		Cursor tmpcur = cur;
		findInset(tmpcur, BIBTEX_CODE, false);
		InsetBibtex * inset = getInsetByCode<InsetBibtex>(tmpcur,
						BIBTEX_CODE);
		if (inset) {
			if (inset->addDatabase(cmd.argument())) {
				buffer_.invalidateBibfileCache();
				dr.forceBufferUpdate();
			}
		}
		break;
	}

	case LFUN_BIBTEX_DATABASE_DEL: {
		Cursor tmpcur = cur;
		findInset(tmpcur, BIBTEX_CODE, false);
		InsetBibtex * inset = getInsetByCode<InsetBibtex>(tmpcur,
						BIBTEX_CODE);
		if (inset) {
			if (inset->delDatabase(cmd.argument())) {
				buffer_.invalidateBibfileCache();
				dr.forceBufferUpdate();
			}
		}
		break;
	}

	case LFUN_STATISTICS: {
		DocIterator from, to;
		if (cur.selection()) {
			from = cur.selectionBegin();
			to = cur.selectionEnd();
		} else {
			from = doc_iterator_begin(&buffer_);
			to = doc_iterator_end(&buffer_);
		}
		buffer_.updateStatistics(from, to);
		int const words = buffer_.wordCount();
		int const chars = buffer_.charCount(false);
		int const chars_blanks = buffer_.charCount(true);
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

	case LFUN_SCREEN_UP:
	case LFUN_SCREEN_DOWN: {
		Point p = getPos(cur);
		// This code has been commented out to enable to scroll down a
		// document, even if there are large insets in it (see bug #5465).
		/*if (p.y_ < 0 || p.y_ > height_) {
			// The cursor is off-screen so recenter before proceeding.
			showCursor();
			p = getPos(cur);
		}*/
		int const scrolled = scroll(act == LFUN_SCREEN_UP
			? -height_ : height_);
		if (act == LFUN_SCREEN_UP && scrolled > -height_)
			p = Point(0, 0);
		if (act == LFUN_SCREEN_DOWN && scrolled < height_)
			p = Point(width_, height_);
		bool const in_texted = cur.inTexted();
		cur.setCursor(doc_iterator_begin(cur.buffer()));
		cur.selHandle(false);
		buffer_.changed(true);
		updateHoveredInset();

		d->text_metrics_[&buffer_.text()].editXY(cur, p.x_, p.y_,
			true, act == LFUN_SCREEN_UP);
		//FIXME: what to do with cur.x_target()?
		bool update = in_texted && cur.bv().checkDepm(cur, old);
		cur.finishUndo();

		if (update || cur.mark())
			dr.screenUpdate(Update::Force | Update::FitCursor);
		if (update)
			dr.forceBufferUpdate();
		break;
	}

	case LFUN_SCROLL: {
		string const scroll_type = cmd.getArg(0);
		int scroll_step = 0;
		if (scroll_type == "line")
			scroll_step = d->scrollbarParameters_.single_step;
		else if (scroll_type == "page")
			scroll_step = d->scrollbarParameters_.page_step;
		else
			return;
		string const scroll_quantity = cmd.getArg(1);
		if (scroll_quantity == "up")
			scrollUp(scroll_step);
		else if (scroll_quantity == "down")
			scrollDown(scroll_step);
		else {
			int const scroll_value = convert<int>(scroll_quantity);
			if (scroll_value)
				scroll(scroll_step * scroll_value);
		}
		buffer_.changed(true);
		updateHoveredInset();
		dr.forceBufferUpdate();
		break;
	}

	case LFUN_SCREEN_UP_SELECT: {
		// FIXME: why is the algorithm different from LFUN_SCREEN_UP?
		cur.selHandle(true);
		if (isTopScreen()) {
			lyx::dispatch(FuncRequest(LFUN_BUFFER_BEGIN_SELECT));
			cur.finishUndo();
			break;
		}
		int y = getPos(cur).y_;
		int const ymin = y - height_ + defaultRowHeight();
		while (y > ymin && cur.up())
			y = getPos(cur).y_;

		cur.finishUndo();
		dr.screenUpdate(Update::SinglePar | Update::FitCursor);
		break;
	}

	case LFUN_SCREEN_DOWN_SELECT: {
		// FIXME: why is the algorithm different from LFUN_SCREEN_DOWN?
		cur.selHandle(true);
		if (isBottomScreen()) {
			lyx::dispatch(FuncRequest(LFUN_BUFFER_END_SELECT));
			cur.finishUndo();
			break;
		}
		int y = getPos(cur).y_;
		int const ymax = y + height_ - defaultRowHeight();
		while (y < ymax && cur.down())
			y = getPos(cur).y_;

		cur.finishUndo();
		dr.screenUpdate(Update::SinglePar | Update::FitCursor);
		break;
	}


	case LFUN_INSET_SELECT_ALL: {
		// true if all cells are selected
		bool const all_selected = cur.depth() > 1
		    && cur.selBegin().at_begin()
		    && cur.selEnd().at_end();
		// true if some cells are selected
		bool const cells_selected = cur.depth() > 1
		    && cur.selBegin().at_cell_begin()
			&& cur.selEnd().at_cell_end();
		if (all_selected || (cells_selected && !cur.inset().isTable())) {
			// All the contents of the inset if selected, or only at
			// least one cell but inset is not a table.
			// Select the inset from outside.
			cur.pop();
			cur.resetAnchor();
			cur.selection(true);
			cur.posForward();
		} else if (cells_selected) {
			// At least one complete cell is selected and inset is a table.
			// Select all cells
			cur.idx() = 0;
			cur.pos() = 0;
			cur.resetAnchor();
			cur.selection(true);
			cur.idx() = cur.lastidx();
			cur.pos() = cur.lastpos();
		} else {
			// select current cell
			cur.pit() = 0;
			cur.pos() = 0;
			cur.resetAnchor();
			cur.selection(true);
			cur.pit() = cur.lastpit();
			cur.pos() = cur.lastpos();
		}
		cur.setCurrentFont();
		dr.screenUpdate(Update::Force);
		break;
	}


	// This would be in Buffer class if only Cursor did not
	// require a bufferview
	case LFUN_INSET_FORALL: {
		docstring const name = from_utf8(cmd.getArg(0));
		string const commandstr = cmd.getLongArg(1);
		FuncRequest const fr = lyxaction.lookupFunc(commandstr);

		// an arbitrary number to limit number of iterations
		const int max_iter = 100000;
		int iterations = 0;
		Cursor & cur = d->cursor_;
		Cursor const savecur = cur;
		cur.reset();
		if (!cur.nextInset())
			cur.forwardInset();
		cur.beginUndoGroup();
		while(cur && iterations < max_iter) {
			Inset * const ins = cur.nextInset();
			if (!ins)
				break;
			docstring insname = ins->layoutName();
			while (!insname.empty()) {
				if (insname == name || name == from_utf8("*")) {
					cur.recordUndo();
					lyx::dispatch(fr, dr);
					++iterations;
					break;
				}
				size_t const i = insname.rfind(':');
				if (i == string::npos)
					break;
				insname = insname.substr(0, i);
			}
			// if we did not delete the inset, skip it
			if (!cur.nextInset() || cur.nextInset() == ins)
				cur.forwardInset();
		}
		cur = savecur;
		cur.fixIfBroken();
		/** This is a dummy undo record only to remember the cursor
		 * that has just been set; this will be used on a redo action
		 * (see ticket #10097)

		 * FIXME: a better fix would be to have a way to set the
		 * cursor value directly, but I am not sure it is worth it.
		 */
		cur.recordUndo();
		cur.endUndoGroup();
		dr.screenUpdate(Update::Force);
		dr.forceBufferUpdate();

		if (iterations >= max_iter) {
			dr.setError(true);
			dr.setMessage(bformat(_("`inset-forall' interrupted because number of actions is larger than %1$d"), max_iter));
		} else
			dr.setMessage(bformat(_("Applied \"%1$s\" to %2$d insets"), from_utf8(commandstr), iterations));
		break;
	}


	case LFUN_BRANCH_ADD_INSERT: {
		docstring branch_name = from_utf8(cmd.getArg(0));
		if (branch_name.empty())
			if (!Alert::askForText(branch_name, _("Branch name")) ||
						branch_name.empty())
				break;

		DispatchResult drtmp;
		buffer_.dispatch(FuncRequest(LFUN_BRANCH_ADD, branch_name), drtmp);
		if (drtmp.error()) {
			Alert::warning(_("Branch already exists"), drtmp.message());
			break;
		}
		BranchList & branch_list = buffer_.params().branchlist();
		vector<docstring> const branches =
			getVectorFromString(branch_name, branch_list.separator());
		for (vector<docstring>::const_iterator it = branches.begin();
		     it != branches.end(); ++it) {
			branch_name = *it;
			lyx::dispatch(FuncRequest(LFUN_BRANCH_INSERT, branch_name));
		}
		break;
	}

	case LFUN_KEYMAP_OFF:
		getIntl().keyMapOn(false);
		break;

	case LFUN_KEYMAP_PRIMARY:
		getIntl().keyMapPrim();
		break;

	case LFUN_KEYMAP_SECONDARY:
		getIntl().keyMapSec();
		break;

	case LFUN_KEYMAP_TOGGLE:
		getIntl().toggleKeyMap();
		break;

	case LFUN_DIALOG_SHOW_NEW_INSET: {
		string const name = cmd.getArg(0);
		string data = trim(to_utf8(cmd.argument()).substr(name.size()));
		if (decodeInsetParam(name, data, buffer_))
			lyx::dispatch(FuncRequest(LFUN_DIALOG_SHOW, name + " " + data));
		else
			lyxerr << "Inset type '" << name <<
			"' not recognized in LFUN_DIALOG_SHOW_NEW_INSET" <<  endl;
		break;
	}

	case LFUN_CITATION_INSERT: {
		if (argument.empty()) {
			lyx::dispatch(FuncRequest(LFUN_DIALOG_SHOW_NEW_INSET, "citation"));
			break;
		}
		// we can have one optional argument, delimited by '|'
		// citation-insert <key>|<text_before>
		// this should be enhanced to also support text_after
		// and citation style
		string arg = argument;
		string opt1;
		if (contains(argument, "|")) {
			arg = token(argument, '|', 0);
			opt1 = token(argument, '|', 1);
		}

		// if our cursor is directly in front of or behind a citation inset,
		// we will instead add the new key to it.
		Inset * inset = cur.nextInset();
		if (!inset || inset->lyxCode() != CITE_CODE)
			inset = cur.prevInset();
		if (inset && inset->lyxCode() == CITE_CODE) {
			InsetCitation * icite = static_cast<InsetCitation *>(inset);
			if (icite->addKey(arg)) {
				dr.forceBufferUpdate();
				dr.screenUpdate(Update::FitCursor | Update::SinglePar);
				if (!opt1.empty())
					LYXERR0("Discarding optional argument to citation-insert.");
			}
			dispatched = true;
			break;
		}
		InsetCommandParams icp(CITE_CODE);
		icp["key"] = from_utf8(arg);
		if (!opt1.empty())
			icp["before"] = from_utf8(opt1);
		string icstr = InsetCommand::params2string(icp);
		FuncRequest fr(LFUN_INSET_INSERT, icstr);
		lyx::dispatch(fr);
		break;
	}

	case LFUN_INSET_APPLY: {
		string const name = cmd.getArg(0);
		Inset * inset = editedInset(name);
		if (!inset) {
			FuncRequest fr(LFUN_INSET_INSERT, cmd.argument());
			lyx::dispatch(fr);
			break;
		}
		// put cursor in front of inset.
		if (!setCursorFromInset(inset)) {
			LASSERT(false, break);
		}
		cur.recordUndo();
		FuncRequest fr(LFUN_INSET_MODIFY, cmd.argument());
		inset->dispatch(cur, fr);
		dr.screenUpdate(cur.result().screenUpdate());
		if (cur.result().needBufferUpdate())
			dr.forceBufferUpdate();
		break;
	}

	// FIXME:
	// The change of language of buffer belongs to the Buffer class.
	// We have to do it here because we need a cursor for Undo.
	// When Undo::recordUndoBufferParams() is implemented someday
	// LFUN_BUFFER_LANGUAGE should be handled by the Buffer class.
	case LFUN_BUFFER_LANGUAGE: {
		Language const * oldL = buffer_.params().language;
		Language const * newL = languages.getLanguage(argument);
		if (!newL || oldL == newL)
			break;
		if (oldL->rightToLeft() == newL->rightToLeft()) {
			cur.recordUndoFullBuffer();
			buffer_.changeLanguage(oldL, newL);
			cur.setCurrentFont();
			dr.forceBufferUpdate();
		}
		break;
	}

	case LFUN_FILE_INSERT_PLAINTEXT_PARA:
	case LFUN_FILE_INSERT_PLAINTEXT: {
		bool const as_paragraph = (act == LFUN_FILE_INSERT_PLAINTEXT_PARA);
		string const fname = to_utf8(cmd.argument());
		if (!FileName::isAbsolute(fname))
			dr.setMessage(_("Absolute filename expected."));
		else
			insertPlaintextFile(FileName(fname), as_paragraph);
		break;
	}

	default:
		// OK, so try the Buffer itself...
		buffer_.dispatch(cmd, dr);
		dispatched = dr.dispatched();
		break;
	}

	buffer_.undo().endUndoGroup();
	dr.dispatched(dispatched);

	// NOTE: The code below is copied from Cursor::dispatch. If you
	// need to modify this, please update the other one too.

	// notify insets we just entered/left
	if (cursor() != old) {
		old.beginUndoGroup();
		old.fixIfBroken();
		bool badcursor = notifyCursorLeavesOrEnters(old, cursor());
		if (badcursor) {
			cursor().fixIfBroken();
			resetInlineCompletionPos();
		}
		old.endUndoGroup();
	}
}


docstring const BufferView::requestSelection()
{
	Cursor & cur = d->cursor_;

	LYXERR(Debug::SELECTION, "requestSelection: cur.selection: " << cur.selection());
	if (!cur.selection()) {
		d->xsel_cache_.set = false;
		return docstring();
	}

	LYXERR(Debug::SELECTION, "requestSelection: xsel_cache.set: " << d->xsel_cache_.set);
	if (!d->xsel_cache_.set ||
	    cur.top() != d->xsel_cache_.cursor ||
	    cur.realAnchor().top() != d->xsel_cache_.anchor)
	{
		d->xsel_cache_.cursor = cur.top();
		d->xsel_cache_.anchor = cur.realAnchor().top();
		d->xsel_cache_.set = cur.selection();
		return cur.selectionAsString(false);
	}
	return docstring();
}


void BufferView::clearSelection()
{
	d->cursor_.clearSelection();
	// Clear the selection buffer. Otherwise a subsequent
	// middle-mouse-button paste would use the selection buffer,
	// not the more current external selection.
	cap::clearSelection();
	d->xsel_cache_.set = false;
	// The buffer did not really change, but this causes the
	// redraw we need because we cleared the selection above.
	buffer_.changed(false);
}


void BufferView::resize(int width, int height)
{
	// Update from work area
	width_ = width;
	height_ = height;

	// Clear the paragraph height cache.
	d->par_height_.clear();
	// Redo the metrics.
	updateMetrics();
}


Inset const * BufferView::getCoveringInset(Text const & text,
		int x, int y) const
{
	TextMetrics & tm = d->text_metrics_[&text];
	Inset * inset = tm.checkInsetHit(x, y);
	if (!inset)
		return 0;

	if (!inset->descendable(*this))
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


void BufferView::updateHoveredInset() const
{
	// Get inset under mouse, if there is one.
	int const x = d->mouse_position_cache_.x_;
	int const y = d->mouse_position_cache_.y_;
	Inset const * covering_inset = getCoveringInset(buffer_.text(), x, y);

	d->clickable_inset_ = covering_inset && covering_inset->clickable(*this, x, y);

	if (covering_inset == d->last_inset_)
		// Same inset, no need to do anything...
		return;

	bool need_redraw = false;
	if (d->last_inset_) {
		// Remove the hint on the last hovered inset (if any).
		need_redraw |= d->last_inset_->setMouseHover(this, false);
		d->last_inset_ = 0;
	}

	if (covering_inset && covering_inset->setMouseHover(this, true)) {
		need_redraw = true;
		// Only the insets that accept the hover state, do
		// clear the last_inset_, so only set the last_inset_
		// member if the hovered setting is accepted.
		d->last_inset_ = covering_inset;
	}

	if (need_redraw) {
		LYXERR(Debug::PAINTING, "Mouse hover detected at: ("
				<< d->mouse_position_cache_.x_ << ", "
				<< d->mouse_position_cache_.y_ << ")");

		d->update_strategy_ = DecorationUpdate;

		// This event (moving without mouse click) is not passed further.
		// This should be changed if it is further utilized.
		buffer_.changed(false);
	}
}


void BufferView::clearLastInset(Inset * inset) const
{
	if (d->last_inset_ != inset) {
		LYXERR0("Wrong last_inset!");
		LATTEST(false);
	}
	d->last_inset_ = 0;
}


void BufferView::mouseEventDispatch(FuncRequest const & cmd0)
{
	//lyxerr << "[ cmd0 " << cmd0 << "]" << endl;

	// This is only called for mouse related events including
	// LFUN_FILE_OPEN generated by drag-and-drop.
	FuncRequest cmd = cmd0;

	Cursor old = cursor();
	Cursor cur(*this);
	cur.push(buffer_.inset());
	cur.selection(d->cursor_.selection());

	// Either the inset under the cursor or the
	// surrounding Text will handle this event.

	// make sure we stay within the screen...
	cmd.set_y(min(max(cmd.y(), -1), height_));

	d->mouse_position_cache_.x_ = cmd.x();
	d->mouse_position_cache_.y_ = cmd.y();

	if (cmd.action() == LFUN_MOUSE_MOTION && cmd.button() == mouse_button::none) {
		updateHoveredInset();
		return;
	}

	// Build temporary cursor.
	Inset * inset = d->text_metrics_[&buffer_.text()].editXY(cur, cmd.x(), cmd.y());
	if (inset) {
		// If inset is not editable, cur.pos() might point behind the
		// inset (depending on cmd.x(), cmd.y()). This is needed for
		// editing to fix bug 9628, but e.g. the context menu needs a
		// cursor in front of the inset.
		if ((inset->hasSettings() || !inset->contextMenuName().empty()
		     || inset->lyxCode() == SEPARATOR_CODE) &&
		    cur.nextInset() != inset && cur.prevInset() == inset)
			cur.posBackward();
	} else if (cur.inTexted() && cur.pos()
			&& cur.paragraph().isEnvSeparator(cur.pos() - 1)) {
		// Always place cursor in front of a separator inset.
		cur.posBackward();
	}

	// Put anchor at the same position.
	cur.resetAnchor();

	cur.beginUndoGroup();

	// Try to dispatch to an non-editable inset near this position
	// via the temp cursor. If the inset wishes to change the real
	// cursor it has to do so explicitly by using
	//  cur.bv().cursor() = cur;  (or similar)
	if (inset)
		inset->dispatch(cur, cmd);

	// Now dispatch to the temporary cursor. If the real cursor should
	// be modified, the inset's dispatch has to do so explicitly.
	if (!inset || !cur.result().dispatched())
		cur.dispatch(cmd);

	// Notify left insets
	if (cur != old) {
		bool badcursor = old.fixIfBroken() | cur.fixIfBroken();
		badcursor |= notifyCursorLeavesOrEnters(old, cur);
		if (badcursor)
			cursor().fixIfBroken();
	}

	cur.endUndoGroup();

	// Do we have a selection?
	theSelection().haveSelection(cursor().selection());

	if (cur.needBufferUpdate()) {
		cur.clearBufferUpdate();
		buffer().updateBuffer();
	}

	// If the command has been dispatched,
	if (cur.result().dispatched() || cur.result().screenUpdate())
		processUpdateFlags(cur.result().screenUpdate());
}


int BufferView::minVisiblePart()
{
	return 2 * defaultRowHeight();
}


int BufferView::scroll(int y)
{
	if (y > 0)
		return scrollDown(y);
	if (y < 0)
		return scrollUp(-y);
	return 0;
}


int BufferView::scrollDown(int offset)
{
	Text * text = &buffer_.text();
	TextMetrics & tm = d->text_metrics_[text];
	int const ymax = height_ + offset;
	while (true) {
		pair<pit_type, ParagraphMetrics const *> last = tm.last();
		int bottom_pos = last.second->position() + last.second->descent();
		if (lyxrc.scroll_below_document)
			bottom_pos += height_ - minVisiblePart();
		if (last.first + 1 == int(text->paragraphs().size())) {
			if (bottom_pos <= height_)
				return 0;
			offset = min(offset, bottom_pos - height_);
			break;
		}
		if (bottom_pos > ymax)
			break;
		tm.newParMetricsDown();
	}
	d->anchor_ypos_ -= offset;
	return -offset;
}


int BufferView::scrollUp(int offset)
{
	Text * text = &buffer_.text();
	TextMetrics & tm = d->text_metrics_[text];
	int ymin = - offset;
	while (true) {
		pair<pit_type, ParagraphMetrics const *> first = tm.first();
		int top_pos = first.second->position() - first.second->ascent();
		if (first.first == 0) {
			if (top_pos >= 0)
				return 0;
			offset = min(offset, - top_pos);
			break;
		}
		if (top_pos < ymin)
			break;
		tm.newParMetricsUp();
	}
	d->anchor_ypos_ += offset;
	return offset;
}


void BufferView::setCursorFromRow(int row)
{
	setCursorFromRow(row, buffer_.texrow());
}


void BufferView::setCursorFromRow(int row, TexRow const & texrow)
{
	int tmpid;
	int tmppos;
	pit_type newpit = 0;
	pos_type newpos = 0;

	texrow.getIdFromRow(row, tmpid, tmppos);

	bool posvalid = (tmpid != -1);
	if (posvalid) {
		// we need to make sure that the row and position
		// we got back are valid, because the buffer may well
		// have changed since we last generated the LaTeX.
		DocIterator dit = buffer_.getParFromID(tmpid);
		if (dit == doc_iterator_end(&buffer_))
			posvalid = false;
		else if (dit.depth() > 1) {
			// We are in an inset.
			pos_type lastpos = dit.lastpos();
			dit.pos() = tmppos > lastpos ? lastpos : tmppos;
			setCursor(dit);
			recenter();
			return;
		} else {
			newpit = dit.pit();
			// now have to check pos.
			newpos = tmppos;
			Paragraph const & par = buffer_.text().getPar(newpit);
			if (newpos > par.size()) {
				LYXERR0("Requested position no longer valid.");
				newpos = par.size() - 1;
			}
		}
	}
	if (!posvalid) {
		frontend::Alert::error(_("Inverse Search Failed"),
			_("Invalid position requested by inverse search.\n"
		    "You need to update the viewed document."));
		return;
	}
	d->cursor_.reset();
	buffer_.text().setCursor(d->cursor_, newpit, newpos);
	d->cursor_.selection(false);
	d->cursor_.resetAnchor();
	recenter();
}


bool BufferView::setCursorFromInset(Inset const * inset)
{
	// are we already there?
	if (cursor().nextInset() == inset)
		return true;

	// Inset is not at cursor position. Find it in the document.
	Cursor cur(*this);
	cur.reset();
	while (cur && cur.nextInset() != inset)
		cur.forwardInset();

	if (cur) {
		setCursor(cur);
		return true;
	}
	return false;
}


void BufferView::gotoLabel(docstring const & label)
{
	ListOfBuffers bufs = buffer().allRelatives();
	ListOfBuffers::iterator it = bufs.begin();
	for (; it != bufs.end(); ++it) {
		Buffer const * buf = *it;

		// find label
		shared_ptr<Toc> toc = buf->tocBackend().toc("label");
		Toc::const_iterator toc_it = toc->begin();
		Toc::const_iterator end = toc->end();
		for (; toc_it != end; ++toc_it) {
			if (label == toc_it->str()) {
				lyx::dispatch(toc_it->action());
				return;
			}
		}
	}
}


TextMetrics const & BufferView::textMetrics(Text const * t) const
{
	return const_cast<BufferView *>(this)->textMetrics(t);
}


TextMetrics & BufferView::textMetrics(Text const * t)
{
	LBUFERR(t);
	TextMetricsCache::iterator tmc_it  = d->text_metrics_.find(t);
	if (tmc_it == d->text_metrics_.end()) {
		tmc_it = d->text_metrics_.insert(
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
	d->cursor_.reset();
	size_t const n = dit.depth();
	for (size_t i = 0; i < n; ++i)
		dit[i].inset().edit(d->cursor_, true);

	d->cursor_.setCursor(dit);
	d->cursor_.selection(false);
	d->cursor_.setCurrentFont();
	// FIXME
	// It seems on general grounds as if this is probably needed, but
	// it is not yet clear.
	// See bug #7394 and r38388.
	// d->cursor.resetAnchor();
}


bool BufferView::checkDepm(Cursor & cur, Cursor & old)
{
	// Would be wrong to delete anything if we have a selection.
	if (cur.selection())
		return false;

	bool need_anchor_change = false;
	bool changed = d->cursor_.text()->deleteEmptyParagraphMechanism(cur, old,
		need_anchor_change);

	if (need_anchor_change)
		cur.resetAnchor();

	if (!changed)
		return false;

	d->cursor_ = cur;

	// we would rather not do this here, but it needs to be done before
	// the changed() signal is sent.
	buffer_.updateBuffer();

	buffer_.changed(true);
	return true;
}


bool BufferView::mouseSetCursor(Cursor & cur, bool select)
{
	LASSERT(&cur.bv() == this, return false);

	if (!select)
		// this event will clear selection so we save selection for
		// persistent selection
		cap::saveSelection(cursor());

	d->cursor_.macroModeClose();
	// If a macro has been finalized, the cursor might have been broken
	cur.fixIfBroken();

	// Has the cursor just left the inset?
	bool const leftinset = (&d->cursor_.inset() != &cur.inset());
	if (leftinset)
		d->cursor_.fixIfBroken();

	// FIXME: shift-mouse selection doesn't work well across insets.
	bool const do_selection =
			select && &d->cursor_.normalAnchor().inset() == &cur.inset();

	// do the dEPM magic if needed
	// FIXME: (1) move this to InsetText::notifyCursorLeaves?
	// FIXME: (2) if we had a working InsetText::notifyCursorLeaves,
	// the leftinset bool would not be necessary (badcursor instead).
	bool update = leftinset;
	if (!do_selection && d->cursor_.inTexted())
		update |= checkDepm(cur, d->cursor_);

	if (!do_selection)
		d->cursor_.resetAnchor();
	d->cursor_.setCursor(cur);
	d->cursor_.boundary(cur.boundary());
	if (do_selection)
		d->cursor_.setSelection();
	else
		d->cursor_.clearSelection();

	d->cursor_.finishUndo();
	d->cursor_.setCurrentFont();
	if (update)
		cur.forceBufferUpdate();
	return update;
}


void BufferView::putSelectionAt(DocIterator const & cur,
				int length, bool backwards)
{
	d->cursor_.clearSelection();

	setCursor(cur);

	if (length) {
		if (backwards) {
			d->cursor_.pos() += length;
			d->cursor_.setSelection(d->cursor_, -length);
		} else
			d->cursor_.setSelection(d->cursor_, length);
	}
}


bool BufferView::selectIfEmpty(DocIterator & cur)
{
	if ((cur.inTexted() && !cur.paragraph().empty())
	    || (cur.inMathed() && !cur.cell().empty()))
		return false;

	pit_type const beg_pit = cur.pit();
	if (beg_pit > 0) {
		// The paragraph associated to this item isn't
		// the first one, so it can be selected
		cur.backwardPos();
	} else {
		// We have to resort to select the space between the
		// end of this item and the begin of the next one
		cur.forwardPos();
	}
	if (cur.empty()) {
		// If it is the only item in the document,
		// nothing can be selected
		return false;
	}
	pit_type const end_pit = cur.pit();
	pos_type const end_pos = cur.pos();
	d->cursor_.clearSelection();
	d->cursor_.reset();
	d->cursor_.setCursor(cur);
	d->cursor_.pit() = beg_pit;
	d->cursor_.pos() = 0;
	d->cursor_.selection(false);
	d->cursor_.resetAnchor();
	d->cursor_.pit() = end_pit;
	d->cursor_.pos() = end_pos;
	d->cursor_.setSelection();
	return true;
}


Cursor & BufferView::cursor()
{
	return d->cursor_;
}


Cursor const & BufferView::cursor() const
{
	return d->cursor_;
}


pit_type BufferView::anchor_ref() const
{
	return d->anchor_pit_;
}


bool BufferView::singleParUpdate()
{
	Text & buftext = buffer_.text();
	pit_type const bottom_pit = d->cursor_.bottom().pit();
	TextMetrics & tm = textMetrics(&buftext);
	int old_height = tm.parMetrics(bottom_pit).height();

	// make sure inline completion pointer is ok
	if (d->inlineCompletionPos_.fixIfBroken())
		d->inlineCompletionPos_ = DocIterator();

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

	d->update_strategy_ = SingleParUpdate;

	LYXERR(Debug::PAINTING, "\ny1: " << pm.position() - pm.ascent()
		<< " y2: " << pm.position() + pm.descent()
		<< " pit: " << bottom_pit
		<< " singlepar: 1");
	return true;
}


void BufferView::updateMetrics()
{
	if (height_ == 0 || width_ == 0)
		return;

	Text & buftext = buffer_.text();
	pit_type const npit = int(buftext.paragraphs().size());

	// Clear out the position cache in case of full screen redraw,
	d->coord_cache_.clear();

	// Clear out paragraph metrics to avoid having invalid metrics
	// in the cache from paragraphs not relayouted below
	// The complete text metrics will be redone.
	d->text_metrics_.clear();

	TextMetrics & tm = textMetrics(&buftext);

	// make sure inline completion pointer is ok
	if (d->inlineCompletionPos_.fixIfBroken())
		d->inlineCompletionPos_ = DocIterator();

	if (d->anchor_pit_ >= npit)
		// The anchor pit must have been deleted...
		d->anchor_pit_ = npit - 1;

	// Rebreak anchor paragraph.
	tm.redoParagraph(d->anchor_pit_);
	ParagraphMetrics & anchor_pm = tm.par_metrics_[d->anchor_pit_];

	// position anchor
	if (d->anchor_pit_ == 0) {
		int scrollRange = d->scrollbarParameters_.max - d->scrollbarParameters_.min;

		// Complete buffer visible? Then it's easy.
		if (scrollRange == 0)
			d->anchor_ypos_ = anchor_pm.ascent();

		// FIXME: Some clever handling needed to show
		// the _first_ paragraph up to the top if the cursor is
		// in the first line.
	}
	anchor_pm.setPosition(d->anchor_ypos_);

	LYXERR(Debug::PAINTING, "metrics: "
		<< " anchor pit = " << d->anchor_pit_
		<< " anchor ypos = " << d->anchor_ypos_);

	// Redo paragraphs above anchor if necessary.
	int y1 = d->anchor_ypos_ - anchor_pm.ascent();
	// We are now just above the anchor paragraph.
	pit_type pit1 = d->anchor_pit_ - 1;
	for (; pit1 >= 0 && y1 >= 0; --pit1) {
		tm.redoParagraph(pit1);
		ParagraphMetrics & pm = tm.par_metrics_[pit1];
		y1 -= pm.descent();
		// Save the paragraph position in the cache.
		pm.setPosition(y1);
		y1 -= pm.ascent();
	}

	// Redo paragraphs below the anchor if necessary.
	int y2 = d->anchor_ypos_ + anchor_pm.descent();
	// We are now just below the anchor paragraph.
	pit_type pit2 = d->anchor_pit_ + 1;
	for (; pit2 < npit && y2 <= height_; ++pit2) {
		tm.redoParagraph(pit2);
		ParagraphMetrics & pm = tm.par_metrics_[pit2];
		y2 += pm.ascent();
		// Save the paragraph position in the cache.
		pm.setPosition(y2);
		y2 += pm.descent();
	}

	LYXERR(Debug::PAINTING, "Metrics: "
		<< " anchor pit = " << d->anchor_pit_
		<< " anchor ypos = " << d->anchor_ypos_
		<< " y1 = " << y1
		<< " y2 = " << y2
		<< " pit1 = " << pit1
		<< " pit2 = " << pit2);

	d->update_strategy_ = FullScreenUpdate;

	if (lyxerr.debugging(Debug::WORKAREA)) {
		LYXERR(Debug::WORKAREA, "BufferView::updateMetrics");
		d->coord_cache_.dump();
	}
}


void BufferView::insertLyXFile(FileName const & fname)
{
	LASSERT(d->cursor_.inTexted(), return);

	// Get absolute path of file and add ".lyx"
	// to the filename if necessary
	FileName filename = fileSearch(string(), fname.absFileName(), "lyx");

	docstring const disp_fn = makeDisplayPath(filename.absFileName());
	// emit message signal.
	message(bformat(_("Inserting document %1$s..."), disp_fn));

	docstring res;
	Buffer buf(filename.absFileName(), false);
	if (buf.loadLyXFile() == Buffer::ReadSuccess) {
		ErrorList & el = buffer_.errorList("Parse");
		// Copy the inserted document error list into the current buffer one.
		el = buf.errorList("Parse");
		buffer_.undo().recordUndo(d->cursor_);
		cap::pasteParagraphList(d->cursor_, buf.paragraphs(),
					     buf.params().documentClassPtr(), el);
		res = _("Document %1$s inserted.");
	} else {
		res = _("Could not insert document %1$s");
	}

	buffer_.changed(true);
	// emit message signal.
	message(bformat(res, disp_fn));
}


Point BufferView::coordOffset(DocIterator const & dit) const
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
		sl.inset().cursorPos(*this, sl, dit.boundary() && (i + 1 == dit.depth()), xx, yy);

		// Make relative position inside of the edited inset relative to sl.inset()
		x += xx;
		y += yy;

		// In case of an RTL inset, the edited inset will be positioned to the left
		// of xx:yy
		if (sl.text()) {
			bool boundary_i = dit.boundary() && i + 1 == dit.depth();
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
			Dimension const & dim = coordCache().getInsets().dim(&sl.inset());
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

	LBUFERR(!pm.rows().empty());
	y -= pm.rows()[0].ascent();
#if 1
	// FIXME: document this mess
	size_t rend;
	if (sl.pos() > 0 && dit.depth() == 1) {
		int pos = sl.pos();
		if (pos && dit.boundary())
			--pos;
//		lyxerr << "coordOffset: boundary:" << dit.boundary() << " depth:" << dit.depth() << " pos:" << pos << " sl.pos:" << sl.pos() << endl;
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
	int xx = bottom_tm.cursorX(dit.bottom(), dit.boundary() && dit.depth() == 1);
	x += xx;

	// In the RTL case place the nested inset at the left of the cursor in
	// the outer paragraph
	bool boundary_1 = dit.boundary() && 1 == dit.depth();
	bool rtl = bottom_tm.isRTL(dit.bottom(), boundary_1);
	if (rtl)
		x -= lastw;

	return Point(x, y);
}


Point BufferView::getPos(DocIterator const & dit) const
{
	if (!paragraphVisible(dit))
		return Point(-1, -1);

	CursorSlice const & bot = dit.bottom();
	TextMetrics const & tm = textMetrics(bot.text());

	// offset from outer paragraph
	Point p = coordOffset(dit);
	p.y_ += tm.parMetrics(bot.pit()).position();
	return p;
}


bool BufferView::paragraphVisible(DocIterator const & dit) const
{
	CursorSlice const & bot = dit.bottom();
	TextMetrics const & tm = textMetrics(bot.text());

	return tm.contains(bot.pit());
}


void BufferView::cursorPosAndHeight(Point & p, int & h) const
{
	Cursor const & cur = cursor();
	Font const font = cur.getFont();
	frontend::FontMetrics const & fm = theFontMetrics(font);
	int const asc = fm.maxAscent();
	int const des = fm.maxDescent();
	h = asc + des;
	p = getPos(cur);
	p.y_ -= asc;
}


bool BufferView::cursorInView(Point const & p, int h) const
{
	Cursor const & cur = cursor();
	// does the cursor touch the screen ?
	if (p.y_ + h < 0 || p.y_ >= workHeight() || !paragraphVisible(cur))
		return false;
	return true;
}


int BufferView::horizScrollOffset() const
{
	return d->horiz_scroll_offset_;
}


int BufferView::horizScrollOffset(Text const * text,
                                  pit_type pit, pos_type pos) const
{
	// Is this a row that is currently scrolled?
	if (!d->current_row_slice_.empty()
	    && &text->inset() == d->current_row_slice_.inset().asInsetText()
	    && pit ==  d->current_row_slice_.pit()
	    && pos ==  d->current_row_slice_.pos())
		return d->horiz_scroll_offset_;
	return 0;
}


bool BufferView::hadHorizScrollOffset(Text const * text,
                                      pit_type pit, pos_type pos) const
{
	return !d->last_row_slice_.empty()
	       && &text->inset() == d->last_row_slice_.inset().asInsetText()
	       && pit ==  d->last_row_slice_.pit()
	       && pos ==  d->last_row_slice_.pos();
}


void BufferView::setCurrentRowSlice(CursorSlice const & rowSlice)
{
	// nothing to do if the cursor was already on this row
	if (d->current_row_slice_ == rowSlice) {
		d->last_row_slice_ = CursorSlice();
		return;
	}

	// if the (previous) current row was scrolled, we have to
	// remember it in order to repaint it next time.
	if (d->horiz_scroll_offset_ != 0)
		d->last_row_slice_ = d->current_row_slice_;
	else
		d->last_row_slice_ = CursorSlice();

	// Since we changed row, the scroll offset is not valid anymore
	d->horiz_scroll_offset_ = 0;
	d->current_row_slice_ = rowSlice;
}


void BufferView::checkCursorScrollOffset(PainterInfo & pi)
{
	CursorSlice rowSlice = d->cursor_.bottom();
	TextMetrics const & tm = textMetrics(rowSlice.text());

	// Stop if metrics have not been computed yet, since it means
	// that there is nothing to do.
	if (!tm.contains(rowSlice.pit()))
		return;
	ParagraphMetrics const & pm = tm.parMetrics(rowSlice.pit());
	Row const & row = pm.getRow(rowSlice.pos(),
	                            d->cursor_.boundary() && rowSlice == d->cursor_.top());
	rowSlice.pos() = row.pos();

	// Set the row on which the cursor lives.
	setCurrentRowSlice(rowSlice);

	// If insets referred to by cursor are not all in the cache, the positions
	// need to be recomputed.
	if (!d->cursor_.inCoordCache()) {
		/** FIXME: the code below adds an extraneous computation of
		 * inset positions, and can therefore be bad for performance
		 * (think for example about a very large tabular inset.
		 * Redawing the row where it is means redrawing the whole
		 * screen).
		 *
		 * The bug that this fixes is the following: assume that there
		 * is a very large math inset. Upon entering the inset, when
		 * pressing `End', the row is not scrolled and the cursor is
		 * not visible. The extra row computation makes sure that the
		 * inset positions are correctly computed and set in the
		 * cache. This would not happen if we did not have two-stage
		 * drawing.
		 *
		 * A proper fix would be to always have proper inset positions
		 * at this point.
		 */
		// Force the recomputation of inset positions
		bool const drawing = pi.pain.isDrawingEnabled();
		pi.pain.setDrawingEnabled(false);
		// No need to care about vertical position.
		RowPainter rp(pi, buffer().text(), row, -d->horiz_scroll_offset_, 0);
		rp.paintText();
		pi.pain.setDrawingEnabled(drawing);
	}

	// Current x position of the cursor in pixels
	int cur_x = getPos(d->cursor_).x_;

	// Horizontal scroll offset of the cursor row in pixels
	int offset = d->horiz_scroll_offset_;
	int const MARGIN = 2 * theFontMetrics(d->cursor_.real_current_font).em()
	                   + row.right_margin;
	if (row.right_x() <= workWidth() - row.right_margin) {
		// Row is narrower than the work area, no offset needed.
		offset = 0;
	} else {
		if (cur_x - offset < MARGIN) {
			// cursor would be too far right
			offset = cur_x - MARGIN;
		} else if (cur_x - offset > workWidth() - MARGIN) {
			// cursor would be too far left
			offset = cur_x - workWidth() + MARGIN;
		}
		// Correct the offset to make sure that we do not scroll too much
		if (offset < 0)
			offset = 0;
		if (row.right_x() - offset < workWidth() - row.right_margin)
			offset = row.right_x() - workWidth() + row.right_margin;
	}

	//lyxerr << "cur_x=" << cur_x << ", offset=" << offset << ", row.wid=" << row.width() << ", margin=" << MARGIN << endl;

	if (offset != d->horiz_scroll_offset_)
		LYXERR(Debug::PAINTING, "Horiz. scroll offset changed from "
		       << d->horiz_scroll_offset_ << " to " << offset);

	if (d->update_strategy_ == NoScreenUpdate
	    && (offset != d->horiz_scroll_offset_
		|| !d->last_row_slice_.empty())) {
		// FIXME: if one uses SingleParUpdate, then home/end
		// will not work on long rows. Why?
		d->update_strategy_ = FullScreenUpdate;
	}

	d->horiz_scroll_offset_ = offset;
}


void BufferView::draw(frontend::Painter & pain)
{
	if (height_ == 0 || width_ == 0)
		return;
	LYXERR(Debug::PAINTING, "\t\t*** START DRAWING ***");

	Text & text = buffer_.text();
	TextMetrics const & tm = d->text_metrics_[&text];
	int const y = tm.first().second->position();
	PainterInfo pi(this, pain);

	// Check whether the row where the cursor lives needs to be scrolled.
	// Update the drawing strategy if needed.
	checkCursorScrollOffset(pi);

	switch (d->update_strategy_) {

	case NoScreenUpdate:
		// If no screen painting is actually needed, only some the different
		// coordinates of insets and paragraphs needs to be updated.
		LYXERR(Debug::PAINTING, "Strategy: NoScreenUpdate");
		pi.full_repaint = true;
		pi.pain.setDrawingEnabled(false);
		tm.draw(pi, 0, y);
		break;

	case SingleParUpdate:
		pi.full_repaint = false;
		LYXERR(Debug::PAINTING, "Strategy: SingleParUpdate");
		// In general, only the current row of the outermost paragraph
		// will be redrawn. Particular cases where selection spans
		// multiple paragraph are correctly detected in TextMetrics.
		tm.draw(pi, 0, y);
		break;

	case DecorationUpdate:
		// FIXME: We should also distinguish DecorationUpdate to avoid text
		// drawing if possible. This is not possible to do easily right now
		// because of the single backing pixmap.

	case FullScreenUpdate:

		LYXERR(Debug::PAINTING,
		       ((d->update_strategy_ == FullScreenUpdate)
			? "Strategy: FullScreenUpdate"
			: "Strategy: DecorationUpdate"));

		// The whole screen, including insets, will be refreshed.
		pi.full_repaint = true;

		// Clear background.
		pain.fillRectangle(0, 0, width_, height_,
			pi.backgroundColor(&buffer_.inset()));

		// Draw everything.
		tm.draw(pi, 0, y);

		// and possibly grey out below
		pair<pit_type, ParagraphMetrics const *> lastpm = tm.last();
		int const y2 = lastpm.second->position() + lastpm.second->descent();

		if (y2 < height_) {
			Color color = buffer().isInternal()
				? Color_background : Color_bottomarea;
			pain.fillRectangle(0, y2, width_, height_ - y2, color);
		}
		break;
	}
	LYXERR(Debug::PAINTING, "\n\t\t*** END DRAWING  ***");

	// The scrollbar needs an update.
	updateScrollbar();

	// Normalize anchor for next time
	pair<pit_type, ParagraphMetrics const *> firstpm = tm.first();
	pair<pit_type, ParagraphMetrics const *> lastpm = tm.last();
	for (pit_type pit = firstpm.first; pit <= lastpm.first; ++pit) {
		ParagraphMetrics const & pm = tm.parMetrics(pit);
		if (pm.position() + pm.descent() > 0) {
			d->anchor_pit_ = pit;
			d->anchor_ypos_ = pm.position();
			break;
		}
	}
	LYXERR(Debug::PAINTING, "Found new anchor pit = " << d->anchor_pit_
		<< "  anchor ypos = " << d->anchor_ypos_);
}


void BufferView::message(docstring const & msg)
{
	if (d->gui_)
		d->gui_->message(msg);
}


void BufferView::showDialog(string const & name)
{
	if (d->gui_)
		d->gui_->showDialog(name, string());
}


void BufferView::showDialog(string const & name,
	string const & data, Inset * inset)
{
	if (d->gui_)
		d->gui_->showDialog(name, data, inset);
}


void BufferView::updateDialog(string const & name, string const & data)
{
	if (d->gui_)
		d->gui_->updateDialog(name, data);
}


void BufferView::setGuiDelegate(frontend::GuiBufferViewDelegate * gui)
{
	d->gui_ = gui;
}


// FIXME: Move this out of BufferView again
docstring BufferView::contentsOfPlaintextFile(FileName const & fname)
{
	if (!fname.isReadableFile()) {
		docstring const error = from_ascii(strerror(errno));
		docstring const file = makeDisplayPath(fname.absFileName(), 50);
		docstring const text =
		  bformat(_("Could not read the specified document\n"
			    "%1$s\ndue to the error: %2$s"), file, error);
		Alert::error(_("Could not read file"), text);
		return docstring();
	}

	if (!fname.isReadableFile()) {
		docstring const file = makeDisplayPath(fname.absFileName(), 50);
		docstring const text =
		  bformat(_("%1$s\n is not readable."), file);
		Alert::error(_("Could not open file"), text);
		return docstring();
	}

	// FIXME UNICODE: We don't know the encoding of the file
	docstring file_content = fname.fileContents("UTF-8");
	if (file_content.empty()) {
		Alert::error(_("Reading not UTF-8 encoded file"),
			     _("The file is not UTF-8 encoded.\n"
			       "It will be read as local 8Bit-encoded.\n"
			       "If this does not give the correct result\n"
			       "then please change the encoding of the file\n"
			       "to UTF-8 with a program other than LyX.\n"));
		file_content = fname.fileContents("local8bit");
	}

	return normalize_c(file_content);
}


void BufferView::insertPlaintextFile(FileName const & f, bool asParagraph)
{
	docstring const tmpstr = contentsOfPlaintextFile(f);

	if (tmpstr.empty())
		return;

	Cursor & cur = cursor();
	cap::replaceSelection(cur);
	buffer_.undo().recordUndo(cur);
	if (asParagraph)
		cur.innerText()->insertStringAsParagraphs(cur, tmpstr, cur.current_font);
	else
		cur.innerText()->insertStringAsLines(cur, tmpstr, cur.current_font);

	buffer_.changed(true);
}


docstring const & BufferView::inlineCompletion() const
{
	return d->inlineCompletion_;
}


size_t const & BufferView::inlineCompletionUniqueChars() const
{
	return d->inlineCompletionUniqueChars_;
}


DocIterator const & BufferView::inlineCompletionPos() const
{
	return d->inlineCompletionPos_;
}


void BufferView::resetInlineCompletionPos()
{
	d->inlineCompletionPos_ = DocIterator();
}


bool samePar(DocIterator const & a, DocIterator const & b)
{
	if (a.empty() && b.empty())
		return true;
	if (a.empty() || b.empty())
		return false;
	if (a.depth() != b.depth())
		return false;
	return &a.innerParagraph() == &b.innerParagraph();
}


void BufferView::setInlineCompletion(Cursor const & cur, DocIterator const & pos,
	docstring const & completion, size_t uniqueChars)
{
	uniqueChars = min(completion.size(), uniqueChars);
	bool changed = d->inlineCompletion_ != completion
		|| d->inlineCompletionUniqueChars_ != uniqueChars;
	bool singlePar = true;
	d->inlineCompletion_ = completion;
	d->inlineCompletionUniqueChars_ = min(completion.size(), uniqueChars);

	//lyxerr << "setInlineCompletion pos=" << pos << " completion=" << completion << " uniqueChars=" << uniqueChars << std::endl;

	// at new position?
	DocIterator const & old = d->inlineCompletionPos_;
	if (old != pos) {
		//lyxerr << "inlineCompletionPos changed" << std::endl;
		// old or pos are in another paragraph?
		if ((!samePar(cur, pos) && !pos.empty())
		    || (!samePar(cur, old) && !old.empty())) {
			singlePar = false;
			//lyxerr << "different paragraph" << std::endl;
		}
		d->inlineCompletionPos_ = pos;
	}

	// set update flags
	if (changed) {
		if (singlePar && !(cur.result().screenUpdate() & Update::Force))
			cur.screenUpdateFlags(cur.result().screenUpdate() | Update::SinglePar);
		else
			cur.screenUpdateFlags(cur.result().screenUpdate() | Update::Force);
	}
}


bool BufferView::clickableInset() const
{
	return d->clickable_inset_;
}

} // namespace lyx
