/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include "lyxtext.h"
#include "lyxrow.h"
#include "paragraph.h"
#include "gettext.h"
#include "bufferparams.h"
#include "buffer.h"
#include "debug.h"
#include "intl.h"
#include "lyxrc.h"
#include "encoding.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"
#include "frontends/font_metrics.h"
#include "frontends/screen.h"
#include "frontends/WorkArea.h"
#include "bufferview_funcs.h"
#include "BufferView.h"
#include "language.h"
#include "ParagraphParameters.h"
#include "undo_funcs.h"
#include "WordLangTuple.h"
#include "paragraph_funcs.h"
#include "rowpainter.h"
#include "lyxrow_funcs.h"

#include "insets/insettext.h"

#include "support/textutils.h"
#include "support/LAssert.h"
#include "support/lstrings.h"

#include <algorithm>

using std::max;
using std::min;
using std::endl;
using std::pair;
using lyx::pos_type;

/// top, right, bottom pixel margin
extern int const PAPER_MARGIN = 20;
/// margin for changebar
extern int const CHANGEBAR_MARGIN = 10;
/// left margin
extern int const LEFT_MARGIN = PAPER_MARGIN + CHANGEBAR_MARGIN;

extern int bibitemMaxWidth(BufferView *, LyXFont const &);


BufferView * LyXText::bv()
{
	lyx::Assert(bv_owner != 0);
	return bv_owner;
}


BufferView * LyXText::bv() const
{
	lyx::Assert(bv_owner != 0);
	return bv_owner;
}


int LyXText::top_y() const
{
	if (anchor_row_ == rowlist_.end())
		return 0;

	int y = 0;

	RowList::iterator rit = rowlist_.begin();
	RowList::iterator end = rowlist_.end();
	for (; rit != end && rit != anchor_row_; ++rit) {
		y += rit->height();
	}
	return y + anchor_row_offset_;
}


void LyXText::top_y(int newy)
{
	if (rows().empty())
		return;
	lyxerr[Debug::GUI] << "setting top y = " << newy << endl;

	int y = newy;
	RowList::iterator rit = getRowNearY(y);

	if (rit == anchor_row_ && anchor_row_offset_ == newy - y) {
		lyxerr[Debug::GUI] << "top_y to same value, skipping update" << endl;
		return;
	}

	anchor_row_ = rit;
	anchor_row_offset_ = newy - y;
	lyxerr[Debug::GUI] << "changing reference to row: " << &*anchor_row_
	       << " offset: " << anchor_row_offset_ << endl;
	postPaint(0);
}


void LyXText::anchor_row(RowList::iterator rit)
{
	int old_y = top_y();
	anchor_row_offset_ = 0;
	anchor_row_ = rit;
	anchor_row_offset_ = old_y - top_y();
	lyxerr[Debug::GUI] << "anchor_row(): changing reference to row: "
			   << &*anchor_row_ << " offset: " << anchor_row_offset_
			   << endl;
}


int LyXText::workWidth() const
{
	if (inset_owner) {
		// FIXME: pass (const ?) ref
		return inset_owner->textWidth(bv());
	}
	return bv()->workWidth();
}


int LyXText::workWidth(Inset * inset) const
{
	Paragraph * par = inset->parOwner();
	lyx::Assert(par);

	pos_type pos = par->getPositionOfInset(inset);
	lyx::Assert(pos != -1);

	LyXLayout_ptr const & layout = par->layout();

	if (layout->margintype != MARGIN_RIGHT_ADDRESS_BOX) {
		// Optimization here: in most cases, the real row is
		// not needed, but only the par/pos values. So we just
		// construct a dummy row for leftMargin. (JMarc)
		Row dummyrow;
		dummyrow.par(par);
		dummyrow.pos(pos);
		return workWidth() - leftMargin(dummyrow);
	} else {
		int dummy_y;
		RowList::iterator row = getRow(par, pos, dummy_y);
		RowList::iterator frow = row;
		RowList::iterator beg = rowlist_.begin();

		while (frow != beg && frow->par() == boost::prior(frow)->par())
			--frow;

		// FIXME: I don't understand this code - jbl

		unsigned int maxw = 0;
		while (!isParEnd(*this, frow)) {
			if ((frow != row) && (maxw < frow->width()))
				maxw = frow->width();
			++frow;
		}
		if (maxw)
			return maxw;

	}
	return workWidth();
}


int LyXText::getRealCursorX() const
{
	int x = cursor.x();
	if (the_locking_inset && (the_locking_inset->getLyXText(bv())!= this))
		x = the_locking_inset->getLyXText(bv())->getRealCursorX();
	return x;
}


unsigned char LyXText::transformChar(unsigned char c, Paragraph * par,
			pos_type pos) const
{
	if (!Encodings::is_arabic(c))
		if (lyxrc.font_norm_type == LyXRC::ISO_8859_6_8 && IsDigit(c))
			return c + (0xb0 - '0');
		else
			return c;

	unsigned char const prev_char = pos > 0 ? par->getChar(pos-1) : ' ';
	unsigned char next_char = ' ';

	for (pos_type i = pos+1; i < par->size(); ++i)
		if (!Encodings::IsComposeChar_arabic(par->getChar(i))) {
			next_char = par->getChar(i);
			break;
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

int LyXText::singleWidth(Paragraph * par,
			 pos_type pos) const
{
	if (pos >= par->size())
		return 0;

	char const c = par->getChar(pos);
	return singleWidth(par, pos, c);
}


int LyXText::singleWidth(Paragraph * par,
			 pos_type pos, char c) const
{
	if (pos >= par->size())
		return 0;

	LyXFont const font = getFont(bv()->buffer(), par, pos);

	// The most common case is handled first (Asger)
	if (IsPrintable(c)) {
		if (font.language()->RightToLeft()) {
			if (font.language()->lang() == "arabic" &&
			    (lyxrc.font_norm_type == LyXRC::ISO_8859_6_8 ||
			     lyxrc.font_norm_type == LyXRC::ISO_10646_1)) {
				if (Encodings::IsComposeChar_arabic(c))
					return 0;
				else
					c = transformChar(c, par, pos);
			} else if (font.language()->lang() == "hebrew" &&
				 Encodings::IsComposeChar_hebrew(c))
				return 0;
		}
		return font_metrics::width(c, font);

	}

	if (c == Paragraph::META_INSET) {
		Inset * tmpinset = par->getInset(pos);
		if (tmpinset) {
			if (tmpinset->lyxCode() == Inset::HFILL_CODE) {
				// Because of the representation as vertical lines
				return 3;
			}
#if 1
#warning inset->update FIXME
			// this IS needed otherwise on initialitation we don't get the fill
			// of the row right (ONLY on initialization if we read a file!)
			// should be changed! (Jug 20011204)
			tmpinset->update(bv());
#endif
			return tmpinset->width(bv(), font);
		}
		return 0;
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
		return log2vis_list[pos-bidi_start];
}


lyx::pos_type LyXText::vis2log(lyx::pos_type pos) const
{
	if (bidi_start == -1)
		return pos;
	else
		return vis2log_list[pos-bidi_start];
}


lyx::pos_type LyXText::bidi_level(lyx::pos_type pos) const
{
	if (bidi_start == -1)
		return 0;
	else
		return bidi_levels[pos-bidi_start];
}


bool LyXText::bidi_InRange(lyx::pos_type pos) const
{
	return bidi_start == -1 ||
		(bidi_start <= pos && pos <= bidi_end);
}


void LyXText::computeBidiTables(Buffer const * buf,
				RowList::iterator row) const
{
	bidi_same_direction = true;
	if (!lyxrc.rtl_support) {
		bidi_start = -1;
		return;
	}

	Inset * inset = row->par()->inInset();
	if (inset && inset->owner() &&
	    inset->owner()->lyxCode() == Inset::ERT_CODE) {
		bidi_start = -1;
		return;
	}

	bidi_start = row->pos();
	bidi_end = lastPrintablePos(*this, row);

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

	pos_type stack[2];
	bool const rtl_par =
		row->par()->isRightToLeftPar(buf->params);
	int level = 0;
	bool rtl = false;
	bool rtl0 = false;
	pos_type const body_pos = row->par()->beginningOfBody();

	for (pos_type lpos = bidi_start; lpos <= bidi_end; ++lpos) {
		bool is_space = row->par()->isLineSeparator(lpos);
		pos_type const pos =
			(is_space && lpos + 1 <= bidi_end &&
			 !row->par()->isLineSeparator(lpos + 1) &&
			 !row->par()->isNewline(lpos + 1))
			? lpos + 1 : lpos;
		LyXFont font = row->par()->getFontSettings(buf->params, pos);
		if (pos != lpos && 0 < lpos && rtl0 && font.isRightToLeft() &&
		    font.number() == LyXFont::ON &&
		    row->par()->getFontSettings(buf->params, lpos - 1).number()
		    == LyXFont::ON) {
			font = row->par()->getFontSettings(buf->params, lpos);
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
bool LyXText::isBoundary(Buffer const * buf, Paragraph * par,
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
		: par->isRightToLeftPar(buf->params);
	return rtl != rtl2;
}


bool LyXText::isBoundary(Buffer const * buf, Paragraph * par,
			 pos_type pos, LyXFont const & font) const
{
	if (!lyxrc.rtl_support)
		return false;    // This is just for speedup

	bool const rtl = font.isVisibleRightToLeft();
	bool const rtl2 = bidi_InRange(pos)
		? bidi_level(pos) % 2
		: par->isRightToLeftPar(buf->params);
	return rtl != rtl2;
}


int LyXText::leftMargin(Row const & row) const
{
	Inset * ins;

	if (row.pos() < row.par()->size())
		if ((row.par()->getChar(row.pos()) == Paragraph::META_INSET) &&
		    (ins = row.par()->getInset(row.pos())) &&
		    (ins->needFullRow() || ins->display()))
			return LEFT_MARGIN;

	LyXTextClass const & tclass =
		bv()->buffer()->params.getLyXTextClass();
	LyXLayout_ptr const & layout = row.par()->layout();

	string parindent = layout->parindent;

	int x = LEFT_MARGIN;

	x += font_metrics::signedWidth(tclass.leftmargin(), tclass.defaultfont());

	// this is the way, LyX handles the LaTeX-Environments.
	// I have had this idea very late, so it seems to be a
	// later added hack and this is true
	if (!row.par()->getDepth()) {
		if (row.par()->layout() == tclass.defaultLayout()) {
			// find the previous same level paragraph
			if (row.par()->previous()) {
				Paragraph * newpar = row.par()
					->depthHook(row.par()->getDepth());
				if (newpar &&
				    newpar->layout()->nextnoindent)
					parindent.erase();
			}
		}
	} else {
		// find the next level paragraph

		Paragraph * newpar = row.par()->outerHook();

		// make a corresponding row. Needed to call LeftMargin()

		// check wether it is a sufficent paragraph
		if (newpar && newpar->layout()->isEnvironment()) {
			Row dummyrow;
			dummyrow.par(newpar);
			dummyrow.pos(newpar->size());
			x = leftMargin(dummyrow);
		} else {
			// this is no longer an error, because this function
			// is used to clear impossible depths after changing
			// a layout. Since there is always a redo,
			// LeftMargin() is always called
			row.par()->params().depth(0);
		}

		if (newpar && row.par()->layout() == tclass.defaultLayout()) {
			if (newpar->params().noindent())
				parindent.erase();
			else {
				parindent = newpar->layout()->parindent;
			}

		}
	}

	LyXFont const labelfont = getLabelFont(bv()->buffer(), row.par());
	switch (layout->margintype) {
	case MARGIN_DYNAMIC:
		if (!layout->leftmargin.empty()) {
			x += font_metrics::signedWidth(layout->leftmargin,
						  tclass.defaultfont());
		}
		if (!row.par()->getLabelstring().empty()) {
			x += font_metrics::signedWidth(layout->labelindent,
						  labelfont);
			x += font_metrics::width(row.par()->getLabelstring(),
					    labelfont);
			x += font_metrics::width(layout->labelsep, labelfont);
		}
		break;
	case MARGIN_MANUAL:
		x += font_metrics::signedWidth(layout->labelindent, labelfont);
		// The width of an empty par, even with manual label, should be 0
		if (!row.par()->empty() && row.pos() >= row.par()->beginningOfBody()) {
			if (!row.par()->getLabelWidthString().empty()) {
				x += font_metrics::width(row.par()->getLabelWidthString(),
					       labelfont);
				x += font_metrics::width(layout->labelsep, labelfont);
			}
		}
		break;
	case MARGIN_STATIC:
		x += font_metrics::signedWidth(layout->leftmargin, tclass.defaultfont()) * 4
			/ (row.par()->getDepth() + 4);
		break;
	case MARGIN_FIRST_DYNAMIC:
		if (layout->labeltype == LABEL_MANUAL) {
			if (row.pos() >= row.par()->beginningOfBody()) {
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
			       && ! row.par()->isFirstInSequence())) {
			x += font_metrics::signedWidth(layout->leftmargin,
						  labelfont);
		} else if (layout->labeltype != LABEL_TOP_ENVIRONMENT
			   && layout->labeltype != LABEL_BIBLIO
			   && layout->labeltype !=
			   LABEL_CENTERED_TOP_ENVIRONMENT) {
			x += font_metrics::signedWidth(layout->labelindent,
						  labelfont);
			x += font_metrics::width(layout->labelsep, labelfont);
			x += font_metrics::width(row.par()->getLabelstring(),
					    labelfont);
		}
		break;

	case MARGIN_RIGHT_ADDRESS_BOX:
	{
		// ok, a terrible hack. The left margin depends on the widest
		// row in this paragraph. Do not care about footnotes, they
		// are *NOT* allowed in the LaTeX realisation of this layout.

		// find the first row of this paragraph
		RowList::iterator tmprit = rowlist_.begin();
		while (tmprit != rowlist_.end()
		       && tmprit->par() != row.par())
			++tmprit;

		int minfill = tmprit->fill();
		while (boost::next(tmprit) != rowlist_.end() &&
		       boost::next(tmprit)->par() == row.par()) {
			++tmprit;
			if (tmprit->fill() < minfill)
				minfill = tmprit->fill();
		}

		x += font_metrics::signedWidth(layout->leftmargin,
			tclass.defaultfont());
		x += minfill;
	}
	break;
	}

	if ((workWidth() > 0) &&
		!row.par()->params().leftIndent().zero())
	{
		LyXLength const len = row.par()->params().leftIndent();
		int const tw = inset_owner ?
			inset_owner->latexTextWidth(bv()) : workWidth();
		x += len.inPixels(tw);
	}

	LyXAlignment align; // wrong type

	if (row.par()->params().align() == LYX_ALIGN_LAYOUT)
		align = layout->align;
	else
		align = row.par()->params().align();

	// set the correct parindent
	if (row.pos() == 0) {
		if ((layout->labeltype == LABEL_NO_LABEL
		     || layout->labeltype == LABEL_TOP_ENVIRONMENT
		     || layout->labeltype == LABEL_CENTERED_TOP_ENVIRONMENT
		     || (layout->labeltype == LABEL_STATIC
			 && layout->latextype == LATEX_ENVIRONMENT
			 && ! row.par()->isFirstInSequence()))
		    && align == LYX_ALIGN_BLOCK
		    && !row.par()->params().noindent()
			// in tabulars and ert paragraphs are never indented!
			&& (!row.par()->inInset() || !row.par()->inInset()->owner() ||
				(row.par()->inInset()->owner()->lyxCode() != Inset::TABULAR_CODE &&
				 row.par()->inInset()->owner()->lyxCode() != Inset::ERT_CODE))
		    && (row.par()->layout() != tclass.defaultLayout() ||
			bv()->buffer()->params.paragraph_separation ==
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


int LyXText::rightMargin(Buffer const & buf, Row const & row) const
{
	Inset * ins;

	if (row.pos() < row.par()->size())
		if ((row.par()->getChar(row.pos()) == Paragraph::META_INSET) &&
		    (ins = row.par()->getInset(row.pos())) &&
		    (ins->needFullRow() || ins->display()))
			return PAPER_MARGIN;

	LyXTextClass const & tclass = buf.params.getLyXTextClass();
	LyXLayout_ptr const & layout = row.par()->layout();

	int x = PAPER_MARGIN
		+ font_metrics::signedWidth(tclass.rightmargin(),
				       tclass.defaultfont());

	// this is the way, LyX handles the LaTeX-Environments.
	// I have had this idea very late, so it seems to be a
	// later added hack and this is true
	if (row.par()->getDepth()) {
		// find the next level paragraph

		Paragraph const * newpar = row.par();

		do {
			newpar = newpar->previous();
		} while (newpar
			 && newpar->getDepth() >= row.par()->getDepth());

		// make a corresponding row. Needed to call LeftMargin()

		// check wether it is a sufficent paragraph
		if (newpar && newpar->layout()->isEnvironment()) {
			Row dummyrow;
			dummyrow.par(const_cast<Paragraph *>(newpar));
			dummyrow.pos(0);
			x = rightMargin(buf, dummyrow);
		} else {
			// this is no longer an error, because this function
			// is used to clear impossible depths after changing
			// a layout. Since there is always a redo,
			// LeftMargin() is always called
			row.par()->params().depth(0);
		}
	}

	//lyxerr << "rightmargin: " << layout->rightmargin << endl;
	x += font_metrics::signedWidth(layout->rightmargin,
				       tclass.defaultfont())
		* 4 / (row.par()->getDepth() + 4);
	return x;
}


int LyXText::labelEnd(Row const & row) const
{
	if (row.par()->layout()->margintype == MARGIN_MANUAL) {
		Row tmprow = row;
		tmprow.pos(row.par()->size());
		// return the beginning of the body
		return leftMargin(tmprow);
	}

	// LabelEnd is only needed if the layout
	// fills a flushleft label.
	return 0;
}


namespace {

// this needs special handling - only newlines count as a break point
pos_type addressBreakPoint(pos_type i, Paragraph * par)
{
	for (; i < par->size(); ++i) {
		if (par->isNewline(i))
			return i;
	}

	return par->size();
}

};


pos_type
LyXText::rowBreakPoint(Row const & row) const
{
	Paragraph * par = row.par();

	// maximum pixel width of a row.
	int width = workWidth() - rightMargin(*bv()->buffer(), row);

	// inset->textWidth() returns -1 via workWidth(),
	// but why ?
	if (width < 0)
		return par->size();

	LyXLayout_ptr const & layout = par->layout();

	if (layout->margintype == MARGIN_RIGHT_ADDRESS_BOX)
		return addressBreakPoint(row.pos(), par);

	pos_type const pos = row.pos();
	pos_type const body_pos = par->beginningOfBody();
	pos_type const last = par->size();
	pos_type point = last;

	if (pos == last)
		return last;

	// Now we iterate through until we reach the right margin
	// or the end of the par, then choose the possible break
	// nearest that.

	int const left = leftMargin(const_cast<Row&>(row));
	int x = left;

	// pixel width since last breakpoint
	int chunkwidth = 0;

	pos_type i = pos;
	for (; i < last; ++i) {

		if (par->isNewline(i)) {
			point = i;
			break;
		}

		char const c = par->getChar(i);

		int thiswidth = singleWidth(par, i, c);

		// add the auto-hfill from label end to the body
		if (body_pos && i == body_pos) {
			thiswidth += font_metrics::width(layout->labelsep,
				    getLabelFont(bv()->buffer(), par));
			if (par->isLineSeparator(i - 1))
				thiswidth -= singleWidth(par, i - 1);
			int left_margin = labelEnd(row);
			if (thiswidth < left_margin)
				thiswidth = left_margin;
		}

		x += thiswidth;
		chunkwidth += thiswidth;

		Inset * in = par->isInset(i) ? par->getInset(i) : 0;
		bool fullrow = (in && (in->display() || in->needFullRow()));

		// break before a character that will fall off
		// the right of the row
		if (x >= width) {
			// if no break before or we are at an inset
			// that will take up a row, break here
			if (point == last || fullrow || chunkwidth >= (width - left)) {
				if (pos < i)
					point = i - 1;
				else
					point = i;
			}
			break;
		}

		if (!in || in->isChar()) {
			// some insets are line separators too
			if (par->isLineSeparator(i)) {
				point = i;
				chunkwidth = 0;
			}
			continue;
		}

		if (!fullrow)
			continue;

		// full row insets start at a new row
		if (i == pos) {
			if (pos < last - 1) {
				point = i;
				if (par->isLineSeparator(i + 1))
					++point;
			} else {
				// to avoid extra rows
				point = last;
			}
		} else {
			point = i - 1;
		}
		break;
	}

	if (point == last && x >= width) {
		// didn't find one, break at the point we reached the edge
		point = i;
	} else if (i == last && x < width) {
		// found one, but we fell off the end of the par, so prefer
		// that.
		point = last;
	}

	// manual labels cannot be broken in LaTeX
	if (body_pos && point < body_pos)
		point = body_pos - 1;

	return point;
}


// returns the minimum space a row needs on the screen in pixel
int LyXText::fill(RowList::iterator row, int paper_width) const
{
	if (paper_width < 0)
		return 0;

	int w;
	// get the pure distance
	pos_type const last = lastPrintablePos(*this, row);

	// special handling of the right address boxes
	if (row->par()->layout()->margintype == MARGIN_RIGHT_ADDRESS_BOX) {
		int const tmpfill = row->fill();
		row->fill(0); // the minfill in MarginLeft()
		w = leftMargin(*row);
		row->fill(tmpfill);
	} else
		w = leftMargin(*row);

	Paragraph * par = row->par();
	LyXLayout_ptr const & layout = par->layout();

	pos_type const body_pos = par->beginningOfBody();
	pos_type i = row->pos();

	while (i <= last) {
		if (body_pos > 0 && i == body_pos) {
			w += font_metrics::width(layout->labelsep, getLabelFont(bv()->buffer(), par));
			if (par->isLineSeparator(i - 1))
				w -= singleWidth(par, i - 1);
			int left_margin = labelEnd(*row);
			if (w < left_margin)
				w = left_margin;
		}
		w += singleWidth(par, i);
		++i;
	}
	if (body_pos > 0 && body_pos > last) {
		w += font_metrics::width(layout->labelsep, getLabelFont(bv()->buffer(), par));
		if (last >= 0 && par->isLineSeparator(last))
			w -= singleWidth(par, last);
		int const left_margin = labelEnd(*row);
		if (w < left_margin)
			w = left_margin;
	}

	int const fill = paper_width - w - rightMargin(*bv()->buffer(), *row);
	return fill;
}


// returns the minimum space a manual label needs on the screen in pixel
int LyXText::labelFill(Row const & row) const
{
	pos_type last = row.par()->beginningOfBody();

	lyx::Assert(last > 0);

	// -1 because a label ends either with a space that is in the label,
	// or with the beginning of a footnote that is outside the label.
	--last;

	// a separator at this end does not count
	if (row.par()->isLineSeparator(last))
		--last;

	int w = 0;
	pos_type i = row.pos();
	while (i <= last) {
		w += singleWidth(row.par(), i);
		++i;
	}

	int fill = 0;
	string const & labwidstr = row.par()->params().labelWidthString();
	if (!labwidstr.empty()) {
		LyXFont const labfont = getLabelFont(bv()->buffer(), row.par());
		int const labwidth = font_metrics::width(labwidstr, labfont);
		fill = max(labwidth - w, 0);
	}

	return fill;
}


LColor::color LyXText::backgroundColor() const
{
	if (inset_owner)
		return inset_owner->backgroundColor();
	else
		return LColor::background;
}


void LyXText::setHeightOfRow(RowList::iterator rit)
{
	// No need to do anything then...
	if (rit == rows().end())
		return;

	// get the maximum ascent and the maximum descent
	int asc = 0;
	int desc = 0;
	float layoutasc = 0;
	float layoutdesc = 0;
	float tmptop = 0;
	LyXFont tmpfont;
	Inset * tmpinset = 0;

	// ok , let us initialize the maxasc and maxdesc value.
	// This depends in LaTeX of the font of the last character
	// in the paragraph. The hack below is necessary because
	// of the possibility of open footnotes

	// Correction: only the fontsize count. The other properties
	//  are taken from the layoutfont. Nicer on the screen :)
	Paragraph * par = rit->par();
	Paragraph * firstpar = par;

	LyXLayout_ptr const & layout = firstpar->layout();

	// as max get the first character of this row then it can increase but not
	// decrease the height. Just some point to start with so we don't have to
	// do the assignment below too often.
	LyXFont font = getFont(bv()->buffer(), par, rit->pos());
	LyXFont::FONT_SIZE const tmpsize = font.size();
	font = getLayoutFont(bv()->buffer(), par);
	LyXFont::FONT_SIZE const size = font.size();
	font.setSize(tmpsize);

	LyXFont labelfont = getLabelFont(bv()->buffer(), par);

	float spacing_val = 1.0;
	if (!rit->par()->params().spacing().isDefault()) {
		spacing_val = rit->par()->params().spacing().getValue();
	} else {
		spacing_val = bv()->buffer()->params.spacing.getValue();
	}
	//lyxerr << "spacing_val = " << spacing_val << endl;

	int maxasc = int(font_metrics::maxAscent(font) *
			 layout->spacing.getValue() *
			 spacing_val);
	int maxdesc = int(font_metrics::maxDescent(font) *
			  layout->spacing.getValue() *
			  spacing_val);

	pos_type const pos_end = lastPos(*this, rit);
	int labeladdon = 0;
	int maxwidth = 0;

	if (!rit->par()->empty()) {
		// Check if any insets are larger
		for (pos_type pos = rit->pos(); pos <= pos_end; ++pos) {
			if (rit->par()->isInset(pos)) {
				tmpfont = getFont(bv()->buffer(), rit->par(), pos);
				tmpinset = rit->par()->getInset(pos);
				if (tmpinset) {
#if 1 // this is needed for deep update on initialitation
#warning inset->update FIXME
					tmpinset->update(bv());
#endif
					asc = tmpinset->ascent(bv(), tmpfont);
					desc = tmpinset->descent(bv(), tmpfont);
					maxwidth += tmpinset->width(bv(), tmpfont);
					maxasc = max(maxasc, asc);
					maxdesc = max(maxdesc, desc);
				}
			} else {
				maxwidth += singleWidth(rit->par(), pos);
			}
		}
	}

	// Check if any custom fonts are larger (Asger)
	// This is not completely correct, but we can live with the small,
	// cosmetic error for now.
	LyXFont::FONT_SIZE maxsize =
		rit->par()->highestFontInRange(rit->pos(), pos_end, size);
	if (maxsize > font.size()) {
		font.setSize(maxsize);

		asc = font_metrics::maxAscent(font);
		desc = font_metrics::maxDescent(font);
		if (asc > maxasc)
			maxasc = asc;
		if (desc > maxdesc)
			maxdesc = desc;
	}

	// This is nicer with box insets:
	++maxasc;
	++maxdesc;

	rit->ascent_of_text(maxasc);

	// is it a top line?
	if (!rit->pos() && (rit->par() == firstpar)) {

		// some parksips VERY EASY IMPLEMENTATION
		if (bv()->buffer()->params.paragraph_separation ==
			BufferParams::PARSEP_SKIP)
		{
			if (layout->isParagraph()
				&& firstpar->getDepth() == 0
				&& firstpar->previous())
			{
				maxasc += bv()->buffer()->params.getDefSkip().inPixels(*bv());
			} else if (firstpar->previous() &&
				   firstpar->previous()->layout()->isParagraph() &&
				   firstpar->previous()->getDepth() == 0)
			{
				// is it right to use defskip here too? (AS)
				maxasc += bv()->buffer()->params.getDefSkip().inPixels(*bv());
			}
		}

		// the top margin
		if (!rit->par()->previous() && !isInInset())
			maxasc += PAPER_MARGIN;

		// add the vertical spaces, that the user added
		maxasc += getLengthMarkerHeight(*bv(), firstpar->params().spaceTop());

		// do not forget the DTP-lines!
		// there height depends on the font of the nearest character
		if (firstpar->params().lineTop())

			maxasc += 2 * font_metrics::ascent('x', getFont(bv()->buffer(),
					firstpar, 0));
		// and now the pagebreaks
		if (firstpar->params().pagebreakTop())
			maxasc += 3 * defaultRowHeight();

		if (firstpar->params().startOfAppendix())
			maxasc += 3 * defaultRowHeight();

		// This is special code for the chapter, since the label of this
		// layout is printed in an extra row
		if (layout->labeltype == LABEL_COUNTER_CHAPTER
			&& bv()->buffer()->params.secnumdepth >= 0)
		{
			float spacing_val = 1.0;
			if (!rit->par()->params().spacing().isDefault()) {
				spacing_val = rit->par()->params().spacing().getValue();
			} else {
				spacing_val = bv()->buffer()->params.spacing.getValue();
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
		    && rit->par()->isFirstInSequence()
		    && !rit->par()->getLabelstring().empty())
		{
			float spacing_val = 1.0;
			if (!rit->par()->params().spacing().isDefault()) {
				spacing_val = rit->par()->params().spacing().getValue();
			} else {
				spacing_val = bv()->buffer()->params.spacing.getValue();
			}

			labeladdon = int(
				(font_metrics::maxAscent(labelfont) *
				 layout->spacing.getValue() *
				 spacing_val)
				+(font_metrics::maxDescent(labelfont) *
				  layout->spacing.getValue() *
				  spacing_val)
				+ layout->topsep * defaultRowHeight()
				+ layout->labelbottomsep * defaultRowHeight());
		}

		// and now the layout spaces, for example before and after a section,
		// or between the items of a itemize or enumerate environment

		if (!firstpar->params().pagebreakTop()) {
			Paragraph * prev = rit->par()->previous();
			if (prev)
				prev = rit->par()->depthHook(rit->par()->getDepth());
			if (prev && prev->layout() == firstpar->layout() &&
				prev->getDepth() == firstpar->getDepth() &&
				prev->getLabelWidthString() == firstpar->getLabelWidthString())
			{
				layoutasc = (layout->itemsep * defaultRowHeight());
			} else if (rit != rows().begin()) {
				tmptop = layout->topsep;

				if (boost::prior(rit)->par()->getDepth() >= rit->par()->getDepth())
					tmptop -= boost::prior(rit)->par()->layout()->bottomsep;

				if (tmptop > 0)
					layoutasc = (tmptop * defaultRowHeight());
			} else if (rit->par()->params().lineTop()) {
				tmptop = layout->topsep;

				if (tmptop > 0)
					layoutasc = (tmptop * defaultRowHeight());
			}

			prev = rit->par()->outerHook();
			if (prev)  {
				maxasc += int(prev->layout()->parsep * defaultRowHeight());
			} else {
				if (firstpar->previous() &&
					firstpar->previous()->getDepth() == 0 &&
					firstpar->previous()->layout() !=
					firstpar->layout())
				{
					// avoid parsep
				} else if (firstpar->previous()) {
					maxasc += int(layout->parsep * defaultRowHeight());
				}
			}
		}
	}

	// is it a bottom line?
	if (rit->par() == par
		&& (boost::next(rit) == rows().end() ||
		    boost::next(rit)->par() != rit->par())) {
		// the bottom margin
		if (!par->next() && !isInInset())
			maxdesc += PAPER_MARGIN;

		// add the vertical spaces, that the user added
		maxdesc += getLengthMarkerHeight(*bv(), firstpar->params().spaceBottom());

		// do not forget the DTP-lines!
		// there height depends on the font of the nearest character
		if (firstpar->params().lineBottom())
			maxdesc += 2 * font_metrics::ascent('x',
						       getFont(bv()->buffer(),
							       par,
							       max(pos_type(0), par->size() - 1)));

		// and now the pagebreaks
		if (firstpar->params().pagebreakBottom())
			maxdesc += 3 * defaultRowHeight();

		// and now the layout spaces, for example before and after
		// a section, or between the items of a itemize or enumerate
		// environment
		if (!firstpar->params().pagebreakBottom()
		    && rit->par()->next()) {
			Paragraph * nextpar = rit->par()->next();
			Paragraph * comparepar = rit->par();
			float usual = 0;
			float unusual = 0;

			if (comparepar->getDepth() > nextpar->getDepth()) {
				usual = (comparepar->layout()->bottomsep * defaultRowHeight());
				comparepar = comparepar->depthHook(nextpar->getDepth());
				if (comparepar->layout()!= nextpar->layout()
					|| nextpar->getLabelWidthString() !=
					comparepar->getLabelWidthString())
				{
					unusual = (comparepar->layout()->bottomsep * defaultRowHeight());
				}
				if (unusual > usual)
					layoutdesc = unusual;
				else
					layoutdesc = usual;
			} else if (comparepar->getDepth() ==  nextpar->getDepth()) {

				if (comparepar->layout() != nextpar->layout()
					|| nextpar->getLabelWidthString() !=
					comparepar->getLabelWidthString())
					layoutdesc = int(comparepar->layout()->bottomsep * defaultRowHeight());
			}
		}
	}

	// incalculate the layout spaces
	maxasc += int(layoutasc * 2 / (2 + firstpar->getDepth()));
	maxdesc += int(layoutdesc * 2 / (2 + firstpar->getDepth()));

	// calculate the new height of the text
	height -= rit->height();

	rit->height(maxasc + maxdesc + labeladdon);
	rit->baseline(maxasc + labeladdon);

	height += rit->height();

	rit->top_of_text(rit->baseline() - font_metrics::maxAscent(font));

	float x = 0;
	if (layout->margintype != MARGIN_RIGHT_ADDRESS_BOX) {
		float dummy;
		// this IS needed
		rit->width(maxwidth);
		prepareToPrint(rit, x, dummy, dummy, dummy, false);
	}
	rit->width(int(maxwidth + x));
	if (inset_owner) {
		width = max(0, workWidth());
		RowList::iterator it = rows().begin();
		RowList::iterator end = rows().end();
		for (; it != end; ++it) {
			if (it->width() > width)
				width = it->width();
		}
	}
}


// Appends the implicit specified paragraph before the specified row,
// start at the implicit given position
void LyXText::appendParagraph(RowList::iterator rowit)
{
	lyx::Assert(rowit != rowlist_.end());

	pos_type const last = rowit->par()->size();
	bool done = false;

	do {
		pos_type z = rowBreakPoint(*rowit);

		RowList::iterator tmprow = rowit;

		if (z < last) {
			++z;
			Row newrow(rowit->par(), z);
			rowit = rowlist_.insert(boost::next(rowit), newrow);
		} else {
			done = true;
		}

		// Set the dimensions of the row
		// fixed fill setting now by calling inset->update() in
		// SingleWidth when needed!
		tmprow->fill(fill(tmprow, workWidth()));
		setHeightOfRow(tmprow);

	} while (!done);
}


void LyXText::breakAgain(RowList::iterator rit)
{
	lyx::Assert(rit != rows().end());

	bool not_ready = true;

	do  {
		pos_type z = rowBreakPoint(*rit);
		RowList::iterator tmprit = rit;
		RowList::iterator end = rows().end();

		if (z < rit->par()->size()) {
			if (boost::next(rit) == end ||
			    (boost::next(rit) != end &&
			     boost::next(rit)->par() != rit->par())) {
				// insert a new row
				++z;
				Row newrow(rit->par(), z);
				rit = rowlist_.insert(boost::next(rit), newrow);
			} else  {
				++rit;
				++z;
				if (rit->pos() == z)
					not_ready = false; // the rest will not change
				else {
					rit->pos(z);
				}
			}
		} else {
			// if there are some rows too much, delete them
			// only if you broke the whole paragraph!
			RowList::iterator tmprit2 = rit;
			while (boost::next(tmprit2) != end
			       && boost::next(tmprit2)->par() == rit->par()) {
				++tmprit2;
			}
			while (tmprit2 != rit) {
				--tmprit2;
				removeRow(boost::next(tmprit2));
			}
			not_ready = false;
		}

		// set the dimensions of the row
		tmprit->fill(fill(tmprit, workWidth()));
		setHeightOfRow(tmprit);
	} while (not_ready);
}


// this is just a little changed version of break again
void LyXText::breakAgainOneRow(RowList::iterator rit)
{
	lyx::Assert(rit != rows().end());

	pos_type z = rowBreakPoint(*rit);
	RowList::iterator tmprit = rit;
	RowList::iterator end = rows().end();

	if (z < rit->par()->size()) {
		if (boost::next(rit) == end ||
		    (boost::next(rit) != end &&
		     boost::next(rit)->par() != rit->par())) {
			// insert a new row
			++z;
			Row newrow(rit->par(), z);
			rit = rowlist_.insert(boost::next(rit), newrow);
		} else  {
			++rit;
			++z;
			if (rit->pos() != z)
				rit->pos(z);
		}
	} else {
		// if there are some rows too much, delete them
		// only if you broke the whole paragraph!
		RowList::iterator tmprit2 = rit;
		while (boost::next(tmprit2) != end
		       && boost::next(tmprit2)->par() == rit->par()) {
			++tmprit2;
		}
		while (tmprit2 != rit) {
			--tmprit2;
			removeRow(boost::next(tmprit2));
		}
	}

	// set the dimensions of the row
	tmprit->fill(fill(tmprit, workWidth()));
	setHeightOfRow(tmprit);
}


void LyXText::breakParagraph(ParagraphList & paragraphs, char keep_layout)
{
	// allow only if at start or end, or all previous is new text
	if (cursor.pos() && cursor.pos() != cursor.par()->size()
		&& cursor.par()->isChangeEdited(0, cursor.pos()))
		return;

	LyXTextClass const & tclass =
		bv()->buffer()->params.getLyXTextClass();
	LyXLayout_ptr const & layout = cursor.par()->layout();

	// this is only allowed, if the current paragraph is not empty or caption
	// and if it has not the keepempty flag aktive
	if (cursor.par()->empty()
	   && layout->labeltype != LABEL_SENSITIVE
	   && !layout->keepempty)
		return;

	setUndo(bv(), Undo::FINISH, cursor.par(), cursor.par()->next());

	// Always break behind a space
	//
	// It is better to erase the space (Dekel)
	if (cursor.pos() < cursor.par()->size()
	     && cursor.par()->isLineSeparator(cursor.pos()))
	   cursor.par()->erase(cursor.pos());
	// cursor.pos(cursor.pos() + 1);

	// break the paragraph
	if (keep_layout)
		keep_layout = 2;
	else
		keep_layout = layout->isEnvironment();

	// we need to set this before we insert the paragraph. IMO the
	// breakParagraph call should return a bool if it inserts the
	// paragraph before or behind and we should react on that one
	// but we can fix this in 1.3.0 (Jug 20020509)
	bool const isempty = (layout->keepempty && cursor.par()->empty());
	::breakParagraph(bv()->buffer()->params, paragraphs, cursor.par(), cursor.pos(),
		       keep_layout);

	// well this is the caption hack since one caption is really enough
	if (layout->labeltype == LABEL_SENSITIVE) {
		if (!cursor.pos())
			// set to standard-layout
			cursor.par()->applyLayout(tclass.defaultLayout());
		else
			// set to standard-layout
			cursor.par()->next()->applyLayout(tclass.defaultLayout());
	}

	// if the cursor is at the beginning of a row without prior newline,
	// move one row up!
	// This touches only the screen-update. Otherwise we would may have
	// an empty row on the screen
	if (cursor.pos() && !cursor.row()->par()->isNewline(cursor.row()->pos() - 1)
			 && cursor.row()->pos() == cursor.pos())
	{
		cursorLeft(bv());
	}

	int y = cursor.y() - cursor.row()->baseline();

	// Do not forget the special right address boxes
	if (layout->margintype == MARGIN_RIGHT_ADDRESS_BOX) {
		RowList::iterator r = cursor.row();
		while (r != rows().begin() && boost::prior(r)->par() == r->par()) {
			--r;
			y -= r->height();
		}
	}

	postPaint(y);

	removeParagraph(cursor.row());

	// set the dimensions of the cursor row
	cursor.row()->fill(fill(cursor.row(), workWidth()));

	setHeightOfRow(cursor.row());

#warning Trouble Point! (Lgb)
	// When ::breakParagraph is called from within an inset we must
	// ensure that the correct ParagraphList is used. Today that is not
	// the case and the Buffer::paragraphs is used. Not good. (Lgb)
	while (!cursor.par()->next()->empty()
	  && cursor.par()->next()->isNewline(0))
	   cursor.par()->next()->erase(0);

	insertParagraph(cursor.par()->next(), boost::next(cursor.row()));
	updateCounters();

	// This check is necessary. Otherwise the new empty paragraph will
	// be deleted automatically. And it is more friendly for the user!
	if (cursor.pos() || isempty)
		setCursor(cursor.par()->next(), 0);
	else
		setCursor(cursor.par(), 0);

	if (boost::next(cursor.row()) != rows().end())
		breakAgain(boost::next(cursor.row()));

	need_break_row = rows().end();
}


// Just a macro to make some thing easier.
void LyXText::redoParagraph()
{
	clearSelection();
	redoParagraphs(cursor, cursor.par()->next());
	setCursorIntern(cursor.par(), cursor.pos());
}


// insert a character, moves all the following breaks in the
// same Paragraph one to the right and make a rebreak
void LyXText::insertChar(char c)
{
	setUndo(bv(), Undo::INSERT, cursor.par(), cursor.par()->next());

	// When the free-spacing option is set for the current layout,
	// disable the double-space checking

	bool const freeSpacing = cursor.row()->par()->layout()->free_spacing ||
		cursor.row()->par()->isFreeSpacing();

	if (lyxrc.auto_number) {
		static string const number_operators = "+-/*";
		static string const number_unary_operators = "+-";
		static string const number_seperators = ".,:";

		if (current_font.number() == LyXFont::ON) {
			if (!IsDigit(c) && !contains(number_operators, c) &&
			    !(contains(number_seperators, c) &&
			      cursor.pos() >= 1 &&
			      cursor.pos() < cursor.par()->size() &&
			      getFont(bv()->buffer(),
				      cursor.par(),
				      cursor.pos()).number() == LyXFont::ON &&
			      getFont(bv()->buffer(),
				      cursor.par(),
				      cursor.pos() - 1).number() == LyXFont::ON)
			   )
				number(bv()); // Set current_font.number to OFF
		} else if (IsDigit(c) &&
			   real_current_font.isVisibleRightToLeft()) {
			number(bv()); // Set current_font.number to ON

			if (cursor.pos() > 0) {
				char const c = cursor.par()->getChar(cursor.pos() - 1);
				if (contains(number_unary_operators, c) &&
				    (cursor.pos() == 1 ||
				     cursor.par()->isSeparator(cursor.pos() - 2) ||
				     cursor.par()->isNewline(cursor.pos() - 2))
				  ) {
					setCharFont(bv()->buffer(),
						    cursor.par(),
						    cursor.pos() - 1,
						    current_font);
				} else if (contains(number_seperators, c) &&
					   cursor.pos() >= 2 &&
					   getFont(bv()->buffer(),
						   cursor.par(),
						   cursor.pos() - 2).number() == LyXFont::ON) {
					setCharFont(bv()->buffer(),
						    cursor.par(),
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

	LyXFont realtmpfont = real_current_font;
	LyXFont rawtmpfont = current_font;
	// store the current font.  This is because of the use of cursor
	// movements. The moving cursor would refresh the current font

	// Get the font that is used to calculate the baselineskip
	pos_type const lastpos = cursor.par()->size();
	LyXFont rawparfont =
		cursor.par()->getFontSettings(bv()->buffer()->params,
					      lastpos - 1);

	bool jumped_over_space = false;

	if (!freeSpacing && IsLineSeparatorChar(c)) {
		if ((cursor.pos() > 0
		     && cursor.par()->isLineSeparator(cursor.pos() - 1))
		    || (cursor.pos() > 0
			&& cursor.par()->isNewline(cursor.pos() - 1))
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

	// the display inset stuff
	if (cursor.row()->pos() < cursor.row()->par()->size()
	    && cursor.row()->par()->isInset(cursor.row()->pos())) {
		Inset * inset = cursor.row()->par()->getInset(cursor.row()->pos());
		if (inset && (inset->display() || inset->needFullRow())) {
			// force a new break
			cursor.row()->fill(-1); // to force a new break
		}
	}

	// get the cursor row fist
	RowList::iterator row = cursor.row();
	int y = cursor.y() - row->baseline();
	if (c != Paragraph::META_INSET) {
		// Here case LyXText::InsertInset  already insertet the character
		cursor.par()->insertChar(cursor.pos(), c);
	}
	setCharFont(bv()->buffer(), cursor.par(), cursor.pos(), rawtmpfont);

	if (!jumped_over_space) {
		// refresh the positions
		RowList::iterator tmprow = row;
		while (boost::next(tmprow) != rows().end() &&
		       boost::next(tmprow)->par() == row->par()) {
			++tmprow;
			tmprow->pos(tmprow->pos() + 1);
		}
	}

	// Is there a break one row above
	if (row != rows().begin() &&
	    boost::prior(row)->par() == row->par()
	    && (cursor.par()->isLineSeparator(cursor.pos())
		|| cursor.par()->isNewline(cursor.pos())
		|| ((cursor.pos() + 1 < cursor.par()->size()) &&
		    cursor.par()->isInset(cursor.pos() + 1))
		|| cursor.row()->fill() == -1))
	{
		pos_type z = rowBreakPoint(*boost::prior(row));

		if (z >= row->pos()) {
			row->pos(z + 1);

			// set the dimensions of the row above
			boost::prior(row)->fill(fill(
						   boost::prior(row),
						   workWidth()));

			setHeightOfRow(boost::prior(row));

			y -= boost::prior(row)->height();

			postPaint(y);

			breakAgainOneRow(row);

			current_font = rawtmpfont;
			real_current_font = realtmpfont;
			setCursor(cursor.par(), cursor.pos() + 1,
				  false, cursor.boundary());
			// cursor MUST be in row now.

			if (boost::next(row) != rows().end() &&
			    boost::next(row)->par() == row->par())
				need_break_row = boost::next(row);
			else
				need_break_row = rows().end();

			// check, wether the last characters font has changed.
			if (cursor.pos() && cursor.pos() == cursor.par()->size()
			    && rawparfont != rawtmpfont)
				redoHeightOfParagraph();

			charInserted();
			return;
		}
	}

	// recalculate the fill of the row
	if (row->fill() >= 0) {
		// needed because a newline will set fill to -1. Otherwise
		// we would not get a rebreak!
		row->fill(fill(row, workWidth()));
	}

	if (c == Paragraph::META_INSET || row->fill() < 0) {
		postPaint(y);
		breakAgainOneRow(row);
		// will the cursor be in another row now?
		if (lastPos(*this, row) <= cursor.pos() + 1 &&
		    boost::next(row) != rows().end()) {
			if (boost::next(row) != rows().end() &&
			    boost::next(row)->par() == row->par())
				// this should always be true
				++row;
			breakAgainOneRow(row);
		}
		current_font = rawtmpfont;
		real_current_font = realtmpfont;

		setCursor(cursor.par(), cursor.pos() + 1, false,
			  cursor.boundary());
		if (isBoundary(bv()->buffer(), cursor.par(), cursor.pos())
		    != cursor.boundary())
			setCursor(cursor.par(), cursor.pos(), false,
			  !cursor.boundary());
		if (boost::next(row) != rows().end() &&
		    boost::next(row)->par() == row->par())
			need_break_row = boost::next(row);
		else
			need_break_row = rows().end();
	} else {
		// FIXME: similar code is duplicated all over - make resetHeightOfRow
		int const tmpheight = row->height();

		setHeightOfRow(row);

		if (tmpheight == row->height()) {
			postRowPaint(row, y);
		} else {
			postPaint(y);
		}

		current_font = rawtmpfont;
		real_current_font = realtmpfont;
		setCursor(cursor.par(), cursor.pos() + 1, false,
			  cursor.boundary());
	}

	// check, wether the last characters font has changed.
	if (cursor.pos() && cursor.pos() == cursor.par()->size()
	    && rawparfont != rawtmpfont) {
		redoHeightOfParagraph();
	} else {
		// now the special right address boxes
		if (cursor.par()->layout()->margintype
		    == MARGIN_RIGHT_ADDRESS_BOX) {
			redoDrawingOfParagraph(cursor);
		}
	}

	charInserted();
}


void LyXText::charInserted()
{
	// Here we could call FinishUndo for every 20 characters inserted.
	// This is from my experience how emacs does it.
	static unsigned int counter;
	if (counter < 20) {
		++counter;
	} else {
		finishUndo();
		counter = 0;
	}
}


void LyXText::prepareToPrint(RowList::iterator rit, float & x,
			     float & fill_separator,
			     float & fill_hfill,
			     float & fill_label_hfill,
			     bool bidi) const
{
	float nlh;

	float w = rit->fill();
	fill_hfill = 0;
	fill_label_hfill = 0;
	fill_separator = 0;
	fill_label_hfill = 0;

	bool const is_rtl =
		rit->par()->isRightToLeftPar(bv()->buffer()->params);
	if (is_rtl) {
		x = (workWidth() > 0)
			? rightMargin(*bv()->buffer(), *rit) : 0;
	} else
		x = (workWidth() > 0)
			? leftMargin(*rit) : 0;

	// is there a manual margin with a manual label
	LyXLayout_ptr const & layout = rit->par()->layout();

	if (layout->margintype == MARGIN_MANUAL
	    && layout->labeltype == LABEL_MANUAL) {
		/// We might have real hfills in the label part
		nlh = numberOfLabelHfills(*this, rit);

		// A manual label par (e.g. List) has an auto-hfill
		// between the label text and the body of the
		// paragraph too.
		// But we don't want to do this auto hfill if the par
		// is empty.
		if (!rit->par()->empty())
			++nlh;

		if (nlh && !rit->par()->getLabelWidthString().empty()) {
			fill_label_hfill = labelFill(*rit) / nlh;
		}
	}

	// are there any hfills in the row?
	float const nh = numberOfHfills(*this, rit);

	if (nh) {
		if (w > 0)
			fill_hfill = w / nh;
	// we don't have to look at the alignment if it is ALIGN_LEFT and
	// if the row is already larger then the permitted width as then
	// we force the LEFT_ALIGN'edness!
	} else if (static_cast<int>(rit->width()) < workWidth()) {
		// is it block, flushleft or flushright?
		// set x how you need it
		int align;
		if (rit->par()->params().align() == LYX_ALIGN_LAYOUT) {
			align = layout->align;
		} else {
			align = rit->par()->params().align();
		}

		// center displayed insets
		Inset * inset;
		if (rit->pos() < rit->par()->size()
		    && rit->par()->isInset(rit->pos())
		    && (inset = rit->par()->getInset(rit->pos()))
		    && (inset->display())) // || (inset->scroll() < 0)))
		    align = (inset->lyxCode() == Inset::MATHMACRO_CODE)
			? LYX_ALIGN_BLOCK : LYX_ALIGN_CENTER;
		// ERT insets should always be LEFT ALIGNED on screen
		inset = rit->par()->inInset();
		if (inset && inset->owner() &&
			inset->owner()->lyxCode() == Inset::ERT_CODE)
		{
			align = LYX_ALIGN_LEFT;
		}

		switch (align) {
	    case LYX_ALIGN_BLOCK:
	    {
			float const ns = numberOfSeparators(*this, rit);
			RowList::iterator next_row = boost::next(rit);

			if (ns && next_row != rowlist_.end() &&
			    next_row->par() == rit->par() &&
			    !(next_row->par()->isNewline(next_row->pos() - 1))
			    && !(next_row->par()->isInset(next_row->pos()) &&
				 next_row->par()->getInset(next_row->pos()) &&
				 next_row->par()->getInset(next_row->pos())->display())
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
	if (!bidi)
		return;

	computeBidiTables(bv()->buffer(), rit);
	if (is_rtl) {
		pos_type body_pos = rit->par()->beginningOfBody();
		pos_type last = lastPos(*this, rit);

		if (body_pos > 0 &&
		    (body_pos - 1 > last ||
		     !rit->par()->isLineSeparator(body_pos - 1))) {
			x += font_metrics::width(layout->labelsep,
					    getLabelFont(bv()->buffer(), rit->par()));
			if (body_pos - 1 <= last)
				x += fill_label_hfill;
		}
	}
}


// important for the screen


// the cursor set functions have a special mechanism. When they
// realize, that you left an empty paragraph, they will delete it.
// They also delete the corresponding row

void LyXText::cursorRightOneWord()
{
	// treat floats, HFills and Insets as words
	LyXCursor tmpcursor = cursor;
	// CHECK See comment on top of text.C

	if (tmpcursor.pos() == tmpcursor.par()->size()
	    && tmpcursor.par()->next()) {
			tmpcursor.par(tmpcursor.par()->next());
			tmpcursor.pos(0);
	} else {
		int steps = 0;

		// Skip through initial nonword stuff.
		while (tmpcursor.pos() < tmpcursor.par()->size() &&
		       ! tmpcursor.par()->isWord(tmpcursor.pos())) {
		  //    printf("Current pos1 %d", tmpcursor.pos()) ;
			tmpcursor.pos(tmpcursor.pos() + 1);
			++steps;
		}
		// Advance through word.
		while (tmpcursor.pos() < tmpcursor.par()->size() &&
			tmpcursor.par()->isWord(tmpcursor.pos())) {
		  //     printf("Current pos2 %d", tmpcursor.pos()) ;
			tmpcursor.pos(tmpcursor.pos() + 1);
			++steps;
		}
	}
	setCursor(tmpcursor.par(), tmpcursor.pos());
}


// Skip initial whitespace at end of word and move cursor to *start*
// of prior word, not to end of next prior word.
void LyXText::cursorLeftOneWord()
{
	LyXCursor tmpcursor = cursor;
	cursorLeftOneWord(tmpcursor);
	setCursor(tmpcursor.par(), tmpcursor.pos());
}


void LyXText::cursorLeftOneWord(LyXCursor & cur)
{
	// treat HFills, floats and Insets as words
	cur = cursor;
	while (cur.pos()
	       && (cur.par()->isSeparator(cur.pos() - 1)
		   || cur.par()->isKomma(cur.pos() - 1)
		   || cur.par()->isNewline(cur.pos() - 1))
	       && !(cur.par()->isHfill(cur.pos() - 1)
		    || cur.par()->isInset(cur.pos() - 1)))
		cur.pos(cur.pos() - 1);

	if (cur.pos()
	    && (cur.par()->isInset(cur.pos() - 1)
		|| cur.par()->isHfill(cur.pos() - 1))) {
		cur.pos(cur.pos() - 1);
	} else if (!cur.pos()) {
		if (cur.par()->previous()) {
			cur.par(cur.par()->previous());
			cur.pos(cur.par()->size());
		}
	} else {		// Here, cur != 0
		while (cur.pos() > 0 &&
		       cur.par()->isWord(cur.pos() - 1))
			cur.pos(cur.pos() - 1);
	}
}


// Select current word. This depends on behaviour of
// CursorLeftOneWord(), so it is patched as well.
void LyXText::getWord(LyXCursor & from, LyXCursor & to,
		      word_location const loc)
{
	// first put the cursor where we wana start to select the word
	from = cursor;
	switch (loc) {
	case WHOLE_WORD_STRICT:
		if (cursor.pos() == 0 || cursor.pos() == cursor.par()->size()
		    || cursor.par()->isSeparator(cursor.pos())
		    || cursor.par()->isKomma(cursor.pos())
		    || cursor.par()->isNewline(cursor.pos())
		    || cursor.par()->isSeparator(cursor.pos() - 1)
		    || cursor.par()->isKomma(cursor.pos() - 1)
		    || cursor.par()->isNewline(cursor.pos() - 1)) {
			to = from;
			return;
		}
		// no break here, we go to the next

	case WHOLE_WORD:
		// Move cursor to the beginning, when not already there.
		if (from.pos() && !from.par()->isSeparator(from.pos() - 1)
		    && !(from.par()->isKomma(from.pos() - 1)
			 || from.par()->isNewline(from.pos() - 1)))
			cursorLeftOneWord(from);
		break;
	case PREVIOUS_WORD:
		// always move the cursor to the beginning of previous word
		cursorLeftOneWord(from);
		break;
	case NEXT_WORD:
		lyxerr << "LyXText::getWord: NEXT_WORD not implemented yet\n";
		break;
	case PARTIAL_WORD:
		break;
	}
	to = from;
	while (to.pos() < to.par()->size()
	       && !to.par()->isSeparator(to.pos())
	       && !to.par()->isKomma(to.pos())
	       && !to.par()->isNewline(to.pos())
	       && !to.par()->isHfill(to.pos())
	       && !to.par()->isInset(to.pos()))
	{
		to.pos(to.pos() + 1);
	}
}


void LyXText::selectWord(word_location loc)
{
	LyXCursor from;
	LyXCursor to;
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
	if (!selection.set() && cursor.par()->size())
		return;

	bv()->hideCursor();

	if (selection.start.par() == selection.end.par()) {
		LyXCursor & startc = selection.start;
		LyXCursor & endc = selection.end;
		setUndo(bv(), Undo::INSERT, startc.par(), startc.par()->next());
		startc.par()->acceptChange(startc.pos(), endc.pos());
		finishUndo();
		clearSelection();
		redoParagraphs(startc, startc.par()->next());
		setCursorIntern(startc.par(), 0);
	}
#warning handle multi par selection
}


void LyXText::rejectChange()
{
	if (!selection.set() && cursor.par()->size())
		return;

	bv()->hideCursor();

	if (selection.start.par() == selection.end.par()) {
		LyXCursor & startc = selection.start;
		LyXCursor & endc = selection.end;
		setUndo(bv(), Undo::INSERT, startc.par(), startc.par()->next());
		startc.par()->rejectChange(startc.pos(), endc.pos());
		finishUndo();
		clearSelection();
		redoParagraphs(startc, startc.par()->next());
		setCursorIntern(startc.par(), 0);
	}
#warning handle multi par selection
}


// This function is only used by the spellchecker for NextWord().
// It doesn't handle LYX_ACCENTs and probably never will.
WordLangTuple const
LyXText::selectNextWordToSpellcheck(float & value)
{
	if (the_locking_inset) {
		WordLangTuple word = the_locking_inset->selectNextWordToSpellcheck(bv(), value);
		if (!word.word().empty()) {
			value += float(cursor.y());
			value /= float(height);
			return word;
		}
		// we have to go on checking so move cursor to the next char
		if (cursor.pos() == cursor.par()->size()) {
			if (!cursor.par()->next())
				return word;
			cursor.par(cursor.par()->next());
			cursor.pos(0);
		} else
			cursor.pos(cursor.pos() + 1);
	}
	Paragraph * tmppar = cursor.par();

	// If this is not the very first word, skip rest of
	// current word because we are probably in the middle
	// of a word if there is text here.
	if (cursor.pos() || cursor.par()->previous()) {
		while (cursor.pos() < cursor.par()->size()
		       && cursor.par()->isLetter(cursor.pos()))
			cursor.pos(cursor.pos() + 1);
	}

	// Now, skip until we have real text (will jump paragraphs)
	while (1) {
		Paragraph * cpar(cursor.par());
		pos_type const cpos(cursor.pos());

		if (cpos == cpar->size()) {
			if (cpar->next()) {
				cursor.par(cpar->next());
				cursor.pos(0);
				continue;
			}
			break;
		}

		bool const is_bad_inset(cpar->isInset(cpos)
			&& !cpar->getInset(cpos)->allowSpellcheck());

		if (cpar->isLetter(cpos) && !isDeletedText(*cpar, cpos)
			&& !is_bad_inset)
			break;

		cursor.pos(cpos + 1);
	}

	// now check if we hit an inset so it has to be a inset containing text!
	if (cursor.pos() < cursor.par()->size() &&
	    cursor.par()->isInset(cursor.pos())) {
		// lock the inset!
		cursor.par()->getInset(cursor.pos())->edit(bv());
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

	string lang_code(
		getFont(bv()->buffer(), cursor.par(), cursor.pos())
			.language()->code());
	// and find the end of the word (insets like optional hyphens
	// and ligature break are part of a word)
	while (cursor.pos() < cursor.par()->size()
	       && cursor.par()->isLetter(cursor.pos())
	       && !isDeletedText(*cursor.par(), cursor.pos()))
		cursor.pos(cursor.pos() + 1);

	// Finally, we copy the word to a string and return it
	string str;
	if (selection.cursor.pos() < cursor.pos()) {
		pos_type i;
		for (i = selection.cursor.pos(); i < cursor.pos(); ++i) {
			if (!cursor.par()->isInset(i))
				str += cursor.par()->getChar(i);
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
	while (cursor.pos() < cursor.par()->size()
	       && (cursor.par()->isLetter(cursor.pos())))
		cursor.pos(cursor.pos() + 1);

	setCursor(cursor.par(), cursor.pos());

	// finally set the selection
	setSelection();
}


// Delete from cursor up to the end of the current or next word.
void LyXText::deleteWordForward()
{
	if (cursor.par()->empty())
		cursorRight(bv());
	else {
		LyXCursor tmpcursor = cursor;
		tmpcursor.row(0); // ??
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
	if (cursor.par()->empty())
		cursorLeft(bv());
	else {
		LyXCursor tmpcursor = cursor;
		tmpcursor.row(0); // ??
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
	if (cursor.par()->empty())
		// Paragraph is empty, so we just go to the right
		cursorRight(bv());
	else {
		LyXCursor tmpcursor = cursor;
		// We can't store the row over a regular setCursor
		// so we set it to 0 and reset it afterwards.
		tmpcursor.row(0); // ??
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
		getWord(from, to, PARTIAL_WORD);
		setCursor(to.par(), to.pos() + 1);
	}

	lyx::Assert(from <= to);

	setUndo(bv(), Undo::FINISH, from.par(), to.par()->next());

	pos_type pos = from.pos();
	Paragraph * par = from.par();

	while (par && (pos != to.pos() || par != to.par())) {
		if (pos == par->size()) {
			par = par->next();
			pos = 0;
			continue;
		}
		unsigned char c = par->getChar(pos);
		if (!IsInsetChar(c)) {
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
		par->setChar(pos, c);
		checkParagraph(par, pos);

		++pos;
	}

	if (to.row() != from.row())
		postPaint(from.y() - from.row()->baseline());
}


void LyXText::Delete()
{
	// this is a very easy implementation

	LyXCursor old_cursor = cursor;
	int const old_cur_par_id = old_cursor.par()->id();
	int const old_cur_par_prev_id = old_cursor.par()->previous() ?
		old_cursor.par()->previous()->id() : -1;

	// just move to the right
	cursorRight(bv());

	// CHECK Look at the comment here.
	// This check is not very good...
	// The cursorRightIntern calls DeleteEmptyParagrapgMechanism
	// and that can very well delete the par or par->previous in
	// old_cursor. Will a solution where we compare paragraph id's
	//work better?
	if ((cursor.par()->previous() ? cursor.par()->previous()->id() : -1)
	    == old_cur_par_prev_id
	    && cursor.par()->id() != old_cur_par_id) {
		// delete-empty-paragraph-mechanism has done it
		return;
	}

	// if you had success make a backspace
	if (old_cursor.par() != cursor.par() || old_cursor.pos() != cursor.pos()) {
		LyXCursor tmpcursor = cursor;
		// to make sure undo gets the right cursor position
		cursor = old_cursor;
		setUndo(bv(), Undo::DELETE,
			cursor.par(), cursor.par()->next());
		cursor = tmpcursor;
		backspace();
	}
}


void LyXText::backspace()
{
	// Get the font that is used to calculate the baselineskip
	pos_type lastpos = cursor.par()->size();
	LyXFont rawparfont =
		cursor.par()->getFontSettings(bv()->buffer()->params,
					      lastpos - 1);

	if (cursor.pos() == 0) {
		// The cursor is at the beginning of a paragraph,
		// so the the backspace will collapse two paragraphs into one.

		// but it's not allowed unless it's new
		if (cursor.par()->isChangeEdited(0, cursor.par()->size()))
			return;

		// we may paste some paragraphs

		// is it an empty paragraph?

		if ((lastpos == 0
		     || (lastpos == 1 && cursor.par()->isSeparator(0)))) {
			// This is an empty paragraph and we delete it just by moving the cursor one step
			// left and let the DeleteEmptyParagraphMechanism handle the actual deletion
			// of the paragraph.

			if (cursor.par()->previous()) {
				Paragraph * tmppar = cursor.par()->previous();
				if (cursor.par()->layout() == tmppar->layout()
				    && cursor.par()->getAlign() == tmppar->getAlign()) {
					// Inherit bottom DTD from the paragraph below.
					// (the one we are deleting)
					tmppar->params().lineBottom(cursor.par()->params().lineBottom());
					tmppar->params().spaceBottom(cursor.par()->params().spaceBottom());
					tmppar->params().pagebreakBottom(cursor.par()->params().pagebreakBottom());
				}

				cursorLeft(bv());

				// the layout things can change the height of a row !
				int const tmpheight = cursor.row()->height();
				setHeightOfRow(cursor.row());
				if (cursor.row()->height() != tmpheight) {
					postPaint(cursor.y() - cursor.row()->baseline());
				}
				return;
			}
		}

		if (cursor.par()->previous()) {
			setUndo(bv(), Undo::DELETE,
				cursor.par()->previous(), cursor.par()->next());
		}

		Paragraph * tmppar = cursor.par();
		RowList::iterator tmprow = cursor.row();

		// We used to do cursorLeftIntern() here, but it is
		// not a good idea since it triggers the auto-delete
		// mechanism. So we do a cursorLeftIntern()-lite,
		// without the dreaded mechanism. (JMarc)
		if (cursor.par()->previous()) {
			// steps into the above paragraph.
			setCursorIntern(cursor.par()->previous(),
					cursor.par()->previous()->size(),
					false);
		}

		// Pasting is not allowed, if the paragraphs have different
		// layout. I think it is a real bug of all other
		// word processors to allow it. It confuses the user.
		// Even so with a footnote paragraph and a non-footnote
		// paragraph. I will not allow pasting in this case,
		// because the user would be confused if the footnote behaves
		// different wether it is open or closed.

		//	Correction: Pasting is always allowed with standard-layout
		LyXTextClass const & tclass =
			bv()->buffer()->params.getLyXTextClass();

		if (cursor.par() != tmppar
		    && (cursor.par()->layout() == tmppar->layout()
			|| tmppar->layout() == tclass.defaultLayout())
		    && cursor.par()->getAlign() == tmppar->getAlign()) {
			removeParagraph(tmprow);
			removeRow(tmprow);
			mergeParagraph(bv()->buffer()->params, bv()->buffer()->paragraphs, cursor.par());

			if (!cursor.pos() || !cursor.par()->isSeparator(cursor.pos() - 1))
				; //cursor.par()->insertChar(cursor.pos(), ' ');
			// strangely enough it seems that commenting out the line above removes
			// most or all of the segfaults. I will however also try to move the
			// two Remove... lines in front of the PasteParagraph too.
			else
				if (cursor.pos())
					cursor.pos(cursor.pos() - 1);

			postPaint(cursor.y() - cursor.row()->baseline());

			// remove the lost paragraph
			// This one is not safe, since the paragraph that the tmprow and the
			// following rows belong to has been deleted by the PasteParagraph
			// above. The question is... could this be moved in front of the
			// PasteParagraph?
			//RemoveParagraph(tmprow);
			//RemoveRow(tmprow);

			// This rebuilds the rows.
			appendParagraph(cursor.row());
			updateCounters();

			// the row may have changed, block, hfills etc.
			setCursor(cursor.par(), cursor.pos(), false);
		}
	} else {
		// this is the code for a normal backspace, not pasting
		// any paragraphs
		setUndo(bv(), Undo::DELETE,
			cursor.par(), cursor.par()->next());
		// We used to do cursorLeftIntern() here, but it is
		// not a good idea since it triggers the auto-delete
		// mechanism. So we do a cursorLeftIntern()-lite,
		// without the dreaded mechanism. (JMarc)
		setCursorIntern(cursor.par(), cursor.pos()- 1,
				false, cursor.boundary());

		if (cursor.par()->isInset(cursor.pos())) {
			// force complete redo when erasing display insets
			// this is a cruel method but safe..... Matthias
			if (cursor.par()->getInset(cursor.pos())->display() ||
			    cursor.par()->getInset(cursor.pos())->needFullRow()) {
				cursor.par()->erase(cursor.pos());
				redoParagraph();
				return;
			}
		}

		RowList::iterator row = cursor.row();
		int y = cursor.y() - row->baseline();
		pos_type z;
		// remember that a space at the end of a row doesnt count
		// when calculating the fill
		if (cursor.pos() < lastPos(*this, row) ||
		    !cursor.par()->isLineSeparator(cursor.pos())) {
			row->fill(row->fill() + singleWidth(
							    cursor.par(),
							    cursor.pos()));
		}

		// some special code when deleting a newline. This is similar
		// to the behavior when pasting paragraphs
		if (cursor.pos() && cursor.par()->isNewline(cursor.pos())) {
			cursor.par()->erase(cursor.pos());
			// refresh the positions
			RowList::iterator tmprow = row;
			while (boost::next(tmprow) != rows().end() &&
			       boost::next(tmprow)->par() == row->par()) {
				++tmprow;
				tmprow->pos(tmprow->pos() - 1);
			}
			if (cursor.par()->isLineSeparator(cursor.pos() - 1))
				cursor.pos(cursor.pos() - 1);

			if (cursor.pos() < cursor.par()->size()
			    && !cursor.par()->isSeparator(cursor.pos())) {
				cursor.par()->insertChar(cursor.pos(), ' ');
				setCharFont(bv()->buffer(), cursor.par(),
					    cursor.pos(), current_font);
				// refresh the positions
				tmprow = row;
				while (boost::next(tmprow) != rows().end() &&
				       boost::next(tmprow)->par() == row->par()) {
					++tmprow;
					tmprow->pos(tmprow->pos() + 1);
				}
			}
		} else {
			cursor.par()->erase(cursor.pos());

			// refresh the positions
			RowList::iterator tmprow = row;
			while (boost::next(tmprow) != rows().end() &&
			       boost::next(tmprow)->par() == row->par()) {
				++tmprow;
				tmprow->pos(tmprow->pos() - 1);
			}

			// delete newlines at the beginning of paragraphs
			while (!cursor.par()->empty() &&
			       cursor.pos() < cursor.par()->size() &&
			       cursor.par()->isNewline(cursor.pos()) &&
			       cursor.pos() == cursor.par()->beginningOfBody()) {
				cursor.par()->erase(cursor.pos());
				// refresh the positions
				tmprow = row;
				while (boost::next(tmprow) != rows().end() &&
				       boost::next(tmprow)->par() == row->par()) {
					++tmprow;
					tmprow->pos(tmprow->pos() - 1);
				}
			}
		}

		// is there a break one row above
		if (row != rows().begin() && boost::prior(row)->par() == row->par()) {
			z = rowBreakPoint(*boost::prior(row));
			if (z >= row->pos()) {
				row->pos(z + 1);

				RowList::iterator tmprow = boost::prior(row);

				// maybe the current row is now empty
				if (row->pos() >= row->par()->size()) {
					// remove it
					removeRow(row);
					need_break_row = rows().end();
				} else {
					breakAgainOneRow(row);
					if (boost::next(row) != rows().end() &&
					    boost::next(row)->par() == row->par())
						need_break_row = boost::next(row);
					else
						need_break_row = rows().end();
				}

				// set the dimensions of the row above
				y -= tmprow->height();
				tmprow->fill(fill(tmprow, workWidth()));
				setHeightOfRow(tmprow);

				postPaint(y);

				setCursor(cursor.par(), cursor.pos(),
					  false, cursor.boundary());
				//current_font = rawtmpfont;
				//real_current_font = realtmpfont;
				// check, whether the last character's font has changed.
				if (rawparfont !=
				    cursor.par()->getFontSettings(bv()->buffer()->params,
								  cursor.par()->size() - 1))
					redoHeightOfParagraph();
				return;
			}
		}

		// break the cursor row again
		if (boost::next(row) != rows().end() &&
		    boost::next(row)->par() == row->par() &&
		    (lastPos(*this, row) == row->par()->size() - 1 ||
		     rowBreakPoint(*row) != lastPos(*this, row))) {

			// it can happen that a paragraph loses one row
			// without a real breakup. This is when a word
			// is to long to be broken. Well, I don t care this
			// hack ;-)
			if (lastPos(*this, row) == row->par()->size() - 1)
				removeRow(boost::next(row));

			postPaint(y);

			breakAgainOneRow(row);
			// will the cursor be in another row now?
			if (boost::next(row) != rows().end() &&
			    boost::next(row)->par() == row->par() &&
			    lastPos(*this, row) <= cursor.pos()) {
				++row;
				breakAgainOneRow(row);
			}

			setCursor(cursor.par(), cursor.pos(), false, cursor.boundary());

			if (boost::next(row) != rows().end() &&
			    boost::next(row)->par() == row->par())
				need_break_row = boost::next(row);
			else
				need_break_row = rows().end();
		} else  {
			// set the dimensions of the row
			row->fill(fill(row, workWidth()));
			int const tmpheight = row->height();
			setHeightOfRow(row);
			if (tmpheight == row->height()) {
				postRowPaint(row, y);
			} else {
				postPaint(y);
			}
			setCursor(cursor.par(), cursor.pos(), false, cursor.boundary());
		}
	}

	// current_font = rawtmpfont;
	// real_current_font = realtmpfont;

	if (isBoundary(bv()->buffer(), cursor.par(), cursor.pos())
	    != cursor.boundary())
		setCursor(cursor.par(), cursor.pos(), false,
			  !cursor.boundary());

	lastpos = cursor.par()->size();
	if (cursor.pos() == lastpos)
		setCurrentFont();

	// check, whether the last characters font has changed.
	if (rawparfont !=
	    cursor.par()->getFontSettings(bv()->buffer()->params, lastpos - 1)) {
		redoHeightOfParagraph();
	} else {
		// now the special right address boxes
		if (cursor.par()->layout()->margintype
		    == MARGIN_RIGHT_ADDRESS_BOX) {
			redoDrawingOfParagraph(cursor);
		}
	}
}


// returns pointer to a specified row
RowList::iterator
LyXText::getRow(Paragraph * par, pos_type pos, int & y) const
{
	y = 0;

	if (rows().empty())
		return rowlist_.end();

	// find the first row of the specified paragraph
	RowList::iterator rit = rowlist_.begin();
	RowList::iterator end = rowlist_.end();
	while (boost::next(rit) != end && rit->par() != par) {
		y += rit->height();
		++rit;
	}

	// now find the wanted row
	while (rit->pos() < pos
	       && boost::next(rit) != end
	       && boost::next(rit)->par() == par
	       && boost::next(rit)->pos() <= pos) {
		y += rit->height();
		++rit;
	}

	return rit;
}


RowList::iterator LyXText::getRowNearY(int & y) const
{
	// If possible we should optimize this method. (Lgb)
	int tmpy = 0;

	RowList::iterator rit = rowlist_.begin();
	RowList::iterator end = rowlist_.end();

	while (rit != end && boost::next(rit) != end && tmpy + rit->height() <= y) {
		tmpy += rit->height();
		++rit;
	}

	// return the real y
	y = tmpy;

	return rit;
}


int LyXText::getDepth() const
{
	return cursor.par()->getDepth();
}
