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
#include "FloatList.h"
#include "funcrequest.h"
#include "gettext.h"
#include "intl.h"
#include "language.h"
#include "lyxfunc.h"
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
#include "insets/insetfloatlist.h"
#include "insets/insetnewline.h"
#include "insets/insetspecialchar.h"
#include "insets/insettext.h"

#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/tostr.h"
#include "support/std_sstream.h"

#include "mathed/math_hullinset.h"
#include "mathed/formulamacro.h"

#include <clocale>


using bv_funcs::replaceSelection;

using lyx::pos_type;

using lyx::support::isStrUnsignedInt;
using lyx::support::strToUnsignedInt;
using lyx::support::atoi;
using lyx::support::token;

using std::endl;
using std::find;
using std::string;
using std::istringstream;
using std::vector;


extern string current_layout;

// the selection possible is needed, that only motion events are
// used, where the button press event was on the drawing area too
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
			CursorSlice & cur = text->cursor();
			Paragraph & par = *text->cursorPar();
			text->bidi.computeTables(par, *bv->buffer(),
				*par.getRow(cur.pos()));
			if (cur.boundary() !=
					text->bidi.isBoundary(*bv->buffer(), par,
							cur.pos(),
							text->real_current_font))
				text->setCursor(cur.par(), cur.pos(),
						false, !cur.boundary());
		}
	}


	void moveCursor(LCursor & cur, bool selecting)
	{
		if (selecting || cur.mark())
			cur.setSelection();
		if (!cur.selection())
			cur.bv().haveSelection(false);
		cur.bv().switchKeyMap();
	}


	void finishChange(LCursor & cur, bool selecting = false)
	{
		finishUndo();
		moveCursor(cur, selecting);
		cur.bv().owner()->view_state_changed();
	}


	void mathDispatch(LCursor & cur, FuncRequest const & cmd, bool display)
	{
		string sel = cur.selectionAsString(false);
		lyxerr << "selection is: '" << sel << "'" << endl;

		if (sel.empty()) {
			cur.insert(new MathHullInset);
			cur.dispatch(FuncRequest(LFUN_RIGHT));
			cur.dispatch(FuncRequest(LFUN_MATH_MUTATE, "simple"));
			// don't do that also for LFUN_MATH_MODE unless you want end up with
			// always changing to mathrm when opening an inlined inset
			// -- I really hate "LyXfunc overloading"...
			if (display)
				cur.dispatch(FuncRequest(LFUN_MATH_DISPLAY));
			cur.dispatch(FuncRequest(LFUN_INSERT_MATH, cmd.argument));
		} else {
			// create a macro if we see "\\newcommand" somewhere, and an ordinary
			// formula otherwise
			cur.bv().getLyXText()->cutSelection(true, true);
			if (sel.find("\\newcommand") == string::npos &&
					sel.find("\\def") == string::npos)
			{
				cur.insert(new MathHullInset);
				cur.dispatch(FuncRequest(LFUN_RIGHT));
				cur.dispatch(FuncRequest(LFUN_MATH_MUTATE, "simple"));
				cur.dispatch(FuncRequest(LFUN_INSERT_MATH, sel));
			} else {
				cur.insert(new InsetFormulaMacro(sel));
				cur.dispatch(FuncRequest(LFUN_RIGHT));
			}
		}
		cur.message(N_("Math editor mode"));
	}

} // namespace anon



namespace bv_funcs {

string const freefont2string()
{
	string data;
	if (font2string(freefont, toggleall, data))
		return data;
	return string();
}

}


//takes absolute x,y coordinates
InsetBase * LyXText::checkInsetHit(int x, int y)
{
	ParagraphList::iterator pit;
	ParagraphList::iterator end;

	getParsInRange(paragraphs(),
		       bv()->top_y() - yo_,
		       bv()->top_y() - yo_ + bv()->workHeight(),
		       pit, end);

	lyxerr << "checkInsetHit: x: " << x << " y: " << y << endl;
	for ( ; pit != end; ++pit) {
		InsetList::iterator iit = pit->insetlist.begin();
		InsetList::iterator iend = pit->insetlist.end();
		for ( ; iit != iend; ++iit) {
			InsetBase * inset = iit->inset;
#if 1
			lyxerr << "examining inset " << inset
			//<< " xo/yo: " << inset->xo() << "/" << inset->yo()
				<< " xo: " << inset->xo() << "..." << inset->xo() + inset->width()
				<< " yo: " << inset->yo() - inset->ascent() << "..."
				<< inset->yo() + inset->descent() << endl;
#endif
			if (inset->covers(x, y)) {
				lyxerr << "Hit inset: " << inset << endl;
				return inset;
			}
		}
	}
	lyxerr << "No inset hit. " << endl;
	return 0;
}


bool LyXText::gotoNextInset(vector<InsetOld_code> const & codes,
			    string const & contents)
{
	ParagraphList::iterator end = paragraphs().end();
	ParagraphList::iterator pit = cursorPar();
	pos_type pos = cursor().pos();

	InsetBase * inset;
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


void LyXText::gotoInset(vector<InsetOld_code> const & codes, bool same_content)
{
	LCursor & cur = bv()->cursor();
	cur.clearSelection();

	string contents;
	if (same_content
	    && cur.pos() < cur.lastpos()
	    && cur.paragraph().isInset(cur.pos())) {
		InsetBase const * inset = cur.paragraph().getInset(cur.pos());
		if (find(codes.begin(), codes.end(), inset->lyxCode())
		    != codes.end())
			contents = static_cast<InsetCommand const *>(inset)->getContents();
	}

	if (!gotoNextInset(codes, contents)) {
		if (cur.pos() || cur.par() != 0) {
			CursorSlice tmp = cursor();
			cur.par() = 0;
			cur.pos() = 0;
			if (!gotoNextInset(codes, contents)) {
				cursor() = tmp;
				cur.bv().owner()->message(_("No more insets"));
			}
		} else {
			cur.bv().owner()->message(_("No more insets"));
		}
	}
	cur.bv().update();
	cur.resetAnchor();
}


void LyXText::gotoInset(InsetOld_code code, bool same_content)
{
	gotoInset(vector<InsetOld_code>(1, code), same_content);
}


void LyXText::cursorPrevious(LCursor & cur)
{
	pos_type cpos = cur.pos();
	lyx::paroffset_type cpar = cur.par();

	int x = cur.x_target();
	int y = bv()->top_y();
	setCursorFromCoordinates(x, y);

	if (cpar == cur.par() && cpos == cur.pos()) {
		// we have a row which is taller than the workarea. The
		// simplest solution is to move to the previous row instead.
		cursorUp(cur, true);
	}

	bv()->updateScrollbar();
	finishUndo();
}


void LyXText::cursorNext(LCursor & cur)
{
	pos_type cpos = cur.pos();
	lyx::paroffset_type cpar = cur.par();

	int x = cur.x_target();
	int y = bv()->top_y() + bv()->workHeight();
	setCursorFromCoordinates(x, y);

	if (cpar == cur.par() && cpos == cur.pos()) {
		// we have a row which is taller than the workarea. The
		// simplest solution is to move to the next row instead.
		cursorDown(cur, true);
	}

	bv()->updateScrollbar();
	finishUndo();
}


namespace {

void specialChar(LyXText * text, BufferView * bv, InsetSpecialChar::Kind kind)
{
	bv->update();
	replaceSelection(text);
	text->insertInset(new InsetSpecialChar(kind));
	bv->update();
}


void doInsertInset(LyXText * text, BufferView * bv, FuncRequest const & cmd,
	bool edit, bool pastesel)
{
	InsetBase * inset = createInset(bv, cmd);
	if (!inset)
		return;

	bool gotsel = false;
	if (bv->cursor().selection()) {
		bv->owner()->dispatch(FuncRequest(LFUN_CUT));
		gotsel = true;
	}
	text->insertInset(inset);
	if (edit)
		inset->edit(bv->cursor(), true);
	if (gotsel && pastesel)
		bv->owner()->dispatch(FuncRequest(LFUN_PASTE));
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


DispatchResult LyXText::dispatch(LCursor & cur, FuncRequest const & cmd)
{
	lyxerr[Debug::ACTION] << "LyXText::dispatch: cmd: " << cmd << endl;
	//lyxerr << "*** LyXText::dispatch: cmd: " << cmd << endl;

	BufferView * bv = &cur.bv();

	switch (cmd.action) {

	case LFUN_APPENDIX: {
		ParagraphList::iterator pit = cursorPar();
		bool start = !pit->params().startOfAppendix();

		// ensure that we have only one start_of_appendix in this document
		ParagraphList::iterator tmp = paragraphs().begin();
		ParagraphList::iterator end = paragraphs().end();

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
		bv->update();
		break;
	}

	case LFUN_DELETE_WORD_FORWARD:
		cur.clearSelection();
		deleteWordForward(cur);
		finishChange(cur);
		break;

	case LFUN_DELETE_WORD_BACKWARD:
		cur.clearSelection();
		deleteWordBackward(cur);
		finishChange(cur);
		break;

	case LFUN_DELETE_LINE_FORWARD:
		cur.clearSelection();
		deleteLineForward(cur);
		finishChange(cur);
		break;

	case LFUN_WORDRIGHT:
		if (!cur.mark())
			cur.clearSelection();
		if (rtl())
			cursorLeftOneWord(cur);
		else
			cursorRightOneWord(cur);
		finishChange(cur);
		break;

	case LFUN_WORDLEFT:
		if (!cur.mark())
			cur.clearSelection();
		if (rtl())
			cursorRightOneWord(cur);
		else
			cursorLeftOneWord(cur);
		finishChange(cur);
		break;

	case LFUN_BEGINNINGBUF:
		if (!cur.mark())
			cur.clearSelection();
		cursorTop(cur);
		finishChange(cur);
		break;

	case LFUN_ENDBUF:
		if (!cur.mark())
			cur.clearSelection();
		cursorBottom(cur);
		finishChange(cur);
		break;

	case LFUN_RIGHTSEL:
		if (!cur.selection())
			cur.resetAnchor();
		if (rtl())
			cursorLeft(cur, true);
		else
			cursorRight(cur, true);
		finishChange(cur, true);
		break;

	case LFUN_LEFTSEL:
		if (!cur.selection())
			cur.resetAnchor();
		if (rtl())
			cursorRight(cur, true);
		else
			cursorLeft(cur, true);
		finishChange(cur, true);
		break;

	case LFUN_UPSEL:
		if (!cur.selection())
			cur.resetAnchor();
		cursorUp(cur, true);
		finishChange(cur, true);
		break;

	case LFUN_DOWNSEL:
		if (!cur.selection())
			cur.resetAnchor();
		cursorDown(cur, true);
		finishChange(cur, true);
		break;

	case LFUN_UP_PARAGRAPHSEL:
		if (!cur.selection())
			cur.resetAnchor();
		cursorUpParagraph(cur);
		finishChange(cur, true);
		break;

	case LFUN_DOWN_PARAGRAPHSEL:
		if (!cur.selection())
			cur.resetAnchor();
		cursorDownParagraph(cur);
		finishChange(cur, true);
		break;

	case LFUN_PRIORSEL:
		if (!cur.selection())
			cur.resetAnchor();
		cursorPrevious(cur);
		finishChange(cur, true);
		break;

	case LFUN_NEXTSEL:
		if (!cur.selection())
			cur.resetAnchor();
		cursorNext(cur);
		finishChange(cur, true);
		break;

	case LFUN_HOMESEL:
		if (!cur.selection())
			cur.resetAnchor();
		cursorHome(cur);
		finishChange(cur, true);
		break;

	case LFUN_ENDSEL:
		if (!cur.selection())
			cur.resetAnchor();
		cursorEnd(cur);
		finishChange(cur, true);
		break;

	case LFUN_WORDRIGHTSEL:
		if (!cur.selection())
			cur.resetAnchor();
		if (rtl())
			cursorLeftOneWord(cur);
		else
			cursorRightOneWord(cur);
		finishChange(cur, true);
		break;

	case LFUN_WORDLEFTSEL:
		if (!cur.selection())
			cur.resetAnchor();
		if (rtl())
			cursorRightOneWord(cur);
		else
			cursorLeftOneWord(cur);
		finishChange(cur, true);
		break;

	case LFUN_WORDSEL: {
		selectWord(lyx::WHOLE_WORD);
		finishChange(cur, true);
		break;
	}

	case LFUN_RIGHT:
		finishChange(cur);
		return moveRight(cur);

	case LFUN_LEFT:
		finishChange(cur);
		return moveLeft(cur);

	case LFUN_UP:
		finishChange(cur);
		return moveUp(cur);

	case LFUN_DOWN:
		finishChange(cur);
		return moveDown(cur);

	case LFUN_UP_PARAGRAPH:
		if (!cur.mark())
			cur.clearSelection();
		cursorUpParagraph(cur);
		finishChange(cur);
		break;

	case LFUN_DOWN_PARAGRAPH:
		if (!cur.mark())
			cur.clearSelection();
		cursorDownParagraph(cur);
		finishChange(cur, false);
		break;

	case LFUN_PRIOR:
		if (!cur.mark())
			cur.clearSelection();
		finishChange(cur, false);
		if (cur.par() == 0 && cur.textRow().pos() == 0)
			return DispatchResult(false, FINISHED_UP);
		cursorPrevious(cur);
		break;

	case LFUN_NEXT:
		if (!cur.mark())
			cur.clearSelection();
		finishChange(cur, false);
		if (cur.par() == cur.lastpar()
			  && cur.textRow().endpos() == cur.lastpos())
			return DispatchResult(false, FINISHED_DOWN);
		cursorNext(cur);
		break;

	case LFUN_HOME:
		if (!cur.mark())
			cur.clearSelection();
		cursorHome(cur);
		finishChange(cur, false);
		break;

	case LFUN_END:
		if (!cur.mark())
			cur.clearSelection();
		cursorEnd(cur);
		finishChange(cur, false);
		break;

	case LFUN_BREAKLINE: {
		// Not allowed by LaTeX (labels or empty par)
		if (cursor().pos() > cursorPar()->beginOfBody()) {
			replaceSelection(this);
			insertInset(new InsetNewline);
			moveCursor(cur, false);
		}
		break;
	}

	case LFUN_DELETE:
		if (!cur.selection()) {
			Delete(cur);
			cur.resetAnchor();
			// It is possible to make it a lot faster still
			// just comment out the line below...
		} else {
			cutSelection(true, false);
		}
		moveCursor(cur, false);
		bv->owner()->view_state_changed();
		break;

	case LFUN_DELETE_SKIP:
		// Reverse the effect of LFUN_BREAKPARAGRAPH_SKIP.
		if (!cur.selection()) {
			if (cur.pos() == cur.lastpos()) {
				cursorRight(cur, true);
				cursorLeft(cur, true);
			}
			Delete(cur);
			cur.resetAnchor();
		} else {
			cutSelection(true, false);
		}
		bv->update();
		break;


	case LFUN_BACKSPACE:
		if (!cur.selection()) {
			if (bv->owner()->getIntl().getTransManager().backspace()) {
				backspace(cur);
				cur.resetAnchor();
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
		if (!cur.selection()) {
#warning look here
			//CursorSlice cur = cursor();
			backspace(cur);
			//anchor() = cur;
		} else {
			cutSelection(true, false);
		}
		bv->update();
		break;

	case LFUN_BREAKPARAGRAPH:
		replaceSelection(this);
		breakParagraph(cur, 0);
		bv->update();
		cur.resetAnchor();
		bv->switchKeyMap();
		bv->owner()->view_state_changed();
		break;

	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
		replaceSelection(this);
		breakParagraph(cur, 1);
		bv->update();
		cur.resetAnchor();
		bv->switchKeyMap();
		bv->owner()->view_state_changed();
		break;

	case LFUN_BREAKPARAGRAPH_SKIP: {
		// When at the beginning of a paragraph, remove
		// indentation and add a "defskip" at the top.
		// Otherwise, do the same as LFUN_BREAKPARAGRAPH.
#warning look here
		replaceSelection(this);
		if (cur.pos() == 0) {
			ParagraphParameters & params = cur.paragraph().params();
			setParagraph(
					params.spacing(),
					params.align(),
					params.labelWidthString(), 1);
		} else {
			breakParagraph(cur, 0);
		}
		bv->update();
//	anchor() = cur;
		bv->switchKeyMap();
		bv->owner()->view_state_changed();
		break;
	}

	case LFUN_PARAGRAPH_SPACING: {
		Paragraph & par = cur.paragraph();
		Spacing::Space cur_spacing = par.params().spacing().getSpace();
		float cur_value = 1.0;
		if (cur_spacing == Spacing::Other)
			cur_value = par.params().spacing().getValue();

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
			par.params().spacing(Spacing(new_spacing, new_value));
			redoParagraph();
			bv->update();
		}
		break;
	}

	case LFUN_INSET_APPLY: {
		string const name = cmd.getArg(0);
		InsetBase * inset = bv->owner()->getDialogs().getOpenInset(name);
		if (inset)
			inset->dispatch(cur, FuncRequest(LFUN_INSET_MODIFY, cmd.argument));
		else
			dispatch(cur, FuncRequest(LFUN_INSET_INSERT, cmd.argument));
		break;
	}

	case LFUN_INSET_INSERT: {
		InsetBase * inset = createInset(bv, cmd);
		if (inset)
			insertInset(inset);
		break;
	}

	case LFUN_INSET_SETTINGS:
		if (cur.inset() && cur.inset()->asUpdatableInset())
			cur.inset()->asUpdatableInset()->showInsetDialog(bv);
		break;

	case LFUN_INSET_TOGGLE:
		cur.clearSelection();
		if (!toggleInset())
			return DispatchResult(false);
		bv->update();
		bv->switchKeyMap();
		break;

	case LFUN_SPACE_INSERT:
		if (cursorPar()->layout()->free_spacing)
			insertChar(' ');
		else
			doInsertInset(this, bv, cmd, false, false);
		moveCursor(cur, false);
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
		recUndo(cursor().par());
		redoParagraph();
		bv->update();
		break;

	case LFUN_PASTE:
		cur.message(_("Paste"));
		replaceSelection(this);
#warning FIXME Check if the arg is in the domain of available selections.
		if (isStrUnsignedInt(cmd.argument))
			pasteSelection(strToUnsignedInt(cmd.argument));
		else
			pasteSelection(0);
		cur.clearSelection(); // bug 393
		bv->update();
		bv->switchKeyMap();
		finishUndo();
		break;

	case LFUN_CUT:
		cutSelection(true, true);
		cur.message(_("Cut"));
		bv->update();
		break;

	case LFUN_COPY:
		copySelection();
		cur.message(_("Copy"));
		break;

	case LFUN_BEGINNINGBUFSEL:
		if (in_inset_)
			return DispatchResult(false);
		if (!cur.selection())
			cur.resetAnchor();
		cursorTop(cur);
		finishChange(cur, true);
		break;

	case LFUN_ENDBUFSEL:
		if (in_inset_)
			return DispatchResult(false);
		if (!cur.selection())
			cur.resetAnchor();
		cursorBottom(cur);
		finishChange(cur, true);
		break;

	case LFUN_GETXY:
		cur.message(tostr(cursorX(cur.current())) + ' '
		          + tostr(cursorY(cur.current())));
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
			cur.message("E");
		else if (current_font.shape() == LyXFont::SMALLCAPS_SHAPE)
			cur.message("N");
		else
			cur.message("0");
		break;

	case LFUN_GETLAYOUT:
		cur.message(cursorPar()->layout()->name());
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
			cur.errorMessage(_("LyX function 'layout' needs an argument."));
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
			cur.errorMessage(string(N_("Layout ")) + cmd.argument +
				N_(" not known"));
			break;
		}

		bool change_layout = (current_layout != layout);

		if (!change_layout && cur.selection() &&
			cur.selBegin().par() != cur.selEnd().par())
		{
			ParagraphList::iterator spit = getPar(cur.selBegin());
			ParagraphList::iterator epit = boost::next(getPar(cur.selEnd()));
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
		cur.clearSelection();
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
		gotoInset(InsetBase::ERROR_CODE, false);
		break;

	case LFUN_GOTONOTE:
		gotoInset(InsetBase::NOTE_CODE, false);
		break;

	case LFUN_REFERENCE_GOTO: {
		vector<InsetOld_code> tmp;
		tmp.push_back(InsetBase::LABEL_CODE);
		tmp.push_back(InsetBase::REF_CODE);
		gotoInset(tmp, true);
		break;
	}

	case LFUN_QUOTE: {
		replaceSelection(this);
		ParagraphList::iterator pit = cursorPar();
		lyx::pos_type pos = cursor().pos();
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
		    pit->getFontSettings(bufparams,pos).language()->lang() == "hebrew")
		  insertInset(new InsetQuotes(c, bufparams));
		else
			bv->owner()->dispatch(FuncRequest(LFUN_SELFINSERT, "\""));
		break;
	}

	case LFUN_DATE_INSERT: {
		replaceSelection(this);
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

		cur.resetAnchor();
		moveCursor(cur, false);
		break;
	}

	case LFUN_MOUSE_TRIPLE:
		if (cmd.button() == mouse_button::button1) {
			selection_possible = true;
			cursorHome(cur);
			cur.resetAnchor();
			cursorEnd(cur);
			cur.setSelection();
			bv->haveSelection(cur.selection());
		}
		break;

	case LFUN_MOUSE_DOUBLE:
		if (cmd.button() == mouse_button::button1) {
			selection_possible = true;
			selectWord(lyx::WHOLE_WORD_STRICT);
			bv->haveSelection(cur.selection());
		}
		break;

	case LFUN_MOUSE_MOTION: {
		// Only use motion with button 1
		//if (ev.button() != mouse_button::button1)
		//	return false;
		// The test for not selection possible is needed, that
		// only motion events are used, where the bottom press
		// event was on the drawing area too
		if (!selection_possible) {
			lyxerr[Debug::ACTION] << "BufferView::Pimpl::"
				"Dispatch: no selection possible\n";
			break;
		}
		RowList::iterator cursorrow = cursorRow();

#warning
		setCursorFromCoordinates(cmd.x, cmd.y);

		// This is to allow jumping over large insets
		// FIXME: shouldn't be top-text-specific
		if (cursorrow == cursorRow() && !in_inset_) {
			if (cmd.y - bv->top_y() >= bv->workHeight())
				cursorDown(cur, true);
			else if (cmd.y - bv->top_y() < 0)
				cursorUp(cur, true);
		}

		// don't set anchor_
		bv->cursor().cursor_ = cur.cursor_;
		bv->cursor().setSelection();

		break;
	}

	// Single-click on work area
	case LFUN_MOUSE_PRESS: {
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

		// Right click on a footnote flag opens float menu
		if (cmd.button() == mouse_button::button3) {
			cur.clearSelection();
			selection_possible = false;
			break;
		}

		// Middle button press pastes if we have a selection
		// We do this here as if the selection was inside an inset
		// it could get cleared on the unlocking of the inset so
		// we have to check this first
		bool paste_internally = false;
		if (cmd.button() == mouse_button::button2 && cur.selection()) {
			bv->owner()->dispatch(FuncRequest(LFUN_COPY));
			paste_internally = true;
		}

		selection_possible = true;

		// Clear the selection
		cur.clearSelection();

		setCursorFromCoordinates(cur.current(), cmd.x - xo_,
					 cmd.y - yo_);
		cur.resetAnchor();
		finishUndo();
		cur.x_target() = cursorX(cur.current());

		// set cursor and anchor to this position
		bv->cursor() = cur;

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
		// do nothing if we used the mouse wheel
		if (cmd.button() == mouse_button::button4
		 || cmd.button() == mouse_button::button5)
			return DispatchResult(true, false);

		selection_possible = false;

		if (cmd.button() == mouse_button::button2)
			break;

		// finish selection
		if (cmd.button() == mouse_button::button1)
			bv->haveSelection(cur.selection());

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
			if (cur.selection())
				cutSelection(false, false);
			bv->haveSelection(false);
		}

		cur.clearSelection();
		LyXFont const old_font = real_current_font;

		string::const_iterator cit = cmd.argument.begin();
		string::const_iterator end = cmd.argument.end();
		for (; cit != end; ++cit)
			bv->owner()->getIntl().getTransManager().
				TranslateAndInsert(*cit, this);

		cur.resetAnchor();
		moveCursor(cur, false);

		// real_current_font.number can change so we need to
		// update the minibuffer
		if (old_font != real_current_font)
			bv->owner()->view_state_changed();
		bv->updateScrollbar();
		break;
	}

	case LFUN_URL: {
		InsetCommandParams p("url");
		string const data = InsetCommandMailer::params2string("url", p);
		bv->owner()->getDialogs().show("url", data, 0);
		break;
	}

	case LFUN_HTMLURL: {
		InsetCommandParams p("htmlurl");
		string const data = InsetCommandMailer::params2string("url", p);
		bv->owner()->getDialogs().show("url", data, 0);
		break;
	}

	case LFUN_INSERT_LABEL: {
		InsetCommandParams p("label");
		string const data = InsetCommandMailer::params2string("label", p);
		bv->owner()->getDialogs().show("label", data, 0);
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
	case LFUN_INSET_OPTARG:
	case LFUN_INSET_WIDE_FLOAT:
	case LFUN_INSET_WRAP:
	case LFUN_TABULAR_INSERT:
	case LFUN_ENVIRONMENT_INSERT:
		// Open the inset, and move the current selection
		// inside it.
		doInsertInset(this, bv, cmd, true, true);
		break;

	case LFUN_INDEX_INSERT:
		// Just open the inset
		doInsertInset(this, bv, cmd, true, false);
		break;

	case LFUN_INDEX_PRINT:
	case LFUN_TOC_INSERT:
	case LFUN_HFILL:
	case LFUN_INSERT_LINE:
	case LFUN_INSERT_PAGEBREAK:
		// do nothing fancy
		doInsertInset(this, bv, cmd, false, false);
		break;

	case LFUN_DEPTH_MIN:
		bv_funcs::changeDepth(bv, this, bv_funcs::DEC_DEPTH);
		bv->update();
		break;

	case LFUN_DEPTH_PLUS:
		bv_funcs::changeDepth(bv, this, bv_funcs::INC_DEPTH);
		bv->update();
		break;

	case LFUN_MATH_DISPLAY:
		mathDispatch(cur, cmd, true);
		break;

	case LFUN_MATH_IMPORT_SELECTION:
	case LFUN_MATH_MODE:
		mathDispatch(cur, cmd, false);
		break;

	case LFUN_MATH_MACRO:
		if (cmd.argument.empty())
			cur.errorMessage(N_("Missing argument"));
		else {
			string s = cmd.argument;
			string const s1 = token(s, ' ', 1);
			int const nargs = s1.empty() ? 0 : atoi(s1);
			string const s2 = token(s, ' ', 2);
			string const type = s2.empty() ? "newcommand" : s2;
			cur.insert(new InsetFormulaMacro(token(s, ' ', 0), nargs, s2));
			cur.nextInset()->edit(cur, true);
		}
		break;

	case LFUN_INSERT_MATH:
	case LFUN_INSERT_MATRIX:
	case LFUN_MATH_DELIM: {
		cur.insert(new MathHullInset);
		cur.dispatch(FuncRequest(LFUN_RIGHT));
		cur.dispatch(FuncRequest(LFUN_MATH_MUTATE, "simple"));
		cur.dispatch(cmd);
		break;
	}

	case LFUN_EMPH: {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setEmph(LyXFont::TOGGLE);
		toggleAndShow(bv, this, font);
		bv->owner()->view_state_changed();
		break;
	}

	case LFUN_BOLD: {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setSeries(LyXFont::BOLD_SERIES);
		toggleAndShow(bv, this, font);
		bv->owner()->view_state_changed();
		break;
	}

	case LFUN_NOUN: {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setNoun(LyXFont::TOGGLE);
		toggleAndShow(bv, this, font);
		bv->owner()->view_state_changed();
		break;
	}

	case LFUN_CODE: {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setFamily(LyXFont::TYPEWRITER_FAMILY); // no good
		toggleAndShow(bv, this, font);
		bv->owner()->view_state_changed();
		break;
	}

	case LFUN_SANS: {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setFamily(LyXFont::SANS_FAMILY);
		toggleAndShow(bv, this, font);
		bv->owner()->view_state_changed();
		break;
	}

	case LFUN_ROMAN: {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setFamily(LyXFont::ROMAN_FAMILY);
		toggleAndShow(bv, this, font);
		bv->owner()->view_state_changed();
		break;
	}

	case LFUN_DEFAULT: {
		LyXFont font(LyXFont::ALL_INHERIT, ignore_language);
		toggleAndShow(bv, this, font);
		bv->owner()->view_state_changed();
		break;
	}

	case LFUN_UNDERLINE: {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setUnderbar(LyXFont::TOGGLE);
		toggleAndShow(bv, this, font);
		bv->owner()->view_state_changed();
		break;
	}

	case LFUN_FONT_SIZE: {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLyXSize(cmd.argument);
		toggleAndShow(bv, this, font);
		bv->owner()->view_state_changed();
		break;
	}

	case LFUN_LANGUAGE: {
		Language const * lang = languages.getLanguage(cmd.argument);
		if (!lang)
			break;
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(lang);
		toggleAndShow(bv, this, font);
		bv->switchKeyMap();
		bv->owner()->view_state_changed();
		break;
	}

	case LFUN_FREEFONT_APPLY:
		toggleAndShow(bv, this, freefont, toggleall);
		bv->owner()->view_state_changed();
		bv->owner()->message(_("Character set"));
		break;

	// Set the freefont using the contents of \param data dispatched from
	// the frontends and apply it at the current cursor location.
	case LFUN_FREEFONT_UPDATE: {
		LyXFont font;
		bool toggle;
		if (bv_funcs::string2font(cmd.argument, font, toggle)) {
			freefont = font;
			toggleall = toggle;
			toggleAndShow(bv, this, freefont, toggleall);
			bv->owner()->view_state_changed();
			bv->owner()->message(_("Character set"));
		}
		break;
	}

	case LFUN_FINISHED_LEFT:
		lyxerr << "handle LFUN_FINISHED_LEFT" << endl;
		cur.pop(cur.currentDepth());
		if (rtl())
			cursorLeft(cur, true);
		cur.bv().cursor() = cur;
		break;

	case LFUN_FINISHED_RIGHT:
		lyxerr << "handle LFUN_FINISHED_RIGHT" << endl;
		cur.pop(cur.currentDepth());
		if (!rtl())
			cursorRight(cur, true);
		cur.bv().cursor() = cur;
		break;

	case LFUN_FINISHED_UP:
		lyxerr << "handle LFUN_FINISHED_UP" << endl;
		cur.pop(cur.currentDepth());
		cursorUp(cur, true);
		cur.bv().cursor() = cur;
		break;

	case LFUN_FINISHED_DOWN:
		lyxerr << "handle LFUN_FINISHED_DOWN" << endl;
		cur.pop(cur.currentDepth());
		cursorDown(cur, true);
		cur.bv().cursor() = cur;
		break;

	case LFUN_LAYOUT_PARAGRAPH: {
		string data;
		params2string(cur.paragraph(), data);
		data = "show\n" + data;
		bv->owner()->getDialogs().show("paragraph", data);
		break;
	}

	case LFUN_PARAGRAPH_UPDATE: {
		if (!bv->owner()->getDialogs().visible("paragraph"))
			break;
		string data;
		params2string(cur.paragraph(), data);

		// Will the paragraph accept changes from the dialog?
		InsetBase * const inset = cur.inset();
		bool const accept =
			!(inset && inset->forceDefaultParagraphs(inset));

		data = "update " + tostr(accept) + '\n' + data;
		bv->owner()->getDialogs().update("paragraph", data);
		break;
	}

	case LFUN_UMLAUT:
	case LFUN_CIRCUMFLEX:
	case LFUN_GRAVE:
	case LFUN_ACUTE:
	case LFUN_TILDE:
	case LFUN_CEDILLA:
	case LFUN_MACRON:
	case LFUN_DOT:
	case LFUN_UNDERDOT:
	case LFUN_UNDERBAR:
	case LFUN_CARON:
	case LFUN_SPECIAL_CARON:
	case LFUN_BREVE:
	case LFUN_TIE:
	case LFUN_HUNG_UMLAUT:
	case LFUN_CIRCLE:
	case LFUN_OGONEK:
		bv->owner()->getLyXFunc().handleKeyFunc(cmd.action);
		if (!cmd.argument.empty())
			bv->owner()->getIntl().getTransManager()
				.TranslateAndInsert(cmd.argument[0], this);
		break;

	case LFUN_FLOAT_LIST: {
		LyXTextClass const & tclass = bv->buffer()->params().getLyXTextClass();
		if (tclass.floats().typeExist(cmd.argument)) {
			// not quite sure if we want this...
			recordUndo(cur);
			freezeUndo();
			cur.clearSelection();
			breakParagraph(cur);

			if (cur.lastpos() != 0) {
				cursorLeft(cur, true);
				breakParagraph(cur);
			}

			setLayout(tclass.defaultLayoutName());
			setParagraph(Spacing(), LYX_ALIGN_LAYOUT, string(), 0);
			insertInset(new InsetFloatList(cmd.argument));
			unFreezeUndo();
		} else {
			lyxerr << "Non-existent float type: "
			       << cmd.argument << endl;
		}
		break;
	}

	default:
		return DispatchResult(false);
	}

	return DispatchResult(true, true);
}
