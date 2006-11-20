/**
 * \file BufferView.C
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

#include "buffer.h"
#include "buffer_funcs.h"
#include "bufferlist.h"
#include "bufferparams.h"
#include "coordcache.h"
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
#include "language.h"
#include "LaTeXFeatures.h"
#include "lyx_cb.h" // added for Dispatch functions
#include "lyx_main.h"
#include "lyxfind.h"
#include "lyxfunc.h"
#include "lyxlayout.h"
#include "lyxtext.h"
#include "lyxtextclass.h"
#include "lyxrc.h"
#include "session.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "pariterator.h"
#include "texrow.h"
#include "toc.h"
#include "undo.h"
#include "vspace.h"
#include "WordLangTuple.h"
#include "metricsinfo.h"

#include "insets/insetbibtex.h"
#include "insets/insetcommand.h" // ChangeRefs
#include "insets/insetref.h"
#include "insets/insettext.h"

#include "frontends/Alert.h"
#include "frontends/FileDialog.h"
#include "frontends/FontMetrics.h"

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


namespace lyx {

using support::addPath;
using support::bformat;
using support::FileFilterList;
using support::fileSearch;
using support::isDirWriteable;
using support::makeDisplayPath;
using support::makeAbsPath;
using support::package;

using std::distance;
using std::endl;
using std::istringstream;
using std::make_pair;
using std::min;
using std::max;
using std::mem_fun_ref;
using std::string;
using std::vector;

namespace Alert = frontend::Alert;

namespace {

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


BufferView::BufferView()
	: width_(0), height_(0), buffer_(0), wh_(0),
	  cursor_(*this),
	  multiparsel_cache_(false), anchor_ref_(0), offset_ref_(0),
	  intl_(new Intl)
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


void BufferView::setBuffer(Buffer * b)
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
		LyX::ref().session().lastFilePos().save(buffer_->fileName(),
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
		buffer_ = theBufferList().first();
	} else {
		// Set current buffer
		buffer_ = b;
	}

	// Reset old cursor
	cursor_ = LCursor(*this);
	anchor_ref_ = 0;
	offset_ref_ = 0;

	if (buffer_) {
		lyxerr[Debug::INFO] << BOOST_CURRENT_FUNCTION
				    << "Buffer addr: " << buffer_ << endl;
		cursor_.push(buffer_->inset());
		cursor_.resetAnchor();
		buffer_->text().init(this);
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

	if (buffer_)
		updateMetrics(false);    

	if (buffer_ && graphics::Previews::status() != LyXRC::PREVIEW_OFF)
		graphics::Previews::get().generateBufferPreviews(*buffer_);
}


bool BufferView::loadLyXFile(string const & filename, bool tolastfiles)
{
	// Get absolute path of file and add ".lyx"
	// to the filename if necessary
	string s = fileSearch(string(), filename, "lyx");

	bool const found = !s.empty();

	if (!found)
		s = filename;

	// File already open?
	if (theBufferList().exists(s)) {
		docstring const file = makeDisplayPath(s, 20);
		docstring text = bformat(_("The document %1$s is already "
						     "loaded.\n\nDo you want to revert "
						     "to the saved version?"), file);
		int const ret = Alert::prompt(_("Revert to saved document?"),
			text, 0, 1,  _("&Revert"), _("&Switch to document"));

		if (ret != 0) {
			setBuffer(theBufferList().getBuffer(s));
			return true;
		}
		// FIXME: should be LFUN_REVERT
		if (!theBufferList().close(theBufferList().getBuffer(s), false))
			return false;
		// Fall through to new load. (Asger)
		buffer_ = 0;
	}

	Buffer * b = 0;

	if (found) {
		b = theBufferList().newBuffer(s);
		if (!lyx::loadLyXFile(b, s)) {
			theBufferList().release(b);
			return false;
		}
	} else {
		docstring text = bformat(_("The document %1$s does not yet "
						     "exist.\n\nDo you want to create "
						     "a new document?"), from_utf8(s));
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
		pit_type pit;
		pos_type pos;
		boost::tie(pit, pos) = LyX::ref().session().lastFilePos().load(s);
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
					setCursor(makeDocIterator(it, min(pos, it->size())));
					// No need to update the metrics if fitCursor returns false.
					if (fitCursor())
						updateMetrics(false);
					break;
				}
		}
	}

	if (tolastfiles)
		LyX::ref().session().lastFiles().add(b->fileName());

	return true;
}


void BufferView::reload()
{
	string const fn = buffer_->fileName();
	if (theBufferList().close(buffer_, false))
		loadLyXFile(fn);
}


void BufferView::resize()
{
	if (!buffer_)
		return;

	lyxerr[Debug::DEBUG] << BOOST_CURRENT_FUNCTION << endl;

	buffer_->text().init(this);
	update();
	switchKeyMap();
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


std::pair<bool, bool> BufferView::update(Update::flags flags)
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
		return make_pair(false, false);

	if (lyxerr.debugging(Debug::WORKAREA)) {
		lyxerr[Debug::WORKAREA] << "BufferView::update" << std::endl;
	}

	// Update macro store
	buffer_->buildMacros();

	// Now do the first drawing step if needed. This consists on updating
	// the CoordCache in updateMetrics().
	// The second drawing step is done in WorkArea::redraw() if needed.

	// Case when no explicit update is requested.
	if (!flags) {
		// no need to do anything.
		return make_pair(false, false);
	}

	bool full_metrics = flags & Update::Force;
	if (flags & Update::MultiParSel)
		full_metrics |= multiParSel();

	bool const single_par = !full_metrics;
	updateMetrics(single_par);

	if (flags & Update::FitCursor && fitCursor())
		updateMetrics(false);

	// tell the frontend to update the screen.
	return make_pair(true, single_par);
}


void BufferView::updateScrollbar()
{
	if (!buffer_) {
		lyxerr[Debug::DEBUG] << BOOST_CURRENT_FUNCTION
				     << " no text in updateScrollbar" << endl;
		scrollbarParameters_.reset();
		return;
	}

	LyXText & t = buffer_->text();
	int const parsize = int(t.paragraphs().size() - 1);
	if (anchor_ref_ >  parsize)  {
		anchor_ref_ = parsize;
		offset_ref_ = 0;
	}

	if (lyxerr.debugging(Debug::GUI)) {
		lyxerr[Debug::GUI]
			<< BOOST_CURRENT_FUNCTION
			<< " Updating scrollbar: height: " << t.paragraphs().size()
			<< " curr par: " << cursor_.bottom().pit()
			<< " default height " << defaultRowHeight() << endl;
	}

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
	for (pit_type pit = anchor_ref_; pit <= parsize; ++pit) {
		if (sumh > height_)
			break;
		int const h2 = t.getPar(pit).height();
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
	lyxerr[Debug::GUI] << BOOST_CURRENT_FUNCTION
			   << "[ value = " << value << "]" << endl;

	if (!buffer_)
		return;

	LyXText & t = buffer_->text();

	float const bar = value / float(wh_ * t.paragraphs().size());

	anchor_ref_ = int(bar * t.paragraphs().size());
	if (anchor_ref_ >  int(t.paragraphs().size()) - 1)
		anchor_ref_ = int(t.paragraphs().size()) - 1;
	t.redoParagraph(*this, anchor_ref_);
	int const h = t.getPar(anchor_ref_).height();
	offset_ref_ = int((bar * t.paragraphs().size() - anchor_ref_) * h);
	updateMetrics(false);
}


void BufferView::setCursorFromScrollbar()
{
	LyXText & t = buffer_->text();

	int const height = 2 * defaultRowHeight();
	int const first = height;
	int const last = height_ - height;
	LCursor & cur = cursor_;

	bv_funcs::CurStatus st = bv_funcs::status(this, cur);

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
		int const y = bv_funcs::getPos(*this, cur, cur.boundary()).y_;
		int const newy = min(last, max(y, first));
		if (y != newy) {
			cur.reset(buffer_->inset());
			t.setCursorFromCoordinates(cur, 0, newy);
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


void BufferView::saveBookmark(bool persistent)
{
	LyX::ref().session().bookmarks().save(
		buffer_->fileName(),
		cursor_.paragraph().id(),
		cursor_.pos(),
		persistent
	);
	if (persistent)
		// emit message signal.
		message(_("Save bookmark"));
}


void BufferView::moveToPosition(int par_id, pos_type par_pos)
{
	cursor_.clearSelection();

	ParIterator par = buffer_->getParFromID(par_id);
	if (par == buffer_->par_iterator_end())
		return;

	setCursor(makeDocIterator(par, min(par->size(), par_pos)));
}


void BufferView::switchKeyMap()
{
	if (!lyxrc.rtl_support)
		return;

	if (getLyXText()->real_current_font.isRightToLeft()) {
		if (intl_->keymap == Intl::PRIMARY)
			intl_->keyMapSec();
	} else {
		if (intl_->keymap == Intl::SECONDARY)
			intl_->keyMapPrim();
	}
}


int BufferView::workWidth() const
{
	return width_;
}


void BufferView::center()
{
	CursorSlice & bot = cursor_.bottom();
	pit_type const pit = bot.pit();
	bot.text()->redoParagraph(*this, pit);
	Paragraph const & par = bot.text()->paragraphs()[pit];
	anchor_ref_ = pit;
	offset_ref_ = bv_funcs::coordOffset(*this, cursor_, cursor_.boundary()).y_
		+ par.ascent() - height_ / 2;
}


FuncStatus BufferView::getStatus(FuncRequest const & cmd)
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
		flag.enabled(!cmd.argument().empty()
		    || getInsetByCode<InsetRef>(cursor_, InsetBase::REF_CODE));
		break;
	}

	case LFUN_CHANGES_TRACK:
		flag.enabled(true);
		flag.setOnOff(buffer_->params().trackChanges);
		break;

	case LFUN_CHANGES_OUTPUT: {
		OutputParams runparams;
		LaTeXFeatures features(*buffer_, buffer_->params(), runparams);
		flag.enabled(buffer_ && features.isAvailable("dvipost"));
		flag.setOnOff(buffer_->params().outputChanges);
		break;
	}

	case LFUN_CHANGES_MERGE:
	case LFUN_CHANGE_NEXT:
	case LFUN_ALL_CHANGES_ACCEPT:
	case LFUN_ALL_CHANGES_REJECT:
		flag.enabled(buffer_); // FIXME: Change tracking (MG)
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


bool BufferView::dispatch(FuncRequest const & cmd)
{
	//lyxerr << BOOST_CURRENT_FUNCTION
	//       << [ cmd = " << cmd << "]" << endl;

	// Make sure that the cached BufferView is correct.
	lyxerr[Debug::ACTION] << BOOST_CURRENT_FUNCTION
		<< " action[" << cmd.action << ']'
		<< " arg[" << to_utf8(cmd.argument()) << ']'
		<< " x[" << cmd.x << ']'
		<< " y[" << cmd.y << ']'
		<< " button[" << cmd.button() << ']'
		<< endl;

	LCursor & cur = cursor_;

	switch (cmd.action) {

	case LFUN_UNDO:
		if (buffer_) {
			cur.message(_("Undo"));
			cur.clearSelection();
			if (!textUndo(*this))
				cur.message(_("No further undo information"));
			update();
			switchKeyMap();
		}
		break;

	case LFUN_REDO:
		if (buffer_) {
			cur.message(_("Redo"));
			cur.clearSelection();
			if (!textRedo(*this))
				cur.message(_("No further redo information"));
			update();
			switchKeyMap();
		}
		break;

	case LFUN_FILE_INSERT:
		// FIXME: We don't know the encoding of filenames
		menuInsertLyXFile(to_utf8(cmd.argument()));
		break;

	case LFUN_FILE_INSERT_ASCII_PARA:
		// FIXME: We don't know the encoding of filenames
		insertAsciiFile(this, to_utf8(cmd.argument()), true);
		break;

	case LFUN_FILE_INSERT_ASCII:
		// FIXME: We don't know the encoding of filenames
		insertAsciiFile(this, to_utf8(cmd.argument()), false);
		break;

	case LFUN_FONT_STATE:
		cur.message(from_utf8(cur.currentState()));
		break;

	case LFUN_BOOKMARK_SAVE:
		saveBookmark(convert<unsigned int>(to_utf8(cmd.argument())));
		break;

	case LFUN_LABEL_GOTO: {
		docstring label = cmd.argument();
		if (label.empty()) {
			InsetRef * inset =
				getInsetByCode<InsetRef>(cursor_,
							 InsetBase::REF_CODE);
			if (inset) {
				label = inset->getParam("reference");
				// persistent=false: use temp_bookmark
				saveBookmark(false);
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
				lyxerr[Debug::INFO]
					<< "No matching paragraph found! ["
					<< id << "]." << endl;
			} else {
				lyxerr[Debug::INFO]
					<< "Paragraph " << par->id()
					<< " found in buffer `"
					<< b->fileName() << "'." << endl;

				if (b == buffer_) {
					// Set the cursor
					setCursor(makeDocIterator(par, 0));
					update();
					switchKeyMap();
				} else {
					// Switch to other buffer view and resend cmd
					theLyXFunc().dispatch(FuncRequest(
						LFUN_BUFFER_SWITCH, b->fileName()));
					theLyXFunc().dispatch(cmd);
				}
				break;
			}
			++i;
		}
		break;
	}

	case LFUN_OUTLINE_UP:
		toc::outline(toc::Up, cursor_);
		cursor_.text()->setCursor(cursor_, cursor_.pit(), 0);
		updateLabels(*buffer_);
		break;
	case LFUN_OUTLINE_DOWN:
		toc::outline(toc::Down, cursor_);
		cursor_.text()->setCursor(cursor_, cursor_.pit(), 0);
		updateLabels(*buffer_);
		break;
	case LFUN_OUTLINE_IN:
		toc::outline(toc::In, cursor_);
		updateLabels(*buffer_);
		break;
	case LFUN_OUTLINE_OUT:
		toc::outline(toc::Out, cursor_);
		updateLabels(*buffer_);
		break;

	case LFUN_NOTE_NEXT:
		bv_funcs::gotoInset(this, InsetBase::NOTE_CODE, false);
		break;

	case LFUN_REFERENCE_NEXT: {
		vector<InsetBase_code> tmp;
		tmp.push_back(InsetBase::LABEL_CODE);
		tmp.push_back(InsetBase::REF_CODE);
		bv_funcs::gotoInset(this, tmp, true);
		break;
	}

	case LFUN_CHANGES_TRACK:
		buffer_->params().trackChanges = !buffer_->params().trackChanges;
		break;

	case LFUN_CHANGES_OUTPUT: {
		buffer_->params().outputChanges = !buffer_->params().outputChanges;
		break;
	}

	case LFUN_CHANGE_NEXT:
		findNextChange(this);
		break;

	case LFUN_CHANGES_MERGE:
		if (findNextChange(this))
			showDialog("changes");
		break;

	case LFUN_ALL_CHANGES_ACCEPT: {
		cursor_.reset(buffer_->inset());
#ifdef WITH_WARNINGS
#warning FIXME changes
#endif
		while (findNextChange(this))
			getLyXText()->acceptChange(cursor_);
		update();
		break;
	}

	case LFUN_ALL_CHANGES_REJECT: {
		cursor_.reset(buffer_->inset());
#ifdef WITH_WARNINGS
#warning FIXME changes
#endif
		while (findNextChange(this))
			getLyXText()->rejectChange(cursor_);
		break;
	}

	case LFUN_WORD_FIND:
		find(this, cmd);
		break;

	case LFUN_WORD_REPLACE:
		replace(this, cmd);
		break;

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
		LCursor tmpcur = cursor_;
		bv_funcs::findInset(tmpcur, InsetBase::BIBTEX_CODE, false);
		InsetBibtex * inset = getInsetByCode<InsetBibtex>(tmpcur,
						InsetBase::BIBTEX_CODE);
		if (inset) {
			if (inset->addDatabase(to_utf8(cmd.argument())))
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


docstring const BufferView::requestSelection()
{
	if (!buffer_)
		return docstring();

	LCursor & cur = cursor_;

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
		xsel_cache_.set = false;
	}
}


void BufferView::workAreaResize(int width, int height)
{
	bool const widthChange = width != width_;
	bool const heightChange = height != height_;

	// Update from work area
	width_ = width;
	height_ = height;

	if (buffer_)
		resize();

	update();
}


std::pair<bool, bool> BufferView::workAreaDispatch(FuncRequest const & cmd0)
{
	//lyxerr << BOOST_CURRENT_FUNCTION << "[ cmd0 " << cmd0 << "]" << endl;

	// This is only called for mouse related events including
	// LFUN_FILE_OPEN generated by drag-and-drop.
	FuncRequest cmd = cmd0;

	// E.g. Qt mouse press when no buffer
	if (!buffer_)
		return make_pair(false, false);

	LCursor cur(*this);
	cur.push(buffer_->inset());
	cur.selection() = cursor_.selection();

	// Either the inset under the cursor or the
	// surrounding LyXText will handle this event.

	// Build temporary cursor.
	cmd.y = min(max(cmd.y, -1), height_);
	InsetBase * inset = buffer_->text().editXY(cur, cmd.x, cmd.y);

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
	if (inset) {
		inset->dispatch(cur, cmd);
	}

	// Now dispatch to the temporary cursor. If the real cursor should
	// be modified, the inset's dispatch has to do so explicitly.
	if (!cur.result().dispatched())
		cur.dispatch(cmd);

	// Redraw if requested and necessary.
	if (cur.result().dispatched() && cur.result().update())
		return update(cur.result().update());

	// When the above and the inner function are fixed, we can do this:
	return make_pair(false, false);
}


void BufferView::scroll(int /*lines*/)
{
//	if (!buffer_)
//		return;
//
//	LyXText const * t = &buffer_->text();
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


void BufferView::setCursorFromRow(int row)
{
	int tmpid = -1;
	int tmppos = -1;

	buffer_->texrow().getIdFromRow(row, tmpid, tmppos);

	if (tmpid == -1)
		buffer_->text().setCursor(cursor_, 0, 0);
	else
		buffer_->text().setCursor(cursor_, buffer_->getParFromID(tmpid).pit(), tmppos);
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


LyXText * BufferView::getLyXText()
{
	LyXText * text = cursor_.innerText();
	BOOST_ASSERT(text);
	return text;
}


LyXText const * BufferView::getLyXText() const
{
	LyXText const * text = cursor_.innerText();
	BOOST_ASSERT(text);
	return text;
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


void BufferView::mouseSetCursor(LCursor & cur)
{
	BOOST_ASSERT(&cur.bv() == this);

	// Has the cursor just left the inset?
	bool badcursor = false;
	if (&cursor_.inset() != &cur.inset())
		badcursor = cursor_.inset().notifyCursorLeaves(cursor_);

	// do the dEPM magic if needed
	// FIXME: move this to InsetText::notifyCursorLeaves?
	if (!badcursor && cursor_.inTexted())
		cursor_.text()->deleteEmptyParagraphMechanism(cur, cursor_);

	cursor_ = cur;
	cursor_.clearSelection();
	cursor_.setTargetX();
	finishUndo();

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


LCursor & BufferView::cursor()
{
	return cursor_;
}


LCursor const & BufferView::cursor() const
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


void BufferView::updateMetrics(bool singlepar)
{
	// Clear out the position cache in case of full screen redraw.
	if (!singlepar)
		coord_cache_.clear();

	LyXText & buftext = buffer_->text();
	pit_type size = int(buftext.paragraphs().size());

	if (anchor_ref_ > int(buftext.paragraphs().size() - 1)) {
		anchor_ref_ = int(buftext.paragraphs().size() - 1);
		offset_ref_ = 0;
	}

	pit_type const pit = anchor_ref_;
	int pit1 = pit;
	int pit2 = pit;
	size_t const npit = buftext.paragraphs().size();

	// Rebreak anchor paragraph. In Single Paragraph mode, rebreak only
	// the (main text, not inset!) paragraph containing the cursor.
	// (if this paragraph contains insets etc., rebreaking will
	// recursively descend)
	if (!singlepar || pit == cursor_.bottom().pit())
		buftext.redoParagraph(*this, pit);
	int y0 = buftext.getPar(pit).ascent() - offset_ref_;

	// Redo paragraphs above anchor if necessary; again, in Single Par
	// mode, only if we encounter the (main text) one having the cursor.
	int y1 = y0;
	while (y1 > 0 && pit1 > 0) {
		y1 -= buftext.getPar(pit1).ascent();
		--pit1;
		if (!singlepar || pit1 == cursor_.bottom().pit())
			buftext.redoParagraph(*this, pit1);
		y1 -= buftext.getPar(pit1).descent();
	}


	// Take care of ascent of first line
	y1 -= buftext.getPar(pit1).ascent();

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
	while (y2 < height_ && pit2 < int(npit) - 1) {
		y2 += buftext.getPar(pit2).descent();
		++pit2;
		if (!singlepar || pit2 == cursor_.bottom().pit())
			buftext.redoParagraph(*this, pit2);
		y2 += buftext.getPar(pit2).ascent();
	}

	// Take care of descent of last line
	y2 += buftext.getPar(pit2).descent();

	// The coordinates of all these paragraphs are correct, cache them
	int y = y1;
	CoordCache::InnerParPosCache & parPos = coord_cache_.parPos()[&buftext];
	for (pit_type pit = pit1; pit <= pit2; ++pit) {
		Paragraph const & par = buftext.getPar(pit);
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

	if (lyxerr.debugging(Debug::WORKAREA)) {
		lyxerr[Debug::WORKAREA] << "BufferView::updateMetrics" << endl;
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
			if (isDirWriteable(trypath))
				initpath = trypath;
		}

		// FIXME UNICODE
		FileDialog fileDlg(_("Select LyX document to insert"),
			LFUN_FILE_INSERT,
			make_pair(_("Documents|#o#O"), from_utf8(lyxrc.document_path)),
			make_pair(_("Examples|#E#e"), from_utf8(addPath(package().system_support(), "examples"))));

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
	filename = fileSearch(string(), filename, "lyx");

	docstring const disp_fn = makeDisplayPath(filename);
	// emit message signal.
	message(bformat(_("Inserting document %1$s..."), disp_fn));

	docstring res;
	Buffer buf("", false);
	if (lyx::loadLyXFile(&buf, makeAbsPath(filename))) {
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
