/**
 * \file rowpainter.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author various
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "rowpainter.h"

#include "buffer.h"
#include "coordcache.h"
#include "cursor.h"
#include "debug.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "encoding.h"
#include "gettext.h"
#include "language.h"
#include "LColor.h"
#include "lyxrc.h"
#include "lyxrow.h"
#include "lyxrow_funcs.h"
#include "metricsinfo.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "vspace.h"

#include "frontends/font_metrics.h"
#include "frontends/nullpainter.h"
#include "frontends/Painter.h"

#include "insets/insettext.h"

#include "support/textutils.h"

using lyx::pos_type;
using lyx::par_type;

using std::endl;
using std::max;
using std::string;


namespace {

/**
 * A class used for painting an individual row of text.
 */
class RowPainter {
public:
	/// initialise and run painter
	RowPainter(BufferView const & bv, Painter & pain, LyXText const & text,
		par_type pit, RowList::iterator rit, int y);
private:
	// paint various parts
	void paintBackground();
	void paintSelection();
	void paintAppendix();
	void paintDepthBar();
	void paintChangeBar();
	void paintFirst();
	void paintLast();
	void paintForeignMark(double orig_x, LyXFont const & orig_font);
	void paintHebrewComposeChar(lyx::pos_type & vpos);
	void paintArabicComposeChar(lyx::pos_type & vpos);
	void paintChars(lyx::pos_type & vpos, bool hebrew, bool arabic);
	int paintAppendixStart(int y);
	void paintText();
	void paintFromPos(lyx::pos_type & vpos);
	void paintInset(lyx::pos_type const pos);

	/// return left margin
	int leftMargin() const;

	/// return the font at the given pos
	LyXFont const getFont(lyx::pos_type pos) const;

	/// return the label font for this row
	LyXFont const getLabelFont() const;

	/// return pixel width for the given pos
	int singleWidth(lyx::pos_type pos) const;
	int singleWidth(lyx::pos_type pos, char c) const;

	/// bufferview to paint on
	BufferView const & bv_;

	/// Painter to use
	Painter & pain_;

	/// LyXText for the row
	LyXText const & text_;
	ParagraphList & pars_;

	/// The row to paint
	RowList::iterator const rit_;
	Row & row_;

	/// Row's paragraph
	mutable par_type pit_;

	// Looks ugly - is
	double xo_;
	int yo_;
	double x_;
	int width_;
	double separator_;
	double hfill_;
	double label_hfill_;
};


RowPainter::RowPainter(BufferView const & bv, Painter & pain,
	LyXText const & text, par_type pit, RowList::iterator rit, int y)
	: bv_(bv), pain_(pain), text_(text), pars_(text.paragraphs()),
	  rit_(rit), row_(*rit), pit_(pit),
	  xo_(text_.xo_), yo_(y), width_(text_.width())
{
	//lyxerr << "RowPainter: x: " << x_ << " xo: " << xo << " yo: " << yo
	//	<< " pit->y: " << pit_->y
	//	<< " row: " << (pars_[pit_].size() ? pars_[pit_].getChar(row_.pos()) : 'X') << endl;

	RowMetrics m = text_.computeRowMetrics(pit, row_);
	x_ = m.x + xo_;

	separator_ = m.separator;
	hfill_ = m.hfill;
	label_hfill_ = m.label_hfill;

	// background has already been cleared.
	if (&text_ == bv_.text())
		paintBackground();

	// paint the selection background
	if (bv_.cursor().selection() && &text_ == bv_.cursor().text())
		paintSelection();

	// vertical lines for appendix
	paintAppendix();

	// environment depth brackets
	paintDepthBar();

	// changebar
	paintChangeBar();

	if (row_.pos() == 0)
		paintFirst();

	if (row_.endpos() >= pars_[pit_].size())
		paintLast();

	// paint text
	paintText();
}


/// "temporary"
LyXFont const RowPainter::getFont(pos_type pos) const
{
	return text_.getFont(pit_, pos);
}


int RowPainter::singleWidth(lyx::pos_type pos) const
{
	return text_.singleWidth(pit_, pos);
}


int RowPainter::singleWidth(lyx::pos_type pos, char c) const
{
	LyXFont const & font = text_.getFont(pit_, pos);
	return text_.singleWidth(pit_, pos, c, font);
}


LyXFont const RowPainter::getLabelFont() const
{
	return text_.getLabelFont(pit_);
}


int RowPainter::leftMargin() const
{
	return text_.leftMargin(pit_, row_.pos());
}


void RowPainter::paintInset(pos_type const pos)
{
	InsetBase const * inset = pars_[pit_].getInset(pos);
	BOOST_ASSERT(inset);
	PainterInfo pi(const_cast<BufferView *>(&bv_), pain_);
	pi.base.font = getFont(pos);
	theCoords.insets_.add(inset, int(x_), yo_ + row_.baseline());
	inset->drawSelection(pi, int(x_), yo_ + row_.baseline());
	inset->draw(pi, int(x_), yo_ + row_.baseline());
	x_ += inset->width();
}


void RowPainter::paintHebrewComposeChar(pos_type & vpos)
{
	pos_type pos = text_.bidi.vis2log(vpos);

	string str;

	// first char
	char c = pars_[pit_].getChar(pos);
	str += c;
	++vpos;

	LyXFont const & font = getFont(pos);
	int const width = font_metrics::width(c, font);
	int dx = 0;

	for (pos_type i = pos - 1; i >= 0; --i) {
		c = pars_[pit_].getChar(i);
		if (!Encodings::IsComposeChar_hebrew(c)) {
			if (IsPrintableNonspace(c)) {
				int const width2 = singleWidth(i, c);
				// dalet / resh
				dx = (c == 'ø' || c == 'ã')
					? width2 - width
					: (width2 - width) / 2;
			}
			break;
		}
	}

	// Draw nikud
	pain_.text(int(x_) + dx, yo_ + row_.baseline(), str, font);
}


void RowPainter::paintArabicComposeChar(pos_type & vpos)
{
	pos_type pos = text_.bidi.vis2log(vpos);
	string str;

	// first char
	char c = pars_[pit_].getChar(pos);
	c = pars_[pit_].transformChar(c, pos);
	str += c;
	++vpos;

	LyXFont const & font = getFont(pos);
	int const width = font_metrics::width(c, font);
	int dx = 0;

	for (pos_type i = pos - 1; i >= 0; --i) {
		c = pars_[pit_].getChar(i);
		if (!Encodings::IsComposeChar_arabic(c)) {
			if (IsPrintableNonspace(c)) {
				int const width2 = singleWidth(i, c);
				dx = (width2 - width) / 2;
			}
			break;
		}
	}
	// Draw nikud
	pain_.text(int(x_) + dx, yo_ + row_.baseline(), str, font);
}


void RowPainter::paintChars(pos_type & vpos, bool hebrew, bool arabic)
{
	pos_type pos = text_.bidi.vis2log(vpos);
	pos_type const end = row_.endpos();
	LyXFont orig_font = getFont(pos);

	// first character
	string str;
	str += pars_[pit_].getChar(pos);
	if (arabic) {
		unsigned char c = str[0];
		str[0] = pars_[pit_].transformChar(c, pos);
	}

	bool prev_struckout = isDeletedText(pars_[pit_], pos);
	bool prev_newtext = isInsertedText(pars_[pit_], pos);

	// collect as much similar chars as we can
	for (++vpos; vpos < end && (pos = text_.bidi.vis2log(vpos)) >= 0; ++vpos) {
		char c = pars_[pit_].getChar(pos);

		if (!IsPrintableNonspace(c))
			break;

		if (prev_struckout != isDeletedText(pars_[pit_], pos))
			break;

		if (prev_newtext != isInsertedText(pars_[pit_], pos))
			break;

		if (arabic && Encodings::IsComposeChar_arabic(c))
			break;

		if (hebrew && Encodings::IsComposeChar_hebrew(c))
			break;

		if (orig_font != getFont(pos))
			break;

		if (arabic)
			c = pars_[pit_].transformChar(c, pos);

		str += c;
	}

	if (prev_struckout)
		orig_font.setColor(LColor::strikeout);
	else if (prev_newtext)
		orig_font.setColor(LColor::newtext);

	// Draw text and set the new x position
	//lyxerr << "paint row: yo_ " << yo_ << " baseline: " << row_.baseline()
	//	<< "\n";
	pain_.text(int(x_), yo_ + row_.baseline(), str, orig_font);
	x_ += font_metrics::width(str, orig_font);
}


void RowPainter::paintForeignMark(double orig_x, LyXFont const & orig_font)
{
	if (!lyxrc.mark_foreign_language)
		return;
	if (orig_font.language() == latex_language)
		return;
	if (orig_font.language() == bv_.buffer()->params().language)
		return;

	int const y = yo_ + row_.baseline() + 1;
	pain_.line(int(orig_x), y, int(x_), y, LColor::language);
}


void RowPainter::paintFromPos(pos_type & vpos)
{
	pos_type const pos = text_.bidi.vis2log(vpos);

	LyXFont const & orig_font = getFont(pos);

	double const orig_x = x_;

	char const c = pars_[pit_].getChar(pos);

	if (c == Paragraph::META_INSET) {
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
}


void RowPainter::paintBackground()
{
	int const x = int(xo_);
	int const y = yo_ < 0 ? 0 : yo_;
	int const h = yo_ < 0 ? row_.height() + yo_ : row_.height();
	pain_.fillRectangle(x, y, width_, h, text_.backgroundColor());
}


void RowPainter::paintSelection()
{
	bool const is_rtl = text_.isRTL(pars_[pit_]);

	// the current selection
	LCursor const & cur = bv_.cursor();
	int const starty = text_.cursorY(cur.selBegin());
	int const endy = text_.cursorY(cur.selEnd());
	par_type startpit = cur.selBegin().par();
	par_type endpit = cur.selEnd().par();
	RowList::iterator startrow = pars_[startpit].getRow(cur.selBegin().pos());
	RowList::iterator endrow = pars_[endpit].getRow(cur.selEnd().pos());
	int const h = row_.height();

	int const row_y = text_.yo_ + pars_[pit_].y + row_.y_offset();

	bool const sel_starts_here = startpit == pit_ && startrow == rit_;
	bool const sel_ends_here   = endpit == pit_ && endrow == rit_;
	bool const sel_on_one_row  = sel_starts_here && sel_ends_here;

	if (text_.bidi.same_direction()) {
		if (sel_on_one_row) {
			int const startx = text_.cursorX(cur.selBegin());
			int const endx = text_.cursorX(cur.selEnd());
			int const x1 = is_rtl ? endx : startx;
			int const x2 = is_rtl ? startx : endx;
			pain_.fillRectangle(x1, yo_, x2 - x1, h, LColor::selection);
		} else if (sel_starts_here) {
			int const startx = text_.cursorX(cur.selBegin());
			int const x1 = is_rtl ? int(xo_) : startx;
			int const x2 = is_rtl ? startx : int(xo_) + width_;
			pain_.fillRectangle(x1, yo_, x2 - x1, h, LColor::selection);
		} else if (sel_ends_here) {
			int const endx = text_.cursorX(cur.selEnd());
			int const x1 = is_rtl ? endx : int(xo_);
			int const x2 = is_rtl ? int(xo_) + width_ : endx;
			pain_.fillRectangle(x1, yo_, x2 - x1, h, LColor::selection);
		} else if (row_y > starty && row_y < endy) {
			pain_.fillRectangle(int(xo_), yo_, width_, h, LColor::selection);
		}
		return;
	}

	if ((startpit != pit_ && startrow != rit_ && !is_rtl)
		|| (endpit != pit_ && endrow != rit_ && is_rtl))
		pain_.fillRectangle(int(xo_), yo_,
			int(x_), h, LColor::selection);

	pos_type const body_pos = pars_[pit_].beginOfBody();
	pos_type const end = row_.endpos();
	double tmpx = x_;

	for (pos_type vpos = row_.pos(); vpos < end; ++vpos)  {
		pos_type pos = text_.bidi.vis2log(vpos);
		double const old_tmpx = tmpx;
		if (body_pos > 0 && pos == body_pos - 1) {
			LyXLayout_ptr const & layout = pars_[pit_].layout();
			LyXFont const lfont = getLabelFont();

			tmpx += label_hfill_ + font_metrics::width(layout->labelsep, lfont);

			if (pars_[pit_].isLineSeparator(body_pos - 1))
				tmpx -= singleWidth(body_pos - 1);
		}

		tmpx += singleWidth(pos);

		if (hfillExpansion(pars_[pit_], row_, pos)) {
			if (pos >= body_pos)
				tmpx += hfill_;
			else
				tmpx += label_hfill_;
		} else {
			if (pars_[pit_].isSeparator(pos) && pos >= body_pos)
				tmpx += separator_;
		}

		if (((startpit != pit_ && startrow != rit_)
				|| cur.selBegin().pos() <= pos) &&
			((endpit != pit_ && endrow != rit_)
				|| pos < cur.selEnd().pos())) {
			// Here we do not use x_ as xo_ was added to x_.
			pain_.fillRectangle(int(old_tmpx), yo_,
				int(tmpx - old_tmpx + 1), h, LColor::selection);
		}
	}

	if ((startpit != pit_ && startrow != rit_ && is_rtl) ||
	    (endpit != pit_ && endrow != rit_ && !is_rtl)) {
		pain_.fillRectangle(int(xo_ + tmpx),
			yo_, int(width_ - tmpx), h, LColor::selection);
	}
}


void RowPainter::paintChangeBar()
{
	pos_type const start = row_.pos();
	pos_type const end = row_.endpos();

	if (start == end || !pars_[pit_].isChanged(start, end - 1))
		return;

	int const height = text_.isLastRow(pit_, row_)
		? row_.baseline()
		: row_.height() + boost::next(rit_)->top_of_text();

	pain_.fillRectangle(4, yo_, 5, height, LColor::changebar);
}


void RowPainter::paintAppendix()
{
	if (!pars_[pit_].params().appendix())
		return;

	int y = yo_;

	if (pars_[pit_].params().startOfAppendix())
		y += 2 * defaultRowHeight();

	pain_.line(1, y, 1, yo_ + row_.height(), LColor::appendix);
	pain_.line(width_ - 2, y, width_ - 2, yo_ + row_.height(), LColor::appendix);
}


void RowPainter::paintDepthBar()
{
	Paragraph::depth_type const depth = pars_[pit_].getDepth();

	if (depth <= 0)
		return;

	Paragraph::depth_type prev_depth = 0;
	if (!text_.isFirstRow(pit_, row_)) {
		par_type pit2 = pit_;
		if (row_.pos() == 0)
			--pit2;
		prev_depth = pars_[pit2].getDepth();
	}

	Paragraph::depth_type next_depth = 0;
	if (!text_.isLastRow(pit_, row_)) {
		par_type pit2 = pit_;
		if (row_.endpos() >= pars_[pit2].size())
			++pit2;
		next_depth = pars_[pit2].getDepth();
	}

	for (Paragraph::depth_type i = 1; i <= depth; ++i) {
		int const w = nestMargin() / 5;
		int x = xo_ + w * i;
		// only consider the changebar space if we're drawing outer left
		if (xo_ == 0)
			x += changebarMargin();

		int const h = yo_ + row_.height() - 1 - (i - next_depth - 1) * 3;

		pain_.line(x, yo_, x, h, LColor::depthbar);

		if (i > prev_depth)
			pain_.fillRectangle(x, yo_, w, 2, LColor::depthbar);
		if (i > next_depth)
			pain_.fillRectangle(x, h, w, 2, LColor::depthbar);
	}
}


int RowPainter::paintAppendixStart(int y)
{
	LyXFont pb_font;
	pb_font.setColor(LColor::appendix);
	pb_font.decSize();

	string const label = _("Appendix");
	int w = 0;
	int a = 0;
	int d = 0;
	font_metrics::rectText(label, pb_font, w, a, d);

	int const text_start = int(xo_ + (width_ - w) / 2);
	int const text_end = text_start + w;

	pain_.rectText(text_start, y + d, label, pb_font, LColor::none, LColor::none);

	pain_.line(int(xo_ + 1), y, text_start, y, LColor::appendix);
	pain_.line(text_end, y, int(xo_ + width_ - 2), y, LColor::appendix);

	return 3 * defaultRowHeight();
}


void RowPainter::paintFirst()
{
	ParagraphParameters const & parparams = pars_[pit_].params();

	int y_top = 0;

	// start of appendix?
	if (parparams.startOfAppendix())
		y_top += paintAppendixStart(yo_ + y_top + 2 * defaultRowHeight());

	Buffer const & buffer = *bv_.buffer();

	LyXLayout_ptr const & layout = pars_[pit_].layout();

	if (buffer.params().paragraph_separation == BufferParams::PARSEP_SKIP) {
		if (pit_ != 0) {
			if (layout->latextype == LATEX_PARAGRAPH
				&& !pars_[pit_].getDepth()) {
				y_top += buffer.params().getDefSkip().inPixels(bv_);
			} else {
				LyXLayout_ptr const & playout = pars_[pit_ - 1].layout();
				if (playout->latextype == LATEX_PARAGRAPH
					&& !pars_[pit_ - 1].getDepth()) {
					// is it right to use defskip here, too? (AS)
					y_top += buffer.params().getDefSkip().inPixels(bv_);
				}
			}
		}
	}

	bool const is_rtl = text_.isRTL(pars_[pit_]);
	bool const is_seq = isFirstInSequence(pit_, text_.paragraphs());
	//lyxerr << "paintFirst: " << pars_[pit_].id() << " is_seq: " << is_seq << std::endl;

	// should we print a label?
	if (layout->labeltype >= LABEL_STATIC
	    && (layout->labeltype != LABEL_STATIC
		      || layout->latextype != LATEX_ENVIRONMENT
		      || is_seq)) {

		LyXFont font = getLabelFont();
		if (!pars_[pit_].getLabelstring().empty()) {
			double x = x_;
			string const str = pars_[pit_].getLabelstring();

			// this is special code for the chapter layout. This is
			// printed in an extra row and has a pagebreak at
			// the top.
			if (layout->counter == "chapter") {
				if (buffer.params().secnumdepth >= 0) {
					float spacing_val = 1.0;
					if (!parparams.spacing().isDefault()) {
						spacing_val = parparams.spacing().getValue();
					} else {
						spacing_val = buffer.params().spacing().getValue();
					}

					int const maxdesc =
						int(font_metrics::maxDescent(font) * layout->spacing.getValue() * spacing_val)
						+ int(layout->parsep) * defaultRowHeight();

					if (is_rtl) {
						x = width_ - leftMargin() -
							font_metrics::width(str, font);
					}

					pain_.text(int(x),
						yo_ + row_.baseline() -
						row_.ascent_of_text() - maxdesc,
						str, font);
				}
			} else {
				if (is_rtl) {
					x = width_ - leftMargin()
						+ font_metrics::width(layout->labelsep, font);
				} else {
					x = x_ - font_metrics::width(layout->labelsep, font)
						- font_metrics::width(str, font);
				}

				pain_.text(int(x), yo_ + row_.baseline(), str, font);
			}
		}

	// the labels at the top of an environment.
	// More or less for bibliography
	} else if (is_seq &&
		(layout->labeltype == LABEL_TOP_ENVIRONMENT ||
		layout->labeltype == LABEL_BIBLIO ||
		layout->labeltype == LABEL_CENTERED_TOP_ENVIRONMENT)) {
		LyXFont font = getLabelFont();
		if (!pars_[pit_].getLabelstring().empty()) {
			string const str = pars_[pit_].getLabelstring();
			float spacing_val = 1.0;
			if (!parparams.spacing().isDefault()) {
				spacing_val = parparams.spacing().getValue();
			} else {
				spacing_val = buffer.params().spacing().getValue();
			}

			int maxdesc =
				int(font_metrics::maxDescent(font) * layout->spacing.getValue() * spacing_val
				+ (layout->labelbottomsep * defaultRowHeight()));

			double x = x_;
			if (layout->labeltype == LABEL_CENTERED_TOP_ENVIRONMENT) {
				x = ((is_rtl ? leftMargin() : x_)
					 + width_ - text_.rightMargin(pars_[pit_])) / 2;
				x -= font_metrics::width(str, font) / 2;
			} else if (is_rtl) {
				x = width_ - leftMargin() -
					font_metrics::width(str, font);
			}
			pain_.text(int(x),
			    yo_ + row_.baseline() - row_.ascent_of_text() - maxdesc,
				  str, font);
		}
	}
}


void RowPainter::paintLast()
{
	bool const is_rtl = text_.isRTL(pars_[pit_]);
	int const endlabel = getEndLabel(pit_, text_.paragraphs());

	// draw an endlabel
	switch (endlabel) {
	case END_LABEL_BOX:
	case END_LABEL_FILLED_BOX: {
		LyXFont const font = getLabelFont();
		int const size = int(0.75 * font_metrics::maxAscent(font));
		int const y = yo_ + row_.baseline() - size;
		int x = is_rtl ? nestMargin() + changebarMargin() : width_ - size;

		if (width_ - int(row_.width()) <= size)
			x += (size - width_ + row_.width() + 1) * (is_rtl ? -1 : 1);

		if (endlabel == END_LABEL_BOX)
			pain_.rectangle(x, y, size, size, LColor::eolmarker);
		else
			pain_.fillRectangle(x, y, size, size, LColor::eolmarker);
		break;
	}

	case END_LABEL_STATIC: {
		LyXFont font = getLabelFont();
		string const & str = pars_[pit_].layout()->endlabelstring();
		double const x = is_rtl ?
			x_ - font_metrics::width(str, font)
			: - text_.rightMargin(pars_[pit_]) - row_.width();
		pain_.text(int(x), yo_ + row_.baseline(), str, font);
		break;
	}

	case END_LABEL_NO_LABEL:
		break;
	}
}


void RowPainter::paintText()
{
	pos_type const end = row_.endpos();
	pos_type body_pos = pars_[pit_].beginOfBody();
	if (body_pos > 0 &&
		(body_pos > end || !pars_[pit_].isLineSeparator(body_pos - 1))) {
		body_pos = 0;
	}

	LyXLayout_ptr const & layout = pars_[pit_].layout();

	bool running_strikeout = false;
	bool is_struckout = false;
	int last_strikeout_x = 0;

	for (pos_type vpos = row_.pos(); vpos < end; ) {
		if (x_ > bv_.workWidth())
			break;

		pos_type pos = text_.bidi.vis2log(vpos);

		if (pos >= pars_[pit_].size()) {
			++vpos;
			continue;
		}

		if (x_ + singleWidth(pos) < 0) {
			x_ += singleWidth(pos);
			++vpos;
			continue;
		}

		is_struckout = isDeletedText(pars_[pit_], pos);

		if (is_struckout && !running_strikeout) {
			running_strikeout = true;
			last_strikeout_x = int(x_);
		}

		bool const highly_editable_inset = pars_[pit_].isInset(pos)
			&& isHighlyEditableInset(pars_[pit_].getInset(pos));

		// if we reach the end of a struck out range, paint it
		// we also don't paint across things like tables
		if (running_strikeout && (highly_editable_inset || !is_struckout)) {
			int const middle = yo_ + (row_.baseline() + row_.top_of_text()) / 2;
			pain_.line(last_strikeout_x, middle, int(x_), middle,
				LColor::strikeout, Painter::line_solid, Painter::line_thin);
			running_strikeout = false;
		}

		if (body_pos > 0 && pos == body_pos - 1) {
			int const lwidth = font_metrics::width(layout->labelsep,
				getLabelFont());

			x_ += label_hfill_ + lwidth - singleWidth(body_pos - 1);
		}

		if (pars_[pit_].isHfill(pos)) {
			x_ += 1;

			int const y0 = yo_ + row_.baseline();
			int const y1 = y0 - defaultRowHeight() / 2;

			pain_.line(int(x_), y1, int(x_), y0, LColor::added_space);

			if (hfillExpansion(pars_[pit_], row_, pos)) {
				int const y2 = (y0 + y1) / 2;

				if (pos >= body_pos) {
					pain_.line(int(x_), y2, int(x_ + hfill_), y2,
						  LColor::added_space,
						  Painter::line_onoffdash);
					x_ += hfill_;
				} else {
					pain_.line(int(x_), y2, int(x_ + label_hfill_), y2,
						  LColor::added_space,
						  Painter::line_onoffdash);
					x_ += label_hfill_;
				}
				pain_.line(int(x_), y1, int(x_), y0, LColor::added_space);
			}
			x_ += 2;
			++vpos;
		} else if (pars_[pit_].isSeparator(pos)) {
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
		int const middle = yo_ + (row_.baseline() + row_.top_of_text()) / 2;
		pain_.line(last_strikeout_x, middle, int(x_), middle,
			LColor::strikeout, Painter::line_solid, Painter::line_thin);
		running_strikeout = false;
	}
}


int paintPars(BufferView const & bv, Painter & pain,
	      LyXText const & text, par_type pit, par_type end)
{
	//lyxerr << "  paintRows: pit: " << &*pit << endl;
	ParagraphList & pars = text.paragraphs();

	int y = pars[pit].y + text.yo_ - bv.top_y();

	for (; pit != end; ++pit) {
		RowList::iterator row = pars[pit].rows.begin();
		RowList::iterator rend = pars[pit].rows.end();

		// We draw full paragraphs to get the (xo, yo) cache of all
		// contained insets right. This is needed for properly working
		// editXY. And maybe not even sufficient.
		// FIXME: Alfredo, please have a look at the coordinate business
		// again.
		for ( ; row != rend; ++row) {
			RowPainter(bv, pain, text, pit, row, y);
			y += row->height();
		}

	}
	return y;
}

} // namespace anon


void refreshPar(BufferView const & bv, LyXText const & text, par_type pit)
{
	static NullPainter nop;
	paintPars(bv, nop, text, pit, pit + 1);
}


int paintText(BufferView const & bv)
{
	par_type pit, end;
	getParsInRange(bv.text()->paragraphs(), bv.top_y(),
		       bv.top_y() + bv.workHeight(), pit, end);
	//lyxerr << "top_y: " << bv.top_y() << " y: " << pit->y << endl;
	return paintPars(bv, bv.painter(), *bv.text(), pit, end);
}


void paintTextInset(LyXText const & text, PainterInfo & pi)
{
	paintPars(*pi.base.bv, pi.pain, text, 0, text.paragraphs().size());
}
