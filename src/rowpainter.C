/**
 * \file rowpainter.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author various
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include <algorithm>

#include "frontends/Painter.h"
#include "frontends/screen.h"
#include "frontends/font_metrics.h"
#include "support/LAssert.h"
#include "paragraph.h"
#include "support/textutils.h"

#include "insets/insettext.h"
#include "ParagraphParameters.h"
#include "BufferView.h"
#include "buffer.h"
#include "gettext.h"
#include "language.h"
#include "encoding.h"
#include "lyxtext.h"
#include "lyxrow.h"
#include "rowpainter.h"
#include "lyxrc.h"


using std::max;
using lyx::pos_type;

extern int PAPER_MARGIN;
extern int CHANGEBAR_MARGIN;
extern int LEFT_MARGIN;

namespace {

// "temporary". We'll never get to use more
// references until we start adding hacks like
// these until other places catch up.
BufferView * perv(BufferView const & bv)
{
	return const_cast<BufferView *>(&bv);
}

} // namespace anon


RowPainter::RowPainter(BufferView const & bv, LyXText const & text, RowList::iterator rit)
	: bv_(bv), pain_(bv_.painter()), text_(text), row_(rit), par_(*rit->par())
{
}


/// "temporary"
LyXFont const RowPainter::getFont(pos_type pos) const
{
	Paragraph * perverted_par = const_cast<Paragraph *>(&par_);
	return text_.getFont(bv_.buffer(), perverted_par, pos);
}


int RowPainter::singleWidth(lyx::pos_type pos) const
{
	Paragraph * par(const_cast<Paragraph*>(&par_));
	return text_.singleWidth(par, pos);
}


int RowPainter::singleWidth(lyx::pos_type pos, char c) const
{
	Paragraph * par(const_cast<Paragraph*>(&par_));
	return text_.singleWidth(par, pos, c);
}


LyXFont const RowPainter::getLabelFont() const
{
	Paragraph * par(const_cast<Paragraph*>(&par_));
	return text_.getLabelFont(bv_.buffer(), par);
}


char const RowPainter::transformChar(char c, lyx::pos_type pos) const
{
	Paragraph * par(const_cast<Paragraph*>(&par_));
	return text_.transformChar(c, par, pos);
}


int RowPainter::leftMargin() const
{
	return text_.leftMargin(row_);
}


void RowPainter::paintInset(pos_type const pos)
{
	Inset * inset = const_cast<Inset*>(par_.getInset(pos));

	lyx::Assert(inset);

	LyXFont const & font = getFont(pos);

#warning inset->update FIXME
	inset->update(perv(bv_), false);

	inset->draw(perv(bv_), font, yo_ + row_->baseline(), x_);
}


void RowPainter::paintHebrewComposeChar(pos_type & vpos)
{
	pos_type pos = text_.vis2log(vpos);

	string str;

	// first char
	char c = par_.getChar(pos);
	str += c;
	++vpos;

	LyXFont const & font = getFont(pos);
	int const width = font_metrics::width(c, font);
	int dx = 0;

	for (pos_type i = pos - 1; i >= 0; --i) {
		c = par_.getChar(i);
		if (!Encodings::IsComposeChar_hebrew(c)) {
			if (IsPrintableNonspace(c)) {
				int const width2 =
					singleWidth(i, c);
				// dalet / resh
				dx = (c == 'ø' || c == 'ã')
					? width2 - width
					: (width2 - width) / 2;
			}
			break;
		}
	}

	// Draw nikud
	pain_.text(int(x_) + dx, yo_ + row_->baseline(), str, font);
}


void RowPainter::paintArabicComposeChar(pos_type & vpos)
{
	pos_type pos = text_.vis2log(vpos);
	string str;

	// first char
	char c = par_.getChar(pos);
	c = transformChar(c, pos);
	str +=c;
	++vpos;

	LyXFont const & font = getFont(pos);
	int const width = font_metrics::width(c, font);
	int dx = 0;

	for (pos_type i = pos-1; i >= 0; --i) {
		c = par_.getChar(i);
		if (!Encodings::IsComposeChar_arabic(c)) {
			if (IsPrintableNonspace(c)) {
				int const width2 =
					singleWidth(i, c);
				dx = (width2 - width) / 2;
			}
			break;
		}
	}
	// Draw nikud
	pain_.text(int(x_) + dx, yo_ + row_->baseline(), str, font);
}


void RowPainter::paintChars(pos_type & vpos, bool hebrew, bool arabic)
{
	pos_type pos = text_.vis2log(vpos);
	pos_type const last = row_->lastPrintablePos();
	LyXFont orig_font(getFont(pos));

	// first character
	string str;
	str += par_.getChar(pos);
	if (arabic) {
		unsigned char c = str[0];
		str[0] = transformChar(c, pos);
	}

	bool prev_struckout(isDeletedText(par_, pos));
	bool prev_newtext(isInsertedText(par_, pos));

	++vpos;

	// collect as much similar chars as we can
	while (vpos <= last && (pos = text_.vis2log(vpos)) >= 0) {
		char c = par_.getChar(pos);

		if (!IsPrintableNonspace(c))
			break;

		if (prev_struckout != isDeletedText(par_, pos))
			break;

		if (prev_newtext != isInsertedText(par_, pos))
			break;

		if (arabic && Encodings::IsComposeChar_arabic(c))
			break;
		if (hebrew && Encodings::IsComposeChar_hebrew(c))
			break;

		if (orig_font != getFont(pos))
			break;

		if (arabic)
			c = transformChar(c, pos);
		str += c;
		++vpos;
	}

	if (prev_struckout) {
		orig_font.setColor(LColor::strikeout);
	} else if (prev_newtext) {
		orig_font.setColor(LColor::newtext);
	}

	// Draw text and set the new x position
	pain_.text(int(x_), yo_ + row_->baseline(), str, orig_font);
	x_ += font_metrics::width(str, orig_font);
}


void RowPainter::paintForeignMark(float const orig_x, LyXFont const & orig_font)
{
	if (!lyxrc.mark_foreign_language)
		return;
	if (orig_font.language() == latex_language)
		return;
	if (orig_font.language() == bv_.buffer()->params.language)
		return;

	int const y = yo_ + row_->baseline() + 1;
	pain_.line(int(orig_x), y, int(x_), y, LColor::language);
}


void RowPainter::paintFromPos(pos_type & vpos)
{
	pos_type const pos = text_.vis2log(vpos);

	LyXFont const & orig_font = getFont(pos);

	float const orig_x = x_;

	char const c = par_.getChar(pos);

	if (IsInsetChar(c)) {
		paintInset(pos);
		++vpos;
		paintForeignMark(orig_x, orig_font);
		return;
	}

	// usual characters, no insets

	// special case languages
	bool const hebrew = (orig_font.language()->lang() == "hebrew");
	bool const arabic =
		orig_font.language()->lang() == "arabic" &&
		(lyxrc.font_norm_type == LyXRC::ISO_8859_6_8 ||
		lyxrc.font_norm_type == LyXRC::ISO_10646_1);

	// draw as many chars as we can
	if ((!hebrew && !arabic)
		|| (hebrew && !Encodings::IsComposeChar_hebrew(c))
		|| (arabic && !Encodings::IsComposeChar_arabic(c))) {
		paintChars(vpos, hebrew, arabic);
	} else if (hebrew) {
		paintHebrewComposeChar(vpos);
	} else if (arabic) {
		paintArabicComposeChar(vpos);
	}

	paintForeignMark(orig_x, orig_font);

	return;
}


void RowPainter::paintBackground()
{
	int const x = xo_;
	int const y = yo_ < 0 ? 0 : yo_;
	int const h = yo_ < 0 ? row_->height() + yo_ : row_->height();
	pain_.fillRectangle(x, y, width_, h, text_.backgroundColor());
}


void RowPainter::paintSelection()
{
	bool const is_rtl = par_.isRightToLeftPar(bv_.buffer()->params);

	// the current selection
	int const startx = text_.selection.start.x();
	int const endx = text_.selection.end.x();
	int const starty = text_.selection.start.y();
	int const endy = text_.selection.end.y();
	RowList::iterator startrow = text_.selection.start.row();
	RowList::iterator endrow = text_.selection.end.row();

	if (text_.bidi_same_direction) {
		int x;
		int y = yo_;
		int w;
		int h = row_->height();

		if (startrow == row_ && endrow == row_) {
			if (startx < endx) {
				x = xo_ + startx;
				w = endx - startx;
				pain_.fillRectangle(x, y, w, h, LColor::selection);
			} else {
				x = xo_ + endx;
				w = startx - endx;
				pain_.fillRectangle(x, y, w, h, LColor::selection);
			}
		} else if (startrow == row_) {
			int const x = (is_rtl) ? xo_ : (xo_ + startx);
			int const w = (is_rtl) ? startx : (width_ - startx);
			pain_.fillRectangle(x, y, w, h, LColor::selection);
		} else if (endrow == row_) {
			int const x = (is_rtl) ? (xo_ + endx) : xo_;
			int const w = (is_rtl) ? (width_ - endx) : endx;
			pain_.fillRectangle(x, y, w, h, LColor::selection);
		} else if (y_ > starty && y_ < endy) {
			pain_.fillRectangle(xo_, y, width_, h, LColor::selection);
		}
		return;
	} else if (startrow != row_ && endrow != row_) {
		if (y_ > starty && y_ < endy) {
			int w = width_;
			int h = row_->height();
			pain_.fillRectangle(xo_, yo_, w, h, LColor::selection);
		}
		return;
	}

	if ((startrow != row_ && !is_rtl) || (endrow != row_ && is_rtl))
		pain_.fillRectangle(xo_, yo_, int(x_), row_->height(), LColor::selection);

	pos_type const body_pos = par_.beginningOfBody();
	pos_type const last = row_->lastPrintablePos();
	float tmpx = x_;

	for (pos_type vpos = row_->pos(); vpos <= last; ++vpos)  {
		pos_type pos = text_.vis2log(vpos);
		float const old_tmpx = tmpx;
		if (body_pos > 0 && pos == body_pos - 1) {
			LyXLayout_ptr const & layout = par_.layout();
			LyXFont const lfont = getLabelFont();

			tmpx += label_hfill_ + font_metrics::width(layout->labelsep, lfont);

			if (par_.isLineSeparator(body_pos - 1))
				tmpx -= singleWidth(body_pos - 1);
		}

		if (row_->hfillExpansion(pos)) {
			tmpx += singleWidth(pos);
			if (pos >= body_pos)
				tmpx += hfill_;
			else
				tmpx += label_hfill_;
		}

		else if (par_.isSeparator(pos)) {
			tmpx += singleWidth(pos);
			if (pos >= body_pos)
				tmpx += separator_;
		} else {
			tmpx += singleWidth(pos);
		}

		if ((startrow != row_ || text_.selection.start.pos() <= pos) &&
			(endrow != row_ || pos < text_.selection.end.pos())) {
			// Here we do not use x_ as xo_ was added to x_.
			pain_.fillRectangle(int(old_tmpx), yo_,
				int(tmpx - old_tmpx + 1),
				row_->height(), LColor::selection);
		}
	}

	if ((startrow != row_ && is_rtl) || (endrow != row_ && !is_rtl)) {
		pain_.fillRectangle(xo_ + int(tmpx),
				      yo_, int(bv_.workWidth() - tmpx),
				      row_->height(), LColor::selection);
	}
}


void RowPainter::paintChangeBar()
{
	pos_type const start = row_->pos();
	pos_type const end = row_->lastPrintablePos();

	if (!par_.isChanged(start, end))
		return;

	int const height = (row_->next()
		? row_->height() + row_->next()->top_of_text()
		: row_->baseline());

	pain_.fillRectangle(4, yo_, 5, height, LColor::changebar);
}


void RowPainter::paintAppendix()
{
	if (!par_.params().appendix())
		return;

	// FIXME: can be just width_ ?
	int const ww = bv_.workWidth();

	int y = yo_;

	if (par_.params().startOfAppendix())
		y += 2 * defaultRowHeight();

	pain_.line(1, y, 1, yo_ + row_->height(), LColor::appendix);
	pain_.line(ww - 2, y, ww - 2, yo_ + row_->height(), LColor::appendix);
}


void RowPainter::paintDepthBar()
{
	Paragraph::depth_type const depth = par_.getDepth();

	if (depth <= 0)
		return;

	Paragraph::depth_type prev_depth = 0;
	if (row_->previous())
		prev_depth = row_->previous()->par()->getDepth();
	Paragraph::depth_type next_depth = 0;
	if (row_->next())
		next_depth = row_->next()->par()->getDepth();

	for (Paragraph::depth_type i = 1; i <= depth; ++i) {
		int x = (PAPER_MARGIN / 5) * i + xo_;
		// only consider the changebar space if we're drawing outer left
		if (!xo_)
			x += CHANGEBAR_MARGIN;
		int const h = yo_ + row_->height() - 1 - (i - next_depth - 1) * 3;

		pain_.line(x, yo_, x, h, LColor::depthbar);

		int const w = PAPER_MARGIN / 5;

		if (i > prev_depth) {
			pain_.fillRectangle(x, yo_, w, 2, LColor::depthbar);
		}
		if (i > next_depth) {
			pain_.fillRectangle(x, h, w, 2, LColor::depthbar);
		}
	}
}


int getLengthMarkerHeight(BufferView const & bv, VSpace const & vsp)
{
	if (vsp.kind() == VSpace::NONE)
		return 0;

	int const arrow_size = 4;
	int const space_size = int(vsp.inPixels(bv));

	LyXFont font;
	font.decSize();
	int const min_size = max(3 * arrow_size,
		font_metrics::maxAscent(font)
		+ font_metrics::maxDescent(font));

	if (vsp.length().len().value() < 0.0)
		return min_size;
	else
		return max(min_size, space_size);
}


int RowPainter::paintLengthMarker(string const & prefix, VSpace const & vsp, int start)
{
	if (vsp.kind() == VSpace::NONE)
		return 0;

	int const arrow_size = 4;
	int const size = getLengthMarkerHeight(bv_, vsp);
	int const end = start + size;

	// the label to display (if any)
	string str;
	// y-values for top arrow
	int ty1, ty2;
	// y-values for bottom arrow
	int by1, by2;

	str = prefix + " (" + vsp.asLyXCommand() + ")";

	if (vsp.kind() == VSpace::VFILL) {
		ty1 = ty2 = start;
		by1 = by2 = end;
	} else {
		// adding or removing space
		bool const added = vsp.kind() != VSpace::LENGTH ||
				   vsp.length().len().value() > 0.0;
		ty1 = added ? (start + arrow_size) : start;
		ty2 = added ? start : (start + arrow_size);
		by1 = added ? (end - arrow_size) : end;
		by2 = added ? end : (end - arrow_size);
	}

	int const leftx = xo_ + leftMargin();
	int const midx = leftx + arrow_size;
	int const rightx = midx + arrow_size;

	// first the string
	int w = 0;
	int a = 0;
	int d = 0;

	LyXFont font;
	font.setColor(LColor::added_space).decSize().decSize();
	font_metrics::rectText(str, font, w, a, d);

	pain_.rectText(leftx + 2 * arrow_size + 5,
			 start + ((end - start) / 2) + d,
			 str, font);

	// top arrow
	pain_.line(leftx, ty1, midx, ty2, LColor::added_space);
	pain_.line(midx, ty2, rightx, ty1, LColor::added_space);

	// bottom arrow
	pain_.line(leftx, by1, midx, by2, LColor::added_space);
	pain_.line(midx, by2, rightx, by1, LColor::added_space);

	// joining line
	pain_.line(midx, ty2, midx, by2, LColor::added_space);

	return size;
}


int RowPainter::paintPageBreak(string const & label, int y)
{
	LyXFont pb_font;
	pb_font.setColor(LColor::pagebreak).decSize();

	int w = 0;
	int a = 0;
	int d = 0;
	font_metrics::rectText(label, pb_font, w, a, d);

	int const text_start = xo_ + ((width_ - w) / 2);
	int const text_end = text_start + w;

	pain_.rectText(text_start, y + d, label, pb_font);

	pain_.line(xo_, y, text_start, y,
		LColor::pagebreak, Painter::line_onoffdash);
	pain_.line(text_end, y, xo_ + width_, y,
		LColor::pagebreak, Painter::line_onoffdash);

	return 3 * defaultRowHeight();
}


int RowPainter::paintAppendixStart(int y)
{
	LyXFont pb_font;
	pb_font.setColor(LColor::appendix).decSize();

	string const label = _("Appendix");
	int w = 0;
	int a = 0;
	int d = 0;
	font_metrics::rectText(label, pb_font, w, a, d);

	int const text_start = xo_ + ((width_ - w) / 2);
	int const text_end = text_start + w;

	pain_.rectText(text_start, y + d, label, pb_font);

	pain_.line(xo_ + 1, y, text_start, y, LColor::appendix);
	pain_.line(text_end, y, xo_ + width_ - 2, y, LColor::appendix);

	return 3 * defaultRowHeight();
}


void RowPainter::paintFirst()
{
	ParagraphParameters const & parparams = par_.params();

	int y_top = 0;

	// start of appendix?
	if (parparams.startOfAppendix()) {
		y_top += paintAppendixStart(yo_ + y_top + 2 * defaultRowHeight());
	}

	// the top margin
	if (!row_->previous() && !text_.isInInset())
		y_top += PAPER_MARGIN;

	// draw a top pagebreak
	if (parparams.pagebreakTop()) {
		y_top += paintPageBreak(_("Page Break (top)"),
			yo_ + y_top + 2 * defaultRowHeight());
	}

	// draw the additional space if needed:
	y_top += paintLengthMarker(_("Space above"), parparams.spaceTop(),
			yo_ + y_top);

	Buffer const * buffer = bv_.buffer();

	LyXLayout_ptr const & layout = par_.layout();

	if (buffer->params.paragraph_separation == BufferParams::PARSEP_SKIP) {
		if (par_.previous()) {
			if (layout->latextype == LATEX_PARAGRAPH
				&& !par_.getDepth()) {
				y_top += buffer->params.getDefSkip().inPixels(bv_);
			} else {
				LyXLayout_ptr const & playout =
					par_.previous()->layout();
				if (playout->latextype == LATEX_PARAGRAPH
					&& !par_.previous()->getDepth()) {
					// is it right to use defskip here, too? (AS)
					y_top += buffer->params.getDefSkip().inPixels(bv_);
				}
			}
		}
	}

	int const ww = bv_.workWidth();

	// draw a top line
	if (parparams.lineTop()) {
		LyXFont font(LyXFont::ALL_SANE);
		int const asc = font_metrics::ascent('x', getFont(0));

		y_top += asc;

		int const w = (text_.isInInset() ? text_.inset_owner->width(perv(bv_), font) : ww);
		int const xp = static_cast<int>(text_.isInInset() ? xo_ : 0);
		pain_.line(xp, yo_ + y_top, xp + w, yo_ + y_top,
			LColor::topline, Painter::line_solid,
			Painter::line_thick);

		y_top += asc;
	}

	bool const is_rtl = par_.isRightToLeftPar(bv_.buffer()->params);

	// should we print a label?
	if (layout->labeltype >= LABEL_STATIC
	    && (layout->labeltype != LABEL_STATIC
		|| layout->latextype != LATEX_ENVIRONMENT
		|| par_.isFirstInSequence())) {

		LyXFont font = getLabelFont();
		if (!par_.getLabelstring().empty()) {
			float x = x_;
			string const str = par_.getLabelstring();

			// this is special code for the chapter layout. This is
			// printed in an extra row and has a pagebreak at
			// the top.
			if (layout->labeltype == LABEL_COUNTER_CHAPTER) {
				if (buffer->params.secnumdepth >= 0) {
					float spacing_val = 1.0;
					if (!parparams.spacing().isDefault()) {
						spacing_val = parparams.spacing().getValue();
					} else {
						spacing_val = buffer->params.spacing.getValue();
					}

					int const maxdesc =
						int(font_metrics::maxDescent(font) * layout->spacing.getValue() * spacing_val)
						+ int(layout->parsep) * defaultRowHeight();

					if (is_rtl) {
						x = ww - leftMargin() -
							font_metrics::width(str, font);
					}

					pain_.text(int(x),
						yo_ + row_->baseline() -
						row_->ascent_of_text() - maxdesc,
						str, font);
				}
			} else {
				if (is_rtl) {
					x = ww - leftMargin()
						+ font_metrics::width(layout->labelsep, font);
				} else {
					x = x_ - font_metrics::width(layout->labelsep, font)
						- font_metrics::width(str, font);
				}

				pain_.text(int(x), yo_ + row_->baseline(), str, font);
			}
		}

	// the labels at the top of an environment.
	// More or less for bibliography
	} else if (par_.isFirstInSequence() &&
		(layout->labeltype == LABEL_TOP_ENVIRONMENT ||
		layout->labeltype == LABEL_BIBLIO ||
		layout->labeltype == LABEL_CENTERED_TOP_ENVIRONMENT)) {
		LyXFont font = getLabelFont();
		if (!par_.getLabelstring().empty()) {
			string const str = par_.getLabelstring();
			float spacing_val = 1.0;
			if (!parparams.spacing().isDefault()) {
				spacing_val = parparams.spacing().getValue();
			} else {
				spacing_val = buffer->params.spacing.getValue();
			}

			int maxdesc =
				int(font_metrics::maxDescent(font) * layout->spacing.getValue() * spacing_val
				+ (layout->labelbottomsep * defaultRowHeight()));

			float x = x_;
			if (layout->labeltype == LABEL_CENTERED_TOP_ENVIRONMENT) {
				x = ((is_rtl ? leftMargin() : x_)
					 + ww - text_.rightMargin(*bv_.buffer(), *row_)) / 2;
				x -= font_metrics::width(str, font) / 2;
			} else if (is_rtl) {
				x = ww - leftMargin() -
					font_metrics::width(str, font);
			}
			pain_.text(int(x), yo_ + row_->baseline()
				  - row_->ascent_of_text() - maxdesc,
				  str, font);
		}
	}
}


void RowPainter::paintLast()
{
	ParagraphParameters const & parparams = par_.params();
	int y_bottom = row_->height() - 1;

	// the bottom margin
	if (!row_->next() && !text_.isInInset())
		y_bottom -= PAPER_MARGIN;

	int const ww = bv_.workWidth();

	// draw a bottom pagebreak
	if (parparams.pagebreakBottom()) {
		y_bottom -= paintPageBreak(_("Page Break (bottom)"),
			yo_ + y_bottom - 2 * defaultRowHeight());
	}

	// draw the additional space if needed:
	int const height = getLengthMarkerHeight(bv_, parparams.spaceBottom());
	y_bottom -= paintLengthMarker(_("Space below"), parparams.spaceBottom(),
			     yo_ + y_bottom - height);

	// draw a bottom line
	if (parparams.lineBottom()) {
		LyXFont font(LyXFont::ALL_SANE);
		int const asc = font_metrics::ascent('x',
			getFont(max(pos_type(0), par_.size() - 1)));

		y_bottom -= asc;

		int const w = (text_.isInInset() ? text_.inset_owner->width(perv(bv_), font) : ww);
		int const xp = static_cast<int>(text_.isInInset() ? xo_ : 0);
		int const y = yo_ + y_bottom;
		pain_.line(xp, y, xp + w, y, LColor::topline, Painter::line_solid,
			  Painter::line_thick);

		y_bottom -= asc;
	}

	bool const is_rtl = par_.isRightToLeftPar(bv_.buffer()->params);
	int const endlabel = par_.getEndLabel();

	// draw an endlabel
	switch (endlabel) {
	case END_LABEL_BOX:
	case END_LABEL_FILLED_BOX:
	{
		LyXFont const font = getLabelFont();
		int const size = int(0.75 * font_metrics::maxAscent(font));
		int const y = (yo_ + row_->baseline()) - size;
		int x = is_rtl ? LEFT_MARGIN : ww - PAPER_MARGIN - size;

		if (row_->fill() <= size)
			x += (size - row_->fill() + 1) * (is_rtl ? -1 : 1);

		if (endlabel == END_LABEL_BOX) {
			pain_.rectangle(x, y, size, size, LColor::eolmarker);
		} else {
			pain_.fillRectangle(x, y, size, size, LColor::eolmarker);
		}
		break;
	}
	case END_LABEL_STATIC:
	{
#if 0
		LyXFont font(LyXFont::ALL_SANE);
		font = getLabelFont();
#else
		LyXFont font = getLabelFont();
#endif
		string const & str = par_.layout()->endlabelstring();
		int const x = is_rtl ?
			int(x_) - font_metrics::width(str, font)
			: ww - text_.rightMargin(*bv_.buffer(), *row_) - row_->fill();
		pain_.text(x, yo_ + row_->baseline(), str, font);
		break;
	}
	case END_LABEL_NO_LABEL:
		break;
	}
}


void RowPainter::paintText()
{
	pos_type const last = row_->lastPrintablePos();
	pos_type body_pos = par_.beginningOfBody();
	if (body_pos > 0 &&
		(body_pos - 1 > last ||
		!par_.isLineSeparator(body_pos - 1))) {
		body_pos = 0;
	}

	LyXLayout_ptr const & layout = par_.layout();

	bool running_strikeout = false;
	bool is_struckout = false;
	float last_strikeout_x = 0.0;

	pos_type vpos = row_->pos();
	while (vpos <= last) {
		if (x_ > bv_.workWidth())
			break;
		pos_type pos = text_.vis2log(vpos);

		if (pos >= par_.size()) {
			++vpos;
			continue;
		}

		if (x_ + singleWidth(pos) < 0) {
			x_ += singleWidth(pos);
			++vpos;
			continue;
		}

		is_struckout = isDeletedText(par_, pos);

		if (is_struckout && !running_strikeout) {
			running_strikeout = true;
			last_strikeout_x = x_;
		}

		bool const highly_editable_inset = par_.isInset(pos)
			&& isHighlyEditableInset(par_.getInset(pos));

		// if we reach the end of a struck out range, paint it
		// we also don't paint across things like tables
		if (running_strikeout && (highly_editable_inset || !is_struckout)) {
			int const middle = yo_ + row_->top_of_text()
				+ ((row_->baseline() - row_->top_of_text()) / 2);
			pain_.line(int(last_strikeout_x), middle, int(x_), middle,
				LColor::strikeout, Painter::line_solid, Painter::line_thin);
			running_strikeout = false;
		}

		if (body_pos > 0 && pos == body_pos - 1) {
			int const lwidth = font_metrics::width(layout->labelsep,
				getLabelFont());

			x_ += label_hfill_ + lwidth
				- singleWidth(body_pos - 1);
		}

		if (par_.isHfill(pos)) {
			x_ += 1;

			int const y0 = yo_ + row_->baseline();
			int const y1 = y0 - defaultRowHeight() / 2;

			pain_.line(int(x_), y1, int(x_), y0,
				     LColor::added_space);

			if (row_->hfillExpansion(pos)) {
				int const y2 = (y0 + y1) / 2;

				if (pos >= body_pos) {
					pain_.line(int(x_), y2,
						  int(x_ + hfill_), y2,
						  LColor::added_space,
						  Painter::line_onoffdash);
					x_ += hfill_;
				} else {
					pain_.line(int(x_), y2,
						  int(x_ + label_hfill_), y2,
						  LColor::added_space,
						  Painter::line_onoffdash);
					x_ += label_hfill_;
				}
				pain_.line(int(x_), y1,
					     int(x_), y0,
					     LColor::added_space);
			}
			x_ += 2;
			++vpos;
		} else if (par_.isSeparator(pos)) {
			x_ += singleWidth(pos);
			if (pos >= body_pos)
				x_ += separator_;
			++vpos;
		} else {
			paintFromPos(vpos);
		}
	}

	// if we reach the end of a struck out range, paint it
	if (running_strikeout) {
		int const middle = yo_ + row_->top_of_text()
			+ ((row_->baseline() - row_->top_of_text()) / 2);
		pain_.line(int(last_strikeout_x), middle, int(x_), middle,
			LColor::strikeout, Painter::line_solid, Painter::line_thin);
		running_strikeout = false;
	}
	return;
}


void RowPainter::paint(int y_offset, int x_offset, int y)
{
	xo_ = x_offset;
	yo_ = y_offset;
	y_ = y;
	width_ = text_.isInInset()
		? text_.inset_owner->textWidth(perv(bv_), true) : bv_.workWidth();

	// FIXME: must be a cleaner way here. Aren't these calculations
	// belonging to row metrics ?
	text_.prepareToPrint(row_, x_, separator_, hfill_, label_hfill_);

	// FIXME: what is this fixing ?
	if (text_.isInInset() && (x_ < 0))
		x_ = 0;
	x_ += xo_;

	// If we're *not* at the top-level of rows, then the
	// background has already been cleared.
	if (&text_ == bv_.text)
		paintBackground();

	// paint the selection background
	if (text_.selection.set()) {
		paintSelection();
	}

	// vertical lines for appendix
	paintAppendix();

	// environment depth brackets
	paintDepthBar();

	// changebar
	paintChangeBar();

	if (row_->isParStart()) {
		paintFirst();
	}

	if (row_->isParEnd()) {
		paintLast();
	}

	// paint text
	paintText();
}
