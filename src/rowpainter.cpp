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

#include "rowpainter.h"

#include "Bidi.h"
#include "Buffer.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "debug.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Encoding.h"
#include "gettext.h"
#include "Language.h"
#include "Color.h"
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

#include "support/textutils.h"

#include <boost/crc.hpp>

using std::endl;
using std::max;
using std::string;


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

	BOOST_ASSERT(pit >= 0);
	BOOST_ASSERT(pit < int(text.paragraphs().size()));
}


Font const RowPainter::getLabelFont() const
{
	return text_.getLabelFont(pi_.base.bv->buffer(), par_);
}


int RowPainter::leftMargin() const
{
	return text_metrics_.leftMargin(text_metrics_.width(), pit_,
		row_.pos());
}


void RowPainter::paintHfill(pos_type const pos, pos_type const body_pos)
{
	x_ += 1;

	int const y0 = yo_;
	int const y1 = y0 - defaultRowHeight() / 2;

	pi_.pain.line(int(x_), y1, int(x_), y0, Color::added_space);

	if (pm_.hfillExpansion(row_, pos)) {
		int const y2 = (y0 + y1) / 2;

		if (pos >= body_pos) {
			pi_.pain.line(int(x_), y2, int(x_ + row_.hfill), y2,
				Color::added_space,
				Painter::line_onoffdash);
			x_ += row_.hfill;
		} else {
			pi_.pain.line(int(x_), y2, int(x_ + row_.label_hfill), y2,
				Color::added_space,
				Painter::line_onoffdash);
			x_ += row_.label_hfill;
		}
		pi_.pain.line(int(x_), y1, int(x_), y0, Color::added_space);
	}
	x_ += 2;
}


// If you want to debug inset metrics uncomment the following line:
//#define DEBUG_METRICS
// This draws green lines around each inset.


void RowPainter::paintInset(Inset const * inset, pos_type const pos)
{
	Font font = text_.getFont(pi_.base.bv->buffer(), par_, pos);

	BOOST_ASSERT(inset);
	// FIXME: We should always use font, see documentation of
	// noFontChange() in Inset.h.
	pi_.base.font = inset->noFontChange() ?
		pi_.base.bv->buffer().params().getFont() :
		font;
	pi_.ltr_pos = (bidi_.level(pos) % 2 == 0);
	pi_.erased_ = erased_ || par_.isDeleted(pos);
	// insets are painted completely. Recursive
	inset->drawSelection(pi_, int(x_), yo_);
	inset->draw(pi_, int(x_), yo_);

	paintForeignMark(x_, font, inset->descent());

	x_ += inset->width();

#ifdef DEBUG_METRICS
	int const x1 = int(x_ - inset->width());
	Dimension dim;
	BOOST_ASSERT(max_witdh_ > 0);
	int right_margin = text_metrics_.rightMargin(pm_);
	int const w = max_witdh_ - leftMargin() - right_margin;
	MetricsInfo mi(pi_.base.bv, font, w);
	inset->metrics(mi, dim);
	if (inset->width() > dim.wid)
		lyxerr << "Error: inset " << to_ascii(inset->getInsetName())
		       << " draw width " << inset->width()
		       << "> metrics width " << dim.wid << "." << std::endl;
	if (inset->ascent() > dim.asc)
		lyxerr << "Error: inset " << to_ascii(inset->getInsetName())
		       << " draw ascent " << inset->ascent()
		       << "> metrics ascent " << dim.asc << "." << std::endl;
	if (inset->descent() > dim.des)
		lyxerr << "Error: inset " << to_ascii(inset->getInsetName())
		       << " draw ascent " << inset->descent()
		       << "> metrics descent " << dim.des << "." << std::endl;
	BOOST_ASSERT(inset->width() <= dim.wid);
	BOOST_ASSERT(inset->ascent() <= dim.asc);
	BOOST_ASSERT(inset->descent() <= dim.des);
	int const x2 = x1 + dim.wid;
	int const y1 = yo_ + dim.des;
	int const y2 = yo_ - dim.asc;
	pi_.pain.line(x1, y1, x1, y2, Color::green);
	pi_.pain.line(x1, y1, x2, y1, Color::green);
	pi_.pain.line(x2, y1, x2, y2, Color::green);
	pi_.pain.line(x1, y2, x2, y2, Color::green);
#endif
}


void RowPainter::paintHebrewComposeChar(pos_type & vpos, Font const & font)
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
		if (!Encodings::isComposeChar_hebrew(c)) {
			if (isPrintableNonspace(c)) {
				int const width2 = pm_.singleWidth(i,
					text_.getFont(pi_.base.bv->buffer(), par_, i));
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


void RowPainter::paintArabicComposeChar(pos_type & vpos, Font const & font)
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
		if (!Encodings::isComposeChar_arabic(c)) {
			if (isPrintableNonspace(c)) {
				int const width2 = pm_.singleWidth(i,
						text_.getFont(pi_.base.bv->buffer(), par_, i));
				dx = (width2 - width) / 2;
			}
			break;
		}
	}
	// Draw nikud
	pi_.pain.text(int(x_) + dx, yo_, str, font);
}


void RowPainter::paintChars(pos_type & vpos, Font const & font,
			    bool hebrew, bool arabic)
{
	// This method takes up 70% of time when typing
	pos_type pos = bidi_.vis2log(vpos);
	pos_type const end = row_.endpos();
	FontSpan const font_span = par_.fontSpan(pos);
	Change::Type const prev_change = par_.lookupChange(pos).type;

	// first character
	std::vector<char_type> str;
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

	// collect as much similar chars as we can
	for (++vpos ; vpos < end ; ++vpos) {
		pos = bidi_.vis2log(vpos);
		if (pos < font_span.first || pos > font_span.last)
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

		if (arabic && Encodings::isComposeChar_arabic(c))
			break;

		if (hebrew && Encodings::isComposeChar_hebrew(c))
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

	if (prev_change != Change::UNCHANGED) {
		Font copy(font);
		if (prev_change == Change::DELETED) {
			copy.setColor(Color::deletedtext);
		} else if (prev_change == Change::INSERTED) {
			copy.setColor(Color::addedtext);
		}
		x_ += pi_.pain.text(int(x_), yo_, s, copy);
	} else {
		x_ += pi_.pain.text(int(x_), yo_, s, font);
	}
}


void RowPainter::paintForeignMark(double orig_x, Font const & font, int desc)
{
	if (!lyxrc.mark_foreign_language)
		return;
	if (font.language() == latex_language)
		return;
	if (font.language() == pi_.base.bv->buffer().params().language)
		return;

	int const y = yo_ + 1 + desc;
	pi_.pain.line(int(orig_x), y, int(x_), y, Color::language);
}


void RowPainter::paintFromPos(pos_type & vpos)
{
	pos_type const pos = bidi_.vis2log(vpos);
	Font orig_font = text_.getFont(pi_.base.bv->buffer(), par_, pos);
	double const orig_x = x_;

	// usual characters, no insets
	char_type const c = par_.getChar(pos);

	// special case languages
	std::string const & lang = orig_font.language()->lang();
	bool const hebrew = lang == "hebrew";
	bool const arabic = lang == "arabic_arabtex" || lang == "arabic_arabi" || 
						lang == "farsi";

	// draw as many chars as we can
	if ((!hebrew && !arabic)
		|| (hebrew && !Encodings::isComposeChar_hebrew(c))
		|| (arabic && !Encodings::isComposeChar_arabic(c))) {
		paintChars(vpos, orig_font, hebrew, arabic);
	} else if (hebrew) {
		paintHebrewComposeChar(vpos, orig_font);
	} else if (arabic) {
		paintArabicComposeChar(vpos, orig_font);
	}

	paintForeignMark(orig_x, orig_font);
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

	int const height = text_.isLastRow(pit_, row_)
		? row_.ascent()
		: row_.height();

	pi_.pain.fillRectangle(5, yo_ - row_.ascent(), 3, height, Color::changebar);
}


void RowPainter::paintAppendix()
{
	// only draw the appendix frame once (for the main text)
	if (!par_.params().appendix() || !text_.isMainText(pi_.base.bv->buffer()))
		return;

	int y = yo_ - row_.ascent();

	if (par_.params().startOfAppendix())
		y += 2 * defaultRowHeight();

	pi_.pain.line(1, y, 1, yo_ + row_.height(), Color::appendix);
	pi_.pain.line(width_ - 2, y, width_ - 2, yo_ + row_.height(), Color::appendix);
}


void RowPainter::paintDepthBar()
{
	depth_type const depth = par_.getDepth();

	if (depth <= 0)
		return;

	depth_type prev_depth = 0;
	if (!text_.isFirstRow(pit_, row_)) {
		pit_type pit2 = pit_;
		if (row_.pos() == 0)
			--pit2;
		prev_depth = pars_[pit2].getDepth();
	}

	depth_type next_depth = 0;
	if (!text_.isLastRow(pit_, row_)) {
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

		pi_.pain.line(x, starty, x, starty + h, Color::depthbar);

		if (i > prev_depth)
			pi_.pain.fillRectangle(x, starty, w, 2, Color::depthbar);
		if (i > next_depth)
			pi_.pain.fillRectangle(x, starty + h, w, 2, Color::depthbar);
	}
}


int RowPainter::paintAppendixStart(int y)
{
	Font pb_font;
	pb_font.setColor(Color::appendix);
	pb_font.decSize();

	int w = 0;
	int a = 0;
	int d = 0;

	docstring const label = _("Appendix");
	theFontMetrics(pb_font).rectText(label, w, a, d);

	int const text_start = int(xo_ + (width_ - w) / 2);
	int const text_end = text_start + w;

	pi_.pain.rectText(text_start, y + d, label, pb_font, Color::none, Color::none);

	pi_.pain.line(int(xo_ + 1), y, text_start, y, Color::appendix);
	pi_.pain.line(text_end, y, int(xo_ + width_ - 2), y, Color::appendix);

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

	LayoutPtr const & layout = par_.layout();

	if (buffer.params().paragraph_separation == BufferParams::PARSEP_SKIP) {
		if (pit_ != 0) {
			if (layout->latextype == LATEX_PARAGRAPH
				&& !par_.getDepth()) {
				y_top += buffer.params().getDefSkip().inPixels(*pi_.base.bv);
			} else {
				LayoutPtr const & playout = pars_[pit_ - 1].layout();
				if (playout->latextype == LATEX_PARAGRAPH
					&& !pars_[pit_ - 1].getDepth()) {
					// is it right to use defskip here, too? (AS)
					y_top += buffer.params().getDefSkip().inPixels(*pi_.base.bv);
				}
			}
		}
	}

	bool const is_rtl = text_.isRTL(buffer, par_);
	bool const is_seq = isFirstInSequence(pit_, text_.paragraphs());
	//lyxerr << "paintFirst: " << par_.id() << " is_seq: " << is_seq << std::endl;

	// should we print a label?
	if (layout->labeltype >= LABEL_STATIC
	    && (layout->labeltype != LABEL_STATIC
		      || layout->latextype != LATEX_ENVIRONMENT
		      || is_seq)) {

		Font const font = getLabelFont();
		FontMetrics const & fm = theFontMetrics(font);

		docstring const str = par_.getLabelstring();
		if (!str.empty()) {
			double x = x_;

			// this is special code for the chapter layout. This is
			// printed in an extra row and has a pagebreak at
			// the top.
			if (layout->counter == "chapter") {
				double spacing_val = 1.0;
				if (!parparams.spacing().isDefault()) {
					spacing_val = parparams.spacing().getValue();
				} else {
					spacing_val = buffer.params().spacing().getValue();
				}

				int const labeladdon = int(fm.maxHeight() * layout->spacing.getValue() * spacing_val);

				int const maxdesc = int(fm.maxDescent() * layout->spacing.getValue() * spacing_val)
					+ int(layout->parsep) * defaultRowHeight();

				if (is_rtl) {
					x = width_ - leftMargin() -
						fm.width(str);
				}

				pi_.pain.text(int(x), yo_ - maxdesc - labeladdon, str, font);
			} else {
				if (is_rtl) {
					x = width_ - leftMargin()
						+ fm.width(layout->labelsep);
				} else {
					x = x_ - fm.width(layout->labelsep)
						- fm.width(str);
				}

				pi_.pain.text(int(x), yo_, str, font);
			}
		}

	// the labels at the top of an environment.
	// More or less for bibliography
	} else if (is_seq &&
		(layout->labeltype == LABEL_TOP_ENVIRONMENT ||
		layout->labeltype == LABEL_BIBLIO ||
		layout->labeltype == LABEL_CENTERED_TOP_ENVIRONMENT)) {
		Font font = getLabelFont();
		if (!par_.getLabelstring().empty()) {
			docstring const str = par_.getLabelstring();
			double spacing_val = 1.0;
			if (!parparams.spacing().isDefault())
				spacing_val = parparams.spacing().getValue();
			else
				spacing_val = buffer.params().spacing().getValue();

			FontMetrics const & fm = theFontMetrics(font);

			int const labeladdon = int(fm.maxHeight()
				* layout->spacing.getValue() * spacing_val);

			int maxdesc =
				int(fm.maxDescent() * layout->spacing.getValue() * spacing_val
				+ (layout->labelbottomsep * defaultRowHeight()));

			double x = x_;
			if (layout->labeltype == LABEL_CENTERED_TOP_ENVIRONMENT) {
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
		Color::color col = par_.isInserted(par_.size()) ? Color::addedtext : Color::deletedtext;

		pi_.pain.line(int(x_) + 1, yo_ + 2, int(x_) + 1, yo_ + 2 - length, col,
			   Painter::line_solid, Painter::line_thick);
		pi_.pain.line(int(x_) + 1 - length, yo_ + 2, int(x_) + 1, yo_ + 2, col,
			   Painter::line_solid, Painter::line_thick);
	}

	// draw an endlabel

	switch (endlabel) {
	case END_LABEL_BOX:
	case END_LABEL_FILLED_BOX: {
		Font const font = getLabelFont();
		FontMetrics const & fm = theFontMetrics(font);
		int const size = int(0.75 * fm.maxAscent());
		int const y = yo_ - size;
		int x = is_rtl ? nestMargin() + changebarMargin() : width_ - size;

		if (width_ - int(row_.width()) <= size)
			x += (size - width_ + row_.width() + 1) * (is_rtl ? -1 : 1);

		if (endlabel == END_LABEL_BOX)
			pi_.pain.rectangle(x, y, size, size, Color::eolmarker);
		else
			pi_.pain.fillRectangle(x, y, size, size, Color::eolmarker);
		break;
	}

	case END_LABEL_STATIC: {
		Font font = getLabelFont();
		FontMetrics const & fm = theFontMetrics(font);
		docstring const & str = par_.layout()->endlabelstring();
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
		if (!par_.isInset(pos))
			continue;

		// If outer row has changed, nested insets are repaint completely.
		Inset const * inset = par_.getInset(pos);

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

	LayoutPtr const & layout = par_.layout();

	bool running_strikeout = false;
	bool is_struckout = false;
	int last_strikeout_x = 0;

	// Use font span to speed things up, see below
	FontSpan font_span;
	Font font;
	Buffer const & buffer = pi_.base.bv->buffer();

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
			font = text_.getFont(buffer, par_, vpos);
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

		bool const highly_editable_inset = par_.isInset(pos)
			&& isHighlyEditableInset(par_.getInset(pos));

		// If we reach the end of a struck out range, paint it.
		// We also don't paint across things like tables
		if (running_strikeout && (highly_editable_inset || !is_struckout)) {
			// Calculate 1/3 height of the buffer's default font
			FontMetrics const & fm
				= theFontMetrics(pi_.base.bv->buffer().params().getFont());
			int const middle = yo_ - fm.maxAscent() / 3;
			pi_.pain.line(last_strikeout_x, middle, int(x_), middle,
				Color::deletedtext, Painter::line_solid, Painter::line_thin);
			running_strikeout = false;
		}

		if (body_pos > 0 && pos == body_pos - 1) {
			int const lwidth = theFontMetrics(getLabelFont())
				.width(layout->labelsep);

			x_ += row_.label_hfill + lwidth - width_pos;
		}

		if (par_.isHfill(pos)) {
			paintHfill(pos, body_pos);
			++vpos;

		} else if (par_.isSeparator(pos)) {
			Font orig_font = text_.getFont(buffer, par_, pos);
			double const orig_x = x_;
			x_ += width_pos;
			if (pos >= body_pos)
				x_ += row_.separator;
			paintForeignMark(orig_x, orig_font);
			++vpos;

		} else if (par_.isInset(pos)) {
			// If outer row has changed, nested insets are repaint completely.
			Inset const * inset = par_.getInset(pos);
			pi_.base.bv->coordCache().insets().add(inset, int(x_), yo_);
			paintInset(inset, pos);
			++vpos;

		} else {
			// paint as many characters as possible.
			paintFromPos(vpos);
		}
	}

	// if we reach the end of a struck out range, paint it
	if (running_strikeout) {
		// calculate 1/3 height of the buffer's default font
		FontMetrics const & fm
			= theFontMetrics(pi_.base.bv->buffer().params().getFont());
		int const middle = yo_ - fm.maxAscent() / 3;
		pi_.pain.line(last_strikeout_x, middle, int(x_), middle,
			Color::deletedtext, Painter::line_solid, Painter::line_thin);
		running_strikeout = false;
	}
}

} // namespace lyx
