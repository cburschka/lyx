/**
 * \file src/TextMetrics.C
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

#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "pariterator.h"
#include "coordcache.h"
#include "debug.h"
#include "funcrequest.h"
#include "FontIterator.h"
#include "LColor.h"
#include "lyxlength.h"
#include "lyxtext.h"
#include "metricsinfo.h"
#include "ParagraphParameters.h"
#include "vspace.h"

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
	pos_type last = row.endpos();
	pos_type first = row.pos();
	pos_type const par_size = par.size();

	// hfill *DO* count at the beginning of paragraphs!
	if (first) {
		while (first < last && first < par_size && par.isHfill(first))
			++first;
	}

	first = max(first, par.beginOfBody());
	last = min(last, par_size);

	int n = 0;
	for (pos_type p = first; p < last; ++p) {
		if (par.isHfill(p))
			++n;
	}
	return n;
}

} // namespace anon

TextMetrics::TextMetrics(BufferView * bv, LyXText * text)
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


bool TextMetrics::metrics(MetricsInfo & mi, Dimension & dim)
{
	BOOST_ASSERT(mi.base.textwidth);
	max_width_ = mi.base.textwidth;

	//lyxerr << "LyXText::metrics: width: " << mi.base.textwidth
	//	<< " maxWidth: " << max_width_ << "\nfont: " << mi.base.font << endl;
	
	bool changed = false;

	unsigned int h = 0;
	unsigned int w = 0;
	for (pit_type pit = 0, n = text_->paragraphs().size(); pit != n; ++pit) {
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

	// FIXME: this has nothing to do here and is the reason why text_ is not
	// const.
	if (par.checkBiblio(buffer.params().trackChanges))
		const_cast<LCursor &>(bv_->cursor()).posRight();

	// Optimisation: this is used in the next two loops
	// so better to calculate that once here.
	int const right_margin = rightMargin(pm);

	// redo insets
	// FIXME: We should always use getFont(), see documentation of
	// noFontChange() in insetbase.h.
	LyXFont const bufferfont = buffer.params().getFont();
	InsetList::const_iterator ii = par.insetlist.begin();
	InsetList::const_iterator iend = par.insetlist.end();
	for (; ii != iend; ++ii) {
		Dimension dim;
		int const w = max_width_ - text_->leftMargin(buffer, max_width_, pit, ii->pos)
			- right_margin;
		LyXFont const & font = ii->inset->noFontChange() ?
			bufferfont : text_->getFont(buffer, par, ii->pos);
		MetricsInfo mi(bv_, font, w);
		changed |= ii->inset->metrics(mi, dim);
	}

	// rebreak the paragraph
	pm.rows().clear();

	par.setBeginOfBody();
	pos_type z = 0;
	do {
		Row row(z);
		text_->rowBreakPoint(buffer, right_margin, max_width_, pit, row);
		text_->setRowWidth(buffer, right_margin, max_width_, pit, row);
		text_->setHeightOfRow(*bv_, pit, row);
		pm.rows().push_back(row);
		pm.dim().wid = std::max(pm.dim().wid, row.width());
		pm.dim().des += row.height();
		z = row.endpos();
	} while (z < par.size());

	// Make sure that if a par ends in newline, there is one more row
	// under it
	// FIXME this is a dirty trick. Now the _same_ position in the
	// paragraph occurs in _two_ different rows, and has two different
	// display positions, leading to weird behaviour when moving up/down.
	if (z > 0 && par.isNewline(z - 1)) {
		Row row(z - 1);
		row.endpos(z - 1);
		text_->setRowWidth(buffer, right_margin, max_width_, pit, row);
		text_->setHeightOfRow(*bv_, pit, row);
		pm.rows().push_back(row);
		pm.dim().des += row.height();
	}

	pm.dim().asc += pm.rows()[0].ascent();
	pm.dim().des -= pm.rows()[0].ascent();

	// IMPORTANT NOTE: We pass 'false' explicitely in order to not call
	// redoParagraph() recursively inside parMetrics.
	Dimension old_dim = parMetrics(pit, false).dim();

	changed |= old_dim.height() != pm.dim().height();

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
	LyXLayout_ptr const & layout = par.layout();

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
		if (!par.empty()
		    && par.isInset(row.pos())
		    && par.getInset(row.pos())->display())
		{
			align = LYX_ALIGN_CENTER;
		}

		switch (align) {
		case LYX_ALIGN_BLOCK: {
			int const ns = numberOfSeparators(par, row);
			bool disp_inset = false;
			if (row.endpos() < par.size()) {
				InsetBase const * in = par.getInset(row.endpos());
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

	text_->bidi.computeTables(par, buffer, row);
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


int defaultRowHeight()
{
	return int(theFontMetrics(LyXFont(LyXFont::ALL_SANE)).maxHeight() *  1.2);
}

} // namespace lyx
