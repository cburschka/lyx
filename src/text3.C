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

#include "FloatList.h"
#include "FuncStatus.h"
#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "cursor.h"
#include "coordcache.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "dispatchresult.h"
#include "factory.h"
#include "funcrequest.h"
#include "gettext.h"
#include "intl.h"
#include "language.h"
#include "LyXAction.h"
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
#include "support/convert.h"

#include "mathed/math_hullinset.h"
#include "mathed/math_macrotemplate.h"

#include <clocale>
#include <sstream>

using lyx::pos_type;

using lyx::cap::copySelection;
using lyx::cap::cutSelection;
using lyx::cap::pasteSelection;
using lyx::cap::replaceSelection;

using lyx::support::isStrUnsignedInt;
using lyx::support::token;

using std::endl;
using std::find;
using std::string;
using std::istringstream;
using std::vector;


extern string current_layout;


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
			text->bidi.computeTables(par, cur.buffer(), cur.textRow());
			if (cur.boundary() !=
					text->bidi.isBoundary(cur.buffer(), par,
							cur.pos(),
							text->real_current_font))
				text->setCursor(cur, cur.pit(), cur.pos(),
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


	void mathDispatch(LCursor & cur, FuncRequest const & cmd, bool display)
	{
		recordUndo(cur);
		string sel = cur.selectionAsString(false);
		lyxerr << "selection is: '" << sel << "'" << endl;

		if (sel.empty()) {
			const int old_pos = cur.pos();
			cur.insert(new MathHullInset);
			BOOST_ASSERT(old_pos == cur.pos());
			cur.nextInset()->edit(cur, true);
			cur.dispatch(FuncRequest(LFUN_MATH_MUTATE, "simple"));
			// don't do that also for LFUN_MATH_MODE
			// unless you want end up with always changing
			// to mathrm when opening an inlined inset --
			// I really hate "LyXfunc overloading"...
			if (display)
				cur.dispatch(FuncRequest(LFUN_MATH_DISPLAY));
			cur.dispatch(FuncRequest(LFUN_INSERT_MATH, cmd.argument));
		} else {
			// create a macro if we see "\\newcommand"
			// somewhere, and an ordinary formula
			// otherwise
			cutSelection(cur, true, true);
			if (sel.find("\\newcommand") == string::npos
			    && sel.find("\\def") == string::npos)
			{
				cur.insert(new MathHullInset);
				cur.dispatch(FuncRequest(LFUN_RIGHT));
				cur.dispatch(FuncRequest(LFUN_MATH_MUTATE, "simple"));
				cur.dispatch(FuncRequest(LFUN_INSERT_MATH, sel));
			} else {
				istringstream is(sel);
				cur.insert(new MathMacroTemplate(is));
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

bool LyXText::gotoNextInset(LCursor & cur,
	vector<InsetBase_code> const & codes, string const & contents)
{
	BOOST_ASSERT(this == cur.text());
	pit_type end = paragraphs().size();
	pit_type pit = cur.pit();
	pos_type pos = cur.pos();

	InsetBase * inset;
	do {
		if (pos + 1 < pars_[pit].size()) {
			++pos;
		} else  {
			++pit;
			pos = 0;
		}

	} while (pit != end &&
		 !(pars_[pit].isInset(pos) &&
		   (inset = pars_[pit].getInset(pos)) != 0 &&
		   find(codes.begin(), codes.end(), inset->lyxCode()) != codes.end() &&
		   (contents.empty() ||
		    static_cast<InsetCommand *>(pars_[pit].getInset(pos))->getContents()
		    == contents)));

	if (pit == end)
		return false;

	setCursor(cur, pit, pos, false);
	return true;
}


void LyXText::gotoInset(LCursor & cur,
	vector<InsetBase_code> const & codes, bool same_content)
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
		if (cur.pos() || cur.pit() != 0) {
			CursorSlice tmp = cur.top();
			cur.pit() = 0;
			cur.pos() = 0;
			if (!gotoNextInset(cur, codes, contents)) {
				cur.top() = tmp;
				cur.message(_("No more insets"));
			}
		} else {
			cur.message(_("No more insets"));
		}
	}
	cur.resetAnchor();
}


void LyXText::gotoInset(LCursor & cur, InsetBase_code code, bool same_content)
{
	gotoInset(cur, vector<InsetBase_code>(1, code), same_content);
}


bool LyXText::cursorPrevious(LCursor & cur)
{
	pos_type cpos = cur.pos();
	lyx::pit_type cpar = cur.pit();

	int x = cur.x_target();

	setCursorFromCoordinates(cur, x, 0);
	bool updated = cursorUp(cur);

	if (cpar == cur.pit() && cpos == cur.pos()) {
		// we have a row which is taller than the workarea. The
		// simplest solution is to move to the previous row instead.
		updated |= cursorUp(cur);
	}

	cur.bv().updateScrollbar();
	finishUndo();
	return updated;
}


bool LyXText::cursorNext(LCursor & cur)
{
	pos_type cpos = cur.pos();
	lyx::pit_type cpar = cur.pit();

	int x = cur.x_target();
	setCursorFromCoordinates(cur, x, cur.bv().workHeight() - 1);
	bool updated = cursorDown(cur);

	if (cpar == cur.pit() && cpos == cur.pos()) {
		// we have a row which is taller than the workarea. The
		// simplest solution is to move to the next row instead.
		updated |= cursorDown(cur);
	}

	cur.bv().updateScrollbar();
	finishUndo();
	return updated;
}


namespace {

void specialChar(LCursor & cur, InsetSpecialChar::Kind kind)
{
	lyx::cap::replaceSelection(cur);
	cur.insert(new InsetSpecialChar(kind));
	cur.posRight();
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


void update(LCursor & cur)
{
	//we don't call update(true, false) directly to save a metrics call
	if (cur.bv().fitCursor())
		cur.bv().update(false, true);
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
	lyxerr << "*** LyXText::dispatch: cmd: " << cmd << endl;

	BOOST_ASSERT(cur.text() == this);
	BufferView * bv = &cur.bv();
	CursorSlice oldTopSlice = cur.top();
	bool sel = cur.selection();
	bool needsUpdate = !lyxaction.funcHasFlag(cmd.action, LyXAction::NoUpdate);

	switch (cmd.action) {

	case LFUN_APPENDIX: {
		Paragraph & par = cur.paragraph();
		bool start = !par.params().startOfAppendix();

		// ensure that we have only one start_of_appendix in this document
		for (pit_type tmp = 0, end = pars_.size(); tmp != end; ++tmp) {
			if (pars_[tmp].params().startOfAppendix()) {
				recUndo(tmp);
				pars_[tmp].params().startOfAppendix(false);
				break;
			}
		}

		recordUndo(cur);
		par.params().startOfAppendix(start);

		// we can set the refreshing parameters now
		updateCounters();
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
			needsUpdate = cursorLeftOneWord(cur);
		else
			needsUpdate = cursorRightOneWord(cur);
		finishChange(cur, false);
		break;

	case LFUN_WORDLEFT:
		if (!cur.mark())
			cur.clearSelection();
		if (isRTL(cur.paragraph()))
			needsUpdate = cursorRightOneWord(cur);
		else
			needsUpdate = cursorLeftOneWord(cur);
		finishChange(cur, false);
		break;

	case LFUN_BEGINNINGBUF:
		if (cur.depth() == 1) {
			if (!cur.mark())
				cur.clearSelection();
			cursorTop(cur);
			finishChange(cur, false);
		} else {
			cur.undispatched();
		}
		break;

	case LFUN_BEGINNINGBUFSEL:
		if (cur.depth() == 1) {
			if (!cur.selection())
				cur.resetAnchor();
			cursorTop(cur);
			finishChange(cur, true);
		} else {
			cur.undispatched();
		}
		break;

	case LFUN_ENDBUF:
		if (cur.depth() == 1) {
			if (!cur.mark())
				cur.clearSelection();
			cursorBottom(cur);
			finishChange(cur, false);
		} else {
			cur.undispatched();
		}
		break;

	case LFUN_ENDBUFSEL:
		if (cur.depth() == 1) {
			if (!cur.selection())
				cur.resetAnchor();
			cursorBottom(cur);
			finishChange(cur, true);
		} else {
			cur.undispatched();
		}
		break;

	case LFUN_RIGHT:
	case LFUN_RIGHTSEL:
		lyxerr << BOOST_CURRENT_FUNCTION
		       << " LFUN_RIGHT[SEL]:\n" << cur << endl;
		cur.selHandle(cmd.action == LFUN_RIGHTSEL);
		if (isRTL(cur.paragraph()))
			needsUpdate = cursorLeft(cur);
		else
			needsUpdate = cursorRight(cur);
		if (!needsUpdate && oldTopSlice == cur.top()) {
			cur.undispatched();
			cmd = FuncRequest(LFUN_FINISHED_RIGHT);
		}
		break;

	case LFUN_LEFT:
	case LFUN_LEFTSEL:
		//lyxerr << "handle LFUN_LEFT[SEL]:\n" << cur << endl;
		cur.selHandle(cmd.action == LFUN_LEFTSEL);
		if (isRTL(cur.paragraph()))
			needsUpdate = cursorRight(cur);
		else
			needsUpdate = cursorLeft(cur);
		if (oldTopSlice == cur.top()) {
			cur.undispatched();
			cmd = FuncRequest(LFUN_FINISHED_LEFT);
		}
		break;

	case LFUN_UP:
	case LFUN_UPSEL:
		update(cur);
		//lyxerr << "handle LFUN_UP[SEL]:\n" << cur << endl;
		cur.selHandle(cmd.action == LFUN_UPSEL);
		needsUpdate = cursorUp(cur);
		if (oldTopSlice == cur.top()) {
			cur.undispatched();
			cmd = FuncRequest(LFUN_FINISHED_UP);
		}
		break;

	case LFUN_DOWN:
	case LFUN_DOWNSEL:
		update(cur);
		//lyxerr << "handle LFUN_DOWN[SEL]:\n" << cur << endl;
		cur.selHandle(cmd.action == LFUN_DOWNSEL);
		needsUpdate = cursorDown(cur);
		if (oldTopSlice == cur.top()) {
			cur.undispatched();
			cmd = FuncRequest(LFUN_FINISHED_DOWN);
		}
		break;

	case LFUN_UP_PARAGRAPH:
		if (!cur.mark())
			cur.clearSelection();
		needsUpdate = cursorUpParagraph(cur);
		finishChange(cur, false);
		break;

	case LFUN_UP_PARAGRAPHSEL:
		if (!cur.selection())
			cur.resetAnchor();
		cursorUpParagraph(cur);
		finishChange(cur, true);
		break;

	case LFUN_DOWN_PARAGRAPH:
		if (!cur.mark())
			cur.clearSelection();
		needsUpdate = cursorDownParagraph(cur);
		finishChange(cur, false);
		break;

	case LFUN_DOWN_PARAGRAPHSEL:
		if (!cur.selection())
			cur.resetAnchor();
		cursorDownParagraph(cur);
		finishChange(cur, true);
		break;

	case LFUN_PRIORSEL:
		update(cur);
		if (!cur.selection())
			cur.resetAnchor();
		cursorPrevious(cur);
		finishChange(cur, true);
		break;

	case LFUN_NEXTSEL:
		update(cur);
		if (!cur.selection())
			cur.resetAnchor();
		cursorNext(cur);
		finishChange(cur, true);
		break;

	case LFUN_HOMESEL:
		update(cur);
		if (!cur.selection())
			cur.resetAnchor();
		cursorHome(cur);
		finishChange(cur, true);
		break;

	case LFUN_ENDSEL:
		update(cur);
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

	case LFUN_PRIOR:
		update(cur);
		if (!cur.mark())
			cur.clearSelection();
		finishChange(cur, false);
		if (cur.pit() == 0 && cur.textRow().pos() == 0) {
			cur.undispatched();
			cmd = FuncRequest(LFUN_FINISHED_UP);
		} else {
			needsUpdate = cursorPrevious(cur);
		}
		break;

	case LFUN_NEXT:
		update(cur);
		if (!cur.mark())
			cur.clearSelection();
		finishChange(cur, false);
		if (cur.pit() == cur.lastpit()
			  && cur.textRow().endpos() == cur.lastpos()) {
			cur.undispatched();
			cmd = FuncRequest(LFUN_FINISHED_DOWN);
		} else {
			needsUpdate = cursorNext(cur);
		}
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
			lyx::cap::replaceSelection(cur);
			cur.insert(new InsetNewline);
			cur.posRight();
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
		break;

	case LFUN_BACKSPACE_SKIP:
		// Reverse the effect of LFUN_BREAKPARAGRAPH_SKIP.
		if (!cur.selection()) {
#ifdef WITH_WARNINGS
#warning look here
#endif
			//CursorSlice cur = cursor();
			backspace(cur);
			//anchor() = cur;
		} else {
			cutSelection(cur, true, false);
		}
		break;

	case LFUN_BREAKPARAGRAPH:
		lyx::cap::replaceSelection(cur);
		breakParagraph(cur, 0);
		cur.resetAnchor();
		bv->switchKeyMap();
		break;

	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
		lyx::cap::replaceSelection(cur);
		breakParagraph(cur, 1);
		cur.resetAnchor();
		bv->switchKeyMap();
		break;

	case LFUN_BREAKPARAGRAPH_SKIP: {
		// When at the beginning of a paragraph, remove
		// indentation.  Otherwise, do the same as LFUN_BREAKPARAGRAPH.
		lyx::cap::replaceSelection(cur);
		if (cur.pos() == 0)
			cur.paragraph().params().labelWidthString(string());
		else
			breakParagraph(cur, 0);
		cur.resetAnchor();
		bv->switchKeyMap();
		break;
	}

	case LFUN_PARAGRAPH_SPACING: {
		Paragraph & par = cur.paragraph();
		Spacing::Space cur_spacing = par.params().spacing().getSpace();
		string cur_value = "1.0";
		if (cur_spacing == Spacing::Other)
			cur_value = par.params().spacing().getValueAsString();

		istringstream is(cmd.argument);
		string tmp;
		is >> tmp;
		Spacing::Space new_spacing = cur_spacing;
		string new_value = cur_value;
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
			string tmpval = "0.0";
			is >> tmpval;
			lyxerr << "new_value = " << tmpval << endl;
			if (tmpval != "0.0")
				new_value = tmpval;
		} else if (tmp == "default") {
			new_spacing = Spacing::Default;
		} else {
			lyxerr << _("Unknown spacing argument: ")
			       << cmd.argument << endl;
		}
		if (cur_spacing != new_spacing || cur_value != new_value)
			par.params().spacing(Spacing(new_spacing, new_value));
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
		if (inset) {
			insertInset(cur, inset);
			cur.posRight();
		}
		break;
	}

	case LFUN_INSET_SETTINGS:
		if (cur.inset().asUpdatableInset())
			cur.inset().asUpdatableInset()->showInsetDialog(bv);
		break;

	case LFUN_NEXT_INSET_TOGGLE: {
		InsetBase * inset = cur.nextInset();
		if (inset) {
			cur.clearSelection();
			FuncRequest fr = cmd;
			fr.action = LFUN_INSET_TOGGLE;
			inset->dispatch(cur, fr);
		}
		break;
	}

	case LFUN_KEYMAP_TOGGLE:
		cur.clearSelection();
		bv->switchKeyMap();
		break;

	case LFUN_SPACE_INSERT:
		if (cur.paragraph().layout()->free_spacing)
			insertChar(cur, ' ');
		else {
			doInsertInset(cur, this, cmd, false, false);
			cur.posRight();
		}
		moveCursor(cur, false);
		break;

	case LFUN_HYPHENATION:
		specialChar(cur, InsetSpecialChar::HYPHENATION);
		break;

	case LFUN_LIGATURE_BREAK:
		specialChar(cur, InsetSpecialChar::LIGATURE_BREAK);
		break;

	case LFUN_LDOTS:
		specialChar(cur, InsetSpecialChar::LDOTS);
		break;

	case LFUN_END_OF_SENTENCE:
		specialChar(cur, InsetSpecialChar::END_OF_SENTENCE);
		break;

	case LFUN_MENU_SEPARATOR:
		specialChar(cur, InsetSpecialChar::MENU_SEPARATOR);
		break;

	case LFUN_UPCASE_WORD:
		changeCase(cur, LyXText::text_uppercase);
		break;

	case LFUN_LOWCASE_WORD:
		changeCase(cur, LyXText::text_lowercase);
		break;

	case LFUN_CAPITALIZE_WORD:
		changeCase(cur, LyXText::text_capitalization);
		break;

	case LFUN_TRANSPOSE_CHARS:
		recordUndo(cur);
		break;

	case LFUN_PASTE:
		cur.message(_("Paste"));
		lyx::cap::replaceSelection(cur);
#ifdef WITH_WARNINGS
#warning FIXME Check if the arg is in the domain of available selections.
#endif
		if (isStrUnsignedInt(cmd.argument))
			pasteSelection(cur, convert<unsigned int>(cmd.argument));
		else
			pasteSelection(cur, 0);
		cur.clearSelection(); // bug 393
		bv->switchKeyMap();
		finishUndo();
		break;

	case LFUN_CUT:
		cutSelection(cur, true, true);
		cur.message(_("Cut"));
		break;

	case LFUN_COPY:
		copySelection(cur);
		cur.message(_("Copy"));
		break;

	case LFUN_GETXY:
		cur.message(convert<string>(cursorX(cur.top())) + ' '
		          + convert<string>(cursorY(cur.top())));
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
			cur.selBegin().pit() != cur.selEnd().pit())
		{
			pit_type spit = cur.selBegin().pit();
			pit_type epit = cur.selEnd().pit() + 1;
			while (spit != epit) {
				if (pars_[spit].layout()->name() != current_layout) {
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
		vector<InsetBase_code> tmp;
		tmp.push_back(InsetBase::LABEL_CODE);
		tmp.push_back(InsetBase::REF_CODE);
		gotoInset(cur, tmp, true);
		break;
	}

	case LFUN_QUOTE: {
		lyx::cap::replaceSelection(cur);
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
			cur.posRight();
		}
		else
			bv->owner()->dispatch(FuncRequest(LFUN_SELFINSERT, "\""));
		break;
	}

	case LFUN_DATE_INSERT: {
		lyx::cap::replaceSelection(cur);
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
			cursorHome(cur);
			cur.resetAnchor();
			cursorEnd(cur);
			cur.setSelection();
			bv->haveSelection(cur.selection());
		}
		break;

	case LFUN_MOUSE_DOUBLE:
		if (cmd.button() == mouse_button::button1) {
			selectWord(cur, lyx::WHOLE_WORD_STRICT);
			bv->haveSelection(cur.selection());
		}
		break;

	// Single-click on work area
	case LFUN_MOUSE_PRESS: {
		// Right click on a footnote flag opens float menu
		if (cmd.button() == mouse_button::button3) {
			cur.clearSelection();
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

		// Clear the selection
		cur.clearSelection();

		setCursorFromCoordinates(cur, cmd.x, cmd.y);
		cur.resetAnchor();
		finishUndo();
		cur.setTargetX();

		// Has the cursor just left the inset?
		if (bv->cursor().inMathed() && !cur.inMathed())
			bv->cursor().inset().notifyCursorLeaves(bv->cursor());

		// Set cursor here.
		bv->cursor() = cur;

		// Insert primary selection with middle mouse
		// if there is a local selection in the current buffer,
		// insert this
		if (cmd.button() == mouse_button::button2) {
			if (paste_internally)
				bv->owner()->dispatch(FuncRequest(LFUN_PASTE));
			else
				bv->owner()->dispatch(FuncRequest(LFUN_PASTESELECTION, "paragraph"));
		}

		break;
	}

	case LFUN_MOUSE_MOTION: {
		// Only use motion with button 1
		//if (cmd.button() != mouse_button::button1)
		//	return false;

		// ignore motions deeper nested than the real anchor
		LCursor & bvcur = cur.bv().cursor();
		if (bvcur.anchor_.hasPart(cur)) {
			CursorSlice old = bvcur.top();

			int const wh = bv->workHeight();
			int const y = std::max(0, std::min(wh - 1, cmd.y));

			setCursorFromCoordinates(cur, cmd.x, y);
			cur.x_target() = cmd.x;
			if (cmd.y >= wh)
				cursorDown(cur);
			else if (cmd.y < 0)
				cursorUp(cur);
			// This is to allow jumping over large insets
			if (cur.top() == old) {
				if (cmd.y >= wh)
					cursorDown(cur);
				else if (cmd.y < 0)
					cursorUp(cur);
			}

			if (cur.top() == old)
				cur.noUpdate();
			else {
				// don't set anchor_
				bvcur.setCursor(cur);
				bvcur.selection() = true;
				lyxerr << "MOTION: " << bv->cursor() << endl;
			}

		} else
			cur.undispatched();
		break;
	}

	case LFUN_MOUSE_RELEASE: {
		if (cmd.button() == mouse_button::button2)
			break;

		// finish selection
		if (cmd.button() == mouse_button::button1)
			bv->haveSelection(cur.selection());

		bv->switchKeyMap();
		bv->owner()->updateMenubar();
		bv->owner()->updateToolbars();
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
		// Try to generate a valid label
		string const contents = cmd.argument.empty() ?
			cur.getPossibleLabel() : cmd.argument;

		InsetCommandParams p("label", contents);
		string const data = InsetCommandMailer::params2string("label", p);

		if (cmd.argument.empty()) {
			bv->owner()->getDialogs().show("label", data, 0);
		} else {
			FuncRequest fr(LFUN_INSET_INSERT, data);
			dispatch(cur, fr);
		}
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
		cur.posRight();
		break;

	case LFUN_INDEX_INSERT:
		// Just open the inset
		doInsertInset(cur, this, cmd, true, false);
		cur.posRight();
		break;

	case LFUN_INDEX_PRINT:
	case LFUN_TOC_INSERT:
	case LFUN_HFILL:
	case LFUN_INSERT_LINE:
	case LFUN_INSERT_PAGEBREAK:
		// do nothing fancy
		doInsertInset(cur, this, cmd, false, false);
		cur.posRight();
		break;

	case LFUN_DEPTH_MIN:
		changeDepth(cur, DEC_DEPTH);
		break;

	case LFUN_DEPTH_PLUS:
		changeDepth(cur, INC_DEPTH);
		break;

	case LFUN_MATH_DISPLAY:
		mathDispatch(cur, cmd, true);
		break;

	case LFUN_MATH_IMPORT_SELECTION:
	case LFUN_MATH_MODE:
		if (cmd.argument == "on")
			// don't pass "on" as argument
			mathDispatch(cur, FuncRequest(LFUN_MATH_MODE), false);
		else
			mathDispatch(cur, cmd, false);
		break;

	case LFUN_MATH_MACRO:
		if (cmd.argument.empty())
			cur.errorMessage(N_("Missing argument"));
		else {
			string s = cmd.argument;
			string const s1 = token(s, ' ', 1);
			int const nargs = s1.empty() ? 0 : convert<int>(s1);
			string const s2 = token(s, ' ', 2);
			string const type = s2.empty() ? "newcommand" : s2;
			cur.insert(new MathMacroTemplate(token(s, ' ', 0), nargs, s2));
			//cur.nextInset()->edit(cur, true);
		}
		break;

	// passthrough hat and underscore outside mathed:
	case LFUN_SUBSCRIPT:
		mathDispatch(cur, FuncRequest(LFUN_SELFINSERT, "_"), false);
		break;
	case LFUN_SUPERSCRIPT:
		mathDispatch(cur, FuncRequest(LFUN_SELFINSERT, "^"), false);
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
		lyxerr[Debug::DEBUG] << "handle LFUN_FINISHED_LEFT:\n" << cur << endl;
		break;

	case LFUN_FINISHED_RIGHT:
		lyxerr[Debug::DEBUG] << "handle LFUN_FINISHED_RIGHT:\n" << cur << endl;
		++cur.pos();
		break;

	case LFUN_FINISHED_UP:
		lyxerr[Debug::DEBUG] << "handle LFUN_FINISHED_UP:\n" << cur << endl;
		cursorUp(cur);
		break;

	case LFUN_FINISHED_DOWN:
		lyxerr[Debug::DEBUG] << "handle LFUN_FINISHED_DOWN:\n" << cur << endl;
		cursorDown(cur);
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

		data = "update " + convert<string>(accept) + '\n' + data;
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
			cur.posRight();
		} else {
			lyxerr << "Non-existent float type: "
			       << cmd.argument << endl;
		}
		break;
	}

	case LFUN_ACCEPT_CHANGE: {
		acceptChange(cur);
		break;
	}

	case LFUN_REJECT_CHANGE: {
		rejectChange(cur);
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
		if (cur.selection()) {
			cur.selection() = false;
		} else {
			cur.undispatched();
			cmd = FuncRequest(LFUN_FINISHED_LEFT);
		}
		break;

	default:
		lyxerr << BOOST_CURRENT_FUNCTION
		       << " Not DISPATCHED by LyXText" << endl;
		cur.undispatched();
		break;
	}

	if (!needsUpdate
	    && &oldTopSlice.inset() == &cur.inset()
	    && oldTopSlice.idx() == cur.idx()
	    && !sel
	    && !cur.selection())
		cur.noUpdate();
	else
		cur.needsUpdate();
}


bool LyXText::getStatus(LCursor & cur, FuncRequest const & cmd,
			FuncStatus & flag) const
{
	BOOST_ASSERT(cur.text() == this);
	LyXFont const & font = real_current_font;
	bool enable = true;

	switch (cmd.action) {

	case LFUN_DEPTH_MIN:
		enable = changeDepthAllowed(cur, DEC_DEPTH);
		break;

	case LFUN_DEPTH_PLUS:
		enable = changeDepthAllowed(cur, INC_DEPTH);
		break;

	case LFUN_INSET_OPTARG:
		enable = numberOfOptArgs(cur.paragraph())
			< cur.paragraph().layout()->optionalargs;
		break;

	case LFUN_APPENDIX:
		flag.setOnOff(cur.paragraph().params().startOfAppendix());
		break;

#if 0
	// the functions which insert insets
	InsetBase::Code code = InsetBase::NO_CODE;
	switch (cmd.action) {
	case LFUN_DIALOG_SHOW_NEW_INSET:
		if (cmd.argument == "bibitem")
			code = InsetBase::BIBITEM_CODE;
		else if (cmd.argument == "bibtex")
			code = InsetBase::BIBTEX_CODE;
		else if (cmd.argument == "box")
			code = InsetBase::BOX_CODE;
		else if (cmd.argument == "branch")
			code = InsetBase::BRANCH_CODE;
		else if (cmd.argument == "citation")
			code = InsetBase::CITE_CODE;
		else if (cmd.argument == "ert")
			code = InsetBase::ERT_CODE;
		else if (cmd.argument == "external")
			code = InsetBase::EXTERNAL_CODE;
		else if (cmd.argument == "float")
			code = InsetBase::FLOAT_CODE;
		else if (cmd.argument == "graphics")
			code = InsetBase::GRAPHICS_CODE;
		else if (cmd.argument == "include")
			code = InsetBase::INCLUDE_CODE;
		else if (cmd.argument == "index")
			code = InsetBase::INDEX_CODE;
		else if (cmd.argument == "label")
			code = InsetBase::LABEL_CODE;
		else if (cmd.argument == "note")
			code = InsetBase::NOTE_CODE;
		else if (cmd.argument == "ref")
			code = InsetBase::REF_CODE;
		else if (cmd.argument == "toc")
			code = InsetBase::TOC_CODE;
		else if (cmd.argument == "url")
			code = InsetBase::URL_CODE;
		else if (cmd.argument == "vspace")
			code = InsetBase::VSPACE_CODE;
		else if (cmd.argument == "wrap")
			code = InsetBase::WRAP_CODE;
		break;

	case LFUN_INSET_ERT:
		code = InsetBase::ERT_CODE;
		break;
	case LFUN_INSET_FOOTNOTE:
		code = InsetBase::FOOT_CODE;
		break;
	case LFUN_TABULAR_INSERT:
		code = InsetBase::TABULAR_CODE;
		break;
	case LFUN_INSET_MARGINAL:
		code = InsetBase::MARGIN_CODE;
		break;
	case LFUN_INSET_FLOAT:
	case LFUN_INSET_WIDE_FLOAT:
		code = InsetBase::FLOAT_CODE;
		break;
	case LFUN_INSET_WRAP:
		code = InsetBase::WRAP_CODE;
		break;
	case LFUN_FLOAT_LIST:
		code = InsetBase::FLOAT_LIST_CODE;
		break;
#if 0
	case LFUN_INSET_LIST:
		code = InsetBase::LIST_CODE;
		break;
	case LFUN_INSET_THEOREM:
		code = InsetBase::THEOREM_CODE;
		break;
#endif
	case LFUN_INSET_CAPTION:
		code = InsetBase::CAPTION_CODE;
		break;
	case LFUN_INSERT_NOTE:
		code = InsetBase::NOTE_CODE;
		break;
	case LFUN_INSERT_CHARSTYLE:
		code = InsetBase::CHARSTYLE_CODE;
		if (buf->params().getLyXTextClass().charstyles().empty())
			enable = false;
		break;
	case LFUN_INSERT_BOX:
		code = InsetBase::BOX_CODE;
		break;
	case LFUN_INSERT_BRANCH:
		code = InsetBase::BRANCH_CODE;
		if (buf->params().branchlist().empty())
			enable = false;
		break;
	case LFUN_INSERT_LABEL:
		code = InsetBase::LABEL_CODE;
		break;
	case LFUN_INSET_OPTARG:
		code = InsetBase::OPTARG_CODE;
		break;
	case LFUN_ENVIRONMENT_INSERT:
		code = InsetBase::BOX_CODE;
		break;
	case LFUN_INDEX_INSERT:
		code = InsetBase::INDEX_CODE;
		break;
	case LFUN_INDEX_PRINT:
		code = InsetBase::INDEX_PRINT_CODE;
		break;
	case LFUN_TOC_INSERT:
		code = InsetBase::TOC_CODE;
		break;
	case LFUN_HTMLURL:
	case LFUN_URL:
		code = InsetBase::URL_CODE;
		break;
	case LFUN_QUOTE:
		// always allow this, since we will inset a raw quote
		// if an inset is not allowed.
		break;
	case LFUN_HYPHENATION:
	case LFUN_LIGATURE_BREAK:
	case LFUN_HFILL:
	case LFUN_MENU_SEPARATOR:
	case LFUN_LDOTS:
	case LFUN_END_OF_SENTENCE:
		code = InsetBase::SPECIALCHAR_CODE;
		break;
	case LFUN_SPACE_INSERT:
		// slight hack: we know this is allowed in math mode
		if (cur.inTexted())
			code = InsetBase::SPACE_CODE;
		break;
	case LFUN_INSET_DIALOG_SHOW: {
		InsetBase * inset = cur.nextInset();
		enable = inset;
		if (inset) {
			code = inset->lyxCode();
			if (!(code == InsetBase::INCLUDE_CODE
				|| code == InsetBase::BIBTEX_CODE
				|| code == InsetBase::FLOAT_LIST_CODE
				|| code == InsetBase::TOC_CODE))
				enable = false;
		}
		break;
	}
	default:
		break;
	}

	if (code != InsetBase::NO_CODE
			&& (cur.empty() || !cur.inset().insetAllowed(code)))
		enable = false;

#endif

	case LFUN_DIALOG_SHOW_NEW_INSET:
	case LFUN_INSET_ERT:
	case LFUN_INSERT_BOX:
	case LFUN_INSERT_BRANCH:
	case LFUN_ENVIRONMENT_INSERT:
	case LFUN_INDEX_INSERT:
	case LFUN_INDEX_PRINT:
	case LFUN_TOC_INSERT:
	case LFUN_HTMLURL:
	case LFUN_URL:
	case LFUN_QUOTE:
	case LFUN_HYPHENATION:
	case LFUN_LIGATURE_BREAK:
	case LFUN_HFILL:
	case LFUN_MENU_SEPARATOR:
	case LFUN_LDOTS:
	case LFUN_END_OF_SENTENCE:
	case LFUN_SPACE_INSERT:
	case LFUN_INSET_DIALOG_SHOW:
		break;

	case LFUN_EMPH:
		flag.setOnOff(font.emph() == LyXFont::ON);
		break;

	case LFUN_NOUN:
		flag.setOnOff(font.noun() == LyXFont::ON);
		break;

	case LFUN_BOLD:
		flag.setOnOff(font.series() == LyXFont::BOLD_SERIES);
		break;

	case LFUN_SANS:
		flag.setOnOff(font.family() == LyXFont::SANS_FAMILY);
		break;

	case LFUN_ROMAN:
		flag.setOnOff(font.family() == LyXFont::ROMAN_FAMILY);
		break;

	case LFUN_CODE:
		flag.setOnOff(font.family() == LyXFont::TYPEWRITER_FAMILY);
		break;

	case LFUN_DELETE_WORD_FORWARD:
	case LFUN_DELETE_WORD_BACKWARD:
	case LFUN_DELETE_LINE_FORWARD:
	case LFUN_WORDRIGHT:
	case LFUN_WORDLEFT:
	case LFUN_RIGHT:
	case LFUN_RIGHTSEL:
	case LFUN_LEFT:
	case LFUN_LEFTSEL:
	case LFUN_UP:
	case LFUN_UPSEL:
	case LFUN_DOWN:
	case LFUN_DOWNSEL:
	case LFUN_UP_PARAGRAPHSEL:
	case LFUN_DOWN_PARAGRAPHSEL:
	case LFUN_PRIORSEL:
	case LFUN_NEXTSEL:
	case LFUN_HOMESEL:
	case LFUN_ENDSEL:
	case LFUN_WORDRIGHTSEL:
	case LFUN_WORDLEFTSEL:
	case LFUN_WORDSEL:
	case LFUN_UP_PARAGRAPH:
	case LFUN_DOWN_PARAGRAPH:
	case LFUN_PRIOR:
	case LFUN_NEXT:
	case LFUN_HOME:
	case LFUN_END:
	case LFUN_BREAKLINE:
	case LFUN_DELETE:
	case LFUN_DELETE_SKIP:
	case LFUN_BACKSPACE:
	case LFUN_BACKSPACE_SKIP:
	case LFUN_BREAKPARAGRAPH:
	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
	case LFUN_BREAKPARAGRAPH_SKIP:
	case LFUN_PARAGRAPH_SPACING:
	case LFUN_INSET_APPLY:
	case LFUN_INSET_INSERT:
	case LFUN_NEXT_INSET_TOGGLE:
	case LFUN_UPCASE_WORD:
	case LFUN_LOWCASE_WORD:
	case LFUN_CAPITALIZE_WORD:
	case LFUN_TRANSPOSE_CHARS:
	case LFUN_PASTE:
	case LFUN_CUT:
	case LFUN_COPY:
	case LFUN_GETXY:
	case LFUN_SETXY:
	case LFUN_GETFONT:
	case LFUN_GETLAYOUT:
	case LFUN_LAYOUT:
	case LFUN_PASTESELECTION:
	case LFUN_GOTOERROR:
	case LFUN_GOTONOTE:
	case LFUN_REFERENCE_GOTO:
	case LFUN_DATE_INSERT:
	case LFUN_SELFINSERT:
	case LFUN_INSERT_LABEL:
	case LFUN_INSERT_NOTE:
	case LFUN_INSERT_CHARSTYLE:
	case LFUN_INSERT_BIBITEM:
	case LFUN_INSET_FLOAT:
	case LFUN_INSET_FOOTNOTE:
	case LFUN_INSET_MARGINAL:
	case LFUN_INSET_WIDE_FLOAT:
	case LFUN_INSET_WRAP:
	case LFUN_TABULAR_INSERT:
	case LFUN_INSERT_LINE:
	case LFUN_INSERT_PAGEBREAK:
	case LFUN_MATH_DISPLAY:
	case LFUN_MATH_IMPORT_SELECTION:
	case LFUN_MATH_MODE:
	case LFUN_MATH_MACRO:
	case LFUN_INSERT_MATH:
	case LFUN_INSERT_MATRIX:
	case LFUN_MATH_DELIM:
	case LFUN_SUBSCRIPT:
	case LFUN_SUPERSCRIPT:
	case LFUN_DEFAULT:
	case LFUN_UNDERLINE:
	case LFUN_FONT_SIZE:
	case LFUN_LANGUAGE:
	case LFUN_FREEFONT_APPLY:
	case LFUN_FREEFONT_UPDATE:
	case LFUN_LAYOUT_PARAGRAPH:
	case LFUN_PARAGRAPH_UPDATE:
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
	case LFUN_FLOAT_LIST:
	case LFUN_ACCEPT_CHANGE:
	case LFUN_REJECT_CHANGE:
	case LFUN_THESAURUS_ENTRY:
	case LFUN_PARAGRAPH_APPLY:
	case LFUN_ESCAPE:
	case LFUN_KEYMAP_TOGGLE:
	case LFUN_ENDBUF:
	case LFUN_BEGINNINGBUF:
	case LFUN_BEGINNINGBUFSEL:
	case LFUN_ENDBUFSEL:
		// these are handled in our dispatch()
		enable = true;
		break;

	default:
		return false;
	}
	flag.enabled(enable);
	return true;
}
