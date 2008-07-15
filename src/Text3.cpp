/**
 * \file Text3.cpp
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

#include "Text.h"

#include "BranchList.h"
#include "FloatList.h"
#include "FuncStatus.h"
#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "bufferview_funcs.h"
#include "Cursor.h"
#include "CoordCache.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "DispatchResult.h"
#include "ErrorList.h"
#include "factory.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "Intl.h"
#include "Language.h"
#include "LyXAction.h"
#include "LyXFunc.h"
#include "Lexer.h"
#include "LyXRC.h"
#include "Row.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "toc.h"
#include "Undo.h"
#include "VSpace.h"
#include "ParIterator.h"

#include "frontends/Clipboard.h"
#include "frontends/Selection.h"

#include "insets/InsetCommand.h"
#include "insets/InsetFloatList.h"
#include "insets/InsetNewline.h"
#include "insets/InsetQuotes.h"
#include "insets/InsetSpecialChar.h"
#include "insets/InsetText.h"

#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/convert.h"
#include "support/lyxtime.h"

#include "mathed/InsetMathHull.h"
#include "mathed/MathMacroTemplate.h"

#include <boost/current_function.hpp>

#include <clocale>
#include <sstream>


namespace lyx {

using cap::copySelection;
using cap::cutSelection;
using cap::pasteFromStack;
using cap::pasteClipboard;
using cap::replaceSelection;

using support::isStrUnsignedInt;
using support::token;

using std::endl;
using std::string;
using std::istringstream;
using std::ostringstream;


extern docstring current_layout;


namespace {

	// globals...
	Font freefont(Font::ALL_IGNORE);
	bool toggleall = false;

	void toggleAndShow(Cursor & cur, Text * text,
		Font const & font, bool toggleall = true)
	{
		text->toggleFree(cur, font, toggleall);

		if (font.language() != ignore_language ||
				font.number() != Font::IGNORE) {
			Paragraph & par = cur.paragraph();
			if (cur.boundary() != text->isRTLBoundary(cur.buffer(), par,
			                        cur.pos(), text->real_current_font))
				text->setCursor(cur, cur.pit(), cur.pos(),
				                false, !cur.boundary());
		}
	}


	void moveCursor(Cursor & cur, bool selecting)
	{
		if (selecting || cur.mark())
			cur.setSelection();
	}


	void finishChange(Cursor & cur, bool selecting)
	{
		finishUndo();
		moveCursor(cur, selecting);
	}


	void mathDispatch(Cursor & cur, FuncRequest const & cmd, bool display)
	{
		recordUndo(cur);
		docstring sel = cur.selectionAsString(false);

		// It may happen that sel is empty but there is a selection
		replaceSelection(cur);

		if (sel.empty()) {
#ifdef ENABLE_ASSERTIONS
			const int old_pos = cur.pos();
#endif
			cur.insert(new InsetMathHull(hullSimple));
			BOOST_ASSERT(old_pos == cur.pos());
			Inset * inset = cur.nextInset();
			inset->edit(cur, true);
			// don't do that also for LFUN_MATH_MODE
			// unless you want end up with always changing
			// to mathrm when opening an inlined inset --
			// I really hate "LyXfunc overloading"...
			if (display) {
				FuncRequest cmdm(LFUN_MATH_DISPLAY);
				inset->dispatch(cur, cmdm);
			}
			// Avoid an unnecessary undo step if cmd.argument
			// is empty
			if (!cmd.argument().empty()) {
				FuncRequest cmdm(LFUN_MATH_INSERT, cmd.argument());
				inset->dispatch(cur, cmdm);
			}
		} else {
			// create a macro if we see "\\newcommand"
			// somewhere, and an ordinary formula
			// otherwise
			if (sel.find(from_ascii("\\newcommand")) == string::npos
			    && sel.find(from_ascii("\\def")) == string::npos)
			{
				InsetMathHull * formula = new InsetMathHull;
				istringstream is(to_utf8(sel));
				Lexer lex(0, 0);
				lex.setStream(is);
				formula->read(cur.buffer(), lex);
				if (formula->getType() == hullNone)
					// Don't create pseudo formulas if
					// delimiters are left out
					formula->mutate(hullSimple);
				cur.insert(formula);
			} else {
				cur.insert(new MathMacroTemplate(sel));
			}
		}
		cur.message(from_utf8(N_("Math editor mode")));
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

void Text::cursorPrevious(Cursor & cur)
{
	pos_type cpos = cur.pos();
	pit_type cpar = cur.pit();

	setCursorFromCoordinates(cur, cur.x_target(), 0);
	cur.dispatch(FuncRequest(cur.selection()? LFUN_UP_SELECT: LFUN_UP));

	if (cpar == cur.pit() && cpos == cur.pos())
		// we have a row which is taller than the workarea. The
		// simplest solution is to move to the previous row instead.
		cur.dispatch(FuncRequest(cur.selection()? LFUN_UP_SELECT: LFUN_UP));

	finishUndo();
	cur.updateFlags(Update::Force | Update::FitCursor);
	/* one of the dispatch calls above may have set the dispatched
	 * status of the cursor to false. Did I ever say that calling
	 * cur.dispatch directly is a bad idea? See bug 4570 for a
	 * possible consequence (JMarc)
	 */
	cur.dispatched();
}


void Text::cursorNext(Cursor & cur)
{
	pos_type cpos = cur.pos();
	pit_type cpar = cur.pit();

	setCursorFromCoordinates(cur, cur.x_target(), cur.bv().workHeight() - 1);
	cur.dispatch(FuncRequest(cur.selection()? LFUN_DOWN_SELECT: LFUN_DOWN));

	if (cpar == cur.pit() && cpos == cur.pos())
		// we have a row which is taller than the workarea. The
		// simplest solution is to move to the next row instead.
		cur.dispatch(
			FuncRequest(cur.selection()? LFUN_DOWN_SELECT: LFUN_DOWN));

	finishUndo();
	cur.updateFlags(Update::Force | Update::FitCursor);
	/* one of the dispatch calls above may have set the dispatched
	 * status of the cursor to false. Did I ever say that calling
	 * cur.dispatch directly is a bad idea? See bug 4570 for a
	 * possible consequence (JMarc)
	 */
	cur.dispatched();
}


namespace {

void specialChar(Cursor & cur, InsetSpecialChar::Kind kind)
{
	recordUndo(cur);
	cap::replaceSelection(cur);
	cur.insert(new InsetSpecialChar(kind));
	cur.posRight();
}


bool doInsertInset(Cursor & cur, Text * text,
	FuncRequest const & cmd, bool edit, bool pastesel)
{
	Inset * inset = createInset(&cur.bv(), cmd);
	if (!inset)
		return false;

	recordUndo(cur);
	bool gotsel = false;
	if (cur.selection()) {
		cutSelection(cur, false, pastesel);
		cur.clearSelection();
		gotsel = true;
	}
	bool const emptypar = cur.lastpos() == 0;
	pos_type ins_pos = cur.pos();
	text->insertInset(cur, inset);

	if (edit)
		inset->edit(cur, true);

	if (gotsel && pastesel) {
		// metrics might be invalid at this point (bug 4502)
		cur.bv().updateMetrics();
		pasteFromStack(cur, cur.buffer().errorList("Paste"), 0);
		cur.buffer().errors("Paste");
		cur.clearSelection(); // bug 393
		finishUndo();

		if ((cur.lastpit() == 0 || ins_pos != 0) && !emptypar) {
			// reset first par to default
			Layout_ptr const layout =
				cur.buffer().params().getTextClass().defaultLayout();
			cur.text()->paragraphs().begin()->layout(layout);
		} else {
			// reset surrounding par to default
			docstring const layoutname =
				cur.buffer().params().getTextClass().defaultLayoutName();
			cur.leaveInset(*inset);
			text->setLayout(cur, layoutname);
		}
	}
	return true;
}


} // anon namespace


void Text::number(Cursor & cur)
{
	Font font(Font::ALL_IGNORE);
	font.setNumber(Font::TOGGLE);
	toggleAndShow(cur, this, font);
}


bool Text::isRTL(Buffer const & buffer, Paragraph const & par) const
{
	return par.isRightToLeftPar(buffer.params());
}


bool Text::isRTL(Buffer const & buffer, CursorSlice const & sl, bool boundary) const
{
	if (!lyxrc.rtl_support || !sl.text())
		return false;

	int correction = 0;
	if (boundary && sl.pos() > 0)
		correction = -1;
		
	Paragraph const & par = getPar(sl.pit());
	return getFont(buffer, par, sl.pos() + correction).isVisibleRightToLeft();
}


bool Text::isRTLBoundary(Buffer const & buffer, Paragraph const & par,
                         pos_type pos) const
{
	if (!lyxrc.rtl_support)
		return false;

	// no RTL boundary at line start
	if (pos == 0)
		return false;

	bool left = getFont(buffer, par, pos - 1).isVisibleRightToLeft();
	bool right;
	if (pos == par.size())
		right = par.isRightToLeftPar(buffer.params());
	else
		right = getFont(buffer, par, pos).isVisibleRightToLeft();
	return left != right;
}


bool Text::isRTLBoundary(Buffer const & buffer, Paragraph const & par,
                         pos_type pos, Font const & font) const
{
	if (!lyxrc.rtl_support)
		return false;

	bool left = font.isVisibleRightToLeft();
	bool right;
	if (pos == par.size())
		right = par.isRightToLeftPar(buffer.params());
	else
		right = getFont(buffer, par, pos).isVisibleRightToLeft();
	return left != right;
}


void Text::dispatch(Cursor & cur, FuncRequest & cmd)
{
	LYXERR(Debug::ACTION) << "Text::dispatch: cmd: " << cmd << endl;

	// FIXME: We use the update flag to indicates wether a singlePar or a
	// full screen update is needed. We reset it here but shall we restore it
	// at the end?
	cur.noUpdate();

	BOOST_ASSERT(cur.text() == this);
	BufferView * bv = &cur.bv();
	CursorSlice oldTopSlice = cur.top();
	bool oldBoundary = cur.boundary();
	bool sel = cur.selection();
	// Signals that, even if needsUpdate == false, an update of the
	// cursor paragraph is required
	bool singleParUpdate = lyxaction.funcHasFlag(cmd.action,
		LyXAction::SingleParUpdate);
	// Signals that a full-screen update is required
	bool needsUpdate = !(lyxaction.funcHasFlag(cmd.action,
		LyXAction::NoUpdate) || singleParUpdate);
	// Remember the old paragraph metric (_outer_ paragraph!)
	ParagraphMetrics const & pm = cur.bv().parMetrics(
		cur.bottom().text(), cur.bottom().pit());
	Dimension olddim = pm.dim();

	switch (cmd.action) {

	case LFUN_PARAGRAPH_MOVE_DOWN: {
		pit_type const pit = cur.pit();
		recUndo(cur, pit, pit + 1);
		finishUndo();
		std::swap(pars_[pit], pars_[pit + 1]);
		updateLabels(cur.buffer());
		needsUpdate = true;
		++cur.pit();
		break;
	}

	case LFUN_PARAGRAPH_MOVE_UP: {
		pit_type const pit = cur.pit();
		recUndo(cur, pit - 1, pit);
		finishUndo();
		std::swap(pars_[pit], pars_[pit - 1]);
		updateLabels(cur.buffer());
		--cur.pit();
		needsUpdate = true;
		break;
	}

	case LFUN_APPENDIX: {
		Paragraph & par = cur.paragraph();
		bool start = !par.params().startOfAppendix();

#ifdef WITH_WARNINGS
#warning The code below only makes sense at top level.
// Should LFUN_APPENDIX be restricted to top-level paragraphs?
#endif
		// ensure that we have only one start_of_appendix in this document
		// FIXME: this don't work for multipart document!
		for (pit_type tmp = 0, end = pars_.size(); tmp != end; ++tmp) {
			if (pars_[tmp].params().startOfAppendix()) {
				recUndo(cur, tmp);
				pars_[tmp].params().startOfAppendix(false);
				break;
			}
		}

		recordUndo(cur);
		par.params().startOfAppendix(start);

		// we can set the refreshing parameters now
		updateLabels(cur.buffer());
		break;
	}

	case LFUN_WORD_DELETE_FORWARD:
		if (cur.selection()) {
			cutSelection(cur, true, false);
		} else
			deleteWordForward(cur);
		finishChange(cur, false);
		break;

	case LFUN_WORD_DELETE_BACKWARD:
		if (cur.selection()) {
			cutSelection(cur, true, false);
		} else
			deleteWordBackward(cur);
		finishChange(cur, false);
		break;

	case LFUN_LINE_DELETE:
		if (cur.selection()) {
			cutSelection(cur, true, false);
		} else
			deleteLineForward(cur);
		finishChange(cur, false);
		break;

	case LFUN_BUFFER_BEGIN:
	case LFUN_BUFFER_BEGIN_SELECT:
		needsUpdate |= cur.selHandle(cmd.action == LFUN_BUFFER_BEGIN_SELECT);
		if (cur.depth() == 1) {
			needsUpdate |= cursorTop(cur);
		} else {
			cur.undispatched();
		}
		break;

	case LFUN_BUFFER_END:
	case LFUN_BUFFER_END_SELECT:
		needsUpdate |= cur.selHandle(cmd.action == LFUN_BUFFER_END_SELECT);
		if (cur.depth() == 1) {
			needsUpdate |= cursorBottom(cur);
		} else {
			cur.undispatched();
		}
		break;

	case LFUN_CHAR_FORWARD:
	case LFUN_CHAR_FORWARD_SELECT:
		//lyxerr << BOOST_CURRENT_FUNCTION
		//       << " LFUN_CHAR_FORWARD[SEL]:\n" << cur << endl;
		needsUpdate |= cur.selHandle(cmd.action == LFUN_CHAR_FORWARD_SELECT);
		if (reverseDirectionNeeded(cur))
			needsUpdate |= cursorLeft(cur);
		else
			needsUpdate |= cursorRight(cur);

		if (!needsUpdate && oldTopSlice == cur.top()
				&& cur.boundary() == oldBoundary) {
			cur.undispatched();
			cmd = FuncRequest(LFUN_FINISHED_RIGHT);
		}
		break;

	case LFUN_CHAR_BACKWARD:
	case LFUN_CHAR_BACKWARD_SELECT:
		//lyxerr << "handle LFUN_CHAR_BACKWARD[_SELECT]:\n" << cur << endl;
		needsUpdate |= cur.selHandle(cmd.action == LFUN_CHAR_BACKWARD_SELECT);
		if (reverseDirectionNeeded(cur))
			needsUpdate |= cursorRight(cur);
		else
			needsUpdate |= cursorLeft(cur);

		if (!needsUpdate && oldTopSlice == cur.top()
			&& cur.boundary() == oldBoundary) {
			cur.undispatched();
			cmd = FuncRequest(LFUN_FINISHED_LEFT);
		}
		break;

	case LFUN_UP_SELECT:
	case LFUN_DOWN_SELECT:
	case LFUN_UP:
	case LFUN_DOWN: {
		// stop/start the selection
		bool select = cmd.action == LFUN_DOWN_SELECT ||
			cmd.action == LFUN_UP_SELECT;
		cur.selHandle(select);
		
		// move cursor up/down
		bool up = cmd.action == LFUN_UP_SELECT || cmd.action == LFUN_UP;
		bool const successful = cur.upDownInText(up, needsUpdate);
		if (successful) {
			// notify insets which were left and get their update flags 
			notifyCursorLeaves(cur.beforeDispatchCursor(), cur);
			cur.fixIfBroken();
			
			// redraw if you leave mathed (for the decorations)
			needsUpdate |= cur.beforeDispatchCursor().inMathed();
		} else
			cur.undispatched();
		
		break;
	}

	case LFUN_PARAGRAPH_UP:
	case LFUN_PARAGRAPH_UP_SELECT:
		needsUpdate |= cur.selHandle(cmd.action == LFUN_PARAGRAPH_UP_SELECT);
		needsUpdate |= cursorUpParagraph(cur);
		break;

	case LFUN_PARAGRAPH_DOWN:
	case LFUN_PARAGRAPH_DOWN_SELECT:
		needsUpdate |= cur.selHandle(cmd.action == LFUN_PARAGRAPH_DOWN_SELECT);
		needsUpdate |= cursorDownParagraph(cur);
		break;

	case LFUN_SCREEN_UP:
	case LFUN_SCREEN_UP_SELECT:
		needsUpdate |= cur.selHandle(cmd.action == LFUN_SCREEN_UP_SELECT);
		if (cur.pit() == 0 && cur.textRow().pos() == 0)
			cur.undispatched();
		else {
			cursorPrevious(cur);
		}
		break;

	case LFUN_SCREEN_DOWN:
	case LFUN_SCREEN_DOWN_SELECT:
		needsUpdate |= cur.selHandle(cmd.action == LFUN_SCREEN_DOWN_SELECT);
		if (cur.pit() == cur.lastpit()
			  && cur.textRow().endpos() == cur.lastpos())
			cur.undispatched();
		else {
			cursorNext(cur);
		}
		break;

	case LFUN_LINE_BEGIN:
	case LFUN_LINE_BEGIN_SELECT:
		needsUpdate |= cur.selHandle(cmd.action == LFUN_LINE_BEGIN_SELECT);
		needsUpdate |= cursorHome(cur);
		break;

	case LFUN_LINE_END:
	case LFUN_LINE_END_SELECT:
		needsUpdate |= cur.selHandle(cmd.action == LFUN_LINE_END_SELECT);
		needsUpdate |= cursorEnd(cur);
		break;

	case LFUN_WORD_FORWARD:
	case LFUN_WORD_FORWARD_SELECT:
		needsUpdate |= cur.selHandle(cmd.action == LFUN_WORD_FORWARD_SELECT);
		if (reverseDirectionNeeded(cur))
			needsUpdate |= cursorLeftOneWord(cur);
		else
			needsUpdate |= cursorRightOneWord(cur);
		break;

	case LFUN_WORD_BACKWARD:
	case LFUN_WORD_BACKWARD_SELECT:
		needsUpdate |= cur.selHandle(cmd.action == LFUN_WORD_BACKWARD_SELECT);
		if (reverseDirectionNeeded(cur))
			needsUpdate |= cursorRightOneWord(cur);
		else
			needsUpdate |= cursorLeftOneWord(cur);
		break;

	case LFUN_WORD_SELECT: {
		selectWord(cur, WHOLE_WORD);
		finishChange(cur, true);
		break;
	}

	case LFUN_BREAK_LINE: {
		// Not allowed by LaTeX (labels or empty par)
		if (cur.pos() > cur.paragraph().beginOfBody()) {
			// this avoids a double undo
			// FIXME: should not be needed, ideally
			if (!cur.selection())
				recordUndo(cur);
			cap::replaceSelection(cur);
			cur.insert(new InsetNewline);
			cur.posRight();
			moveCursor(cur, false);
		}
		break;
	}

	case LFUN_CHAR_DELETE_FORWARD:
		if (!cur.selection()) {
			if (cur.pos() == cur.paragraph().size())
				// Par boundary, force full-screen update
				singleParUpdate = false;
			needsUpdate |= erase(cur);
			cur.resetAnchor();
			// It is possible to make it a lot faster still
			// just comment out the line below...
		} else {
			cutSelection(cur, true, false);
			singleParUpdate = false;
		}
		moveCursor(cur, false);
		break;

	case LFUN_DELETE_FORWARD_SKIP:
		// Reverse the effect of LFUN_BREAK_PARAGRAPH_SKIP.
		if (!cur.selection()) {
			if (cur.pos() == cur.lastpos()) {
				cursorRight(cur);
				cursorLeft(cur);
			}
			erase(cur);
			cur.resetAnchor();
		} else {
			cutSelection(cur, true, false);
		}
		break;


	case LFUN_CHAR_DELETE_BACKWARD:
		if (!cur.selection()) {
			if (bv->getIntl().getTransManager().backspace()) {
				// Par boundary, full-screen update
				if (cur.pos() == 0)
					singleParUpdate = false;
				needsUpdate |= backspace(cur);
				cur.resetAnchor();
				// It is possible to make it a lot faster still
				// just comment out the line below...
			}
		} else {
			cutSelection(cur, true, false);
			singleParUpdate = false;
		}
		break;

	case LFUN_DELETE_BACKWARD_SKIP:
		// Reverse the effect of LFUN_BREAK_PARAGRAPH_SKIP.
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

	case LFUN_BREAK_PARAGRAPH:
		cap::replaceSelection(cur);
		breakParagraph(cur, cmd.argument() == "inverse");
		cur.resetAnchor();
		break;

	case LFUN_BREAK_PARAGRAPH_KEEP_LAYOUT:
		cap::replaceSelection(cur);
		breakParagraph(cur, true);
		cur.resetAnchor();
		break;

	case LFUN_BREAK_PARAGRAPH_SKIP: {
		// When at the beginning of a paragraph, remove
		// indentation.  Otherwise, do the same as LFUN_BREAK_PARAGRAPH.
		cap::replaceSelection(cur);
		if (cur.pos() == 0)
			cur.paragraph().params().labelWidthString(docstring());
		else
			breakParagraph(cur, false);
		cur.resetAnchor();
		break;
	}

	// TODO
	// With the creation of LFUN_PARAGRAPH_PARAMS, this is now redundant,
	// as its duties can be performed there. Should it be removed??
	// FIXME For now, it can just dispatch LFUN_PARAGRAPH_PARAMS...
	case LFUN_PARAGRAPH_SPACING: {
		Paragraph & par = cur.paragraph();
		Spacing::Space cur_spacing = par.params().spacing().getSpace();
		string cur_value = "1.0";
		if (cur_spacing == Spacing::Other)
			cur_value = par.params().spacing().getValueAsString();

		istringstream is(to_utf8(cmd.argument()));
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
			lyxerr << to_utf8(_("Unknown spacing argument: "))
			       << to_utf8(cmd.argument()) << endl;
		}
		if (cur_spacing != new_spacing || cur_value != new_value)
			par.params().spacing(Spacing(new_spacing, new_value));
		break;
	}

	case LFUN_INSET_INSERT: {
		recordUndo(cur);
		Inset * inset = createInset(bv, cmd);
		if (inset) {
			// FIXME (Abdel 01/02/2006):
			// What follows would be a partial fix for bug 2154:
			//   http://bugzilla.lyx.org/show_bug.cgi?id=2154
			// This automatically put the label inset _after_ a
			// numbered section. It should be possible to extend the mechanism
			// to any kind of LateX environement.
			// The correct way to fix that bug would be at LateX generation.
			// I'll let the code here for reference as it could be used for some
			// other feature like "automatic labelling".
			/*
			Paragraph & par = pars_[cur.pit()];
			if (inset->lyxCode() == Inset::LABEL_CODE
				&& par.layout()->labeltype == LABEL_COUNTER) {
				// Go to the end of the paragraph
				// Warning: Because of Change-Tracking, the last
				// position is 'size()' and not 'size()-1':
				cur.pos() = par.size();
				// Insert a new paragraph
				FuncRequest fr(LFUN_BREAK_PARAGRAPH);
				dispatch(cur, fr);
			}
			*/
			if (cur.selection())
				cutSelection(cur, true, false);
			insertInset(cur, inset);
			cur.posRight();
		}
		break;
	}

	case LFUN_INSET_DISSOLVE:
		needsUpdate |= dissolveInset(cur);
		break;

	case LFUN_INSET_SETTINGS:
		cur.inset().showInsetDialog(bv);
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

	case LFUN_HYPHENATION_POINT_INSERT:
		specialChar(cur, InsetSpecialChar::HYPHENATION);
		break;

	case LFUN_LIGATURE_BREAK_INSERT:
		specialChar(cur, InsetSpecialChar::LIGATURE_BREAK);
		break;

	case LFUN_DOTS_INSERT:
		specialChar(cur, InsetSpecialChar::LDOTS);
		break;

	case LFUN_END_OF_SENTENCE_PERIOD_INSERT:
		specialChar(cur, InsetSpecialChar::END_OF_SENTENCE);
		break;

	case LFUN_MENU_SEPARATOR_INSERT:
		specialChar(cur, InsetSpecialChar::MENU_SEPARATOR);
		break;

	case LFUN_WORD_UPCASE:
		changeCase(cur, Text::text_uppercase);
		break;

	case LFUN_WORD_LOWCASE:
		changeCase(cur, Text::text_lowercase);
		break;

	case LFUN_WORD_CAPITALIZE:
		changeCase(cur, Text::text_capitalization);
		break;

	case LFUN_CHARS_TRANSPOSE:
		charsTranspose(cur);
		break;

	case LFUN_PASTE:
		cur.message(_("Paste"));
		cap::replaceSelection(cur);
		if (cmd.argument().empty() && !theClipboard().isInternal())
			pasteClipboard(cur, bv->buffer()->errorList("Paste"));
		else {
			string const arg(to_utf8(cmd.argument()));
			pasteFromStack(cur, bv->buffer()->errorList("Paste"),
					isStrUnsignedInt(arg) ?
						convert<unsigned int>(arg) :
						0);
		}
		bv->buffer()->errors("Paste");
		cur.clearSelection(); // bug 393
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

	case LFUN_SERVER_GET_XY:
		cur.message(from_utf8(
			convert<string>(cursorX(cur.bv(), cur.top(), cur.boundary()))
			+ ' ' + convert<string>(cursorY(cur.bv(), cur.top(), cur.boundary()))));
		break;

	case LFUN_SERVER_SET_XY: {
		int x = 0;
		int y = 0;
		istringstream is(to_utf8(cmd.argument()));
		is >> x >> y;
		if (!is)
			lyxerr << "SETXY: Could not parse coordinates in '"
			       << to_utf8(cmd.argument()) << std::endl;
		else
			setCursorFromCoordinates(cur, x, y);
		break;
	}

	case LFUN_SERVER_GET_FONT:
		if (current_font.shape() == Font::ITALIC_SHAPE)
			cur.message(from_ascii("E"));
		else if (current_font.shape() == Font::SMALLCAPS_SHAPE)
			cur.message(from_ascii("N"));
		else
			cur.message(from_ascii("0"));
		break;

	case LFUN_SERVER_GET_LAYOUT:
		cur.message(cur.paragraph().layout()->name());
		break;

	case LFUN_LAYOUT: {
		docstring layout = cmd.argument();
		LYXERR(Debug::INFO) << "LFUN_LAYOUT: (arg) " << to_utf8(layout) << endl;

		// Derive layout number from given argument (string)
		// and current buffer's textclass (number)
		TextClass const & tclass = bv->buffer()->params().getTextClass();
		if (layout.empty())
			layout = tclass.defaultLayoutName();
		bool hasLayout = tclass.hasLayout(layout);

		// If the entry is obsolete, use the new one instead.
		if (hasLayout) {
			docstring const & obs = tclass[layout]->obsoleted_by();
			if (!obs.empty())
				layout = obs;
		}

		if (!hasLayout) {
			cur.errorMessage(from_utf8(N_("Layout ")) + cmd.argument() +
				from_utf8(N_(" not known")));
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
			setLayout(cur, layout);
			// inform the GUI that the layout has changed.
			bv->layoutChanged(layout);
		}
		break;
	}

	case LFUN_CLIPBOARD_PASTE:
		cur.clearSelection();
		pasteClipboard(cur, bv->buffer()->errorList("Paste"),
			       cmd.argument() == "paragraph");
		bv->buffer()->errors("Paste");
		break;

	case LFUN_PRIMARY_SELECTION_PASTE:
		pasteString(cur, theSelection().get(),
			    cmd.argument() == "paragraph");
		break;

	case LFUN_UNICODE_INSERT: {
		if (cmd.argument().empty())
			break;
		docstring hexstring = cmd.argument();
		if (lyx::support::isHex(hexstring)) {
			char_type c = lyx::support::hexToInt(hexstring);
			if (c >= 32 && c < 0x10ffff) {
				lyxerr << "Inserting c: " << c << endl;
				docstring s = docstring(1, c);
				lyx::dispatch(FuncRequest(LFUN_SELF_INSERT, s));
			}
		}
		break;
	}

	case LFUN_QUOTE_INSERT: {
		Paragraph & par = cur.paragraph();
		pos_type pos = cur.pos();
		BufferParams const & bufparams = bv->buffer()->params();
		Layout_ptr const & style = par.layout();
		if (!style->pass_thru
		    && par.getFontSettings(bufparams, pos).language()->lang() != "hebrew") {
			// this avoids a double undo
			// FIXME: should not be needed, ideally
			if (!cur.selection())
				recordUndo(cur);
			cap::replaceSelection(cur);
			pos = cur.pos();
			char_type c;
			if (pos == 0)
				c = ' ';
			else if (cur.prevInset() && cur.prevInset()->isSpace())
				c = ' ';
			else
				c = par.getChar(pos - 1);
			string arg = to_utf8(cmd.argument());
			if (arg == "single")
				cur.insert(new InsetQuotes(c,
				    bufparams.quotes_language,
				    InsetQuotes::SingleQ));
			else
				cur.insert(new InsetQuotes(c,
				    bufparams.quotes_language,
				    InsetQuotes::DoubleQ));
			cur.posRight();
		}
		else
			lyx::dispatch(FuncRequest(LFUN_SELF_INSERT, "\""));
		break;
	}

	case LFUN_DATE_INSERT:
		if (cmd.argument().empty())
			lyx::dispatch(FuncRequest(LFUN_SELF_INSERT,
				formatted_time(current_time())));
		else
			lyx::dispatch(FuncRequest(LFUN_SELF_INSERT,
				formatted_time(current_time(), to_utf8(cmd.argument()))));
		break;

	case LFUN_MOUSE_TRIPLE:
		if (cmd.button() == mouse_button::button1) {
			cursorHome(cur);
			cur.resetAnchor();
			cursorEnd(cur);
			cur.setSelection();
			bv->cursor() = cur;
		}
		break;

	case LFUN_MOUSE_DOUBLE:
		if (cmd.button() == mouse_button::button1) {
			selectWord(cur, WHOLE_WORD_STRICT);
			bv->cursor() = cur;
		}
		break;

	// Single-click on work area
	case LFUN_MOUSE_PRESS: {
		// Right click on a footnote flag opens float menu
		if (cmd.button() == mouse_button::button3)
			cur.clearSelection();

		bool do_selection = cmd.button() == mouse_button::button1
			&& cmd.argument() == "region-select";
		// Set the cursor
		bool update = bv->mouseSetCursor(cur, do_selection);

		// Insert primary selection with middle mouse
		// if there is a local selection in the current buffer,
		// insert this
		if (cmd.button() == mouse_button::button2) {
			if (cap::selection()) {
				// Copy the selection buffer to the clipboard
				// stack, because we want it to appear in the
				// "Edit->Paste recent" menu.
				cap::copySelectionToStack();

				cap::pasteSelection(bv->cursor(), 
						    bv->buffer()->errorList("Paste"));
				bv->buffer()->errors("Paste");
				bv->buffer()->markDirty();
				finishUndo();
			} else {
				lyx::dispatch(FuncRequest(LFUN_PRIMARY_SELECTION_PASTE, "paragraph"));
			}
		}

		// we have to update after dEPM triggered
		if (!update && cmd.button() == mouse_button::button1) {
			needsUpdate = false;
			cur.noUpdate();
		}

		break;
	}

	case LFUN_MOUSE_MOTION: {
		// Only use motion with button 1
		//if (cmd.button() != mouse_button::button1)
		//	return false;

		// ignore motions deeper nested than the real anchor
		Cursor & bvcur = cur.bv().cursor();
		if (bvcur.anchor_.hasPart(cur)) {
			CursorSlice old = bvcur.top();

			int const wh = bv->workHeight();
			int const y = std::max(0, std::min(wh - 1, cmd.y));

			setCursorFromCoordinates(cur, cmd.x, y);
			cur.setTargetX(cmd.x);
			if (cmd.y >= wh)
				lyx::dispatch(FuncRequest(LFUN_DOWN_SELECT));
			else if (cmd.y < 0)
				lyx::dispatch(FuncRequest(LFUN_UP_SELECT));
			// This is to allow jumping over large insets
			if (cur.top() == old) {
				if (cmd.y >= wh)
					lyx::dispatch(FuncRequest(LFUN_DOWN_SELECT));
				else if (cmd.y < 0)
					lyx::dispatch(FuncRequest(LFUN_UP_SELECT));
			}

			if (cur.top() == old)
				cur.noUpdate();
			else {
				// don't set anchor_
				bvcur.setCursor(cur);
				bvcur.selection() = true;
				//lyxerr << "MOTION: " << bv->cursor() << endl;
			}

		} else
			cur.undispatched();
		break;
	}

	case LFUN_MOUSE_RELEASE: {
		if (cmd.button() == mouse_button::button2)
			break;

		if (cmd.button() == mouse_button::button1) {
			// if there is new selection, update persistent
			// selection, otherwise, single click does not
			// clear persistent selection buffer
			if (cur.selection()) {
				// finish selection
				// if double click, cur is moved to the end of word by selectWord
				// but bvcur is current mouse position
				Cursor & bvcur = cur.bv().cursor();
				bvcur.selection() = true;
			}
			needsUpdate = false;
			cur.noUpdate();
		}

		break;
	}

	case LFUN_SELF_INSERT: {
		if (cmd.argument().empty())
			break;

		// Automatically delete the currently selected
		// text and replace it with what is being
		// typed in now. Depends on lyxrc settings
		// "auto_region_delete", which defaults to
		// true (on).

		if (lyxrc.auto_region_delete && cur.selection()) {
			pit_type const begpit = cur.selBegin().pit();
			pit_type const endpit = cur.selEnd().pit();
			cutSelection(cur, false, false);
			// When a selection spans multiple paragraphs, the metrics update
			// mechanism sometimes fails to detect that a full update is
			// needed. In this case, we force the full update:
			// (see http://bugzilla.lyx.org/show_bug.cgi?id=4317)
			if (isMainText(*cur.bv().buffer()) && begpit != endpit)
				cur.updateFlags(Update::Force);
		}

		cur.clearSelection();
		Font const old_font = real_current_font;

		docstring::const_iterator cit = cmd.argument().begin();
		docstring::const_iterator end = cmd.argument().end();
		for (; cit != end; ++cit)
			bv->translateAndInsert(*cit, this, cur);

		cur.resetAnchor();
		moveCursor(cur, false);
		break;
	}

	case LFUN_URL_INSERT: {
		InsetCommandParams p("url");
		docstring content;
		if (cur.selection()) {
			content = cur.selectionAsString(false);
			cutSelection(cur, true, false);
		}
		p["target"] = (cmd.argument().empty()) ?
			content : cmd.argument();
		string const data = InsetCommandMailer::params2string("url", p);
		if (p["target"].empty()) {
			bv->showInsetDialog("url", data, 0);
		} else {
			FuncRequest fr(LFUN_INSET_INSERT, data);
			dispatch(cur, fr);
		}
		break;
	}

	case LFUN_HTML_INSERT: {
		InsetCommandParams p("htmlurl");
		docstring content;
		if (cur.selection()) {
			content = cur.selectionAsString(false);
			cutSelection(cur, true, false);
		}
		p["target"] = (cmd.argument().empty()) ?
			content : cmd.argument();
		string const data = InsetCommandMailer::params2string("url", p);
		if (p["target"].empty()) {
			bv->showInsetDialog("url", data, 0);
		} else {
			FuncRequest fr(LFUN_INSET_INSERT, data);
			dispatch(cur, fr);
		}
		break;
	}

	case LFUN_LABEL_INSERT: {
		InsetCommandParams p("label");
		// Try to generate a valid label
		p["name"] = (cmd.argument().empty()) ?
			cur.getPossibleLabel() :
			cmd.argument();
		string const data = InsetCommandMailer::params2string("label", p);

		if (cmd.argument().empty()) {
			bv->showInsetDialog("label", data, 0);
		} else {
			FuncRequest fr(LFUN_INSET_INSERT, data);
			dispatch(cur, fr);
		}
		break;
	}


#if 0
	case LFUN_LIST_INSERT:
	case LFUN_THEOREM_INSERT:
#endif
	case LFUN_CAPTION_INSERT:
		// Open the inset, and move the current selection
		// inside it.
		doInsertInset(cur, this, cmd, true, true);
		cur.posRight();
		updateLabels(*bv->buffer());
		break;
	case LFUN_NOTE_INSERT:
	case LFUN_CHARSTYLE_INSERT:
	case LFUN_BOX_INSERT:
	case LFUN_BRANCH_INSERT:
	case LFUN_BIBITEM_INSERT:
	case LFUN_ERT_INSERT:
	case LFUN_LISTING_INSERT:
	case LFUN_FOOTNOTE_INSERT:
	case LFUN_MARGINALNOTE_INSERT:
	case LFUN_OPTIONAL_INSERT:
	case LFUN_ENVIRONMENT_INSERT:
		// Open the inset, and move the current selection
		// inside it.
		doInsertInset(cur, this, cmd, true, true);
		cur.posRight();
		break;

	case LFUN_TABULAR_INSERT:
		// if there were no arguments, just open the dialog
		if (doInsertInset(cur, this, cmd, false, true))
			cur.posRight();
		else
			bv->showDialog("tabularcreate");

		break;

	case LFUN_FLOAT_INSERT:
	case LFUN_FLOAT_WIDE_INSERT:
	case LFUN_WRAP_INSERT: {
		bool content = cur.selection();  // will some text be moved into the inset?

		doInsertInset(cur, this, cmd, true, true);
		cur.posRight();
		ParagraphList & pars = cur.text()->paragraphs();

		TextClass const & tclass = bv->buffer()->params().getTextClass();

		// add a separate paragraph for the caption inset
		pars.push_back(Paragraph());
		pars.back().setInsetOwner(pars[0].inInset());
		pars.back().layout(tclass.defaultLayout());

		int cap_pit = pars.size() - 1;

		// if an empty inset was created, we create an additional empty
		// paragraph at the bottom so that the user can choose where to put
		// the graphics (or table).
		if (!content) {
			pars.push_back(Paragraph());
			pars.back().setInsetOwner(pars[0].inInset());
			pars.back().layout(tclass.defaultLayout());

		}

		// reposition the cursor to the caption
		cur.pit() = cap_pit;
		cur.pos() = 0;
		// FIXME: This Text/Cursor dispatch handling is a mess!
		// We cannot use Cursor::dispatch here it needs access to up to
		// date metrics.
		FuncRequest cmd_caption(LFUN_CAPTION_INSERT);
		cur.text()->dispatch(cur, cmd_caption);
		cur.updateFlags(Update::Force);
		// FIXME: When leaving the Float (or Wrap) inset we should
		// delete any empty paragraph left above or below the
		// caption.
		break;
	}

	case LFUN_INDEX_INSERT:
	case LFUN_NOMENCL_INSERT: {
		Inset * inset = createInset(&cur.bv(), cmd);
		if (!inset)
			break;
		recordUndo(cur);
		cur.clearSelection();
		insertInset(cur, inset);
		// Show the dialog for the nomenclature entry, since the
		// description entry still needs to be filled in.
		if (cmd.action == LFUN_NOMENCL_INSERT)
			inset->edit(cur, true);
		cur.posRight();
		break;
	}

	case LFUN_INDEX_PRINT:
	case LFUN_NOMENCL_PRINT:
	case LFUN_TOC_INSERT:
	case LFUN_HFILL_INSERT:
	case LFUN_LINE_INSERT:
	case LFUN_PAGEBREAK_INSERT:
	case LFUN_CLEARPAGE_INSERT:
	case LFUN_CLEARDOUBLEPAGE_INSERT:
		// do nothing fancy
		doInsertInset(cur, this, cmd, false, false);
		cur.posRight();
		break;

	case LFUN_DEPTH_DECREMENT:
		changeDepth(cur, DEC_DEPTH);
		break;

	case LFUN_DEPTH_INCREMENT:
		changeDepth(cur, INC_DEPTH);
		break;

	case LFUN_MATH_DISPLAY:
		mathDispatch(cur, cmd, true);
		break;

	case LFUN_MATH_IMPORT_SELECTION:
	case LFUN_MATH_MODE:
		if (cmd.argument() == "on")
			// don't pass "on" as argument
			mathDispatch(cur, FuncRequest(LFUN_MATH_MODE), false);
		else
			mathDispatch(cur, cmd, false);
		break;

	case LFUN_MATH_MACRO:
		if (cmd.argument().empty())
			cur.errorMessage(from_utf8(N_("Missing argument")));
		else {
			string s = to_utf8(cmd.argument());
			string const s1 = token(s, ' ', 1);
			int const nargs = s1.empty() ? 0 : convert<int>(s1);
			string const s2 = token(s, ' ', 2);
			string const type = s2.empty() ? "newcommand" : s2;
			cur.insert(new MathMacroTemplate(from_utf8(token(s, ' ', 0)), nargs, from_utf8(type)));
			//cur.nextInset()->edit(cur, true);
		}
		break;

	// passthrough hat and underscore outside mathed:
	case LFUN_MATH_SUBSCRIPT:
		mathDispatch(cur, FuncRequest(LFUN_SELF_INSERT, "_"), false);
		break;
	case LFUN_MATH_SUPERSCRIPT:
		mathDispatch(cur, FuncRequest(LFUN_SELF_INSERT, "^"), false);
		break;

	case LFUN_MATH_INSERT:
	case LFUN_MATH_MATRIX:
	case LFUN_MATH_DELIM:
	case LFUN_MATH_BIGDELIM: {
		cap::replaceSelection(cur);
		InsetMathHull * inset = new InsetMathHull(hullSimple);
		cur.insert(inset);
		checkAndActivateInset(cur, true);
		BOOST_ASSERT(cur.inMathed());
		inset->dispatch(cur, cmd);
		break;
	}

	case LFUN_FONT_EMPH: {
		Font font(Font::ALL_IGNORE);
		font.setEmph(Font::TOGGLE);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_BOLD: {
		Font font(Font::ALL_IGNORE);
		font.setSeries(Font::BOLD_SERIES);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_NOUN: {
		Font font(Font::ALL_IGNORE);
		font.setNoun(Font::TOGGLE);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_CODE: {
		Font font(Font::ALL_IGNORE);
		font.setFamily(Font::TYPEWRITER_FAMILY); // no good
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_SANS: {
		Font font(Font::ALL_IGNORE);
		font.setFamily(Font::SANS_FAMILY);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_ROMAN: {
		Font font(Font::ALL_IGNORE);
		font.setFamily(Font::ROMAN_FAMILY);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_DEFAULT: {
		Font font(Font::ALL_INHERIT, ignore_language);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_UNDERLINE: {
		Font font(Font::ALL_IGNORE);
		font.setUnderbar(Font::TOGGLE);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_SIZE: {
		Font font(Font::ALL_IGNORE);
		font.setLyXSize(to_utf8(cmd.argument()));
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_LANGUAGE: {
		Language const * lang = languages.getLanguage(to_utf8(cmd.argument()));
		if (!lang)
			break;
		Font font(Font::ALL_IGNORE);
		font.setLanguage(lang);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_FREE_APPLY:
		toggleAndShow(cur, this, freefont, toggleall);
		cur.message(_("Character set"));
		break;

	// Set the freefont using the contents of \param data dispatched from
	// the frontends and apply it at the current cursor location.
	case LFUN_FONT_FREE_UPDATE: {
		Font font;
		bool toggle;
		if (bv_funcs::string2font(to_utf8(cmd.argument()), font, toggle)) {
			freefont = font;
			toggleall = toggle;
			toggleAndShow(cur, this, freefont, toggleall);
			cur.message(_("Character set"));
		}
		break;
	}

	case LFUN_FINISHED_LEFT:
		LYXERR(Debug::DEBUG) << "handle LFUN_FINISHED_LEFT:\n" << cur << endl;
		if (reverseDirectionNeeded(cur))
			++cur.pos();
		break;

	case LFUN_FINISHED_RIGHT:
		LYXERR(Debug::DEBUG) << "handle LFUN_FINISHED_RIGHT:\n" << cur << endl;
		if (!reverseDirectionNeeded(cur))
			++cur.pos();
		break;

	case LFUN_LAYOUT_PARAGRAPH: {
		string data;
		params2string(cur.paragraph(), data);
		data = "show\n" + data;
		bv->showDialogWithData("paragraph", data);
		break;
	}

	case LFUN_PARAGRAPH_UPDATE: {
		string data;
		params2string(cur.paragraph(), data);

		// Will the paragraph accept changes from the dialog?
		bool const accept = !cur.inset().forceDefaultParagraphs(cur.idx());

		data = "update " + convert<string>(accept) + '\n' + data;
		bv->updateDialog("paragraph", data);
		break;
	}

	case LFUN_ACCENT_UMLAUT:
	case LFUN_ACCENT_CIRCUMFLEX:
	case LFUN_ACCENT_GRAVE:
	case LFUN_ACCENT_ACUTE:
	case LFUN_ACCENT_TILDE:
	case LFUN_ACCENT_CEDILLA:
	case LFUN_ACCENT_MACRON:
	case LFUN_ACCENT_DOT:
	case LFUN_ACCENT_UNDERDOT:
	case LFUN_ACCENT_UNDERBAR:
	case LFUN_ACCENT_CARON:
	case LFUN_ACCENT_SPECIAL_CARON:
	case LFUN_ACCENT_BREVE:
	case LFUN_ACCENT_TIE:
	case LFUN_ACCENT_HUNGARIAN_UMLAUT:
	case LFUN_ACCENT_CIRCLE:
	case LFUN_ACCENT_OGONEK:
		theLyXFunc().handleKeyFunc(cmd.action);
		if (!cmd.argument().empty())
			// FIXME: Are all these characters encoded in one byte in utf8?
			bv->translateAndInsert(cmd.argument()[0], this, cur);
		break;

	case LFUN_FLOAT_LIST: {
		TextClass const & tclass = bv->buffer()->params().getTextClass();
		if (tclass.floats().typeExist(to_utf8(cmd.argument()))) {
			recordUndo(cur);
			if (cur.selection())
				cutSelection(cur, true, false);
			breakParagraph(cur);

			if (cur.lastpos() != 0) {
				cursorLeft(cur);
				breakParagraph(cur);
			}

			setLayout(cur, tclass.defaultLayoutName());
			ParagraphParameters p;
			setParagraphs(cur, p);
			insertInset(cur, new InsetFloatList(to_utf8(cmd.argument())));
			cur.posRight();
		} else {
			lyxerr << "Non-existent float type: "
			       << to_utf8(cmd.argument()) << endl;
		}
		break;
	}

	case LFUN_CHANGE_ACCEPT: {
		acceptOrRejectChanges(cur, ACCEPT);
		break;
	}

	case LFUN_CHANGE_REJECT: {
		acceptOrRejectChanges(cur, REJECT);
		break;
	}

	case LFUN_THESAURUS_ENTRY: {
		docstring arg = cmd.argument();
		if (arg.empty()) {
			arg = cur.selectionAsString(false);
			// FIXME
			if (arg.size() > 100 || arg.empty()) {
				// Get word or selection
				selectWordWhenUnderCursor(cur, WHOLE_WORD);
				arg = cur.selectionAsString(false);
			}
		}
		bv->showDialogWithData("thesaurus", to_utf8(arg));
		break;
	}

	case LFUN_PARAGRAPH_PARAMS_APPLY: {
		// Given data, an encoding of the ParagraphParameters
		// generated in the Paragraph dialog, this function sets
		// the current paragraph, or currently selected paragraphs,
		// appropriately. 
		// NOTE: This function overrides all existing settings.
		setParagraphs(cur, cmd.argument());
		cur.message(_("Paragraph layout set"));
		break;
	}
	
	case LFUN_PARAGRAPH_PARAMS: {
		// Given data, an encoding of the ParagraphParameters as we'd
		// find them in a LyX file, this function modifies the current paragraph, 
		// or currently selected paragraphs. 
		// NOTE: This function only modifies, and does not override, existing
		// settings.
		setParagraphs(cur, cmd.argument(), true);
		cur.message(_("Paragraph layout set"));
		break;
	}

	case LFUN_ESCAPE:
		if (cur.selection()) {
			cur.selection() = false;
		} else {
			cur.undispatched();
			cmd = FuncRequest(LFUN_FINISHED_RIGHT);
		}
		break;

	case LFUN_OUTLINE_UP:
		toc::outline(toc::Up, cur);
		setCursor(cur, cur.pit(), 0);
		updateLabels(cur.buffer());
		needsUpdate = true;
		break;

	case LFUN_OUTLINE_DOWN:
		toc::outline(toc::Down, cur);
		setCursor(cur, cur.pit(), 0);
		updateLabels(cur.buffer());
		needsUpdate = true;
		break;

	case LFUN_OUTLINE_IN:
		toc::outline(toc::In, cur);
		updateLabels(cur.buffer());
		needsUpdate = true;
		break;

	case LFUN_OUTLINE_OUT:
		toc::outline(toc::Out, cur);
		updateLabels(cur.buffer());
		needsUpdate = true;
		break;

	default:
		LYXERR(Debug::ACTION)
			<< BOOST_CURRENT_FUNCTION
			<< ": Command " << cmd
			<< " not DISPATCHED by Text" << endl;
		cur.undispatched();
		break;
	}

	needsUpdate |= (cur.pos() != cur.lastpos()) && cur.selection();

	// FIXME: The cursor flag is reset two lines below
	// so we need to check here if some of the LFUN did touch that.
	// for now only Text::erase() and Text::backspace() do that.
	// The plan is to verify all the LFUNs and then to remove this
	// singleParUpdate boolean altogether.
	if (cur.result().update() & Update::Force) {
		singleParUpdate = false;
		needsUpdate = true;
	}

	// FIXME: the following code should go in favor of fine grained
	// update flag treatment.
	if (singleParUpdate) {
		// Inserting characters does not change par height
		ParagraphMetrics const & pms
			= cur.bv().parMetrics(cur.bottom().text(), cur.bottom().pit());
		if (pms.dim().height()
		    == olddim.height()) {
			// if so, update _only_ this paragraph
			cur.updateFlags(Update::SinglePar |
				Update::FitCursor |
				Update::MultiParSel);
			return;
		} else
			needsUpdate = true;
	}

	if (!needsUpdate
	    && &oldTopSlice.inset() == &cur.inset()
	    && oldTopSlice.idx() == cur.idx()
	    && !sel // sel is a backup of cur.selection() at the biginning of the function.
	    && !cur.selection())
		// FIXME: it would be better if we could just do this
		//
		//if (cur.result().update() != Update::FitCursor)
		//	cur.noUpdate();
		//
		// But some LFUNs do not set Update::FitCursor when needed, so we
		// do it for all. This is not very harmfull as FitCursor will provoke
		// a full redraw only if needed but still, a proper review of all LFUN
		// should be done and this needsUpdate boolean can then be removed.
		cur.updateFlags(Update::FitCursor);
	else
		cur.updateFlags(Update::Force | Update::FitCursor);
}


bool Text::getStatus(Cursor & cur, FuncRequest const & cmd,
			FuncStatus & flag) const
{
	BOOST_ASSERT(cur.text() == this);

	Font const & font = real_current_font;
	bool enable = true;
	Inset::Code code = Inset::NO_CODE;

	switch (cmd.action) {

	case LFUN_DEPTH_DECREMENT:
		enable = changeDepthAllowed(cur, DEC_DEPTH);
		break;

	case LFUN_DEPTH_INCREMENT:
		enable = changeDepthAllowed(cur, INC_DEPTH);
		break;

	case LFUN_APPENDIX:
		flag.setOnOff(cur.paragraph().params().startOfAppendix());
		break;

	case LFUN_BIBITEM_INSERT:
		enable = (cur.paragraph().layout()->labeltype == LABEL_BIBLIO
			  && cur.pos() == 0);
		break;

	case LFUN_DIALOG_SHOW_NEW_INSET:
		if (cmd.argument() == "bibitem")
			code = Inset::BIBITEM_CODE;
		else if (cmd.argument() == "bibtex")
			code = Inset::BIBTEX_CODE;
		else if (cmd.argument() == "box")
			code = Inset::BOX_CODE;
		else if (cmd.argument() == "branch")
			code = Inset::BRANCH_CODE;
		else if (cmd.argument() == "citation")
			code = Inset::CITE_CODE;
		else if (cmd.argument() == "ert")
			code = Inset::ERT_CODE;
		else if (cmd.argument() == "external")
			code = Inset::EXTERNAL_CODE;
		else if (cmd.argument() == "float")
			code = Inset::FLOAT_CODE;
		else if (cmd.argument() == "graphics")
			code = Inset::GRAPHICS_CODE;
		else if (cmd.argument() == "include")
			code = Inset::INCLUDE_CODE;
		else if (cmd.argument() == "index")
			code = Inset::INDEX_CODE;
		else if (cmd.argument() == "nomenclature")
			code = Inset::NOMENCL_CODE;
		else if (cmd.argument() == "label")
			code = Inset::LABEL_CODE;
		else if (cmd.argument() == "note")
			code = Inset::NOTE_CODE;
		else if (cmd.argument() == "ref")
			code = Inset::REF_CODE;
		else if (cmd.argument() == "toc")
			code = Inset::TOC_CODE;
		else if (cmd.argument() == "url")
			code = Inset::URL_CODE;
		else if (cmd.argument() == "vspace")
			code = Inset::VSPACE_CODE;
		else if (cmd.argument() == "wrap")
			code = Inset::WRAP_CODE;
		else if (cmd.argument() == "listings")
			code = Inset::LISTINGS_CODE;
		break;

	case LFUN_ERT_INSERT:
		code = Inset::ERT_CODE;
		break;
	case LFUN_LISTING_INSERT:
		code = Inset::LISTINGS_CODE;
		break;
	case LFUN_FOOTNOTE_INSERT:
		code = Inset::FOOT_CODE;
		break;
	case LFUN_TABULAR_INSERT:
		code = Inset::TABULAR_CODE;
		break;
	case LFUN_MARGINALNOTE_INSERT:
		code = Inset::MARGIN_CODE;
		break;
	case LFUN_FLOAT_INSERT:
	case LFUN_FLOAT_WIDE_INSERT:
		code = Inset::FLOAT_CODE;
		break;
	case LFUN_WRAP_INSERT:
		code = Inset::WRAP_CODE;
		break;
	case LFUN_FLOAT_LIST:
		code = Inset::FLOAT_LIST_CODE;
		break;
#if 0
	case LFUN_LIST_INSERT:
		code = Inset::LIST_CODE;
		break;
	case LFUN_THEOREM_INSERT:
		code = Inset::THEOREM_CODE;
		break;
#endif
	case LFUN_CAPTION_INSERT:
		code = Inset::CAPTION_CODE;
		break;
	case LFUN_NOTE_INSERT:
		code = Inset::NOTE_CODE;
		// in commands (sections etc., only Notes are allowed)
		enable = (cmd.argument().empty() || cmd.getArg(0) == "Note" ||
			  !cur.paragraph().layout()->isCommand());
		break;
	case LFUN_CHARSTYLE_INSERT:
		code = Inset::CHARSTYLE_CODE;
		if (cur.buffer().params().getTextClass().charstyles().empty())
			enable = false;
		break;
	case LFUN_BOX_INSERT:
		code = Inset::BOX_CODE;
		break;
	case LFUN_BRANCH_INSERT:
		code = Inset::BRANCH_CODE;
		if (cur.buffer().getMasterBuffer()->params().branchlist().empty())
			enable = false;
		break;
	case LFUN_LABEL_INSERT:
		code = Inset::LABEL_CODE;
		break;
	case LFUN_OPTIONAL_INSERT:
		code = Inset::OPTARG_CODE;
		enable = numberOfOptArgs(cur.paragraph())
			< cur.paragraph().layout()->optionalargs;
		break;
	case LFUN_ENVIRONMENT_INSERT:
		code = Inset::BOX_CODE;
		break;
	case LFUN_INDEX_INSERT:
		code = Inset::INDEX_CODE;
		break;
	case LFUN_INDEX_PRINT:
		code = Inset::INDEX_PRINT_CODE;
		break;
	case LFUN_NOMENCL_INSERT:
		code = Inset::NOMENCL_CODE;
		break;
	case LFUN_NOMENCL_PRINT:
		code = Inset::NOMENCL_PRINT_CODE;
		break;
	case LFUN_TOC_INSERT:
		code = Inset::TOC_CODE;
		break;
	case LFUN_HTML_INSERT:
	case LFUN_URL_INSERT:
		code = Inset::URL_CODE;
		break;
	case LFUN_QUOTE_INSERT:
		// always allow this, since we will inset a raw quote
		// if an inset is not allowed.
		break;
	case LFUN_HYPHENATION_POINT_INSERT:
	case LFUN_LIGATURE_BREAK_INSERT:
	case LFUN_HFILL_INSERT:
	case LFUN_MENU_SEPARATOR_INSERT:
	case LFUN_DOTS_INSERT:
	case LFUN_END_OF_SENTENCE_PERIOD_INSERT:
		code = Inset::SPECIALCHAR_CODE;
		break;
	case LFUN_SPACE_INSERT:
		// slight hack: we know this is allowed in math mode
		if (cur.inTexted())
			code = Inset::SPACE_CODE;
		break;

	case LFUN_INSET_MODIFY:
		// We need to disable this, because we may get called for a
		// tabular cell via
		// InsetTabular::getStatus() -> InsetText::getStatus()
		// and we don't handle LFUN_INSET_MODIFY.
		enable = false;
		break;

	case LFUN_FONT_EMPH:
		flag.setOnOff(font.emph() == Font::ON);
		break;

	case LFUN_FONT_NOUN:
		flag.setOnOff(font.noun() == Font::ON);
		break;

	case LFUN_FONT_BOLD:
		flag.setOnOff(font.series() == Font::BOLD_SERIES);
		break;

	case LFUN_FONT_SANS:
		flag.setOnOff(font.family() == Font::SANS_FAMILY);
		break;

	case LFUN_FONT_ROMAN:
		flag.setOnOff(font.family() == Font::ROMAN_FAMILY);
		break;

	case LFUN_FONT_CODE:
		flag.setOnOff(font.family() == Font::TYPEWRITER_FAMILY);
		break;

	case LFUN_CUT:
	case LFUN_COPY:
		enable = cur.selection();
		break;

	case LFUN_PASTE:
		if (cmd.argument().empty()) {
			if (theClipboard().isInternal())
				enable = cap::numberOfSelections() > 0;
			else
				enable = !theClipboard().empty();
		} else {
			string const arg = to_utf8(cmd.argument());
			if (isStrUnsignedInt(arg)) {
				unsigned int n = convert<unsigned int>(arg);
				enable = cap::numberOfSelections() > n;
			} else
				// unknown argument
				enable = false;
		}
		break;

	case LFUN_CLIPBOARD_PASTE:
		enable = !theClipboard().empty();
		break;

	case LFUN_PRIMARY_SELECTION_PASTE:
		enable = cur.selection() || !theSelection().empty();
		break;

	case LFUN_PARAGRAPH_MOVE_UP:
		enable = cur.pit() > 0 && !cur.selection();
		break;

	case LFUN_PARAGRAPH_MOVE_DOWN:
		enable = cur.pit() < cur.lastpit() && !cur.selection();
		break;

	case LFUN_INSET_DISSOLVE:
		enable = !isMainText(*cur.bv().buffer()) && cur.inset().nargs() == 1;
		break;

	case LFUN_CHANGE_ACCEPT:
	case LFUN_CHANGE_REJECT:
		// TODO: context-sensitive enabling of LFUN_CHANGE_ACCEPT/REJECT
		// In principle, these LFUNs should only be enabled if there
		// is a change at the current position/in the current selection.
		// However, without proper optimizations, this will inevitably
		// result in unacceptable performance - just imagine a user who
		// wants to select the complete content of a long document.
		enable = true;
		break;

	case LFUN_OUTLINE_UP:
	case LFUN_OUTLINE_DOWN:
	case LFUN_OUTLINE_IN:
	case LFUN_OUTLINE_OUT:
		enable = (cur.paragraph().layout()->toclevel != Layout::NOT_IN_TOC);
		break;

	case LFUN_WORD_DELETE_FORWARD:
	case LFUN_WORD_DELETE_BACKWARD:
	case LFUN_LINE_DELETE:
	case LFUN_WORD_FORWARD:
	case LFUN_WORD_BACKWARD:
	case LFUN_CHAR_FORWARD:
	case LFUN_CHAR_FORWARD_SELECT:
	case LFUN_CHAR_BACKWARD:
	case LFUN_CHAR_BACKWARD_SELECT:
	case LFUN_UP:
	case LFUN_UP_SELECT:
	case LFUN_DOWN:
	case LFUN_DOWN_SELECT:
	case LFUN_PARAGRAPH_UP_SELECT:
	case LFUN_PARAGRAPH_DOWN_SELECT:
	case LFUN_SCREEN_UP_SELECT:
	case LFUN_SCREEN_DOWN_SELECT:
	case LFUN_LINE_BEGIN_SELECT:
	case LFUN_LINE_END_SELECT:
	case LFUN_WORD_FORWARD_SELECT:
	case LFUN_WORD_BACKWARD_SELECT:
	case LFUN_WORD_SELECT:
	case LFUN_PARAGRAPH_UP:
	case LFUN_PARAGRAPH_DOWN:
	case LFUN_SCREEN_UP:
	case LFUN_SCREEN_DOWN:
	case LFUN_LINE_BEGIN:
	case LFUN_LINE_END:
	case LFUN_BREAK_LINE:
	case LFUN_CHAR_DELETE_FORWARD:
	case LFUN_DELETE_FORWARD_SKIP:
	case LFUN_CHAR_DELETE_BACKWARD:
	case LFUN_DELETE_BACKWARD_SKIP:
	case LFUN_BREAK_PARAGRAPH:
	case LFUN_BREAK_PARAGRAPH_KEEP_LAYOUT:
	case LFUN_BREAK_PARAGRAPH_SKIP:
	case LFUN_PARAGRAPH_SPACING:
	case LFUN_INSET_INSERT:
	case LFUN_WORD_UPCASE:
	case LFUN_WORD_LOWCASE:
	case LFUN_WORD_CAPITALIZE:
	case LFUN_CHARS_TRANSPOSE:
	case LFUN_SERVER_GET_XY:
	case LFUN_SERVER_SET_XY:
	case LFUN_SERVER_GET_FONT:
	case LFUN_SERVER_GET_LAYOUT:
	case LFUN_LAYOUT:
	case LFUN_DATE_INSERT:
	case LFUN_SELF_INSERT:
	case LFUN_LINE_INSERT:
	case LFUN_PAGEBREAK_INSERT:
	case LFUN_CLEARPAGE_INSERT:
	case LFUN_CLEARDOUBLEPAGE_INSERT:
	case LFUN_MATH_DISPLAY:
	case LFUN_MATH_IMPORT_SELECTION:
	case LFUN_MATH_MODE:
	case LFUN_MATH_MACRO:
	case LFUN_MATH_MATRIX:
	case LFUN_MATH_DELIM:
	case LFUN_MATH_BIGDELIM:
	case LFUN_MATH_INSERT:
	case LFUN_MATH_SUBSCRIPT:
	case LFUN_MATH_SUPERSCRIPT:
	case LFUN_FONT_DEFAULT:
	case LFUN_FONT_UNDERLINE:
	case LFUN_FONT_SIZE:
	case LFUN_LANGUAGE:
	case LFUN_FONT_FREE_APPLY:
	case LFUN_FONT_FREE_UPDATE:
	case LFUN_LAYOUT_PARAGRAPH:
	case LFUN_PARAGRAPH_UPDATE:
	case LFUN_ACCENT_UMLAUT:
	case LFUN_ACCENT_CIRCUMFLEX:
	case LFUN_ACCENT_GRAVE:
	case LFUN_ACCENT_ACUTE:
	case LFUN_ACCENT_TILDE:
	case LFUN_ACCENT_CEDILLA:
	case LFUN_ACCENT_MACRON:
	case LFUN_ACCENT_DOT:
	case LFUN_ACCENT_UNDERDOT:
	case LFUN_ACCENT_UNDERBAR:
	case LFUN_ACCENT_CARON:
	case LFUN_ACCENT_SPECIAL_CARON:
	case LFUN_ACCENT_BREVE:
	case LFUN_ACCENT_TIE:
	case LFUN_ACCENT_HUNGARIAN_UMLAUT:
	case LFUN_ACCENT_CIRCLE:
	case LFUN_ACCENT_OGONEK:
	case LFUN_THESAURUS_ENTRY:
	case LFUN_PARAGRAPH_PARAMS_APPLY:
	case LFUN_PARAGRAPH_PARAMS:
	case LFUN_ESCAPE:
	case LFUN_BUFFER_END:
	case LFUN_BUFFER_BEGIN:
	case LFUN_BUFFER_BEGIN_SELECT:
	case LFUN_BUFFER_END_SELECT:
	case LFUN_UNICODE_INSERT:
		// these are handled in our dispatch()
		enable = true;
		break;

	default:
		return false;
	}

	if (code != Inset::NO_CODE
	    && (cur.empty() || !cur.inset().insetAllowed(code)))
		enable = false;

	flag.enabled(enable);
	return true;
}


void Text::pasteString(Cursor & cur, docstring const & clip,
		bool asParagraphs)
{
	cur.clearSelection();
	if (!clip.empty()) {
		recordUndo(cur);
		if (asParagraphs)
			insertStringAsParagraphs(cur, clip);
		else
			insertStringAsLines(cur, clip);
	}
}

} // namespace lyx
