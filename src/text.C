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

#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "debug.h"
#include "dispatchresult.h"
#include "encoding.h"
#include "funcrequest.h"
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

using lyx::pos_type;
using lyx::word_location;

using lyx::support::contains;
using lyx::support::lowercase;
using lyx::support::uppercase;

using std::max;
using std::endl;
using std::string;


/// some space for drawing the 'nested' markers (in pixel)
extern int const NEST_MARGIN = 20;
/// margin for changebar
extern int const CHANGEBAR_MARGIN = 10;
/// right margin
extern int const RIGHT_MARGIN = 10;


namespace {

unsigned int maxParagraphWidth(ParagraphList const & plist)
{
	unsigned int width = 0;
	ParagraphList::const_iterator pit = plist.begin();
	ParagraphList::const_iterator end = plist.end();
		for (; pit != end; ++pit)
			width = std::max(width, pit->width);
	return width;
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


	// We re-use the font resolution for the entire font span when possible
	LyXFont font = getFont(pit, pos);
	lyx::pos_type endPosOfFontSpan = pit->getEndPosOfFontSpan(pos);

	pos_type point = end;
	pos_type i = pos;
	for ( ; i < end; ++i) {
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

		if (i > endPosOfFontSpan) {
			font = getFont(pit, i);
			endPosOfFontSpan = pit->getEndPosOfFontSpan(i);
		}

		{
			int thiswidth = singleWidth(pit, i, c, font);

			// add the auto-hfill from label end to the body
			if (body_pos && i == body_pos) {
				int add = font_metrics::width(layout->labelsep, getLabelFont(pit));
				if (pit->isLineSeparator(i - 1))
					add -= singleWidth(pit, i - 1);

				add = std::max(add, labelEnd(pit) - x);
				thiswidth += add;
			}

			x += thiswidth;
			//lyxerr << "i: " << i << " x: "
			//<< x << " width: " << width << endl;
			chunkwidth += thiswidth;
		}

		// break before a character that will fall off
		// the right of the row
		if (x >= width) {
			// if no break before, break here
			if (point == end || chunkwidth >= width - left) {
				if (i > pos) {
					point = i;
					break;
				}
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


// returns the minimum space a row needs on the screen in pixel
void LyXText::fill(ParagraphList::iterator pit, Row & row, int workwidth) const
{
	// get the pure distance
	pos_type const end = row.endpos();

	string labelsep = pit->layout()->labelsep;
	int w = leftMargin(pit, row.pos());

	pos_type const body_pos = pit->beginOfBody();
	pos_type i = row.pos();

	if (i < end) {
		// We re-use the font resolution for the entire span when possible
		LyXFont font = getFont(pit, i);
		lyx::pos_type endPosOfFontSpan = pit->getEndPosOfFontSpan(i);
		for ( ; i < end; ++i) {
			if (body_pos > 0 && i == body_pos) {
				w += font_metrics::width(labelsep, getLabelFont(pit));
				if (pit->isLineSeparator(i - 1))
					w -= singleWidth(pit, i - 1);
				w = max(w, labelEnd(pit));
			}
			char const c = pit->getChar(i);
			if (IsPrintable(c) && i > endPosOfFontSpan) {
				// We need to get the next font
				font = getFont(pit, i);
				endPosOfFontSpan = pit->getEndPosOfFontSpan(i);
			}
			w += singleWidth(pit, i, c, font);
		}
	}

	if (body_pos > 0 && body_pos >= end) {
		w += font_metrics::width(labelsep, getLabelFont(pit));
		if (end > 0 && pit->isLineSeparator(end - 1))
			w -= singleWidth(pit, end - 1);
		w = max(w, labelEnd(pit));
	}

	int const fill = workwidth - w - rightMargin(*pit);
	row.fill(fill);
	row.width(workwidth - fill);
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

	// as max get the first character of this row then it can increase but not
	// decrease the height. Just some point to start with so we don't have to
	// do the assignment below too often.
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


void LyXText::breakParagraph(ParagraphList & paragraphs, char keep_layout)
{
	// allow only if at start or end, or all previous is new text
	ParagraphList::iterator cpit = cursorPar();
	if (cursor.pos() && cursor.pos() != cpit->size()
	    && cpit->isChangeEdited(0, cursor.pos()))
		return;

	LyXTextClass const & tclass =
		bv()->buffer()->params().getLyXTextClass();
	LyXLayout_ptr const & layout = cpit->layout();

	// this is only allowed, if the current paragraph is not empty or caption
	// and if it has not the keepempty flag active
	if (cpit->empty() && !cpit->allowEmpty()
	   && layout->labeltype != LABEL_SENSITIVE)
		return;

	recUndo(cursor.par());

	// Always break behind a space
	//
	// It is better to erase the space (Dekel)
	if (cursor.pos() < cpit->size() && cpit->isLineSeparator(cursor.pos()))
	   cpit->erase(cursor.pos());

	// break the paragraph
	if (keep_layout)
		keep_layout = 2;
	else
		keep_layout = layout->isEnvironment();

	// we need to set this before we insert the paragraph. IMO the
	// breakParagraph call should return a bool if it inserts the
	// paragraph before or behind and we should react on that one
	// but we can fix this in 1.3.0 (Jug 20020509)
	bool const isempty = cpit->allowEmpty() && cpit->empty();
	::breakParagraph(bv()->buffer()->params(), paragraphs, cpit,
			 cursor.pos(), keep_layout);

#warning Trouble Point! (Lgb)
	// When ::breakParagraph is called from within an inset we must
	// ensure that the correct ParagraphList is used. Today that is not
	// the case and the Buffer::paragraphs is used. Not good. (Lgb)
	cpit = cursorPar();
	ParagraphList::iterator next_par = boost::next(cpit);

	// well this is the caption hack since one caption is really enough
	if (layout->labeltype == LABEL_SENSITIVE) {
		if (!cursor.pos())
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
	RowList::iterator crit = cpit->getRow(cursor.pos());
	if (cursor.pos() && crit->pos() == cursor.pos()
	    && !cpit->isNewline(cursor.pos() - 1))
	{
		cursorLeft(bv());
	}

	while (!next_par->empty() && next_par->isNewline(0))
		next_par->erase(0);

	updateCounters();
	redoParagraph(cpit);
	redoParagraph(next_par);

	// This check is necessary. Otherwise the new empty paragraph will
	// be deleted automatically. And it is more friendly for the user!
	if (cursor.pos() || isempty)
		setCursor(next_par, 0);
	else
		setCursor(cpit, 0);
}


// convenience function
void LyXText::redoParagraph()
{
	clearSelection();
	redoParagraph(cursorPar());
	setCursorIntern(cursor.par(), cursor.pos());
}


// insert a character, moves all the following breaks in the
// same Paragraph one to the right and make a rebreak
void LyXText::insertChar(char c)
{
	recordUndo(Undo::INSERT, this, cursor.par(), cursor.par());

	// When the free-spacing option is set for the current layout,
	// disable the double-space checking

	bool const freeSpacing = cursorPar()->layout()->free_spacing ||
		cursorPar()->isFreeSpacing();

	if (lyxrc.auto_number) {
		static string const number_operators = "+-/*";
		static string const number_unary_operators = "+-";
		static string const number_seperators = ".,:";

		if (current_font.number() == LyXFont::ON) {
			if (!IsDigit(c) && !contains(number_operators, c) &&
			    !(contains(number_seperators, c) &&
			      cursor.pos() >= 1 &&
			      cursor.pos() < cursorPar()->size() &&
			      getFont(cursorPar(), cursor.pos()).number() == LyXFont::ON &&
			      getFont(cursorPar(), cursor.pos() - 1).number() == LyXFont::ON)
			   )
				number(); // Set current_font.number to OFF
		} else if (IsDigit(c) &&
			   real_current_font.isVisibleRightToLeft()) {
			number(); // Set current_font.number to ON

			if (cursor.pos() > 0) {
				char const c = cursorPar()->getChar(cursor.pos() - 1);
				if (contains(number_unary_operators, c) &&
				    (cursor.pos() == 1 ||
				     cursorPar()->isSeparator(cursor.pos() - 2) ||
				     cursorPar()->isNewline(cursor.pos() - 2))
				  ) {
					setCharFont(
						    cursorPar(),
						    cursor.pos() - 1,
						    current_font);
				} else if (contains(number_seperators, c) &&
					   cursor.pos() >= 2 &&
					   getFont(
						   cursorPar(),
						   cursor.pos() - 2).number() == LyXFont::ON) {
					setCharFont(
						    cursorPar(),
						    cursor.pos() - 1,
						    current_font);
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

	if (!freeSpacing && IsLineSeparatorChar(c)) {
		if ((cursor.pos() > 0
		     && cursorPar()->isLineSeparator(cursor.pos() - 1))
		    || (cursor.pos() > 0
			&& cursorPar()->isNewline(cursor.pos() - 1))
		    || (cursor.pos() == 0)) {
			static bool sent_space_message = false;
			if (!sent_space_message) {
				if (cursor.pos() == 0)
					bv()->owner()->message(_("You cannot insert a space at the beginning of a paragraph. Please read the Tutorial."));
				else
					bv()->owner()->message(_("You cannot type two spaces this way. Please read the Tutorial."));
				sent_space_message = true;
			}
			charInserted();
			return;
		}
	}

	// Here case LyXText::InsertInset already inserted the character
	if (c != Paragraph::META_INSET)
		cursorPar()->insertChar(cursor.pos(), c);

	setCharFont(cursorPar(), cursor.pos(), rawtmpfont);

	current_font = rawtmpfont;
	real_current_font = realtmpfont;
	redoParagraph(cursorPar());
	setCursor(cursor.par(), cursor.pos() + 1, false, cursor.boundary());

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


void LyXText::prepareToPrint(ParagraphList::iterator pit, Row & row) const
{
	double w = row.fill();
	double fill_hfill = 0;
	double fill_label_hfill = 0;
	double fill_separator = 0;
	double x = 0;

	bool const is_rtl =
		pit->isRightToLeftPar(bv()->buffer()->params());
	if (is_rtl)
		x = rightMargin(*pit);
	else
		x = leftMargin(pit, row.pos());

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
			fill_label_hfill = labelFill(pit, row) / double(nlh);
	}

	// are there any hfills in the row?
	int const nh = numberOfHfills(*pit, row);

	if (nh) {
		if (w > 0)
			fill_hfill = w / nh;
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
					InsetOld * in = pit->getInset(row.endpos());
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
						fill_separator = w / ns;
				} else if (is_rtl) {
					x += w;
				}
				break;
			}
    case LYX_ALIGN_RIGHT:
			x += w;
			break;
    case LYX_ALIGN_CENTER:
			x += w / 2;
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
			x += font_metrics::width(layout->labelsep, getLabelFont(pit));
			if (body_pos <= end)
				x += fill_label_hfill;
		}
	}

	row.fill_hfill(fill_hfill);
	row.fill_label_hfill(fill_label_hfill);
	row.fill_separator(fill_separator);
	row.x(x);
}


// the cursor set functions have a special mechanism. When they
// realize, that you left an empty paragraph, they will delete it.

void LyXText::cursorRightOneWord()
{
	cursorRightOneWord(cursor);
	setCursor(cursorPar(), cursor.pos());
}


// Skip initial whitespace at end of word and move cursor to *start*
// of prior word, not to end of next prior word.
void LyXText::cursorLeftOneWord()
{
	LyXCursor tmpcursor = cursor;
	cursorLeftOneWord(tmpcursor);
	setCursor(getPar(tmpcursor), tmpcursor.pos());
}


void LyXText::selectWord(word_location loc)
{
	LyXCursor from = cursor;
	LyXCursor to = cursor;
	getWord(from, to, loc);
	if (cursor != from)
		setCursor(from.par(), from.pos());
	if (to == from)
		return;
	selection.cursor = cursor;
	setCursor(to.par(), to.pos());
	setSelection();
}


// Select the word currently under the cursor when no
// selection is currently set
bool LyXText::selectWordWhenUnderCursor(word_location loc)
{
	if (!selection.set()) {
		selectWord(loc);
		return selection.set();
	}
	return false;
}


void LyXText::acceptChange()
{
	if (!selection.set() && cursorPar()->size())
		return;

	if (selStart().par() == selEnd().par()) {
		LyXCursor const & startc = selStart();
		LyXCursor const & endc = selEnd();
		recordUndo(Undo::INSERT, this, startc.par());
		getPar(startc)->acceptChange(startc.pos(), endc.pos());
		finishUndo();
		clearSelection();
		redoParagraph(getPar(startc));
		setCursorIntern(startc.par(), 0);
	}
#warning handle multi par selection
}


void LyXText::rejectChange()
{
	if (!selection.set() && cursorPar()->size())
		return;

	if (selStart().par() == selEnd().par()) {
		LyXCursor const & startc = selStart();
		LyXCursor const & endc = selEnd();
		recordUndo(Undo::INSERT, this, startc.par());
		getPar(startc)->rejectChange(startc.pos(), endc.pos());
		finishUndo();
		clearSelection();
		redoParagraph(getPar(startc));
		setCursorIntern(startc.par(), 0);
	}
#warning handle multi par selection
}


// Delete from cursor up to the end of the current or next word.
void LyXText::deleteWordForward()
{
	if (cursorPar()->empty())
		cursorRight(bv());
	else {
		LyXCursor tmpcursor = cursor;
		selection.set(true); // to avoid deletion
		cursorRightOneWord();
		setCursor(tmpcursor, tmpcursor.par(), tmpcursor.pos());
		selection.cursor = cursor;
		cursor = tmpcursor;
		setSelection();
		cutSelection(true, false);
	}
}


// Delete from cursor to start of current or prior word.
void LyXText::deleteWordBackward()
{
	if (cursorPar()->empty())
		cursorLeft(bv());
	else {
		LyXCursor tmpcursor = cursor;
		selection.set(true); // to avoid deletion
		cursorLeftOneWord();
		setCursor(tmpcursor, tmpcursor.par(), tmpcursor.pos());
		selection.cursor = cursor;
		cursor = tmpcursor;
		setSelection();
		cutSelection(true, false);
	}
}


// Kill to end of line.
void LyXText::deleteLineForward()
{
	if (cursorPar()->empty()) {
		// Paragraph is empty, so we just go to the right
		cursorRight(bv());
	} else {
		LyXCursor tmpcursor = cursor;
		selection.set(true); // to avoid deletion
		cursorEnd();
		setCursor(tmpcursor, tmpcursor.par(), tmpcursor.pos());
		selection.cursor = cursor;
		cursor = tmpcursor;
		setSelection();
		// What is this test for ??? (JMarc)
		if (!selection.set())
			deleteWordForward();
		else
			cutSelection(true, false);
	}
}


void LyXText::changeCase(LyXText::TextCase action)
{
	LyXCursor from;
	LyXCursor to;

	if (selection.set()) {
		from = selStart();
		to = selEnd();
	} else {
		from = cursor;
		getWord(from, to, lyx::PARTIAL_WORD);
		setCursor(to.par(), to.pos() + 1);
	}

	recordUndo(Undo::ATOMIC, this, from.par(), to.par());

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


void LyXText::Delete()
{
	// this is a very easy implementation
	LyXCursor old_cursor = cursor;

	// just move to the right
	cursorRight(true);

	// if you had success make a backspace
	if (old_cursor.par() != cursor.par() || old_cursor.pos() != cursor.pos()) {
		recordUndo(Undo::DELETE, this, old_cursor.par());
		backspace();
	}
}


void LyXText::backspace()
{
	// Get the font that is used to calculate the baselineskip
	ParagraphList::iterator pit = cursorPar();
	pos_type lastpos = pit->size();

	if (cursor.pos() == 0) {
		// The cursor is at the beginning of a paragraph,
		// so the the backspace will collapse two paragraphs into one.

		// but it's not allowed unless it's new
		if (pit->isChangeEdited(0, pit->size()))
			return;

		// we may paste some paragraphs

		// is it an empty paragraph?

		if (lastpos == 0 || (lastpos == 1 && pit->isSeparator(0))) {
			// This is an empty paragraph and we delete it just
			// by moving the cursor one step
			// left and let the DeleteEmptyParagraphMechanism
			// handle the actual deletion of the paragraph.

			if (cursor.par()) {
				cursorLeft(bv());
				// the layout things can change the height of a row !
				redoParagraph();
				return;
			}
		}

		if (cursor.par() != 0)
			recordUndo(Undo::DELETE, this, cursor.par() - 1, cursor.par());

		ParagraphList::iterator tmppit = cursorPar();
		// We used to do cursorLeftIntern() here, but it is
		// not a good idea since it triggers the auto-delete
		// mechanism. So we do a cursorLeftIntern()-lite,
		// without the dreaded mechanism. (JMarc)
		if (cursor.par() != 0) {
			// steps into the above paragraph.
			setCursorIntern(cursor.par() - 1,
					getPar(cursor.par() - 1)->size(),
					false);
		}

		// Pasting is not allowed, if the paragraphs have different
		// layout. I think it is a real bug of all other
		// word processors to allow it. It confuses the user.
		// Correction: Pasting is always allowed with standard-layout
		Buffer & buf = *bv()->buffer();
		BufferParams const & bufparams = buf.params();
		LyXTextClass const & tclass = bufparams.getLyXTextClass();
		ParagraphList::iterator const cpit = cursorPar();

		if (cpit != tmppit
		    && (cpit->layout() == tmppit->layout()
		        || tmppit->layout() == tclass.defaultLayout())
		    && cpit->getAlign() == tmppit->getAlign()) {
			mergeParagraph(bufparams, buf.paragraphs(), cpit);

			if (cursor.pos() && cpit->isSeparator(cursor.pos() - 1))
				cursor.pos(cursor.pos() - 1);

			// the counters may have changed
			updateCounters();
			setCursor(cursor.par(), cursor.pos(), false);
		}
	} else {
		// this is the code for a normal backspace, not pasting
		// any paragraphs
		recordUndo(Undo::DELETE, this, cursor.par());
		// We used to do cursorLeftIntern() here, but it is
		// not a good idea since it triggers the auto-delete
		// mechanism. So we do a cursorLeftIntern()-lite,
		// without the dreaded mechanism. (JMarc)
		setCursorIntern(cursor.par(), cursor.pos() - 1,
				false, cursor.boundary());
		cursorPar()->erase(cursor.pos());
	}

	lastpos = cursorPar()->size();
	if (cursor.pos() == lastpos)
		setCurrentFont();

	redoParagraph();
	setCursor(cursor.par(), cursor.pos(), false, cursor.boundary());
}


ParagraphList::iterator LyXText::cursorPar() const
{
	return getPar(cursor.par());
}


RowList::iterator LyXText::cursorRow() const
{
	return cursorPar()->getRow(cursor.pos());
}


ParagraphList::iterator LyXText::getPar(LyXCursor const & cur) const
{
	return getPar(cur.par());
}


ParagraphList::iterator LyXText::getPar(int par) const
{
	BOOST_ASSERT(par >= 0);
	BOOST_ASSERT(par < int(paragraphs().size()));
	ParagraphList::iterator pit = paragraphs().begin();
	std::advance(pit, par);
	return pit;
}


RowList::iterator
LyXText::getRowNearY(int y, ParagraphList::iterator & pit) const
{
	//lyxerr << "getRowNearY: y " << y << endl;
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


ParagraphList::iterator LyXText::firstPar() const
{
	return paragraphs().begin();
}


RowList::iterator LyXText::lastRow() const
{
	return boost::prior(endRow());
}


ParagraphList::iterator LyXText::lastPar() const
{
	return boost::prior(endPar());
}


RowList::iterator LyXText::endRow() const
{
	return paragraphs().back().rows.end();
}


ParagraphList::iterator LyXText::endPar() const
{
	return paragraphs().end();
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


string LyXText::selectionAsString(Buffer const & buffer, bool label) const
{
	if (!selection.set())
		return string();

	// should be const ...
	ParagraphList::iterator startpit = getPar(selStart());
	ParagraphList::iterator endpit = getPar(selEnd());
	size_t const startpos = selStart().pos();
	size_t const endpos = selEnd().pos();

	if (startpit == endpit)
		return startpit->asString(buffer, startpos, endpos, label);

	// First paragraph in selection
	string result =
		startpit->asString(buffer, startpos, startpit->size(), label) + "\n\n";

	// The paragraphs in between (if any)
	ParagraphList::iterator pit = startpit;
	for (++pit; pit != endpit; ++pit)
		result += pit->asString(buffer, 0, pit->size(), label) + "\n\n";

	// Last paragraph in selection
	result += endpit->asString(buffer, 0, endpos, label);

	return result;
}


int LyXText::parOffset(ParagraphList::iterator pit) const
{
	return std::distance(paragraphs().begin(), pit);
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
		z = row.endpos();
		fill(pit, row, textwidth_);
		prepareToPrint(pit, row);
		setHeightOfRow(pit, row);
		row.y_offset(pit->height);
		pit->rows.push_back(row);
		pit->width = std::max(pit->width, row.width());
		pit->height += row.height();
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
	redoCursor();
	selection.cursor = cursor;
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
void LyXText::draw(PainterInfo &, int x, int y) const
{
	xo_ = x;
	yo_ = y;
	paintTextInset(*bv(), *this, x, y);
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


void LyXText::cursorLeftOneWord(LyXCursor & cursor)
{
	// treat HFills, floats and Insets as words

	ParagraphList::iterator pit = cursorPar();
	size_t pos = cursor.pos();

	while (pos &&
	       (pit->isSeparator(pos - 1) ||
	        pit->isKomma(pos - 1) ||
	        pit->isNewline(pos - 1)) &&
	       !(pit->isHfill(pos - 1) ||
		 pit->isInset(pos - 1)))
		--pos;

	if (pos &&
	    (pit->isInset(pos - 1) ||
	     pit->isHfill(pos - 1))) {
		--pos;
	} else if (!pos) {
		if (pit != paragraphs().begin()) {
			--pit;
			pos = pit->size();
		}
	} else {		// Here, cur != 0
		while (pos > 0 && pit->isWord(pos - 1))
			--pos;
	}

	cursor.par(parOffset(pit));
	cursor.pos(pos);
}


void LyXText::cursorRightOneWord(LyXCursor & cursor)
{
	// treat floats, HFills and Insets as words
	ParagraphList::iterator pit = cursorPar();
	pos_type pos = cursor.pos();

	if (pos == pit->size() &&
		boost::next(pit) != paragraphs().end()) {
		++pit;
		pos = 0;
	} else {
		// Skip through initial nonword stuff.
		while (pos < pit->size() && !pit->isWord(pos)) {
			++pos;
		}
		// Advance through word.
		while (pos < pit->size() && pit->isWord(pos)) {
			++pos;
		}
	}

	cursor.par(parOffset(pit));
	cursor.pos(pos);
}


void LyXText::getWord(LyXCursor & from, LyXCursor & to, word_location const loc)
{
	ParagraphList::iterator from_par = getPar(from);
	ParagraphList::iterator to_par = getPar(to);
	switch (loc) {
	case lyx::WHOLE_WORD_STRICT:
		if (from.pos() == 0 || from.pos() == from_par->size()
		    || from_par->isSeparator(from.pos())
		    || from_par->isKomma(from.pos())
		    || from_par->isNewline(from.pos())
		    || from_par->isSeparator(from.pos() - 1)
		    || from_par->isKomma(from.pos() - 1)
		    || from_par->isNewline(from.pos() - 1)) {
			to = from;
			return;
		}
		// no break here, we go to the next

	case lyx::WHOLE_WORD:
		// Move cursor to the beginning, when not already there.
		if (from.pos() && !from_par->isSeparator(from.pos() - 1)
		    && !(from_par->isKomma(from.pos() - 1)
			 || from_par->isNewline(from.pos() - 1)))
			cursorLeftOneWord(from);
		break;
	case lyx::PREVIOUS_WORD:
		// always move the cursor to the beginning of previous word
		cursorLeftOneWord(from);
		break;
	case lyx::NEXT_WORD:
		lyxerr << "LyXText::getWord: NEXT_WORD not implemented yet"
		       << endl;
		break;
	case lyx::PARTIAL_WORD:
		break;
	}
	to = from;
	while (to.pos() < to_par->size()
	       && !to_par->isSeparator(to.pos())
	       && !to_par->isKomma(to.pos())
	       && !to_par->isNewline(to.pos())
	       && !to_par->isHfill(to.pos())
	       && !to_par->isInset(to.pos()))
	{
		to.pos(to.pos() + 1);
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

