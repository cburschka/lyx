/**
 * \file text3.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author Alfredo Braunstein
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyxtext.h"

#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "factory.h"
#include "funcrequest.h"
#include "gettext.h"
#include "intl.h"
#include "language.h"
#include "lyxrc.h"
#include "lyxrow.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "undo.h"
#include "vspace.h"

#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"

#include "insets/insetcommand.h"
#include "insets/insetnewline.h"
#include "insets/insetspecialchar.h"
#include "insets/insettext.h"

#include "support/lstrings.h"
#include "support/tostr.h"
#include "support/std_sstream.h"

#include "mathed/formulabase.h"

#include <clocale>

using bv_funcs::replaceSelection;

using lyx::pos_type;

using lyx::support::isStrUnsignedInt;
using lyx::support::strToUnsignedInt;

using std::endl;
using std::find;
using std::string;
using std::istringstream;
using std::vector;


extern string current_layout;
extern int bibitemMaxWidth(BufferView *, LyXFont const &);

// the selection possible is needed, that only motion events are
// used, where the bottom press event was on the drawing area too
bool selection_possible = false;


namespace {

	// globals...
	LyXFont freefont(LyXFont::ALL_IGNORE);
	bool toggleall = false;


	void toggleAndShow(BufferView * bv, LyXText * text,
		LyXFont const & font, bool toggleall = true)
	{
		if (!bv->available())
			return;

		text->toggleFree(font, toggleall);
		bv->update();

		if (font.language() != ignore_language ||
				font.number() != LyXFont::IGNORE) {
			LyXCursor & cursor = text->cursor;
			Paragraph & par = *text->cursorPar();
			text->bidi.computeTables(par, *bv->buffer(),
				*par.getRow(cursor.pos()));
			if (cursor.boundary() !=
					text->bidi.isBoundary(*bv->buffer(), par,
							cursor.pos(),
							text->real_current_font))
				text->setCursor(cursor.par(), cursor.pos(),
						false, !cursor.boundary());
		}
	}


	/// Apply the contents of freefont at the current cursor location.
	void apply_freefont(BufferView * bv, LyXText * text)
	{
		toggleAndShow(bv, text, freefont, toggleall);
		bv->owner()->view_state_changed();
		bv->owner()->message(_("Character set"));
	}


	/** Set the freefont using the contents of \param data dispatched from
	 *  the frontends and apply it at the current cursor location.
	 */
	void update_and_apply_freefont(BufferView * bv, LyXText * text,
		string const & data)
	{
		LyXFont font;
		bool toggle;
		if (bv_funcs::string2font(data, font, toggle)) {
			freefont = font;
			toggleall = toggle;
			apply_freefont(bv, text);
		}
	}


	void emph(BufferView * bv, LyXText * text)
	{
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setEmph(LyXFont::TOGGLE);
		toggleAndShow(bv, text, font);
	}


	void bold(BufferView * bv, LyXText * text)
	{
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setSeries(LyXFont::BOLD_SERIES);
		toggleAndShow(bv, text, font);
	}


	void noun(BufferView * bv, LyXText * text)
	{
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setNoun(LyXFont::TOGGLE);
		toggleAndShow(bv, text, font);
	}


	void lang(BufferView * bv, string const & l, LyXText * text)
	{
		Language const * lang = languages.getLanguage(l);
		if (!lang)
			return;

		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(lang);
		toggleAndShow(bv, text, font);
	}


	void code(BufferView * bv, LyXText * text)
	{
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setFamily(LyXFont::TYPEWRITER_FAMILY); // no good
		toggleAndShow(bv, text, font);
	}


	void sans(BufferView * bv, LyXText * text)
	{
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setFamily(LyXFont::SANS_FAMILY);
		toggleAndShow(bv, text, font);
	}


	void roman(BufferView * bv, LyXText * text)
	{
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setFamily(LyXFont::ROMAN_FAMILY);
		toggleAndShow(bv, text, font);
	}


	void styleReset(BufferView * bv, LyXText * text)
	{
		LyXFont font(LyXFont::ALL_INHERIT, ignore_language);
		toggleAndShow(bv, text, font);
	}


	void underline(BufferView * bv, LyXText * text)
	{
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setUnderbar(LyXFont::TOGGLE);
		toggleAndShow(bv, text, font);
	}


	void fontSize(BufferView * bv, string const & size, LyXText * text)
	{
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLyXSize(size);
		toggleAndShow(bv, text, font);
	}


	void moveCursor(BufferView * bv, bool selecting)
	{
		LyXText * lt = bv->getLyXText();

//		if (!lt->selection.set())
//			lt->selection.cursor = lt->cursor;

		if (selecting || lt->selection.mark())
			lt->setSelection();

		if (!lt->selection.set())
			bv->haveSelection(false);
		bv->switchKeyMap();
	}


	void finishChange(BufferView * bv, bool selecting = false)
	{
		finishUndo();
		moveCursor(bv, selecting);
		bv->owner()->view_state_changed();
	}

} // anon namespace


namespace bv_funcs {

string const freefont2string()
{
	string data;
	if (font2string(freefont, toggleall, data))
		return data;
	return string();
}

}



InsetOld * LyXText::checkInsetHit(int x, int y)
{
	ParagraphList::iterator pit;
	ParagraphList::iterator end;

	getParsInRange(ownerParagraphs(),
		       bv()->top_y(), bv()->top_y() + bv()->workHeight(),
		       pit, end);
	
	lyxerr << "checkInsetHit: x: " << x << " y: " << y << endl;
	for ( ; pit != end; ++pit) {
		InsetList::iterator iit = pit->insetlist.begin();
		InsetList::iterator iend = pit->insetlist.end();
		for ( ; iit != iend; ++iit) {
			InsetOld * inset = iit->inset;
			//lyxerr << "examining inset " << inset
			//	<< " xy: " << inset->x() << "/" << inset->y()
			//	<< " x: " << inset->x() << "..." << inset->x() + inset->width()
			//	<< " y: " << inset->y() - inset->ascent() << "..."
			//	<< inset->y() + inset->descent()
			//	<< endl;
			if (x >= inset->x()
			    && x <= inset->x() + inset->width()
			    && y >= inset->y() - inset->ascent()
			    && y <= inset->y() + inset->descent())
			{
				lyxerr << "Hit inset: " << inset << endl;
				return inset;
			}
		}
	}
	lyxerr << "No inset hit. " << endl;
	return 0;
}


bool LyXText::gotoNextInset(vector<InsetOld::Code> const & codes,
			    string const & contents)
{
	ParagraphList::iterator end = ownerParagraphs().end();
	ParagraphList::iterator pit = cursorPar();
	pos_type pos = cursor.pos();

	InsetOld * inset;
	do {
		if (pos + 1 < pit->size()) {
			++pos;
		} else  {
			++pit;
			pos = 0;
		}

	} while (pit != end &&
		 !(pit->isInset(pos) &&
		   (inset = pit->getInset(pos)) != 0 &&
		   find(codes.begin(), codes.end(), inset->lyxCode()) != codes.end() &&
		   (contents.empty() ||
		    static_cast<InsetCommand *>(pit->getInset(pos))->getContents()
		    == contents)));

	if (pit == end)
		return false;

	setCursor(parOffset(pit), pos, false);
	return true;
}


void LyXText::gotoInset(vector<InsetOld::Code> const & codes,
			bool same_content)
{
	clearSelection();

	string contents;
	if (same_content && cursor.pos() < cursorPar()->size()
	    && cursorPar()->isInset(cursor.pos())) {
		InsetOld const * inset = cursorPar()->getInset(cursor.pos());
		if (find(codes.begin(), codes.end(), inset->lyxCode())
		    != codes.end())
			contents = static_cast<InsetCommand const *>(inset)->getContents();
	}

	if (!gotoNextInset(codes, contents)) {
		if (cursor.pos() || cursorPar() != ownerParagraphs().begin()) {
			LyXCursor tmp = cursor;
			cursor.par(0);
			cursor.pos(0);
			if (!gotoNextInset(codes, contents)) {
				cursor = tmp;
				bv()->owner()->message(_("No more insets"));
			}
		} else {
			bv()->owner()->message(_("No more insets"));
		}
	}
	bv()->update();
	selection.cursor = cursor;
}


void LyXText::gotoInset(InsetOld::Code code, bool same_content)
{
	gotoInset(vector<InsetOld::Code>(1, code), same_content);
}


void LyXText::cursorPrevious()
{
	int y = bv()->top_y();

	ParagraphList::iterator cpit = cursorPar();
	RowList::iterator crit = cpit->getRow(cursor.pos());

	if (isFirstRow(cpit, *crit)) {
		if (y > 0)
			bv()->updateScrollbar();
		return;
	}

	setCursorFromCoordinates(bv()->x_target(), y);
	finishUndo();

	if (crit == bv()->text->cursorRow()) {
		// we have a row which is taller than the workarea. The
		// simplest solution is to move to the previous row instead.
		cursorUp(true);
		return;
	}

	int new_y = + crit->height() - bv()->workHeight() + 1;

	if (inset_owner) {
		new_y += bv()->text->cursor.y()
			+ bv()->cursor().innerInset()->insetInInsetY()
			+ y;
	} else {
		new_y += cursor.y() - crit->baseline();
	}

	previousRow(cpit, crit);
	LyXCursor cur;
	setCursor(cur, parOffset(cpit), crit->pos(), false);
	if (cur.y() > bv()->top_y())
		cursorUp(true);
	bv()->updateScrollbar();
}


void LyXText::cursorNext()
{
	int topy = bv()->top_y();

	ParagraphList::iterator cpit = cursorPar();
	RowList::iterator crit = cpit->getRow(cursor.pos());

	if (isLastRow(cpit, *crit)) {
		int y = cursor.y() - crit->baseline() + crit->height();
		if (y > topy + bv()->workHeight())
			bv()->updateScrollbar();
		return;
	}

	int y = topy + bv()->workHeight();
	if (inset_owner && !topy) {
		y += - bv()->text->cursor.y()
			   + bv()->top_y()
			   - bv()->cursor().innerInset()->insetInInsetY();
	}

	ParagraphList::iterator dummypit;
	Row const & row = *getRowNearY(y, dummypit);
	y = dummypit->y + row.y_offset();

	setCursorFromCoordinates(bv()->x_target(), y);
	// + bv->workHeight());
	finishUndo();

	int new_y;
	if (crit == bv()->text->cursorRow()) {
		// we have a row which is taller than the workarea. The
		// simplest solution is to move to the next row instead.
		cursorDown(true);
		return;
		// This is what we used to do, so we wouldn't skip right past
		// tall rows, but it's not working right now.
#if 0
		new_y = bv->top_y() + bv->workHeight();
#endif
	}

	if (inset_owner) {
		new_y = bv()->text->cursor.y()
			+ bv()->cursor().innerInset()->insetInInsetY()
			+ y - crit->baseline();
	} else {
		new_y = cursor.y() - crit->baseline();
	}


	nextRow(cpit, crit);
	LyXCursor cur;
	setCursor(cur, parOffset(cpit), crit->pos(), false);
	if (cur.y() < bv()->top_y() + bv()->workHeight())
		cursorDown(true);
	bv()->updateScrollbar();
}


namespace {

void specialChar(LyXText * lt, BufferView * bv, InsetSpecialChar::Kind kind)
{
	bv->update();
	InsetSpecialChar * new_inset = new InsetSpecialChar(kind);
	replaceSelection(lt);
	if (!bv->insertInset(new_inset))
		delete new_inset;
	else
		bv->update();
}


void doInsertInset(LyXText * lt, FuncRequest const & cmd,
		   bool edit, bool pastesel)
{
	InsetOld * inset = createInset(cmd);
	BufferView * bv = cmd.view();

	if (inset) {
		bool gotsel = false;
		if (lt->selection.set()) {
			bv->owner()->dispatch(FuncRequest(LFUN_CUT));
			gotsel = true;
		}
		if (bv->insertInset(inset)) {
			if (edit)
				inset->edit(bv, true);
			if (gotsel && pastesel)
				bv->owner()->dispatch(FuncRequest(LFUN_PASTE));
		} else
			delete inset;
	}
}

} // anon namespace


void LyXText::number()
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setNumber(LyXFont::TOGGLE);
	toggleAndShow(bv(), this, font);
}


bool LyXText::rtl() const
{
	return cursorPar()->isRightToLeftPar(bv()->buffer()->params());
}


DispatchResult LyXText::dispatch(FuncRequest const & cmd)
{
	//lyxerr[Debug::ACTION] << "LyXText::dispatch: cmd: " << cmd << endl;
	lyxerr << "LyXText::dispatch: cmd: " << cmd << endl;

	BufferView * bv = cmd.view();

	switch (cmd.action) {

	case LFUN_APPENDIX: {
		ParagraphList::iterator pit = cursorPar();
		bool start = !pit->params().startOfAppendix();

		// ensure that we have only one start_of_appendix in this document
		ParagraphList::iterator tmp = ownerParagraphs().begin();
		ParagraphList::iterator end = ownerParagraphs().end();

		for (; tmp != end; ++tmp) {
			if (tmp->params().startOfAppendix()) {
				recUndo(parOffset(tmp));
				tmp->params().startOfAppendix(false);
				redoParagraph(tmp);
				break;
			}
		}

		recUndo(parOffset(pit));
		pit->params().startOfAppendix(start);

		// we can set the refreshing parameters now
		updateCounters();
		redoParagraph(cursorPar());
		setCursor(cursorPar(), cursor.pos());
		bv->update();
		break;
	}

	case LFUN_DELETE_WORD_FORWARD:
		clearSelection();
		deleteWordForward();
		finishChange(bv);
		break;

	case LFUN_DELETE_WORD_BACKWARD:
		clearSelection();
		deleteWordBackward();
		finishChange(bv);
		break;

	case LFUN_DELETE_LINE_FORWARD:
		clearSelection();
		deleteLineForward();
		finishChange(bv);
		break;

	case LFUN_WORDRIGHT:
		if (!selection.mark())
			clearSelection();
		if (rtl())
			cursorLeftOneWord();
		else
			cursorRightOneWord();
		finishChange(bv);
		break;

	case LFUN_WORDLEFT:
		if (!selection.mark())
			clearSelection();
		if (rtl())
			cursorRightOneWord();
		else
			cursorLeftOneWord();
		finishChange(bv);
		break;

	case LFUN_BEGINNINGBUF:
		if (!selection.mark())
			clearSelection();
		cursorTop();
		finishChange(bv);
		break;

	case LFUN_ENDBUF:
		if (selection.mark())
			clearSelection();
		cursorBottom();
		finishChange(bv);
		break;

	case LFUN_RIGHTSEL:
		if (!selection.set())
			selection.cursor = cursor;
		if (rtl())
			cursorLeft(bv);
		else
			cursorRight(bv);
		finishChange(bv, true);
		break;

	case LFUN_LEFTSEL:
		if (!selection.set())
			selection.cursor = cursor;
		if (rtl())
			cursorRight(bv);
		else
			cursorLeft(bv);
		finishChange(bv, true);
		break;

	case LFUN_UPSEL:
		if (!selection.set())
			selection.cursor = cursor;
		cursorUp(true);
		finishChange(bv, true);
		break;

	case LFUN_DOWNSEL:
		if (!selection.set())
			selection.cursor = cursor;
		cursorDown(true);
		finishChange(bv, true);
		break;

	case LFUN_UP_PARAGRAPHSEL:
		if (!selection.set())
			selection.cursor = cursor;
		cursorUpParagraph();
		finishChange(bv, true);
		break;

	case LFUN_DOWN_PARAGRAPHSEL:
		if (!selection.set())
			selection.cursor = cursor;
		cursorDownParagraph();
		finishChange(bv, true);
		break;

	case LFUN_PRIORSEL:
		if (!selection.set())
			selection.cursor = cursor;
		cursorPrevious();
		finishChange(bv, true);
		break;

	case LFUN_NEXTSEL:
		if (!selection.set())
			selection.cursor = cursor;
		cursorNext();
		finishChange(bv, true);
		break;

	case LFUN_HOMESEL:
		if (!selection.set())
			selection.cursor = cursor;
		cursorHome();
		finishChange(bv, true);
		break;

	case LFUN_ENDSEL:
		if (!selection.set())
			selection.cursor = cursor;
		cursorEnd();
		finishChange(bv, true);
		break;

	case LFUN_WORDRIGHTSEL:
		if (rtl())
			cursorLeftOneWord();
		else
			cursorRightOneWord();
		finishChange(bv, true);
		break;

	case LFUN_WORDLEFTSEL:
		if (rtl())
			cursorRightOneWord();
		else
			cursorLeftOneWord();
		finishChange(bv, true);
		break;

	case LFUN_WORDSEL: {
		LyXCursor cur1 = cursor;
		LyXCursor cur2;
		getWord(cur1, cur2, lyx::WHOLE_WORD);
		setCursor(cur1.par(), cur1.pos());
		clearSelection();
		setCursor(cur2.par(), cur2.pos());
		finishChange(bv, true);
		break;
	}

	case LFUN_RIGHT: {
		bool is_rtl = rtl();
		if (!selection.mark())
			clearSelection();
		if (is_rtl)
			cursorLeft(false);
		if (cursor.pos() < cursorPar()->size()
		    && cursorPar()->isInset(cursor.pos())
		    && isHighlyEditableInset(cursorPar()->getInset(cursor.pos()))) {
			InsetOld * tmpinset = cursorPar()->getInset(cursor.pos());
			cmd.message(tmpinset->editMessage());
			tmpinset->edit(bv, !is_rtl);
			break;
		}
		if (!is_rtl)
			cursorRight(false);
		finishChange(bv);
		break;
	}

	case LFUN_LEFT: {
		// This is soooo ugly. Isn't it possible to make
		// it simpler? (Lgb)
		bool const is_rtl = rtl();
		if (!selection.mark())
			clearSelection();
		LyXCursor const cur = cursor;
		if (!is_rtl)
			cursorLeft(false);
		if ((is_rtl || cur != cursor) && // only if really moved!
		    cursor.pos() < cursorPar()->size() &&
		    cursorPar()->isInset(cursor.pos()) &&
		    isHighlyEditableInset(cursorPar()->getInset(cursor.pos()))) {
			InsetOld * tmpinset = cursorPar()->getInset(cursor.pos());
			cmd.message(tmpinset->editMessage());
			tmpinset->edit(bv, is_rtl);
			break;
		}
		if (is_rtl)
			cursorRight(false);
		finishChange(bv);
		break;
	}

	case LFUN_UP:
		if (!selection.mark())
			clearSelection();
		cursorUp(false);
		finishChange(bv);
		break;

	case LFUN_DOWN:
		if (!selection.mark())
			clearSelection();
		cursorDown(false);
		finishChange(bv);
		break;

	case LFUN_UP_PARAGRAPH:
		if (!selection.mark())
			clearSelection();
		cursorUpParagraph();
		finishChange(bv);
		break;

	case LFUN_DOWN_PARAGRAPH:
		if (!selection.mark())
			clearSelection();
		cursorDownParagraph();
		finishChange(bv, false);
		break;

	case LFUN_PRIOR:
		if (!selection.mark())
			clearSelection();
		cursorPrevious();
		finishChange(bv, false);
		break;

	case LFUN_NEXT:
		if (!selection.mark())
			clearSelection();
		cursorNext();
		finishChange(bv, false);
		break;

	case LFUN_HOME:
		if (!selection.mark())
			clearSelection();
		cursorHome();
		finishChange(bv, false);
		break;

	case LFUN_END:
		if (!selection.mark())
			clearSelection();
		cursorEnd();
		finishChange(bv, false);
		break;

	case LFUN_BREAKLINE: {
		lyx::pos_type body = cursorPar()->beginOfBody();

		// Not allowed by LaTeX (labels or empty par)
		if (cursor.pos() <= body)
			break;

		replaceSelection(bv->getLyXText());
		insertInset(new InsetNewline);
		setCursor(cursorPar(), cursor.pos());
		moveCursor(bv, false);
		break;
	}

	case LFUN_DELETE:
		if (!selection.set()) {
			Delete();
			selection.cursor = cursor;
			// It is possible to make it a lot faster still
			// just comment out the line below...
		} else {
			cutSelection(true, false);
		}
		moveCursor(bv, false);
		bv->owner()->view_state_changed();
		break;

	case LFUN_DELETE_SKIP:
		// Reverse the effect of LFUN_BREAKPARAGRAPH_SKIP.
		if (!selection.set()) {
			if (cursor.pos() == cursorPar()->size()) {
				cursorRight(bv);
				ParagraphParameters & params = cursorPar()->params();
				if (cursor.pos() == 0
				    && !(params.spaceTop() == VSpace (VSpace::NONE))) {
					setParagraph(
						 VSpace(VSpace::NONE),
						 params.spaceBottom(),
						 params.spacing(),
						 params.align(),
						 params.labelWidthString(), 0);
					cursorLeft(bv);
				} else {
					cursorLeft(bv);
					Delete();
					selection.cursor = cursor;
				}
			} else {
				Delete();
				selection.cursor = cursor;
			}
		} else {
			cutSelection(true, false);
		}
		bv->update();
		break;


	case LFUN_BACKSPACE:
		if (!selection.set()) {
			if (bv->owner()->getIntl().getTransManager().backspace()) {
				backspace();
				selection.cursor = cursor;
				// It is possible to make it a lot faster still
				// just comment out the line below...
			}
		} else {
			cutSelection(true, false);
		}
		bv->owner()->view_state_changed();
		bv->switchKeyMap();
		bv->update();
		break;

	case LFUN_BACKSPACE_SKIP:
		// Reverse the effect of LFUN_BREAKPARAGRAPH_SKIP.
		if (!selection.set()) {
			ParagraphParameters & params = cursorPar()->params();
			if (cursor.pos() == 0 && !(params.spaceTop() == VSpace(VSpace::NONE))) {
				setParagraph(
					 VSpace(VSpace::NONE),
				   params.spaceBottom(),
					 params.spacing(),
					 params.align(),
					 params.labelWidthString(), 0);
			} else {
				LyXCursor cur = cursor;
				backspace();
				selection.cursor = cur;
			}
		} else {
			cutSelection(true, false);
		}
		bv->update();
		break;

	case LFUN_BREAKPARAGRAPH:
		replaceSelection(bv->getLyXText());
		breakParagraph(bv->buffer()->paragraphs(), 0);
		bv->update();
		selection.cursor = cursor;
		bv->switchKeyMap();
		bv->owner()->view_state_changed();
		break;

	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
		replaceSelection(bv->getLyXText());
		breakParagraph(bv->buffer()->paragraphs(), 1);
		bv->update();
		selection.cursor = cursor;
		bv->switchKeyMap();
		bv->owner()->view_state_changed();
		break;

	case LFUN_BREAKPARAGRAPH_SKIP: {
		// When at the beginning of a paragraph, remove
		// indentation and add a "defskip" at the top.
		// Otherwise, do the same as LFUN_BREAKPARAGRAPH.
		LyXCursor cur = cursor;
		replaceSelection(bv->getLyXText());
		if (cur.pos() == 0) {
			ParagraphParameters & params = getPar(cur)->params();
			if (params.spaceTop() == VSpace(VSpace::NONE)) {
				setParagraph(
					 VSpace(VSpace::DEFSKIP), params.spaceBottom(),
					 params.spacing(),
					 params.align(),
					 params.labelWidthString(), 1);
			}
		}
		else {
			breakParagraph(bv->buffer()->paragraphs(), 0);
		}
		bv->update();
		selection.cursor = cur;
		bv->switchKeyMap();
		bv->owner()->view_state_changed();
		break;
	}

	case LFUN_PARAGRAPH_SPACING: {
		ParagraphList::iterator pit = cursorPar();
		Spacing::Space cur_spacing = pit->params().spacing().getSpace();
		float cur_value = 1.0;
		if (cur_spacing == Spacing::Other)
			cur_value = pit->params().spacing().getValue();

		istringstream is(cmd.argument);
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
			pit->params().spacing(Spacing(new_spacing, new_value));
			redoParagraph();
			bv->update();
		}
		break;
	}

	case LFUN_INSET_SETTINGS:
		bv->cursor().innerInset()->showInsetDialog(bv);
		break;

	case LFUN_INSET_TOGGLE:
		clearSelection();
		toggleInset();
		bv->update();
		bv->switchKeyMap();
		break;

	case LFUN_SPACE_INSERT:
		if (cursorPar()->layout()->free_spacing)
			insertChar(' ');
		else
			doInsertInset(this, cmd, false, false);
		moveCursor(bv, false);
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

	case LFUN_END_OF_SENTENCE:
		specialChar(this, bv, InsetSpecialChar::END_OF_SENTENCE);
		break;

	case LFUN_MENU_SEPARATOR:
		specialChar(this, bv, InsetSpecialChar::MENU_SEPARATOR);
		break;

	case LFUN_MARK_OFF:
		clearSelection();
		bv->update();
		selection.cursor = cursor;
		cmd.message(N_("Mark off"));
		break;

	case LFUN_MARK_ON:
		clearSelection();
		selection.mark(true);
		bv->update();
		selection.cursor = cursor;
		cmd.message(N_("Mark on"));
		break;

	case LFUN_SETMARK:
		clearSelection();
		if (selection.mark()) {
			cmd.message(N_("Mark removed"));
		} else {
			selection.mark(true);
			cmd.message(N_("Mark set"));
		}
		selection.cursor = cursor;
		bv->update();
		break;

	case LFUN_UPCASE_WORD:
		changeCase(LyXText::text_uppercase);
		bv->update();
		break;

	case LFUN_LOWCASE_WORD:
		changeCase(LyXText::text_lowercase);
		bv->update();
		break;

	case LFUN_CAPITALIZE_WORD:
		changeCase(LyXText::text_capitalization);
		bv->update();
		break;

	case LFUN_TRANSPOSE_CHARS:
		recUndo(cursor.par());
		redoParagraph();
		bv->update();
		break;

	case LFUN_PASTE:
		cmd.message(_("Paste"));
		replaceSelection(bv->getLyXText());
#warning FIXME Check if the arg is in the domain of available selections.
		if (isStrUnsignedInt(cmd.argument))
			pasteSelection(strToUnsignedInt(cmd.argument));
		else
			pasteSelection(0);
		clearSelection(); // bug 393
		bv->update();
		bv->switchKeyMap();
		finishUndo();
		break;

	case LFUN_CUT:
		cutSelection(true, true);
		cmd.message(_("Cut"));
		bv->update();
		break;

	case LFUN_COPY:
		copySelection();
		cmd.message(_("Copy"));
		break;

	case LFUN_BEGINNINGBUFSEL:
		if (inset_owner)
			return DispatchResult(false);
		cursorTop();
		finishChange(bv, true);
		break;

	case LFUN_ENDBUFSEL:
		if (inset_owner)
			return DispatchResult(false);
		cursorBottom();
		finishChange(bv, true);
		break;

	case LFUN_GETXY:
		cmd.message(tostr(cursor.x()) + ' ' + tostr(cursor.y()));
		break;

	case LFUN_SETXY: {
		int x = 0;
		int y = 0;
		istringstream is(cmd.argument);
		is >> x >> y;
		if (!is)
			lyxerr << "SETXY: Could not parse coordinates in '"
			       << cmd.argument << std::endl;
		else
			setCursorFromCoordinates(x, y);
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
		cmd.message(tostr(cursorPar()->layout()));
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
		LyXTextClass const & tclass = bv->buffer()->params().getLyXTextClass();
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
			ParagraphList::iterator spit = getPar(selection.start);
			ParagraphList::iterator epit = boost::next(getPar(selection.end));
			while (spit != epit) {
				if (spit->layout()->name() != current_layout) {
					change_layout = true;
					break;
				}
				++spit;
			}
		}

		if (change_layout) {
			current_layout = layout;
			setLayout(layout);
			bv->owner()->setLayout(layout);
			bv->update();
			bv->switchKeyMap();
		}
		break;
	}

	case LFUN_PASTESELECTION: {
		// this was originally a bv->text->clearSelection(), i.e
		// the outermost LyXText!
		clearSelection();
		string const clip = bv->getClipboard();
		if (!clip.empty()) {
			if (cmd.argument == "paragraph")
				insertStringAsParagraphs(clip);
			else
				insertStringAsLines(clip);
			bv->update();
		}
		break;
	}

	case LFUN_GOTOERROR:
		gotoInset(InsetOld::ERROR_CODE, false);
		break;

	case LFUN_GOTONOTE:
		gotoInset(InsetOld::NOTE_CODE, false);
		break;

	case LFUN_REFERENCE_GOTO: {
		vector<InsetOld::Code> tmp;
		tmp.push_back(InsetOld::LABEL_CODE);
		tmp.push_back(InsetOld::REF_CODE);
		gotoInset(tmp, true);
		break;
	}

	case LFUN_QUOTE: {
		replaceSelection(bv->getLyXText());
		ParagraphList::iterator pit = cursorPar();
		lyx::pos_type pos = cursor.pos();
		char c;
		if (!pos)
			c = ' ';
		else if (pit->isInset(pos - 1) && pit->getInset(pos - 1)->isSpace())
			c = ' ';
		else
			c = pit->getChar(pos - 1);

		LyXLayout_ptr const & style = pit->layout();

		BufferParams const & bufparams = bv->buffer()->params();
		if (style->pass_thru ||
		    pit->getFontSettings(bufparams,pos).language()->lang() == "hebrew" ||
		    !bv->insertInset(new InsetQuotes(c, bufparams)))
			bv->owner()->dispatch(FuncRequest(LFUN_SELFINSERT, "\""));
		break;
	}

	case LFUN_DATE_INSERT: {
		replaceSelection(bv->getLyXText());
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

		for (int i = 0; i < datetmp_len; i++)
			insertChar(datetmp[i]);

		selection.cursor = cursor;
		moveCursor(bv, false);
		break;
	}

	case LFUN_MOUSE_TRIPLE:
		if (!bv->buffer())
			break;
		if (cmd.button() == mouse_button::button1) {
			cursorHome();
			selection.cursor = cursor;
			cursorEnd();
			setSelection();
			bv->haveSelection(selection.set());
		}
		break;

	case LFUN_MOUSE_DOUBLE:
		if (!bv->buffer())
			break;
		if (cmd.button() == mouse_button::button1) {
			selectWord(lyx::WHOLE_WORD_STRICT);
			bv->haveSelection(selection.set());
		}
		break;

	case LFUN_MOUSE_MOTION: {
		// Only use motion with button 1
		//if (ev.button() != mouse_button::button1)
		//	return false;

		if (!bv->buffer())
			break;

		// Check for inset locking
#ifdef LOCK
		if (bv->innerInset()) {
			InsetOld * tli = bv->innerInset();
			LyXCursor cursor = bv->text->cursor;
			LyXFont font = bv->text->getFont(bv->text->cursorPar(), cursor.pos());
			int width = tli->width();
			int inset_x = font.isVisibleRightToLeft()
				? cursor.x() - width : cursor.x();
			int start_x = inset_x + tli->scroll();
			FuncRequest cmd1 = cmd;
			cmd1.x = cmd.x - start_x;
			cmd1.y = cmd.y - cursor.y() + bv->top_y();
			tli->dispatch(cmd1);
			break;
		}
#endif

		// The test for not selection possible is needed, that only motion
		// events are used, where the bottom press event was on
		//  the drawing area too
		if (!selection_possible) {
			lyxerr[Debug::ACTION]
				<< "BufferView::Pimpl::Dispatch: no selection possible\n";
			break;
		}

		RowList::iterator cursorrow = bv->text->cursorRow();
		bv->text->setCursorFromCoordinates(cmd.x, cmd.y + bv->top_y());
	#if 0
		// sorry for this but I have a strange error that the y value jumps at
		// a certain point. This seems like an error in my xforms library or
		// in some other local environment, but I would like to leave this here
		// for the moment until I can remove this (Jug 20020418)
		if (y_before < bv->text->cursor.y())
			lyxerr << y_before << ':'
			       << bv->text->cursor.y() << endl;
	#endif
		// This is to allow jumping over large insets
		if (cursorrow == cursorRow()) {
			if (cmd.y >= bv->workHeight())
				cursorDown(false);
			else if (cmd.y < 0)
				cursorUp(false);
		}

		bv->text->setSelection();
//		bv->update();
		break;
	}

	// Single-click on work area
	case LFUN_MOUSE_PRESS: {
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

		// Middle button press pastes if we have a selection
		// We do this here as if the selection was inside an inset
		// it could get cleared on the unlocking of the inset so
		// we have to check this first
		bool paste_internally = false;
		if (cmd.button() == mouse_button::button2 && selection.set()) {
			bv->owner()->dispatch(FuncRequest(LFUN_COPY));
			paste_internally = true;
		}

		int const screen_first = bv->top_y();
		selection_possible = true;

		// Clear the selection
		bv->text->clearSelection();
		bv->update();
		bv->updateScrollbar();

		// Right click on a footnote flag opens float menu
		if (cmd.button() == mouse_button::button3) {
			selection_possible = false;
			break;
		}

		bv->text->setCursorFromCoordinates(cmd.x, cmd.y + screen_first);
		finishUndo();
		bv->text->selection.cursor = bv->text->cursor;
		bv->x_target(bv->text->cursor.x());

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

	case LFUN_MOUSE_RELEASE: {
		if (!bv->buffer())
			break;

		// do nothing if we used the mouse wheel
		if (cmd.button() == mouse_button::button4
		 || cmd.button() == mouse_button::button5)
			return DispatchResult(true, false);

		selection_possible = false;

		if (cmd.button() == mouse_button::button2)
			break;

		// finish selection
		if (cmd.button() == mouse_button::button1)
			bv->haveSelection(selection.set());

		bv->switchKeyMap();
		bv->owner()->view_state_changed();
		bv->owner()->updateMenubar();
		bv->owner()->updateToolbar();
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
			if (selection.set())
				cutSelection(false, false);
			bv->haveSelection(false);
		}

		clearSelection();
		LyXFont const old_font = real_current_font;

		string::const_iterator cit = cmd.argument.begin();
		string::const_iterator end = cmd.argument.end();
		for (; cit != end; ++cit)
			bv->owner()->getIntl().getTransManager().
				TranslateAndInsert(*cit, this);

		selection.cursor = cursor;
		moveCursor(bv, false);

		// real_current_font.number can change so we need to
		// update the minibuffer
		if (old_font != real_current_font)
			bv->owner()->view_state_changed();
		bv->updateScrollbar();
		break;
	}

	case LFUN_URL: {
		doInsertInset(this, cmd, true, false);
		InsetCommandParams p("url");
		string const data = InsetCommandMailer::params2string("url", p);
		bv->owner()->getDialogs().show("url", data, 0);
		break;
	}

	case LFUN_HTMLURL: {
		doInsertInset(this, cmd, true, false);
		InsetCommandParams p("htmlurl");
		string const data = InsetCommandMailer::params2string("url", p);
		bv->owner()->getDialogs().show("url", data, 0);
		break;
	}


#if 0
	case LFUN_INSET_LIST:
	case LFUN_INSET_THEOREM:
	case LFUN_INSET_CAPTION:
#endif
	case LFUN_INSERT_NOTE:
	case LFUN_INSERT_CHARSTYLE:
	case LFUN_INSERT_BOX:
	case LFUN_INSERT_BRANCH:
	case LFUN_INSERT_BIBITEM:
	case LFUN_INSET_ERT:
	case LFUN_INSET_FLOAT:
	case LFUN_INSET_FOOTNOTE:
	case LFUN_INSET_MARGINAL:
	case LFUN_INSET_MINIPAGE:
	case LFUN_INSET_OPTARG:
	case LFUN_INSET_WIDE_FLOAT:
	case LFUN_INSET_WRAP:
	case LFUN_TABULAR_INSERT:
	case LFUN_ENVIRONMENT_INSERT:
		// Open the inset, and move the current selection
		// inside it.
		doInsertInset(this, cmd, true, true);
		break;

	case LFUN_INDEX_INSERT:
		// Just open the inset
		doInsertInset(this, cmd, true, false);
		break;

	case LFUN_INDEX_PRINT:
	case LFUN_TOC_INSERT:
	case LFUN_HFILL:
	case LFUN_INSERT_LINE:
	case LFUN_INSERT_PAGEBREAK:
		// do nothing fancy
		doInsertInset(this, cmd, false, false);
		break;

	case LFUN_DEPTH_MIN:
		clearSelection();
		bv_funcs::changeDepth(bv, this, bv_funcs::DEC_DEPTH, false);
		bv->update();
		break;

	case LFUN_DEPTH_PLUS:
		clearSelection();
		bv_funcs::changeDepth(bv, this, bv_funcs::INC_DEPTH, false);
		bv->update();
		break;

	case LFUN_MATH_DELIM:
	case LFUN_MATH_DISPLAY:
	case LFUN_INSERT_MATH:
	case LFUN_MATH_LIMITS:
	case LFUN_MATH_MACRO:
	case LFUN_MATH_MUTATE:
	case LFUN_MATH_SPACE:
	case LFUN_MATH_IMPORT_SELECTION:
	case LFUN_MATH_MODE:
	case LFUN_MATH_NONUMBER:
	case LFUN_MATH_NUMBER:
	case LFUN_MATH_EXTERN:
	case LFUN_MATH_SIZE:
		mathDispatch(cmd);
		break;

	case LFUN_EMPH:
		emph(bv, this);
		bv->owner()->view_state_changed();
		break;

	case LFUN_BOLD:
		bold(bv, this);
		bv->owner()->view_state_changed();
		break;

	case LFUN_NOUN:
		noun(bv, this);
		bv->owner()->view_state_changed();
		break;

	case LFUN_CODE:
		code(bv, this);
		bv->owner()->view_state_changed();
		break;

	case LFUN_SANS:
		sans(bv, this);
		bv->owner()->view_state_changed();
		break;

	case LFUN_ROMAN:
		roman(bv, this);
		bv->owner()->view_state_changed();
		break;

	case LFUN_DEFAULT:
		styleReset(bv, this);
		bv->owner()->view_state_changed();
		break;

	case LFUN_UNDERLINE:
		underline(bv, this);
		bv->owner()->view_state_changed();
		break;

	case LFUN_FONT_SIZE:
		fontSize(bv, cmd.argument, this);
		bv->owner()->view_state_changed();
		break;

	case LFUN_LANGUAGE:
		lang(bv, cmd.argument, this);
		bv->switchKeyMap();
		bv->owner()->view_state_changed();
		break;

	case LFUN_FREEFONT_APPLY:
		apply_freefont(bv, this);
		break;

	case LFUN_FREEFONT_UPDATE:
		update_and_apply_freefont(bv, this, cmd.argument);
		break;
	
	case LFUN_FINISHED_LEFT:
		lyxerr << "swallow LFUN_FINISHED_LEFT" << endl;
		if (rtl())
			cursorRight(bv);
		break;

	case LFUN_FINISHED_RIGHT:
		lyxerr << "swallow LFUN_FINISHED_RIGHT" << endl;
		if (!rtl())
			cursorRight(bv);
		break;

	case LFUN_FINISHED_UP:
		lyxerr << "swallow LFUN_FINISHED_UP" << endl;
		break;

	case LFUN_FINISHED_DOWN:
		lyxerr << "swallow LFUN_FINISHED_DOWN" << endl;
		cursorRight(bv);
		break;

	default:
		return DispatchResult(false);
	}

	return DispatchResult(true, true);
}
