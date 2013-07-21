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
	Text const & text, pit_type pit, Row const & row, Bidi & bidi, int x, int y)
	: pi_(pi), text_(text),
	  text_metrics_(pi_.base.bv->textMetrics(&text)),
	  pars_(text.paragraphs()),
	  row_(row), pit_(pit), par_(text.paragraphs()[pit]),
	  pm_(text_metrics_.parMetrics(pit)),
	  bidi_(bidi), change_(pi_.change_),
	  xo_(x), yo_(y), width_(text_metrics_.width()),
	  solid_line_thickness_(1.0), solid_line_offset_(1),
	  dotted_line_thickness_(1.0), dotted_line_offset_(2)
{
	bidi_.computeTables(par_, pi_.base.bv->buffer(), row_);

	if (lyxrc.zoom >= 200) {
		// derive the line thickness from zoom factor
		// the zoom is given in percent
		// (increase thickness at 250%, 450% etc.)
		solid_line_thickness_ = (float)(int((lyxrc.zoom + 50) / 200.0));
		// adjust line_offset_ too
		solid_line_offset_ = 1 + int(0.5 * solid_line_thickness_);
	}
	if (lyxrc.zoom >= 100) {
		// derive the line thickness from zoom factor
		// the zoom is given in percent
		// (increase thickness at 150%, 250% etc.)
		dotted_line_thickness_ = (float)(int((lyxrc.zoom + 50) / 100.0));
		// adjust line_offset_ too
		dotted_line_offset_ = int(0.5 * dotted_line_thickness_) + 1;
	}

	x_ = row_.x + xo_;

	//lyxerr << "RowPainter: x: " << x_ << " xo: " << xo_ << " yo: " << yo_ << endl;
	//row_.dump();

	LBUFERR(pit >= 0);
	LBUFERR(pit < int(text.paragraphs().size()));
}


FontInfo RowPainter::labelFont() const
{
	FontInfo f = text_.labelFont(par_);
	// selected text?
	if (row_.begin_margin_sel || pi_.selected)
		f.setPaintColor(Color_selectiontext);
	return f;
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

	LASSERT(inset, return);
	// Backup full_repaint status because some insets (InsetTabular)
	// requires a full repaint
	bool pi_full_repaint = pi_.full_repaint;

	pi_.base.font = inset->inheritFont() ? font.fontInfo() :
		pi_.base.bv->buffer().params().getFont().fontInfo();
	pi_.ltr_pos = (bidi_.level(pos) % 2 == 0);
	Change prev_change = change_;
	pi_.change_ = change_.changed() ? change_ : par_.lookupChange(pos);

	int const x1 = int(x_);
	pi_.base.bv->coordCache().insets().add(inset, x1, yo_);
	// insets are painted completely. Recursive
	// FIXME: it is wrong to completely paint the background
	// if we want to do single row painting.
	inset->drawBackground(pi_, x1, yo_);
	inset->drawSelection(pi_, x1, yo_);
	inset->draw(pi_, x1, yo_);

	Dimension const & dim = pm_.insetDimension(inset);

	paintForeignMark(x_, font.language(), dim.descent());

	x_ += dim.width();

	// Restore full_repaint status.
	pi_.full_repaint = pi_full_repaint;
	pi_.change_ = prev_change;

#ifdef DEBUG_METRICS
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
	// first character
	char_type prev_char = par_.getChar(pos);
	vector<char_type> str;
	str.reserve(100);
	str.push_back(prev_char);

	// FIXME: Why only round brackets and why the difference to
	// Hebrew? See also Paragraph::getUChar
	if (arabic) {
		char_type c = str[0];
		if (c == '(')
			c = ')';
		else if (c == ')')
			c = '(';
		str[0] = par_.transformChar(c, pos);
	}

	pos_type const end = row_.endpos();
	FontSpan const font_span = par_.fontSpan(pos);
	// Track-change status.
	Change const & change_running = par_.lookupChange(pos);

	// selected text?
	bool const selection = (pos >= row_.sel_beg && pos < row_.sel_end)
		|| pi_.selected;

	// spelling correct?
	bool const spell_state =
		lyxrc.spellcheck_continuously && par_.isMisspelled(pos);

	// collect as much similar chars as we can
	for (++vpos ; vpos < end ; ++vpos) {
		if (lyxrc.force_paint_single_char)
			break;

		pos = bidi_.vis2log(vpos);
		if (pos < font_span.first || pos > font_span.last)
			break;

		bool const new_selection = pos >= row_.sel_beg && pos < row_.sel_end;
		if (new_selection != selection)
			// Selection ends or starts here.
			break;

		bool const new_spell_state =
			lyxrc.spellcheck_continuously && par_.isMisspelled(pos);
		if (new_spell_state != spell_state)
			// Spell checker state changed here.
			break;

		Change const & change = par_.lookupChange(pos);
		if (!change_running.isSimilarTo(change))
			// Track change type or author has changed.
			break;

		char_type c = par_.getChar(pos);

		if (c == '\t')
			break;

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

		// FIXME: Why only round brackets and why the difference to
		// Hebrew? See also Paragraph::getUChar
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
		prev_char = c;
	}

	docstring s(&str[0], str.size());

	if (s[0] == '\t')
		s.replace(0,1,from_ascii("    "));

	if (!selection && !change_running.changed()) {
		x_ += pi_.pain.text(int(x_), yo_, s, font);
		return;
	}

	FontInfo copy = font;
	if (change_running.changed())
		copy.setPaintColor(change_running.color());
	else if (selection)
		copy.setPaintColor(Color_selectiontext);

	x_ += pi_.pain.text(int(x_), yo_, s, copy);
}


void RowPainter::paintSeparator(double orig_x, double width,
	FontInfo const & font)
{
	pi_.pain.textDecoration(font, int(orig_x), yo_, int(width));
	x_ += width;
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

	int const y = yo_ + solid_line_offset_ + desc + int(solid_line_thickness_/2);
	pi_.pain.line(int(orig_x), y, int(x_), y, Color_language,
		Painter::line_solid, solid_line_thickness_);
}


void RowPainter::paintMisspelledMark(double orig_x, bool changed)
{
	// if changed the misspelled marker gets placed slightly lower than normal
	// to avoid drawing at the same vertical offset
	float const y = yo_ + solid_line_offset_ + solid_line_thickness_
		+ (changed ? solid_line_thickness_ + 1 : 0)
		+ dotted_line_offset_;
	pi_.pain.line(int(orig_x), int(y), int(x_), int(y), Color_error,
		Painter::line_onoffdash, dotted_line_thickness_);
}


void RowPainter::paintFromPos(pos_type & vpos, bool changed)
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

	// spelling correct?
	bool const misspelled =
		lyxrc.spellcheck_continuously && par_.isMisspelled(pos);

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

	if (lyxrc.spellcheck_continuously && misspelled) {
		// check for cursor position
		// don't draw misspelled marker for words at cursor position
		// we don't want to disturb the process of text editing
		BufferView const * bv = pi_.base.bv;
		DocIterator const nw = bv->cursor().newWord();
		bool new_word = false;
		if (!nw.empty() && par_.id() == nw.paragraph().id()) {
			pos_type cpos = nw.pos();
			if (cpos > 0 && cpos == par_.size() && !par_.isWordSeparator(cpos-1))
				--cpos;
			else if (cpos > 0 && par_.isWordSeparator(cpos))
				--cpos;
			new_word = par_.isSameSpellRange(pos, cpos) ;
		}
		if (!new_word)
			paintMisspelledMark(orig_x, changed);
	}
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
	if (!par_.params().appendix() || !text_.isMainText())
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
	BufferParams const & bparams = pi_.base.bv->buffer().params();
	Layout const & layout = par_.layout();

	int y_top = 0;

	// start of appendix?
	if (par_.params().startOfAppendix())
		y_top += paintAppendixStart(yo_ - row_.ascent() + 2 * defaultRowHeight());

	if (bparams.paragraph_separation == BufferParams::ParagraphSkipSeparation
		&& pit_ != 0) {
		if (layout.latextype == LATEX_PARAGRAPH
		    && !par_.getDepth()) {
			y_top += bparams.getDefSkip().inPixels(*pi_.base.bv);
		} else {
			Layout const & playout = pars_[pit_ - 1].layout();
			if (playout.latextype == LATEX_PARAGRAPH
			    && !pars_[pit_ - 1].getDepth()) {
				// is it right to use defskip here, too? (AS)
				y_top += bparams.getDefSkip().inPixels(*pi_.base.bv);
			}
		}
	}

	bool const is_first =
		text_.isFirstInSequence(pit_) || !layout.isParagraphGroup();
	//lyxerr << "paintFirst: " << par_.id() << " is_seq: " << is_seq << endl;

	if (layout.labelIsInline()
			&& (layout.labeltype != LABEL_STATIC || is_first)) {
		paintLabel();
	} else if (is_first && layout.labelIsAbove()) {
		paintTopLevelLabel();
	}
}


void RowPainter::paintLabel()
{
	docstring const str = par_.labelString();
	if (str.empty())
		return;

	bool const is_rtl = text_.isRTL(par_);
	Layout const & layout = par_.layout();
	FontInfo const font = labelFont();
	FontMetrics const & fm = theFontMetrics(font);
	double x = x_;

	if (is_rtl) {
		x = width_ - leftMargin()
			+ fm.width(layout.labelsep);
	} else {
		x = x_ - fm.width(layout.labelsep)
			- fm.width(str);
	}

	pi_.pain.text(int(x), yo_, str, font);
}


void RowPainter::paintTopLevelLabel()
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
		if (is_rtl)
			x = leftMargin();
		x += (width_ - text_metrics_.rightMargin(pm_) - leftMargin()) / 2;
		x -= fm.width(str) / 2;
	} else if (is_rtl) {
		x = width_ - leftMargin() -	fm.width(str);
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


void RowPainter::paintLast()
{
	bool const is_rtl = text_.isRTL(par_);
	int const endlabel = getEndLabel(pit_, text_);

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
		double const x = is_rtl ? x_ - fm.width(str) : x_;
		pi_.pain.text(int(x), yo_, str, font);
		break;
	}

	case END_LABEL_NO_LABEL:
		if (lyxrc.paragraph_markers && size_type(pit_ + 1) < pars_.size()) {
			docstring const s = docstring(1, char_type(0x00B6));
			FontInfo f = FontInfo(text_.layoutFont(pit_));
			f.setColor(Color_paragraphmarker);
			pi_.pain.text(int(x_), yo_, s, f);
			x_ += theFontMetrics(f).width(s);
		}
		break;
	}
}


void RowPainter::paintOnlyInsets()
{
	CoordCache const & cache = pi_.base.bv->coordCache();
	pos_type const end = row_.endpos();
	for (pos_type pos = row_.pos(); pos != end; ++pos) {
		// If outer row has changed, nested insets are repaint completely.
		Inset const * inset = par_.getInset(pos);
		bool const nested_inset = inset &&
				((inset->asInsetMath() &&
				  !inset->asInsetMath()->asMacroTemplate())
				 || inset->asInsetText()
				 || inset->asInsetTabular());
		if (!nested_inset)
			continue;
		if (x_ > pi_.base.bv->workWidth()
		    || !cache.getInsets().has(inset))
			continue;
		x_ = cache.getInsets().x(inset);

		bool const pi_selected = pi_.selected;
		Cursor const & cur = pi_.base.bv->cursor();
		if (cur.selection() && cur.text() == &text_
			  && cur.normalAnchor().text() == &text_)
			pi_.selected = row_.sel_beg <= pos && row_.sel_end > pos;
		paintInset(inset, pos);
		pi_.selected = pi_selected;
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

	Change change_running;
	int change_last_x = 0;

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
		Change const & change = par_.lookupChange(pos);
		if (change.changed() && !change_running.changed()) {
			change_running = change;
			change_last_x = int(x_);
		}

		Inset const * inset = par_.getInset(pos);
		bool const highly_editable_inset = inset
			&& inset->editable();

		// If we reach the end of a change or if the author changes, paint it.
		// We also don't paint across things like tables
		if (change_running.changed() && (highly_editable_inset
			|| !change.changed() || !change_running.isSimilarTo(change))) {
			// Calculate 1/3 height of the buffer's default font
			FontMetrics const & fm
				= theFontMetrics(pi_.base.bv->buffer().params().getFont());
			float const y_bar = change_running.deleted() ?
				yo_ - fm.maxAscent() / 3 : yo_ + 2 * solid_line_offset_ + solid_line_thickness_;
			pi_.pain.line(change_last_x, int(y_bar), int(x_), int(y_bar),
				change_running.color(), Painter::line_solid, solid_line_thickness_);

			// Change might continue with a different author or type
			if (change.changed() && !highly_editable_inset) {
				change_running = change;
				change_last_x = int(x_);
			} else
				change_running.setUnchanged();
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
			double separator_width = width_pos;
			if (pos >= body_pos)
				separator_width += row_.separator;
			paintSeparator(orig_x, separator_width, orig_font.fontInfo());
			paintForeignMark(orig_x, orig_font.language());
			++vpos;

		} else if (inset) {
			// If outer row has changed, nested insets are repaint completely.
			pi_.base.bv->coordCache().insets().add(inset, int(x_), yo_);

			bool const pi_selected = pi_.selected;
			Cursor const & cur = pi_.base.bv->cursor();
			if (cur.selection() && cur.text() == &text_
				  && cur.normalAnchor().text() == &text_)
				pi_.selected = row_.sel_beg <= pos && row_.sel_end > pos;
			paintInset(inset, pos);
			pi_.selected = pi_selected;
			++vpos;

		} else {
			// paint as many characters as possible.
			paintFromPos(vpos, change_running.changed());
		}

		// Is the inline completion after character?
		if (!font.isRightToLeft() && vpos - 1 == inlineCompletionVPos)
			paintInlineCompletion(font);
	}

	// if we reach the end of a struck out range, paint it
	if (change_running.changed()) {
		FontMetrics const & fm
			= theFontMetrics(pi_.base.bv->buffer().params().getFont());
		float const y_bar = change_running.deleted() ?
				yo_ - fm.maxAscent() / 3 : yo_ + 2 * solid_line_offset_ + solid_line_thickness_;
		pi_.pain.line(change_last_x, int(y_bar), int(x_), int(y_bar),
			change_running.color(), Painter::line_solid, solid_line_thickness_);
		change_running.setUnchanged();
	}
}


void RowPainter::paintSelection()
{
	if (!row_.selection())
		return;
	Cursor const & curs = pi_.base.bv->cursor();
	DocIterator beg = curs.selectionBegin();
	beg.pit() = pit_;
	beg.pos() = row_.sel_beg;

	DocIterator end = curs.selectionEnd();
	end.pit() = pit_;
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
		swap(c1, c2);
	}

	if (!s1.empty()) {
		f.setColor(c1);
		pi_.pain.text(int(x_), yo_, s1, f);
		x_ += theFontMetrics(font).width(s1);
	}

	if (!s2.empty()) {
		f.setColor(c2);
		pi_.pain.text(int(x_), yo_, s2, f);
		x_ += theFontMetrics(font).width(s2);
	}
}

} // namespace lyx
