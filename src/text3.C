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
#include "paragraph.h"
#include "BufferView.h"
#include "funcrequest.h"
#include "lyxrc.h"
#include "Lsstream.h"
#include "debug.h"
#include "bufferparams.h"
#include "buffer.h"
#include "bufferview_funcs.h"
#include "ParagraphParameters.h"
#include "gettext.h"
#include "factory.h"
#include "intl.h"
#include "box.h"
#include "language.h"
#include "support/tostr.h"
#include "support/lstrings.h"
#include "support/LAssert.h"
#include "frontends/LyXView.h"
#include "frontends/screen.h"
#include "frontends/Dialogs.h"
#include "insets/insetspecialchar.h"
#include "insets/insettext.h"
#include "insets/insetquotes.h"
#include "insets/insetcommand.h"
#include "insets/insetnewline.h"
#include "undo_funcs.h"
#include "text_funcs.h"
#include "Lsstream.h"

#include <ctime>
#include <clocale>

using namespace lyx::support;
using namespace bv_funcs;

using std::endl;
using std::find;
using std::vector;
using lyx::pos_type;

extern string current_layout;
extern int bibitemMaxWidth(BufferView *, LyXFont const &);

// the selection possible is needed, that only motion events are
// used, where the bottom press event was on the drawing area too
bool selection_possible = false;


namespace {

	void moveCursorUpdate(BufferView * bv, bool selecting)
	{
		LyXText * lt = bv->getLyXText();

		//if (!lt->selection.set())
    //	lt->selection.cursor = lt->cursor;

		if (selecting || lt->selection.mark())
			lt->setSelection();

		if (!lt->selection.set())
			bv->haveSelection(false);

		bv->update();
		bv->switchKeyMap();
	}


	void finishChange(BufferView * bv, bool selecting = false)
	{
		finishUndo();
		moveCursorUpdate(bv, selecting);
		bv->owner()->view_state_changed();
	}

	// check if the given co-ordinates are inside an inset at the
	// given cursor, if one exists. If so, the inset is returned,
	// and the co-ordinates are made relative. Otherwise, 0 is returned.
	InsetOld * checkInset(BufferView * /*bv*/, LyXText & text,
		LyXCursor const & cur, int & x, int & y)
	{
		lyx::pos_type const pos = cur.pos();
		ParagraphList::iterator par = cur.par();

		if (pos >= par->size() || !par->isInset(pos))
			return 0;

		InsetOld /*const*/ * inset = par->getInset(pos);

		if (!isEditableInset(inset))
			return 0;

		// get inset dimensions
		Assert(par->getInset(pos));

		LyXFont const & font = text.getFont(par, pos);

		int const width = inset->width();
		int const inset_x = font.isVisibleRightToLeft()
			? (cur.ix() - width) : cur.ix();

		Box b(
			inset_x + inset->scroll(),
			inset_x + width,
			cur.iy() - inset->ascent(),
			cur.iy() + inset->descent()
		);

		if (!b.contained(x, y)) {
			lyxerr[Debug::GUI] << "Missed inset at x,y "
					   << x << ',' << y
					   << " box " << b << endl;
			return 0;
		}

		text.setCursor(par, pos, true);

		x -= b.x1;
		// The origin of an inset is on the baseline
		y -= text.cursor.iy();

		return inset;
	}

} // anon namespace


InsetOld * LyXText::checkInsetHit(int & x, int & y)
{
	int y_tmp = y + top_y();

	LyXCursor cur;
	setCursorFromCoordinates(cur, x, y_tmp);

	InsetOld * inset = checkInset(bv(), *this, cur, x, y_tmp);
	if (inset) {
		y = y_tmp;
		return inset;
	}

	// look at previous position
	if (cur.pos() == 0)
		return 0;

	// move back one
	setCursor(cur, cur.par(), cur.pos() - 1, true);

	inset = checkInset(bv(), *this, cur, x, y_tmp);
	if (inset)
		y = y_tmp;
	return inset;
}


bool LyXText::gotoNextInset(vector<InsetOld::Code> const & codes,
			    string const & contents)
{
	ParagraphList::iterator end = ownerParagraphs().end();
	ParagraphList::iterator pit = cursor.par();
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

	if (pit != end) {
		setCursor(pit, pos, false);
		return true;
	}
	return false;
}


void LyXText::gotoInset(vector<InsetOld::Code> const & codes,
			bool same_content)
{
	bv()->beforeChange(this);

	string contents;
	if (same_content && cursor.pos() < cursor.par()->size()
	    && cursor.par()->isInset(cursor.pos())) {
		InsetOld const * inset = cursor.par()->getInset(cursor.pos());
		if (find(codes.begin(), codes.end(), inset->lyxCode())
		    != codes.end())
			contents = static_cast<InsetCommand const *>(inset)->getContents();
	}

	if (!gotoNextInset(codes, contents)) {
		if (cursor.pos() || cursor.par() != ownerParagraphs().begin()) {
			LyXCursor tmp = cursor;
			cursor.par(ownerParagraphs().begin());
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
	int y = top_y();

	RowList::iterator rit = cursorRow();

	if (rit == firstRow()) {
		if (y > 0)
			bv()->updateScrollbar();
		return;
	}

	setCursorFromCoordinates(cursor.x_fix(), y);
	finishUndo();

	int new_y;
	if (rit == bv()->text->cursorRow()) {
		// we have a row which is taller than the workarea. The
		// simplest solution is to move to the previous row instead.
		cursorUp(true);
		return;
		// This is what we used to do, so we wouldn't skip right past
		// tall rows, but it's not working right now.
	} else {
		if (inset_owner) {
			new_y = bv()->text->cursor.iy()
				+ bv()->theLockingInset()->insetInInsetY() + y
				+ rit->height()
				- bv()->workHeight() + 1;
		} else {
			new_y = cursor.y()
				- rit->baseline()
				+ rit->height()
				- bv()->workHeight() + 1;
		}
	}

	LyXCursor cur;
	ParagraphList::iterator pit = cursor.par();
	previousRow(pit, rit);
	setCursor(cur, pit, rit->pos(), false);
	if (cur.y() > top_y())
		cursorUp(true);
	bv()->updateScrollbar();
}


void LyXText::cursorNext()
{
	int topy = top_y();

	RowList::iterator rit = cursorRow();
	if (rit == lastRow()) {
		int y = cursor.y() - rit->baseline() + cursorRow()->height();
		if (y > topy + bv()->workHeight())
			bv()->updateScrollbar();
		return;
	}

	int y = topy + bv()->workHeight();
	if (inset_owner && !topy) {
		y -= (bv()->text->cursor.iy()
			  - bv()->text->top_y()
			  + bv()->theLockingInset()->insetInInsetY());
	}

	ParagraphList::iterator dummypit;
	getRowNearY(y, dummypit);

	setCursorFromCoordinates(cursor.x_fix(), y);
	// + bv->workHeight());
	finishUndo();

	int new_y;
	if (rit == bv()->text->cursorRow()) {
		// we have a row which is taller than the workarea. The
		// simplest solution is to move to the next row instead.
		cursorDown(true);
		return;
		// This is what we used to do, so we wouldn't skip right past
		// tall rows, but it's not working right now.
#if 0
		new_y = bv->text->top_y() + bv->workHeight();
#endif
	} else {
		if (inset_owner) {
			new_y = bv()->text->cursor.iy()
				+ bv()->theLockingInset()->insetInInsetY()
				+ y - rit->baseline();
		} else {
			new_y = cursor.y() - cursorRow()->baseline();
		}
	}

	ParagraphList::iterator pit = cursor.par();
	nextRow(pit, rit);	
	LyXCursor cur;
	setCursor(cur, pit, rit->pos(), false);
	if (cur.y() < top_y() + bv()->workHeight())
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
		bv->updateInset();
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
			if (edit) {
				FuncRequest cmd(bv, LFUN_INSET_EDIT, "left");
				inset->localDispatch(cmd);
			}
			if (gotsel && pastesel)
				bv->owner()->dispatch(FuncRequest(LFUN_PASTE));
		}
		else
			delete inset;
	}
}

}

InsetOld::RESULT LyXText::dispatch(FuncRequest const & cmd)
{
	lyxerr[Debug::ACTION] << "LyXFunc::dispatch: action[" << cmd.action
			      <<"] arg[" << cmd.argument << ']' << endl;

	BufferView * bv = cmd.view();

	switch (cmd.action) {

	case LFUN_APPENDIX: {
		ParagraphList::iterator pit = cursor.par();
		bool start = !pit->params().startOfAppendix();

		// ensure that we have only one start_of_appendix in this document
		ParagraphList::iterator tmp = ownerParagraphs().begin();
		ParagraphList::iterator end = ownerParagraphs().end();

		for (; tmp != end; ++tmp) {
			if (tmp->params().startOfAppendix()) {
				recordUndo(bv, Undo::ATOMIC, tmp);
				tmp->params().startOfAppendix(false);
				setHeightOfRow(tmp, getRow(tmp, 0));
				break;
			}
		}

		recordUndo(bv, Undo::ATOMIC, pit);
		pit->params().startOfAppendix(start);

		// we can set the refreshing parameters now
		updateCounters();
		redoParagraph(cursor.par());
		setCursor(cursor.par(), cursor.pos());
		bv->update();
		break;
	}

	case LFUN_DELETE_WORD_FORWARD:
		bv->beforeChange(this);
		deleteWordForward();
		finishChange(bv);
		break;

	case LFUN_DELETE_WORD_BACKWARD:
		bv->beforeChange(this);
		deleteWordBackward();
		finishChange(bv);
		break;

	case LFUN_DELETE_LINE_FORWARD:
		bv->beforeChange(this);
		deleteLineForward();
		finishChange(bv);
		break;

	case LFUN_WORDRIGHT:
		if (!selection.mark())
			bv->beforeChange(this);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorLeftOneWord();
		else
			cursorRightOneWord();
		finishChange(bv);
		break;

	case LFUN_WORDLEFT:
		if (!selection.mark())
			bv->beforeChange(this);
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorRightOneWord();
		else
			cursorLeftOneWord();
		finishChange(bv);
		break;

	case LFUN_BEGINNINGBUF:
		if (!selection.mark())
			bv->beforeChange(this);
		cursorTop();
		finishChange(bv);
		break;

	case LFUN_ENDBUF:
		if (selection.mark())
			bv->beforeChange(this);
		cursorBottom();
		finishChange(bv);
		break;

	case LFUN_RIGHTSEL:
		if (!selection.set())
			selection.cursor = cursor;
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorLeft(bv);
		else
			cursorRight(bv);
		finishChange(bv, true);
		break;

	case LFUN_LEFTSEL:
		if (!selection.set())
			selection.cursor = cursor;
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
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
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorLeftOneWord();
		else
			cursorRightOneWord();
		finishChange(bv, true);
		break;

	case LFUN_WORDLEFTSEL:
		if (cursor.par()->isRightToLeftPar(bv->buffer()->params))
			cursorRightOneWord();
		else
			cursorLeftOneWord();
		finishChange(bv, true);
		break;

	case LFUN_WORDSEL: {
		LyXCursor cur1 = cursor;
		LyXCursor cur2;
		::getWord(cur1, cur2, lyx::WHOLE_WORD, ownerParagraphs());
		setCursor(cur1.par(), cur1.pos());
		bv->beforeChange(this);
		setCursor(cur2.par(), cur2.pos());
		finishChange(bv, true);
		break;
	}

	case LFUN_RIGHT: {
		bool is_rtl = cursor.par()->isRightToLeftPar(bv->buffer()->params);
		if (!selection.mark())
			bv->beforeChange(this);
		if (is_rtl)
			cursorLeft(false);
		if (cursor.pos() < cursor.par()->size()
		    && cursor.par()->isInset(cursor.pos())
		    && isHighlyEditableInset(cursor.par()->getInset(cursor.pos()))) {
			InsetOld * tmpinset = cursor.par()->getInset(cursor.pos());
			cmd.message(tmpinset->editMessage());
			FuncRequest cmd1(bv, LFUN_INSET_EDIT, is_rtl ? "right" : "left");
			tmpinset->localDispatch(cmd1);
			break;
		}
		if (!is_rtl)
			cursorRight(false);
		finishChange(bv);
		break;
	}

	case LFUN_LEFT: {
		// This is soooo ugly. Isn`t it possible to make
		// it simpler? (Lgb)
		bool const is_rtl = cursor.par()->isRightToLeftPar(bv->buffer()->params);
		if (!selection.mark())
			bv->beforeChange(this);
		LyXCursor const cur = cursor;
		if (!is_rtl)
			cursorLeft(false);
		if ((is_rtl || cur != cursor) && // only if really moved!
		    cursor.pos() < cursor.par()->size() &&
		    cursor.par()->isInset(cursor.pos()) &&
		    isHighlyEditableInset(cursor.par()->getInset(cursor.pos()))) {
			InsetOld * tmpinset = cursor.par()->getInset(cursor.pos());
			cmd.message(tmpinset->editMessage());
			FuncRequest cmd1(bv, LFUN_INSET_EDIT, is_rtl ? "left" : "right");
			tmpinset->localDispatch(cmd1);
			break;
		}
		if (is_rtl)
			cursorRight(false);
		finishChange(bv);
		break;
	}

	case LFUN_UP:
		if (!selection.mark())
			bv->beforeChange(this);
		cursorUp(false);
		finishChange(bv);
		break;

	case LFUN_DOWN:
		if (!selection.mark())
			bv->beforeChange(this);
		cursorDown(false);
		finishChange(bv);
		break;

	case LFUN_UP_PARAGRAPH:
		if (!selection.mark())
			bv->beforeChange(this);
		cursorUpParagraph();
		finishChange(bv);
		break;

	case LFUN_DOWN_PARAGRAPH:
		if (!selection.mark())
			bv->beforeChange(this);
		cursorDownParagraph();
		finishChange(bv, false);
		break;

	case LFUN_PRIOR:
		if (!selection.mark())
			bv->beforeChange(this);
		cursorPrevious();
		finishChange(bv, false);
		break;

	case LFUN_NEXT:
		if (!selection.mark())
			bv->beforeChange(this);
		cursorNext();
		finishChange(bv, false);
		break;

	case LFUN_HOME:
		if (!selection.mark())
			bv->beforeChange(this);
		cursorHome();
		finishChange(bv, false);
		break;

	case LFUN_END:
		if (!selection.mark())
			bv->beforeChange(this);
		cursorEnd();
		finishChange(bv, false);
		break;

	case LFUN_BREAKLINE: {
		lyx::pos_type body = cursor.par()->beginningOfBody();

		// Not allowed by LaTeX (labels or empty par)
		if (cursor.pos() <= body)
			break;

		replaceSelection(bv->getLyXText());
		insertInset(new InsetNewline);
		setCursor(cursor.par(), cursor.pos());
		moveCursorUpdate(bv, false);
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
		moveCursorUpdate(bv, false);
		bv->owner()->view_state_changed();
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
					setParagraph(
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
			LyXCursor cur = cursor;
			if (cur.pos() == 0
			    && !(cur.par()->params().spaceTop() == VSpace(VSpace::NONE))) {
				setParagraph(
					 cur.par()->params().lineTop(),
					 cur.par()->params().lineBottom(),
					 cur.par()->params().pagebreakTop(),
					 cur.par()->params().pagebreakBottom(),
					 VSpace(VSpace::NONE),
				   cur.par()->params().spaceBottom(),
					 cur.par()->params().spacing(),
					 cur.par()->params().align(),
					 cur.par()->params().labelWidthString(), 0);
			} else {
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
		breakParagraph(bv->buffer()->paragraphs, 0);
		bv->update();
		selection.cursor = cursor;
		bv->switchKeyMap();
		bv->owner()->view_state_changed();
		break;

	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
		replaceSelection(bv->getLyXText());
		breakParagraph(bv->buffer()->paragraphs, 1);
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
			if (cur.par()->params().spaceTop() == VSpace(VSpace::NONE)) {
				setParagraph(
					 cur.par()->params().lineTop(),
					 cur.par()->params().lineBottom(),
					 cur.par()->params().pagebreakTop(),
					 cur.par()->params().pagebreakBottom(),
					 VSpace(VSpace::DEFSKIP), cur.par()->params().spaceBottom(),
					 cur.par()->params().spacing(),
					 cur.par()->params().align(),
					 cur.par()->params().labelWidthString(), 1);
			}
		}
		else {
			breakParagraph(bv->buffer()->paragraphs, 0);
		}
		bv->update();
		selection.cursor = cur;
		bv->switchKeyMap();
		bv->owner()->view_state_changed();
		break;
	}

	case LFUN_PARAGRAPH_SPACING: {
		ParagraphList::iterator pit = cursor.par();
		Spacing::Space cur_spacing = pit->params().spacing().getSpace();
		float cur_value = 1.0;
		if (cur_spacing == Spacing::Other)
			cur_value = pit->params().spacing().getValue();

		istringstream is(STRCONV(cmd.argument));
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
		Assert(bv->theLockingInset());
		bv->theLockingInset()->getLockingInset()->showInsetDialog(bv);
		break;

	case LFUN_INSET_TOGGLE:
		bv->beforeChange(this);
		toggleInset();
		bv->update();
		bv->switchKeyMap();
		break;

	case LFUN_SPACE_INSERT:
		if (cursor.par()->layout()->free_spacing)
			insertChar(' ');
		else
			doInsertInset(this, cmd, false, false);
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

	case LFUN_END_OF_SENTENCE:
		specialChar(this, bv, InsetSpecialChar::END_OF_SENTENCE);
		break;

	case LFUN_MENU_SEPARATOR:
		specialChar(this, bv, InsetSpecialChar::MENU_SEPARATOR);
		break;

	case LFUN_MARK_OFF:
		bv->beforeChange(this);
		bv->update();
		selection.cursor = cursor;
		cmd.message(N_("Mark off"));
		break;

	case LFUN_MARK_ON:
		bv->beforeChange(this);
		selection.mark(true);
		bv->update();
		selection.cursor = cursor;
		cmd.message(N_("Mark on"));
		break;

	case LFUN_SETMARK:
		bv->beforeChange(this);
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
		recordUndo(bv, Undo::ATOMIC, cursor.par());
		redoParagraph();
		bv->update();
		break;

	case LFUN_PASTE: {
		cmd.message(_("Paste"));
		replaceSelection(bv->getLyXText());
		size_t sel_index = 0;
		string const & arg = cmd.argument;
		if (isStrUnsignedInt(arg)) {
			size_t const paste_arg = strToUnsignedInt(arg);
#warning FIXME Check if the arg is in the domain of available selections.
			sel_index = paste_arg;
		}
		pasteSelection(sel_index);
		clearSelection(); // bug 393
		bv->update();
		bv->switchKeyMap();
		break;
	}

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
			return UNDISPATCHED;
		cursorTop();
		finishChange(bv, true);
		break;

	case LFUN_ENDBUFSEL:
		if (inset_owner)
			return UNDISPATCHED;
		cursorBottom();
		finishChange(bv, true);
		break;

	case LFUN_GETXY:
		cmd.message(tostr(cursor.x()) + ' ' + tostr(cursor.y()));
		break;

	case LFUN_SETXY: {
		int x = 0;
		int y = 0;
		istringstream is(STRCONV(cmd.argument));
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
			ParagraphList::iterator spit = selection.start.par();
			ParagraphList::iterator epit = boost::next(selection.end.par());
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
		// this was originally a beforeChange(bv->text), i.e
		// the outermost LyXText!
		bv->beforeChange(this);
		string const clip = bv->getClipboard();
		if (!clip.empty()) {
			if (cmd.argument == "paragraph")
				insertStringAsParagraphs(clip);
			else
				insertStringAsLines(clip);
			clearSelection();
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

	case LFUN_REFERENCE_GOTO:
	{
		vector<InsetOld::Code> tmp;
		tmp.push_back(InsetOld::LABEL_CODE);
		tmp.push_back(InsetOld::REF_CODE);
		gotoInset(tmp, true);
		break;
	}

	case LFUN_QUOTE: {
		replaceSelection(bv->getLyXText());
		ParagraphList::iterator pit = cursor.par();
		lyx::pos_type pos = cursor.pos();
		char c;
		if (!pos)
			c = ' ';
		else if (pit->isInset(pos - 1) && pit->getInset(pos - 1)->isSpace())
			c = ' ';
		else
			c = pit->getChar(pos - 1);

		LyXLayout_ptr const & style = pit->layout();

		if (style->pass_thru ||
				pit->getFontSettings(bv->buffer()->params,
					 pos).language()->lang() == "hebrew" ||
			(!bv->insertInset(new InsetQuotes(c, bv->buffer()->params))))
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
		moveCursorUpdate(bv, false);
		break;
	}

	case LFUN_MOUSE_TRIPLE:
		if (!bv->buffer())
			break;
		if (!isInInset() && bv->theLockingInset())
			break;
		if (cmd.button() == mouse_button::button1) {
			cursorHome();
			selection.cursor = cursor;
			cursorEnd();
			setSelection();
			bv->update();
			bv->haveSelection(selection.set());
		}
		break;

	case LFUN_MOUSE_DOUBLE:
		if (!bv->buffer())
			break;
		if (!isInInset() && bv->theLockingInset())
			break;
		if (cmd.button() == mouse_button::button1) {
			selectWord(lyx::WHOLE_WORD_STRICT);
			bv->update();
			bv->haveSelection(selection.set());
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
			InsetOld * tli = bv->theLockingInset();
			LyXCursor cursor = bv->text->cursor;
			LyXFont font = bv->text->getFont(cursor.par(), cursor.pos());
			int width = tli->width();
			int inset_x = font.isVisibleRightToLeft()
				? cursor.ix() - width : cursor.ix();
			int start_x = inset_x + tli->scroll();
			FuncRequest cmd1 = cmd;
			cmd1.x = cmd.x - start_x;
			cmd1.y = cmd.y - cursor.iy() + bv->text->top_y();
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

		RowList::iterator cursorrow = bv->text->cursorRow();
		bv->text->setCursorFromCoordinates(cmd.x, cmd.y + bv->text->top_y());
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
		if (cursorrow == bv->text->cursorRow()) {
			if (cmd.y >= bv->workHeight())
				bv->text->cursorDown(false);
			else if (cmd.y < 0)
				bv->text->cursorUp(false);
		}

		bv->text->setSelection();
		bv->update();
		bv->fitCursor();
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
		InsetOld * inset_hit = bv->text->checkInsetHit(x, y);

		// Middle button press pastes if we have a selection
		// We do this here as if the selection was inside an inset
		// it could get cleared on the unlocking of the inset so
		// we have to check this first
		bool paste_internally = false;
		if (cmd.button() == mouse_button::button2 && selection.set()) {
			bv->owner()->dispatch(FuncRequest(LFUN_COPY));
			paste_internally = true;
		}

		int const screen_first = bv->text->top_y();

		if (bv->theLockingInset()) {
			// We are in inset locking mode

			// Check whether the inset was hit. If not reset mode,
			// otherwise give the event to the inset
			if (inset_hit == bv->theLockingInset()) {
				FuncRequest cmd1(bv, LFUN_MOUSE_PRESS, x, y, cmd.button());
				bv->theLockingInset()->localDispatch(cmd1);
				break;
			}
			bv->unlockInset(bv->theLockingInset());
		}

		if (!inset_hit)
			selection_possible = true;

		// Clear the selection
		bv->text->clearSelection();
		bv->update();
		bv->updateScrollbar();

		// Single left click in math inset?
		if (isHighlyEditableInset(inset_hit)) {
			// Highly editable inset, like math
			UpdatableInset * inset = static_cast<UpdatableInset *>(inset_hit);
			selection_possible = false;
			bv->owner()->message(inset->editMessage());
			// We just have to lock the inset before calling a PressEvent on it!
			if (!bv->lockInset(inset))
				lyxerr[Debug::INSETS] << "Cannot lock inset" << endl;
			FuncRequest cmd1(bv, LFUN_MOUSE_PRESS, x, y, cmd.button());
			inset->localDispatch(cmd1);
			break;
		}
		// I'm not sure we should continue here if we hit an inset (Jug20020403)

		// Right click on a footnote flag opens float menu
		if (cmd.button() == mouse_button::button3) {
			selection_possible = false;
			break;
		}

		if (!inset_hit) // otherwise it was already set in checkInsetHit(...)
			bv->text->setCursorFromCoordinates(x, y + screen_first);
		finishUndo();
		bv->text->selection.cursor = bv->text->cursor;
		bv->text->cursor.x_fix(bv->text->cursor.x());

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
		InsetOld * inset_hit = bv->text->checkInsetHit(x, y);

		if (bv->theLockingInset()) {
			// We are in inset locking mode.

			// LyX does a kind of work-area grabbing for insets.
			// Only a ButtonPress FuncRequest outside the inset will
			// force a insetUnlock.
			FuncRequest cmd1(bv, LFUN_MOUSE_RELEASE, x, y, cmd.button());
			bv->theLockingInset()->localDispatch(cmd1);
			break;
		}

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
			if (inset_hit->lyxCode() == InsetOld::REF_CODE)
				recordUndo(bv, Undo::ATOMIC);

			bv->owner()->message(inset_hit->editMessage());

			FuncRequest cmd1(bv, LFUN_MOUSE_RELEASE, x, y, cmd.button());
			inset_hit->localDispatch(cmd1);
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
			if (selection.set())
				cutSelection(false, false);
			bv->haveSelection(false);
		}

		bv->beforeChange(this);
		LyXFont const old_font(real_current_font);

		string::const_iterator cit = cmd.argument.begin();
		string::const_iterator end = cmd.argument.end();
		for (; cit != end; ++cit)
			bv->owner()->getIntl().getTransManager().
				TranslateAndInsert(*cit, this);

		selection.cursor = cursor;
		moveCursorUpdate(bv, false);

		// real_current_font.number can change so we need to
		// update the minibuffer
		if (old_font != real_current_font)
			bv->owner()->view_state_changed();
		break;
	}

	case LFUN_HTMLURL: {
		InsetCommandParams p("htmlurl");
		string const data = InsetCommandMailer::params2string("url", p);
		bv->owner()->getDialogs().show("url", data, 0);
		break;
	}

	case LFUN_URL: {
		InsetCommandParams p("url");
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
		// do nothing fancy
		doInsertInset(this, cmd, false, false);
		break;

	default:
		return UNDISPATCHED;
	}

	return DISPATCHED;
}
