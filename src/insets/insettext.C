/**
 * \file insettext.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insettext.h"
#include "insetnewline.h"

#include "buffer.h"
#include "BufferView.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "errorlist.h"
#include "funcrequest.h"
#include "gettext.h"
#include "intl.h"
#include "lyxfind.h"
#include "lyxlex.h"
#include "lyxrc.h"
#include "metricsinfo.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "rowpainter.h"
#include "sgml.h"
#include "undo_funcs.h"
#include "WordLangTuple.h"

#include "frontends/Alert.h"
#include "frontends/font_metrics.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"

#include "support/lyxalgo.h" // lyx::count

#include <boost/bind.hpp>

using std::endl;
using std::for_each;
using std::min;
using std::max;
using std::make_pair;
using std::auto_ptr;
using std::ostream;
using std::ifstream;
using std::pair;
using std::vector;

using namespace lyx::support;
using namespace lyx::graphics;
using namespace bv_funcs;

using lyx::pos_type;
using lyx::textclass_type;


InsetText::InsetText(BufferParams const & bp)
	: UpdatableInset(), text_(0, this, true, paragraphs)
{
	paragraphs.push_back(Paragraph());
	paragraphs.begin()->layout(bp.getLyXTextClass().defaultLayout());
	if (bp.tracking_changes)
		paragraphs.begin()->trackChanges();
	init(0);
}


InsetText::InsetText(InsetText const & in)
	: UpdatableInset(in), text_(0, this, true, paragraphs)
{
	init(&in);
}


InsetText & InsetText::operator=(InsetText const & it)
{
	init(&it);
	return *this;
}


void InsetText::init(InsetText const * ins)
{
	if (ins) {
		textwidth_ = ins->textwidth_;
		text_.bv_owner = ins->text_.bv_owner;

		paragraphs = ins->paragraphs;

		ParagraphList::iterator pit = paragraphs.begin();
		ParagraphList::iterator end = paragraphs.end();
		for (; pit != end; ++pit)
			pit->setInsetOwner(this);

		autoBreakRows = ins->autoBreakRows;
		drawFrame_ = ins->drawFrame_;
		frame_color = ins->frame_color;
	} else {
		textwidth_ = 0; // broken
		drawFrame_ = NEVER;
		frame_color = LColor::insetframe;
		autoBreakRows = false;
	}
	the_locking_inset = 0;
	for_each(paragraphs.begin(), paragraphs.end(),
		 boost::bind(&Paragraph::setInsetOwner, _1, this));
	top_y = 0;
	no_selection = true;
	drawTextXOffset = 0;
	drawTextYOffset = 0;
	locked = false;
	old_par = paragraphs.end();
	in_insetAllowed = false;
}


void InsetText::clear(bool just_mark_erased)
{
	if (just_mark_erased) {
		ParagraphList::iterator it = paragraphs.begin();
		ParagraphList::iterator end = paragraphs.end();
		for (; it != end; ++it)
			it->markErased();
		return;
	}

	// This is a gross hack...
	LyXLayout_ptr old_layout = paragraphs.begin()->layout();

	paragraphs.clear();
	paragraphs.push_back(Paragraph());
	paragraphs.begin()->setInsetOwner(this);
	paragraphs.begin()->layout(old_layout);
}


auto_ptr<InsetBase> InsetText::clone() const
{
	return auto_ptr<InsetBase>(new InsetText(*this));
}


void InsetText::write(Buffer const & buf, ostream & os) const
{
	os << "Text\n";
	writeParagraphData(buf, os);
}


void InsetText::writeParagraphData(Buffer const & buf, ostream & os) const
{
	ParagraphList::const_iterator it = paragraphs.begin();
	ParagraphList::const_iterator end = paragraphs.end();
	Paragraph::depth_type dth = 0;
	for (; it != end; ++it) {
		it->write(buf, os, buf.params, dth);
	}
}


void InsetText::read(Buffer const & buf, LyXLex & lex)
{
	string token;
	Paragraph::depth_type depth = 0;

	clear(false);

	if (buf.params.tracking_changes)
		paragraphs.begin()->trackChanges();

	// delete the initial paragraph
	paragraphs.clear();
	ParagraphList::iterator pit = paragraphs.begin();

	while (lex.isOK()) {
		lex.nextToken();
		token = lex.getString();
		if (token.empty())
			continue;
		if (token == "\\end_inset") {
			break;
		}

		if (token == "\\end_document") {
			lex.printError("\\end_document read in inset! Error in document!");
			return;
		}

		// FIXME: ugly.
		const_cast<Buffer&>(buf).readParagraph(lex, token, paragraphs, pit, depth);
	}

	pit = paragraphs.begin();
	ParagraphList::iterator const end = paragraphs.end();
	for (; pit != end; ++pit)
		pit->setInsetOwner(this);

	if (token != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point. "
					   "Read: `$$Token'");
	}
}


void InsetText::metrics(MetricsInfo & mi, Dimension & dim) const
{
	//lyxerr << "InsetText::metrics: width: " << mi.base.textwidth << endl;

	textwidth_ = mi.base.textwidth;
	BufferView * bv = mi.base.bv;
	setViewCache(bv);
	text_.metrics(mi, dim);
	dim.asc += TEXT_TO_INSET_OFFSET;
	dim.des += TEXT_TO_INSET_OFFSET;
	dim.wid += 2 * TEXT_TO_INSET_OFFSET;
	dim.wid = max(dim.wid, 10);
	dim_ = dim;
}


int InsetText::textWidth() const
{
	return textwidth_;
}


void InsetText::draw(PainterInfo & pi, int x, int y) const
{
	// update our idea of where we are. Clearly, we should
	// not have to know this information.
	top_x = x;

	int const start_x = x;

	BufferView * bv = pi.base.bv;
	Painter & pain = pi.pain;

	// repaint the background if needed
	if (backgroundColor() != LColor::background)
		clearInset(bv, start_x + TEXT_TO_INSET_OFFSET, y);

	// no draw is necessary !!!
	if (drawFrame_ == LOCKED && !locked && paragraphs.begin()->empty()) {
		top_baseline = y;
		return;
	}

	bv->hideCursor();

	if (!owner())
		x += scroll();

	top_baseline = y;
	top_y = y - dim_.asc;

	if (the_locking_inset && cpar() == inset_par && cpos() == inset_pos) {
		inset_x = cx() - x + drawTextXOffset;
		inset_y = cy() + drawTextYOffset;
	}

	x += TEXT_TO_INSET_OFFSET;

	paintTextInset(*bv, text_, x, y);

	if (drawFrame_ == ALWAYS || (drawFrame_ == LOCKED && locked))
		drawFrame(pain, start_x);
}


void InsetText::drawFrame(Painter & pain, int x) const
{
	int const ttoD2 = TEXT_TO_INSET_OFFSET / 2;
	int const frame_x = x + ttoD2;
	int const frame_y = top_baseline - dim_.asc + ttoD2;
	int const frame_w = dim_.wid - TEXT_TO_INSET_OFFSET;
	int const frame_h = dim_.asc + dim_.des - TEXT_TO_INSET_OFFSET;
	pain.rectangle(frame_x, frame_y, frame_w, frame_h, frame_color);
}


void InsetText::updateLocal(BufferView * bv, bool /*mark_dirty*/)
{
	if (!bv)
		return;

	if (!autoBreakRows && paragraphs.size() > 1)
		collapseParagraphs(bv);

	if (!text_.selection.set())
		text_.selection.cursor = text_.cursor;

	bv->fitCursor();
	bv->updateInset(this);
	bv->owner()->view_state_changed();
	bv->owner()->updateMenubar();
	bv->owner()->updateToolbar();
	if (old_par != cpar()) {
		bv->owner()->setLayout(cpar()->layout()->name());
		old_par = cpar();
	}
}


string const InsetText::editMessage() const
{
	return _("Opened Text Inset");
}


void InsetText::insetUnlock(BufferView * bv)
{
	if (the_locking_inset) {
		the_locking_inset->insetUnlock(bv);
		the_locking_inset = 0;
		updateLocal(bv, false);
	}
	no_selection = true;
	locked = false;

	if (text_.selection.set()) {
		text_.clearSelection();
	} else if (owner()) {
		bv->owner()->setLayout(owner()->getLyXText(bv)
				       ->cursor.par()->layout()->name());
	} else
		bv->owner()->setLayout(bv->text->cursor.par()->layout()->name());
	// hack for deleteEmptyParMech
	ParagraphList::iterator first_par = paragraphs.begin();
	if (!first_par->empty()) {
		text_.setCursor(first_par, 0);
	} else if (paragraphs.size() > 1) {
		text_.setCursor(boost::next(first_par), 0);
	}
}


void InsetText::lockInset(BufferView * bv)
{
	locked = true;
	the_locking_inset = 0;
	inset_pos = inset_x = inset_y = 0;
	inset_boundary = false;
	inset_par = paragraphs.end();
	old_par = paragraphs.end();
	text_.setCursorIntern(paragraphs.begin(), 0);
	text_.clearSelection();
	finishUndo();
	// If the inset is empty set the language of the current font to the
	// language to the surronding text (if different).
	if (paragraphs.begin()->empty() && paragraphs.size() == 1 &&
		bv->getParentLanguage(this) != text_.current_font.language()) {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(bv->getParentLanguage(this));
		setFont(bv, font, false);
	}
}


void InsetText::lockInset(BufferView * /*bv*/, UpdatableInset * inset)
{
	the_locking_inset = inset;
	inset_x = cx() - top_x + drawTextXOffset;
	inset_y = cy() + drawTextYOffset;
	inset_pos = cpos();
	inset_par = cpar();
	inset_boundary = cboundary();
}


bool InsetText::lockInsetInInset(BufferView * bv, UpdatableInset * inset)
{
	lyxerr[Debug::INSETS] << "InsetText::LockInsetInInset("
			      << inset << "): " << endl;
	if (!inset)
		return false;
	if (!the_locking_inset) {
		ParagraphList::iterator pit = paragraphs.begin();
		ParagraphList::iterator pend = paragraphs.end();

		int const id = inset->id();
		for (; pit != pend; ++pit) {
			InsetList::iterator it = pit->insetlist.begin();
			InsetList::iterator const end = pit->insetlist.end();
			for (; it != end; ++it) {
				if (it->inset == inset) {
					lyxerr << "InsetText::lockInsetInInset: 1 a" << endl;
					text_.setCursorIntern(pit, it->pos);
					lyxerr << "InsetText::lockInsetInInset: 1 b" << endl;
					lyxerr << "bv: " << bv << " inset: " << inset << endl;
					lockInset(bv, inset);
					lyxerr << "InsetText::lockInsetInInset: 1 c" << endl;
					return true;
				}
				if (it->inset->getInsetFromID(id)) {
					lyxerr << "InsetText::lockInsetInInset: 2" << endl;
					text_.setCursorIntern(pit, it->pos);
					it->inset->localDispatch(FuncRequest(bv, LFUN_INSET_EDIT));
					return the_locking_inset->lockInsetInInset(bv, inset);
				}
			}
		}
		lyxerr << "InsetText::lockInsetInInset: 3" << endl;
		return false;
	}
	if (inset == cpar()->getInset(cpos())) {
		lyxerr[Debug::INSETS] << "OK" << endl;
		lockInset(bv, inset);
		return true;
	}

	if (the_locking_inset && the_locking_inset == inset) {
		if (cpar() == inset_par && cpos() == inset_pos) {
			lyxerr[Debug::INSETS] << "OK" << endl;
			inset_x = cx() - top_x + drawTextXOffset;
			inset_y = cy() + drawTextYOffset;
		} else {
			lyxerr[Debug::INSETS] << "cursor.pos != inset_pos" << endl;
		}
	} else if (the_locking_inset) {
		lyxerr[Debug::INSETS] << "MAYBE" << endl;
		return the_locking_inset->lockInsetInInset(bv, inset);
	}
	lyxerr[Debug::INSETS] << "NOT OK" << endl;
	return false;
}


bool InsetText::unlockInsetInInset(BufferView * bv, UpdatableInset * inset,
				   bool lr)
{
	if (!the_locking_inset)
		return false;
	if (the_locking_inset == inset) {
		the_locking_inset->insetUnlock(bv);
		the_locking_inset = 0;
		if (lr)
			moveRightIntern(bv, true, false);
		old_par = paragraphs.end(); // force layout setting
		if (scroll())
			scroll(bv, 0.0F);
		else
			updateLocal(bv, false);
		return true;
	}
	return the_locking_inset->unlockInsetInInset(bv, inset, lr);
}


void InsetText::lfunMousePress(FuncRequest const & cmd)
{
	no_selection = true;

	// use this to check mouse motion for selection!
	mouse_x = cmd.x;
	mouse_y = cmd.y;

	BufferView * bv = cmd.view();
	FuncRequest cmd1 = cmd;
	cmd1.x -= inset_x;
	cmd1.y -= inset_y;
	if (!locked)
		lockInset(bv);

	int tmp_x = cmd.x - drawTextXOffset;
	int tmp_y = cmd.y + dim_.asc - bv->top_y();
	InsetOld * inset = getLyXText(bv)->checkInsetHit(tmp_x, tmp_y);

	if (the_locking_inset) {
		if (the_locking_inset == inset) {
			the_locking_inset->localDispatch(cmd1);
			return;
		}
		// otherwise only unlock the_locking_inset
		the_locking_inset->insetUnlock(bv);
		the_locking_inset = 0;
	}
	if (!inset)
		no_selection = false;

	if (bv->theLockingInset()) {
		if (isHighlyEditableInset(inset)) {
			// We just have to lock the inset before calling a
			// PressEvent on it!
			UpdatableInset * uinset = static_cast<UpdatableInset*>(inset);
			if (!bv->lockInset(uinset)) {
				lyxerr[Debug::INSETS] << "Cannot lock inset" << endl;
			}
			inset->localDispatch(cmd1);
			if (the_locking_inset)
				updateLocal(bv, false);
			return;
		}
	}
	if (!inset) {
		bool paste_internally = false;
		if (cmd.button() == mouse_button::button2 && getLyXText(bv)->selection.set()) {
			localDispatch(FuncRequest(bv, LFUN_COPY));
			paste_internally = true;
		}
		int old_top_y = bv->top_y();

		text_.setCursorFromCoordinates(cmd.x - drawTextXOffset,
					     cmd.y + dim_.asc);
		// set the selection cursor!
		text_.selection.cursor = text_.cursor;
		text_.cursor.x_fix(text_.cursor.x());

		text_.clearSelection();
		updateLocal(bv, false);

		bv->owner()->setLayout(cpar()->layout()->name());

		// we moved the view we cannot do mouse selection in this case!
		if (bv->top_y() != old_top_y)
			no_selection = true;
		old_par = cpar();
		// Insert primary selection with middle mouse
		// if there is a local selection in the current buffer,
		// insert this
		if (cmd.button() == mouse_button::button2) {
			if (paste_internally)
				localDispatch(FuncRequest(bv, LFUN_PASTE));
			else
				localDispatch(FuncRequest(bv, LFUN_PASTESELECTION, "paragraph"));
		}
	} else {
		getLyXText(bv)->clearSelection();
	}
}


bool InsetText::lfunMouseRelease(FuncRequest const & cmd)
{
	BufferView * bv = cmd.view();
	FuncRequest cmd1 = cmd;
	cmd1.x -= inset_x;
	cmd1.y -= inset_y;

	no_selection = true;
	if (the_locking_inset)
		return the_locking_inset->localDispatch(cmd1);

	int tmp_x = cmd.x - drawTextXOffset;
	int tmp_y = cmd.y + dim_.asc - bv->top_y();
	InsetOld * inset = getLyXText(bv)->checkInsetHit(tmp_x, tmp_y);
	bool ret = false;
	if (inset) {
// This code should probably be removed now. Simple insets
// (!highlyEditable) can actually take the localDispatch,
// and turn it into edit() if necessary. But we still
// need to deal properly with the whole relative vs.
// absolute mouse co-ords thing in a realiable, sensible way
#if 0
		if (isHighlyEditableInset(inset))
			ret = inset->localDispatch(cmd1);
		else {
			inset_x = cx(bv) - top_x + drawTextXOffset;
			inset_y = cy() + drawTextYOffset;
			cmd1.x = cmd.x - inset_x;
			cmd1.y = cmd.x - inset_y;
			inset->edit(bv, cmd1.x, cmd1.y, cmd.button());
			ret = true;
		}
#endif
		ret = inset->localDispatch(cmd1);
		updateLocal(bv, false);

	}
	return ret;
}


void InsetText::lfunMouseMotion(FuncRequest const & cmd)
{
	FuncRequest cmd1 = cmd;
	cmd1.x -= inset_x;
	cmd1.y -= inset_y;

	if (the_locking_inset) {
		the_locking_inset->localDispatch(cmd1);
		return;
	}

	if (no_selection || (mouse_x == cmd.x && mouse_y == cmd.y))
		return;

	BufferView * bv = cmd.view();
	LyXCursor cur = text_.cursor;
	text_.setCursorFromCoordinates
		(cmd.x - drawTextXOffset, cmd.y + dim_.asc);
	text_.cursor.x_fix(text_.cursor.x());
	if (cur == text_.cursor)
		return;
	text_.setSelection();
	updateLocal(bv, false);
}


InsetOld::RESULT InsetText::localDispatch(FuncRequest const & cmd)
{
	BufferView * bv = cmd.view();
	setViewCache(bv);

	switch (cmd.action) {
	case LFUN_INSET_EDIT: {
		UpdatableInset::localDispatch(cmd);

		if (!bv->lockInset(this)) {
			lyxerr[Debug::INSETS] << "Cannot lock inset" << endl;
			return DISPATCHED;
		}

		locked = true;
		the_locking_inset = 0;
		inset_pos = 0;
		inset_x = 0;
		inset_y = 0;
		inset_boundary = false;
		inset_par = paragraphs.end();
		old_par = paragraphs.end();


		if (cmd.argument.size()) {
			if (cmd.argument == "left")
				text_.setCursorIntern(paragraphs.begin(), 0);
			else {
				ParagraphList::iterator it = boost::prior(paragraphs.end());
				text_.setCursor(it, it->size());
			}
		} else {
			int tmp_y = (cmd.y < 0) ? 0 : cmd.y;
			// we put here -1 and not button as now the button in the
			// edit call should not be needed we will fix this in 1.3.x
			// cycle hopefully (Jug 20020509)
			// FIXME: GUII I've changed this to none: probably WRONG
			if (!checkAndActivateInset(bv, cmd.x, tmp_y, mouse_button::none)) {
				text_.setCursorFromCoordinates(cmd.x - drawTextXOffset,
									cmd.y + dim_.asc);
				text_.cursor.x(text_.cursor.x());
				text_.cursor.x_fix(text_.cursor.x());
			}
		}

		text_.clearSelection();
		finishUndo();

		// If the inset is empty set the language of the current font to the
		// language to the surronding text (if different).
		if (paragraphs.begin()->empty() &&
		    paragraphs.size() == 1 &&
		    bv->getParentLanguage(this) != text_.current_font.language())
		{
			LyXFont font(LyXFont::ALL_IGNORE);
			font.setLanguage(bv->getParentLanguage(this));
			setFont(bv, font, false);
		}

		updateLocal(bv, false);
		// Tell the paragraph dialog that we've entered an insettext.
		bv->dispatch(FuncRequest(LFUN_PARAGRAPH_UPDATE));
		return DISPATCHED;
	}

	case LFUN_MOUSE_PRESS:
		lfunMousePress(cmd);
		return DISPATCHED;

	case LFUN_MOUSE_MOTION:
		lfunMouseMotion(cmd);
		return DISPATCHED;

	case LFUN_MOUSE_RELEASE:
		return lfunMouseRelease(cmd) ? DISPATCHED : UNDISPATCHED;

	default:
		break;
	}

	bool was_empty = paragraphs.begin()->empty() && paragraphs.size() == 1;
	no_selection = false;

	RESULT result = UpdatableInset::localDispatch(cmd);
	if (result != UNDISPATCHED)
		return DISPATCHED;

	result = DISPATCHED;
	if (cmd.action < 0 && cmd.argument.empty())
		return FINISHED;

	if (the_locking_inset) {
		result = the_locking_inset->localDispatch(cmd);
		if (result == DISPATCHED_NOUPDATE)
			return result;
		if (result == DISPATCHED) {
			updateLocal(bv, false);
			return result;
		}
		if (result >= FINISHED) {
			switch (result) {
			case FINISHED_RIGHT:
				moveRightIntern(bv, false, false);
				result = DISPATCHED;
				break;
			case FINISHED_UP:
				result = moveUp(bv);
				if (result >= FINISHED) {
					updateLocal(bv, false);
					bv->unlockInset(this);
				}
				break;
			case FINISHED_DOWN:
				result = moveDown(bv);
				if (result >= FINISHED) {
					updateLocal(bv, false);
					bv->unlockInset(this);
				}
				break;
			default:
				result = DISPATCHED;
				break;
			}
			the_locking_inset = 0;
			updateLocal(bv, false);
			// make sure status gets reset immediately
			bv->owner()->clearMessage();
			return result;
		}
	}
	bool updflag = false;

	switch (cmd.action) {

	// Normal chars
	case LFUN_SELFINSERT:
		if (bv->buffer()->isReadonly()) {
//	    setErrorMessage(N_("Document is read only"));
			break;
		}
		if (!cmd.argument.empty()) {
			/* Automatically delete the currently selected
			 * text and replace it with what is being
			 * typed in now. Depends on lyxrc settings
			 * "auto_region_delete", which defaults to
			 * true (on). */
#if 0
			// This should not be needed here and is also WRONG!
			recordUndo(bv, Undo::INSERT, text_.cursor.par());
#endif
			bv->switchKeyMap();

			if (lyxrc.auto_region_delete && text_.selection.set())
				text_.cutSelection(false, false);
			text_.clearSelection();

			for (string::size_type i = 0; i < cmd.argument.length(); ++i)
				bv->owner()->getIntl().getTransManager().
					TranslateAndInsert(cmd.argument[i], &text_);
		}
		text_.selection.cursor = text_.cursor;
		updflag = true;
		result = DISPATCHED_NOUPDATE;
		break;

	// cursor movements that need special handling

	case LFUN_RIGHT:
		result = moveRight(bv);
		finishUndo();
		break;
	case LFUN_LEFT:
		finishUndo();
		result = moveLeft(bv);
		break;
	case LFUN_DOWN:
		finishUndo();
		result = moveDown(bv);
		break;
	case LFUN_UP:
		finishUndo();
		result = moveUp(bv);
		break;

	case LFUN_PRIOR:
		if (crow() == text_.firstRow())
			result = FINISHED_UP;
		else {
			text_.cursorPrevious();
			text_.clearSelection();
			result = DISPATCHED_NOUPDATE;
		}
		break;

	case LFUN_NEXT:
		if (crow() == text_.lastRow())
			result = FINISHED_DOWN;
		else {
			text_.cursorNext();
			text_.clearSelection();
			result = DISPATCHED_NOUPDATE;
		}
		break;

	case LFUN_BACKSPACE:
		if (text_.selection.set())
			text_.cutSelection(true, false);
		else
			text_.backspace();
		updflag = true;
		break;

	case LFUN_DELETE:
		if (text_.selection.set())
			text_.cutSelection(true, false);
		else
			text_.Delete();
		updflag = true;
		break;

	case LFUN_PASTE: {
		if (!autoBreakRows) {
			if (CutAndPaste::nrOfParagraphs() > 1) {
#ifdef WITH_WARNINGS
#warning FIXME horrendously bad UI
#endif
				Alert::error(_("Paste failed"), _("Cannot include more than one paragraph."));
				break;
			}
		}

		replaceSelection(bv->getLyXText());
		size_t sel_index = 0;
		string const & arg = cmd.argument;
		if (isStrUnsignedInt(arg)) {
			size_t const paste_arg = strToUnsignedInt(arg);
#warning FIXME Check if the arg is in the domain of available selections.
			sel_index = paste_arg;
		}
		text_.pasteSelection(sel_index);
		// bug 393
		text_.clearSelection();
		updflag = true;
		break;
	}

	case LFUN_BREAKPARAGRAPH:
		if (!autoBreakRows) {
			result = DISPATCHED;
			break;
		}
		replaceSelection(bv->getLyXText());
		text_.breakParagraph(paragraphs, 0);
		updflag = true;
		break;

	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
		if (!autoBreakRows) {
			result = DISPATCHED;
			break;
		}
		replaceSelection(bv->getLyXText());
		text_.breakParagraph(paragraphs, 1);
		updflag = true;
		break;

	case LFUN_BREAKLINE: {
		if (!autoBreakRows) {
			result = DISPATCHED;
			break;
		}

		replaceSelection(bv->getLyXText());
		text_.insertInset(new InsetNewline);
		updflag = true;
		break;
	}

	case LFUN_LAYOUT:
		// do not set layouts on non breakable textinsets
		if (autoBreakRows) {
			string cur_layout = cpar()->layout()->name();

			// Derive layout number from given argument (string)
			// and current buffer's textclass (number).
			LyXTextClass const & tclass =
				bv->buffer()->params.getLyXTextClass();
			string layout = cmd.argument;
			bool hasLayout = tclass.hasLayout(layout);

			// If the entry is obsolete, use the new one instead.
			if (hasLayout) {
				string const & obs = tclass[layout]->obsoleted_by();
				if (!obs.empty())
					layout = obs;
			}

			// see if we found the layout number:
			if (!hasLayout) {
				FuncRequest lf(LFUN_MESSAGE, N_("Layout ") + cmd.argument + N_(" not known"));
				bv->owner()->dispatch(lf);
				break;
			}

			if (cur_layout != layout) {
				cur_layout = layout;
				text_.setLayout(layout);
				bv->owner()->setLayout(cpar()->layout()->name());
				updflag = true;
			}
		} else {
			// reset the layout box
			bv->owner()->setLayout(cpar()->layout()->name());
		}
		break;

	default:
		if (!bv->dispatch(cmd))
			result = UNDISPATCHED;
		break;
	}

	updateLocal(bv, updflag);

	/// If the action has deleted all text in the inset, we need to change the
	// language to the language of the surronding text.
	if (!was_empty && paragraphs.begin()->empty() &&
	    paragraphs.size() == 1) {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(bv->getParentLanguage(this));
		setFont(bv, font, false);
	}

	if (result >= FINISHED)
		bv->unlockInset(this);

	if (result == DISPATCHED_NOUPDATE)
		result = DISPATCHED;
	return result;
}


int InsetText::latex(Buffer const & buf, ostream & os,
		     LatexRunParams const & runparams) const
{
	TexRow texrow;
	latexParagraphs(buf, paragraphs, os, texrow, runparams);
	return texrow.rows();
}


int InsetText::ascii(Buffer const & buf, ostream & os, int linelen) const
{
	unsigned int lines = 0;

	ParagraphList::const_iterator beg = paragraphs.begin();
	ParagraphList::const_iterator end = paragraphs.end();
	ParagraphList::const_iterator it = beg;
	for (; it != end; ++it) {
		string const tmp = buf.asciiParagraph(*it, linelen, it == beg);
		lines += lyx::count(tmp.begin(), tmp.end(), '\n');
		os << tmp;
	}
	return lines;
}


int InsetText::linuxdoc(Buffer const & buf, ostream & os) const
{
	ParagraphList::iterator pit = const_cast<ParagraphList&>(paragraphs).begin();
	ParagraphList::iterator pend = const_cast<ParagraphList&>(paragraphs).end();

	// There is a confusion between the empty paragraph and the default paragraph
	// The default paragraph is <p></p>, the empty paragraph is *empty*
	// Since none of the floats of linuxdoc accepts standard paragraphs
	// I disable them. I don't expect problems. (jamatos 2003/07/27)
	for (; pit != pend; ++pit) {
		const string name = pit->layout()->latexname();
		if (name != "p")
			sgml::openTag(os, 1, 0, name);
		buf.simpleLinuxDocOnePar(os, pit, 0);
		if (name != "p")
			sgml::closeTag(os, 1, 0, name);
	}
	return 0;
}


int InsetText::docbook(Buffer const & buf, ostream & os, bool mixcont) const
{
	unsigned int lines = 0;

	vector<string> environment_stack(10);
	vector<string> environment_inner(10);

	int const command_depth = 0;
	string item_name;

	Paragraph::depth_type depth = 0; // paragraph depth

	ParagraphList::iterator pit = const_cast<ParagraphList&>(paragraphs).begin();
	ParagraphList::iterator pend = const_cast<ParagraphList&>(paragraphs).end();

	for (; pit != pend; ++pit) {
		int desc_on = 0; // description mode

		LyXLayout_ptr const & style = pit->layout();

		// environment tag closing
		for (; depth > pit->params().depth(); --depth) {
			if (environment_inner[depth] != "!-- --") {
				item_name = "listitem";
				lines += sgml::closeTag(os, command_depth + depth, mixcont, item_name);
				if (environment_inner[depth] == "varlistentry")
					lines += sgml::closeTag(os, depth+command_depth, mixcont, environment_inner[depth]);
			}
			lines += sgml::closeTag(os, depth + command_depth, mixcont, environment_stack[depth]);
			environment_stack[depth].erase();
			environment_inner[depth].erase();
		}

		if (depth == pit->params().depth()
		   && environment_stack[depth] != style->latexname()
		   && !environment_stack[depth].empty()) {
			if (environment_inner[depth] != "!-- --") {
				item_name= "listitem";
				lines += sgml::closeTag(os, command_depth+depth, mixcont, item_name);
				if (environment_inner[depth] == "varlistentry")
					lines += sgml::closeTag(os, depth + command_depth, mixcont, environment_inner[depth]);
			}

			lines += sgml::closeTag(os, depth + command_depth, mixcont, environment_stack[depth]);

			environment_stack[depth].erase();
			environment_inner[depth].erase();
		}

		// Write opening SGML tags.
		switch (style->latextype) {
		case LATEX_PARAGRAPH:
			lines += sgml::openTag(os, depth + command_depth, mixcont, style->latexname());
			break;

		case LATEX_COMMAND:
			buf.error(ErrorItem(_("Error"), _("LatexType Command not allowed here.\n"), pit->id(), 0, pit->size()));
			return -1;
			break;

		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
			if (depth < pit->params().depth()) {
				depth = pit->params().depth();
				environment_stack[depth].erase();
			}

			if (environment_stack[depth] != style->latexname()) {
				if (environment_stack.size() == depth + 1) {
					environment_stack.push_back("!-- --");
					environment_inner.push_back("!-- --");
				}
				environment_stack[depth] = style->latexname();
				environment_inner[depth] = "!-- --";
				lines += sgml::openTag(os, depth + command_depth, mixcont, environment_stack[depth]);
			} else {
				if (environment_inner[depth] != "!-- --") {
					item_name= "listitem";
					lines += sgml::closeTag(os, command_depth + depth, mixcont, item_name);
					if (environment_inner[depth] == "varlistentry")
						lines += sgml::closeTag(os, depth + command_depth, mixcont, environment_inner[depth]);
				}
			}

			if (style->latextype == LATEX_ENVIRONMENT) {
				if (!style->latexparam().empty()) {
					if (style->latexparam() == "CDATA")
						os << "<![CDATA[";
					else
					  lines += sgml::openTag(os, depth + command_depth, mixcont, style->latexparam());
				}
				break;
			}

			desc_on = (style->labeltype == LABEL_MANUAL);

			environment_inner[depth] = desc_on ? "varlistentry" : "listitem";
			lines += sgml::openTag(os, depth + 1 + command_depth, mixcont, environment_inner[depth]);

			item_name = desc_on ? "term" : "para";
			lines += sgml::openTag(os, depth + 1 + command_depth, mixcont, item_name);

			break;
		default:
			lines += sgml::openTag(os, depth + command_depth, mixcont, style->latexname());
			break;
		}

		buf.simpleDocBookOnePar(os, pit, desc_on, depth + 1 + command_depth);

		string end_tag;
		// write closing SGML tags
		switch (style->latextype) {
		case LATEX_ENVIRONMENT:
			if (!style->latexparam().empty()) {
				if (style->latexparam() == "CDATA")
					os << "]]>";
				else
					lines += sgml::closeTag(os, depth + command_depth, mixcont, style->latexparam());
			}
			break;
		case LATEX_ITEM_ENVIRONMENT:
			if (desc_on == 1)
				break;
			end_tag= "para";
			lines += sgml::closeTag(os, depth + 1 + command_depth, mixcont, end_tag);
			break;
		case LATEX_PARAGRAPH:
			lines += sgml::closeTag(os, depth + command_depth, mixcont, style->latexname());
			break;
		default:
			lines += sgml::closeTag(os, depth + command_depth, mixcont, style->latexname());
			break;
		}
	}

	// Close open tags
	for (int d = depth; d >= 0; --d) {
		if (!environment_stack[depth].empty()) {
			if (environment_inner[depth] != "!-- --") {
				item_name = "listitem";
				lines += sgml::closeTag(os, command_depth + depth, mixcont, item_name);
			       if (environment_inner[depth] == "varlistentry")
				       lines += sgml::closeTag(os, depth + command_depth, mixcont, environment_inner[depth]);
			}

			lines += sgml::closeTag(os, depth + command_depth, mixcont, environment_stack[depth]);
		}
	}

	return lines;
}


void InsetText::validate(LaTeXFeatures & features) const
{
	for_each(paragraphs.begin(), paragraphs.end(),
		 boost::bind(&Paragraph::validate, _1, boost::ref(features)));
}


void InsetText::getCursor(BufferView & bv, int & x, int & y) const
{
	if (the_locking_inset) {
		the_locking_inset->getCursor(bv, x, y);
		return;
	}
	x = cx();
	y = cy() + InsetText::y();
}


void InsetText::getCursorPos(BufferView * bv, int & x, int & y) const
{
	if (the_locking_inset) {
		the_locking_inset->getCursorPos(bv, x, y);
		return;
	}
	x = cx() - top_x - TEXT_TO_INSET_OFFSET;
	y = cy() - TEXT_TO_INSET_OFFSET;
}


int InsetText::insetInInsetY() const
{
	if (!the_locking_inset)
		return 0;

	return inset_y + the_locking_inset->insetInInsetY();
}


void InsetText::fitInsetCursor(BufferView * bv) const
{
	if (the_locking_inset) {
		the_locking_inset->fitInsetCursor(bv);
		return;
	}

	LyXFont const font = text_.getFont(cpar(), cpos());

	int const asc = font_metrics::maxAscent(font);
	int const desc = font_metrics::maxDescent(font);

	bv->fitLockedInsetCursor(cx(), cy(), asc, desc);
}


InsetOld::RESULT InsetText::moveRight(BufferView * bv)
{
	if (text_.cursor.par()->isRightToLeftPar(bv->buffer()->params))
		return moveLeftIntern(bv, false, true, false);
	else
		return moveRightIntern(bv, true, true, false);
}


InsetOld::RESULT InsetText::moveLeft(BufferView * bv)
{
	if (text_.cursor.par()->isRightToLeftPar(bv->buffer()->params))
		return moveRightIntern(bv, true, true, false);
	else
		return moveLeftIntern(bv, false, true, false);
}


InsetOld::RESULT
InsetText::moveRightIntern(BufferView * bv, bool front,
			   bool activate_inset, bool selecting)
{
	ParagraphList::iterator c_par = cpar();

	if (boost::next(c_par) == paragraphs.end() && cpos() >= c_par->size())
		return FINISHED_RIGHT;
	if (activate_inset && checkAndActivateInset(bv, front))
		return DISPATCHED;
	text_.cursorRight(bv);
	if (!selecting)
		text_.clearSelection();
	return DISPATCHED_NOUPDATE;
}


InsetOld::RESULT
InsetText::moveLeftIntern(BufferView * bv, bool front,
			  bool activate_inset, bool selecting)
{
	if (cpar() == paragraphs.begin() && cpos() <= 0)
		return FINISHED;
	text_.cursorLeft(bv);
	if (!selecting)
		text_.clearSelection();
	if (activate_inset && checkAndActivateInset(bv, front))
		return DISPATCHED;
	return DISPATCHED_NOUPDATE;
}


InsetOld::RESULT InsetText::moveUp(BufferView * bv)
{
	if (crow() == text_.firstRow())
		return FINISHED_UP;
	text_.cursorUp(bv);
	text_.clearSelection();
	return DISPATCHED_NOUPDATE;
}


InsetOld::RESULT InsetText::moveDown(BufferView * bv)
{
	if (crow() == text_.lastRow())
		return FINISHED_DOWN;
	text_.cursorDown(bv);
	text_.clearSelection();
	return DISPATCHED_NOUPDATE;
}


bool InsetText::insertInset(BufferView * bv, InsetOld * inset)
{
	if (the_locking_inset) {
		if (the_locking_inset->insetAllowed(inset))
			return the_locking_inset->insertInset(bv, inset);
		return false;
	}
	inset->setOwner(this);
	text_.insertInset(inset);
	bv->fitCursor();
	updateLocal(bv, true);
	return true;
}


bool InsetText::insetAllowed(InsetOld::Code code) const
{
	// in_insetAllowed is a really gross hack,
	// to allow us to call the owner's insetAllowed
	// without stack overflow, which can happen
	// when the owner uses InsetCollapsable::insetAllowed()
	bool ret = true;
	if (in_insetAllowed)
		return ret;
	in_insetAllowed = true;
	if (the_locking_inset)
		ret = the_locking_inset->insetAllowed(code);
	else if (owner())
		ret = owner()->insetAllowed(code);
	in_insetAllowed = false;
	return ret;
}


UpdatableInset * InsetText::getLockingInset() const
{
	return the_locking_inset ? the_locking_inset->getLockingInset() :
		const_cast<InsetText *>(this);
}


UpdatableInset * InsetText::getFirstLockingInsetOfType(InsetOld::Code c)
{
	if (c == lyxCode())
		return this;
	if (the_locking_inset)
		return the_locking_inset->getFirstLockingInsetOfType(c);
	return 0;
}


bool InsetText::showInsetDialog(BufferView * bv) const
{
	if (the_locking_inset)
		return the_locking_inset->showInsetDialog(bv);
	return false;
}


void InsetText::getLabelList(std::vector<string> & list) const
{
	ParagraphList::const_iterator pit = paragraphs.begin();
	ParagraphList::const_iterator pend = paragraphs.end();
	for (; pit != pend; ++pit) {
		InsetList::const_iterator beg = pit->insetlist.begin();
		InsetList::const_iterator end = pit->insetlist.end();
		for (; beg != end; ++beg)
			beg->inset->getLabelList(list);
	}
}


void InsetText::setFont(BufferView * bv, LyXFont const & font, bool toggleall,
			bool selectall)
{
	if (the_locking_inset) {
		the_locking_inset->setFont(bv, font, toggleall, selectall);
		return;
	}

	if ((paragraphs.size() == 1 && paragraphs.begin()->empty())
	    || cpar()->empty()) {
		text_.setFont(font, toggleall);
		return;
	}


	if (text_.selection.set())
		recordUndo(bv, Undo::ATOMIC, text_.cursor.par());

	if (selectall) {
		text_.cursorTop();
		text_.selection.cursor = text_.cursor;
		text_.cursorBottom();
		text_.setSelection();
	}

	text_.toggleFree(font, toggleall);

	if (selectall)
		text_.clearSelection();

	bv->fitCursor();
	updateLocal(bv, true);
}


bool InsetText::checkAndActivateInset(BufferView * bv, bool front)
{
	if (cpos() == cpar()->size())
		return false;
	InsetOld * inset = cpar()->getInset(cpos());
	if (!isHighlyEditableInset(inset))
		return false;
	FuncRequest cmd(bv, LFUN_INSET_EDIT, front ? "left" : "right");
	inset->localDispatch(cmd);
	if (!the_locking_inset)
		return false;
	updateLocal(bv, false);
	return true;
}


bool InsetText::checkAndActivateInset(BufferView * bv, int x, int y,
				      mouse_button::state button)
{
	x -= drawTextXOffset;
	int dummyx = x;
	int dummyy = y + dim_.asc;
	InsetOld * inset = getLyXText(bv)->checkInsetHit(dummyx, dummyy);
	// we only do the edit() call if the inset was hit by the mouse
	// or if it is a highly editable inset. So we should call this
	// function from our own edit with button < 0.
	// FIXME: GUII jbl. I've changed this to ::none for now which is probably
	// WRONG
	if (button == mouse_button::none && !isHighlyEditableInset(inset))
		return false;

	if (!inset)
		return false;
	if (x < 0)
		x = dim_.wid;
	if (y < 0)
		y = dim_.des;
	inset_x = cx() - top_x + drawTextXOffset;
	inset_y = cy() + drawTextYOffset;
	FuncRequest cmd(bv, LFUN_INSET_EDIT, x - inset_x, y - inset_y, button);
	inset->localDispatch(cmd);
	if (!the_locking_inset)
		return false;
	updateLocal(bv, false);
	return true;
}


void InsetText::markNew(bool track_changes)
{
	ParagraphList::iterator pit = paragraphs.begin();
	ParagraphList::iterator end = paragraphs.end();
	for (; pit != end; ++pit) {
		if (track_changes) {
			pit->trackChanges();
		} else {
			// no-op when not tracking
			pit->cleanChanges();
		}
	}
}


void InsetText::setText(string const & data, LyXFont const & font)
{
	clear(false);
	for (unsigned int i = 0; i < data.length(); ++i)
		paragraphs.begin()->insertChar(i, data[i], font);
}


void InsetText::setAutoBreakRows(bool flag)
{
	if (flag != autoBreakRows) {
		autoBreakRows = flag;
		if (!flag)
			removeNewlines();
	}
}


void InsetText::setDrawFrame(DrawFrame how)
{
	drawFrame_ = how;
}


void InsetText::setFrameColor(LColor::color col)
{
	frame_color = col;
}


int InsetText::cx() const
{
	int x = text_.cursor.x() + top_x + TEXT_TO_INSET_OFFSET;
	if (the_locking_inset) {
		LyXFont font = text_.getFont(text_.cursor.par(), text_.cursor.pos());
		if (font.isVisibleRightToLeft())
			x -= the_locking_inset->width();
	}
	return x;
}


int InsetText::cy() const
{
	return text_.cursor.y() - dim_.asc + TEXT_TO_INSET_OFFSET;
}


pos_type InsetText::cpos() const
{
	return text_.cursor.pos();
}


ParagraphList::iterator InsetText::cpar() const
{
	return text_.cursor.par();
}


bool InsetText::cboundary() const
{
	return text_.cursor.boundary();
}


RowList::iterator InsetText::crow() const
{
	return text_.cursorRow();
}


LyXText * InsetText::getLyXText(BufferView const * bv,
				bool const recursive) const
{
	setViewCache(bv);
	if (recursive && the_locking_inset)
		return the_locking_inset->getLyXText(bv, true);
	return &text_;
}


void InsetText::setViewCache(BufferView const * bv) const
{
	if (bv) {
		if (bv != text_.bv_owner) {
			//lyxerr << "setting view cache from "
			//	<< text_.bv_owner << " to " << bv << "\n";
			text_.init(const_cast<BufferView *>(bv));
		}
		text_.bv_owner = const_cast<BufferView *>(bv);
	}
}


void InsetText::deleteLyXText(BufferView * bv, bool recursive) const
{
	if (recursive) {
		/// then remove all LyXText in text-insets
		for_each(const_cast<ParagraphList&>(paragraphs).begin(),
			 const_cast<ParagraphList&>(paragraphs).end(),
			 boost::bind(&Paragraph::deleteInsetsLyXText, _1, bv));
	}
}


void InsetText::removeNewlines()
{
	ParagraphList::iterator it = paragraphs.begin();
	ParagraphList::iterator end = paragraphs.end();
	for (; it != end; ++it)
		for (int i = 0; i < it->size(); ++i)
			if (it->isNewline(i))
				it->erase(i);
}


int InsetText::scroll(bool recursive) const
{
	int sx = UpdatableInset::scroll(false);

	if (recursive && the_locking_inset)
		sx += the_locking_inset->scroll(recursive);

	return sx;
}


void InsetText::clearSelection(BufferView *)
{
	text_.clearSelection();
}


void InsetText::clearInset(BufferView * bv, int start_x, int baseline) const
{
	Painter & pain = bv->painter();
	int w = dim_.wid;
	int h = dim_.asc + dim_.des;
	int ty = baseline - dim_.asc;

	if (ty < 0) {
		h += ty;
		ty = 0;
	}
	if (ty + h > pain.paperHeight())
		h = pain.paperHeight();
	if (top_x + drawTextXOffset + w > pain.paperWidth())
		w = pain.paperWidth();
	pain.fillRectangle(start_x + 1, ty + 1, w - 3, h - 1, backgroundColor());
}


ParagraphList * InsetText::getParagraphs(int i) const
{
	return (i == 0) ? const_cast<ParagraphList*>(&paragraphs) : 0;
}


LyXCursor const & InsetText::cursor(BufferView * bv) const
{
	if (the_locking_inset)
		return the_locking_inset->cursor(bv);
	return getLyXText(bv)->cursor;
}


InsetOld * InsetText::getInsetFromID(int id_arg) const
{
	if (id_arg == id())
		return const_cast<InsetText *>(this);

	ParagraphList::const_iterator pit = paragraphs.begin();
	ParagraphList::const_iterator pend = paragraphs.end();
	for (; pit != pend; ++pit) {
		InsetList::const_iterator it = pit->insetlist.begin();
		InsetList::const_iterator end = pit->insetlist.end();
		for (; it != end; ++it) {
			if (it->inset->id() == id_arg)
				return it->inset;
			InsetOld * in = it->inset->getInsetFromID(id_arg);
			if (in)
				return in;
		}
	}
	return 0;
}


WordLangTuple const
InsetText::selectNextWordToSpellcheck(BufferView * bv, float & value) const
{
	WordLangTuple word;
	if (the_locking_inset) {
		word = the_locking_inset->selectNextWordToSpellcheck(bv, value);
		if (!word.word().empty()) {
			value += cy();
			return word;
		}
		// we have to go on checking so move cursor to the next char
		text_.cursor.pos(text_.cursor.pos() + 1);
	}
	word = text_.selectNextWordToSpellcheck(value);
	if (word.word().empty())
		bv->unlockInset(const_cast<InsetText *>(this));
	else
		value = cy();
	return word;
}


void InsetText::selectSelectedWord(BufferView * bv)
{
	if (the_locking_inset) {
		the_locking_inset->selectSelectedWord(bv);
		return;
	}
	getLyXText(bv)->selectSelectedWord();
	updateLocal(bv, false);
}


bool InsetText::nextChange(BufferView * bv, lyx::pos_type & length)
{
	if (the_locking_inset) {
		if (the_locking_inset->nextChange(bv, length))
			return true;
		text_.cursorRight(true);
	}
	lyx::find::SearchResult result =
		lyx::find::findNextChange(bv, &text_, length);

	if (result == lyx::find::SR_FOUND) {
		LyXCursor cur = text_.cursor;
		bv->unlockInset(bv->theLockingInset());
		if (bv->lockInset(this))
			locked = true;
		text_.cursor = cur;
		text_.setSelectionRange(length);
		updateLocal(bv, false);
	}
	return result != lyx::find::SR_NOT_FOUND;
}


bool InsetText::searchForward(BufferView * bv, string const & str,
			      bool cs, bool mw)
{
	if (the_locking_inset) {
		if (the_locking_inset->searchForward(bv, str, cs, mw))
			return true;
		text_.cursorRight(true);
	}
	lyx::find::SearchResult result =
		lyx::find::find(bv, &text_, str, true, cs, mw);

	if (result == lyx::find::SR_FOUND) {
		LyXCursor cur = text_.cursor;
		bv->unlockInset(bv->theLockingInset());
		if (bv->lockInset(this))
			locked = true;
		text_.cursor = cur;
		text_.setSelectionRange(str.length());
		updateLocal(bv, false);
	}
	return result != lyx::find::SR_NOT_FOUND;
}


bool InsetText::searchBackward(BufferView * bv, string const & str,
			       bool cs, bool mw)
{
	if (the_locking_inset) {
		if (the_locking_inset->searchBackward(bv, str, cs, mw))
			return true;
	}
	if (!locked) {
		ParagraphList::iterator pit = boost::prior(paragraphs.end());
		text_.setCursor(pit, pit->size());
	}
	lyx::find::SearchResult result =
		lyx::find::find(bv, &text_, str, false, cs, mw);

	if (result == lyx::find::SR_FOUND) {
		LyXCursor cur = text_.cursor;
		bv->unlockInset(bv->theLockingInset());
		if (bv->lockInset(this))
			locked = true;
		text_.cursor = cur;
		text_.setSelectionRange(str.length());
		updateLocal(bv, false);
	}
	return result != lyx::find::SR_NOT_FOUND;
}


bool InsetText::checkInsertChar(LyXFont & font)
{
	return owner() ? owner()->checkInsertChar(font) : true;
}


void InsetText::collapseParagraphs(BufferView * bv)
{
	while (paragraphs.size() > 1) {
		ParagraphList::iterator first_par = paragraphs.begin();
		ParagraphList::iterator next_par = boost::next(first_par);
		size_t const first_par_size = first_par->size();

		if (!first_par->empty() &&
		    !next_par->empty() &&
		    !first_par->isSeparator(first_par_size - 1)) {
			first_par->insertChar(first_par_size, ' ');
		}

		if (text_.selection.set()) {
			if (text_.selection.start.par() == next_par) {
				text_.selection.start.par(first_par);
				text_.selection.start.pos(
					text_.selection.start.pos() + first_par_size);
			}
			if (text_.selection.end.par() == next_par) {
				text_.selection.end.par(first_par);
				text_.selection.end.pos(
					text_.selection.end.pos() + first_par_size);
			}
		}

		mergeParagraph(bv->buffer()->params, paragraphs, first_par);
	}
}


void InsetText::getDrawFont(LyXFont & font) const
{
	if (!owner())
		return;
	owner()->getDrawFont(font);
}


void InsetText::appendParagraphs(Buffer * buffer, ParagraphList & plist)
{
#warning FIXME Check if Changes stuff needs changing here. (Lgb)
// And it probably does. You have to take a look at this John. (Lgb)
#warning John, have a look here. (Lgb)
	ParagraphList::iterator pit = plist.begin();
	ParagraphList::iterator ins = paragraphs.insert(paragraphs.end(), *pit);
	++pit;
	mergeParagraph(buffer->params, paragraphs, boost::prior(ins));

	ParagraphList::iterator pend = plist.end();
	for (; pit != pend; ++pit)
		paragraphs.push_back(*pit);
}


void InsetText::addPreview(PreviewLoader & loader) const
{
	ParagraphList::const_iterator pit = paragraphs.begin();
	ParagraphList::const_iterator pend = paragraphs.end();

	for (; pit != pend; ++pit) {
		InsetList::const_iterator it  = pit->insetlist.begin();
		InsetList::const_iterator end = pit->insetlist.end();
		for (; it != end; ++it)
			it->inset->addPreview(loader);
	}
}
