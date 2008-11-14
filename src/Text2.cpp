/**
 * \file text2.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author Alfredo Braunstein
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 * \author Allan Rae
 * \author Stefan Schimanski
 * \author Dekel Tsur
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Text.h"

#include "Bidi.h"
#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Changes.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "DispatchResult.h"
#include "ErrorList.h"
#include "FuncRequest.h"
#include "Language.h"
#include "Layout.h"
#include "Lexer.h"
#include "LyXFunc.h"
#include "LyXRC.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "TextClass.h"
#include "TextMetrics.h"
#include "VSpace.h"

#include "insets/InsetCollapsable.h"

#include "mathed/InsetMathHull.h"

#include "support/lassert.h"
#include "support/debug.h"
#include "support/gettext.h"
#include "support/textutils.h"

#include <boost/next_prior.hpp>

#include <sstream>

using namespace std;

namespace lyx {

Text::Text()
	: autoBreakRows_(false)
{}


bool Text::isMainText(Buffer const & buffer) const
{
	return &buffer.text() == this;
}


// Note that this is supposed to return a fully realized font.
FontInfo Text::layoutFont(Buffer const & buffer, pit_type const pit) const
{
	Layout const & layout = pars_[pit].layout();

	if (!pars_[pit].getDepth())  {
		FontInfo lf = layout.resfont;
		// In case the default family has been customized
		if (layout.font.family() == INHERIT_FAMILY)
			lf.setFamily(buffer.params().getFont().fontInfo().family());
		// FIXME
		// It ought to be possible here just to use Inset::getLayout() and skip
		// the asInsetCollapsable() bit. Unfortunatley, that doesn't work right
		// now, because Inset::getLayout() will return a default-constructed
		// InsetLayout, and that e.g. sets the foreground color to red. So we
		// need to do some work to make that possible.
		InsetCollapsable const * icp = pars_[pit].inInset().asInsetCollapsable();
		if (!icp)
			return lf;
		FontInfo icf = icp->getLayout().font();
		icf.realize(lf);
		return icf;
	}

	FontInfo font = layout.font;
	// Realize with the fonts of lesser depth.
	//font.realize(outerFont(pit, paragraphs()));
	font.realize(buffer.params().getFont().fontInfo());

	return font;
}


// Note that this is supposed to return a fully realized font.
FontInfo Text::labelFont(Buffer const & buffer, Paragraph const & par) const
{
	Layout const & layout = par.layout();

	if (!par.getDepth()) {
		FontInfo lf = layout.reslabelfont;
		// In case the default family has been customized
		if (layout.labelfont.family() == INHERIT_FAMILY)
			lf.setFamily(buffer.params().getFont().fontInfo().family());
		return lf;
	}

	FontInfo font = layout.labelfont;
	// Realize with the fonts of lesser depth.
	font.realize(buffer.params().getFont().fontInfo());

	return font;
}


void Text::setCharFont(Buffer const & buffer, pit_type pit,
		pos_type pos, Font const & fnt, Font const & display_font)
{
	Font font = fnt;
	Layout const & layout = pars_[pit].layout();

	// Get concrete layout font to reduce against
	FontInfo layoutfont;

	if (pos < pars_[pit].beginOfBody())
		layoutfont = layout.labelfont;
	else
		layoutfont = layout.font;

	// Realize against environment font information
	if (pars_[pit].getDepth()) {
		pit_type tp = pit;
		while (!layoutfont.resolved() &&
		       tp != pit_type(paragraphs().size()) &&
		       pars_[tp].getDepth()) {
			tp = outerHook(tp, paragraphs());
			if (tp != pit_type(paragraphs().size()))
				layoutfont.realize(pars_[tp].layout().font);
		}
	}

	// Inside inset, apply the inset's font attributes if any
	// (charstyle!)
	if (!isMainText(buffer))
		layoutfont.realize(display_font.fontInfo());

	layoutfont.realize(buffer.params().getFont().fontInfo());

	// Now, reduce font against full layout font
	font.fontInfo().reduce(layoutfont);

	pars_[pit].setFont(pos, font);
}


void Text::setInsetFont(BufferView const & bv, pit_type pit,
		pos_type pos, Font const & font, bool toggleall)
{
	Inset * const inset = pars_[pit].getInset(pos);
	LASSERT(inset && inset->noFontChange(), /**/);

	CursorSlice::idx_type endidx = inset->nargs();
	for (CursorSlice cs(*inset); cs.idx() != endidx; ++cs.idx()) {
		Text * text = cs.text();
		if (text) {
			// last position of the cell
			CursorSlice cellend = cs;
			cellend.pit() = cellend.lastpit();
			cellend.pos() = cellend.lastpos();
			text->setFont(bv, cs, cellend, font, toggleall);
		}
	}
}


// return past-the-last paragraph influenced by a layout change on pit
pit_type Text::undoSpan(pit_type pit)
{
	pit_type const end = paragraphs().size();
	pit_type nextpit = pit + 1;
	if (nextpit == end)
		return nextpit;
	//because of parindents
	if (!pars_[pit].getDepth())
		return boost::next(nextpit);
	//because of depth constrains
	for (; nextpit != end; ++pit, ++nextpit) {
		if (!pars_[pit].getDepth())
			break;
	}
	return nextpit;
}


void Text::setLayout(Buffer const & buffer, pit_type start, pit_type end,
		docstring const & layout)
{
	LASSERT(start != end, /**/);

	BufferParams const & bufparams = buffer.params();
	Layout const & lyxlayout = bufparams.documentClass()[layout];

	for (pit_type pit = start; pit != end; ++pit) {
		Paragraph & par = pars_[pit];
		par.applyLayout(lyxlayout);
		if (lyxlayout.margintype == MARGIN_MANUAL)
			par.setLabelWidthString(par.translateIfPossible(
				lyxlayout.labelstring(), buffer.params()));
	}
}


// set layout over selection and make a total rebreak of those paragraphs
void Text::setLayout(Cursor & cur, docstring const & layout)
{
	LASSERT(this == cur.text(), /**/);

	pit_type start = cur.selBegin().pit();
	pit_type end = cur.selEnd().pit() + 1;
	pit_type undopit = undoSpan(end - 1);
	recUndo(cur, start, undopit - 1);
	setLayout(cur.buffer(), start, end, layout);
	updateLabels(cur.buffer());
}


static bool changeDepthAllowed(Text::DEPTH_CHANGE type,
			Paragraph const & par, int max_depth)
{
	if (par.layout().labeltype == LABEL_BIBLIO)
		return false;
	int const depth = par.params().depth();
	if (type == Text::INC_DEPTH && depth < max_depth)
		return true;
	if (type == Text::DEC_DEPTH && depth > 0)
		return true;
	return false;
}


bool Text::changeDepthAllowed(Cursor & cur, DEPTH_CHANGE type) const
{
	LASSERT(this == cur.text(), /**/);
	// this happens when selecting several cells in tabular (bug 2630)
	if (cur.selBegin().idx() != cur.selEnd().idx())
		return false;

	pit_type const beg = cur.selBegin().pit();
	pit_type const end = cur.selEnd().pit() + 1;
	int max_depth = (beg != 0 ? pars_[beg - 1].getMaxDepthAfter() : 0);

	for (pit_type pit = beg; pit != end; ++pit) {
		if (lyx::changeDepthAllowed(type, pars_[pit], max_depth))
			return true;
		max_depth = pars_[pit].getMaxDepthAfter();
	}
	return false;
}


void Text::changeDepth(Cursor & cur, DEPTH_CHANGE type)
{
	LASSERT(this == cur.text(), /**/);
	pit_type const beg = cur.selBegin().pit();
	pit_type const end = cur.selEnd().pit() + 1;
	cur.recordUndoSelection();
	int max_depth = (beg != 0 ? pars_[beg - 1].getMaxDepthAfter() : 0);

	for (pit_type pit = beg; pit != end; ++pit) {
		Paragraph & par = pars_[pit];
		if (lyx::changeDepthAllowed(type, par, max_depth)) {
			int const depth = par.params().depth();
			if (type == INC_DEPTH)
				par.params().depth(depth + 1);
			else
				par.params().depth(depth - 1);
		}
		max_depth = par.getMaxDepthAfter();
	}
	// this handles the counter labels, and also fixes up
	// depth values for follow-on (child) paragraphs
	updateLabels(cur.buffer());
}


void Text::setFont(Cursor & cur, Font const & font, bool toggleall)
{
	LASSERT(this == cur.text(), /**/);
	// Set the current_font
	// Determine basis font
	FontInfo layoutfont;
	pit_type pit = cur.pit();
	if (cur.pos() < pars_[pit].beginOfBody())
		layoutfont = labelFont(cur.buffer(), pars_[pit]);
	else
		layoutfont = layoutFont(cur.buffer(), pit);

	// Update current font
	cur.real_current_font.update(font,
					cur.buffer().params().language,
					toggleall);

	// Reduce to implicit settings
	cur.current_font = cur.real_current_font;
	cur.current_font.fontInfo().reduce(layoutfont);
	// And resolve it completely
	cur.real_current_font.fontInfo().realize(layoutfont);

	// if there is no selection that's all we need to do
	if (!cur.selection())
		return;

	// Ok, we have a selection.
	cur.recordUndoSelection();

	setFont(cur.bv(), cur.selectionBegin().top(), 
		cur.selectionEnd().top(), font, toggleall);
}


void Text::setFont(BufferView const & bv, CursorSlice const & begin,
		CursorSlice const & end, Font const & font,
		bool toggleall)
{
	Buffer const & buffer = bv.buffer();

	// Don't use forwardChar here as ditend might have
	// pos() == lastpos() and forwardChar would miss it.
	// Can't use forwardPos either as this descends into
	// nested insets.
	Language const * language = buffer.params().language;
	for (CursorSlice dit = begin; dit != end; dit.forwardPos()) {
		if (dit.pos() == dit.lastpos())
			continue;
		pit_type const pit = dit.pit();
		pos_type const pos = dit.pos();
		Inset * inset = pars_[pit].getInset(pos);
		if (inset && inset->noFontChange()) {
			// We need to propagate the font change to all
			// text cells of the inset (bug 1973).
			// FIXME: This should change, see documentation
			// of noFontChange in Inset.h
			setInsetFont(bv, pit, pos, font, toggleall);
		}
		TextMetrics const & tm = bv.textMetrics(this);
		Font f = tm.displayFont(pit, pos);
		f.update(font, language, toggleall);
		setCharFont(buffer, pit, pos, f, tm.font_);
	}
}


bool Text::cursorTop(Cursor & cur)
{
	LASSERT(this == cur.text(), /**/);
	return setCursor(cur, 0, 0);
}


bool Text::cursorBottom(Cursor & cur)
{
	LASSERT(this == cur.text(), /**/);
	return setCursor(cur, cur.lastpit(), boost::prior(paragraphs().end())->size());
}


void Text::toggleFree(Cursor & cur, Font const & font, bool toggleall)
{
	LASSERT(this == cur.text(), /**/);
	// If the mask is completely neutral, tell user
	if (font.fontInfo() == ignore_font && font.language() == ignore_language) {
		// Could only happen with user style
		cur.message(_("No font change defined."));
		return;
	}

	// Try implicit word selection
	// If there is a change in the language the implicit word selection
	// is disabled.
	CursorSlice resetCursor = cur.top();
	bool implicitSelection =
		font.language() == ignore_language
		&& font.fontInfo().number() == FONT_IGNORE
		&& selectWordWhenUnderCursor(cur, WHOLE_WORD_STRICT);

	// Set font
	setFont(cur, font, toggleall);

	// Implicit selections are cleared afterwards
	// and cursor is set to the original position.
	if (implicitSelection) {
		cur.clearSelection();
		cur.top() = resetCursor;
		cur.resetAnchor();
	}
}


docstring Text::getStringToIndex(Cursor const & cur)
{
	LASSERT(this == cur.text(), /**/);

	if (cur.selection())
		return cur.selectionAsString(false);

	// Try implicit word selection. If there is a change
	// in the language the implicit word selection is
	// disabled.
	Cursor tmpcur = cur;
	selectWord(tmpcur, PREVIOUS_WORD);

	if (!tmpcur.selection())
		cur.message(_("Nothing to index!"));
	else if (tmpcur.selBegin().pit() != tmpcur.selEnd().pit())
		cur.message(_("Cannot index more than one paragraph!"));
	else
		return tmpcur.selectionAsString(false);
	
	return docstring();
}


void Text::setParagraphs(Cursor & cur, docstring arg, bool merge) 
{
	LASSERT(cur.text(), /**/);
	// make sure that the depth behind the selection are restored, too
	pit_type undopit = undoSpan(cur.selEnd().pit());
	recUndo(cur, cur.selBegin().pit(), undopit - 1);

	//FIXME UNICODE
	string const argument = to_utf8(arg);
	for (pit_type pit = cur.selBegin().pit(), end = cur.selEnd().pit();
	     pit <= end; ++pit) {
		Paragraph & par = pars_[pit];
		ParagraphParameters params = par.params();
		params.read(argument, merge);
		par.params().apply(params, par.layout());
	}
}


//FIXME This is a little redundant now, but it's probably worth keeping,
//especially if we're going to go away from using serialization internally
//quite so much.
void Text::setParagraphs(Cursor & cur, ParagraphParameters const & p) 
{
	LASSERT(cur.text(), /**/);
	// make sure that the depth behind the selection are restored, too
	pit_type undopit = undoSpan(cur.selEnd().pit());
	recUndo(cur, cur.selBegin().pit(), undopit - 1);

	for (pit_type pit = cur.selBegin().pit(), end = cur.selEnd().pit();
	     pit <= end; ++pit) {
		Paragraph & par = pars_[pit];
		par.params().apply(p, par.layout());
	}	
}


// this really should just insert the inset and not move the cursor.
void Text::insertInset(Cursor & cur, Inset * inset)
{
	LASSERT(this == cur.text(), /**/);
	LASSERT(inset, /**/);
	cur.paragraph().insertInset(cur.pos(), inset, cur.current_font,
		Change(cur.buffer().params().trackChanges
		? Change::INSERTED : Change::UNCHANGED));
}


// needed to insert the selection
void Text::insertStringAsLines(Cursor & cur, docstring const & str)
{
	cur.buffer().insertStringAsLines(pars_, cur.pit(), cur.pos(),
		cur.current_font, str, autoBreakRows_);
}


// turn double CR to single CR, others are converted into one
// blank. Then insertStringAsLines is called
void Text::insertStringAsParagraphs(Cursor & cur, docstring const & str)
{
	docstring linestr = str;
	bool newline_inserted = false;

	for (string::size_type i = 0, siz = linestr.size(); i < siz; ++i) {
		if (linestr[i] == '\n') {
			if (newline_inserted) {
				// we know that \r will be ignored by
				// insertStringAsLines. Of course, it is a dirty
				// trick, but it works...
				linestr[i - 1] = '\r';
				linestr[i] = '\n';
			} else {
				linestr[i] = ' ';
				newline_inserted = true;
			}
		} else if (isPrintable(linestr[i])) {
			newline_inserted = false;
		}
	}
	insertStringAsLines(cur, linestr);
}


bool Text::setCursor(Cursor & cur, pit_type par, pos_type pos,
			bool setfont, bool boundary)
{
	TextMetrics const & tm = cur.bv().textMetrics(this);
	bool const update_needed = !tm.contains(par);
	Cursor old = cur;
	setCursorIntern(cur, par, pos, setfont, boundary);
	return cur.bv().checkDepm(cur, old) || update_needed;
}


void Text::setCursor(CursorSlice & cur, pit_type par, pos_type pos)
{
	LASSERT(par != int(paragraphs().size()), /**/);
	cur.pit() = par;
	cur.pos() = pos;

	// now some strict checking
	Paragraph & para = getPar(par);

	// None of these should happen, but we're scaredy-cats
	if (pos < 0) {
		lyxerr << "dont like -1" << endl;
		LASSERT(false, /**/);
	}

	if (pos > para.size()) {
		lyxerr << "dont like 1, pos: " << pos
		       << " size: " << para.size()
		       << " par: " << par << endl;
		LASSERT(false, /**/);
	}
}


void Text::setCursorIntern(Cursor & cur,
			      pit_type par, pos_type pos, bool setfont, bool boundary)
{
	LASSERT(this == cur.text(), /**/);
	cur.boundary(boundary);
	setCursor(cur.top(), par, pos);
	if (setfont)
		cur.setCurrentFont();
}


bool Text::checkAndActivateInset(Cursor & cur, bool front)
{
	if (cur.selection())
		return false;
	if (front && cur.pos() == cur.lastpos())
		return false;
	if (!front && cur.pos() == 0)
		return false;
	Inset * inset = front ? cur.nextInset() : cur.prevInset();
	if (!inset || inset->editable() != Inset::HIGHLY_EDITABLE)
		return false;
	/*
	 * Apparently, when entering an inset we are expected to be positioned
	 * *before* it in the containing paragraph, regardless of the direction
	 * from which we are entering. Otherwise, cursor placement goes awry,
	 * and when we exit from the beginning, we'll be placed *after* the
	 * inset.
	 */
	if (!front)
		--cur.pos();
	inset->edit(cur, front);
	return true;
}


bool Text::checkAndActivateInsetVisual(Cursor & cur, bool movingForward, bool movingLeft)
{
	if (cur.selection())
		return false;
	if (cur.pos() == -1)
		return false;
	if (cur.pos() == cur.lastpos())
		return false;
	Paragraph & par = cur.paragraph();
	Inset * inset = par.isInset(cur.pos()) ? par.getInset(cur.pos()) : 0;
	if (!inset || inset->editable() != Inset::HIGHLY_EDITABLE)
		return false;
	inset->edit(cur, movingForward, 
		movingLeft ? Inset::ENTRY_DIRECTION_RIGHT : Inset::ENTRY_DIRECTION_LEFT);
	return true;
}


bool Text::cursorBackward(Cursor & cur)
{
	// Tell BufferView to test for FitCursor in any case!
	cur.updateFlags(Update::FitCursor);

	// not at paragraph start?
	if (cur.pos() > 0) {
		// if on right side of boundary (i.e. not at paragraph end, but line end)
		// -> skip it, i.e. set boundary to true, i.e. go only logically left
		// there are some exceptions to ignore this: lineseps, newlines, spaces
#if 0
		// some effectless debug code to see the values in the debugger
		bool bound = cur.boundary();
		int rowpos = cur.textRow().pos();
		int pos = cur.pos();
		bool sep = cur.paragraph().isSeparator(cur.pos() - 1);
		bool newline = cur.paragraph().isNewline(cur.pos() - 1);
		bool linesep = cur.paragraph().isLineSeparator(cur.pos() - 1);
#endif
		if (!cur.boundary() &&
				cur.textRow().pos() == cur.pos() &&
				!cur.paragraph().isLineSeparator(cur.pos() - 1) &&
				!cur.paragraph().isNewline(cur.pos() - 1) &&
				!cur.paragraph().isSeparator(cur.pos() - 1)) {
			return setCursor(cur, cur.pit(), cur.pos(), true, true);
		}
		
		// go left and try to enter inset
		if (checkAndActivateInset(cur, false))
			return false;
		
		// normal character left
		return setCursor(cur, cur.pit(), cur.pos() - 1, true, false);
	}

	// move to the previous paragraph or do nothing
	if (cur.pit() > 0)
		return setCursor(cur, cur.pit() - 1, getPar(cur.pit() - 1).size(), true, false);
	return false;
}


bool Text::cursorVisLeft(Cursor & cur, bool skip_inset)
{
	Cursor temp_cur = cur;
	temp_cur.posVisLeft(skip_inset);
	if (temp_cur.depth() > cur.depth()) {
		cur = temp_cur;
		return false;
	}
	return setCursor(cur, temp_cur.pit(), temp_cur.pos(), 
		true, temp_cur.boundary());
}


bool Text::cursorVisRight(Cursor & cur, bool skip_inset)
{
	Cursor temp_cur = cur;
	temp_cur.posVisRight(skip_inset);
	if (temp_cur.depth() > cur.depth()) {
		cur = temp_cur;
		return false;
	}
	return setCursor(cur, temp_cur.pit(), temp_cur.pos(),
		true, temp_cur.boundary());
}


bool Text::cursorForward(Cursor & cur)
{
	// Tell BufferView to test for FitCursor in any case!
	cur.updateFlags(Update::FitCursor);

	// not at paragraph end?
	if (cur.pos() != cur.lastpos()) {
		// in front of editable inset, i.e. jump into it?
		if (checkAndActivateInset(cur, true))
			return false;

		TextMetrics const & tm = cur.bv().textMetrics(this);
		// if left of boundary -> just jump to right side
		// but for RTL boundaries don't, because: abc|DDEEFFghi -> abcDDEEF|Fghi
		if (cur.boundary() && !tm.isRTLBoundary(cur.pit(), cur.pos()))
			return setCursor(cur, cur.pit(), cur.pos(), true, false);

		// next position is left of boundary, 
		// but go to next line for special cases like space, newline, linesep
#if 0
		// some effectless debug code to see the values in the debugger
		int endpos = cur.textRow().endpos();
		int lastpos = cur.lastpos();
		int pos = cur.pos();
		bool linesep = cur.paragraph().isLineSeparator(cur.pos());
		bool newline = cur.paragraph().isNewline(cur.pos());
		bool sep = cur.paragraph().isSeparator(cur.pos());
		if (cur.pos() != cur.lastpos()) {
			bool linesep2 = cur.paragraph().isLineSeparator(cur.pos()+1);
			bool newline2 = cur.paragraph().isNewline(cur.pos()+1);
			bool sep2 = cur.paragraph().isSeparator(cur.pos()+1);
		}
#endif
		if (cur.textRow().endpos() == cur.pos() + 1 &&
		    cur.textRow().endpos() != cur.lastpos() &&
				!cur.paragraph().isNewline(cur.pos()) &&
				!cur.paragraph().isLineSeparator(cur.pos()) &&
				!cur.paragraph().isSeparator(cur.pos())) {
			return setCursor(cur, cur.pit(), cur.pos() + 1, true, true);
		}
		
		// in front of RTL boundary? Stay on this side of the boundary because:
		//   ab|cDDEEFFghi -> abc|DDEEFFghi
		if (tm.isRTLBoundary(cur.pit(), cur.pos() + 1))
			return setCursor(cur, cur.pit(), cur.pos() + 1, true, true);
		
		// move right
		return setCursor(cur, cur.pit(), cur.pos() + 1, true, false);
	}

	// move to next paragraph
	if (cur.pit() != cur.lastpit())
		return setCursor(cur, cur.pit() + 1, 0, true, false);
	return false;
}


bool Text::cursorUpParagraph(Cursor & cur)
{
	bool updated = false;
	if (cur.pos() > 0)
		updated = setCursor(cur, cur.pit(), 0);
	else if (cur.pit() != 0)
		updated = setCursor(cur, cur.pit() - 1, 0);
	return updated;
}


bool Text::cursorDownParagraph(Cursor & cur)
{
	bool updated = false;
	if (cur.pit() != cur.lastpit())
		updated = setCursor(cur, cur.pit() + 1, 0);
	else
		updated = setCursor(cur, cur.pit(), cur.lastpos());
	return updated;
}


// fix the cursor `cur' after a characters has been deleted at `where'
// position. Called by deleteEmptyParagraphMechanism
void Text::fixCursorAfterDelete(CursorSlice & cur, CursorSlice const & where)
{
	// Do nothing if cursor is not in the paragraph where the
	// deletion occured,
	if (cur.pit() != where.pit())
		return;

	// If cursor position is after the deletion place update it
	if (cur.pos() > where.pos())
		--cur.pos();

	// Check also if we don't want to set the cursor on a spot behind the
	// pagragraph because we erased the last character.
	if (cur.pos() > cur.lastpos())
		cur.pos() = cur.lastpos();
}


bool Text::deleteEmptyParagraphMechanism(Cursor & cur,
		Cursor & old, bool & need_anchor_change)
{
	//LYXERR(Debug::DEBUG, "DEPM: cur:\n" << cur << "old:\n" << old);

	Paragraph & oldpar = old.paragraph();

	// We allow all kinds of "mumbo-jumbo" when freespacing.
	if (oldpar.isFreeSpacing())
		return false;

	/* Ok I'll put some comments here about what is missing.
	   There are still some small problems that can lead to
	   double spaces stored in the document file or space at
	   the beginning of paragraphs(). This happens if you have
	   the cursor between to spaces and then save. Or if you
	   cut and paste and the selection have a space at the
	   beginning and then save right after the paste. (Lgb)
	*/

	// If old.pos() == 0 and old.pos()(1) == LineSeparator
	// delete the LineSeparator.
	// MISSING

	// If old.pos() == 1 and old.pos()(0) == LineSeparator
	// delete the LineSeparator.
	// MISSING

	bool const same_inset = &old.inset() == &cur.inset();
	bool const same_par = same_inset && old.pit() == cur.pit();
	bool const same_par_pos = same_par && old.pos() == cur.pos();

	// If the chars around the old cursor were spaces, delete one of them.
	if (!same_par_pos) {
		// Only if the cursor has really moved.
		if (old.pos() > 0
		    && old.pos() < oldpar.size()
		    && oldpar.isLineSeparator(old.pos())
		    && oldpar.isLineSeparator(old.pos() - 1)
		    && !oldpar.isDeleted(old.pos() - 1)
		    && !oldpar.isDeleted(old.pos())) {
			oldpar.eraseChar(old.pos() - 1, cur.buffer().params().trackChanges);
// FIXME: This will not work anymore when we have multiple views of the same buffer
// In this case, we will have to correct also the cursors held by
// other bufferviews. It will probably be easier to do that in a more
// automated way in CursorSlice code. (JMarc 26/09/2001)
			// correct all cursor parts
			if (same_par) {
				fixCursorAfterDelete(cur.top(), old.top());
				need_anchor_change = true;
			}
			return true;
		}
	}

	// only do our magic if we changed paragraph
	if (same_par)
		return false;

	// don't delete anything if this is the ONLY paragraph!
	if (old.lastpit() == 0)
		return false;

	// Do not delete empty paragraphs with keepempty set.
	if (oldpar.allowEmpty())
		return false;

	if (oldpar.empty() || (oldpar.size() == 1 && oldpar.isLineSeparator(0))) {
		// Delete old par.
		old.recordUndo(ATOMIC_UNDO,
			   max(old.pit() - 1, pit_type(0)),
			   min(old.pit() + 1, old.lastpit()));
		ParagraphList & plist = old.text()->paragraphs();
		bool const soa = oldpar.params().startOfAppendix();
		plist.erase(boost::next(plist.begin(), old.pit()));
		// do not lose start of appendix marker (bug 4212)
		if (soa && old.pit() < pit_type(plist.size()))
			plist[old.pit()].params().startOfAppendix(true);

		// see #warning (FIXME?) above 
		if (cur.depth() >= old.depth()) {
			CursorSlice & curslice = cur[old.depth() - 1];
			if (&curslice.inset() == &old.inset()
			    && curslice.pit() > old.pit()) {
				--curslice.pit();
				// since a paragraph has been deleted, all the
				// insets after `old' have been copied and
				// their address has changed. Therefore we
				// need to `regenerate' cur. (JMarc)
				cur.updateInsets(&(cur.bottom().inset()));
				need_anchor_change = true;
			}
		}
		return true;
	}

	if (oldpar.stripLeadingSpaces(cur.buffer().params().trackChanges)) {
		need_anchor_change = true;
		// We return true here because the Paragraph contents changed and
		// we need a redraw before further action is processed.
		return true;
	}

	return false;
}


void Text::deleteEmptyParagraphMechanism(pit_type first, pit_type last, bool trackChanges)
{
	LASSERT(first >= 0 && first <= last && last < (int) pars_.size(), /**/);

	for (pit_type pit = first; pit <= last; ++pit) {
		Paragraph & par = pars_[pit];

		// We allow all kinds of "mumbo-jumbo" when freespacing.
		if (par.isFreeSpacing())
			continue;

		for (pos_type pos = 1; pos < par.size(); ++pos) {
			if (par.isLineSeparator(pos) && par.isLineSeparator(pos - 1)
			    && !par.isDeleted(pos - 1)) {
				if (par.eraseChar(pos - 1, trackChanges)) {
					--pos;
				}
			}
		}

		// don't delete anything if this is the only remaining paragraph within the given range
		// note: Text::acceptOrRejectChanges() sets the cursor to 'first' after calling DEPM
		if (first == last)
			continue;

		// don't delete empty paragraphs with keepempty set
		if (par.allowEmpty())
			continue;

		if (par.empty() || (par.size() == 1 && par.isLineSeparator(0))) {
			pars_.erase(boost::next(pars_.begin(), pit));
			--pit;
			--last;
			continue;
		}

		par.stripLeadingSpaces(trackChanges);
	}
}


void Text::recUndo(Cursor & cur, pit_type first, pit_type last) const
{
	cur.recordUndo(ATOMIC_UNDO, first, last);
}


void Text::recUndo(Cursor & cur, pit_type par) const
{
	cur.recordUndo(ATOMIC_UNDO, par, par);
}

} // namespace lyx
