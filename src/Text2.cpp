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

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Changes.h"
#include "Cursor.h"
#include "Language.h"
#include "Layout.h"
#include "LyXRC.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "TextClass.h"
#include "TextMetrics.h"

#include "insets/InsetText.h"

#include "support/lassert.h"
#include "support/gettext.h"

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

	idx_type endidx = inset->nargs();
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
		// Is this a separating paragraph? If so,
		// this needs to be standard layout
		bool const is_separator = par.size() == 1
				&& par.isEnvSeparator(0);
		par.applyLayout(is_separator ? bp.documentClass().defaultLayout() : lyxlayout);
		if (lyxlayout.margintype == MARGIN_MANUAL)
			par.setLabelWidthString(par.expandLabel(lyxlayout, bp));
	}

	deleteEmptyParagraphMechanism(start, end - 1, bp.track_changes);
}


// set layout over selection and make a total rebreak of those paragraphs
void Text::setLayout(Cursor & cur, docstring const & layout)
{
	LBUFERR(this == cur.text());

	pit_type start = cur.selBegin().pit();
	pit_type end = cur.selEnd().pit() + 1;
	cur.recordUndoSelection();
	setLayout(start, end, layout);
	cur.fixIfBroken();
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


bool Text::changeDepthAllowed(Cursor const & cur, DEPTH_CHANGE type) const
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


void Text::changeDepth(Cursor const & cur, DEPTH_CHANGE type)
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
		if (newfi.xout() == FONT_TOGGLE)
			newfi.setXout(oldfi.xout() == FONT_OFF ? FONT_ON : FONT_OFF);
		if (newfi.uuline() == FONT_TOGGLE)
			newfi.setUuline(oldfi.uuline() == FONT_OFF ? FONT_ON : FONT_OFF);
		if (newfi.uwave() == FONT_TOGGLE)
			newfi.setUwave(oldfi.uwave() == FONT_OFF ? FONT_ON : FONT_OFF);
		if (newfi.noun() == FONT_TOGGLE)
			newfi.setNoun(oldfi.noun() == FONT_OFF ? FONT_ON : FONT_OFF);
		if (newfi.number() == FONT_TOGGLE)
			newfi.setNumber(oldfi.number() == FONT_OFF ? FONT_ON : FONT_OFF);
		if (newfi.nospellcheck() == FONT_TOGGLE)
			newfi.setNoSpellcheck(oldfi.nospellcheck() == FONT_OFF ? FONT_ON : FONT_OFF);
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
	return setCursor(cur, cur.lastpit(), prev(paragraphs().end(), 1)->size());
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


docstring Text::getStringForDialog(Cursor & cur)
{
	LBUFERR(this == cur.text());

	if (cur.selection())
		return cur.selectionAsString(false);

	// Try implicit word selection. If there is a change
	// in the language the implicit word selection is
	// disabled.
	selectWordWhenUnderCursor(cur, WHOLE_WORD);
	docstring const & retval = cur.selectionAsString(false);
	cur.clearSelection();
	return retval;
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


void Text::setParagraphs(Cursor const & cur, docstring const & arg, bool merge)
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


void Text::setParagraphs(Cursor const & cur, ParagraphParameters const & p)
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
		Change(cur.buffer()->params().track_changes
		? Change::INSERTED : Change::UNCHANGED));
}


bool Text::setCursor(Cursor & cur, pit_type pit, pos_type pos,
			bool setfont, bool boundary)
{
	TextMetrics const & tm = cur.bv().textMetrics(this);
	bool const update_needed = !tm.contains(pit);
	Cursor old = cur;
	setCursorIntern(cur, pit, pos, setfont, boundary);
	return cur.bv().checkDepm(cur, old) || update_needed;
}


void Text::setCursorIntern(Cursor & cur, pit_type pit, pos_type pos,
                           bool setfont, bool boundary)
{
	LBUFERR(this == cur.text());
	cur.boundary(boundary);
	cur.top().setPitPos(pit, pos);
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
	cur.boundary(false);
	return true;
}


bool Text::checkAndActivateInsetVisual(Cursor & cur, bool movingForward, bool movingLeft)
{
	if (cur.pos() == -1)
		return false;
	if (cur.pos() == cur.lastpos())
		return false;
	Paragraph & par = cur.paragraph();
	Inset * inset = par.isInset(cur.pos()) ? par.getInset(cur.pos()) : nullptr;
	if (!inset || !inset->editable())
		return false;
	if (cur.selection() && cur.realAnchor().find(inset) == -1)
		return false;
	inset->edit(cur, movingForward,
		movingLeft ? Inset::ENTRY_DIRECTION_RIGHT : Inset::ENTRY_DIRECTION_LEFT);
	cur.setCurrentFont();
	cur.boundary(false);
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
				!cur.paragraph().isEnvSeparator(cur.pos() - 1) &&
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
	if (cur.pit() > 0) {
		Paragraph & par = getPar(cur.pit() - 1);
		pos_type lastpos = par.size();
		if (lastpos > 0 && par.isEnvSeparator(lastpos - 1))
			return setCursor(cur, cur.pit() - 1, lastpos - 1, true, false);
		else
			return setCursor(cur, cur.pit() - 1, lastpos, true, false);
	}
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
		if (cur.textRow().endpos() == cur.pos() + 1) {
			if (cur.paragraph().isEnvSeparator(cur.pos()) &&
			    cur.pos() + 1 == cur.lastpos() &&
			    cur.pit() != cur.lastpit()) {
				// move to next paragraph
				return setCursor(cur, cur.pit() + 1, 0, true, false);
			} else if (cur.textRow().endpos() != cur.lastpos() &&
				   !cur.paragraph().isNewline(cur.pos()) &&
				   !cur.paragraph().isEnvSeparator(cur.pos()) &&
				   !cur.paragraph().isLineSeparator(cur.pos()) &&
				   !cur.paragraph().isSeparator(cur.pos())) {
				return setCursor(cur, cur.pit(), cur.pos() + 1, true, true);
			}
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

namespace {

/** delete num_spaces characters between from and to. Return the
 * number of spaces that got physically deleted (not marked as
 * deleted) */
int deleteSpaces(Paragraph & par, pos_type const from, pos_type to,
				  int num_spaces, bool const trackChanges)
{
	if (num_spaces <= 0)
		return 0;

	// First, delete spaces marked as inserted
	int pos = from;
	while (pos < to && num_spaces > 0) {
		Change const & change = par.lookupChange(pos);
		if (change.inserted() && change.currentAuthor()) {
			par.eraseChar(pos, trackChanges);
			--num_spaces;
			--to;
		} else
			++pos;
	}

	// Then remove remaining spaces
	int const psize = par.size();
	par.eraseChars(from, from + num_spaces, trackChanges);
	return psize - par.size();
}

}


bool Text::deleteEmptyParagraphMechanism(Cursor & cur,
		Cursor & old, bool & need_anchor_change)
{
	//LYXERR(Debug::DEBUG, "DEPM: cur:\n" << cur << "old:\n" << old);

	Paragraph & oldpar = old.paragraph();
	bool const trackChanges = cur.buffer()->params().track_changes;
	bool result = false;

	// We do nothing if cursor did not move
	if (cur.top() == old.top())
		return false;

	// We do not do anything on read-only documents
	if (cur.buffer()->isReadonly())
		return false;

	// Whether a common inset is found and whether the cursor is still in
	// the same paragraph (possibly nested).
	int const depth = cur.find(&old.inset());
	bool const same_par = depth != -1 && old.idx() == cur[depth].idx()
		&& old.pit() == cur[depth].pit();

	/*
	 * (1) If the chars around the old cursor were spaces and the
	 * paragraph is not in free spacing mode, delete some of them, but
	 * only if the cursor has really moved.
	 */

	/* There are still some small problems that can lead to
	   double spaces stored in the document file or space at
	   the beginning of paragraphs(). This happens if you have
	   the cursor between two spaces and then save. Or if you
	   cut and paste and the selection has a space at the
	   beginning and then save right after the paste. (Lgb)
	*/
	if (!oldpar.isFreeSpacing()) {
		// find range of spaces around cursors
		pos_type from = old.pos();
		while (from > 0
			   && oldpar.isLineSeparator(from - 1)
			   && !oldpar.isDeleted(from - 1))
			--from;
		pos_type to = old.pos();
		while (to < old.lastpos()
			   && oldpar.isLineSeparator(to)
			   && !oldpar.isDeleted(to))
			++to;

		int num_spaces = to - from;
		// If we are not at the start of the paragraph, keep one space
		if (from != to && from > 0)
			--num_spaces;

		// If cursor is inside range, keep one additional space
		if (same_par && cur.pos() > from && cur.pos() < to)
			--num_spaces;

		// Remove spaces and adapt cursor.
		if (num_spaces > 0) {
			old.recordUndo();
			int const deleted =
				deleteSpaces(oldpar, from, to, num_spaces, trackChanges);
			// correct cur position
			// FIXME: there can be other cursors pointing there, we should update them
			if (same_par) {
				if (cur[depth].pos() >= to)
					cur[depth].pos() -= deleted;
				else if (cur[depth].pos() > from)
					cur[depth].pos() = min(from + 1, old.lastpos());
				need_anchor_change = true;
			}
			result = true;
		}
	}

	/*
	 * (2) If the paragraph where the cursor was is empty, delete it
	 */

	// only do our other magic if we changed paragraph
	if (same_par)
		return result;

	// only do our magic if the paragraph is empty
	if (!oldpar.empty())
		return result;

	// don't delete anything if this is the ONLY paragraph!
	if (old.lastpit() == 0)
		return result;

	// Do not delete empty paragraphs with keepempty set.
	if (oldpar.allowEmpty())
		return result;

	// Delete old par.
	old.recordUndo(max(old.pit() - 1, pit_type(0)),
	               min(old.pit() + 1, old.lastpit()));
	ParagraphList & plist = old.text()->paragraphs();
	bool const soa = oldpar.params().startOfAppendix();
	plist.erase(plist.iterator_at(old.pit()));
	// do not lose start of appendix marker (bug 4212)
	if (soa && old.pit() < pit_type(plist.size()))
		plist[old.pit()].params().startOfAppendix(true);

	// see #warning (FIXME?) above
	if (cur.depth() >= old.depth()) {
		CursorSlice & curslice = cur[old.depth() - 1];
		if (&curslice.inset() == &old.inset()
		    && curslice.idx() == old.idx()
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


void Text::deleteEmptyParagraphMechanism(pit_type first, pit_type last, bool trackChanges)
{
	LASSERT(first >= 0 && first <= last && last < (int) pars_.size(), return);

	for (pit_type pit = first; pit <= last; ++pit) {
		Paragraph & par = pars_[pit];

		/*
		 * (1) Delete consecutive spaces
		 */
		if (!par.isFreeSpacing()) {
			pos_type from = 0;
			while (from < par.size()) {
				// skip non-spaces
				while (from < par.size()
					   && (!par.isLineSeparator(from) || par.isDeleted(from)))
					++from;
				// find string of spaces
				pos_type to = from;
				while (to < par.size()
					   && par.isLineSeparator(to) && !par.isDeleted(to))
					++to;
				// empty? We are done
				if (from == to)
					break;

				int num_spaces = to - from;

				// If we are not at the extremity of the paragraph, keep one space
				if (from != to && from > 0 && to < par.size())
					--num_spaces;

				// Remove spaces if needed
				int const deleted = deleteSpaces(par, from , to, num_spaces, trackChanges);
				from = to - deleted;
			}
		}

		/*
		 * (2) Delete empty pragraphs
		 */

		// don't delete anything if this is the only remaining paragraph
		// within the given range. Note: Text::acceptOrRejectChanges()
		// sets the cursor to 'first' after calling DEPM
		if (first == last)
			continue;

		// don't delete empty paragraphs with keepempty set
		if (par.allowEmpty())
			continue;

		if (par.empty() || (par.size() == 1 && par.isLineSeparator(0))) {
			pars_.erase(pars_.iterator_at(pit));
			--pit;
			--last;
			continue;
		}
	}
}


} // namespace lyx
