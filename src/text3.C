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
#include "frontends/WorkArea.h"
#include "insets/insetspecialchar.h"
#include "insets/insettext.h"


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
		return Inset::DISPATCHED;
	}

	case LFUN_DELETE_WORD_FORWARD:
		bv->beforeChange(this);
		update(bv, false);
		deleteWordForward(bv);
		update(bv);
		bv->finishChange();
		return Inset::DISPATCHED;

	case LFUN_DELETE_WORD_BACKWARD:
		bv->beforeChange(this);
		update(bv, false);
		deleteWordBackward(bv);
		update(bv, true);
		bv->finishChange();
		return Inset::DISPATCHED;

	case LFUN_DELETE_LINE_FORWARD:
		bv->beforeChange(this);
		update(bv, false);
		deleteLineForward(bv);
		update(bv);
		bv->finishChange();
		return Inset::DISPATCHED;

	case LFUN_SHIFT_TAB:
	case LFUN_TAB:
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv, false);
		cursorTab(bv);
		bv->finishChange();
		return Inset::DISPATCHED;

	case LFUN_WORDRIGHT: 
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorLeftOneWord(bv);
		else
			cursorRightOneWord(bv);
		bv->finishChange();
		return Inset::DISPATCHED;

	case LFUN_WORDLEFT:
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorRightOneWord(bv);
		else
			cursorLeftOneWord(bv);
		bv->finishChange();
		return Inset::DISPATCHED;

	case LFUN_BEGINNINGBUF:
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv, false);
		cursorTop(bv);
		bv->finishChange();
		return Inset::DISPATCHED;

	case LFUN_ENDBUF:
		if (selection.mark())
			bv->beforeChange(this);
		update(bv, false);
		cursorBottom(bv);
		bv->finishChange();
		return Inset::DISPATCHED;

	case LFUN_RIGHTSEL:
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorLeft(bv);
		else
			cursorRight(bv);
		bv->finishChange(true);
		return Inset::DISPATCHED;

	case LFUN_LEFTSEL:
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorRight(bv);
		else
			cursorLeft(bv);
		bv->finishChange(true);
		return Inset::DISPATCHED;

	case LFUN_UPSEL:
		update(bv, false);
		cursorUp(bv, true);
		bv->finishChange(true);
		return Inset::DISPATCHED;

	case LFUN_DOWNSEL:
		update(bv, false);
		cursorDown(bv, true);
		bv->finishChange(true);
		return Inset::DISPATCHED;

	case LFUN_UP_PARAGRAPHSEL:
		update(bv, false);
		cursorUpParagraph(bv);
		bv->finishChange(true);
		return Inset::DISPATCHED;

	case LFUN_DOWN_PARAGRAPHSEL:
		update(bv, false);
		cursorDownParagraph(bv);
		bv->finishChange(true);
		return Inset::DISPATCHED;

	case LFUN_PRIORSEL:
		update(bv, false);
		bv->cursorPrevious(this);
		bv->finishChange(true);
		return Inset::DISPATCHED;

	case LFUN_NEXTSEL:
		update(bv, false);
		bv->cursorNext(this);
		bv->finishChange();
		return Inset::DISPATCHED;

	case LFUN_HOMESEL:
		update(bv, false);
		cursorHome(bv);
		bv->finishChange(true);
		return Inset::DISPATCHED;

	case LFUN_ENDSEL:
		update(bv, false);
		cursorEnd(bv);
		bv->finishChange(true);
		return Inset::DISPATCHED;

	case LFUN_WORDRIGHTSEL:
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorLeftOneWord(bv);
		else
			cursorRightOneWord(bv);
		bv->finishChange(true);
		return Inset::DISPATCHED;

	case LFUN_WORDLEFTSEL:
		update(bv, false);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorRightOneWord(bv);
		else
			cursorLeftOneWord(bv);
		bv->finishChange(true);
		return Inset::DISPATCHED;

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
			if (is_rtl)
				tmpinset->edit(bv, false);
			else
				tmpinset->edit(bv);
			return Inset::DISPATCHED;
		}
		if (!is_rtl)
			cursorRight(bv, false);
		bv->finishChange(false);
		return Inset::DISPATCHED;
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
			if (is_rtl)
				tmpinset->edit(bv);
			else
				tmpinset->edit(bv, false);
			break;
			return Inset::DISPATCHED;
		}
		if (is_rtl)
			cursorRight(bv, false);
		bv->finishChange(false);
		return Inset::DISPATCHED;
	}

	case LFUN_UP:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		cursorUp(bv);
		bv->finishChange(false);
		return Inset::DISPATCHED;

	case LFUN_DOWN:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		cursorDown(bv);
		bv->finishChange();
		return Inset::DISPATCHED;

	case LFUN_UP_PARAGRAPH:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		cursorUpParagraph(bv);
		bv->finishChange();
		return Inset::DISPATCHED;

	case LFUN_DOWN_PARAGRAPH:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		cursorDownParagraph(bv);
		bv->finishChange(false);
		return Inset::DISPATCHED;

	case LFUN_PRIOR:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		bv->cursorPrevious(this);
		bv->finishChange(false);
		// was:
		// finishUndo();
		// moveCursorUpdate(false, false);
		// owner_->view_state_changed();
		return Inset::DISPATCHED;

	case LFUN_NEXT:
		if (!selection.mark())
			bv->beforeChange(this);
		bv->update(this, BufferView::UPDATE);
		bv->cursorNext(this);
		bv->finishChange(false);
		return Inset::DISPATCHED;

	case LFUN_HOME:
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv);
		cursorHome(bv);
		bv->finishChange(false);
		return Inset::DISPATCHED;

	case LFUN_END:
		if (!selection.mark())
			bv->beforeChange(this);
		update(bv);
		cursorEnd(bv);
		bv->finishChange(false);
		return Inset::DISPATCHED;

	case LFUN_BREAKLINE:
		bv->beforeChange(this);
		insertChar(bv, Paragraph::META_NEWLINE);
		update(bv, true);
		setCursor(bv, cursor.par(), cursor.pos());
		bv->moveCursorUpdate(false);
		return Inset::DISPATCHED;

	case LFUN_PROTECTEDSPACE:
		if (cursor.par()->layout()->free_spacing) {
			insertChar(bv, ' ');
			update(bv);
		} else {
			specialChar(this, bv, InsetSpecialChar::PROTECTED_SEPARATOR);
		}
		bv->moveCursorUpdate(false);
		return Inset::DISPATCHED;

	case LFUN_HYPHENATION:
		specialChar(this, bv, InsetSpecialChar::HYPHENATION);
		return Inset::DISPATCHED;

	case LFUN_LIGATURE_BREAK:
		specialChar(this, bv, InsetSpecialChar::LIGATURE_BREAK);
		return Inset::DISPATCHED;

	case LFUN_LDOTS:
		specialChar(this, bv, InsetSpecialChar::LDOTS);
		return Inset::DISPATCHED;

	case LFUN_HFILL:
		bv->hideCursor();
		update(bv, false);
		insertChar(bv, Paragraph::META_HFILL);
		update(bv);
		return Inset::DISPATCHED;

	case LFUN_END_OF_SENTENCE:
		specialChar(this, bv, InsetSpecialChar::END_OF_SENTENCE);
		return Inset::DISPATCHED;

	case LFUN_MENU_SEPARATOR:
		specialChar(this, bv, InsetSpecialChar::MENU_SEPARATOR);
		return Inset::DISPATCHED;

	case LFUN_MARK_OFF:
		bv->beforeChange(this);
		update(bv, false);
		selection.cursor = cursor;
		cmd.message(N_("Mark off"));
		return Inset::DISPATCHED;

	case LFUN_MARK_ON:
		bv->beforeChange(this);
		selection.mark(true);
		update(bv, false);
		selection.cursor = cursor;
		cmd.message(N_("Mark on"));
		return Inset::DISPATCHED;

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
		return Inset::DISPATCHED;

	case LFUN_UPCASE_WORD:
		update(bv, false);
		changeCase(bv, LyXText::text_uppercase);
		if (inset_owner)
			bv->updateInset(inset_owner, true);
		update(bv);
		return Inset::DISPATCHED;

	case LFUN_LOWCASE_WORD:
		update(bv, false);
		changeCase(bv, LyXText::text_lowercase);
		if (inset_owner)
			bv->updateInset(inset_owner, true);
		update(bv);
		return Inset::DISPATCHED;

	case LFUN_CAPITALIZE_WORD:
		update(bv, false);
		changeCase(bv, LyXText::text_capitalization);
		if (inset_owner)
			bv->updateInset(inset_owner, true);
		update(bv);
		return Inset::DISPATCHED;

	case LFUN_TRANSPOSE_CHARS:
		update(bv, false);
		transposeChars(*bv);
		if (inset_owner)
			bv->updateInset(inset_owner, true);
		update(bv);
		return Inset::DISPATCHED;

	case LFUN_BEGINNINGBUFSEL:
		if (inset_owner)
			return Inset::UNDISPATCHED;
		update(bv, false);
		cursorTop(bv);
		bv->finishChange(true);
		return Inset::DISPATCHED;

	case LFUN_ENDBUFSEL:
		if (inset_owner)
			return Inset::UNDISPATCHED;
		update(bv, false);
		cursorBottom(bv);
		bv->finishChange(true);
		return Inset::DISPATCHED;

	case LFUN_GETXY:
		cmd.message(tostr(cursor.x()) + ' ' + tostr(cursor.y()));
		return Inset::DISPATCHED;

	case LFUN_SETXY: {
		int x = 0;
		int y = 0;
		if (::sscanf(cmd.argument.c_str(), " %d %d", &x, &y) != 2)
			lyxerr << "SETXY: Could not parse coordinates in '"
			       << cmd.argument << std::endl;
		setCursorFromCoordinates(bv, x, y);
		return Inset::DISPATCHED;
	}

	case LFUN_GETFONT:
		if (current_font.shape() == LyXFont::ITALIC_SHAPE)
			cmd.message("E");
		else if (current_font.shape() == LyXFont::SMALLCAPS_SHAPE)
			cmd.message("N");
		else
			cmd.message("0");
		return Inset::DISPATCHED;

	case LFUN_GETLAYOUT:
		cmd.message(tostr(cursor.par()->layout()));
		return Inset::DISPATCHED;

	case LFUN_SELFINSERT: {
		if (cmd.argument.empty())
			return Inset::DISPATCHED;

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
		return Inset::DISPATCHED;
	}

	default:
		return Inset::UNDISPATCHED;
	}

	// shut up compiler
	return Inset::UNDISPATCHED;
}
