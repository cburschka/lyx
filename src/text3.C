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
#include "intl.h"
#include "support/lstrings.h"
#include "frontends/LyXView.h"
#include "frontends/screen.h"
#include "frontends/WorkArea.h"
#include "insets/insetspecialchar.h"
#include "insets/insettext.h"
#include "undo_funcs.h"

using std::endl;

extern string current_layout;


namespace {

void cursorPrevious(LyXText * text, BufferView * bv)
{
	if (!text->cursor.row()->previous()) {
		if (text->first_y > 0) {
			int new_y = bv->text->first_y - bv->workarea().workHeight();
			bv->screen().draw(bv->text, bv, new_y < 0 ? 0 : new_y);
			bv->updateScrollbar();
		}
		return;
	}

	int y = text->first_y;
	Row * cursorrow = text->cursor.row();

	text->setCursorFromCoordinates(bv, text->cursor.x_fix(), y);
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
		if (text->inset_owner) {
			new_y = bv->text->cursor.iy()
				+ bv->theLockingInset()->insetInInsetY() + y
				+ text->cursor.row()->height()
				- bv->workarea().workHeight() + 1;
		} else {
			new_y = text->cursor.y()
				- text->cursor.row()->baseline()
				+ text->cursor.row()->height()
				- bv->workarea().workHeight() + 1;
		}
	}
	bv->screen().draw(bv->text, bv, new_y < 0 ? 0 : new_y);
	if (text->cursor.row()->previous()) {
		LyXCursor cur;
		text->setCursor(bv, cur, text->cursor.row()->previous()->par(),
						text->cursor.row()->previous()->pos(), false);
		if (cur.y() > text->first_y) {
			text->cursorUp(bv, true);
		}
	}
	bv->updateScrollbar();
}


void cursorNext(LyXText * text, BufferView * bv)
{
	if (!text->cursor.row()->next()) {
		int y = text->cursor.y() - text->cursor.row()->baseline() +
			text->cursor.row()->height();
		if (y > int(text->first_y + bv->workarea().workHeight())) {
			bv->screen().draw(bv->text, bv,
						  bv->text->first_y + bv->workarea().workHeight());
			bv->updateScrollbar();
		}
		return;
	}

	int y = text->first_y + bv->workarea().workHeight();
	if (text->inset_owner && !text->first_y) {
		y -= (bv->text->cursor.iy()
			  - bv->text->first_y
			  + bv->theLockingInset()->insetInInsetY());
	}

	text->getRowNearY(y);

	Row * cursorrow = text->cursor.row();
	text->setCursorFromCoordinates(bv, text->cursor.x_fix(), y);
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
		if (text->inset_owner) {
			new_y = bv->text->cursor.iy()
				+ bv->theLockingInset()->insetInInsetY()
				+ y - text->cursor.row()->baseline();
		} else {
			new_y =  text->cursor.y() - text->cursor.row()->baseline();
		}
	}
	bv->screen().draw(bv->text, bv, new_y);
	if (text->cursor.row()->next()) {
		LyXCursor cur;
		text->setCursor(bv, cur, text->cursor.row()->next()->par(),
						text->cursor.row()->next()->pos(), false);
		if (cur.y() < int(text->first_y + bv->workarea().workHeight())) {
			text->cursorDown(bv, true);
		}
	}
	bv->updateScrollbar();
}

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
		bv->finishChange();
		break;

	case LFUN_DELETE_WORD_BACKWARD:
		bv->beforeChange(this);
		update(bv, false);
		deleteWordBackward(bv);
		update(bv, true);
		bv->finishChange();
		break;

	case LFUN_DELETE_LINE_FORWARD:
		bv->beforeChange(this);
		update(bv, false);
		deleteLineForward(bv);
		update(bv);
		bv->finishChange();
		break;

	case LFUN_SHIFT_TAB:
	case LFUN_TAB:
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv, false);
		cursorTab(bv);
		bv->finishChange();
		break;

	case LFUN_WORDRIGHT:
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorLeftOneWord(bv);
		else
			cursorRightOneWord(bv);
		bv->finishChange();
		break;

	case LFUN_WORDLEFT:
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorRightOneWord(bv);
		else
			cursorLeftOneWord(bv);
		bv->finishChange();
		break;

	case LFUN_BEGINNINGBUF:
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv, false);
		cursorTop(bv);
		bv->finishChange();
		break;

	case LFUN_ENDBUF:
		if (selection.mark())
			bv->beforeChange(this);
		update(bv, false);
		cursorBottom(bv);
		bv->finishChange();
		break;

	case LFUN_RIGHTSEL:
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorLeft(bv);
		else
			cursorRight(bv);
		bv->finishChange(true);
		break;

	case LFUN_LEFTSEL:
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorRight(bv);
		else
			cursorLeft(bv);
		bv->finishChange(true);
		break;

	case LFUN_UPSEL:
		update(bv, false);
		cursorUp(bv, true);
		bv->finishChange(true);
		break;

	case LFUN_DOWNSEL:
		update(bv, false);
		cursorDown(bv, true);
		bv->finishChange(true);
		break;

	case LFUN_UP_PARAGRAPHSEL:
		update(bv, false);
		cursorUpParagraph(bv);
		bv->finishChange(true);
		break;

	case LFUN_DOWN_PARAGRAPHSEL:
		update(bv, false);
		cursorDownParagraph(bv);
		bv->finishChange(true);
		break;

	case LFUN_PRIORSEL:
		update(bv, false);
		cursorPrevious(this, bv);
		bv->finishChange(true);
		break;

	case LFUN_NEXTSEL:
		update(bv, false);
		cursorNext(this, bv);
		bv->finishChange();
		break;

	case LFUN_HOMESEL:
		update(bv, false);
		cursorHome(bv);
		bv->finishChange(true);
		break;

	case LFUN_ENDSEL:
		update(bv, false);
		cursorEnd(bv);
		bv->finishChange(true);
		break;

	case LFUN_WORDRIGHTSEL:
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorLeftOneWord(bv);
		else
			cursorRightOneWord(bv);
		bv->finishChange(true);
		break;

	case LFUN_WORDLEFTSEL:
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorRightOneWord(bv);
		else
			cursorLeftOneWord(bv);
		bv->finishChange(true);
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
		bv->finishChange(false);
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
		bv->finishChange(false);
		break;
	}

	case LFUN_UP:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		cursorUp(bv);
		bv->finishChange(false);
		break;

	case LFUN_DOWN:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		cursorDown(bv);
		bv->finishChange();
		break;

	case LFUN_UP_PARAGRAPH:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		cursorUpParagraph(bv);
		bv->finishChange();
		break;

	case LFUN_DOWN_PARAGRAPH:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		cursorDownParagraph(bv);
		bv->finishChange(false);
		break;

	case LFUN_PRIOR:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		cursorPrevious(this, bv);
		bv->finishChange(false);
		// was:
		// finishUndo();
		// moveCursorUpdate(false, false);
		// owner_->view_state_changed();
		break;

	case LFUN_NEXT:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		cursorNext(this, bv);
		bv->finishChange(false);
		break;

	case LFUN_HOME:
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv);
		cursorHome(bv);
		bv->finishChange(false);
		break;

	case LFUN_END:
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv);
		cursorEnd(bv);
		bv->finishChange(false);
		break;

	case LFUN_BREAKLINE:
		bv->beforeChange(this);
		insertChar(bv, Paragraph::META_NEWLINE);
		update(bv, true);
		setCursor(bv, cursor.par(), cursor.pos());
		bv->moveCursorUpdate(false);
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
		bv->moveCursorUpdate(false);
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
		bv->moveCursorUpdate(false);
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
		bv->finishChange(true);
		break;

	case LFUN_ENDBUFSEL:
		if (inset_owner)
			return Inset::UNDISPATCHED;
		update(bv, false);
		cursorBottom(bv);
		bv->finishChange(true);
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
		string const clip(bv->workarea().getClipboard());
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
		bv->moveCursorUpdate(false);

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
