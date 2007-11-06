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
#include "CoordCache.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "DispatchResult.h"
#include "EmbeddedFiles.h"
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
#include "Text.h"
#include "TextClass.h"
#include "TextMetrics.h"
#include "TexRow.h"
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
#include "support/fs_extras.h"

#include <boost/bind.hpp>
#include <boost/current_function.hpp>
#include <boost/filesystem/operations.hpp>

#include <cerrno>
#include <fstream>
#include <functional>
#include <vector>

using std::distance;
using std::endl;
using std::ifstream;
using std::istringstream;
using std::istream_iterator;
using std::make_pair;
using std::min;
using std::max;
using std::mem_fun_ref;
using std::string;
using std::vector;

namespace fs = boost::filesystem;

namespace lyx {

using support::addPath;
using support::bformat;
using support::FileFilterList;
using support::FileName;
using support::fileSearch;
using support::makeDisplayPath;
using support::makeAbsPath;
using support::package;

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


bool findInset(DocIterator & dit, vector<InsetCode> const & codes,
	bool same_content);

bool findNextInset(DocIterator & dit, vector<InsetCode> const & codes,
	docstring const & contents)
{
	DocIterator tmpdit = dit;

	while (tmpdit) {
		Inset const * inset = tmpdit.nextInset();
		if (inset
		    && find(codes.begin(), codes.end(), inset->lyxCode()) != codes.end()
		    && (contents.empty() ||
		    static_cast<InsetCommand const *>(inset)->getFirstNonOptParam() == contents)) {
			dit = tmpdit;
			return true;
		}
		tmpdit.forwardInset();
	}

	return false;
}


/// Looks for next inset with one of the given codes.
bool findInset(DocIterator & dit, vector<InsetCode> const & codes,
	bool same_content)
{
	docstring contents;
	DocIterator tmpdit = dit;
	tmpdit.forwardInset();
	if (!tmpdit)
		return false;

	if (same_content) {
		Inset const * inset = tmpdit.nextInset();
		if (inset
		    && find(codes.begin(), codes.end(), inset->lyxCode()) != codes.end()) {
			contents = static_cast<InsetCommand const *>(inset)->getFirstNonOptParam();
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
}


/// Moves cursor to the next inset with given code.
void gotoInset(BufferView * bv, InsetCode code, bool same_content)
{
	gotoInset(bv, vector<InsetCode>(1, code), same_content);
}


/// A map from a Text to the associated text metrics
typedef std::map<Text const *, TextMetrics> TextMetricsCache;

} // anon namespace


/////////////////////////////////////////////////////////////////////
//
// BufferView
//
/////////////////////////////////////////////////////////////////////

struct BufferView::Private
{
	Private(BufferView & bv): wh_(0), cursor_(bv),
		anchor_ref_(0), offset_ref_(0), need_centering_(false),
		last_inset_(0), gui_(0)
	{}

	///
	ScrollbarParameters scrollbarParameters_;
	///
	ViewMetricsInfo metrics_info_;
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
	pit_type anchor_ref_;
	///
	int offset_ref_;
	///
	bool need_centering_;

	/// keyboard mapping object.
	Intl intl_;

	/// last visited inset.
	/** kept to send setMouseHover(false).
	  * Not owned, so don't delete.
	  */
	Inset * last_inset_;

	mutable TextMetricsCache text_metrics_;

	/// Whom to notify.
	/** Not owned, so don't delete.
	  */
	frontend::GuiBufferViewDelegate * gui_;
};


BufferView::BufferView(Buffer & buf)
	: width_(0), height_(0), buffer_(buf), d(new Private(*this))
{
	d->xsel_cache_.set = false;
	d->intl_.initKeyMapper(lyxrc.use_kbmap);

	d->cursor_.push(buffer_.inset());
	d->cursor_.resetAnchor();
	d->cursor_.setCurrentFont();

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
	LastFilePosSection::FilePos fp;
	fp.pit = d->cursor_.bottom().pit();
	fp.pos = d->cursor_.bottom().pos();
	LyX::ref().session().lastFilePos().save(buffer_.fileName(), fp);

	delete d;
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


bool BufferView::fitCursor()
{
	if (cursorStatus(d->cursor_) == CUR_INSIDE) {
		frontend::FontMetrics const & fm =
			theFontMetrics(d->cursor_.getFont().fontInfo());
		int const asc = fm.maxAscent();
		int const des = fm.maxDescent();
		Point const p = getPos(d->cursor_, d->cursor_.boundary());
		if (p.y_ - asc >= 0 && p.y_ + des < height_)
			return false;
	}
	center();
	return true;
}


void BufferView::processUpdateFlags(Update::flags flags)
{
	// last_inset_ points to the last visited inset. This pointer may become
	// invalid because of keyboard editing. Since all such operations
	// causes screen update(), I reset last_inset_ to avoid such a problem.
	d->last_inset_ = 0;
	// This is close to a hot-path.
	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< "[fitcursor = " << (flags & Update::FitCursor)
		<< ", forceupdate = " << (flags & Update::Force)
		<< ", singlepar = " << (flags & Update::SinglePar)
		<< "]  buffer: " << &buffer_ << endl;

	// Update macro store
	if (!(cursor().inMathed() && cursor().inMacroMode()))
		buffer_.updateMacros();

	// Now do the first drawing step if needed. This consists on updating
	// the CoordCache in updateMetrics().
	// The second drawing step is done in WorkArea::redraw() if needed.

	// Case when no explicit update is requested.
	if (!flags) {
		// no need to redraw anything.
		d->metrics_info_.update_strategy = NoScreenUpdate;
		return;
	}

	if (flags == Update::Decoration) {
		d->metrics_info_.update_strategy = DecorationUpdate;
		buffer_.changed();
		return;
	}

	if (flags == Update::FitCursor
		|| flags == (Update::Decoration | Update::FitCursor)) {
		bool const fit_cursor = fitCursor();
		// tell the frontend to update the screen if needed.
		if (fit_cursor) {
			updateMetrics();
			buffer_.changed();
			return;
		}
		if (flags & Update::Decoration) {
			d->metrics_info_.update_strategy = DecorationUpdate;
			buffer_.changed();
			return;
		}
		// no screen update is needed.
		d->metrics_info_.update_strategy = NoScreenUpdate;
		return;
	}

	bool const full_metrics = flags & Update::Force;

	if (full_metrics || !singleParUpdate())
		// We have to update the full screen metrics.
		updateMetrics();

	if (!(flags & Update::FitCursor)) {
		buffer_.changed();
		return;
	}

	//FIXME: updateMetrics() does not update paragraph position
	// This is done at draw() time. So we need a redraw!
	buffer_.changed();
	if (!fitCursor())
		// The screen has already been updated thanks to the
		// 'buffer_.changed()' call three line above. So no need
		// to redraw again.
		return;

	// The screen has been recentered around the cursor position so
	// refresh it:
	updateMetrics();
	buffer_.changed();
}


void BufferView::updateScrollbar()
{
	Text & t = buffer_.text();
	TextMetrics & tm = d->text_metrics_[&t];

	int const parsize = int(t.paragraphs().size() - 1);
	if (d->anchor_ref_ >  parsize)  {
		d->anchor_ref_ = parsize;
		d->offset_ref_ = 0;
	}

	LYXERR(Debug::GUI)
		<< BOOST_CURRENT_FUNCTION
		<< " Updating scrollbar: height: " << t.paragraphs().size()
		<< " curr par: " << d->cursor_.bottom().pit()
		<< " default height " << defaultRowHeight() << endl;

	// It would be better to fix the scrollbar to understand
	// values in [0..1] and divide everything by wh

	// estimated average paragraph height:
	if (d->wh_ == 0)
		d->wh_ = height_ / 4;

	int h = tm.parMetrics(d->anchor_ref_).height();

	// Normalize anchor/offset (MV):
	while (d->offset_ref_ > h && d->anchor_ref_ < parsize) {
		d->anchor_ref_++;
		d->offset_ref_ -= h;
		h = tm.parMetrics(d->anchor_ref_).height();
	}
	// Look at paragraph heights on-screen
	int sumh = 0;
	int nh = 0;
	for (pit_type pit = d->anchor_ref_; pit <= parsize; ++pit) {
		if (sumh > height_)
			break;
		int const h2 = tm.parMetrics(pit).height();
		sumh += h2;
		nh++;
	}

	BOOST_ASSERT(nh);
	int const hav = sumh / nh;
	// More realistic average paragraph height
	if (hav > d->wh_)
		d->wh_ = hav;

	BOOST_ASSERT(h);
	d->scrollbarParameters_.height = (parsize + 1) * d->wh_;
	d->scrollbarParameters_.position = d->anchor_ref_ * d->wh_ + int(d->offset_ref_ * d->wh_ / float(h));
	d->scrollbarParameters_.lineScrollHeight = int(d->wh_ * defaultRowHeight() / float(h));
}


ScrollbarParameters const & BufferView::scrollbarParameters() const
{
	return d->scrollbarParameters_;
}


void BufferView::scrollDocView(int value)
{
	LYXERR(Debug::GUI) << BOOST_CURRENT_FUNCTION
			   << "[ value = " << value << "]" << endl;

	Text & t = buffer_.text();
	TextMetrics & tm = d->text_metrics_[&t];

	float const bar = value / float(d->wh_ * t.paragraphs().size());

	d->anchor_ref_ = int(bar * t.paragraphs().size());
	if (d->anchor_ref_ >  int(t.paragraphs().size()) - 1)
		d->anchor_ref_ = int(t.paragraphs().size()) - 1;

	tm.redoParagraph(d->anchor_ref_);
	int const h = tm.parMetrics(d->anchor_ref_).height();
	d->offset_ref_ = int((bar * t.paragraphs().size() - d->anchor_ref_) * h);
	updateMetrics();
	buffer_.changed();
}


void BufferView::setCursorFromScrollbar()
{
	TextMetrics & tm = d->text_metrics_[&buffer_.text()];

	int const height = 2 * defaultRowHeight();
	int const first = height;
	int const last = height_ - height;
	Cursor & cur = d->cursor_;

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
	if (!d->cursor_.selection())
		return Change(Change::UNCHANGED);

	DocIterator dit = d->cursor_.selectionBegin();
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


bool BufferView::moveToPosition(pit_type bottom_pit, pos_type bottom_pos,
	int top_id, pos_type top_pos)
{
	bool success = false;
	DocIterator doc_it;

	d->cursor_.clearSelection();

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
		d->cursor_.setCurrentFont();
		// center the screen on this new position.
		center();
	}

	return success;
}


void BufferView::translateAndInsert(char_type c, Text * t, Cursor & cur)
{
	if (lyxrc.rtl_support) {
		if (d->cursor_.real_current_font.isRightToLeft()) {
			if (d->intl_.keymap == Intl::PRIMARY)
				d->intl_.keyMapSec();
		} else {
			if (d->intl_.keymap == Intl::SECONDARY)
				d->intl_.keyMapPrim();
		}
	}

	d->intl_.getTransManager().translateAndInsert(c, t, cur);
}


int BufferView::workWidth() const
{
	return width_;
}


void BufferView::updateOffsetRef()
{
	// No need to update d->offset_ref_ in this case.
	if (!d->need_centering_)
		return;

	// We are not properly started yet, delay until resizing is
	// done.
	if (height_ == 0)
		return;

	CursorSlice & bot = d->cursor_.bottom();
	TextMetrics & tm = d->text_metrics_[bot.text()];
	ParagraphMetrics const & pm = tm.parMetrics(bot.pit());
	int y = coordOffset(d->cursor_, d->cursor_.boundary()).y_;
	d->offset_ref_ = y + pm.ascent() - height_ / 2;

	d->need_centering_ = false;
}


void BufferView::center()
{
	d->anchor_ref_ = d->cursor_.bottom().pit();
	d->need_centering_ = true;
}


FuncStatus BufferView::getStatus(FuncRequest const & cmd)
{
	FuncStatus flag;

	Cursor & cur = d->cursor_;

	switch (cmd.action) {

	case LFUN_UNDO:
		flag.enabled(buffer_.undo().hasUndoStack());
		break;
	case LFUN_REDO:
		flag.enabled(buffer_.undo().hasRedoStack());
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
	case LFUN_INFO_INSERT:
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
		    || getInsetByCode<InsetRef>(cur, REF_CODE));
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
	
	case LFUN_BUFFER_TOGGLE_EMBEDDING: {
		flag.setOnOff(buffer_.params().embedded);
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

	case LFUN_LAYOUT_TABULAR:
		flag.enabled(cur.innerInsetOfType(TABULAR_CODE));
		break;

	case LFUN_LAYOUT:
	case LFUN_LAYOUT_PARAGRAPH:
		flag.enabled(cur.inset().forceDefaultParagraphs(cur.idx()));
		break;

	case LFUN_INSET_SETTINGS: {
		InsetCode code = cur.inset().lyxCode();
		bool enable = false;
		switch (code) {
			case TABULAR_CODE:
				enable = cmd.argument() == "tabular";
				break;
			case ERT_CODE:
				enable = cmd.argument() == "ert";
				break;
			case FLOAT_CODE:
				enable = cmd.argument() == "float";
				break;
			case WRAP_CODE:
				enable = cmd.argument() == "wrap";
				break;
			case NOTE_CODE:
				enable = cmd.argument() == "note";
				break;
			case BRANCH_CODE:
				enable = cmd.argument() == "branch";
				break;
			case BOX_CODE:
				enable = cmd.argument() == "box";
				break;
			case LISTINGS_CODE:
				enable = cmd.argument() == "listings";
				break;
			default:
				break;
		}
		flag.enabled(enable);
		break;
	}

	case LFUN_DIALOG_SHOW_NEW_INSET:
		flag.enabled(cur.inset().lyxCode() != ERT_CODE &&
			cur.inset().lyxCode() != LISTINGS_CODE);
		if (cur.inset().lyxCode() == CAPTION_CODE) {
			FuncStatus flag;
			if (cur.inset().getStatus(cur, cmd, flag))
				return flag;
		}
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

	Cursor & cur = d->cursor_;
	// Default Update flags.
	Update::flags updateFlags = Update::Force | Update::FitCursor;

	switch (cmd.action) {

	case LFUN_UNDO:
		cur.message(_("Undo"));
		cur.clearSelection();
		if (!cur.textUndo()) {
			cur.message(_("No further undo information"));
			updateFlags = Update::None;
		}
		break;

	case LFUN_REDO:
		cur.message(_("Redo"));
		cur.clearSelection();
		if (!cur.textRedo()) {
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
		insertPlaintextFile(to_utf8(cmd.argument()), true);
		break;

	case LFUN_FILE_INSERT_PLAINTEXT:
		// FIXME UNICODE
		insertPlaintextFile(to_utf8(cmd.argument()), false);
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
				getInsetByCode<InsetRef>(d->cursor_,
							 REF_CODE);
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
					<< b->absFileName() << "'." << endl;

				if (b == &buffer_) {
					// Set the cursor
					setCursor(makeDocIterator(par, 0));
				} else {
					// Switch to other buffer view and resend cmd
					theLyXFunc().dispatch(FuncRequest(
						LFUN_BUFFER_SWITCH, b->absFileName()));
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
		gotoInset(this, NOTE_CODE, false);
		break;

	case LFUN_REFERENCE_NEXT: {
		vector<InsetCode> tmp;
		tmp.push_back(LABEL_CODE);
		tmp.push_back(REF_CODE);
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
		d->cursor_.reset(buffer_.inset());
		d->cursor_.selHandle(true);
		buffer_.text().cursorBottom(d->cursor_);
		// accept everything in a single step to support atomic undo
		buffer_.text().acceptOrRejectChanges(d->cursor_, Text::ACCEPT);
		break;

	case LFUN_ALL_CHANGES_REJECT:
		// select complete document
		d->cursor_.reset(buffer_.inset());
		d->cursor_.selHandle(true);
		buffer_.text().cursorBottom(d->cursor_);
		// reject everything in a single step to support atomic undo
		// Note: reject does not work recursively; the user may have to repeat the operation
		buffer_.text().acceptOrRejectChanges(d->cursor_, Text::REJECT);
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
		Cursor tmpcur = d->cursor_;
		findInset(tmpcur, BIBTEX_CODE, false);
		InsetBibtex * inset = getInsetByCode<InsetBibtex>(tmpcur,
						BIBTEX_CODE);
		if (inset) {
			if (inset->addDatabase(to_utf8(cmd.argument())))
				buffer_.updateBibfilesCache();
		}
		break;
	}

	case LFUN_BIBTEX_DATABASE_DEL: {
		Cursor tmpcur = d->cursor_;
		findInset(tmpcur, BIBTEX_CODE, false);
		InsetBibtex * inset = getInsetByCode<InsetBibtex>(tmpcur,
						BIBTEX_CODE);
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
	
	case LFUN_BUFFER_TOGGLE_EMBEDDING:
		// turn embedding on/off
		buffer_.embeddedFiles().enable(!buffer_.params().embedded);
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
				tmpcur.pushBackward(*inset);
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
			updateMetrics();
			//FIXME: updateMetrics() does not update paragraph position
			// This is done at draw() time. So we need a redraw!
			// But no screen update is needed.
			d->metrics_info_.update_strategy = NoScreenUpdate;
			buffer_.changed();
			p = getPos(cur, cur.boundary());
		}
		scroll(cmd.action == LFUN_SCREEN_UP? - height_ : height_);
		cur.reset(buffer_.inset());
		d->text_metrics_[&buffer_.text()].editXY(cur, p.x_, p.y_);
		//FIXME: what to do with cur.x_target()?
		cur.finishUndo();
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
		d->text_metrics_[&buffer_.text()].editXY(cur, p.x_, p.y_);
		cur.finishUndo();
		while (cur.depth() > initial_depth) {
			cur.forwardInset();
		}
		// FIXME: we need to do a redraw again because of the selection
		// But no screen update is needed.
		d->metrics_info_.update_strategy = NoScreenUpdate;
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
	Cursor & cur = d->cursor_;

	if (!cur.selection()) {
		d->xsel_cache_.set = false;
		return docstring();
	}

	if (!d->xsel_cache_.set ||
	    cur.top() != d->xsel_cache_.cursor ||
	    cur.anchor_.top() != d->xsel_cache_.anchor)
	{
		d->xsel_cache_.cursor = cur.top();
		d->xsel_cache_.anchor = cur.anchor_.top();
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
	buffer_.changed();
}


void BufferView::resize(int width, int height)
{
	// Update from work area
	width_ = width;
	height_ = height;

	updateMetrics();
}


Inset const * BufferView::getCoveringInset(Text const & text, int x, int y)
{
	TextMetrics & tm = d->text_metrics_[&text];
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


void BufferView::mouseEventDispatch(FuncRequest const & cmd0)
{
	//lyxerr << BOOST_CURRENT_FUNCTION << "[ cmd0 " << cmd0 << "]" << endl;

	// This is only called for mouse related events including
	// LFUN_FILE_OPEN generated by drag-and-drop.
	FuncRequest cmd = cmd0;

	Cursor cur(*this);
	cur.push(buffer_.inset());
	cur.selection() = d->cursor_.selection();

	// Either the inset under the cursor or the
	// surrounding Text will handle this event.

	// make sure we stay within the screen...
	cmd.y = min(max(cmd.y, -1), height_);

	if (cmd.action == LFUN_MOUSE_MOTION && cmd.button() == mouse_button::none) {

		// Get inset under mouse, if there is one.
		Inset const * covering_inset =
			getCoveringInset(buffer_.text(), cmd.x, cmd.y);
		if (covering_inset == d->last_inset_)
			// Same inset, no need to do anything...
			return;

		bool need_redraw = false;
		// const_cast because of setMouseHover().
		Inset * inset = const_cast<Inset *>(covering_inset);
		if (d->last_inset_)
			// Remove the hint on the last hovered inset (if any).
			need_redraw |= d->last_inset_->setMouseHover(false);
		if (inset)
			// Highlighted the newly hovered inset (if any).
			need_redraw |= inset->setMouseHover(true);
		d->last_inset_ = inset;
		if (!need_redraw)
			return;

		// if last metrics update was in singlepar mode, WorkArea::redraw() will
		// not expose the button for redraw. We adjust here the metrics dimension
		// to enable a full redraw in any case as this is not costly.
		TextMetrics & tm = d->text_metrics_[&buffer_.text()];
		std::pair<pit_type, ParagraphMetrics const *> firstpm = tm.first();
		std::pair<pit_type, ParagraphMetrics const *> lastpm = tm.last();
		int y1 = firstpm.second->position() - firstpm.second->ascent();
		int y2 = lastpm.second->position() + lastpm.second->descent();
		d->metrics_info_ = ViewMetricsInfo(firstpm.first, lastpm.first, y1, y2,
			FullScreenUpdate, buffer_.text().paragraphs().size());
		// Reinitialize anchor to first pit.
		d->anchor_ref_ = firstpm.first;
		d->offset_ref_ = -y1;
		LYXERR(Debug::PAINTING)
			<< "Mouse hover detected at: (" << cmd.x << ", " << cmd.y << ")"
			<< "\nTriggering redraw: y1: " << y1 << " y2: " << y2
			<< " pit1: " << firstpm.first << " pit2: " << lastpm.first << endl;

		// This event (moving without mouse click) is not passed further.
		// This should be changed if it is further utilized.
		buffer_.changed();
		return;
	}

	// Build temporary cursor.
	Inset * inset = d->text_metrics_[&buffer_.text()].editXY(cur, cmd.x, cmd.y);

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

	//Do we have a selection?
	theSelection().haveSelection(cursor().selection());

	// If the command has been dispatched,
	if (cur.result().dispatched()
		// an update is asked,
		&& cur.result().update())
		processUpdateFlags(cur.result().update());
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
	TextMetrics & tm = d->text_metrics_[text];
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
	d->offset_ref_ += offset;
	updateMetrics();
	buffer_.changed();
}


void BufferView::scrollUp(int offset)
{
	Text * text = &buffer_.text();
	TextMetrics & tm = d->text_metrics_[text];
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
	d->offset_ref_ -= offset;
	updateMetrics();
	buffer_.changed();
}


void BufferView::setCursorFromRow(int row)
{
	int tmpid = -1;
	int tmppos = -1;

	buffer_.texrow().getIdFromRow(row, tmpid, tmppos);

	d->cursor_.reset(buffer_.inset());
	if (tmpid == -1)
		buffer_.text().setCursor(d->cursor_, 0, 0);
	else
		buffer_.text().setCursor(d->cursor_, buffer_.getParFromID(tmpid).pit(), tmppos);
}


void BufferView::gotoLabel(docstring const & label)
{
	for (InsetIterator it = inset_iterator_begin(buffer_.inset()); it; ++it) {
		vector<docstring> labels;
		it->getLabelList(buffer_, labels);
		if (std::find(labels.begin(), labels.end(), label) != labels.end()) {
			setCursor(it);
			processUpdateFlags(Update::FitCursor);
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
	size_t const n = dit.depth();
	for (size_t i = 0; i < n; ++i)
		dit[i].inset().edit(d->cursor_, true);

	d->cursor_.setCursor(dit);
	d->cursor_.selection() = false;
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

	updateLabels(buffer_);

	updateMetrics();
	buffer_.changed();
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
	bool leftinset = (&d->cursor_.inset() != &cur.inset());
	if (leftinset)
		badcursor = notifyCursorLeaves(d->cursor_, cur);

	// FIXME: shift-mouse selection doesn't work well across insets.
	bool do_selection = select && &d->cursor_.anchor().inset() == &cur.inset();

	// do the dEPM magic if needed
	// FIXME: (1) move this to InsetText::notifyCursorLeaves?
	// FIXME: (2) if we had a working InsetText::notifyCursorLeaves,
	// the leftinset bool would not be necessary (badcursor instead).
	bool update = leftinset;
	if (!do_selection && !badcursor && d->cursor_.inTexted())
		update |= checkDepm(cur, d->cursor_);

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

	d->cursor_.setCursor(dit);
	d->cursor_.boundary(cur.boundary());
	if (do_selection)
		d->cursor_.setSelection();
	else
		d->cursor_.clearSelection();

	d->cursor_.finishUndo();
	d->cursor_.setCurrentFont();
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
	return d->anchor_ref_;
}


ViewMetricsInfo const & BufferView::viewMetricsInfo()
{
	return d->metrics_info_;
}


bool BufferView::singleParUpdate()
{
	Text & buftext = buffer_.text();
	pit_type const bottom_pit = d->cursor_.bottom().pit();
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
	d->metrics_info_ = ViewMetricsInfo(bottom_pit, bottom_pit, y1, y2,
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


void BufferView::updateMetrics()
{
	Text & buftext = buffer_.text();
	pit_type const npit = int(buftext.paragraphs().size());

	if (d->anchor_ref_ > int(npit - 1)) {
		d->anchor_ref_ = int(npit - 1);
		d->offset_ref_ = 0;
	}

	// Clear out the position cache in case of full screen redraw,
	d->coord_cache_.clear();

	// Clear out paragraph metrics to avoid having invalid metrics
	// in the cache from paragraphs not relayouted below
	// The complete text metrics will be redone.
	d->text_metrics_.clear();

	TextMetrics & tm = textMetrics(&buftext);

	pit_type const pit = d->anchor_ref_;
	int pit1 = pit;
	int pit2 = pit;

	// Rebreak anchor paragraph.
	tm.redoParagraph(pit);

	// Take care of anchor offset if case a recentering is needed.
	updateOffsetRef();

	int y0 = tm.parMetrics(pit).ascent() - d->offset_ref_;

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
	d->anchor_ref_ = pit1;
	d->offset_ref_ = -y1;

	// Grey at the beginning is ugly
	if (pit1 == 0 && y1 > 0) {
		y0 -= y1;
		y1 = 0;
		d->anchor_ref_ = 0;
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

	d->metrics_info_ = ViewMetricsInfo(pit1, pit2, y1, y2,
		FullScreenUpdate, npit);

	if (lyxerr.debugging(Debug::WORKAREA)) {
		LYXERR(Debug::WORKAREA) << "BufferView::updateMetrics" << endl;
		d->coord_cache_.dump();
	}
}


void BufferView::menuInsertLyXFile(string const & filenm)
{
	BOOST_ASSERT(d->cursor_.inTexted());
	string filename = filenm;

	if (filename.empty()) {
		// Launch a file browser
		// FIXME UNICODE
		string initpath = lyxrc.document_path;
		string const trypath = buffer_.filePath();
		// If directory is writeable, use this as default.
		if (FileName(trypath).isDirWritable())
			initpath = trypath;

		// FIXME UNICODE
		FileDialog dlg(_("Select LyX document to insert"), LFUN_FILE_INSERT);
		dlg.setButton1(_("Documents|#o#O"), from_utf8(lyxrc.document_path));
		dlg.setButton2(_("Examples|#E#e"),
			from_utf8(addPath(package().system_support().absFilename(),
			"examples")));

		FileDialog::Result result =
			dlg.open(from_utf8(initpath),
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
	if (buf.loadLyXFile(FileName(filename))) {
		ErrorList & el = buffer_.errorList("Parse");
		// Copy the inserted document error list into the current buffer one.
		el = buf.errorList("Parse");
		buffer_.undo().recordUndo(d->cursor_);
		cap::pasteParagraphList(d->cursor_, buf.paragraphs(),
					     buf.params().getTextClassPtr(), el);
		res = _("Document %1$s inserted.");
	} else {
		res = _("Could not insert document %1$s");
	}

	// emit message signal.
	message(bformat(res, disp_fn));
	buffer_.errors("Parse");
	updateMetrics();
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
	LYXERR(Debug::PAINTING) << "\t\t*** START DRAWING ***" << endl;
	Text & text = buffer_.text();
	TextMetrics const & tm = d->text_metrics_[&text];
	int const y = d->metrics_info_.y1 
		+ tm.parMetrics(d->metrics_info_.p1).ascent();
	PainterInfo pi(this, pain);

	switch (d->metrics_info_.update_strategy) {

	case NoScreenUpdate:
		// If no screen painting is actually needed, only some the different
		// coordinates of insets and paragraphs needs to be updated.
		pi.full_repaint = true;
		pi.pain.setDrawingEnabled(false);
 		tm.draw(pi, 0, y);
		break;

	case SingleParUpdate:
		// Only the current outermost paragraph will be redrawn.
		pi.full_repaint = false;
		tm.drawParagraph(pi, d->metrics_info_.p1, 0, y);
		break;

	case DecorationUpdate:
		// FIXME: We should also distinguish DecorationUpdate to avoid text
		// drawing if possible. This is not possible to do easily right now
		// because of the single backing pixmap.

	case FullScreenUpdate:
		// The whole screen, including insets, will be refreshed.
		pi.full_repaint = true;

		// Clear background (if not delegated to rows)
		pain.fillRectangle(0, d->metrics_info_.y1, width_,
			d->metrics_info_.y2 - d->metrics_info_.y1,
			buffer_.inset().backgroundColor());
		tm.draw(pi, 0, y);

		// and grey out above (should not happen later)
		if (d->metrics_info_.y1 > 0)
			pain.fillRectangle(0, 0, width_,
				d->metrics_info_.y1, Color_bottomarea);

		// and possibly grey out below
		if (d->metrics_info_.y2 < height_)
			pain.fillRectangle(0, d->metrics_info_.y2, width_,
				height_ - d->metrics_info_.y2, Color_bottomarea);
		break;
	}

	LYXERR(Debug::PAINTING) << "\n\t\t*** END DRAWING  ***" << endl;
}


void BufferView::message(docstring const & msg)
{
	if (d->gui_)
		d->gui_->message(msg);
}


void BufferView::showDialog(std::string const & name)
{
	if (d->gui_)
		d->gui_->showDialog(name);
}


void BufferView::showDialogWithData(std::string const & name,
	std::string const & data)
{
	if (d->gui_)
		d->gui_->showDialogWithData(name, data);
}


void BufferView::showInsetDialog(std::string const & name,
	std::string const & data, Inset * inset)
{
	if (d->gui_)
		d->gui_->showInsetDialog(name, data, inset);
}


void BufferView::updateDialog(std::string const & name, std::string const & data)
{
	if (d->gui_)
		d->gui_->updateDialog(name, data);
}


void BufferView::setGuiDelegate(frontend::GuiBufferViewDelegate * gui)
{
	d->gui_ = gui;
}


// FIXME: Move this out of BufferView again
docstring BufferView::contentsOfPlaintextFile(string const & f,
	bool asParagraph)
{
	FileName fname(f);

	if (fname.empty()) {
		FileDialog dlg(_("Select file to insert"),
				   ( asParagraph
				     ? LFUN_FILE_INSERT_PLAINTEXT_PARA 
				     : LFUN_FILE_INSERT_PLAINTEXT) );

		FileDialog::Result result =
			dlg.open(from_utf8(buffer().filePath()),
				     FileFilterList(), docstring());

		if (result.first == FileDialog::Later)
			return docstring();

		fname = makeAbsPath(to_utf8(result.second));

		if (fname.empty())
			return docstring();
	}

	if (!fs::is_readable(fname.toFilesystemEncoding())) {
		docstring const error = from_ascii(strerror(errno));
		docstring const file = makeDisplayPath(fname.absFilename(), 50);
		docstring const text =
		  bformat(_("Could not read the specified document\n"
			    "%1$s\ndue to the error: %2$s"), file, error);
		Alert::error(_("Could not read file"), text);
		return docstring();
	}

	ifstream ifs(fname.toFilesystemEncoding().c_str());
	if (!ifs) {
		docstring const error = from_ascii(strerror(errno));
		docstring const file = makeDisplayPath(fname.absFilename(), 50);
		docstring const text =
		  bformat(_("Could not open the specified document\n"
			    "%1$s\ndue to the error: %2$s"), file, error);
		Alert::error(_("Could not open file"), text);
		return docstring();
	}

	ifs.unsetf(std::ios::skipws);
	istream_iterator<char> ii(ifs);
	istream_iterator<char> end;
#if !defined(USE_INCLUDED_STRING) && !defined(STD_STRING_IS_GOOD)
	// We use this until the compilers get better...
	std::vector<char> tmp;
	copy(ii, end, back_inserter(tmp));
	string const tmpstr(tmp.begin(), tmp.end());
#else
	// This is what we want to use and what we will use once the
	// compilers get good enough.
	//string tmpstr(ii, end); // yet a reason for using std::string
	// alternate approach to get the file into a string:
	string tmpstr;
	copy(ii, end, back_inserter(tmpstr));
#endif

	// FIXME UNICODE: We don't know the encoding of the file
	docstring file_content = from_utf8(tmpstr);
	if (file_content.empty()) {
		Alert::error(_("Reading not UTF-8 encoded file"),
			     _("The file is not UTF-8 encoded.\n"
			       "It will be read as local 8Bit-encoded.\n"
			       "If this does not give the correct result\n"
			       "then please change the encoding of the file\n"
			       "to UTF-8 with a program other than LyX.\n"));
		file_content = from_local8bit(tmpstr);
	}

	return normalize_c(file_content);
}


void BufferView::insertPlaintextFile(string const & f, bool asParagraph)
{
	docstring const tmpstr = contentsOfPlaintextFile(f, asParagraph);

	if (tmpstr.empty())
		return;

	Cursor & cur = cursor();
	cap::replaceSelection(cur);
	buffer_.undo().recordUndo(cur);
	if (asParagraph)
		cur.innerText()->insertStringAsParagraphs(cur, tmpstr);
	else
		cur.innerText()->insertStringAsLines(cur, tmpstr);
}

} // namespace lyx
