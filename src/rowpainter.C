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
using lyx::pit_type;

using std::endl;
using std::max;
using std::min;
using std::string;


namespace {

/**
 * A class used for painting an individual row of text.
 */
class RowPainter {
public:
	/// initialise and run painter
	RowPainter(PainterInfo & pi, LyXText const & text,
		pit_type pit, Row const & row, int x, int y);

	// paint various parts
	void paintAppendix();
	void paintDepthBar();
	void paintChangeBar();
	void paintFirst();
	void paintLast();
	void paintText();

private:
	void paintForeignMark(double orig_x, LyXFont const & font);
	void paintHebrewComposeChar(lyx::pos_type & vpos, LyXFont const & font);
	void paintArabicComposeChar(lyx::pos_type & vpos, LyXFont const & font);
	void paintChars(lyx::pos_type & vpos, LyXFont font, 
	                bool hebrew, bool arabic);
	int paintAppendixStart(int y);
	void paintFromPos(lyx::pos_type & vpos);
	void paintInset(lyx::pos_type const pos, LyXFont const & font);

	/// return left margin
	int leftMargin() const;

	/// return the label font for this row
	LyXFont const getLabelFont() const;

	/// bufferview to paint on
	BufferView const & bv_;

	/// Painter to use
	Painter & pain_;

	/// LyXText for the row
	LyXText const & text_;
	ParagraphList const & pars_;

	/// The row to paint
	Row const & row_;

	/// Row's paragraph
	pit_type const pit_;
	Paragraph const & par_;

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
	LyXText const & text, pit_type pit, Row const & row, int x, int y)
	: bv_(*pi.base.bv), pain_(pi.pain), text_(text), pars_(text.paragraphs()),
	  row_(row), pit_(pit), par_(text.paragraphs()[pit]),
	  erased_(pi.erased_),
	  xo_(x), yo_(y), width_(text_.width())
{
	RowMetrics m = text_.computeRowMetrics(pit, row_);
	x_ = m.x + xo_;

	//lyxerr << "RowPainter: x: " << x_ << " xo: " << xo_ << " yo: " << yo_ << endl;
	//row_.dump();

	separator_ = m.separator;
	hfill_ = m.hfill;
	label_hfill_ = m.label_hfill;

	BOOST_ASSERT(pit >= 0);
	BOOST_ASSERT(pit < int(text.paragraphs().size()));
}


LyXFont const RowPainter::getLabelFont() const
{
	return text_.getLabelFont(par_);
}


int RowPainter::leftMargin() const
{
	return text_.leftMargin(pit_, row_.pos());
}


void RowPainter::paintInset(pos_type const pos, LyXFont const & font)
{
	InsetBase const * inset = par_.getInset(pos);
	BOOST_ASSERT(inset);
	PainterInfo pi(const_cast<BufferView *>(&bv_), pain_);
	pi.base.font = font;
	pi.ltr_pos = (text_.bidi.level(pos) % 2 == 0);
	pi.erased_ = erased_ || isDeletedText(par_, pos);
	theCoords.insets().add(inset, int(x_), yo_);
	inset->drawSelection(pi, int(x_), yo_);
	inset->draw(pi, int(x_), yo_);
	x_ += inset->width();
}


void RowPainter::paintHebrewComposeChar(pos_type & vpos, LyXFont const & font)
{
	pos_type pos = text_.bidi.vis2log(vpos);

	string str;

	// first char
	char c = par_.getChar(pos);
	str += c;
	++vpos;

	int const width = font_metrics::width(c, font);
	int dx = 0;

	for (pos_type i = pos - 1; i >= 0; --i) {
		c = par_.getChar(i);
		if (!Encodings::IsComposeChar_hebrew(c)) {
			if (IsPrintableNonspace(c)) {
				int const width2 =
					text_.singleWidth(par_, i, c, text_.getFont(par_, i));
				// dalet / resh
				dx = (c == 'ø' || c == 'ã')
					? width2 - width
					: (width2 - width) / 2;
			}
			break;
		}
	}

	// Draw nikud
	pain_.text(int(x_) + dx, yo_, str, font);
}


void RowPainter::paintArabicComposeChar(pos_type & vpos, LyXFont const & font)
{
	pos_type pos = text_.bidi.vis2log(vpos);
	string str;

	// first char
	char c = par_.getChar(pos);
	c = par_.transformChar(c, pos);
	str += c;
	++vpos;

	int const width = font_metrics::width(c, font);
	int dx = 0;

	for (pos_type i = pos - 1; i >= 0; --i) {
		c = par_.getChar(i);
		if (!Encodings::IsComposeChar_arabic(c)) {
			if (IsPrintableNonspace(c)) {
				int const width2 = 
					text_.singleWidth(par_, i, c, text_.getFont(par_, i));
				dx = (width2 - width) / 2;
			}
			break;
		}
	}
	// Draw nikud
	pain_.text(int(x_) + dx, yo_, str, font);
}


void RowPainter::paintChars(pos_type & vpos, LyXFont font, 
			    bool hebrew, bool arabic)
{
	pos_type pos = text_.bidi.vis2log(vpos);
	pos_type const end = row_.endpos();
	FontSpan const font_span = par_.fontSpan(pos);
	Change::Type const prev_change = par_.lookupChange(pos);

	// first character
	string str;
	str += par_.getChar(pos);
	if (arabic) {
		unsigned char c = str[0];
		str[0] = par_.transformChar(c, pos);
	}

	// collect as much similar chars as we can
	for (++vpos ; vpos < end ; ++vpos) {
		pos = text_.bidi.vis2log(vpos);
		if (pos < font_span.first || pos > font_span.last)
			break;

		if (prev_change != par_.lookupChange(pos))
			break;

		char c = par_.getChar(pos);

		if (!IsPrintableNonspace(c))
			break;

		if (arabic && Encodings::IsComposeChar_arabic(c))
			break;

		if (hebrew && Encodings::IsComposeChar_hebrew(c))
			break;

		if (arabic)
			c = par_.transformChar(c, pos);

		str += c;
	}

	if (prev_change == Change::DELETED)
		font.setColor(LColor::strikeout);
	else if (prev_change == Change::INSERTED)
		font.setColor(LColor::newtext);

	// Draw text and set the new x position
	//lyxerr << "paint row: yo_ " << yo_ << "\n";
	pain_.text(int(x_), yo_, str, font);
	x_ += font_metrics::width(str, font);
}


void RowPainter::paintForeignMark(double orig_x, LyXFont const & font)
{
	if (!lyxrc.mark_foreign_language)
		return;
	if (font.language() == latex_language)
		return;
	if (font.language() == bv_.buffer()->params().language)
		return;

	int const y = yo_ + 1;
	pain_.line(int(orig_x), y, int(x_), y, LColor::language);
}


void RowPainter::paintFromPos(pos_type & vpos)
{
	pos_type const pos = text_.bidi.vis2log(vpos);
	LyXFont orig_font = text_.getFont(par_, pos);
	text_.applyOuterFont(orig_font);

	double const orig_x = x_;

	if (par_.isInset(pos)) {
		paintInset(pos, orig_font);
		++vpos;
		paintForeignMark(orig_x, orig_font);
		return;
	}

	// usual characters, no insets
	char const c = par_.getChar(pos);

	// special case languages
	std::string const & lang = orig_font.language()->lang();
	bool const hebrew = lang == "hebrew";
	bool const arabic = lang == "arabic" &&
		(lyxrc.font_norm_type == LyXRC::ISO_8859_6_8 ||
		lyxrc.font_norm_type == LyXRC::ISO_10646_1);

	// draw as many chars as we can
	if ((!hebrew && !arabic)
		|| (hebrew && !Encodings::IsComposeChar_hebrew(c))
		|| (arabic && !Encodings::IsComposeChar_arabic(c))) {
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
	pos_type const end = row_.endpos();

	if (start == end || !par_.isChanged(start, end - 1))
		return;

	int const height = text_.isLastRow(pit_, row_)
		? row_.ascent()
		: row_.height();

	pain_.fillRectangle(4, yo_ - row_.ascent(), 5, height, LColor::changebar);
}


void RowPainter::paintAppendix()
{
	if (!par_.params().appendix())
		return;

	int y = yo_ - row_.ascent();

	if (par_.params().startOfAppendix())
		y += 2 * defaultRowHeight();

	pain_.line(1, y, 1, yo_ + row_.height(), LColor::appendix);
	pain_.line(width_ - 2, y, width_ - 2, yo_ + row_.height(), LColor::appendix);
}


void RowPainter::paintDepthBar()
{
	Paragraph::depth_type const depth = par_.getDepth();

	if (depth <= 0)
		return;

	Paragraph::depth_type prev_depth = 0;
	if (!text_.isFirstRow(pit_, row_)) {
		pit_type pit2 = pit_;
		if (row_.pos() == 0)
			--pit2;
		prev_depth = pars_[pit2].getDepth();
	}

	Paragraph::depth_type next_depth = 0;
	if (!text_.isLastRow(pit_, row_)) {
		pit_type pit2 = pit_;
		if (row_.endpos() >= pars_[pit2].size())
			++pit2;
		next_depth = pars_[pit2].getDepth();
	}

	for (Paragraph::depth_type i = 1; i <= depth; ++i) {
		int const w = nestMargin() / 5;
		int x = int(xo_) + w * i;
		// only consider the changebar space if we're drawing outermost text
		if (text_.isMainText())
			x += changebarMargin();

		int const starty = yo_ - row_.ascent();
		int const h =  row_.height() - 1 - (i - next_depth - 1) * 3;

		pain_.line(x, starty, x, starty + h, LColor::depthbar);

		if (i > prev_depth)
			pain_.fillRectangle(x, starty, w, 2, LColor::depthbar);
		if (i > next_depth)
			pain_.fillRectangle(x, starty + h, w, 2, LColor::depthbar);
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
	ParagraphParameters const & parparams = par_.params();

	int y_top = 0;

	// start of appendix?
	if (parparams.startOfAppendix())
		y_top += paintAppendixStart(yo_ - row_.ascent() + 2 * defaultRowHeight());

	Buffer const & buffer = *bv_.buffer();

	LyXLayout_ptr const & layout = par_.layout();

	if (buffer.params().paragraph_separation == BufferParams::PARSEP_SKIP) {
		if (pit_ != 0) {
			if (layout->latextype == LATEX_PARAGRAPH
				&& !par_.getDepth()) {
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

	bool const is_rtl = text_.isRTL(par_);
	bool const is_seq = isFirstInSequence(pit_, text_.paragraphs());
	//lyxerr << "paintFirst: " << par_.id() << " is_seq: " << is_seq << std::endl;

	// should we print a label?
	if (layout->labeltype >= LABEL_STATIC
	    && (layout->labeltype != LABEL_STATIC
		      || layout->latextype != LATEX_ENVIRONMENT
		      || is_seq)) {

		LyXFont const font = getLabelFont();
		string const str = par_.getLabelstring();
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

				int const labeladdon = int(font_metrics::maxHeight(font) * layout->spacing.getValue() * spacing_val);

				int const maxdesc = int(font_metrics::maxDescent(font) * layout->spacing.getValue() * spacing_val)
					+ int(layout->parsep) * defaultRowHeight();

				if (is_rtl) {
					x = width_ - leftMargin() -
						font_metrics::width(str, font);
				}

				pain_.text(int(x), yo_ - maxdesc - labeladdon, str, font);
			} else {
				if (is_rtl) {
					x = width_ - leftMargin()
						+ font_metrics::width(layout->labelsep, font);
				} else {
					x = x_ - font_metrics::width(layout->labelsep, font)
						- font_metrics::width(str, font);
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
		LyXFont font = getLabelFont();
		if (!par_.getLabelstring().empty()) {
			string const str = par_.getLabelstring();
			double spacing_val = 1.0;
			if (!parparams.spacing().isDefault())
				spacing_val = parparams.spacing().getValue();
			else
				spacing_val = buffer.params().spacing().getValue();

			int const labeladdon = int(font_metrics::maxHeight(font) * layout->spacing.getValue() * spacing_val);

			int maxdesc =
				int(font_metrics::maxDescent(font) * layout->spacing.getValue() * spacing_val
				+ (layout->labelbottomsep * defaultRowHeight()));

			double x = x_;
			if (layout->labeltype == LABEL_CENTERED_TOP_ENVIRONMENT) {
				x = ((is_rtl ? leftMargin() : x_)
					 + width_ - text_.rightMargin(par_)) / 2;
				x -= font_metrics::width(str, font) / 2;
			} else if (is_rtl) {
				x = width_ - leftMargin() -
					font_metrics::width(str, font);
			}
			pain_.text(int(x), yo_ - maxdesc - labeladdon, str, font);
		}
	}
}


void RowPainter::paintLast()
{
	bool const is_rtl = text_.isRTL(par_);
	int const endlabel = getEndLabel(pit_, text_.paragraphs());

	// draw an endlabel
	switch (endlabel) {
	case END_LABEL_BOX:
	case END_LABEL_FILLED_BOX: {
		LyXFont const font = getLabelFont();
		int const size = int(0.75 * font_metrics::maxAscent(font));
		int const y = yo_ - size;
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
		string const & str = par_.layout()->endlabelstring();
		double const x = is_rtl ?
			x_ - font_metrics::width(str, font)
			: - text_.rightMargin(par_) - row_.width();
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
	pos_type body_pos = par_.beginOfBody();
	if (body_pos > 0 &&
		(body_pos > end || !par_.isLineSeparator(body_pos - 1))) {
		body_pos = 0;
	}

	LyXLayout_ptr const & layout = par_.layout();

	bool running_strikeout = false;
	bool is_struckout = false;
	int last_strikeout_x = 0;

	// Use font span to speed things up, see below 
	FontSpan font_span;
	LyXFont font;

	for (pos_type vpos = row_.pos(); vpos < end; ) {
		if (x_ > bv_.workWidth())
			break;

		pos_type const pos = text_.bidi.vis2log(vpos);

		if (pos >= par_.size()) {
			++vpos;
			continue;
		}

		// Use font span to speed things up, see above
		if (vpos < font_span.first || vpos > font_span.last) {
			font_span = par_.fontSpan(vpos);
			font = text_.getFont(par_, vpos);
		}

		const int width_pos =
			text_.singleWidth(par_, vpos, par_.getChar(vpos), font);

		if (x_ + width_pos < 0) {
			x_ += width_pos;
			++vpos;
			continue;
		}

		is_struckout = isDeletedText(par_, pos);

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
			int const middle =
				yo_ - font_metrics::maxAscent(text_.defaultfont_) / 3;
			pain_.line(last_strikeout_x, middle, int(x_), middle,
				LColor::strikeout, Painter::line_solid, Painter::line_thin);
			running_strikeout = false;
		}

		if (body_pos > 0 && pos == body_pos - 1) {
			int const lwidth = font_metrics::width(layout->labelsep,
				getLabelFont());

			x_ += label_hfill_ + lwidth - width_pos;
		}

		if (par_.isHfill(pos)) {
			x_ += 1;

			int const y0 = yo_;
			int const y1 = y0 - defaultRowHeight() / 2;

			pain_.line(int(x_), y1, int(x_), y0, LColor::added_space);

			if (hfillExpansion(par_, row_, pos)) {
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
		} else if (par_.isSeparator(pos)) {
			x_ += width_pos;
			if (pos >= body_pos)
				x_ += separator_;
			++vpos;
		} else {
			paintFromPos(vpos);
		}
	}

	// if we reach the end of a struck out range, paint it
	if (running_strikeout) {
		// calculate 1/3 height of the buffer's default font
		int const middle =
			yo_ - font_metrics::maxAscent(text_.defaultfont_) / 3;
		pain_.line(last_strikeout_x, middle, int(x_), middle,
			LColor::strikeout, Painter::line_solid, Painter::line_thin);
		running_strikeout = false;
	}
}


void paintPar
	(PainterInfo & pi, LyXText const & text, pit_type pit, int x, int y)
{
//	lyxerr << "  paintPar: pit: " << pit << " at y: " << y << endl;
	static NullPainter nop;
	static PainterInfo nullpi(pi.base.bv, nop);
	int const ww = pi.base.bv->workHeight();

	Paragraph const & par = text.paragraphs()[pit];

	RowList::const_iterator const rb = par.rows().begin();
	RowList::const_iterator const re = par.rows().end();
	theCoords.parPos()[&text][pit] = Point(x, y);

	y -= rb->ascent();
	for (RowList::const_iterator rit = rb; rit != re; ++rit) {
		y += rit->ascent();
		bool const inside = (y + rit->descent() >= 0
				       && y - rit->ascent() < ww);
		RowPainter rp(inside ? pi : nullpi, text, pit, *rit, x, y);

		y += rit->descent();
		rp.paintAppendix();
		rp.paintDepthBar();
		rp.paintChangeBar();
		if (rit == rb)
			rp.paintFirst();
		if (rit + 1 == re)
			rp.paintLast();
		rp.paintText();
	}
}

} // namespace anon


void paintText(BufferView const & bv, ViewMetricsInfo const & vi)
{
	Painter & pain = bv.painter();
	LyXText * const text = bv.text();

	// clear background
	pain.fillRectangle(0, vi.y1, bv.workWidth(), vi.y2 - vi.y1,
			   LColor::background);

	// draw selection
	PainterInfo pi(const_cast<BufferView *>(&bv), pain);

	text->drawSelection(pi, 0, 0);

	int yy = vi.y1;
	// draw contents
	for (pit_type pit = vi.p1; pit <= vi.p2; ++pit) {
		yy += text->getPar(pit).ascent();
		paintPar(pi, *bv.text(), pit, 0, yy);
		yy += text->getPar(pit).descent();
	}


	// paint one paragraph above and one below
	// Note MV: this cannot be suppressed even for singlepar.
	// Try viewing the User Guide Mobius figure
	if (vi.p1 > 0) {
		text->redoParagraph(vi.p1 - 1);
		paintPar(pi, *bv.text(), vi.p1 - 1, 0,
			 vi.y1 -  text->getPar(vi.p1 - 1).descent());
	}

	if (vi.p2 < lyx::pit_type(text->paragraphs().size()) - 1) {
		text->redoParagraph(vi.p2 + 1);
		paintPar(pi, *bv.text(), vi.p2 + 1, 0,
			 vi.y2 + text->getPar(vi.p2 + 1).ascent());
	}

	// and grey out above (should not happen later)
//	lyxerr << "par ascent: " << text->getPar(vi.p1).ascent() << endl;
	if (vi.y1 > 0)
		pain.fillRectangle(0, 0, bv.workWidth(), vi.y1, LColor::bottomarea);

	// and possibly grey out below
//	lyxerr << "par descent: " << text->getPar(vi.p1).ascent() << endl;
	if (vi.y2 < bv.workHeight())
		pain.fillRectangle(0, vi.y2, bv.workWidth(), bv.workHeight() - vi.y2, LColor::bottomarea);
}


void paintTextInset(LyXText const & text, PainterInfo & pi, int x, int y)
{
//	lyxerr << "  paintTextInset: y: " << y << endl;

	y -= text.getPar(0).ascent();
	for (int pit = 0; pit < int(text.paragraphs().size()); ++pit) {
		y += text.getPar(pit).ascent();
		paintPar(pi, text, pit, x, y);
		y += text.getPar(pit).descent();
	}
}
