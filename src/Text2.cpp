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
#include "Language.h"
#include "Layout.h"
#include "Lexer.h"
#include "LyX.h"
#include "LyXRC.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "TextClass.h"
#include "TextMetrics.h"

#include "insets/InsetCollapsable.h"

#include "mathed/InsetMathHull.h"

#include "support/lassert.h"
#include "support/debug.h"
#include "support/gettext.h"
#include "support/lyxalgo.h"
#include "support/textutils.h"

#include <sstream>

using namespace std;

namespace lyx {

bool Text::isMainText() const
{
	return &owner_->buffer().text() == this;
}


// Note that this is supposed to return a fully realized font.
FontInfo Text::layoutFont(pit_type const pit) const
{
	Layout const & layout = pars_[pit].layout();

	if (!pars_[pit].getDepth())  {
		FontInfo lf = layout.resfont;
		// In case the default family has been customized
		if (layout.font.family() == INHERIT_FAMILY)
			lf.setFamily(owner_->buffer().params().getFont().fontInfo().family());
		FontInfo icf = owner_->getLayout().font();
		icf.realize(lf);
		return icf;
	}

	FontInfo font = layout.font;
	// Realize with the fonts of lesser depth.
	//font.realize(outerFont(pit));
	font.realize(owner_->buffer().params().getFont().fontInfo());

	return font;
}


// Note that this is supposed to return a fully realized font.
FontInfo Text::labelFont(Paragraph const & par) const
{
	Buffer const & buffer = owner_->buffer();
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


void Text::setCharFont(pit_type pit,
		pos_type pos, Font const & fnt, Font const & display_font)
{
	Buffer const & buffer = owner_->buffer();
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
			tp = outerHook(tp);
			if (tp != pit_type(paragraphs().size()))
				layoutfont.realize(pars_[tp].layout().font);
		}
	}

	// Inside inset, apply the inset's font attributes if any
	// (charstyle!)
	if (!isMainText())
		layoutfont.realize(display_font.fontInfo());

	layoutfont.realize(buffer.params().getFont().fontInfo());

	// Now, reduce font against full layout font
	font.fontInfo().reduce(layoutfont);

	pars_[pit].setFont(pos, font);
}


void Text::setInsetFont(BufferView const & bv, pit_type pit,
		pos_type pos, Font const & font)
{
	Inset * const inset = pars_[pit].getInset(pos);
	LASSERT(inset && inset->resetFontEdit(), return);

	CursorSlice::idx_type endidx = inset->nargs();
	for (CursorSlice cs(*inset); cs.idx() != endidx; ++cs.idx()) {
		Text * text = cs.text();
		if (text) {
			// last position of the cell
			CursorSlice cellend = cs;
			cellend.pit() = cellend.lastpit();
			cellend.pos() = cellend.lastpos();
			text->setFont(bv, cs, cellend, font);
		}
	}
}


void Text::setLayout(pit_type start, pit_type end,
		     docstring const & layout)
{
	LASSERT(start != end, return);

	Buffer const & buffer = owner_->buffer();
	BufferParams const & bp = buffer.params();
	Layout const & lyxlayout = bp.documentClass()[layout];

	for (pit_type pit = start; pit != end; ++pit) {
		Paragraph & par = pars_[pit];
		par.applyLayout(lyxlayout);
		if (lyxlayout.margintype == MARGIN_MANUAL)
			par.setLabelWidthString(par.expandLabel(lyxlayout, bp));
	}
}


// set layout over selection and make a total rebreak of those paragraphs
void Text::setLayout(Cursor & cur, docstring const & layout)
{
	LBUFERR(this == cur.text());

	pit_type start = cur.selBegin().pit();
	pit_type end = cur.selEnd().pit() + 1;
	cur.recordUndoSelection();
	setLayout(start, end, layout);
	cur.setCurrentFont();
	cur.forceBufferUpdate();
}


static bool changeDepthAllowed(Text::DEPTH_CHANGE type,
			Paragraph const & par, int max_depth)
{
	int const depth = par.params().depth();
	if (type == Text::INC_DEPTH && depth < max_depth)
		return true;
	if (type == Text::DEC_DEPTH && depth > 0)
		return true;
	return false;
}


bool Text::changeDepthAllowed(Cursor & cur, DEPTH_CHANGE type) const
{
	LBUFERR(this == cur.text());
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
	LBUFERR(this == cur.text());
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
	cur.forceBufferUpdate();
}


void Text::setFont(Cursor & cur, Font const & font, bool toggleall)
{
	LASSERT(this == cur.text(), return);

	// If there is a selection, record undo before the cursor font is changed.
	if (cur.selection())
		cur.recordUndoSelection();

	// Set the current_font
	// Determine basis font
	FontInfo layoutfont;
	pit_type pit = cur.pit();
	if (cur.pos() < pars_[pit].beginOfBody())
		layoutfont = labelFont(pars_[pit]);
	else
		layoutfont = layoutFont(pit);

	// Update current font
	cur.real_current_font.update(font,
					cur.buffer()->params().language,
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
	Font newfont = font;

	if (toggleall) {	
		// Toggling behaves as follows: We check the first character of the
		// selection. If it's (say) got EMPH on, then we set to off; if off,
		// then to on. With families and the like, we set it to INHERIT, if
		// we already have it.
		CursorSlice const & sl = cur.selBegin();
		Text const & text = *sl.text();
		Paragraph const & par = text.getPar(sl.pit());
	
		// get font at the position
		Font oldfont = par.getFont(cur.bv().buffer().params(), sl.pos(),
			text.outerFont(sl.pit()));
		FontInfo const & oldfi = oldfont.fontInfo();
	
		FontInfo & newfi = newfont.fontInfo();
	
		FontFamily newfam = newfi.family();
		if (newfam !=	INHERIT_FAMILY && newfam != IGNORE_FAMILY &&
				newfam == oldfi.family())
			newfi.setFamily(INHERIT_FAMILY);
		
		FontSeries newser = newfi.series();
		if (newser == BOLD_SERIES && oldfi.series() == BOLD_SERIES)
			newfi.setSeries(INHERIT_SERIES);
	
		FontShape newshp = newfi.shape();
		if (newshp != INHERIT_SHAPE && newshp != IGNORE_SHAPE &&
				newshp == oldfi.shape())
			newfi.setShape(INHERIT_SHAPE);

		ColorCode newcol = newfi.color();
		if (newcol != Color_none && newcol != Color_inherit 
		    && newcol != Color_ignore && newcol == oldfi.color())
			newfi.setColor(Color_none);

		// ON/OFF ones
		if (newfi.emph() == FONT_TOGGLE)
			newfi.setEmph(oldfi.emph() == FONT_OFF ? FONT_ON : FONT_OFF);
		if (newfi.underbar() == FONT_TOGGLE)
			newfi.setUnderbar(oldfi.underbar() == FONT_OFF ? FONT_ON : FONT_OFF);
		if (newfi.strikeout() == FONT_TOGGLE)
			newfi.setStrikeout(oldfi.strikeout() == FONT_OFF ? FONT_ON : FONT_OFF);
		if (newfi.uuline() == FONT_TOGGLE)
			newfi.setUuline(oldfi.uuline() == FONT_OFF ? FONT_ON : FONT_OFF);
		if (newfi.uwave() == FONT_TOGGLE)
			newfi.setUwave(oldfi.uwave() == FONT_OFF ? FONT_ON : FONT_OFF);
		if (newfi.noun() == FONT_TOGGLE)
			newfi.setNoun(oldfi.noun() == FONT_OFF ? FONT_ON : FONT_OFF);
		if (newfi.number() == FONT_TOGGLE)
			newfi.setNumber(oldfi.number() == FONT_OFF ? FONT_ON : FONT_OFF);
	}

	setFont(cur.bv(), cur.selectionBegin().top(), 
		cur.selectionEnd().top(), newfont);
}


void Text::setFont(BufferView const & bv, CursorSlice const & begin,
		CursorSlice const & end, Font const & font)
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
		if (inset && inset->resetFontEdit()) {
			// We need to propagate the font change to all
			// text cells of the inset (bugs 1973, 6919).
			setInsetFont(bv, pit, pos, font);
		}
		TextMetrics const & tm = bv.textMetrics(this);
		Font f = tm.displayFont(pit, pos);
		f.update(font, language);
		setCharFont(pit, pos, f, tm.font_);
		// font change may change language... 
		// spell checker has to know that
		pars_[pit].requestSpellCheck(pos);
	}
}


bool Text::cursorTop(Cursor & cur)
{
	LBUFERR(this == cur.text());
	return setCursor(cur, 0, 0);
}


bool Text::cursorBottom(Cursor & cur)
{
	LBUFERR(this == cur.text());
	return setCursor(cur, cur.lastpit(), lyx::prev(paragraphs().end(), 1)->size());
}


void Text::toggleFree(Cursor & cur, Font const & font, bool toggleall)
{
	LBUFERR(this == cur.text());
	// If the mask is completely neutral, tell user
	if (font.fontInfo() == ignore_font && font.language() == ignore_language) {
		// Could only happen with user style
		cur.message(_("No font change defined."));
		return;
	}

	// Try implicit word selection
	// If there is a change in the language the implicit word selection
	// is disabled.
	CursorSlice const resetCursor = cur.top();
	bool const implicitSelection =
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
	LBUFERR(this == cur.text());

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


void Text::setLabelWidthStringToSequence(Cursor const & cur,
		docstring const & s)
{
	Cursor c = cur;
	// Find first of same layout in sequence
	while (!isFirstInSequence(c.pit())) {
		c.pit() = depthHook(c.pit(), c.paragraph().getDepth());
	}

	// now apply label width string to every par
	// in sequence
	depth_type const depth = c.paragraph().getDepth();
	Layout const & layout = c.paragraph().layout();
	for ( ; c.pit() <= c.lastpit() ; ++c.pit()) {
		while (c.paragraph().getDepth() > depth) {
			++c.pit();
			if (c.pit() > c.lastpit())
				return;
		}
		if (c.paragraph().getDepth() < depth)
			return;
		if (c.paragraph().layout() != layout)
			return;
		c.recordUndo();
		c.paragraph().setLabelWidthString(s);
	}
}


void Text::setParagraphs(Cursor & cur, docstring arg, bool merge) 
{
	LBUFERR(cur.text());

	//FIXME UNICODE
	string const argument = to_utf8(arg);
	depth_type priordepth = -1;
	Layout priorlayout;
	Cursor c(cur.bv());
	c.setCursor(cur.selectionBegin());
	for ( ; c <= cur.selectionEnd() ; ++c.pit()) {
		Paragraph & par = c.paragraph();
		ParagraphParameters params = par.params();
		params.read(argument, merge);
		// Changes to label width string apply to all paragraphs
		// with same layout in a sequence.
		// Do this only once for a selected range of paragraphs
		// of the same layout and depth.
		c.recordUndo();
		par.params().apply(params, par.layout());
		if (par.getDepth() != priordepth || par.layout() != priorlayout)
			setLabelWidthStringToSequence(c, params.labelWidthString());
		priordepth = par.getDepth();
		priorlayout = par.layout();
	}
}


void Text::setParagraphs(Cursor & cur, ParagraphParameters const & p) 
{
	LBUFERR(cur.text());

	depth_type priordepth = -1;
	Layout priorlayout;
	Cursor c(cur.bv());
	c.setCursor(cur.selectionBegin());
	for ( ; c < cur.selectionEnd() ; ++c.pit()) {
		Paragraph & par = c.paragraph();
		// Changes to label width string apply to all paragraphs
		// with same layout in a sequence.
		// Do this only once for a selected range of paragraphs
		// of the same layout and depth.
		cur.recordUndo();
		par.params().apply(p, par.layout());
		if (par.getDepth() != priordepth || par.layout() != priorlayout)
			setLabelWidthStringToSequence(c,
				par.params().labelWidthString());
		priordepth = par.getDepth();
		priorlayout = par.layout();
	}
}


// this really should just insert the inset and not move the cursor.
void Text::insertInset(Cursor & cur, Inset * inset)
{
	LBUFERR(this == cur.text());
	LBUFERR(inset);
	cur.paragraph().insertInset(cur.pos(), inset, cur.current_font,
		Change(cur.buffer()->params().trackChanges
		? Change::INSERTED : Change::UNCHANGED));
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
	LASSERT(par != int(paragraphs().size()), return);
	cur.pit() = par;
	cur.pos() = pos;

	// now some strict checking
	Paragraph & para = getPar(par);

	// None of these should happen, but we're scaredy-cats
	if (pos < 0) {
		LYXERR0("Don't like -1!");
		LATTEST(false);
	}

	if (pos > para.size()) {
		LYXERR0("Don't like 1, pos: " << pos
		       << " size: " << para.size()
		       << " par: " << par);
		LATTEST(false);
	}
}


void Text::setCursorIntern(Cursor & cur,
			      pit_type par, pos_type pos, bool setfont, bool boundary)
{
	LBUFERR(this == cur.text());
	cur.boundary(boundary);
	setCursor(cur.top(), par, pos);
	if (setfont)
		cur.setCurrentFont();
}


bool Text::checkAndActivateInset(Cursor & cur, bool front)
{
	if (front && cur.pos() == cur.lastpos())
		return false;
	if (!front && cur.pos() == 0)
		return false;
	Inset * inset = front ? cur.nextInset() : cur.prevInset();
	if (!inset || !inset->editable())
		return false;
	if (cur.selection() && cur.realAnchor().find(inset) == -1)
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
	cur.setCurrentFont();
	return true;
}


bool Text::checkAndActivateInsetVisual(Cursor & cur, bool movingForward, bool movingLeft)
{
	if (cur.pos() == -1)
		return false;
	if (cur.pos() == cur.lastpos())
		return false;
	Paragraph & par = cur.paragraph();
	Inset * inset = par.isInset(cur.pos()) ? par.getInset(cur.pos()) : 0;
	if (!inset || !inset->editable())
		return false;
	if (cur.selection() && cur.realAnchor().find(inset) == -1)
		return false;
	inset->edit(cur, movingForward, 
		movingLeft ? Inset::ENTRY_DIRECTION_RIGHT : Inset::ENTRY_DIRECTION_LEFT);
	cur.setCurrentFont();
	return true;
}


bool Text::cursorBackward(Cursor & cur)
{
	// Tell BufferView to test for FitCursor in any case!
	cur.screenUpdateFlags(Update::FitCursor);

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
	cur.screenUpdateFlags(Update::FitCursor);

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
		if (lyxrc.mac_like_cursor_movement)
			if (cur.pos() == cur.lastpos())
				updated = setCursor(cur, cur.pit() + 1, getPar(cur.pit() + 1).size());
			else
				updated = setCursor(cur, cur.pit(), cur.lastpos());
		else
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

	// Find a common inset and the corresponding depth.
	size_t depth = 0;
	for (; depth < cur.depth(); ++depth)
		if (&old.inset() == &cur[depth].inset())
			break;

	// Whether a common inset is found and whether the cursor is still in 
	// the same paragraph (possibly nested).
	bool const same_par = depth < cur.depth() && old.pit() == cur[depth].pit();
	bool const same_par_pos = depth == cur.depth() - 1 && same_par 
		&& old.pos() == cur[depth].pos();
	
	// If the chars around the old cursor were spaces, delete one of them.
	if (!same_par_pos) {
		// Only if the cursor has really moved.
		if (old.pos() > 0
		    && old.pos() < oldpar.size()
		    && oldpar.isLineSeparator(old.pos())
		    && oldpar.isLineSeparator(old.pos() - 1)
		    && !oldpar.isDeleted(old.pos() - 1)
		    && !oldpar.isDeleted(old.pos())) {
			oldpar.eraseChar(old.pos() - 1, cur.buffer()->params().trackChanges);
// FIXME: This will not work anymore when we have multiple views of the same buffer
// In this case, we will have to correct also the cursors held by
// other bufferviews. It will probably be easier to do that in a more
// automated way in CursorSlice code. (JMarc 26/09/2001)
			// correct all cursor parts
			if (same_par) {
				fixCursorAfterDelete(cur[depth], old.top());
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
		plist.erase(lyx::next(plist.begin(), old.pit()));
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

	if (oldpar.stripLeadingSpaces(cur.buffer()->params().trackChanges)) {
		need_anchor_change = true;
		// We return true here because the Paragraph contents changed and
		// we need a redraw before further action is processed.
		return true;
	}

	return false;
}


void Text::deleteEmptyParagraphMechanism(pit_type first, pit_type last, bool trackChanges)
{
	LASSERT(first >= 0 && first <= last && last < (int) pars_.size(), return);

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

		// don't delete anything if this is the only remaining paragraph
		// within the given range. Note: Text::acceptOrRejectChanges()
		// sets the cursor to 'first' after calling DEPM
		if (first == last)
			continue;

		// don't delete empty paragraphs with keepempty set
		if (par.allowEmpty())
			continue;

		if (par.empty() || (par.size() == 1 && par.isLineSeparator(0))) {
			pars_.erase(lyx::next(pars_.begin(), pit));
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
