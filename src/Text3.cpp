/**
 * \file text3.cpp
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

#include "Bidi.h"
#include "BranchList.h"
#include "FloatList.h"
#include "FuncStatus.h"
#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "DispatchResult.h"
#include "ErrorList.h"
#include "factory.h"
#include "FuncRequest.h"
#include "InsetList.h"
#include "Intl.h"
#include "Language.h"
#include "Layout.h"
#include "LyXAction.h"
#include "LyXFunc.h"
#include "Lexer.h"
#include "LyXRC.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "TextClass.h"
#include "TextMetrics.h"
#include "VSpace.h"

#include "frontends/Clipboard.h"
#include "frontends/Selection.h"

#include "insets/InsetCollapsable.h"
#include "insets/InsetCommand.h"
#include "insets/InsetFloatList.h"
#include "insets/InsetNewline.h"
#include "insets/InsetQuotes.h"
#include "insets/InsetSpecialChar.h"
#include "insets/InsetText.h"
#include "insets/InsetGraphics.h"
#include "insets/InsetGraphicsParams.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/lyxtime.h"

#include "mathed/InsetMathHull.h"
#include "mathed/MathMacroTemplate.h"

#include <boost/next_prior.hpp>

#include <clocale>
#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

using cap::copySelection;
using cap::cutSelection;
using cap::pasteFromStack;
using cap::pasteClipboardText;
using cap::pasteClipboardGraphics;
using cap::replaceSelection;

// globals...
static Font freefont(ignore_font, ignore_language);
static bool toggleall = false;

static void toggleAndShow(Cursor & cur, Text * text,
	Font const & font, bool toggleall = true)
{
	text->toggleFree(cur, font, toggleall);

	if (font.language() != ignore_language ||
	    font.fontInfo().number() != FONT_IGNORE) {
		TextMetrics const & tm = cur.bv().textMetrics(text);
		if (cur.boundary() != tm.isRTLBoundary(cur.pit(), cur.pos(),
						       cur.real_current_font))
			text->setCursor(cur, cur.pit(), cur.pos(),
					false, !cur.boundary());
	}
}


static void moveCursor(Cursor & cur, bool selecting)
{
	if (selecting || cur.mark())
		cur.setSelection();
}


static void finishChange(Cursor & cur, bool selecting)
{
	cur.finishUndo();
	moveCursor(cur, selecting);
}


static void mathDispatch(Cursor & cur, FuncRequest const & cmd, bool display)
{
	cur.recordUndo();
	docstring sel = cur.selectionAsString(false);

	// It may happen that sel is empty but there is a selection
	replaceSelection(cur);

	// Is this a valid formula?
	bool valid = true;

	if (sel.empty()) {
#ifdef ENABLE_ASSERTIONS
		const int old_pos = cur.pos();
#endif
		cur.insert(new InsetMathHull(hullSimple));
#ifdef ENABLE_ASSERTIONS
		LASSERT(old_pos == cur.pos(), /**/);
#endif
		cur.nextInset()->edit(cur, true);
		// don't do that also for LFUN_MATH_MODE
		// unless you want end up with always changing
		// to mathrm when opening an inlined inset --
		// I really hate "LyXfunc overloading"...
		if (display)
			cur.dispatch(FuncRequest(LFUN_MATH_DISPLAY));
		// Avoid an unnecessary undo step if cmd.argument
		// is empty
		if (!cmd.argument().empty())
			cur.dispatch(FuncRequest(LFUN_MATH_INSERT,
						 cmd.argument()));
	} else {
		// create a macro if we see "\\newcommand"
		// somewhere, and an ordinary formula
		// otherwise
		if (sel.find(from_ascii("\\newcommand")) == string::npos
				&& sel.find(from_ascii("\\newlyxcommand")) == string::npos
				&& sel.find(from_ascii("\\def")) == string::npos)
		{
			InsetMathHull * formula = new InsetMathHull;
			string const selstr = to_utf8(sel);
			istringstream is(selstr);
			Lexer lex;
			lex.setStream(is);
			formula->readQuiet(lex);
			if (formula->getType() == hullNone) {
				// No valid formula, let's try with delims
				is.str("$" + selstr + "$");
				lex.setStream(is);
				formula->readQuiet(lex);
				if (formula->getType() == hullNone) {
					// Still not valid, leave it as is
					valid = false;
					delete formula;
					cur.insert(sel);
				} else
					cur.insert(formula);
			} else
				cur.insert(formula);
		} else {
			cur.insert(new MathMacroTemplate(sel));
		}
	}
	if (valid)
		cur.message(from_utf8(N_("Math editor mode")));
	else
		cur.message(from_utf8(N_("No valid math formula")));
}


static void specialChar(Cursor & cur, InsetSpecialChar::Kind kind)
{
	cur.recordUndo();
	cap::replaceSelection(cur);
	cur.insert(new InsetSpecialChar(kind));
	cur.posForward();
}


static bool doInsertInset(Cursor & cur, Text * text,
	FuncRequest const & cmd, bool edit, bool pastesel)
{
	Buffer & buffer = cur.bv().buffer();
	BufferParams const & bparams = buffer.params();
	Inset * inset = createInset(buffer, cmd);
	if (!inset)
		return false;

	if (InsetCollapsable * ci = inset->asInsetCollapsable())
		ci->setLayout(bparams);

	cur.recordUndo();
	if (cmd.action == LFUN_INDEX_INSERT) {
		docstring ds = subst(text->getStringToIndex(cur), '\n', ' ');
		text->insertInset(cur, inset);
		if (edit)
			inset->edit(cur, true);
		// Now put this into inset
		static_cast<InsetCollapsable *>(inset)->text().insertStringAsParagraphs(cur, ds);
		return true;
	}

	bool gotsel = false;
	if (cur.selection()) {
		cutSelection(cur, false, pastesel);
		cur.clearSelection();
		gotsel = true;
	}
	text->insertInset(cur, inset);

	if (edit)
		inset->edit(cur, true);

	if (!gotsel || !pastesel)
		return true;

	pasteFromStack(cur, cur.buffer().errorList("Paste"), 0);
	cur.buffer().errors("Paste");
	cur.clearSelection(); // bug 393
	cur.finishUndo();
	InsetText * insetText = dynamic_cast<InsetText *>(inset);
	if (insetText && (!insetText->allowMultiPar() || cur.lastpit() == 0)) {
		// reset first par to default
		cur.text()->paragraphs().begin()
			->setPlainOrDefaultLayout(bparams.documentClass());
		cur.pos() = 0;
		cur.pit() = 0;
		// Merge multiple paragraphs -- hack
		while (cur.lastpit() > 0)
			mergeParagraph(bparams, cur.text()->paragraphs(), 0);
		cur.leaveInset(*inset);
	} else {
		cur.leaveInset(*inset);
		// reset surrounding par to default
		DocumentClass const & dc = bparams.documentClass();
		docstring const layoutname = inset->usePlainLayout()
			? dc.plainLayoutName()
			: dc.defaultLayoutName();
		text->setLayout(cur, layoutname);
	}

	return true;
}


string const freefont2string()
{
	return freefont.toString(toggleall);
}


/// the type of outline operation
enum OutlineOp {
	OutlineUp, // Move this header with text down
	OutlineDown,   // Move this header with text up
	OutlineIn, // Make this header deeper
	OutlineOut // Make this header shallower
};


static void outline(OutlineOp mode, Cursor & cur)
{
	Buffer & buf = cur.buffer();
	pit_type & pit = cur.pit();
	ParagraphList & pars = buf.text().paragraphs();
	ParagraphList::iterator bgn = pars.begin();
	// The first paragraph of the area to be copied:
	ParagraphList::iterator start = boost::next(bgn, pit);
	// The final paragraph of area to be copied:
	ParagraphList::iterator finish = start;
	ParagraphList::iterator end = pars.end();

	DocumentClass const & tc = buf.params().documentClass();

	int const thistoclevel = start->layout().toclevel;
	int toclevel;

	// Move out (down) from this section header
	if (finish != end)
		++finish;
	// Seek the one (on same level) below
	for (; finish != end; ++finish) {
		toclevel = finish->layout().toclevel;
		if (toclevel != Layout::NOT_IN_TOC && toclevel <= thistoclevel) {
			break;
		}
	}

	switch (mode) {
		case OutlineUp: {
			if (start == pars.begin())
				// Nothing to move.
				return;
			ParagraphList::iterator dest = start;
			// Move out (up) from this header
			if (dest == bgn)
				return;
			// Search previous same-level header above
			do {
				--dest;
				toclevel = dest->layout().toclevel;
			} while(dest != bgn
				&& (toclevel == Layout::NOT_IN_TOC
				    || toclevel > thistoclevel));
			// Not found; do nothing
			if (toclevel == Layout::NOT_IN_TOC || toclevel > thistoclevel)
				return;
			pit_type const newpit = distance(bgn, dest);
			pit_type const len = distance(start, finish);
			pit_type const deletepit = pit + len;
			buf.undo().recordUndo(cur, ATOMIC_UNDO, newpit, deletepit - 1);
			pars.insert(dest, start, finish);
			start = boost::next(pars.begin(), deletepit);
			pit = newpit;
			pars.erase(start, finish);
			return;
		}
		case OutlineDown: {
			if (finish == end)
				// Nothing to move.
				return;
			// Go one down from *this* header:
			ParagraphList::iterator dest = boost::next(finish, 1);
			// Go further down to find header to insert in front of:
			for (; dest != end; ++dest) {
				toclevel = dest->layout().toclevel;
				if (toclevel != Layout::NOT_IN_TOC
				    && toclevel <= thistoclevel) {
					break;
				}
			}
			// One such was found:
			pit_type newpit = distance(bgn, dest);
			pit_type const len = distance(start, finish);
			buf.undo().recordUndo(cur, ATOMIC_UNDO, pit, newpit - 1);
			pars.insert(dest, start, finish);
			start = boost::next(bgn, pit);
			pit = newpit - len;
			pars.erase(start, finish);
			return;
		}
		case OutlineIn: {
			pit_type const len = distance(start, finish);
			buf.undo().recordUndo(cur, ATOMIC_UNDO, pit, pit + len - 1);
			for (; start != finish; ++start) {
				toclevel = start->layout().toclevel;
				if (toclevel == Layout::NOT_IN_TOC)
					continue;
				DocumentClass::const_iterator lit = tc.begin();
				DocumentClass::const_iterator len = tc.end();
				for (; lit != len; ++lit) {
					if (lit->toclevel == toclevel + 1 &&
					    start->layout().labeltype == lit->labeltype) {
						start->setLayout(*lit);
						break;
					}
				}
			}
			return;
		}
		case OutlineOut: {
			pit_type const len = distance(start, finish);
			buf.undo().recordUndo(cur, ATOMIC_UNDO, pit, pit + len - 1);
			for (; start != finish; ++start) {
				toclevel = start->layout().toclevel;
				if (toclevel == Layout::NOT_IN_TOC)
					continue;
				DocumentClass::const_iterator lit = tc.begin();
				DocumentClass::const_iterator len = tc.end();
				for (; lit != len; ++lit) {
					if (lit->toclevel == toclevel - 1 &&
						start->layout().labeltype == lit->labeltype) {
							start->setLayout(*lit);
							break;
					}
				}
			}
			return;
		}
	}
}


void Text::number(Cursor & cur)
{
	FontInfo font = ignore_font;
	font.setNumber(FONT_TOGGLE);
	toggleAndShow(cur, this, Font(font, ignore_language));
}


bool Text::isRTL(Buffer const & buffer, Paragraph const & par) const
{
	return par.isRTL(buffer.params());
}


void Text::dispatch(Cursor & cur, FuncRequest & cmd)
{
	LYXERR(Debug::ACTION, "Text::dispatch: cmd: " << cmd);

	BufferView * bv = &cur.bv();
	TextMetrics & tm = bv->textMetrics(this);
	if (!tm.contains(cur.pit()))
		lyx::dispatch(FuncRequest(LFUN_SCREEN_RECENTER));

	// FIXME: We use the update flag to indicates wether a singlePar or a
	// full screen update is needed. We reset it here but shall we restore it
	// at the end?
	cur.noUpdate();

	LASSERT(cur.text() == this, /**/);
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

	switch (cmd.action) {

	case LFUN_PARAGRAPH_MOVE_DOWN: {
		pit_type const pit = cur.pit();
		recUndo(cur, pit, pit + 1);
		cur.finishUndo();
		swap(pars_[pit], pars_[pit + 1]);
		updateLabels(cur.buffer());
		needsUpdate = true;
		++cur.pit();
		break;
	}

	case LFUN_PARAGRAPH_MOVE_UP: {
		pit_type const pit = cur.pit();
		recUndo(cur, pit - 1, pit);
		cur.finishUndo();
		swap(pars_[pit], pars_[pit - 1]);
		updateLabels(cur.buffer());
		--cur.pit();
		needsUpdate = true;
		break;
	}

	case LFUN_APPENDIX: {
		Paragraph & par = cur.paragraph();
		bool start = !par.params().startOfAppendix();

// FIXME: The code below only makes sense at top level.
// Should LFUN_APPENDIX be restricted to top-level paragraphs?
		// ensure that we have only one start_of_appendix in this document
		// FIXME: this don't work for multipart document!
		for (pit_type tmp = 0, end = pars_.size(); tmp != end; ++tmp) {
			if (pars_[tmp].params().startOfAppendix()) {
				recUndo(cur, tmp);
				pars_[tmp].params().startOfAppendix(false);
				break;
			}
		}

		cur.recordUndo();
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
			tm.deleteLineForward(cur);
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
		cur.updateFlags(Update::FitCursor);
		break;

	case LFUN_BUFFER_END:
	case LFUN_BUFFER_END_SELECT:
		needsUpdate |= cur.selHandle(cmd.action == LFUN_BUFFER_END_SELECT);
		if (cur.depth() == 1) {
			needsUpdate |= cursorBottom(cur);
		} else {
			cur.undispatched();
		}
		cur.updateFlags(Update::FitCursor);
		break;

	case LFUN_CHAR_FORWARD:
	case LFUN_CHAR_FORWARD_SELECT:
		//LYXERR0(" LFUN_CHAR_FORWARD[SEL]:\n" << cur);
		needsUpdate |= cur.selHandle(cmd.action == LFUN_CHAR_FORWARD_SELECT);
		needsUpdate |= cursorForward(cur);

		if (!needsUpdate && oldTopSlice == cur.top()
				&& cur.boundary() == oldBoundary) {
			cur.undispatched();
			cmd = FuncRequest(LFUN_FINISHED_FORWARD);
		}
		break;

	case LFUN_CHAR_BACKWARD:
	case LFUN_CHAR_BACKWARD_SELECT:
		//lyxerr << "handle LFUN_CHAR_BACKWARD[_SELECT]:\n" << cur << endl;
		needsUpdate |= cur.selHandle(cmd.action == LFUN_CHAR_BACKWARD_SELECT);
		needsUpdate |= cursorBackward(cur);

		if (!needsUpdate && oldTopSlice == cur.top()
			&& cur.boundary() == oldBoundary) {
			cur.undispatched();
			cmd = FuncRequest(LFUN_FINISHED_BACKWARD);
		}
		break;

	case LFUN_CHAR_LEFT:
	case LFUN_CHAR_LEFT_SELECT:
		if (lyxrc.visual_cursor) {
			needsUpdate |= cur.selHandle(cmd.action == LFUN_CHAR_LEFT_SELECT);
			needsUpdate |= cursorVisLeft(cur);
			if (!needsUpdate && oldTopSlice == cur.top()
					&& cur.boundary() == oldBoundary) {
				cur.undispatched();
				cmd = FuncRequest(LFUN_FINISHED_LEFT);
			}
		} else {
			if (reverseDirectionNeeded(cur)) {
				cmd.action = cmd.action == LFUN_CHAR_LEFT_SELECT ?
					LFUN_CHAR_FORWARD_SELECT : LFUN_CHAR_FORWARD;
			} else {
				cmd.action = cmd.action == LFUN_CHAR_LEFT_SELECT ?
					LFUN_CHAR_BACKWARD_SELECT : LFUN_CHAR_BACKWARD;
			}
			dispatch(cur, cmd);
			return;
		}
		break;

	case LFUN_CHAR_RIGHT:
	case LFUN_CHAR_RIGHT_SELECT:
		if (lyxrc.visual_cursor) {
			needsUpdate |= cur.selHandle(cmd.action == LFUN_CHAR_RIGHT_SELECT);
			needsUpdate |= cursorVisRight(cur);
			if (!needsUpdate && oldTopSlice == cur.top()
					&& cur.boundary() == oldBoundary) {
				cur.undispatched();
				cmd = FuncRequest(LFUN_FINISHED_RIGHT);
			}
		} else {
			if (reverseDirectionNeeded(cur)) {
				cmd.action = cmd.action == LFUN_CHAR_RIGHT_SELECT ?
					LFUN_CHAR_BACKWARD_SELECT : LFUN_CHAR_BACKWARD;
			} else {
				cmd.action = cmd.action == LFUN_CHAR_RIGHT_SELECT ?
					LFUN_CHAR_FORWARD_SELECT : LFUN_CHAR_FORWARD;
			}
			dispatch(cur, cmd);
			return;
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

	case LFUN_LINE_BEGIN:
	case LFUN_LINE_BEGIN_SELECT:
		needsUpdate |= cur.selHandle(cmd.action == LFUN_LINE_BEGIN_SELECT);
		needsUpdate |= tm.cursorHome(cur);
		break;

	case LFUN_LINE_END:
	case LFUN_LINE_END_SELECT:
		needsUpdate |= cur.selHandle(cmd.action == LFUN_LINE_END_SELECT);
		needsUpdate |= tm.cursorEnd(cur);
		break;

	case LFUN_WORD_RIGHT:
	case LFUN_WORD_RIGHT_SELECT:
		if (lyxrc.visual_cursor) {
			needsUpdate |= cur.selHandle(cmd.action == LFUN_WORD_RIGHT_SELECT);
			needsUpdate |= cursorVisRightOneWord(cur);
			if (!needsUpdate && oldTopSlice == cur.top()
					&& cur.boundary() == oldBoundary) {
				cur.undispatched();
				cmd = FuncRequest(LFUN_FINISHED_RIGHT);
			}
		} else {
			if (reverseDirectionNeeded(cur)) {
				cmd.action = cmd.action == LFUN_WORD_RIGHT_SELECT ?
						LFUN_WORD_BACKWARD_SELECT : LFUN_WORD_BACKWARD;
			} else {
				cmd.action = cmd.action == LFUN_WORD_RIGHT_SELECT ?
						LFUN_WORD_FORWARD_SELECT : LFUN_WORD_FORWARD;
			}
			dispatch(cur, cmd);
			return;
		}
		break;

	case LFUN_WORD_FORWARD:
	case LFUN_WORD_FORWARD_SELECT:
		needsUpdate |= cur.selHandle(cmd.action == LFUN_WORD_FORWARD_SELECT);
		needsUpdate |= cursorForwardOneWord(cur);
		break;

	case LFUN_WORD_LEFT:
	case LFUN_WORD_LEFT_SELECT:
		if (lyxrc.visual_cursor) {
			needsUpdate |= cur.selHandle(cmd.action == LFUN_WORD_LEFT_SELECT);
			needsUpdate |= cursorVisLeftOneWord(cur);
			if (!needsUpdate && oldTopSlice == cur.top()
					&& cur.boundary() == oldBoundary) {
				cur.undispatched();
				cmd = FuncRequest(LFUN_FINISHED_LEFT);
			}
		} else {
			if (reverseDirectionNeeded(cur)) {
				cmd.action = cmd.action == LFUN_WORD_LEFT_SELECT ?
						LFUN_WORD_FORWARD_SELECT : LFUN_WORD_FORWARD;
			} else {
				cmd.action = cmd.action == LFUN_WORD_LEFT_SELECT ?
						LFUN_WORD_BACKWARD_SELECT : LFUN_WORD_BACKWARD;
			}
			dispatch(cur, cmd);
			return;
		}
		break;

	case LFUN_WORD_BACKWARD:
	case LFUN_WORD_BACKWARD_SELECT:
		needsUpdate |= cur.selHandle(cmd.action == LFUN_WORD_BACKWARD_SELECT);
		needsUpdate |= cursorBackwardOneWord(cur);
		break;

	case LFUN_WORD_SELECT: {
		selectWord(cur, WHOLE_WORD);
		finishChange(cur, true);
		break;
	}

	case LFUN_NEWLINE_INSERT: {
		InsetNewlineParams inp;
		docstring arg = cmd.argument();
		// this avoids a double undo
		// FIXME: should not be needed, ideally
		if (!cur.selection())
			cur.recordUndo();
		cap::replaceSelection(cur);
		if (arg == "linebreak")
			inp.kind = InsetNewlineParams::LINEBREAK;
		else
			inp.kind = InsetNewlineParams::NEWLINE;
		cur.insert(new InsetNewline(inp));
		cur.posForward();
		moveCursor(cur, false);
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

	case LFUN_BREAK_PARAGRAPH:
		cap::replaceSelection(cur);
		breakParagraph(cur, cmd.argument() == "inverse");
		cur.resetAnchor();
		break;

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
		cur.recordUndo();
		Inset * inset = createInset(bv->buffer(), cmd);
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
			if (inset->lyxCode() == LABEL_CODE
				&& par.layout().labeltype == LABEL_COUNTER) {
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
			cur.insert(inset);
			cur.posForward();
		}
		break;
	}

	case LFUN_INSET_DISSOLVE:
		needsUpdate |= dissolveInset(cur);
		break;

	case LFUN_INSET_SETTINGS: {
		Inset & inset = cur.inset();
		if (cmd.getArg(0) == insetName(inset.lyxCode())) {
			// This inset dialog has been explicitely requested.
			inset.showInsetDialog(bv);
			break;
		}
		// else, if there is an inset at the cursor, access this
		Inset * next_inset = cur.nextInset();
		if (next_inset) {
			next_inset->showInsetDialog(bv);
			break;
		}
		// if not then access the underlying inset.
		inset.showInsetDialog(bv);
		break;
	}

	case LFUN_SET_GRAPHICS_GROUP: {
		InsetGraphics * ins = graphics::getCurrentGraphicsInset(cur);
		if (!ins)
			break;

		cur.recordUndo();

		string id = to_utf8(cmd.argument());
		string grp = graphics::getGroupParams(bv->buffer(), id);
		InsetGraphicsParams tmp, inspar = ins->getParams();

		if (id.empty())
			inspar.groupId = to_utf8(cmd.argument());
		else {
			InsetGraphics::string2params(grp, bv->buffer(), tmp);
			tmp.filename = inspar.filename;
			inspar = tmp;
		}

		ins->setParams(inspar);
	}

	case LFUN_SPACE_INSERT:
		if (cur.paragraph().layout().free_spacing)
			insertChar(cur, ' ');
		else {
			doInsertInset(cur, this, cmd, false, false);
			cur.posForward();
		}
		moveCursor(cur, false);
		break;

	case LFUN_SPECIALCHAR_INSERT: {
		string const name = to_utf8(cmd.argument());
		if (name == "hyphenation")
			specialChar(cur, InsetSpecialChar::HYPHENATION);
		else if (name == "ligature-break")
			specialChar(cur, InsetSpecialChar::LIGATURE_BREAK);
		else if (name == "slash")
			specialChar(cur, InsetSpecialChar::SLASH);
		else if (name == "nobreakdash")
			specialChar(cur, InsetSpecialChar::NOBREAKDASH);
		else if (name == "dots")
			specialChar(cur, InsetSpecialChar::LDOTS);
		else if (name == "end-of-sentence")
			specialChar(cur, InsetSpecialChar::END_OF_SENTENCE);
		else if (name == "menu-separator")
			specialChar(cur, InsetSpecialChar::MENU_SEPARATOR);
		else if (name.empty())
			lyxerr << "LyX function 'specialchar-insert' needs an argument." << endl;
		else
			lyxerr << "Wrong argument for LyX function 'specialchar-insert'." << endl;
		break;
	}

	case LFUN_WORD_UPCASE:
		changeCase(cur, text_uppercase);
		break;

	case LFUN_WORD_LOWCASE:
		changeCase(cur, text_lowercase);
		break;

	case LFUN_WORD_CAPITALIZE:
		changeCase(cur, text_capitalization);
		break;

	case LFUN_CHARS_TRANSPOSE:
		charsTranspose(cur);
		break;

	case LFUN_PASTE: {
		cur.message(_("Paste"));
		LASSERT(cur.selBegin().idx() == cur.selEnd().idx(), /**/);
		cap::replaceSelection(cur);

		// without argument?
		string const arg = to_utf8(cmd.argument());
		if (arg.empty()) {
			if (theClipboard().isInternal())
				pasteFromStack(cur, bv->buffer().errorList("Paste"), 0);
			else if (theClipboard().hasGraphicsContents())
				pasteClipboardGraphics(cur, bv->buffer().errorList("Paste"));
			else
				pasteClipboardText(cur, bv->buffer().errorList("Paste"));
		} else if (isStrUnsignedInt(arg)) {
			// we have a numerical argument
			pasteFromStack(cur, bv->buffer().errorList("Paste"),
				       convert<unsigned int>(arg));
		} else {
			Clipboard::GraphicsType type = Clipboard::AnyGraphicsType;
			if (arg == "pdf")
				type = Clipboard::PdfGraphicsType;
			else if (arg == "png")
				type = Clipboard::PngGraphicsType;
			else if (arg == "jpeg")
				type = Clipboard::JpegGraphicsType;
			else if (arg == "linkback")
				type = Clipboard::LinkBackGraphicsType;
			else
				LASSERT(false, /**/);

			pasteClipboardGraphics(cur, bv->buffer().errorList("Paste"), type);
		}

		bv->buffer().errors("Paste");
		cur.clearSelection(); // bug 393
		cur.finishUndo();
		break;
	}

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
			convert<string>(tm.cursorX(cur.top(), cur.boundary()))
			+ ' ' + convert<string>(tm.cursorY(cur.top(), cur.boundary()))));
		break;

	case LFUN_SERVER_SET_XY: {
		int x = 0;
		int y = 0;
		istringstream is(to_utf8(cmd.argument()));
		is >> x >> y;
		if (!is)
			lyxerr << "SETXY: Could not parse coordinates in '"
			       << to_utf8(cmd.argument()) << endl;
		else
			tm.setCursorFromCoordinates(cur, x, y);
		break;
	}

	case LFUN_SERVER_GET_LAYOUT:
		cur.message(cur.paragraph().layout().name());
		break;

	case LFUN_LAYOUT: {
		docstring layout = cmd.argument();
		LYXERR(Debug::INFO, "LFUN_LAYOUT: (arg) " << to_utf8(layout));

		Paragraph const & para = cur.paragraph();
		docstring const old_layout = para.layout().name();
		DocumentClass const & tclass = bv->buffer().params().documentClass();

		if (layout.empty())
			layout = tclass.defaultLayoutName();

		if (para.forcePlainLayout())
			// in this case only the empty layout is allowed
			layout = tclass.plainLayoutName();
		else if (para.usePlainLayout()) {
			// in this case, default layout maps to empty layout
			if (layout == tclass.defaultLayoutName())
				layout = tclass.plainLayoutName();
		} else {
			// otherwise, the empty layout maps to the default
			if (layout == tclass.plainLayoutName())
				layout = tclass.defaultLayoutName();
		}

		bool hasLayout = tclass.hasLayout(layout);

		// If the entry is obsolete, use the new one instead.
		if (hasLayout) {
			docstring const & obs = tclass[layout].obsoleted_by();
			if (!obs.empty())
				layout = obs;
		}

		if (!hasLayout) {
			cur.errorMessage(from_utf8(N_("Layout ")) + cmd.argument() +
				from_utf8(N_(" not known")));
			break;
		}

		bool change_layout = (old_layout != layout);

		if (!change_layout && cur.selection() &&
			cur.selBegin().pit() != cur.selEnd().pit())
		{
			pit_type spit = cur.selBegin().pit();
			pit_type epit = cur.selEnd().pit() + 1;
			while (spit != epit) {
				if (pars_[spit].layout().name() != old_layout) {
					change_layout = true;
					break;
				}
				++spit;
			}
		}

		if (change_layout)
			setLayout(cur, layout);

		break;
	}

	case LFUN_CLIPBOARD_PASTE:
		cur.clearSelection();
		pasteClipboardText(cur, bv->buffer().errorList("Paste"),
			       cmd.argument() == "paragraph");
		bv->buffer().errors("Paste");
		break;

	case LFUN_PRIMARY_SELECTION_PASTE:
		pasteString(cur, theSelection().get(),
			    cmd.argument() == "paragraph");
		break;

	case LFUN_UNICODE_INSERT: {
		if (cmd.argument().empty())
			break;
		docstring hexstring = cmd.argument();
		if (isHex(hexstring)) {
			char_type c = hexToInt(hexstring);
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
		BufferParams const & bufparams = bv->buffer().params();
		Layout const & style = par.layout();
		if (!style.pass_thru
		    && par.getFontSettings(bufparams, pos).language()->lang() != "hebrew") {
			// this avoids a double undo
			// FIXME: should not be needed, ideally
			if (!cur.selection())
				cur.recordUndo();
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
			cur.insert(new InsetQuotes(bv->buffer(), c, (arg == "single")
				? InsetQuotes::SingleQuotes : InsetQuotes::DoubleQuotes));
			cur.posForward();
		}
		else
			lyx::dispatch(FuncRequest(LFUN_SELF_INSERT, "\""));
		break;
	}

	case LFUN_DATE_INSERT: {
		string const format = cmd.argument().empty()
			? lyxrc.date_insert_format : to_utf8(cmd.argument());
		string const time = formatted_time(current_time(), format);
		lyx::dispatch(FuncRequest(LFUN_SELF_INSERT, time));
		break;
	}

	case LFUN_MOUSE_TRIPLE:
		if (cmd.button() == mouse_button::button1) {
			tm.cursorHome(cur);
			cur.resetAnchor();
			tm.cursorEnd(cur);
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
	case LFUN_MOUSE_PRESS:
		// We are not marking a selection with the keyboard in any case.
		cur.bv().cursor().setMark(false);
		switch (cmd.button()) {
		case mouse_button::button1:
			// Set the cursor
			if (!bv->mouseSetCursor(cur, cmd.argument() == "region-select"))
				cur.updateFlags(Update::SinglePar | Update::FitCursor);
			break;

		case mouse_button::button2:
			// Middle mouse pasting.
			bv->mouseSetCursor(cur);
			if (!cap::selection()) {
				// There is no local selection in the current buffer, so try to
				// paste primary selection instead.
				lyx::dispatch(FuncRequest(LFUN_PRIMARY_SELECTION_PASTE,
					"paragraph"));
				// Nothing else to do.
				cur.noUpdate();
				return;
			}
			// Copy the selection buffer to the clipboard stack, because we want it
			// to appear in the "Edit->Paste recent" menu.
			cap::copySelectionToStack();
			cap::pasteSelection(bv->cursor(), bv->buffer().errorList("Paste"));
			cur.updateFlags(Update::Force | Update::FitCursor);
			bv->buffer().errors("Paste");
			bv->buffer().markDirty();
			bv->cursor().finishUndo();
			break;

		case mouse_button::button3: {
			Cursor const & bvcur = cur.bv().cursor();
			// Don't do anything if we right-click a
			// selection, a context menu will popup.
			if (bvcur.selection() && cur >= bvcur.selectionBegin()
			    && cur < bvcur.selectionEnd()) {
				cur.noUpdate();
				return;
			}
			if (!bv->mouseSetCursor(cur, false))
				cur.updateFlags(Update::SinglePar | Update::FitCursor);
			break;
		}

		default:
			break;
		} // switch (cmd.button())
		break;

	case LFUN_MOUSE_MOTION: {
		// Mouse motion with right or middle mouse do nothing for now.
		if (cmd.button() != mouse_button::button1) {
			cur.noUpdate();
			return;
		}
		// ignore motions deeper nested than the real anchor
		Cursor & bvcur = cur.bv().cursor();
		if (!bvcur.anchor_.hasPart(cur)) {
			cur.undispatched();
			break;
		}
		CursorSlice old = bvcur.top();

		int const wh = bv->workHeight();
		int const y = max(0, min(wh - 1, cmd.y));

		tm.setCursorFromCoordinates(cur, cmd.x, y);
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
		// We continue with our existing selection or start a new one, so don't
		// reset the anchor.
		bvcur.setCursor(cur);
		bvcur.setSelection(true);
		if (cur.top() == old) {
			// We didn't move one iota, so no need to update the screen.
			cur.updateFlags(Update::SinglePar | Update::FitCursor);
			//cur.noUpdate();
			return;
		}
		break;
	}

	case LFUN_MOUSE_RELEASE:
		switch (cmd.button()) {
		case mouse_button::button1:
			// Cursor was set at LFUN_MOUSE_PRESS or LFUN_MOUSE_MOTION time.
			// If there is a new selection, update persistent selection;
			// otherwise, single click does not clear persistent selection
			// buffer.
			if (cur.selection()) {
				// Finish selection. If double click,
				// cur is moved to the end of word by
				// selectWord but bvcur is current
				// mouse position.
				cur.bv().cursor().setSelection();
				// We might have removed an empty but drawn selection
				// (probably a margin)
				cur.updateFlags(Update::SinglePar | Update::FitCursor);
			} else
				cur.noUpdate();
			// FIXME: We could try to handle drag and drop of selection here.
			return;

		case mouse_button::button2:
			// Middle mouse pasting is handled at mouse press time,
			// see LFUN_MOUSE_PRESS.
			cur.noUpdate();
			return;

		case mouse_button::button3:
			// Cursor was set at LFUN_MOUSE_PRESS time.
			// FIXME: If there is a selection we could try to handle a special
			// drag & drop context menu.
			cur.noUpdate();
			return;

		case mouse_button::none:
		case mouse_button::button4:
		case mouse_button::button5:
			break;
		} // switch (cmd.button())

		break;

	case LFUN_SELF_INSERT: {
		if (cmd.argument().empty())
			break;

		// Automatically delete the currently selected
		// text and replace it with what is being
		// typed in now. Depends on lyxrc settings
		// "auto_region_delete", which defaults to
		// true (on).

		if (lyxrc.auto_region_delete && cur.selection())
			cutSelection(cur, false, false);

		cur.clearSelection();

		docstring::const_iterator cit = cmd.argument().begin();
		docstring::const_iterator const end = cmd.argument().end();
		for (; cit != end; ++cit)
			bv->translateAndInsert(*cit, this, cur);

		cur.resetAnchor();
		moveCursor(cur, false);
		break;
	}

	case LFUN_HYPERLINK_INSERT: {
		InsetCommandParams p(HYPERLINK_CODE);
		docstring content;
		if (cur.selection()) {
			content = cur.selectionAsString(false);
			cutSelection(cur, true, false);
		}
		p["target"] = (cmd.argument().empty()) ?
			content : cmd.argument();
		string const data = InsetCommand::params2string("href", p);
		if (p["target"].empty()) {
			bv->showDialog("href", data);
		} else {
			FuncRequest fr(LFUN_INSET_INSERT, data);
			dispatch(cur, fr);
		}
		break;
	}

	case LFUN_LABEL_INSERT: {
		InsetCommandParams p(LABEL_CODE);
		// Try to generate a valid label
		p["name"] = (cmd.argument().empty()) ?
			cur.getPossibleLabel() :
			cmd.argument();
		string const data = InsetCommand::params2string("label", p);

		if (cmd.argument().empty()) {
			bv->showDialog("label", data);
		} else {
			FuncRequest fr(LFUN_INSET_INSERT, data);
			dispatch(cur, fr);
		}
		break;
	}

	case LFUN_INFO_INSERT: {
		Inset * inset;
		if (cmd.argument().empty() && cur.selection()) {
			// if command argument is empty use current selection as parameter.
			docstring ds = cur.selectionAsString(false);
			cutSelection(cur, true, false);
			FuncRequest cmd0(cmd, ds);
			inset = createInset(cur.bv().buffer(), cmd0);
		} else {
			inset = createInset(cur.bv().buffer(), cmd);
		}
		if (!inset)
			break;
		insertInset(cur, inset);
		cur.posForward();
		break;
	}
	case LFUN_CAPTION_INSERT:
	case LFUN_FOOTNOTE_INSERT:
	case LFUN_NOTE_INSERT:
	case LFUN_FLEX_INSERT:
	case LFUN_BOX_INSERT:
	case LFUN_BRANCH_INSERT:
	case LFUN_ERT_INSERT:
	case LFUN_LISTING_INSERT:
	case LFUN_MARGINALNOTE_INSERT:
	case LFUN_OPTIONAL_INSERT:
	case LFUN_INDEX_INSERT:
		// Open the inset, and move the current selection
		// inside it.
		doInsertInset(cur, this, cmd, true, true);
		cur.posForward();
		// Some insets are numbered, others are shown in the outline pane so
		// let's update the labels and the toc backend.
		updateLabels(bv->buffer());
		break;

	case LFUN_TABULAR_INSERT:
		// if there were no arguments, just open the dialog
		if (doInsertInset(cur, this, cmd, false, true))
			cur.posForward();
		else
			bv->showDialog("tabularcreate");

		break;

	case LFUN_FLOAT_INSERT:
	case LFUN_FLOAT_WIDE_INSERT:
	case LFUN_WRAP_INSERT: {
		// will some text be moved into the inset?
		bool content = cur.selection();

		doInsertInset(cur, this, cmd, true, true);
		cur.posForward();

		if (content)
			cur.backwardPos();

		ParagraphList & pars = cur.text()->paragraphs();

		DocumentClass const & tclass = bv->buffer().params().documentClass();

		// add a separate paragraph for the caption inset
		pars.push_back(Paragraph());
		pars.back().setInsetOwner(&pars[0].inInset());
		pars.back().setPlainOrDefaultLayout(tclass);
		int cap_pit = pars.size() - 1;

		// if an empty inset was created, we create an additional empty
		// paragraph at the bottom so that the user can choose where to put
		// the graphics (or table).
		if (!content) {
			pars.push_back(Paragraph());
			pars.back().setInsetOwner(&pars[0].inInset());
			pars.back().setPlainOrDefaultLayout(tclass);
		}

		// reposition the cursor to the caption
		cur.pit() = cap_pit;
		cur.pos() = 0;
		// FIXME: This Text/Cursor dispatch handling is a mess!
		// We cannot use Cursor::dispatch here it needs access to up to
		// date metrics.
		FuncRequest cmd_caption(LFUN_CAPTION_INSERT);
		doInsertInset(cur, cur.text(), cmd_caption, true, false);
		updateLabels(bv->buffer());
		cur.updateFlags(Update::Force);
		// FIXME: When leaving the Float (or Wrap) inset we should
		// delete any empty paragraph left above or below the
		// caption.
		break;
	}

	case LFUN_NOMENCL_INSERT: {
		InsetCommandParams p(NOMENCL_CODE);
		if (cmd.argument().empty())
			p["symbol"] = bv->cursor().innerText()->getStringToIndex(bv->cursor());
		else
			p["symbol"] = cmd.argument();
		string const data = InsetCommand::params2string("nomenclature", p);
		bv->showDialog("nomenclature", data);
		break;
	}

	case LFUN_INDEX_PRINT:
	case LFUN_NOMENCL_PRINT:
	case LFUN_TOC_INSERT:
	case LFUN_LINE_INSERT:
	case LFUN_NEWPAGE_INSERT:
		// do nothing fancy
		doInsertInset(cur, this, cmd, false, false);
		cur.posForward();
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
			MacroType type = MacroTypeNewcommand;
			if (s2 == "def")
				type = MacroTypeDef;
			MathMacroTemplate * inset = new MathMacroTemplate(from_utf8(token(s, ' ', 0)), nargs, false, type);
			inset->setBuffer(bv->buffer());
			insertInset(cur, inset);

			// enter macro inset and select the name
			cur.push(*inset);
			cur.top().pos() = cur.top().lastpos();
			cur.resetAnchor();
			cur.setSelection(true);
			cur.top().pos() = 0;
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
		cur.recordUndo();
		cap::replaceSelection(cur);
		cur.insert(new InsetMathHull(hullSimple));
		checkAndActivateInset(cur, true);
		LASSERT(cur.inMathed(), /**/);
		cur.dispatch(cmd);
		break;
	}

	case LFUN_FONT_EMPH: {
		Font font(ignore_font, ignore_language);
		font.fontInfo().setEmph(FONT_TOGGLE);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_BOLD:
	case LFUN_FONT_BOLDSYMBOL: {
		Font font(ignore_font, ignore_language);
		font.fontInfo().setSeries(BOLD_SERIES);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_NOUN: {
		Font font(ignore_font, ignore_language);
		font.fontInfo().setNoun(FONT_TOGGLE);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_TYPEWRITER: {
		Font font(ignore_font, ignore_language);
		font.fontInfo().setFamily(TYPEWRITER_FAMILY); // no good
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_SANS: {
		Font font(ignore_font, ignore_language);
		font.fontInfo().setFamily(SANS_FAMILY);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_ROMAN: {
		Font font(ignore_font, ignore_language);
		font.fontInfo().setFamily(ROMAN_FAMILY);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_DEFAULT: {
		Font font(inherit_font, ignore_language);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_UNDERLINE: {
		Font font(ignore_font, ignore_language);
		font.fontInfo().setUnderbar(FONT_TOGGLE);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_SIZE: {
		Font font(ignore_font, ignore_language);
		setLyXSize(to_utf8(cmd.argument()), font.fontInfo());
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_LANGUAGE: {
		Language const * lang = languages.getLanguage(to_utf8(cmd.argument()));
		if (!lang)
			break;
		Font font(ignore_font, lang);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_TEXTSTYLE_APPLY:
		toggleAndShow(cur, this, freefont, toggleall);
		cur.message(_("Character set"));
		break;

	// Set the freefont using the contents of \param data dispatched from
	// the frontends and apply it at the current cursor location.
	case LFUN_TEXTSTYLE_UPDATE: {
		Font font;
		bool toggle;
		if (font.fromString(to_utf8(cmd.argument()), toggle)) {
			freefont = font;
			toggleall = toggle;
			toggleAndShow(cur, this, freefont, toggleall);
			cur.message(_("Character set"));
		} else {
			lyxerr << "Argument not ok";
		}
		break;
	}

	case LFUN_FINISHED_LEFT:
		LYXERR(Debug::DEBUG, "handle LFUN_FINISHED_LEFT:\n" << cur);
		// We're leaving an inset, going left. If the inset is LTR, we're
		// leaving from the front, so we should not move (remain at --- but
		// not in --- the inset). If the inset is RTL, move left, without
		// entering the inset itself; i.e., move to after the inset.
		if (cur.paragraph().getFontSettings(
				cur.bv().buffer().params(), cur.pos()).isRightToLeft())
			cursorVisLeft(cur, true);
		break;

	case LFUN_FINISHED_RIGHT:
		LYXERR(Debug::DEBUG, "handle LFUN_FINISHED_RIGHT:\n" << cur);
		// We're leaving an inset, going right. If the inset is RTL, we're
		// leaving from the front, so we should not move (remain at --- but
		// not in --- the inset). If the inset is LTR, move right, without
		// entering the inset itself; i.e., move to after the inset.
		if (!cur.paragraph().getFontSettings(
				cur.bv().buffer().params(), cur.pos()).isRightToLeft())
			cursorVisRight(cur, true);
		break;

	case LFUN_FINISHED_BACKWARD:
		LYXERR(Debug::DEBUG, "handle LFUN_FINISHED_BACKWARD:\n" << cur);
		break;

	case LFUN_FINISHED_FORWARD:
		LYXERR(Debug::DEBUG, "handle LFUN_FINISHED_FORWARD:\n" << cur);
		++cur.pos();
		cur.setCurrentFont();
		break;

	case LFUN_LAYOUT_PARAGRAPH: {
		string data;
		params2string(cur.paragraph(), data);
		data = "show\n" + data;
		bv->showDialog("paragraph", data);
		break;
	}

	case LFUN_PARAGRAPH_UPDATE: {
		string data;
		params2string(cur.paragraph(), data);

		// Will the paragraph accept changes from the dialog?
		bool const accept =
			cur.inset().allowParagraphCustomization(cur.idx());

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

	case LFUN_FLOAT_LIST_INSERT: {
		DocumentClass const & tclass = bv->buffer().params().documentClass();
		if (tclass.floats().typeExist(to_utf8(cmd.argument()))) {
			cur.recordUndo();
			if (cur.selection())
				cutSelection(cur, true, false);
			breakParagraph(cur);

			if (cur.lastpos() != 0) {
				cursorBackward(cur);
				breakParagraph(cur);
			}

			docstring const laystr = cur.inset().usePlainLayout() ?
				tclass.plainLayoutName() :
				tclass.defaultLayoutName();
			setLayout(cur, laystr);
			ParagraphParameters p;
			// FIXME If this call were replaced with one to clearParagraphParams(),
			// then we could get rid of this method altogether.
			setParagraphs(cur, p);
			// FIXME This should be simplified when InsetFloatList takes a
			// Buffer in its constructor.
			InsetFloatList * ifl = new InsetFloatList(to_utf8(cmd.argument()));
			ifl->setBuffer(bv->buffer());
			insertInset(cur, ifl);
			cur.posForward();
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
		bv->showDialog("thesaurus", to_utf8(arg));
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
			cur.setSelection(false);
		} else {
			cur.undispatched();
			// This used to be LFUN_FINISHED_RIGHT, I think FORWARD is more
			// correct, but I'm not 100% sure -- dov, 071019
			cmd = FuncRequest(LFUN_FINISHED_FORWARD);
		}
		break;

	case LFUN_OUTLINE_UP:
		outline(OutlineUp, cur);
		setCursor(cur, cur.pit(), 0);
		updateLabels(cur.buffer());
		needsUpdate = true;
		break;

	case LFUN_OUTLINE_DOWN:
		outline(OutlineDown, cur);
		setCursor(cur, cur.pit(), 0);
		updateLabels(cur.buffer());
		needsUpdate = true;
		break;

	case LFUN_OUTLINE_IN:
		outline(OutlineIn, cur);
		updateLabels(cur.buffer());
		needsUpdate = true;
		break;

	case LFUN_OUTLINE_OUT:
		outline(OutlineOut, cur);
		updateLabels(cur.buffer());
		needsUpdate = true;
		break;

	default:
		LYXERR(Debug::ACTION, "Command " << cmd << " not DISPATCHED by Text");
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
		// Inserting characters does not change par height in general. So, try
		// to update _only_ this paragraph. BufferView will detect if a full
		// metrics update is needed anyway.
		cur.updateFlags(Update::SinglePar | Update::FitCursor);
		return;
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
	LASSERT(cur.text() == this, /**/);

	FontInfo const & fontinfo = cur.real_current_font.fontInfo();
	bool enable = true;
	InsetCode code = NO_CODE;

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

	case LFUN_DIALOG_SHOW_NEW_INSET:
		if (cmd.argument() == "bibitem")
			code = BIBITEM_CODE;
		else if (cmd.argument() == "bibtex")
			code = BIBTEX_CODE;
		else if (cmd.argument() == "box")
			code = BOX_CODE;
		else if (cmd.argument() == "branch")
			code = BRANCH_CODE;
		else if (cmd.argument() == "citation")
			code = CITE_CODE;
		else if (cmd.argument() == "ert")
			code = ERT_CODE;
		else if (cmd.argument() == "external")
			code = EXTERNAL_CODE;
		else if (cmd.argument() == "float")
			code = FLOAT_CODE;
		else if (cmd.argument() == "graphics")
			code = GRAPHICS_CODE;
		else if (cmd.argument() == "href")
			code = HYPERLINK_CODE;
		else if (cmd.argument() == "include")
			code = INCLUDE_CODE;
		else if (cmd.argument() == "index")
			code = INDEX_CODE;
		else if (cmd.argument() == "nomenclature")
			code = NOMENCL_CODE;
		else if (cmd.argument() == "label")
			code = LABEL_CODE;
		else if (cmd.argument() == "note")
			code = NOTE_CODE;
		else if (cmd.argument() == "ref")
			code = REF_CODE;
		else if (cmd.argument() == "space")
			code = SPACE_CODE;
		else if (cmd.argument() == "toc")
			code = TOC_CODE;
		else if (cmd.argument() == "vspace")
			code = VSPACE_CODE;
		else if (cmd.argument() == "wrap")
			code = WRAP_CODE;
		else if (cmd.argument() == "listings")
			code = LISTINGS_CODE;
		break;

	case LFUN_ERT_INSERT:
		code = ERT_CODE;
		break;
	case LFUN_LISTING_INSERT:
		code = LISTINGS_CODE;
		// not allowed in description items
		enable = !inDescriptionItem(cur);
		break;
	case LFUN_FOOTNOTE_INSERT:
		code = FOOT_CODE;
		break;
	case LFUN_TABULAR_INSERT:
		code = TABULAR_CODE;
		break;
	case LFUN_MARGINALNOTE_INSERT:
		code = MARGIN_CODE;
		break;
	case LFUN_FLOAT_INSERT:
	case LFUN_FLOAT_WIDE_INSERT:
		code = FLOAT_CODE;
		// not allowed in description items
		enable = !inDescriptionItem(cur);
		break;
	case LFUN_WRAP_INSERT:
		code = WRAP_CODE;
		// not allowed in description items
		enable = !inDescriptionItem(cur);
		break;
	case LFUN_FLOAT_LIST_INSERT:
		code = FLOAT_LIST_CODE;
		break;
	case LFUN_CAPTION_INSERT:
		code = CAPTION_CODE;
		// not allowed in description items
		enable = !inDescriptionItem(cur);
		break;
	case LFUN_NOTE_INSERT:
		code = NOTE_CODE;
		// in commands (sections etc.) and description items,
		// only Notes are allowed
		enable = (cmd.argument().empty() || cmd.getArg(0) == "Note" ||
			  (!cur.paragraph().layout().isCommand()
			   && !inDescriptionItem(cur)));
		break;
	case LFUN_FLEX_INSERT: {
		code = FLEX_CODE;
		string s = cmd.getArg(0);
		InsetLayout il =
			cur.buffer().params().documentClass().insetLayout(from_utf8(s));
		if (il.lyxtype() != "charstyle" &&
		    il.lyxtype() != "custom" &&
		    il.lyxtype() != "element" &&
		    il.lyxtype ()!= "standard")
			enable = false;
		break;
		}
	case LFUN_BOX_INSERT:
		code = BOX_CODE;
		break;
	case LFUN_BRANCH_INSERT:
		code = BRANCH_CODE;
		if (cur.buffer().masterBuffer()->params().branchlist().empty())
			enable = false;
		break;
	case LFUN_LABEL_INSERT:
		code = LABEL_CODE;
		break;
	case LFUN_INFO_INSERT:
		code = INFO_CODE;
		break;
	case LFUN_OPTIONAL_INSERT:
		code = OPTARG_CODE;
		enable = cur.paragraph().insetList().count(OPTARG_CODE)
			< cur.paragraph().layout().optionalargs;
		break;
	case LFUN_INDEX_INSERT:
		code = INDEX_CODE;
		break;
	case LFUN_INDEX_PRINT:
		code = INDEX_PRINT_CODE;
		break;
	case LFUN_NOMENCL_INSERT:
		if (cur.selIsMultiCell() || cur.selIsMultiLine()) {
			enable = false;
			break;
		}
		code = NOMENCL_CODE;
		break;
	case LFUN_NOMENCL_PRINT:
		code = NOMENCL_PRINT_CODE;
		break;
	case LFUN_TOC_INSERT:
		code = TOC_CODE;
		break;
	case LFUN_HYPERLINK_INSERT:
		if (cur.selIsMultiCell() || cur.selIsMultiLine()) {
			enable = false;
			break;
		}
		code = HYPERLINK_CODE;
		break;
	case LFUN_QUOTE_INSERT:
		// always allow this, since we will inset a raw quote
		// if an inset is not allowed.
		break;
	case LFUN_SPECIALCHAR_INSERT:
		code = SPECIALCHAR_CODE;
		break;
	case LFUN_SPACE_INSERT:
		// slight hack: we know this is allowed in math mode
		if (cur.inTexted())
			code = SPACE_CODE;
		break;

	case LFUN_INSET_MODIFY:
		// We need to disable this, because we may get called for a
		// tabular cell via
		// InsetTabular::getStatus() -> InsetText::getStatus()
		// and we don't handle LFUN_INSET_MODIFY.
		enable = false;
		break;

	case LFUN_FONT_EMPH:
		flag.setOnOff(fontinfo.emph() == FONT_ON);
		break;

	case LFUN_FONT_NOUN:
		flag.setOnOff(fontinfo.noun() == FONT_ON);
		break;

	case LFUN_FONT_BOLD:
	case LFUN_FONT_BOLDSYMBOL:
		flag.setOnOff(fontinfo.series() == BOLD_SERIES);
		break;

	case LFUN_FONT_SANS:
		flag.setOnOff(fontinfo.family() == SANS_FAMILY);
		break;

	case LFUN_FONT_ROMAN:
		flag.setOnOff(fontinfo.family() == ROMAN_FAMILY);
		break;

	case LFUN_FONT_TYPEWRITER:
		flag.setOnOff(fontinfo.family() == TYPEWRITER_FAMILY);
		break;

	case LFUN_CUT:
	case LFUN_COPY:
		enable = cur.selection();
		break;

	case LFUN_PASTE: {
		if (cmd.argument().empty()) {
			if (theClipboard().isInternal())
				enable = cap::numberOfSelections() > 0;
			else
				enable = !theClipboard().empty();
			break;
		}

		// we have an argument
		string const arg = to_utf8(cmd.argument());
		if (isStrUnsignedInt(arg)) {
			// it's a number and therefore means the internal stack
			unsigned int n = convert<unsigned int>(arg);
			enable = cap::numberOfSelections() > n;
			break;
		}

		// explicit graphics type?
		if ((arg == "pdf" && theClipboard().hasGraphicsContents(Clipboard::PdfGraphicsType))
		    || (arg == "png" && theClipboard().hasGraphicsContents(Clipboard::PngGraphicsType))
		    || (arg == "jpeg" && theClipboard().hasGraphicsContents(Clipboard::JpegGraphicsType))
		    || (arg == "linkback" && theClipboard().hasGraphicsContents(Clipboard::LinkBackGraphicsType))) {
			enable = true;
			break;
		}

		// unknown argument
		enable = false;
		break;
	 }

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
		if (!cmd.argument().empty()) {
			InsetLayout il = cur.inset().getLayout(cur.buffer().params());
			enable = cur.inset().lyxCode() == FLEX_CODE
			         && il.lyxtype() == to_utf8(cmd.argument());
		} else {
			enable = !isMainText(cur.bv().buffer())
			         && cur.inset().nargs() == 1;
		}
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
		// FIXME: LyX is not ready for outlining within inset.
		enable = isMainText(cur.bv().buffer())
			&& cur.paragraph().layout().toclevel != Layout::NOT_IN_TOC;
		break;

	case LFUN_NEWLINE_INSERT:
		// LaTeX restrictions (labels or empty par)
		enable = (cur.pos() > cur.paragraph().beginOfBody());
		break;

	case LFUN_SET_GRAPHICS_GROUP: {
		InsetGraphics * ins = graphics::getCurrentGraphicsInset(cur);
		if (!ins)
			enable = false;
		else
			flag.setOnOff(to_utf8(cmd.argument()) == ins->getParams().groupId);
		break;
	}

	case LFUN_NEWPAGE_INSERT:
		// not allowed in description items
		enable = !inDescriptionItem(cur);
		break;

	case LFUN_WORD_DELETE_FORWARD:
	case LFUN_WORD_DELETE_BACKWARD:
	case LFUN_LINE_DELETE:
	case LFUN_WORD_FORWARD:
	case LFUN_WORD_BACKWARD:
	case LFUN_WORD_RIGHT:
	case LFUN_WORD_LEFT:
	case LFUN_CHAR_FORWARD:
	case LFUN_CHAR_FORWARD_SELECT:
	case LFUN_CHAR_BACKWARD:
	case LFUN_CHAR_BACKWARD_SELECT:
	case LFUN_CHAR_LEFT:
	case LFUN_CHAR_LEFT_SELECT:
	case LFUN_CHAR_RIGHT:
	case LFUN_CHAR_RIGHT_SELECT:
	case LFUN_UP:
	case LFUN_UP_SELECT:
	case LFUN_DOWN:
	case LFUN_DOWN_SELECT:
	case LFUN_PARAGRAPH_UP_SELECT:
	case LFUN_PARAGRAPH_DOWN_SELECT:
	case LFUN_LINE_BEGIN_SELECT:
	case LFUN_LINE_END_SELECT:
	case LFUN_WORD_FORWARD_SELECT:
	case LFUN_WORD_BACKWARD_SELECT:
	case LFUN_WORD_RIGHT_SELECT:
	case LFUN_WORD_LEFT_SELECT:
	case LFUN_WORD_SELECT:
	case LFUN_PARAGRAPH_UP:
	case LFUN_PARAGRAPH_DOWN:
	case LFUN_LINE_BEGIN:
	case LFUN_LINE_END:
	case LFUN_CHAR_DELETE_FORWARD:
	case LFUN_CHAR_DELETE_BACKWARD:
	case LFUN_BREAK_PARAGRAPH:
	case LFUN_PARAGRAPH_SPACING:
	case LFUN_INSET_INSERT:
	case LFUN_WORD_UPCASE:
	case LFUN_WORD_LOWCASE:
	case LFUN_WORD_CAPITALIZE:
	case LFUN_CHARS_TRANSPOSE:
	case LFUN_SERVER_GET_XY:
	case LFUN_SERVER_SET_XY:
	case LFUN_SERVER_GET_LAYOUT:
	case LFUN_LAYOUT:
	case LFUN_DATE_INSERT:
	case LFUN_SELF_INSERT:
	case LFUN_LINE_INSERT:
	case LFUN_MATH_DISPLAY:
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
	case LFUN_TEXTSTYLE_APPLY:
	case LFUN_TEXTSTYLE_UPDATE:
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

	if (code != NO_CODE
	    && (cur.empty() || !cur.inset().insetAllowed(code)))
		enable = false;

	flag.setEnabled(enable);
	return true;
}


void Text::pasteString(Cursor & cur, docstring const & clip,
		bool asParagraphs)
{
	cur.clearSelection();
	if (!clip.empty()) {
		cur.recordUndo();
		if (asParagraphs)
			insertStringAsParagraphs(cur, clip);
		else
			insertStringAsLines(cur, clip);
	}
}


// FIXME: an item inset would make things much easier.
bool Text::inDescriptionItem(Cursor & cur) const
{
	Paragraph & par = cur.paragraph();
	pos_type const pos = cur.pos();
	pos_type const body_pos = par.beginOfBody();

	if (par.layout().latextype != LATEX_LIST_ENVIRONMENT
	    && (par.layout().latextype != LATEX_ITEM_ENVIRONMENT
		|| par.layout().margintype != MARGIN_FIRST_DYNAMIC))
		return false;

	return (pos < body_pos
		|| (pos == body_pos
		    && (pos == 0 || par.getChar(pos - 1) != ' ')));
}

} // namespace lyx
