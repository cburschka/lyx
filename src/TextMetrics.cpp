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
#include "bufferview_funcs.h"
#include "Color.h"
#include "CoordCache.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "FontIterator.h"
#include "FuncRequest.h"
#include "Length.h"
#include "LyXRC.h"
#include "MetricsInfo.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "rowpainter.h"
#include "Text.h"
#include "Undo.h"
#include "VSpace.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include <boost/current_function.hpp>

using std::max;
using std::min;
using std::endl;

namespace lyx {

using frontend::FontMetrics;

namespace {

int numberOfSeparators(Paragraph const & par, Row const & row)
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


int numberOfHfills(Paragraph const & par, Row const & row)
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

} // namespace anon

TextMetrics::TextMetrics(BufferView * bv, Text * text)
	: bv_(bv), text_(text)
{
	BOOST_ASSERT(bv_);
	max_width_ = bv_->workWidth();
	dim_.wid = max_width_;
	dim_.asc = 10;
	dim_.des = 10;

	//text_->updateLabels(bv->buffer());
}


ParagraphMetrics const & TextMetrics::parMetrics(pit_type pit) const
{
	return const_cast<TextMetrics *>(this)->parMetrics(pit, true);
}


ParagraphMetrics & TextMetrics::parMetrics(pit_type pit,
		bool redo)
{
	ParMetricsCache::iterator pmc_it = par_metrics_.find(pit);
	if (pmc_it == par_metrics_.end()) {
		pmc_it = par_metrics_.insert(
			std::make_pair(pit, ParagraphMetrics(text_->getPar(pit)))).first;
	}
	if (pmc_it->second.rows().empty() && redo) {
		redoParagraph(pit);
	}
	return pmc_it->second;
}


bool TextMetrics::metrics(MetricsInfo & mi, Dimension & dim)
{
	BOOST_ASSERT(mi.base.textwidth);
	max_width_ = mi.base.textwidth;
	// backup old dimension.
	Dimension const old_dim = dim_;
	// reset dimension.
	dim_ = Dimension();
	size_t npar = text_->paragraphs().size();
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
	return main_text_? pm.rightMargin(bv_->buffer()) : 0;
}


int TextMetrics::rightMargin(pit_type const pit) const
{
	return main_text_? par_metrics_[pit].rightMargin(bv_->buffer()) : 0;
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
	BufferParams const & bparams = buffer.params();
	main_text_ = (text_ == &buffer.text());
	bool changed = false;

	// FIXME This check ought to be done somewhere else. It is the reason
	// why text_ is not	const. But then, where else to do it?
	// Well, how can you end up with either (a) a biblio environment that
	// has no InsetBibitem or (b) a biblio environment with more than one
	// InsetBibitem? I think the answer is: when paragraphs are merged;
	// when layout is set; when material is pasted.
	int const moveCursor = par.checkBiblio(buffer.params().trackChanges);
	if (moveCursor > 0)
		const_cast<Cursor &>(bv_->cursor()).posRight();
	else if (moveCursor < 0) {
		Cursor & cursor = const_cast<Cursor &>(bv_->cursor());
		if (cursor.pos() >= -moveCursor)
			cursor.posLeft();
	}

	// Optimisation: this is used in the next two loops
	// so better to calculate that once here.
	int const right_margin = rightMargin(pm);

	// redo insets
	// FIXME: We should always use getFont(), see documentation of
	// noFontChange() in Inset.h.
	Font const bufferfont = buffer.params().getFont();
	InsetList::const_iterator ii = par.insetlist.begin();
	InsetList::const_iterator iend = par.insetlist.end();
	for (; ii != iend; ++ii) {
		Dimension old_dim = ii->inset->dimension();
		Dimension dim;
		int const w = max_width_ - leftMargin(max_width_, pit, ii->pos)
			- right_margin;
		Font const & font = ii->inset->noFontChange() ?
			bufferfont : text_->getFont(buffer, par, ii->pos);
		MetricsInfo mi(bv_, font, w);
		changed |= ii->inset->metrics(mi, dim);
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
			// for the computeRowMetrics below().
			dim_.wid = max_width_;

		dim.wid = rowWidth(right_margin, pit, first, end);
		boost::tie(dim.asc, dim.des) = rowHeight(pit, first, end);
		if (row_index == pm.rows().size())
			pm.rows().push_back(Row());
		Row & row = pm.rows()[row_index];
		row.setChanged(false);
		row.pos(first);
		row.endpos(end);
		row.setDimension(dim);
		computeRowMetrics(pit, row);
		pm.computeRowSignature(row, bparams);
		first = end;
		++row_index;

		pm.dim().wid = std::max(pm.dim().wid, dim.wid);
		pm.dim().des += dim.height();
	} while (first < par.size());

	if (row_index < pm.rows().size())
		pm.rows().resize(row_index);

	// Make sure that if a par ends in newline, there is one more row
	// under it
	if (first > 0 && par.isNewline(first - 1)) {
		Dimension dim;
		dim.wid = rowWidth(right_margin, pit, first, first);
		boost::tie(dim.asc, dim.des) = rowHeight(pit, first, first);
		if (row_index == pm.rows().size())
			pm.rows().push_back(Row());
		Row & row = pm.rows()[row_index];
		row.setChanged(false);
		row.pos(first);
		row.endpos(first);
		row.setDimension(dim);
		computeRowMetrics(pit, row);
		pm.computeRowSignature(row, bparams);
		pm.dim().des += dim.height();
	}

	pm.dim().asc += pm.rows()[0].ascent();
	pm.dim().des -= pm.rows()[0].ascent();

	changed |= old_dim.height() != pm.dim().height();

	return changed;
}


void TextMetrics::computeRowMetrics(pit_type const pit,
		Row & row) const
{

	row.label_hfill = 0;
	row.hfill = 0;
	row.separator = 0;

	Buffer & buffer = bv_->buffer();
	Paragraph const & par = text_->getPar(pit);

	double w = dim_.wid - row.width();
	// FIXME: put back this assertion when the crash on new doc is solved.
	//BOOST_ASSERT(w >= 0);

	//lyxerr << "\ndim_.wid " << dim_.wid << endl;
	//lyxerr << "row.width() " << row.width() << endl;
	//lyxerr << "w " << w << endl;

	bool const is_rtl = text_->isRTL(buffer, par);
	if (is_rtl)
		row.x = rightMargin(pit);
	else
		row.x = leftMargin(max_width_, pit, row.pos());

	// is there a manual margin with a manual label
	LayoutPtr const & layout = par.layout();

	if (layout->margintype == MARGIN_MANUAL
	    && layout->labeltype == LABEL_MANUAL) {
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

	// are there any hfills in the row?
	int const nh = numberOfHfills(par, row);

	if (nh) {
		if (w > 0)
			row.hfill = w / nh;
	// we don't have to look at the alignment if it is ALIGN_LEFT and
	// if the row is already larger then the permitted width as then
	// we force the LEFT_ALIGN'edness!
	} else if (int(row.width()) < max_width_) {
		// is it block, flushleft or flushright?
		// set x how you need it
		int align;
		if (par.params().align() == LYX_ALIGN_LAYOUT)
			align = layout->align;
		else
			align = par.params().align();

		// Display-style insets should always be on a centred row
		// The test on par.size() is to catch zero-size pars, which
		// would trigger the assert in Paragraph::getInset().
		//inset = par.size() ? par.getInset(row.pos()) : 0;
		if (row.pos() < par.size()
		    && par.isInset(row.pos()))
		{
		    switch(par.getInset(row.pos())->display()) {
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
			row.x += theFontMetrics(text_->getLabelFont(buffer, par)).
				width(layout->labelsep);
			if (body_pos <= end)
				row.x += row.label_hfill;
		}
	}
}


int TextMetrics::labelFill(pit_type const pit, Row const & row) const
{
	Buffer & buffer = bv_->buffer();
	Paragraph const & par = text_->getPar(pit);

	pos_type last = par.beginOfBody();
	BOOST_ASSERT(last > 0);

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
		= theFontMetrics(text_->getLabelFont(buffer, par));

	return max(0, fm.width(label) - w);
}


namespace {

// this needs special handling - only newlines count as a break point
pos_type addressBreakPoint(pos_type i, Paragraph const & par)
{
	pos_type const end = par.size();

	for (; i < end; ++i)
		if (par.isNewline(i))
			return i + 1;

	return end;
}

};


int TextMetrics::labelEnd(pit_type const pit) const
{
	// labelEnd is only needed if the layout fills a flushleft label.
	if (text_->getPar(pit).layout()->margintype != MARGIN_MANUAL)
		return 0;
	// return the beginning of the body
	return leftMargin(max_width_, pit);
}


pit_type TextMetrics::rowBreakPoint(int width, pit_type const pit,
		pit_type pos) const
{
	Buffer & buffer = bv_->buffer();
	ParagraphMetrics const & pm = par_metrics_[pit];
	Paragraph const & par = text_->getPar(pit);
	pos_type const end = par.size();
	if (pos == end || width < 0)
		return end;

	LayoutPtr const & layout = par.layout();

	if (layout->margintype == MARGIN_RIGHT_ADDRESS_BOX)
		return addressBreakPoint(pos, par);

	pos_type const body_pos = par.beginOfBody();


	// Now we iterate through until we reach the right margin
	// or the end of the par, then choose the possible break
	// nearest that.

	int label_end = labelEnd(pit);
	int const left = leftMargin(max_width_, pit, pos);
	int x = left;

	// pixel width since last breakpoint
	int chunkwidth = 0;

	FontIterator fi = FontIterator(buffer, *text_, par, pos);
	pos_type point = end;
	pos_type i = pos;
	for ( ; i < end; ++i, ++fi) {
		int thiswidth = pm.singleWidth(i, *fi);

		// add the auto-hfill from label end to the body
		if (body_pos && i == body_pos) {
			FontMetrics const & fm = theFontMetrics(
				text_->getLabelFont(buffer, par));
			int add = fm.width(layout->labelsep);
			if (par.isLineSeparator(i - 1))
				add -= singleWidth(pit, i - 1);

			add = std::max(add, label_end - x);
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
		// Break before...
		if (i + 1 < end) {
			if (par.isInset(i + 1) && par.getInset(i + 1)->display()) {
				point = i + 1;
				break;
			}
			// ...and after.
			if (par.isInset(i) && par.getInset(i)->display()) {
				point = i + 1;
				break;
			}
		}

		if (!par.isInset(i) || par.getInset(i)->isChar()) {
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

	pos_type const body_pos = par.beginOfBody();
	pos_type i = first;

	if (i < end) {
		FontIterator fi = FontIterator(buffer, *text_, par, i);
		for ( ; i < end; ++i, ++fi) {
			if (body_pos > 0 && i == body_pos) {
				FontMetrics const & fm = theFontMetrics(
					text_->getLabelFont(buffer, par));
				w += fm.width(par.layout()->labelsep);
				if (par.isLineSeparator(i - 1))
					w -= singleWidth(pit, i - 1);
				w = max(w, label_end);
			}
			w += pm.singleWidth(i, *fi);
		}
	}

	if (body_pos > 0 && body_pos >= end) {
		FontMetrics const & fm = theFontMetrics(
			text_->getLabelFont(buffer, par));
		w += fm.width(par.layout()->labelsep);
		if (end > 0 && par.isLineSeparator(end - 1))
			w -= singleWidth(pit, end - 1);
		w = max(w, label_end);
	}

	return w + right_margin;
}


boost::tuple<int, int> TextMetrics::rowHeight(pit_type const pit, pos_type const first,
		pos_type const end) const
{
	Paragraph const & par = text_->getPar(pit);
	// get the maximum ascent and the maximum descent
	double layoutasc = 0;
	double layoutdesc = 0;
	double const dh = defaultRowHeight();

	// ok, let us initialize the maxasc and maxdesc value.
	// Only the fontsize count. The other properties
	// are taken from the layoutfont. Nicer on the screen :)
	LayoutPtr const & layout = par.layout();

	// as max get the first character of this row then it can
	// increase but not decrease the height. Just some point to
	// start with so we don't have to do the assignment below too
	// often.
	Buffer const & buffer = bv_->buffer();
	Font font = text_->getFont(buffer, par, first);
	Font::FONT_SIZE const tmpsize = font.size();
	font = text_->getLayoutFont(buffer, pit);
	Font::FONT_SIZE const size = font.size();
	font.setSize(tmpsize);

	Font labelfont = text_->getLabelFont(buffer, par);

	FontMetrics const & labelfont_metrics = theFontMetrics(labelfont);
	FontMetrics const & fontmetrics = theFontMetrics(font);

	// these are minimum values
	double const spacing_val = layout->spacing.getValue()
		* text_->spacing(buffer, par);
	//lyxerr << "spacing_val = " << spacing_val << endl;
	int maxasc  = int(fontmetrics.maxAscent()  * spacing_val);
	int maxdesc = int(fontmetrics.maxDescent() * spacing_val);

	// insets may be taller
	InsetList::const_iterator ii = par.insetlist.begin();
	InsetList::const_iterator iend = par.insetlist.end();
	for ( ; ii != iend; ++ii) {
		if (ii->pos >= first && ii->pos < end) {
			maxasc  = max(maxasc,  ii->inset->ascent());
			maxdesc = max(maxdesc, ii->inset->descent());
		}
	}

	// Check if any custom fonts are larger (Asger)
	// This is not completely correct, but we can live with the small,
	// cosmetic error for now.
	int labeladdon = 0;

	Font::FONT_SIZE maxsize =
		par.highestFontInRange(first, end, size);
	if (maxsize > font.size()) {
		// use standard paragraph font with the maximal size
		Font maxfont = font;
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
	if (first == 0) {
		BufferParams const & bufparams = buffer.params();
		// some parskips VERY EASY IMPLEMENTATION
		if (bufparams.paragraph_separation
		    == BufferParams::PARSEP_SKIP
			&& par.ownerCode() != Inset::ERT_CODE
			&& par.ownerCode() != Inset::LISTINGS_CODE
			&& pit > 0
			&& ((layout->isParagraph() && par.getDepth() == 0)
			    || (pars[pit - 1].layout()->isParagraph()
				&& pars[pit - 1].getDepth() == 0)))
		{
				maxasc += bufparams.getDefSkip().inPixels(*bv_);
		}

		if (par.params().startOfAppendix())
			maxasc += int(3 * dh);

		// This is special code for the chapter, since the label of this
		// layout is printed in an extra row
		if (layout->counter == "chapter"
		    && !par.params().labelString().empty()) {
			labeladdon = int(labelfont_metrics.maxHeight()
				     * layout->spacing.getValue()
				     * text_->spacing(buffer, par));
		}

		// special code for the top label
		if ((layout->labeltype == LABEL_TOP_ENVIRONMENT
		     || layout->labeltype == LABEL_BIBLIO
		     || layout->labeltype == LABEL_CENTERED_TOP_ENVIRONMENT)
		    && isFirstInSequence(pit, pars)
		    && !par.getLabelstring().empty())
		{
			labeladdon = int(
				  labelfont_metrics.maxHeight()
					* layout->spacing.getValue()
					* text_->spacing(buffer, par)
				+ (layout->topsep + layout->labelbottomsep) * dh);
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
			layoutasc = layout->itemsep * dh;
		} else if (pit != 0 || first != 0) {
			if (layout->topsep > 0)
				layoutasc = layout->topsep * dh;
		}

		prev = outerHook(pit, pars);
		if (prev != pit_type(pars.size())) {
			maxasc += int(pars[prev].layout()->parsep * dh);
		} else if (pit != 0) {
			Paragraph const & prevpar = pars[pit - 1];
			if (prevpar.getDepth() != 0 ||
					prevpar.layout() == layout) {
				maxasc += int(layout->parsep * dh);
			}
		}
	}

	// is it a bottom line?
	if (end >= par.size()) {
		// add the layout spaces, for example before and after
		// a section, or between the items of a itemize or enumerate
		// environment
		pit_type nextpit = pit + 1;
		if (nextpit != pit_type(pars.size())) {
			pit_type cpit = pit;
			double usual = 0;
			double unusual = 0;

			if (pars[cpit].getDepth() > pars[nextpit].getDepth()) {
				usual = pars[cpit].layout()->bottomsep * dh;
				cpit = depthHook(cpit, pars, pars[nextpit].getDepth());
				if (pars[cpit].layout() != pars[nextpit].layout()
					|| pars[nextpit].getLabelWidthString() != pars[cpit].getLabelWidthString())
				{
					unusual = pars[cpit].layout()->bottomsep * dh;
				}
				layoutdesc = max(unusual, usual);
			} else if (pars[cpit].getDepth() == pars[nextpit].getDepth()) {
				if (pars[cpit].layout() != pars[nextpit].layout()
					|| pars[nextpit].getLabelWidthString() != pars[cpit].getLabelWidthString())
					layoutdesc = int(pars[cpit].layout()->bottomsep * dh);
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
	if (main_text_) {
		if (pit == 0 && first == 0)
			maxasc += 20;
		if (pit + 1 == pit_type(pars.size()) &&
		    end == par.size() &&
				!(end > 0 && par.isNewline(end - 1)))
			maxdesc += 20;
	}

	return boost::make_tuple(maxasc + labeladdon, maxdesc);
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
	int const xo = main_text_? 0 : bv_->coordCache().get(text_, pit).x_;
	x -= xo;
	Paragraph const & par = text_->getPar(pit);
	ParagraphMetrics const & pm = par_metrics_[pit];
	Bidi bidi;
	bidi.computeTables(par, buffer, row);

	pos_type vc = row.pos();
	pos_type end = row.endpos();
	pos_type c = 0;
	LayoutPtr const & layout = par.layout();

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
				text_->getLabelFont(buffer, par));
			tmpx += row.label_hfill + fm.width(layout->labelsep);
			if (par.isLineSeparator(body_pos - 1))
				tmpx -= singleWidth(pit, body_pos - 1);
		}

		if (pm.hfillExpansion(row, c)) {
			tmpx += singleWidth(pit, c);
			if (c >= body_pos)
				tmpx += row.hfill;
			else
				tmpx += row.label_hfill;
		} else if (par.isSeparator(c)) {
			tmpx += singleWidth(pit, c);
			if (c >= body_pos)
				tmpx += row.separator;
		} else {
			tmpx += singleWidth(pit, c);
		}
		++vc;
	}

	if ((tmpx + last_tmpx) / 2 > x) {
		tmpx = last_tmpx;
		left_side = true;
	}

	BOOST_ASSERT(vc <= end);  // This shouldn't happen.

	boundary = false;
	// This (rtl_support test) is not needed, but gives
	// some speedup if rtl_support == false
	bool const lastrow = lyxrc.rtl_support && row.endpos() == par.size();

	// If lastrow is false, we don't need to compute
	// the value of rtl.
	bool const rtl = lastrow ? text_->isRTL(buffer, par) : false;
	if (lastrow &&
	    ((rtl  &&  left_side && vc == row.pos() && x < tmpx - 5) ||
	     (!rtl && !left_side && vc == end  && x > tmpx + 5)))
		c = end;
	else if (vc == row.pos()) {
		c = bidi.vis2log(vc);
		if (bidi.level(c) % 2 == 1)
			++c;
	} else {
		c = bidi.vis2log(vc - 1);
		bool const rtl = (bidi.level(c) % 2 == 1);
		if (left_side == rtl) {
			++c;
			boundary = text_->isRTLBoundary(buffer, par, c);
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
	ParagraphMetrics const & pm = par_metrics_[pit];
	BOOST_ASSERT(!pm.rows().empty());
	BOOST_ASSERT(row < int(pm.rows().size()));
	bool bound = false;
	Row const & r = pm.rows()[row];
	return r.pos() + getColumnNearX(pit, r, x, bound);
}


// y is screen coordinate
pit_type TextMetrics::getPitNearY(int y)
{
	BOOST_ASSERT(!text_->paragraphs().empty());
	BOOST_ASSERT(bv_->coordCache().getParPos().find(text_) != bv_->coordCache().getParPos().end());
	CoordCache::InnerParPosCache const & cc = bv_->coordCache().getParPos().find(text_)->second;
	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< ": y: " << y << " cache size: " << cc.size()
		<< endl;

	// look for highest numbered paragraph with y coordinate less than given y
	pit_type pit = 0;
	int yy = -1;
	CoordCache::InnerParPosCache::const_iterator it = cc.begin();
	CoordCache::InnerParPosCache::const_iterator et = cc.end();
	CoordCache::InnerParPosCache::const_iterator last = et; last--;

	ParagraphMetrics const & pm = par_metrics_[it->first];

	// If we are off-screen (before the visible part)
	if (y < 0
		// and even before the first paragraph in the cache.
		&& y < it->second.y_ - int(pm.ascent())) {
		//  and we are not at the first paragraph in the inset.
		if (it->first == 0)
			return 0;
		// then this is the paragraph we are looking for.
		pit = it->first - 1;
		// rebreak it and update the CoordCache.
		redoParagraph(pit);
		bv_->coordCache().parPos()[text_][pit] =
			Point(0, it->second.y_ - pm.descent());
		return pit;
	}

	ParagraphMetrics const & pm_last = par_metrics_[last->first];

	// If we are off-screen (after the visible part)
	if (y > bv_->workHeight()
		// and even after the first paragraph in the cache.
		&& y >= last->second.y_ + int(pm_last.descent())) {
		pit = last->first + 1;
		//  and we are not at the last paragraph in the inset.
		if (pit == int(text_->paragraphs().size()))
			return last->first;
		// then this is the paragraph we are looking for.
		// rebreak it and update the CoordCache.
		redoParagraph(pit);
		bv_->coordCache().parPos()[text_][pit] =
			Point(0, last->second.y_ + pm_last.ascent());
		return pit;
	}

	for (; it != et; ++it) {
		LYXERR(Debug::DEBUG)
			<< BOOST_CURRENT_FUNCTION
			<< "  examining: pit: " << it->first
			<< " y: " << it->second.y_
			<< endl;

		ParagraphMetrics const & pm = par_metrics_[it->first];

		if (it->first >= pit && int(it->second.y_) - int(pm.ascent()) <= y) {
			pit = it->first;
			yy = it->second.y_;
		}
	}

	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< ": found best y: " << yy << " for pit: " << pit
		<< endl;

	return pit;
}


Row const & TextMetrics::getRowNearY(int y, pit_type pit) const
{
	ParagraphMetrics const & pm = par_metrics_[pit];

	int yy = bv_->coordCache().get(text_, pit).y_ - pm.ascent();
	BOOST_ASSERT(!pm.rows().empty());
	RowList::const_iterator rit = pm.rows().begin();
	RowList::const_iterator const rlast = boost::prior(pm.rows().end());
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
		lyxerr << "TextMetrics::editXY(cur, " << x << ", " << y << ")" << std::endl;
		cur.bv().coordCache().dump();
	}
	pit_type pit = getPitNearY(y);
	BOOST_ASSERT(pit != -1);

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
	BOOST_ASSERT((pos != 0 && inset == insetBefore)
		|| inset == pars[pit].getInset(pos));

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
	BOOST_ASSERT(text_ == cur.text());
	pit_type pit = getPitNearY(y);

	ParagraphMetrics const & pm = par_metrics_[pit];

	int yy = bv_->coordCache().get(text_, pit).y_ - pm.ascent();
	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< ": x: " << x
		<< " y: " << y
		<< " pit: " << pit
		<< " yy: " << yy << endl;

	int r = 0;
	BOOST_ASSERT(pm.rows().size());
	for (; r < int(pm.rows().size()) - 1; ++r) {
		Row const & row = pm.rows()[r];
		if (int(yy + row.height()) > y)
			break;
		yy += row.height();
	}

	Row const & row = pm.rows()[r];

	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< ": row " << r
		<< " from pos: " << row.pos()
		<< endl;

	bool bound = false;
	int xx = x;
	pos_type const pos = row.pos() + getColumnNearX(pit, row, xx, bound);

	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< ": setting cursor pit: " << pit
		<< " pos: " << pos
		<< endl;

	text_->setCursor(cur, pit, pos, true, bound);
	// remember new position.
	cur.setTargetX();
}


//takes screen x,y coordinates
Inset * TextMetrics::checkInsetHit(int x, int y)
{
	pit_type pit = getPitNearY(y);
	BOOST_ASSERT(pit != -1);

	Paragraph const & par = text_->paragraphs()[pit];

	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< ": x: " << x
		<< " y: " << y
		<< "  pit: " << pit
		<< endl;
	InsetList::const_iterator iit = par.insetlist.begin();
	InsetList::const_iterator iend = par.insetlist.end();
	for (; iit != iend; ++iit) {
		Inset * inset = iit->inset;
#if 1
		LYXERR(Debug::DEBUG)
			<< BOOST_CURRENT_FUNCTION
			<< ": examining inset " << inset << endl;

		if (bv_->coordCache().getInsets().has(inset))
			LYXERR(Debug::DEBUG)
				<< BOOST_CURRENT_FUNCTION
				<< ": xo: " << inset->xo(*bv_) << "..."
				<< inset->xo(*bv_) + inset->width()
				<< " yo: " << inset->yo(*bv_) - inset->ascent()
				<< "..."
				<< inset->yo(*bv_) + inset->descent()
				<< endl;
		else
			LYXERR(Debug::DEBUG)
				<< BOOST_CURRENT_FUNCTION
				<< ": inset has no cached position" << endl;
#endif
		if (inset->covers(*bv_, x, y)) {
			LYXERR(Debug::DEBUG)
				<< BOOST_CURRENT_FUNCTION
				<< ": Hit inset: " << inset << endl;
			return inset;
		}
	}
	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< ": No inset hit. " << endl;
	return 0;
}


int TextMetrics::cursorX(CursorSlice const & sl,
		bool boundary) const
{
	BOOST_ASSERT(sl.text() == text_);
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

	// Use font span to speed things up, see below
	FontSpan font_span;
	Font font;

	// If the last logical character is a separator, skip it, unless
	// it's in the last row of a paragraph; see skipped_sep_vpos declaration
	if (end > 0 && end < par.size() && par.isSeparator(end - 1))
		skipped_sep_vpos = bidi.log2vis(end - 1);
	
	for (pos_type vpos = row_pos; vpos < cursor_vpos; ++vpos) {
		// Skip the separator which is at the logical end of the row
		if (vpos == skipped_sep_vpos)
			continue;
		pos_type pos = bidi.vis2log(vpos);
		if (body_pos > 0 && pos == body_pos - 1) {
			FontMetrics const & labelfm = theFontMetrics(
				text_->getLabelFont(buffer, par));
			x += row.label_hfill + labelfm.width(par.layout()->labelsep);
			if (par.isLineSeparator(body_pos - 1))
				x -= singleWidth(pit, body_pos - 1);
		}

		// Use font span to speed things up, see above
		if (pos < font_span.first || pos > font_span.last) {
			font_span = par.fontSpan(pos);
			font = text_->getFont(buffer, par, pos);
		}

		x += pm.singleWidth(pos, font);

		if (pm.hfillExpansion(row, pos))
			x += (pos >= body_pos) ? row.hfill : row.label_hfill;
		else if (par.isSeparator(pos) && pos >= body_pos)
			x += row.separator;
	}

	// see correction above
	if (boundary_correction) {
		if (text_->isRTL(buffer, sl, boundary))
			x -= singleWidth(pit, ppos);
		else
			x += singleWidth(pit, ppos);
	}

	return int(x);
}


int TextMetrics::cursorY(CursorSlice const & sl, bool boundary) const
{
	//lyxerr << "TextMetrics::cursorY: boundary: " << boundary << std::endl;
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


void TextMetrics::cursorPrevious(Cursor & cur)
{
	pos_type cpos = cur.pos();
	pit_type cpar = cur.pit();

	int x = cur.x_target();
	setCursorFromCoordinates(cur, x, 0);
	cur.dispatch(FuncRequest(cur.selection()? LFUN_UP_SELECT: LFUN_UP));

	if (cpar == cur.pit() && cpos == cur.pos())
		// we have a row which is taller than the workarea. The
		// simplest solution is to move to the previous row instead.
		cur.dispatch(FuncRequest(cur.selection()? LFUN_UP_SELECT: LFUN_UP));

	finishUndo();
	cur.updateFlags(Update::Force | Update::FitCursor);
}


void TextMetrics::cursorNext(Cursor & cur)
{
	pos_type cpos = cur.pos();
	pit_type cpar = cur.pit();

	int x = cur.x_target();
	setCursorFromCoordinates(cur, x, cur.bv().workHeight() - 1);
	cur.dispatch(FuncRequest(cur.selection()? LFUN_DOWN_SELECT: LFUN_DOWN));

	if (cpar == cur.pit() && cpos == cur.pos())
		// we have a row which is taller than the workarea. The
		// simplest solution is to move to the next row instead.
		cur.dispatch(
			FuncRequest(cur.selection()? LFUN_DOWN_SELECT: LFUN_DOWN));

	finishUndo();
	cur.updateFlags(Update::Force | Update::FitCursor);
}


// the cursor set functions have a special mechanism. When they
// realize you left an empty paragraph, they will delete it.

bool TextMetrics::cursorHome(Cursor & cur)
{
	BOOST_ASSERT(text_ == cur.text());
	ParagraphMetrics const & pm = par_metrics_[cur.pit()];
	Row const & row = pm.getRow(cur.pos(),cur.boundary());
	return text_->setCursor(cur, cur.pit(), row.pos());
}


bool TextMetrics::cursorEnd(Cursor & cur)
{
	BOOST_ASSERT(text_ == cur.text());
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
	BOOST_ASSERT(text_ == cur.text());
	if (cur.lastpos() == 0) {
		// Paragraph is empty, so we just go to the right
		text_->cursorRight(cur);
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
		checkBufferStructure(cur.buffer(), cur);
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
	BOOST_ASSERT(pit >= 0);
	BOOST_ASSERT(pit < int(text_->paragraphs().size()));
	return leftMargin(max_width, pit, text_->paragraphs()[pit].size());
}


int TextMetrics::leftMargin(int max_width,
		pit_type const pit, pos_type const pos) const
{
	ParagraphList const & pars = text_->paragraphs();

	BOOST_ASSERT(pit >= 0);
	BOOST_ASSERT(pit < int(pars.size()));
	Paragraph const & par = pars[pit];
	BOOST_ASSERT(pos >= 0);
	BOOST_ASSERT(pos <= par.size());
	Buffer const & buffer = bv_->buffer();
	//lyxerr << "TextMetrics::leftMargin: pit: " << pit << " pos: " << pos << endl;
	TextClass const & tclass = buffer.params().getTextClass();
	LayoutPtr const & layout = par.layout();

	docstring parindent = layout->parindent;

	int l_margin = 0;

	if (text_->isMainText(buffer))
		l_margin += changebarMargin();

	l_margin += theFontMetrics(buffer.params().getFont()).signedWidth(
		tclass.leftmargin());

	if (par.getDepth() != 0) {
		// find the next level paragraph
		pit_type newpar = outerHook(pit, pars);
		if (newpar != pit_type(pars.size())) {
			if (pars[newpar].layout()->isEnvironment()) {
				l_margin = leftMargin(max_width, newpar);
			}
			if (par.layout() == tclass.defaultLayout()) {
				if (pars[newpar].params().noindent())
					parindent.erase();
				else
					parindent = pars[newpar].layout()->parindent;
			}
		}
	}

	// This happens after sections in standard classes. The 1.3.x
	// code compared depths too, but it does not seem necessary
	// (JMarc)
	if (par.layout() == tclass.defaultLayout()
	    && pit > 0 && pars[pit - 1].layout()->nextnoindent)
		parindent.erase();

	Font const labelfont = text_->getLabelFont(buffer, par);
	FontMetrics const & labelfont_metrics = theFontMetrics(labelfont);

	switch (layout->margintype) {
	case MARGIN_DYNAMIC:
		if (!layout->leftmargin.empty()) {
			l_margin += theFontMetrics(buffer.params().getFont()).signedWidth(
				layout->leftmargin);
		}
		if (!par.getLabelstring().empty()) {
			l_margin += labelfont_metrics.signedWidth(layout->labelindent);
			l_margin += labelfont_metrics.width(par.getLabelstring());
			l_margin += labelfont_metrics.width(layout->labelsep);
		}
		break;

	case MARGIN_MANUAL: {
		l_margin += labelfont_metrics.signedWidth(layout->labelindent);
		// The width of an empty par, even with manual label, should be 0
		if (!par.empty() && pos >= par.beginOfBody()) {
			if (!par.getLabelWidthString().empty()) {
				docstring labstr = par.getLabelWidthString();
				l_margin += labelfont_metrics.width(labstr);
				l_margin += labelfont_metrics.width(layout->labelsep);
			}
		}
		break;
	}

	case MARGIN_STATIC: {
		l_margin += theFontMetrics(buffer.params().getFont()).
			signedWidth(layout->leftmargin) * 4	/ (par.getDepth() + 4);
		break;
	}

	case MARGIN_FIRST_DYNAMIC:
		if (layout->labeltype == LABEL_MANUAL) {
			if (pos >= par.beginOfBody()) {
				l_margin += labelfont_metrics.signedWidth(layout->leftmargin);
			} else {
				l_margin += labelfont_metrics.signedWidth(layout->labelindent);
			}
		} else if (pos != 0
			   // Special case to fix problems with
			   // theorems (JMarc)
			   || (layout->labeltype == LABEL_STATIC
			       && layout->latextype == LATEX_ENVIRONMENT
			       && !isFirstInSequence(pit, pars))) {
			l_margin += labelfont_metrics.signedWidth(layout->leftmargin);
		} else if (layout->labeltype != LABEL_TOP_ENVIRONMENT
			   && layout->labeltype != LABEL_BIBLIO
			   && layout->labeltype !=
			   LABEL_CENTERED_TOP_ENVIRONMENT) {
			l_margin += labelfont_metrics.signedWidth(layout->labelindent);
			l_margin += labelfont_metrics.width(layout->labelsep);
			l_margin += labelfont_metrics.width(par.getLabelstring());
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
		l_margin += theFontMetrics(params.getFont()).signedWidth(layout->leftmargin);
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
		align = layout->align;
	else
		align = par.params().align();

	// set the correct parindent
	if (pos == 0
	    && (layout->labeltype == LABEL_NO_LABEL
	       || layout->labeltype == LABEL_TOP_ENVIRONMENT
	       || layout->labeltype == LABEL_CENTERED_TOP_ENVIRONMENT
	       || (layout->labeltype == LABEL_STATIC
		   && layout->latextype == LATEX_ENVIRONMENT
		   && !isFirstInSequence(pit, pars)))
	    && align == LYX_ALIGN_BLOCK
	    && !par.params().noindent()
	    // in some insets, paragraphs are never indented
	    && !(par.inInset() && par.inInset()->neverIndent(buffer))
	    // display style insets are always centered, omit indentation
	    && !(!par.empty()
		    && par.isInset(pos)
		    && par.getInset(pos)->display())
	    && (par.layout() != tclass.defaultLayout()
		|| buffer.params().paragraph_separation ==
		   BufferParams::PARSEP_INDENT))
	{
		l_margin += theFontMetrics(buffer.params().getFont()).signedWidth(
			parindent);
	}

	return l_margin;
}


int TextMetrics::singleWidth(pit_type pit, pos_type pos) const
{
	Buffer const & buffer = bv_->buffer();
	ParagraphMetrics const & pm = par_metrics_[pit];

	return pm.singleWidth(pos, text_->getFont(buffer, text_->getPar(pit), pos));
}


// only used for inset right now. should also be used for main text
void TextMetrics::draw(PainterInfo & pi, int x, int y) const
{
	if (par_metrics_.empty())
		return;

	ParMetricsCache::const_iterator it = par_metrics_.begin();
	ParMetricsCache::const_iterator const end = par_metrics_.end();
	y -= it->second.ascent();
	for (; it != end; ++it) {
		ParagraphMetrics const & pmi = it->second;
		y += pmi.ascent();
		drawParagraph(pi, it->first, x, y);
		y += pmi.descent();
	}
}


void TextMetrics::drawParagraph(PainterInfo & pi, pit_type pit, int x, int y) const
{
//	lyxerr << "  paintPar: pit: " << pit << " at y: " << y << endl;
	int const ww = bv_->workHeight();

	bv_->coordCache().parPos()[text_][pit] = Point(x, y);

	ParagraphMetrics const & pm = par_metrics_[pit];
	if (pm.rows().empty())
		return;

	RowList::const_iterator const rb = pm.rows().begin();
	RowList::const_iterator const re = pm.rows().end();

	Bidi bidi;

	y -= rb->ascent();
	for (RowList::const_iterator rit = rb; rit != re; ++rit) {
		y += rit->ascent();

		bool const inside = (y + rit->descent() >= 0
			&& y - rit->ascent() < ww);
		// it is not needed to draw on screen if we are not inside.
		pi.pain.setDrawingEnabled(inside);
		RowPainter rp(pi, *text_, pit, *rit, bidi, x, y);

		// Row signature; has row changed since last paint?
		bool row_has_changed = rit->changed();
		
		if (!pi.full_repaint && !row_has_changed) {
			// Paint the only the insets if the text itself is
			// unchanged.
			rp.paintOnlyInsets();
			y += rit->descent();
			continue;
		}

		// Paint the row if a full repaint has been requested or it has
		// changed.
		// Clear background of this row
		// (if paragraph background was not cleared)
		if (!pi.full_repaint && row_has_changed) {
			pi.pain.fillRectangle(x, y - rit->ascent(),
			width(), rit->height(),
			text_->backgroundColor());
		}

		// Instrumentation for testing row cache (see also
		// 12 lines lower):
		if (lyxerr.debugging(Debug::PAINTING)) {
			if (text_->isMainText(bv_->buffer()))
				LYXERR(Debug::PAINTING) << "\n{" <<
				pi.full_repaint << row_has_changed << "}";
			else
				LYXERR(Debug::PAINTING) << "[" <<
				pi.full_repaint << row_has_changed << "]";
		}

		// Backup full_repaint status and force full repaint
		// for inner insets as the Row has been cleared out.
		bool tmp = pi.full_repaint;
		pi.full_repaint = true;
		rp.paintAppendix();
		rp.paintDepthBar();
		rp.paintChangeBar();
		if (rit == rb)
			rp.paintFirst();
		rp.paintText();
		if (rit + 1 == re)
			rp.paintLast();
		y += rit->descent();
		// Restore full_repaint status.
		pi.full_repaint = tmp;
	}
	// Re-enable screen drawing for future use of the painter.
	pi.pain.setDrawingEnabled(true);

	//LYXERR(Debug::PAINTING) << "." << endl;
}


// only used for inset right now. should also be used for main text
void TextMetrics::drawSelection(PainterInfo & pi, int x, int) const
{
	Cursor & cur = bv_->cursor();
	if (!cur.selection())
		return;
	if (!ptr_cmp(cur.text(), text_))
		return;

	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< "draw selection at " << x
		<< endl;

	DocIterator beg = cur.selectionBegin();
	DocIterator end = cur.selectionEnd();

	// the selection doesn't touch the visible screen?
	if (bv_funcs::status(bv_, beg) == bv_funcs::CUR_BELOW
	    || bv_funcs::status(bv_, end) == bv_funcs::CUR_ABOVE)
		return;

	if (beg.pit() < par_metrics_.begin()->first)
		beg.pit() = par_metrics_.begin()->first;
	if (end.pit() > par_metrics_.rbegin()->first)
		end.pit() = par_metrics_.rbegin()->first;

	ParagraphMetrics const & pm1 = par_metrics_[beg.pit()];
	ParagraphMetrics const & pm2 = par_metrics_[end.pit()];
	Row const & row1 = pm1.getRow(beg.pos(), beg.boundary());
	Row const & row2 = pm2.getRow(end.pos(), end.boundary());

	// clip above
	int middleTop;
	bool const clipAbove = 
		(bv_funcs::status(bv_, beg) == bv_funcs::CUR_ABOVE);
	if (clipAbove)
		middleTop = 0;
	else
		middleTop = bv_funcs::getPos(*bv_, beg, beg.boundary()).y_ + row1.descent();
	
	// clip below
	int middleBottom;
	bool const clipBelow = 
		(bv_funcs::status(bv_, end) == bv_funcs::CUR_BELOW);
	if (clipBelow)
		middleBottom = bv_->workHeight();
	else
		middleBottom = bv_funcs::getPos(*bv_, end, end.boundary()).y_ - row2.ascent();

	// start and end in the same line?
	if (!(clipAbove || clipBelow) && &row1 == &row2)
		// then only draw this row's selection
		drawRowSelection(pi, x, row1, beg, end, false, false);
	else {
		if (!clipAbove) {
			// get row end
			DocIterator begRowEnd = beg;
			begRowEnd.pos() = row1.endpos();
			begRowEnd.boundary(true);
			
			// draw upper rectangle
			drawRowSelection(pi, x, row1, beg, begRowEnd, false, true);
		}
			
		if (middleTop < middleBottom) {
			// draw middle rectangle
			pi.pain.fillRectangle(x, middleTop, width(), middleBottom - middleTop,
				Color::selection);
		}

		if (!clipBelow) {
			// get row begin
			DocIterator endRowBeg = end;
			endRowBeg.pos() = row2.pos();
			endRowBeg.boundary(false);
			
			// draw low rectangle
			drawRowSelection(pi, x, row2, endRowBeg, end, true, false);
		}
	}
}


void TextMetrics::drawRowSelection(PainterInfo & pi, int x, Row const & row,
		DocIterator const & beg, DocIterator const & end,
		bool drawOnBegMargin, bool drawOnEndMargin) const
{
	Buffer & buffer = bv_->buffer();
	DocIterator cur = beg;
	int x1 = cursorX(beg.top(), beg.boundary());
	int x2 = cursorX(end.top(), end.boundary());
	int y1 = bv_funcs::getPos(*bv_, cur, cur.boundary()).y_ - row.ascent();
	int y2 = y1 + row.height();
	
	// draw the margins
	if (drawOnBegMargin) {
		if (text_->isRTL(buffer, beg.paragraph()))
			pi.pain.fillRectangle(x + x1, y1, width() - x1, y2 - y1, Color::selection);
		else
			pi.pain.fillRectangle(x, y1, x1, y2 - y1, Color::selection);
	}
	
	if (drawOnEndMargin) {
		if (text_->isRTL(buffer, beg.paragraph()))
			pi.pain.fillRectangle(x, y1, x2, y2 - y1, Color::selection);
		else
			pi.pain.fillRectangle(x + x2, y1, width() - x2, y2 - y1, Color::selection);
	}
	
	// if we are on a boundary from the beginning, it's probably
	// a RTL boundary and we jump to the other side directly as this
	// segement is 0-size and confuses the logic below
	if (cur.boundary())
		cur.boundary(false);
	
	// go through row and draw from RTL boundary to RTL boundary
	while (cur < end) {
		bool drawNow = false;
		
		// simplified cursorRight code below which does not
		// descend into insets and which does not go into the
		// next line. Compare the logic with the original cursorRight
		
		// if left of boundary -> just jump to right side
		// but for RTL boundaries don't, because: abc|DDEEFFghi -> abcDDEEF|Fghi
		if (cur.boundary()) {
			cur.boundary(false);
		}	else if (text_->isRTLBoundary(buffer, cur.paragraph(), cur.pos() + 1)) {
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
				Color::selection);
			
			// reset x1, so it is set again next round (which will be on the 
			// right side of a boundary or at the selection end)
			x1 = -1;
		}
	}
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
	return int(theFontMetrics(Font(Font::ALL_SANE)).maxHeight() *  1.2);
}

} // namespace lyx
