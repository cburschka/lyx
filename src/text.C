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
#include "encoding.h"
#include "funcrequest.h"
#include "gettext.h"
#include "language.h"
#include "LColor.h"
#include "lyxlength.h"
#include "lyxrc.h"
#include "lyxrow.h"
#include "lyxrow_funcs.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "rowpainter.h"
#include "text_funcs.h"
#include "undo_funcs.h"
#include "vspace.h"
#include "WordLangTuple.h"

#include "frontends/font_metrics.h"
#include "frontends/LyXView.h"

#include "insets/insettext.h"

#include "support/lstrings.h"
#include "support/textutils.h"

using bv_funcs::number;

using lyx::pos_type;
using lyx::word_location;

using lyx::support::contains;
using lyx::support::lowercase;
using lyx::support::uppercase;

using std::max;
using std::endl;
using std::string;


/// top, right, bottom pixel margin
extern int const PAPER_MARGIN = 20;
/// margin for changebar
extern int const CHANGEBAR_MARGIN = 10;
/// left margin
extern int const LEFT_MARGIN = PAPER_MARGIN + CHANGEBAR_MARGIN;



int bibitemMaxWidth(BufferView *, LyXFont const &);


BufferView * LyXText::bv()
{
	BOOST_ASSERT(bv_owner != 0);
	return bv_owner;
}


BufferView * LyXText::bv() const
{
	BOOST_ASSERT(bv_owner != 0);
	return bv_owner;
}


void LyXText::updateRowPositions()
{
	ParagraphList::iterator pit = ownerParagraphs().begin();
	ParagraphList::iterator end = ownerParagraphs().end();
	for (height = 0; pit != end; ++pit) {
		RowList::iterator rit = pit->rows.begin();
		RowList::iterator rend = pit->rows.end();
		for ( ; rit != rend ; ++rit) {
			rit->y(height);
			height += rit->height();
		}
	}
}


int LyXText::workWidth() const
{
	return inset_owner ? inset_owner->textWidth() : bv()->workWidth();
}


int LyXText::getRealCursorX() const
{
	int x = cursor.x();
	if (the_locking_inset && (the_locking_inset->getLyXText(bv())!= this))
		x = the_locking_inset->getLyXText(bv())->getRealCursorX();
	return x;
}


#warning FIXME  This function seems to belong outside of LyxText.
unsigned char LyXText::transformChar(unsigned char c, Paragraph const & par,
				     pos_type pos) const
{
	if (!Encodings::is_arabic(c))
		if (lyxrc.font_norm_type == LyXRC::ISO_8859_6_8 && IsDigit(c))
			return c + (0xb0 - '0');
		else
			return c;

	unsigned char const prev_char = pos > 0 ? par.getChar(pos - 1) : ' ';
	unsigned char next_char = ' ';

	pos_type const par_size = par.size();

	for (pos_type i = pos + 1; i < par_size; ++i) {
		unsigned char const par_char = par.getChar(i);
		if (!Encodings::IsComposeChar_arabic(par_char)) {
			next_char = par_char;
			break;
		}
	}

	if (Encodings::is_arabic(next_char)) {
		if (Encodings::is_arabic(prev_char) &&
			!Encodings::is_arabic_special(prev_char))
			return Encodings::TransformChar(c, Encodings::FORM_MEDIAL);
		else
			return Encodings::TransformChar(c, Encodings::FORM_INITIAL);
	} else {
		if (Encodings::is_arabic(prev_char) &&
			!Encodings::is_arabic_special(prev_char))
			return Encodings::TransformChar(c, Encodings::FORM_FINAL);
		else
			return Encodings::TransformChar(c, Encodings::FORM_ISOLATED);
	}
}

// This is the comments that some of the warnings below refers to.
// There are some issues in this file and I don't think they are
// really related to the FIX_DOUBLE_SPACE patch. I'd rather think that
// this is a problem that has been here almost from day one and that a
// larger userbase with differenct access patters triggers the bad
// behaviour. (segfaults.) What I think happen is: In several places
// we store the paragraph in the current cursor and then moves the
// cursor. This movement of the cursor will delete paragraph at the
// old position if it is now empty. This will make the temporary
// pointer to the old cursor paragraph invalid and dangerous to use.
// And is some cases this will trigger a segfault. I have marked some
// of the cases where this happens with a warning, but I am sure there
// are others in this file and in text2.C. There is also a note in
// Delete() that you should read. In Delete I store the paragraph->id
// instead of a pointer to the paragraph. I am pretty sure this faulty
// use of temporary pointers to paragraphs that might have gotten
// invalidated (through a cursor movement) before they are used, are
// the cause of the strange crashes we get reported often.
//
// It is very tiresom to change this code, especially when it is as
// hard to read as it is. Help to fix all the cases where this is done
// would be greately appreciated.
//
// Lgb

int LyXText::singleWidth(ParagraphList::iterator pit, pos_type pos) const
{
	if (pos >= pit->size())
		return 0;

	char const c = pit->getChar(pos);
	LyXFont const & font = getFont(pit, pos);
	return singleWidth(pit, pos, c, font);
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
		if (font.language()->RightToLeft()) {
			if ((lyxrc.font_norm_type == LyXRC::ISO_8859_6_8 ||
			     lyxrc.font_norm_type == LyXRC::ISO_10646_1)
			    && font.language()->lang() == "arabic") {
				if (Encodings::IsComposeChar_arabic(c))
					return 0;
				else
					c = transformChar(c, *pit, pos);
			} else if (font.language()->lang() == "hebrew" &&
				 Encodings::IsComposeChar_hebrew(c))
				return 0;
		}
		return font_metrics::width(c, font);
	}

	if (c == Paragraph::META_INSET) {
		InsetOld * tmpinset = pit->getInset(pos);
		BOOST_ASSERT(tmpinset);
		if (tmpinset->lyxCode() == InsetOld::HFILL_CODE) {
			// Because of the representation as vertical lines
			return 3;
		}
		return tmpinset->width();
	}

	if (IsSeparatorChar(c))
		c = ' ';
	return font_metrics::width(c, font);
}


lyx::pos_type LyXText::log2vis(lyx::pos_type pos) const
{
	if (bidi_start == -1)
		return pos;
	else
		return log2vis_list[pos - bidi_start];
}


lyx::pos_type LyXText::vis2log(lyx::pos_type pos) const
{
	if (bidi_start == -1)
		return pos;
	else
		return vis2log_list[pos - bidi_start];
}


lyx::pos_type LyXText::bidi_level(lyx::pos_type pos) const
{
	if (bidi_start == -1)
		return 0;
	else
		return bidi_levels[pos - bidi_start];
}


bool LyXText::bidi_InRange(lyx::pos_type pos) const
{
	return bidi_start == -1 ||
		(bidi_start <= pos && pos <= bidi_end);
}


void LyXText::computeBidiTables(ParagraphList::iterator pit,
   Buffer const & buf, RowList::iterator row) const
{
	bidi_same_direction = true;
	if (!lyxrc.rtl_support) {
		bidi_start = -1;
		return;
	}

	InsetOld * inset = pit->inInset();
	if (inset && inset->owner() &&
	    inset->owner()->lyxCode() == InsetOld::ERT_CODE) {
		bidi_start = -1;
		return;
	}

	bidi_start = row->pos();
	bidi_end = lastPos(*pit, row);

	if (bidi_start > bidi_end) {
		bidi_start = -1;
		return;
	}

	if (bidi_end + 2 - bidi_start >
	    static_cast<pos_type>(log2vis_list.size())) {
		pos_type new_size =
			(bidi_end + 2 - bidi_start < 500) ?
			500 : 2 * (bidi_end + 2 - bidi_start);
		log2vis_list.resize(new_size);
		vis2log_list.resize(new_size);
		bidi_levels.resize(new_size);
	}

	vis2log_list[bidi_end + 1 - bidi_start] = -1;
	log2vis_list[bidi_end + 1 - bidi_start] = -1;

	BufferParams const & bufparams = buf.params();
	pos_type stack[2];
	bool const rtl_par =
		pit->isRightToLeftPar(bufparams);
	int level = 0;
	bool rtl = false;
	bool rtl0 = false;
	pos_type const body_pos = pit->beginningOfBody();

	for (pos_type lpos = bidi_start; lpos <= bidi_end; ++lpos) {
		bool is_space = pit->isLineSeparator(lpos);
		pos_type const pos =
			(is_space && lpos + 1 <= bidi_end &&
			 !pit->isLineSeparator(lpos + 1) &&
			 !pit->isNewline(lpos + 1))
			? lpos + 1 : lpos;
		LyXFont font = pit->getFontSettings(bufparams, pos);
		if (pos != lpos && 0 < lpos && rtl0 && font.isRightToLeft() &&
		    font.number() == LyXFont::ON &&
		    pit->getFontSettings(bufparams, lpos - 1).number()
		    == LyXFont::ON) {
			font = pit->getFontSettings(bufparams, lpos);
			is_space = false;
		}


		bool new_rtl = font.isVisibleRightToLeft();
		bool new_rtl0 = font.isRightToLeft();
		int new_level;

		if (lpos == body_pos - 1
		    && row->pos() < body_pos - 1
		    && is_space) {
			new_level = (rtl_par) ? 1 : 0;
			new_rtl = new_rtl0 = rtl_par;
		} else if (new_rtl0)
			new_level = (new_rtl) ? 1 : 2;
		else
			new_level = (rtl_par) ? 2 : 0;

		if (is_space && new_level >= level) {
			new_level = level;
			new_rtl = rtl;
			new_rtl0 = rtl0;
		}

		int new_level2 = new_level;

		if (level == new_level && rtl0 != new_rtl0) {
			--new_level2;
			log2vis_list[lpos - bidi_start] = (rtl) ? 1 : -1;
		} else if (level < new_level) {
			log2vis_list[lpos - bidi_start] =  (rtl) ? -1 : 1;
			if (new_level > rtl_par)
				bidi_same_direction = false;
		} else
			log2vis_list[lpos - bidi_start] = (new_rtl) ? -1 : 1;
		rtl = new_rtl;
		rtl0 = new_rtl0;
		bidi_levels[lpos - bidi_start] = new_level;

		while (level > new_level2) {
			pos_type old_lpos = stack[--level];
			int delta = lpos - old_lpos - 1;
			if (level % 2)
				delta = -delta;
			log2vis_list[lpos - bidi_start] += delta;
			log2vis_list[old_lpos - bidi_start] += delta;
		}
		while (level < new_level)
			stack[level++] = lpos;
	}

	while (level > 0) {
		pos_type const old_lpos = stack[--level];
		int delta = bidi_end - old_lpos;
		if (level % 2)
			delta = -delta;
		log2vis_list[old_lpos - bidi_start] += delta;
	}

	pos_type vpos = bidi_start - 1;
	for (pos_type lpos = bidi_start;
	     lpos <= bidi_end; ++lpos) {
		vpos += log2vis_list[lpos - bidi_start];
		vis2log_list[vpos - bidi_start] = lpos;
		log2vis_list[lpos - bidi_start] = vpos;
	}
}


// This method requires a previous call to ComputeBidiTables()
bool LyXText::isBoundary(Buffer const & buf, Paragraph const & par,
			 pos_type pos) const
{
	if (!lyxrc.rtl_support || pos == 0)
		return false;

	if (!bidi_InRange(pos - 1)) {
		/// This can happen if pos is the first char of a row.
		/// Returning false in this case is incorrect!
		return false;
	}

	bool const rtl = bidi_level(pos - 1) % 2;
	bool const rtl2 = bidi_InRange(pos)
		? bidi_level(pos) % 2
		: par.isRightToLeftPar(buf.params());
	return rtl != rtl2;
}


bool LyXText::isBoundary(Buffer const & buf, Paragraph const & par,
			 pos_type pos, LyXFont const & font) const
{
	if (!lyxrc.rtl_support)
		return false;    // This is just for speedup

	bool const rtl = font.isVisibleRightToLeft();
	bool const rtl2 = bidi_InRange(pos)
		? bidi_level(pos) % 2
		: par.isRightToLeftPar(buf.params());
	return rtl != rtl2;
}


int LyXText::leftMargin(ParagraphList::iterator pit, Row const & row) const
{
	LyXTextClass const & tclass =
		bv()->buffer()->params().getLyXTextClass();
	LyXLayout_ptr const & layout = pit->layout();

	string parindent = layout->parindent;

	int x = LEFT_MARGIN;

	x += font_metrics::signedWidth(tclass.leftmargin(), tclass.defaultfont());

	// this is the way, LyX handles the LaTeX-Environments.
	// I have had this idea very late, so it seems to be a
	// later added hack and this is true
	if (!pit->getDepth()) {
		if (pit->layout() == tclass.defaultLayout()) {
			// find the previous same level paragraph
			if (pit != ownerParagraphs().begin()) {
				ParagraphList::iterator newpit =
					depthHook(pit, ownerParagraphs(),
						  pit->getDepth());
				if (newpit == pit &&
				    newpit->layout()->nextnoindent)
					parindent.erase();
			}
		}
	} else {
		// find the next level paragraph

		ParagraphList::iterator newpar = outerHook(pit,
							   ownerParagraphs());

		// make a corresponding row. Needed to call leftMargin()

		// check wether it is a sufficent paragraph
		if (newpar != ownerParagraphs().end() &&
		    newpar->layout()->isEnvironment()) {
			x = leftMargin(newpar, Row(newpar->size()));
		}

		if (newpar != ownerParagraphs().end() &&
		    pit->layout() == tclass.defaultLayout()) {
			if (newpar->params().noindent())
				parindent.erase();
			else {
				parindent = newpar->layout()->parindent;
			}

		}
	}

	LyXFont const labelfont = getLabelFont(pit);
	switch (layout->margintype) {
	case MARGIN_DYNAMIC:
		if (!layout->leftmargin.empty()) {
			x += font_metrics::signedWidth(layout->leftmargin,
						  tclass.defaultfont());
		}
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
		if (!pit->empty() && row.pos() >= pit->beginningOfBody()) {
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
			if (row.pos() >= pit->beginningOfBody()) {
				x += font_metrics::signedWidth(layout->leftmargin,
							  labelfont);
			} else {
				x += font_metrics::signedWidth(layout->labelindent,
							  labelfont);
			}
		} else if (row.pos()
			   // Special case to fix problems with
			   // theorems (JMarc)
			   || (layout->labeltype == LABEL_STATIC
			       && layout->latextype == LATEX_ENVIRONMENT
			       && !isFirstInSequence(pit, ownerParagraphs()))) {
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

	case MARGIN_RIGHT_ADDRESS_BOX:
	{
		// ok, a terrible hack. The left margin depends on the widest
		// row in this paragraph. Do not care about footnotes, they
		// are *NOT* allowed in the LaTeX realisation of this layout.

		// find the first row of this paragraph
		RowList::iterator rit = pit->rows.begin();
		RowList::iterator end = pit->rows.end();
		int minfill = rit->fill();
		for ( ; rit != end; ++rit)
			if (rit->fill() < minfill)
				minfill = rit->fill();

		x += font_metrics::signedWidth(layout->leftmargin,
			tclass.defaultfont());
		x += minfill;
	}
	break;
	}

	if (workWidth() > 0 && !pit->params().leftIndent().zero()) {
		LyXLength const len = pit->params().leftIndent();
		int const tw = inset_owner ?
			inset_owner->latexTextWidth(bv()) : workWidth();
		x += len.inPixels(tw);
	}

	LyXAlignment align;

	if (pit->params().align() == LYX_ALIGN_LAYOUT)
		align = layout->align;
	else
		align = pit->params().align();

	// set the correct parindent
	if (row.pos() == 0) {
		if ((layout->labeltype == LABEL_NO_LABEL
		     || layout->labeltype == LABEL_TOP_ENVIRONMENT
		     || layout->labeltype == LABEL_CENTERED_TOP_ENVIRONMENT
		     || (layout->labeltype == LABEL_STATIC
			 && layout->latextype == LATEX_ENVIRONMENT
			 && !isFirstInSequence(pit, ownerParagraphs())))
		    && align == LYX_ALIGN_BLOCK
		    && !pit->params().noindent()
			// in tabulars and ert paragraphs are never indented!
			&& (!pit->inInset() || !pit->inInset()->owner() ||
				(pit->inInset()->owner()->lyxCode() != InsetOld::TABULAR_CODE &&
				 pit->inInset()->owner()->lyxCode() != InsetOld::ERT_CODE))
		    && (pit->layout() != tclass.defaultLayout() ||
			bv()->buffer()->params().paragraph_separation ==
			BufferParams::PARSEP_INDENT)) {
			x += font_metrics::signedWidth(parindent,
						  tclass.defaultfont());
		} else if (layout->labeltype == LABEL_BIBLIO) {
			// ale970405 Right width for bibitems
			x += bibitemMaxWidth(bv(), tclass.defaultfont());
		}
	}

	return x;
}


int LyXText::rightMargin(ParagraphList::iterator pit,
	Buffer const & buf, Row const &) const
{
	LyXTextClass const & tclass = buf.params().getLyXTextClass();
	LyXLayout_ptr const & layout = pit->layout();

	return PAPER_MARGIN
		+ font_metrics::signedWidth(tclass.rightmargin(),
				       tclass.defaultfont());
		+ font_metrics::signedWidth(layout->rightmargin,
				       tclass.defaultfont())
		* 4 / (pit->getDepth() + 4);
}


int LyXText::labelEnd(ParagraphList::iterator pit, Row const & row) const
{
	if (pit->layout()->margintype == MARGIN_MANUAL) {
		Row tmprow = row;
		tmprow.pos(pit->size());
		// return the beginning of the body
		return leftMargin(pit, tmprow);
	}

	// LabelEnd is only needed if the layout
	// fills a flushleft label.
	return 0;
}


namespace {

// this needs special handling - only newlines count as a break point
pos_type addressBreakPoint(pos_type i, Paragraph const & par)
{
	for (; i < par.size(); ++i) {
		if (par.isNewline(i))
			return i;
	}

	return par.size();
}

};


pos_type LyXText::rowBreakPoint(ParagraphList::iterator pit,
	Row const & row) const
{
	// maximum pixel width of a row.
	int width = workWidth()
		- rightMargin(pit, *bv()->buffer(), row);

	// inset->textWidth() returns -1 via workWidth(),
	// but why ?
	if (width < 0)
		return pit->size();

	LyXLayout_ptr const & layout = pit->layout();

	if (layout->margintype == MARGIN_RIGHT_ADDRESS_BOX)
		return addressBreakPoint(row.pos(), *pit);

	pos_type const pos = row.pos();
	pos_type const body_pos = pit->beginningOfBody();
	pos_type const last = pit->size();
	pos_type point = last;

	if (pos == last)
		return last;

	// Now we iterate through until we reach the right margin
	// or the end of the par, then choose the possible break
	// nearest that.

	int const left = leftMargin(pit, row);
	int x = left;

	// pixel width since last breakpoint
	int chunkwidth = 0;

	pos_type i = pos;

	// We re-use the font resolution for the entire font span when possible
	LyXFont font = getFont(pit, i);
	lyx::pos_type endPosOfFontSpan = pit->getEndPosOfFontSpan(i);

	for (; i < last; ++i) {
		if (pit->isNewline(i)) {
			point = i;
			break;
		}

		char const c = pit->getChar(i);
		if (i > endPosOfFontSpan) {
			font = getFont(pit, i);
			endPosOfFontSpan = pit->getEndPosOfFontSpan(i);
		}

		int thiswidth;

		// add the auto-hfill from label end to the body
		if (body_pos && i == body_pos) {
			thiswidth = font_metrics::width(layout->labelsep, getLabelFont(pit));
			if (pit->isLineSeparator(i - 1))
				thiswidth -= singleWidth(pit, i - 1);
			int left_margin = labelEnd(pit, row);
			if (thiswidth + x < left_margin)
				thiswidth = left_margin - x;
			thiswidth += singleWidth(pit, i, c, font);
		} else {
			thiswidth = singleWidth(pit, i, c, font);
		}

		x += thiswidth;
		chunkwidth += thiswidth;

		InsetOld * in = pit->isInset(i) ? pit->getInset(i) : 0;

		// break before a character that will fall off
		// the right of the row
		if (x >= width) {
			// if no break before, break here
			if (point == last || chunkwidth >= (width - left)) {
				if (pos < i)
					point = i - 1;
				else
					point = i;
			}
			break;
		}

		if (!in || in->isChar()) {
			// some insets are line separators too
			if (pit->isLineSeparator(i)) {
				point = i;
				chunkwidth = 0;
			}
			continue;
		}

		continue;
	}

	if (point == last && x >= width) {
		// didn't find one, break at the point we reached the edge
		point = i;
	} else if (i == last && x < width) {
		// found one, but we fell off the end of the par, so prefer
		// that.
		point = last;
	}

	// manual labels cannot be broken in LaTeX. But we
	// want to make our on-screen rendering of footnotes
	// etc. still break
	if (body_pos && point < body_pos)
		point = body_pos - 1;

	return point;
}


// returns the minimum space a row needs on the screen in pixel
int LyXText::fill(ParagraphList::iterator pit,
	RowList::iterator row, int paper_width) const
{
	if (paper_width < 0)
		return 0;

	int w;
	// get the pure distance
	pos_type const last = lastPos(*pit, row);

	LyXLayout_ptr const & layout = pit->layout();

	// special handling of the right address boxes
	if (layout->margintype == MARGIN_RIGHT_ADDRESS_BOX) {
		int const tmpfill = row->fill();
		row->fill(0); // the minfill in MarginLeft()
		w = leftMargin(pit, *row);
		row->fill(tmpfill);
	} else
		w = leftMargin(pit, *row);

	pos_type const body_pos = pit->beginningOfBody();
	pos_type i = row->pos();

	if (! pit->empty() && i <= last) {
		// We re-use the font resolution for the entire span when possible
		LyXFont font = getFont(pit, i);
		lyx::pos_type endPosOfFontSpan = pit->getEndPosOfFontSpan(i);
		while (i <= last) {
			if (body_pos > 0 && i == body_pos) {
				w += font_metrics::width(layout->labelsep, getLabelFont(pit));
				if (pit->isLineSeparator(i - 1))
					w -= singleWidth(pit, i - 1);
				int left_margin = labelEnd(pit, *row);
				if (w < left_margin)
					w = left_margin;
			}
			char const c = pit->getChar(i);
			if (IsPrintable(c) && i > endPosOfFontSpan) {
				// We need to get the next font
				font = getFont(pit, i);
				endPosOfFontSpan = pit->getEndPosOfFontSpan(i);
			}
			w += singleWidth(pit, i, c, font);
			++i;
		}
	}
	if (body_pos > 0 && body_pos > last) {
		w += font_metrics::width(layout->labelsep, getLabelFont(pit));
		if (last >= 0 && pit->isLineSeparator(last))
			w -= singleWidth(pit, last);
		int const left_margin = labelEnd(pit, *row);
		if (w < left_margin)
			w = left_margin;
	}

	int const fill = paper_width - w - rightMargin(pit, *bv()->buffer(), *row);

	// If this case happens, it means that our calculation
	// of the widths of the chars when we do rowBreakPoint()
	// went wrong for some reason. Typically in list bodies.
	// Things just about hobble on anyway, though you'll end
	// up with a "fill_separator" less than zero, which corresponds
	// to inter-word spacing being too small. Hopefully this problem
	// will die when the label hacks die.
	if (lyxerr.debugging() && fill < 0) {
		lyxerr[Debug::GUI] << "Eek, fill() was < 0: " << fill
			<< " w " << w << " paper_width " << paper_width
			<< " right margin " << rightMargin(pit, *bv()->buffer(), *row) << endl;
	}
	return fill;
}


// returns the minimum space a manual label needs on the screen in pixel
int LyXText::labelFill(ParagraphList::iterator pit, Row const & row) const
{
	pos_type last = pit->beginningOfBody();

	BOOST_ASSERT(last > 0);

	// -1 because a label ends either with a space that is in the label,
	// or with the beginning of a footnote that is outside the label.
	--last;

	// a separator at this end does not count
	if (pit->isLineSeparator(last))
		--last;

	int w = 0;
	pos_type i = row.pos();
	while (i <= last) {
		w += singleWidth(pit, i);
		++i;
	}

	int fill = 0;
	string const & labwidstr = pit->params().labelWidthString();
	if (!labwidstr.empty()) {
		LyXFont const labfont = getLabelFont(pit);
		int const labwidth = font_metrics::width(labwidstr, labfont);
		fill = max(labwidth - w, 0);
	}

	return fill;
}


LColor_color LyXText::backgroundColor() const
{
	if (inset_owner)
		return inset_owner->backgroundColor();
	else
		return LColor::background;
}


void LyXText::setHeightOfRow(ParagraphList::iterator pit, RowList::iterator rit)
{
	// get the maximum ascent and the maximum descent
	double layoutasc = 0;
	double layoutdesc = 0;
	double tmptop = 0;

	// ok, let us initialize the maxasc and maxdesc value.
	// Only the fontsize count. The other properties
	// are taken from the layoutfont. Nicer on the screen :)
	LyXLayout_ptr const & layout = pit->layout();

	// as max get the first character of this row then it can increase but not
	// decrease the height. Just some point to start with so we don't have to
	// do the assignment below too often.
	LyXFont font = getFont(pit, rit->pos());
	LyXFont::FONT_SIZE const tmpsize = font.size();
	font = getLayoutFont(pit);
	LyXFont::FONT_SIZE const size = font.size();
	font.setSize(tmpsize);

	LyXFont labelfont = getLabelFont(pit);

	double spacing_val = 1.0;
	if (!pit->params().spacing().isDefault())
		spacing_val = pit->params().spacing().getValue();
	else
		spacing_val = bv()->buffer()->params().spacing().getValue();
	//lyxerr << "spacing_val = " << spacing_val << endl;

	int maxasc  = int(font_metrics::maxAscent(font) *
	                  layout->spacing.getValue() * spacing_val);
	int maxdesc = int(font_metrics::maxDescent(font) *
	                  layout->spacing.getValue() * spacing_val);

	pos_type const pos_end = lastPos(*pit, rit);
	int labeladdon = 0;
	int maxwidth = 0;

	if (!pit->empty()) {
		// We re-use the font resolution for the entire font span when possible
		LyXFont font = getFont(pit, rit->pos());
		lyx::pos_type endPosOfFontSpan = pit->getEndPosOfFontSpan(rit->pos());

		// Optimisation
		Paragraph const & par = *pit;

		// Check if any insets are larger
		for (pos_type pos = rit->pos(); pos <= pos_end; ++pos) {
			// Manual inlined optimised version of common case of
			// "maxwidth += singleWidth(pit, pos);"
			char const c = par.getChar(pos);

			if (IsPrintable(c)) {
				if (pos > endPosOfFontSpan) {
					// We need to get the next font
					font = getFont(pit, pos);
					endPosOfFontSpan = par.getEndPosOfFontSpan(pos);
				}
				if (! font.language()->RightToLeft()) {
					maxwidth += font_metrics::width(c, font);
				} else {
					// Fall-back to normal case
					maxwidth += singleWidth(pit, pos, c, font);
					// And flush font cache
					endPosOfFontSpan = 0;
				}
			} else {
				// Special handling of insets - are any larger?
				if (par.isInset(pos)) {
					InsetOld const * tmpinset = par.getInset(pos);
					if (tmpinset) {
						maxwidth += tmpinset->width();
						maxasc = max(maxasc, tmpinset->ascent());
						maxdesc = max(maxdesc, tmpinset->descent());
					}
				} else {
					// Fall-back to normal case
					maxwidth += singleWidth(pit, pos, c, font);
					// And flush font cache
					endPosOfFontSpan = 0;
				}
			}
		}
	}

	// Check if any custom fonts are larger (Asger)
	// This is not completely correct, but we can live with the small,
	// cosmetic error for now.
	LyXFont::FONT_SIZE maxsize =
		pit->highestFontInRange(rit->pos(), pos_end, size);
	if (maxsize > font.size()) {
		font.setSize(maxsize);
		maxasc = max(maxasc, font_metrics::maxAscent(font));
		maxdesc = max(maxdesc, font_metrics::maxDescent(font));
	}

	// This is nicer with box insets:
	++maxasc;
	++maxdesc;

	rit->ascent_of_text(maxasc);

	// is it a top line?
	if (!rit->pos()) {
		BufferParams const & bufparams = bv()->buffer()->params();
		// some parksips VERY EASY IMPLEMENTATION
		if (bv()->buffer()->params().paragraph_separation ==
			BufferParams::PARSEP_SKIP)
		{
			if (layout->isParagraph()
				&& pit->getDepth() == 0
				&& pit != ownerParagraphs().begin())
			{
				maxasc += bufparams.getDefSkip().inPixels(*bv());
			} else if (pit != ownerParagraphs().begin() &&
				   boost::prior(pit)->layout()->isParagraph() &&
				   boost::prior(pit)->getDepth() == 0)
			{
				// is it right to use defskip here too? (AS)
				maxasc += bufparams.getDefSkip().inPixels(*bv());
			}
		}

		// the top margin
		if (pit == ownerParagraphs().begin() && !isInInset())
			maxasc += PAPER_MARGIN;

		// add the vertical spaces, that the user added
		maxasc += getLengthMarkerHeight(*bv(), pit->params().spaceTop());

		// do not forget the DTP-lines!
		// there height depends on the font of the nearest character
		if (pit->params().lineTop())

			maxasc += 2 * font_metrics::ascent('x', getFont(pit, 0));
		// and now the pagebreaks
		if (pit->params().pagebreakTop())
			maxasc += 3 * defaultRowHeight();

		if (pit->params().startOfAppendix())
			maxasc += 3 * defaultRowHeight();

		// This is special code for the chapter, since the label of this
		// layout is printed in an extra row
		if (layout->counter == "chapter" && bufparams.secnumdepth >= 0) {
			float spacing_val = 1.0;
			if (!pit->params().spacing().isDefault()) {
				spacing_val = pit->params().spacing().getValue();
			} else {
				spacing_val = bufparams.spacing().getValue();
			}

			labeladdon = int(font_metrics::maxDescent(labelfont) *
					 layout->spacing.getValue() *
					 spacing_val)
				+ int(font_metrics::maxAscent(labelfont) *
				      layout->spacing.getValue() *
				      spacing_val);
		}

		// special code for the top label
		if ((layout->labeltype == LABEL_TOP_ENVIRONMENT
		     || layout->labeltype == LABEL_BIBLIO
		     || layout->labeltype == LABEL_CENTERED_TOP_ENVIRONMENT)
		    && isFirstInSequence(pit, ownerParagraphs())
		    && !pit->getLabelstring().empty())
		{
			float spacing_val = 1.0;
			if (!pit->params().spacing().isDefault()) {
				spacing_val = pit->params().spacing().getValue();
			} else {
				spacing_val = bufparams.spacing().getValue();
			}

			labeladdon = int(
				(font_metrics::maxAscent(labelfont) +
				 font_metrics::maxDescent(labelfont)) *
				  layout->spacing.getValue() *
				  spacing_val
				+ layout->topsep * defaultRowHeight()
				+ layout->labelbottomsep * defaultRowHeight());
		}

		// And now the layout spaces, for example before and after
		// a section, or between the items of a itemize or enumerate
		// environment.

		if (!pit->params().pagebreakTop()) {
			ParagraphList::iterator prev =
				depthHook(pit, ownerParagraphs(),
					  pit->getDepth());
			if (prev != pit && prev->layout() == layout &&
				prev->getDepth() == pit->getDepth() &&
				prev->getLabelWidthString() == pit->getLabelWidthString())
			{
				layoutasc = (layout->itemsep * defaultRowHeight());
			} else if (rit != firstRow()) {
				tmptop = layout->topsep;

				//if (boost::prior(pit)->getDepth() >= pit->getDepth())
				//	tmptop -= getPar(previousRow(rit))->layout()->bottomsep;

				if (tmptop > 0)
					layoutasc = (tmptop * defaultRowHeight());
			} else if (pit->params().lineTop()) {
				tmptop = layout->topsep;

				if (tmptop > 0)
					layoutasc = (tmptop * defaultRowHeight());
			}

			prev = outerHook(pit, ownerParagraphs());
			if (prev != ownerParagraphs().end())  {
				maxasc += int(prev->layout()->parsep * defaultRowHeight());
			} else if (pit != ownerParagraphs().begin()) {
				ParagraphList::iterator prior_pit = boost::prior(pit);
				if (prior_pit->getDepth() != 0 ||
				    prior_pit->layout() == layout) {
					maxasc += int(layout->parsep * defaultRowHeight());
				}
			}
		}
	}

	// is it a bottom line?
	if (boost::next(rit) == pit->rows.end()) {
		// the bottom margin
		ParagraphList::iterator nextpit = boost::next(pit);
		if (nextpit == ownerParagraphs().end() && !isInInset())
			maxdesc += PAPER_MARGIN;

		// add the vertical spaces, that the user added
		maxdesc += getLengthMarkerHeight(*bv(), pit->params().spaceBottom());

		// do not forget the DTP-lines!
		// there height depends on the font of the nearest character
		if (pit->params().lineBottom())
			maxdesc += 2 * font_metrics::ascent('x',
					getFont(pit, max(pos_type(0), pit->size() - 1)));

		// and now the pagebreaks
		if (pit->params().pagebreakBottom())
			maxdesc += 3 * defaultRowHeight();

		// and now the layout spaces, for example before and after
		// a section, or between the items of a itemize or enumerate
		// environment
		if (!pit->params().pagebreakBottom()
		    && nextpit != ownerParagraphs().end()) {
			ParagraphList::iterator comparepit = pit;
			float usual = 0;
			float unusual = 0;

			if (comparepit->getDepth() > nextpit->getDepth()) {
				usual = (comparepit->layout()->bottomsep * defaultRowHeight());
				comparepit = depthHook(comparepit, ownerParagraphs(), nextpit->getDepth());
				if (comparepit->layout()!= nextpit->layout()
					|| nextpit->getLabelWidthString() !=
					comparepit->getLabelWidthString())
				{
					unusual = (comparepit->layout()->bottomsep * defaultRowHeight());
				}
				if (unusual > usual)
					layoutdesc = unusual;
				else
					layoutdesc = usual;
			} else if (comparepit->getDepth() ==  nextpit->getDepth()) {

				if (comparepit->layout() != nextpit->layout()
					|| nextpit->getLabelWidthString() !=
					comparepit->getLabelWidthString())
					layoutdesc = int(comparepit->layout()->bottomsep * defaultRowHeight());
			}
		}
	}

	// incalculate the layout spaces
	maxasc += int(layoutasc * 2 / (2 + pit->getDepth()));
	maxdesc += int(layoutdesc * 2 / (2 + pit->getDepth()));

	rit->height(maxasc + maxdesc + labeladdon);
	rit->baseline(maxasc + labeladdon);
	rit->top_of_text(rit->baseline() - font_metrics::maxAscent(font));

	double x = 0;
	rit->width(int(maxwidth + x));
	if (inset_owner) {
		width = max(0, workWidth());
		RowList::iterator rit = firstRow();
		RowList::iterator end = endRow();
		ParagraphList::iterator it = ownerParagraphs().begin();
		while (rit != end) {
			if (rit->width() > width)
				width = rit->width();
			nextRow(it, rit);
		}
	}
}


void LyXText::breakParagraph(ParagraphList & paragraphs, char keep_layout)
{
	// allow only if at start or end, or all previous is new text
	if (cursor.pos() && cursor.pos() != cursorPar()->size()
		&& cursorPar()->isChangeEdited(0, cursor.pos()))
		return;

	LyXTextClass const & tclass =
		bv()->buffer()->params().getLyXTextClass();
	LyXLayout_ptr const & layout = cursorPar()->layout();

	// this is only allowed, if the current paragraph is not empty or caption
	// and if it has not the keepempty flag active
	if (cursorPar()->empty() && !cursorPar()->allowEmpty()
	   && layout->labeltype != LABEL_SENSITIVE)
		return;

	recUndo(cursor.par());

	// Always break behind a space
	//
	// It is better to erase the space (Dekel)
	if (cursor.pos() < cursorPar()->size()
	     && cursorPar()->isLineSeparator(cursor.pos()))
	   cursorPar()->erase(cursor.pos());

	// break the paragraph
	if (keep_layout)
		keep_layout = 2;
	else
		keep_layout = layout->isEnvironment();

	// we need to set this before we insert the paragraph. IMO the
	// breakParagraph call should return a bool if it inserts the
	// paragraph before or behind and we should react on that one
	// but we can fix this in 1.3.0 (Jug 20020509)
	bool const isempty = (cursorPar()->allowEmpty() && cursorPar()->empty());
	::breakParagraph(bv()->buffer()->params(), paragraphs, cursorPar(),
			 cursor.pos(), keep_layout);

#warning Trouble Point! (Lgb)
	// When ::breakParagraph is called from within an inset we must
	// ensure that the correct ParagraphList is used. Today that is not
	// the case and the Buffer::paragraphs is used. Not good. (Lgb)
	ParagraphList::iterator next_par = boost::next(cursorPar());

	// well this is the caption hack since one caption is really enough
	if (layout->labeltype == LABEL_SENSITIVE) {
		if (!cursor.pos())
			// set to standard-layout
			cursorPar()->applyLayout(tclass.defaultLayout());
		else
			// set to standard-layout
			next_par->applyLayout(tclass.defaultLayout());
	}

	// if the cursor is at the beginning of a row without prior newline,
	// move one row up!
	// This touches only the screen-update. Otherwise we would may have
	// an empty row on the screen
	if (cursor.pos() && cursorRow()->pos() == cursor.pos()
	    && !cursorPar()->isNewline(cursor.pos() - 1))
	{
		cursorLeft(bv());
	}

	while (!next_par->empty() && next_par->isNewline(0))
		next_par->erase(0);

	updateCounters();
	redoParagraph(cursorPar());
	redoParagraph(next_par);

	// This check is necessary. Otherwise the new empty paragraph will
	// be deleted automatically. And it is more friendly for the user!
	if (cursor.pos() || isempty)
		setCursor(next_par, 0);
	else
		setCursor(cursorPar(), 0);
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
	recordUndo(bv(), Undo::INSERT, ownerParagraphs(), cursor.par(), cursor.par());

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
				number(bv()); // Set current_font.number to OFF
		} else if (IsDigit(c) &&
			   real_current_font.isVisibleRightToLeft()) {
			number(bv()); // Set current_font.number to ON

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
	// Here we could call finishUndo for every 20 characters inserted.
	// This is from my experience how emacs does it. (Lgb)
	static unsigned int counter;
	if (counter < 20) {
		++counter;
	} else {
		finishUndo();
		counter = 0;
	}
}


void LyXText::prepareToPrint(ParagraphList::iterator pit,
           RowList::iterator const rit) const
{
	double w = rit->fill();
	double fill_hfill = 0;
	double fill_label_hfill = 0;
	double fill_separator = 0;
	double x = 0;

	bool const is_rtl =
		pit->isRightToLeftPar(bv()->buffer()->params());
	if (is_rtl)
		x = workWidth() > 0 ? rightMargin(pit, *bv()->buffer(), *rit) : 0;
	else
		x = workWidth() > 0 ? leftMargin(pit, *rit) : 0;

	// is there a manual margin with a manual label
	LyXLayout_ptr const & layout = pit->layout();

	if (layout->margintype == MARGIN_MANUAL
	    && layout->labeltype == LABEL_MANUAL) {
		/// We might have real hfills in the label part
		int nlh = numberOfLabelHfills(*pit, rit);

		// A manual label par (e.g. List) has an auto-hfill
		// between the label text and the body of the
		// paragraph too.
		// But we don't want to do this auto hfill if the par
		// is empty.
		if (!pit->empty())
			++nlh;

		if (nlh && !pit->getLabelWidthString().empty()) {
			fill_label_hfill = labelFill(pit, *rit) / double(nlh);
		}
	}

	// are there any hfills in the row?
	int const nh = numberOfHfills(*pit, rit);

	if (nh) {
		if (w > 0)
			fill_hfill = w / nh;
	// we don't have to look at the alignment if it is ALIGN_LEFT and
	// if the row is already larger then the permitted width as then
	// we force the LEFT_ALIGN'edness!
	} else if (int(rit->width()) < workWidth()) {
		// is it block, flushleft or flushright?
		// set x how you need it
		int align;
		if (pit->params().align() == LYX_ALIGN_LAYOUT) {
			align = layout->align;
		} else {
			align = pit->params().align();
		}
		InsetOld * inset = 0;
		// ERT insets should always be LEFT ALIGNED on screen
		inset = pit->inInset();
		if (inset && inset->owner() &&
			inset->owner()->lyxCode() == InsetOld::ERT_CODE)
		{
			align = LYX_ALIGN_LEFT;
		}

		switch (align) {
	    case LYX_ALIGN_BLOCK:
		{
			int const ns = numberOfSeparators(*pit, rit);
			RowList::iterator next_row = boost::next(rit);
			if (ns
				&& next_row != pit->rows.end()
				&& !pit->isNewline(next_row->pos() - 1)
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

	computeBidiTables(pit, *bv()->buffer(), rit);
	if (is_rtl) {
		pos_type body_pos = pit->beginningOfBody();
		pos_type last = lastPos(*pit, rit);

		if (body_pos > 0 &&
				(body_pos - 1 > last ||
				 !pit->isLineSeparator(body_pos - 1))) {
			x += font_metrics::width(layout->labelsep, getLabelFont(pit));
			if (body_pos - 1 <= last)
				x += fill_label_hfill;
		}
	}

	rit->fill_hfill(fill_hfill);
	rit->fill_label_hfill(fill_label_hfill);
	rit->fill_separator(fill_separator);
	rit->x(x);
}


// important for the screen


// the cursor set functions have a special mechanism. When they
// realize, that you left an empty paragraph, they will delete it.
// They also delete the corresponding row

void LyXText::cursorRightOneWord()
{
	::cursorRightOneWord(*this, cursor, ownerParagraphs());
	setCursor(cursorPar(), cursor.pos());
}


// Skip initial whitespace at end of word and move cursor to *start*
// of prior word, not to end of next prior word.
void LyXText::cursorLeftOneWord()
{
	LyXCursor tmpcursor = cursor;
	::cursorLeftOneWord(*this, tmpcursor, ownerParagraphs());
	setCursor(getPar(tmpcursor), tmpcursor.pos());
}


void LyXText::selectWord(word_location loc)
{
	LyXCursor from = cursor;
	LyXCursor to = cursor;
	::getWord(*this, from, to, loc, ownerParagraphs());
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

	if (selection.start.par() == selection.end.par()) {
		LyXCursor & startc = selection.start;
		LyXCursor & endc = selection.end;
		recordUndo(bv(), Undo::INSERT, ownerParagraphs(), startc.par());
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

	if (selection.start.par() == selection.end.par()) {
		LyXCursor & startc = selection.start;
		LyXCursor & endc = selection.end;
		recordUndo(bv(), Undo::INSERT, ownerParagraphs(), startc.par());
		getPar(startc)->rejectChange(startc.pos(), endc.pos());
		finishUndo();
		clearSelection();
		redoParagraph(getPar(startc));
		setCursorIntern(startc.par(), 0);
	}
#warning handle multi par selection
}


// This function is only used by the spellchecker for NextWord().
// It doesn't handle LYX_ACCENTs and probably never will.
WordLangTuple const LyXText::selectNextWordToSpellcheck(float & value)
{
	if (the_locking_inset) {
		WordLangTuple word = the_locking_inset->selectNextWordToSpellcheck(bv(), value);
		if (!word.word().empty()) {
			value += float(cursor.y());
			value /= float(height);
			return word;
		}
		// we have to go on checking so move cursor to the next char
		if (cursor.pos() == cursorPar()->size()) {
			if (cursor.par() + 1 == int(ownerParagraphs().size()))
				return word;
			cursor.par(cursor.par() + 1);
			cursor.pos(0);
		} else
				cursor.pos(cursor.pos() + 1);
	}
	int const tmppar = cursor.par();

	// If this is not the very first word, skip rest of
	// current word because we are probably in the middle
	// of a word if there is text here.
	if (cursor.pos() || cursor.par() != 0) {
		while (cursor.pos() < cursorPar()->size()
		       && cursorPar()->isLetter(cursor.pos()))
			cursor.pos(cursor.pos() + 1);
	}

	// Now, skip until we have real text (will jump paragraphs)
	while (true) {
		ParagraphList::iterator cpit = cursorPar();
		pos_type const cpos = cursor.pos();

		if (cpos == cpit->size()) {
			if (cursor.par() + 1 != int(ownerParagraphs().size())) {
				cursor.par(cursor.par() + 1);
				cursor.pos(0);
				continue;
			}
			break;
		}

		bool const is_good_inset = cpit->isInset(cpos)
			&& cpit->getInset(cpos)->allowSpellcheck();

		if (!isDeletedText(*cpit, cpos)
		    && (is_good_inset || cpit->isLetter(cpos)))
			break;

		cursor.pos(cpos + 1);
	}

	// now check if we hit an inset so it has to be a inset containing text!
	if (cursor.pos() < cursorPar()->size() &&
	    cursorPar()->isInset(cursor.pos())) {
		// lock the inset!
		FuncRequest cmd(bv(), LFUN_INSET_EDIT, "left");
		cursorPar()->getInset(cursor.pos())->localDispatch(cmd);
		// now call us again to do the above trick
		// but obviously we have to start from down below ;)
		return bv()->text->selectNextWordToSpellcheck(value);
	}

	// Update the value if we changed paragraphs
	if (cursor.par() != tmppar) {
		setCursor(cursor.par(), cursor.pos());
		value = float(cursor.y())/float(height);
	}

	// Start the selection from here
	selection.cursor = cursor;

	string lang_code = getFont(cursorPar(), cursor.pos()).language()->code();
	// and find the end of the word (insets like optional hyphens
	// and ligature break are part of a word)
	while (cursor.pos() < cursorPar()->size()
	       && cursorPar()->isLetter(cursor.pos())
	       && !isDeletedText(*cursorPar(), cursor.pos()))
		cursor.pos(cursor.pos() + 1);

	// Finally, we copy the word to a string and return it
	string str;
	if (selection.cursor.pos() < cursor.pos()) {
		pos_type i;
		for (i = selection.cursor.pos(); i < cursor.pos(); ++i) {
			if (!cursorPar()->isInset(i))
				str += cursorPar()->getChar(i);
		}
	}
	return WordLangTuple(str, lang_code);
}


// This one is also only for the spellchecker
void LyXText::selectSelectedWord()
{
	if (the_locking_inset) {
		the_locking_inset->selectSelectedWord(bv());
		return;
	}
	// move cursor to the beginning
	setCursor(selection.cursor.par(), selection.cursor.pos());

	// set the sel cursor
	selection.cursor = cursor;

	// now find the end of the word
	while (cursor.pos() < cursorPar()->size()
	       && cursorPar()->isLetter(cursor.pos()))
		cursor.pos(cursor.pos() + 1);

	setCursor(cursorPar(), cursor.pos());

	// finally set the selection
	setSelection();
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

		// Great, CutSelection() gets rid of multiple spaces.
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
	if (cursorPar()->empty())
		// Paragraph is empty, so we just go to the right
		cursorRight(bv());
	else {
		LyXCursor tmpcursor = cursor;
		// We can't store the row over a regular setCursor
		// so we set it to 0 and reset it afterwards.
		selection.set(true); // to avoid deletion
		cursorEnd();
		setCursor(tmpcursor, tmpcursor.par(), tmpcursor.pos());
		selection.cursor = cursor;
		cursor = tmpcursor;
		setSelection();
		// What is this test for ??? (JMarc)
		if (!selection.set()) {
			deleteWordForward();
		} else {
			cutSelection(true, false);
		}
	}
}


void LyXText::changeCase(LyXText::TextCase action)
{
	LyXCursor from;
	LyXCursor to;

	if (selection.set()) {
		from = selection.start;
		to = selection.end;
	} else {
		from = cursor;
		::getWord(*this, from, to, lyx::PARTIAL_WORD, ownerParagraphs());
		setCursor(to.par(), to.pos() + 1);
	}

	recordUndo(bv(), Undo::ATOMIC, ownerParagraphs(), from.par(), to.par());

	pos_type pos = from.pos();
	int par = from.par();

	while (par != int(ownerParagraphs().size()) &&
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
	int const old_cur_par_id = cursorPar()->id();
	int const old_cur_par_prev_id =
		old_cursor.par() ? getPar(old_cursor.par() - 1)->id() : -1;

	// just move to the right
	cursorRight(bv());

	// CHECK Look at the comment here.
	// This check is not very good...
	// The cursorRightIntern calls DeleteEmptyParagraphMechanism
	// and that can very well delete the par or par->previous in
	// old_cursor. Will a solution where we compare paragraph id's
	//work better?
	int iid = cursor.par() ? getPar(cursor.par() - 1)->id() : -1;
	if (iid == old_cur_par_prev_id && cursorPar()->id() != old_cur_par_id) {
		// delete-empty-paragraph-mechanism has done it
		return;
	}

	// if you had success make a backspace
	if (old_cursor.par() != cursor.par() || old_cursor.pos() != cursor.pos()) {
		recordUndo(bv(), Undo::DELETE, ownerParagraphs(), old_cursor.par());
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
				ParagraphList::iterator tmppit = getPar(cursor.par() - 1);
				if (cursorPar()->layout() == tmppit->layout()
				    && cursorPar()->getAlign() == tmppit->getAlign()) {
					// Inherit bottom DTD from the paragraph below.
					// (the one we are deleting)
					tmppit->params().lineBottom(cursorPar()->params().lineBottom());
					tmppit->params().spaceBottom(cursorPar()->params().spaceBottom());
					tmppit->params().pagebreakBottom(cursorPar()->params().pagebreakBottom());
				}

				cursorLeft(bv());

				// the layout things can change the height of a row !
				redoParagraph();
				return;
			}
		}

		if (cursor.par() != 0)
			recordUndo(bv(), Undo::DELETE, ownerParagraphs(),
				cursor.par() - 1, cursor.par());

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

		if (cursorPar() != tmppit
		    && (cursorPar()->layout() == tmppit->layout()
			|| tmppit->layout() == tclass.defaultLayout())
		    && cursorPar()->getAlign() == tmppit->getAlign()) {
			mergeParagraph(bufparams,
				       buf.paragraphs(), cursorPar());

			if (cursor.pos() && cursorPar()->isSeparator(cursor.pos() - 1))
				cursor.pos(cursor.pos() - 1);

			// the row may have changed, block, hfills etc.
			updateCounters();
			setCursor(cursor.par(), cursor.pos(), false);
		}
	} else {
		// this is the code for a normal backspace, not pasting
		// any paragraphs
		recordUndo(bv(), Undo::DELETE, ownerParagraphs(), cursor.par());
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
	setCursor(cursor.par(), cursor.pos(), false, !cursor.boundary());
}


ParagraphList::iterator LyXText::cursorPar() const
{
	return getPar(cursor.par());
}


ParagraphList::iterator LyXText::getPar(LyXCursor const & cur) const
{
	return getPar(cur.par());
}


ParagraphList::iterator LyXText::getPar(int par) const
{
	BOOST_ASSERT(par >= 0);
	BOOST_ASSERT(par < int(ownerParagraphs().size()));
	ParagraphList::iterator pit = ownerParagraphs().begin();
	std::advance(pit, par);
	return pit;
}



RowList::iterator LyXText::cursorRow() const
{
	return getRow(cursorPar(), cursor.pos());
}


RowList::iterator LyXText::getRow(LyXCursor const & cur) const
{
	return getRow(getPar(cur), cur.pos());
}


RowList::iterator
LyXText::getRow(ParagraphList::iterator pit, pos_type pos) const
{
	RowList::iterator rit = boost::prior(pit->rows.end());
	RowList::iterator const begin = pit->rows.begin();

	while (rit != begin && rit->pos() > pos)
		--rit;

	return rit;
}


// returns pointer to some fancy row 'below' specified row
RowList::iterator LyXText::cursorIRow() const
{
	return getRow(cursorPar(), cursor.pos());
}


RowList::iterator LyXText::getRowNearY(int y,
	ParagraphList::iterator & pit) const
{
	//lyxerr << "getRowNearY: y " << y << endl;

	pit = boost::prior(ownerParagraphs().end());

	RowList::iterator rit = lastRow();
	RowList::iterator rbegin = firstRow();

	while (rit != rbegin && static_cast<int>(rit->y()) > y)
		previousRow(pit, rit);

	return rit;
}


int LyXText::getDepth() const
{
	return cursorPar()->getDepth();
}


RowList::iterator LyXText::firstRow() const
{
	return ownerParagraphs().front().rows.begin();
}


RowList::iterator LyXText::lastRow() const
{
	return boost::prior(endRow());
}


RowList::iterator LyXText::endRow() const
{
	return ownerParagraphs().back().rows.end();
}


void LyXText::nextRow(ParagraphList::iterator & pit,
	RowList::iterator & rit) const
{
	++rit;
	if (rit == pit->rows.end()) {
		++pit;
		if (pit == ownerParagraphs().end())
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
		BOOST_ASSERT(pit != ownerParagraphs().begin());
		--pit;
		rit = boost::prior(pit->rows.end());
	}
}


string LyXText::selectionAsString(Buffer const & buffer, bool label) const
{
	if (!selection.set())
		return string();

	// should be const ...
	ParagraphList::iterator startpit = getPar(selection.start);
	ParagraphList::iterator endpit = getPar(selection.end);
	size_t const startpos = selection.start.pos();
	size_t const endpos = selection.end.pos();

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
	return std::distance(ownerParagraphs().begin(), pit);
}
