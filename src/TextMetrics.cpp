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
#include "HSpace.h"
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

#include "mathed/MathMacroTemplate.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/lassert.h"

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


int numberOfHfills(Row const & row, pos_type const body_pos)
{
	int n = 0;
	Row::const_iterator cit = row.begin();
	Row::const_iterator const end = row.end();
	for ( ; cit != end ; ++cit)
		if (cit->pos >= body_pos
		    && cit->inset && cit->inset->isHfill())
			++n;
	return n;
}


}

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
	main_text_ = (text_ == &buffer.text());
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

		// do the metric calculation
		Dimension dim;
		int const w = max_width_ - leftMargin(max_width_, pit, ii->pos)
			- right_margin;
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

	par.setBeginOfBody();
	pos_type first = 0;
	size_t row_index = 0;
	// maximum pixel width of a row
	do {
		if (row_index == pm.rows().size())
			pm.rows().push_back(Row());
		Row & row = pm.rows()[row_index];
		row.pos(first);
		breakRow(row, right_margin, pit);
		setRowHeight(row, pit);
		row.setChanged(false);
		if (row_index || row.right_boundary() || row.endpos() < par.size())
			// If there is more than one row or the row has been
			// broken by a display inset or a newline, expand the text
			// to the full allowable width. This setting here is
			// needed for the computeRowMetrics() below.
			dim_.wid = max_width_;
		int const max_row_width = max(dim_.wid, row.width());
		computeRowMetrics(pit, row, max_row_width);
		first = row.endpos();
		++row_index;

		pm.dim().wid = max(pm.dim().wid, row.width());
		pm.dim().des += row.height();
	} while (first < par.size());

	if (row_index < pm.rows().size())
		pm.rows().resize(row_index);

	// Make sure that if a par ends in newline, there is one more row
	// under it
	if (first > 0 && par.isNewline(first - 1)) {
		if (row_index == pm.rows().size())
			pm.rows().push_back(Row());
		Row & row = pm.rows()[row_index];
		row.pos(first);
		row.endpos(first);
		setRowHeight(row, pit);
		row.setChanged(false);
		int const max_row_width = max(dim_.wid, row.width());
		computeRowMetrics(pit, row, max_row_width);
		pm.dim().des += row.height();
	}

	pm.dim().asc += pm.rows()[0].ascent();
	pm.dim().des -= pm.rows()[0].ascent();

	changed |= old_dim.height() != pm.dim().height();

	return changed;
}


LyXAlignment TextMetrics::getAlign(Paragraph const & par, pos_type const pos) const
{
	Layout const & layout = par.layout();

	LyXAlignment align;
	if (par.params().align() == LYX_ALIGN_LAYOUT)
		align = layout.align;
	else
		align = par.params().align();

	// handle alignment inside tabular cells
	Inset const & owner = text_->inset();
	switch (owner.contentAlignment()) {
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
	if (Inset const * inset = par.getInset(pos)) {
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

	// Has the user requested we not justify stuff?
	if (!bv_->buffer().params().justification
	    && align == LYX_ALIGN_BLOCK)
		align = LYX_ALIGN_LEFT;

	return align;
}


void TextMetrics::computeRowMetrics(pit_type const pit,
		Row & row, int width) const
{
	row.label_hfill = 0;
	row.separator = 0;

	Paragraph const & par = text_->getPar(pit);

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
			row.label_hfill = labelFill(pit, row) / double(nlh);
	}

	double hfill = 0;
	// are there any hfills in the row?
	if (int const nh = numberOfHfills(row, par.beginOfBody())) {
		if (w > 0)
			hfill = double(w) / nh;
	// we don't have to look at the alignment if it is ALIGN_LEFT and
	// if the row is already larger then the permitted width as then
	// we force the LEFT_ALIGN'edness!
	} else if (int(row.width()) < max_width_) {
		// is it block, flushleft or flushright?
		// set x how you need it
		switch (getAlign(par, row.pos())) {
		case LYX_ALIGN_BLOCK: {
			int const ns = row.countSeparators();
			/** If we have separators, and this row has
			 * not be broken abruptly by a display inset
			 * or newline, then stretch it */
			if (ns && !row.right_boundary()
			    && row.endpos() != par.size()) {
				row.setSeparatorExtraWidth(double(w) / ns);
				row.dimension().wid = width;
			} else if (text_->isRTL(par)) {
				row.dimension().wid = width;
				row.left_margin += w;
			}
			break;
		}
		case LYX_ALIGN_RIGHT:
			row.left_margin += w;
			row.dimension().wid += w;
			break;
		case LYX_ALIGN_CENTER:
			row.dimension().wid = width - w / 2;
			row.left_margin += w / 2;
			break;
		case LYX_ALIGN_LEFT:
		case LYX_ALIGN_NONE:
		case LYX_ALIGN_LAYOUT:
		case LYX_ALIGN_SPECIAL:
		case LYX_ALIGN_DECIMAL:
			break;
		}
	}

	// Finally,  handle hfill insets
	pos_type const endpos = row.endpos();
	pos_type body_pos = par.beginOfBody();
	if (body_pos > 0
	    && (body_pos > endpos || !par.isLineSeparator(body_pos - 1)))
		body_pos = 0;
	ParagraphMetrics & pm = par_metrics_[pit];
	CoordCache::Insets & insetCache = bv_->coordCache().insets();
	Row::iterator cit = row.begin();
	Row::iterator const cend = row.end();
	for ( ; cit != cend; ++cit) {
		if (row.label_hfill && cit->endpos == body_pos
		    && cit->type == Row::SPACE)
			cit->dim.wid -= int(row.label_hfill * (nlh - 1));
		if (!cit->inset || !cit->inset->isHfill())
			continue;
		if (pm.hfillExpansion(row, cit->pos))
			cit->dim.wid = int(cit->pos >= body_pos ?
					   max(hfill, 5.0) : row.label_hfill);
		else
			cit->dim.wid = 5;
		// Cache the inset dimension.
		insetCache.add(cit->inset, cit->dim);
	}
}


int TextMetrics::labelFill(pit_type const pit, Row const & row) const
{
	Paragraph const & par = text_->getPar(pit);
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

} // anon namespace

/** This is the function where the hard work is done. The code here is
 * very sensitive to small changes :) Note that part of the
 * intelligence is also in Row::shortenIfNeeded.
 */
void TextMetrics::breakRow(Row & row, int const right_margin, pit_type const pit) const
{
	Paragraph const & par = text_->getPar(pit);
	pos_type const end = par.size();
	pos_type const pos = row.pos();
	pos_type const body_pos = par.beginOfBody();
	bool const is_rtl = text_->isRTL(par);

	row.clear();
	row.left_margin = leftMargin(max_width_, pit, pos);
	row.right_margin = right_margin;
	if (is_rtl)
		swap(row.left_margin, row.right_margin);
	// Remember that the row width takes into account the left_margin
	// but not the right_margin.
	row.dimension().wid = row.left_margin;
	// the width available for the row.
	int const width = max_width_ - row.right_margin;

	if (pos >= end || row.width() > width) {
		row.endpos(end);
		return;
	}

	ParagraphList const & pars = text_->paragraphs();

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
	if (ic_it.inTexted() && ic_it.text() == text_ && ic_it.pit() == pit)
		ic_pos = ic_it.pos();

	// Now we iterate through until we reach the right margin
	// or the end of the par, then build a representation of the row.
	pos_type i = pos;
	FontIterator fi = FontIterator(*this, par, pit, pos);
	while (i < end && row.width() <= width) {
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
					    labelEnd(pit) - row.width());
			row.addSpace(i, add, *fi, par.lookupChange(i));
		} else if (c == '\t')
			row.addSpace(i, theFontMetrics(*fi).width(from_ascii("    ")),
				     *fi, par.lookupChange(i));
		else {
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
			row.right_boundary(true);
			++i;
			break;
		}

		++i;
		++fi;
	}
	row.finalizeLast();
	row.endpos(i);

	// End of paragraph marker
	if (lyxrc.paragraph_markers
	    && i == end && size_type(pit + 1) < pars.size()) {
		// add a virtual element for the end-of-paragraph
		// marker; it is shown on screen, but does not exist
		// in the paragraph.
		Font f(text_->layoutFont(pit));
		f.fontInfo().setColor(Color_paragraphmarker);
		BufferParams const & bparams
			= text_->inset().buffer().params();
		f.setLanguage(par.getParLanguage(bparams));
		row.addVirtual(end, docstring(1, char_type(0x00B6)), f, Change());
	}

	// if the row is too large, try to cut at last separator.
	row.shortenIfNeeded(body_pos, width);

	// make sure that the RTL elements are in reverse ordering
	row.reverseRTL(is_rtl);
	//LYXERR0("breakrow: row is " << row);
}


void TextMetrics::setRowHeight(Row & row, pit_type const pit,
				    bool topBottomSpace) const
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
	Font font = displayFont(pit, row.pos());
	FontSize const tmpsize = font.fontInfo().size();
	font.fontInfo() = text_->layoutFont(pit);
	FontSize const size = font.fontInfo().size();
	font.fontInfo().setSize(tmpsize);

	FontInfo labelfont = text_->labelFont(par);

	FontMetrics const & labelfont_metrics = theFontMetrics(labelfont);
	FontMetrics const & fontmetrics = theFontMetrics(font);

	// these are minimum values
	double const spacing_val = layout.spacing.getValue()
		* text_->spacing(par);
	//lyxerr << "spacing_val = " << spacing_val << endl;
	int maxasc  = int(fontmetrics.maxAscent()  * spacing_val);
	int maxdesc = int(fontmetrics.maxDescent() * spacing_val);

	// insets may be taller
	CoordCache::Insets const & insetCache = bv_->coordCache().getInsets();
	Row::const_iterator cit = row.begin();
	Row::const_iterator cend = row.end();
	for ( ; cit != cend; ++cit) {
		if (cit->inset) {
			Dimension const & dim = insetCache.dim(cit->inset);
			maxasc  = max(maxasc,  dim.ascent());
			maxdesc = max(maxdesc, dim.descent());
		}
	}

	// Check if any custom fonts are larger (Asger)
	// This is not completely correct, but we can live with the small,
	// cosmetic error for now.
	int labeladdon = 0;

	FontSize maxsize =
		par.highestFontInRange(row.pos(), row.endpos(), size);
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
	Inset const & inset = text_->inset();

	// is it a top line?
	if (row.pos() == 0 && topBottomSpace) {
		BufferParams const & bufparams = buffer.params();
		// some parskips VERY EASY IMPLEMENTATION
		if (bufparams.paragraph_separation == BufferParams::ParagraphSkipSeparation
		    && !inset.getLayout().parbreakIsNewline()
		    && !par.layout().parbreak_is_newline
		    && pit > 0
		    && ((layout.isParagraph() && par.getDepth() == 0)
			|| (pars[pit - 1].layout().isParagraph()
			    && pars[pit - 1].getDepth() == 0))) {
			maxasc += bufparams.getDefSkip().inPixels(*bv_);
		}

		if (par.params().startOfAppendix())
			maxasc += int(3 * dh);

		// special code for the top label
		if (layout.labelIsAbove()
		    && (!layout.isParagraphGroup() || text_->isFirstInSequence(pit))
		    && !par.labelString().empty()) {
			labeladdon = int(
				  labelfont_metrics.maxHeight()
					* layout.spacing.getValue()
					* text_->spacing(par)
				+ (layout.topsep + layout.labelbottomsep) * dh);
		}

		// Add the layout spaces, for example before and after
		// a section, or between the items of a itemize or enumerate
		// environment.

		pit_type prev = text_->depthHook(pit, par.getDepth());
		Paragraph const & prevpar = pars[prev];
		if (prev != pit
		    && prevpar.layout() == layout
		    && prevpar.getDepth() == par.getDepth()
		    && prevpar.getLabelWidthString()
					== par.getLabelWidthString()) {
			layoutasc = layout.itemsep * dh;
		} else if (pit != 0 || row.pos() != 0) {
			if (layout.topsep > 0)
				layoutasc = layout.topsep * dh;
		}

		prev = text_->outerHook(pit);
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
	if (row.endpos() >= par.size() && topBottomSpace) {
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
	}

	// incalculate the layout spaces
	maxasc  += int(layoutasc  * 2 / (2 + pars[pit].getDepth()));
	maxdesc += int(layoutdesc * 2 / (2 + pars[pit].getDepth()));

	// FIXME: the correct way is to do the following is to move the
	// following code in another method specially tailored for the
	// main Text. The following test is thus bogus.
	// Top and bottom margin of the document (only at top-level)
	if (main_text_ && topBottomSpace) {
		if (pit == 0 && row.pos() == 0)
			maxasc += 20;
		if (pit + 1 == pit_type(pars.size()) &&
		    row.endpos() == par.size() &&
				!(row.endpos() > 0 && par.isNewline(row.endpos() - 1)))
			maxdesc += 20;
	}

	row.dimension().asc = maxasc + labeladdon;
	row.dimension().des = maxdesc;
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
			 && cit + 1 != row.end()
			 && cit->isRTL() != (cit + 1)->isRTL())
			boundary = true;
	}

	/** This tests for the case where the cursor is set at the end
	 * of a row which has been broken due something else than a
	 * separator (a display inset or a forced breaking of the
	 * row). We know that there is a separator when the end of the
	 * row is larger than the end of its last element.
	 */
	if (!row.empty() && pos == row.back().endpos
	    && row.back().endpos == row.endpos())
		boundary = true;

	x += xo;
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

	int yy = y; // is modified by getPitAndRowNearY
	Row const & row = getPitAndRowNearY(yy, pit, assert_in_view, up);

	cur.pit() = pit;

	// Do we cover an inset?
	InsetList::InsetTable * it = checkInsetHit(pit, x, yy);

	if (!it) {
		// No inset, set position in the text
		bool bound = false; // is modified by getPosNearX
		int xx = x; // is modified by getPosNearX
		cur.pos() = getPosNearX(row, xx, bound);
		cur.boundary(bound);
		cur.setCurrentFont();
		cur.setTargetX(xx);
		return 0;
	}

	Inset * inset = it->inset;
	//lyxerr << "inset " << inset << " hit at x: " << x << " y: " << y << endl;

	// Set position in front of inset
	cur.pos() = it->pos;
	cur.boundary(false);
	cur.setTargetX(x);

	// Try to descend recursively inside the inset.
	Inset * edited = inset->editXY(cur, x, yy);
	if (edited == inset && cur.pos() == it->pos) {
		// non-editable inset, set cursor after the inset if x is
		// nearer to that position (bug 9628)
		CoordCache::Insets const & insetCache = bv_->coordCache().getInsets();
		Dimension const & dim = insetCache.dim(inset);
		Point p = insetCache.xy(inset);
		bool const is_rtl = text_->isRTL(text_->getPar(pit));
		if (is_rtl) {
			// "in front of" == "right of"
			if (abs(p.x_ - x) < abs(p.x_ + dim.wid - x))
				cur.posForward();
		} else {
			// "in front of" == "left of"
			if (abs(p.x_ + dim.wid - x) < abs(p.x_ - x))
				cur.posForward();
		}
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

	InsetList::const_iterator iit = par.insetList().begin();
	InsetList::const_iterator iend = par.insetList().end();
	for (; iit != iend; ++iit) {
		Inset * inset = iit->inset;

		LYXERR(Debug::DEBUG, "examining inset " << inset);

		if (!insetCache.has(inset)) {
			LYXERR(Debug::DEBUG, "inset has no cached position");
			return 0;
		}

		Dimension const & dim = insetCache.dim(inset);
		Point p = insetCache.xy(inset);

		LYXERR(Debug::DEBUG, "xo: " << p.x_ << "..." << p.x_ + dim.wid
			<< " yo: " << p.y_ - dim.asc << "..." << p.y_ + dim.des);

		if (x >= p.x_ && x <= p.x_ + dim.wid
		    && y >= p.y_ - dim.asc && y <= p.y_ + dim.des) {
			LYXERR(Debug::DEBUG, "Hit inset: " << inset);
			return const_cast<InsetList::InsetTable *>(&(*iit));
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


Row::const_iterator const
TextMetrics::findRowElement(Row const & row, pos_type const pos,
                            bool const boundary, double & x) const
{
	/**
	 * When boundary is true, position i is in the row element (pos, endpos)
	 * if
	 *    pos < i <= endpos
	 * whereas, when boundary is false, the test is
	 *    pos <= i < endpos
	 * The correction below allows to handle both cases.
	*/
	int const boundary_corr = (boundary && pos) ? -1 : 0;

	x = row.left_margin;

	/** Early return in trivial cases
	 * 1) the row is empty
	 * 2) the position is the left-most position of the row; there
	 * is a quirk here however: if the first element is virtual
	 * (end-of-par marker for example), then we have to look
	 * closer
	 */
	if (row.empty()
	    || (pos == row.begin()->left_pos() && !boundary
			&& !row.begin()->isVirtual()))
		return row.begin();

	Row::const_iterator cit = row.begin();
	for ( ; cit != row.end() ; ++cit) {
		/** Look whether the cursor is inside the element's
		 * span. Note that it is necessary to take the
		 * boundary into account, and to accept virtual
		 * elements, which have pos == endpos.
		 */
		if (pos + boundary_corr >= cit->pos
		    && (pos + boundary_corr < cit->endpos || cit->isVirtual())) {
				x += cit->pos2x(pos);
				break;
		}
		x += cit->full_width();
	}

	if (cit == row.end())
		--cit;

	return cit;
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
	findRowElement(row, pos, boundary, x);
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
	}
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
		cur.setSelection(true); // to avoid deletion
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
	return leftMargin(max_width, pit, text_->paragraphs()[pit].size());
}


int TextMetrics::leftMargin(int max_width,
		pit_type const pit, pos_type const pos) const
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

	docstring parindent = layout.parindent;

	int l_margin = 0;

	if (text_->isMainText())
		l_margin += bv_->leftMargin();

	l_margin += theFontMetrics(buffer.params().getFont()).signedWidth(
		tclass.leftmargin());

	int depth = par.getDepth();
	if (depth != 0) {
		// find the next level paragraph
		pit_type newpar = text_->outerHook(pit);
		if (newpar != pit_type(pars.size())) {
			if (pars[newpar].layout().isEnvironment()) {
				int nestmargin = depth * nestMargin();
				if (text_->isMainText())
					nestmargin += changebarMargin();
				l_margin = max(leftMargin(max_width, newpar), nestmargin);
				// Remove the parindent that has been added
				// if the paragraph was empty.
				if (pars[newpar].empty() &&
				    buffer.params().paragraph_separation ==
				    BufferParams::ParagraphIndentSeparation) {
					docstring pi = pars[newpar].layout().parindent;
					l_margin -= theFontMetrics(
						buffer.params().getFont()).signedWidth(pi);
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
			// if we are at position 0, we are never in the body
			if (pos > 0 && pos >= par.beginOfBody())
				l_margin += labelfont_metrics.signedWidth(layout.leftmargin);
			else
				l_margin += labelfont_metrics.signedWidth(layout.labelindent);
		} else if (pos != 0
			   // Special case to fix problems with
			   // theorems (JMarc)
			   || (layout.labeltype == LABEL_STATIC
			       && layout.latextype == LATEX_ENVIRONMENT
			       && !text_->isFirstInSequence(pit))) {
			l_margin += labelfont_metrics.signedWidth(layout.leftmargin);
		} else if (!layout.labelIsAbove()) {
			l_margin += labelfont_metrics.signedWidth(layout.labelindent);
			l_margin += labelfont_metrics.width(layout.labelsep);
			l_margin += labelfont_metrics.width(par.labelString());
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
		int minfill = max_width;
		for ( ; rit != end; ++rit)
			if (rit->fill() < minfill)
				minfill = rit->fill();
		l_margin += theFontMetrics(buffer.params().getFont()).signedWidth(layout.leftmargin);
		l_margin += minfill;
#endif
		// also wrong, but much shorter.
		l_margin += max_width / 2;
		break;
	}
	}

	if (!par.params().leftIndent().zero())
		l_margin += par.params().leftIndent().inPixels(max_width, labelfont_metrics.em());

	LyXAlignment align;

	if (par.params().align() == LYX_ALIGN_LAYOUT)
		align = layout.align;
	else
		align = par.params().align();

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
			// use the parindent of the layout when the
			// default indentation is used otherwise use
			// the indentation set in the document
			// settings
			if (buffer.params().getIndentation().asLyXCommand() == "default")
				l_margin += theFontMetrics(
					buffer.params().getFont()).signedWidth(parindent);
			else
				l_margin += buffer.params().getIndentation().inPixels(*bv_);
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
	BufferParams const & bparams = bv_->buffer().params();
	ParagraphMetrics const & pm = par_metrics_[pit];
	if (pm.rows().empty())
		return;

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
		int row_x = x;
		if (i)
			y += row.ascent();

		CursorSlice rowSlice(const_cast<InsetText &>(text_->inset()));
		rowSlice.pit() = pit;
		rowSlice.pos() = row.pos();

		bool const inside = (y + row.descent() >= 0
			&& y - row.ascent() < ww);

		// Adapt to cursor row scroll offset if applicable.
		if (bv_->currentRowSlice() == rowSlice)
			row_x -= bv_->horizScrollOffset();

		// It is not needed to draw on screen if we are not inside.
		pi.pain.setDrawingEnabled(inside && original_drawing_state);

		RowPainter rp(pi, *text_, pit, row, row_x, y);

		if (selection)
			row.setSelectionAndMargins(sel_beg_par, sel_end_par);
		else
			row.setSelection(-1, -1);

		// The row knows nothing about the paragraph, so we have to check
		// whether this row is the first or last and update the margins.
		if (row.selection()) {
			if (row.sel_beg == 0)
				row.begin_margin_sel = sel_beg.pit() < pit;
			if (row.sel_end == sel_end_par.lastpos())
				row.end_margin_sel = sel_end.pit() > pit;
		}

		// Row signature; has row changed since last paint?
		if (pi.pain.isDrawingEnabled())
			row.setCrc(pm.computeRowSignature(row, bparams));
		bool row_has_changed = row.changed()
			|| rowSlice == bv_->lastRowSlice();

		// Take this opportunity to spellcheck the row contents.
		if (row_has_changed && pi.do_spellcheck && lyxrc.spellcheck_continuously) {
			text_->getPar(pit).spellCheck();
		}

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
			LYXERR(Debug::PAINTING, "Clear rect@("
			       << max(row_x, 0) << ", " << y - row.ascent() << ")="
			       << width() << " x " << row.height());
			pi.pain.fillRectangle(max(row_x, 0), y - row.ascent(),
				width(), row.height(), pi.background_color);
		}

		// Instrumentation for testing row cache (see also
		// 12 lines lower):
		if (lyxerr.debugging(Debug::PAINTING) && inside
			&& (row.selection() || pi.full_repaint || row_has_changed)) {
				string const foreword = text_->isMainText() ?
					"main text redraw " : "inset text redraw: ";
			LYXERR(Debug::PAINTING, foreword << "pit=" << pit << " row=" << i
				<< " row_selection="	<< row.selection()
				<< " full_repaint="	<< pi.full_repaint
				<< " row_has_changed="	<< row_has_changed
				<< " drawingEnabled=" << pi.pain.isDrawingEnabled());
		}

		// Backup full_repaint status and force full repaint
		// for inner insets as the Row has been cleared out.
		bool tmp = pi.full_repaint;
		pi.full_repaint = true;

		rp.paintSelection();
		rp.paintAppendix();
		rp.paintDepthBar();
		rp.paintChangeBar();
		bool const is_rtl = text_->isRTL(text_->getPar(pit));
		if (i == 0 && !is_rtl)
			rp.paintFirst();
		if (i == nrows - 1 && is_rtl)
			rp.paintLast();
		rp.paintText();
		if (i == nrows - 1 && !is_rtl)
			rp.paintLast();
		if (i == 0 && is_rtl)
			rp.paintFirst();
		rp.paintTooLargeMarks(row_x < 0,
				      row_x + row.width() > bv_->workWidth());
		y += row.descent();

		// Restore full_repaint status.
		pi.full_repaint = tmp;
	}
	// Re-enable screen drawing for future use of the painter.
	pi.pain.setDrawingEnabled(original_drawing_state);

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

	// get position on screen of the word start and end
	//FIXME: Is it necessary to explicitly set this to false?
	wordStart.boundary(false);
	Point lxy = cur.bv().getPos(wordStart);
	Point rxy = cur.bv().getPos(bvcur);

	// calculate dimensions of the word
	Row row;
	row.pos(wordStart.pos());
	row.endpos(bvcur.pos());
	setRowHeight(row, bvcur.pit(), false);
	dim = row.dimension();
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
