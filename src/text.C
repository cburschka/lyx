/**
 * \file src/text.C
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
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyxtext.h"

#include "author.h"
#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "encoding.h"
#include "funcrequest.h"
#include "FontIterator.h"
#include "gettext.h"
#include "language.h"
#include "LColor.h"
#include "lyxlength.h"
#include "lyxlex.h"
#include "lyxrc.h"
#include "lyxrow.h"
#include "lyxrow_funcs.h"
#include "metricsinfo.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "rowpainter.h"
#include "undo.h"
#include "vspace.h"
#include "WordLangTuple.h"

#include "frontends/font_metrics.h"
#include "frontends/LyXView.h"

#include "insets/insettext.h"

#include "support/lstrings.h"
#include "support/textutils.h"
#include "support/tostr.h"
#include "support/std_sstream.h"

using lyx::pos_type;
using lyx::word_location;

using lyx::support::bformat;
using lyx::support::contains;
using lyx::support::lowercase;
using lyx::support::split;
using lyx::support::uppercase;

using std::advance;
using std::distance;
using std::max;
using std::min;
using std::endl;
using std::string;


/// some space for drawing the 'nested' markers (in pixel)
extern int const NEST_MARGIN = 20;
/// margin for changebar
extern int const CHANGEBAR_MARGIN = 10;
/// right margin
extern int const RIGHT_MARGIN = 10;


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


unsigned int maxParagraphWidth(ParagraphList const & plist)
{
	unsigned int width = 0;
	ParagraphList::const_iterator pit = plist.begin();
	ParagraphList::const_iterator end = plist.end();
		for (; pit != end; ++pit)
			width = std::max(width, pit->width);
	return width;
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
	pos_type const last = row.endpos() - 1;
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


BufferView * LyXText::bv()
{
	BOOST_ASSERT(bv_owner != 0);
	return bv_owner;
}


double LyXText::spacing(Paragraph const & par) const
{
	if (par.params().spacing().isDefault())
		return bv()->buffer()->params().spacing().getValue();
	return par.params().spacing().getValue();
}


BufferView * LyXText::bv() const
{
	BOOST_ASSERT(bv_owner != 0);
	return bv_owner;
}


void LyXText::updateParPositions()
{
	ParagraphList::iterator pit = paragraphs().begin();
	ParagraphList::iterator end = paragraphs().end();
	for (height = 0; pit != end; ++pit) {
		pit->y = height;
		height += pit->height;
	}
}


int LyXText::textWidth() const
{
	return textwidth_;
}


int LyXText::singleWidth(ParagraphList::iterator pit, pos_type pos) const
{
	if (pos >= pit->size())
		return 0;

	char const c = pit->getChar(pos);
	return singleWidth(pit, pos, c, getFont(pit, pos));
}


int LyXText::singleWidth(ParagraphList::iterator pit,
			 pos_type pos, char c, LyXFont const & font) const
{
	if (pos >= pit->size()) {
		lyxerr << "in singleWidth(), pos: " << pos << endl;
		BOOST_ASSERT(false);
		return 0;
	}

	// The most common case is handled first (Asger)
	if (IsPrintable(c)) {
		if (!font.language()->RightToLeft()) {
			if ((lyxrc.font_norm_type == LyXRC::ISO_8859_6_8 ||
			     lyxrc.font_norm_type == LyXRC::ISO_10646_1)
			    && font.language()->lang() == "arabic") {
				if (Encodings::IsComposeChar_arabic(c))
					return 0;
				else
					c = pit->transformChar(c, pos);
			} else if (font.language()->lang() == "hebrew" &&
				 Encodings::IsComposeChar_hebrew(c))
				return 0;
		}
		return font_metrics::width(c, font);
	}

	if (c == Paragraph::META_INSET)
		return pit->getInset(pos)->width();

	if (IsSeparatorChar(c))
		c = ' ';
	return font_metrics::width(c, font);
}


int LyXText::leftMargin(ParagraphList::iterator pit) const
{
	return leftMargin(pit, pit->size());
}


int LyXText::leftMargin(ParagraphList::iterator pit, pos_type pos) const
{
	LyXTextClass const & tclass =
		bv()->buffer()->params().getLyXTextClass();
	LyXLayout_ptr const & layout = pit->layout();

	string parindent = layout->parindent;

	int x = NEST_MARGIN + CHANGEBAR_MARGIN;

	x += font_metrics::signedWidth(tclass.leftmargin(), tclass.defaultfont());

	// This is the way LyX handles LaTeX-Environments.
	// I have had this idea very late, so it seems to be a
	// later added hack and this is true
	if (pit->getDepth() == 0) {
		if (pit->layout() == tclass.defaultLayout()) {
			// find the previous same level paragraph
			if (pit != paragraphs().begin()) {
				ParagraphList::iterator newpit =
					depthHook(pit, paragraphs(), pit->getDepth());
				if (newpit == pit && newpit->layout()->nextnoindent)
					parindent.erase();
			}
		}
	} else {
		// find the next level paragraph
		ParagraphList::iterator newpar =
			outerHook(pit, paragraphs());

		// Make a corresponding row. Need to call leftMargin()
		// to check whether it is a sufficent paragraph.
		if (newpar != paragraphs().end()
		    && newpar->layout()->isEnvironment()) {
			x = leftMargin(newpar);
		}

		if (newpar != paragraphs().end()
		    && pit->layout() == tclass.defaultLayout()) {
			if (newpar->params().noindent())
				parindent.erase();
			else
				parindent = newpar->layout()->parindent;
		}
	}

	LyXFont const labelfont = getLabelFont(pit);
	switch (layout->margintype) {
	case MARGIN_DYNAMIC:
		if (!layout->leftmargin.empty())
			x += font_metrics::signedWidth(layout->leftmargin,
						  tclass.defaultfont());
		if (!pit->getLabelstring().empty()) {
			x += font_metrics::signedWidth(layout->labelindent,
						  labelfont);
			x += font_metrics::width(pit->getLabelstring(),
					    labelfont);
			x += font_metrics::width(layout->labelsep, labelfont);
		}
		break;

	case MARGIN_MANUAL:
		x += font_metrics::signedWidth(layout->labelindent, labelfont);
		// The width of an empty par, even with manual label, should be 0
		if (!pit->empty() && pos >= pit->beginOfBody()) {
			if (!pit->getLabelWidthString().empty()) {
				x += font_metrics::width(pit->getLabelWidthString(),
					       labelfont);
				x += font_metrics::width(layout->labelsep, labelfont);
			}
		}
		break;

	case MARGIN_STATIC:
		x += font_metrics::signedWidth(layout->leftmargin, tclass.defaultfont()) * 4
			/ (pit->getDepth() + 4);
		break;

	case MARGIN_FIRST_DYNAMIC:
		if (layout->labeltype == LABEL_MANUAL) {
			if (pos >= pit->beginOfBody()) {
				x += font_metrics::signedWidth(layout->leftmargin,
							  labelfont);
			} else {
				x += font_metrics::signedWidth(layout->labelindent,
							  labelfont);
			}
		} else if (pos != 0
			   // Special case to fix problems with
			   // theorems (JMarc)
			   || (layout->labeltype == LABEL_STATIC
			       && layout->latextype == LATEX_ENVIRONMENT
			       && !isFirstInSequence(pit, paragraphs()))) {
			x += font_metrics::signedWidth(layout->leftmargin,
						  labelfont);
		} else if (layout->labeltype != LABEL_TOP_ENVIRONMENT
			   && layout->labeltype != LABEL_BIBLIO
			   && layout->labeltype !=
			   LABEL_CENTERED_TOP_ENVIRONMENT) {
			x += font_metrics::signedWidth(layout->labelindent,
						  labelfont);
			x += font_metrics::width(layout->labelsep, labelfont);
			x += font_metrics::width(pit->getLabelstring(),
					    labelfont);
		}
		break;

	case MARGIN_RIGHT_ADDRESS_BOX: {
#if 0
		// ok, a terrible hack. The left margin depends on the widest
		// row in this paragraph.
		RowList::iterator rit = pit->rows.begin();
		RowList::iterator end = pit->rows.end();
#warning This is wrong.
		int minfill = textwidth_;
		for ( ; rit != end; ++rit)
			if (rit->fill() < minfill)
				minfill = rit->fill();
		x += font_metrics::signedWidth(layout->leftmargin,
			tclass.defaultfont());
		x += minfill;
#endif
		// also wrong, but much shorter.
		x += textwidth_ / 2;
		break;
	}
	}
	

	if (!pit->params().leftIndent().zero())
		x += pit->params().leftIndent().inPixels(textWidth());

	LyXAlignment align;

	if (pit->params().align() == LYX_ALIGN_LAYOUT)
		align = layout->align;
	else
		align = pit->params().align();

	// set the correct parindent
	if (pos == 0
	    && (layout->labeltype == LABEL_NO_LABEL
	       || layout->labeltype == LABEL_TOP_ENVIRONMENT
	       || layout->labeltype == LABEL_CENTERED_TOP_ENVIRONMENT
	       || (layout->labeltype == LABEL_STATIC
	           && layout->latextype == LATEX_ENVIRONMENT
	           && !isFirstInSequence(pit, paragraphs())))
	    && align == LYX_ALIGN_BLOCK
	    && !pit->params().noindent()
	    // in tabulars and ert paragraphs are never indented!
	    && (!pit->inInset()
	        || !pit->inInset()->owner()
	        || (pit->inInset()->owner()->lyxCode() != InsetOld::TABULAR_CODE
	            && pit->inInset()->owner()->lyxCode() != InsetOld::ERT_CODE))
	    && (pit->layout() != tclass.defaultLayout()
	        || bv()->buffer()->params().paragraph_separation ==
	           BufferParams::PARSEP_INDENT))
	{
		x += font_metrics::signedWidth(parindent, tclass.defaultfont());
	}

	return x;
}


int LyXText::rightMargin(Paragraph const & par) const
{
	LyXTextClass const & tclass = bv()->buffer()->params().getLyXTextClass();

	return
		RIGHT_MARGIN
		+ font_metrics::signedWidth(tclass.rightmargin(),
				       tclass.defaultfont())
		+ font_metrics::signedWidth(par.layout()->rightmargin,
				       tclass.defaultfont())
		* 4 / (par.getDepth() + 4);
}


int LyXText::labelEnd(ParagraphList::iterator pit) const
{
	// labelEnd is only needed if the layout fills a flushleft label.
	if (pit->layout()->margintype != MARGIN_MANUAL)
		return 0;
	// return the beginning of the body
	return leftMargin(pit);
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


void LyXText::rowBreakPoint(ParagraphList::iterator pit, Row & row) const
{
	pos_type const end = pit->size();
	pos_type const pos = row.pos();
	if (pos == end) {
		row.endpos(end);
		return;
	}

	// maximum pixel width of a row
	int width = textWidth() - rightMargin(*pit); // - leftMargin(pit, row);
	if (width < 0) {
		row.endpos(end);
		return;
	}

	LyXLayout_ptr const & layout = pit->layout();

	if (layout->margintype == MARGIN_RIGHT_ADDRESS_BOX) {
		row.endpos(addressBreakPoint(pos, *pit));
		return;
	}

	pos_type const body_pos = pit->beginOfBody();


	// Now we iterate through until we reach the right margin
	// or the end of the par, then choose the possible break
	// nearest that.

	int const left = leftMargin(pit, pos);
	int x = left;

	// pixel width since last breakpoint
	int chunkwidth = 0;

	FontIterator fi = FontIterator(*this, pit, pos);
	pos_type point = end;
	pos_type i = pos;
	for ( ; i < end; ++i, ++fi) {
		if (pit->isNewline(i)) {
			point = i + 1;
			break;
		}
		// Break before...
		if (i + 1 < end) {
			if (pit->isInset(i + 1) && pit->getInset(i + 1)->display()) {
				point = i + 1;
				break;
			}
			// ...and after.
			if (pit->isInset(i) && pit->getInset(i)->display()) {
				point = i + 1;
				break;
			}
		}

		char const c = pit->getChar(i);

		{
			int thiswidth = singleWidth(pit, i, c, *fi);

			// add the auto-hfill from label end to the body
			if (body_pos && i == body_pos) {
				int add = font_metrics::width(layout->labelsep, getLabelFont(pit));
				if (pit->isLineSeparator(i - 1))
					add -= singleWidth(pit, i - 1);

				add = std::max(add, labelEnd(pit) - x);
				thiswidth += add;
			}

			x += thiswidth;
			chunkwidth += thiswidth;
		}

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

		if (!pit->isInset(i) || pit->getInset(i)->isChar()) {
			// some insets are line separators too
			if (pit->isLineSeparator(i)) {
				// register breakpoint:
				point = i + 1;
				chunkwidth = 0;
			}
		}
	}

	if (i == end && x < width) {
		// maybe found one, but the par is short enough.
		point = end;
	}

	// manual labels cannot be broken in LaTeX. But we
	// want to make our on-screen rendering of footnotes
	// etc. still break
	if (body_pos && point < body_pos)
		point = body_pos;

	row.endpos(point);
}


void LyXText::setRowWidth(ParagraphList::iterator pit, Row & row) const
{
	// get the pure distance
	pos_type const end = row.endpos();

	string labelsep = pit->layout()->labelsep;
	int w = leftMargin(pit, row.pos());

	pos_type const body_pos = pit->beginOfBody();
	pos_type i = row.pos();

	if (i < end) {
		FontIterator fi = FontIterator(*this, pit, i);
		for ( ; i < end; ++i, ++fi) {
			if (body_pos > 0 && i == body_pos) {
				w += font_metrics::width(labelsep, getLabelFont(pit));
				if (pit->isLineSeparator(i - 1))
					w -= singleWidth(pit, i - 1);
				w = max(w, labelEnd(pit));
			}
			char const c = pit->getChar(i);
			w += singleWidth(pit, i, c, *fi);
		}
	}

	if (body_pos > 0 && body_pos >= end) {
		w += font_metrics::width(labelsep, getLabelFont(pit));
		if (end > 0 && pit->isLineSeparator(end - 1))
			w -= singleWidth(pit, end - 1);
		w = max(w, labelEnd(pit));
	}

	row.width(w + rightMargin(*pit));
}


// returns the minimum space a manual label needs on the screen in pixel
int LyXText::labelFill(ParagraphList::iterator pit, Row const & row) const
{
	pos_type last = pit->beginOfBody();

	BOOST_ASSERT(last > 0);

	// -1 because a label ends with a space that is in the label
	--last;

	// a separator at this end does not count
	if (pit->isLineSeparator(last))
		--last;

	int w = 0;
	for (pos_type i = row.pos(); i <= last; ++i)
		w += singleWidth(pit, i);

	string const & label = pit->params().labelWidthString();
	if (label.empty())
		return 0;

	return max(0, font_metrics::width(label, getLabelFont(pit)) - w);
}


LColor_color LyXText::backgroundColor() const
{
	return LColor_color(LColor::color(background_color_));
}


void LyXText::setHeightOfRow(ParagraphList::iterator pit, Row & row)
{
	// get the maximum ascent and the maximum descent
	double layoutasc = 0;
	double layoutdesc = 0;
	double const dh = defaultRowHeight();

	// ok, let us initialize the maxasc and maxdesc value.
	// Only the fontsize count. The other properties
	// are taken from the layoutfont. Nicer on the screen :)
	LyXLayout_ptr const & layout = pit->layout();

	// as max get the first character of this row then it can
	// increase but not decrease the height. Just some point to
	// start with so we don't have to do the assignment below too
	// often.
	LyXFont font = getFont(pit, row.pos());
	LyXFont::FONT_SIZE const tmpsize = font.size();
	font = getLayoutFont(pit);
	LyXFont::FONT_SIZE const size = font.size();
	font.setSize(tmpsize);

	LyXFont labelfont = getLabelFont(pit);

	// these are minimum values
	double const spacing_val = layout->spacing.getValue() * spacing(*pit);
	//lyxerr << "spacing_val = " << spacing_val << endl;
	int maxasc  = int(font_metrics::maxAscent(font)  * spacing_val);
	int maxdesc = int(font_metrics::maxDescent(font) * spacing_val);

	// insets may be taller
	InsetList::iterator ii = pit->insetlist.begin();
	InsetList::iterator iend = pit->insetlist.end();
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

	LyXFont::FONT_SIZE maxsize =
		pit->highestFontInRange(row.pos(), pos_end, size);
	if (maxsize > font.size()) {
		font.setSize(maxsize);
		maxasc  = max(maxasc,  font_metrics::maxAscent(font));
		maxdesc = max(maxdesc, font_metrics::maxDescent(font));
	}

	// This is nicer with box insets:
	++maxasc;
	++maxdesc;

	row.ascent_of_text(maxasc);

	// is it a top line?
	if (row.pos() == 0) {
		BufferParams const & bufparams = bv()->buffer()->params();
		// some parksips VERY EASY IMPLEMENTATION
		if (bv()->buffer()->params().paragraph_separation
		    == BufferParams::PARSEP_SKIP
			&& pit != paragraphs().begin()
			&& ((layout->isParagraph() && pit->getDepth() == 0)
			    || (boost::prior(pit)->layout()->isParagraph()
			        && boost::prior(pit)->getDepth() == 0)))
		{
				maxasc += bufparams.getDefSkip().inPixels(*bv());
		}

		if (pit->params().startOfAppendix())
			maxasc += int(3 * dh);

		// This is special code for the chapter, since the label of this
		// layout is printed in an extra row
		if (layout->counter == "chapter" && bufparams.secnumdepth >= 0) {
			labeladdon = int(font_metrics::maxHeight(labelfont)
			             * layout->spacing.getValue() * spacing(*pit));
		}

		// special code for the top label
		if ((layout->labeltype == LABEL_TOP_ENVIRONMENT
		     || layout->labeltype == LABEL_BIBLIO
		     || layout->labeltype == LABEL_CENTERED_TOP_ENVIRONMENT)
		    && isFirstInSequence(pit, paragraphs())
		    && !pit->getLabelstring().empty())
		{
			labeladdon = int(
				  font_metrics::maxHeight(labelfont)
					* layout->spacing.getValue()
					* spacing(*pit)
				+ (layout->topsep + layout->labelbottomsep) * dh);
		}

		// Add the layout spaces, for example before and after
		// a section, or between the items of a itemize or enumerate
		// environment.

		ParagraphList::iterator prev =
			depthHook(pit, paragraphs(), pit->getDepth());
		if (prev != pit
		    && prev->layout() == layout
		    && prev->getDepth() == pit->getDepth()
		    && prev->getLabelWidthString() == pit->getLabelWidthString())
		{
			layoutasc = layout->itemsep * dh;
		} else if (pit != paragraphs().begin() || row.pos() != 0) {
			if (layout->topsep > 0)
				layoutasc = layout->topsep * dh;
		}

		prev = outerHook(pit, paragraphs());
		if (prev != paragraphs().end()) {
			maxasc += int(prev->layout()->parsep * dh);
		} else if (pit != paragraphs().begin()) {
			ParagraphList::iterator prior_pit = boost::prior(pit);
			if (prior_pit->getDepth() != 0 ||
					prior_pit->layout() == layout) {
				maxasc += int(layout->parsep * dh);
			}
		}
	}

	// is it a bottom line?
	if (row.endpos() >= pit->size()) {
		// add the layout spaces, for example before and after
		// a section, or between the items of a itemize or enumerate
		// environment
		ParagraphList::iterator nextpit = boost::next(pit);
		if (nextpit != paragraphs().end()) {
			ParagraphList::iterator cpit = pit;
			double usual = 0;
			double unusual = 0;

			if (cpit->getDepth() > nextpit->getDepth()) {
				usual = cpit->layout()->bottomsep * dh;
				cpit = depthHook(cpit, paragraphs(), nextpit->getDepth());
				if (cpit->layout() != nextpit->layout()
					|| nextpit->getLabelWidthString() != cpit->getLabelWidthString())
				{
					unusual = cpit->layout()->bottomsep * dh;
				}
				layoutdesc = max(unusual, usual);
			} else if (cpit->getDepth() == nextpit->getDepth()) {
				if (cpit->layout() != nextpit->layout()
					|| nextpit->getLabelWidthString() != cpit->getLabelWidthString())
					layoutdesc = int(cpit->layout()->bottomsep * dh);
			}
		}
	}

	// incalculate the layout spaces
	maxasc  += int(layoutasc  * 2 / (2 + pit->getDepth()));
	maxdesc += int(layoutdesc * 2 / (2 + pit->getDepth()));

	row.height(maxasc + maxdesc + labeladdon);
	row.baseline(maxasc + labeladdon);
	row.top_of_text(row.baseline() - font_metrics::maxAscent(font));
}


void LyXText::breakParagraph(LCursor & cur, char keep_layout)
{
	BOOST_ASSERT(this == cur.text());
	// allow only if at start or end, or all previous is new text
	Paragraph & cpar = cur.paragraph();
	ParagraphList::iterator cpit = getPar(cur.par());

	if (cur.pos() != 0 && cur.pos() != cur.lastpos()
	    && cpar.isChangeEdited(0, cur.pos()))
		return;

	LyXTextClass const & tclass =
		bv()->buffer()->params().getLyXTextClass();
	LyXLayout_ptr const & layout = cpar.layout();

	// this is only allowed, if the current paragraph is not empty
	// or caption and if it has not the keepempty flag active
	if (cur.lastpos() == 0 && !cpar.allowEmpty()
	   && layout->labeltype != LABEL_SENSITIVE)
		return;

	// a layout change may affect also the following paragraph
	recUndo(cur.par(), parOffset(undoSpan(cpit)) - 1);

	// Always break behind a space
	// It is better to erase the space (Dekel)
	if (cur.pos() != cur.lastpos() && cpar.isLineSeparator(cur.pos()))
		cpar.erase(cur.pos());

	// break the paragraph
	if (keep_layout)
		keep_layout = 2;
	else
		keep_layout = layout->isEnvironment();

	// we need to set this before we insert the paragraph. IMO the
	// breakParagraph call should return a bool if it inserts the
	// paragraph before or behind and we should react on that one
	// but we can fix this in 1.3.0 (Jug 20020509)
	bool const isempty = cpar.allowEmpty() && cpar.empty();
	::breakParagraph(bv()->buffer()->params(), paragraphs(), cpit,
			 cur.pos(), keep_layout);

	cpit = getPar(cur.par());
	ParagraphList::iterator next_par = boost::next(cpit);

	// well this is the caption hack since one caption is really enough
	if (layout->labeltype == LABEL_SENSITIVE) {
		if (!cur.pos())
			// set to standard-layout
			cpit->applyLayout(tclass.defaultLayout());
		else
			// set to standard-layout
			next_par->applyLayout(tclass.defaultLayout());
	}

	// if the cursor is at the beginning of a row without prior newline,
	// move one row up!
	// This touches only the screen-update. Otherwise we would may have
	// an empty row on the screen
	if (cur.pos() != 0 && cur.textRow().pos() == cur.pos()
	    && !cpit->isNewline(cur.pos() - 1))
	{
		cursorLeft(cur);
	}

	while (!next_par->empty() && next_par->isNewline(0))
		next_par->erase(0);

	updateCounters();
	redoParagraph(cpit);
	redoParagraph(next_par);

	// This check is necessary. Otherwise the new empty paragraph will
	// be deleted automatically. And it is more friendly for the user!
	if (cur.pos() != 0 || isempty)
		setCursor(cur, cur.par() + 1, 0);
	else
		setCursor(cur, cur.par(), 0);
}


// convenience function
void LyXText::redoParagraph(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	cur.clearSelection();
	redoParagraph(getPar(cur.par()));
	setCursorIntern(cur, cur.par(), cur.pos());
}


// insert a character, moves all the following breaks in the
// same Paragraph one to the right and make a rebreak
void LyXText::insertChar(LCursor & cur, char c)
{
	BOOST_ASSERT(this == cur.text());
	recordUndo(cur, Undo::INSERT);

	Paragraph & par = cur.paragraph();
	// try to remove this
	ParagraphList::iterator pit = getPar(cur.par());

	bool const freeSpacing = par.layout()->free_spacing ||
		par.isFreeSpacing();

	if (lyxrc.auto_number) {
		static string const number_operators = "+-/*";
		static string const number_unary_operators = "+-";
		static string const number_seperators = ".,:";

		if (current_font.number() == LyXFont::ON) {
			if (!IsDigit(c) && !contains(number_operators, c) &&
			    !(contains(number_seperators, c) &&
			      cur.pos() != 0 &&
			      cur.pos() != cur.lastpos() &&
			      getFont(pit, cur.pos()).number() == LyXFont::ON &&
			      getFont(pit, cur.pos() - 1).number() == LyXFont::ON)
			   )
				number(cur); // Set current_font.number to OFF
		} else if (IsDigit(c) &&
			   real_current_font.isVisibleRightToLeft()) {
			number(cur); // Set current_font.number to ON

			if (cur.pos() != 0) {
				char const c = par.getChar(cur.pos() - 1);
				if (contains(number_unary_operators, c) &&
				    (cur.pos() == 1
				     || par.isSeparator(cur.pos() - 2)
				     || par.isNewline(cur.pos() - 2))
				  ) {
					setCharFont(pit, cur.pos() - 1, current_font);
				} else if (contains(number_seperators, c)
				     && cur.pos() >= 2
				     && getFont(pit, cur.pos() - 2).number() == LyXFont::ON) {
					setCharFont(pit, cur.pos() - 1, current_font);
				}
			}
		}
	}

	// First check, if there will be two blanks together or a blank at
	// the beginning of a paragraph.
	// I decided to handle blanks like normal characters, the main
	// difference are the special checks when calculating the row.fill
	// (blank does not count at the end of a row) and the check here

	// The bug is triggered when we type in a description environment:
	// The current_font is not changed when we go from label to main text
	// and it should (along with realtmpfont) when we type the space.
	// CHECK There is a bug here! (Asger)

	// store the current font.  This is because of the use of cursor
	// movements. The moving cursor would refresh the current font
	LyXFont realtmpfont = real_current_font;
	LyXFont rawtmpfont = current_font;

	// When the free-spacing option is set for the current layout,
	// disable the double-space checking
	if (!freeSpacing && IsLineSeparatorChar(c)) {
		if (cur.pos() == 0) {
			static bool sent_space_message = false;
			if (!sent_space_message) {
				cur.message(_("You cannot insert a space at the "
					"beginning of a paragraph. Please read the Tutorial."));
				sent_space_message = true;
				return;
			}
		}
		BOOST_ASSERT(cur.pos() > 0);
		if (par.isLineSeparator(cur.pos() - 1)
		    || par.isNewline(cur.pos() - 1)) {
			static bool sent_space_message = false;
			if (!sent_space_message) {
				cur.message(_("You cannot type two spaces this way. "
					"Please read the Tutorial."));
				sent_space_message = true;
			}
			return;
		}
	}

	// Here case LyXText::InsertInset already inserted the character
	if (c != Paragraph::META_INSET)
		par.insertChar(cur.pos(), c);

	setCharFont(pit, cur.pos(), rawtmpfont);

	current_font = rawtmpfont;
	real_current_font = realtmpfont;
	redoParagraph(cur);
	setCursor(cur, cur.par(), cur.pos() + 1, false, cur.boundary());
	charInserted();
}


void LyXText::charInserted()
{
	// Here we call finishUndo for every 20 characters inserted.
	// This is from my experience how emacs does it. (Lgb)
	static unsigned int counter;
	if (counter < 20) {
		++counter;
	} else {
		finishUndo();
		counter = 0;
	}
}


RowMetrics
LyXText::prepareToPrint(ParagraphList::iterator pit, Row const & row) const
{
	RowMetrics result;

	double w = width - row.width();

	bool const is_rtl = isRTL(*pit);
	if (is_rtl)
		result.x = rightMargin(*pit);
	else
		result.x = leftMargin(pit, row.pos());

	// is there a manual margin with a manual label
	LyXLayout_ptr const & layout = pit->layout();

	if (layout->margintype == MARGIN_MANUAL
	    && layout->labeltype == LABEL_MANUAL) {
		/// We might have real hfills in the label part
		int nlh = numberOfLabelHfills(*pit, row);

		// A manual label par (e.g. List) has an auto-hfill
		// between the label text and the body of the
		// paragraph too.
		// But we don't want to do this auto hfill if the par
		// is empty.
		if (!pit->empty())
			++nlh;

		if (nlh && !pit->getLabelWidthString().empty())
			result.label_hfill = labelFill(pit, row) / double(nlh);
	}

	// are there any hfills in the row?
	int const nh = numberOfHfills(*pit, row);

	if (nh) {
		if (w > 0)
			result.hfill = w / nh;
	// we don't have to look at the alignment if it is ALIGN_LEFT and
	// if the row is already larger then the permitted width as then
	// we force the LEFT_ALIGN'edness!
	} else if (int(row.width()) < textWidth()) {
		// is it block, flushleft or flushright?
		// set x how you need it
		int align;
		if (pit->params().align() == LYX_ALIGN_LAYOUT)
			align = layout->align;
		else
			align = pit->params().align();

		// Display-style insets should always be on a centred row
		// The test on pit->size() is to catch zero-size pars, which
		// would trigger the assert in Paragraph::getInset().
		//inset = pit->size() ? pit->getInset(row.pos()) : 0;
		if (!pit->empty()
		    && pit->isInset(row.pos())
		    && pit->getInset(row.pos())->display())
		{
			align = LYX_ALIGN_CENTER;
		}

		switch (align) {
		case LYX_ALIGN_BLOCK: {
			int const ns = numberOfSeparators(*pit, row);
			bool disp_inset = false;
			if (row.endpos() < pit->size()) {
				InsetBase * in = pit->getInset(row.endpos());
				if (in)
					disp_inset = in->display();
			}
			// If we have separators, this is not the last row of a
			// par, does not end in newline, and is not row above a
			// display inset... then stretch it
			if (ns
			    && row.endpos() < pit->size()
			    && !pit->isNewline(row.endpos() - 1)
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

	bidi.computeTables(*pit, *bv()->buffer(), row);
	if (is_rtl) {
		pos_type body_pos = pit->beginOfBody();
		pos_type end = row.endpos();

		if (body_pos > 0
		    && (body_pos > end || !pit->isLineSeparator(body_pos - 1)))
		{
			result.x += font_metrics::width(layout->labelsep, getLabelFont(pit));
			if (body_pos <= end)
				result.x += result.label_hfill;
		}
	}

	return result;
}


// the cursor set functions have a special mechanism. When they
// realize, that you left an empty paragraph, they will delete it.

void LyXText::cursorRightOneWord(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.pos() == cur.lastpos() && cur.par() != cur.lastpar()) {
		++cur.par();
		cur.pos() = 0;
	} else {
		// Skip through initial nonword stuff.
		// Treat floats and insets as words.
		while (cur.pos() != cur.lastpos() && !cur.paragraph().isWord(cur.pos()))
			++cur.pos();
		// Advance through word.
		while (cur.pos() != cur.lastpos() && cur.paragraph().isWord(cur.pos()))
			++cur.pos();
	}
	setCursor(cur, cur.par(), cur.pos());
}


void LyXText::cursorLeftOneWord(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.pos() == 0 && cur.par() != 0) {
		--cur.par();
		cur.pos() = cur.lastpos();
	} else { 
		// Skip through initial nonword stuff.
		// Treat floats and insets as words.
		while (cur.pos() != 0 && !cur.paragraph().isWord(cur.pos() - 1))
			--cur.pos();
		// Advance through word.
		while (cur.pos() != 0 && cur.paragraph().isWord(cur.pos() - 1))
			--cur.pos();
	}
	setCursor(cur, cur.par(), cur.pos());
}


void LyXText::selectWord(LCursor & cur, word_location loc)
{
	BOOST_ASSERT(this == cur.text());
	CursorSlice from = cur.current();
	CursorSlice to = cur.current();
	getWord(from, to, loc);
	if (cur.current() != from)
		setCursor(cur, from.par(), from.pos());
	if (to == from)
		return;
	cur.resetAnchor();
	setCursor(cur, to.par(), to.pos());
	cur.setSelection();
}


// Select the word currently under the cursor when no
// selection is currently set
bool LyXText::selectWordWhenUnderCursor(LCursor & cur, word_location loc)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.selection())
		return false;
	selectWord(cur, loc);
	return cur.selection();
}


void LyXText::acceptChange(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (!cur.selection() && cur.lastpos() != 0)
		return;

	CursorSlice const & startc = cur.selBegin();
	CursorSlice const & endc = cur.selEnd();
	if (startc.par() == endc.par()) {
		recordUndoSelection(cur, Undo::INSERT);
		getPar(startc)->acceptChange(startc.pos(), endc.pos());
		finishUndo();
		cur.clearSelection();
		redoParagraph(getPar(startc));
		setCursorIntern(cur, startc.par(), 0);
	}
#warning handle multi par selection
}


void LyXText::rejectChange(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (!cur.selection() && cur.lastpos() != 0)
		return;

	CursorSlice const & startc = cur.selBegin();
	CursorSlice const & endc = cur.selEnd();
	if (startc.par() == endc.par()) {
		recordUndoSelection(cur, Undo::INSERT);
		getPar(startc)->rejectChange(startc.pos(), endc.pos());
		finishUndo();
		cur.clearSelection();
		redoParagraph(getPar(startc));
		setCursorIntern(cur, startc.par(), 0);
	}
#warning handle multi par selection
}


// Delete from cursor up to the end of the current or next word.
void LyXText::deleteWordForward(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.lastpos() == 0)
		cursorRight(cur);
	else {
		cur.resetAnchor();
		cur.selection() = true;
		cursorRightOneWord(cur);
		cur.setSelection();
		cutSelection(cur, true, false);
	}
}


// Delete from cursor to start of current or prior word.
void LyXText::deleteWordBackward(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.lastpos() == 0)
		cursorLeft(cur);
	else {
		cur.resetAnchor();
		cur.selection() = true;
		cursorLeftOneWord(cur);
		cur.setSelection();
		cutSelection(cur, true, false);
	}
}


// Kill to end of line.
void LyXText::deleteLineForward(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.lastpos() == 0) {
		// Paragraph is empty, so we just go to the right
		cursorRight(cur);
	} else {
		cur.resetAnchor();
		cur.selection() = true; // to avoid deletion
		cursorEnd(cur);
		cur.setSelection();
		// What is this test for ??? (JMarc)
		if (!cur.selection())
			deleteWordForward(cur);
		else
			cutSelection(cur, true, false);
	}
}


void LyXText::changeCase(LCursor & cur, LyXText::TextCase action)
{
	BOOST_ASSERT(this == cur.text());
	CursorSlice from;
	CursorSlice to;

	if (cur.selection()) {
		from = cur.selBegin();
		to = cur.selEnd();
	} else {
		from = cursor();
		getWord(from, to, lyx::PARTIAL_WORD);
		setCursor(cur, to.par(), to.pos() + 1);
	}

	recordUndoSelection(cur);

	pos_type pos = from.pos();
	int par = from.par();

	while (par != int(paragraphs().size()) &&
	       (pos != to.pos() || par != to.par())) {
		ParagraphList::iterator pit = getPar(par);
		if (pos == pit->size()) {
			++par;
			pos = 0;
			continue;
		}
		unsigned char c = pit->getChar(pos);
		if (c != Paragraph::META_INSET) {
			switch (action) {
			case text_lowercase:
				c = lowercase(c);
				break;
			case text_capitalization:
				c = uppercase(c);
				action = text_lowercase;
				break;
			case text_uppercase:
				c = uppercase(c);
				break;
			}
		}
#warning changes
		pit->setChar(pos, c);
		++pos;
	}
}


void LyXText::Delete(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	// just move to the right, if we had success make a backspace
	CursorSlice sl = cur.current();
	cursorRight(cur);
	if (sl == cur.current()) {
		recordUndo(cur, Undo::DELETE, cur.par(), max(0, cur.par() - 1));
		backspace(cur);
	}
}


void LyXText::backspace(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.pos() == 0) {
		// The cursor is at the beginning of a paragraph, so
		// the the backspace will collapse two paragraphs into
		// one.

		// but it's not allowed unless it's new
		Paragraph & par = cur.paragraph();
		if (par.isChangeEdited(0, par.size()))
			return;

		// we may paste some paragraphs

		// is it an empty paragraph?
		pos_type lastpos = cur.lastpos();
		if (lastpos == 0 || (lastpos == 1 && par.isSeparator(0))) {
			// This is an empty paragraph and we delete it just
			// by moving the cursor one step
			// left and let the DeleteEmptyParagraphMechanism
			// handle the actual deletion of the paragraph.

			if (cur.par() != 0) {
				cursorLeft(cur);
				// the layout things can change the height of a row !
				redoParagraph(cur);
				return;
			}
		}

		if (cur.par() != 0)
			recordUndo(cur, Undo::DELETE, cur.par() - 1);

		ParagraphList::iterator tmppit = getPar(cur.par());
		// We used to do cursorLeftIntern() here, but it is
		// not a good idea since it triggers the auto-delete
		// mechanism. So we do a cursorLeftIntern()-lite,
		// without the dreaded mechanism. (JMarc)
		if (cur.par() != 0) {
			// steps into the above paragraph.
			setCursorIntern(cur, cur.par() - 1,
					getPar(cur.par() - 1)->size(),
					false);
		}

		// Pasting is not allowed, if the paragraphs have different
		// layout. I think it is a real bug of all other
		// word processors to allow it. It confuses the user.
		// Correction: Pasting is always allowed with standard-layout
		Buffer & buf = *bv()->buffer();
		BufferParams const & bufparams = buf.params();
		LyXTextClass const & tclass = bufparams.getLyXTextClass();
		ParagraphList::iterator const cpit = getPar(cur.par());

		if (cpit != tmppit
		    && (cpit->layout() == tmppit->layout()
		        || tmppit->layout() == tclass.defaultLayout())
		    && cpit->getAlign() == tmppit->getAlign()) {
			mergeParagraph(bufparams, buf.paragraphs(), cpit);

			if (cur.pos() != 0 && cpit->isSeparator(cur.pos() - 1))
				--cur.pos();

			// the counters may have changed
			updateCounters();
			setCursor(cur, cur.par(), cur.pos(), false);
		}
	} else {
		// this is the code for a normal backspace, not pasting
		// any paragraphs
		recordUndo(cur, Undo::DELETE);
		// We used to do cursorLeftIntern() here, but it is
		// not a good idea since it triggers the auto-delete
		// mechanism. So we do a cursorLeftIntern()-lite,
		// without the dreaded mechanism. (JMarc)
		setCursorIntern(cur, cur.par(), cur.pos() - 1,
				false, cur.boundary());
		cur.paragraph().erase(cur.pos());
	}

	if (cur.pos() == cur.lastpos())
		setCurrentFont(cur);

	redoParagraph(cur);
	setCursor(cur, cur.par(), cur.pos(), false, cur.boundary());
}


ParagraphList::iterator LyXText::cursorPar() const
{
	//lyxerr << "### cursorPar: cursor: " << bv()->cursor() << endl;
	//lyxerr << "xxx cursorPar: cursor: " << cursor() << endl;
	return getPar(cursor().par());
}


ParagraphList::iterator LyXText::getPar(CursorSlice const & cur) const
{
	return getPar(cur.par());
}


ParagraphList::iterator LyXText::getPar(int par) const
{
	//lyxerr << "getPar: " << par << " from " << paragraphs().size() << endl;
	BOOST_ASSERT(par >= 0);
	BOOST_ASSERT(par < int(paragraphs().size()));
	ParagraphList::iterator pit = paragraphs().begin();
	advance(pit, par);
	return pit;
}


// y is relative to this LyXText's top
RowList::iterator
LyXText::getRowNearY(int y, ParagraphList::iterator & pit) const
{
	BOOST_ASSERT(!paragraphs().empty());
	BOOST_ASSERT(!paragraphs().begin()->rows.empty());
#if 1
	ParagraphList::iterator const
		pend = boost::prior(paragraphs().end());
	pit = paragraphs().begin();
	while (int(pit->y + pit->height) < y && pit != pend)
		++pit;

	RowList::iterator rit = pit->rows.end();
	RowList::iterator const rbegin = pit->rows.begin();
	do {
		--rit;
	} while (rit != rbegin && int(pit->y + rit->y_offset()) > y);

	return rit;
#else
	pit = boost::prior(paragraphs().end());

	RowList::iterator rit = lastRow();
	RowList::iterator rbegin = firstRow();

	while (rit != rbegin && int(pit->y + rit->y_offset()) > y)
		previousRow(pit, rit);

	return rit;
#endif
}


int LyXText::getDepth() const
{
	return cursorPar()->getDepth();
}


RowList::iterator LyXText::firstRow() const
{
	return paragraphs().front().rows.begin();
}


RowList::iterator LyXText::lastRow() const
{
	return boost::prior(endRow());
}


RowList::iterator LyXText::endRow() const
{
	return paragraphs().back().rows.end();
}


void LyXText::nextRow(ParagraphList::iterator & pit,
	RowList::iterator & rit) const
{
	++rit;
	if (rit == pit->rows.end()) {
		++pit;
		if (pit == paragraphs().end())
			--pit;
		else
			rit = pit->rows.begin();
	}
}


void LyXText::previousRow(ParagraphList::iterator & pit,
	RowList::iterator & rit) const
{
	if (rit != pit->rows.begin())
		--rit;
	else {
		BOOST_ASSERT(pit != paragraphs().begin());
		--pit;
		rit = boost::prior(pit->rows.end());
	}
}


int LyXText::parOffset(ParagraphList::iterator pit) const
{
	return distance(paragraphs().begin(), pit);
}


void LyXText::redoParagraphInternal(ParagraphList::iterator pit)
{
	// remove rows of paragraph, keep track of height changes
	height -= pit->height;

	// clear old data
	pit->rows.clear();
	pit->height = 0;
	pit->width = 0;

	// redo insets
	InsetList::iterator ii = pit->insetlist.begin();
	InsetList::iterator iend = pit->insetlist.end();
	for (; ii != iend; ++ii) {
		Dimension dim;
		int const w = textWidth() - leftMargin(pit) - rightMargin(*pit);
		MetricsInfo mi(bv(), getFont(pit, ii->pos), w);
		ii->inset->metrics(mi, dim);
	}

	// rebreak the paragraph
	pit->setBeginOfBody();
	pos_type z = 0;
	do {
		Row row(z);
		rowBreakPoint(pit, row);
		setRowWidth(pit, row);
		setHeightOfRow(pit, row);
		row.y_offset(pit->height);
		pit->rows.push_back(row);
		pit->width = std::max(pit->width, row.width());
		pit->height += row.height();
		z = row.endpos();
	} while (z < pit->size());

	height += pit->height;
	//lyxerr << "redoParagraph: " << pit->rows.size() << " rows\n";
}


void LyXText::redoParagraphs(ParagraphList::iterator pit,
  ParagraphList::iterator end)
{
	for ( ; pit != end; ++pit)
		redoParagraphInternal(pit);
	updateParPositions();
}


void LyXText::redoParagraph(ParagraphList::iterator pit)
{
	redoParagraphInternal(pit);
	updateParPositions();
}


void LyXText::fullRebreak()
{
	redoParagraphs(paragraphs().begin(), paragraphs().end());
	bv()->cursor().resetAnchor();
}


void LyXText::metrics(MetricsInfo & mi, Dimension & dim)
{
	//BOOST_ASSERT(mi.base.textwidth);
	if (mi.base.textwidth)
		textwidth_ = mi.base.textwidth;
	//lyxerr << "LyXText::metrics: width: " << mi.base.textwidth
	//	<< " textWidth: " << textWidth() << "\nfont: " << mi.base.font << endl;

	// Rebuild row cache. This recomputes height as well.
	redoParagraphs(paragraphs().begin(), paragraphs().end());

	width = maxParagraphWidth(paragraphs());

	// final dimension
	dim.asc = firstRow()->ascent_of_text();
	dim.des = height - dim.asc;
	dim.wid = width;
}


// only used for inset right now. should also be used for main text
void LyXText::draw(PainterInfo & pi, int x, int y) const
{
	xo_ = x;
	yo_ = y;
	paintTextInset(*this, pi, x, y);
}


// only used for inset right now. should also be used for main text
void LyXText::drawSelection(PainterInfo &, int, int) const
{
	//lyxerr << "LyXText::drawSelection at " << x << " " << y << endl;
}


bool LyXText::isLastRow(ParagraphList::iterator pit, Row const & row) const
{
	return row.endpos() >= pit->size()
	       && boost::next(pit) == paragraphs().end();
}


bool LyXText::isFirstRow(ParagraphList::iterator pit, Row const & row) const
{
	return row.pos() == 0 && pit == paragraphs().begin();
}


void LyXText::getWord(CursorSlice & from, CursorSlice & to,
	word_location const loc)
{
	Paragraph & from_par = *getPar(from);
	switch (loc) {
	case lyx::WHOLE_WORD_STRICT:
		if (from.pos() == 0 || from.pos() == from_par.size()
		    || from_par.isSeparator(from.pos())
		    || from_par.isKomma(from.pos())
		    || from_par.isNewline(from.pos())
		    || from_par.isSeparator(from.pos() - 1)
		    || from_par.isKomma(from.pos() - 1)
		    || from_par.isNewline(from.pos() - 1)) {
			to = from;
			return;
		}
		// no break here, we go to the next

	case lyx::WHOLE_WORD:
		// Move cursor to the beginning, when not already there.
		if (from.pos() && !from_par.isSeparator(from.pos() - 1)
		    && !(from_par.isKomma(from.pos() - 1)
			 || from_par.isNewline(from.pos() - 1)))
			cursorLeftOneWord(bv()->cursor());
		break;
	case lyx::PREVIOUS_WORD:
		// always move the cursor to the beginning of previous word
		cursorLeftOneWord(bv()->cursor());
		break;
	case lyx::NEXT_WORD:
		lyxerr << "LyXText::getWord: NEXT_WORD not implemented yet"
		       << endl;
		break;
	case lyx::PARTIAL_WORD:
		break;
	}
	to = from;
	Paragraph & to_par = *getPar(to);
	while (to.pos() < to_par.size()
	       && !to_par.isSeparator(to.pos())
	       && !to_par.isKomma(to.pos())
	       && !to_par.isNewline(to.pos())
	       && !to_par.isHfill(to.pos())
	       && !to_par.isInset(to.pos()))
	{
		++to.pos();
	}
}


void LyXText::write(Buffer const & buf, std::ostream & os) const
{
	ParagraphList::const_iterator pit = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();
	Paragraph::depth_type dth = 0;
	for (; pit != end; ++pit)
		pit->write(buf, os, buf.params(), dth);
}


bool LyXText::read(Buffer const & buf, LyXLex & lex)
{
	static Change current_change;

	bool the_end_read = false;
	ParagraphList::iterator pit = paragraphs().begin();
	Paragraph::depth_type depth = 0;

	while (lex.isOK()) {
		lex.nextToken();
		string token = lex.getString();

		if (token.empty())
			continue;

		if (in_inset_) {

			if (token == "\\end_inset") {
				the_end_read = true;
				break;
			}

			if (token == "\\end_document") {
				lex.printError("\\end_document read in inset! Error in document!");
				return false;
			}

		} else {

			if (token == "\\end_document") {
				the_end_read = true;
				continue;
			}

		}

		// FIXME: ugly.
		int unknown = 0;

		if (token == "\\begin_layout") {
			lex.pushToken(token);

			Paragraph par;
			par.params().depth(depth);
			if (buf.params().tracking_changes)
				par.trackChanges();
			LyXFont f(LyXFont::ALL_INHERIT, buf.params().language);
			par.setFont(0, f);

			// insert after
			if (pit != paragraphs().end())
				++pit;

			pit = paragraphs().insert(pit, par);

			// FIXME: goddamn InsetTabular makes us pass a Buffer
			// not BufferParams
			::readParagraph(buf, *pit, lex);

		} else if (token == "\\begin_deeper") {
			++depth;
		} else if (token == "\\end_deeper") {
			if (!depth) {
				lex.printError("\\end_deeper: " "depth is already null");
			} else {
				--depth;
			}
		} else {
			++unknown;
		}

	}
	return the_end_read;
}


int LyXText::ascent() const
{
	return firstRow()->ascent_of_text();
}


int LyXText::descent() const
{
	return height - firstRow()->ascent_of_text();
}


int LyXText::cursorX(CursorSlice const & cur) const
{
	ParagraphList::iterator pit = getPar(cur);
	if (pit->rows.empty())
		return xo_;
	Row const & row = *pit->getRow(cur.pos());
	
	
	pos_type pos = cur.pos();
	pos_type cursor_vpos = 0;

	RowMetrics const m = prepareToPrint(pit, row);
	double x = m.x;

	pos_type const row_pos  = row.pos();
	pos_type const end      = row.endpos();

	if (end <= row_pos)
		cursor_vpos = row_pos;
	else if (pos >= end)
		cursor_vpos = isRTL(*pit) ? row_pos : end;
	else if (pos > row_pos && pos >= end)
		// Place cursor after char at (logical) position pos - 1
		cursor_vpos = (bidi.level(pos - 1) % 2 == 0)
			? bidi.log2vis(pos - 1) + 1 : bidi.log2vis(pos - 1);
	else
		// Place cursor before char at (logical) position pos
		cursor_vpos = (bidi.level(pos) % 2 == 0)
			? bidi.log2vis(pos) : bidi.log2vis(pos) + 1;

	pos_type body_pos = pit->beginOfBody();
	if (body_pos > 0 &&
	    (body_pos > end || !pit->isLineSeparator(body_pos - 1)))
		body_pos = 0;

	for (pos_type vpos = row_pos; vpos < cursor_vpos; ++vpos) {
		pos_type pos = bidi.vis2log(vpos);
		if (body_pos > 0 && pos == body_pos - 1) {
			x += m.label_hfill
				+ font_metrics::width(pit->layout()->labelsep,
						      getLabelFont(pit));
			if (pit->isLineSeparator(body_pos - 1))
				x -= singleWidth(pit, body_pos - 1);
		}

		if (hfillExpansion(*pit, row, pos)) {
			x += singleWidth(pit, pos);
			if (pos >= body_pos)
				x += m.hfill;
			else
				x += m.label_hfill;
		} else if (pit->isSeparator(pos)) {
			x += singleWidth(pit, pos);
			if (pos >= body_pos)
				x += m.separator;
		} else
			x += singleWidth(pit, pos);
	}
	return xo_ + int(x);
}


int LyXText::cursorY(CursorSlice const & cur) const
{
	Paragraph & par = *getPar(cur);
	Row & row = *par.getRow(cur.pos());
	return yo_ + par.y + row.y_offset() + row.baseline();
}


CursorSlice & LyXText::cursor()
{
	//lyxerr << "# accessing slice " << findText(this) << endl;
	if (this != bv()->cursor().text()) {
		lyxerr << "cursor: " << bv()->cursor()
			<< "\ntext: " << bv()->cursor().text() 
			<< "\nthis: " << this << endl;
		BOOST_ASSERT(false);
	}
	return bv()->cursor().current();
}


CursorSlice const & LyXText::cursor() const
{
	if (this != bv()->cursor().text()) {
		lyxerr << "cursor: " << bv()->cursor()
			<< "\ntext: " << bv()->cursor().text() 
			<< "\nthis: " << this << endl;
		BOOST_ASSERT(false);
	}
	return bv()->cursor().current();
}


void LyXText::replaceSelection(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.selection()) {
		cutSelection(cur, true, false);
		cur.update();
	}
}


// Returns the current font and depth as a message.
string LyXText::currentState(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	Buffer * buffer = bv()->buffer();
	Paragraph const & par = cur.paragraph();
	std::ostringstream os;

	bool const show_change = buffer->params().tracking_changes
		&& cur.pos() != cur.lastpos()
		&& par.lookupChange(cur.pos()) != Change::UNCHANGED;

	if (show_change) {
		Change change = par.lookupChangeFull(cur.pos());
		Author const & a = buffer->params().authors().get(change.author);
		os << _("Change: ") << a.name();
		if (!a.email().empty())
			os << " (" << a.email() << ")";
		if (change.changetime)
			os << _(" at ") << ctime(&change.changetime);
		os << " : ";
	}

	// I think we should only show changes from the default
	// font. (Asger)
	LyXFont font = real_current_font;
	font.reduce(buffer->params().getLyXTextClass().defaultfont());

	// avoid _(...) re-entrance problem
	string const s = font.stateText(&buffer->params());
	os << bformat(_("Font: %1$s"), s);

	// os << bformat(_("Font: %1$s"), font.stateText(&buffer->params));

	// The paragraph depth
	int depth = getDepth();
	if (depth > 0)
		os << bformat(_(", Depth: %1$s"), tostr(depth));

	// The paragraph spacing, but only if different from
	// buffer spacing.
	Spacing const & spacing = par.params().spacing();
	if (!spacing.isDefault()) {
		os << _(", Spacing: ");
		switch (spacing.getSpace()) {
		case Spacing::Single:
			os << _("Single");
			break;
		case Spacing::Onehalf:
			os << _("OneHalf");
			break;
		case Spacing::Double:
			os << _("Double");
			break;
		case Spacing::Other:
			os << _("Other (") << spacing.getValue() << ')';
			break;
		case Spacing::Default:
			// should never happen, do nothing
			break;
		}
	}
#ifdef DEVEL_VERSION
	os << _(", Paragraph: ") << par.id();
	os << _(", Position: ") << cur.pos();
	Row & row = cur.textRow();
	os << bformat(_(", Row b:%1$d e:%2$d"), row.pos(), row.endpos());
	os << _(", Inset: ");
	InsetOld * inset = par.inInset();
	if (inset)
		os << inset << " owner: " << inset->owner();
	else
		os << -1;
#endif
	return os.str();
}


string LyXText::getPossibleLabel(LCursor & cur) const
{
	ParagraphList & plist = paragraphs();
	ParagraphList::iterator pit = getPar(cur.par());

	LyXLayout_ptr layout = pit->layout();

	if (layout->latextype == LATEX_PARAGRAPH && pit != plist.begin()) {
		ParagraphList::iterator pit2 = boost::prior(pit);

		LyXLayout_ptr const & layout2 = pit2->layout();

		if (layout2->latextype != LATEX_PARAGRAPH) {
			pit = pit2;
			layout = layout2;
		}
	}

	string text = layout->latexname().substr(0, 3);
	if (layout->latexname() == "theorem")
		text = "thm"; // Create a correct prefix for prettyref

	text += ':';
	if (layout->latextype == LATEX_PARAGRAPH || lyxrc.label_init_length < 0)
		text.erase();

	string par_text = pit->asString(*cur.bv().buffer(), false);
	for (int i = 0; i < lyxrc.label_init_length; ++i) {
		if (par_text.empty())
			break;
		string head;
		par_text = split(par_text, head, ' ');
		// Is it legal to use spaces in labels ?
		if (i > 0)
			text += '-';
		text += head;
	}

	return text;
}


int LyXText::dist(int x, int y) const
{
	int xx = 0;
	int yy = 0;

	if (x < xo_)
		xx = xo_ - x;
	else if (x > xo_ + width)
		xx = x - xo_ - width;

	if (y < yo_ - ascent())
		yy = yo_ - ascent() - y;
	else if (y > yo_ + descent())
		yy = y - yo_ - descent();

	return xx + yy;
}
