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

#include "Bidi.h"
#include "BranchList.h"
#include "FloatList.h"
#include "FuncStatus.h"
#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Changes.h"
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
#include "LyX.h"
#include "Lexer.h"
#include "LyXRC.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "SpellChecker.h"
#include "TextClass.h"
#include "TextMetrics.h"
#include "WordLangTuple.h"

#include "frontends/Application.h"
#include "frontends/Clipboard.h"
#include "frontends/Selection.h"

#include "insets/InsetArgument.h"
#include "insets/InsetCollapsable.h"
#include "insets/InsetCommand.h"
#include "insets/InsetExternal.h"
#include "insets/InsetFloat.h"
#include "insets/InsetFloatList.h"
#include "insets/InsetGraphics.h"
#include "insets/InsetGraphicsParams.h"
#include "insets/InsetIPAMacro.h"
#include "insets/InsetNewline.h"
#include "insets/InsetQuotes.h"
#include "insets/InsetSpecialChar.h"
#include "insets/InsetText.h"
#include "insets/InsetWrap.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/lyxtime.h"
#include "support/os.h"
#include "support/regex.h"

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
using cap::grabAndEraseSelection;
using cap::selClearOrDel;
using cap::pasteSimpleText;

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
		cur.insert(new InsetMathHull(cur.buffer(), hullSimple));
#ifdef ENABLE_ASSERTIONS
		LATTEST(old_pos == cur.pos());
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
			InsetMathHull * formula = new InsetMathHull(cur.buffer());
			string const selstr = to_utf8(sel);
			istringstream is(selstr);
			Lexer lex;
			lex.setStream(is);
			if (!formula->readQuiet(lex)) {
				// No valid formula, let's try with delims
				is.str("$" + selstr + "$");
				lex.setStream(is);
				if (!formula->readQuiet(lex)) {
					// Still not valid, leave it as is
					valid = false;
					delete formula;
					cur.insert(sel);
				} else
					cur.insert(formula);
			} else
				cur.insert(formula);
		} else {
			cur.insert(new MathMacroTemplate(cur.buffer(), sel));
		}
	}
	if (valid)
		cur.message(from_utf8(N_("Math editor mode")));
	else
		cur.message(from_utf8(N_("No valid math formula")));
}


void regexpDispatch(Cursor & cur, FuncRequest const & cmd)
{
	LASSERT(cmd.action() == LFUN_REGEXP_MODE, return);
	if (cur.inRegexped()) {
		cur.message(_("Already in regular expression mode"));
		return;
	}
	cur.recordUndo();
	docstring sel = cur.selectionAsString(false);

	// It may happen that sel is empty but there is a selection
	replaceSelection(cur);

	cur.insert(new InsetMathHull(cur.buffer(), hullRegexp));
	cur.nextInset()->edit(cur, true);
	cur.niceInsert(sel);

	cur.message(_("Regexp editor mode"));
}


static void specialChar(Cursor & cur, InsetSpecialChar::Kind kind)
{
	cur.recordUndo();
	cap::replaceSelection(cur);
	cur.insert(new InsetSpecialChar(kind));
	cur.posForward();
}


static void ipaChar(Cursor & cur, InsetIPAChar::Kind kind)
{
	cur.recordUndo();
	cap::replaceSelection(cur);
	cur.insert(new InsetIPAChar(kind));
	cur.posForward();
}


static bool doInsertInset(Cursor & cur, Text * text,
	FuncRequest const & cmd, bool edit, bool pastesel)
{
	Buffer & buffer = cur.bv().buffer();
	BufferParams const & bparams = buffer.params();
	Inset * inset = createInset(&buffer, cmd);
	if (!inset)
		return false;

	if (InsetCollapsable * ci = inset->asInsetCollapsable())
		ci->setButtonLabel();

	cur.recordUndo();
	if (cmd.action() == LFUN_INDEX_INSERT) {
		docstring ds = subst(text->getStringToIndex(cur), '\n', ' ');
		text->insertInset(cur, inset);
		if (edit)
			inset->edit(cur, true);
		// Now put this into inset
		Font const f(inherit_font, cur.current_font.language());
		if (!ds.empty()) {
			cur.text()->insertStringAsLines(cur, ds, f);
			cur.leaveInset(*inset);
		}
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

	pasteFromStack(cur, cur.buffer()->errorList("Paste"), 0);
	cur.buffer()->errors("Paste");
	cur.clearSelection(); // bug 393
	cur.finishUndo();
	InsetText * inset_text = inset->asInsetText();
	if (inset_text) {
		inset_text->fixParagraphsFont();
		if (!inset_text->allowMultiPar() || cur.lastpit() == 0) {
			// reset first par to default
			cur.text()->paragraphs().begin()
				->setPlainOrDefaultLayout(bparams.documentClass());
			cur.pos() = 0;
			cur.pit() = 0;
			// Merge multiple paragraphs -- hack
			while (cur.lastpit() > 0)
				mergeParagraph(bparams, cur.text()->paragraphs(), 0);
			if (cmd.action() == LFUN_FLEX_INSERT)
				return true;
			Cursor old = cur;
			cur.leaveInset(*inset);
			if (cmd.action() == LFUN_PREVIEW_INSERT
			    || cmd.action() == LFUN_IPA_INSERT)
				// trigger preview
				notifyCursorLeavesOrEnters(old, cur);
		}
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
	Buffer & buf = *cur.buffer();
	pit_type & pit = cur.pit();
	ParagraphList & pars = buf.text().paragraphs();
	ParagraphList::iterator const bgn = pars.begin();
	// The first paragraph of the area to be copied:
	ParagraphList::iterator start = boost::next(bgn, pit);
	// The final paragraph of area to be copied:
	ParagraphList::iterator finish = start;
	ParagraphList::iterator const end = pars.end();

	DocumentClass const & tc = buf.params().documentClass();

	int const thistoclevel = buf.text().getTocLevel(distance(bgn, start));
	int toclevel;

	// Move out (down) from this section header
	if (finish != end)
		++finish;

	// Seek the one (on same level) below
	for (; finish != end; ++finish) {
		toclevel = buf.text().getTocLevel(distance(bgn, finish));
		if (toclevel != Layout::NOT_IN_TOC && toclevel <= thistoclevel)
			break;
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
				toclevel = buf.text().getTocLevel(distance(bgn, dest));
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
			pars.splice(dest, start, finish);
			cur.pit() = newpit;
			break;
		}
		case OutlineDown: {
			if (finish == end)
				// Nothing to move.
				return;
			// Go one down from *this* header:
			ParagraphList::iterator dest = boost::next(finish, 1);
			// Go further down to find header to insert in front of:
			for (; dest != end; ++dest) {
				toclevel = buf.text().getTocLevel(distance(bgn, dest));
				if (toclevel != Layout::NOT_IN_TOC
				      && toclevel <= thistoclevel)
					break;
			}
			// One such was found:
			pit_type newpit = distance(bgn, dest);
			buf.undo().recordUndo(cur, ATOMIC_UNDO, pit, newpit - 1);
			pit_type const len = distance(start, finish);
			pars.splice(dest, start, finish);
			cur.pit() = newpit - len;
			break;
		}
		case OutlineIn: {
			pit_type const len = distance(start, finish);
			buf.undo().recordUndo(cur, ATOMIC_UNDO, pit, pit + len - 1);
			for (; start != finish; ++start) {
				toclevel = buf.text().getTocLevel(distance(bgn, start));
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
			break;
		}
		case OutlineOut: {
			pit_type const len = distance(start, finish);
			buf.undo().recordUndo(cur, ATOMIC_UNDO, pit, pit + len - 1);
			for (; start != finish; ++start) {
				toclevel = buf.text().getTocLevel(distance(bgn, start));
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
			break;
		}
	}
}


void Text::number(Cursor & cur)
{
	FontInfo font = ignore_font;
	font.setNumber(FONT_TOGGLE);
	toggleAndShow(cur, this, Font(font, ignore_language));
}


bool Text::isRTL(Paragraph const & par) const
{
	Buffer const & buffer = owner_->buffer();
	return par.isRTL(buffer.params());
}


void Text::dispatch(Cursor & cur, FuncRequest & cmd)
{
	LYXERR(Debug::ACTION, "Text::dispatch: cmd: " << cmd);

	// Dispatch if the cursor is inside the text. It is not the
	// case for context menus (bug 5797).
	if (cur.text() != this) {
		cur.undispatched();
		return;
	}

	BufferView * bv = &cur.bv();
	TextMetrics * tm = &bv->textMetrics(this);
	if (!tm->contains(cur.pit())) {
		lyx::dispatch(FuncRequest(LFUN_SCREEN_SHOW_CURSOR));
		tm = &bv->textMetrics(this);
	}

	// FIXME: We use the update flag to indicates wether a singlePar or a
	// full screen update is needed. We reset it here but shall we restore it
	// at the end?
	cur.noScreenUpdate();

	LBUFERR(this == cur.text());
	CursorSlice const oldTopSlice = cur.top();
	bool const oldBoundary = cur.boundary();
	bool const oldSelection = cur.selection();
	// Signals that, even if needsUpdate == false, an update of the
	// cursor paragraph is required
	bool singleParUpdate = lyxaction.funcHasFlag(cmd.action(),
		LyXAction::SingleParUpdate);
	// Signals that a full-screen update is required
	bool needsUpdate = !(lyxaction.funcHasFlag(cmd.action(),
		LyXAction::NoUpdate) || singleParUpdate);
	bool const last_misspelled = lyxrc.spellcheck_continuously
		&& cur.paragraph().isMisspelled(cur.pos(), true);

	FuncCode const act = cmd.action();
	switch (act) {

	case LFUN_PARAGRAPH_MOVE_DOWN: {
		pit_type const pit = cur.pit();
		recUndo(cur, pit, pit + 1);
		cur.finishUndo();
		pars_.swap(pit, pit + 1);
		needsUpdate = true;
		cur.forceBufferUpdate();
		++cur.pit();
		break;
	}

	case LFUN_PARAGRAPH_MOVE_UP: {
		pit_type const pit = cur.pit();
		recUndo(cur, pit - 1, pit);
		cur.finishUndo();
		pars_.swap(pit, pit - 1);
		--cur.pit();
		needsUpdate = true;
		cur.forceBufferUpdate();
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
		cur.forceBufferUpdate();
		break;
	}

	case LFUN_WORD_DELETE_FORWARD:
		if (cur.selection())
			cutSelection(cur, true, false);
		else
			deleteWordForward(cur);
		finishChange(cur, false);
		break;

	case LFUN_WORD_DELETE_BACKWARD:
		if (cur.selection())
			cutSelection(cur, true, false);
		else
			deleteWordBackward(cur);
		finishChange(cur, false);
		break;

	case LFUN_LINE_DELETE_FORWARD:
		if (cur.selection())
			cutSelection(cur, true, false);
		else
			tm->deleteLineForward(cur);
		finishChange(cur, false);
		break;

	case LFUN_BUFFER_BEGIN:
	case LFUN_BUFFER_BEGIN_SELECT:
		needsUpdate |= cur.selHandle(act == LFUN_BUFFER_BEGIN_SELECT);
		if (cur.depth() == 1)
			needsUpdate |= cursorTop(cur);
		else
			cur.undispatched();
		cur.screenUpdateFlags(Update::FitCursor);
		break;

	case LFUN_BUFFER_END:
	case LFUN_BUFFER_END_SELECT:
		needsUpdate |= cur.selHandle(act == LFUN_BUFFER_END_SELECT);
		if (cur.depth() == 1)
			needsUpdate |= cursorBottom(cur);
		else
			cur.undispatched();
		cur.screenUpdateFlags(Update::FitCursor);
		break;

	case LFUN_INSET_BEGIN:
	case LFUN_INSET_BEGIN_SELECT:
		needsUpdate |= cur.selHandle(act == LFUN_INSET_BEGIN_SELECT);
		if (cur.depth() == 1 || !cur.top().at_begin())
			needsUpdate |= cursorTop(cur);
		else
			cur.undispatched();
		cur.screenUpdateFlags(Update::FitCursor);
		break;

	case LFUN_INSET_END:
	case LFUN_INSET_END_SELECT:
		needsUpdate |= cur.selHandle(act == LFUN_INSET_END_SELECT);
		if (cur.depth() == 1 || !cur.top().at_end())
			needsUpdate |= cursorBottom(cur);
		else
			cur.undispatched();
		cur.screenUpdateFlags(Update::FitCursor);
		break;

	case LFUN_CHAR_FORWARD:
	case LFUN_CHAR_FORWARD_SELECT:
		//LYXERR0(" LFUN_CHAR_FORWARD[SEL]:\n" << cur);
		needsUpdate |= cur.selHandle(act == LFUN_CHAR_FORWARD_SELECT);
		needsUpdate |= cursorForward(cur);

		if (!needsUpdate && oldTopSlice == cur.top()
				&& cur.boundary() == oldBoundary) {
			cur.undispatched();
			cmd = FuncRequest(LFUN_FINISHED_FORWARD);

			// we will probably be moving out the inset, so we should execute
			// the depm-mechanism, but only when the cursor has a place to
			// go outside this inset, i.e. in a slice above.
			if (cur.depth() > 1 && cur.pos() == cur.lastpos()
				  && cur.pit() == cur.lastpit()) {
				// The cursor hasn't changed yet. To give the
				// DEPM the possibility of doing something we must
				// provide it with two different cursors.
				Cursor dummy = cur;
				dummy.pos() = dummy.pit() = 0;
				if (cur.bv().checkDepm(dummy, cur))
					cur.forceBufferUpdate();
			}
		}
		break;

	case LFUN_CHAR_BACKWARD:
	case LFUN_CHAR_BACKWARD_SELECT:
		//lyxerr << "handle LFUN_CHAR_BACKWARD[_SELECT]:\n" << cur << endl;
		needsUpdate |= cur.selHandle(act == LFUN_CHAR_BACKWARD_SELECT);
		needsUpdate |= cursorBackward(cur);

		if (!needsUpdate && oldTopSlice == cur.top()
			&& cur.boundary() == oldBoundary) {
			cur.undispatched();
			cmd = FuncRequest(LFUN_FINISHED_BACKWARD);

			// we will probably be moving out the inset, so we should execute
			// the depm-mechanism, but only when the cursor has a place to
			// go outside this inset, i.e. in a slice above.
			if (cur.depth() > 1 && cur.pos() == 0 && cur.pit() == 0) {
				// The cursor hasn't changed yet. To give the
				// DEPM the possibility of doing something we must
				// provide it with two different cursors.
				Cursor dummy = cur;
				dummy.pos() = cur.lastpos();
				dummy.pit() = cur.lastpit();
				if (cur.bv().checkDepm(dummy, cur))
					cur.forceBufferUpdate();
			}
		}
		break;

	case LFUN_CHAR_LEFT:
	case LFUN_CHAR_LEFT_SELECT:
		if (lyxrc.visual_cursor) {
			needsUpdate |= cur.selHandle(act == LFUN_CHAR_LEFT_SELECT);
			needsUpdate |= cursorVisLeft(cur);
			if (!needsUpdate && oldTopSlice == cur.top()
					&& cur.boundary() == oldBoundary) {
				cur.undispatched();
				cmd = FuncRequest(LFUN_FINISHED_LEFT);
			}
		} else {
			if (reverseDirectionNeeded(cur)) {
				cmd.setAction(cmd.action() == LFUN_CHAR_LEFT_SELECT ?
					LFUN_CHAR_FORWARD_SELECT : LFUN_CHAR_FORWARD);
			} else {
				cmd.setAction(cmd.action() == LFUN_CHAR_LEFT_SELECT ?
					LFUN_CHAR_BACKWARD_SELECT : LFUN_CHAR_BACKWARD);
			}
			dispatch(cur, cmd);
			return;
		}
		break;

	case LFUN_CHAR_RIGHT:
	case LFUN_CHAR_RIGHT_SELECT:
		if (lyxrc.visual_cursor) {
			needsUpdate |= cur.selHandle(cmd.action() == LFUN_CHAR_RIGHT_SELECT);
			needsUpdate |= cursorVisRight(cur);
			if (!needsUpdate && oldTopSlice == cur.top()
					&& cur.boundary() == oldBoundary) {
				cur.undispatched();
				cmd = FuncRequest(LFUN_FINISHED_RIGHT);
			}
		} else {
			if (reverseDirectionNeeded(cur)) {
				cmd.setAction(cmd.action() == LFUN_CHAR_RIGHT_SELECT ?
					LFUN_CHAR_BACKWARD_SELECT : LFUN_CHAR_BACKWARD);
			} else {
				cmd.setAction(cmd.action() == LFUN_CHAR_RIGHT_SELECT ?
					LFUN_CHAR_FORWARD_SELECT : LFUN_CHAR_FORWARD);
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
		bool select = cmd.action() == LFUN_DOWN_SELECT ||
			cmd.action() == LFUN_UP_SELECT;

		// move cursor up/down
		bool up = cmd.action() == LFUN_UP_SELECT || cmd.action() == LFUN_UP;
		bool const atFirstOrLastRow = cur.atFirstOrLastRow(up);

		if (!atFirstOrLastRow) {
			needsUpdate |= cur.selHandle(select);
			cur.upDownInText(up, needsUpdate);
			needsUpdate |= cur.beforeDispatchCursor().inMathed();
		} else {
			// if the cursor cannot be moved up or down do not remove
			// the selection right now, but wait for the next dispatch.
			if (select)
				needsUpdate |= cur.selHandle(select);
			cur.upDownInText(up, needsUpdate);
			cur.undispatched();
		}

		break;
	}

	case LFUN_PARAGRAPH_UP:
	case LFUN_PARAGRAPH_UP_SELECT:
		needsUpdate |= cur.selHandle(cmd.action() == LFUN_PARAGRAPH_UP_SELECT);
		needsUpdate |= cursorUpParagraph(cur);
		break;

	case LFUN_PARAGRAPH_DOWN:
	case LFUN_PARAGRAPH_DOWN_SELECT:
		needsUpdate |= cur.selHandle(cmd.action() == LFUN_PARAGRAPH_DOWN_SELECT);
		needsUpdate |= cursorDownParagraph(cur);
		break;

	case LFUN_LINE_BEGIN:
	case LFUN_LINE_BEGIN_SELECT:
		needsUpdate |= cur.selHandle(cmd.action() == LFUN_LINE_BEGIN_SELECT);
		needsUpdate |= tm->cursorHome(cur);
		break;

	case LFUN_LINE_END:
	case LFUN_LINE_END_SELECT:
		needsUpdate |= cur.selHandle(cmd.action() == LFUN_LINE_END_SELECT);
		needsUpdate |= tm->cursorEnd(cur);
		break;

	case LFUN_SECTION_SELECT: {
		Buffer const & buf = *cur.buffer();
		pit_type const pit = cur.pit();
		ParagraphList & pars = buf.text().paragraphs();
		ParagraphList::iterator bgn = pars.begin();
		// The first paragraph of the area to be selected:
		ParagraphList::iterator start = boost::next(bgn, pit);
		// The final paragraph of area to be selected:
		ParagraphList::iterator finish = start;
		ParagraphList::iterator end = pars.end();

		int const thistoclevel = buf.text().getTocLevel(distance(bgn, start));
		if (thistoclevel == Layout::NOT_IN_TOC)
			break;

		cur.pos() = 0;
		Cursor const old_cur = cur;
		needsUpdate |= cur.selHandle(true);

		// Move out (down) from this section header
		if (finish != end)
			++finish;

		// Seek the one (on same level) below
		for (; finish != end; ++finish, ++cur.pit()) {
			int const toclevel = buf.text().getTocLevel(distance(bgn, finish));
			if (toclevel != Layout::NOT_IN_TOC && toclevel <= thistoclevel)
				break;
		}
		cur.pos() = cur.lastpos();

		needsUpdate |= cur != old_cur;
		break;
	}

	case LFUN_WORD_RIGHT:
	case LFUN_WORD_RIGHT_SELECT:
		if (lyxrc.visual_cursor) {
			needsUpdate |= cur.selHandle(cmd.action() == LFUN_WORD_RIGHT_SELECT);
			needsUpdate |= cursorVisRightOneWord(cur);
			if (!needsUpdate && oldTopSlice == cur.top()
					&& cur.boundary() == oldBoundary) {
				cur.undispatched();
				cmd = FuncRequest(LFUN_FINISHED_RIGHT);
			}
		} else {
			if (reverseDirectionNeeded(cur)) {
				cmd.setAction(cmd.action() == LFUN_WORD_RIGHT_SELECT ?
						LFUN_WORD_BACKWARD_SELECT : LFUN_WORD_BACKWARD);
			} else {
				cmd.setAction(cmd.action() == LFUN_WORD_RIGHT_SELECT ?
						LFUN_WORD_FORWARD_SELECT : LFUN_WORD_FORWARD);
			}
			dispatch(cur, cmd);
			return;
		}
		break;

	case LFUN_WORD_FORWARD:
	case LFUN_WORD_FORWARD_SELECT:
		needsUpdate |= cur.selHandle(cmd.action() == LFUN_WORD_FORWARD_SELECT);
		needsUpdate |= cursorForwardOneWord(cur);

		if (!needsUpdate && oldTopSlice == cur.top()
				&& cur.boundary() == oldBoundary) {
			cur.undispatched();
			cmd = FuncRequest(LFUN_FINISHED_FORWARD);

			// we will probably be moving out the inset, so we should execute
			// the depm-mechanism, but only when the cursor has a place to
			// go outside this inset, i.e. in a slice above.
			if (cur.depth() > 1 && cur.pos() == cur.lastpos()
				  && cur.pit() == cur.lastpit()) {
				// The cursor hasn't changed yet. To give the
				// DEPM the possibility of doing something we must
				// provide it with two different cursors.
				Cursor dummy = cur;
				dummy.pos() = dummy.pit() = 0;
				if (cur.bv().checkDepm(dummy, cur))
					cur.forceBufferUpdate();
			}
		}
		break;

	case LFUN_WORD_LEFT:
	case LFUN_WORD_LEFT_SELECT:
		if (lyxrc.visual_cursor) {
			needsUpdate |= cur.selHandle(cmd.action() == LFUN_WORD_LEFT_SELECT);
			needsUpdate |= cursorVisLeftOneWord(cur);
			if (!needsUpdate && oldTopSlice == cur.top()
					&& cur.boundary() == oldBoundary) {
				cur.undispatched();
				cmd = FuncRequest(LFUN_FINISHED_LEFT);
			}
		} else {
			if (reverseDirectionNeeded(cur)) {
				cmd.setAction(cmd.action() == LFUN_WORD_LEFT_SELECT ?
						LFUN_WORD_FORWARD_SELECT : LFUN_WORD_FORWARD);
			} else {
				cmd.setAction(cmd.action() == LFUN_WORD_LEFT_SELECT ?
						LFUN_WORD_BACKWARD_SELECT : LFUN_WORD_BACKWARD);
			}
			dispatch(cur, cmd);
			return;
		}
		break;

	case LFUN_WORD_BACKWARD:
	case LFUN_WORD_BACKWARD_SELECT:
		needsUpdate |= cur.selHandle(cmd.action() == LFUN_WORD_BACKWARD_SELECT);
		needsUpdate |= cursorBackwardOneWord(cur);

		if (!needsUpdate && oldTopSlice == cur.top()
				&& cur.boundary() == oldBoundary) {
			cur.undispatched();
			cmd = FuncRequest(LFUN_FINISHED_BACKWARD);

			// we will probably be moving out the inset, so we should execute
			// the depm-mechanism, but only when the cursor has a place to
			// go outside this inset, i.e. in a slice above.
			if (cur.depth() > 1 && cur.pos() == 0
				  && cur.pit() == 0) {
				// The cursor hasn't changed yet. To give the
				// DEPM the possibility of doing something we must
				// provide it with two different cursors.
				Cursor dummy = cur;
				dummy.pos() = cur.lastpos();
				dummy.pit() = cur.lastpit();
				if (cur.bv().checkDepm(dummy, cur))
					cur.forceBufferUpdate();
			}
		}
		break;

	case LFUN_WORD_SELECT: {
		selectWord(cur, WHOLE_WORD);
		finishChange(cur, true);
		break;
	}

	case LFUN_NEWLINE_INSERT: {
		InsetNewlineParams inp;
		docstring arg = cmd.argument();
		if (arg == "linebreak")
			inp.kind = InsetNewlineParams::LINEBREAK;
		else
			inp.kind = InsetNewlineParams::NEWLINE;
		cap::replaceSelection(cur);
		cur.recordUndo();
		cur.insert(new InsetNewline(inp));
		cur.posForward();
		moveCursor(cur, false);
		break;
	}

	case LFUN_TAB_INSERT: {
		bool const multi_par_selection = cur.selection() &&
			cur.selBegin().pit() != cur.selEnd().pit();
		if (multi_par_selection) {
			// If there is a multi-paragraph selection, a tab is inserted
			// at the beginning of each paragraph.
			cur.recordUndoSelection();
			pit_type const pit_end = cur.selEnd().pit();
			for (pit_type pit = cur.selBegin().pit(); pit <= pit_end; pit++) {
				pars_[pit].insertChar(0, '\t',
						      bv->buffer().params().trackChanges);
				// Update the selection pos to make sure the selection does not
				// change as the inserted tab will increase the logical pos.
				if (cur.realAnchor().pit() == pit)
					cur.realAnchor().forwardPos();
				if (cur.pit() == pit)
					cur.forwardPos();
			}
			cur.finishUndo();
		} else {
			// Maybe we shouldn't allow tabs within a line, because they
			// are not (yet) aligned as one might do expect.
			FuncRequest cmd(LFUN_SELF_INSERT, from_ascii("\t"));
			dispatch(cur, cmd);
		}
		break;
	}

	case LFUN_TAB_DELETE: {
		bool const tc = bv->buffer().params().trackChanges;
		if (cur.selection()) {
			// If there is a selection, a tab (if present) is removed from
			// the beginning of each paragraph.
			cur.recordUndoSelection();
			pit_type const pit_end = cur.selEnd().pit();
			for (pit_type pit = cur.selBegin().pit(); pit <= pit_end; pit++) {
				Paragraph & par = paragraphs()[pit];
				if (par.empty())
					continue;
				char_type const c = par.getChar(0);
				if (c == '\t' || c == ' ') {
					// remove either 1 tab or 4 spaces.
					int const n = (c == ' ' ? 4 : 1);
					for (int i = 0; i < n
						  && !par.empty() && par.getChar(0) == c; ++i) {
						if (cur.pit() == pit)
							cur.posBackward();
						if (cur.realAnchor().pit() == pit
							  && cur.realAnchor().pos() > 0 )
							cur.realAnchor().backwardPos();
						par.eraseChar(0, tc);
					}
				}
			}
			cur.finishUndo();
		} else {
			// If there is no selection, try to remove a tab or some spaces
			// before the position of the cursor.
			Paragraph & par = paragraphs()[cur.pit()];
			pos_type const pos = cur.pos();

			if (pos == 0)
				break;

			char_type const c = par.getChar(pos - 1);
			cur.recordUndo();
			if (c == '\t') {
				cur.posBackward();
				par.eraseChar(cur.pos(), tc);
			} else
				for (int n_spaces = 0;
				     cur.pos() > 0
					     && par.getChar(cur.pos() - 1) == ' '
					     && n_spaces < 4;
				     ++n_spaces) {
					cur.posBackward();
					par.eraseChar(cur.pos(), tc);
				}
			cur.finishUndo();
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

	case LFUN_PARAGRAPH_BREAK:
		cap::replaceSelection(cur);
		breakParagraph(cur, cmd.argument() == "inverse");
		cur.resetAnchor();
		break;

	case LFUN_INSET_INSERT: {
		cur.recordUndo();

		// We have to avoid triggering InstantPreview loading
		// before inserting into the document. See bug #5626.
		bool loaded = bv->buffer().isFullyLoaded();
		bv->buffer().setFullyLoaded(false);
		Inset * inset = createInset(&bv->buffer(), cmd);
		bv->buffer().setFullyLoaded(loaded);

		if (inset) {
			// FIXME (Abdel 01/02/2006):
			// What follows would be a partial fix for bug 2154:
			//   http://www.lyx.org/trac/ticket/2154
			// This automatically put the label inset _after_ a
			// numbered section. It should be possible to extend the mechanism
			// to any kind of LateX environement.
			// The correct way to fix that bug would be at LateX generation.
			// I'll let the code here for reference as it could be used for some
			// other feature like "automatic labelling".
			/*
			Paragraph & par = pars_[cur.pit()];
			if (inset->lyxCode() == LABEL_CODE
				&& !par.layout().counter.empty() {
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
			if (inset->editable() && inset->asInsetText())
				inset->edit(cur, true);
			else
				cur.posForward();

			// trigger InstantPreview now
			if (inset->lyxCode() == EXTERNAL_CODE) {
				InsetExternal & ins =
					static_cast<InsetExternal &>(*inset);
				ins.updatePreview();
			}
		}

		break;
	}

	case LFUN_INSET_DISSOLVE: {
		if (dissolveInset(cur)) {
			needsUpdate = true;
			cur.forceBufferUpdate();
		}
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

	case LFUN_IPAMACRO_INSERT: {
		string const arg = cmd.getArg(0);
		if (arg == "deco") {
			// Open the inset, and move the current selection
			// inside it.
			doInsertInset(cur, this, cmd, true, true);
			cur.posForward();
			// Some insets are numbered, others are shown in the outline pane so
			// let's update the labels and the toc backend.
			cur.forceBufferUpdate();
			break;
		}
		if (arg == "tone-falling")
			ipaChar(cur, InsetIPAChar::TONE_FALLING);
		else if (arg == "tone-rising")
			ipaChar(cur, InsetIPAChar::TONE_RISING);
		else if (arg == "tone-high-rising")
			ipaChar(cur, InsetIPAChar::TONE_HIGH_RISING);
		else if (arg == "tone-low-rising")
			ipaChar(cur, InsetIPAChar::TONE_LOW_RISING);
		else if (arg == "tone-high-rising-falling")
			ipaChar(cur, InsetIPAChar::TONE_HIGH_RISING_FALLING);
		else if (arg.empty())
			lyxerr << "LyX function 'ipamacro-insert' needs an argument." << endl;
		else
			lyxerr << "Wrong argument for LyX function 'ipamacro-insert'." << endl;
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
		LASSERT(cur.selBegin().idx() == cur.selEnd().idx(), break);
		cap::replaceSelection(cur);

		// without argument?
		string const arg = to_utf8(cmd.argument());
		if (arg.empty()) {
			bool tryGraphics = true;
			if (theClipboard().isInternal())
				pasteFromStack(cur, bv->buffer().errorList("Paste"), 0);
			else if (theClipboard().hasTextContents()) {
				if (pasteClipboardText(cur, bv->buffer().errorList("Paste"),
				                       true, Clipboard::AnyTextType))
					tryGraphics = false;
			}
			if (tryGraphics && theClipboard().hasGraphicsContents())
				pasteClipboardGraphics(cur, bv->buffer().errorList("Paste"));
		} else if (isStrUnsignedInt(arg)) {
			// we have a numerical argument
			pasteFromStack(cur, bv->buffer().errorList("Paste"),
				       convert<unsigned int>(arg));
		} else if (arg == "html" || arg == "latex") {
			Clipboard::TextType type = (arg == "html") ?
				Clipboard::HtmlTextType : Clipboard::LaTeXTextType;
			pasteClipboardText(cur, bv->buffer().errorList("Paste"), true, type);
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
			else if (arg == "emf")
				type = Clipboard::EmfGraphicsType;
			else if (arg == "wmf")
				type = Clipboard::WmfGraphicsType;
			else
				// We used to assert, but couldn't the argument come from, say, the
				// minibuffer and just be mistyped?
				LYXERR0("Unrecognized graphics type: " << arg);

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
			convert<string>(tm->cursorX(cur.top(), cur.boundary()))
			+ ' ' + convert<string>(tm->cursorY(cur.top(), cur.boundary()))));
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
			tm->setCursorFromCoordinates(cur, x, y);
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

		if (owner_->forcePlainLayout())
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

		Layout::LaTeXArgMap args = tclass[layout].args();
		Layout::LaTeXArgMap::const_iterator lait = args.begin();
		Layout::LaTeXArgMap::const_iterator const laend = args.end();
		for (; lait != laend; ++lait) {
			Layout::latexarg arg = (*lait).second;
			if (arg.autoinsert) {
				FuncRequest cmd(LFUN_ARGUMENT_INSERT, (*lait).first);
				lyx::dispatch(cmd);
			}
		}

		break;
	}

	case LFUN_ENVIRONMENT_SPLIT: {
		bool const outer = cmd.argument() == "outer";
		Paragraph const & para = cur.paragraph();
		docstring layout = para.layout().name();
		depth_type split_depth = cur.paragraph().params().depth();
		if (outer) {
			// check if we have an environment in our nesting hierarchy
			pit_type pit = cur.pit();
			Paragraph cpar = pars_[pit];
			while (true) {
				if (pit == 0 || cpar.params().depth() == 0)
					break;
				--pit;
				cpar = pars_[pit];
				if (cpar.params().depth() < split_depth
				    && cpar.layout().isEnvironment()) {
						layout = cpar.layout().name();
						split_depth = cpar.params().depth();
				}
			}
		}
		if (cur.pos() > 0)
			lyx::dispatch(FuncRequest(LFUN_PARAGRAPH_BREAK));
		if (outer) {
			while (cur.paragraph().params().depth() > split_depth)
				lyx::dispatch(FuncRequest(LFUN_DEPTH_DECREMENT));
		}
		bool const morecont = cur.lastpos() > cur.pos();
		// FIXME This hardcoding is bad
		docstring const sep =
				cur.buffer()->params().documentClass().hasLayout(from_ascii("Separator"))
					? from_ascii("Separator") : from_ascii("--Separator--");
		lyx::dispatch(FuncRequest(LFUN_LAYOUT, sep));
		lyx::dispatch(FuncRequest(LFUN_PARAGRAPH_BREAK, "inverse"));
		if (morecont)
			lyx::dispatch(FuncRequest(LFUN_DOWN));
		lyx::dispatch(FuncRequest(LFUN_LAYOUT, layout));

		break;
	}

	case LFUN_CLIPBOARD_PASTE:
		cap::replaceSelection(cur);
		pasteClipboardText(cur, bv->buffer().errorList("Paste"),
			       cmd.argument() == "paragraph");
		bv->buffer().errors("Paste");
		break;

	case LFUN_CLIPBOARD_PASTE_SIMPLE:
		cap::replaceSelection(cur);
		pasteSimpleText(cur, cmd.argument() == "paragraph");
		break;

	case LFUN_PRIMARY_SELECTION_PASTE:
		cap::replaceSelection(cur);
		pasteString(cur, theSelection().get(),
			    cmd.argument() == "paragraph");
		break;

	case LFUN_SELECTION_PASTE:
		// Copy the selection buffer to the clipboard stack,
		// because we want it to appear in the "Edit->Paste
		// recent" menu.
		cap::replaceSelection(cur);
		cap::copySelectionToStack();
		cap::pasteSelection(bv->cursor(), bv->buffer().errorList("Paste"));
		bv->buffer().errors("Paste");
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
		cap::replaceSelection(cur);
		cur.recordUndo();

		Paragraph const & par = cur.paragraph();
		pos_type pos = cur.pos();
		// Ignore deleted text before cursor
		while (pos > 0 && par.isDeleted(pos - 1))
			--pos;

		BufferParams const & bufparams = bv->buffer().params();
		bool const hebrew =
			par.getFontSettings(bufparams, pos).language()->lang() == "hebrew";
		bool const allow_inset_quote = !(par.isPassThru() || hebrew);

		string const arg = to_utf8(cmd.argument());
		if (allow_inset_quote) {
			char_type c = ' ';
			if (pos > 0 && (!cur.prevInset() || !cur.prevInset()->isSpace()))
				c = par.getChar(pos - 1);
			InsetQuotes::QuoteTimes const quote_type = (arg == "single")
				? InsetQuotes::SingleQuotes : InsetQuotes::DoubleQuotes;
			cur.insert(new InsetQuotes(cur.buffer(), c, quote_type));
			cur.posForward();
		} else {
			// The cursor might have been invalidated by the replaceSelection.
			cur.buffer()->changed(true);
			string const quote_string = (arg == "single") ? "'" : "\"";
			lyx::dispatch(FuncRequest(LFUN_SELF_INSERT, quote_string));
		}
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
			tm->cursorHome(cur);
			cur.resetAnchor();
			tm->cursorEnd(cur);
			cur.setSelection();
			bv->cursor() = cur;
		}
		break;

	case LFUN_MOUSE_DOUBLE:
		if (cmd.button() == mouse_button::button1) {
			selectWord(cur, WHOLE_WORD);
			bv->cursor() = cur;
		}
		break;

	// Single-click on work area
	case LFUN_MOUSE_PRESS: {
		// We are not marking a selection with the keyboard in any case.
		Cursor & bvcur = cur.bv().cursor();
		bvcur.setMark(false);
		switch (cmd.button()) {
		case mouse_button::button1:
			// Set the cursor
			if (!bv->mouseSetCursor(cur, cmd.argument() == "region-select"))
				cur.screenUpdateFlags(Update::SinglePar | Update::FitCursor);
			if (bvcur.wordSelection())
				selectWord(bvcur, WHOLE_WORD);
			break;

		case mouse_button::button2:
			// Middle mouse pasting.
			bv->mouseSetCursor(cur);
			lyx::dispatch(
				FuncRequest(LFUN_COMMAND_ALTERNATIVES,
					    "selection-paste ; primary-selection-paste paragraph"));
			cur.noScreenUpdate();
			break;

		case mouse_button::button3: {
			// Don't do anything if we right-click a
			// selection, a context menu will popup.
			if (bvcur.selection() && cur >= bvcur.selectionBegin()
			    && cur < bvcur.selectionEnd()) {
				cur.noScreenUpdate();
				return;
			}
			if (!bv->mouseSetCursor(cur, false))
				cur.screenUpdateFlags(Update::SinglePar | Update::FitCursor);
			break;
		}

		default:
			break;
		} // switch (cmd.button())
		break;
	}
	case LFUN_MOUSE_MOTION: {
		// Mouse motion with right or middle mouse do nothing for now.
		if (cmd.button() != mouse_button::button1) {
			cur.noScreenUpdate();
			return;
		}
		// ignore motions deeper nested than the real anchor
		Cursor & bvcur = cur.bv().cursor();
		if (!bvcur.realAnchor().hasPart(cur)) {
			cur.undispatched();
			break;
		}
		CursorSlice old = bvcur.top();

		int const wh = bv->workHeight();
		int const y = max(0, min(wh - 1, cmd.y()));

		tm->setCursorFromCoordinates(cur, cmd.x(), y);
		cur.setTargetX(cmd.x());
		if (cmd.y() >= wh)
			lyx::dispatch(FuncRequest(LFUN_DOWN_SELECT));
		else if (cmd.y() < 0)
			lyx::dispatch(FuncRequest(LFUN_UP_SELECT));
		// This is to allow jumping over large insets
		if (cur.top() == old) {
			if (cmd.y() >= wh)
				lyx::dispatch(FuncRequest(LFUN_DOWN_SELECT));
			else if (cmd.y() < 0)
				lyx::dispatch(FuncRequest(LFUN_UP_SELECT));
		}
		// We continue with our existing selection or start a new one, so don't
		// reset the anchor.
		bvcur.setCursor(cur);
		bvcur.setSelection(true);
		if (cur.top() == old) {
			// We didn't move one iota, so no need to update the screen.
			cur.screenUpdateFlags(Update::SinglePar | Update::FitCursor);
			//cur.noScreenUpdate();
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
				cur.screenUpdateFlags(Update::SinglePar | Update::FitCursor);
			} else
				cur.noScreenUpdate();
			// FIXME: We could try to handle drag and drop of selection here.
			return;

		case mouse_button::button2:
			// Middle mouse pasting is handled at mouse press time,
			// see LFUN_MOUSE_PRESS.
			cur.noScreenUpdate();
			return;

		case mouse_button::button3:
			// Cursor was set at LFUN_MOUSE_PRESS time.
			// FIXME: If there is a selection we could try to handle a special
			// drag & drop context menu.
			cur.noScreenUpdate();
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
		cur.markNewWordPosition();
		bv->bookmarkEditPosition();
		break;
	}

	case LFUN_HREF_INSERT: {
		// FIXME If we're actually given an argument, shouldn't
		// we use it, whether or not we have a selection?
		docstring content = cmd.argument();
		if (cur.selection()) {
			content = cur.selectionAsString(false);
			cutSelection(cur, true, false);
		}

		InsetCommandParams p(HYPERLINK_CODE);
		if (!content.empty()){
			// if it looks like a link, we'll put it as target,
			// otherwise as name (bug #8792).

			// We can't do:
			//   regex_match(to_utf8(content), matches, link_re)
			// because smatch stores pointers to the substrings rather
			// than making copies of them. And those pointers become
			// invalid after regex_match returns, since it is then
			// being given a temporary object. (Thanks to Georg for
			// figuring that out.)
			regex const link_re("^([a-z]+):.*");
			smatch matches;
			string const c = to_utf8(lowercase(content));

			if (c.substr(0,7) == "mailto:") {
				p["target"] = content;
				p["type"] = from_ascii("mailto:");
			} else if (regex_match(c, matches, link_re)) {
				p["target"] = content;
				string protocol = matches.str(1);
				if (protocol == "file")
					p["type"] = from_ascii("file:");
			} else
				p["name"] = content;
		}
		string const data = InsetCommand::params2string(p);

		// we need to have a target. if we already have one, then
		// that gets used at the default for the name, too, which
		// is probably what is wanted.
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
		string const data = InsetCommand::params2string(p);

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
			inset = createInset(cur.buffer(), cmd0);
		} else {
			inset = createInset(cur.buffer(), cmd);
		}
		if (!inset)
			break;
		cur.recordUndo();
		insertInset(cur, inset);
		cur.posForward();
		break;
	}
	case LFUN_CAPTION_INSERT:
	case LFUN_FOOTNOTE_INSERT:
	case LFUN_NOTE_INSERT:
	case LFUN_BOX_INSERT:
	case LFUN_BRANCH_INSERT:
	case LFUN_PHANTOM_INSERT:
	case LFUN_ERT_INSERT:
	case LFUN_LISTING_INSERT:
	case LFUN_MARGINALNOTE_INSERT:
	case LFUN_ARGUMENT_INSERT:
	case LFUN_INDEX_INSERT:
	case LFUN_PREVIEW_INSERT:
	case LFUN_SCRIPT_INSERT:
	case LFUN_IPA_INSERT:
		// Open the inset, and move the current selection
		// inside it.
		doInsertInset(cur, this, cmd, true, true);
		cur.posForward();
		// Some insets are numbered, others are shown in the outline pane so
		// let's update the labels and the toc backend.
		cur.forceBufferUpdate();
		break;

	case LFUN_FLEX_INSERT: {
		// Open the inset, and move the current selection
		// inside it.
		bool const sel = cur.selection();
		doInsertInset(cur, this, cmd, true, true);
		// Insert auto-insert arguments
		bool autoargs = false;
		Layout::LaTeXArgMap args = cur.inset().getLayout().latexargs();
		Layout::LaTeXArgMap::const_iterator lait = args.begin();
		Layout::LaTeXArgMap::const_iterator const laend = args.end();
		for (; lait != laend; ++lait) {
			Layout::latexarg arg = (*lait).second;
			if (arg.autoinsert) {
				// The cursor might have been invalidated by the replaceSelection.
				cur.buffer()->changed(true);
				FuncRequest cmd(LFUN_ARGUMENT_INSERT, (*lait).first);
				lyx::dispatch(cmd);
				autoargs = true;
			}
		}
		if (!autoargs) {
			if (sel)
				cur.leaveInset(cur.inset());
			cur.posForward();
		}
		// Some insets are numbered, others are shown in the outline pane so
		// let's update the labels and the toc backend.
		cur.forceBufferUpdate();
		break;
	}

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
		// will some content be moved into the inset?
		bool const content = cur.selection();
		// does the content consist of multiple paragraphs?
		bool const singlepar = (cur.selBegin().pit() == cur.selEnd().pit());

		doInsertInset(cur, this, cmd, true, true);
		cur.posForward();

		// If some single-par content is moved into the inset,
		// doInsertInset puts the cursor outside the inset.
		// To insert the caption we put it back into the inset.
		// FIXME cleanup doInsertInset to avoid such dances!
		if (content && singlepar)
			cur.backwardPos();

		ParagraphList & pars = cur.text()->paragraphs();

		DocumentClass const & tclass = bv->buffer().params().documentClass();

		// add a separate paragraph for the caption inset
		pars.push_back(Paragraph());
		pars.back().setInsetOwner(&cur.text()->inset());
		pars.back().setPlainOrDefaultLayout(tclass);
		int cap_pit = pars.size() - 1;

		// if an empty inset was created, we create an additional empty
		// paragraph at the bottom so that the user can choose where to put
		// the graphics (or table).
		if (!content) {
			pars.push_back(Paragraph());
			pars.back().setInsetOwner(&cur.text()->inset());
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
		cur.forceBufferUpdate();
		cur.screenUpdateFlags(Update::Force);
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
		string const data = InsetCommand::params2string(p);
		bv->showDialog("nomenclature", data);
		break;
	}

	case LFUN_INDEX_PRINT: {
		InsetCommandParams p(INDEX_PRINT_CODE);
		if (cmd.argument().empty())
			p["type"] = from_ascii("idx");
		else
			p["type"] = cmd.argument();
		string const data = InsetCommand::params2string(p);
		FuncRequest fr(LFUN_INSET_INSERT, data);
		dispatch(cur, fr);
		break;
	}

	case LFUN_NOMENCL_PRINT:
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

	case LFUN_REGEXP_MODE:
		regexpDispatch(cur, cmd);
		break;

	case LFUN_MATH_MODE:
		if (cmd.argument() == "on")
			// don't pass "on" as argument
			// (it would appear literally in the first cell)
			mathDispatch(cur, FuncRequest(LFUN_MATH_MODE), false);
		else
			mathDispatch(cur, cmd, false);
		break;

	case LFUN_MATH_MACRO:
		if (cmd.argument().empty())
			cur.errorMessage(from_utf8(N_("Missing argument")));
		else {
			cur.recordUndo();
			string s = to_utf8(cmd.argument());
			string const s1 = token(s, ' ', 1);
			int const nargs = s1.empty() ? 0 : convert<int>(s1);
			string const s2 = token(s, ' ', 2);
			MacroType type = MacroTypeNewcommand;
			if (s2 == "def")
				type = MacroTypeDef;
			MathMacroTemplate * inset = new MathMacroTemplate(cur.buffer(),
				from_utf8(token(s, ' ', 0)), nargs, false, type);
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
	case LFUN_MATH_AMS_MATRIX:
	case LFUN_MATH_MATRIX:
	case LFUN_MATH_DELIM:
	case LFUN_MATH_BIGDELIM: {
		cur.recordUndo();
		cap::replaceSelection(cur);
		cur.insert(new InsetMathHull(cur.buffer(), hullSimple));
		checkAndActivateInset(cur, true);
		LASSERT(cur.inMathed(), break);
		cur.dispatch(cmd);
		break;
	}

	case LFUN_FONT_EMPH: {
		Font font(ignore_font, ignore_language);
		font.fontInfo().setEmph(FONT_TOGGLE);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_ITAL: {
		Font font(ignore_font, ignore_language);
		font.fontInfo().setShape(ITALIC_SHAPE);
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

	case LFUN_FONT_STRIKEOUT: {
		Font font(ignore_font, ignore_language);
		font.fontInfo().setStrikeout(FONT_TOGGLE);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_UNDERUNDERLINE: {
		Font font(ignore_font, ignore_language);
		font.fontInfo().setUuline(FONT_TOGGLE);
		toggleAndShow(cur, this, font);
		break;
	}

	case LFUN_FONT_UNDERWAVE: {
		Font font(ignore_font, ignore_language);
		font.fontInfo().setUwave(FONT_TOGGLE);
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
		string const lang_arg = cmd.getArg(0);
		bool const reset = (lang_arg.empty() || lang_arg == "reset");
		Language const * lang =
			reset ? reset_language
			      : languages.getLanguage(lang_arg);
		// we allow reset_language, which is 0, but only if it
		// was requested via empty or "reset" arg.
		if (!lang && !reset)
			break;
		bool const toggle = (cmd.getArg(1) != "set");
		selectWordWhenUnderCursor(cur, WHOLE_WORD_STRICT);
		Font font(ignore_font, lang);
		toggleAndShow(cur, this, font, toggle);
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
		theApp()->handleKeyFunc(cmd.action());
		if (!cmd.argument().empty())
			// FIXME: Are all these characters encoded in one byte in utf8?
			bv->translateAndInsert(cmd.argument()[0], this, cur);
		cur.screenUpdateFlags(Update::FitCursor);
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
			InsetFloatList * ifl = new InsetFloatList(cur.buffer(), to_utf8(cmd.argument()));
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
				arg += " lang=" + from_ascii(cur.getFont().language()->lang());
			}
		}
		bv->showDialog("thesaurus", to_utf8(arg));
		break;
	}

	case LFUN_SPELLING_ADD: {
		docstring word = from_utf8(cmd.getArg(0));
		Language * lang;
		if (word.empty()) {
			word = cur.selectionAsString(false);
			// FIXME
			if (word.size() > 100 || word.empty()) {
				// Get word or selection
				selectWordWhenUnderCursor(cur, WHOLE_WORD);
				word = cur.selectionAsString(false);
			}
			lang = const_cast<Language *>(cur.getFont().language());
		} else if (cmd.getArg(1).empty()) {
			// optional language argument is missing
			// use the language at cursor position
			lang = const_cast<Language *>(cur.getFont().language());
		} else {
			lang = const_cast<Language *>(languages.getLanguage(cmd.getArg(1)));
		}
		WordLangTuple wl(word, lang);
		theSpellChecker()->insert(wl);
		break;
	}

	case LFUN_SPELLING_IGNORE: {
		docstring word = from_utf8(cmd.getArg(0));
		Language * lang;
		if (word.empty()) {
			word = cur.selectionAsString(false);
			// FIXME
			if (word.size() > 100 || word.empty()) {
				// Get word or selection
				selectWordWhenUnderCursor(cur, WHOLE_WORD);
				word = cur.selectionAsString(false);
			}
			lang = const_cast<Language *>(cur.getFont().language());
		} else if (cmd.getArg(1).empty()) {
			lang = const_cast<Language *>(cur.getFont().language());
		} else {
			lang = const_cast<Language *>(languages.getLanguage(cmd.getArg(1)));
		}
		WordLangTuple wl(word, lang);
		theSpellChecker()->accept(wl);
		break;
	}

	case LFUN_SPELLING_REMOVE: {
		docstring word = from_utf8(cmd.getArg(0));
		Language * lang;
		if (word.empty()) {
			word = cur.selectionAsString(false);
			// FIXME
			if (word.size() > 100 || word.empty()) {
				// Get word or selection
				selectWordWhenUnderCursor(cur, WHOLE_WORD);
				word = cur.selectionAsString(false);
			}
			lang = const_cast<Language *>(cur.getFont().language());
		} else if (cmd.getArg(1).empty()) {
			lang = const_cast<Language *>(cur.getFont().language());
		} else {
			lang = const_cast<Language *>(languages.getLanguage(cmd.getArg(1)));
		}
		WordLangTuple wl(word, lang);
		theSpellChecker()->remove(wl);
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
		cur.forceBufferUpdate();
		needsUpdate = true;
		break;

	case LFUN_OUTLINE_DOWN:
		outline(OutlineDown, cur);
		setCursor(cur, cur.pit(), 0);
		cur.forceBufferUpdate();
		needsUpdate = true;
		break;

	case LFUN_OUTLINE_IN:
		outline(OutlineIn, cur);
		cur.forceBufferUpdate();
		needsUpdate = true;
		break;

	case LFUN_OUTLINE_OUT:
		outline(OutlineOut, cur);
		cur.forceBufferUpdate();
		needsUpdate = true;
		break;

	default:
		LYXERR(Debug::ACTION, "Command " << cmd << " not DISPATCHED by Text");
		cur.undispatched();
		break;
	}

	needsUpdate |= (cur.pos() != cur.lastpos()) && cur.selection();

	if (lyxrc.spellcheck_continuously && !needsUpdate) {
		// Check for misspelled text
		// The redraw is useful because of the painting of
		// misspelled markers depends on the cursor position.
		// Trigger a redraw for cursor moves inside misspelled text.
		if (!cur.inTexted()) {
			// move from regular text to math
			needsUpdate = last_misspelled;
		} else if (oldTopSlice != cur.top() || oldBoundary != cur.boundary()) {
			// move inside regular text
			needsUpdate = last_misspelled
				|| cur.paragraph().isMisspelled(cur.pos(), true);
		}
	}

	// FIXME: The cursor flag is reset two lines below
	// so we need to check here if some of the LFUN did touch that.
	// for now only Text::erase() and Text::backspace() do that.
	// The plan is to verify all the LFUNs and then to remove this
	// singleParUpdate boolean altogether.
	if (cur.result().screenUpdate() & Update::Force) {
		singleParUpdate = false;
		needsUpdate = true;
	}

	// FIXME: the following code should go in favor of fine grained
	// update flag treatment.
	if (singleParUpdate) {
		// Inserting characters does not change par height in general. So, try
		// to update _only_ this paragraph. BufferView will detect if a full
		// metrics update is needed anyway.
		cur.screenUpdateFlags(Update::SinglePar | Update::FitCursor);
		return;
	}
	if (!needsUpdate
	    && &oldTopSlice.inset() == &cur.inset()
	    && oldTopSlice.idx() == cur.idx()
	    && !oldSelection // oldSelection is a backup of cur.selection() at the beginning of the function.
	    && !cur.selection())
		// FIXME: it would be better if we could just do this
		//
		//if (cur.result().update() != Update::FitCursor)
		//	cur.noScreenUpdate();
		//
		// But some LFUNs do not set Update::FitCursor when needed, so we
		// do it for all. This is not very harmfull as FitCursor will provoke
		// a full redraw only if needed but still, a proper review of all LFUN
		// should be done and this needsUpdate boolean can then be removed.
		cur.screenUpdateFlags(Update::FitCursor);
	else
		cur.screenUpdateFlags(Update::Force | Update::FitCursor);
}


bool Text::getStatus(Cursor & cur, FuncRequest const & cmd,
			FuncStatus & flag) const
{
	LBUFERR(this == cur.text());

	FontInfo const & fontinfo = cur.real_current_font.fontInfo();
	bool enable = true;
	bool allow_in_passthru = false;
	InsetCode code = NO_CODE;

	switch (cmd.action()) {

	case LFUN_DEPTH_DECREMENT:
		enable = changeDepthAllowed(cur, DEC_DEPTH);
		break;

	case LFUN_DEPTH_INCREMENT:
		enable = changeDepthAllowed(cur, INC_DEPTH);
		break;

	case LFUN_APPENDIX:
		// FIXME We really should not allow this to be put, e.g.,
		// in a footnote, or in ERT. But it would make sense in a
		// branch, so I'm not sure what to do.
		flag.setOnOff(cur.paragraph().params().startOfAppendix());
		break;

	case LFUN_DIALOG_SHOW_NEW_INSET:
		if (cmd.argument() == "bibitem")
			code = BIBITEM_CODE;
		else if (cmd.argument() == "bibtex") {
			code = BIBTEX_CODE;
			// not allowed in description items
			enable = !inDescriptionItem(cur);
		}
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
		else if (cmd.argument() == "index_print")
			code = INDEX_PRINT_CODE;
		else if (cmd.argument() == "listings")
			code = LISTINGS_CODE;
		else if (cmd.argument() == "mathspace")
			code = MATH_HULL_CODE;
		else if (cmd.argument() == "nomenclature")
			code = NOMENCL_CODE;
		else if (cmd.argument() == "nomencl_print")
			code = NOMENCL_PRINT_CODE;
		else if (cmd.argument() == "label")
			code = LABEL_CODE;
		else if (cmd.argument() == "line")
			code = LINE_CODE;
		else if (cmd.argument() == "note")
			code = NOTE_CODE;
		else if (cmd.argument() == "phantom")
			code = PHANTOM_CODE;
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
		// FIXME: If there is a selection, we should check whether there
		// are floats in the selection, but this has performance issues, see
		// LFUN_CHANGE_ACCEPT/REJECT.
		code = FLOAT_CODE;
		if (inDescriptionItem(cur))
			// not allowed in description items
			enable = false;
		else {
			InsetCode const inset_code = cur.inset().lyxCode();

			// algorithm floats cannot be put in another float
			if (to_utf8(cmd.argument()) == "algorithm") {
				enable = inset_code != WRAP_CODE && inset_code != FLOAT_CODE;
				break;
			}

			// for figures and tables: only allow in another
			// float or wrap if it is of the same type and
			// not a subfloat already
			if(cur.inset().lyxCode() == code) {
				InsetFloat const & ins =
					static_cast<InsetFloat const &>(cur.inset());
				enable = ins.params().type == to_utf8(cmd.argument())
					&& !ins.params().subfloat;
			} else if(cur.inset().lyxCode() == WRAP_CODE) {
				InsetWrap const & ins =
					static_cast<InsetWrap const &>(cur.inset());
				enable = ins.params().type == to_utf8(cmd.argument());
			}
		}
		break;
	case LFUN_WRAP_INSERT:
		code = WRAP_CODE;
		// not allowed in description items
		enable = !inDescriptionItem(cur);
		break;
	case LFUN_FLOAT_LIST_INSERT: {
		code = FLOAT_LIST_CODE;
		// not allowed in description items
		enable = !inDescriptionItem(cur);
		if (enable) {
			FloatList const & floats = cur.buffer()->params().documentClass().floats();
			FloatList::const_iterator cit = floats[to_ascii(cmd.argument())];
			// make sure we know about such floats
			if (cit == floats.end() ||
					// and that we know how to generate a list of them
			    (!cit->second.usesFloatPkg() && cit->second.listCommand().empty())) {
				flag.setUnknown(true);
				// probably not necessary, but...
				enable = false;
			}
		}
		break;
	}
	case LFUN_CAPTION_INSERT: {
		code = CAPTION_CODE;
		string arg = cmd.getArg(0);
		bool varia = arg != "LongTableNoNumber";
		if (cur.depth() > 0) {
			if (&cur[cur.depth() - 1].inset())
				varia = cur[cur.depth() - 1].inset().allowsCaptionVariation(arg);
		}
		// not allowed in description items,
		// and in specific insets
		enable = !inDescriptionItem(cur)
			&& (varia || arg.empty() || arg == "Standard");
		break;
	}
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
			cur.buffer()->params().documentClass().insetLayout(from_utf8(s));
		if (il.lyxtype() != InsetLayout::CHARSTYLE &&
		    il.lyxtype() != InsetLayout::CUSTOM &&
		    il.lyxtype() != InsetLayout::ELEMENT &&
		    il.lyxtype ()!= InsetLayout::STANDARD)
			enable = false;
		break;
		}
	case LFUN_BOX_INSERT:
		code = BOX_CODE;
		break;
	case LFUN_BRANCH_INSERT:
		code = BRANCH_CODE;
		if (cur.buffer()->masterBuffer()->params().branchlist().empty()
		    && cur.buffer()->params().branchlist().empty())
			enable = false;
		break;
	case LFUN_IPA_INSERT:
		code = IPA_CODE;
		break;
	case LFUN_PHANTOM_INSERT:
		code = PHANTOM_CODE;
		break;
	case LFUN_LABEL_INSERT:
		code = LABEL_CODE;
		break;
	case LFUN_INFO_INSERT:
		code = INFO_CODE;
		break;
	case LFUN_ARGUMENT_INSERT: {
		code = ARG_CODE;
		allow_in_passthru = true;
		string const arg = cmd.getArg(0);
		if (arg.empty()) {
			enable = false;
			break;
		}
		Layout const & lay = cur.paragraph().layout();
		Layout::LaTeXArgMap args = lay.args();
		Layout::LaTeXArgMap::const_iterator const lait =
				args.find(arg);
		if (lait != args.end()) {
			enable = true;
			pit_type pit = cur.pit();
			pit_type lastpit = cur.pit();
			if (lay.isEnvironment() && !prefixIs(arg, "item:")) {
				// In a sequence of "merged" environment layouts, we only allow
				// non-item arguments once.
				lastpit = cur.lastpit();
				// get the first paragraph in sequence with this layout
				depth_type const current_depth = cur.paragraph().params().depth();
				while (true) {
					if (pit == 0)
						break;
					Paragraph cpar = pars_[pit - 1];
					if (cpar.layout() == lay && cpar.params().depth() == current_depth)
						--pit;
					else
						break;
				}
			}
			for (; pit <= lastpit; ++pit) {
				if (pars_[pit].layout() != lay)
					break;
				InsetList::const_iterator it = pars_[pit].insetList().begin();
				InsetList::const_iterator end = pars_[pit].insetList().end();
				for (; it != end; ++it) {
					if (it->inset->lyxCode() == ARG_CODE) {
						InsetArgument const * ins =
							static_cast<InsetArgument const *>(it->inset);
						if (ins->name() == arg) {
							// we have this already
							enable = false;
							break;
						}
					}
				}
			}
		} else
			enable = false;
		break;
	}
	case LFUN_INDEX_INSERT:
		code = INDEX_CODE;
		break;
	case LFUN_INDEX_PRINT:
		code = INDEX_PRINT_CODE;
		// not allowed in description items
		enable = !inDescriptionItem(cur);
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
		// not allowed in description items
		enable = !inDescriptionItem(cur);
		break;
	case LFUN_HREF_INSERT:
		if (cur.selIsMultiCell() || cur.selIsMultiLine()) {
			enable = false;
			break;
		}
		code = HYPERLINK_CODE;
		break;
	case LFUN_IPAMACRO_INSERT: {
		string const arg = cmd.getArg(0);
		if (arg == "deco")
			code = IPADECO_CODE;
		else
			code = IPACHAR_CODE;
		break;
	}
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
	case LFUN_PREVIEW_INSERT:
		code = PREVIEW_CODE;
		break;
	case LFUN_SCRIPT_INSERT:
		code = SCRIPT_CODE;
		break;

	case LFUN_MATH_INSERT:
	case LFUN_MATH_AMS_MATRIX:
	case LFUN_MATH_MATRIX:
	case LFUN_MATH_DELIM:
	case LFUN_MATH_BIGDELIM:
	case LFUN_MATH_DISPLAY:
	case LFUN_MATH_MODE:
	case LFUN_MATH_MACRO:
	case LFUN_MATH_SUBSCRIPT:
	case LFUN_MATH_SUPERSCRIPT:
		code = MATH_HULL_CODE;
		break;

	case LFUN_REGEXP_MODE:
		code = MATH_HULL_CODE;
		enable = cur.buffer()->isInternal() && !cur.inRegexped();
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
		enable = !cur.paragraph().isPassThru();
		break;

	case LFUN_FONT_ITAL:
		flag.setOnOff(fontinfo.shape() == ITALIC_SHAPE);
		enable = !cur.paragraph().isPassThru();
		break;

	case LFUN_FONT_NOUN:
		flag.setOnOff(fontinfo.noun() == FONT_ON);
		enable = !cur.paragraph().isPassThru();
		break;

	case LFUN_FONT_BOLD:
	case LFUN_FONT_BOLDSYMBOL:
		flag.setOnOff(fontinfo.series() == BOLD_SERIES);
		enable = !cur.paragraph().isPassThru();
		break;

	case LFUN_FONT_SANS:
		flag.setOnOff(fontinfo.family() == SANS_FAMILY);
		enable = !cur.paragraph().isPassThru();
		break;

	case LFUN_FONT_ROMAN:
		flag.setOnOff(fontinfo.family() == ROMAN_FAMILY);
		enable = !cur.paragraph().isPassThru();
		break;

	case LFUN_FONT_TYPEWRITER:
		flag.setOnOff(fontinfo.family() == TYPEWRITER_FAMILY);
		enable = !cur.paragraph().isPassThru();
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

		// explicit text type?
		if (arg == "html") {
			// Do not enable for PlainTextType, since some tidying in the
			// frontend is needed for HTML, which is too unsafe for plain text.
			enable = theClipboard().hasTextContents(Clipboard::HtmlTextType);
			break;
		} else if (arg == "latex") {
			// LaTeX is usually not available on the clipboard with
			// the correct MIME type, but in plain text.
			enable = theClipboard().hasTextContents(Clipboard::PlainTextType) ||
			         theClipboard().hasTextContents(Clipboard::LaTeXTextType);
			break;
		}

		// explicit graphics type?
		Clipboard::GraphicsType type = Clipboard::AnyGraphicsType;
		if ((arg == "pdf" && (type = Clipboard::PdfGraphicsType))
			  || (arg == "png" && (type = Clipboard::PngGraphicsType))
			  || (arg == "jpeg" && (type = Clipboard::JpegGraphicsType))
			  || (arg == "linkback" &&  (type = Clipboard::LinkBackGraphicsType))
			  || (arg == "emf" &&  (type = Clipboard::EmfGraphicsType))
			  || (arg == "wmf" &&  (type = Clipboard::WmfGraphicsType))) {
			enable = theClipboard().hasGraphicsContents(type);
			break;
		}

		// unknown argument
		enable = false;
		break;
	 }

	case LFUN_CLIPBOARD_PASTE:
	case LFUN_CLIPBOARD_PASTE_SIMPLE:
		enable = !theClipboard().empty();
		break;

	case LFUN_PRIMARY_SELECTION_PASTE:
		enable = cur.selection() || !theSelection().empty();
		break;

	case LFUN_SELECTION_PASTE:
		enable = cap::selection();
		break;

	case LFUN_PARAGRAPH_MOVE_UP:
		enable = cur.pit() > 0 && !cur.selection();
		break;

	case LFUN_PARAGRAPH_MOVE_DOWN:
		enable = cur.pit() < cur.lastpit() && !cur.selection();
		break;

	case LFUN_CHANGE_ACCEPT:
	case LFUN_CHANGE_REJECT:
		// In principle, these LFUNs should only be enabled if there
		// is a change at the current position/in the current selection.
		// However, without proper optimizations, this will inevitably
		// result in unacceptable performance - just imagine a user who
		// wants to select the complete content of a long document.
		if (!cur.selection())
			enable = cur.paragraph().isChanged(cur.pos());
		else
			// TODO: context-sensitive enabling of LFUN_CHANGE_ACCEPT/REJECT
			// for selections.
			enable = true;
		break;

	case LFUN_OUTLINE_UP:
	case LFUN_OUTLINE_DOWN:
	case LFUN_OUTLINE_IN:
	case LFUN_OUTLINE_OUT:
		// FIXME: LyX is not ready for outlining within inset.
		enable = isMainText()
			&& cur.buffer()->text().getTocLevel(cur.pit()) != Layout::NOT_IN_TOC;
		break;

	case LFUN_NEWLINE_INSERT:
		// LaTeX restrictions (labels or empty par)
		enable = !cur.paragraph().isPassThru()
			&& cur.pos() > cur.paragraph().beginOfBody();
		break;

	case LFUN_TAB_INSERT:
	case LFUN_TAB_DELETE:
		enable = cur.paragraph().isPassThru();
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
		code = NEWPAGE_CODE;
		enable = !inDescriptionItem(cur);
		break;

	case LFUN_DATE_INSERT: {
		string const format = cmd.argument().empty()
			? lyxrc.date_insert_format : to_utf8(cmd.argument());
		enable = support::os::is_valid_strftime(format);
		break;
	}

	case LFUN_LANGUAGE:
		enable = !cur.paragraph().isPassThru();
		flag.setOnOff(cmd.getArg(0) == cur.real_current_font.language()->lang());
		break;

	case LFUN_PARAGRAPH_BREAK:
		enable = cur.inset().getLayout().isMultiPar();
		break;

	case LFUN_SPELLING_ADD:
	case LFUN_SPELLING_IGNORE:
	case LFUN_SPELLING_REMOVE:
		enable = theSpellChecker() != NULL;
		if (enable && !cmd.getArg(1).empty()) {
			// validate explicitly given language
			Language const * const lang = const_cast<Language *>(languages.getLanguage(cmd.getArg(1)));
			enable &= lang != NULL;
		}
		break;

	case LFUN_LAYOUT: {
		enable = !cur.inset().forcePlainLayout();

		docstring layout = cmd.argument();
		if (layout.empty()) {
			DocumentClass const & tclass = cur.buffer()->params().documentClass();
			layout = tclass.defaultLayoutName();
		}
		flag.setOnOff(layout == cur.paragraph().layout().name());
		break;
	}

	case LFUN_ENVIRONMENT_SPLIT: {
		// FIXME This hardcoding is bad
		if (!cur.buffer()->params().documentClass().hasLayout(from_ascii("Separator"))
		    && !cur.buffer()->params().documentClass().hasLayout(from_ascii("--Separator--"))) {
			enable = false;
			break;
		}
		if (cmd.argument() == "outer") {
			// check if we have an environment in our nesting hierarchy
			bool res = false;
			depth_type const current_depth = cur.paragraph().params().depth();
			pit_type pit = cur.pit();
			Paragraph cpar = pars_[pit];
			while (true) {
				if (pit == 0 || cpar.params().depth() == 0)
					break;
				--pit;
				cpar = pars_[pit];
				if (cpar.params().depth() < current_depth)
					res = cpar.layout().isEnvironment();
			}
			enable = res;
			break;
		}
		else if (cur.paragraph().layout().isEnvironment()) {
			enable = true;
			break;
		}
		enable = false;
		break;
	}

	case LFUN_LAYOUT_PARAGRAPH:
	case LFUN_PARAGRAPH_PARAMS:
	case LFUN_PARAGRAPH_PARAMS_APPLY:
	case LFUN_PARAGRAPH_UPDATE:
		enable = cur.inset().allowParagraphCustomization();
		break;

	// FIXME: why are accent lfuns forbidden with pass_thru layouts?
	case LFUN_ACCENT_ACUTE:
	case LFUN_ACCENT_BREVE:
	case LFUN_ACCENT_CARON:
	case LFUN_ACCENT_CEDILLA:
	case LFUN_ACCENT_CIRCLE:
	case LFUN_ACCENT_CIRCUMFLEX:
	case LFUN_ACCENT_DOT:
	case LFUN_ACCENT_GRAVE:
	case LFUN_ACCENT_HUNGARIAN_UMLAUT:
	case LFUN_ACCENT_MACRON:
	case LFUN_ACCENT_OGONEK:
	case LFUN_ACCENT_TIE:
	case LFUN_ACCENT_TILDE:
	case LFUN_ACCENT_UMLAUT:
	case LFUN_ACCENT_UNDERBAR:
	case LFUN_ACCENT_UNDERDOT:
	case LFUN_FONT_DEFAULT:
	case LFUN_FONT_FRAK:
	case LFUN_FONT_SIZE:
	case LFUN_FONT_STATE:
	case LFUN_FONT_UNDERLINE:
	case LFUN_FONT_STRIKEOUT:
	case LFUN_FONT_UNDERUNDERLINE:
	case LFUN_FONT_UNDERWAVE:
	case LFUN_TEXTSTYLE_APPLY:
	case LFUN_TEXTSTYLE_UPDATE:
		enable = !cur.paragraph().isPassThru();
		break;

	case LFUN_WORD_DELETE_FORWARD:
	case LFUN_WORD_DELETE_BACKWARD:
	case LFUN_LINE_DELETE_FORWARD:
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
	case LFUN_SECTION_SELECT:
	case LFUN_BUFFER_BEGIN:
	case LFUN_BUFFER_END:
	case LFUN_BUFFER_BEGIN_SELECT:
	case LFUN_BUFFER_END_SELECT:
	case LFUN_INSET_BEGIN:
	case LFUN_INSET_END:
	case LFUN_INSET_BEGIN_SELECT:
	case LFUN_INSET_END_SELECT:
	case LFUN_PARAGRAPH_UP:
	case LFUN_PARAGRAPH_DOWN:
	case LFUN_LINE_BEGIN:
	case LFUN_LINE_END:
	case LFUN_CHAR_DELETE_FORWARD:
	case LFUN_CHAR_DELETE_BACKWARD:
	case LFUN_WORD_UPCASE:
	case LFUN_WORD_LOWCASE:
	case LFUN_WORD_CAPITALIZE:
	case LFUN_CHARS_TRANSPOSE:
	case LFUN_SERVER_GET_XY:
	case LFUN_SERVER_SET_XY:
	case LFUN_SERVER_GET_LAYOUT:
	case LFUN_SELF_INSERT:
	case LFUN_UNICODE_INSERT:
	case LFUN_THESAURUS_ENTRY:
	case LFUN_ESCAPE:
		// these are handled in our dispatch()
		enable = true;
		break;

	case LFUN_INSET_INSERT: {
		string const type = cmd.getArg(0);
		if (type == "toc") {
			code = TOC_CODE;
			// not allowed in description items
			//FIXME: couldn't this be merged in Inset::insetAllowed()?
			enable = !inDescriptionItem(cur);
		} else {
			enable = true;
		}
		break;
	}

	default:
		return false;
	}

	if (code != NO_CODE
	    && (cur.empty()
		|| !cur.inset().insetAllowed(code)
		|| (cur.paragraph().layout().pass_thru && !allow_in_passthru)))
		enable = false;

	flag.setEnabled(enable);
	return true;
}


void Text::pasteString(Cursor & cur, docstring const & clip,
		bool asParagraphs)
{
	if (!clip.empty()) {
		cur.recordUndo();
		if (asParagraphs)
			insertStringAsParagraphs(cur, clip, cur.current_font);
		else
			insertStringAsLines(cur, clip, cur.current_font);
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
