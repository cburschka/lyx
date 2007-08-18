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


namespace lyx {

using frontend::Painter;
using frontend::FontMetrics;

using std::endl;
using std::max;
using std::string;


namespace {

/// Flag: do a full redraw of inside text of inset
/// Working variable indicating a full screen refresh
bool refreshInside;

/**
 * A class used for painting an individual row of text.
 */
class RowPainter {
public:
	/// initialise and run painter
	RowPainter(PainterInfo & pi, Text const & text,
		pit_type pit, Row const & row, Bidi & bidi, int x, int y);

	// paint various parts
	void paintAppendix();
	void paintDepthBar();
	void paintChangeBar();
	void paintFirst();
	void paintLast();
	void paintText();
	int maxWidth() { return max_width_; }

private:
	void paintForeignMark(double orig_x, Font const & font, int desc = 0);
	void paintHebrewComposeChar(pos_type & vpos, Font const & font);
	void paintArabicComposeChar(pos_type & vpos, Font const & font);
	void paintChars(pos_type & vpos, Font const & font,
			bool hebrew, bool arabic);
	int paintAppendixStart(int y);
	void paintFromPos(pos_type & vpos);
	void paintInset(pos_type const pos, Font const & font);

	/// return left margin
	int leftMargin() const;

	/// return the label font for this row
	Font const getLabelFont() const;

	/// bufferview to paint on
	BufferView & bv_;

	/// Painter to use
	Painter & pain_;

	/// Text for the row
	Text const & text_;
	TextMetrics & text_metrics_;
	ParagraphList const & pars_;

	/// The row to paint
	Row const & row_;

	/// Row's paragraph
	pit_type const pit_;
	Paragraph const & par_;
	ParagraphMetrics const & pm_;
	int max_width_;

	/// bidi cache, comes from outside the rowpainter because
	/// rowpainters are normally created in a for loop and there only
	/// one of them is active at a time.
	Bidi & bidi_;

	/// is row erased? (change tracking)
	bool erased_;

	// Looks ugly - is
	double const xo_;
	int const yo_;    // current baseline
	double x_;
	int width_;
	double separator_;
	double hfill_;
	double label_hfill_;
};


RowPainter::RowPainter(PainterInfo & pi,
	Text const & text, pit_type pit, Row const & row, Bidi & bidi, int x, int y)
	: bv_(*pi.base.bv), pain_(pi.pain), text_(text),
	  text_metrics_(pi.base.bv->textMetrics(&text)),
	  pars_(text.paragraphs()),
	  row_(row), pit_(pit), par_(text.paragraphs()[pit]),
	  pm_(text_metrics_.parMetrics(pit)),
	  max_width_(bv_.workWidth()),
		bidi_(bidi), erased_(pi.erased_),
	  xo_(x), yo_(y), width_(text_metrics_.width())
{
	RowMetrics m = text_metrics_.computeRowMetrics(pit_, row_);
	bidi_.computeTables(par_, *bv_.buffer(), row_);
	x_ = m.x + xo_;

	//lyxerr << "RowPainter: x: " << x_ << " xo: " << xo_ << " yo: " << yo_ << endl;
	//row_.dump();

	separator_ = m.separator;
	hfill_ = m.hfill;
	label_hfill_ = m.label_hfill;

	BOOST_ASSERT(pit >= 0);
	BOOST_ASSERT(pit < int(text.paragraphs().size()));
}


Font const RowPainter::getLabelFont() const
{
	return text_.getLabelFont(*bv_.buffer(), par_);
}


int RowPainter::leftMargin() const
{
	return text_.leftMargin(*bv_.buffer(), max_width_, pit_, row_.pos());
}


// If you want to debug inset metrics uncomment the following line:
// #define DEBUG_METRICS
// This draws green lines around each inset.


void RowPainter::paintInset(pos_type const pos, Font const & font)
{
	Inset const * inset = par_.getInset(pos);
	BOOST_ASSERT(inset);
	PainterInfo pi(const_cast<BufferView *>(&bv_), pain_);
	// FIXME: We should always use font, see documentation of
	// noFontChange() in Inset.h.
	pi.base.font = inset->noFontChange() ?
		bv_.buffer()->params().getFont() :
		font;
	pi.ltr_pos = (bidi_.level(pos) % 2 == 0);
	pi.erased_ = erased_ || par_.isDeleted(pos);
#ifdef DEBUG_METRICS
	int const x1 = int(x_);
#endif
	bv_.coordCache().insets().add(inset, int(x_), yo_);
	InsetText const * const in = inset->asTextInset();
	// non-wide insets are painted completely. Recursive
	bool tmp = refreshInside;
	if (!in || !in->wide()) {
		refreshInside = true;
		LYXERR(Debug::PAINTING) << endl << "Paint inset fully" << endl;
	}
	if (refreshInside)
		inset->drawSelection(pi, int(x_), yo_);
	inset->draw(pi, int(x_), yo_);
	refreshInside = tmp;
	x_ += inset->width();
#ifdef DEBUG_METRICS
	Dimension dim;
	BOOST_ASSERT(max_witdh_ > 0);
	int right_margin = text_metrics_.rightMargin(pm_);
	int const w = max_witdh_ - leftMargin() - right_margin;
	MetricsInfo mi(&bv_, font, w);
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
	pi.pain.line(x1, y1, x1, y2, Color::green);
	pi.pain.line(x1, y1, x2, y1, Color::green);
	pi.pain.line(x2, y1, x2, y2, Color::green);
	pi.pain.line(x1, y2, x2, y2, Color::green);
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
					text_.getFont(*bv_.buffer(), par_, i));
				dx = (c == 0x05e8 || // resh
				      c == 0x05d3)   // dalet
					? width2 - width
					: (width2 - width) / 2;
			}
			break;
		}
	}

	// Draw nikud
	pain_.text(int(x_) + dx, yo_, str, font);
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
						text_.getFont(*bv_.buffer(), par_, i));
				dx = (width2 - width) / 2;
			}
			break;
		}
	}
	// Draw nikud
	pain_.text(int(x_) + dx, yo_, str, font);
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
		x_ += pain_.text(int(x_), yo_, s, copy);
	} else {
		x_ += pain_.text(int(x_), yo_, s, font);
	}
}


void RowPainter::paintForeignMark(double orig_x, Font const & font, int desc)
{
	if (!lyxrc.mark_foreign_language)
		return;
	if (font.language() == latex_language)
		return;
	if (font.language() == bv_.buffer()->params().language)
		return;

	int const y = yo_ + 1 + desc;
	pain_.line(int(orig_x), y, int(x_), y, Color::language);
}


void RowPainter::paintFromPos(pos_type & vpos)
{
	pos_type const pos = bidi_.vis2log(vpos);
	Font orig_font = text_.getFont(*bv_.buffer(), par_, pos);

	double const orig_x = x_;

	if (par_.isInset(pos)) {
		paintInset(pos, orig_font);
		++vpos;
		paintForeignMark(orig_x, orig_font,
			par_.getInset(pos)->descent());
		return;
	}

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

	pain_.fillRectangle(5, yo_ - row_.ascent(), 3, height, Color::changebar);
}


void RowPainter::paintAppendix()
{
	// only draw the appendix frame once (for the main text)
	if (!par_.params().appendix() || !text_.isMainText(*bv_.buffer()))
		return;

	int y = yo_ - row_.ascent();

	if (par_.params().startOfAppendix())
		y += 2 * defaultRowHeight();

	pain_.line(1, y, 1, yo_ + row_.height(), Color::appendix);
	pain_.line(width_ - 2, y, width_ - 2, yo_ + row_.height(), Color::appendix);
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
		if (text_.isMainText(*bv_.buffer()))
			x += changebarMargin();

		int const starty = yo_ - row_.ascent();
		int const h =  row_.height() - 1 - (i - next_depth - 1) * 3;

		pain_.line(x, starty, x, starty + h, Color::depthbar);

		if (i > prev_depth)
			pain_.fillRectangle(x, starty, w, 2, Color::depthbar);
		if (i > next_depth)
			pain_.fillRectangle(x, starty + h, w, 2, Color::depthbar);
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

	pain_.rectText(text_start, y + d, label, pb_font, Color::none, Color::none);

	pain_.line(int(xo_ + 1), y, text_start, y, Color::appendix);
	pain_.line(text_end, y, int(xo_ + width_ - 2), y, Color::appendix);

	return 3 * defaultRowHeight();
}


void RowPainter::paintFirst()
{
	ParagraphParameters const & parparams = par_.params();

	int y_top = 0;

	// start of appendix?
	if (parparams.startOfAppendix())
		y_top += paintAppendixStart(yo_ - row_.ascent() + 2 * defaultRowHeight());

	Buffer const & buffer = *bv_.buffer();

	Layout_ptr const & layout = par_.layout();

	if (buffer.params().paragraph_separation == BufferParams::PARSEP_SKIP) {
		if (pit_ != 0) {
			if (layout->latextype == LATEX_PARAGRAPH
				&& !par_.getDepth()) {
				y_top += buffer.params().getDefSkip().inPixels(bv_);
			} else {
				Layout_ptr const & playout = pars_[pit_ - 1].layout();
				if (playout->latextype == LATEX_PARAGRAPH
					&& !pars_[pit_ - 1].getDepth()) {
					// is it right to use defskip here, too? (AS)
					y_top += buffer.params().getDefSkip().inPixels(bv_);
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

				pain_.text(int(x), yo_ - maxdesc - labeladdon, str, font);
			} else {
				if (is_rtl) {
					x = width_ - leftMargin()
						+ fm.width(layout->labelsep);
				} else {
					x = x_ - fm.width(layout->labelsep)
						- fm.width(str);
				}

				pain_.text(int(x), yo_, str, font);
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
			pain_.text(int(x), yo_ - maxdesc - labeladdon, str, font);
		}
	}
}


void RowPainter::paintLast()
{
	bool const is_rtl = text_.isRTL(*bv_.buffer(), par_);
	int const endlabel = getEndLabel(pit_, text_.paragraphs());

	// paint imaginary end-of-paragraph character

	if (par_.isInserted(par_.size()) || par_.isDeleted(par_.size())) {
		FontMetrics const & fm = theFontMetrics(bv_.buffer()->params().getFont());
		int const length = fm.maxAscent() / 2;
		Color::color col = par_.isInserted(par_.size()) ? Color::addedtext : Color::deletedtext;

		pain_.line(int(x_) + 1, yo_ + 2, int(x_) + 1, yo_ + 2 - length, col,
			   Painter::line_solid, Painter::line_thick);
		pain_.line(int(x_) + 1 - length, yo_ + 2, int(x_) + 1, yo_ + 2, col,
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
			pain_.rectangle(x, y, size, size, Color::eolmarker);
		else
			pain_.fillRectangle(x, y, size, size, Color::eolmarker);
		break;
	}

	case END_LABEL_STATIC: {
		Font font = getLabelFont();
		FontMetrics const & fm = theFontMetrics(font);
		docstring const & str = par_.layout()->endlabelstring();
		double const x = is_rtl ?
			x_ - fm.width(str)
			: - text_metrics_.rightMargin(pm_) - row_.width();
		pain_.text(int(x), yo_, str, font);
		break;
	}

	case END_LABEL_NO_LABEL:
		break;
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

	Layout_ptr const & layout = par_.layout();

	bool running_strikeout = false;
	bool is_struckout = false;
	int last_strikeout_x = 0;

	// Use font span to speed things up, see below
	FontSpan font_span;
	Font font;
	Buffer const & buffer = *bv_.buffer();

	// If the last logical character is a separator, don't paint it, unless
	// it's in the last row of a paragraph; see skipped_sep_vpos declaration
	if (end > 0 && end < par_.size() && par_.isSeparator(end - 1))
		skipped_sep_vpos = bidi_.log2vis(end - 1);
	
	for (pos_type vpos = row_.pos(); vpos < end; ) {
		if (x_ > bv_.workWidth())
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
				= theFontMetrics(bv_.buffer()->params().getFont());
			int const middle = yo_ - fm.maxAscent() / 3;
			pain_.line(last_strikeout_x, middle, int(x_), middle,
				Color::deletedtext, Painter::line_solid, Painter::line_thin);
			running_strikeout = false;
		}

		if (body_pos > 0 && pos == body_pos - 1) {
			int const lwidth = theFontMetrics(getLabelFont())
				.width(layout->labelsep);

			x_ += label_hfill_ + lwidth - width_pos;
		}

		if (par_.isHfill(pos)) {
			x_ += 1;

			int const y0 = yo_;
			int const y1 = y0 - defaultRowHeight() / 2;

			pain_.line(int(x_), y1, int(x_), y0, Color::added_space);

			if (par_.hfillExpansion(row_, pos)) {
				int const y2 = (y0 + y1) / 2;

				if (pos >= body_pos) {
					pain_.line(int(x_), y2, int(x_ + hfill_), y2,
						  Color::added_space,
						  Painter::line_onoffdash);
					x_ += hfill_;
				} else {
					pain_.line(int(x_), y2, int(x_ + label_hfill_), y2,
						  Color::added_space,
						  Painter::line_onoffdash);
					x_ += label_hfill_;
				}
				pain_.line(int(x_), y1, int(x_), y0, Color::added_space);
			}
			x_ += 2;
			++vpos;
		} else if (par_.isSeparator(pos)) {
			Font orig_font = text_.getFont(*bv_.buffer(), par_, pos);
			double const orig_x = x_;
			x_ += width_pos;
			if (pos >= body_pos)
				x_ += separator_;
			++vpos;
			paintForeignMark(orig_x, orig_font);
		} else {
			paintFromPos(vpos);
		}
	}

	// if we reach the end of a struck out range, paint it
	if (running_strikeout) {
		// calculate 1/3 height of the buffer's default font
		FontMetrics const & fm
			= theFontMetrics(bv_.buffer()->params().getFont());
		int const middle = yo_ - fm.maxAscent() / 3;
		pain_.line(last_strikeout_x, middle, int(x_), middle,
			Color::deletedtext, Painter::line_solid, Painter::line_thin);
		running_strikeout = false;
	}
}


bool CursorOnRow(PainterInfo & pi, pit_type const pit,
	RowList::const_iterator rit, Text const & text)
{
	// Is there a cursor on this row (or inside inset on row)
	Cursor & cur = pi.base.bv->cursor();
	for (size_type d = 0; d < cur.depth(); ++d) {
		CursorSlice const & sl = cur[d];
		if (sl.text() == &text
		    && sl.pit() == pit
		    && sl.pos() >= rit->pos()
		    && sl.pos() <= rit->endpos())
			return true;
	}
	return false;
}


bool innerCursorOnRow(PainterInfo & pi, pit_type pit,
	RowList::const_iterator rit, Text const & text)
{
	// Is there a cursor inside an inset on this row, and is this inset
	// the only "character" on this row
	Cursor & cur = pi.base.bv->cursor();
	if (rit->pos() + 1 != rit->endpos())
		return false;
	for (size_type d = 0; d < cur.depth(); d++) {
		CursorSlice const & sl = cur[d];
		if (sl.text() == &text
		    && sl.pit() == pit
		    && sl.pos() == rit->pos())
			return d < cur.depth() - 1;
	}
	return false;
}


// FIXME: once wide() is obsolete, remove this as well!
bool inNarrowInset(PainterInfo & pi)
{
	// check whether the current inset is nested in a non-wide inset
	Cursor & cur = pi.base.bv->cursor();
	Inset const * cur_in = &cur.inset();
	// check all higher nested insets
	for (size_type i = 1; i < cur.depth(); ++i) {
		Inset * const in = &cur[i].inset();
		if (in == cur_in)
			// we reached the level of the current inset, so stop
			return false;
		else if (in) {
			if (in->hasFixedWidth())
				return true;
			InsetText * t =
				const_cast<InsetText *>(in->asTextInset());
			if (t && !t->wide())
				// OK, we are in a non-wide() inset
				return true;
		}
	}
	return false;
}


void paintPar
	(PainterInfo & pi, Text const & text, pit_type pit, int x, int y,
	 bool repaintAll)
{
//	lyxerr << "  paintPar: pit: " << pit << " at y: " << y << endl;
	int const ww = pi.base.bv->workHeight();

	pi.base.bv->coordCache().parPos()[&text][pit] = Point(x, y);

	Paragraph const & par = text.paragraphs()[pit];
	ParagraphMetrics const & pm = pi.base.bv->parMetrics(&text, pit);
	if (pm.rows().empty())
		return;

	RowList::const_iterator const rb = pm.rows().begin();
	RowList::const_iterator const re = pm.rows().end();

	Bidi bidi;

	y -= rb->ascent();
	size_type rowno = 0;
	for (RowList::const_iterator rit = rb; rit != re; ++rit, ++rowno) {
		y += rit->ascent();
		// Allow setting of refreshInside for nested insets in
		// this row only
		bool tmp = refreshInside;

		// Row signature; has row changed since last paint?
		bool row_has_changed = pm.rowChangeStatus()[rowno];

		bool cursor_on_row = CursorOnRow(pi, pit, rit, text);
		bool in_inset_alone_on_row =
			innerCursorOnRow(pi, pit, rit, text);
		bool leftEdgeFixed =
			(par.getAlign() == LYX_ALIGN_LEFT ||
			 par.getAlign() == LYX_ALIGN_BLOCK);
		bool inNarrowIns = inNarrowInset(pi);

		// If this is the only object on the row, we can make it wide
		//
		// FIXME: there is a const_cast here because paintPar() is not supposed
		// to touch the paragraph contents. So either we move this "wide"
		// property out of InsetText or we localize the feature to the painting
		// done here.
		// JSpitzm: We should aim at removing wide() altogether while retaining
		// typing speed within insets.
		for (pos_type i = rit->pos() ; i != rit->endpos(); ++i) {
			Inset const * const in = par.getInset(i);
			if (in) {
				InsetText * t = const_cast<InsetText *>(in->asTextInset());
				if (t)
					t->setWide(in_inset_alone_on_row
						   && leftEdgeFixed
						   && !inNarrowIns);
			}
		}

		// If selection is on, the current row signature differs
		// from cache, or cursor is inside an inset _on this row_,
		// then paint the row
		if (repaintAll || row_has_changed || cursor_on_row) {
			bool const inside = (y + rit->descent() >= 0
				&& y - rit->ascent() < ww);
			// it is not needed to draw on screen if we are not inside.
			pi.pain.setDrawingEnabled(inside);
			RowPainter rp(pi, text, pit, *rit, bidi, x, y);
			// Clear background of this row
			// (if paragraph background was not cleared)
			if (!repaintAll &&
			    (!(in_inset_alone_on_row && leftEdgeFixed && !inNarrowIns)
				|| row_has_changed)) {
				pi.pain.fillRectangle(x, y - rit->ascent(),
				    rp.maxWidth(), rit->height(),
				    text.backgroundColor());
				// If outer row has changed, force nested
				// insets to repaint completely
				if (row_has_changed)
					refreshInside = true;
			}

			// Instrumentation for testing row cache (see also
			// 12 lines lower):
			if (lyxerr.debugging(Debug::PAINTING)) {
				if (text.isMainText(*pi.base.bv->buffer()))
					LYXERR(Debug::PAINTING) << "#";
				else
					LYXERR(Debug::PAINTING) << "[" <<
						repaintAll << row_has_changed <<
						cursor_on_row << "]";
			}
			rp.paintAppendix();
			rp.paintDepthBar();
			rp.paintChangeBar();
			if (rit == rb)
				rp.paintFirst();
			rp.paintText();
			if (rit + 1 == re)
				rp.paintLast();
		}
		y += rit->descent();
		// Restore, see above
		refreshInside = tmp;
	}
	// Re-enable screen drawing for future use of the painter.
	pi.pain.setDrawingEnabled(true);

	LYXERR(Debug::PAINTING) << "." << endl;
}

} // namespace anon


void paintText(BufferView & bv,
	       Painter & pain)
{
	BOOST_ASSERT(bv.buffer());
	Buffer const & buffer = *bv.buffer();
	Text & text = buffer.text();
	bool const select = bv.cursor().selection();
	ViewMetricsInfo const & vi = bv.viewMetricsInfo();

	PainterInfo pi(const_cast<BufferView *>(&bv), pain);
	// Should the whole screen, including insets, be refreshed?
	// FIXME: We should also distinguish DecorationUpdate to avoid text
	// drawing if possible. This is not possible to do easily right now
	// because of the single backing pixmap.
	bool repaintAll = select || vi.update_strategy != SingleParUpdate;

	if (repaintAll) {
		// Clear background (if not delegated to rows)
		pain.fillRectangle(0, vi.y1, bv.workWidth(), vi.y2 - vi.y1,
			text.backgroundColor());
	}
	if (select) {
		text.drawSelection(pi, 0, 0);
	}

	int yy = vi.y1;
	// draw contents
	for (pit_type pit = vi.p1; pit <= vi.p2; ++pit) {
		refreshInside = repaintAll;
		ParagraphMetrics const & pm = bv.parMetrics(&text, pit);
		yy += pm.ascent();
		paintPar(pi, text, pit, 0, yy, repaintAll);
		yy += pm.descent();
	}

	// and grey out above (should not happen later)
//	lyxerr << "par ascent: " << text.getPar(vi.p1).ascent() << endl;
	if (vi.y1 > 0 && vi.update_strategy == FullScreenUpdate)
		pain.fillRectangle(0, 0, bv.workWidth(), vi.y1, Color::bottomarea);

	// and possibly grey out below
//	lyxerr << "par descent: " << text.getPar(vi.p1).ascent() << endl;
	if (vi.y2 < bv.workHeight() && vi.update_strategy == FullScreenUpdate)
		pain.fillRectangle(0, vi.y2, bv.workWidth(), bv.workHeight() - vi.y2, Color::bottomarea);
}


void paintTextInset(Text const & text, PainterInfo & pi, int x, int y)
{
//	lyxerr << "  paintTextInset: y: " << y << endl;

	y -= pi.base.bv->parMetrics(&text, 0).ascent();
	// This flag cannot be set from within same inset:
	bool repaintAll = refreshInside;
	for (int pit = 0; pit < int(text.paragraphs().size()); ++pit) {
		ParagraphMetrics const & pmi
			= pi.base.bv->parMetrics(&text, pit);
		y += pmi.ascent();
		paintPar(pi, text, pit, x, y, repaintAll);
		y += pmi.descent();
	}
}


} // namespace lyx
