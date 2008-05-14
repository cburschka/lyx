/**
 * \file rowpainter.cpp
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

#include "rowpainter.h"

#include "Bidi.h"
#include "Buffer.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Changes.h"
#include "Encoding.h"
#include "support/gettext.h"
#include "Language.h"
#include "Layout.h"
#include "LyXRC.h"
#include "Row.h"
#include "MetricsInfo.h"
#include "Paragraph.h"
#include "ParagraphMetrics.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "TextMetrics.h"
#include "VSpace.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "insets/InsetText.h"

#include "support/debug.h"
#include "support/textutils.h"

#include "support/lassert.h"
#include <boost/crc.hpp>

using namespace std;

namespace lyx {

using frontend::Painter;
using frontend::FontMetrics;

RowPainter::RowPainter(PainterInfo & pi,
	Text const & text, pit_type pit, Row const & row, Bidi & bidi, int x, int y)
	: pi_(pi), text_(text),
	  text_metrics_(pi_.base.bv->textMetrics(&text)),
	  pars_(text.paragraphs()),
	  row_(row), pit_(pit), par_(text.paragraphs()[pit]),
	  pm_(text_metrics_.parMetrics(pit)),
	  bidi_(bidi), erased_(pi_.erased_),
	  xo_(x), yo_(y), width_(text_metrics_.width())
{
	bidi_.computeTables(par_, pi_.base.bv->buffer(), row_);
	x_ = row_.x + xo_;

	//lyxerr << "RowPainter: x: " << x_ << " xo: " << xo_ << " yo: " << yo_ << endl;
	//row_.dump();

	LASSERT(pit >= 0, /**/);
	LASSERT(pit < int(text.paragraphs().size()), /**/);
}


FontInfo RowPainter::labelFont() const
{
	return text_.labelFont(pi_.base.bv->buffer(), par_);
}


int RowPainter::leftMargin() const
{
	return text_metrics_.leftMargin(text_metrics_.width(), pit_,
		row_.pos());
}

// If you want to debug inset metrics uncomment the following line:
//#define DEBUG_METRICS
// This draws green lines around each inset.


void RowPainter::paintInset(Inset const * inset, pos_type const pos)
{
	Font const font = text_metrics_.displayFont(pit_, pos);

	LASSERT(inset, /**/);
	// Backup full_repaint status because some insets (InsetTabular)
	// requires a full repaint
	bool pi_full_repaint = pi_.full_repaint;

	// FIXME: We should always use font, see documentation of
	// noFontChange() in Inset.h.
	pi_.base.font = inset->noFontChange() ?
		pi_.base.bv->buffer().params().getFont().fontInfo() :
		font.fontInfo();
	pi_.ltr_pos = (bidi_.level(pos) % 2 == 0);
	pi_.erased_ = erased_ || par_.isDeleted(pos);
	pi_.base.bv->coordCache().insets().add(inset, int(x_), yo_);
	// insets are painted completely. Recursive
	inset->drawSelection(pi_, int(x_), yo_);
	inset->draw(pi_, int(x_), yo_);

	Dimension const & dim = pm_.insetDimension(inset);

	paintForeignMark(x_, font.language(), dim.descent());

	x_ += dim.width();

	// Restore full_repaint status.
	pi_.full_repaint = pi_full_repaint;

#ifdef DEBUG_METRICS
	int const x1 = int(x_ - dim.width());
	Dimension dim2;
	LASSERT(max_witdh_ > 0, /**/);
	int right_margin = text_metrics_.rightMargin(pm_);
	int const w = max_witdh_ - leftMargin() - right_margin;
	MetricsInfo mi(pi_.base.bv, font.fontInfo(), w);
	inset->metrics(mi, dim2);
	if (dim.wid != dim2.wid)
		lyxerr << "Error: inset " << to_ascii(inset->getInsetName())
		       << " draw width " << dim.width()
		       << "> metrics width " << dim2.wid << "." << endl;
	if (dim->asc != dim2.asc)
		lyxerr << "Error: inset " << to_ascii(inset->getInsetName())
		       << " draw ascent " << dim.ascent()
		       << "> metrics ascent " << dim2.asc << "." << endl;
	if (dim2.descent() != dim.des)
		lyxerr << "Error: inset " << to_ascii(inset->getInsetName())
		       << " draw ascent " << dim.descent()
		       << "> metrics descent " << dim2.des << "." << endl;
	LASSERT(dim2.wid == dim.wid, /**/);
	LASSERT(dim2.asc == dim.asc, /**/);
	LASSERT(dim2.des == dim.des, /**/);
	int const x2 = x1 + dim.wid;
	int const y1 = yo_ + dim.des;
	int const y2 = yo_ - dim.asc;
	pi_.pain.line(x1, y1, x1, y2, Color_green);
	pi_.pain.line(x1, y1, x2, y1, Color_green);
	pi_.pain.line(x2, y1, x2, y2, Color_green);
	pi_.pain.line(x1, y2, x2, y2, Color_green);
#endif
}


void RowPainter::paintHebrewComposeChar(pos_type & vpos, FontInfo const & font)
{
	pos_type pos = bidi_.vis2log(vpos);

	docstring str;

	// first char
	char_type c = par_.getChar(pos);
	str += c;
	++vpos;

	int const width = theFontMetrics(font).width(c);
	int dx = 0;

	for (pos_type i = pos - 1; i >= 0; --i) {
		c = par_.getChar(i);
		if (!Encodings::isHebrewComposeChar(c)) {
			if (isPrintableNonspace(c)) {
				int const width2 = pm_.singleWidth(i,
					text_metrics_.displayFont(pit_, i));
				dx = (c == 0x05e8 || // resh
				      c == 0x05d3)   // dalet
					? width2 - width
					: (width2 - width) / 2;
			}
			break;
		}
	}

	// Draw nikud
	pi_.pain.text(int(x_) + dx, yo_, str, font);
}


void RowPainter::paintArabicComposeChar(pos_type & vpos, FontInfo const & font)
{
	pos_type pos = bidi_.vis2log(vpos);
	docstring str;

	// first char
	char_type c = par_.getChar(pos);
	c = par_.transformChar(c, pos);
	str += c;
	++vpos;

	int const width = theFontMetrics(font).width(c);
	int dx = 0;

	for (pos_type i = pos - 1; i >= 0; --i) {
		c = par_.getChar(i);
		if (!Encodings::isArabicComposeChar(c)) {
			if (isPrintableNonspace(c)) {
				int const width2 = pm_.singleWidth(i,
						text_metrics_.displayFont(pit_, i));
				dx = (width2 - width) / 2;
			}
			break;
		}
	}
	// Draw nikud
	pi_.pain.text(int(x_) + dx, yo_, str, font);
}


void RowPainter::paintChars(pos_type & vpos, FontInfo const & font,
			    bool hebrew, bool arabic)
{
	// This method takes up 70% of time when typing
	pos_type pos = bidi_.vis2log(vpos);
	pos_type const end = row_.endpos();
	FontSpan const font_span = par_.fontSpan(pos);
	Change::Type const prev_change = par_.lookupChange(pos).type;

	// first character
	vector<char_type> str;
	str.reserve(100);
	str.push_back(par_.getChar(pos));

	if (arabic) {
		char_type c = str[0];
		if (c == '(')
			c = ')';
		else if (c == ')')
			c = '(';
		str[0] = par_.transformChar(c, pos);
	}

	// selected text?
	bool const selection = pos >= row_.sel_beg && pos < row_.sel_end;

	// collect as much similar chars as we can
	for (++vpos ; vpos < end ; ++vpos) {
		pos = bidi_.vis2log(vpos);
		if (pos < font_span.first || pos > font_span.last)
			break;

		bool const new_selection = pos >= row_.sel_beg && pos < row_.sel_end;
		if (selection != new_selection)
			break;

		if (prev_change != par_.lookupChange(pos).type)
			break;

		char_type c = par_.getChar(pos);

		if (!isPrintableNonspace(c))
			break;

		/* Because we do our own bidi, at this point the strings are
		 * already in visual order. However, Qt also applies its own
		 * bidi algorithm to strings that it paints to the screen.
		 * Therefore, if we were to paint Hebrew/Arabic words as a
		 * single string, the letters in the words would get reversed
		 * again. In order to avoid that, we don't collect Hebrew/
		 * Arabic characters, but rather paint them one at a time.
		 * See also http://thread.gmane.org/gmane.editors.lyx.devel/79740
		 */
		if (hebrew)
			break;

		/* FIXME: these checks are irrelevant, since 'arabic' and
		 * 'hebrew' alone are already going to trigger a break.
		 * However, this should not be removed completely, because
		 * if an alternative solution is found which allows grouping
		 * of arabic and hebrew characters, then these breaks may have
		 * to be re-applied.

		if (arabic && Encodings::isArabicComposeChar(c))
			break;

		if (hebrew && Encodings::isHebrewComposeChar(c))
			break;
		*/

		if (arabic) {
			if (c == '(')
				c = ')';
			else if (c == ')')
				c = '(';
			c = par_.transformChar(c, pos);
			/* see comment in hebrew, explaining why we break */
			break;
		}

		str.push_back(c);
	}

	docstring s(&str[0], str.size());

	if (selection || prev_change != Change::UNCHANGED) {
		FontInfo copy = font;
		if (selection) {
			copy.setColor(Color_selectiontext);
		} else if (prev_change == Change::DELETED) {
			copy.setColor(Color_deletedtext);
		} else if (prev_change == Change::INSERTED) {
			copy.setColor(Color_addedtext);
		}
		x_ += pi_.pain.text(int(x_), yo_, s, copy);
	} else {
		x_ += pi_.pain.text(int(x_), yo_, s, font);
	}
}


void RowPainter::paintForeignMark(double orig_x, Language const * lang,
		int desc)
{
	if (!lyxrc.mark_foreign_language)
		return;
	if (lang == latex_language)
		return;
	if (lang == pi_.base.bv->buffer().params().language)
		return;

	int const y = yo_ + 1 + desc;
	pi_.pain.line(int(orig_x), y, int(x_), y, Color_language);
}


void RowPainter::paintFromPos(pos_type & vpos)
{
	pos_type const pos = bidi_.vis2log(vpos);
	Font const orig_font = text_metrics_.displayFont(pit_, pos);
	double const orig_x = x_;

	// usual characters, no insets
	char_type const c = par_.getChar(pos);

	// special case languages
	string const & lang = orig_font.language()->lang();
	bool const hebrew = lang == "hebrew";
	bool const arabic = lang == "arabic_arabtex" || lang == "arabic_arabi" || 
						lang == "farsi";

	// draw as many chars as we can
	if ((!hebrew && !arabic)
		|| (hebrew && !Encodings::isHebrewComposeChar(c))
		|| (arabic && !Encodings::isArabicComposeChar(c))) {
		paintChars(vpos, orig_font.fontInfo(), hebrew, arabic);
	} else if (hebrew) {
		paintHebrewComposeChar(vpos, orig_font.fontInfo());
	} else if (arabic) {
		paintArabicComposeChar(vpos, orig_font.fontInfo());
	}

	paintForeignMark(orig_x, orig_font.language());
}


void RowPainter::paintChangeBar()
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

	int const height = text_metrics_.isLastRow(pit_, row_)
		? row_.ascent()
		: row_.height();

	pi_.pain.fillRectangle(5, yo_ - row_.ascent(), 3, height, Color_changebar);
}


void RowPainter::paintAppendix()
{
	// only draw the appendix frame once (for the main text)
	if (!par_.params().appendix() || !text_.isMainText(pi_.base.bv->buffer()))
		return;

	int y = yo_ - row_.ascent();

	if (par_.params().startOfAppendix())
		y += 2 * defaultRowHeight();

	pi_.pain.line(1, y, 1, yo_ + row_.height(), Color_appendix);
	pi_.pain.line(width_ - 2, y, width_ - 2, yo_ + row_.height(), Color_appendix);
}


void RowPainter::paintDepthBar()
{
	depth_type const depth = par_.getDepth();

	if (depth <= 0)
		return;

	depth_type prev_depth = 0;
	if (!text_metrics_.isFirstRow(pit_, row_)) {
		pit_type pit2 = pit_;
		if (row_.pos() == 0)
			--pit2;
		prev_depth = pars_[pit2].getDepth();
	}

	depth_type next_depth = 0;
	if (!text_metrics_.isLastRow(pit_, row_)) {
		pit_type pit2 = pit_;
		if (row_.endpos() >= pars_[pit2].size())
			++pit2;
		next_depth = pars_[pit2].getDepth();
	}

	for (depth_type i = 1; i <= depth; ++i) {
		int const w = nestMargin() / 5;
		int x = int(xo_) + w * i;
		// only consider the changebar space if we're drawing outermost text
		if (text_.isMainText(pi_.base.bv->buffer()))
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


int RowPainter::paintAppendixStart(int y)
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

	return 3 * defaultRowHeight();
}


void RowPainter::paintFirst()
{
	ParagraphParameters const & parparams = par_.params();

	int y_top = 0;

	// start of appendix?
	if (parparams.startOfAppendix())
		y_top += paintAppendixStart(yo_ - row_.ascent() + 2 * defaultRowHeight());

	Buffer const & buffer = pi_.base.bv->buffer();
	Layout const & layout = par_.layout();

	if (buffer.params().paragraph_separation == BufferParams::ParagraphSkipSeparation) {
		if (pit_ != 0) {
			if (layout.latextype == LATEX_PARAGRAPH
				&& !par_.getDepth()) {
				y_top += buffer.params().getDefSkip().inPixels(*pi_.base.bv);
			} else {
				Layout const & playout = pars_[pit_ - 1].layout();
				if (playout.latextype == LATEX_PARAGRAPH
					&& !pars_[pit_ - 1].getDepth()) {
					// is it right to use defskip here, too? (AS)
					y_top += buffer.params().getDefSkip().inPixels(*pi_.base.bv);
				}
			}
		}
	}

	bool const is_rtl = text_.isRTL(buffer, par_);
	bool const is_seq = isFirstInSequence(pit_, text_.paragraphs());
	//lyxerr << "paintFirst: " << par_.id() << " is_seq: " << is_seq << endl;

	// should we print a label?
	if (layout.labeltype >= LABEL_STATIC
	    && (layout.labeltype != LABEL_STATIC
		      || layout.latextype != LATEX_ENVIRONMENT
		      || is_seq)) {

		FontInfo const font = labelFont();
		FontMetrics const & fm = theFontMetrics(font);

		docstring const str = par_.labelString();
		if (!str.empty()) {
			double x = x_;

			// this is special code for the chapter layout. This is
			// printed in an extra row and has a pagebreak at
			// the top.
			if (layout.counter == "chapter") {
				double spacing_val = 1.0;
				if (!parparams.spacing().isDefault()) {
					spacing_val = parparams.spacing().getValue();
				} else {
					spacing_val = buffer.params().spacing().getValue();
				}

				int const labeladdon = int(fm.maxHeight() * layout.spacing.getValue() * spacing_val);

				int const maxdesc = int(fm.maxDescent() * layout.spacing.getValue() * spacing_val)
					+ int(layout.parsep) * defaultRowHeight();

				if (is_rtl) {
					x = width_ - leftMargin() -
						fm.width(str);
				}

				pi_.pain.text(int(x), yo_ - maxdesc - labeladdon, str, font);
			} else {
				if (is_rtl) {
					x = width_ - leftMargin()
						+ fm.width(layout.labelsep);
				} else {
					x = x_ - fm.width(layout.labelsep)
						- fm.width(str);
				}

				pi_.pain.text(int(x), yo_, str, font);
			}
		}

	// the labels at the top of an environment.
	// More or less for bibliography
	} else if (is_seq &&
		(layout.labeltype == LABEL_TOP_ENVIRONMENT ||
		layout.labeltype == LABEL_BIBLIO ||
		layout.labeltype == LABEL_CENTERED_TOP_ENVIRONMENT)) {
		FontInfo const font = labelFont();
		docstring const str = par_.labelString();
		if (!str.empty()) {
			double spacing_val = 1.0;
			if (!parparams.spacing().isDefault())
				spacing_val = parparams.spacing().getValue();
			else
				spacing_val = buffer.params().spacing().getValue();

			FontMetrics const & fm = theFontMetrics(font);

			int const labeladdon = int(fm.maxHeight()
				* layout.spacing.getValue() * spacing_val);

			int maxdesc =
				int(fm.maxDescent() * layout.spacing.getValue() * spacing_val
				+ (layout.labelbottomsep * defaultRowHeight()));

			double x = x_;
			if (layout.labeltype == LABEL_CENTERED_TOP_ENVIRONMENT) {
				if (is_rtl)
					x = leftMargin();
				x += (width_ - text_metrics_.rightMargin(pm_) - leftMargin()) / 2;
				x -= fm.width(str) / 2;
			} else if (is_rtl) {
				x = width_ - leftMargin() -	fm.width(str);
			}
			pi_.pain.text(int(x), yo_ - maxdesc - labeladdon, str, font);
		}
	}
}


void RowPainter::paintLast()
{
	bool const is_rtl = text_.isRTL(pi_.base.bv->buffer(), par_);
	int const endlabel = getEndLabel(pit_, text_.paragraphs());

	// paint imaginary end-of-paragraph character

	if (par_.isInserted(par_.size()) || par_.isDeleted(par_.size())) {
		FontMetrics const & fm = theFontMetrics(pi_.base.bv->buffer().params().getFont());
		int const length = fm.maxAscent() / 2;
		ColorCode col = par_.isInserted(par_.size()) ? Color_addedtext : Color_deletedtext;

		pi_.pain.line(int(x_) + 1, yo_ + 2, int(x_) + 1, yo_ + 2 - length, col,
			   Painter::line_solid, Painter::line_thick);
		pi_.pain.line(int(x_) + 1 - length, yo_ + 2, int(x_) + 1, yo_ + 2, col,
			   Painter::line_solid, Painter::line_thick);
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
			: max_row_width - text_metrics_.rightMargin(pm_);

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
		double const x = is_rtl ?
			x_ - fm.width(str)
			: - text_metrics_.rightMargin(pm_) - row_.width();
		pi_.pain.text(int(x), yo_, str, font);
		break;
	}

	case END_LABEL_NO_LABEL:
		break;
	}
}


void RowPainter::paintOnlyInsets()
{
	pos_type const end = row_.endpos();
	for (pos_type pos = row_.pos(); pos != end; ++pos) {
		// If outer row has changed, nested insets are repaint completely.
		Inset const * inset = par_.getInset(pos);
		if (!inset)
			continue;
		if (x_ > pi_.base.bv->workWidth())
			continue;
		x_ = pi_.base.bv->coordCache().getInsets().x(inset);
		paintInset(inset, pos);
	}
}


void RowPainter::paintText()
{
	pos_type const end = row_.endpos();
	// Spaces at logical line breaks in bidi text must be skipped during 
	// painting. However, they may appear visually in the middle
	// of a row; they must be skipped, wherever they are...
	// * logically "abc_[HEBREW_\nHEBREW]"
	// * visually "abc_[_WERBEH\nWERBEH]"
	pos_type skipped_sep_vpos = -1;
	pos_type body_pos = par_.beginOfBody();
	if (body_pos > 0 &&
		(body_pos > end || !par_.isLineSeparator(body_pos - 1))) {
		body_pos = 0;
	}

	Layout const & layout = par_.layout();

	bool running_strikeout = false;
	bool is_struckout = false;
	int last_strikeout_x = 0;

	// check for possible inline completion
	DocIterator const & inlineCompletionPos = pi_.base.bv->inlineCompletionPos();
	pos_type inlineCompletionVPos = -1;
	if (inlineCompletionPos.inTexted()
	    && inlineCompletionPos.text() == &text_
	    && inlineCompletionPos.pit() == pit_
	    && inlineCompletionPos.pos() - 1 >= row_.pos()
	    && inlineCompletionPos.pos() - 1 < row_.endpos()) {
		// draw logically behind the previous character
		inlineCompletionVPos = bidi_.log2vis(inlineCompletionPos.pos() - 1);
	}

	// Use font span to speed things up, see below
	FontSpan font_span;
	Font font;

	// If the last logical character is a separator, don't paint it, unless
	// it's in the last row of a paragraph; see skipped_sep_vpos declaration
	if (end > 0 && end < par_.size() && par_.isSeparator(end - 1))
		skipped_sep_vpos = bidi_.log2vis(end - 1);

	for (pos_type vpos = row_.pos(); vpos < end; ) {
		if (x_ > pi_.base.bv->workWidth())
			break;

		// Skip the separator at the logical end of the row
		if (vpos == skipped_sep_vpos) {
			++vpos;
			continue;
		}

		pos_type const pos = bidi_.vis2log(vpos);

		if (pos >= par_.size()) {
			++vpos;
			continue;
		}

		// Use font span to speed things up, see above
		if (vpos < font_span.first || vpos > font_span.last) {
			font_span = par_.fontSpan(vpos);
			font = text_metrics_.displayFont(pit_, vpos);

			// split font span if inline completion is inside
			if (font_span.first <= inlineCompletionVPos
			    && font_span.last > inlineCompletionVPos)
				font_span.last = inlineCompletionVPos;
		}

		const int width_pos = pm_.singleWidth(pos, font);

		if (x_ + width_pos < 0) {
			x_ += width_pos;
			++vpos;
			continue;
		}

		is_struckout = par_.isDeleted(pos);

		if (is_struckout && !running_strikeout) {
			running_strikeout = true;
			last_strikeout_x = int(x_);
		}

		Inset const * inset = par_.getInset(pos);
		bool const highly_editable_inset = inset
			&& inset->editable() == Inset::HIGHLY_EDITABLE;

		// If we reach the end of a struck out range, paint it.
		// We also don't paint across things like tables
		if (running_strikeout && (highly_editable_inset || !is_struckout)) {
			// Calculate 1/3 height of the buffer's default font
			FontMetrics const & fm
				= theFontMetrics(pi_.base.bv->buffer().params().getFont());
			int const middle = yo_ - fm.maxAscent() / 3;
			pi_.pain.line(last_strikeout_x, middle, int(x_), middle,
				Color_deletedtext, Painter::line_solid, Painter::line_thin);
			running_strikeout = false;
		}

		if (body_pos > 0 && pos == body_pos - 1) {
			int const lwidth = theFontMetrics(labelFont())
				.width(layout.labelsep);

			x_ += row_.label_hfill + lwidth - width_pos;
		}
		
		// Is the inline completion in front of character?
		if (font.isRightToLeft() && vpos == inlineCompletionVPos)
			paintInlineCompletion(font);

		if (par_.isSeparator(pos)) {
			Font const orig_font = text_metrics_.displayFont(pit_, pos);
			double const orig_x = x_;
			x_ += width_pos;
			if (pos >= body_pos)
				x_ += row_.separator;
			paintForeignMark(orig_x, orig_font.language());
			++vpos;

		} else if (inset) {
			// If outer row has changed, nested insets are repaint completely.
			pi_.base.bv->coordCache().insets().add(inset, int(x_), yo_);
			paintInset(inset, pos);
			++vpos;

		} else {
			// paint as many characters as possible.
			paintFromPos(vpos);
		}

		// Is the inline completion after character?
		if (!font.isRightToLeft() && vpos - 1 == inlineCompletionVPos)
			paintInlineCompletion(font);
	}

	// if we reach the end of a struck out range, paint it
	if (running_strikeout) {
		// calculate 1/3 height of the buffer's default font
		FontMetrics const & fm
			= theFontMetrics(pi_.base.bv->buffer().params().getFont());
		int const middle = yo_ - fm.maxAscent() / 3;
		pi_.pain.line(last_strikeout_x, middle, int(x_), middle,
			Color_deletedtext, Painter::line_solid, Painter::line_thin);
		running_strikeout = false;
	}
}


void RowPainter::paintInlineCompletion(Font const & font)
{
	docstring completion = pi_.base.bv->inlineCompletion();
	FontInfo f = font.fontInfo();
	bool rtl = font.isRightToLeft();
	
	// draw the unique and the non-unique completion part
	// Note: this is not time-critical as it is
	// only done once per screen.
	size_t uniqueTo = pi_.base.bv->inlineCompletionUniqueChars();
	docstring s1 = completion.substr(0, uniqueTo);
	docstring s2 = completion.substr(uniqueTo);
	ColorCode c1 = Color_inlinecompletion;
	ColorCode c2 = Color_nonunique_inlinecompletion;
	
	// right to left?
	if (rtl) {
		swap(s1, s2);
		reverse(s1.begin(), s1.end());
		reverse(s2.begin(), s2.end());
		swap(c1, c2);
	}

	if (s1.size() > 0) {
		f.setColor(c1);
		pi_.pain.text(int(x_), yo_, s1, f);
		x_ += theFontMetrics(font).width(s1);
	}
	
	if (s2.size() > 0) {
		f.setColor(c2);
		pi_.pain.text(int(x_), yo_, s2, f);
		x_ += theFontMetrics(font).width(s2);
	}
}

} // namespace lyx
