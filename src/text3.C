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
#include "lyxlex.h"
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
#include "insets/insetquotes.h"
#include "insets/insetspecialchar.h"
#include "insets/insettext.h"

#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/tostr.h"
#include "support/std_sstream.h"

#include "mathed/math_hullinset.h"
#include "mathed/formulamacro.h"

#include <clocale>

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


	void toggleAndShow(LCursor & cur, LyXText * text,
		LyXFont const & font, bool toggleall = true)
	{
		text->toggleFree(cur, font, toggleall);

		if (font.language() != ignore_language ||
				font.number() != LyXFont::IGNORE) {
			Paragraph & par = cur.paragraph();
			text->bidi.computeTables(par, *cur.bv().buffer(), cur.textRow());
			if (cur.boundary() !=
					text->bidi.isBoundary(*cur.bv().buffer(), par,
							cur.pos(),
							text->real_current_font))
				text->setCursor(cur, cur.par(), cur.pos(),
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


	void finishChange(LCursor & cur, bool selecting)
	{
		finishUndo();
		moveCursor(cur, selecting);
	}


	void mathDispatch(LCursor & cur, LyXText * text,
		FuncRequest const & cmd, bool display)
	{
		recordUndo(cur);
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
			text->cutSelection(cur, true, true);
			if (sel.find("\\newcommand") == string::npos
			    && sel.find("\\def") == string::npos)
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


bool LyXText::gotoNextInset(LCursor & cur,
	vector<InsetOld_code> const & codes, string const & contents)
{
	BOOST_ASSERT(this == cur.text());
	ParagraphList::iterator end = paragraphs().end();
	ParagraphList::iterator pit = getPar(cur.par());
	pos_type pos = cur.pos();

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

	setCursor(cur, parOffset(pit), pos, false);
	return true;
}


void LyXText::gotoInset(LCursor & cur,
	vector<InsetOld_code> const & codes, bool same_content)
{
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

	if (!gotoNextInset(cur, codes, contents)) {
		if (cur.pos() || cur.par() != 0) {
			CursorSlice tmp = cur.top();
			cur.par() = 0;
			cur.pos() = 0;
			if (!gotoNextInset(cur, codes, contents)) {
				cur.top() = tmp;
				cur.message(_("No more insets"));
			}
		} else {
			cur.message(_("No more insets"));
		}
	}
	cur.update();
	cur.resetAnchor();
}


void LyXText::gotoInset(LCursor & cur, InsetOld_code code, bool same_content)
{
	gotoInset(cur, vector<InsetOld_code>(1, code), same_content);
}


void LyXText::cursorPrevious(LCursor & cur)
{
	pos_type cpos = cur.pos();
	lyx::paroffset_type cpar = cur.par();

	int x = cur.x_target();
	int y = bv()->top_y();
	setCursorFromCoordinates(cur, x, y);

	if (cpar == cur.par() && cpos == cur.pos()) {
		// we have a row which is taller than the workarea. The
		// simplest solution is to move to the previous row instead.
		cursorUp(cur);
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
	setCursorFromCoordinates(cur, x, y);

	if (cpar == cur.par() && cpos == cur.pos()) {
		// we have a row which is taller than the workarea. The
		// simplest solution is to move to the next row instead.
		cursorDown(cur);
	}

	bv()->updateScrollbar();
	finishUndo();
}


namespace {

void specialChar(LCursor & cur, LyXText * text, InsetSpecialChar::Kind kind)
{
	text->replaceSelection(cur);
	cur.insert(new InsetSpecialChar(kind));
	cur.update();
}


void doInsertInset(LCursor & cur, LyXText * text,
	FuncRequest const & cmd, bool edit, bool pastesel)
{
	InsetBase * inset = createInset(&cur.bv(), cmd);
	if (!inset)
		return;

	recordUndo(cur);
	bool gotsel = false;
	if (cur.selection()) {
		cur.bv().owner()->dispatch(FuncRequest(LFUN_CUT));
		gotsel = true;
	}
	text->insertInset(cur, inset);
	if (edit)
		inset->edit(cur, true);
	if (gotsel && pastesel)
		cur.bv().owner()->dispatch(FuncRequest(LFUN_PASTE));
}

} // anon namespace


void LyXText::number(LCursor & cur)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setNumber(LyXFont::TOGGLE);
	toggleAndShow(cur, this, font);
}


bool LyXText::isRTL(Paragraph const & par) const
{
	return par.isRightToLeftPar(bv()->buffer()->params());
}


void LyXText::dispatch(LCursor & cur, FuncRequest & cmd)
{
	lyxerr[Debug::ACTION] << "LyXText::dispatch: cmd: " << cmd << endl;
	//lyxerr << "*** LyXText::dispatch: cmd: " << cmd << endl;

	BufferView * bv = &cur.bv();
	CursorSlice sl = cur.top();

	switch (cmd.action) {

	case LFUN_APPENDIX: {
		Paragraph & par = cur.paragraph();
		bool start = !par.params().startOfAppendix();

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

		recordUndo(cur);
		par.params().startOfAppendix(start);

		// we can set the refreshing parameters now
		updateCounters();
		redoParagraph(cur);
		cur.update();
		break;
	}

	case LFUN_DELETE_WORD_FORWARD:
		cur.clearSelection();
		deleteWordForward(cur);
		finishChange(cur, false);
		break;

	case LFUN_DELETE_WORD_BACKWARD:
		cur.clearSelection();
		deleteWordBackward(cur);
		finishChange(cur, false);
		break;

	case LFUN_DELETE_LINE_FORWARD:
		cur.clearSelection();
		deleteLineForward(cur);
		finishChange(cur, false);
		break;

	case LFUN_WORDRIGHT:
		if (!cur.mark())
			cur.clearSelection();
		if (isRTL(cur.paragraph()))
			cursorLeftOneWord(cur);
		else
			cursorRightOneWord(cur);
		finishChange(cur, false);
		break;

	case LFUN_WORDLEFT:
		if (!cur.mark())
			cur.clearSelection();
		if (isRTL(cur.paragraph()))
			cursorRightOneWord(cur);
		else
			cursorLeftOneWord(cur);
		finishChange(cur, false);
		break;

	case LFUN_BEGINNINGBUF:
		if (!cur.mark())
			cur.clearSelection();
		cursorTop(cur);
		finishChange(cur, false);
		break;

	case LFUN_ENDBUF:
		if (!cur.mark())
			cur.clearSelection();
		cursorBottom(cur);
		finishChange(cur, false);
		break;

	case LFUN_RIGHT:
	case LFUN_RIGHTSEL:
		cur.selHandle(cmd.action == LFUN_RIGHTSEL);
		if (isRTL(cur.paragraph()))
			cursorLeft(cur);
		else
			cursorRight(cur);
		if (sl == cur.top())
			cmd = FuncRequest(LFUN_FINISHED_RIGHT);
		break;

	case LFUN_LEFT:
	case LFUN_LEFTSEL:
		cur.selHandle(cmd.action == LFUN_LEFTSEL);
		if (isRTL(cur.paragraph()))
			cursorRight(cur);
		else
			cursorLeft(cur);
		if (sl == cur.top())
			cmd = FuncRequest(LFUN_FINISHED_LEFT);
		break;

	case LFUN_UP:
	case LFUN_UPSEL:
		cur.selHandle(cmd.action == LFUN_UPSEL);
		cursorUp(cur);
		if (sl == cur.top())
			cmd = FuncRequest(LFUN_FINISHED_UP);
		break;

	case LFUN_DOWN:
	case LFUN_DOWNSEL:
		cur.selHandle(cmd.action == LFUN_DOWNSEL);
		cursorDown(cur);
		if (sl == cur.top())
			cmd = FuncRequest(LFUN_FINISHED_DOWN);
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
		if (isRTL(cur.paragraph()))
			cursorLeftOneWord(cur);
		else
			cursorRightOneWord(cur);
		finishChange(cur, true);
		break;

	case LFUN_WORDLEFTSEL:
		if (!cur.selection())
			cur.resetAnchor();
		if (isRTL(cur.paragraph()))
			cursorRightOneWord(cur);
		else
			cursorLeftOneWord(cur);
		finishChange(cur, true);
		break;

	case LFUN_WORDSEL: {
		selectWord(cur, lyx::WHOLE_WORD);
		finishChange(cur, true);
		break;
	}

	case LFUN_UP_PARAGRAPH:
		if (!cur.mark())
			cur.clearSelection();
		cursorUpParagraph(cur);
		finishChange(cur, false);
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
			cmd = FuncRequest(LFUN_FINISHED_UP);
		else
			cursorPrevious(cur);
		break;

	case LFUN_NEXT:
		if (!cur.mark())
			cur.clearSelection();
		finishChange(cur, false);
		if (cur.par() == cur.lastpar()
			  && cur.textRow().endpos() == cur.lastpos())
			cmd = FuncRequest(LFUN_FINISHED_DOWN);
		else
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
		if (cur.pos() > cur.paragraph().beginOfBody()) {
			replaceSelection(cur);
			cur.insert(new InsetNewline);
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
			cutSelection(cur, true, false);
		}
		moveCursor(cur, false);
		break;

	case LFUN_DELETE_SKIP:
		// Reverse the effect of LFUN_BREAKPARAGRAPH_SKIP.
		if (!cur.selection()) {
			if (cur.pos() == cur.lastpos()) {
				cursorRight(cur);
				cursorLeft(cur);
			}
			Delete(cur);
			cur.resetAnchor();
		} else {
			cutSelection(cur, true, false);
		}
		cur.update();
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
			cutSelection(cur, true, false);
		}
		bv->switchKeyMap();
		cur.update();
		break;

	case LFUN_BACKSPACE_SKIP:
		// Reverse the effect of LFUN_BREAKPARAGRAPH_SKIP.
		if (!cur.selection()) {
#warning look here
			//CursorSlice cur = cursor();
			backspace(cur);
			//anchor() = cur;
		} else {
			cutSelection(cur, true, false);
		}
		cur.update();
		break;

	case LFUN_BREAKPARAGRAPH:
		replaceSelection(cur);
		breakParagraph(cur, 0);
		cur.update();
		cur.resetAnchor();
		bv->switchKeyMap();
		break;

	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
		replaceSelection(cur);
		breakParagraph(cur, 1);
		cur.update();
		cur.resetAnchor();
		bv->switchKeyMap();
		break;

	case LFUN_BREAKPARAGRAPH_SKIP: {
		// When at the beginning of a paragraph, remove
		// indentation and add a "defskip" at the top.
		// Otherwise, do the same as LFUN_BREAKPARAGRAPH.
		replaceSelection(cur);
		if (cur.pos() == 0) {
			ParagraphParameters & params = cur.paragraph().params();
			setParagraph(cur,
					params.spacing(),
					params.align(),
					params.labelWidthString(), 1);
		} else {
			breakParagraph(cur, 0);
		}
		cur.update();
//	anchor() = cur;
		bv->switchKeyMap();
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
			redoParagraph(cur);
			cur.update();
		}
		break;
	}

	case LFUN_INSET_APPLY: {
		string const name = cmd.getArg(0);
		InsetBase * inset = bv->owner()->getDialogs().getOpenInset(name);
		if (inset) {
			FuncRequest fr(LFUN_INSET_MODIFY, cmd.argument);
			inset->dispatch(cur, fr);
		} else {
			FuncRequest fr(LFUN_INSET_INSERT, cmd.argument);
			dispatch(cur, fr);
		}
		break;
	}

	case LFUN_INSET_INSERT: {
		recordUndo(cur);
		InsetBase * inset = createInset(bv, cmd);
		if (inset)
			insertInset(cur, inset);
		break;
	}

	case LFUN_INSET_SETTINGS:
		if (cur.inset().asUpdatableInset())
			cur.inset().asUpdatableInset()->showInsetDialog(bv);
		break;

	case LFUN_INSET_TOGGLE:
		cur.clearSelection();
		if (!toggleInset(cur))
			cur.undispatched();
		else
			bv->switchKeyMap();
		break;

	case LFUN_SPACE_INSERT:
		if (cur.paragraph().layout()->free_spacing)
			insertChar(cur, ' ');
		else
			doInsertInset(cur, this, cmd, false, false);
		moveCursor(cur, false);
		break;

	case LFUN_HYPHENATION:
		specialChar(cur, this, InsetSpecialChar::HYPHENATION);
		break;

	case LFUN_LIGATURE_BREAK:
		specialChar(cur, this, InsetSpecialChar::LIGATURE_BREAK);
		break;

	case LFUN_LDOTS:
		specialChar(cur, this, InsetSpecialChar::LDOTS);
		break;

	case LFUN_END_OF_SENTENCE:
		specialChar(cur, this, InsetSpecialChar::END_OF_SENTENCE);
		break;

	case LFUN_MENU_SEPARATOR:
		specialChar(cur, this, InsetSpecialChar::MENU_SEPARATOR);
		break;

	case LFUN_UPCASE_WORD:
		changeCase(cur, LyXText::text_uppercase);
		cur.update();
		break;

	case LFUN_LOWCASE_WORD:
		changeCase(cur, LyXText::text_lowercase);
		cur.update();
		break;

	case LFUN_CAPITALIZE_WORD:
		changeCase(cur, LyXText::text_capitalization);
		cur.update();
		break;

	case LFUN_TRANSPOSE_CHARS:
		recordUndo(cur);
		redoParagraph(cur);
		cur.update();
		break;

	case LFUN_PASTE:
		cur.message(_("Paste"));
		replaceSelection(cur);
#warning FIXME Check if the arg is in the domain of available selections.
		if (isStrUnsignedInt(cmd.argument))
			pasteSelection(cur, strToUnsignedInt(cmd.argument));
		else
			pasteSelection(cur, 0);
		cur.clearSelection(); // bug 393
		cur.update();
		bv->switchKeyMap();
		finishUndo();
		break;

	case LFUN_CUT:
		cutSelection(cur, true, true);
		cur.message(_("Cut"));
		cur.update();
		break;

	case LFUN_COPY:
		copySelection(cur);
		cur.message(_("Copy"));
		break;

	case LFUN_GETXY:
		cur.message(tostr(cursorX(cur.top())) + ' '
		          + tostr(cursorY(cur.top())));
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
			setCursorFromCoordinates(cur, x, y);
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
		cur.message(cur.paragraph().layout()->name());
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
			setLayout(cur, layout);
			bv->owner()->setLayout(layout);
			cur.update();
			bv->switchKeyMap();
		}
		break;
	}

	case LFUN_PASTESELECTION: {
		cur.clearSelection();
		string const clip = bv->getClipboard();
		if (!clip.empty()) {
			if (cmd.argument == "paragraph")
				insertStringAsParagraphs(cur, clip);
			else
				insertStringAsLines(cur, clip);
			cur.update();
		}
		break;
	}

	case LFUN_GOTOERROR:
		gotoInset(cur, InsetBase::ERROR_CODE, false);
		break;

	case LFUN_GOTONOTE:
		gotoInset(cur, InsetBase::NOTE_CODE, false);
		break;

	case LFUN_REFERENCE_GOTO: {
		vector<InsetOld_code> tmp;
		tmp.push_back(InsetBase::LABEL_CODE);
		tmp.push_back(InsetBase::REF_CODE);
		gotoInset(cur, tmp, true);
		break;
	}

	case LFUN_QUOTE: {
		replaceSelection(cur);
		Paragraph & par = cur.paragraph();
		lyx::pos_type pos = cur.pos();
		char c;
		if (pos == 0)
			c = ' ';
		else if (cur.prevInset() && cur.prevInset()->isSpace())
			c = ' ';
		else
			c = par.getChar(pos - 1);

		LyXLayout_ptr const & style = par.layout();
		
		BufferParams const & bufparams = bv->buffer()->params();
		if (!style->pass_thru
		    && par.getFontSettings(bufparams, pos).language()->lang() != "hebrew") {
		    	string arg = cmd.argument;
			if (arg == "single")
				cur.insert(new InsetQuotes(c,
				    bufparams.quotes_language, 
				    InsetQuotes::SingleQ));
			else if (arg == "double")
				cur.insert(new InsetQuotes(c,
				    bufparams.quotes_language, 
				    InsetQuotes::DoubleQ));
			else
		  		cur.insert(new InsetQuotes(c, bufparams));
		}
		else
			bv->owner()->dispatch(FuncRequest(LFUN_SELFINSERT, "\""));
		break;
	}

	case LFUN_DATE_INSERT: {
		replaceSelection(cur);
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
			insertChar(cur, datetmp[i]);

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
			selectWord(cur, lyx::WHOLE_WORD_STRICT);
			bv->haveSelection(cur.selection());
		}
		break;

	case LFUN_MOUSE_MOTION: {
		// Only use motion with button 1
		//if (cmd.button() != mouse_button::button1)
		//	return false;
		// The test for not selection possible is needed, that
		// only motion events are used, where the bottom press
		// event was on the drawing area too
		if (!selection_possible) {
			lyxerr[Debug::ACTION] << "BufferView::Pimpl::"
				"Dispatch: no selection possible\n";
			break;
		}
		CursorSlice old = cur.top();
		setCursorFromCoordinates(cur, cmd.x, cmd.y);

		// This is to allow jumping over large insets
		// FIXME: shouldn't be top-text-specific
		if (isMainText() && cur.top() == old) {
			if (cmd.y - bv->top_y() >= bv->workHeight())
				cursorDown(cur);
			else if (cmd.y - bv->top_y() < 0)
				cursorUp(cur);
		}

		// don't set anchor_
		bv->cursor().setCursor(cur, false);
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

		setCursorFromCoordinates(cur, cmd.x, cmd.y);
		cur.resetAnchor();
		finishUndo();
		cur.x_target() = cursorX(cur.top());

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
		 || cmd.button() == mouse_button::button5) {
			cur.undispatched();
			break;
		}

		selection_possible = false;

		if (cmd.button() == mouse_button::button2)
			break;

		// finish selection
		if (cmd.button() == mouse_button::button1)
			bv->haveSelection(cur.selection());

		bv->switchKeyMap();
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
				cutSelection(cur, false, false);
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
		doInsertInset(cur, this, cmd, true, true);
		break;

	case LFUN_INDEX_INSERT:
		// Just open the inset
		doInsertInset(cur, this, cmd, true, false);
		break;

	case LFUN_INDEX_PRINT:
	case LFUN_TOC_INSERT:
	case LFUN_HFILL:
	case LFUN_INSERT_LINE:
	case LFUN_INSERT_PAGEBREAK:
		// do nothing fancy
		doInsertInset(cur, this, cmd, false, false);
		break;

	case LFUN_DEPTH_MIN:
		changeDepth(cur, bv_funcs::DEC_DEPTH);
		cur.update();
		break;

	case LFUN_DEPTH_PLUS:
		changeDepth(cur, bv_funcs::INC_DEPTH);
		cur.update();
		break;

	case LFUN_MATH_DISPLAY:
		mathDispatch(cur, this, cmd, true);
		break;

	case LFUN_MATH_IMPORT_SELECTION:
	case LFUN_MATH_MODE:
		mathDispatch(cur, this, cmd, false);
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
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_BOLD: {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setSeries(LyXFont::BOLD_SERIES);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_NOUN: {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setNoun(LyXFont::TOGGLE);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_CODE: {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setFamily(LyXFont::TYPEWRITER_FAMILY); // no good
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_SANS: {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setFamily(LyXFont::SANS_FAMILY);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_ROMAN: {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setFamily(LyXFont::ROMAN_FAMILY);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_DEFAULT: {
		LyXFont font(LyXFont::ALL_INHERIT, ignore_language);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_UNDERLINE: {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setUnderbar(LyXFont::TOGGLE);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_SIZE: {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLyXSize(cmd.argument);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_LANGUAGE: {
		Language const * lang = languages.getLanguage(cmd.argument);
		if (!lang)
			break;
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(lang);
		toggleAndShow(cur, this, font);
		bv->switchKeyMap();
		break;
	}

	case LFUN_FREEFONT_APPLY:
		toggleAndShow(cur, this, freefont, toggleall);
		cur.message(_("Character set"));
		break;

	// Set the freefont using the contents of \param data dispatched from
	// the frontends and apply it at the current cursor location.
	case LFUN_FREEFONT_UPDATE: {
		LyXFont font;
		bool toggle;
		if (bv_funcs::string2font(cmd.argument, font, toggle)) {
			freefont = font;
			toggleall = toggle;
			toggleAndShow(cur, this, freefont, toggleall);
			cur.message(_("Character set"));
		}
		break;
	}

	case LFUN_FINISHED_LEFT:
		lyxerr << "handle LFUN_FINISHED_LEFT" << endl;
		if (isRTL(cur.paragraph()))
			cursorLeft(cur);
		cur.bv().cursor() = cur;
		break;

	case LFUN_FINISHED_RIGHT:
		lyxerr << "handle LFUN_FINISHED_RIGHT" << endl;
		if (!isRTL(cur.paragraph()))
			cursorRight(cur);
		cur.bv().cursor() = cur;
		break;

	case LFUN_FINISHED_UP:
		lyxerr << "handle LFUN_FINISHED_UP" << endl;
		cursorUp(cur);
		cur.bv().cursor() = cur;
		break;

	case LFUN_FINISHED_DOWN:
		lyxerr << "handle LFUN_FINISHED_DOWN" << endl;
		cursorDown(cur);
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
		InsetBase & inset = cur.inset();
		bool const accept = !inset.forceDefaultParagraphs(&inset);

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
			cur.clearSelection();
			breakParagraph(cur);

			if (cur.lastpos() != 0) {
				cursorLeft(cur);
				breakParagraph(cur);
			}

			setLayout(cur, tclass.defaultLayoutName());
			setParagraph(cur, Spacing(), LYX_ALIGN_LAYOUT, string(), 0);
			insertInset(cur, new InsetFloatList(cmd.argument));
		} else {
			lyxerr << "Non-existent float type: "
			       << cmd.argument << endl;
		}
		break;
	}

	case LFUN_ACCEPT_CHANGE: {
		acceptChange(cur);
		cur.update();
		break;
	}

	case LFUN_REJECT_CHANGE: {
		rejectChange(cur);
		cur.update();
		break;
	}

	case LFUN_THESAURUS_ENTRY: {
		string arg = cmd.argument;
		if (arg.empty()) {
			arg = cur.selectionAsString(false);
			// FIXME
			if (arg.size() > 100 || arg.empty()) {
				// Get word or selection
				selectWordWhenUnderCursor(cur, lyx::WHOLE_WORD);
				arg = cur.selectionAsString(false);
			}
		}
		bv->owner()->getDialogs().show("thesaurus", arg);
		break;
	}

	case LFUN_PARAGRAPH_APPLY: {
		// Given data, an encoding of the ParagraphParameters
		// generated in the Paragraph dialog, this function sets
		// the current paragraph appropriately.
		istringstream is(cmd.argument);
		LyXLex lex(0, 0);
		lex.setStream(is);
		ParagraphParameters params;
		params.read(lex);
		setParagraph(cur,
					 params.spacing(),
					 params.align(),
					 params.labelWidthString(),
					 params.noindent());
		cur.update();
		cur.message(_("Paragraph layout set"));
		break;
	}

	case LFUN_INSET_DIALOG_SHOW: {
		InsetBase * inset = cur.nextInset();
		if (inset) {
			FuncRequest fr(LFUN_INSET_DIALOG_SHOW);
			inset->dispatch(cur, fr);
		}
		break;
	}

	case LFUN_ESCAPE:
		if (cur.selection())
			cur.selection() = false;
		else
			cmd = FuncRequest(LFUN_FINISHED_LEFT);
		break;

	default:
		cur.undispatched();
		break;
	}
}


bool LyXText::getStatus(LCursor &, FuncRequest const &, FuncStatus &)
{
	return false;
}
