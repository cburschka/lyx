/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2002 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include "lyxtext.h"
#include "lyxrow.h"
#include "paragraph.h"
#include "BufferView.h"
#include "funcrequest.h"
#include "lyxrc.h"
#include "debug.h"
#include "bufferparams.h"
#include "buffer.h"
#include "ParagraphParameters.h"
#include "gettext.h"
#include "factory.h"
#include "intl.h"
#include "language.h"
#include "support/lstrings.h"
#include "frontends/LyXView.h"
#include "frontends/screen.h"
#include "frontends/WorkArea.h"
#include "insets/insetspecialchar.h"
#include "insets/insettext.h"
#include "insets/insetbib.h"
#include "insets/insetquotes.h"
#include "insets/insetcommand.h"
#include "undo_funcs.h"

#include <ctime>
#include <clocale>

using std::endl;

extern string current_layout;
extern int bibitemMaxWidth(BufferView *, LyXFont const &);

// the selection possible is needed, that only motion events are
// used, where the bottom press event was on the drawing area too 
bool selection_possible = false;


namespace {

	void moveCursorUpdate(BufferView * bv, bool selecting)
	{
		LyXText * lt = bv->getLyXText();

		if (selecting || lt->selection.mark()) {
			lt->setSelection(bv);
			if (lt->bv_owner)
				bv->toggleToggle();
			else
				bv->updateInset(lt->inset_owner, false);
		}
		if (lt->bv_owner) {
			//if (fitcur)
			//	bv->update(lt, BufferView::SELECT|BufferView::FITCUR);
			//else
			bv->update(lt, BufferView::SELECT|BufferView::FITCUR);
			bv->showCursor();
		} else if (bv->text->status() != LyXText::UNCHANGED) {
			bv->theLockingInset()->hideInsetCursor(bv);
			bv->update(bv->text, BufferView::SELECT|BufferView::FITCUR);
			bv->showCursor();
		}

		if (!lt->selection.set())
			bv->workarea().haveSelection(false);

		bv->switchKeyMap();
	}


	void finishChange(BufferView * bv, bool selecting = false)
	{
		finishUndo();
		moveCursorUpdate(bv, selecting);
		bv->owner()->view_state_changed();
	}

}


bool LyXText::gotoNextInset(BufferView * bv,
	vector<Inset::Code> const & codes, string const & contents) const
{
	LyXCursor res = cursor;
	Inset * inset;
	do {
		if (res.pos() < res.par()->size() - 1) {
			res.pos(res.pos() + 1);
		} else  {
			res.par(res.par()->next());
			res.pos(0);
		}

	} while (res.par() &&
		 !(res.par()->isInset(res.pos())
		   && (inset = res.par()->getInset(res.pos())) != 0
		   && find(codes.begin(), codes.end(), inset->lyxCode())
		   != codes.end()
		   && (contents.empty() ||
		       static_cast<InsetCommand *>(
							res.par()->getInset(res.pos()))->getContents()
		       == contents)));

	if (res.par()) {
		setCursor(bv, res.par(), res.pos(), false);
		return true;
	}
	return false;
}


void LyXText::gotoInset(BufferView * bv, vector<Inset::Code> const & codes,
				  bool same_content)
{
	bv->hideCursor();
	bv->beforeChange(this);
	update(bv, false);

	string contents;
	if (same_content && cursor.par()->isInset(cursor.pos())) {
		Inset const * inset = cursor.par()->getInset(cursor.pos());
		if (find(codes.begin(), codes.end(), inset->lyxCode())
		    != codes.end())
			contents = static_cast<InsetCommand const *>(inset)->getContents();
	}

	if (!gotoNextInset(bv, codes, contents)) {
		if (cursor.pos() || cursor.par() != ownerParagraph()) {
			LyXCursor tmp = cursor;
			cursor.par(ownerParagraph());
			cursor.pos(0);
			if (!gotoNextInset(bv, codes, contents)) {
				cursor = tmp;
				bv->owner()->message(_("No more insets"));
			}
		} else {
			bv->owner()->message(_("No more insets"));
		}
	}
	update(bv, false);
	selection.cursor = cursor;
}


void LyXText::gotoInset(BufferView * bv, Inset::Code code, bool same_content)
{
	gotoInset(bv, vector<Inset::Code>(1, code), same_content);
}


void LyXText::cursorPrevious(BufferView * bv)
{
	if (!cursor.row()->previous()) {
		if (first_y > 0) {
			int new_y = bv->text->first_y - bv->workarea().workHeight();
			bv->screen().draw(bv->text, bv, new_y < 0 ? 0 : new_y);
			bv->updateScrollbar();
		}
		return;
	}

	int y = first_y;
	Row * cursorrow = cursor.row();

	setCursorFromCoordinates(bv, cursor.x_fix(), y);
	finishUndo();

	int new_y;
	if (cursorrow == bv->text->cursor.row()) {
		// we have a row which is higher than the workarea so we leave the
		// cursor on the start of the row and move only the draw up as soon
		// as we move the cursor or do something while inside the row (it may
		// span several workarea-heights) we'll move to the top again, but this
		// is better than just jump down and only display part of the row.
		new_y = bv->text->first_y - bv->workarea().workHeight();
	} else {
		if (inset_owner) {
			new_y = bv->text->cursor.iy()
				+ bv->theLockingInset()->insetInInsetY() + y
				+ cursor.row()->height()
				- bv->workarea().workHeight() + 1;
		} else {
			new_y = cursor.y()
				- cursor.row()->baseline()
				+ cursor.row()->height()
				- bv->workarea().workHeight() + 1;
		}
	}
	bv->screen().draw(bv->text, bv, new_y < 0 ? 0 : new_y);
	if (cursor.row()->previous()) {
		LyXCursor cur;
		setCursor(bv, cur, cursor.row()->previous()->par(),
						cursor.row()->previous()->pos(), false);
		if (cur.y() > first_y) {
			cursorUp(bv, true);
		}
	}
	bv->updateScrollbar();
}


void LyXText::cursorNext(BufferView * bv)
{
	if (!cursor.row()->next()) {
		int y = cursor.y() - cursor.row()->baseline() +
			cursor.row()->height();
		if (y > int(first_y + bv->workarea().workHeight())) {
			bv->screen().draw(bv->text, bv,
						  bv->text->first_y + bv->workarea().workHeight());
			bv->updateScrollbar();
		}
		return;
	}

	int y = first_y + bv->workarea().workHeight();
	if (inset_owner && !first_y) {
		y -= (bv->text->cursor.iy()
			  - bv->text->first_y
			  + bv->theLockingInset()->insetInInsetY());
	}

	getRowNearY(y);

	Row * cursorrow = cursor.row();
	setCursorFromCoordinates(bv, cursor.x_fix(), y);
	// + workarea().workHeight());
	finishUndo();

	int new_y;
	if (cursorrow == bv->text->cursor.row()) {
		// we have a row which is higher than the workarea so we leave the
		// cursor on the start of the row and move only the draw down as soon
		// as we move the cursor or do something while inside the row (it may
		// span several workarea-heights) we'll move to the top again, but this
		// is better than just jump down and only display part of the row.
		new_y = bv->text->first_y + bv->workarea().workHeight();
	} else {
		if (inset_owner) {
			new_y = bv->text->cursor.iy()
				+ bv->theLockingInset()->insetInInsetY()
				+ y - cursor.row()->baseline();
		} else {
			new_y =  cursor.y() - cursor.row()->baseline();
		}
	}
	bv->screen().draw(bv->text, bv, new_y);
	if (cursor.row()->next()) {
		LyXCursor cur;
		setCursor(bv, cur, cursor.row()->next()->par(),
						cursor.row()->next()->pos(), false);
		if (cur.y() < int(first_y + bv->workarea().workHeight())) {
			cursorDown(bv, true);
		}
	}
	bv->updateScrollbar();
}


void LyXText::update(BufferView * bv, bool changed)
{
	BufferView::UpdateCodes c = BufferView::SELECT | BufferView::FITCUR;
	if (changed)
		bv->update(this, c | BufferView::CHANGE);
	else
		bv->update(this, c);
}


void specialChar(LyXText * lt, BufferView * bv, InsetSpecialChar::Kind kind)
{
	bv->hideCursor();
	lt->update(bv);
	InsetSpecialChar * new_inset = new InsetSpecialChar(kind);
	if (!bv->insertInset(new_inset))
		delete new_inset;
	else
		bv->updateInset(new_inset, true);
}


Inset::RESULT LyXText::dispatch(FuncRequest const & cmd)
{
	lyxerr[Debug::ACTION] << "LyXFunc::dispatch: action[" << cmd.action
			      <<"] arg[" << cmd.argument << "]" << endl;

	BufferView * bv = cmd.view();

	switch (cmd.action) {

	case LFUN_APPENDIX: {
		Paragraph * par = cursor.par();
		bool start = !par->params().startOfAppendix();

		// ensure that we have only one start_of_appendix in this document
		Paragraph * tmp = ownerParagraph();
		for (; tmp; tmp = tmp->next())
			tmp->params().startOfAppendix(false);

		par->params().startOfAppendix(start);

		// we can set the refreshing parameters now
		status(cmd.view(), LyXText::NEED_MORE_REFRESH);
		refresh_y = 0;
		refresh_row = 0; // not needed for full update
		updateCounters(cmd.view());
		setCursor(cmd.view(), cursor.par(), cursor.pos());
		update(bv);
		break;
	}

	case LFUN_DELETE_WORD_FORWARD:
		bv->beforeChange(this);
		update(bv, false);
		deleteWordForward(bv);
		update(bv);
		finishChange(bv);
		break;

	case LFUN_DELETE_WORD_BACKWARD:
		bv->beforeChange(this);
		update(bv, false);
		deleteWordBackward(bv);
		update(bv, true);
		finishChange(bv);
		break;

	case LFUN_DELETE_LINE_FORWARD:
		bv->beforeChange(this);
		update(bv, false);
		deleteLineForward(bv);
		update(bv);
		finishChange(bv);
		break;

	case LFUN_SHIFT_TAB:
	case LFUN_TAB:
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv, false);
		cursorTab(bv);
		finishChange(bv);
		break;

	case LFUN_WORDRIGHT:
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorLeftOneWord(bv);
		else
			cursorRightOneWord(bv);
		finishChange(bv);
		break;

	case LFUN_WORDLEFT:
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorRightOneWord(bv);
		else
			cursorLeftOneWord(bv);
		finishChange(bv);
		break;

	case LFUN_BEGINNINGBUF:
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv, false);
		cursorTop(bv);
		finishChange(bv);
		break;

	case LFUN_ENDBUF:
		if (selection.mark())
			bv->beforeChange(this);
		update(bv, false);
		cursorBottom(bv);
		finishChange(bv);
		break;

	case LFUN_RIGHTSEL:
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorLeft(bv);
		else
			cursorRight(bv);
		finishChange(bv, true);
		break;

	case LFUN_LEFTSEL:
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorRight(bv);
		else
			cursorLeft(bv);
		finishChange(bv, true);
		break;

	case LFUN_UPSEL:
		update(bv, false);
		cursorUp(bv, true);
		finishChange(bv, true);
		break;

	case LFUN_DOWNSEL:
		update(bv, false);
		cursorDown(bv, true);
		finishChange(bv, true);
		break;

	case LFUN_UP_PARAGRAPHSEL:
		update(bv, false);
		cursorUpParagraph(bv);
		finishChange(bv, true);
		break;

	case LFUN_DOWN_PARAGRAPHSEL:
		update(bv, false);
		cursorDownParagraph(bv);
		finishChange(bv, true);
		break;

	case LFUN_PRIORSEL:
		update(bv, false);
		cursorPrevious(bv);
		finishChange(bv, true);
		break;

	case LFUN_NEXTSEL:
		update(bv, false);
		cursorNext(bv);
		finishChange(bv, true);
		break;

	case LFUN_HOMESEL:
		update(bv, false);
		cursorHome(bv);
		finishChange(bv, true);
		break;

	case LFUN_ENDSEL:
		update(bv, false);
		cursorEnd(bv);
		finishChange(bv, true);
		break;

	case LFUN_WORDRIGHTSEL:
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorLeftOneWord(bv);
		else
			cursorRightOneWord(bv);
		finishChange(bv, true);
		break;

	case LFUN_WORDLEFTSEL:
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorRightOneWord(bv);
		else
			cursorLeftOneWord(bv);
		finishChange(bv, true);
		break;

	case LFUN_RIGHT: {
		bool is_rtl = cursor.par()->isRightToLeftPar(bv->buffer()->params);
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv);
		if (is_rtl)
			cursorLeft(bv, false);
		if (cursor.pos() < cursor.par()->size()
		    && cursor.par()->isInset(cursor.pos())
		    && isHighlyEditableInset(cursor.par()->getInset(cursor.pos()))) {
			Inset * tmpinset = cursor.par()->getInset(cursor.pos());
			cmd.message(tmpinset->editMessage());
			tmpinset->edit(bv, !is_rtl);
			break;
		}
		if (!is_rtl)
			cursorRight(bv, false);
		finishChange(bv);
		break;
	}

	case LFUN_LEFT: {
		// This is soooo ugly. Isn`t it possible to make
		// it simpler? (Lgb)
		bool const is_rtl = cursor.par()->isRightToLeftPar(bv->buffer()->params);
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv);
		LyXCursor const cur = cursor;
		if (!is_rtl)
			cursorLeft(bv, false);
		if ((is_rtl || cur != cursor) && // only if really moved!
		    cursor.pos() < cursor.par()->size() &&
		    cursor.par()->isInset(cursor.pos()) &&
		    isHighlyEditableInset(cursor.par()->getInset(cursor.pos()))) {
			Inset * tmpinset = cursor.par()->getInset(cursor.pos());
			cmd.message(tmpinset->editMessage());
			tmpinset->edit(bv, is_rtl);
			break;
		}
		if (is_rtl)
			cursorRight(bv, false);
		finishChange(bv);
		break;
	}

	case LFUN_UP:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		cursorUp(bv);
		finishChange(bv);
		break;

	case LFUN_DOWN:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		cursorDown(bv);
		finishChange(bv);
		break;

	case LFUN_UP_PARAGRAPH:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		cursorUpParagraph(bv);
		finishChange(bv);
		break;

	case LFUN_DOWN_PARAGRAPH:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		cursorDownParagraph(bv);
		finishChange(bv, false);
		break;

	case LFUN_PRIOR:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		cursorPrevious(bv);
		finishChange(bv, false);
		// was:
		// finishUndo();
		// moveCursorUpdate(bv, false, false);
		// owner_->view_state_changed();
		break;

	case LFUN_NEXT:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		cursorNext(bv);
		finishChange(bv, false);
		break;

	case LFUN_HOME:
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv);
		cursorHome(bv);
		finishChange(bv, false);
		break;

	case LFUN_END:
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv);
		cursorEnd(bv);
		finishChange(bv, false);
		break;

	case LFUN_BREAKLINE:
		bv->beforeChange(this);
		insertChar(bv, Paragraph::META_NEWLINE);
		update(bv, true);
		setCursor(bv, cursor.par(), cursor.pos());
		moveCursorUpdate(bv, false);
		break;

	case LFUN_DELETE:
		if (!selection.set()) {
			Delete(bv);
			selection.cursor = cursor;
			update(bv);
			// It is possible to make it a lot faster still
			// just comment out the line below...
			bv->showCursor();
		} else {
			update(bv, false);
			cutSelection(bv, true);
			update(bv);
		}
		moveCursorUpdate(bv, false);
		bv->owner()->view_state_changed();
		bv->switchKeyMap();
		break;

	case LFUN_DELETE_SKIP:
		// Reverse the effect of LFUN_BREAKPARAGRAPH_SKIP.
		if (!selection.set()) {
			LyXCursor cur = cursor;
			if (cur.pos() == cur.par()->size()) {
				cursorRight(bv);
				cur = cursor;
				if (cur.pos() == 0
				    && !(cur.par()->params().spaceTop()
					 == VSpace (VSpace::NONE))) {
					setParagraph(bv,
						 cur.par()->params().lineTop(),
						 cur.par()->params().lineBottom(),
						 cur.par()->params().pagebreakTop(),
						 cur.par()->params().pagebreakBottom(),
						 VSpace(VSpace::NONE),
						 cur.par()->params().spaceBottom(),
						 cur.par()->params().spacing(),
						 cur.par()->params().align(),
						 cur.par()->params().labelWidthString(), 0);
					cursorLeft(bv);
					update(bv);
				} else {
					cursorLeft(bv);
					Delete(bv);
					selection.cursor = cursor;
				}
			} else {
				Delete(bv);
				selection.cursor = cursor;
			}
		} else {
			update(bv, false);
			cutSelection(bv, true);
		}
		update(bv);
		break;


	case LFUN_BACKSPACE:
		if (!selection.set()) {
			if (bv->owner()->getIntl().getTransManager().backspace()) {
				backspace(bv);
				selection.cursor = cursor;
				update(bv);
				// It is possible to make it a lot faster still
				// just comment out the line below...
				bv->showCursor();
			}
		} else {
			update(bv, false);
			cutSelection(bv, true);
			update(bv);
		}
		bv->owner()->view_state_changed();
		bv->switchKeyMap();
		break;

	case LFUN_BACKSPACE_SKIP:
		// Reverse the effect of LFUN_BREAKPARAGRAPH_SKIP.
		if (!selection.set()) {
			LyXCursor cur = cursor;
			if (cur.pos() == 0
			    && !(cur.par()->params().spaceTop()
				 == VSpace (VSpace::NONE))) {
				setParagraph(bv,
					 cur.par()->params().lineTop(),
					 cur.par()->params().lineBottom(),
					 cur.par()->params().pagebreakTop(),
					 cur.par()->params().pagebreakBottom(),
					 VSpace(VSpace::NONE), cur.par()->params().spaceBottom(),
					 cur.par()->params().spacing(),
					 cur.par()->params().align(),
					 cur.par()->params().labelWidthString(), 0);
			} else {
				backspace(bv);
				selection.cursor = cur;
			}
		} else {
			update(bv, false);
			cutSelection(bv, true);
		}
		update(bv);
		break;

	case LFUN_BREAKPARAGRAPH:
		bv->beforeChange(this);
		breakParagraph(bv, 0);
		update(bv);
		selection.cursor = cursor;
		bv->switchKeyMap();
		bv->owner()->view_state_changed();
		break;

	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
		bv->beforeChange(this);
		breakParagraph(bv, 1);
		update(bv);
		selection.cursor = cursor;
		bv->switchKeyMap();
		bv->owner()->view_state_changed();
		break;

	case LFUN_BREAKPARAGRAPH_SKIP: {
		// When at the beginning of a paragraph, remove
		// indentation and add a "defskip" at the top.
		// Otherwise, do the same as LFUN_BREAKPARAGRAPH.
		LyXCursor cur = cursor;
		bv->beforeChange(this);
		if (cur.pos() == 0) {
			if (cur.par()->params().spaceTop() == VSpace(VSpace::NONE)) {
				setParagraph(bv,
					 cur.par()->params().lineTop(),
					 cur.par()->params().lineBottom(),
					 cur.par()->params().pagebreakTop(),
					 cur.par()->params().pagebreakBottom(),
					 VSpace(VSpace::DEFSKIP), cur.par()->params().spaceBottom(),
					 cur.par()->params().spacing(),
					 cur.par()->params().align(),
					 cur.par()->params().labelWidthString(), 1);
				//update(bv);
			}
		}
		else {
			breakParagraph(bv, 0);
			//update(bv);
		}
		update(bv);
		selection.cursor = cur;
		bv->switchKeyMap();
		bv->owner()->view_state_changed();
		break;
	}

	case LFUN_PARAGRAPH_SPACING: {
		Paragraph * par = cursor.par();
		Spacing::Space cur_spacing = par->params().spacing().getSpace();
		float cur_value = 1.0;
		if (cur_spacing == Spacing::Other)
			cur_value = par->params().spacing().getValue();

		istringstream is(cmd.argument.c_str());
		string tmp;
		is >> tmp;
		Spacing::Space new_spacing = cur_spacing;
		float new_value = cur_value;
		if (tmp.empty()) {
			lyxerr << "Missing argument to `paragraph-spacing'"
			       << endl;
		} else if (tmp == "single") {
			new_spacing = Spacing::Single;
		} else if (tmp == "onehalf") {
			new_spacing = Spacing::Onehalf;
		} else if (tmp == "double") {
			new_spacing = Spacing::Double;
		} else if (tmp == "other") {
			new_spacing = Spacing::Other;
			float tmpval = 0.0;
			is >> tmpval;
			lyxerr << "new_value = " << tmpval << endl;
			if (tmpval != 0.0)
				new_value = tmpval;
		} else if (tmp == "default") {
			new_spacing = Spacing::Default;
		} else {
			lyxerr << _("Unknown spacing argument: ")
			       << cmd.argument << endl;
		}
		if (cur_spacing != new_spacing || cur_value != new_value) {
			par->params().spacing(Spacing(new_spacing, new_value));
			redoParagraph(bv);
			update(bv);
		}
		break;
	}

	case LFUN_INSET_TOGGLE:
		bv->hideCursor();
		bv->beforeChange(this);
		update(bv, false);
		toggleInset(bv);
		update(bv, false);
		bv->switchKeyMap();
		break;

	case LFUN_PROTECTEDSPACE:
		if (cursor.par()->layout()->free_spacing) {
			insertChar(bv, ' ');
			update(bv);
		} else {
			specialChar(this, bv, InsetSpecialChar::PROTECTED_SEPARATOR);
		}
		moveCursorUpdate(bv, false);
		break;

	case LFUN_HYPHENATION:
		specialChar(this, bv, InsetSpecialChar::HYPHENATION);
		break;

	case LFUN_LIGATURE_BREAK:
		specialChar(this, bv, InsetSpecialChar::LIGATURE_BREAK);
		break;

	case LFUN_LDOTS:
		specialChar(this, bv, InsetSpecialChar::LDOTS);
		break;

	case LFUN_HFILL:
		bv->hideCursor();
		update(bv, false);
		insertChar(bv, Paragraph::META_HFILL);
		update(bv);
		break;

	case LFUN_END_OF_SENTENCE:
		specialChar(this, bv, InsetSpecialChar::END_OF_SENTENCE);
		break;

	case LFUN_MENU_SEPARATOR:
		specialChar(this, bv, InsetSpecialChar::MENU_SEPARATOR);
		break;

	case LFUN_MARK_OFF:
		bv->beforeChange(this);
		update(bv, false);
		selection.cursor = cursor;
		cmd.message(N_("Mark off"));
		break;

	case LFUN_MARK_ON:
		bv->beforeChange(this);
		selection.mark(true);
		update(bv, false);
		selection.cursor = cursor;
		cmd.message(N_("Mark on"));
		break;

	case LFUN_SETMARK:
		bv->beforeChange(this);
		if (selection.mark()) {
			update(bv);
			cmd.message(N_("Mark removed"));
		} else {
			selection.mark(true);
			update(bv);
			cmd.message(N_("Mark set"));
		}
		selection.cursor = cursor;
		break;

	case LFUN_UPCASE_WORD:
		update(bv, false);
		changeCase(bv, LyXText::text_uppercase);
		if (inset_owner)
			bv->updateInset(inset_owner, true);
		update(bv);
		break;

	case LFUN_LOWCASE_WORD:
		update(bv, false);
		changeCase(bv, LyXText::text_lowercase);
		if (inset_owner)
			bv->updateInset(inset_owner, true);
		update(bv);
		break;

	case LFUN_CAPITALIZE_WORD:
		update(bv, false);
		changeCase(bv, LyXText::text_capitalization);
		if (inset_owner)
			bv->updateInset(inset_owner, true);
		update(bv);
		break;

	case LFUN_TRANSPOSE_CHARS:
		update(bv, false);
		transposeChars(*bv);
		if (inset_owner)
			bv->updateInset(inset_owner, true);
		update(bv);
		break;

	case LFUN_PASTE:
		cmd.message(_("Paste"));
		bv->hideCursor();
		// clear the selection
		bv->toggleSelection();
		clearSelection();
		update(bv, false);
		pasteSelection(bv);
		clearSelection(); // bug 393
		update(bv, false);
		update(bv);
		bv->switchKeyMap();
		break;

	case LFUN_CUT:
		bv->hideCursor();
		update(bv, false);
		cutSelection(bv, true);
		update(bv);
		cmd.message(_("Cut"));
		break;

	case LFUN_COPY:
		copySelection(bv);
		cmd.message(_("Copy"));
		break;

	case LFUN_BEGINNINGBUFSEL:
		if (inset_owner)
			return Inset::UNDISPATCHED;
		update(bv, false);
		cursorTop(bv);
		finishChange(bv, true);
		break;

	case LFUN_ENDBUFSEL:
		if (inset_owner)
			return Inset::UNDISPATCHED;
		update(bv, false);
		cursorBottom(bv);
		finishChange(bv, true);
		break;

	case LFUN_GETXY:
		cmd.message(tostr(cursor.x()) + ' ' + tostr(cursor.y()));
		break;

	case LFUN_SETXY: {
		int x = 0;
		int y = 0;
		istringstream is(cmd.argument.c_str());
		is >> x >> y;
		if (!is)
			lyxerr << "SETXY: Could not parse coordinates in '"
			       << cmd.argument << std::endl;
		else 
			setCursorFromCoordinates(bv, x, y);
		break;
	}

	case LFUN_GETFONT:
		if (current_font.shape() == LyXFont::ITALIC_SHAPE)
			cmd.message("E");
		else if (current_font.shape() == LyXFont::SMALLCAPS_SHAPE)
			cmd.message("N");
		else
			cmd.message("0");
		break;

	case LFUN_GETLAYOUT:
		cmd.message(tostr(cursor.par()->layout()));
		break;

	case LFUN_LAYOUT: {
		lyxerr[Debug::INFO] << "LFUN_LAYOUT: (arg) "
		  << cmd.argument << endl;

		// This is not the good solution to the empty argument
		// problem, but it will hopefully suffice for 1.2.0.
		// The correct solution would be to augument the
		// function list/array with information about what
		// functions needs arguments and their type.
		if (cmd.argument.empty()) {
			cmd.errorMessage(_("LyX function 'layout' needs an argument."));
			break;
		}

		// Derive layout number from given argument (string)
		// and current buffer's textclass (number)
		LyXTextClass const & tclass = bv->buffer()->params.getLyXTextClass();
		bool hasLayout = tclass.hasLayout(cmd.argument);
		string layout = cmd.argument;

		// If the entry is obsolete, use the new one instead.
		if (hasLayout) {
			string const & obs = tclass[layout]->obsoleted_by();
			if (!obs.empty())
				layout = obs;
		}

		if (!hasLayout) {
			cmd.errorMessage(string(N_("Layout ")) + cmd.argument +
				N_(" not known"));
			break;
		}

		bool change_layout = (current_layout != layout);
		if (!change_layout && selection.set() &&
			selection.start.par() != selection.end.par())
		{
			Paragraph * spar = selection.start.par();
			Paragraph * epar = selection.end.par()->next();
			while (spar != epar) {
				if (spar->layout()->name() != current_layout) {
					change_layout = true;
					break;
				}
			}
		}
		if (change_layout) {
			bv->hideCursor();
			current_layout = layout;
			update(bv, false);
			setLayout(bv, layout);
			bv->owner()->setLayout(layout);
			update(bv);
			bv->switchKeyMap();
		}
		break;
	}

	case LFUN_PASTESELECTION: {
		if (!bv->buffer())
			break;
		bv->hideCursor();
		// this was originally a beforeChange(bv->text), i.e
		// the outermost LyXText!
		bv->beforeChange(this);
		string const clip = bv->workarea().getClipboard();
		if (!clip.empty()) {
			if (cmd.argument == "paragraph")
				insertStringAsParagraphs(bv, clip);
			else
				insertStringAsLines(bv, clip);
			clearSelection();
			update(bv);
		}
		break;
	}

	case LFUN_GOTOERROR:
		gotoInset(bv, Inset::ERROR_CODE, false);
		break;

	case LFUN_GOTONOTE:
		gotoInset(bv, Inset::NOTE_CODE, false);
		break;

	case LFUN_REFERENCE_GOTO:
	{
		vector<Inset::Code> tmp;
		tmp.push_back(Inset::LABEL_CODE);
		tmp.push_back(Inset::REF_CODE);
		gotoInset(bv, tmp, true);
		break;
	}

#if 0
	case LFUN_INSET_LIST:
	case LFUN_INSET_THEOREM:
#endif
	case LFUN_INSERT_NOTE:
	case LFUN_INSET_ERT:
	case LFUN_INSET_EXTERNAL:
	case LFUN_INSET_FLOAT:
	case LFUN_INSET_FOOTNOTE:
	case LFUN_INSET_MARGINAL:
	case LFUN_INSET_MINIPAGE:
	case LFUN_INSET_OPTARG:
	case LFUN_INSET_WIDE_FLOAT:
	case LFUN_TABULAR_INSERT:
	{
		Inset * inset = createInset(cmd);
		if (inset) {
			bool gotsel = false;
			if (selection.set()) {
				cutSelection(bv, true, false);
				gotsel = true;
			}
			if (bv->insertInset(inset)) {
				inset->edit(bv);
				if (gotsel)
					bv->owner()->dispatch(FuncRequest(LFUN_PASTESELECTION));
			}
			else
				delete inset;
		}
		break;
	}


	case LFUN_QUOTE: {
		Paragraph const * par = cursor.par();
		lyx::pos_type pos = cursor.pos();
		char c;
		if (!pos)
			c = ' ';
		else if (par->isInset(pos - 1) && par->getInset(pos - 1)->isSpace())
			c = ' ';
		else
			c = par->getChar(pos - 1);

		bv->hideCursor();
		LyXLayout_ptr const & style = par->layout();

		if (style->pass_thru ||
				par->getFontSettings(bv->buffer()->params,
					 pos).language()->lang() == "hebrew" ||
			(!bv->insertInset(new InsetQuotes(c, bv->buffer()->params))))
			bv->owner()->dispatch(FuncRequest(LFUN_SELFINSERT, "\""));
		break;
	}

	case LFUN_DATE_INSERT: {
		time_t now_time_t = time(NULL);
		struct tm * now_tm = localtime(&now_time_t);
		setlocale(LC_TIME, "");
		string arg;
		if (!cmd.argument.empty())
			arg = cmd.argument;
		else
			arg = lyxrc.date_insert_format;
		char datetmp[32];
		int const datetmp_len =
			::strftime(datetmp, 32, arg.c_str(), now_tm);

		for (int i = 0; i < datetmp_len; i++) {
			insertChar(bv, datetmp[i]);
			update(bv, true);
		}
		selection.cursor = cursor;
		moveCursorUpdate(bv, false);
		break;
	}

	case LFUN_MOUSE_TRIPLE:
		if (!bv->buffer())
			break;
		if (bv_owner && bv->theLockingInset())
			break;
		if (cmd.button() == mouse_button::button1) {
			if (bv_owner) {
				bv->screen().hideCursor();
				bv->screen().toggleSelection(this, bv);
			}
			cursorHome(bv);
			selection.cursor = cursor;
			cursorEnd(bv);
			setSelection(bv);
			if (bv_owner)
				bv->screen().toggleSelection(this, bv, false);
			update(bv, false);
			bv->workarea().haveSelection(selection.set());
		}
		break;

	case LFUN_MOUSE_DOUBLE:
		if (!bv->buffer())
			break;
		if (bv_owner && bv->theLockingInset())
			break;
		if (cmd.button() == mouse_button::button1) {
			if (bv_owner) {
				bv->screen().hideCursor();
				bv->screen().toggleSelection(this, bv);
				selectWord(bv, LyXText::WHOLE_WORD_STRICT);
				bv->screen().toggleSelection(this, bv, false);
			} else {
				selectWord(bv, LyXText::WHOLE_WORD_STRICT);
			}
			update(bv, false);
			bv->workarea().haveSelection(selection.set());
		}
		break;

	case LFUN_MOUSE_MOTION:
	{
		// Only use motion with button 1
		//if (ev.button() != mouse_button::button1)
		//	return false;

		if (!bv->buffer())
			break;

		// Check for inset locking
		if (bv->theLockingInset()) {
			Inset * tli = bv->theLockingInset();
			LyXCursor cursor = bv->text->cursor;
			LyXFont font = bv->text->getFont(bv->buffer(),
								cursor.par(), cursor.pos());
			int width = tli->width(bv, font);
			int inset_x = font.isVisibleRightToLeft()
				? cursor.ix() - width : cursor.ix();
			int start_x = inset_x + tli->scroll();
			FuncRequest cmd1 = cmd;
			cmd1.x -= start_x;
			cmd1.y -= cursor.iy() + bv->text->first_y;
			tli->localDispatch(cmd1);
			break;
		}

		// The test for not selection possible is needed, that only motion
		// events are used, where the bottom press event was on
		//  the drawing area too
		if (!selection_possible) {
			lyxerr[Debug::ACTION]
				<< "BufferView::Pimpl::Dispatch: no selection possible\n";
			break;
		}

		bv->screen().hideCursor();

		Row * cursorrow = bv->text->cursor.row();
		bv->text->setCursorFromCoordinates(bv, cmd.x, cmd.y + bv->text->first_y);
	#if 0
		// sorry for this but I have a strange error that the y value jumps at
		// a certain point. This seems like an error in my xforms library or
		// in some other local environment, but I would like to leave this here
		// for the moment until I can remove this (Jug 20020418)
		if (y_before < bv->text->cursor.y())
			lyxerr << y_before << ":" << bv->text->cursor.y() << endl;
	#endif
		// This is to allow jumping over large insets
		if (cursorrow == bv->text->cursor.row()) {
			if (cmd.y >= int(bv->workarea().workHeight())) {
				bv->text->cursorDown(bv, false);
			} else if (cmd.y < 0) {
				bv->text->cursorUp(bv, false);
			}
		}

		if (!bv->text->selection.set())
			bv->update(bv->text, BufferView::UPDATE); // Maybe an empty line was deleted

		bv->text->setSelection(bv);
		bv->screen().toggleToggle(bv->text, bv);
		bv->fitCursor();
		bv->showCursor();
		break;
	}

	// Single-click on work area
	case LFUN_MOUSE_PRESS:
	{
		if (!bv->buffer())
			break;

		// ok ok, this is a hack (for xforms)
		// We shouldn't go further down as we really should only do the
		// scrolling and be done with this. Otherwise we may open some
		// dialogs (Jug 20020424).
		if (cmd.button() == mouse_button::button4) {
			bv->scroll(-lyxrc.wheel_jump);
			break;
		}
		if (cmd.button() == mouse_button::button5) {
			bv->scroll(lyxrc.wheel_jump);
			break;
		}

		int x = cmd.x;
		int y = cmd.y;
		Inset * inset_hit = bv->checkInsetHit(bv->text, x, y);

		// Middle button press pastes if we have a selection
		// We do this here as if the selection was inside an inset
		// it could get cleared on the unlocking of the inset so
		// we have to check this first
		bool paste_internally = false;
		if (cmd.button() == mouse_button::button2 && selection.set()) {
			bv->owner()->dispatch(FuncRequest(LFUN_COPY));
			paste_internally = true;
		}

		int const screen_first = bv->text->first_y;

		if (bv->theLockingInset()) {
			// We are in inset locking mode

			// Check whether the inset was hit. If not reset mode,
			// otherwise give the event to the inset
			if (inset_hit == bv->theLockingInset()) {
				FuncRequest cmd(bv, LFUN_MOUSE_PRESS, x, y, cmd.button());
				bv->theLockingInset()->localDispatch(cmd);
				break;
			}
			bv->unlockInset(bv->theLockingInset());
		}

		if (!inset_hit)
			selection_possible = true;
		bv->screen().hideCursor();

		// Clear the selection
		bv->screen().toggleSelection(bv->text, bv);
		bv->text->clearSelection();
		bv->text->fullRebreak(bv);
		bv->update();
		bv->updateScrollbar();

		// Single left click in math inset?
		if (isHighlyEditableInset(inset_hit)) {
			// Highly editable inset, like math
			UpdatableInset * inset = static_cast<UpdatableInset *>(inset_hit);
			selection_possible = false;
			bv->owner()->updateLayoutChoice();
			bv->owner()->message(inset->editMessage());
			//inset->edit(bv, x, y, cmd.button());
			// We just have to lock the inset before calling a PressEvent on it!
			// we don't need the edit() call here! (Jug20020329)
			if (!bv->lockInset(inset))
				lyxerr[Debug::INSETS] << "Cannot lock inset" << endl;
			FuncRequest cmd(bv, LFUN_MOUSE_PRESS, x, y, cmd.button());
			inset->localDispatch(cmd);
			break;
		}
		// I'm not sure we should continue here if we hit an inset (Jug20020403)

		// Right click on a footnote flag opens float menu
		if (cmd.button() == mouse_button::button3) {
			selection_possible = false;
			break;
		}

		if (!inset_hit) // otherwise it was already set in checkInsetHit(...)
			bv->text->setCursorFromCoordinates(bv, x, y + screen_first);
		finishUndo();
		bv->text->selection.cursor = bv->text->cursor;
		bv->text->cursor.x_fix(bv->text->cursor.x());

		bv->owner()->updateLayoutChoice();
		if (bv->fitCursor())
			selection_possible = false;

		// Insert primary selection with middle mouse
		// if there is a local selection in the current buffer,
		// insert this
		if (cmd.button() == mouse_button::button2) {
			if (paste_internally)
				bv->owner()->dispatch(FuncRequest(LFUN_PASTE));
			else
				bv->owner()->dispatch(FuncRequest(LFUN_PASTESELECTION, "paragraph"));
			selection_possible = false;
		}
		break;
	}

	case LFUN_MOUSE_RELEASE:
	{
		// do nothing if we used the mouse wheel
		if (!bv->buffer())
			break;
	
		if (cmd.button() == mouse_button::button4
		 || cmd.button() == mouse_button::button5)
			break;

		// If we hit an inset, we have the inset coordinates in these
		// and inset_hit points to the inset.  If we do not hit an
		// inset, inset_hit is 0, and inset_x == x, inset_y == y.
		int x = cmd.x;
		int y = cmd.y;
		Inset * inset_hit = bv->checkInsetHit(bv->text, x, y);

		if (bv->theLockingInset()) {
			// We are in inset locking mode.

			// LyX does a kind of work-area grabbing for insets.
			// Only a ButtonPress FuncRequest outside the inset will
			// force a insetUnlock.
			FuncRequest cmd(bv, LFUN_MOUSE_RELEASE, x, y, cmd.button());
			bv->theLockingInset()->localDispatch(cmd);
			break;
		}

		selection_possible = false;

		if (cmd.button() == mouse_button::button2)
			break;

		// finish selection
		if (cmd.button() == mouse_button::button1)
			bv->workarea().haveSelection(selection.set());

		bv->switchKeyMap();
		bv->owner()->view_state_changed();
		bv->owner()->updateMenubar();
		bv->owner()->updateToolbar();

		// Did we hit an editable inset?
		if (inset_hit) {
			selection_possible = false;

			// if we reach this point with a selection, it
			// must mean we are currently selecting.
			// But we don't want to open the inset
			// because that is annoying for the user.
			// So just pretend we didn't hit it.
			// this is OK because a "kosher" ButtonRelease
			// will follow a ButtonPress that clears
			// the selection.
			// Note this also fixes selection drawing
			// problems if we end up opening an inset
			if (selection.set())
				break;

			// CHECK fix this proper in 0.13
			// well, maybe 13.0 !!!!!!!!!

			// Following a ref shouldn't issue
			// a push on the undo-stack
			// anylonger, now that we have
			// keybindings for following
			// references and returning from
			// references.  IMHO though, it
			// should be the inset's own business
			// to push or not push on the undo
			// stack. They don't *have* to
			// alter the document...
			// (Joacim)
			// ...or maybe the SetCursorParUndo()
			// below isn't necessary at all anylonger?
			if (inset_hit->lyxCode() == Inset::REF_CODE)
				setCursorParUndo(bv);

			bv->owner()->message(inset_hit->editMessage());

			if (isHighlyEditableInset(inset_hit)) {
				// Highly editable inset, like math
				UpdatableInset * inset = (UpdatableInset *) inset_hit;
				FuncRequest cmd(bv, LFUN_MOUSE_RELEASE, x, y, cmd.button());
				inset->localDispatch(cmd);
			} else {
				FuncRequest cmd(bv, LFUN_MOUSE_RELEASE, x, y, cmd.button());
				inset_hit->localDispatch(cmd);
				// IMO this is a grosshack! Inset's should be changed so that
				// they call the actions they have to do with the insetButtonRel.
				// function and not in the edit(). This should be changed
				// (Jug 20020329)
#ifdef WITH_WARNINGS
#warning Please remove donot call inset->edit() here (Jug 20020812)
#endif
				inset_hit->edit(bv, x, y, cmd.button());
			}
			break;
		}

		// Maybe we want to edit a bibitem ale970302
		if (bv->text->cursor.par()->bibkey) {
			bool const is_rtl =
				bv->text->cursor.par()->isRightToLeftPar(bv->buffer()->params);
			int const width =
				bibitemMaxWidth(bv, bv->buffer()->params.getLyXTextClass().defaultfont());
			if ((is_rtl && x > bv->text->workWidth(bv)-20-width) ||
					(!is_rtl && x < 20 + width)) {
				bv->text->cursor.par()->bibkey->edit(bv, 0, 0, mouse_button::none);
			}
		}
		break;
	}

	case LFUN_SELFINSERT: {
		if (cmd.argument.empty())
			break;

		// Automatically delete the currently selected
		// text and replace it with what is being
		// typed in now. Depends on lyxrc settings
		// "auto_region_delete", which defaults to
		// true (on).

		if (lyxrc.auto_region_delete) {
			if (selection.set()) {
				cutSelection(bv, false, false);
				update(bv);
			}
			bv->workarea().haveSelection(false);
		}

		bv->beforeChange(this);
		LyXFont const old_font(real_current_font);

		string::const_iterator cit = cmd.argument.begin();
		string::const_iterator end = cmd.argument.end();
		for (; cit != end; ++cit)
			bv->owner()->getIntl().getTransManager().
				TranslateAndInsert(*cit, this);

		update(bv);
		selection.cursor = cursor;
		moveCursorUpdate(bv, false);

		// real_current_font.number can change so we need to
		// update the minibuffer
		if (old_font != real_current_font)
			bv->owner()->view_state_changed();
		break;
	}

	default:
		return Inset::UNDISPATCHED;
	}

	return Inset::DISPATCHED;
}
