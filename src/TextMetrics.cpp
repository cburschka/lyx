/**
 * \file src/TextMetrics.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 * \author André Pönitz
 * \author Dekel Tsur
 * \author Jürgen Vigna
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "TextMetrics.h"

#include "Bidi.h"
#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "FuncRequest.h"
#include "InsetList.h"
#include "Layout.h"
#include "Length.h"
#include "LyXRC.h"
#include "MetricsInfo.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "rowpainter.h"
#include "Text.h"
#include "TextClass.h"
#include "VSpace.h"

#include "mathed/MacroTable.h"
#include "mathed/MathMacroTemplate.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include <cstdlib>
#include "support/lassert.h"

using namespace std;


namespace lyx {

using frontend::FontMetrics;

static int numberOfSeparators(Paragraph const & par, Row const & row)
{
	pos_type const first = max(row.pos(), par.beginOfBody());
	pos_type const last = row.endpos() - 1;
	int n = 0;
	for (pos_type p = first; p < last; ++p) {
		if (par.isSeparator(p))
			++n;
	}
	return n;
}


static int numberOfLabelHfills(Paragraph const & par, Row const & row)
{
	pos_type last = row.endpos() - 1;
	pos_type first = row.pos();

	// hfill *DO* count at the beginning of paragraphs!
	if (first) {
		while (first < last && par.isHfill(first))
			++first;
	}

	last = min(last, par.beginOfBody());
	int n = 0;
	for (pos_type p = first; p < last; ++p) {
		if (par.isHfill(p))
			++n;
	}
	return n;
}


static int numberOfHfills(Paragraph const & par, Row const & row)
{
	pos_type const last = row.endpos();
	pos_type first = row.pos();

	// hfill *DO* count at the beginning of paragraphs!
	if (first) {
		while (first < last && par.isHfill(first))
			++first;
	}

	first = max(first, par.beginOfBody());

	int n = 0;
	for (pos_type p = first; p < last; ++p) {
		if (par.isHfill(p))
			++n;
	}
	return n;
}


/////////////////////////////////////////////////////////////////////
//
// TextMetrics
//
/////////////////////////////////////////////////////////////////////


TextMetrics::TextMetrics(BufferView * bv, Text * text)
	: bv_(bv), text_(text)
{
	LASSERT(bv_, /**/);
	max_width_ = bv_->workWidth();
	dim_.wid = max_width_;
	dim_.asc = 10;
	dim_.des = 10;

	//text_->updateLabels(bv->buffer());
}


bool TextMetrics::contains(pit_type pit) const
{
	return par_metrics_.find(pit) != par_metrics_.end();
}


ParagraphMetrics const & TextMetrics::parMetrics(pit_type pit) const
{
	return const_cast<TextMetrics *>(this)->parMetrics(pit, true);
}



pair<pit_type, ParagraphMetrics const *> TextMetrics::first() const
{
	ParMetricsCache::const_iterator it = par_metrics_.begin();
	return make_pair(it->first, &it->second);
}


pair<pit_type, ParagraphMetrics const *> TextMetrics::last() const
{
	ParMetricsCache::const_reverse_iterator it = par_metrics_.rbegin();
	return make_pair(it->first, &it->second);
}


ParagraphMetrics & TextMetrics::parMetrics(pit_type pit, bool redo)
{
	ParMetricsCache::iterator pmc_it = par_metrics_.find(pit);
	if (pmc_it == par_metrics_.end()) {
		pmc_it = par_metrics_.insert(
			make_pair(pit, ParagraphMetrics(text_->getPar(pit)))).first;
	}
	if (pmc_it->second.rows().empty() && redo)
		redoParagraph(pit);
	return pmc_it->second;
}


int TextMetrics::parPosition(pit_type pit) const
{
	if (pit < par_metrics_.begin()->first)
		return -1000000;
	if (pit > par_metrics_.rbegin()->first)
		return +1000000;

	return par_metrics_[pit].position();
}


bool TextMetrics::metrics(MetricsInfo & mi, Dimension & dim, int min_width)
{
	LASSERT(mi.base.textwidth, /**/);
	max_width_ = mi.base.textwidth;
	// backup old dimension.
	Dimension const old_dim = dim_;
	// reset dimension.
	dim_ = Dimension();
	dim_.wid = min_width;
	pit_type const npar = text_->paragraphs().size();
	if (npar > 1)
		// If there is more than one row, expand the text to
		// the full allowable width.
		dim_.wid = max_width_;

	//lyxerr << "TextMetrics::metrics: width: " << mi.base.textwidth
	//	<< " maxWidth: " << max_width_ << "\nfont: " << mi.base.font << endl;

	bool changed = false;
	unsigned int h = 0;
	for (pit_type pit = 0; pit != npar; ++pit) {
		changed |= redoParagraph(pit);
		ParagraphMetrics const & pm = par_metrics_[pit];
		h += pm.height();
		if (dim_.wid < pm.width())
			dim_.wid = pm.width();
	}

	dim_.asc = par_metrics_[0].ascent();
	dim_.des = h - dim_.asc;
	//lyxerr << "dim_.wid " << dim_.wid << endl;
	//lyxerr << "dim_.asc " << dim_.asc << endl;
	//lyxerr << "dim_.des " << dim_.des << endl;

	changed |= dim_ != old_dim;
	dim = dim_;
	return changed;
}


int TextMetrics::rightMargin(ParagraphMetrics const & pm) const
{
	return main_text_? pm.rightMargin(*bv_) : 0;
}


int TextMetrics::rightMargin(pit_type const pit) const
{
	return main_text_? par_metrics_[pit].rightMargin(*bv_) : 0;
}


void TextMetrics::applyOuterFont(Font & font) const
{
	Font lf(font_);
	lf.fontInfo().reduce(bv_->buffer().params().getFont().fontInfo());
	lf.fontInfo().realize(font.fontInfo());
	lf.setLanguage(font.language());
	font = lf;
}


Font TextMetrics::displayFont(pit_type pit, pos_type pos) const
{
	LASSERT(pos >= 0, /**/);

	ParagraphList const & pars = text_->paragraphs();
	Paragraph const & par = pars[pit];
	Layout const & layout = par.layout();
	Buffer const & buffer = bv_->buffer();
	// FIXME: broken?
	BufferParams const & params = buffer.params();
	pos_type const body_pos = par.beginOfBody();

	// We specialize the 95% common case:
	if (!par.getDepth()) {
		Font f = par.getFontSettings(params, pos);
		if (!text_->isMainText(buffer))
			applyOuterFont(f);
		bool lab = layout.labeltype == LABEL_MANUAL && pos < body_pos;

		FontInfo const & lf = lab ? layout.labelfont : layout.font;
		FontInfo rlf = lab ? layout.reslabelfont : layout.resfont;

		// In case the default family has been customized
		if (lf.family() == INHERIT_FAMILY)
			rlf.setFamily(params.getFont().fontInfo().family());
		f.fontInfo().realize(rlf);
		return f;
	}

	// The uncommon case need not be optimized as much
	FontInfo const & layoutfont = pos < body_pos ?
		layout.labelfont : layout.font;

	Font font = par.getFontSettings(params, pos);
	font.fontInfo().realize(layoutfont);

	if (!text_->isMainText(buffer))
		applyOuterFont(font);

	// Realize against environment font information
	// NOTE: the cast to pit_type should be removed when pit_type
	// changes to a unsigned integer.
	if (pit < pit_type(pars.size()))
		font.fontInfo().realize(outerFont(pit, pars).fontInfo());

	// Realize with the fonts of lesser depth.
	font.fontInfo().realize(params.getFont().fontInfo());

	return font;
}


bool TextMetrics::isRTL(CursorSlice const & sl, bool boundary) const
{
	if (!lyxrc.rtl_support || !sl.text())
		return false;

	int correction = 0;
	if (boundary && sl.pos() > 0)
		correction = -1;

	return displayFont(sl.pit(), sl.pos() + correction).isVisibleRightToLeft();
}


bool TextMetrics::isRTLBoundary(pit_type pit, pos_type pos) const
{
	if (!lyxrc.rtl_support)
		return false;

	// no RTL boundary at line start
	if (pos == 0)
		return false;

	Paragraph const & par = text_->getPar(pit);

	bool left = displayFont(pit, pos - 1).isVisibleRightToLeft();
	bool right;
	if (pos == par.size())
		right = par.isRTL(bv_->buffer().params());
	else
		right = displayFont(pit, pos).isVisibleRightToLeft();
	return left != right;
}


bool TextMetrics::isRTLBoundary(pit_type pit, pos_type pos,
		Font const & font) const
{
	if (!lyxrc.rtl_support)
		return false;

	Paragraph const & par = text_->getPar(pit);
	bool left = font.isVisibleRightToLeft();
	bool right;
	if (pos == par.size())
		right = par.isRTL(bv_->buffer().params());
	else
		right = displayFont(pit, pos).isVisibleRightToLeft();
	return left != right;
}


bool TextMetrics::redoParagraph(pit_type const pit)
{
	Paragraph & par = text_->getPar(pit);
	// IMPORTANT NOTE: We pass 'false' explicitely in order to not call
	// redoParagraph() recursively inside parMetrics.
	Dimension old_dim = parMetrics(pit, false).dim();
	ParagraphMetrics & pm = par_metrics_[pit];
	pm.reset(par);

	Buffer & buffer = bv_->buffer();
	main_text_ = (text_ == &buffer.text());
	bool changed = false;

	// FIXME: This check ought to be done somewhere else. It is the reason
	// why text_ is not	const. But then, where else to do it?
	// Well, how can you end up with either (a) a biblio environment that
	// has no InsetBibitem or (b) a biblio environment with more than one
	// InsetBibitem? I think the answer is: when paragraphs are merged;
	// when layout is set; when material is pasted.
	int const moveCursor = par.checkBiblio(buffer);
	if (moveCursor > 0)
		const_cast<Cursor &>(bv_->cursor()).posForward();
	else if (moveCursor < 0) {
		Cursor & cursor = const_cast<Cursor &>(bv_->cursor());
		if (cursor.pos() >= -moveCursor)
			cursor.posBackward();
	}

	// Optimisation: this is used in the next two loops
	// so better to calculate that once here.
	int const right_margin = rightMargin(pm);

	// iterator pointing to paragraph to resolve macros
	DocIterator parPos = text_->macrocontextPosition();
	if (!parPos.empty())
		parPos.pit() = pit;
	else {
		LYXERR(Debug::INFO, "MacroContext not initialised!"
			<< " Going through the buffer again and hope"
			<< " the context is better then.");
		updateLabels(bv_->buffer());
		parPos = text_->macrocontextPosition();
		LASSERT(!parPos.empty(), /**/);
		parPos.pit() = pit;
	}

	// redo insets
	// FIXME: We should always use getFont(), see documentation of
	// noFontChange() in Inset.h.
	Font const bufferfont = buffer.params().getFont();
	InsetList::const_iterator ii = par.insetList().begin();
	InsetList::const_iterator iend = par.insetList().end();
	for (; ii != iend; ++ii) {
		// position already initialized?
		if (!parPos.empty()) {
			parPos.pos() = ii->pos;

			// A macro template would normally not be visible
			// by itself. But the tex macro semantics allow
			// recursion, so we artifically take the context
			// after the macro template to simulate this.
			if (ii->inset->lyxCode() == MATHMACRO_CODE)
				parPos.pos()++;
		}

		// do the metric calculation
		Dimension dim;
		int const w = max_width_ - leftMargin(max_width_, pit, ii->pos)
			- right_margin;
		Font const & font = ii->inset->noFontChange() ?
			bufferfont : displayFont(pit, ii->pos);
		MacroContext mc(buffer, parPos);
		MetricsInfo mi(bv_, font.fontInfo(), w, mc);
		ii->inset->metrics(mi, dim);
		Dimension const old_dim = pm.insetDimension(ii->inset);
		pm.setInsetDimension(ii->inset, dim);
		changed |= (old_dim != dim);
	}

	par.setBeginOfBody();
	pos_type first = 0;
	size_t row_index = 0;
	// maximum pixel width of a row
	int width = max_width_ - right_margin; // - leftMargin(max_width_, pit, row);
	do {
		Dimension dim;
		pos_type end = rowBreakPoint(width, pit, first);
		if (row_index || end < par.size())
			// If there is more than one row, expand the text to
			// the full allowable width. This setting here is needed
			// for the computeRowMetrics() below.
			dim_.wid = max_width_;

		dim = rowHeight(pit, first, end);
		dim.wid = rowWidth(right_margin, pit, first, end);
		if (row_index == pm.rows().size())
			pm.rows().push_back(Row());
		Row & row = pm.rows()[row_index];
		row.setChanged(false);
		row.pos(first);
		row.endpos(end);
		row.setDimension(dim);
		int const max_row_width = max(dim_.wid, dim.wid);
		computeRowMetrics(pit, row, max_row_width);
		first = end;
		++row_index;

		pm.dim().wid = max(pm.dim().wid, dim.wid);
		pm.dim().des += dim.height();
	} while (first < par.size());

	if (row_index < pm.rows().size())
		pm.rows().resize(row_index);

	// Make sure that if a par ends in newline, there is one more row
	// under it
	if (first > 0 && par.isNewline(first - 1)) {
		Dimension dim = rowHeight(pit, first, first);
		dim.wid = rowWidth(right_margin, pit, first, first);
		if (row_index == pm.rows().size())
			pm.rows().push_back(Row());
		Row & row = pm.rows()[row_index];
		row.setChanged(false);
		row.pos(first);
		row.endpos(first);
		row.setDimension(dim);
		int const max_row_width = max(dim_.wid, dim.wid);
		computeRowMetrics(pit, row, max_row_width);
		pm.dim().des += dim.height();
	}

	pm.dim().asc += pm.rows()[0].ascent();
	pm.dim().des -= pm.rows()[0].ascent();

	changed |= old_dim.height() != pm.dim().height();

	return changed;
}


void TextMetrics::computeRowMetrics(pit_type const pit,
		Row & row, int width) const
{
	row.label_hfill = 0;
	row.separator = 0;

	Buffer & buffer = bv_->buffer();
	Paragraph const & par = text_->getPar(pit);

	double w = width - row.width();
	// FIXME: put back this assertion when the crash on new doc is solved.
	//LASSERT(w >= 0, /**/);

	//lyxerr << "\ndim_.wid " << dim_.wid << endl;
	//lyxerr << "row.width() " << row.width() << endl;
	//lyxerr << "w " << w << endl;

	bool const is_rtl = text_->isRTL(buffer, par);
	if (is_rtl)
		row.x = rightMargin(pit);
	else
		row.x = leftMargin(max_width_, pit, row.pos());

	// is there a manual margin with a manual label
	Layout const & layout = par.layout();

	if (layout.margintype == MARGIN_MANUAL
	    && layout.labeltype == LABEL_MANUAL) {
		/// We might have real hfills in the label part
		int nlh = numberOfLabelHfills(par, row);

		// A manual label par (e.g. List) has an auto-hfill
		// between the label text and the body of the
		// paragraph too.
		// But we don't want to do this auto hfill if the par
		// is empty.
		if (!par.empty())
			++nlh;

		if (nlh && !par.getLabelWidthString().empty())
			row.label_hfill = labelFill(pit, row) / double(nlh);
	}

	double hfill = 0;
	// are there any hfills in the row?
	if (int const nh = numberOfHfills(par, row)) {
		if (w > 0)
			hfill = w / double(nh);
	// we don't have to look at the alignment if it is ALIGN_LEFT and
	// if the row is already larger then the permitted width as then
	// we force the LEFT_ALIGN'edness!
	} else if (int(row.width()) < max_width_) {
		// is it block, flushleft or flushright?
		// set x how you need it
		int align;
		if (par.params().align() == LYX_ALIGN_LAYOUT)
			align = layout.align;
		else
			align = par.params().align();

		// Display-style insets should always be on a centred row
		if (Inset const * inset = par.getInset(row.pos())) {
			switch (inset->display()) {
				case Inset::AlignLeft:
					align = LYX_ALIGN_BLOCK;
					break;
				case Inset::AlignCenter:
					align = LYX_ALIGN_CENTER;
					break;
				case Inset::Inline:
				case Inset::AlignRight:
					// unchanged (use align)
					break;
			}
		}

		switch (align) {
		case LYX_ALIGN_BLOCK: {
			int const ns = numberOfSeparators(par, row);
			bool disp_inset = false;
			if (row.endpos() < par.size()) {
				Inset const * in = par.getInset(row.endpos());
				if (in)
					disp_inset = in->display();
			}
			// If we have separators, this is not the last row of a
			// par, does not end in newline, and is not row above a
			// display inset... then stretch it
			if (ns
			    && row.endpos() < par.size()
			    && !par.isNewline(row.endpos() - 1)
			    && !disp_inset
				) {
				row.separator = w / ns;
				//lyxerr << "row.separator " << row.separator << endl;
				//lyxerr << "ns " << ns << endl;
			} else if (is_rtl) {
				row.x += w;
			}
			break;
		}
		case LYX_ALIGN_RIGHT:
			row.x += w;
			break;
		case LYX_ALIGN_CENTER:
			row.x += w / 2;
			break;
		}
	}

	if (is_rtl) {
		pos_type body_pos = par.beginOfBody();
		pos_type end = row.endpos();

		if (body_pos > 0
		    && (body_pos > end || !par.isLineSeparator(body_pos - 1)))
		{
			row.x += theFontMetrics(text_->labelFont(buffer, par)).
				width(layout.labelsep);
			if (body_pos <= end)
				row.x += row.label_hfill;
		}
	}

	pos_type const endpos = row.endpos();
	pos_type body_pos = par.beginOfBody();
	if (body_pos > 0
		&& (body_pos > endpos || !par.isLineSeparator(body_pos - 1)))
		body_pos = 0;

	ParagraphMetrics & pm = par_metrics_[pit];
	InsetList::const_iterator ii = par.insetList().begin();
	InsetList::const_iterator iend = par.insetList().end();
	for ( ; ii != iend; ++ii) {
		if (ii->pos >= endpos || ii->pos < row.pos()
			|| (ii->inset->lyxCode() != SPACE_CODE ||
			    !ii->inset->isStretchableSpace()))
			continue;
		Dimension dim = row.dimension();
		if (pm.hfillExpansion(row, ii->pos))
			dim.wid = int(ii->pos >= body_pos ?
				max(hfill, 5.0) : row.label_hfill);
		else
			dim.wid = 5;
		// Cache the inset dimension.
		bv_->coordCache().insets().add(ii->inset, dim);
		pm.setInsetDimension(ii->inset, dim);
	}
}


int TextMetrics::labelFill(pit_type const pit, Row const & row) const
{
	Buffer & buffer = bv_->buffer();
	Paragraph const & par = text_->getPar(pit);

	pos_type last = par.beginOfBody();
	LASSERT(last > 0, /**/);

	// -1 because a label ends with a space that is in the label
	--last;

	// a separator at this end does not count
	if (par.isLineSeparator(last))
		--last;

	int w = 0;
	for (pos_type i = row.pos(); i <= last; ++i)
		w += singleWidth(pit, i);

	docstring const & label = par.params().labelWidthString();
	if (label.empty())
		return 0;

	FontMetrics const & fm
		= theFontMetrics(text_->labelFont(buffer, par));

	return max(0, fm.width(label) - w);
}


// this needs special handling - only newlines count as a break point
static pos_type addressBreakPoint(pos_type i, Paragraph const & par)
{
	pos_type const end = par.size();

	for (; i < end; ++i)
		if (par.isNewline(i))
			return i + 1;

	return end;
}


int TextMetrics::labelEnd(pit_type const pit) const
{
	// labelEnd is only needed if the layout fills a flushleft label.
	if (text_->getPar(pit).layout().margintype != MARGIN_MANUAL)
		return 0;
	// return the beginning of the body
	return leftMargin(max_width_, pit);
}

namespace {

/**
 * Calling Text::getFont is slow. While rebreaking we scan a
 * paragraph from left to right calling getFont for every char.  This
 * simple class address this problem by hidding an optimization trick
 * (not mine btw -AB): the font is reused in the whole font span.  The
 * class handles transparently the "hidden" (not part of the fontlist)
 * label font (as getFont does).
 **/
class FontIterator
{
public:
	///
	FontIterator(TextMetrics const & tm,
		Paragraph const & par, pit_type pit, pos_type pos)
		: tm_(tm), par_(par), pit_(pit), pos_(pos),
		font_(tm.displayFont(pit, pos)),
		endspan_(par.fontSpan(pos).last),
		bodypos_(par.beginOfBody())
	{}

	///
	Font const & operator*() const { return font_; }

	///
	FontIterator & operator++()
	{
		++pos_;
		if (pos_ > endspan_ || pos_ == bodypos_) {
			font_ = tm_.displayFont(pit_, pos_);
			endspan_ = par_.fontSpan(pos_).last;
		}
		return *this;
	}

	///
	Font * operator->() { return &font_; }

private:
	///
	TextMetrics const & tm_;
	///
	Paragraph const & par_;
	///
	pit_type pit_;
	///
	pos_type pos_;
	///
	Font font_;
	///
	pos_type endspan_;
	///
	pos_type bodypos_;
};

} // anon namespace

pit_type TextMetrics::rowBreakPoint(int width, pit_type const pit,
		pit_type pos) const
{
	Buffer & buffer = bv_->buffer();
	ParagraphMetrics const & pm = par_metrics_[pit];
	Paragraph const & par = text_->getPar(pit);
	pos_type const end = par.size();
	if (pos == end || width < 0)
		return end;

	Layout const & layout = par.layout();

	if (layout.margintype == MARGIN_RIGHT_ADDRESS_BOX)
		return addressBreakPoint(pos, par);

	pos_type const body_pos = par.beginOfBody();

	// check for possible inline completion
	DocIterator const & inlineCompletionPos = bv_->inlineCompletionPos();
	pos_type inlineCompletionLPos = -1;
	if (inlineCompletionPos.inTexted()
	    && inlineCompletionPos.text() == text_
	    && inlineCompletionPos.pit() == pit) {
		// draw logically behind the previous character
		inlineCompletionLPos = inlineCompletionPos.pos() - 1;
	}

	// Now we iterate through until we reach the right margin
	// or the end of the par, then choose the possible break
	// nearest that.

	int label_end = labelEnd(pit);
	int const left = leftMargin(max_width_, pit, pos);
	int x = left;

	// pixel width since last breakpoint
	int chunkwidth = 0;

	FontIterator fi = FontIterator(*this, par, pit, pos);
	pos_type point = end;
	pos_type i = pos;
	for ( ; i < end; ++i, ++fi) {
		int thiswidth = pm.singleWidth(i, *fi);

		// add inline completion width
		if (inlineCompletionLPos == i) {
			docstring const & completion = bv_->inlineCompletion();
			if (completion.length() > 0)
				thiswidth += theFontMetrics(*fi).width(completion);
		}

		// add the auto-hfill from label end to the body
		if (body_pos && i == body_pos) {
			FontMetrics const & fm = theFontMetrics(
				text_->labelFont(buffer, par));
			int add = fm.width(layout.labelsep);
			if (par.isLineSeparator(i - 1))
				add -= singleWidth(pit, i - 1);

			add = max(add, label_end - x);
			thiswidth += add;
		}

		x += thiswidth;
		chunkwidth += thiswidth;

		// break before a character that will fall off
		// the right of the row
		if (x >= width) {
			// if no break before, break here
			if (point == end || chunkwidth >= width - left) {
				if (i > pos)
					point = i;
				else
					point = i + 1;
			}
			// exit on last registered breakpoint:
			break;
		}

		if (par.isNewline(i)) {
			point = i + 1;
			break;
		}
		Inset const * inset = 0;
		// Break before...
		if (i + 1 < end) {
			if ((inset = par.getInset(i + 1)) && inset->display()) {
				point = i + 1;
				break;
			}
			// ...and after.
			if ((inset = par.getInset(i)) && inset->display()) {
				point = i + 1;
				break;
			}
		}

		inset = par.getInset(i);
		if (!inset || inset->isChar()) {
			// some insets are line separators too
			if (par.isLineSeparator(i)) {
				// register breakpoint:
				point = i + 1;
				chunkwidth = 0;
			}
		}
	}

	// maybe found one, but the par is short enough.
	if (i == end && x < width)
		point = end;

	// manual labels cannot be broken in LaTeX. But we
	// want to make our on-screen rendering of footnotes
	// etc. still break
	if (body_pos && point < body_pos)
		point = body_pos;

	return point;
}


int TextMetrics::rowWidth(int right_margin, pit_type const pit,
		pos_type const first, pos_type const end) const
{
	Buffer & buffer = bv_->buffer();
	// get the pure distance
	ParagraphMetrics const & pm = par_metrics_[pit];
	Paragraph const & par = text_->getPar(pit);
	int w = leftMargin(max_width_, pit, first);
	int label_end = labelEnd(pit);

	// check for possible inline completion
	DocIterator const & inlineCompletionPos = bv_->inlineCompletionPos();
	pos_type inlineCompletionLPos = -1;
	if (inlineCompletionPos.inTexted()
	    && inlineCompletionPos.text() == text_
	    && inlineCompletionPos.pit() == pit) {
		// draw logically behind the previous character
		inlineCompletionLPos = inlineCompletionPos.pos() - 1;
	}

	pos_type const body_pos = par.beginOfBody();
	pos_type i = first;

	if (i < end) {
		FontIterator fi = FontIterator(*this, par, pit, i);
		for ( ; i < end; ++i, ++fi) {
			if (body_pos > 0 && i == body_pos) {
				FontMetrics const & fm = theFontMetrics(
					text_->labelFont(buffer, par));
				w += fm.width(par.layout().labelsep);
				if (par.isLineSeparator(i - 1))
					w -= singleWidth(pit, i - 1);
				w = max(w, label_end);
			}
			w += pm.singleWidth(i, *fi);

			// add inline completion width
			if (inlineCompletionLPos == i) {
				docstring const & completion = bv_->inlineCompletion();
				if (completion.length() > 0)
					w += theFontMetrics(*fi).width(completion);
			}
		}
	}

	if (body_pos > 0 && body_pos >= end) {
		FontMetrics const & fm = theFontMetrics(
			text_->labelFont(buffer, par));
		w += fm.width(par.layout().labelsep);
		if (end > 0 && par.isLineSeparator(end - 1))
			w -= singleWidth(pit, end - 1);
		w = max(w, label_end);
	}

	return w + right_margin;
}


Dimension TextMetrics::rowHeight(pit_type const pit, pos_type const first,
		pos_type const end, bool topBottomSpace) const
{
	Paragraph const & par = text_->getPar(pit);
	// get the maximum ascent and the maximum descent
	double layoutasc = 0;
	double layoutdesc = 0;
	double const dh = defaultRowHeight();

	// ok, let us initialize the maxasc and maxdesc value.
	// Only the fontsize count. The other properties
	// are taken from the layoutfont. Nicer on the screen :)
	Layout const & layout = par.layout();

	// as max get the first character of this row then it can
	// increase but not decrease the height. Just some point to
	// start with so we don't have to do the assignment below too
	// often.
	Buffer const & buffer = bv_->buffer();
	Font font = displayFont(pit, first);
	FontSize const tmpsize = font.fontInfo().size();
	font.fontInfo() = text_->layoutFont(buffer, pit);
	FontSize const size = font.fontInfo().size();
	font.fontInfo().setSize(tmpsize);

	FontInfo labelfont = text_->labelFont(buffer, par);

	FontMetrics const & labelfont_metrics = theFontMetrics(labelfont);
	FontMetrics const & fontmetrics = theFontMetrics(font);

	// these are minimum values
	double const spacing_val = layout.spacing.getValue()
		* text_->spacing(buffer, par);
	//lyxerr << "spacing_val = " << spacing_val << endl;
	int maxasc  = int(fontmetrics.maxAscent()  * spacing_val);
	int maxdesc = int(fontmetrics.maxDescent() * spacing_val);

	// insets may be taller
	ParagraphMetrics const & pm = par_metrics_[pit];
	InsetList::const_iterator ii = par.insetList().begin();
	InsetList::const_iterator iend = par.insetList().end();
	for ( ; ii != iend; ++ii) {
		Dimension const & dim = pm.insetDimension(ii->inset);
		if (ii->pos >= first && ii->pos < end) {
			maxasc  = max(maxasc,  dim.ascent());
			maxdesc = max(maxdesc, dim.descent());
		}
	}

	// Check if any custom fonts are larger (Asger)
	// This is not completely correct, but we can live with the small,
	// cosmetic error for now.
	int labeladdon = 0;

	FontSize maxsize =
		par.highestFontInRange(first, end, size);
	if (maxsize > font.fontInfo().size()) {
		// use standard paragraph font with the maximal size
		FontInfo maxfont = font.fontInfo();
		maxfont.setSize(maxsize);
		FontMetrics const & maxfontmetrics = theFontMetrics(maxfont);
		maxasc  = max(maxasc,  maxfontmetrics.maxAscent());
		maxdesc = max(maxdesc, maxfontmetrics.maxDescent());
	}

	// This is nicer with box insets:
	++maxasc;
	++maxdesc;

	ParagraphList const & pars = text_->paragraphs();

	// is it a top line?
	if (first == 0 && topBottomSpace) {
		BufferParams const & bufparams = buffer.params();
		// some parskips VERY EASY IMPLEMENTATION
		if (bufparams.paragraph_separation
		    == BufferParams::ParagraphSkipSeparation
			&& par.ownerCode() != ERT_CODE
			&& par.ownerCode() != LISTINGS_CODE
			&& pit > 0
			&& ((layout.isParagraph() && par.getDepth() == 0)
			    || (pars[pit - 1].layout().isParagraph()
				&& pars[pit - 1].getDepth() == 0)))
		{
				maxasc += bufparams.getDefSkip().inPixels(*bv_);
		}

		if (par.params().startOfAppendix())
			maxasc += int(3 * dh);

		// This is special code for the chapter, since the label of this
		// layout is printed in an extra row
		if (layout.counter == "chapter"
		    && !par.params().labelString().empty()) {
			labeladdon = int(labelfont_metrics.maxHeight()
				     * layout.spacing.getValue()
				     * text_->spacing(buffer, par));
		}

		// special code for the top label
		if ((layout.labeltype == LABEL_TOP_ENVIRONMENT
		     || layout.labeltype == LABEL_BIBLIO
		     || layout.labeltype == LABEL_CENTERED_TOP_ENVIRONMENT)
		    && isFirstInSequence(pit, pars)
		    && !par.labelString().empty())
		{
			labeladdon = int(
				  labelfont_metrics.maxHeight()
					* layout.spacing.getValue()
					* text_->spacing(buffer, par)
				+ (layout.topsep + layout.labelbottomsep) * dh);
		}

		// Add the layout spaces, for example before and after
		// a section, or between the items of a itemize or enumerate
		// environment.

		pit_type prev = depthHook(pit, pars, par.getDepth());
		Paragraph const & prevpar = pars[prev];
		if (prev != pit
		    && prevpar.layout() == layout
		    && prevpar.getDepth() == par.getDepth()
		    && prevpar.getLabelWidthString()
					== par.getLabelWidthString()) {
			layoutasc = layout.itemsep * dh;
		} else if (pit != 0 || first != 0) {
			if (layout.topsep > 0)
				layoutasc = layout.topsep * dh;
		}

		prev = outerHook(pit, pars);
		if (prev != pit_type(pars.size())) {
			maxasc += int(pars[prev].layout().parsep * dh);
		} else if (pit != 0) {
			Paragraph const & prevpar = pars[pit - 1];
			if (prevpar.getDepth() != 0 ||
					prevpar.layout() == layout) {
				maxasc += int(layout.parsep * dh);
			}
		}
	}

	// is it a bottom line?
	if (end >= par.size() && topBottomSpace) {
		// add the layout spaces, for example before and after
		// a section, or between the items of a itemize or enumerate
		// environment
		pit_type nextpit = pit + 1;
		if (nextpit != pit_type(pars.size())) {
			pit_type cpit = pit;
			double usual = 0;
			double unusual = 0;

			if (pars[cpit].getDepth() > pars[nextpit].getDepth()) {
				usual = pars[cpit].layout().bottomsep * dh;
				cpit = depthHook(cpit, pars, pars[nextpit].getDepth());
				if (pars[cpit].layout() != pars[nextpit].layout()
					|| pars[nextpit].getLabelWidthString() != pars[cpit].getLabelWidthString())
				{
					unusual = pars[cpit].layout().bottomsep * dh;
				}
				layoutdesc = max(unusual, usual);
			} else if (pars[cpit].getDepth() == pars[nextpit].getDepth()) {
				if (pars[cpit].layout() != pars[nextpit].layout()
					|| pars[nextpit].getLabelWidthString() != pars[cpit].getLabelWidthString())
					layoutdesc = int(pars[cpit].layout().bottomsep * dh);
			}
		}
	}

	// incalculate the layout spaces
	maxasc  += int(layoutasc  * 2 / (2 + pars[pit].getDepth()));
	maxdesc += int(layoutdesc * 2 / (2 + pars[pit].getDepth()));

	// FIXME: the correct way is to do the following is to move the
	// following code in another method specially tailored for the
	// main Text. The following test is thus bogus.
	// Top and bottom margin of the document (only at top-level)
	if (main_text_ && topBottomSpace) {
		if (pit == 0 && first == 0)
			maxasc += 20;
		if (pit + 1 == pit_type(pars.size()) &&
		    end == par.size() &&
				!(end > 0 && par.isNewline(end - 1)))
			maxdesc += 20;
	}

	return Dimension(0, maxasc + labeladdon, maxdesc);
}


// x is an absolute screen coord
// returns the column near the specified x-coordinate of the row
// x is set to the real beginning of this column
pos_type TextMetrics::getColumnNearX(pit_type const pit,
		Row const & row, int & x, bool & boundary) const
{
	Buffer const & buffer = bv_->buffer();

	/// For the main Text, it is possible that this pit is not
	/// yet in the CoordCache when moving cursor up.
	/// x Paragraph coordinate is always 0 for main text anyway.
	int const xo = origin_.x_;
	x -= xo;
	Paragraph const & par = text_->getPar(pit);
	Bidi bidi;
	bidi.computeTables(par, buffer, row);

	pos_type vc = row.pos();
	pos_type end = row.endpos();
	pos_type c = 0;
	Layout const & layout = par.layout();

	bool left_side = false;

	pos_type body_pos = par.beginOfBody();

	double tmpx = row.x;
	double last_tmpx = tmpx;

	if (body_pos > 0 &&
	    (body_pos > end || !par.isLineSeparator(body_pos - 1)))
		body_pos = 0;

	// check for empty row
	if (vc == end) {
		x = int(tmpx) + xo;
		return 0;
	}

	while (vc < end && tmpx <= x) {
		c = bidi.vis2log(vc);
		last_tmpx = tmpx;
		if (body_pos > 0 && c == body_pos - 1) {
			FontMetrics const & fm = theFontMetrics(
				text_->labelFont(buffer, par));
			tmpx += row.label_hfill + fm.width(layout.labelsep);
			if (par.isLineSeparator(body_pos - 1))
				tmpx -= singleWidth(pit, body_pos - 1);
		}

		tmpx += singleWidth(pit, c);
		if (par.isSeparator(c) && c >= body_pos)
				tmpx += row.separator;
		++vc;
	}

	if ((tmpx + last_tmpx) / 2 > x) {
		tmpx = last_tmpx;
		left_side = true;
	}

	LASSERT(vc <= end, /**/);  // This shouldn't happen.

	boundary = false;
	// This (rtl_support test) is not needed, but gives
	// some speedup if rtl_support == false
	bool const lastrow = lyxrc.rtl_support && row.endpos() == par.size();

	// If lastrow is false, we don't need to compute
	// the value of rtl.
	bool const rtl = lastrow ? text_->isRTL(buffer, par) : false;
	if (lastrow &&
	    ((rtl  &&  left_side && vc == row.pos() && x < tmpx - 5) ||
	     (!rtl && !left_side && vc == end  && x > tmpx + 5))) {
		if (!par.isNewline(end - 1))
			c = end;
	} else if (vc == row.pos()) {
		c = bidi.vis2log(vc);
		if (bidi.level(c) % 2 == 1)
			++c;
	} else {
		c = bidi.vis2log(vc - 1);
		bool const rtl = (bidi.level(c) % 2 == 1);
		if (left_side == rtl) {
			++c;
			boundary = isRTLBoundary(pit, c);
		}
	}

// I believe this code is not needed anymore (Jug 20050717)
#if 0
	// The following code is necessary because the cursor position past
	// the last char in a row is logically equivalent to that before
	// the first char in the next row. That's why insets causing row
	// divisions -- Newline and display-style insets -- must be treated
	// specially, so cursor up/down doesn't get stuck in an air gap -- MV
	// Newline inset, air gap below:
	if (row.pos() < end && c >= end && par.isNewline(end - 1)) {
		if (bidi.level(end -1) % 2 == 0)
			tmpx -= singleWidth(pit, end - 1);
		else
			tmpx += singleWidth(pit, end - 1);
		c = end - 1;
	}

	// Air gap above display inset:
	if (row.pos() < end && c >= end && end < par.size()
	    && par.isInset(end) && par.getInset(end)->display()) {
		c = end - 1;
	}
	// Air gap below display inset:
	if (row.pos() < end && c >= end && par.isInset(end - 1)
	    && par.getInset(end - 1)->display()) {
		c = end - 1;
	}
#endif

	x = int(tmpx) + xo;
	pos_type const col = c - row.pos();

	if (!c || end == par.size())
		return col;

	if (c==end && !par.isLineSeparator(c-1) && !par.isNewline(c-1)) {
		boundary = true;
		return col;
	}

	return min(col, end - 1 - row.pos());
}


pos_type TextMetrics::x2pos(pit_type pit, int row, int x) const
{
	// We play safe and use parMetrics(pit) to make sure the
	// ParagraphMetrics will be redone and OK to use if needed.
	// Otherwise we would use an empty ParagraphMetrics in
	// upDownInText() while in selection mode.
	ParagraphMetrics const & pm = parMetrics(pit);

	LASSERT(row < int(pm.rows().size()), /**/);
	bool bound = false;
	Row const & r = pm.rows()[row];
	return r.pos() + getColumnNearX(pit, r, x, bound);
}


void TextMetrics::newParMetricsDown()
{
	pair<pit_type, ParagraphMetrics> const & last = *par_metrics_.rbegin();
	pit_type const pit = last.first + 1;
	if (pit == int(text_->paragraphs().size()))
		return;

	// do it and update its position.
	redoParagraph(pit);
	par_metrics_[pit].setPosition(last.second.position()
		+ last.second.descent() + par_metrics_[pit].ascent());
}


void TextMetrics::newParMetricsUp()
{
	pair<pit_type, ParagraphMetrics> const & first = *par_metrics_.begin();
	if (first.first == 0)
		return;

	pit_type const pit = first.first - 1;
	// do it and update its position.
	redoParagraph(pit);
	par_metrics_[pit].setPosition(first.second.position()
		- first.second.ascent() - par_metrics_[pit].descent());
}

// y is screen coordinate
pit_type TextMetrics::getPitNearY(int y)
{
	LASSERT(!text_->paragraphs().empty(), /**/);
	LYXERR(Debug::DEBUG, "y: " << y << " cache size: " << par_metrics_.size());

	// look for highest numbered paragraph with y coordinate less than given y
	pit_type pit = -1;
	int yy = -1;
	ParMetricsCache::const_iterator it = par_metrics_.begin();
	ParMetricsCache::const_iterator et = par_metrics_.end();
	ParMetricsCache::const_iterator last = et; last--;

	ParagraphMetrics const & pm = it->second;

	if (y < it->second.position() - int(pm.ascent())) {
		// We are looking for a position that is before the first paragraph in
		// the cache (which is in priciple off-screen, that is before the
		// visible part.
		if (it->first == 0)
			// We are already at the first paragraph in the inset.
			return 0;
		// OK, this is the paragraph we are looking for.
		pit = it->first - 1;
		newParMetricsUp();
		return pit;
	}

	ParagraphMetrics const & pm_last = par_metrics_[last->first];

	if (y >= last->second.position() + int(pm_last.descent())) {
		// We are looking for a position that is after the last paragraph in
		// the cache (which is in priciple off-screen, that is before the
		// visible part.
		pit = last->first + 1;
		if (pit == int(text_->paragraphs().size()))
			//  We are already at the last paragraph in the inset.
			return last->first;
		// OK, this is the paragraph we are looking for.
		newParMetricsDown();
		return pit;
	}

	for (; it != et; ++it) {
		LYXERR(Debug::DEBUG, "examining: pit: " << it->first
			<< " y: " << it->second.position());

		ParagraphMetrics const & pm = par_metrics_[it->first];

		if (it->first >= pit && int(it->second.position()) - int(pm.ascent()) <= y) {
			pit = it->first;
			yy = it->second.position();
		}
	}

	LYXERR(Debug::DEBUG, "found best y: " << yy << " for pit: " << pit);

	return pit;
}


Row const & TextMetrics::getRowNearY(int y, pit_type pit) const
{
	ParagraphMetrics const & pm = par_metrics_[pit];

	int yy = pm.position() - pm.ascent();
	LASSERT(!pm.rows().empty(), /**/);
	RowList::const_iterator rit = pm.rows().begin();
	RowList::const_iterator rlast = pm.rows().end();
	--rlast;
	for (; rit != rlast; yy += rit->height(), ++rit)
		if (yy + rit->height() > y)
			break;
	return *rit;
}


// x,y are absolute screen coordinates
// sets cursor recursively descending into nested editable insets
Inset * TextMetrics::editXY(Cursor & cur, int x, int y)
{
	if (lyxerr.debugging(Debug::WORKAREA)) {
		LYXERR0("TextMetrics::editXY(cur, " << x << ", " << y << ")");
		cur.bv().coordCache().dump();
	}
	pit_type pit = getPitNearY(y);
	LASSERT(pit != -1, /**/);

	Row const & row = getRowNearY(y, pit);
	bool bound = false;

	int xx = x; // is modified by getColumnNearX
	pos_type const pos = row.pos()
		+ getColumnNearX(pit, row, xx, bound);
	cur.pit() = pit;
	cur.pos() = pos;
	cur.boundary(bound);
	cur.setTargetX(x);

	// try to descend into nested insets
	Inset * inset = checkInsetHit(x, y);
	//lyxerr << "inset " << inset << " hit at x: " << x << " y: " << y << endl;
	if (!inset) {
		// Either we deconst editXY or better we move current_font
		// and real_current_font to Cursor
		// FIXME: what is needed now that current_font and real_current_font
		// are transferred?
		cur.setCurrentFont();
		return 0;
	}

	ParagraphList const & pars = text_->paragraphs();
	Inset const * insetBefore = pos? pars[pit].getInset(pos - 1): 0;
	//Inset * insetBehind = pars[pit].getInset(pos);

	// This should be just before or just behind the
	// cursor position set above.
	LASSERT((pos != 0 && inset == insetBefore)
		|| inset == pars[pit].getInset(pos), /**/);

	// Make sure the cursor points to the position before
	// this inset.
	if (inset == insetBefore) {
		--cur.pos();
		cur.boundary(false);
	}

	// Try to descend recursively inside the inset.
	inset = inset->editXY(cur, x, y);

	if (cur.top().text() == text_)
		cur.setCurrentFont();
	return inset;
}


void TextMetrics::setCursorFromCoordinates(Cursor & cur, int const x, int const y)
{
	LASSERT(text_ == cur.text(), /**/);
	pit_type pit = getPitNearY(y);

	ParagraphMetrics const & pm = par_metrics_[pit];

	int yy = pm.position() - pm.ascent();
	LYXERR(Debug::DEBUG, "x: " << x << " y: " << y <<
		" pit: " << pit << " yy: " << yy);

	int r = 0;
	LASSERT(pm.rows().size(), /**/);
	for (; r < int(pm.rows().size()) - 1; ++r) {
		Row const & row = pm.rows()[r];
		if (int(yy + row.height()) > y)
			break;
		yy += row.height();
	}

	Row const & row = pm.rows()[r];

	LYXERR(Debug::DEBUG, "row " << r << " from pos: " << row.pos());

	bool bound = false;
	int xx = x;
	pos_type const pos = row.pos() + getColumnNearX(pit, row, xx, bound);

	LYXERR(Debug::DEBUG, "setting cursor pit: " << pit << " pos: " << pos);

	text_->setCursor(cur, pit, pos, true, bound);
	// remember new position.
	cur.setTargetX();
}


//takes screen x,y coordinates
Inset * TextMetrics::checkInsetHit(int x, int y)
{
	pit_type pit = getPitNearY(y);
	LASSERT(pit != -1, /**/);

	Paragraph const & par = text_->paragraphs()[pit];
	ParagraphMetrics const & pm = par_metrics_[pit];

	LYXERR(Debug::DEBUG, "x: " << x << " y: " << y << "  pit: " << pit);

	InsetList::const_iterator iit = par.insetList().begin();
	InsetList::const_iterator iend = par.insetList().end();
	for (; iit != iend; ++iit) {
		Inset * inset = iit->inset;

		LYXERR(Debug::DEBUG, "examining inset " << inset);

		if (!bv_->coordCache().getInsets().has(inset)) {
			LYXERR(Debug::DEBUG, "inset has no cached position");
			return 0;
		}

		Dimension const & dim = pm.insetDimension(inset);
		Point p = bv_->coordCache().getInsets().xy(inset);

		LYXERR(Debug::DEBUG, "xo: " << p.x_ << "..." << p.x_ + dim.wid
			<< " yo: " << p.y_ - dim.asc << "..." << p.y_ + dim.des);

		if (x >= p.x_
			&& x <= p.x_ + dim.wid
			&& y >= p.y_ - dim.asc
			&& y <= p.y_ + dim.des) {
			LYXERR(Debug::DEBUG, "Hit inset: " << inset);
			return inset;
		}
	}

	LYXERR(Debug::DEBUG, "No inset hit. ");
	return 0;
}


int TextMetrics::cursorX(CursorSlice const & sl,
		bool boundary) const
{
	LASSERT(sl.text() == text_, /**/);
	pit_type const pit = sl.pit();
	Paragraph const & par = text_->paragraphs()[pit];
	ParagraphMetrics const & pm = par_metrics_[pit];
	if (pm.rows().empty())
		return 0;

	pos_type ppos = sl.pos();
	// Correct position in front of big insets
	bool const boundary_correction = ppos != 0 && boundary;
	if (boundary_correction)
		--ppos;

	Row const & row = pm.getRow(sl.pos(), boundary);

	pos_type cursor_vpos = 0;

	Buffer const & buffer = bv_->buffer();
	double x = row.x;
	Bidi bidi;
	bidi.computeTables(par, buffer, row);

	pos_type const row_pos  = row.pos();
	pos_type const end      = row.endpos();
	// Spaces at logical line breaks in bidi text must be skipped during
	// cursor positioning. However, they may appear visually in the middle
	// of a row; they must be skipped, wherever they are...
	// * logically "abc_[HEBREW_\nHEBREW]"
	// * visually "abc_[_WERBEH\nWERBEH]"
	pos_type skipped_sep_vpos = -1;

	if (end <= row_pos)
		cursor_vpos = row_pos;
	else if (ppos >= end)
		cursor_vpos = text_->isRTL(buffer, par) ? row_pos : end;
	else if (ppos > row_pos && ppos >= end)
		// Place cursor after char at (logical) position pos - 1
		cursor_vpos = (bidi.level(ppos - 1) % 2 == 0)
			? bidi.log2vis(ppos - 1) + 1 : bidi.log2vis(ppos - 1);
	else
		// Place cursor before char at (logical) position ppos
		cursor_vpos = (bidi.level(ppos) % 2 == 0)
			? bidi.log2vis(ppos) : bidi.log2vis(ppos) + 1;

	pos_type body_pos = par.beginOfBody();
	if (body_pos > 0 &&
	    (body_pos > end || !par.isLineSeparator(body_pos - 1)))
		body_pos = 0;

	// check for possible inline completion in this row
	DocIterator const & inlineCompletionPos = bv_->inlineCompletionPos();
	pos_type inlineCompletionVPos = -1;
	if (inlineCompletionPos.inTexted()
	    && inlineCompletionPos.text() == text_
	    && inlineCompletionPos.pit() == pit
	    && inlineCompletionPos.pos() - 1 >= row_pos
	    && inlineCompletionPos.pos() - 1 < end) {
		// draw logically behind the previous character
		inlineCompletionVPos = bidi.log2vis(inlineCompletionPos.pos() - 1);
	}

	// Use font span to speed things up, see below
	FontSpan font_span;
	Font font;

	// If the last logical character is a separator, skip it, unless
	// it's in the last row of a paragraph; see skipped_sep_vpos declaration
	if (end > 0 && end < par.size() && par.isSeparator(end - 1))
		skipped_sep_vpos = bidi.log2vis(end - 1);

	// Inline completion RTL special case row_pos == cursor_pos:
	// "__|b" => cursor_pos is right of __
	if (row_pos == inlineCompletionVPos && row_pos == cursor_vpos) {
		font = displayFont(pit, row_pos + 1);
		docstring const & completion = bv_->inlineCompletion();
		if (font.isRightToLeft() && completion.length() > 0)
			x += theFontMetrics(font.fontInfo()).width(completion);
	}

	for (pos_type vpos = row_pos; vpos < cursor_vpos; ++vpos) {
		// Skip the separator which is at the logical end of the row
		if (vpos == skipped_sep_vpos)
			continue;
		pos_type pos = bidi.vis2log(vpos);
		if (body_pos > 0 && pos == body_pos - 1) {
			FontMetrics const & labelfm = theFontMetrics(
				text_->labelFont(buffer, par));
			x += row.label_hfill + labelfm.width(par.layout().labelsep);
			if (par.isLineSeparator(body_pos - 1))
				x -= singleWidth(pit, body_pos - 1);
		}

		// Use font span to speed things up, see above
		if (pos < font_span.first || pos > font_span.last) {
			font_span = par.fontSpan(pos);
			font = displayFont(pit, pos);
		}

		x += pm.singleWidth(pos, font);

		// Inline completion RTL case:
		// "a__|b", __ of b => non-boundary a-pos is right of __
		if (vpos + 1 == inlineCompletionVPos
		    && (vpos + 1 < cursor_vpos || !boundary_correction)) {
			font = displayFont(pit, vpos + 1);
			docstring const & completion = bv_->inlineCompletion();
			if (font.isRightToLeft() && completion.length() > 0)
				x += theFontMetrics(font.fontInfo()).width(completion);
		}

		//  Inline completion LTR case:
		// "b|__a", __ of b => non-boundary a-pos is in front of __
		if (vpos == inlineCompletionVPos
		    && (vpos + 1 < cursor_vpos || boundary_correction)) {
			font = displayFont(pit, vpos);
			docstring const & completion = bv_->inlineCompletion();
			if (!font.isRightToLeft() && completion.length() > 0)
				x += theFontMetrics(font.fontInfo()).width(completion);
		}

		if (par.isSeparator(pos) && pos >= body_pos)
			x += row.separator;
	}

	// see correction above
	if (boundary_correction) {
		if (isRTL(sl, boundary))
			x -= singleWidth(pit, ppos);
		else
			x += singleWidth(pit, ppos);
	}

	return int(x);
}


int TextMetrics::cursorY(CursorSlice const & sl, bool boundary) const
{
	//lyxerr << "TextMetrics::cursorY: boundary: " << boundary << endl;
	ParagraphMetrics const & pm = par_metrics_[sl.pit()];
	if (pm.rows().empty())
		return 0;

	int h = 0;
	h -= par_metrics_[0].rows()[0].ascent();
	for (pit_type pit = 0; pit < sl.pit(); ++pit) {
		h += par_metrics_[pit].height();
	}
	int pos = sl.pos();
	if (pos && boundary)
		--pos;
	size_t const rend = pm.pos2row(pos);
	for (size_t rit = 0; rit != rend; ++rit)
		h += pm.rows()[rit].height();
	h += pm.rows()[rend].ascent();
	return h;
}


// the cursor set functions have a special mechanism. When they
// realize you left an empty paragraph, they will delete it.

bool TextMetrics::cursorHome(Cursor & cur)
{
	LASSERT(text_ == cur.text(), /**/);
	ParagraphMetrics const & pm = par_metrics_[cur.pit()];
	Row const & row = pm.getRow(cur.pos(),cur.boundary());
	return text_->setCursor(cur, cur.pit(), row.pos());
}


bool TextMetrics::cursorEnd(Cursor & cur)
{
	LASSERT(text_ == cur.text(), /**/);
	// if not on the last row of the par, put the cursor before
	// the final space exept if I have a spanning inset or one string
	// is so long that we force a break.
	pos_type end = cur.textRow().endpos();
	if (end == 0)
		// empty text, end-1 is no valid position
		return false;
	bool boundary = false;
	if (end != cur.lastpos()) {
		if (!cur.paragraph().isLineSeparator(end-1)
		    && !cur.paragraph().isNewline(end-1))
			boundary = true;
		else
			--end;
	}
	return text_->setCursor(cur, cur.pit(), end, true, boundary);
}


void TextMetrics::deleteLineForward(Cursor & cur)
{
	LASSERT(text_ == cur.text(), /**/);
	if (cur.lastpos() == 0) {
		// Paragraph is empty, so we just go forward
		text_->cursorForward(cur);
	} else {
		cur.resetAnchor();
		cur.selection() = true; // to avoid deletion
		cursorEnd(cur);
		cur.setSelection();
		// What is this test for ??? (JMarc)
		if (!cur.selection())
			text_->deleteWordForward(cur);
		else
			cap::cutSelection(cur, true, false);
		cur.checkBufferStructure();
	}
}


bool TextMetrics::isLastRow(pit_type pit, Row const & row) const
{
	ParagraphList const & pars = text_->paragraphs();
	return row.endpos() >= pars[pit].size()
		&& pit + 1 == pit_type(pars.size());
}


bool TextMetrics::isFirstRow(pit_type pit, Row const & row) const
{
	return row.pos() == 0 && pit == 0;
}


int TextMetrics::leftMargin(int max_width, pit_type pit) const
{
	LASSERT(pit >= 0, /**/);
	LASSERT(pit < int(text_->paragraphs().size()), /**/);
	return leftMargin(max_width, pit, text_->paragraphs()[pit].size());
}


int TextMetrics::leftMargin(int max_width,
		pit_type const pit, pos_type const pos) const
{
	ParagraphList const & pars = text_->paragraphs();

	LASSERT(pit >= 0, /**/);
	LASSERT(pit < int(pars.size()), /**/);
	Paragraph const & par = pars[pit];
	LASSERT(pos >= 0, /**/);
	LASSERT(pos <= par.size(), /**/);
	Buffer const & buffer = bv_->buffer();
	//lyxerr << "TextMetrics::leftMargin: pit: " << pit << " pos: " << pos << endl;
	DocumentClass const & tclass = buffer.params().documentClass();
	Layout const & layout = par.layout();

	docstring parindent = layout.parindent;

	int l_margin = 0;

	if (text_->isMainText(buffer))
		l_margin += bv_->leftMargin();

	l_margin += theFontMetrics(buffer.params().getFont()).signedWidth(
		tclass.leftmargin());

	if (par.getDepth() != 0) {
		// find the next level paragraph
		pit_type newpar = outerHook(pit, pars);
		if (newpar != pit_type(pars.size())) {
			if (pars[newpar].layout().isEnvironment()) {
				l_margin = leftMargin(max_width, newpar);
			}
			if (tclass.isDefaultLayout(par.layout())
			    || tclass.isEmptyLayout(par.layout())) {
				if (pars[newpar].params().noindent())
					parindent.erase();
				else
					parindent = pars[newpar].layout().parindent;
			}
		}
	}

	// This happens after sections in standard classes. The 1.3.x
	// code compared depths too, but it does not seem necessary
	// (JMarc)
	if (tclass.isDefaultLayout(par.layout())
	    && pit > 0 && pars[pit - 1].layout().nextnoindent)
		parindent.erase();

	FontInfo const labelfont = text_->labelFont(buffer, par);
	FontMetrics const & labelfont_metrics = theFontMetrics(labelfont);

	switch (layout.margintype) {
	case MARGIN_DYNAMIC:
		if (!layout.leftmargin.empty()) {
			l_margin += theFontMetrics(buffer.params().getFont()).signedWidth(
				layout.leftmargin);
		}
		if (!par.labelString().empty()) {
			l_margin += labelfont_metrics.signedWidth(layout.labelindent);
			l_margin += labelfont_metrics.width(par.labelString());
			l_margin += labelfont_metrics.width(layout.labelsep);
		}
		break;

	case MARGIN_MANUAL: {
		l_margin += labelfont_metrics.signedWidth(layout.labelindent);
		// The width of an empty par, even with manual label, should be 0
		if (!par.empty() && pos >= par.beginOfBody()) {
			if (!par.getLabelWidthString().empty()) {
				docstring labstr = par.getLabelWidthString();
				l_margin += labelfont_metrics.width(labstr);
				l_margin += labelfont_metrics.width(layout.labelsep);
			}
		}
		break;
	}

	case MARGIN_STATIC: {
		l_margin += theFontMetrics(buffer.params().getFont()).
			signedWidth(layout.leftmargin) * 4	/ (par.getDepth() + 4);
		break;
	}

	case MARGIN_FIRST_DYNAMIC:
		if (layout.labeltype == LABEL_MANUAL) {
			if (pos >= par.beginOfBody()) {
				l_margin += labelfont_metrics.signedWidth(layout.leftmargin);
			} else {
				l_margin += labelfont_metrics.signedWidth(layout.labelindent);
			}
		} else if (pos != 0
			   // Special case to fix problems with
			   // theorems (JMarc)
			   || (layout.labeltype == LABEL_STATIC
			       && layout.latextype == LATEX_ENVIRONMENT
			       && !isFirstInSequence(pit, pars))) {
			l_margin += labelfont_metrics.signedWidth(layout.leftmargin);
		} else if (layout.labeltype != LABEL_TOP_ENVIRONMENT
			   && layout.labeltype != LABEL_BIBLIO
			   && layout.labeltype !=
			   LABEL_CENTERED_TOP_ENVIRONMENT) {
			l_margin += labelfont_metrics.signedWidth(layout.labelindent);
			l_margin += labelfont_metrics.width(layout.labelsep);
			l_margin += labelfont_metrics.width(par.labelString());
		}
		break;

	case MARGIN_RIGHT_ADDRESS_BOX: {
#if 0
		// ok, a terrible hack. The left margin depends on the widest
		// row in this paragraph.
		RowList::iterator rit = par.rows().begin();
		RowList::iterator end = par.rows().end();
		// FIXME: This is wrong.
		int minfill = max_width;
		for ( ; rit != end; ++rit)
			if (rit->fill() < minfill)
				minfill = rit->fill();
		l_margin += theFontMetrics(params.getFont()).signedWidth(layout.leftmargin);
		l_margin += minfill;
#endif
		// also wrong, but much shorter.
		l_margin += max_width / 2;
		break;
	}
	}

	if (!par.params().leftIndent().zero())
		l_margin += par.params().leftIndent().inPixels(max_width);

	LyXAlignment align;

	if (par.params().align() == LYX_ALIGN_LAYOUT)
		align = layout.align;
	else
		align = par.params().align();

	// set the correct parindent
	if (pos == 0
	    && (layout.labeltype == LABEL_NO_LABEL
	       || layout.labeltype == LABEL_TOP_ENVIRONMENT
	       || layout.labeltype == LABEL_CENTERED_TOP_ENVIRONMENT
	       || (layout.labeltype == LABEL_STATIC
	           && layout.latextype == LATEX_ENVIRONMENT
	           && !isFirstInSequence(pit, pars)))
	    && align == LYX_ALIGN_BLOCK
	    && !par.params().noindent()
	    // in some insets, paragraphs are never indented
	    && !(par.inInset() && par.inInset()->neverIndent())
	    // display style insets are always centered, omit indentation
	    && !(!par.empty()
		    && par.isInset(pos)
		    && par.getInset(pos)->display())
			&& (!(tclass.isDefaultLayout(par.layout())
	         || tclass.isEmptyLayout(par.layout()))
	        || buffer.params().paragraph_separation == BufferParams::ParagraphIndentSeparation)
	    )
	{
		l_margin += theFontMetrics(buffer.params().getFont()).signedWidth(
			parindent);
	}

	return l_margin;
}


int TextMetrics::singleWidth(pit_type pit, pos_type pos) const
{
	ParagraphMetrics const & pm = par_metrics_[pit];

	return pm.singleWidth(pos, displayFont(pit, pos));
}


void TextMetrics::draw(PainterInfo & pi, int x, int y) const
{
	if (par_metrics_.empty())
		return;

	origin_.x_ = x;
	origin_.y_ = y;

	ParMetricsCache::iterator it = par_metrics_.begin();
	ParMetricsCache::iterator const pm_end = par_metrics_.end();
	y -= it->second.ascent();
	for (; it != pm_end; ++it) {
		ParagraphMetrics const & pmi = it->second;
		y += pmi.ascent();
		pit_type const pit = it->first;
		// Save the paragraph position in the cache.
		it->second.setPosition(y);
		drawParagraph(pi, pit, x, y);
		y += pmi.descent();
	}
}


void TextMetrics::drawParagraph(PainterInfo & pi, pit_type pit, int x, int y) const
{
	BufferParams const & bparams = bv_->buffer().params();
	ParagraphMetrics const & pm = par_metrics_[pit];
	if (pm.rows().empty())
		return;

	Bidi bidi;
	bool const original_drawing_state = pi.pain.isDrawingEnabled();
	int const ww = bv_->workHeight();
	size_t const nrows = pm.rows().size();

	Cursor const & cur = bv_->cursor();
	DocIterator sel_beg = cur.selectionBegin();
	DocIterator sel_end = cur.selectionEnd();
	bool selection = cur.selection()
		// This is our text.
		&& cur.text() == text_
		// if the anchor is outside, this is not our selection
		&& cur.anchor().text() == text_
		&& pit >= sel_beg.pit() && pit <= sel_end.pit();

	// We care only about visible selection.
	if (selection) {
		if (pit != sel_beg.pit()) {
			sel_beg.pit() = pit;
			sel_beg.pos() = 0;
		}
		if (pit != sel_end.pit()) {
			sel_end.pit() = pit;
			sel_end.pos() = sel_end.lastpos();
		}
	}

	for (size_t i = 0; i != nrows; ++i) {

		Row const & row = pm.rows()[i];
		if (i)
			y += row.ascent();

		bool const inside = (y + row.descent() >= 0
			&& y - row.ascent() < ww);
		// It is not needed to draw on screen if we are not inside.
		pi.pain.setDrawingEnabled(inside && original_drawing_state);
		RowPainter rp(pi, *text_, pit, row, bidi, x, y);

		if (selection)
			row.setSelection(sel_beg.pos(), sel_end.pos());
		else
			row.setSelection(-1, -1);

		// Row signature; has row changed since last paint?
		row.setCrc(pm.computeRowSignature(row, bparams));
		bool row_has_changed = row.changed();

		// Don't paint the row if a full repaint has not been requested
		// and if it has not changed.
		if (!pi.full_repaint && !row_has_changed) {
			// Paint only the insets if the text itself is
			// unchanged.
			rp.paintOnlyInsets();
			y += row.descent();
			continue;
		}

		// Clear background of this row if paragraph background was not
		// already cleared because of a full repaint.
		if (!pi.full_repaint && row_has_changed) {
			pi.pain.fillRectangle(x, y - row.ascent(),
				width(), row.height(), pi.background_color);
		}

		bool row_selection = row.sel_beg != -1 && row.sel_end != -1;
		if (row_selection) {
			DocIterator beg = bv_->cursor().selectionBegin();
			DocIterator end = bv_->cursor().selectionEnd();
			// FIXME (not here): pit is not updated when extending
			// a selection to a new row with cursor right/left
			bool const beg_margin = beg.pit() < pit;
			bool const end_margin = end.pit() > pit;
			beg.pit() = pit;
			beg.pos() = row.sel_beg;
			end.pit() = pit;
			end.pos() = row.sel_end;
			drawRowSelection(pi, x, row, beg, end, beg_margin, end_margin);
		}

		// Instrumentation for testing row cache (see also
		// 12 lines lower):
		if (lyxerr.debugging(Debug::PAINTING) && inside
			&& (row_selection || pi.full_repaint || row_has_changed)) {
				string const foreword = text_->isMainText(bv_->buffer()) ?
					"main text redraw " : "inset text redraw: ";
			LYXERR(Debug::PAINTING, foreword << "pit=" << pit << " row=" << i
				<< " row_selection="	<< row_selection
				<< " full_repaint="	<< pi.full_repaint
				<< " row_has_changed="	<< row_has_changed);
		}

		// Backup full_repaint status and force full repaint
		// for inner insets as the Row has been cleared out.
		bool tmp = pi.full_repaint;
		pi.full_repaint = true;
		rp.paintAppendix();
		rp.paintDepthBar();
		rp.paintChangeBar();
		if (i == 0)
			rp.paintFirst();
		rp.paintText();
		if (i == nrows - 1)
			rp.paintLast();
		y += row.descent();
		// Restore full_repaint status.
		pi.full_repaint = tmp;
	}
	// Re-enable screen drawing for future use of the painter.
	pi.pain.setDrawingEnabled(original_drawing_state);

	//LYXERR(Debug::PAINTING, ".");
}


void TextMetrics::drawRowSelection(PainterInfo & pi, int x, Row const & row,
		DocIterator const & beg, DocIterator const & end,
		bool drawOnBegMargin, bool drawOnEndMargin) const
{
	Buffer & buffer = bv_->buffer();
	DocIterator cur = beg;
	int x1 = cursorX(beg.top(), beg.boundary());
	int x2 = cursorX(end.top(), end.boundary());
	int y1 = bv_->getPos(cur, cur.boundary()).y_ - row.ascent();
	int y2 = y1 + row.height();

	// draw the margins
	if (drawOnBegMargin) {
		if (text_->isRTL(buffer, beg.paragraph())) {
			int lm = bv_->leftMargin();
			pi.pain.fillRectangle(x + x1, y1, width() - lm - x1, y2 - y1, Color_selection);
		} else {
			int rm = bv_->rightMargin();
			pi.pain.fillRectangle(rm, y1, x1 - rm, y2 - y1, Color_selection);
		}
	}

	if (drawOnEndMargin) {
		if (text_->isRTL(buffer, beg.paragraph())) {
			int rm = bv_->rightMargin();
			pi.pain.fillRectangle(x + rm, y1, x2 - rm, y2 - y1, Color_selection);
		} else {
			int lm = bv_->leftMargin();
			pi.pain.fillRectangle(x + x2, y1, width() - lm - x2, y2 - y1, Color_selection);
		}
	}

	// if we are on a boundary from the beginning, it's probably
	// a RTL boundary and we jump to the other side directly as this
	// segement is 0-size and confuses the logic below
	if (cur.boundary())
		cur.boundary(false);

	// go through row and draw from RTL boundary to RTL boundary
	while (cur < end) {
		bool drawNow = false;

		// simplified cursorForward code below which does not
		// descend into insets and which does not go into the
		// next line. Compare the logic with the original cursorForward

		// if left of boundary -> just jump to right side
		// but for RTL boundaries don't, because: abc|DDEEFFghi -> abcDDEEF|Fghi
		if (cur.boundary()) {
			cur.boundary(false);
		}	else if (isRTLBoundary(cur.pit(), cur.pos() + 1)) {
			// in front of RTL boundary -> Stay on this side of the boundary because:
			//   ab|cDDEEFFghi -> abc|DDEEFFghi
			++cur.pos();
			cur.boundary(true);
			drawNow = true;
		} else {
			// move right
			++cur.pos();

			// line end?
			if (cur.pos() == row.endpos())
				cur.boundary(true);
		}

		if (x1 == -1) {
			// the previous segment was just drawn, now the next starts
			x1 = cursorX(cur.top(), cur.boundary());
		}

		if (!(cur < end) || drawNow) {
			x2 = cursorX(cur.top(), cur.boundary());
			pi.pain.fillRectangle(x + min(x1,x2), y1, abs(x2 - x1), y2 - y1,
				Color_selection);

			// reset x1, so it is set again next round (which will be on the
			// right side of a boundary or at the selection end)
			x1 = -1;
		}
	}
}


void TextMetrics::completionPosAndDim(Cursor const & cur, int & x, int & y,
	Dimension & dim) const
{
	Cursor const & bvcur = cur.bv().cursor();

	// get word in front of cursor
	docstring word = text_->previousWord(bvcur.top());
	DocIterator wordStart = bvcur;
	wordStart.pos() -= word.length();

	// get position on screen of the word start and end
	Point lxy = cur.bv().getPos(wordStart, false);
	Point rxy = cur.bv().getPos(bvcur, bvcur.boundary());

	// calculate dimensions of the word
	dim = rowHeight(bvcur.pit(), wordStart.pos(), bvcur.pos(), false);
	dim.wid = abs(rxy.x_ - lxy.x_);

	// calculate position of word
	y = lxy.y_;
	x = min(rxy.x_, lxy.x_);

	//lyxerr << "wid=" << dim.width() << " x=" << x << " y=" << y << " lxy.x_=" << lxy.x_ << " rxy.x_=" << rxy.x_ << " word=" << word << std::endl;
	//lyxerr << " wordstart=" << wordStart << " bvcur=" << bvcur << " cur=" << cur << std::endl;
}

//int TextMetrics::pos2x(pit_type pit, pos_type pos) const
//{
//	ParagraphMetrics const & pm = par_metrics_[pit];
//	Row const & r = pm.rows()[row];
//	int x = 0;
//	pos -= r.pos();
//}


int defaultRowHeight()
{
	return int(theFontMetrics(sane_font).maxHeight() *  1.2);
}

} // namespace lyx
