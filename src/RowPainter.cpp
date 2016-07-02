/**
 * \file RowPainter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author various
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include <algorithm>

#include "RowPainter.h"

#include "Buffer.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Changes.h"
#include "Language.h"
#include "Layout.h"
#include "LyXRC.h"
#include "Row.h"
#include "MetricsInfo.h"
#include "Paragraph.h"
#include "ParagraphMetrics.h"
#include "ParagraphParameters.h"
#include "TextMetrics.h"
#include "VSpace.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "insets/InsetText.h"

#include "mathed/InsetMath.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/textutils.h"

#include "support/lassert.h"
#include <boost/crc.hpp>

using namespace std;

namespace lyx {

using frontend::Painter;
using frontend::FontMetrics;


RowPainter::RowPainter(PainterInfo & pi,
	Text const & text, Row const & row, int x, int y)
	: pi_(pi), text_(text),
	  text_metrics_(pi_.base.bv->textMetrics(&text)),
	  pars_(text.paragraphs()),
	  row_(row), par_(text.paragraphs()[row.pit()]),
	  pm_(text_metrics_.parMetrics(row.pit())), change_(pi_.change_),
	  xo_(x), yo_(y), width_(text_metrics_.width())
{
	x_ = row_.left_margin + xo_;

	//lyxerr << "RowPainter: x: " << x_ << " xo: " << xo_ << " yo: " << yo_ << endl;
	//row_.dump();

	LBUFERR(row.pit() >= 0);
	LBUFERR(row.pit() < int(text.paragraphs().size()));
}


FontInfo RowPainter::labelFont() const
{
	FontInfo f = text_.labelFont(par_);
	// selected text?
	if (row_.begin_margin_sel || pi_.selected)
		f.setPaintColor(Color_selectiontext);
	return f;
}


// If you want to debug inset metrics uncomment the following line:
//#define DEBUG_METRICS
// This draws green lines around each inset.


void RowPainter::paintInset(Row::Element const & e) const
{
	// Handle selection
	bool const pi_selected = pi_.selected;
	Cursor const & cur = pi_.base.bv->cursor();
	if (cur.selection() && cur.text() == &text_
		&& cur.normalAnchor().text() == &text_)
		pi_.selected = row_.sel_beg <= e.pos && row_.sel_end > e.pos;

	LASSERT(e.inset, return);
	// Backup full_repaint status because some insets (InsetTabular)
	// requires a full repaint
	bool const pi_full_repaint = pi_.full_repaint;
	bool const pi_do_spellcheck = pi_.do_spellcheck;
	Change const pi_change = pi_.change_;

	pi_.base.font = e.inset->inheritFont() ? e.font.fontInfo() :
		pi_.base.bv->buffer().params().getFont().fontInfo();
	pi_.ltr_pos = !e.font.isVisibleRightToLeft();
	pi_.change_ = change_.changed() ? change_ : e.change;
	pi_.do_spellcheck &= e.inset->allowSpellCheck();

	int const x1 = int(x_);
	pi_.base.bv->coordCache().insets().add(e.inset, x1, yo_);
	// insets are painted completely. Recursive
	// FIXME: it is wrong to completely paint the background
	// if we want to do single row painting.
	e.inset->drawBackground(pi_, x1, yo_);
	e.inset->drawSelection(pi_, x1, yo_);
	e.inset->draw(pi_, x1, yo_);

	// Restore full_repaint status.
	pi_.full_repaint = pi_full_repaint;
	pi_.change_ = pi_change;
	pi_.do_spellcheck = pi_do_spellcheck;
	pi_.selected = pi_selected;

#ifdef DEBUG_METRICS
	Dimension const & dim = pi_.base.bv->coordCache().insets().dim(e.inset);
	int const x2 = x1 + dim.wid;
	int const y1 = yo_ + dim.des;
	int const y2 = yo_ - dim.asc;
	pi_.pain.line(x1, y1, x1, y2, Color_green);
	pi_.pain.line(x1, y1, x2, y1, Color_green);
	pi_.pain.line(x2, y1, x2, y2, Color_green);
	pi_.pain.line(x1, y2, x2, y2, Color_green);
#endif
}


void RowPainter::paintForeignMark(Row::Element const & e) const
{
	Language const * lang = e.font.language();
	if (!lyxrc.mark_foreign_language)
		return;
	if (lang == latex_language)
		return;
	if (lang == pi_.base.bv->buffer().params().language)
		return;

	int const desc = e.inset ? e.dim.descent() : 0;
	int const y = yo_ + pi_.base.solidLineOffset()
		+ desc + pi_.base.solidLineThickness() / 2;
	pi_.pain.line(int(x_), y, int(x_ + e.full_width()), y, Color_language,
	              Painter::line_solid, pi_.base.solidLineThickness());
}


void RowPainter::paintMisspelledMark(Row::Element const & e) const
{
	// if changed the misspelled marker gets placed slightly lower than normal
	// to avoid drawing at the same vertical offset
	FontMetrics const & fm = theFontMetrics(e.font);
	int const thickness = max(fm.lineWidth(), 2);
	int const y = yo_ + pi_.base.solidLineOffset() + pi_.base.solidLineThickness()
		+ (e.change.changed() ? pi_.base.solidLineThickness() + 1 : 0)
		+ 1 + thickness / 2;

	//FIXME: this could be computed only once, it is probably not costly.
	// check for cursor position
	// don't draw misspelled marker for words at cursor position
	// we don't want to disturb the process of text editing
	DocIterator const nw = pi_.base.bv->cursor().newWord();
	pos_type cpos = -1;
	if (!nw.empty() && par_.id() == nw.paragraph().id()) {
		cpos = nw.pos();
		if (cpos > 0 && cpos == par_.size() && !par_.isWordSeparator(cpos-1))
			--cpos;
		else if (cpos > 0 && par_.isWordSeparator(cpos))
			--cpos;
	}

	pos_type pos = e.pos;
	while (pos < e.pos + pos_type(e.str.length())) {
		if (!par_.isMisspelled(pos)) {
			++pos;
			continue;
		}

		FontSpan const & range = par_.getSpellRange(pos);

		// Skip element which are being edited
		if (range.contains(cpos)) {
			// the range includes the last element
			pos = range.last + 1;
			continue;
		}

		FontMetrics const & fm = theFontMetrics(e.font);
		int x1 = fm.pos2x(e.str, range.first - e.pos,
		                  e.isRTL(), e.extra);
		int x2 = fm.pos2x(e.str, min(range.last - e.pos + 1,
									 pos_type(e.str.length())),
									 e.isRTL(), e.extra);
		if (x1 > x2)
			swap(x1, x2);

		pi_.pain.line(int(x_ + x1), y, int(x_ + x2), y, 
		              Color_error,
		              Painter::line_onoffdash, thickness);
		pos = range.last + 1;
	}
}


void RowPainter::paintStringAndSel(Row::Element const & e) const
{
	// at least part of text selected?
	bool const some_sel = (e.endpos >= row_.sel_beg && e.pos < row_.sel_end)
		|| pi_.selected;
	// all the text selected?
	bool const all_sel = (e.pos >= row_.sel_beg && e.endpos < row_.sel_end)
		|| pi_.selected;

	if (all_sel || e.change.changed()) {
		Font copy = e.font;
		Color const col = e.change.changed() ? e.change.color()
		                                     : Color_selectiontext;
		copy.fontInfo().setPaintColor(col);
		pi_.pain.text(int(x_), yo_, e.str, copy, e.extra, e.full_width());
	} else if (!some_sel) {
		pi_.pain.text(int(x_), yo_, e.str, e.font, e.extra, e.full_width());
	} else {
		pi_.pain.text(int(x_), yo_, e.str, e.font, Color_selectiontext,
		              max(row_.sel_beg, e.pos) - e.pos,
		              min(row_.sel_end, e.endpos) - e.pos,
		              e.extra, e.full_width());
	}
}


void RowPainter::paintChange(Row::Element const & e) const
{
	e.change.paintCue(pi_, x_, yo_, x_ + e.full_width(), e.font.fontInfo());
}


void RowPainter::paintChangeBar() const
{
	pos_type const start = row_.pos();
	pos_type end = row_.endpos();

	if (par_.size() == end) {
		// this is the last row of the paragraph;
		// thus, we must also consider the imaginary end-of-par character
		end++;
	}

	if (start == end || !par_.isChanged(start, end))
		return;

	int const height = text_metrics_.isLastRow(row_)
		? row_.ascent()
		: row_.height();

	pi_.pain.fillRectangle(5, yo_ - row_.ascent(), 3, height, Color_changebar);
}


void RowPainter::paintAppendix() const
{
	// only draw the appendix frame once (for the main text)
	if (!par_.params().appendix() || !text_.isMainText())
		return;

	int y = yo_ - row_.ascent();

	if (par_.params().startOfAppendix())
		y += 2 * defaultRowHeight();

	pi_.pain.line(1, y, 1, yo_ + row_.height(), Color_appendix);
	pi_.pain.line(width_ - 2, y, width_ - 2, yo_ + row_.height(), Color_appendix);
}


void RowPainter::paintDepthBar() const
{
	depth_type const depth = par_.getDepth();

	if (depth <= 0)
		return;

	depth_type prev_depth = 0;
	if (!text_metrics_.isFirstRow(row_)) {
		pit_type pit2 = row_.pit();
		if (row_.pos() == 0)
			--pit2;
		prev_depth = pars_[pit2].getDepth();
	}

	depth_type next_depth = 0;
	if (!text_metrics_.isLastRow(row_)) {
		pit_type pit2 = row_.pit();
		if (row_.endpos() >= pars_[pit2].size())
			++pit2;
		next_depth = pars_[pit2].getDepth();
	}

	for (depth_type i = 1; i <= depth; ++i) {
		int const w = nestMargin() / 5;
		int x = int(xo_) + w * i;
		// only consider the changebar space if we're drawing outermost text
		if (text_.isMainText())
			x += changebarMargin();

		int const starty = yo_ - row_.ascent();
		int const h =  row_.height() - 1 - (i - next_depth - 1) * 3;

		pi_.pain.line(x, starty, x, starty + h, Color_depthbar);

		if (i > prev_depth)
			pi_.pain.fillRectangle(x, starty, w, 2, Color_depthbar);
		if (i > next_depth)
			pi_.pain.fillRectangle(x, starty + h, w, 2, Color_depthbar);
	}
}


void RowPainter::paintAppendixStart(int y) const
{
	FontInfo pb_font = sane_font;
	pb_font.setColor(Color_appendix);
	pb_font.decSize();

	int w = 0;
	int a = 0;
	int d = 0;

	docstring const label = _("Appendix");
	theFontMetrics(pb_font).rectText(label, w, a, d);

	int const text_start = int(xo_ + (width_ - w) / 2);
	int const text_end = text_start + w;

	pi_.pain.rectText(text_start, y + d, label, pb_font, Color_none, Color_none);

	pi_.pain.line(int(xo_ + 1), y, text_start, y, Color_appendix);
	pi_.pain.line(text_end, y, int(xo_ + width_ - 2), y, Color_appendix);
}


void RowPainter::paintTooLargeMarks(bool const left, bool const right) const
{
	if (left)
		pi_.pain.line(pi_.base.dottedLineThickness(), yo_ - row_.ascent(),
					  pi_.base.dottedLineThickness(), yo_ + row_.descent(),
					  Color_scroll, Painter::line_onoffdash,
		              pi_.base.dottedLineThickness());
	if (right) {
		int const wwidth =
			pi_.base.bv->workWidth() - pi_.base.dottedLineThickness();
		pi_.pain.line(wwidth, yo_ - row_.ascent(),
					  wwidth, yo_ + row_.descent(),
					  Color_scroll, Painter::line_onoffdash,
		              pi_.base.dottedLineThickness());
	}
}


void RowPainter::paintFirst() const
{
	Layout const & layout = par_.layout();

	// start of appendix?
	if (par_.params().startOfAppendix())
	    paintAppendixStart(yo_ - row_.ascent() + 2 * defaultRowHeight());

	bool const is_first =
		text_.isFirstInSequence(row_.pit()) || !layout.isParagraphGroup();
	//lyxerr << "paintFirst: " << par_.id() << " is_seq: " << is_seq << endl;

	if (layout.labelIsInline()
	    && (layout.labeltype != LABEL_STATIC || is_first))
		paintLabel();
	else if (is_first && layout.labelIsAbove())
		paintTopLevelLabel();
}


void RowPainter::paintLabel() const
{
	docstring const & str = par_.labelString();
	if (str.empty())
		return;

	bool const is_rtl = text_.isRTL(par_);
	Layout const & layout = par_.layout();
	FontInfo const font = labelFont();
	FontMetrics const & fm = theFontMetrics(font);
	double x = x_;

	if (is_rtl)
		x = width_ - row_.right_margin + fm.width(layout.labelsep);
	else
		x = x_ - fm.width(layout.labelsep) - fm.width(str);

	pi_.pain.text(int(x), yo_, str, font);
}


void RowPainter::paintTopLevelLabel() const
{
	BufferParams const & bparams = pi_.base.bv->buffer().params();
	bool const is_rtl = text_.isRTL(par_);
	ParagraphParameters const & pparams = par_.params();
	Layout const & layout = par_.layout();
	FontInfo const font = labelFont();
	docstring const str = par_.labelString();
	if (str.empty())
		return;

	double spacing_val = 1.0;
	if (!pparams.spacing().isDefault())
		spacing_val = pparams.spacing().getValue();
	else
		spacing_val = bparams.spacing().getValue();

	FontMetrics const & fm = theFontMetrics(font);

	int const labeladdon = int(fm.maxHeight()
		* layout.spacing.getValue() * spacing_val);

	int maxdesc =
		int(fm.maxDescent() * layout.spacing.getValue() * spacing_val
		+ (layout.labelbottomsep * defaultRowHeight()));

	double x = x_;
	if (layout.labeltype == LABEL_CENTERED) {
		x = row_.left_margin + (width_ - row_.left_margin - row_.right_margin) / 2;
		x -= fm.width(str) / 2;
	} else if (is_rtl) {
		x = width_ - row_.right_margin - fm.width(str);
	}
	pi_.pain.text(int(x), yo_ - maxdesc - labeladdon, str, font);
}


/** Check if the current paragraph is the last paragraph in a
    proof environment */
static int getEndLabel(pit_type p, Text const & text)
{
	ParagraphList const & pars = text.paragraphs();
	pit_type pit = p;
	depth_type par_depth = pars[p].getDepth();
	while (pit != pit_type(pars.size())) {
		Layout const & layout = pars[pit].layout();
		int const endlabeltype = layout.endlabeltype;

		if (endlabeltype != END_LABEL_NO_LABEL) {
			if (p + 1 == pit_type(pars.size()))
				return endlabeltype;

			depth_type const next_depth =
				pars[p + 1].getDepth();
			if (par_depth > next_depth ||
			    (par_depth == next_depth && layout != pars[p + 1].layout()))
				return endlabeltype;
			break;
		}
		if (par_depth == 0)
			break;
		pit = text.outerHook(pit);
		if (pit != pit_type(pars.size()))
			par_depth = pars[pit].getDepth();
	}
	return END_LABEL_NO_LABEL;
}


void RowPainter::paintLast() const
{
	bool const is_rtl = text_.isRTL(par_);
	int const endlabel = getEndLabel(row_.pit(), text_);

	// paint imaginary end-of-paragraph character

	Change const & change = par_.lookupChange(par_.size());
	if (change.changed()) {
		FontMetrics const & fm =
			theFontMetrics(pi_.base.bv->buffer().params().getFont());
		int const length = fm.maxAscent() / 2;
		Color col = change.color();

		pi_.pain.line(int(x_) + 1, yo_ + 2, int(x_) + 1, yo_ + 2 - length, col,
			   Painter::line_solid, 3);

		if (change.deleted()) {
			pi_.pain.line(int(x_) + 1 - length, yo_ + 2, int(x_) + 1 + length,
				yo_ + 2, col, Painter::line_solid, 3);
		} else {
			pi_.pain.line(int(x_) + 1 - length, yo_ + 2, int(x_) + 1,
				yo_ + 2, col, Painter::line_solid, 3);
		}
	}

	// draw an endlabel

	switch (endlabel) {
	case END_LABEL_BOX:
	case END_LABEL_FILLED_BOX: {
		FontInfo const font = labelFont();
		FontMetrics const & fm = theFontMetrics(font);
		int const size = int(0.75 * fm.maxAscent());
		int const y = yo_ - size;
		int const max_row_width = width_ - size - Inset::TEXT_TO_INSET_OFFSET;
		int x = is_rtl ? nestMargin() + changebarMargin()
			: max_row_width - row_.right_margin;

		// If needed, move the box a bit to avoid overlapping with text.
		int const rem = max_row_width - row_.width();
		if (rem <= 0)
			x += is_rtl ? rem : - rem;

		if (endlabel == END_LABEL_BOX)
			pi_.pain.rectangle(x, y, size, size, Color_eolmarker);
		else
			pi_.pain.fillRectangle(x, y, size, size, Color_eolmarker);
		break;
	}

	case END_LABEL_STATIC: {
		FontInfo const font = labelFont();
		FontMetrics const & fm = theFontMetrics(font);
		docstring const & str = par_.layout().endlabelstring();
		double const x = is_rtl ? x_ - fm.width(str) : x_;
		pi_.pain.text(int(x), yo_, str, font);
		break;
	}

	case END_LABEL_NO_LABEL:
		break;
	}
}


void RowPainter::paintOnlyInsets()
{
	Row::const_iterator cit = row_.begin();
	Row::const_iterator const & end = row_.end();
	for ( ; cit != end ; ++cit) {
		Row::Element const & e = *cit;
		if (e.type == Row::INSET) {
			// If outer row has changed, nested insets are repainted completely.
			// FIXME: check what this really does. The test is weird.
			bool const nested_inset =
				(e.inset->asInsetMath() && !e.inset->asInsetMath()->asMacroTemplate())
				|| e.inset->asInsetText() || e.inset->asInsetTabular();
			if (nested_inset)
				paintInset(e);
		}
		x_ += e.full_width();
	}
}


void RowPainter::paintText()
{
	Row::const_iterator cit = row_.begin();
	Row::const_iterator const & end = row_.end();
	for ( ; cit != end ; ++cit) {
		Row::Element const & e = *cit;

		switch (e.type) {
		case Row::STRING:
		case Row::VIRTUAL:
			paintStringAndSel(e);

			// Paint the spelling marks if enabled.
			if (lyxrc.spellcheck_continuously && pi_.do_spellcheck && pi_.pain.isDrawingEnabled())
				paintMisspelledMark(e);
			break;

		case Row::INSET:
			paintInset(e);
			break;

		case Row::SPACE:
			pi_.pain.textDecoration(e.font.fontInfo(), int(x_), yo_, int(e.full_width()));
		}

		// The line that indicates word in a different language
		paintForeignMark(e);

		// change tracking (not for insets that handle it themselves)
		if (e.type != Row::INSET || ! e.inset->canPaintChange(*pi_.base.bv))
			paintChange(e);

		x_ += e.full_width();
	}
}


void RowPainter::paintSelection() const
{
	if (!row_.selection())
		return;
	Cursor const & curs = pi_.base.bv->cursor();
	DocIterator beg = curs.selectionBegin();
	beg.pit() = row_.pit();
	beg.pos() = row_.sel_beg;

	DocIterator end = curs.selectionEnd();
	end.pit() = row_.pit();
	end.pos() = row_.sel_end;

	bool const begin_boundary = beg.pos() >= row_.endpos();
	bool const end_boundary = row_.sel_end == row_.endpos();

	DocIterator cur = beg;
	cur.boundary(begin_boundary);
	int x1 = text_metrics_.cursorX(beg.top(), begin_boundary);
	int x2 = text_metrics_.cursorX(end.top(), end_boundary);
	int const y1 = yo_ - row_.ascent();
	int const y2 = y1 + row_.height();

	int const rm = text_.isMainText() ? pi_.base.bv->rightMargin() : 0;
	int const lm = text_.isMainText() ? pi_.base.bv->leftMargin() : 0;

	// draw the margins
	if (row_.begin_margin_sel) {
		if (text_.isRTL(beg.paragraph())) {
			pi_.pain.fillRectangle(int(xo_ + x1), y1,
				text_metrics_.width() - rm - x1, y2 - y1, Color_selection);
		} else {
			pi_.pain.fillRectangle(int(xo_ + lm), y1, x1 - lm, y2 - y1,
				Color_selection);
		}
	}

	if (row_.end_margin_sel) {
		if (text_.isRTL(beg.paragraph())) {
			pi_.pain.fillRectangle(int(xo_ + lm), y1, x2 - lm, y2 - y1,
				Color_selection);
		} else {
			pi_.pain.fillRectangle(int(xo_ + x2), y1, text_metrics_.width() - rm - x2,
				y2 - y1, Color_selection);
		}
	}

	// if we are on a boundary from the beginning, it's probably
	// a RTL boundary and we jump to the other side directly as this
	// segement is 0-size and confuses the logic below
	if (cur.boundary())
		cur.boundary(false);

	// go through row and draw from RTL boundary to RTL boundary
	while (cur < end) {
		bool draw_now = false;

		// simplified cursorForward code below which does not
		// descend into insets and which does not go into the
		// next line. Compare the logic with the original cursorForward

		// if left of boundary -> just jump to right side, but
		// for RTL boundaries don't, because: abc|DDEEFFghi -> abcDDEEF|Fghi
		if (cur.boundary()) {
			cur.boundary(false);
		}	else if (text_metrics_.isRTLBoundary(cur.pit(), cur.pos() + 1)) {
			// in front of RTL boundary -> Stay on this side of the boundary
			// because:  ab|cDDEEFFghi -> abc|DDEEFFghi
			++cur.pos();
			cur.boundary(true);
			draw_now = true;
		} else {
			// move right
			++cur.pos();

			// line end?
			if (cur.pos() == row_.endpos())
				cur.boundary(true);
		}

		if (x1 == -1) {
			// the previous segment was just drawn, now the next starts
			x1 = text_metrics_.cursorX(cur.top(), cur.boundary());
		}

		if (!(cur < end) || draw_now) {
			x2 = text_metrics_.cursorX(cur.top(), cur.boundary());
			pi_.pain.fillRectangle(int(xo_ + min(x1, x2)), y1, abs(x2 - x1),
				y2 - y1, Color_selection);

			// reset x1, so it is set again next round (which will be on the
			// right side of a boundary or at the selection end)
			x1 = -1;
		}
	}
}


} // namespace lyx
