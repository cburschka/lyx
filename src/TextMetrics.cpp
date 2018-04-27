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

#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "InsetList.h"
#include "Language.h"
#include "Layout.h"
#include "LyXRC.h"
#include "MetricsInfo.h"
#include "ParagraphParameters.h"
#include "RowPainter.h"
#include "Text.h"
#include "TextClass.h"
#include "VSpace.h"

#include "insets/InsetText.h"

#include "mathed/InsetMathMacroTemplate.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"
#include "frontends/NullPainter.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/lassert.h"

#include <stdlib.h>
#include <cmath>

using namespace std;


namespace lyx {

using frontend::FontMetrics;

namespace {


int numberOfLabelHfills(Paragraph const & par, Row const & row)
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

// FIXME: this needs to be rewritten, probably by merging it into some
// code that, besides counting, sets the active status of the space
// inset in the row element.
int numberOfHfills(Row const & row, ParagraphMetrics const & pm,
                   pos_type const body_pos)
{
	int n = 0;
	Row::const_iterator cit = row.begin();
	Row::const_iterator const end = row.end();
	for ( ; cit != end ; ++cit)
		if (cit->pos >= body_pos
		    && cit->inset && pm.hfillExpansion(row, cit->pos))
			++n;
	return n;
}


} // namespace

/////////////////////////////////////////////////////////////////////
//
// TextMetrics
//
/////////////////////////////////////////////////////////////////////


TextMetrics::TextMetrics(BufferView * bv, Text * text)
	: bv_(bv), text_(text)
{
	LBUFERR(bv_);
	max_width_ = bv_->workWidth();
	dim_.wid = max_width_;
	dim_.asc = 10;
	dim_.des = 10;
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
	LBUFERR(!par_metrics_.empty());
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


bool TextMetrics::metrics(MetricsInfo & mi, Dimension & dim, int min_width)
{
	LBUFERR(mi.base.textwidth > 0);
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


void TextMetrics::updatePosCache(pit_type pit) const
{
	frontend::NullPainter np;
	PainterInfo pi(bv_, np);
	drawParagraph(pi, pit, origin_.x_, par_metrics_[pit].position());
}


int TextMetrics::rightMargin(ParagraphMetrics const & pm) const
{
	return text_->isMainText() ? pm.rightMargin(*bv_) : 0;
}


int TextMetrics::rightMargin(pit_type const pit) const
{
	return text_->isMainText() ? par_metrics_[pit].rightMargin(*bv_) : 0;
}


void TextMetrics::applyOuterFont(Font & font) const
{
	FontInfo lf(font_.fontInfo());
	lf.reduce(bv_->buffer().params().getFont().fontInfo());
	font.fontInfo().realize(lf);
}


Font TextMetrics::displayFont(pit_type pit, pos_type pos) const
{
	LASSERT(pos >= 0, { static Font f; return f; });

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
		if (!text_->isMainText())
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

	if (!text_->isMainText())
		applyOuterFont(font);

	// Realize against environment font information
	// NOTE: the cast to pit_type should be removed when pit_type
	// changes to a unsigned integer.
	if (pit < pit_type(pars.size()))
		font.fontInfo().realize(text_->outerFont(pit).fontInfo());

	// Realize with the fonts of lesser depth.
	font.fontInfo().realize(params.getFont().fontInfo());

	return font;
}


bool TextMetrics::isRTL(CursorSlice const & sl, bool boundary) const
{
	if (!sl.text())
		return false;

	int correction = 0;
	if (boundary && sl.pos() > 0)
		correction = -1;

	return displayFont(sl.pit(), sl.pos() + correction).isVisibleRightToLeft();
}


bool TextMetrics::isRTLBoundary(pit_type pit, pos_type pos) const
{
	// no RTL boundary at paragraph start
	if (pos == 0)
		return false;

	Font const & left_font = displayFont(pit, pos - 1);

	return isRTLBoundary(pit, pos, left_font);
}


// isRTLBoundary returns false on a real end-of-line boundary,
// because otherwise the two boundary types get mixed up.
// This is the whole purpose of this being in TextMetrics.
bool TextMetrics::isRTLBoundary(pit_type pit, pos_type pos,
		Font const & font) const
{
	if (// no RTL boundary at paragraph start
	    pos == 0
	    // if the metrics have not been calculated, then we are not
	    // on screen and can safely ignore issues about boundaries.
	    || !contains(pit))
		return false;

	ParagraphMetrics & pm = par_metrics_[pit];
	// no RTL boundary in empty paragraph
	if (pm.rows().empty())
		return false;

	pos_type endpos = pm.getRow(pos - 1, false).endpos();
	pos_type startpos = pm.getRow(pos, false).pos();
	// no RTL boundary at line start:
	// abc\n   -> toggle to RTL ->    abc\n     (and not:    abc\n|
	// |                              |                               )
	if (pos == startpos && pos == endpos) // start of cur row, end of prev row
		return false;

	Paragraph const & par = text_->getPar(pit);
	// no RTL boundary at line break:
	// abc|\n    -> move right ->   abc\n       (and not:    abc\n|
	// FED                          FED|                     FED     )
	if (startpos == pos && endpos == pos && endpos != par.size()
		&& (par.isNewline(pos - 1)
			|| par.isEnvSeparator(pos - 1)
			|| par.isLineSeparator(pos - 1)
			|| par.isSeparator(pos - 1)))
		return false;

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
	// IMPORTANT NOTE: We pass 'false' explicitly in order to not call
	// redoParagraph() recursively inside parMetrics.
	Dimension old_dim = parMetrics(pit, false).dim();
	ParagraphMetrics & pm = par_metrics_[pit];
	pm.reset(par);

	Buffer & buffer = bv_->buffer();
	bool changed = false;

	// Check whether there are InsetBibItems that need fixing
	// FIXME: This check ought to be done somewhere else. It is the reason
	// why text_ is not const. But then, where else to do it?
	// Well, how can you end up with either (a) a biblio environment that
	// has no InsetBibitem or (b) a biblio environment with more than one
	// InsetBibitem? I think the answer is: when paragraphs are merged;
	// when layout is set; when material is pasted.
	if (par.brokenBiblio()) {
		Cursor & cur = const_cast<Cursor &>(bv_->cursor());
		// In some cases, we do not know how to record undo
		if (&cur.inset() == &text_->inset())
			cur.recordUndo(pit, pit);

		int const moveCursor = par.fixBiblio(buffer);

		// Is it necessary to update the cursor?
		if (&cur.inset() == &text_->inset() && cur.pit() == pit) {
			if (moveCursor > 0)
				cur.posForward();
			else if (moveCursor < 0 && cur.pos() >= -moveCursor)
				cur.posBackward();
		}
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
		// FIXME audit updateBuffer calls
		// This should not be here, but it is not clear yet where else it
		// should be.
		bv_->buffer().updateBuffer();
		parPos = text_->macrocontextPosition();
		LBUFERR(!parPos.empty());
		parPos.pit() = pit;
	}

	// redo insets
	par.setBeginOfBody();
	Font const bufferfont = buffer.params().getFont();
	CoordCache::Insets & insetCache = bv_->coordCache().insets();
	InsetList::const_iterator ii = par.insetList().begin();
	InsetList::const_iterator iend = par.insetList().end();
	for (; ii != iend; ++ii) {
		// FIXME Doesn't this HAVE to be non-empty?
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

		// If there is an end of paragraph marker, its size should be
		// substracted to the available width. The logic here is
		// almost the same as in breakRow, remember keep them in sync.
		int eop = 0;
		if (lyxrc.paragraph_markers && ii->pos + 1 == par.size()
		    && size_type(pit + 1) < text_->paragraphs().size()) {
			Font f(text_->layoutFont(pit));
			// ¶ U+00B6 PILCROW SIGN
			eop = theFontMetrics(f).width(char_type(0x00B6));
		}

		// do the metric calculation
		Dimension dim;
		int const w = max_width_ - leftMargin(pit, ii->pos)
			- right_margin - eop;
		Font const & font = ii->inset->inheritFont() ?
			displayFont(pit, ii->pos) : bufferfont;
		MacroContext mc(&buffer, parPos);
		MetricsInfo mi(bv_, font.fontInfo(), w, mc);
		ii->inset->metrics(mi, dim);
		if (!insetCache.has(ii->inset) || insetCache.dim(ii->inset) != dim) {
			insetCache.add(ii->inset, dim);
			changed = true;
		}
	}

	pos_type first = 0;
	size_t row_index = 0;
	bool need_new_row = false;
	// maximum pixel width of a row
	do {
		if (row_index == pm.rows().size())
			pm.rows().push_back(Row());
		Row & row = pm.rows()[row_index];
		row.pit(pit);
		row.pos(first);
		row.pit(pit);
		need_new_row = breakRow(row, right_margin);
		setRowHeight(row);
		row.changed(true);
		if (row_index || row.endpos() < par.size()
		    || (row.right_boundary() && par.inInset().lyxCode() != CELL_CODE)) {
			/* If there is more than one row or the row has been
			 * broken by a display inset or a newline, expand the text
			 * to the full allowable width. This setting here is
			 * needed for the computeRowMetrics() below. In the case
			 * of a display inset, we do nothing when inside a table
			 * cell, because the tabular code is not prepared for
			 * that, and it triggers when using a caption in a
			 * longtable (see bugs #9945 and #9757).
			 */
			if (dim_.wid < max_width_)
				dim_.wid = max_width_;
		}
		int const max_row_width = max(dim_.wid, row.width());
		computeRowMetrics(row, max_row_width);
		first = row.endpos();
		++row_index;

		pm.dim().wid = max(pm.dim().wid, row.width());
		pm.dim().des += row.height();
	} while (first < par.size() || need_new_row);

	if (row_index < pm.rows().size())
		pm.rows().resize(row_index);

	// FIXME: It might be better to move this in another method
	// specially tailored for the main text.
	// Top and bottom margin of the document (only at top-level)
	if (text_->isMainText()) {
		// original value was 20px, which is 0.2in at 100dpi
		int const margin = Length(0.2, Length::IN).inPixels(0);
		if (pit == 0) {
			pm.rows().front().dimension().asc += margin;
			/* coverity thinks that we should update pm.dim().asc
			 * below, but all the rows heights are actually counted as
			 * part of the paragraph metric descent see loop above).
			 */
			// coverity[copy_paste_error]
			pm.dim().des += margin;
		}
		ParagraphList const & pars = text_->paragraphs();
		if (pit + 1 == pit_type(pars.size())) {
			pm.rows().back().dimension().des += margin;
			pm.dim().des += margin;
		}
	}

	// The space above and below the paragraph.
	int const top = parTopSpacing(pit);
	pm.rows().front().dimension().asc += top;
	int const bottom = parBottomSpacing(pit);
	pm.rows().back().dimension().des += bottom;
	pm.dim().des += top + bottom;

	pm.dim().asc += pm.rows()[0].ascent();
	pm.dim().des -= pm.rows()[0].ascent();

	changed |= old_dim.height() != pm.dim().height();

	return changed;
}


LyXAlignment TextMetrics::getAlign(Paragraph const & par, Row const & row) const
{
	LyXAlignment align = par.getAlign();

	// handle alignment inside tabular cells
	Inset const & owner = text_->inset();
	bool forced_block = false;
	switch (owner.contentAlignment()) {
	case LYX_ALIGN_BLOCK:
		// In general block align is the default state, but here it is
		// an explicit choice. Therefore it should not be overridden
		// later.
		forced_block = true;
		// fall through
	case LYX_ALIGN_CENTER:
	case LYX_ALIGN_LEFT:
	case LYX_ALIGN_RIGHT:
		if (align == LYX_ALIGN_NONE || align == LYX_ALIGN_BLOCK)
			align = owner.contentAlignment();
		break;
	default:
		// unchanged (use align)
		break;
	}

	// Display-style insets should always be on a centered row
	if (Inset const * inset = par.getInset(row.pos())) {
		switch (inset->display()) {
		case Inset::AlignLeft:
			align = LYX_ALIGN_BLOCK;
			break;
		case Inset::AlignCenter:
			align = LYX_ALIGN_CENTER;
			break;
		case Inset::Inline:
			// unchanged (use align)
			break;
		case Inset::AlignRight:
			align = LYX_ALIGN_RIGHT;
			break;
		}
	}

	if (align == LYX_ALIGN_BLOCK) {
		// If this row has been broken abruptly by a display inset, or
		// it is the end of the paragraph, or the user requested we
		// not justify stuff, then don't stretch.
		// A forced block alignment can only be overridden the 'no
		// justification on screen' setting.
		if ((row.flushed() && !forced_block)
		    || !bv_->buffer().params().justification)
			align = row.isRTL() ? LYX_ALIGN_RIGHT : LYX_ALIGN_LEFT;
	}

	return align;
}


void TextMetrics::computeRowMetrics(Row & row, int width) const
{
	row.label_hfill = 0;
	row.separator = 0;

	Paragraph const & par = text_->getPar(row.pit());

	int const w = width - row.right_margin - row.width();
	// FIXME: put back this assertion when the crash on new doc is solved.
	//LASSERT(w >= 0, /**/);

	// is there a manual margin with a manual label
	Layout const & layout = par.layout();

	int nlh = 0;
	if (layout.margintype == MARGIN_MANUAL
	    && layout.labeltype == LABEL_MANUAL) {
		/// We might have real hfills in the label part
		nlh = numberOfLabelHfills(par, row);

		// A manual label par (e.g. List) has an auto-hfill
		// between the label text and the body of the
		// paragraph too.
		// But we don't want to do this auto hfill if the par
		// is empty.
		if (!par.empty())
			++nlh;

		if (nlh && !par.getLabelWidthString().empty())
			row.label_hfill = labelFill(row) / double(nlh);
	}

	// are there any hfills in the row?
	ParagraphMetrics & pm = par_metrics_[row.pit()];
	int nh = numberOfHfills(row, pm, par.beginOfBody());
	int hfill = 0;
	int hfill_rem = 0;

	// We don't have to look at the alignment if the row is already
	// larger then the permitted width as then we force the
	// LEFT_ALIGN'edness!
	if (int(row.width()) >= max_width_)
		return;

	if (nh == 0) {
		// Common case : there is no hfill, and the alignment will be
		// meaningful
		switch (getAlign(par, row)) {
		case LYX_ALIGN_BLOCK:
			// Expand expanding characters by a total of w
			if (!row.setExtraWidth(w) && row.isRTL()) {
				// Justification failed and the text is RTL: align to the right
				row.left_margin += w;
				row.dimension().wid += w;
			}
			break;
		case LYX_ALIGN_LEFT:
			// a displayed inset that is flushed
			if (Inset const * inset = par.getInset(row.pos())) {
				row.left_margin += inset->indent(*bv_);
				row.dimension().wid += inset->indent(*bv_);
			}
			break;
		case LYX_ALIGN_RIGHT:
			if (Inset const * inset = par.getInset(row.pos())) {
				int const new_w = max(w - inset->indent(*bv_), 0);
				row.left_margin += new_w;
				row.dimension().wid += new_w;
			} else {
				row.left_margin += w;
				row.dimension().wid += w;
			}
			break;
		case LYX_ALIGN_CENTER:
			row.dimension().wid += w / 2;
			row.left_margin += w / 2;
			break;
		case LYX_ALIGN_NONE:
		case LYX_ALIGN_LAYOUT:
		case LYX_ALIGN_SPECIAL:
		case LYX_ALIGN_DECIMAL:
			break;
		}
		return;
	}

	// Case nh > 0. There are hfill separators.
	hfill = w / nh;
	hfill_rem = w % nh;
	row.dimension().wid += w;
	// Set size of hfill insets
	pos_type const endpos = row.endpos();
	pos_type body_pos = par.beginOfBody();
	if (body_pos > 0
	    && (body_pos > endpos || !par.isLineSeparator(body_pos - 1)))
		body_pos = 0;

	CoordCache::Insets & insetCache = bv_->coordCache().insets();
	Row::iterator cit = row.begin();
	Row::iterator const cend = row.end();
	for ( ; cit != cend; ++cit) {
		if (row.label_hfill && cit->endpos == body_pos
		    && cit->type == Row::SPACE)
			cit->dim.wid -= int(row.label_hfill * (nlh - 1));
		if (cit->inset && pm.hfillExpansion(row, cit->pos)) {
			if (cit->pos >= body_pos) {
				cit->dim.wid += hfill;
				--nh;
				if (nh == 0)
					cit->dim.wid += hfill_rem;
			} else
				cit->dim.wid += int(row.label_hfill);
			// Cache the inset dimension.
			insetCache.add(cit->inset, cit->dim);
		}
	}
}


int TextMetrics::labelFill(Row const & row) const
{
	Paragraph const & par = text_->getPar(row.pit());
	LBUFERR(par.beginOfBody() > 0 || par.isEnvSeparator(0));

	int w = 0;
	Row::const_iterator cit = row.begin();
	Row::const_iterator const end = row.end();
	// iterate over elements before main body (except the last one,
	// which is extra space).
	while (cit!= end && cit->endpos < par.beginOfBody()) {
		w += cit->dim.wid;
		++cit;
	}

	docstring const & label = par.params().labelWidthString();
	if (label.empty())
		return 0;

	FontMetrics const & fm
		= theFontMetrics(text_->labelFont(par));

	return max(0, fm.width(label) - w);
}


#if 0
// Not used, see TextMetrics::breakRow
// this needs special handling - only newlines count as a break point
static pos_type addressBreakPoint(pos_type i, Paragraph const & par)
{
	pos_type const end = par.size();

	for (; i < end; ++i)
		if (par.isNewline(i))
			return i + 1;

	return end;
}
#endif


int TextMetrics::labelEnd(pit_type const pit) const
{
	// labelEnd is only needed if the layout fills a flushleft label.
	if (text_->getPar(pit).layout().margintype != MARGIN_MANUAL)
		return 0;
	// return the beginning of the body
	return leftMargin(pit);
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
		if (pos_ < par_.size() && (pos_ > endspan_ || pos_ == bodypos_)) {
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

} // namespace

/** This is the function where the hard work is done. The code here is
 * very sensitive to small changes :) Note that part of the
 * intelligence is also in Row::shortenIfNeeded.
 */
bool TextMetrics::breakRow(Row & row, int const right_margin) const
{
	Paragraph const & par = text_->getPar(row.pit());
	pos_type const end = par.size();
	pos_type const pos = row.pos();
	pos_type const body_pos = par.beginOfBody();
	bool const is_rtl = text_->isRTL(par);
	bool need_new_row = false;

	row.clear();
	row.left_margin = leftMargin(row.pit(), pos);
	row.right_margin = right_margin;
	if (is_rtl)
		swap(row.left_margin, row.right_margin);
	// Remember that the row width takes into account the left_margin
	// but not the right_margin.
	row.dimension().wid = row.left_margin;
	// the width available for the row.
	int const width = max_width_ - row.right_margin;

#if 0
	//FIXME: As long as leftMargin() is not correctly implemented for
	// MARGIN_RIGHT_ADDRESS_BOX, we should also not do this here.
	// Otherwise, long rows will be painted off the screen.
	if (par.layout().margintype == MARGIN_RIGHT_ADDRESS_BOX)
		return addressBreakPoint(pos, par);
#endif

	// check for possible inline completion
	DocIterator const & ic_it = bv_->inlineCompletionPos();
	pos_type ic_pos = -1;
	if (ic_it.inTexted() && ic_it.text() == text_ && ic_it.pit() == row.pit())
		ic_pos = ic_it.pos();

	// Now we iterate through until we reach the right margin
	// or the end of the par, then build a representation of the row.
	pos_type i = pos;
	FontIterator fi = FontIterator(*this, par, row.pit(), pos);
	while (i < end && (i == pos || row.width() <= width)) {
		char_type c = par.getChar(i);
		// The most special cases are handled first.
		if (par.isInset(i)) {
			Inset const * ins = par.getInset(i);
			Dimension dim = bv_->coordCache().insets().dim(ins);
			row.add(i, ins, dim, *fi, par.lookupChange(i));
		} else if (c == ' ' && i + 1 == body_pos) {
			// There is a space at i, but it should not be
			// added as a separator, because it is just
			// before body_pos. Instead, insert some spacing to
			// align text
			FontMetrics const & fm = theFontMetrics(text_->labelFont(par));
			// this is needed to make sure that the row width is correct
			row.finalizeLast();
			int const add = max(fm.width(par.layout().labelsep),
			                    labelEnd(row.pit()) - row.width());
			row.addSpace(i, add, *fi, par.lookupChange(i));
		} else if (c == '\t')
			row.addSpace(i, theFontMetrics(*fi).width(from_ascii("    ")),
				     *fi, par.lookupChange(i));
		else if (c == 0x2028 || c == 0x2029) {
			/**
			 * U+2028 LINE SEPARATOR
			 * U+2029 PARAGRAPH SEPARATOR

			 * These are special unicode characters that break
			 * lines/pragraphs. Not handling them lead to trouble wrt
			 * Qt QTextLayout formatting. We add a visible character
			 * on screen so that the user can see that something is
			 * happening.
			*/
			row.finalizeLast();
			// ⤶ U+2936 ARROW POINTING DOWNWARDS THEN CURVING LEFTWARDS
			// ¶ U+00B6 PILCROW SIGN
			char_type const screen_char = (c == 0x2028) ? 0x2936 : 0x00B6;
			row.add(i, screen_char, *fi, par.lookupChange(i));
		} else {
			// FIXME: please someone fix the Hebrew/Arabic parenthesis mess!
			// see also Paragraph::getUChar.
			if (fi->language()->lang() == "hebrew") {
				if (c == '(')
					c = ')';
				else if (c == ')')
					c = '(';
			}
			row.add(i, c, *fi, par.lookupChange(i));
		}

		// add inline completion width
		// draw logically behind the previous character
		if (ic_pos == i + 1 && !bv_->inlineCompletion().empty()) {
			docstring const comp = bv_->inlineCompletion();
			size_t const uniqueTo =bv_->inlineCompletionUniqueChars();
			Font f = *fi;

			if (uniqueTo > 0) {
				f.fontInfo().setColor(Color_inlinecompletion);
				row.addVirtual(i + 1, comp.substr(0, uniqueTo), f, Change());
			}
			f.fontInfo().setColor(Color_nonunique_inlinecompletion);
			row.addVirtual(i + 1, comp.substr(uniqueTo), f, Change());
		}

		// Handle some situations that abruptly terminate the row
		// - A newline inset
		// - Before a display inset
		// - After a display inset
		Inset const * inset = 0;
		if (par.isNewline(i) || par.isEnvSeparator(i)
		    || (i + 1 < end && (inset = par.getInset(i + 1))
			&& inset->display())
		    || (!row.empty() && row.back().inset
			&& row.back().inset->display())) {
			row.flushed(true);
			need_new_row = par.isNewline(i);
			++i;
			break;
		}

		++i;
		++fi;
	}
	row.finalizeLast();
	row.endpos(i);

	// End of paragraph marker. The logic here is almost the
	// same as in redoParagraph, remember keep them in sync.
	ParagraphList const & pars = text_->paragraphs();
	if (lyxrc.paragraph_markers && !need_new_row
	    && i == end && size_type(row.pit() + 1) < pars.size()) {
		// add a virtual element for the end-of-paragraph
		// marker; it is shown on screen, but does not exist
		// in the paragraph.
		Font f(text_->layoutFont(row.pit()));
		f.fontInfo().setColor(Color_paragraphmarker);
		BufferParams const & bparams
			= text_->inset().buffer().params();
		f.setLanguage(par.getParLanguage(bparams));
		// ¶ U+00B6 PILCROW SIGN
		row.addVirtual(end, docstring(1, char_type(0x00B6)), f, Change());
	}

	// Is there a end-of-paragaph change?
	if (i == end && par.lookupChange(end).changed() && !need_new_row)
		row.needsChangeBar(true);

	// if the row is too large, try to cut at last separator. In case
	// of success, reset indication that the row was broken abruptly.
	int const next_width = max_width_ - leftMargin(row.pit(), row.endpos())
		- rightMargin(row.pit());

	if (row.shortenIfNeeded(body_pos, width, next_width))
		row.flushed(false);
	row.right_boundary(!row.empty() && row.endpos() < end
	                   && row.back().endpos == row.endpos());
	// Last row in paragraph is flushed
	if (row.endpos() == end)
		row.flushed(true);

	// make sure that the RTL elements are in reverse ordering
	row.reverseRTL(is_rtl);
	//LYXERR0("breakrow: row is " << row);

	return need_new_row;
}

int TextMetrics::parTopSpacing(pit_type const pit) const
{
	Paragraph const & par = text_->getPar(pit);
	Layout const & layout = par.layout();

	int asc = 0;
	ParagraphList const & pars = text_->paragraphs();
	double const dh = defaultRowHeight();

	BufferParams const & bparams = bv_->buffer().params();
	Inset const & inset = text_->inset();
	// some parskips VERY EASY IMPLEMENTATION
	if (bparams.paragraph_separation == BufferParams::ParagraphSkipSeparation
		&& !inset.getLayout().parbreakIsNewline()
		&& !par.layout().parbreak_is_newline
		&& pit > 0
		&& ((layout.isParagraph() && par.getDepth() == 0)
		    || (pars[pit - 1].layout().isParagraph()
		        && pars[pit - 1].getDepth() == 0))) {
		asc += bparams.getDefSkip().inPixels(*bv_);
	}

	if (par.params().startOfAppendix())
		asc += int(3 * dh);

	// special code for the top label
	if (layout.labelIsAbove()
	    && (!layout.isParagraphGroup() || text_->isFirstInSequence(pit))
	    && !par.labelString().empty()) {
		FontInfo labelfont = text_->labelFont(par);
		FontMetrics const & lfm = theFontMetrics(labelfont);
		asc += int(lfm.maxHeight() * layout.spacing.getValue()
		                           * text_->spacing(par)
		           + (layout.topsep + layout.labelbottomsep) * dh);
	}

	// Add the layout spaces, for example before and after
	// a section, or between the items of a itemize or enumerate
	// environment.

	pit_type prev = text_->depthHook(pit, par.getDepth());
	Paragraph const & prevpar = pars[prev];
	double layoutasc = 0;
	if (prev != pit
	    && prevpar.layout() == layout
	    && prevpar.getDepth() == par.getDepth()
	    && prevpar.getLabelWidthString() == par.getLabelWidthString()) {
		layoutasc = layout.itemsep * dh;
	} else if (pit != 0 && layout.topsep > 0)
		layoutasc = layout.topsep * dh;

	asc += int(layoutasc * 2 / (2 + pars[pit].getDepth()));

	prev = text_->outerHook(pit);
	if (prev != pit_type(pars.size())) {
		asc += int(pars[prev].layout().parsep * dh);
	} else if (pit != 0) {
		Paragraph const & prevpar = pars[pit - 1];
		if (prevpar.getDepth() != 0 || prevpar.layout() == layout)
			asc += int(layout.parsep * dh);
	}

	return asc;
}


int TextMetrics::parBottomSpacing(pit_type const pit) const
{
	double layoutdesc = 0;
	ParagraphList const & pars = text_->paragraphs();
	double const dh = defaultRowHeight();

	// add the layout spaces, for example before and after
	// a section, or between the items of a itemize or enumerate
	// environment
	pit_type nextpit = pit + 1;
	if (nextpit != pit_type(pars.size())) {
		pit_type cpit = pit;

		if (pars[cpit].getDepth() > pars[nextpit].getDepth()) {
			double usual = pars[cpit].layout().bottomsep * dh;
			double unusual = 0;
			cpit = text_->depthHook(cpit, pars[nextpit].getDepth());
			if (pars[cpit].layout() != pars[nextpit].layout()
				|| pars[nextpit].getLabelWidthString() != pars[cpit].getLabelWidthString())
				unusual = pars[cpit].layout().bottomsep * dh;
			layoutdesc = max(unusual, usual);
		} else if (pars[cpit].getDepth() == pars[nextpit].getDepth()) {
			if (pars[cpit].layout() != pars[nextpit].layout()
				|| pars[nextpit].getLabelWidthString() != pars[cpit].getLabelWidthString())
				layoutdesc = int(pars[cpit].layout().bottomsep * dh);
		}
	}

	return int(layoutdesc * 2 / (2 + pars[pit].getDepth()));
}


void TextMetrics::setRowHeight(Row & row) const
{
	Paragraph const & par = text_->getPar(row.pit());
	Layout const & layout = par.layout();
	double const spacing_val = layout.spacing.getValue() * text_->spacing(par);

	// Initial value for ascent (useful if row is empty).
	Font const font = displayFont(row.pit(), row.pos());
	FontMetrics const & fm = theFontMetrics(font);
	int maxasc = int(fm.maxAscent() * spacing_val);
	int maxdes = int(fm.maxDescent() * spacing_val);

	// Find the ascent/descent of the row contents
	Row::const_iterator cit = row.begin();
	Row::const_iterator cend = row.end();
	for ( ; cit != cend; ++cit) {
		if (cit->inset) {
			maxasc = max(maxasc, cit->dim.ascent());
			maxdes = max(maxdes, cit->dim.descent());
		} else {
			FontMetrics const & fm = theFontMetrics(cit->font);
			maxasc = max(maxasc, int(fm.maxAscent() * spacing_val));
			maxdes = max(maxdes, int(fm.maxDescent() * spacing_val));
		}
	}

	// This is nicer with box insets
	++maxasc;
	++maxdes;

	row.dimension().asc = maxasc;
	row.dimension().des = maxdes;
}


// x is an absolute screen coord
// returns the column near the specified x-coordinate of the row
// x is set to the real beginning of this column
pos_type TextMetrics::getPosNearX(Row const & row, int & x,
				  bool & boundary) const
{
	//LYXERR0("getPosNearX(" << x << ") row=" << row);
	/// For the main Text, it is possible that this pit is not
	/// yet in the CoordCache when moving cursor up.
	/// x Paragraph coordinate is always 0 for main text anyway.
	int const xo = origin_.x_;
	x -= xo;

	// Adapt to cursor row scroll offset if applicable.
	int const offset = bv_->horizScrollOffset(text_, row.pit(), row.pos());
	x += offset;

	pos_type pos = row.pos();
	boundary = false;
	if (row.empty())
		x = row.left_margin;
	else if (x <= row.left_margin) {
		pos = row.front().left_pos();
		x = row.left_margin;
	} else if (x >= row.width()) {
		pos = row.back().right_pos();
		x = row.width();
	} else {
		double w = row.left_margin;
		Row::const_iterator cit = row.begin();
		Row::const_iterator cend = row.end();
		for ( ; cit != cend; ++cit) {
			if (w <= x &&  w + cit->full_width() > x) {
				int x_offset = int(x - w);
				pos = cit->x2pos(x_offset);
				x = int(x_offset + w);
				break;
			}
			w += cit->full_width();
		}
		if (cit == row.end()) {
			pos = row.back().right_pos();
			x = row.width();
		}
		/** This tests for the case where the cursor is placed
		 * just before a font direction change. See comment on
		 * the boundary_ member in DocIterator.h to understand
		 * how boundary helps here.
		 */
		else if (pos == cit->endpos
		         && ((!cit->isRTL() && cit + 1 != row.end()
		              && (cit + 1)->isRTL())
		             || (cit->isRTL() && cit != row.begin()
		                 && !(cit - 1)->isRTL())))
			boundary = true;
	}

	/** This tests for the case where the cursor is set at the end
	 * of a row which has been broken due something else than a
	 * separator (a display inset or a forced breaking of the
	 * row). We know that there is a separator when the end of the
	 * row is larger than the end of its last element.
	 */
	if (!row.empty() && pos == row.back().endpos
	    && row.back().endpos == row.endpos()) {
		Inset const * inset = row.back().inset;
		if (inset && (inset->lyxCode() == NEWLINE_CODE
		              || inset->lyxCode() == SEPARATOR_CODE))
			pos = row.back().pos;
		else
			boundary = row.right_boundary();
	}

	x += xo - offset;
	//LYXERR0("getPosNearX ==> pos=" << pos << ", boundary=" << boundary);

	return pos;
}


pos_type TextMetrics::x2pos(pit_type pit, int row, int x) const
{
	// We play safe and use parMetrics(pit) to make sure the
	// ParagraphMetrics will be redone and OK to use if needed.
	// Otherwise we would use an empty ParagraphMetrics in
	// upDownInText() while in selection mode.
	ParagraphMetrics const & pm = parMetrics(pit);

	LBUFERR(row < int(pm.rows().size()));
	bool bound = false;
	Row const & r = pm.rows()[row];
	return getPosNearX(r, x, bound);
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
	updatePosCache(pit);
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
	updatePosCache(pit);
}

// y is screen coordinate
pit_type TextMetrics::getPitNearY(int y)
{
	LASSERT(!text_->paragraphs().empty(), return -1);
	LASSERT(!par_metrics_.empty(), return -1);
	LYXERR(Debug::DEBUG, "y: " << y << " cache size: " << par_metrics_.size());

	// look for highest numbered paragraph with y coordinate less than given y
	pit_type pit = -1;
	int yy = -1;
	ParMetricsCache::const_iterator it = par_metrics_.begin();
	ParMetricsCache::const_iterator et = par_metrics_.end();
	ParMetricsCache::const_iterator last = et;
	--last;

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
		// the cache (which is in priciple off-screen), that is before the
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


Row const & TextMetrics::getPitAndRowNearY(int & y, pit_type & pit,
	bool assert_in_view, bool up)
{
	ParagraphMetrics const & pm = par_metrics_[pit];

	int yy = pm.position() - pm.ascent();
	LBUFERR(!pm.rows().empty());
	RowList::const_iterator rit = pm.rows().begin();
	RowList::const_iterator rlast = pm.rows().end();
	--rlast;
	for (; rit != rlast; yy += rit->height(), ++rit)
		if (yy + rit->height() > y)
			break;

	if (assert_in_view) {
		if (!up && yy + rit->height() > y) {
			if (rit != pm.rows().begin()) {
				y = yy;
				--rit;
			} else if (pit != 0) {
				--pit;
				newParMetricsUp();
				ParagraphMetrics const & pm2 = par_metrics_[pit];
				rit = pm2.rows().end();
				--rit;
				y = yy;
			}
		} else if (up && yy != y) {
			if (rit != rlast) {
				y = yy + rit->height();
				++rit;
			} else if (pit < int(text_->paragraphs().size()) - 1) {
				++pit;
				newParMetricsDown();
				ParagraphMetrics const & pm2 = par_metrics_[pit];
				rit = pm2.rows().begin();
				y = pm2.position();
			}
		}
	}
	return *rit;
}


// x,y are absolute screen coordinates
// sets cursor recursively descending into nested editable insets
Inset * TextMetrics::editXY(Cursor & cur, int x, int y,
	bool assert_in_view, bool up)
{
	if (lyxerr.debugging(Debug::WORKAREA)) {
		LYXERR0("TextMetrics::editXY(cur, " << x << ", " << y << ")");
		cur.bv().coordCache().dump();
	}
	pit_type pit = getPitNearY(y);
	LASSERT(pit != -1, return 0);
	Row const & row = getPitAndRowNearY(y, pit, assert_in_view, up);
	cur.pit() = pit;

	// Do we cover an inset?
	InsetList::InsetTable * it = checkInsetHit(pit, x, y);

	if (!it) {
		// No inset, set position in the text
		bool bound = false; // is modified by getPosNearX
		cur.pos() = getPosNearX(row, x, bound);
		cur.boundary(bound);
		cur.setCurrentFont();
		cur.setTargetX(x);
		return 0;
	}

	Inset * inset = it->inset;
	//lyxerr << "inset " << inset << " hit at x: " << x << " y: " << y << endl;

	// Set position in front of inset
	cur.pos() = it->pos;
	cur.boundary(false);
	cur.setTargetX(x);

	// Try to descend recursively inside the inset.
	Inset * edited = inset->editXY(cur, x, y);
	// FIXME: it is not clear that the test on position is needed
	// Remove it if/when semantics of editXY is clarified
	if (cur.text() == text_ && cur.pos() == it->pos) {
		// non-editable inset, set cursor after the inset if x is
		// nearer to that position (bug 9628)
		bool bound = false; // is modified by getPosNearX
		cur.pos() = getPosNearX(row, x, bound);
		cur.boundary(bound);
		cur.setCurrentFont();
		cur.setTargetX(x);
	}

	if (cur.top().text() == text_)
		cur.setCurrentFont();
	return edited;
}


void TextMetrics::setCursorFromCoordinates(Cursor & cur, int const x, int const y)
{
	LASSERT(text_ == cur.text(), return);
	pit_type const pit = getPitNearY(y);
	LASSERT(pit != -1, return);

	ParagraphMetrics const & pm = par_metrics_[pit];

	int yy = pm.position() - pm.ascent();
	LYXERR(Debug::DEBUG, "x: " << x << " y: " << y <<
		" pit: " << pit << " yy: " << yy);

	int r = 0;
	LBUFERR(pm.rows().size());
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
	pos_type const pos = getPosNearX(row, xx, bound);

	LYXERR(Debug::DEBUG, "setting cursor pit: " << pit << " pos: " << pos);

	text_->setCursor(cur, pit, pos, true, bound);
	// remember new position.
	cur.setTargetX();
}


//takes screen x,y coordinates
InsetList::InsetTable * TextMetrics::checkInsetHit(pit_type pit, int x, int y)
{
	Paragraph const & par = text_->paragraphs()[pit];
	CoordCache::Insets const & insetCache = bv_->coordCache().getInsets();

	LYXERR(Debug::DEBUG, "x: " << x << " y: " << y << "  pit: " << pit);

	for (auto const & it : par.insetList()) {
		LYXERR(Debug::DEBUG, "examining inset " << it.inset);

		if (insetCache.covers(it.inset, x, y)) {
			LYXERR(Debug::DEBUG, "Hit inset: " << it.inset);
			return const_cast<InsetList::InsetTable *>(&it);
		}
	}

	LYXERR(Debug::DEBUG, "No inset hit. ");
	return 0;
}


//takes screen x,y coordinates
Inset * TextMetrics::checkInsetHit(int x, int y)
{
	pit_type const pit = getPitNearY(y);
	LASSERT(pit != -1, return 0);
	InsetList::InsetTable * it = checkInsetHit(pit, x, y);

	if (!it)
		return 0;

	return it->inset;
}


int TextMetrics::cursorX(CursorSlice const & sl,
		bool boundary) const
{
	LASSERT(sl.text() == text_, return 0);

	ParagraphMetrics const & pm = par_metrics_[sl.pit()];
	if (pm.rows().empty())
		return 0;
	Row const & row = pm.getRow(sl.pos(), boundary);
	pos_type const pos = sl.pos();

	double x = 0;
	row.findElement(pos, boundary, x);
	return int(x);

}


int TextMetrics::cursorY(CursorSlice const & sl, bool boundary) const
{
	//lyxerr << "TextMetrics::cursorY: boundary: " << boundary << endl;
	ParagraphMetrics const & pm = parMetrics(sl.pit());
	if (pm.rows().empty())
		return 0;

	int h = 0;
	h -= parMetrics(0).rows()[0].ascent();
	for (pit_type pit = 0; pit < sl.pit(); ++pit) {
		h += parMetrics(pit).height();
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
	LASSERT(text_ == cur.text(), return false);
	ParagraphMetrics const & pm = par_metrics_[cur.pit()];
	Row const & row = pm.getRow(cur.pos(),cur.boundary());
	return text_->setCursor(cur, cur.pit(), row.pos());
}


bool TextMetrics::cursorEnd(Cursor & cur)
{
	LASSERT(text_ == cur.text(), return false);
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
		    && !cur.paragraph().isNewline(end-1)
		    && !cur.paragraph().isEnvSeparator(end-1))
			boundary = true;
		else
			--end;
	} else if (cur.paragraph().isEnvSeparator(end-1))
		--end;
	return text_->setCursor(cur, cur.pit(), end, true, boundary);
}


void TextMetrics::deleteLineForward(Cursor & cur)
{
	LASSERT(text_ == cur.text(), return);
	if (cur.lastpos() == 0) {
		// Paragraph is empty, so we just go forward
		text_->cursorForward(cur);
	} else {
		cur.resetAnchor();
		cur.selection(true); // to avoid deletion
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


bool TextMetrics::isLastRow(Row const & row) const
{
	ParagraphList const & pars = text_->paragraphs();
	return row.endpos() >= pars[row.pit()].size()
		&& row.pit() + 1 == pit_type(pars.size());
}


bool TextMetrics::isFirstRow(Row const & row) const
{
	return row.pos() == 0 && row.pit() == 0;
}


int TextMetrics::leftMargin(pit_type pit) const
{
	return leftMargin(pit, text_->paragraphs()[pit].size());
}


int TextMetrics::leftMargin(pit_type const pit, pos_type const pos) const
{
	ParagraphList const & pars = text_->paragraphs();

	LASSERT(pit >= 0, return 0);
	LASSERT(pit < int(pars.size()), return 0);
	Paragraph const & par = pars[pit];
	LASSERT(pos >= 0, return 0);
	LASSERT(pos <= par.size(), return 0);
	Buffer const & buffer = bv_->buffer();
	//lyxerr << "TextMetrics::leftMargin: pit: " << pit << " pos: " << pos << endl;
	DocumentClass const & tclass = buffer.params().documentClass();
	Layout const & layout = par.layout();
	FontMetrics const & bfm = theFontMetrics(buffer.params().getFont());

	docstring parindent = layout.parindent;

	int l_margin = 0;

	if (text_->isMainText())
		l_margin += bv_->leftMargin();

	l_margin += bfm.signedWidth(tclass.leftmargin());

	int depth = par.getDepth();
	if (depth != 0) {
		// find the next level paragraph
		pit_type newpar = text_->outerHook(pit);
		if (newpar != pit_type(pars.size())) {
			if (pars[newpar].layout().isEnvironment()) {
				int nestmargin = depth * nestMargin();
				if (text_->isMainText())
					nestmargin += changebarMargin();
				l_margin = max(leftMargin(newpar), nestmargin);
				// Remove the parindent that has been added
				// if the paragraph was empty.
				if (pars[newpar].empty() &&
				    buffer.params().paragraph_separation ==
				    BufferParams::ParagraphIndentSeparation) {
					docstring pi = pars[newpar].layout().parindent;
					l_margin -= bfm.signedWidth(pi);
				}
			}
			if (tclass.isDefaultLayout(par.layout())
			    || tclass.isPlainLayout(par.layout())) {
				if (pars[newpar].params().noindent())
					parindent.erase();
				else
					parindent = pars[newpar].layout().parindent;
			}
		}
	}

	// This happens after sections or environments in standard classes.
	// We have to check the previous layout at same depth.
	if (buffer.params().paragraph_separation ==
			BufferParams::ParagraphSkipSeparation)
		parindent.erase();
	else if (pit > 0 && pars[pit - 1].getDepth() >= par.getDepth()) {
		pit_type prev = text_->depthHook(pit, par.getDepth());
		if (par.layout() == pars[prev].layout()) {
			if (prev != pit - 1
			    && pars[pit - 1].layout().nextnoindent)
				parindent.erase();
		} else if (pars[prev].layout().nextnoindent)
			parindent.erase();
	}

	FontInfo const labelfont = text_->labelFont(par);
	FontMetrics const & lfm = theFontMetrics(labelfont);

	switch (layout.margintype) {
	case MARGIN_DYNAMIC:
		if (!layout.leftmargin.empty()) {
			l_margin += bfm.signedWidth(layout.leftmargin);
		}
		if (!par.labelString().empty()) {
			l_margin += lfm.signedWidth(layout.labelindent);
			l_margin += lfm.width(par.labelString());
			l_margin += lfm.width(layout.labelsep);
		}
		break;

	case MARGIN_MANUAL: {
		l_margin += lfm.signedWidth(layout.labelindent);
		// The width of an empty par, even with manual label, should be 0
		if (!par.empty() && pos >= par.beginOfBody()) {
			if (!par.getLabelWidthString().empty()) {
				docstring labstr = par.getLabelWidthString();
				l_margin += lfm.width(labstr);
				l_margin += lfm.width(layout.labelsep);
			}
		}
		break;
	}

	case MARGIN_STATIC: {
		l_margin += bfm.signedWidth(layout.leftmargin) * 4
		             / (par.getDepth() + 4);
		break;
	}

	case MARGIN_FIRST_DYNAMIC:
		if (layout.labeltype == LABEL_MANUAL) {
			// if we are at position 0, we are never in the body
			if (pos > 0 && pos >= par.beginOfBody())
				l_margin += lfm.signedWidth(layout.leftmargin);
			else
				l_margin += lfm.signedWidth(layout.labelindent);
		} else if (pos != 0
			   // Special case to fix problems with
			   // theorems (JMarc)
			   || (layout.labeltype == LABEL_STATIC
			       && layout.latextype == LATEX_ENVIRONMENT
			       && !text_->isFirstInSequence(pit))) {
			l_margin += lfm.signedWidth(layout.leftmargin);
		} else if (!layout.labelIsAbove()) {
			l_margin += lfm.signedWidth(layout.labelindent);
			l_margin += lfm.width(layout.labelsep);
			l_margin += lfm.width(par.labelString());
		}
		break;

	case MARGIN_RIGHT_ADDRESS_BOX: {
#if 0
		// The left margin depends on the widest row in this paragraph.
		// This code is wrong because it depends on the rows, but at the
		// same time this function is used in redoParagraph to construct
		// the rows.
		ParagraphMetrics const & pm = par_metrics_[pit];
		RowList::const_iterator rit = pm.rows().begin();
		RowList::const_iterator end = pm.rows().end();
		int minfill = max_width_;
		for ( ; rit != end; ++rit)
			if (rit->fill() < minfill)
				minfill = rit->fill();
		l_margin += bfm.signedWidth(layout.leftmargin);
		l_margin += minfill;
#endif
		// also wrong, but much shorter.
		l_margin += max_width_ / 2;
		break;
	}
	}

	if (!par.params().leftIndent().zero())
		l_margin += par.params().leftIndent().inPixels(max_width_, lfm.em());

	LyXAlignment align = par.getAlign();

	// set the correct parindent
	if (pos == 0
	    && (layout.labeltype == LABEL_NO_LABEL
	        || layout.labeltype == LABEL_ABOVE
	        || layout.labeltype == LABEL_CENTERED
	        || (layout.labeltype == LABEL_STATIC
	            && layout.latextype == LATEX_ENVIRONMENT
	            && !text_->isFirstInSequence(pit)))
	    && (align == LYX_ALIGN_BLOCK || align == LYX_ALIGN_LEFT)
	    && !par.params().noindent()
	    // in some insets, paragraphs are never indented
	    && !text_->inset().neverIndent()
	    // display style insets are always centered, omit indentation
	    && !(!par.empty()
	         && par.isInset(pos)
	         && par.getInset(pos)->display())
	    && (!(tclass.isDefaultLayout(par.layout())
	        || tclass.isPlainLayout(par.layout()))
	        || buffer.params().paragraph_separation
				== BufferParams::ParagraphIndentSeparation)) {
		/* use the parindent of the layout when the default
		 * indentation is used otherwise use the indentation set in
		 * the document settings
		 */
		if (buffer.params().getParIndent().empty())
			l_margin += bfm.signedWidth(parindent);
		else
			l_margin += buffer.params().getParIndent().inPixels(max_width_, bfm.em());
	}

	return l_margin;
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


void TextMetrics::drawParagraph(PainterInfo & pi, pit_type const pit, int const x, int y) const
{
	ParagraphMetrics const & pm = par_metrics_[pit];
	if (pm.rows().empty())
		return;
	size_t const nrows = pm.rows().size();

	// Use fast lane in nodraw stage.
	if (pi.pain.isNull()) {
		for (size_t i = 0; i != nrows; ++i) {

			Row const & row = pm.rows()[i];
			// Adapt to cursor row scroll offset if applicable.
			int row_x = x - bv_->horizScrollOffset(text_, pit, row.pos());
			if (i)
				y += row.ascent();

			RowPainter rp(pi, *text_, row, row_x, y);

			rp.paintOnlyInsets();
			y += row.descent();
		}
		return;
	}

	int const ww = bv_->workHeight();
	Cursor const & cur = bv_->cursor();
	DocIterator sel_beg = cur.selectionBegin();
	DocIterator sel_end = cur.selectionEnd();
	bool selection = cur.selection()
		// This is our text.
		&& cur.text() == text_
		// if the anchor is outside, this is not our selection
		&& cur.normalAnchor().text() == text_
		&& pit >= sel_beg.pit() && pit <= sel_end.pit();

	// We store the begin and end pos of the selection relative to this par
	DocIterator sel_beg_par = cur.selectionBegin();
	DocIterator sel_end_par = cur.selectionEnd();

	// We care only about visible selection.
	if (selection) {
		if (pit != sel_beg.pit()) {
			sel_beg_par.pit() = pit;
			sel_beg_par.pos() = 0;
		}
		if (pit != sel_end.pit()) {
			sel_end_par.pit() = pit;
			sel_end_par.pos() = sel_end_par.lastpos();
		}
	}

	for (size_t i = 0; i != nrows; ++i) {

		Row const & row = pm.rows()[i];
		// Adapt to cursor row scroll offset if applicable.
		int row_x = x - bv_->horizScrollOffset(text_, pit, row.pos());
		if (i)
			y += row.ascent();

		// It is not needed to draw on screen if we are not inside.
		bool const inside = (y + row.descent() >= 0
			&& y - row.ascent() < ww);
		if (!inside) {
			// Inset positions have already been set in nodraw stage.
			y += row.descent();
			continue;
		}

		if (selection)
			row.setSelectionAndMargins(sel_beg_par, sel_end_par);
		else
			row.setSelection(-1, -1);

		// The row knows nothing about the paragraph, so we have to check
		// whether this row is the first or last and update the margins.
		if (row.selection()) {
			if (row.sel_beg == 0)
				row.change(row.begin_margin_sel, sel_beg.pit() < pit);
			if (row.sel_end == sel_end_par.lastpos())
				row.change(row.end_margin_sel, sel_end.pit() > pit);
		}

		// has row changed since last paint?
		bool row_has_changed = row.changed()
			|| bv_->hadHorizScrollOffset(text_, pit, row.pos());

		// Take this opportunity to spellcheck the row contents.
		if (row_has_changed && pi.do_spellcheck && lyxrc.spellcheck_continuously) {
			text_->getPar(pit).spellCheck();
		}

		RowPainter rp(pi, *text_, row, row_x, y);

		// Don't paint the row if a full repaint has not been requested
		// and if it has not changed.
		if (!pi.full_repaint && !row_has_changed) {
			// Paint only the insets if the text itself is
			// unchanged.
			rp.paintOnlyInsets();
			row.changed(false);
			y += row.descent();
			continue;
		}

		// Clear background of this row if paragraph background was not
		// already cleared because of a full repaint.
		if (!pi.full_repaint && row_has_changed) {
			LYXERR(Debug::PAINTING, "Clear rect@("
			       << max(row_x, 0) << ", " << y - row.ascent() << ")="
			       << width() << " x " << row.height());
			// FIXME: this is a hack. We know that at least this
			// amount of pixels can be cleared on right and left.
			// Doing so gets rid of caret ghosts when the cursor is at
			// the begining/end of row. However, it will not work if
			// the caret has a ridiculous width like 6. (see ticket
			// #10797)
			pi.pain.fillRectangle(max(row_x, 0) - Inset::TEXT_TO_INSET_OFFSET,
			                      y - row.ascent(),
			                      width() + 2 * Inset::TEXT_TO_INSET_OFFSET,
			                      row.height(), pi.background_color);
		}

		// Instrumentation for testing row cache (see also
		// 12 lines lower):
		if (lyxerr.debugging(Debug::PAINTING)
		    && (row.selection() || pi.full_repaint || row_has_changed)) {
			string const foreword = text_->isMainText() ? "main text redraw "
				: "inset text redraw: ";
			LYXERR0(foreword << "pit=" << pit << " row=" << i
			        << (row.selection() ? " row_selection": "")
			        << (pi.full_repaint ? " full_repaint" : "")
			        << (row_has_changed ? " row_has_changed" : ""));
		}

		// Backup full_repaint status and force full repaint
		// for inner insets as the Row has been cleared out.
		bool tmp = pi.full_repaint;
		pi.full_repaint = true;

		rp.paintSelection();
		rp.paintAppendix();
		rp.paintDepthBar();
		if (row.needsChangeBar())
			rp.paintChangeBar();
		if (i == 0 && !row.isRTL())
			rp.paintFirst();
		if (i == nrows - 1 && row.isRTL())
			rp.paintLast();
		rp.paintText();
		if (i == nrows - 1 && !row.isRTL())
			rp.paintLast();
		if (i == 0 && row.isRTL())
			rp.paintFirst();
		rp.paintTooLargeMarks(row_x + row.left_x() < 0,
				      row_x + row.right_x() > bv_->workWidth());
		y += row.descent();

#if 0
		// This debug code shows on screen which rows are repainted.
		// FIXME: since the updates related to caret blinking restrict
		// the painter to a small rectangle, the numbers are not
		// updated when this happens. Change the code in
		// GuiWorkArea::Private::show/hideCaret if this is important.
		static int count = 0;
		++count;
		FontInfo fi(sane_font);
		fi.setSize(FONT_SIZE_TINY);
		fi.setColor(Color_red);
		pi.pain.text(row_x, y, convert<docstring>(count), fi);
#endif

		// Restore full_repaint status.
		pi.full_repaint = tmp;

		row.changed(false);
	}

	//LYXERR(Debug::PAINTING, ".");
}


void TextMetrics::completionPosAndDim(Cursor const & cur, int & x, int & y,
	Dimension & dim) const
{
	Cursor const & bvcur = cur.bv().cursor();

	// get word in front of cursor
	docstring word = text_->previousWord(bvcur.top());
	DocIterator wordStart = bvcur;
	wordStart.pos() -= word.length();

	// calculate dimensions of the word
	Row row;
	row.pit(bvcur.pit());
	row.pos(wordStart.pos());
	row.endpos(bvcur.pos());
	setRowHeight(row);
	dim = row.dimension();

	// get position on screen of the word start and end
	//FIXME: Is it necessary to explicitly set this to false?
	wordStart.boundary(false);
	Point lxy = cur.bv().getPos(wordStart);
	Point rxy = cur.bv().getPos(bvcur);
	dim.wid = abs(rxy.x_ - lxy.x_);

	// calculate position of word
	y = lxy.y_;
	x = min(rxy.x_, lxy.x_);

	//lyxerr << "wid=" << dim.width() << " x=" << x << " y=" << y << " lxy.x_=" << lxy.x_ << " rxy.x_=" << rxy.x_ << " word=" << word << std::endl;
	//lyxerr << " wordstart=" << wordStart << " bvcur=" << bvcur << " cur=" << cur << std::endl;
}

int defaultRowHeight()
{
	return int(theFontMetrics(sane_font).maxHeight() *  1.2);
}

} // namespace lyx
