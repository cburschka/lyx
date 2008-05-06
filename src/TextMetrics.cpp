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
#include "BufferParams.h"
#include "BufferView.h"
#include "paragraph_funcs.h"
#include "ParIterator.h"
#include "CoordCache.h"
#include "debug.h"
#include "FuncRequest.h"
#include "FontIterator.h"
#include "Color.h"
#include "Length.h"
#include "LyXRC.h"
#include "Text.h"
#include "MetricsInfo.h"
#include "ParagraphParameters.h"
#include "VSpace.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

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

	//text_->updateLabels(*bv->buffer());
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


bool TextMetrics::metrics(MetricsInfo & mi, Dimension & dim, int min_width)
{
	BOOST_ASSERT(mi.base.textwidth);

	//lyxerr << "Text::metrics: width: " << mi.base.textwidth
	//	<< " maxWidth: " << max_width_ << "\nfont: " << mi.base.font << endl;

	bool changed = false;
	max_width_ = mi.base.textwidth;

	pit_type const npar = text_->paragraphs().size();
	unsigned int h = 0;
	unsigned int w = min_width;
	if (npar > 1)
		// If there is more than one row, expand the text to 
		// the full allowable width.
		w = max_width_;
	for (pit_type pit = 0, n = npar; pit != n; ++pit) {
		changed |= redoParagraph(pit);
		ParagraphMetrics const & pm = parMetrics(pit);
		h += pm.height();
		if (w < pm.width())
			w = pm.width();
	}

	dim.wid = w;
	dim.asc = parMetrics(0).ascent();
	dim.des = h - dim.asc;

	changed |= dim_ != dim;
	dim_ = dim;
	return changed;
}


int TextMetrics::rightMargin(ParagraphMetrics const & pm) const
{
	return main_text_? pm.rightMargin(*bv_->buffer()) : 0;
}


int TextMetrics::rightMargin(pit_type const pit) const
{
	return main_text_? par_metrics_[pit].rightMargin(*bv_->buffer()) : 0;
}


bool TextMetrics::redoParagraph(pit_type const pit)
{
	Paragraph & par = text_->getPar(pit);
	ParagraphMetrics pm(par);
	Buffer & buffer = *bv_->buffer();
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
		int const w = max_width_ - text_->leftMargin(buffer, max_width_, pit, ii->pos)
			- right_margin;
		Font const & font = ii->inset->noFontChange() ?
			bufferfont : text_->getFont(buffer, par, ii->pos);
		MetricsInfo mi(bv_, font, w);
		changed |= ii->inset->metrics(mi, dim);
		changed |= (old_dim != dim);
	}

	// rebreak the paragraph
	pm.rows().clear();

	par.setBeginOfBody();
	pos_type z = 0;
	// maximum pixel width of a row
	int width = max_width_ - right_margin; // - leftMargin(buffer, max_width_, pit, row);
	do {
		Row row(z);
		rowBreakPoint(width, pit, row);
		if (!pm.rows().empty() || row.endpos() < par.size())
			// If there is more than one row, expand the text to 
			// the full allowable width.
			pm.dim().wid = width;
		setRowWidth(right_margin, pit, row);
		setHeightOfRow(pit, row);
		pm.rows().push_back(row);
		pm.dim().wid = std::max(pm.dim().wid, row.width());
		pm.dim().des += row.height();
		z = row.endpos();
	} while (z < par.size());

	// Make sure that if a par ends in newline, there is one more row
	// under it
	if (z > 0 && par.isNewline(z - 1)) {
		Row row(z);
		row.endpos(z);
		setRowWidth(right_margin, pit, row);
		setHeightOfRow(pit, row);
		pm.rows().push_back(row);
		pm.dim().des += row.height();
	}

	pm.dim().asc += pm.rows()[0].ascent();
	pm.dim().des -= pm.rows()[0].ascent();

	// IMPORTANT NOTE: We pass 'false' explicitely in order to not call
	// redoParagraph() recursively inside parMetrics.
	Dimension old_dim = parMetrics(pit, false).dim();

	changed |= old_dim != pm.dim();

	par_metrics_[pit] = pm;

	return changed;
}

RowMetrics TextMetrics::computeRowMetrics(pit_type const pit,
		Row const & row) const
{
	RowMetrics result;
	Buffer & buffer = *bv_->buffer();
	Paragraph const & par = text_->getPar(pit);

	double w = dim_.wid - row.width();

	bool const is_rtl = text_->isRTL(buffer, par);
	if (is_rtl)
		result.x = rightMargin(pit);
	else
		result.x = text_->leftMargin(buffer, max_width_, pit, row.pos());

	// is there a manual margin with a manual label
	Layout_ptr const & layout = par.layout();

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
			result.label_hfill = labelFill(par, row) / double(nlh);
	}

	// are there any hfills in the row?
	int const nh = numberOfHfills(par, row);

	if (nh) {
		if (w > 0)
			result.hfill = w / nh;
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
		if (row.pos() < par.size() && par.isInset(row.pos())) {
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
				result.separator = w / ns;
			} else if (is_rtl) {
				result.x += w;
			}
			break;
		}
		case LYX_ALIGN_RIGHT:
			result.x += w;
			break;
		case LYX_ALIGN_CENTER:
			result.x += w / 2;
			break;
		}
	}

	if (is_rtl) {
		pos_type body_pos = par.beginOfBody();
		pos_type end = row.endpos();

		if (body_pos > 0
		    && (body_pos > end || !par.isLineSeparator(body_pos - 1)))
		{
			docstring const lsep = from_utf8(layout->labelsep);
			result.x += theFontMetrics(text_->getLabelFont(buffer, par)).width(lsep);
			if (body_pos <= end)
				result.x += result.label_hfill;
		}
	}

	return result;
}


int TextMetrics::labelFill(Paragraph const & par, Row const & row) const
{
	Buffer & buffer = *bv_->buffer();

	pos_type last = par.beginOfBody();
	BOOST_ASSERT(last > 0);

	// -1 because a label ends with a space that is in the label
	--last;

	// a separator at this end does not count
	if (par.isLineSeparator(last))
		--last;

	int w = 0;
	for (pos_type i = row.pos(); i <= last; ++i)
		w += text_->singleWidth(buffer, par, i);

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
	return text_->leftMargin(*bv_->buffer(), max_width_, pit);
}


void TextMetrics::rowBreakPoint(int width, pit_type const pit,
		Row & row) const
{
	Buffer & buffer = *bv_->buffer();
	Paragraph const & par = text_->getPar(pit);
	pos_type const end = par.size();
	pos_type const pos = row.pos();
	if (pos == end) {
		row.endpos(end);
		return;
	}

	if (width < 0) {
		row.endpos(end);
		return;
	}

	Layout_ptr const & layout = par.layout();

	if (layout->margintype == MARGIN_RIGHT_ADDRESS_BOX) {
		row.endpos(addressBreakPoint(pos, par));
		return;
	}

	pos_type const body_pos = par.beginOfBody();


	// Now we iterate through until we reach the right margin
	// or the end of the par, then choose the possible break
	// nearest that.

	int label_end = labelEnd(pit);
	int const left = text_->leftMargin(buffer, max_width_, pit, pos);
	int x = left;

	// pixel width since last breakpoint
	int chunkwidth = 0;

	FontIterator fi = FontIterator(buffer, *text_, par, pos);
	pos_type point = end;
	pos_type i = pos;
	FontMetrics const & fm = theFontMetrics(text_->getLabelFont(buffer, par));
	for ( ; i < end; ++i, ++fi) {
		char_type const c = par.getChar(i);
		int thiswidth = text_->singleWidth(par, i, c, *fi);

		// add the auto-hfill from label end to the body
		if (body_pos && i == body_pos) {
			docstring lsep = from_utf8(layout->labelsep);
			int add = fm.width(lsep);
			if (par.isLineSeparator(i - 1))
				add -= text_->singleWidth(buffer, par, i - 1);

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

	row.endpos(point);
}


void TextMetrics::setRowWidth(int right_margin,
		pit_type const pit, Row & row) const
{
	Buffer & buffer = *bv_->buffer();
	// get the pure distance
	pos_type const end = row.endpos();

	Paragraph const & par = text_->getPar(pit);
	docstring const labelsep = from_utf8(par.layout()->labelsep);
	int w = text_->leftMargin(buffer, max_width_, pit, row.pos());
	int label_end = labelEnd(pit);

	pos_type const body_pos = par.beginOfBody();
	pos_type i = row.pos();

	FontMetrics const & fm = theFontMetrics(text_->getLabelFont(buffer, par));

	if (i < end) {
		FontIterator fi = FontIterator(buffer, *text_, par, i);
		for ( ; i < end; ++i, ++fi) {
			if (body_pos > 0 && i == body_pos) {
				w += fm.width(labelsep);
				if (par.isLineSeparator(i - 1))
					w -= text_->singleWidth(buffer, par, i - 1);
				w = max(w, label_end);
			}
			char_type const c = par.getChar(i);
			w += text_->singleWidth(par, i, c, *fi);
		}
	}

	if (body_pos > 0 && body_pos >= end) {
		w += fm.width(labelsep);
		if (end > 0 && par.isLineSeparator(end - 1))
			w -= text_->singleWidth(buffer, par, end - 1);
		w = max(w, label_end);
	}

	row.width(w + right_margin);
}


void TextMetrics::setHeightOfRow(pit_type const pit,
		Row & row)
{
	Paragraph const & par = text_->getPar(pit);
	// get the maximum ascent and the maximum descent
	double layoutasc = 0;
	double layoutdesc = 0;
	double const dh = defaultRowHeight();

	// ok, let us initialize the maxasc and maxdesc value.
	// Only the fontsize count. The other properties
	// are taken from the layoutfont. Nicer on the screen :)
	Layout_ptr const & layout = par.layout();

	// as max get the first character of this row then it can
	// increase but not decrease the height. Just some point to
	// start with so we don't have to do the assignment below too
	// often.
	Buffer const & buffer = *bv_->buffer();
	Font font = text_->getFont(buffer, par, row.pos());
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
		if (ii->pos >= row.pos() && ii->pos < row.endpos()) {
			maxasc  = max(maxasc,  ii->inset->ascent());
			maxdesc = max(maxdesc, ii->inset->descent());
		}
	}

	// Check if any custom fonts are larger (Asger)
	// This is not completely correct, but we can live with the small,
	// cosmetic error for now.
	int labeladdon = 0;
	pos_type const pos_end = row.endpos();

	Font::FONT_SIZE maxsize =
		par.highestFontInRange(row.pos(), pos_end, size);
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

	row.ascent(maxasc);
	ParagraphList const & pars = text_->paragraphs();

	// is it a top line?
	if (row.pos() == 0) {
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
		} else if (pit != 0 || row.pos() != 0) {
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
	if (row.endpos() >= par.size()) {
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
		if (pit == 0 && row.pos() == 0)
			maxasc += 20;
		if (pit + 1 == pit_type(pars.size()) &&
		    row.endpos() == par.size() &&
				!(row.endpos() > 0 && par.isNewline(row.endpos() - 1)))
			maxdesc += 20;
	}

	row.ascent(maxasc + labeladdon);
	row.descent(maxdesc);
}


// x is an absolute screen coord
// returns the column near the specified x-coordinate of the row
// x is set to the real beginning of this column
pos_type TextMetrics::getColumnNearX(pit_type const pit,
		Row const & row, int & x, bool & boundary) const
{
	Buffer const & buffer = *bv_->buffer();

	/// For the main Text, it is possible that this pit is not
	/// yet in the CoordCache when moving cursor up.
	/// x Paragraph coordinate is always 0 for main text anyway.
	int const xo = main_text_? 0 : bv_->coordCache().get(text_, pit).x_;
	x -= xo;
	RowMetrics const r = computeRowMetrics(pit, row);
	Paragraph const & par = text_->getPar(pit);
	Bidi bidi;
	bidi.computeTables(par, buffer, row);

	pos_type vc = row.pos();
	pos_type end = row.endpos();
	pos_type c = 0;
	Layout_ptr const & layout = par.layout();

	bool left_side = false;

	pos_type body_pos = par.beginOfBody();

	double tmpx = r.x;
	double last_tmpx = tmpx;

	if (body_pos > 0 &&
	    (body_pos > end || !par.isLineSeparator(body_pos - 1)))
		body_pos = 0;

	// check for empty row
	if (vc == end) {
		x = int(tmpx) + xo;
		return 0;
	}

	frontend::FontMetrics const & fm
		= theFontMetrics(text_->getLabelFont(buffer, par));

	while (vc < end && tmpx <= x) {
		c = bidi.vis2log(vc);
		last_tmpx = tmpx;
		if (body_pos > 0 && c == body_pos - 1) {
			// FIXME UNICODE
			docstring const lsep = from_utf8(layout->labelsep);
			tmpx += r.label_hfill + fm.width(lsep);
			if (par.isLineSeparator(body_pos - 1))
				tmpx -= text_->singleWidth(buffer, par, body_pos - 1);
		}

		if (par.hfillExpansion(row, c)) {
			tmpx += text_->singleWidth(buffer, par, c);
			if (c >= body_pos)
				tmpx += r.hfill;
			else
				tmpx += r.label_hfill;
		} else if (par.isSeparator(c)) {
			tmpx += text_->singleWidth(buffer, par, c);
			if (c >= body_pos)
				tmpx += r.separator;
		} else {
			tmpx += text_->singleWidth(buffer, par, c);
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
			tmpx -= text_->singleWidth(buffer, par, end - 1);
		else
			tmpx += text_->singleWidth(buffer, par, end - 1);
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
	ParagraphMetrics const & pm = parMetrics(pit);
	BOOST_ASSERT(!pm.rows().empty());
	BOOST_ASSERT(row < int(pm.rows().size()));
	bool bound = false;
	Row const & r = pm.rows()[row];
	return r.pos() + getColumnNearX(pit, r, x, bound);
}


//int Text::pos2x(pit_type pit, pos_type pos) const
//{
//	ParagraphMetrics const & pm = parMetrics(pit);
//	Row const & r = pm.rows()[row];
//	int x = 0;
//	pos -= r.pos();
//}


int defaultRowHeight()
{
	return int(theFontMetrics(Font(Font::ALL_SANE)).maxHeight() *  1.2);
}

} // namespace lyx
