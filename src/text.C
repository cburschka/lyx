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
#include "layout.h"
#include "paragraph.h"
#include "lyx_gui_misc.h"
#include "gettext.h"
#include "bufferparams.h"
#include "buffer.h"
#include "debug.h"
#include "lyxrc.h"
#include "LyXView.h"
#include "Painter.h"
#include "tracer.h"
#include "font.h"
#include "encoding.h"
#include "lyxscreen.h"
#include "bufferview_funcs.h"
#include "BufferView.h"
#include "language.h"
#include "ParagraphParameters.h"
#include "undo_funcs.h"
#include "font.h"

#include "insets/insetbib.h"
#include "insets/insettext.h"

#include "support/textutils.h"
#include "support/LAssert.h"
#include "support/lstrings.h"

#include <algorithm>

using std::max;
using std::min;
using std::endl;
using std::pair;

namespace {

int const LYX_PAPER_MARGIN = 20;

} // namespace anon

extern int bibitemMaxWidth(BufferView *, LyXFont const &);


int LyXText::workWidth(BufferView * bview) const
{
	if (inset_owner) {
		return inset_owner->textWidth(bview);
	}
	return bview->workWidth();
}


int LyXText::workWidth(BufferView * bview, Inset * inset) const
{
	Paragraph * par = 0;
	pos_type pos = 0;

	Buffer::inset_iterator it = bview->buffer()->inset_iterator_begin();

	for (; it != bview->buffer()->inset_iterator_end(); ++it) {
		if (*it == inset) {
			par = it.getPar();
			pos = it.getPos();
			break;
		}
	}
	if (!par) {
		return workWidth(bview);
	}
	
	LyXLayout const & layout =
		textclasslist.Style(bview->buffer()->params.textclass,
				    par->getLayout());

	if (layout.margintype != MARGIN_RIGHT_ADDRESS_BOX) {
		// Optimization here: in most cases, the real row is
		// not needed, but only the par/pos values. So we just
		// construct a dummy row for leftMargin. (JMarc)
		Row dummyrow;
		dummyrow.par(par);
		dummyrow.pos(pos);
		return workWidth(bview) - leftMargin(bview, &dummyrow);
	} else {
		int dummy_y;
		Row * row = getRow(par, pos, dummy_y);
		Row * frow = row;
		while(frow->previous() && frow->par() == frow->previous()->par())
			frow = frow->previous();
		unsigned int maxw = 0;
		while(frow->next() && frow->par() == frow->next()->par()) {
			if ((frow != row) && (maxw < frow->width()))
				maxw = frow->width();
			frow = frow->next();
		}
		if (maxw)
			return maxw;
	}
	return workWidth(bview);
}


int LyXText::getRealCursorX(BufferView * bview) const
{
	int x = cursor.x();
	if (the_locking_inset && (the_locking_inset->getLyXText(bview)!=this))
		x = the_locking_inset->getLyXText(bview)->getRealCursorX(bview);
	return x;
}


unsigned char LyXText::transformChar(unsigned char c, Paragraph * par,
			pos_type pos) const
{
	if (!Encodings::is_arabic(c))
		if (lyxrc.font_norm_type == LyXRC::ISO_8859_6_8 && isdigit(c))
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
		if (Encodings::is_arabic(prev_char))
			return Encodings::TransformChar(c, Encodings::FORM_MEDIAL);
		else
			return Encodings::TransformChar(c, Encodings::FORM_INITIAL);
	} else {
		if (Encodings::is_arabic(prev_char))
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

int LyXText::singleWidth(BufferView * bview, Paragraph * par,
			 pos_type pos) const
{
	char const c = par->getChar(pos);
	return singleWidth(bview, par, pos, c);
}


int LyXText::singleWidth(BufferView * bview, Paragraph * par,
                         pos_type pos, char c) const
{
	LyXFont const font = getFont(bview->buffer(), par, pos);

	// The most common case is handled first (Asger)
	if (IsPrintable(c)) {
		if (font.language()->RightToLeft()) {
			if (font.language()->lang() == "arabic" &&
			    (lyxrc.font_norm_type == LyXRC::ISO_8859_6_8 ||
			     lyxrc.font_norm_type == LyXRC::ISO_10646_1))
				if (Encodings::IsComposeChar_arabic(c))
					return 0;
				else
					c = transformChar(c, par, pos);
			else if (font.language()->lang() == "hebrew" &&
				 Encodings::IsComposeChar_hebrew(c))
				return 0;
		}
		return lyxfont::width(c, font);

	} else if (IsHfillChar(c)) {
		return 3;	/* Because of the representation
				 * as vertical lines */
	} else if (c == Paragraph::META_INSET) {
		Inset * tmpinset = par->getInset(pos);
		if (tmpinset) {
#if 0 // seems not to be needed, but ...
			tmpinset->update(bview, font);
#endif
			return tmpinset->width(bview, font);
		} else
			return 0;

	} else if (IsSeparatorChar(c))
		c = ' ';
	else if (IsNewlineChar(c))
		c = 'n';
	return lyxfont::width(c, font);
}


// Returns the paragraph position of the last character in the specified row
LyXText::pos_type LyXText::rowLast(Row const * row) const
{
	if (row->next() == 0)
		return row->par()->size() - 1;
	else if (row->next()->par() != row->par()) 
		return row->par()->size() - 1;
	else 
		return row->next()->pos() - 1;
}


LyXText::pos_type LyXText::rowLastPrintable(Row const * row) const
{
	pos_type const last = rowLast(row);
	if (last >= row->pos()
	    && row->next()
	    && row->next()->par() == row->par()
	    && row->par()->isSeparator(last))
		return last - 1;
	else
		return last;
}


void LyXText::computeBidiTables(Buffer const * buf, Row * row) const
{
	bidi_same_direction = true;
	if (!lyxrc.rtl_support) {
		bidi_start = -1;
		return;
	}

	bidi_start = row->pos();
	bidi_end = rowLastPrintable(row);

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
		row->par()->getParLanguage(buf->params)->RightToLeft();
	int level = 0;
	bool rtl = false;
	bool rtl0 = false;
	pos_type const main_body = beginningOfMainBody(buf, row->par());

	for (pos_type lpos = bidi_start;
	     lpos <= bidi_end; ++lpos) {
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

		if (lpos == main_body - 1
		    && row->pos() < main_body - 1
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


void LyXText::draw(BufferView * bview, Row const * row,
                   pos_type & vpos, int offset, float & x, bool cleared)
{
	Painter & pain = bview->painter();
	
	pos_type pos = vis2log(vpos);
	char c = row->par()->getChar(pos);
	float tmpx = x;

	if (IsNewlineChar(c)) {
		++vpos;
		// Draw end-of-line marker
		LyXFont const font = getFont(bview->buffer(), row->par(), pos);
		int const wid = lyxfont::width('n', font);
		int const asc = lyxfont::maxAscent(font);
		int const y = offset + row->baseline();
		int xp[3];
		int yp[3];
		
		if (bidi_level(pos) % 2 == 0) {
			xp[0] = int(x + wid * 0.375);
			yp[0] = int(y - 0.875 * asc * 0.75);
			
			xp[1] = int(x);
			yp[1] = int(y - 0.500 * asc * 0.75);
			
			xp[2] = int(x + wid * 0.375);
			yp[2] = int(y - 0.125 * asc * 0.75);
			
			pain.lines(xp, yp, 3, LColor::eolmarker);
			
			xp[0] = int(x);
			yp[0] = int(y - 0.500 * asc * 0.75);
			
			xp[1] = int(x + wid);
			yp[1] = int(y - 0.500 * asc * 0.75);
			
			xp[2] = int(x + wid);
			yp[2] = int(y - asc * 0.75);
			
			pain.lines(xp, yp, 3, LColor::eolmarker);
		} else {
			xp[0] = int(x + wid * 0.625);
			yp[0] = int(y - 0.875 * asc * 0.75);
			
			xp[1] = int(x + wid);
			yp[1] = int(y - 0.500 * asc * 0.75);
			
			xp[2] = int(x + wid * 0.625);
			yp[2] = int(y - 0.125 * asc * 0.75);
			
			pain.lines(xp, yp, 3, LColor::eolmarker);
			
			xp[0] = int(x + wid);
			yp[0] = int(y - 0.500 * asc * 0.75);
			
			xp[1] = int(x);
			yp[1] = int(y - 0.500 * asc * 0.75);
			
			xp[2] = int(x);
			yp[2] = int(y - asc * 0.75);
			
			pain.lines(xp, yp, 3, LColor::eolmarker);
		}
		x += wid;
		return;
	}

	LyXFont font = getFont(bview->buffer(), row->par(), pos);
	LyXFont font2 = font;

	if (c == Paragraph::META_INSET) {
		Inset * tmpinset = row->par()->getInset(pos);
		if (tmpinset) {
			tmpinset->update(bview, font, false);
			tmpinset->draw(bview, font, offset+row->baseline(), x,
			               cleared);
			if (!need_break_row && !inset_owner &&
			    bview->text->status() == CHANGED_IN_DRAW)
			{
				if (row->previous() && row->previous()->par() == row->par())
					breakAgainOneRow(bview, row->previous());
				setCursor(bview, cursor.par(), cursor.pos());
				need_break_row = const_cast<Row *>(row);
			}
		}
		++vpos;

		if (lyxrc.mark_foreign_language &&
			font.language() != latex_language &&
		    font.language() != bview->buffer()->params.language) {
			int const y = offset + row->height() - 1;
			pain.line(int(tmpx), y, int(x), y, LColor::language);
		}

		return;
	}

	// usual characters, no insets

	// Collect character that we can draw in one command

	// This is dirty, but fast. Notice that it will never be too small.
	// For the record, I'll note that Microsoft Word has a limit
	// of 768 here. We have none :-) (Asger)
	// Ok. I am the first to admit that the use of std::string will be
	// a tiny bit slower than using a POD char array. However, I claim
	// that this slowdown is so small that it is close to inperceptive.
	// So IMHO we should go with the easier and clearer implementation.
	// And even if 1024 is a large number here it might overflow, string
	// will only overflow if the machine is out of memory...
	static string textstring;
	textstring = c;
	++vpos;

	pos_type const last = rowLastPrintable(row);

	if (font.language()->lang() == "hebrew") {
		if (Encodings::IsComposeChar_hebrew(c)) {
			int const width = lyxfont::width(c, font2);
			int dx = 0;
			for (pos_type i = pos-1; i >= 0; --i) {
				c = row->par()->getChar(i);
				if (!Encodings::IsComposeChar_hebrew(c)) {
					if (IsPrintableNonspace(c)) {
						int const width2 =
							singleWidth(bview,
								    row->par(),
								    i, c);
						dx = (c == 'ø' || c == 'ã') // dalet / resh
							? width2 - width : (width2 - width) / 2;
					}
					break;
				}
			}
			// Draw nikud
			pain.text(int(x) + dx, offset + row->baseline(),
				  textstring, font);
		} else {
			while (vpos <= last &&
			       (pos = vis2log(vpos)) >= 0
			       && IsPrintableNonspace(c = row->par()->getChar(pos))
			       && !Encodings::IsComposeChar_hebrew(c)
			       && font2 == getFont(bview->buffer(), row->par(), pos)) {
				textstring += c;
				++vpos;
			}
			// Draw text and set the new x position
			pain.text(int(x), offset + row->baseline(),
				  textstring, font);
			x += lyxfont::width(textstring, font);
		}
	} else if (font.language()->lang() == "arabic" &&
		   (lyxrc.font_norm_type == LyXRC::ISO_8859_6_8 ||
		    lyxrc.font_norm_type == LyXRC::ISO_10646_1)) {
		if (Encodings::IsComposeChar_arabic(c)) {
			c = transformChar(c, row->par(), pos);
			textstring = c;
			int const width = lyxfont::width(c, font2);
			int dx = 0;
			for (pos_type i = pos-1; i >= 0; --i) {
				c = row->par()->getChar(i);
				if (!Encodings::IsComposeChar_arabic(c)) {
					if (IsPrintableNonspace(c)) {
						int const width2 =
							singleWidth(bview,
								    row->par(),
								    i, c);
						dx = (width2 - width) / 2;
					}
					break;
				}
			}
			// Draw nikud
			pain.text(int(x) + dx, offset + row->baseline(), 
				  textstring, font);
		} else {
			textstring = transformChar(c, row->par(), pos);
			while (vpos <= last &&
			       (pos = vis2log(vpos)) >= 0
			       && IsPrintableNonspace(c = row->par()->getChar(pos))
			       && !Encodings::IsComposeChar_arabic(c)
			       && font2 == getFont(bview->buffer(), row->par(), pos)) {
				c = transformChar(c, row->par(), pos);
				textstring += c;
				++vpos;
			}
			// Draw text and set the new x position
			pain.text(int(x), offset + row->baseline(),
				  textstring, font);
			x += lyxfont::width(textstring, font);
		}
	} else {
		while (vpos <= last &&
		       (pos = vis2log(vpos)) >= 0
		       && IsPrintableNonspace(c = row->par()->getChar(pos))
		       && font2 == getFont(bview->buffer(), row->par(), pos)) {
			textstring += c;
			++vpos;
		}
		// Draw text and set the new x position
		pain.text(int(x), offset + row->baseline(), textstring, font);
		x += lyxfont::width(textstring, font);
	}

#ifdef INHERIT_LANGUAGE
#ifdef WITH_WARNINGS
	if ((font.language() == inherit_language) ||
		(font.language() == ignore_language))
		lyxerr << "No this shouldn't happen!\n";
#endif
#endif
	if (lyxrc.mark_foreign_language &&
	    font.language() != latex_language &&
	    font.language() != bview->buffer()->params.language) {
		int const y = offset + row->height() - 1;
		pain.line(int(tmpx), y, int(x), y,
			  LColor::language);
	}

	// If we want ulem.sty support, drawing
	// routines should go here. (Asger)
	// Why shouldn't LyXFont::drawText handle it internally?
}


// Returns the left beginning of the text. 
// This information cannot be taken from the layouts-objekt, because in 
// LaTeX the beginning of the text fits in some cases (for example sections)
// exactly the label-width.
int LyXText::leftMargin(BufferView * bview, Row const * row) const
{
	LyXTextClass const & tclass =
		textclasslist.TextClass(bview->buffer()->params.textclass);
	LyXLayout const & layout = tclass[row->par()->getLayout()];
	
	string parindent = layout.parindent; 

	int x = LYX_PAPER_MARGIN;
	
	x += lyxfont::signedWidth(tclass.leftmargin(), tclass.defaultfont());

	// this is the way, LyX handles the LaTeX-Environments.
	// I have had this idea very late, so it seems to be a
	// later added hack and this is true
	if (!row->par()->getDepth()) {
		if (!row->par()->getLayout()) {
			// find the previous same level paragraph
			if (row->par()->previous()) {
				Paragraph * newpar = row->par()
					->depthHook(row->par()->getDepth());
				if (newpar &&
				    tclass[newpar->getLayout()].nextnoindent)
					parindent.erase();
			}
		}
	} else {
		// find the next level paragraph
		
		Paragraph * newpar =
			row->par()->outerHook();
		
		// make a corresponding row. Needed to call LeftMargin()
		
		// check wether it is a sufficent paragraph 
		if (newpar && tclass[newpar->getLayout()].isEnvironment())
		{
			Row dummyrow;
			dummyrow.par(newpar);
			dummyrow.pos(newpar->size());
			x = leftMargin(bview, &dummyrow);
		} else {
			// this is no longer an error, because this function
			// is used to clear impossible depths after changing
			// a layout. Since there is always a redo,
			// LeftMargin() is always called
			row->par()->params().depth(0);
		}
		
		if (newpar && !row->par()->getLayout()) {
			if (newpar->params().noindent())
				parindent.erase();
			else
				parindent = tclass[newpar->getLayout()].parindent;
		}
		
	}
	
	LyXFont const labelfont = getLabelFont(bview->buffer(), row->par());
	switch (layout.margintype) {
	case MARGIN_DYNAMIC:
		if (!layout.leftmargin.empty()) {
			x += lyxfont::signedWidth(layout.leftmargin,
						  tclass.defaultfont());
		}
		if (!row->par()->getLabelstring().empty()) {
			x += lyxfont::signedWidth(layout.labelindent,
						  labelfont);
			x += lyxfont::width(row->par()->getLabelstring(),
					    labelfont);
			x += lyxfont::width(layout.labelsep, labelfont);
		}
		break;
	case MARGIN_MANUAL:
		x += lyxfont::signedWidth(layout.labelindent, labelfont);
		if (row->pos() >= beginningOfMainBody(bview->buffer(), row->par())) {
			if (!row->par()->getLabelWidthString().empty()) {
				x += lyxfont::width(row->par()->getLabelWidthString(),
					       labelfont);
				x += lyxfont::width(layout.labelsep, labelfont);
			}
		}
		break;
	case MARGIN_STATIC:
		x += lyxfont::signedWidth(layout.leftmargin, tclass.defaultfont()) * 4
			/ (row->par()->getDepth() + 4);
		break;
	case MARGIN_FIRST_DYNAMIC:
		if (layout.labeltype == LABEL_MANUAL) {
			if (row->pos() >= beginningOfMainBody(bview->buffer(), row->par())) {
				x += lyxfont::signedWidth(layout.leftmargin,
							  labelfont);
			} else {
				x += lyxfont::signedWidth(layout.labelindent,
							  labelfont);
			}
		} else if (row->pos()
			   // Special case to fix problems with
			   // theorems (JMarc)
			   || (layout.labeltype == LABEL_STATIC
			       && layout.latextype == LATEX_ENVIRONMENT
			       && ! row->par()->isFirstInSequence())) {
			x += lyxfont::signedWidth(layout.leftmargin,
						  labelfont);
		} else if (layout.labeltype != LABEL_TOP_ENVIRONMENT
			   && layout.labeltype != LABEL_BIBLIO
			   && layout.labeltype !=
			   LABEL_CENTERED_TOP_ENVIRONMENT) {
			x += lyxfont::signedWidth(layout.labelindent,
						  labelfont);
			x += lyxfont::width(layout.labelsep, labelfont);
			x += lyxfont::width(row->par()->getLabelstring(),
					    labelfont);
		} 
		break;
		
	case MARGIN_RIGHT_ADDRESS_BOX:
	{
		// ok, a terrible hack. The left margin depends on the widest
		// row in this paragraph. Do not care about footnotes, they
		// are *NOT* allowed in the LaTeX realisation of this layout.
		
		// find the first row of this paragraph
		Row const * tmprow = row;
		while (tmprow->previous()
		       && tmprow->previous()->par() == row->par())
			tmprow = tmprow->previous();
		
		int minfill = tmprow->fill();
		while (tmprow->next() && tmprow->next()->par() == row->par()) {
			tmprow = tmprow->next();
			if (tmprow->fill() < minfill)
				minfill = tmprow->fill();
		}
		
		x += lyxfont::signedWidth(layout.leftmargin,
					  tclass.defaultfont());
		x += minfill;
	}
	break;
	}
	
	LyXAlignment align; // wrong type

	if (row->par()->params().align() == LYX_ALIGN_LAYOUT)
		align = layout.align;
	else
		align = row->par()->params().align();

	// set the correct parindent
	if (row->pos() == 0) {
		if ((layout.labeltype == LABEL_NO_LABEL 
		     || layout.labeltype == LABEL_TOP_ENVIRONMENT 
		     || layout.labeltype == LABEL_CENTERED_TOP_ENVIRONMENT
		     || (layout.labeltype == LABEL_STATIC
			 && layout.latextype == LATEX_ENVIRONMENT
			 && ! row->par()->isFirstInSequence()))
		    && align == LYX_ALIGN_BLOCK
		    && !row->par()->params().noindent()
		    && (row->par()->layout ||
			bview->buffer()->params.paragraph_separation ==
			BufferParams::PARSEP_INDENT))
			x += lyxfont::signedWidth(parindent,
						  tclass.defaultfont());
		else if (layout.labeltype == LABEL_BIBLIO) {
			// ale970405 Right width for bibitems
			x += bibitemMaxWidth(bview, tclass.defaultfont());
		}
	}
	return x;
}


int LyXText::rightMargin(Buffer const * buf, Row const * row) const
{
	LyXTextClass const & tclass =
		textclasslist.TextClass(buf->params.textclass);
	LyXLayout const & layout = tclass[row->par()->getLayout()];
		
	int x = LYX_PAPER_MARGIN
		+ lyxfont::signedWidth(tclass.rightmargin(),
				       tclass.defaultfont());

	// this is the way, LyX handles the LaTeX-Environments.
	// I have had this idea very late, so it seems to be a
	// later added hack and this is true
	if (row->par()->getDepth()) {
		// find the next level paragraph
		
		Paragraph * newpar = row->par();
		
		do {
			newpar = newpar->previous();
		} while (newpar
			 && newpar->getDepth() >= row->par()->getDepth());
		
		// make a corresponding row. Needed to call LeftMargin()
		
		// check wether it is a sufficent paragraph
		if (newpar
		    && tclass[newpar->getLayout()].isEnvironment()) {
			Row dummyrow;
			dummyrow.par(newpar);
			dummyrow.pos(0);
			x = rightMargin(buf, &dummyrow);
		} else {
			// this is no longer an error, because this function
			// is used to clear impossible depths after changing
			// a layout. Since there is always a redo,
			// LeftMargin() is always called
			row->par()->params().depth(0);
		}
	}
	
	//lyxerr << "rightmargin: " << layout->rightmargin << endl;
	x += lyxfont::signedWidth(layout.rightmargin, tclass.defaultfont())
		* 4 / (row->par()->getDepth() + 4);
	return x;
}


int LyXText::labelEnd(BufferView * bview, Row const * row) const
{
	if (textclasslist.Style(bview->buffer()->params.textclass,
				row->par()->getLayout()).margintype
	    == MARGIN_MANUAL) {
		Row tmprow;
		tmprow = *row;
		tmprow.pos(row->par()->size());
		return leftMargin(bview, &tmprow);  /* just the beginning 
						of the main body */
	} else
		return 0;  /* LabelEnd is only needed, if the  
			      layout fills a flushleft
			      label. */
}


// get the next breakpoint in a given paragraph
LyXText::pos_type
LyXText::nextBreakPoint(BufferView * bview, Row const * row, int width) const
{
	Paragraph * par = row->par();

	if (width < 0)
		return par->size();

	pos_type const pos = row->pos();


	// position of the last possible breakpoint 
	// -1 isn't a suitable value, but a flag
	pos_type last_separator = -1;
	width -= rightMargin(bview->buffer(), row);
	
	pos_type const main_body =
		beginningOfMainBody(bview->buffer(), par);
	LyXLayout const & layout =
		textclasslist.Style(bview->buffer()->params.textclass,
				    par->getLayout());
	pos_type i = pos;

	if (layout.margintype == MARGIN_RIGHT_ADDRESS_BOX) {
		/* special code for right address boxes, only newlines count */
		while (i < par->size()) {
			if (par->isNewline(i)) {
				last_separator = i;
				i = par->size() - 1; // this means break
				//x = width;
			} else if (par->isInset(i) && par->getInset(i) 
				&& par->getInset(i)->display()) {
				par->getInset(i)->display(false);
			}
			++i;
		}
	} else {
		// Last position is an invariant
		pos_type const last = 
			par->size();
		// this is the usual handling
		int x = leftMargin(bview, row);
		bool doitonetime = true;
		while (doitonetime || ((x < width) && (i < last))) {
			doitonetime = false;
			char const c = par->getChar(i);
			if (IsNewlineChar(c)) {
				last_separator = i;
				x = width; // this means break
			} else if (c == Paragraph::META_INSET &&
				   par->getInset(i)) {
				
				// check wether a Display() inset is
				// valid here. if not, change it to
				// non-display
				if (par->getInset(i)->display() &&
				    (layout.isCommand() ||
				     (layout.labeltype == LABEL_MANUAL
				      && i < beginningOfMainBody(bview->buffer(), par)))) {
					// display istn't allowd
					par->getInset(i)->display(false);
					x += singleWidth(bview, par, i, c);
				} else if (par->getInset(i)->display() ||
					 par->getInset(i)->needFullRow()) {
					// So break the line here
					if (i == pos) {
						if (pos < last-1) {
							last_separator = i;
							if (IsLineSeparatorChar(par->getChar(i+1)))
								++last_separator;
						} else
							last_separator = last; // to avoid extra rows
					} else
						last_separator = i - 1;
					x = width;  // this means break
				} else {
					x += singleWidth(bview, par, i, c);
				}
			} else  {
				if (IsLineSeparatorChar(c))
					last_separator = i;
				x += singleWidth(bview, par, i, c);
			}
			++i;
			if (i == main_body) {
				x += lyxfont::width(layout.labelsep,
						    getLabelFont(bview->buffer(), par));
				if (par->isLineSeparator(i - 1))
					x-= singleWidth(bview, par, i - 1);
				int left_margin = labelEnd(bview, row);
				if (x < left_margin)
					x = left_margin;
			}
		}
		// end of paragraph is always a suitable separator
		if (i == last && x < width)
			last_separator = i;
	}
	
	// well, if last_separator is still 0, the line isn't breakable. 
	// don't care and cut simply at the end
	if (last_separator < 0) {
		last_separator = i;
	}
	
	// manual labels cannot be broken in LaTeX, do not care
	if (main_body && last_separator < main_body)
		last_separator = main_body - 1;
	
	return last_separator;
}


// returns the minimum space a row needs on the screen in pixel
int LyXText::fill(BufferView * bview, Row * row, int paper_width) const
{
	if (paper_width < 0)
		return 0;

	int w;
	// get the pure distance
	pos_type const last = rowLastPrintable(row);
	
	// special handling of the right address boxes
	if (textclasslist.Style(bview->buffer()->params.textclass,
				row->par()->getLayout()).margintype
	    == MARGIN_RIGHT_ADDRESS_BOX)
	{
		int const tmpfill = row->fill();
		row->fill(0); // the minfill in MarginLeft()
		w = leftMargin(bview, row);
		row->fill(tmpfill);
	} else
		w = leftMargin(bview, row);
	
	LyXLayout const & layout = textclasslist.Style(bview->buffer()->params.textclass,
						       row->par()->getLayout());
	pos_type const main_body = 
		beginningOfMainBody(bview->buffer(), row->par());
	pos_type i = row->pos();

	while (i <= last) {
		if (main_body > 0 && i == main_body) {
			w += lyxfont::width(layout.labelsep, getLabelFont(bview->buffer(), row->par()));
			if (row->par()->isLineSeparator(i - 1))
				w -= singleWidth(bview, row->par(), i - 1);
			int left_margin = labelEnd(bview, row);
			if (w < left_margin)
				w = left_margin;
		}
		w += singleWidth(bview, row->par(), i);
		++i;
	}
	if (main_body > 0 && main_body > last) {
		w += lyxfont::width(layout.labelsep, getLabelFont(bview->buffer(), row->par()));
		if (last >= 0 && row->par()->isLineSeparator(last))
			w -= singleWidth(bview, row->par(), last);
		int const left_margin = labelEnd(bview, row);
		if (w < left_margin)
			w = left_margin;
	}
	
	int const fill = paper_width - w - rightMargin(bview->buffer(), row);
#ifdef WITH_WARNINGS
#warning Please fix me (Jug!)
#endif
#if 0
	if (fill < 0)
		return 0;
#endif
	return fill;
}


// returns the minimum space a manual label needs on the screen in pixel
int LyXText::labelFill(BufferView * bview, Row const * row) const
{
	pos_type last = beginningOfMainBody(bview->buffer(), row->par()) - 1;
	// -1 because a label ends either with a space that is in the label, 
	// or with the beginning of a footnote that is outside the label.

	// I don't understand this code in depth, but sometimes "last" is
	// less than 0 and this causes a crash. This fix seems to work
	// correctly, but I bet the real error is elsewhere.  The bug is
	// triggered when you have an open footnote in a paragraph
	// environment with a manual label. (Asger)
	if (last < 0) last = 0;
	
	if (row->par()->isLineSeparator(last)) /* a sepearator at this end 
						does not count */
		--last;
	
	int w = 0;
	pos_type i = row->pos();
	while (i <= last) {
		w += singleWidth(bview, row->par(), i);
		++i;
	}
	
	int fill = 0;
	if (!row->par()->params().labelWidthString().empty()) {
		fill = max(lyxfont::width(row->par()->params().labelWidthString(),
					  getLabelFont(bview->buffer(), row->par())) - w,
			   0);
	}
	
	return fill;
}


// returns the number of separators in the specified row. The separator 
// on the very last column doesnt count
int LyXText::numberOfSeparators(Buffer const * buf, Row const * row) const
{
	pos_type const last = rowLast(row);
	pos_type p = max(row->pos(), beginningOfMainBody(buf, row->par()));
	int n = 0;
	for (; p < last; ++p) {
		if (row->par()->isSeparator(p)) {
			++n;
		}
	}
	return n;
}


// returns the number of hfills in the specified row. The LyX-Hfill is
// a LaTeX \hfill so that the hfills at the beginning and at the end were 
// ignored. This is *MUCH* more usefull than not to ignore!
int LyXText::numberOfHfills(Buffer const * buf, Row const * row) const
{
	pos_type const last = rowLast(row);
	pos_type first = row->pos();
	if (first) { /* hfill *DO* count at the beginning 
		      * of paragraphs! */
		while(first <= last && row->par()->isHfill(first))
			++first;
	}

	first = max(first, beginningOfMainBody(buf, row->par()));
	int n = 0;
	for (pos_type p = first; p <= last; ++p) {
		// last, because the end is ignored!
		if (row->par()->isHfill(p)) {
			++n;
		}
	}
	return n;
}


// like NumberOfHfills, but only those in the manual label!
int LyXText::numberOfLabelHfills(Buffer const * buf, Row const * row) const
{
	pos_type last = rowLast(row);
	pos_type first = row->pos();
	if (first) { /* hfill *DO* count at the beginning 
		      * of paragraphs! */
		while(first < last && row->par()->isHfill(first))
			++first;
	}

	last = min(last, beginningOfMainBody(buf, row->par()));
	int n = 0;
	for (pos_type p = first; p < last; ++p) {
		// last, because the end is ignored!
		if (row->par()->isHfill(p)) {
			++n;
		}
	}
	return n;
}


// returns true, if a expansion is needed.
// Rules are given by LaTeX
bool LyXText::hfillExpansion(Buffer const * buf, Row const * row_ptr,
                             pos_type pos) const
{
	// by the way, is it a hfill?
	if (!row_ptr->par()->isHfill(pos))
		return false;
	
	// at the end of a row it does not count
	if (pos >= rowLast(row_ptr))
		return false;
	
	// at the beginning of a row it does not count, if it is not 
	// the first row of a paragaph
	if (!row_ptr->pos())
		return true;
	
	// in some labels  it does not count
	if (textclasslist.Style(buf->params.textclass,
				row_ptr->par()->getLayout()).margintype
	    != MARGIN_MANUAL
	    && pos < beginningOfMainBody(buf, row_ptr->par()))
		return false; 
	
	// if there is anything between the first char of the row and
	// the sepcified position that is not a newline and not a hfill,
	// the hfill will count, otherwise not
	pos_type i = row_ptr->pos();
	while (i < pos && (row_ptr->par()->isNewline(i)
			   || row_ptr->par()->isHfill(i)))
		++i;
	
	return i != pos;
}


LColor::color LyXText::backgroundColor()
{
	if (inset_owner)
		return inset_owner->backgroundColor();
	else
		return LColor::background;
}

void LyXText::setHeightOfRow(BufferView * bview, Row * row_ptr) const
{
	/* get the maximum ascent and the maximum descent */
	int asc = 0;
	int desc = 0;
	float layoutasc = 0;
	float layoutdesc = 0;
	float tmptop = 0;
	LyXFont tmpfont;
	Inset * tmpinset = 0;

	/* ok , let us initialize the maxasc and maxdesc value. 
	 * This depends in LaTeX of the font of the last character
	 * in the paragraph. The hack below is necessary because
	 * of the possibility of open footnotes */
	
	/* Correction: only the fontsize count. The other properties
	   are taken from the layoutfont. Nicer on the screen :) */
	Paragraph * par = row_ptr->par();
	Paragraph * firstpar = row_ptr->par();
   
	LyXLayout const & layout = textclasslist.Style(bview->buffer()->params.textclass,
	                                               firstpar->getLayout());

	// as max get the first character of this row then it can increes but not
	// decrees the height. Just some point to start with so we don't have to
	// do the assignment below too often.
	LyXFont font = getFont(bview->buffer(), par, row_ptr->pos());
	LyXFont::FONT_SIZE const tmpsize = font.size();
	font = getLayoutFont(bview->buffer(), par);
	LyXFont::FONT_SIZE const size = font.size();
	font.setSize(tmpsize);

	LyXFont labelfont = getLabelFont(bview->buffer(), par);

	float spacing_val = 1.0;
	if (!row_ptr->par()->params().spacing().isDefault()) {
		spacing_val = row_ptr->par()->params().spacing().getValue();
	} else {
		spacing_val = bview->buffer()->params.spacing.getValue();
	}
	//lyxerr << "spacing_val = " << spacing_val << endl;
   
	int maxasc = int(lyxfont::maxAscent(font) *
	                 layout.spacing.getValue() *
	                 spacing_val);
	int maxdesc = int(lyxfont::maxDescent(font) *
	                  layout.spacing.getValue() *
	                  spacing_val);
	pos_type const pos_end = rowLast(row_ptr);
	int labeladdon = 0;
	int maxwidth = 0;

	// Check if any insets are larger
	for (pos_type pos = row_ptr->pos(); pos <= pos_end; ++pos) {
		if (row_ptr->par()->isInset(pos)) {
			tmpfont = getFont(bview->buffer(), row_ptr->par(), pos);
			tmpinset = row_ptr->par()->getInset(pos);
			if (tmpinset) {
#if 1 // this is needed for deep update on initialitation
				tmpinset->update(bview, tmpfont);
#endif
				asc = tmpinset->ascent(bview, tmpfont);
				desc = tmpinset->descent(bview, tmpfont);
				maxwidth += tmpinset->width(bview, tmpfont);
				maxasc = max(maxasc, asc);
				maxdesc = max(maxdesc, desc);
			}
		} else {
			maxwidth += singleWidth(bview, row_ptr->par(), pos);
		}
	}

	// Check if any custom fonts are larger (Asger)
	// This is not completely correct, but we can live with the small,
	// cosmetic error for now.
	LyXFont::FONT_SIZE maxsize =
		row_ptr->par()->highestFontInRange(row_ptr->pos(), pos_end, size);
	if (maxsize > font.size()) {
		font.setSize(maxsize);

		asc = lyxfont::maxAscent(font);
		desc = lyxfont::maxDescent(font);
		if (asc > maxasc) 
			maxasc = asc;
		if (desc > maxdesc)
			maxdesc = desc;
	}

	// This is nicer with box insets:
	++maxasc;
	++maxdesc;

	row_ptr->ascent_of_text(maxasc);
   
	// is it a top line?
	if (!row_ptr->pos() && (row_ptr->par() == firstpar)) {
      
		// some parksips VERY EASY IMPLEMENTATION
		if (bview->buffer()->params.paragraph_separation ==
			BufferParams::PARSEP_SKIP)
		{
			if (layout.isParagraph()
				&& firstpar->getDepth() == 0
				&& firstpar->previous())
			{
				maxasc += bview->buffer()->params.getDefSkip().inPixels(bview);
			} else if (firstpar->previous() &&
			           textclasslist.Style(bview->buffer()->params.textclass,
			                               firstpar->previous()->
			                               getLayout()).isParagraph() &&
			           firstpar->previous()->getDepth() == 0)
			{
				// is it right to use defskip here too? (AS)
				maxasc += bview->buffer()->params.getDefSkip().inPixels(bview);
			}
		}
      
		// the paper margins
		if (!row_ptr->par()->previous() && bv_owner)
			maxasc += LYX_PAPER_MARGIN;
      
		// add the vertical spaces, that the user added
		if (firstpar->params().spaceTop().kind() != VSpace::NONE)
			maxasc += int(firstpar->params().spaceTop().inPixels(bview));
      
		// do not forget the DTP-lines!
		// there height depends on the font of the nearest character
		if (firstpar->params().lineTop())
			maxasc += 2 * lyxfont::ascent('x', getFont(bview->buffer(),
			                                           firstpar, 0));
      
		// and now the pagebreaks
		if (firstpar->params().pagebreakTop())
			maxasc += 3 * defaultHeight();
      
		// This is special code for the chapter, since the label of this
		// layout is printed in an extra row
		if (layout.labeltype == LABEL_COUNTER_CHAPTER
			&& bview->buffer()->params.secnumdepth >= 0)
		{
			float spacing_val = 1.0;
			if (!row_ptr->par()->params().spacing().isDefault()) {
				spacing_val = row_ptr->par()->params().spacing().getValue();
			} else {
				spacing_val = bview->buffer()->params.spacing.getValue();
			}
	      
			labeladdon = int(lyxfont::maxDescent(labelfont) *
			                 layout.spacing.getValue() *
			                 spacing_val)
				+ int(lyxfont::maxAscent(labelfont) *
				      layout.spacing.getValue() *
				      spacing_val);
		}
      
		// special code for the top label
		if ((layout.labeltype == LABEL_TOP_ENVIRONMENT
		     || layout.labeltype == LABEL_BIBLIO
		     || layout.labeltype == LABEL_CENTERED_TOP_ENVIRONMENT)
		    && row_ptr->par()->isFirstInSequence()
		    && !row_ptr->par()->getLabelstring().empty())
		{
			float spacing_val = 1.0;
			if (!row_ptr->par()->params().spacing().isDefault()) {
				spacing_val = row_ptr->par()->params().spacing().getValue();
			} else {
				spacing_val = bview->buffer()->params.spacing.getValue();
			}
	      
			labeladdon = int(
				(lyxfont::maxAscent(labelfont) *
				 layout.spacing.getValue() *
				 spacing_val)
				+(lyxfont::maxDescent(labelfont) *
				  layout.spacing.getValue() *
				  spacing_val)
				+ layout.topsep * defaultHeight()
				+ layout.labelbottomsep *  defaultHeight());
		}
   
		// and now the layout spaces, for example before and after a section, 
		// or between the items of a itemize or enumerate environment
      
		if (!firstpar->params().pagebreakTop()) {
			Paragraph * prev = row_ptr->par()->previous();
			if (prev)
				prev = row_ptr->par()->depthHook(row_ptr->par()->getDepth());
			if (prev && prev->getLayout() == firstpar->getLayout() &&
				prev->getDepth() == firstpar->getDepth() &&
				prev->getLabelWidthString() == firstpar->getLabelWidthString())
			{
				layoutasc = (layout.itemsep * defaultHeight());
			} else if (row_ptr->previous()) {
				tmptop = layout.topsep;
	    
				if (row_ptr->previous()->par()->getDepth() >= row_ptr->par()->getDepth())
					tmptop -= textclasslist.Style(bview->buffer()->params.textclass,
					                              row_ptr->previous()->par()->
					                              getLayout()).bottomsep;
	    
				if (tmptop > 0)
					layoutasc = (tmptop * defaultHeight());
			} else if (row_ptr->par()->params().lineTop()) {
				tmptop = layout.topsep;
	    
				if (tmptop > 0)
					layoutasc = (tmptop * defaultHeight());
			}
	 
			prev = row_ptr->par()->outerHook();
			if (prev)  {
				maxasc += int(textclasslist.Style(bview->buffer()->params.textclass,
				              prev->getLayout()).parsep * defaultHeight());
			} else {
				if (firstpar->previous() &&
					firstpar->previous()->getDepth() == 0 &&
					firstpar->previous()->getLayout() !=
					firstpar->getLayout())
				{
					// avoid parsep
				} else if (firstpar->previous()) {
					maxasc += int(layout.parsep * defaultHeight());
				}
			}
		}
	}
   
	// is it a bottom line?
	if (row_ptr->par() == par
		&& (!row_ptr->next() || row_ptr->next()->par() != row_ptr->par()))
	{
		// the paper margins
		if (!par->next() && bv_owner)
			maxdesc += LYX_PAPER_MARGIN;
	  
		// add the vertical spaces, that the user added
		if (firstpar->params().spaceBottom().kind() != VSpace::NONE)
			maxdesc += int(firstpar->params().spaceBottom().inPixels(bview));
	  
		// do not forget the DTP-lines!
		// there height depends on the font of the nearest character
		if (firstpar->params().lineBottom())
			maxdesc += 2 * lyxfont::ascent('x',
			                               getFont(bview->buffer(),
							       par,
							       max(pos_type(0), par->size() - 1)));
	  
		// and now the pagebreaks
		if (firstpar->params().pagebreakBottom())
			maxdesc += 3 * defaultHeight();
	  
		// and now the layout spaces, for example before and after
		// a section, or between the items of a itemize or enumerate
		// environment
		if (!firstpar->params().pagebreakBottom() && row_ptr->par()->next()) {
			Paragraph * nextpar = row_ptr->par()->next();
			Paragraph * comparepar = row_ptr->par();
			float usual = 0;
			float unusual = 0;
	     
			if (comparepar->getDepth() > nextpar->getDepth()) {
				usual = (textclasslist.Style(bview->buffer()->params.textclass,
				         comparepar->getLayout()).bottomsep * defaultHeight());
				comparepar = comparepar->depthHook(nextpar->getDepth());
				if (comparepar->getLayout()!= nextpar->getLayout()
					|| nextpar->getLabelWidthString() != 
					comparepar->getLabelWidthString())
				{
					unusual = (textclasslist.Style(bview->buffer()->params.textclass,
					           comparepar->getLayout()).bottomsep * defaultHeight());
				}
				if (unusual > usual)
					layoutdesc = unusual;
				else
					layoutdesc = usual;
			} else if (comparepar->getDepth() ==  nextpar->getDepth()) {
				
				if (comparepar->getLayout()!= nextpar->getLayout()
					|| nextpar->getLabelWidthString() != 
					comparepar->getLabelWidthString())
					layoutdesc = int(textclasslist.Style(bview->buffer()->params.textclass,
														 comparepar->getLayout()).bottomsep * defaultHeight());
			}
		}
	}
	
	// incalculate the layout spaces
	maxasc += int(layoutasc * 2 / (2 + firstpar->getDepth()));
	maxdesc += int(layoutdesc * 2 / (2 + firstpar->getDepth()));
	
	// calculate the new height of the text
	height -= row_ptr->height();
	
	row_ptr->height(maxasc + maxdesc + labeladdon);
	row_ptr->baseline(maxasc + labeladdon);
	
	height += row_ptr->height();
	float x = 0;
	if (layout.margintype != MARGIN_RIGHT_ADDRESS_BOX) {
		float dummy;
		prepareToPrint(bview, row_ptr, x, dummy, dummy, dummy, false);
	}
	row_ptr->width(int(maxwidth + x));
	if (inset_owner) {
		Row * r = firstrow;
		width = max(0,workWidth(bview));
		while(r) {
			if (r->width() > width)
				width = r->width();
			r = r->next();
		}
	}
}


/* Appends the implicit specified paragraph behind the specified row,
 * start at the implicit given position */
void LyXText::appendParagraph(BufferView * bview, Row * row) const
{
   bool not_ready = true;
   
   // The last character position of a paragraph is an invariant so we can 
   // safely get it here. (Asger)
   pos_type const lastposition = row->par()->size();
   do {
      // Get the next breakpoint
      pos_type z = nextBreakPoint(bview, row, workWidth(bview));
      
      Row * tmprow = row;

      // Insert the new row
      if (z < lastposition) {
	 ++z;
	 insertRow(row, row->par(), z);
	 row = row->next();

	 row->height(0);
      } else
	 not_ready = false;
      
      // Set the dimensions of the row
#ifdef WITH_WARNINGS
#warning Something is rotten here! (Jug)
#endif
      tmprow->fill(fill(bview, tmprow, workWidth(bview)));
      setHeightOfRow(bview, tmprow);

   } while (not_ready);
}


void LyXText::breakAgain(BufferView * bview, Row * row) const
{
	bool not_ready = true;
   
	do  {
		// get the next breakpoint
		pos_type z = nextBreakPoint(bview, row, workWidth(bview));
		Row * tmprow = row;

		if (z < row->par()->size()) {
			if (!row->next() || (row->next() && row->next()->par() != row->par())) {
				// insert a new row
				++z;
				insertRow(row, row->par(), z);
				row = row->next();
				row->height(0);
			} else  {
				row = row->next();
				++z;
				if (row->pos() == z)
					not_ready = false;     // the rest will not change
				else {
					row->pos(z);
				}
			}
		} else {
			/* if there are some rows too much, delete them */
			/* only if you broke the whole paragraph! */ 
			Row * tmprow2 = row;
			while (tmprow2->next() && tmprow2->next()->par() == row->par()) {
				tmprow2 = tmprow2->next();
			}
			while (tmprow2 != row) {
				tmprow2 = tmprow2->previous();
				removeRow(tmprow2->next());
			}
			not_ready = false;
		}
		
		/* set the dimensions of the row */ 
		tmprow->fill(fill(bview, tmprow, workWidth(bview)));
		setHeightOfRow(bview, tmprow);
	} while (not_ready);
}


// this is just a little changed version of break again
void LyXText::breakAgainOneRow(BufferView * bview, Row * row)
{
	// get the next breakpoint
	pos_type z = nextBreakPoint(bview, row, workWidth(bview));
	Row * tmprow = row;

	if (z < row->par()->size()) {
		if (!row->next()
		    || (row->next() && row->next()->par() != row->par())) {
			/* insert a new row */ 
			++z;
			insertRow(row, row->par(), z);
			row = row->next();
			row->height(0);
		} else  {
			row= row->next();
			++z;
			if (row->pos() != z)
				row->pos(z);
		}
	} else {
		// if there are some rows too much, delete them
		// only if you broke the whole paragraph!
		Row * tmprow2 = row;
		while (tmprow2->next()
		       && tmprow2->next()->par() == row->par()) {
			tmprow2 = tmprow2->next();
		}
		while (tmprow2 != row) {
			tmprow2 = tmprow2->previous();
			removeRow(tmprow2->next());
		}
	}
	
	// set the dimensions of the row
	tmprow->fill(fill(bview, tmprow, workWidth(bview)));
	setHeightOfRow(bview, tmprow);
}


void LyXText::breakParagraph(BufferView * bview, char keep_layout)
{
   LyXLayout const & layout =
	   textclasslist.Style(bview->buffer()->params.textclass,
			       cursor.par()->getLayout());

   // this is only allowed, if the current paragraph is not empty or caption
   if ((cursor.par()->size() <= 0)
       && layout.labeltype!= LABEL_SENSITIVE)
	   return;
   
   setUndo(bview, Undo::INSERT,cursor.par(),cursor.par()->next()); 

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
     keep_layout = layout.isEnvironment();
   cursor.par()->breakParagraph(bview->buffer()->params, cursor.pos(),
				keep_layout);

   // well this is the caption hack since one caption is really enough
   if (layout.labeltype == LABEL_SENSITIVE) {
     if (!cursor.pos())
	     // set to standard-layout
	     cursor.par()->setLayout(0);
     else
	     // set to standard-layout
	     cursor.par()->next()->setLayout(0);
   }
   
   /* if the cursor is at the beginning of a row without prior newline, 
    * move one row up! 
    * This touches only the screen-update. Otherwise we would may have
    * an empty row on the screen */
   if (cursor.pos() && !cursor.row()->par()->isNewline(cursor.row()->pos() - 1)
       && cursor.row()->pos() == cursor.pos()) {
	   cursorLeft(bview);
   } 
   
   status(bview, LyXText::NEED_MORE_REFRESH);
   refresh_row = cursor.row();
   refresh_y = cursor.y() - cursor.row()->baseline();
   
   // Do not forget the special right address boxes
   if (layout.margintype == MARGIN_RIGHT_ADDRESS_BOX) {
      while (refresh_row->previous() &&
	     refresh_row->previous()->par() == refresh_row->par()) {
	      refresh_row = refresh_row->previous();
	      refresh_y -= refresh_row->height();
      }
   }
   removeParagraph(cursor.row());
   
   // set the dimensions of the cursor row
   cursor.row()->fill(fill(bview, cursor.row(), workWidth(bview)));

   setHeightOfRow(bview, cursor.row());

   while (cursor.par()->next()->size()
	  && cursor.par()->next()->isNewline(0))
	   cursor.par()->next()->erase(0);
   
   insertParagraph(bview, cursor.par()->next(), cursor.row());

   updateCounters(bview, cursor.row()->previous());
   
   /* This check is necessary. Otherwise the new empty paragraph will
    * be deleted automatically. And it is more friendly for the user! */ 
   if (cursor.pos())
	   setCursor(bview, cursor.par()->next(), 0);
   else
	   setCursor(bview, cursor.par(), 0);
   
   if (cursor.row()->next())
	   breakAgain(bview, cursor.row()->next());

   need_break_row = 0;
}


// Just a macro to make some thing easier. 
void LyXText::redoParagraph(BufferView * bview) const
{
	clearSelection();
	redoParagraphs(bview, cursor, cursor.par()->next());
	setCursorIntern(bview, cursor.par(), cursor.pos());
}


/* insert a character, moves all the following breaks in the 
 * same Paragraph one to the right and make a rebreak */
void LyXText::insertChar(BufferView * bview, char c)
{
	setUndo(bview, Undo::INSERT,
		cursor.par(), cursor.par()->next());

	// When the free-spacing option is set for the current layout,
	// disable the double-space checking

	bool const freeSpacing = 
		textclasslist.Style(bview->buffer()->params.textclass,
			       cursor.row()->par()->getLayout()).free_spacing;


	if (lyxrc.auto_number) {
		static string const number_operators = "+-/*";
		static string const number_unary_operators = "+-";
		static string const number_seperators = ".,:";

		if (current_font.number() == LyXFont::ON) {
			if (!isdigit(c) && !contains(number_operators, c) &&
			    !(contains(number_seperators, c) &&
			      cursor.pos() >= 1 &&
			      cursor.pos() < cursor.par()->size() &&
			      getFont(bview->buffer(),
				      cursor.par(),
				      cursor.pos()).number() == LyXFont::ON &&
			      getFont(bview->buffer(),
				      cursor.par(),
				      cursor.pos() - 1).number() == LyXFont::ON)
			    )
				number(bview); // Set current_font.number to OFF
		} else if (isdigit(c) &&
			   real_current_font.isVisibleRightToLeft()) {
			number(bview); // Set current_font.number to ON

			if (cursor.pos() > 0) {
				char const c = cursor.par()->getChar(cursor.pos() - 1);
				if (contains(number_unary_operators, c) &&
				    (cursor.pos() == 1 ||
				     cursor.par()->isSeparator(cursor.pos() - 2) ||
				     cursor.par()->isNewline(cursor.pos() - 2) )
				   ) {
					setCharFont(bview->buffer(),
						    cursor.par(),
						    cursor.pos() - 1,
						    current_font);
				} else if (contains(number_seperators, c) &&
					   cursor.pos() >= 2 &&
					   getFont(bview->buffer(),
						   cursor.par(),
						   cursor.pos() - 2).number() == LyXFont::ON) {
					setCharFont(bview->buffer(),
						    cursor.par(),
						    cursor.pos() - 1,
						    current_font);
				}
			}
		}
	}


	/* First check, if there will be two blanks together or a blank at 
	  the beginning of a paragraph. 
	  I decided to handle blanks like normal characters, the main 
	  difference are the special checks when calculating the row.fill
	  (blank does not count at the end of a row) and the check here */ 

	// The bug is triggered when we type in a description environment:
	// The current_font is not changed when we go from label to main text
	// and it should (along with realtmpfont) when we type the space.
	// CHECK There is a bug here! (Asger)
	
	LyXFont realtmpfont = real_current_font;
	LyXFont rawtmpfont = current_font;  /* store the current font.
				     * This is because of the use
				     * of cursor movements. The moving
				     * cursor would refresh the 
				     * current font */

	// Get the font that is used to calculate the baselineskip
	pos_type const lastpos = cursor.par()->size();
	LyXFont rawparfont =
		cursor.par()->getFontSettings(bview->buffer()->params,
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
					bview->owner()->message(_("You cannot insert a space at the beginning of a paragraph.  Please read the Tutorial."));
				else
					bview->owner()->message(_("You cannot type two spaces this way.  Please read the Tutorial."));
				sent_space_message = true;
			}
			charInserted();
			return;
		}
	} else if (IsNewlineChar(c)) {
		if (cursor.par() == cursor.par()
		    && cursor.pos() <= beginningOfMainBody(bview->buffer(), cursor.par())) {
			charInserted();
			return;
		}
		/* No newline at first position 
		 * of a paragraph or behind labels. 
		 * TeX does not allow that. */

		if (cursor.pos() < cursor.par()->size() &&
		    cursor.par()->isLineSeparator(cursor.pos()))
			// newline always after a blank!
			cursorRight(bview);
		cursor.row()->fill(-1);	       // to force a new break
	}
   
	// the display inset stuff
	if (cursor.row()->par()->isInset(cursor.row()->pos())
	    && cursor.row()->par()->getInset(cursor.row()->pos())
	    && (cursor.row()->par()->getInset(cursor.row()->pos())->display() ||
		cursor.row()->par()->getInset(cursor.row()->pos())->needFullRow()))
		cursor.row()->fill(-1); // to force a new break  

	// get the cursor row fist
	Row * row = cursor.row();
	int y = cursor.y() - row->baseline();
	if (c != Paragraph::META_INSET) /* Here case LyXText::InsertInset 
					    * already insertet the character */
		cursor.par()->insertChar(cursor.pos(), c);
	setCharFont(bview->buffer(), cursor.par(), cursor.pos(), rawtmpfont);

	if (!jumped_over_space) {
		// refresh the positions
		Row * tmprow = row;
		while (tmprow->next() && tmprow->next()->par() == row->par()) {
			tmprow = tmprow->next();
			tmprow->pos(tmprow->pos() + 1);
		}
	}
   
	// Is there a break one row above
	if ((cursor.par()->isLineSeparator(cursor.pos())
	     || cursor.par()->isNewline(cursor.pos())
	     || cursor.row()->fill() == -1)
	    && row->previous() && row->previous()->par() == row->par()) {
		pos_type z = nextBreakPoint(bview,
							   row->previous(),
							   workWidth(bview));
		if (z >= row->pos()) {
			row->pos(z + 1);
			
			// set the dimensions of the row above
			row->previous()->fill(fill(bview,
						   row->previous(),
						   workWidth(bview)));

			setHeightOfRow(bview, row->previous());
	     
			y -= row->previous()->height();
			refresh_y = y;
			refresh_row = row->previous();
			status(bview, LyXText::NEED_MORE_REFRESH);
	     
			breakAgainOneRow(bview, row);

			current_font = rawtmpfont;
			real_current_font = realtmpfont;
			setCursor(bview, cursor.par(), cursor.pos() + 1,
				  false, cursor.boundary());
			// cursor MUST be in row now.
	     
			if (row->next() && row->next()->par() == row->par())
				need_break_row = row->next();
			else
				need_break_row = 0;
	     
			// check, wether the last characters font has changed.
			if (cursor.pos() && cursor.pos() == cursor.par()->size()
			    && rawparfont != rawtmpfont)
				redoHeightOfParagraph(bview, cursor);
			
			charInserted();
			return;
		}
	}
   
	// recalculate the fill of the row
	if (row->fill() >= 0)  /* needed because a newline
			      * will set fill to -1. Otherwise
			      * we would not get a rebreak! */
		row->fill(fill(bview, row, workWidth(bview)));
	if (row->fill() < 0) {
		refresh_y = y;
		refresh_row = row; 
		refresh_x = cursor.x();
		refresh_pos = cursor.pos();
		status(bview, LyXText::NEED_MORE_REFRESH);
		breakAgainOneRow(bview, row); 
		// will the cursor be in another row now?
		if (rowLast(row) <= cursor.pos() + 1 && row->next()) {
			if (row->next() && row->next()->par() == row->par())
				// this should always be true
				row = row->next();
			breakAgainOneRow(bview, row);
		}
		current_font = rawtmpfont;
		real_current_font = realtmpfont;

		setCursor(bview, cursor.par(), cursor.pos() + 1, false,
			  cursor.boundary());
		if (isBoundary(bview->buffer(), cursor.par(), cursor.pos())
		    != cursor.boundary())
			setCursor(bview, cursor.par(), cursor.pos(), false,
			  !cursor.boundary());
		if (row->next() && row->next()->par() == row->par())
			need_break_row = row->next();
		else
			need_break_row = 0;		
	} else {
		refresh_y = y;
		refresh_x = cursor.x();
		refresh_row = row;
		refresh_pos = cursor.pos();
		
		int const tmpheight = row->height();
		setHeightOfRow(bview, row);
		if (tmpheight == row->height())
			status(bview, LyXText::NEED_VERY_LITTLE_REFRESH);
		else
			status(bview, LyXText::NEED_MORE_REFRESH);
            
		current_font = rawtmpfont;
		real_current_font = realtmpfont;
		setCursor(bview, cursor.par(), cursor.pos() + 1, false,
			  cursor.boundary());
	}

	// check, wether the last characters font has changed.
	if (cursor.pos() && cursor.pos() == cursor.par()->size()
	    && rawparfont != rawtmpfont) {
		redoHeightOfParagraph(bview, cursor);
	} else {
		// now the special right address boxes
		if (textclasslist.Style(bview->buffer()->params.textclass,
				   cursor.par()->getLayout()).margintype
		    == MARGIN_RIGHT_ADDRESS_BOX) {
			redoDrawingOfParagraph(bview, cursor); 
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


void LyXText::prepareToPrint(BufferView * bview,
                             Row * row, float & x,
                             float & fill_separator, 
                             float & fill_hfill,
                             float & fill_label_hfill,
                             bool bidi) const
{
	float nlh;
	float ns;
	
	float w = row->fill();
	fill_hfill = 0;
	fill_label_hfill = 0;
	fill_separator = 0;
	fill_label_hfill = 0;

        bool const is_rtl =
		row->par()->isRightToLeftPar(bview->buffer()->params);
	if (is_rtl) {
		x = (workWidth(bview) > 0)
			? rightMargin(bview->buffer(), row) : 0;
	} else
		x = (workWidth(bview) > 0) ? leftMargin(bview, row) : 0;
	
	// is there a manual margin with a manual label
	if (textclasslist.Style(bview->buffer()->params.textclass,
			   row->par()->getLayout()).margintype == MARGIN_MANUAL
	    && textclasslist.Style(bview->buffer()->params.textclass,
			      row->par()->getLayout()).labeltype == LABEL_MANUAL) {
	       
		/* one more since labels are left aligned */ 
		nlh = numberOfLabelHfills(bview->buffer(), row) + 1;
		if (nlh && !row->par()->getLabelWidthString().empty()) {
			fill_label_hfill = labelFill(bview, row) / nlh;
		}
	}
		
	// are there any hfills in the row?
	float const nh = numberOfHfills(bview->buffer(), row);

	if (nh) {
		if (w > 0)
			fill_hfill = w / nh;
	} else  {
		// is it block, flushleft or flushright? 
		// set x how you need it
		int align;
		if (row->par()->params().align() == LYX_ALIGN_LAYOUT) {
			align = textclasslist.Style(bview->buffer()->params.textclass, row->par()->getLayout()).align;
		} else {
			align = row->par()->params().align();
		}
		
		// center displayed insets 
		Inset * inset;
		if (row->par()->isInset(row->pos())
		    && (inset=row->par()->getInset(row->pos()))
		    && (inset->display())) // || (inset->scroll() < 0)))
		    align = (inset->lyxCode() == Inset::MATHMACRO_CODE)
		        ? LYX_ALIGN_BLOCK : LYX_ALIGN_CENTER;
		
		switch (align) {
	    case LYX_ALIGN_BLOCK:
			ns = numberOfSeparators(bview->buffer(), row);
			if (ns && row->next() && row->next()->par() == row->par() &&
			    !(row->next()->par()->isNewline(row->next()->pos() - 1))
			    && !(row->next()->par()->isInset(row->next()->pos())
			         && row->next()->par()->getInset(row->next()->pos())
			         && row->next()->par()->getInset(row->next()->pos())->display())
				)
			{
				fill_separator = w / ns;
			} else if (is_rtl) {
				x += w;
			}
			break;
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

	computeBidiTables(bview->buffer(), row);
	if (is_rtl) {
		pos_type main_body = 
			beginningOfMainBody(bview->buffer(), row->par());
		pos_type last = rowLast(row);

		if (main_body > 0 &&
		    (main_body-1 > last || 
		     !row->par()->isLineSeparator(main_body-1))) {
			LyXLayout const & layout =
				textclasslist.Style(bview->buffer()->params.textclass,
						    row->par()->getLayout());
			x += lyxfont::width(layout.labelsep,
					    getLabelFont(bview->buffer(), row->par()));
			if (main_body-1 <= last)
				x += fill_label_hfill;
		}
	}
}
      
/* important for the screen */


/* the cursor set functions have a special mechanism. When they
* realize, that you left an empty paragraph, they will delete it.
* They also delete the corresponding row */

void LyXText::cursorRightOneWord(BufferView * bview) const
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
		        tmpcursor.par()->isWord( tmpcursor.pos())) {
		  //     printf("Current pos2 %d", tmpcursor.pos()) ;
			tmpcursor.pos(tmpcursor.pos() + 1);
			++steps;
		}
	}
	setCursor(bview, tmpcursor.par(), tmpcursor.pos());
}


void LyXText::cursorTab(BufferView * bview) const
{
    LyXCursor tmpcursor = cursor;
    while (tmpcursor.pos() < tmpcursor.par()->size()
           && !tmpcursor.par()->isNewline(tmpcursor.pos()))
        tmpcursor.pos(tmpcursor.pos() + 1);

    if (tmpcursor.pos() == tmpcursor.par()->size()){
        if (tmpcursor.par()->next()) {
            tmpcursor.par(tmpcursor.par()->next());
            tmpcursor.pos(0);
        }
    } else
        tmpcursor.pos(tmpcursor.pos() + 1);
    setCursor(bview, tmpcursor.par(), tmpcursor.pos());
}


/* -------> Skip initial whitespace at end of word and move cursor to *start*
            of prior word, not to end of next prior word. */

void LyXText::cursorLeftOneWord(BufferView * bview)  const
{
	LyXCursor tmpcursor = cursor;
	cursorLeftOneWord(tmpcursor);
	setCursor(bview, tmpcursor.par(), tmpcursor.pos());
}

void LyXText::cursorLeftOneWord(LyXCursor  & cur)  const
{
	// treat HFills, floats and Insets as words
	cur = cursor;
	while (cur.pos() 
	       && (cur.par()->isSeparator(cur.pos() - 1) 
		   || cur.par()->isKomma(cur.pos() - 1))
	       && !(cur.par()->isHfill(cur.pos() - 1)
		    || cur.par()->isInset(cur.pos() - 1)))
		cur.pos(cur.pos() - 1);

	if (cur.pos()
	    && (cur.par()->isInset(cur.pos() - 1)
		|| cur.par()->isHfill(cur.pos() - 1))) {
		cur.pos(cur.pos() - 1);
	} else if (!cur.pos()) {
		if (cur.par()->previous()){
			cur.par(cur.par()->previous());
			cur.pos(cur.par()->size());
		}
	} else {		// Here, cur != 0 
		while (cur.pos() > 0 &&
		       cur.par()->isWord(cur.pos()-1) )
			cur.pos(cur.pos() - 1);
	}
}

/* -------> Select current word. This depends on behaviour of
CursorLeftOneWord(), so it is patched as well. */
void LyXText::getWord(LyXCursor & from, LyXCursor & to, 
		      word_location const loc) const
{
	// first put the cursor where we wana start to select the word
	from = cursor;
	switch(loc) {
	case WHOLE_WORD_STRICT:
		if (cursor.pos() == 0 || cursor.pos() == cursor.par()->size()
		    || cursor.par()->isSeparator(cursor.pos())
		    || cursor.par()->isKomma(cursor.pos())
		    || cursor.par()->isSeparator(cursor.pos() -1)
		    || cursor.par()->isKomma(cursor.pos() -1)) {
			to = from;
			return;
		}
		// no break here, we go to the next
		
	case WHOLE_WORD:
		// Move cursor to the beginning, when not already there.
		if (from.pos() && !from.par()->isSeparator(from.pos() - 1)
		    && !from.par()->isKomma(from.pos() - 1))
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
	       && !to.par()->isHfill(to.pos()) )
	{
		to.pos(to.pos() + 1);
	}
}


void LyXText::selectWord(BufferView * bview, word_location const loc) 
{
	LyXCursor from;
	LyXCursor to;
	getWord(from, to, loc);
	if (cursor != from)
		setCursor(bview, from.par(), from.pos());
	if (to == from)
		return;
	selection.cursor = cursor;
	setCursor(bview, to.par(), to.pos() );
	setSelection(bview);
}


/* -------> Select the word currently under the cursor when no
	selection is currently set */
bool LyXText::selectWordWhenUnderCursor(BufferView * bview, 
					word_location const loc) 
{
	if (!selection.set()) {
		selectWord(bview, loc);
		return selection.set();
	}
	return false;
}


// This function is only used by the spellchecker for NextWord().
// It doesn't handle LYX_ACCENTs and probably never will.
string const LyXText::selectNextWordToSpellcheck(BufferView * bview,
                                                 float & value) const
{
	if (the_locking_inset) {
		string str = the_locking_inset->selectNextWordToSpellcheck(bview, value);
		if (!str.empty()) {
			value += float(cursor.y())/float(height);
			return str;
		}
#warning Dekel please have a look on this one RTL? (Jug)
#warning DEKEL!
		// we have to go on checking so move cusor to the right
		if (cursor.pos() == cursor.par()->size()) {
			if (!cursor.par()->next())
				return str;
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
	while ((cursor.par()->size() > cursor.pos()
	       && (!cursor.par()->isLetter(cursor.pos()))
	       && (!cursor.par()->isInset(cursor.pos()) ||
			   !cursor.par()->getInset(cursor.pos())->allowSpellcheck()))
	       || (cursor.par()->size() == cursor.pos()
		   && cursor.par()->next()))
	{      
		if (cursor.pos() == cursor.par()->size()) {
			cursor.par(cursor.par()->next());
			cursor.pos(0);
		} else
			cursor.pos(cursor.pos() + 1);
	}

	// now check if we hit an inset so it has to be a inset containing text!
	if (cursor.pos() < cursor.par()->size() &&
	    cursor.par()->isInset(cursor.pos()))
	{
		// lock the inset!
		cursor.par()->getInset(cursor.pos())->edit(bview);
		// now call us again to do the above trick
		// but obviously we have to start from down below ;)
		return bview->text->selectNextWordToSpellcheck(bview, value);
	}		
  
	// Update the value if we changed paragraphs
	if (cursor.par() != tmppar){
		setCursor(bview, cursor.par(), cursor.pos());
		value = float(cursor.y())/float(height);
	}

	// Start the selection from here
	selection.cursor = cursor;
	
	// and find the end of the word (insets like optional hyphens
	// and ligature break are part of a word)
	while (cursor.pos() < cursor.par()->size()
	       && (cursor.par()->isLetter(cursor.pos()))) 
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
	return str;
}


// This one is also only for the spellchecker
void LyXText::selectSelectedWord(BufferView * bview)
{
	if (the_locking_inset) {
		the_locking_inset->selectSelectedWord(bview);
		return;
	}
	// move cursor to the beginning
	setCursor(bview, selection.cursor.par(), selection.cursor.pos());
	
	// set the sel cursor
	selection.cursor = cursor;
	
	// now find the end of the word
	while (cursor.pos() < cursor.par()->size()
	       && (cursor.par()->isLetter(cursor.pos())))
		cursor.pos(cursor.pos() + 1);
	
	setCursor(bview, cursor.par(), cursor.pos());
	
	// finally set the selection
	setSelection(bview);
}


/* -------> Delete from cursor up to the end of the current or next word. */
void LyXText::deleteWordForward(BufferView * bview)
{
	if (!cursor.par()->size())
		cursorRight(bview);
	else {
		LyXCursor tmpcursor = cursor;
		tmpcursor.row(0); // ??
		selection.set(true); // to avoid deletion
		cursorRightOneWord(bview);
		setCursor(bview, tmpcursor, tmpcursor.par(), tmpcursor.pos());
		selection.cursor = cursor;
		cursor = tmpcursor;
		setSelection(bview);
		
		/* -----> Great, CutSelection() gets rid of multiple spaces. */
		cutSelection(bview, true, false);
	}
}


/* -------> Delete from cursor to start of current or prior word. */
void LyXText::deleteWordBackward(BufferView * bview)
{
       if (!cursor.par()->size())
	       cursorLeft(bview);
       else {
	       LyXCursor tmpcursor = cursor;
	       tmpcursor.row(0); // ??
	       selection.set(true); // to avoid deletion
	       cursorLeftOneWord(bview);
	       setCursor(bview, tmpcursor, tmpcursor.par(), tmpcursor.pos());
	       selection.cursor = cursor;
	       cursor = tmpcursor;
	       setSelection(bview);
	       cutSelection(bview, true, false);
       }
}


/* -------> Kill to end of line. */
void LyXText::deleteLineForward(BufferView * bview)
{
	if (!cursor.par()->size())
		// Paragraph is empty, so we just go to the right
		cursorRight(bview);
	else {
		LyXCursor tmpcursor = cursor;
		// We can't store the row over a regular setCursor
		// so we set it to 0 and reset it afterwards.
		tmpcursor.row(0); // ??
		selection.set(true); // to avoid deletion
		cursorEnd(bview);
		setCursor(bview, tmpcursor, tmpcursor.par(), tmpcursor.pos());
		selection.cursor = cursor;
		cursor = tmpcursor;
		setSelection(bview);
		// What is this test for ??? (JMarc)
		if (!selection.set()) {
			deleteWordForward(bview);
		} else {
			cutSelection(bview, true, false);
		}
	}
}


// Change the case of a word at cursor position. 
// This function directly manipulates Paragraph::text because there
// is no Paragraph::SetChar currently. I did what I could to ensure
// that it is correct. I guess part of it should be moved to
// Paragraph, but it will have to change for 1.1 anyway. At least
// it does not access outside of the allocated array as the older
// version did. (JMarc) 
void LyXText::changeCase(BufferView * bview, LyXText::TextCase action)
{
	LyXCursor from;
	LyXCursor to;

	if (selection.set()) {
		from = selection.start;
		to = selection.end;
	} else {
		getWord(from, to, PARTIAL_WORD);
		setCursor(bview, to.par(), to.pos() + 1);
	}

	changeRegionCase(bview, from, to, action);
}


void LyXText::changeRegionCase(BufferView * bview,
			       LyXCursor const & from,
			       LyXCursor const & to,
			       LyXText::TextCase action)
{
	lyx::Assert(from <= to);
	
	setUndo(bview, Undo::FINISH,
		from.par(), to.par()->next());

	pos_type pos = from.pos();
	Paragraph * par = from.par();

	while (par && (pos != to.pos() || par != to.par())) {
		unsigned char c = par->getChar(pos);
		if (!IsInsetChar(c) && !IsHfillChar(c)) {
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
		par->setChar(pos, c);
		checkParagraph(bview, par, pos);

		++pos;
		if (pos == par->size()) {
			par = par->next();
			pos = 0;
		}
	}
	if (to.row() != from.row()) {
		refresh_y = from.y() - from.row()->baseline();
		refresh_row = from.row();
		status(bview, LyXText::NEED_MORE_REFRESH);
	}
}


void LyXText::transposeChars(BufferView & bview)
{
	Paragraph * tmppar = cursor.par();

	setUndo(&bview, Undo::FINISH,
		tmppar, tmppar->next()); 

	pos_type tmppos = cursor.pos();

	// First decide if it is possible to transpose at all

	// We are at the beginning of a paragraph.
	if (tmppos == 0) return;

	// We are at the end of a paragraph.
	if (tmppos == tmppar->size() - 1) return;

	unsigned char c1 = tmppar->getChar(tmppos);
	unsigned char c2 = tmppar->getChar(tmppos - 1);

	if (c1 != Paragraph::META_INSET
	    && c2 != Paragraph::META_INSET) {
		tmppar->setChar(tmppos, c2);
		tmppar->setChar(tmppos - 1, c1);
	}
	// We should have an implementation that handles insets
	// as well, but that will have to come later. (Lgb)
	checkParagraph(const_cast<BufferView*>(&bview), tmppar, tmppos);
}


void LyXText::Delete(BufferView * bview)
{
	// this is a very easy implementation

	LyXCursor old_cursor = cursor;
	int const old_cur_par_id = old_cursor.par()->id();
	int const old_cur_par_prev_id = old_cursor.par()->previous() ?
		old_cursor.par()->previous()->id() : 0;
	
	// just move to the right
	cursorRight(bview);

	// CHECK Look at the comment here.
	// This check is not very good...
	// The cursorRightIntern calls DeleteEmptyParagrapgMechanism
	// and that can very well delete the par or par->previous in
	// old_cursor. Will a solution where we compare paragraph id's
	//work better?
	if ((cursor.par()->previous() ? cursor.par()->previous()->id() : 0)
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
		setUndo(bview, Undo::DELETE,
			cursor.par(), cursor.par()->next()); 
		cursor = tmpcursor;
		backspace(bview);
	}
}


void LyXText::backspace(BufferView * bview)
{
	// Get the font that is used to calculate the baselineskip
	pos_type lastpos = cursor.par()->size();
	LyXFont rawparfont =
		cursor.par()->getFontSettings(bview->buffer()->params,
					      lastpos - 1);

	if (cursor.pos() == 0) {
		// The cursor is at the beginning of a paragraph,
		// so the the backspace will collapse two paragraphs into one.
		
		// we may paste some paragraphs
      
		// is it an empty paragraph?
      
		if ((lastpos == 0
		     || (lastpos == 1 && cursor.par()->isSeparator(0)))) {
			// This is an empty paragraph and we delete it just by moving the cursor one step
			// left and let the DeleteEmptyParagraphMechanism handle the actual deletion
			// of the paragraph.
			
			if (cursor.par()->previous()) {
				Paragraph * tmppar = cursor.par()->previous();
				if (cursor.par()->getLayout() == tmppar->getLayout()
				    && cursor.par()->getAlign() == tmppar->getAlign()) {
					// Inherit bottom DTD from the paragraph below.
					// (the one we are deleting)
					tmppar->params().lineBottom(cursor.par()->params().lineBottom());
					tmppar->params().spaceBottom(cursor.par()->params().spaceBottom());
					tmppar->params().pagebreakBottom(cursor.par()->params().pagebreakBottom());
				}
				
				cursorLeft(bview);
		     
				// the layout things can change the height of a row !
				int const tmpheight = cursor.row()->height();
				setHeightOfRow(bview, cursor.row());
				if (cursor.row()->height() != tmpheight) {
					refresh_y = cursor.y() - cursor.row()->baseline();
					refresh_row = cursor.row();
					status(bview, LyXText::NEED_MORE_REFRESH);
				}
				return;
			}
		}

		if (cursor.par()->previous()) {
			setUndo(bview, Undo::DELETE,
			        cursor.par()->previous(), cursor.par()->next());
		}
		
  		Paragraph * tmppar = cursor.par();
  		Row * tmprow = cursor.row();

		// We used to do cursorLeftIntern() here, but it is
		// not a good idea since it triggers the auto-delete
		// mechanism. So we do a cursorLeftIntern()-lite,
		// without the dreaded mechanism. (JMarc)
		if (cursor.par()->previous()) { 
			// steps into the above paragraph.
			setCursorIntern(bview, cursor.par()->previous(),
			                cursor.par()->previous()->size(),
			                false);
		}

		/* Pasting is not allowed, if the paragraphs have different
		   layout. I think it is a real bug of all other
		   word processors to allow it. It confuses the user.
		   Even so with a footnote paragraph and a non-footnote
		   paragraph. I will not allow pasting in this case, 
		   because the user would be confused if the footnote behaves 
		   different wether it is open or closed.
		  
		   Correction: Pasting is always allowed with standard-layout
		*/
		if (cursor.par() != tmppar
		    && (cursor.par()->getLayout() == tmppar->getLayout()
			|| tmppar->getLayout() == 0 /*standard*/)
		    && cursor.par()->getAlign() == tmppar->getAlign())
		{
			removeParagraph(tmprow);
			removeRow(tmprow);
			cursor.par()->pasteParagraph(bview->buffer()->params);
			
			if (!cursor.pos() || !cursor.par()->isSeparator(cursor.pos() - 1))
				; //cursor.par()->insertChar(cursor.pos(), ' ');
			// strangely enough it seems that commenting out the line above removes
			// most or all of the segfaults. I will however also try to move the
			// two Remove... lines in front of the PasteParagraph too.
			else
				if (cursor.pos())
					cursor.pos(cursor.pos() - 1);
			
			status(bview, LyXText::NEED_MORE_REFRESH);
			refresh_row = cursor.row();
			refresh_y = cursor.y() - cursor.row()->baseline();
			
			// remove the lost paragraph
			// This one is not safe, since the paragraph that the tmprow and the
			// following rows belong to has been deleted by the PasteParagraph
			// above. The question is... could this be moved in front of the
			// PasteParagraph?
			//RemoveParagraph(tmprow);
			//RemoveRow(tmprow);  
			
			// This rebuilds the rows.
			appendParagraph(bview, cursor.row());
			updateCounters(bview, cursor.row());
			
			// the row may have changed, block, hfills etc.
			setCursor(bview, cursor.par(), cursor.pos(), false);
		}
	} else {
		/* this is the code for a normal backspace, not pasting
		 * any paragraphs */ 
		setUndo(bview, Undo::DELETE,
		        cursor.par(), cursor.par()->next()); 
		// We used to do cursorLeftIntern() here, but it is
		// not a good idea since it triggers the auto-delete
		// mechanism. So we do a cursorLeftIntern()-lite,
		// without the dreaded mechanism. (JMarc)
		setCursorIntern(bview, cursor.par(), cursor.pos()- 1,
		                false, cursor.boundary());
		
		// some insets are undeletable here
		if (cursor.par()->isInset(cursor.pos())) {
			if (!cursor.par()->getInset(cursor.pos())->deletable())
				return; 
			// force complete redo when erasing display insets
			// this is a cruel method but safe..... Matthias 
			if (cursor.par()->getInset(cursor.pos())->display() ||
			    cursor.par()->getInset(cursor.pos())->needFullRow()) {
				cursor.par()->erase(cursor.pos());
				redoParagraph(bview);
				return;
			}
		}
		
		Row * row = cursor.row();
		int y = cursor.y() - row->baseline();
		pos_type z;
		/* remember that a space at the end of a row doesnt count
		 * when calculating the fill */ 
		if (cursor.pos() < rowLast(row) ||
		    !cursor.par()->isLineSeparator(cursor.pos())) {
			row->fill(row->fill() + singleWidth(bview,
			                                    cursor.par(),
			                                    cursor.pos()));
		}
		
		/* some special code when deleting a newline. This is similar
		 * to the behavior when pasting paragraphs */ 
		if (cursor.pos() && cursor.par()->isNewline(cursor.pos())) {
			cursor.par()->erase(cursor.pos());
			// refresh the positions
			Row * tmprow = row;
			while (tmprow->next() && tmprow->next()->par() == row->par()) {
				tmprow = tmprow->next();
				tmprow->pos(tmprow->pos() - 1);
			}
			if (cursor.par()->isLineSeparator(cursor.pos() - 1))
				cursor.pos(cursor.pos() - 1);

			if (cursor.pos() < cursor.par()->size()
			    && !cursor.par()->isSeparator(cursor.pos())) {
				cursor.par()->insertChar(cursor.pos(), ' ');
				setCharFont(bview->buffer(), cursor.par(), 
					    cursor.pos(), current_font);
				// refresh the positions
				tmprow = row;
				while (tmprow->next() && tmprow->next()->par() == row->par()) {
					tmprow = tmprow->next();
					tmprow->pos(tmprow->pos() + 1);
				}
			}
		} else {
			cursor.par()->erase(cursor.pos());
			
			// refresh the positions
			Row * tmprow = row;
			while (tmprow->next()
			       && tmprow->next()->par() == row->par()) {
				tmprow = tmprow->next();
				tmprow->pos(tmprow->pos() - 1);
			}

			// delete newlines at the beginning of paragraphs
			while (cursor.par()->size() &&
			       cursor.par()->isNewline(cursor.pos()) &&
			       cursor.pos() == beginningOfMainBody(bview->buffer(),
								   cursor.par())) {
				cursor.par()->erase(cursor.pos());
				// refresh the positions
				tmprow = row;
				while (tmprow->next() && 
				       tmprow->next()->par() == row->par()) {
					tmprow = tmprow->next();
					tmprow->pos(tmprow->pos() - 1);
				}
			}
		}
		
		// is there a break one row above
		if (row->previous() && row->previous()->par() == row->par()) {
			z = nextBreakPoint(bview, row->previous(),
					   workWidth(bview));
			if (z >= row->pos()) {
				row->pos(z + 1);
				
				Row * tmprow = row->previous();
				
				// maybe the current row is now empty
				if (row->pos() >= row->par()->size()) {
					// remove it
					removeRow(row);
					need_break_row = 0;
				} else {
					breakAgainOneRow(bview, row);
					if (row->next() && row->next()->par() == row->par())
						need_break_row = row->next();
					else
						need_break_row = 0;
				}
				
				// set the dimensions of the row above
				y -= tmprow->height();
				tmprow->fill(fill(bview, tmprow,
						  workWidth(bview)));
				setHeightOfRow(bview, tmprow);
				
				refresh_y = y;
				refresh_row = tmprow;
				status(bview, LyXText::NEED_MORE_REFRESH);
				setCursor(bview, cursor.par(), cursor.pos(),
					  false, cursor.boundary());
				//current_font = rawtmpfont;
				//real_current_font = realtmpfont;
				// check, whether the last character's font has changed.
				if (rawparfont !=
				    cursor.par()->getFontSettings(bview->buffer()->params,
								  cursor.par()->size() - 1))
					redoHeightOfParagraph(bview, cursor);
				return;
			}
		}
		
		// break the cursor row again
		if (row->next() && row->next()->par() == row->par() &&
		    (rowLast(row) == row->par()->size() - 1 ||
		     nextBreakPoint(bview, row, workWidth(bview)) != rowLast(row))) {
			
			/* it can happen that a paragraph loses one row
			 * without a real breakup. This is when a word
			 * is to long to be broken. Well, I don t care this 
			 * hack ;-) */
			if (rowLast(row) == row->par()->size() - 1)
				removeRow(row->next());
			
			refresh_y = y;
			refresh_row = row;
			status(bview, LyXText::NEED_MORE_REFRESH);
			
			breakAgainOneRow(bview, row);
			// will the cursor be in another row now?
			if (row->next() && row->next()->par() == row->par() &&
			    rowLast(row) <= cursor.pos()) {
				row = row->next();
				breakAgainOneRow(bview, row);
			}

			setCursor(bview, cursor.par(), cursor.pos(), false, cursor.boundary());

			if (row->next() && row->next()->par() == row->par())
				need_break_row = row->next();
			else
				need_break_row = 0;
		} else  {
			// set the dimensions of the row
			row->fill(fill(bview, row, workWidth(bview)));
			int const tmpheight = row->height();
			setHeightOfRow(bview, row);
			if (tmpheight == row->height())
				status(bview, LyXText::NEED_VERY_LITTLE_REFRESH);
			else
				status(bview, LyXText::NEED_MORE_REFRESH);
			refresh_y = y;
			refresh_row = row;
			setCursor(bview, cursor.par(), cursor.pos(), false, cursor.boundary());
		}
	}

	// current_font = rawtmpfont;
	// real_current_font = realtmpfont;

	if (isBoundary(bview->buffer(), cursor.par(), cursor.pos())
	    != cursor.boundary())
		setCursor(bview, cursor.par(), cursor.pos(), false,
			  !cursor.boundary());

	lastpos = cursor.par()->size();
	if (cursor.pos() == lastpos)
		setCurrentFont(bview);
	
	// check, whether the last characters font has changed.
	if (rawparfont != 
	    cursor.par()->getFontSettings(bview->buffer()->params, lastpos - 1)) {
		redoHeightOfParagraph(bview, cursor);
	} else {
		// now the special right address boxes
		if (textclasslist.Style(bview->buffer()->params.textclass,
					cursor.par()->getLayout()).margintype == MARGIN_RIGHT_ADDRESS_BOX) {
			redoDrawingOfParagraph(bview, cursor); 
		}
	}
}


bool LyXText::paintRowBackground(DrawRowParams & p)
{
	bool clear_area = true;
	Inset * inset = 0;
	LyXFont font(LyXFont::ALL_SANE);

	pos_type const last = rowLastPrintable(p.row);

	if (!p.bv->screen()->forceClear() && last == p.row->pos()
		&& p.row->par()->isInset(p.row->pos())) {
		inset = p.row->par()->getInset(p.row->pos());
		if (inset) {
			clear_area = inset->doClearArea();
		}
	}
 
	if (p.cleared) {
		return true;
	} 
	
	if (clear_area) {
		int const x = p.xo;
		int const y = p.yo < 0 ? 0 : p.yo;
		int const h = p.yo < 0 ? p.row->height() + p.yo : p.row->height();
		p.pain->fillRectangle(x, y, p.width, h, backgroundColor());
		return true;
	}
 
	if (inset == 0)
		return false;
 
	int h = p.row->baseline() - inset->ascent(p.bv, font);
 
	// first clear the whole row above the inset!
	if (h > 0) {
		p.pain->fillRectangle(p.xo, p.yo, p.width, h, backgroundColor());
	}

	// clear the space below the inset!
	h += inset->ascent(p.bv, font) + inset->descent(p.bv, font);
	if ((p.row->height() - h) > 0) {
		p.pain->fillRectangle(p.xo, p.yo + h, 
			p.width, p.row->height() - h, backgroundColor());
	}

	// clear the space behind the inset, if needed
	if (!inset->display() && !inset->needFullRow()) {
		int const xp = int(p.x) + inset->width(p.bv, font);
		if (p.width - xp > 0) {
			p.pain->fillRectangle(xp, p.yo, p.width - xp,
				p.row->height(), backgroundColor());
		}
	}
 
	return false;
}


void LyXText::paintRowSelection(DrawRowParams & p)
{
	bool const is_rtl = p.row->par()->isRightToLeftPar(p.bv->buffer()->params);

	// the current selection
	int const startx = selection.start.x();
	int const endx = selection.end.x();
	int const starty = selection.start.y();
	int const endy = selection.end.y();
	Row const * startrow = selection.start.row();
	Row const * endrow = selection.end.row();
 
	Row * row = p.row;
 
	if (bidi_same_direction) {
		int x;
		int y = p.yo;
		int w;
		int h = row->height();
 
		if (startrow == row && endrow == row) {
			if (startx < endx) {
				x = p.xo + startx;
				w = endx - startx;
				p.pain->fillRectangle(x, y, w, h, LColor::selection);
			} else {
				x = p.xo + endx;
				w = startx - endx;
				p.pain->fillRectangle(x, y, w, h, LColor::selection);
			}
		} else if (startrow == row) {
			int const x = (is_rtl) ? p.xo : (p.xo + startx);
			int const w = (is_rtl) ? startx : (p.width - startx);
			p.pain->fillRectangle(x, y, w, h, LColor::selection);
		} else if (endrow == row) {
			int const x = (is_rtl) ? (p.xo + endx) : p.xo;
			int const w = (is_rtl) ? (p.width - endx) : endx;
			p.pain->fillRectangle(x, y, w, h, LColor::selection);
		} else if (p.y > starty && p.y < endy) {
			p.pain->fillRectangle(p.xo, y, p.width, h, LColor::selection);
		}
		return;
	} else if (startrow != row && endrow != row) {
		int w = p.width;
		int h = row->height();
		if (p.y > starty && p.y < endy) {
			p.pain->fillRectangle(p.xo, p.yo, w, h, LColor::selection);
		}
		return;
	}
 
	if (!((startrow != row && !is_rtl) || (endrow != row && is_rtl))) {
		return;
	}
 
	float tmpx = p.x;
 
	p.pain->fillRectangle(p.xo, p.yo, int(p.x), row->height(), LColor::selection);
 
	Buffer const * buffer = p.bv->buffer();
	Paragraph * par = row->par();
	pos_type main_body = beginningOfMainBody(buffer, par);
	pos_type const last = rowLastPrintable(row);
 
	for (pos_type vpos = row->pos(); vpos <= last; ++vpos)  {
		pos_type pos = vis2log(vpos);
		float const old_tmpx = tmpx;
		if (main_body > 0 && pos == main_body - 1) {
			LyXLayout const & layout = textclasslist.Style(buffer->params.textclass,
				par->getLayout());
			LyXFont const lfont = getLabelFont(buffer, par);
			 
 
			tmpx += p.label_hfill + lyxfont::width(layout.labelsep, lfont);

			if (par->isLineSeparator(main_body - 1))
				tmpx -= singleWidth(p.bv, par, main_body - 1);
		}
 
		if (hfillExpansion(buffer, row, pos)) {
			tmpx += singleWidth(p.bv, par, pos);
			if (pos >= main_body)
				tmpx += p.hfill;
			else 
				tmpx += p.label_hfill;
		}
 
		else if (par->isSeparator(pos)) {
			tmpx += singleWidth(p.bv, par, pos);
			if (pos >= main_body)
				tmpx += p.separator;
		} else {
			tmpx += singleWidth(p.bv, par, pos);
		}
		
		if ((startrow != row || selection.start.pos() <= pos) &&
			(endrow != row || pos < selection.end.pos())) {
			// Here we do not use p.x as p.xo was added to p.x.
			p.pain->fillRectangle(int(old_tmpx), p.yo,
				int(tmpx - old_tmpx + 1),
				row->height(), LColor::selection);
		}

		if ((startrow != row && is_rtl) || (endrow != row && !is_rtl)) {
			p.pain->fillRectangle(p.xo + int(tmpx),
				p.yo, int(p.bv->workWidth() - tmpx),
				row->height(), LColor::selection);
		}
	}
}
 

void LyXText::paintRowAppendix(DrawRowParams & p)
{
	// FIXME: can be just p.width ?
	int const ww = p.bv->workWidth();
	Paragraph * firstpar = p.row->par();

	if (firstpar->params().appendix()) {
		p.pain->line(1, p.yo, 1, p.yo + p.row->height(), LColor::appendixline);
		p.pain->line(ww - 2, p.yo, ww - 2, p.yo + p.row->height(), LColor::appendixline);
	}
}

 
void LyXText::paintRowDepthBar(DrawRowParams & p)
{
	Paragraph::depth_type const depth = p.row->par()->getDepth();
 
	if (depth <= 0)
		return;

	Paragraph::depth_type prev_depth = 0;
	if (p.row->previous())
		prev_depth = p.row->previous()->par()->getDepth();
	Paragraph::depth_type next_depth = 0;
	if (p.row->next())
		next_depth = p.row->next()->par()->getDepth();

	for (Paragraph::depth_type i = 1; i <= depth; ++i) {
		int const x = (LYX_PAPER_MARGIN / 5) * i + p.xo;
		int const h = p.yo + p.row->height() - 1 - (i - next_depth - 1) * 3;
 
		p.pain->line(x, p.yo, x, h, LColor::depthbar);
	
		int const w = LYX_PAPER_MARGIN / 5;
 
		if (i > prev_depth) {
			p.pain->fillRectangle(x, p.yo, w, 2, LColor::depthbar);
		}
		if (i > next_depth) {
			p.pain->fillRectangle(x, h, w, 2, LColor::depthbar);
		}
	}
}

 
void LyXText::paintFirstRow(DrawRowParams & p)
{
	Paragraph * par = p.row->par(); 
	ParagraphParameters const & parparams = par->params();
 
	// start of appendix?
	if (parparams.startOfAppendix()) {
		p.pain->line(1, p.yo, p.width - 2, p.yo, LColor::appendixline);
	}
	
	int y_top = 0;
		
	// think about the margins
	if (!p.row->previous() && bv_owner)
		y_top += LYX_PAPER_MARGIN;

	// draw a top pagebreak
	if (parparams.pagebreakTop()) {
		int const y = p.yo + y_top + 2*defaultHeight();
		p.pain->line(0, y, p.width, y, LColor::pagebreak, Painter::line_onoffdash);
 
		int w = 0;
		int a = 0;
		int d = 0;
 
		LyXFont pb_font;
		pb_font.setColor(LColor::pagebreak).decSize();
		lyxfont::rectText(_("Page Break (top)"), pb_font, w, a, d);
		p.pain->rectText((p.width - w)/2, y + d,
			      _("Page Break (top)"), pb_font,
			      backgroundColor(),
			      backgroundColor());
		y_top += 3 * defaultHeight();
	}
	
	// draw a vfill top
	if (parparams.spaceTop().kind() == VSpace::VFILL) {
		int const y1 = p.yo + y_top + 3 * defaultHeight();
		int const y2 = p.yo + 2 + y_top;
 
		p.pain->line(0, y1, LYX_PAPER_MARGIN, y1, LColor::vfillline);
		
		p.pain->line(0, y2, LYX_PAPER_MARGIN, y2, LColor::vfillline);

		int const x = LYX_PAPER_MARGIN / 2;
 
		p.pain->line(x, y2, x, y1, LColor::vfillline);
		
		y_top += 3 * defaultHeight();
	}
	
	// think about user added space
	y_top += int(parparams.spaceTop().inPixels(p.bv));
	
	Buffer const * buffer = p.bv->buffer();
 
	LyXLayout const & layout =
		textclasslist.Style(buffer->params.textclass, par->getLayout());

	// think about the parskip
	// some parskips VERY EASY IMPLEMENTATION
	if (buffer->params.paragraph_separation == BufferParams::PARSEP_SKIP) {
		if (par->previous()) {
			if (layout.latextype == LATEX_PARAGRAPH
				&& !par->getDepth()) {
				y_top += buffer->params.getDefSkip().inPixels(p.bv);
			} else {
				LyXLayout const & playout =
					textclasslist.Style(buffer->params.textclass,
						par->previous()->getLayout()); 
				if (playout.latextype == LATEX_PARAGRAPH
					&& !par->previous()->getDepth()) {
					// is it right to use defskip here, too? (AS) 
					y_top += buffer->params.getDefSkip().inPixels(p.bv);
				}
			}
		}
	}
	
	int const ww = p.bv->workWidth();
 
	// draw a top line
	if (parparams.lineTop()) {
		LyXFont font(LyXFont::ALL_SANE);
		int const asc = lyxfont::ascent('x', getFont(buffer, par, 0));
 
		y_top += asc;
 
		int const w = (inset_owner ?  inset_owner->width(p.bv, font) : ww);
		int const xp = static_cast<int>(inset_owner ? p.x : 0);
		p.pain->line(xp, p.yo + y_top, w, p.yo + y_top,
			LColor::topline, Painter::line_solid,
			Painter::line_thick);
		
		y_top += asc;
	}
	
	bool const is_rtl = p.row->par()->isRightToLeftPar(p.bv->buffer()->params);

	// should we print a label?
	if (layout.labeltype >= LABEL_STATIC
	    && (layout.labeltype != LABEL_STATIC
		|| layout.latextype != LATEX_ENVIRONMENT
		|| par->isFirstInSequence())) {
 
		LyXFont font = getLabelFont(buffer, par);
		if (!par->getLabelstring().empty()) {
			float x = p.x;
			string const str = par->getLabelstring();
			
			// this is special code for the chapter layout. This is
			// printed in an extra row and has a pagebreak at
			// the top.
			if (layout.labeltype == LABEL_COUNTER_CHAPTER) {
				if (buffer->params.secnumdepth >= 0) {
					float spacing_val = 1.0;
					if (!parparams.spacing().isDefault()) {
						spacing_val = parparams.spacing().getValue();
					} else {
						spacing_val = buffer->params.spacing.getValue();
					}
 
					int const maxdesc = 
						int(lyxfont::maxDescent(font) * layout.spacing.getValue() * spacing_val)
						+ int(layout.parsep) * defaultHeight();
 
					if (is_rtl) {
						x = ww - leftMargin(p.bv, p.row) - 
							lyxfont::width(str, font);
					}
 
					p.pain->text(int(x), p.yo +
						p.yo + p.row->baseline() - 
						p.row->ascent_of_text() - maxdesc,
						str, font);
				}
			} else {
				if (is_rtl) {
					x = ww - leftMargin(p.bv, p.row)
						+ lyxfont::width(layout.labelsep, font);
				} else
					x = p.x - lyxfont::width(layout.labelsep, font)
						- lyxfont::width(str, font);

				p.pain->text(int(x), p.yo + p.row->baseline(), str, font);
			}
		}
	// the labels at the top of an environment.
	// More or less for bibliography
	} else if (par->isFirstInSequence() &&
		(layout.labeltype == LABEL_TOP_ENVIRONMENT ||
		layout.labeltype == LABEL_BIBLIO ||
		layout.labeltype == LABEL_CENTERED_TOP_ENVIRONMENT)) {
		LyXFont font = getLabelFont(buffer, par);
		if (!par->getLabelstring().empty()) {
			string const str = par->getLabelstring();
			float spacing_val = 1.0;
			if (!parparams.spacing().isDefault()) {
				spacing_val = parparams.spacing().getValue();
			} else {
				spacing_val = buffer->params.spacing.getValue();
			}
 
			int maxdesc = 
				int(lyxfont::maxDescent(font) * layout.spacing.getValue() * spacing_val
				+ (layout.labelbottomsep * defaultHeight()));
			
			float x = p.x;
			if (layout.labeltype == LABEL_CENTERED_TOP_ENVIRONMENT) {
				x = ((is_rtl ? leftMargin(p.bv, p.row) : p.x)
					 + ww - rightMargin(buffer, p.row) ) / 2; 
				x -= lyxfont::width(str, font) / 2;
			} else if (is_rtl) {
				x = ww - leftMargin(p.bv, p.row) - 
					lyxfont::width(str, font);
			}
			p.pain->text(int(x), p.yo + p.row->baseline()
				  - p.row->ascent_of_text() - maxdesc,
				  str, font);
		}
	}
 
	if (layout.labeltype == LABEL_BIBLIO && par->bibkey) {
		LyXFont font = getLayoutFont(buffer, par);
		float x;
		if (is_rtl) {
			x = ww - leftMargin(p.bv, p.row)
				+ lyxfont::width(layout.labelsep, font);
		} else {
			x = p.x - lyxfont::width(layout.labelsep, font)
				- par->bibkey->width(p.bv, font);
		}
		par->bibkey->draw(p.bv, font, p.yo + p.row->baseline(), x, p.cleared);
	}
}
	
 
void LyXText::paintLastRow(DrawRowParams & p)
{
	Paragraph * par = p.row->par();
	ParagraphParameters const & parparams = par->params();
	int y_bottom = p.row->height();
	
	// think about the margins
	if (!p.row->next() && bv_owner)
		y_bottom -= LYX_PAPER_MARGIN;
	
	int const ww = p.bv->workWidth();
 
	// draw a bottom pagebreak
	if (parparams.pagebreakBottom()) {
		LyXFont pb_font;
		pb_font.setColor(LColor::pagebreak).decSize();
		int const y = p.yo + y_bottom - 2 * defaultHeight();
 
		p.pain->line(0, y, ww, y, LColor::pagebreak, Painter::line_onoffdash);
 
		int w = 0;
		int a = 0;
		int d = 0;
		lyxfont::rectText(_("Page Break (bottom)"), pb_font, w, a, d);
		p.pain->rectText((ww - w) / 2, y + d,
			_("Page Break (bottom)"),
			pb_font, backgroundColor(), backgroundColor());
 
		y_bottom -= 3 * defaultHeight();
	}
	
	// draw a vfill bottom
	if (parparams.spaceBottom().kind() == VSpace::VFILL) {
		int const x = LYX_PAPER_MARGIN / 2; 
		int const x2 = LYX_PAPER_MARGIN;
		int const y = p.yo + y_bottom - 3 * defaultHeight();
		int const y2 = p.yo + y_bottom - 2;
		
		p.pain->line(0, y, x2, y, LColor::vfillline);
		p.pain->line(0, y2, x2, y2, LColor::vfillline);
		p.pain->line(x, y, x, y2, LColor::vfillline);
 
		y_bottom -= 3 * defaultHeight();
	}
	
	// think about user added space
	y_bottom -= int(parparams.spaceBottom().inPixels(p.bv));
	
	Buffer const * buffer = p.bv->buffer();
 
	// draw a bottom line
	if (parparams.lineBottom()) {
		LyXFont font(LyXFont::ALL_SANE);
		int const asc = lyxfont::ascent('x',
			getFont(buffer, par,
			max(pos_type(0), par->size() - 1)));
 
		y_bottom -= asc;
 
		int const w = (inset_owner ?  inset_owner->width(p.bv, font) : ww);
		int const xp = static_cast<int>(inset_owner ? p.x : 0);
		int const y = p.yo + y_bottom; 
		p.pain->line(xp, y, w, y, LColor::topline, Painter::line_solid,
			  Painter::line_thick);
 
		y_bottom -= asc;
	}

	pos_type const last = rowLastPrintable(p.row);
	bool const is_rtl = p.row->par()->isRightToLeftPar(p.bv->buffer()->params);
	int const endlabel = par->getEndLabel(buffer->params);
 
	// draw an endlabel
	switch (endlabel) {
	case END_LABEL_BOX:
	case END_LABEL_FILLED_BOX:
	{
		LyXFont const font = getFont(buffer, par, last);
		int const size = int(0.75 * lyxfont::maxAscent(font));
		int const y = (p.yo + p.row->baseline()) - size;
		int x = is_rtl ? LYX_PAPER_MARGIN : ww - LYX_PAPER_MARGIN - size;

		if (p.row->fill() <= size)
			x += (size - p.row->fill() + 1) * (is_rtl ? -1 : 1);
 
		if (endlabel == END_LABEL_BOX) {
			p.pain->line(x, y, x, y + size, LColor::eolmarker);
			p.pain->line(x + size, y, x + size , y + size, LColor::eolmarker);
			p.pain->line(x, y, x + size, y, LColor::eolmarker);
			p.pain->line(x, y + size, x + size, y + size, LColor::eolmarker);
		} else {
			p.pain->fillRectangle(x, y, size, size, LColor::eolmarker);
		}
		break;
	}
	case END_LABEL_STATIC:
	{
		LyXFont font(LyXFont::ALL_SANE);
		LyXTextClass::LayoutList::size_type layout = par->getLayout();
		string const str = textclasslist.
			Style(buffer->params.textclass, layout).endlabelstring();
		font = getLabelFont(buffer, par);
		int const x = is_rtl ?
			int(p.x) - lyxfont::width(str, font)
			: ww - rightMargin(buffer, p.row) - p.row->fill();
		p.pain->text(x, p.yo + p.row->baseline(), str, font);
		break;
	}
	case END_LABEL_NO_LABEL:
		break;
	}
}

void LyXText::paintRowText(DrawRowParams & p)
{
	Paragraph * par = p.row->par();
	Buffer const * buffer = p.bv->buffer(); 
 
	pos_type const last = rowLastPrintable(p.row);
	pos_type main_body = 
		beginningOfMainBody(buffer, par);
	if (main_body > 0 && 
		(main_body - 1 > last || 
		!par->isLineSeparator(main_body - 1))) {
		main_body = 0;
	}
	
	LyXLayout const & layout =
		textclasslist.Style(buffer->params.textclass, par->getLayout());

	pos_type vpos = p.row->pos();
	while (vpos <= last) {
		pos_type pos = vis2log(vpos);
		if (main_body > 0 && pos == main_body - 1) {
			int const lwidth = lyxfont::width(layout.labelsep,
				getLabelFont(buffer, par));

			p.x += p.label_hfill + lwidth
				- singleWidth(p.bv, par, main_body - 1);
		}
		
		if (par->isHfill(pos)) {
			p.x += 1;

			int const y0 = p.yo + p.row->baseline();
			int const y1 = y0 - defaultHeight() / 2;

			p.pain->line(int(p.x), y1, int(p.x), y0,
				     LColor::vfillline);
			
			if (hfillExpansion(buffer, p.row, pos)) {
				int const y2 = (y0 + y1) / 2;
				
				if (pos >= main_body) {
					p.pain->line(int(p.x), y2,
						  int(p.x + p.hfill), y2,
						  LColor::vfillline,
						  Painter::line_onoffdash);
					p.x += p.hfill;
				} else {
					p.pain->line(int(p.x), y2,
						  int(p.x + p.label_hfill), y2,
						  LColor::vfillline,
						  Painter::line_onoffdash);
					p.x += p.label_hfill;
				}
				p.pain->line(int(p.x), y1,
					     int(p.x), y0,
					     LColor::vfillline);
			}
			p.x += 2;
			++vpos;
		} else if (par->isSeparator(pos)) {
			p.x += singleWidth(p.bv, par, pos);
			if (pos >= main_body)
				p.x += p.separator;
			++vpos;
		} else {
			draw(p.bv, p.row, vpos, p.yo, p.x, p.cleared);
		}
	}
}


void LyXText::getVisibleRow(BufferView * bv, int y_offset, int x_offset,
                            Row * row, int y, bool cleared)
{
	if (row->height() <= 0) {
		lyxerr << "LYX_ERROR: row.height: "
		       << row->height() << endl;
		return;
	}

	DrawRowParams p;

	// set up drawing parameters
	p.bv = bv;
	p.pain = &bv->painter();
	p.row = row;
	p.xo = x_offset;
	p.yo = y_offset;
	prepareToPrint(bv, row, p.x, p.separator, p.hfill, p.label_hfill);
	if (inset_owner && (p.x < 0))
		p.x = 0;
	p.x += p.xo;
	p.y = y;
	p.width = inset_owner ? inset_owner->textWidth(bv, true) : bv->workWidth();
	p.cleared = cleared;
	 
	// start painting

	// clear to background if necessary
	p.cleared = paintRowBackground(p);

	// paint the selection background
	if (selection.set()) {
		paintRowSelection(p);
	}

	// vertical lines for appendix
	paintRowAppendix(p);

	// environment depth brackets
	paintRowDepthBar(p);
 
	// draw any stuff wanted for a first row of a paragraph
	if (!row->pos()) {
		paintFirstRow(p);
	}

	// draw any stuff wanted for the last row of a paragraph
	if (!row->next() || (row->next()->par() != row->par())) {
		paintLastRow(p);
	} 

	// paint text
	paintRowText(p); 
}


int LyXText::defaultHeight() const
{
	LyXFont font(LyXFont::ALL_SANE);
	return int(lyxfont::maxAscent(font) + lyxfont::maxDescent(font) * 1.5);
}

   
/* returns the column near the specified x-coordinate of the row 
* x is set to the real beginning of this column  */ 
LyXText::pos_type
LyXText::getColumnNearX(BufferView * bview, Row * row, int & x,
			bool & boundary) const
{
	float tmpx = 0.0;
	float fill_separator;
	float fill_hfill;
	float fill_label_hfill;
   
	prepareToPrint(bview, row, tmpx, fill_separator,
		       fill_hfill, fill_label_hfill);

	pos_type vc = row->pos();
	pos_type last = rowLastPrintable(row);
	pos_type c = 0;
	LyXLayout const & layout =
		textclasslist.Style(bview->buffer()->params.textclass,
				    row->par()->getLayout());
	bool left_side = false;

	pos_type main_body = beginningOfMainBody(bview->buffer(), row->par());
	float last_tmpx = tmpx;
	
	if (main_body > 0 &&
	    (main_body - 1 > last || 
	     !row->par()->isLineSeparator(main_body - 1)))
		main_body = 0;
	
	while (vc <= last && tmpx <= x) {
		c = vis2log(vc);
		last_tmpx = tmpx;
		if (main_body > 0 && c == main_body-1) {
			tmpx += fill_label_hfill +
				lyxfont::width(layout.labelsep,
					       getLabelFont(bview->buffer(), row->par()));
			if (row->par()->isLineSeparator(main_body - 1))
				tmpx -= singleWidth(bview, row->par(), main_body-1);
		}
		
		if (hfillExpansion(bview->buffer(), row, c)) {
			x += singleWidth(bview, row->par(), c);
			if (c >= main_body)
				tmpx += fill_hfill;
			else
				tmpx += fill_label_hfill;
		}
		else if (row->par()->isSeparator(c)) {
			tmpx += singleWidth(bview, row->par(), c);
			if (c >= main_body)
				tmpx+= fill_separator;
		} else
			tmpx += singleWidth(bview, row->par(), c);
		++vc;
	}
	
	if ((tmpx + last_tmpx) / 2 > x) {
		tmpx = last_tmpx;
		left_side = true;
	}

	if (vc > last + 1)  // This shouldn't happen.
		vc = last + 1;

	boundary = false;
	bool const lastrow = lyxrc.rtl_support // This is not needed, but gives
		                         // some speedup if rtl_support=false
		&& (!row->next() || row->next()->par() != row->par());
	bool const rtl = (lastrow)
		? row->par()->isRightToLeftPar(bview->buffer()->params)
		: false; // If lastrow is false, we don't need to compute
	                 // the value of rtl.

	if (row->pos() > last)  // Row is empty?
		c = row->pos();
	else if (lastrow &&
		 ( ( rtl &&  left_side && vc == row->pos() && x < tmpx - 5) ||
		   (!rtl && !left_side && vc == last + 1   && x > tmpx + 5) ))
		c = last + 1;
	else if (vc == row->pos()) {
		c = vis2log(vc);
		if (bidi_level(c) % 2 == 1)
			++c;
	} else {
		c = vis2log(vc - 1);
		bool const rtl = (bidi_level(c) % 2 == 1);
		if (left_side == rtl) {
			++c;
			boundary = isBoundary(bview->buffer(), row->par(), c);
		}
	}

	if (row->pos() <= last && c > last
	    && row->par()->isNewline(last)) {
		if (bidi_level(last) % 2 == 0)
			tmpx -= singleWidth(bview, row->par(), last);
		else
			tmpx += singleWidth(bview, row->par(), last);
		c = last;
	}

	c -= row->pos();
	x = int(tmpx);
	return c;
}
 

// returns pointer to a specified row
Row * LyXText::getRow(Paragraph * par, pos_type pos, int & y) const
{
	if (!firstrow)
		return 0;
	
	Row * tmprow = firstrow;
	y = 0;
	
	// find the first row of the specified paragraph
	while (tmprow->next() && tmprow->par() != par) {
		y += tmprow->height();
		tmprow = tmprow->next();
	}
	
	// now find the wanted row
	while (tmprow->pos() < pos
	       && tmprow->next()
	       && tmprow->next()->par() == par
	       && tmprow->next()->pos() <= pos) {
		y += tmprow->height();
		tmprow = tmprow->next();
	}
	
	return tmprow;
}


Row * LyXText::getRowNearY(int & y) const
{
	// If possible we should optimize this method. (Lgb)
	Row * tmprow = firstrow;
	int tmpy = 0;
	
	while (tmprow->next() && tmpy + tmprow->height() <= y) {
		tmpy += tmprow->height();
		tmprow = tmprow->next();
	}
	
	y = tmpy;   // return the real y
	return tmprow;
}


int LyXText::getDepth() const
{
	return cursor.par()->getDepth();
}
