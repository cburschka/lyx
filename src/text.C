/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== */

#include <config.h>
#include <cstdlib>
#include <cctype>

#ifdef __GNUG__
#pragma implementation "table.h"
#endif

#include "layout.h"
#include "lyxparagraph.h"
#include "lyxtext.h"
#include "support/textutils.h"
#include "lyx_gui_misc.h"
#include "lyxdraw.h"
#include "gettext.h"
#include "bufferparams.h"
#include "buffer.h"
#include "lyxscreen.h"
#include "minibuffer.h"
#include "debug.h"
#include "lyxrc.h"
#include "LyXView.h"
#include "lyxrow.h"
#include "Painter.h"

using std::max;
using std::min;

static const int LYX_PAPER_MARGIN = 20;

extern int mono_video;
extern int reverse_video;
extern int fast_selection;
extern BufferView * current_view;
extern LyXRC * lyxrc;

// ale070405
#ifdef USE_PAINTER
extern int bibitemMaxWidth(Painter &, LyXFont const &);
#else
extern int bibitemMaxWidth(LyXFont const &);
#endif

#define FIX_DOUBLE_SPACE 1

int LyXText::SingleWidth(LyXParagraph * par,
			 LyXParagraph::size_type pos) const
{
	char c = par->GetChar(pos);
	return SingleWidth(par, pos, c);
}


#ifdef USE_PAINTER
int LyXText::SingleWidth(LyXParagraph * par,
			 LyXParagraph::size_type pos, char c) const
{
	LyXFont font = GetFont(par, pos);

	// The most common case is handled first (Asger)
	if (IsPrintable(c)) {
		return font.width(c);

	} else if (IsHfillChar(c)) {
		return 3;	/* Because of the representation
				 * as vertical lines */
	} else if (c == LyXParagraph::META_FOOTNOTE ||
		   c == LyXParagraph::META_MARGIN ||
		   c == LyXParagraph::META_FIG ||
		   c == LyXParagraph::META_TAB ||
		   c == LyXParagraph::META_WIDE_FIG ||
		   c == LyXParagraph::META_WIDE_TAB ||
		   c == LyXParagraph::META_ALGORITHM) {
		string fs;
		switch (c) {
		case LyXParagraph::META_MARGIN:
			fs = "margin";
			break;
		case LyXParagraph::META_FIG:
			fs = "fig";
			break;
		case LyXParagraph::META_TAB:
			fs = "tab";
			break;
		case LyXParagraph::META_ALGORITHM:
			fs = "alg";
			break;
		case LyXParagraph::META_WIDE_FIG:
			fs = "wide-fig";
			break;
		case LyXParagraph::META_WIDE_TAB:
			fs = "wide-tab";
			break;
		case LyXParagraph::META_FOOTNOTE:
			fs = "foot";
			break;
		}
		font.decSize();
		font.decSize();
		return font.stringWidth(fs);
	} else if (c == LyXParagraph::META_INSET) {
		Inset * tmpinset= par->GetInset(pos);
		if (tmpinset)
			return par->GetInset(pos)->width(owner_->painter(),
							 font);
		else
			return 0;

	} else if (IsSeparatorChar(c))
		c = ' ';
	else if (IsNewlineChar(c))
		c = 'n';
	return font.width(c);
}
#else
int LyXText::SingleWidth(LyXParagraph * par,
			 LyXParagraph::size_type pos, char c) const
{
	LyXFont font = GetFont(par, pos);

	// The most common case is handled first (Asger)
	if (IsPrintable(c)) {
		return font.width(c);

	} else if (IsHfillChar(c)) {
		return 3;	/* Because of the representation
				 * as vertical lines */

	} else if (c == LyXParagraph::META_FOOTNOTE ||
		   c == LyXParagraph::META_MARGIN ||
		   c == LyXParagraph::META_FIG ||
		   c == LyXParagraph::META_TAB ||
		   c == LyXParagraph::META_WIDE_FIG ||
		   c == LyXParagraph::META_WIDE_TAB ||
		   c == LyXParagraph::META_ALGORITHM) 
	{
		string fs;
		switch (c) {
		case LyXParagraph::META_MARGIN:
			fs = "margin";
			break;
		case LyXParagraph::META_FIG:
			fs = "fig";
			break;
		case LyXParagraph::META_TAB:
			fs = "tab";
			break;
		case LyXParagraph::META_ALGORITHM:
			fs = "alg";
			break;
		case LyXParagraph::META_WIDE_FIG:
			fs = "wide-fig";
			break;
		case LyXParagraph::META_WIDE_TAB:
			fs = "wide-tab";
			break;
		case LyXParagraph::META_FOOTNOTE:
			fs = "foot";
			break;
		}
		font.decSize();
		font.decSize();
		return font.stringWidth(fs);
	} 
   
	else if (c == LyXParagraph::META_INSET) {
		Inset *tmpinset= par->GetInset(pos);
		if (tmpinset)
			return par->GetInset(pos)->Width(font);
		else
			return 0;

	} else if (IsSeparatorChar(c))
		c = ' ';
	else if (IsNewlineChar(c))
		c = 'n';
	return font.width(c);
}
#endif


// Returns the paragraph position of the last character in the specified row
LyXParagraph::size_type LyXText::RowLast(Row const * row) const
{
	if (row->next == 0)
		return row->par->Last() - 1;
	else if (row->next->par != row->par) 
		return row->par->Last() - 1;
	else 
		return row->next->pos - 1;
}


LyXDirection BufferParams::getDocumentDirection() const
{
	return (lyxrc->rtl_support && language == "hebrew")
		? LYX_DIR_RIGHT_TO_LEFT : LYX_DIR_LEFT_TO_RIGHT;
}

LyXDirection LyXParagraph::getParDirection() const
{
	if (!lyxrc->rtl_support || table)
		return LYX_DIR_LEFT_TO_RIGHT;

	if (size() > 0)
		return (getFont(0).direction() ==  LyXFont::RTL_DIR)
			? LYX_DIR_RIGHT_TO_LEFT : LYX_DIR_LEFT_TO_RIGHT;
	else
		return current_view->buffer()->params.getDocumentDirection();
}

LyXDirection LyXFont::getFontDirection() const
{
	if (lyxrc->rtl_support 
	    && direction() == LyXFont::RTL_DIR
	    && latex() != LyXFont::ON)
		return LYX_DIR_RIGHT_TO_LEFT;
	else
		return LYX_DIR_LEFT_TO_RIGHT;
}

LyXDirection
LyXParagraph::getLetterDirection(LyXParagraph::size_type pos) const
{
	if (!lyxrc->rtl_support)
		return LYX_DIR_LEFT_TO_RIGHT;

	LyXDirection direction = getFont(pos).getFontDirection();
	if (IsLineSeparator(pos) && 0 < pos && pos < Last() - 1
	    && !IsLineSeparator(pos + 1)
	    && !(table && IsNewline(pos + 1))
	    && (getFont(pos - 1).getFontDirection() != direction
		|| getFont(pos + 1).getFontDirection() != direction))
		return getParDirection();
	else
		return direction;
}


void LyXText::ComputeBidiTables(Row * row) const
{

	if (!lyxrc->rtl_support) {
		bidi_start = -1;
		return;
	}
	LyXParagraph::size_type last = RowLast(row);
	bidi_start = row->pos;

	if (bidi_start > last) {
		bidi_start = -1;
		return;
	}

	if (last + 2 - bidi_start >
	    static_cast<LyXParagraph::size_type>(log2vis_list.size())) {
		LyXParagraph::size_type new_size = 
			(last + 2 - bidi_start < 500) ?
			500 : 2 * (last + 2 - bidi_start);
		log2vis_list.resize(new_size);
		vis2log_list.resize(new_size);
	}

	vis2log_list[last + 1 - bidi_start] = -1;
	log2vis_list[last + 1 - bidi_start] = -1;

	LyXParagraph::size_type main_body = BeginningOfMainBody(row->par);
	if (main_body > 0 && row->pos < main_body - 1 && main_body - 1 <= last
	    && row->par->IsLineSeparator(main_body - 1)) {
		// This is needed in case there is a direction change in
		// the label which is continued into the main body
		if (row->par->getParDirection() == LYX_DIR_LEFT_TO_RIGHT) {
			ComputeBidiTablesFromTo(row, bidi_start,
						main_body - 2, 0);
			log2vis_list[main_body - 1 - bidi_start] =
				main_body - 1;
			vis2log_list[main_body - 1 - bidi_start] =
				main_body - 1;
			if (main_body <= last)
				ComputeBidiTablesFromTo(row,
							main_body,last, 0);
		} else {
			ComputeBidiTablesFromTo(row, bidi_start,
						main_body - 2,
						last - main_body + 2);
			log2vis_list[main_body - 1 - bidi_start] =
				last - main_body + 1 + bidi_start;
			vis2log_list[last - main_body + 1 - bidi_start] =
				main_body - 1;
			if (main_body <= last)
				ComputeBidiTablesFromTo(row, main_body,
							last, -main_body);
		}
	} else
		ComputeBidiTablesFromTo(row, bidi_start, last, 0);
}


void LyXText::ComputeBidiTablesFromTo(Row * row,
				      LyXParagraph::size_type from,
				      LyXParagraph::size_type to,
				      LyXParagraph::size_type offset) const
{
	LyXParagraph::size_type vpos, old_lpos, stack[2];
	LyXDirection par_direction = row->par->getParDirection();
	LyXDirection direction = par_direction;
	LyXParagraph::size_type lpos = from;
	int level = 0;

	while (lpos <= to) {
		if (row->par->getLetterDirection(lpos) == direction) {
			log2vis_list[lpos - bidi_start] = direction;
			++lpos;
		} else {
			if (level == 0 ||
			    (level == 1 && direction == LYX_DIR_RIGHT_TO_LEFT
			     && row->par->getFont(lpos).direction() ==
			     LyXFont::RTL_DIR
			     && row->par->getFont(lpos).latex() ==
			     LyXFont::ON ) ) {
				// The last check is needed when the
				// char is a space
				stack[level++] = lpos;
			} else {
				old_lpos = stack[--level];
				log2vis_list[old_lpos - bidi_start] = 
					log2vis_list[lpos - bidi_start] =
					(old_lpos - lpos) * direction;
				++lpos;
			}
			direction = static_cast<LyXDirection>(-direction);
		}
	}

	while (level > 0) {
		old_lpos = stack[--level];
		log2vis_list[old_lpos - bidi_start] =
			(old_lpos - (to + 1)) * direction; 
		direction = static_cast<LyXDirection>(-direction);
	}

	vpos = (par_direction == LYX_DIR_LEFT_TO_RIGHT)
		? from - 1 : to + 1;
	vpos += offset;
	for (lpos = from; lpos <= to; ++lpos) {
		vpos += log2vis_list[lpos - bidi_start];
		vis2log_list[vpos - bidi_start] = lpos;
		log2vis_list[lpos - bidi_start] = vpos;
	}
}


#ifdef USE_PAINTER
void LyXText::draw(Row const * row,
		   LyXParagraph::size_type & vpos,
		   int offset, float & x)
{
	Painter & pain = owner_->painter();
	
	LyXParagraph::size_type pos = vis2log(vpos);
	char c = row->par->GetChar(pos);

	if (IsNewlineChar(c)) {
		++vpos;
		// Draw end-of-line marker
		LyXFont font = GetFont(row->par, pos);
		int wid = font.width('n');
		int asc = font.maxAscent();
		int y = offset + row->baseline;
#ifdef USE_PAINTER
		int xp[3], yp[3];
		
		if (row->par->getLetterDirection(pos) == LYX_DIR_LEFT_TO_RIGHT) {
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
#else
		XPoint p[3];
		
		if (row->par->getLetterDirection(pos) == LYX_DIR_LEFT_TO_RIGHT) {
			p[0].x = int(x + wid*0.375); p[0].y = int(y - 0.875*asc*0.75);
			p[1].x = int(x);	     p[1].y = int(y - 0.500*asc*0.75);
			p[2].x = int(x + wid*0.375); p[2].y = int(y - 0.125*asc*0.75);
			scr.drawLines(::getGC(gc_new_line), p, 3);
		
			p[0].x = int(x);	     p[0].y = int(y - 0.500*asc*0.75);
			p[1].x = int(x + wid);	     p[1].y = int(y - 0.500*asc*0.75);
			p[2].x = int(x + wid);	     p[2].y = int(y - asc*0.75);
			scr.drawLines(::getGC(gc_new_line), p, 3);
		} else {
			p[0].x = int(x + wid*0.625); p[0].y = int(y - 0.875*asc*0.75);
			p[1].x = int(x + wid);	     p[1].y = int(y - 0.500*asc*0.75);
			p[2].x = int(x + wid*0.625); p[2].y = int(y - 0.125*asc*0.75);
			scr.drawLines(::getGC(gc_new_line), p, 3);
		
			p[0].x = int(x + wid);	     p[0].y = int(y - 0.500*asc*0.75);
			p[1].x = int(x);	     p[1].y = int(y - 0.500*asc*0.75);
			p[2].x = int(x);	     p[2].y = int(y - asc*0.75);
			scr.drawLines(::getGC(gc_new_line), p, 3);
		}
#endif
		x += wid;
		return;
	}

	LyXFont font = GetFont(row->par, pos);
	LyXFont font2 = font;

	if (c == LyXParagraph::META_FOOTNOTE
	    || c == LyXParagraph::META_MARGIN
	    || c == LyXParagraph::META_FIG
	    || c == LyXParagraph::META_TAB
	    || c == LyXParagraph::META_WIDE_FIG
	    || c == LyXParagraph::META_WIDE_TAB
	    || c == LyXParagraph::META_ALGORITHM) {
		string fs;
		switch (c) {
		case LyXParagraph::META_MARGIN:
			fs = "margin";
			// Draw a sign at the left margin!
			owner_->painter()
				.text((LYX_PAPER_MARGIN - font.width('!'))/2,
				      offset + row->baseline, "!", 1, font);
			break;
		case LyXParagraph::META_FIG:
			fs = "fig";
			break;
		case LyXParagraph::META_TAB:
			fs = "tab";
			break;
		case LyXParagraph::META_ALGORITHM:
			fs = "alg";
			break;
		case LyXParagraph::META_WIDE_FIG:
			fs = "wide-fig";
			break;
		case LyXParagraph::META_WIDE_TAB:
			fs = "wide-tab";
			break;
		case LyXParagraph::META_FOOTNOTE:
			fs = "foot";
			break;
		}
		font.decSize();
		font.decSize();
	  
		// calculate the position of the footnotemark
		int y = (row->baseline - font2.maxAscent() 
			 + font.maxAscent());
	  
		font.setColor(LColor::footnote);

		float tmpx = x;

		// draw it and set new x position
		
		pain.text(int(x), offset + y, fs, font);
		x += pain.width(fs, font);
		pain.line(int(tmpx), offset + row->baseline,
			  int(x), offset + row->baseline,
			  LColor::footnote);

	  
		++vpos;
		return;
	} else if (c == LyXParagraph::META_INSET) {
		Inset * tmpinset = row->par->GetInset(pos);
		if (tmpinset) 
			tmpinset->draw(owner_->painter(), font,
				       offset + row->baseline, x);
		++vpos;
		return;
	}

	/* usual characters, no insets */

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

	LyXParagraph::size_type last = RowLast(row);
	
	while (vpos <= last &&
	       (pos = vis2log(vpos)) >= 0
	       && static_cast<unsigned char>(c = row->par->GetChar(pos)) > ' '
	       && font2 == GetFont(row->par, pos)) {
		textstring += c;
		++vpos;
	}
	float tmpx = x;

#if 0
	// If monochrome and LaTeX mode, provide reverse background
	if (mono_video &&
	    font.latex() == LyXFont::ON) {
		int a = font.maxAscent();
		int d = font.maxDescent();
		scr.fillRectangle(gc_copy, int(tmpx),
				  offset + row->baseline - a,
				  font.textWidth(textstring.c_str(),
						 textstring.length()), a + d);
	}
#endif

	// Draw text and set the new x position
	pain.text(int(x), offset + row->baseline, textstring, font);
	x += pain.width(textstring, font);
	
	// what about underbars?
	if (font.underbar() == LyXFont::ON && font.latex() != LyXFont::ON) {
		pain.line(tmpx, offset + row->baseline + 2,
			  x, offset + row->baseline + 2);
		
	}

	// If we want ulem.sty support, drawing
	// routines should go here. (Asger)
	// Why shouldn't LyXFont::drawText handle it internally?
}
#else
void LyXText::Draw(Row const * row, LyXParagraph::size_type & vpos,
		   LyXScreen & scr, int offset, float & x)
{
	LyXParagraph::size_type pos = vis2log(vpos);
	char c = row->par->GetChar(pos);

	if (IsNewlineChar(c)) {
		++vpos;
		// Draw end-of-line marker

		LyXFont font = GetFont(row->par, pos);
		int asc = font.maxAscent();
		int wid = font.width('n');
		int y = (offset + row->baseline);
		XPoint p[3];
		if (row->par->getLetterDirection(pos) == LYX_DIR_LEFT_TO_RIGHT) {
			p[0].x = int(x + wid*0.375); p[0].y = int(y - 0.875*asc*0.75);
			p[1].x = int(x);	     p[1].y = int(y - 0.500*asc*0.75);
			p[2].x = int(x + wid*0.375); p[2].y = int(y - 0.125*asc*0.75);
			scr.drawLines(::getGC(gc_new_line), p, 3);
		
			p[0].x = int(x);	     p[0].y = int(y - 0.500*asc*0.75);
			p[1].x = int(x + wid);	     p[1].y = int(y - 0.500*asc*0.75);
			p[2].x = int(x + wid);	     p[2].y = int(y - asc*0.75);
			scr.drawLines(::getGC(gc_new_line), p, 3);
		} else {
			p[0].x = int(x + wid*0.625); p[0].y = int(y - 0.875*asc*0.75);
			p[1].x = int(x + wid);	     p[1].y = int(y - 0.500*asc*0.75);
			p[2].x = int(x + wid*0.625); p[2].y = int(y - 0.125*asc*0.75);
			scr.drawLines(::getGC(gc_new_line), p, 3);
		
			p[0].x = int(x + wid);	     p[0].y = int(y - 0.500*asc*0.75);
			p[1].x = int(x);	     p[1].y = int(y - 0.500*asc*0.75);
			p[2].x = int(x);	     p[2].y = int(y - asc*0.75);
			scr.drawLines(::getGC(gc_new_line), p, 3);
		}
		x += wid;
		return;
	}

	LyXFont font = GetFont(row->par, pos);
	LyXFont font2 = font;

	if (c == LyXParagraph::META_FOOTNOTE
	    || c == LyXParagraph::META_MARGIN
	    || c == LyXParagraph::META_FIG
	    || c == LyXParagraph::META_TAB
	    || c == LyXParagraph::META_WIDE_FIG
	    || c == LyXParagraph::META_WIDE_TAB
	    || c == LyXParagraph::META_ALGORITHM) {
		string fs;
		switch (c) {
		case LyXParagraph::META_MARGIN:
			fs = "margin";
			// Draw a sign at the left margin! 
			scr.drawText(font, "!", 1, offset + row->baseline,
				     (LYX_PAPER_MARGIN - font.width('!'))/2);
			break;
		case LyXParagraph::META_FIG:
			fs = "fig";
			break;
		case LyXParagraph::META_TAB:
			fs = "tab";
			break;
		case LyXParagraph::META_ALGORITHM:
			fs = "alg";
			break;
		case LyXParagraph::META_WIDE_FIG:
			fs = "wide-fig";
			break;
		case LyXParagraph::META_WIDE_TAB:
			fs = "wide-tab";
			break;
		case LyXParagraph::META_FOOTNOTE:
			fs = "foot";
			break;
		}
		font.decSize();
		font.decSize();
	  
		// calculate the position of the footnotemark
		int y = (row->baseline - font2.maxAscent() 
			 + font.maxAscent());

#ifdef USE_PAINTER
		font.setColor(LColor::footnote);
#else
		font.setColor(LyXFont::RED);
#endif

		float tmpx = x;

		// draw it and set new x position
		x += scr.drawString(font, fs, offset + y, int(x));

		scr.drawLine(gc_foot, offset + row->baseline,
			     int(tmpx), int(x - tmpx));
	  
		++vpos;
		return;
	} else if (c == LyXParagraph::META_INSET) {
		Inset * tmpinset = row->par->GetInset(pos);
		if (tmpinset) 
			tmpinset->Draw(font, scr, offset + row->baseline, x);
		++vpos;
		return;
	}

	/* usual characters, no insets */

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

	LyXParagraph::size_type last = RowLast(row);
	
	while (vpos <= last &&
	       (pos = vis2log(vpos)) >= 0
	       && static_cast<unsigned char>(c = row->par->GetChar(pos)) > ' '
	       && font2 == GetFont(row->par, pos)) {
		textstring += c;
		++vpos;
	}
	float tmpx = x;

	// If monochrome and LaTeX mode, provide reverse background
	if (mono_video &&
	    font.latex() == LyXFont::ON) {
		int a = font.maxAscent();
		int d = font.maxDescent();
		scr.fillRectangle(gc_copy, int(tmpx),
				  offset + row->baseline - a,
				  font.textWidth(textstring.c_str(),
						 textstring.length()), a + d);
	}

	// Draw text and set the new x position
	x += scr.drawText(font, textstring.c_str(), textstring.length(),
			  offset + row->baseline, 
			  int(x));
	
	// what about underbars?
	if (font.underbar() == LyXFont::ON && font.latex() != LyXFont::ON) {
		scr.drawLine(gc_copy, offset + row->baseline + 2,
			     int(tmpx), int(x - tmpx));
	}

	// If we want ulem.sty support, drawing
	// routines should go here. (Asger)
	// Why shouldn't LyXFont::drawText handle it internally?
}
#endif


// Returns the left beginning of the text. 
// This information cannot be taken from the layouts-objekt, because in 
// LaTeX the beginning of the text fits in some cases (for example sections)
// exactly the label-width.
int LyXText::LeftMargin(Row const * row) const
{
	LyXLayout const & layout = textclasslist.Style(parameters->textclass,
						       row->par->GetLayout());
	
	string parindent = layout.parindent; 
	
	/* table stuff -- begin */ 
	if (row->par->table)
		parindent.clear();
	/* table stuff -- end */
	
	int x = LYX_PAPER_MARGIN;
	
	x += textclasslist.TextClass(parameters->textclass)
		.defaultfont()
		.signedStringWidth(textclasslist
				   .TextClass(parameters->textclass)
				   .leftmargin());
	
	if (row->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE)  {
		LyXFont font(LyXFont::ALL_SANE);
		font.setSize(LyXFont::SIZE_SMALL);
		x += font.textWidth("Mwide-figM", 10) + LYX_PAPER_MARGIN/2;
	}
	
	// this is the way, LyX handles the LaTeX-Environments.
	// I have had this idea very late, so it seems to be a
	// later added hack and this is true
	if (!row->par->GetDepth()) {
		if (!row->par->GetLayout()) {
			// find the previous same level paragraph
			if (row->par->FirstPhysicalPar()->Previous()) {
				LyXParagraph * newpar = row->par
					->DepthHook(row->par->GetDepth());
				if (newpar &&
				    textclasslist.Style(parameters->textclass,
							newpar->GetLayout())
				    .nextnoindent)
					parindent.clear();
			}
		}
	} else {
		// find the next level paragraph
		
		LyXParagraph * newpar = row->par->DepthHook(row->par->GetDepth()-1);
		
		// make a corresponding row. Needed to call LeftMargin()
		
		// check wether it is a sufficent paragraph 
		if (newpar && newpar->footnoteflag == row->par->footnoteflag
		    && textclasslist
		        .Style(parameters->textclass, 
			       newpar->GetLayout()).isEnvironment()) {
			Row dummyrow;
			dummyrow.par = newpar;
			dummyrow.pos = newpar->Last();
			x = LeftMargin(&dummyrow);
		} else {
			// this is no longer an error, because this function
			// is used to clear impossible depths after changing
			// a layout. Since there is always a redo,
			// LeftMargin() is always called
			row->par->FirstPhysicalPar()->depth = 0;
		}
		
		if (newpar && !row->par->GetLayout()) {
			if (newpar->FirstPhysicalPar()->noindent)
				parindent.clear();
			else
				parindent = textclasslist
					.Style(parameters->textclass, 
					       newpar->GetLayout()).parindent;
		}
		
	}
	
	LyXFont labelfont = GetFont(row->par, -2);
	switch (layout.margintype) {
	case MARGIN_DYNAMIC:
		if (!layout.leftmargin.empty()) {
			x += textclasslist
				.TextClass(parameters->textclass)
				.defaultfont()
				.signedStringWidth(layout.leftmargin);
		}
		if (!row->par->GetLabestring().empty()) {
			x += labelfont.signedStringWidth(layout.labelindent);
			x += labelfont.stringWidth(row->par->GetLabestring());
			x += labelfont.stringWidth(layout.labelsep);
		}
		break;
	case MARGIN_MANUAL:
		x += labelfont.signedStringWidth(layout.labelindent);
		if (row->pos >= BeginningOfMainBody(row->par)) {
			if (!row->par->GetLabelWidthString().empty()) {
				x += labelfont
					.stringWidth(row->par
						     ->GetLabelWidthString());
				x += labelfont.stringWidth(layout.labelsep);
			}
		}
		break;
	case MARGIN_STATIC:
		x += textclasslist.TextClass(parameters->textclass)
			.defaultfont().signedStringWidth(layout.leftmargin) * 4
			/ (row->par->GetDepth() + 4);
		break;
	case MARGIN_FIRST_DYNAMIC:
		if (layout.labeltype == LABEL_MANUAL) {
			if (row->pos >= BeginningOfMainBody(row->par)) {
				x += labelfont
					.signedStringWidth(layout.leftmargin);
			} else {
				x += labelfont
					.signedStringWidth(layout.labelindent);
			}
		} else if (row->pos
			   // Special case to fix problems with
			   // theorems (JMarc)
			   || (layout.labeltype == LABEL_STATIC
			       && layout.latextype == LATEX_ENVIRONMENT
			       && ! row->par->IsFirstInSequence())) {
			x += labelfont.signedStringWidth(layout.leftmargin);
		} else if (layout.labeltype != LABEL_TOP_ENVIRONMENT
			   && layout.labeltype != LABEL_BIBLIO
			   && layout.labeltype !=
			   LABEL_CENTERED_TOP_ENVIRONMENT) {
			x += labelfont.signedStringWidth(layout.labelindent);
			x += labelfont.stringWidth(layout.labelsep);
			x += labelfont.stringWidth(row->par->GetLabestring());
		} 
		break;
		
	case MARGIN_RIGHT_ADDRESS_BOX:
	{
		// ok, a terrible hack. The left margin depends on the widest
		// row in this paragraph. Do not care about footnotes, they
		// are *NOT* allowed in the LaTeX realisation of this layout.
		
		// find the first row of this paragraph
		Row const * tmprow = row;
		while (tmprow->previous && tmprow->previous->par == row->par)
			tmprow = tmprow->previous;
		
		int minfill = tmprow->fill;
		while (tmprow->next && tmprow->next->par == row->par) {
			tmprow = tmprow->next;
			if (tmprow->fill < minfill)
				minfill = tmprow->fill;
		}
		
		x += textclasslist.TextClass(parameters->textclass)
			.defaultfont().signedStringWidth(layout.leftmargin);
		x += minfill;
	}
	break;
	}
	if (row->par->pextra_type == LyXParagraph::PEXTRA_INDENT) {
		if (!row->par->pextra_widthp.empty()) {
			x += paperwidth *
				atoi(row->par->pextra_widthp.c_str()) / 100;
		} else if (!row->par->pextra_width.empty()) {
			int xx = VSpace(row->par->pextra_width).inPixels();
			
			if (xx > paperwidth)
				xx = paperwidth * 80 / 100;
			x += xx;
		} else { // should not happen
			LyXFont font(LyXFont::ALL_SANE);
			x += font.stringWidth("XXXXXX");
		}
	}
	
	int align;
	if (row->par->FirstPhysicalPar()->align == LYX_ALIGN_LAYOUT)
		align = layout.align;
	else
		align = row->par->FirstPhysicalPar()->align;
	
	// set the correct parindent
	if (row->pos == 0) {
		if ((layout.labeltype == LABEL_NO_LABEL 
		     || layout.labeltype == LABEL_TOP_ENVIRONMENT 
		     || layout.labeltype == LABEL_CENTERED_TOP_ENVIRONMENT
		     || (layout.labeltype == LABEL_STATIC
			 && layout.latextype == LATEX_ENVIRONMENT
			 && ! row->par->IsFirstInSequence()))
		    && row->par == row->par->FirstPhysicalPar()
		    && align == LYX_ALIGN_BLOCK
		    && !row->par->noindent
		    && (row->par->layout ||
			parameters->paragraph_separation ==
			BufferParams::PARSEP_INDENT))
			x += textclasslist.TextClass(parameters->textclass)
				.defaultfont().signedStringWidth(parindent);
		else 
			if (layout.labeltype == LABEL_BIBLIO) {
				// ale970405 Right width for bibitems
#ifdef USE_PAINTER
				x += bibitemMaxWidth(owner_->painter(),
						     textclasslist
						     .TextClass(parameters
								->textclass)
						     .defaultfont());
#else
				x += bibitemMaxWidth(textclasslist
						     .TextClass(parameters
								->textclass)
						     .defaultfont());
#endif
			}
	}
	
	return x;
}
    
   
int LyXText::RightMargin(Row const * row) const
{
	LyXLayout const & layout =
		textclasslist.Style(parameters->textclass,
				    row->par->GetLayout());
	
	int x = LYX_PAPER_MARGIN
		+ textclasslist
		.TextClass(parameters->textclass)
		.defaultfont()
		.signedStringWidth(textclasslist
				   .TextClass(parameters->textclass)
				   .rightmargin());
	
	if (row->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE)  {
		x += LYX_PAPER_MARGIN / 2;
	}
	
	// this is the way, LyX handles the LaTeX-Environments.
	// I have had this idea very late, so it seems to be a
	// later added hack and this is true
	if (row->par->GetDepth()) {
		// find the next level paragraph
		
		LyXParagraph * newpar = row->par;
		
		do {
			newpar = newpar->FirstPhysicalPar()->Previous();
			if (newpar) 
				newpar = newpar->FirstPhysicalPar();
		} while (newpar && newpar->GetDepth() >= row->par->GetDepth()
			 && newpar->footnoteflag == row->par->footnoteflag);
		
		// make a corresponding row. Needed to call LeftMargin()
		
		// check wether it is a sufficent paragraph
		if (newpar && newpar->footnoteflag == row->par->footnoteflag
		    && textclasslist.Style(parameters->textclass,
					   newpar->GetLayout())
		       .isEnvironment()) {
			Row dummyrow;
			dummyrow.par = newpar;
			dummyrow.pos = 0;
			x = RightMargin(&dummyrow);
		} else {
			// this is no longer an error, because this function
			// is used to clear impossible depths after changing
			// a layout. Since there is always a redo,
			// LeftMargin() is always called
			row->par->FirstPhysicalPar()->depth = 0;
		}
	}
	
	//lyxerr << "rightmargin: " << layout->rightmargin << endl;
	x += textclasslist.TextClass(parameters->textclass)
		.defaultfont()
		.signedStringWidth(layout.rightmargin) * 4
	      / (row->par->GetDepth() + 4);
	return x;
}


int LyXText::LabelEnd (Row const * row) const
{
	if (textclasslist.Style(parameters->textclass,
				row->par->GetLayout()).margintype
	    == MARGIN_MANUAL) {
		Row tmprow;
		tmprow = *row;
		tmprow.pos = row->par->Last();
		return LeftMargin(&tmprow);  /* just the beginning 
						of the main body */
	} else
		return 0;  /* LabelEnd is only needed, if the  
			      layout fills a flushleft
			      label. */
}


/* table stuff -- begin*/
int LyXText::NumberOfCell(LyXParagraph * par,
			  LyXParagraph::size_type pos) const
{
   int cell = 0;
   LyXParagraph::size_type tmp_pos = 0;
   while (tmp_pos < pos) {
      if (par->IsNewline(tmp_pos))
      	 ++cell;
      ++tmp_pos;
   }
   return cell;
}


int LyXText::WidthOfCell(LyXParagraph * par,
			 LyXParagraph::size_type & pos) const
{
   int w = 0;
   while (pos < par->Last() && !par->IsNewline(pos)) {
      w += SingleWidth(par, pos);
      ++pos;
   }
   if (par->IsNewline(pos))
      ++pos;
   return w;
}


bool LyXText::HitInTable(Row * row, int x) const
{
	float tmpx;
	float fill_separator, fill_hfill, fill_label_hfill;
	if (!row->par->table)
		return false;
	PrepareToPrint(row, tmpx, fill_separator,
		       fill_hfill, fill_label_hfill, false);
	return (x > tmpx && x < tmpx + row->par->table->WidthOfTable());
}


bool LyXText::MouseHitInTable(int x, long y) const
{
	Row * row = GetRowNearY(y);
        return HitInTable(row, x);
}


/* table stuff -- end*/


// get the next breakpoint in a given paragraph
LyXParagraph::size_type
LyXText::NextBreakPoint(Row const * row, int width) const
{
	LyXParagraph * par = row->par;
	LyXParagraph::size_type pos = row->pos;
	
	/* table stuff -- begin*/ 
	if (par->table) {
		while (pos < par->size()
		       && (!par->IsNewline(pos) 
			   || !par->table->IsFirstCell(NumberOfCell(par, pos+1)))) {
			if (par->GetChar(pos) == LyXParagraph::META_INSET &&
			    par->GetInset(pos) && par->GetInset(pos)->display()){
				par->GetInset(pos)->display(false);
			}
			++pos;
		}
		return pos;
	}
	/* table stuff -- end*/ 
	
	// position of the last possible breakpoint 
	// -1 isn't a suitable value, but a flag
	LyXParagraph::size_type last_separator = -1;
	int left_margin = LabelEnd(row);
	width -= RightMargin(row);
	
	LyXParagraph::size_type main_body = BeginningOfMainBody(par);
	LyXLayout const & layout =
		textclasslist.Style(parameters->textclass, par->GetLayout());
	LyXParagraph::size_type i = pos;

	if (layout.margintype == MARGIN_RIGHT_ADDRESS_BOX) {
		/* special code for right address boxes, only newlines count */
		while (i < par->Last()) {
			if (par->IsNewline(i)) {
				last_separator = i;
				i = par->Last() - 1; // this means break
				//x = width;
			} else if (par->GetChar(i) == LyXParagraph::META_INSET &&
				   par->GetInset(i) && par->GetInset(i)->display()){
				par->GetInset(i)->display(false);
			}
			++i;
		}
	} else {
		// Last position is an invariant
		LyXParagraph::size_type const last = 
			par->Last();
		// this is the usual handling
		int x = LeftMargin(row);
		while (x < width && i < last) {
			char c = par->GetChar(i);
			if (IsNewlineChar(c)) {
				last_separator = i;
				x = width; // this means break
			} else if (c == LyXParagraph::META_INSET &&
				   par->GetInset(i) && par->GetInset(i)->display()){
				// check wether a Display() inset is
				// valid here. if not, change it to
				// non-display
				if (layout.isCommand()
				    || (layout.labeltype == LABEL_MANUAL
					&& i < BeginningOfMainBody(par))){
					// display istn't allowd
					par->GetInset(i)->display(false);
					x += SingleWidth(par, i, c);
				} else {
					// inset is display. So break the line here
					if (i == pos){
						if (pos < last-1) {
							last_separator = i;
							if (IsLineSeparatorChar(par->GetChar(i+1)))
								++last_separator;
						} else
							last_separator = last; // to avoid extra rows
					} else
						last_separator = i - 1;
					x = width;  // this means break
				}
			} else  {
				if (IsLineSeparatorChar(c))
					last_separator = i;
				x += SingleWidth(par, i, c);
			}
			++i;
			if (i == main_body) {
				x += GetFont(par, -2).stringWidth(layout.labelsep);
				if (par->IsLineSeparator(i - 1))
					x-= SingleWidth(par, i - 1);
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
int LyXText::Fill(Row const * row, int paper_width) const
{
	int w, fill;
	// get the pure distance
	LyXParagraph::size_type last = RowLast(row);
	/* table stuff -- begin */
	if (row->par->table) {
		// for tables FILL does calculate the widthes of each cell in 
		// the row
		LyXParagraph::size_type pos = row->pos;
		int cell = NumberOfCell(row->par, pos);
		w = 0;
		do {
			row->par->table->SetWidthOfCell(cell,
							WidthOfCell(row->par,
								    pos));
			++cell;
		} while (pos <= last && !row->par->table->IsFirstCell(cell));
		// don't forget the very last table cell without characters
		if (cell == row->par->table->GetNumberOfCells() - 1)
			row->par->table->SetWidthOfCell(cell,
							WidthOfCell(row->par,
								    pos));
		
		return 0; /* width of table cannot be returned since
			   * we cannot guarantee its correct value at
			   * this point. */ 
	}
	/* table stuff -- end*/ 
	
	int left_margin = LabelEnd(row);
	
	// if the row ends with newline, this newline will not be relevant
	//if (last >= 0 && row->par->IsNewline(last))
	//	--last;
	
	// if the row ends with a space, this space will not be relevant
	if (last >= 0 && row->par->IsLineSeparator(last))
		--last;
	
	// special handling of the right address boxes
	if (textclasslist.Style(parameters->textclass,
				row->par->GetLayout()).margintype
	    == MARGIN_RIGHT_ADDRESS_BOX) {
		int tmpfill = row->fill;
		row->fill = 0; // the minfill in MarginLeft()
		w = LeftMargin(row);
		row->fill = tmpfill;
	} else
		w = LeftMargin(row);
	
	LyXLayout const & layout = textclasslist.Style(parameters->textclass,
						       row->par->GetLayout());
	LyXParagraph::size_type main_body = 
		BeginningOfMainBody(row->par);
	LyXParagraph::size_type i = row->pos;

	while (i <= last) {
		if (main_body > 0 && i == main_body) {
			w += GetFont(row->par, -2).
				stringWidth(layout.labelsep);
			if (row->par->IsLineSeparator(i - 1))
				w -= SingleWidth(row->par, i - 1);
			if (w < left_margin)
				w = left_margin;
		}
		w += SingleWidth(row->par, i);
		++i;
	}
	if (main_body > 0 && main_body > last) {
		w += GetFont(row->par, -2).stringWidth(layout.labelsep);
		if (last >= 0 && row->par->IsLineSeparator(last))
			w -= SingleWidth(row->par, last);
		if (w < left_margin)
			w = left_margin;
	}
	
	fill = paper_width - w - RightMargin(row);
	return fill;
}


// returns the minimum space a manual label needs on the screen in pixel
int LyXText::LabelFill(Row const * row) const
{
	LyXParagraph::size_type last = BeginningOfMainBody(row->par) - 1;
	// -1 because a label ends either with a space that is in the label, 
	// or with the beginning of a footnote that is outside the label.

	// I don't understand this code in depth, but sometimes "last" is
	// less than 0 and this causes a crash. This fix seems to work
	// correctly, but I bet the real error is elsewhere.  The bug is
	// triggered when you have an open footnote in a paragraph
	// environment with a manual label. (Asger)
	if (last < 0) last = 0;
	
	if (row->par->IsLineSeparator(last)) /* a sepearator at this end 
						does not count */
		--last;
	
	int w = 0;
	int i = row->pos;
	while (i <= last) {
		w += SingleWidth(row->par, i);
		++i;
	}
	
	int fill = 0;
	if (!row->par->labelwidthstring.empty()) {
		fill = GetFont(row->par, -2)
			.stringWidth(row->par->labelwidthstring) - w;
	}
	
	if (fill < 0)
		fill = 0;
	
	return fill;
}


// returns the number of separators in the specified row. The separator 
// on the very last column doesnt count
int LyXText::NumberOfSeparators(Row const * row) const
{
	int last = RowLast(row);
	//int p = row->pos;
	//int main_body = BeginningOfMainBody(row->par);
	//if (p < main_body)
	//	p = main_body;
	// I think this is equivalent to the above. (Lgb)
	int p = max(row->pos, BeginningOfMainBody(row->par));
	int n = 0;
	for (; p < last; ++p) {
		if (row->par->IsSeparator(p)) {
			++n;
		}
	}
	return n;
}


// returns the number of hfills in the specified row. The LyX-Hfill is
// a LaTeX \hfill so that the hfills at the beginning and at the end were 
// ignored. This is *MUCH* more usefull than not to ignore!
int LyXText::NumberOfHfills(Row const * row) const
{
	int last = RowLast(row);
	int first = row->pos;
	if (first) { /* hfill *DO* count at the beginning 
		      * of paragraphs! */
		while(first <= last && row->par->IsHfill(first))
			++first;
	}

	//int main_body = BeginningOfMainBody(row->par);
	//if (first < main_body)
	//	first = main_body;
	// I think this is equivalent to the above. (Lgb)
	first = max(first, BeginningOfMainBody(row->par));
	int n = 0;
	for (int p = first; p <= last; ++p) { // last, because the end is ignored!
		if (row->par->IsHfill(p)) {
			++n;
		}
	}
	return n;
}


// like NumberOfHfills, but only those in the manual label!
int LyXText::NumberOfLabelHfills(Row const * row) const
{
	LyXParagraph::size_type last = RowLast(row);
	LyXParagraph::size_type first = row->pos;
	if (first) { /* hfill *DO* count at the beginning 
		      * of paragraphs! */
		while(first < last && row->par->IsHfill(first))
			++first;
	}
	//LyXParagraph::size_type main_body = 
	//BeginningOfMainBody(row->par);
	//if (last > main_body)
	//last = main_body;
	// I think this is eqvialent to the above. (Lgb)
	last = min(last, BeginningOfMainBody(row->par));
	int n = 0;
	for (LyXParagraph::size_type p = first;
	     p < last; ++p) {  // last, because the end is ignored!
		if (row->par->IsHfill(p)) {
			++n;
		}
	}
	return n;
}


// returns true, if a expansion is needed.
// Rules are given by LaTeX
bool LyXText::HfillExpansion(Row const * row_ptr,
			     LyXParagraph::size_type pos) const
{
	// by the way, is it a hfill?
	if (!row_ptr->par->IsHfill(pos))
		return false;
	
	// at the end of a row it does not count
	if (pos >= RowLast(row_ptr))
		return false;
	
	// at the beginning of a row it does not count, if it is not 
	// the first row of a paragaph
	if (!row_ptr->pos)
		return true;
	
	// in some labels  it does not count
	if (textclasslist.Style(parameters->textclass,
				row_ptr->par->GetLayout()).margintype
	    != MARGIN_MANUAL
	    && pos < BeginningOfMainBody(row_ptr->par))
		return false; 
	
	// if there is anything between the first char of the row and
	// the sepcified position that is not a newline and not a hfill,
	// the hfill will count, otherwise not
	LyXParagraph::size_type i = row_ptr->pos;
	while (i < pos && (row_ptr->par->IsNewline(i)
			   || row_ptr->par->IsHfill(i)))
		++i;
	
	return i != pos;
}


#ifdef USE_PAINTER
void LyXText::SetHeightOfRow(Row * row_ptr) const
{
    /* get the maximum ascent and the maximum descent */
   int asc, desc, pos;
   float layoutasc = 0;
   float layoutdesc = 0;
   float tmptop = 0;
   LyXFont tmpfont;
   Inset * tmpinset;

   /* this must not happen before the currentrow for clear reasons.
      so the trick is just to set the current row onto this row */
   long unused_y;
   GetRow(row_ptr->par, row_ptr->pos, unused_y);

   /* ok , let us initialize the maxasc and maxdesc value. 
    * This depends in LaTeX of the font of the last character
    * in the paragraph. The hack below is necessary because
    * of the possibility of open footnotes */

   /* Correction: only the fontsize count. The other properties
      are taken from the layoutfont. Nicer on the screen :) */
   
   LyXParagraph * par = row_ptr->par->LastPhysicalPar();
   LyXParagraph * firstpar = row_ptr->par->FirstPhysicalPar();
   
   LyXLayout const & layout = textclasslist.Style(parameters->textclass,
						  firstpar->GetLayout());
   
   LyXFont font = GetFont(par, par->Last()-1);
   LyXFont::FONT_SIZE size = font.size();
   font = GetFont(par, -1);
   font.setSize(size);

   LyXFont labelfont = GetFont(par, -2);

   int maxasc = int(font.maxAscent() *
		   layout.spacing.getValue() *
		   parameters->spacing.getValue());
   int maxdesc = int(font.maxDescent() *
		    layout.spacing.getValue() *
		    parameters->spacing.getValue());

   int pos_end = RowLast(row_ptr);
   
   int labeladdon = 0;

   // Check if any insets are larger
   for (pos = row_ptr->pos; pos <= pos_end; ++pos) {
      if (row_ptr->par->GetChar(pos) == LyXParagraph::META_INSET) {
	 tmpfont = GetFont(row_ptr->par, pos);
         tmpinset = row_ptr->par->GetInset(pos);
         if (tmpinset) {
            asc = tmpinset->ascent(owner_->painter(), tmpfont);
            desc = tmpinset->descent(owner_->painter(), tmpfont);
	    if (asc > maxasc) 
	      maxasc = asc;
	    if (desc > maxdesc)
	      maxdesc = desc;
	 }
      }
   }

   // Check if any custom fonts are larger (Asger)
   // This is not completely correct, but we can live with the small,
   // cosmetic error for now.
   LyXFont::FONT_SIZE maxsize = row_ptr->par->HighestFontInRange(row_ptr->pos,
								 pos_end);
   if (maxsize > font.size()) {
	font.setSize(maxsize);

	asc = font.maxAscent();
	desc = font.maxDescent();
	if (asc > maxasc) 
		maxasc = asc;
	if (desc > maxdesc)
		maxdesc = desc;
   }

   /* table stuff -- begin*/
   if (row_ptr->par->table){
     // stretch the rows a bit
      maxasc += 1;
      maxdesc += 1;
   }
   /* table stuff -- end*/

   // This is nicer with box insets:
   ++maxasc;
   ++maxdesc;

   row_ptr->ascent_of_text = maxasc;
   
   /* is it a top line? */ 
   if (row_ptr->pos == 0
       && row_ptr->par == firstpar) {
      
      /* some parksips VERY EASY IMPLEMENTATION */ 
      if (parameters->paragraph_separation == BufferParams::PARSEP_SKIP) {
	 if (layout.isParagraph()
	     && firstpar->GetDepth() == 0
	     && firstpar->Previous())
	    maxasc += parameters->getDefSkip().inPixels();
	 else if (firstpar->Previous()
		  && textclasslist.Style(parameters->textclass,
			   firstpar->Previous()->GetLayout()).isParagraph()
		  && firstpar->Previous()->GetDepth() == 0)
	   // is it right to use defskip here too? (AS)
	   maxasc += parameters->getDefSkip().inPixels();
      }
      
      /* the paper margins */ 
      if (!row_ptr->par->previous)
      	 maxasc += LYX_PAPER_MARGIN;
      
      /* add the vertical spaces, that the user added */
      if (firstpar->added_space_top.kind() != VSpace::NONE)
      	 maxasc += int(firstpar->added_space_top.inPixels());
      
      /* do not forget the DTP-lines! 
       * there height depends on the font of the nearest character */
      if (firstpar->line_top)
      	 maxasc += 2 * GetFont(firstpar, 0).ascent('x');
      
      /* and now the pagebreaks */ 
      if (firstpar->pagebreak_top)
      	 maxasc += 3 * DefaultHeight();
      
      /*  this is special code for the chapter, since the label of this
       * layout is printed in an extra row */ 
      if (layout.labeltype == LABEL_COUNTER_CHAPTER
	  && parameters->secnumdepth>= 0) {
	      labeladdon = int(labelfont.maxDescent() *
				  layout.spacing.getValue() *
				  parameters->spacing.getValue())
		      + int(labelfont.maxAscent() *
			       layout.spacing.getValue() *
			       parameters->spacing.getValue());
      }
      
      /* special code for the top label */ 
      if ((layout.labeltype == LABEL_TOP_ENVIRONMENT
	   || layout.labeltype == LABEL_BIBLIO
	   || layout.labeltype == LABEL_CENTERED_TOP_ENVIRONMENT)
	  && row_ptr->par->IsFirstInSequence()
	  && !row_ptr->par->GetLabestring().empty()) {
	 labeladdon = int(
		 (labelfont.maxAscent() *
		  layout.spacing.getValue() *
		  parameters->spacing.getValue())
		 +(labelfont.maxDescent() *
		   layout.spacing.getValue() *
		   parameters->spacing.getValue())
		 + layout.topsep * DefaultHeight()
		 + layout.labelbottomsep *  DefaultHeight());
      }
   
      /* and now the layout spaces, for example before and after a section, 
       * or between the items of a itemize or enumerate environment */ 
      
      if (!firstpar->pagebreak_top) {
	 LyXParagraph * prev = row_ptr->par->Previous();
	 if (prev)
	    prev = row_ptr->par->DepthHook(row_ptr->par->GetDepth());
	 if (prev && prev->GetLayout() == firstpar->GetLayout()
	     && prev->GetDepth() == firstpar->GetDepth()
	     && prev->GetLabelWidthString() == firstpar->GetLabelWidthString())
	   {
	      layoutasc = (layout.itemsep * DefaultHeight());
	   }
	 else if (row_ptr->previous) {
	    tmptop = layout.topsep;
	    
	    if (row_ptr->previous->par->GetDepth() >= row_ptr->par->GetDepth())
	       tmptop-= textclasslist.Style(parameters->textclass, row_ptr->previous->par->GetLayout()).bottomsep;
	    
	    if (tmptop > 0)
	       layoutasc = (tmptop * DefaultHeight());
	 }
	 else if (row_ptr->par->line_top){
	    tmptop = layout.topsep;
	    
	    if (tmptop > 0)
	       layoutasc = (tmptop * DefaultHeight());
	 }
	 
	 prev = row_ptr->par->DepthHook(row_ptr->par->GetDepth()-1);
	 if (prev)  {
	    maxasc += int(textclasslist.Style(parameters->textclass,
					 prev->GetLayout()).parsep * DefaultHeight());
	 }
	 else {
		if (firstpar->Previous()
		    && firstpar->Previous()->GetDepth() == 0
		    && firstpar->Previous()->GetLayout() != firstpar->GetLayout()) {
		   /* avoid parsep */ 
		}
	    else if (firstpar->Previous()){
	       maxasc += int(layout.parsep * DefaultHeight());
	    }
	 }
      }
   }
   
   /* is it a bottom line? */ 
   if (row_ptr->par->ParFromPos(RowLast(row_ptr) + 1) == par
       && (!row_ptr->next || row_ptr->next->par != row_ptr->par)) {     
	  
	  /* the paper margins */ 
	  if (!par->next)
	    maxdesc += LYX_PAPER_MARGIN;
	
	  /* add the vertical spaces, that the user added */
	  if (firstpar->added_space_bottom.kind() != VSpace::NONE)
	    maxdesc += int(firstpar->added_space_bottom.inPixels());
	  
	  /* do not forget the DTP-lines! 
	   * there height depends on the font of the nearest character */
	  if (firstpar->line_bottom)
	    maxdesc += 2 * (GetFont(par, par->Last()-1).ascent('x'));
	  
	  /* and now the pagebreaks */
	  if (firstpar->pagebreak_bottom)
	    maxdesc += 3 * DefaultHeight();
	  
	  /* and now the layout spaces, for example before and after a section, 
	   * or between the items of a itemize or enumerate environment */
	  if (!firstpar->pagebreak_bottom && row_ptr->par->Next()) {
	     LyXParagraph * nextpar = row_ptr->par->Next();
	     LyXParagraph * comparepar = row_ptr->par;
	     float usual = 0;
	     float  unusual = 0;
	     
	     if (comparepar->GetDepth() > nextpar->GetDepth()) {
		usual = (textclasslist.Style(parameters->textclass, comparepar->GetLayout()).bottomsep * DefaultHeight());
		comparepar = comparepar->DepthHook(nextpar->GetDepth());
		if (comparepar->GetLayout()!= nextpar->GetLayout()
		    || nextpar->GetLabelWidthString() != 
		    	comparepar->GetLabelWidthString())
		  unusual = (textclasslist.Style(parameters->textclass, comparepar->GetLayout()).bottomsep * DefaultHeight());
		
		if (unusual > usual)
		  layoutdesc = unusual;
		else
		  layoutdesc = usual;
	     }
	     else if (comparepar->GetDepth() ==  nextpar->GetDepth()) {
		
		if (comparepar->GetLayout()!= nextpar->GetLayout()
		    || nextpar->GetLabelWidthString() != 
			comparepar->GetLabelWidthString())
		  layoutdesc = int(textclasslist.Style(parameters->textclass, comparepar->GetLayout()).bottomsep * DefaultHeight());
	     }
	  }
       }
   
   /* incalculate the layout spaces */ 
   maxasc += int(layoutasc * 2 / (2 + firstpar->GetDepth()));
   maxdesc += int(layoutdesc * 2 / (2 + firstpar->GetDepth()));

   /* table stuff -- begin*/
   if (row_ptr->par->table){
      maxasc += row_ptr->par->table->
	AdditionalHeight(NumberOfCell(row_ptr->par, row_ptr->pos));
   }
   /* table stuff -- end*/
   
   /* calculate the new height of the text */ 
   height -= row_ptr->height;
   
   row_ptr->height = maxasc + maxdesc + labeladdon;
   row_ptr->baseline = maxasc + labeladdon;
   
   height += row_ptr->height;
}
#else
void LyXText::SetHeightOfRow(Row * row_ptr) const
{
    /* get the maximum ascent and the maximum descent */
   int asc, maxasc, desc, maxdesc, pos_end, pos, labeladdon;
   float layoutasc = 0;
   float layoutdesc = 0;
   float tmptop = 0;
   LyXFont tmpfont;
   Inset * tmpinset;

   /* this must not happen before the currentrow for clear reasons.
      so the trick is just to set the current row onto this row */
   long unused_y;
   GetRow(row_ptr->par, row_ptr->pos, unused_y);

   /* ok , let us initialize the maxasc and maxdesc value. 
    * This depends in LaTeX of the font of the last character
    * in the paragraph. The hack below is necessary because
    * of the possibility of open footnotes */

   /* Correction: only the fontsize count. The other properties
      are taken from the layoutfont. Nicer on the screen :) */
   
   LyXParagraph * par = row_ptr->par->LastPhysicalPar();
   LyXParagraph * firstpar = row_ptr->par->FirstPhysicalPar();
   
   LyXLayout const & layout = textclasslist.Style(parameters->textclass, firstpar->GetLayout());
   
   LyXFont font = GetFont(par, par->Last()-1);
   LyXFont::FONT_SIZE size = font.size();
   font = GetFont(par, -1);
   font.setSize(size);

   LyXFont labelfont = GetFont(par, -2);

   maxasc = int(font.maxAscent() *
		   layout.spacing.getValue() *
		   parameters->spacing.getValue());
   maxdesc = int(font.maxDescent() *
		    layout.spacing.getValue() *
		    parameters->spacing.getValue());

   pos_end = RowLast(row_ptr);
   
   labeladdon = 0;

   // Check if any insets are larger
   for (pos = row_ptr->pos; pos <= pos_end; ++pos) {
      if (row_ptr->par->GetChar(pos) == LyXParagraph::META_INSET) {
	 tmpfont = GetFont(row_ptr->par, pos);
         tmpinset = row_ptr->par->GetInset(pos);
         if (tmpinset) {
            asc = tmpinset->Ascent(tmpfont);
            desc = tmpinset->Descent(tmpfont);
	    if (asc > maxasc) 
	      maxasc = asc;
	    if (desc > maxdesc)
	      maxdesc = desc;
	 }
      }
   }

   // Check if any custom fonts are larger (Asger)
   // This is not completely correct, but we can live with the small,
   // cosmetic error for now.
   LyXFont::FONT_SIZE maxsize = row_ptr->par->HighestFontInRange(row_ptr->pos,
								 pos_end);
   if (maxsize > font.size()) {
	font.setSize(maxsize);

	asc = font.maxAscent();
	desc = font.maxDescent();
	if (asc > maxasc) 
		maxasc = asc;
	if (desc > maxdesc)
		maxdesc = desc;
   }

   /* table stuff -- begin*/
   if (row_ptr->par->table){
     // stretch the rows a bit
      maxasc += 1;
      maxdesc += 1;
   }
   /* table stuff -- end*/

   // This is nicer with box insets:
   ++maxasc;
   ++maxdesc;

   row_ptr->ascent_of_text = maxasc;
   
   /* is it a top line? */ 
   if (row_ptr->pos == 0
       && row_ptr->par == firstpar) {
      
      /* some parksips VERY EASY IMPLEMENTATION */ 
      if (parameters->paragraph_separation == BufferParams::PARSEP_SKIP) {
	 if (layout.isParagraph()
	     && firstpar->GetDepth() == 0
	     && firstpar->Previous())
	    maxasc += parameters->getDefSkip().inPixels();
	 else if (firstpar->Previous()
		  && textclasslist.Style(parameters->textclass,
			   firstpar->Previous()->GetLayout()).isParagraph()
		  && firstpar->Previous()->GetDepth() == 0)
	   // is it right to use defskip here too? (AS)
	   maxasc += parameters->getDefSkip().inPixels();
      }
      
      /* the paper margins */ 
      if (!row_ptr->par->previous)
      	 maxasc += LYX_PAPER_MARGIN;
      
      /* add the vertical spaces, that the user added */
      if (firstpar->added_space_top.kind() != VSpace::NONE)
      	 maxasc += int(firstpar->added_space_top.inPixels());
      
      /* do not forget the DTP-lines! 
       * there height depends on the font of the nearest character */
      if (firstpar->line_top)
      	 maxasc += 2 * GetFont(firstpar, 0).ascent('x');
      
      /* and now the pagebreaks */ 
      if (firstpar->pagebreak_top)
      	 maxasc += 3 * DefaultHeight();
      
      /*  this is special code for the chapter, since the label of this
       * layout is printed in an extra row */ 
      if (layout.labeltype == LABEL_COUNTER_CHAPTER
	  && parameters->secnumdepth>= 0) {
	      labeladdon = int(labelfont.maxDescent() *
				  layout.spacing.getValue() *
				  parameters->spacing.getValue())
		      + int(labelfont.maxAscent() *
			       layout.spacing.getValue() *
			       parameters->spacing.getValue());
      }
      
      /* special code for the top label */ 
      if ((layout.labeltype == LABEL_TOP_ENVIRONMENT
	   || layout.labeltype == LABEL_BIBLIO
	   || layout.labeltype == LABEL_CENTERED_TOP_ENVIRONMENT)
	  && row_ptr->par->IsFirstInSequence()
	  && !row_ptr->par->GetLabestring().empty()) {
	 labeladdon = int(
		 (labelfont.maxAscent() *
		  layout.spacing.getValue() *
		  parameters->spacing.getValue())
		 +(labelfont.maxDescent() *
		   layout.spacing.getValue() *
		   parameters->spacing.getValue())
		 + layout.topsep * DefaultHeight()
		 + layout.labelbottomsep *  DefaultHeight());
      }
   
      /* and now the layout spaces, for example before and after a section, 
       * or between the items of a itemize or enumerate environment */ 
      
      if (!firstpar->pagebreak_top) {
	 LyXParagraph *prev = row_ptr->par->Previous();
	 if (prev)
	    prev = row_ptr->par->DepthHook(row_ptr->par->GetDepth());
	 if (prev && prev->GetLayout() == firstpar->GetLayout()
	     && prev->GetDepth() == firstpar->GetDepth()
	     && prev->GetLabelWidthString() == firstpar->GetLabelWidthString())
	   {
	      layoutasc = (layout.itemsep * DefaultHeight());
	   }
	 else if (row_ptr->previous) {
	    tmptop = layout.topsep;
	    
	    if (row_ptr->previous->par->GetDepth() >= row_ptr->par->GetDepth())
	       tmptop-= textclasslist.Style(parameters->textclass, row_ptr->previous->par->GetLayout()).bottomsep;
	    
	    if (tmptop > 0)
	       layoutasc = (tmptop * DefaultHeight());
	 }
	 else if (row_ptr->par->line_top){
	    tmptop = layout.topsep;
	    
	    if (tmptop > 0)
	       layoutasc = (tmptop * DefaultHeight());
	 }
	 
	 prev = row_ptr->par->DepthHook(row_ptr->par->GetDepth()-1);
	 if (prev)  {
	    maxasc += int(textclasslist.Style(parameters->textclass,
					 prev->GetLayout()).parsep * DefaultHeight());
	 }
	 else {
		if (firstpar->Previous()
		    && firstpar->Previous()->GetDepth() == 0
		    && firstpar->Previous()->GetLayout() != firstpar->GetLayout()) {
		   /* avoid parsep */ 
		}
	    else if (firstpar->Previous()){
	       maxasc += int(layout.parsep * DefaultHeight());
	    }
	 }
      }
   }
   
   /* is it a bottom line? */ 
   if (row_ptr->par->ParFromPos(RowLast(row_ptr) + 1) == par
       && (!row_ptr->next || row_ptr->next->par != row_ptr->par)) {     
	  
	  /* the paper margins */ 
	  if (!par->next)
	    maxdesc += LYX_PAPER_MARGIN;
	
	  /* add the vertical spaces, that the user added */
	  if (firstpar->added_space_bottom.kind() != VSpace::NONE)
	    maxdesc += int(firstpar->added_space_bottom.inPixels());
	  
	  /* do not forget the DTP-lines! 
	   * there height depends on the font of the nearest character */
	  if (firstpar->line_bottom)
	    maxdesc += 2 * (GetFont(par, par->Last()-1).ascent('x'));
	  
	  /* and now the pagebreaks */
	  if (firstpar->pagebreak_bottom)
	    maxdesc += 3 * DefaultHeight();
	  
	  /* and now the layout spaces, for example before and after a section, 
	   * or between the items of a itemize or enumerate environment */
	  if (!firstpar->pagebreak_bottom && row_ptr->par->Next()) {
	     LyXParagraph *nextpar = row_ptr->par->Next();
	     LyXParagraph *comparepar = row_ptr->par;
	     float usual = 0;
	     float  unusual = 0;
	     
	     if (comparepar->GetDepth() > nextpar->GetDepth()) {
		usual = (textclasslist.Style(parameters->textclass, comparepar->GetLayout()).bottomsep * DefaultHeight());
		comparepar = comparepar->DepthHook(nextpar->GetDepth());
		if (comparepar->GetLayout()!= nextpar->GetLayout()
		    || nextpar->GetLabelWidthString() != 
		    	comparepar->GetLabelWidthString())
		  unusual = (textclasslist.Style(parameters->textclass, comparepar->GetLayout()).bottomsep * DefaultHeight());
		
		if (unusual > usual)
		  layoutdesc = unusual;
		else
		  layoutdesc = usual;
	     }
	     else if (comparepar->GetDepth() ==  nextpar->GetDepth()) {
		
		if (comparepar->GetLayout()!= nextpar->GetLayout()
		    || nextpar->GetLabelWidthString() != 
			comparepar->GetLabelWidthString())
		  layoutdesc = int(textclasslist.Style(parameters->textclass, comparepar->GetLayout()).bottomsep * DefaultHeight());
	     }
	  }
       }
   
   /* incalculate the layout spaces */ 
   maxasc += int(layoutasc * 2 / (2 + firstpar->GetDepth()));
   maxdesc += int(layoutdesc * 2 / (2 + firstpar->GetDepth()));

   /* table stuff -- begin*/
   if (row_ptr->par->table){
      maxasc += row_ptr->par->table->
	AdditionalHeight(NumberOfCell(row_ptr->par, row_ptr->pos));
   }
   /* table stuff -- end*/
   
   /* calculate the new height of the text */ 
   height -= row_ptr->height;
   
   row_ptr->height= maxasc+maxdesc+labeladdon;
   row_ptr->baseline= maxasc+labeladdon;
   
   height += row_ptr->height;
}
#endif


/* Appends the implicit specified paragraph behind the specified row,
 * start at the implicit given position */
void LyXText::AppendParagraph(Row * row) const
{
   bool not_ready = true;
   
   // The last character position of a paragraph is an invariant so we can 
   // safely get it here. (Asger)
   int lastposition = row->par->Last();

   do {
      // Get the next breakpoint
      int z = NextBreakPoint(row, paperwidth);
      
      Row * tmprow = row;

      // Insert the new row
      if (z < lastposition) {
	 ++z;
	 InsertRow(row, row->par, z);
	 row = row->next;

	 row->height = 0;
      } else
	 not_ready = false;
      
      // Set the dimensions of the row
      tmprow->fill = Fill(tmprow, paperwidth);
      SetHeightOfRow(tmprow);

   } while (not_ready);
}


void LyXText::BreakAgain(Row * row) const
{
   bool not_ready = true;
   
   do  {
      /* get the next breakpoint */
	LyXParagraph::size_type z = 
		NextBreakPoint(row, paperwidth);
      Row * tmprow = row;
      
      if (z < row->par->Last() ) {
	 if (!row->next || (row->next && row->next->par != row->par)) {
		 // insert a new row
	    ++z;
	    InsertRow(row, row->par, z);
	    row = row->next;
	    row->height = 0;
	 } else  {
	    row = row->next;
	    ++z;
	    if (row->pos == z)
		    not_ready = false;     // the rest will not change
	    else {
	       row->pos = z;
	    }
	 }
      } else {
	 /* if there are some rows too much, delete them */
	 /* only if you broke the whole paragraph! */ 
	 Row * tmprow2 = row;
	 while (tmprow2->next && tmprow2->next->par == row->par) {
	    tmprow2 = tmprow2->next;
	 }
	 while (tmprow2 != row) {
	    tmprow2 = tmprow2->previous;
	    RemoveRow(tmprow2->next);
	 }
	 not_ready = false;
      }
       
      /* set the dimensions of the row */ 
      tmprow->fill = Fill(tmprow, paperwidth);
      SetHeightOfRow(tmprow);
   } while (not_ready);
}


/* this is just a little changed version of break again */ 
void LyXText::BreakAgainOneRow(Row * row)
{
   /* get the next breakpoint */
   LyXParagraph::size_type z = NextBreakPoint(row, paperwidth);
   Row * tmprow = row;
   
   if (z < row->par->Last() ) {
      if (!row->next || (row->next && row->next->par != row->par)) {
	 /* insert a new row */ 
	 ++z;
	 InsertRow(row, row->par, z);
	 row = row->next;
	 row->height = 0;
      }
      else  {
	 row= row->next;
	 ++z;
	 if (row->pos != z)
	    row->pos = z;
      }
   }
   else {
      /* if there are some rows too much, delete them */
      /* only if you broke the whole paragraph! */ 
      Row * tmprow2 = row;
      while (tmprow2->next && tmprow2->next->par == row->par) {
	 tmprow2 = tmprow2->next;
      }
      while (tmprow2 != row) {
	 tmprow2 = tmprow2->previous;
	 RemoveRow(tmprow2->next);
      }
   }
   
   /* set the dimensions of the row */ 
   tmprow->fill = Fill(tmprow, paperwidth);
   SetHeightOfRow(tmprow);
}


void LyXText::BreakParagraph(char keep_layout)
{
   LyXLayout const & layout = textclasslist.Style(parameters->textclass,
				      cursor.par->GetLayout());
   
   /* table stuff -- begin */
   if (cursor.par->table) {
       // breaking of tables is only allowed at the beginning or the end */
       if (cursor.pos && cursor.pos < cursor.par->size() &&
           !cursor.par->table->ShouldBeVeryLastCell(NumberOfCell(cursor.par, cursor.pos)))
	       return; // no breaking of tables allowed
   }
   /* table stuff -- end */

   // this is only allowed, if the current paragraph is not empty or caption
   if ((cursor.par->Last() <= 0 && !cursor.par->IsDummy())
       && 
       layout.labeltype!= LABEL_SENSITIVE)
     return;

   SetUndo(Undo::INSERT, 
	   cursor.par->ParFromPos(cursor.pos)->previous, 
	   cursor.par->ParFromPos(cursor.pos)->next); 

   /* table stuff -- begin */
   if (cursor.par->table) {
       int cell = NumberOfCell(cursor.par, cursor.pos);
       if (cursor.par->table->ShouldBeVeryLastCell(cell))
           SetCursor(cursor.par, cursor.par->text.size());
   }
   /* table stuff -- end */
   
   // please break always behind a space
   if (cursor.pos < cursor.par->Last()
       && cursor.par->IsLineSeparator(cursor.pos))
     cursor.pos++;
   
   // break the paragraph
   if (keep_layout)
     keep_layout = 2;
   else	
     keep_layout = layout.isEnvironment();
   cursor.par->BreakParagraph(cursor.pos, keep_layout);

   /* table stuff -- begin */
   if (cursor.par->table){
     // the table should stay with the contents
     if (!cursor.pos){
       cursor.par->Next()->table = cursor.par->table;
       cursor.par->table = 0;
     }
   }
   /* table stuff -- end */

   // well this is the caption hack since one caption is really enough
   if (layout.labeltype == LABEL_SENSITIVE){
     if (!cursor.pos)
	     cursor.par->SetLayout(0); // set to standard-layout
     else
	     cursor.par->Next()->SetLayout(0); // set to standard-layout
   }
   
   /* if the cursor is at the beginning of a row without prior newline, 
    * move one row up! 
    * This touches only the screen-update. Otherwise we would may have
    * an empty row on the screen */
   if (cursor.pos && !cursor.row->par->IsNewline(cursor.row->pos -1) &&
       cursor.row->pos == cursor.pos) {
     CursorLeft();
   } 
   
   status = LyXText::NEED_MORE_REFRESH;
   refresh_row = cursor.row;
   refresh_y = cursor.y - cursor.row->baseline;
   
   // Do not forget the special right address boxes
   if (layout.margintype == MARGIN_RIGHT_ADDRESS_BOX) {
      while (refresh_row->previous &&
	     refresh_row->previous->par == refresh_row->par) {
		refresh_row = refresh_row->previous;
		refresh_y -= refresh_row->height;
	     }
   }
   RemoveParagraph(cursor.row);
   
   // set the dimensions of the cursor row
   cursor.row->fill = Fill(cursor.row, paperwidth);

   SetHeightOfRow(cursor.row);
   
   while (!cursor.par->Next()->table && cursor.par->Next()->Last()
	  && cursor.par->Next()->IsNewline(0))
     cursor.par->Next()->Erase(0);
   
   InsertParagraph(cursor.par->Next(), cursor.row);

   UpdateCounters(cursor.row->previous);
   
   /* This check is necessary. Otherwise the new empty paragraph will
    * be deleted automatically. And it is more friendly for the user! */ 
   if (cursor.pos)
     SetCursor(cursor.par->Next(), 0);
   else
     SetCursor(cursor.par, 0);
   
   if (cursor.row->next)
     BreakAgain(cursor.row->next);

   need_break_row = 0;
}


void LyXText::OpenFootnote()
{
   LyXParagraph * endpar,* tmppar;
   Row * row;
   
   LyXParagraph * par = cursor.par->ParFromPos(cursor.pos);
   
   /* if there is no footnote in this paragraph, just return. */ 
   if (!par->next
       || par->next->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE)
     return;
   
   /* ok, move the cursor right before the footnote */ 
   
   /* just a little faster than using CursorRight() */
   for (cursor.pos = 0;
	cursor.par->ParFromPos(cursor.pos) != par; cursor.pos++);
   /* now the cursor is at the beginning of the physical par */
   SetCursor(cursor.par,
	     cursor.pos + cursor.par->ParFromPos(cursor.pos)->text.size());
   
   /* the cursor must be exactly before the footnote */ 
   par = cursor.par->ParFromPos(cursor.pos);
   
   status = LyXText::NEED_MORE_REFRESH;
   refresh_row = cursor.row;
   refresh_y = cursor.y - cursor.row->baseline;
   
   tmppar = cursor.par;
   endpar = cursor.par->Next();
   row = cursor.row;
   
   tmppar->OpenFootnote(cursor.pos);
   RemoveParagraph(row);
   /* set the dimensions of the cursor row */
   row->fill = Fill(row, paperwidth);
   SetHeightOfRow(row);
   tmppar = tmppar->Next();
   
   while (tmppar != endpar) {
      if (tmppar) {
	 InsertParagraph(tmppar, row);
	 while (row->next && row->next->par == tmppar)
	   row = row->next;
	 tmppar = tmppar->Next();
      }
   }
   SetCursor(par->next, 0);
   sel_cursor = cursor;
}
   

/* table stuff -- begin*/

void LyXText::TableFeatures(int feature, string const & val) const
{
	if (!cursor.par->table)
		return; /* this should never happen */
  
	int actCell = NumberOfCell(cursor.par, cursor.pos);
	SetUndo(Undo::FINISH, 
		cursor.par->ParFromPos(cursor.pos)->previous, 
		cursor.par->ParFromPos(cursor.pos)->next); 
	
	switch (feature){
	case LyXTable::SET_PWIDTH:
		cursor.par->table->SetPWidth(actCell, val);
		break;
	case LyXTable::SET_SPECIAL_COLUMN:
	case LyXTable::SET_SPECIAL_MULTI:
		cursor.par->table->SetAlignSpecial(actCell, val, feature);
		break;
	default:
		break;
	}
	RedoParagraph();
}


void LyXText::TableFeatures(int feature) const
{
	int setLines = 0;
	int setAlign = LYX_ALIGN_LEFT;
	int lineSet;
	bool what;
    
    if (!cursor.par->table)
        return; /* this should never happen */
  
    int actCell = NumberOfCell(cursor.par, cursor.pos);
    SetUndo(Undo::FINISH, 
            cursor.par->ParFromPos(cursor.pos)->previous, 
            cursor.par->ParFromPos(cursor.pos)->next); 

    switch (feature){
      case LyXTable::ALIGN_LEFT:
          setAlign= LYX_ALIGN_LEFT;
          break;
      case LyXTable::ALIGN_RIGHT:
          setAlign= LYX_ALIGN_RIGHT;
          break;
      case LyXTable::ALIGN_CENTER:
          setAlign= LYX_ALIGN_CENTER;
          break;
      default:
          break;
    }
    switch (feature){
      case LyXTable::APPEND_ROW: {
	      LyXParagraph::size_type pos = cursor.pos;

	      /* move to the next row */
          int cell_org = actCell;
          int cell = cell_org;

          // if there is a ContRow following this row I have to add
          // the row after the ContRow's
          if ((pos < cursor.par->Last()) &&
              cursor.par->table->RowHasContRow(cell_org)) {
              while((pos < cursor.par->Last()) &&
                    !cursor.par->table->IsContRow(cell)) {
                  while (pos < cursor.par->Last() &&
                         !cursor.par->IsNewline(pos))
                      ++pos;
                  if (pos < cursor.par->Last())
                      ++pos;
                  ++cell;
              }
              while((pos < cursor.par->Last()) &&
                    cursor.par->table->IsContRow(cell)) {
                  while (pos < cursor.par->Last() &&
                         !cursor.par->IsNewline(pos))
                      ++pos;
                  if (pos < cursor.par->Last())
                      ++pos;
                  ++cell;
              }
              cell_org = --cell;
              if (pos < cursor.par->Last())
                  --pos;
          }
          while (pos < cursor.par->Last() && 
                 (cell == cell_org || !cursor.par->table->IsFirstCell(cell))){
              while (pos < cursor.par->Last() && !cursor.par->IsNewline(pos))
                  ++pos;
              if (pos < cursor.par->Last())
                  ++pos;
              ++cell;
          }
		
          /* insert the new cells */ 
          int number = cursor.par->table->NumberOfCellsInRow(cell_org);
          for (int i = 0; i < number; ++i)
              cursor.par->InsertChar(pos, LyXParagraph::META_NEWLINE);
		
          /* append the row into the table */
          cursor.par->table->AppendRow(cell_org);
          RedoParagraph();
          return;
      }
      case LyXTable::APPEND_CONT_ROW: {
	      LyXParagraph::size_type pos = cursor.pos;
          /* move to the next row */
          int cell_org = actCell;
          int cell = cell_org;

          // if there is already a controw but not for this cell
          // the AppendContRow sets only the right values but does
          // not actually add a row
          if (cursor.par->table->RowHasContRow(cell_org) &&
              (cursor.par->table->CellHasContRow(cell_org)<0)) {
              cursor.par->table->AppendContRow(cell_org);
              RedoParagraph();
              return;
          }
          while (pos < cursor.par->Last() && 
                 (cell == cell_org
                  || !cursor.par->table->IsFirstCell(cell))){
              while (pos < cursor.par->Last() && !cursor.par->IsNewline(pos))
                  ++pos;
              if (pos < cursor.par->Last())
                  ++pos;
              ++cell;
          }
		
          /* insert the new cells */ 
          int number = cursor.par->table->NumberOfCellsInRow(cell_org);
          for (int i = 0; i < number; ++i)
              cursor.par->InsertChar(pos, LyXParagraph::META_NEWLINE);
		
          /* append the row into the table */
          cursor.par->table->AppendContRow(cell_org);
          RedoParagraph();
          return;
      }
      case LyXTable::APPEND_COLUMN: {
	      LyXParagraph::size_type pos = 0;
          int cell_org = actCell;
          int cell = 0;
          do{
              if (pos && (cursor.par->IsNewline(pos-1))){
                  if (cursor.par->table->AppendCellAfterCell(cell_org, cell)){
                      cursor.par->InsertChar(pos, LyXParagraph::META_NEWLINE);
                      if (pos <= cursor.pos)
                          cursor.pos++;
                      ++pos;
                  }
                  ++cell;
              }
              ++pos;
          } while (pos <= cursor.par->Last());
          /* remember that the very last cell doesn't end with a newline.
             This saves one byte memory per table ;-) */ 
          if (cursor.par->table->AppendCellAfterCell(cell_org, cell))
              cursor.par->InsertChar(cursor.par->Last(), LyXParagraph::META_NEWLINE);
		
          /* append the column into the table */ 
          cursor.par->table->AppendColumn(cell_org);
		
          RedoParagraph();
          return;
      }
      case LyXTable::DELETE_ROW:
          if (current_view->the_locking_inset)
              current_view->unlockInset(current_view->the_locking_inset);
          RemoveTableRow(&cursor);
          RedoParagraph();
          return;
	
      case LyXTable::DELETE_COLUMN: {
	      LyXParagraph::size_type pos = 0;
          int cell_org = actCell;
          int cell = 0;
          if (current_view->the_locking_inset)
              current_view->unlockInset(current_view->the_locking_inset);
          do {
              if (!pos || (cursor.par->IsNewline(pos-1))){
                  if (cursor.par->table->DeleteCellIfColumnIsDeleted(cell, cell_org)){
                      // delete one cell
                      while (pos < cursor.par->Last() && !cursor.par->IsNewline(pos))
                          cursor.par->Erase(pos);
                      if (pos < cursor.par->Last())
                          cursor.par->Erase(pos);
                      else 
                          cursor.par->Erase(pos - 1); // the missing newline at the end of a table
                      --pos; // because of pos++ below
                  }   
                  ++cell;
              }
              ++pos;
          } while (pos <= cursor.par->Last());
		
          /* delete the column from the table */ 
          cursor.par->table->DeleteColumn(cell_org);
		
          /* set the cursor to the beginning of the table, where else? */ 
          cursor.pos = 0;
          RedoParagraph();
          return;
      }
      case LyXTable::TOGGLE_LINE_TOP:
          lineSet = !cursor.par->table->TopLine(actCell);
          if (!selection){
              cursor.par->table->SetTopLine(actCell, lineSet);
          } else {
		  LyXParagraph::size_type i;
		  int n = -1, m = -2;
              for (i = sel_start_cursor.pos; i <= sel_end_cursor.pos; ++i){
                  if ((n = NumberOfCell(sel_start_cursor.par, i)) != m) {
                      cursor.par->table->SetTopLine(n, lineSet);
                      m = n;
                  }
              }
          }
          RedoParagraph();
          return;
    
      case LyXTable::TOGGLE_LINE_BOTTOM:
          lineSet = !cursor.par->table->BottomLine(actCell);
          if (!selection){
              cursor.par->table->SetBottomLine(actCell, lineSet);
          } else {
		  LyXParagraph::size_type i;
		  int n = -1, m = -2;
              for (i = sel_start_cursor.pos; i <= sel_end_cursor.pos; ++i) {
                  if ((n = NumberOfCell(sel_start_cursor.par, i)) != m) {
                      cursor.par->table->SetBottomLine(n, lineSet);
                      m = n;
                  }
              }
          }
          RedoParagraph();
          return;
		
      case LyXTable::TOGGLE_LINE_LEFT:
          lineSet = !cursor.par->table->LeftLine(actCell);
          if (!selection){
              cursor.par->table->SetLeftLine(actCell, lineSet);
          } else {
		  LyXParagraph::size_type i;
		  int n = -1, m = -2;
              for (i = sel_start_cursor.pos; i <= sel_end_cursor.pos; ++i){
                  if ((n= NumberOfCell(sel_start_cursor.par, i)) != m) {
                      cursor.par->table->SetLeftLine(n, lineSet);
                      m = n;
                  }
              }
          }
          RedoParagraph();
          return;

      case LyXTable::TOGGLE_LINE_RIGHT:
          lineSet = !cursor.par->table->RightLine(actCell);
          if (!selection){
              cursor.par->table->SetRightLine(actCell, lineSet);
          } else {
		  int n = -1, m = -2;
		  LyXParagraph::size_type i = sel_start_cursor.pos;
              for (; i <= sel_end_cursor.pos; ++i) {
                  if ((n= NumberOfCell(sel_start_cursor.par, i)) != m) {
                      cursor.par->table->SetRightLine(n, lineSet);
                      m = n;
                  }
              }
          }
          RedoParagraph();
          return;
    
      case LyXTable::ALIGN_LEFT:
      case LyXTable::ALIGN_RIGHT:
      case LyXTable::ALIGN_CENTER:
          if (!selection){
              cursor.par->table->SetAlignment(actCell, setAlign);
          } else {
              int n = -1, m = -2;
	      LyXParagraph::size_type i = sel_start_cursor.pos;
              for (; i <= sel_end_cursor.pos; ++i) {
                  if ((n= NumberOfCell(sel_start_cursor.par, i)) != m) {
                      cursor.par->table->SetAlignment(n, setAlign);
                      m = n;
                  }
              }
          }
          RedoParagraph();
          return;
		
      case LyXTable::DELETE_TABLE:
          SetCursorIntern(cursor.par, 0);
          delete cursor.par->table;
          cursor.par->table = 0;
          // temporary: Should put table in simple_cut_buffer (with before and after
          // dummy-paragraph !! 
          // not necessar anymore with UNDO :)
          for (LyXParagraph::size_type i = 
		       cursor.par->text.size() - 1; i >= 0; --i)
	      cursor.par->Erase(i);
          RedoParagraph();
          return;
		
      case LyXTable::MULTICOLUMN: {
          int number = 0;
          // check wether we are completly in a multicol
          int multicol = cursor.par->table->IsMultiColumn(actCell);
          if (multicol && selection && sel_start_cursor.row == sel_end_cursor.row){
	      multicol = NumberOfCell(sel_start_cursor.par, sel_start_cursor.pos)
                  == NumberOfCell(sel_end_cursor.par, sel_end_cursor.pos);
          }

          if (multicol){
	      int newlines = cursor.par->table->UnsetMultiColumn(actCell);
	      LyXParagraph::size_type pos = cursor.pos;
	      while (pos < cursor.par->Last() && !cursor.par->IsNewline(pos))
                  ++pos;
	      for (; newlines; --newlines)
                  cursor.par->InsertChar(pos, LyXParagraph::META_NEWLINE);
	      RedoParagraph();
	      return;
          }
          else {
	      // selection must be in one row (or no selection)
	      if (!selection){
                  cursor.par->table->SetMultiColumn(NumberOfCell(cursor.par,
                                                                 cursor.pos),
                                                    1);
                  RedoParagraph();
                  return;
	      }
	      else {
                  if (sel_start_cursor.row == sel_end_cursor.row){
                      LyXParagraph::size_type i;
                      number = 1;
                      for (i = sel_start_cursor.pos;
			   i < sel_end_cursor.pos; ++i){
                          if (sel_start_cursor.par->IsNewline(i)){
                              sel_start_cursor.par->Erase(i);
                              // check for double-blanks
                              if ((i && !sel_start_cursor.par->IsLineSeparator(i-1))
                                  &&
                                  (i < sel_start_cursor.par->Last() 
                                   && !sel_start_cursor.par->IsLineSeparator(i)))
                                  sel_start_cursor.par->InsertChar(i, ' ');
                              else {
                                  sel_end_cursor.pos--;
                                  --i;
                              }
                              ++number;
                          }
                      }
                      cursor.par->table->
			  SetMultiColumn(NumberOfCell(sel_start_cursor.par,
						      sel_start_cursor.pos),
					 number);
                      cursor.pos = sel_start_cursor.pos;
                      RedoParagraph();
                      return;
                  }
                  else {
                      WriteAlert(_("Impossible Operation!"), 
                                 _("Multicolumns can only be horizontally."), 
                                 _("Sorry."));
                  }
	      }
          }
	  break;
      }
      case LyXTable::SET_ALL_LINES:
          setLines = 1;
      case LyXTable::UNSET_ALL_LINES:
          if (!selection){
              cursor.par->table->SetAllLines(NumberOfCell(cursor.par,
                                                          cursor.pos),
                                             setLines);
          } else {
		  LyXParagraph::size_type i;
		  int n = -1, m = -2;
              for (i = sel_start_cursor.pos; i <= sel_end_cursor.pos; ++i) {
                  if ((n= NumberOfCell(sel_start_cursor.par, i)) != m) {
                      cursor.par->table->SetAllLines(n, setLines);
                      m = n;
                  }
              }
          }
          RedoParagraph();
          return;
      case LyXTable::SET_LONGTABLE:
          cursor.par->table->SetLongTable(true);
          return;
      case LyXTable::UNSET_LONGTABLE:
          cursor.par->table->SetLongTable(false);
          return;
      case LyXTable::SET_ROTATE_TABLE:
          cursor.par->table->SetRotateTable(true);
          return;
      case LyXTable::UNSET_ROTATE_TABLE:
          cursor.par->table->SetRotateTable(false);
          return;
      case LyXTable::SET_ROTATE_CELL:
          if (!selection){
              cursor.par->table->SetRotateCell(actCell, true);
          } else {
		  LyXParagraph::size_type i;
		  int n = -1, m = -2;
              for (i = sel_start_cursor.pos; i <= sel_end_cursor.pos; ++i){
                  if ((n = NumberOfCell(sel_start_cursor.par, i)) != m) {
                      cursor.par->table->SetRotateCell(n, true);
                      m = n;
                  }
              }
          }
          return;
      case LyXTable::UNSET_ROTATE_CELL:
          if (!selection){
              cursor.par->table->SetRotateCell(actCell, false);
          } else {
		  int n = -1, m = -2;
		  LyXParagraph::size_type i = sel_start_cursor.pos;
              for (; i <= sel_end_cursor.pos; ++i) {
                  if ((n= NumberOfCell(sel_start_cursor.par, i)) != m) {
                      cursor.par->table->SetRotateCell(n, false);
                      m = n;
                  }
              }
          }
          return;
      case LyXTable::SET_LINEBREAKS:
          what = !cursor.par->table->Linebreaks(cursor.par->table->FirstVirtualCell(actCell));
          if (!selection){
              cursor.par->table->SetLinebreaks(actCell, what);
          } else {
		  LyXParagraph::size_type i;
		  int n = -1, m = -2;
              for (i = sel_start_cursor.pos; i <= sel_end_cursor.pos; ++i) {
                  if ((n = NumberOfCell(sel_start_cursor.par, i)) != m) {
                      cursor.par->table->SetLinebreaks(n, what);
                      m = n;
                  }
              }
          }
          return;
      case LyXTable::SET_LTFIRSTHEAD:
          cursor.par->table->SetLTHead(actCell, true);
          return;
      case LyXTable::SET_LTHEAD:
          cursor.par->table->SetLTHead(actCell, false);
          return;
      case LyXTable::SET_LTFOOT:
          cursor.par->table->SetLTFoot(actCell, false);
          return;
      case LyXTable::SET_LTLASTFOOT:
          cursor.par->table->SetLTFoot(actCell, true);
          return;
      case LyXTable::SET_LTNEWPAGE:
          what = !cursor.par->table->LTNewPage(actCell);
          cursor.par->table->SetLTNewPage(actCell, what);
          return;
    }
}
	

void LyXText::InsertCharInTable(char c)
{
	Row * row;
	Row * tmprow;
	long y;
	bool jumped_over_space;
	
	/* first check, if there will be two blanks together or a blank at 
	 * the beginning of a paragraph. 
	 * I decided to handle blanks like normal characters, the main 
	 * difference are the special checks when calculating the row.fill
	 * (blank does not count at the end of a row) and the check here */ 
	
	LyXFont realtmpfont = real_current_font;
	LyXFont rawtmpfont = current_font; /* store the current font.
					    * This is because of the use
					    * of cursor movements. The moving
					    * cursor would refresh the 
					    * current font */

	// Get the font that is used to calculate the baselineskip
	LyXParagraph::size_type const lastpos = 
		cursor.par->Last();
	LyXFont rawparfont = cursor.par->GetFontSettings(lastpos - 1);

	jumped_over_space = false;
	if (IsLineSeparatorChar(c)) {
		
		/* avoid double blanks but insert the new blank because
		 * of a possible font change */
		if (cursor.pos < lastpos &&
		    cursor.par->IsLineSeparator(cursor.pos))
		{
			cursor.par->Erase(cursor.pos);
			jumped_over_space = true;
		}
		else if ((cursor.pos > 0 && 
			  cursor.par->IsLineSeparator(cursor.pos - 1))
			 || (cursor.pos > 0 && cursor.par->IsNewline(cursor.pos - 1))
			  || (cursor.pos == 0 &&
			      !(cursor.par->Previous()
			      && cursor.par->Previous()->footnoteflag
			      == LyXParagraph::OPEN_FOOTNOTE)))
			return;
	}
	else if (IsNewlineChar(c)) {
            if (!IsEmptyTableCell()) {
                TableFeatures(LyXTable::APPEND_CONT_ROW);
                CursorDown();
            }
	  return;
	}
   
	row = cursor.row;
	y = cursor.y - row->baseline;
	if (c != LyXParagraph::META_INSET)	/* in this case LyXText::InsertInset 
					 * already inserted the character */
		cursor.par->InsertChar(cursor.pos, c);
	SetCharFont(cursor.par, cursor.pos, rawtmpfont);

	if (!jumped_over_space) {
		/* refresh the positions */
		tmprow = row;
		while (tmprow->next && tmprow->next->par == row->par) {
			tmprow = tmprow->next;
			tmprow->pos++;
		}
	}

	cursor.pos++;

	CheckParagraphInTable(cursor.par, cursor.pos);
	
	current_font = rawtmpfont;
	real_current_font = realtmpfont;
	
	/* check, whether the last character's font has changed. */
	if (cursor.pos && cursor.pos == cursor.par->Last()
	    && rawparfont != rawtmpfont)
		RedoHeightOfParagraph(cursor);
}


void LyXText::CheckParagraphInTable(LyXParagraph * par,
				    LyXParagraph::size_type pos)
{
	
	if (par->GetChar(pos) == LyXParagraph::META_INSET &&
	    par->GetInset(pos) && par->GetInset(pos)->display()){
	  par->GetInset(pos)->display(false);
	}

	long y;
	Row * row = GetRow(par, pos, y);
	
	int tmpheight = row->height;
	SetHeightOfRow(row);

	LyXParagraph::size_type tmp_pos = pos;
	/* update the table information */
	while (tmp_pos && !par->IsNewline(tmp_pos - 1))
		--tmp_pos;
	if (par->table->SetWidthOfCell(NumberOfCell(par, pos),
				       WidthOfCell(par, tmp_pos))) {
		LyXCursor tmpcursor = cursor;
		SetCursorIntern(par, pos);
		/* make a complete redraw */
		RedoDrawingOfParagraph(cursor);
		cursor = tmpcursor;
	}
	else {
		/* redraw only the row */
		LyXCursor tmpcursor = cursor;
		SetCursorIntern(par, pos);
		refresh_y = y;
		refresh_x = cursor.x;
		refresh_row = row;
		refresh_pos = cursor.pos;
		cursor = tmpcursor;
		
		if (tmpheight == row->height)
			status = LyXText::NEED_VERY_LITTLE_REFRESH;
		else
			status = LyXText::NEED_MORE_REFRESH;
	}
        SetCursorIntern(cursor.par, cursor.pos);
}


void LyXText::BackspaceInTable()
{
	Row * tmprow, * row;
	long y;
	
	LyXFont rawtmpfont = current_font;
	LyXFont realtmpfont = real_current_font;

	// Get the font that is used to calculate the baselineskip
	int const lastpos = cursor.par->Last();
	LyXFont rawparfont = cursor.par->GetFontSettings(lastpos - 1);
	
	if (cursor.pos == 0) {
		/* no pasting of table paragraphs */
		
		CursorLeft();
	}
	else {
		/* this is the code for a normal backspace, not pasting
		 * any paragraphs */ 
	  SetUndo(Undo::DELETE, 
		  cursor.par->ParFromPos(cursor.pos)->previous, 
		  cursor.par->ParFromPos(cursor.pos)->next); 
	  
		CursorLeftIntern();
		
		/* some insets are undeletable here */
		if (cursor.par->GetChar(cursor.pos) == LyXParagraph::META_INSET) {
			if (!cursor.par->GetInset(cursor.pos)->Deletable())
				return;
		}
		
		row = cursor.row;
		y = cursor.y - row->baseline;
		
		/* some special code when deleting a newline. */
		if (cursor.par->IsNewline(cursor.pos)) {
			/* nothing :-) */
			return;
		}
		else {
			cursor.par->Erase(cursor.pos);
			
			/* refresh the positions */
			tmprow = row;
			while (tmprow->next && tmprow->next->par == row->par) {
				tmprow = tmprow->next;
				tmprow->pos--;
			}
			
			/* delete superfluous blanks */ 
			if (cursor.pos < cursor.par->Last() - 1 &&
			(cursor.par->IsLineSeparator(cursor.pos))) {
				
				if (cursor.pos == BeginningOfMainBody(cursor.par)
				|| !cursor.pos 
				|| cursor.par->IsLineSeparator(cursor.pos - 1)) {
					cursor.par->Erase(cursor.pos);
					/* refresh the positions */
					tmprow = row;
					while (tmprow->next && 
					       tmprow->next->par == row->par) {
						tmprow = tmprow->next;
						tmprow->pos--;
					}
					if (cursor.pos)   /* move one character left */
						cursor.pos--;
				}
			}
		}
      
		CheckParagraphInTable(cursor.par, cursor.pos);
      
		/* check, wether the last characters font has changed. */ 
		if (cursor.pos && cursor.pos == cursor.par->Last()
		    && rawparfont != rawtmpfont)
			RedoHeightOfParagraph(cursor);

		/* restore the current font 
		 * That is what a user expects! */
		current_font = rawtmpfont;
		real_current_font = realtmpfont;
	}
	SetCursorIntern(cursor.par, cursor.pos);
}

/* table stuff -- end*/


/* just a macro to make some thing easier. */ 
void LyXText::RedoParagraph() const
{
#if 1
	// I suspect this version will work
	// also.
	ClearSelection();
	RedoParagraphs(cursor, cursor.par->Next());
	SetCursorIntern(cursor.par, cursor.pos);
#else
	LyXCursor tmpcursor = cursor;
	ClearSelection();
	RedoParagraphs(cursor, cursor.par->Next());
	SetCursorIntern(tmpcursor.par, tmpcursor.pos);
#endif
}


/* insert a character, moves all the following breaks in the 
 * same Paragraph one to the right and make a rebreak */
void  LyXText::InsertChar(char c)
{
	SetUndo(Undo::INSERT, 
		cursor.par->ParFromPos(cursor.pos)->previous, 
		cursor.par->ParFromPos(cursor.pos)->next);

	/* When the free-spacing option is set for the current layout,
	 * all spaces are converted to protected spaces. */
	bool freeSpacingBo = 
		textclasslist.Style(parameters->textclass,
			       cursor.row->par->GetLayout()).free_spacing;
   
	if (freeSpacingBo && IsLineSeparatorChar(c) 
	    && (!cursor.pos || cursor.par->IsLineSeparator(cursor.pos - 1))) 
		c = LyXParagraph::META_PROTECTED_SEPARATOR;
   
	/* table stuff -- begin*/
  	if (cursor.par->table) {
		InsertCharInTable(c);
		charInserted();
		return;
	}
	/* table stuff -- end*/
   
	/* First check, if there will be two blanks together or a blank at 
	  the beginning of a paragraph. 
	  I decided to handle blanks like normal characters, the main 
	  difference are the special checks when calculating the row.fill
	  (blank does not count at the end of a row) and the check here */ 

	// The bug is triggered when we type in a description environment:
	// The current_font is not changed when we go from label to main text
	// and it should (along with realtmpfont) when we type the space.
#ifdef WITH_WARNINGS
#warning There is a bug here! (Asger)
#endif
	
	LyXFont realtmpfont = real_current_font;
	LyXFont rawtmpfont = current_font;  /* store the current font.
				     * This is because of the use
				     * of cursor movements. The moving
				     * cursor would refresh the 
				     * current font */

	// Get the font that is used to calculate the baselineskip
	LyXParagraph::size_type lastpos = cursor.par->Last();
	LyXFont rawparfont = cursor.par->GetFontSettings(lastpos - 1);

	bool jumped_over_space = false;
   
	if (IsLineSeparatorChar(c)) {
#ifndef FIX_DOUBLE_SPACE
		if (cursor.pos < lastpos
		    && cursor.par->IsLineSeparator(cursor.pos)) {
			/* the user inserted a space before a space. So we
			 * will just make a CursorRight. BUT: The font of this
			 * space should be set to current font. That is why
			 * we need to rebreak perhaps. If there is a protected
			 * blank at the end of a row we have to force
			 * a rebreak.*/ 
	   
			current_view->owner()->getMiniBuffer()
				->Set(_("You cannot type two spaces this way. "
					" Please read the Tutorial."));
#if 1
			// How can this ever happen?
			if (cursor.pos == RowLast(cursor.row)
			    && !IsLineSeparatorChar(c))
				cursor.row->fill = -1;  // force rebreak
			cursor.par->Erase(cursor.pos);
			jumped_over_space = true;
#else
			// Seems to me that this works just as well.
			CursorRight();
			charInserted();
			return;
#endif
		} else
#endif   
		if ((cursor.pos > 0 
		     && cursor.par->IsLineSeparator(cursor.pos - 1))
		    || (cursor.pos > 0
			&& cursor.par->IsNewline(cursor.pos - 1))
		    || (cursor.pos == 0
			&& !(cursor.par->Previous()
			     && cursor.par->Previous()->footnoteflag
			     == LyXParagraph::OPEN_FOOTNOTE))) {
	   		if (cursor.pos == 0 )
				current_view->owner()->getMiniBuffer()->Set(_("You cannot insert a space at the beginning of a paragraph.  Please read the Tutorial."));
			else
				current_view->owner()->getMiniBuffer()->Set(_("You cannot type two spaces this way.  Please read the Tutorial."));
			charInserted();
			return;
		}
	} else if (IsNewlineChar(c)) {
		if (cursor.par->FirstPhysicalPar() == cursor.par
		    && cursor.pos <= BeginningOfMainBody(cursor.par)) {
			charInserted();
			return;
		}
		/* No newline at first position 
		 * of a paragraph or behind labels. 
		 * TeX does not allow that. */
		
		if (cursor.pos < cursor.par->Last() &&
		    cursor.par->IsLineSeparator(cursor.pos))
			CursorRightIntern(); // newline always after a blank!
		cursor.row->fill = -1;	       // to force a new break
	}
   
	// the display inset stuff
	if (cursor.row->par->GetChar(cursor.row->pos) == LyXParagraph::META_INSET
	    && cursor.row->par->GetInset(cursor.row->pos)
	    && cursor.row->par->GetInset(cursor.row->pos)->display())
		cursor.row->fill = -1; // to force a new break  

	// get the cursor row fist
	Row * row = cursor.row;
	long y = cursor.y - row->baseline;
	if (c != LyXParagraph::META_INSET) /* Here case LyXText::InsertInset 
					    * already insertet the character */
		cursor.par->InsertChar(cursor.pos, c);
	SetCharFont(cursor.par, cursor.pos, rawtmpfont);

	if (!jumped_over_space) {
		// refresh the positions
		Row * tmprow = row;
		while (tmprow->next && tmprow->next->par == row->par) {
			tmprow = tmprow->next;
			tmprow->pos++;
		}
	}
   
	// Is there a break one row above
	if ((cursor.par->IsLineSeparator(cursor.pos)
	     || cursor.par->IsNewline(cursor.pos)
	     || cursor.row->fill == -1)
	    && row->previous && row->previous->par == row->par) {
		LyXParagraph::size_type z = NextBreakPoint(row->previous,
							   paperwidth);
		if ( z >= row->pos) {
			row->pos = z + 1;
			
			// set the dimensions of the row above
			row->previous->fill = Fill(row->previous, paperwidth);

			SetHeightOfRow(row->previous);
	     
			y -= row->previous->height;
			refresh_y = y;
			refresh_row = row->previous;
			status = LyXText::NEED_MORE_REFRESH;
	     
			BreakAgainOneRow(row);

			current_font = rawtmpfont;
			real_current_font = realtmpfont;
			SetCursor(cursor.par, cursor.pos + 1, false);
			/* cursor MUST be in row now */
	     
			if (row->next && row->next->par == row->par)
				need_break_row = row->next;
			else
				need_break_row = 0;
	     
			// check, wether the last characters font has changed. 
			if (cursor.pos && cursor.pos == cursor.par->Last()
			    && rawparfont != rawtmpfont)
				RedoHeightOfParagraph(cursor);
			
			charInserted();
			return;
		}
	}
   
	/* recalculate the fill of the row */ 
	if (row->fill >= 0)  /* needed because a newline
			      * will set fill to -1. Otherwise
			      * we would not get a rebreak! */
		row->fill = Fill(row, paperwidth);
	if (row->fill < 0 ) {
		refresh_y = y;
		refresh_row = row; 
		refresh_x = cursor.x;
		refresh_pos = cursor.pos;
		status = LyXText::NEED_MORE_REFRESH;
		BreakAgainOneRow(row); 
		/* will the cursor be in another row now? */ 
		if (RowLast(row) <= cursor.pos + 1 && row->next) {
			if (row->next && row->next->par == row->par)
				/* this should
				 * always be true */
				row = row->next;
			BreakAgainOneRow(row);
		}
		current_font = rawtmpfont;
		real_current_font = realtmpfont;
		SetCursor(cursor.par, cursor.pos + 1, false);
		if (row->next && row->next->par == row->par)
			need_break_row = row->next;
		else
			need_break_row = 0;		
	} else {
		refresh_y = y;
		refresh_x = cursor.x;
		refresh_row = row;
		refresh_pos = cursor.pos;
		
		int tmpheight = row->height;
		SetHeightOfRow(row);
		if (tmpheight == row->height)
			status = LyXText::NEED_VERY_LITTLE_REFRESH;
		else
			status = LyXText::NEED_MORE_REFRESH;
            
		current_font = rawtmpfont;
		real_current_font = realtmpfont;
		SetCursor(cursor.par, cursor.pos + 1, false);
	}

	/* check, wether the last characters font has changed. */ 
	if (cursor.pos && cursor.pos == cursor.par->Last()
	    && rawparfont != rawtmpfont) {
		RedoHeightOfParagraph(cursor);
	} else {
		/* now the special right address boxes */
		if (textclasslist.Style(parameters->textclass,
				   cursor.par->GetLayout()).margintype
		    == MARGIN_RIGHT_ADDRESS_BOX) {
			RedoDrawingOfParagraph(cursor); 
		}
	}

	charInserted();
}
   

void LyXText::charInserted()
{
	// Here we could call FinishUndo for every 20 characters inserted.
	// This is from my experience how emacs does it.
	static unsigned int counter = 0;
	if (counter < 20) {
		++counter;
	} else {
		FinishUndo();
		counter = 0;
	}
}

void LyXText::PrepareToPrint(Row * row, float & x,
			     float & fill_separator, 
			     float & fill_hfill,
			     float & fill_label_hfill,
			     bool bidi) const
{
	float nh, nlh, ns;
	
	float w = row->fill;
	fill_hfill = 0;
	fill_label_hfill = 0;
	fill_separator = 0;
	fill_label_hfill = 0;

	LyXDirection direction = row->par->getParDirection();

	if (direction == LYX_DIR_RIGHT_TO_LEFT) {
		x = RightMargin(row);
		if (row->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE) {
			LyXFont font(LyXFont::ALL_SANE);
			font.setSize(LyXFont::SIZE_SMALL);
			x += font.textWidth("Mwide-figM", 10);
		}
	}
	else
		x = LeftMargin(row);
	
	/* is there a manual margin with a manual label */ 
	if (textclasslist.Style(parameters->textclass,
			   row->par->GetLayout()).margintype == MARGIN_MANUAL
	    && textclasslist.Style(parameters->textclass,
			      row->par->GetLayout()).labeltype == LABEL_MANUAL) {
	       
		nlh = NumberOfLabelHfills(row) + 1; /* one more since labels 
						    * are left aligned*/ 
		if (nlh && !row->par->GetLabelWidthString().empty()) {
			fill_label_hfill = LabelFill(row) / nlh;
		}
	}
		
	/* are there any hfills in the row? */ 
	nh = NumberOfHfills(row);
	
/* table stuff -- begin*/
	if (row->par->table) {
	   w = paperwidth - row->par->table->WidthOfTable()
	   - x - RightMargin(row);
	   nh = 0; /* ignore hfills in tables */ 
	}
/* table stuff -- end*/

	if (nh)
	  fill_hfill = w /nh;
	else  {
	   /* is it block, flushleft or flushright? 
	    * set x how you need it */
	int align;
	if (row->par->FirstPhysicalPar()->align == LYX_ALIGN_LAYOUT)
	  align = textclasslist.Style(parameters->textclass, row->par->GetLayout()).align;
	else
	  align = row->par->FirstPhysicalPar()->align;
	   
	   /* center displayed insets */ 
	   if (row->par->GetChar(row->pos) == LyXParagraph::META_INSET
	       && row->par->GetInset(row->pos)
	       && row->par->GetInset(row->pos)->display())
	     align = LYX_ALIGN_CENTER;

	   switch (align) {
	    case LYX_ALIGN_BLOCK:
	      ns = NumberOfSeparators(row);
	      if (ns && row->next && row->next->par == row->par &&
		  !(row->next->par->IsNewline(row->next->pos-1))
		  && !(row->next->par->GetChar(row->next->pos) == LyXParagraph::META_INSET
		       && row->next->par->GetInset(row->next->pos)
		       && row->next->par->GetInset(row->next->pos)->display())
		  )
	      	fill_separator = w / ns;
	      else if (direction == LYX_DIR_RIGHT_TO_LEFT)
		x += w;
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

	ComputeBidiTables(row);
	if (direction == LYX_DIR_RIGHT_TO_LEFT) {
		LyXParagraph::size_type main_body = 
			BeginningOfMainBody(row->par);
		LyXParagraph::size_type last = RowLast(row);

		if (row->pos <= last
		    && !row->par->table
		    && last != vis2log(last)
		    && row->par->IsLineSeparator(last)) {
			if (!(main_body > 0 && main_body-1 == last))
				x -= fill_separator+SingleWidth(row->par,last);
		} else if (main_body > 0 &&
			   (main_body-1 > last || 
			    !row->par->IsLineSeparator(main_body-1))) {
			LyXLayout const & layout = textclasslist.Style(parameters->textclass,
								       row->par->GetLayout());
			x += GetFont(row->par, -2).stringWidth(layout.labelsep);
			if (main_body-1 <= last)
				x += fill_label_hfill;
		}
	}
}
      
/* important for the screen */


/* the cursor set functions have a special mechanism. When they
* realize, that you left an empty paragraph, they will delete it.
* They also delete the corresponding row */

void LyXText::CursorRightOneWord() const
{
	// treat floats, HFills and Insets as words
	LyXCursor tmpcursor = cursor;

	if (tmpcursor.pos == tmpcursor.par->Last()
	    && tmpcursor.par->Next())
	{
			tmpcursor.par = tmpcursor.par->Next();
			tmpcursor.pos = 0;
	} else {
		int steps = 0;

		// Skip through initial nonword stuff.
		while ( tmpcursor.pos < tmpcursor.par->Last() &&
			! tmpcursor.par->IsWord( tmpcursor.pos ) ) 
		{
		  //    printf("Current pos1 %d", tmpcursor.pos) ;
			tmpcursor.pos++;
			++steps;
		}
		// Advance through word.
		while ( tmpcursor.pos < tmpcursor.par->Last() &&
		        tmpcursor.par->IsWord( tmpcursor.pos ) )
		{
		  //     printf("Current pos2 %d", tmpcursor.pos) ;
			tmpcursor.pos++;
			++steps;
		}
	}
	SetCursor(tmpcursor.par, tmpcursor.pos);
}


void LyXText::CursorTab() const
{
    if (cursor.par->table) {
        int cell = NumberOfCell(cursor.par, cursor.pos);
        while(cursor.par->table->IsContRow(cell)) {
            CursorUp();
            cell = NumberOfCell(cursor.par, cursor.pos);
        }
        if (cursor.par->table->ShouldBeVeryLastCell(cell))
            TableFeatures(LyXTable::APPEND_ROW);
    }
    LyXCursor tmpcursor = cursor;
    while (tmpcursor.pos < tmpcursor.par->Last()
           && !tmpcursor.par->IsNewline(tmpcursor.pos))
        tmpcursor.pos++;
   
    if (tmpcursor.pos == tmpcursor.par->Last()){
        if (tmpcursor.par->Next()) {
            tmpcursor.par = tmpcursor.par->Next();
            tmpcursor.pos = 0;
        }
    }
    else
        tmpcursor.pos++;
    SetCursor(tmpcursor.par, tmpcursor.pos);
    if (cursor.par->table) {
        int cell = NumberOfCell(cursor.par, cursor.pos);
        while (cursor.par->table->IsContRow(cell) &&
               !cursor.par->table->ShouldBeVeryLastCell(cell)) {
            tmpcursor = cursor;
            while (tmpcursor.pos < tmpcursor.par->Last()
                   && !tmpcursor.par->IsNewline(tmpcursor.pos))
                tmpcursor.pos++;
   
            if (tmpcursor.pos == tmpcursor.par->Last()){
                if (tmpcursor.par->Next()) {
                    tmpcursor.par = tmpcursor.par->Next();
                    tmpcursor.pos = 0;
                }
            }
            else
                tmpcursor.pos++;
            SetCursor(tmpcursor.par, tmpcursor.pos);
            cell = NumberOfCell(cursor.par, cursor.pos);
        }
    }
}


/* -------> Skip initial whitespace at end of word and move cursor to *start*
            of prior word, not to end of next prior word. */

void LyXText::CursorLeftOneWord()  const
{
	// treat HFills, floats and Insets as words
	LyXCursor tmpcursor = cursor;
	while (tmpcursor.pos 
	       && (tmpcursor.par->IsSeparator(tmpcursor.pos - 1) 
		   || tmpcursor.par->IsKomma(tmpcursor.pos - 1))
	       && !(tmpcursor.par->IsHfill(tmpcursor.pos - 1)
	            || tmpcursor.par->IsFloat(tmpcursor.pos - 1)
		    || tmpcursor.par->IsInset(tmpcursor.pos - 1)))
		tmpcursor.pos--;

	if (tmpcursor.pos
	    && (tmpcursor.par->IsInset(tmpcursor.pos - 1)
	        || tmpcursor.par->IsFloat(tmpcursor.pos - 1)
		|| tmpcursor.par->IsHfill(tmpcursor.pos - 1))) {
		tmpcursor.pos--;
	} else if (!tmpcursor.pos) {
		if (tmpcursor.par->Previous()){
			tmpcursor.par = tmpcursor.par->Previous();
			tmpcursor.pos = tmpcursor.par->Last();
		}
	} else {		// Here, tmpcursor != 0 
		while (tmpcursor.pos > 0 &&
		       tmpcursor.par->IsWord(tmpcursor.pos-1) )
			tmpcursor.pos-- ;
	}
	SetCursor(tmpcursor.par, tmpcursor.pos);
}

/* -------> Select current word. This depends on behaviour of CursorLeftOneWord(), so it is
			patched as well. */

void LyXText::SelectWord() 
{
	/* Move cursor to the beginning, when not already there. */
	if ( cursor.pos
	     && !cursor.par->IsSeparator(cursor.pos-1)
	     && !cursor.par->IsKomma(cursor.pos-1) )
		CursorLeftOneWord();

	/* set the sel cursor */
	sel_cursor = cursor;

	while ( cursor.pos < cursor.par->Last()
			&& !cursor.par->IsSeparator(cursor.pos)
			&& !cursor.par->IsKomma(cursor.pos) )
		cursor.pos++;
	SetCursor( cursor.par, cursor.pos );
	
	/* finally set the selection */ 
	SetSelection();
}


/* -------> Select the word currently under the cursor when:
			1: no selection is currently set,
			2: the cursor is not at the borders of the word. */

int LyXText::SelectWordWhenUnderCursor() 
{
	if ( selection ) return 0;
	if ( cursor.pos < cursor.par->Last()
		 && !cursor.par->IsSeparator(cursor.pos)
		 && !cursor.par->IsKomma(cursor.pos)
		 && cursor.pos 
		 && !cursor.par->IsSeparator(cursor.pos -1)
		 && !cursor.par->IsKomma(cursor.pos -1) ) {
		SelectWord();
		return 1;
	}
	return 0;
}


// This function is only used by the spellchecker for NextWord().
// It doesn't handle LYX_ACCENTs and probably never will.
char * LyXText::SelectNextWord(float & value)
{
	LyXParagraph * tmppar = cursor.par;
	
	// If this is not the very first word, skip rest of
	// current word because we are probably in the middle
	// of a word if there is text here.
	if (cursor.pos || cursor.par->previous) {
		while (cursor.pos < cursor.par->Last()
		       && cursor.par->IsLetter(cursor.pos))
			cursor.pos++;
	}
	// Now, skip until we have real text (will jump paragraphs)
	while ((cursor.par->Last() > cursor.pos
		&& (!cursor.par->IsLetter(cursor.pos)
		    || cursor.par->getFont(cursor.pos).latex() == LyXFont::ON))
	       || (cursor.par->Last() == cursor.pos
		   && cursor.par->Next())){
		if (cursor.pos == cursor.par->Last()) {
			cursor.par = cursor.par->Next();
			cursor.pos = 0;
		}
		else
			cursor.pos++;
	}
  
	// Update the value if we changed paragraphs
	if (cursor.par != tmppar){
		SetCursor(cursor.par, cursor.pos);
		value = float(cursor.y)/float(height);
	}

	/* Start the selection from here */
	sel_cursor = cursor;

	string latex;
   
	/* and find the end of the word 
	   (optional hyphens are part of a word) */
	while (cursor.pos < cursor.par->Last()
	       && (cursor.par->IsLetter(cursor.pos)) 
	           || (cursor.par->GetChar(cursor.pos) == LyXParagraph::META_INSET &&
		       cursor.par->GetInset(cursor.pos) != 0 &&
		       cursor.par->GetInset(cursor.pos)->Latex(latex, 0) == 0 &&
		       latex == "\\-"))
		cursor.pos++;

	// Finally, we copy the word to a string and return it
	char * str = 0;

	if (sel_cursor.pos < cursor.pos) {
		str = new char [cursor.pos - sel_cursor.pos + 2];
		LyXParagraph::size_type i, j;
		for (i = sel_cursor.pos, j = 0; i < cursor.pos; ++i) {
			if (cursor.par->GetChar(i) != LyXParagraph::META_INSET)
				str[j++] = cursor.par->GetChar(i);
		}
		str[j] = '\0';
	}
	return str;
}


// This one is also only for the spellchecker
void LyXText::SelectSelectedWord()
{
	/* move cursor to the beginning */
	SetCursor(sel_cursor.par, sel_cursor.pos);
	
	/* set the sel cursor */
	sel_cursor = cursor;

	string latex;
	
	/* now find the end of the word */
	while (cursor.pos < cursor.par->Last()
	       && (cursor.par->IsLetter(cursor.pos)
	           || (cursor.par->GetChar(cursor.pos) == LyXParagraph::META_INSET &&
		       cursor.par->GetInset(cursor.pos) != 0 &&
		       cursor.par->GetInset(cursor.pos)->Latex(latex, 0) == 0 &&
		       latex == "\\-")))
		cursor.pos++;
	
	SetCursor(cursor.par, cursor.pos);
	
	/* finally set the selection */ 
	SetSelection();
}


/* -------> Delete from cursor up to the end of the current or next word. */
void LyXText::DeleteWordForward()
{
	LyXCursor tmpcursor = cursor;
        
	if (!cursor.par->Last())
		CursorRight();
	else {
		/* -------> Skip initial non-word stuff. */
		while ( cursor.pos < cursor.par->Last() 
			&& (cursor.par->IsSeparator(cursor.pos)
			    || cursor.par->IsKomma(cursor.pos)) )
			cursor.pos++;
		
		SetCursorIntern(cursor.par, cursor.pos);
		selection = True; // to avoid deletion 
		CursorRightOneWord();
		sel_cursor = cursor;
		cursor = tmpcursor;
		SetSelection(); 
		
		/* -----> Great, CutSelection() gets rid of multiple spaces. */
		CutSelection();
	}
}


/* -------> Delete from cursor to start of current or prior word. */
void LyXText::DeleteWordBackward()
{
       LyXCursor tmpcursor = cursor;
       if (!cursor.par->Last())
         CursorLeft();
       else{
         selection = true; // to avoid deletion 
         CursorLeftOneWord();
         sel_cursor = cursor;
         cursor = tmpcursor;
         SetSelection();
         CutSelection();
       }
}


/* -------> Kill to end of line. */
void LyXText::DeleteLineForward()
{
	LyXCursor tmpcursor = cursor;
	if (!cursor.par->Last())
		CursorRight();
	else {
		CursorEnd();
		sel_cursor = cursor;
		cursor = tmpcursor;
		SetSelection();
		if (selection == false) {
			DeleteWordForward();
		} else {
			CutSelection();
		}
	}
}


// Change the case of a word at cursor position. The meaning of action
// is:
// 0  change to lowercase
// 1  capitalize word
// 2  change to uppercase
// This function directly manipulates LyXParagraph::text because there
// is no LyXParagraph::SetChar currently. I did what I could to ensure
// that it is correct. I guess part of it should be moved to
// LyXParagraph, but it will have to change for 1.1 anyway. At least
// it does not access outside of the allocated array as the older
// version did. (JMarc) 
void LyXText::ChangeWordCase(LyXText::TextCase action) 
{
	LyXParagraph * tmppar = cursor.par->ParFromPos(cursor.pos);

	SetUndo(Undo::FINISH, tmppar->previous, tmppar->next); 

	LyXParagraph::size_type tmppos = 
		cursor.par->PositionInParFromPos(cursor.pos);
	while (tmppos < tmppar->size()) {
		unsigned char c = tmppar->text[tmppos];
		if (IsKommaChar(c) || IsLineSeparatorChar(c))
			break;
		if (c != LyXParagraph::META_INSET) {
			switch (action) {
			case text_lowercase:
				c = tolower(c);
				break;
			case text_capitalization:
				c = toupper(c);
				action = text_lowercase;
				break;
			case text_uppercase:
				c = toupper(c);
				break;
			}
		}
		
		tmppar->text[tmppos] = c;
		++tmppos;
	}
	CheckParagraph(tmppar, tmppos);
	CursorRightOneWord();
}


void LyXText::Delete()
{
	// this is a very easy implementation

	LyXCursor old_cursor = cursor;
	int old_cur_par_id = old_cursor.par->id();
	int old_cur_par_prev_id = old_cursor.par->previous ?
		old_cursor.par->previous->id() : 0;
	
	// just move to the right
	CursorRightIntern();

	// This check is not very good...
	// The CursorRightIntern calls DeleteEmptyParagrapgMechanism
	// and that can very well delete the par or par->previous in
	// old_cursor. Will a solution where we compare paragraph id's
	//work better?
#if 1
	if ((cursor.par->previous ? cursor.par->previous->id() : 0)
	    == old_cur_par_prev_id
	    && cursor.par->id() != old_cur_par_id)
		return; // delete-empty-paragraph-mechanism has done it
#else
	if (cursor.par->previous == old_cursor.par->previous
	    && cursor.par != old_cursor.par)
		return; // delete-empty-paragraph-mechanism has done it
#endif
	// if you had success make a backspace
	if (old_cursor.par != cursor.par || old_cursor.pos != cursor.pos) {
		LyXCursor tmpcursor = cursor;
		cursor = old_cursor; // to make sure undo gets the right cursor position
		SetUndo(Undo::DELETE, 
			cursor.par->ParFromPos(cursor.pos)->previous, 
			cursor.par->ParFromPos(cursor.pos)->next); 
		cursor = tmpcursor;
		Backspace();
	}
}


void  LyXText::Backspace()
{
	LyXParagraph * tmppar;
	Row * tmprow, * row;
	long y;
	int tmpheight;

	/* table stuff -- begin */
	if (cursor.par->table) {
		BackspaceInTable();
		return;
	}
	/* table stuff -- end */
	
	LyXFont rawtmpfont = current_font;
	LyXFont realtmpfont = real_current_font;
   
	// Get the font that is used to calculate the baselineskip
	int const lastpos = cursor.par->Last();
	LyXFont rawparfont = cursor.par->GetFontSettings(lastpos - 1);

	if (cursor.pos == 0) {
		// we may paste some paragraphs
      
		// is it an empty paragraph?
      
		if ((lastpos == 0
		     || (lastpos == 1 && cursor.par->IsSeparator(0)))
		    && !(cursor.par->Next() 
			 && cursor.par->footnoteflag == 
			 LyXParagraph::NO_FOOTNOTE
			 && cursor.par->Next()->footnoteflag == 
			 LyXParagraph::OPEN_FOOTNOTE)) {
			
			if (cursor.par->previous) {
				tmppar = cursor.par->previous->FirstPhysicalPar();
				if (cursor.par->GetLayout() == tmppar->GetLayout()
				    && cursor.par->footnoteflag == tmppar->footnoteflag
				    && cursor.par->GetAlign() == tmppar->GetAlign()) {
					
					tmppar->line_bottom = cursor.par->line_bottom;
					tmppar->added_space_bottom = cursor.par->added_space_bottom;
					tmppar->pagebreak_bottom = cursor.par->pagebreak_bottom;
				}
				
				CursorLeftIntern();
		     
				// the layout things can change the height of a row !
				tmpheight = cursor.row->height;
				SetHeightOfRow(cursor.row);
				if (cursor.row->height != tmpheight) {
					refresh_y = cursor.y - cursor.row->baseline;
					refresh_row = cursor.row;
					status = LyXText::NEED_MORE_REFRESH;
				}
				return;
			}
		}
		if (cursor.par->ParFromPos(cursor.pos)->previous){
			SetUndo(Undo::DELETE,
				cursor.par->ParFromPos(cursor.pos)->previous->previous,
				cursor.par->ParFromPos(cursor.pos)->next);
		}
		tmppar = cursor.par;
		tmprow = cursor.row;
		CursorLeftIntern();
		/* Pasting is not allowed, if the paragraphs have different
		   layout. I think it is a real bug of all other
		   word processors to allow it. It confuses the user.
		   Even so with a footnote paragraph and a non-footnote
		   paragraph. I will not allow pasting in this case, 
		   because the user would be confused if the footnote behaves 
		   different wether it is open or closed.
		  
		   Correction: Pasting is always allowed with standard-layout
		*/
		if (cursor.par != tmppar
		    && (cursor.par->GetLayout() == tmppar->GetLayout()
			|| !tmppar->GetLayout())
		    && cursor.par->footnoteflag == tmppar->footnoteflag
		    /* table stuff -- begin*/
		    && !cursor.par->table /* no pasting of tables */ 
		    /* table stuff -- end*/
		    && cursor.par->GetAlign() == tmppar->GetAlign()) {
			
			cursor.par->PasteParagraph();
			
			if (!(cursor.pos &&
			      cursor.par->IsSeparator(cursor.pos - 1)))
				cursor.par->InsertChar(cursor.pos, ' ');
			else
				if (cursor.pos)
					cursor.pos--;
			
			status = LyXText::NEED_MORE_REFRESH;
			refresh_row = cursor.row;
			refresh_y = cursor.y - cursor.row->baseline;
			
			// remove the lost paragraph
			RemoveParagraph(tmprow);
			RemoveRow(tmprow);  
			
			AppendParagraph(cursor.row);
			UpdateCounters(cursor.row);
			
			// the row may have changed, block, hfills etc.
			SetCursor(cursor.par, cursor.pos);
		}
	} else {
		/* this is the code for a normal backspace, not pasting
		 * any paragraphs */ 
		SetUndo(Undo::DELETE, 
			cursor.par->ParFromPos(cursor.pos)->previous, 
			cursor.par->ParFromPos(cursor.pos)->next); 
		CursorLeftIntern();
		
		// some insets are undeletable here
		if (cursor.par->GetChar(cursor.pos) == LyXParagraph::META_INSET) {
			if (!cursor.par->GetInset(cursor.pos)->Deletable())
				return; 
			// force complete redo when erasing display insets
			// this is a cruel mathod but save..... Matthias 
			if (cursor.par->GetInset(cursor.pos)->display()){
				cursor.par->Erase(cursor.pos);
				RedoParagraph();
				return;
			}
		}
		
		row = cursor.row;
		y = cursor.y - row->baseline;
		LyXParagraph::size_type z;
		/* remember that a space at the end of a row doesnt count
		 * when calculating the fill */ 
		if (cursor.pos < RowLast(row) ||
		    !cursor.par->IsLineSeparator(cursor.pos)) {
			row->fill += SingleWidth(cursor.par, cursor.pos);
		}
		
		/* some special code when deleting a newline. This is similar
		 * to the behavior when pasting paragraphs */ 
		if (cursor.pos && cursor.par->IsNewline(cursor.pos)) {
			cursor.par->Erase(cursor.pos);
			// refresh the positions
			tmprow = row;
			while (tmprow->next && tmprow->next->par == row->par) {
				tmprow = tmprow->next;
				tmprow->pos--;
			}
			if (cursor.par->IsLineSeparator(cursor.pos - 1))
				cursor.pos--;
			
			if (cursor.pos < cursor.par->Last() && !cursor.par->IsSeparator(cursor.pos)) {
				cursor.par->InsertChar(cursor.pos, ' ');
				// refresh the positions
				tmprow = row;
				while (tmprow->next && tmprow->next->par == row->par) {
					tmprow = tmprow->next;
					tmprow->pos++;
				}
			}
		} else {
			cursor.par->Erase(cursor.pos);
			
			// refresh the positions
			tmprow = row;
			while (tmprow->next && tmprow->next->par == row->par) {
				tmprow = tmprow->next;
				tmprow->pos--;
			}

#ifndef FIX_DOUBLE_SPACE
			// delete superfluous blanks 
			if (cursor.pos < cursor.par->Last() - 1 &&
			    (cursor.par->IsLineSeparator(cursor.pos))) {
				
				if (cursor.pos == BeginningOfMainBody(cursor.par)
				    || !cursor.pos 
				    || cursor.par->IsLineSeparator(cursor.pos - 1)) {
					cursor.par->Erase(cursor.pos);
					// refresh the positions
					tmprow = row;
					while (tmprow->next && 
					       tmprow->next->par == row->par) {
						tmprow = tmprow->next;
						tmprow->pos--;
					}
					if (cursor.pos)   // move one character left
						cursor.pos--;
				}
			}
#endif
			
			// delete newlines at the beginning of paragraphs
			while (cursor.par->Last() &&
			       cursor.par->IsNewline(cursor.pos) &&
			       cursor.pos == BeginningOfMainBody(cursor.par)) {
				cursor.par->Erase(cursor.pos);
				// refresh the positions
				tmprow = row;
				while (tmprow->next && 
				       tmprow->next->par == row->par) {
					tmprow = tmprow->next;
					tmprow->pos--;
				}
			}
		}
		
		// is there a break one row above
		if (row->previous && row->previous->par == row->par) {
			z = NextBreakPoint(row->previous, paperwidth);
			if ( z >= row->pos) {
				row->pos = z + 1;
				
				tmprow = row->previous;
				
				// maybe the current row is now empty
				if (row->pos >= row->par->Last()) {
					// remove it
					RemoveRow(row);
					need_break_row = 0;
				} else {
					BreakAgainOneRow(row);
					if (row->next && row->next->par == row->par)
						need_break_row = row->next;
					else
						need_break_row = 0;
				}
				
				// set the dimensions of the row above
				y -= tmprow->height;
				tmprow->fill = Fill(tmprow, paperwidth);
				SetHeightOfRow(tmprow);
				
				refresh_y = y;
				refresh_row = tmprow;
				status = LyXText::NEED_MORE_REFRESH;
				current_font = rawtmpfont;
				real_current_font = realtmpfont;
				SetCursor(cursor.par, cursor.pos, false);
				// check, whether the last character's font has changed.
				rawtmpfont = cursor.par->GetFontSettings(cursor.par->Last() - 1);
				if (rawparfont != rawtmpfont)
					RedoHeightOfParagraph(cursor);
				return;
			}
		}
		
		// break the cursor row again
		z = NextBreakPoint(row, paperwidth);
		
		if (z != RowLast(row) || 
		    (row->next && row->next->par == row->par &&
		     RowLast(row) == row->par->Last() - 1)){
			
			/* it can happen that a paragraph loses one row
			 * without a real breakup. This is when a word
			 * is to long to be broken. Well, I don t care this 
			 * hack ;-) */ 
			if (row->next && row->next->par == row->par &&
			    RowLast(row) == row->par->Last() - 1)
				RemoveRow(row->next);
			
			refresh_y = y;
			refresh_row = row;
			status = LyXText::NEED_MORE_REFRESH;
			
			BreakAgainOneRow(row);
			current_font = rawtmpfont; 
			real_current_font = realtmpfont;
			SetCursor(cursor.par, cursor.pos, false);
			// cursor MUST be in row now
			
			if (row->next && row->next->par == row->par)
				need_break_row = row->next;
			else
				need_break_row = 0;
		} else  {
			// set the dimensions of the row
			row->fill = Fill(row, paperwidth);
			int tmpheight = row->height;
			SetHeightOfRow(row);
			if (tmpheight == row->height)
				status = LyXText::NEED_VERY_LITTLE_REFRESH;
			else
				status = LyXText::NEED_MORE_REFRESH;
			refresh_y = y;
			refresh_row = row;
			current_font = rawtmpfont; 
			real_current_font = realtmpfont;
			SetCursor(cursor.par, cursor.pos, false);
		}
	}
   
	// restore the current font
	// That is what a user expects!
	current_font = rawtmpfont; 
	real_current_font = realtmpfont;
	
	// check, wether the last characters font has changed.
	rawtmpfont = cursor.par->GetFontSettings(cursor.par->Last() - 1);
	if (rawparfont != rawtmpfont) {
		RedoHeightOfParagraph(cursor);
	} else {
		// now the special right address boxes
		if (textclasslist.Style(parameters->textclass,
					cursor.par->GetLayout()).margintype == MARGIN_RIGHT_ADDRESS_BOX) {
			RedoDrawingOfParagraph(cursor); 
		}
	}
}


#ifdef USE_PAINTER
void LyXText::GetVisibleRow(int offset, 
			    Row * row_ptr, long y)
{
	/* returns a printed row */
	Painter & pain = owner_->painter();
	
	LyXDirection direction = row_ptr->par->getParDirection();
	LyXParagraph::size_type vpos, pos, pos_end;
	float x, tmpx;
	int y_top, y_bottom;
	float fill_separator, fill_hfill, fill_label_hfill;
	LyXParagraph * par, * firstpar;
	LyXFont font;
	int maxdesc;
	if (row_ptr->height <= 0) {
		lyxerr << "LYX_ERROR: row.height: " << row_ptr->height << endl;
		return;
	}
	PrepareToPrint(row_ptr, x, fill_separator,
		       fill_hfill, fill_label_hfill);

	/* initialize the pixmap */
	
	pain.fillRectangle(0, offset, paperwidth, row_ptr->height);
	
	// check for NOT FAST SELECTION
	if (!fast_selection && !mono_video && selection) {
		/* selection code */ 
		if (sel_start_cursor.row == row_ptr &&
		    sel_end_cursor.row == row_ptr) {
		        if (sel_start_cursor.x < sel_end_cursor.x)
				pain.fillRectangle(sel_start_cursor.x, offset,
						   sel_end_cursor.x,
						   row_ptr->height,
						   LColor::selection);
			else
				pain.fillRectangle(sel_end_cursor.x, offset,
						   sel_start_cursor.x,
						   row_ptr->height,
						   LColor::selection);
		}
		else if (sel_start_cursor.row == row_ptr) {
		     if (direction == LYX_DIR_LEFT_TO_RIGHT)
			     pain.fillRectangle(sel_start_cursor.x, offset,
						paperwidth - sel_start_cursor.x,
						row_ptr->height,
						LColor::selection);
		     else
			     pain.fillRectangle(0, offset,
						sel_start_cursor.x,
						row_ptr->height,
						LColor::selection);
		} else if (sel_end_cursor.row == row_ptr) {
		     if (direction == LYX_DIR_LEFT_TO_RIGHT)
			     pain.fillRectangle(0, offset,
						sel_end_cursor.x,
						row_ptr->height,
						LColor::selection);
		     else
			     pain.fillRectangle(sel_end_cursor.x, offset,
						paperwidth - sel_end_cursor.x,
						row_ptr->height,
						LColor::selection);

		} else if (y > sel_start_cursor.y && y < sel_end_cursor.y) {
			pain.fillRectangle(0, offset,
					   paperwidth, row_ptr->height,
					   LColor::selection);
		}
	} // end of NOT FAST SELECTION code
	
	if (row_ptr->par->appendix){
		pain.line(1, offset,
			  1, offset + row_ptr->height,
			  LColor::appendixline);
		pain.line(paperwidth - 2, offset,
			  paperwidth - 2, offset + row_ptr->height,
			  LColor::appendixline);
	}

	if (row_ptr->par->pextra_type == LyXParagraph::PEXTRA_MINIPAGE) {
		/* draw a marker at the left margin! */ 
		LyXFont font = GetFont(row_ptr->par, 0);
		int asc = font.maxAscent();
		int x = (LYX_PAPER_MARGIN - font.width('|')) / 2;
		int y1 = (offset + row_ptr->baseline);
		int y2 = (offset + row_ptr->baseline) - asc;
		pain.line(x, y1, x, y2, LColor::minipageline);
	}       
	if (row_ptr->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE) {
		LyXFont font(LyXFont::ALL_SANE);
		font.setSize(LyXFont::SIZE_FOOTNOTE);
		font.setColor(LColor::red);
		
		int box_x = LYX_PAPER_MARGIN;
		box_x += font.textWidth(" wide-tab ", 10);
		if (row_ptr->previous && 
		    row_ptr->previous->par->footnoteflag != LyXParagraph::OPEN_FOOTNOTE){
			string fs;
			switch (row_ptr->par->footnotekind) {
			case LyXParagraph::MARGIN:
				fs = " margin";
				break;
			case LyXParagraph::FIG:
				fs = " fig";
				break;
			case LyXParagraph::TAB:
				fs = " tab";
				break;
			case LyXParagraph::WIDE_FIG:
				fs = " wide-fig";
				break;
			case LyXParagraph::WIDE_TAB:
				fs = " wide-tab";
				break;
			case LyXParagraph::ALGORITHM:
				fs = " alg";
				break;
			case LyXParagraph::FOOTNOTE:
				fs = " foot";
				break;
			}

#if 0
			// Determine background color.
			gc_type back = gc_lighted;
			if (mono_video) {
				back = gc_clear;
			}
#endif
			pain.fillRectangle(LYX_PAPER_MARGIN,
					   offset + 1,
					   box_x - LYX_PAPER_MARGIN,
					   int(font.maxAscent()
					       + font.maxDescent()),
					   LColor::footnotebg);

			pain.line(LYX_PAPER_MARGIN, offset,
				  paperwidth - LYX_PAPER_MARGIN, offset,
				  LColor::footnoteframe);

			pain.text(LYX_PAPER_MARGIN,
				  offset + int(font.maxAscent()) + 1,
				  fs, font);

			pain.line(LYX_PAPER_MARGIN, offset,
				  LYX_PAPER_MARGIN,
				  offset + int(font.maxAscent()
					       + font.maxDescent()),
				  LColor::footnoteframe);

			pain.line(LYX_PAPER_MARGIN,
				  offset + int(font.maxAscent()
					       + font.maxDescent()) + 1,
				  box_x,
				  offset + int(font.maxAscent()
					       + font.maxDescent()) + 1,
				  LColor::footnoteframe);
			
		}
		
		/* draw the open floats in a red box */
		pain.line(box_x, offset,
			  box_x, offset + row_ptr->height,
			  LColor::footnoteframe);

		pain.line(paperwidth - LYX_PAPER_MARGIN,
			  offset,
			  paperwidth - LYX_PAPER_MARGIN,
			  offset + row_ptr->height,
			  LColor::footnoteframe);
	} else  {
		if (row_ptr->previous &&
		    row_ptr->previous->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE) {
			LyXFont font(LyXFont::ALL_SANE);
			font.setSize(LyXFont::SIZE_FOOTNOTE);

			int box_x = LYX_PAPER_MARGIN;
			box_x += font.textWidth(" wide-tab ", 10);

			pain.line(box_x, offset,
				  paperwidth - LYX_PAPER_MARGIN,
				  offset, LColor::footnote);
		}
	}
	
	LyXLayout const & layout = textclasslist.Style(parameters->textclass,
					   row_ptr->par->GetLayout());
	firstpar = row_ptr->par->FirstPhysicalPar();
	
	y_top = 0;
	y_bottom = row_ptr->height;
	
	/* is it a first row? */ 
	if (row_ptr->pos == 0
	    && row_ptr->par == firstpar) {
		
		 /* start of appendix? */
		if (row_ptr->par->start_of_appendix){
			owner_->painter().line(1, offset,
					       paperwidth - 2, offset,
					       LColor::appendixline);
		}

		/* think about the margins */ 
		if (!row_ptr->previous)
			y_top += LYX_PAPER_MARGIN;
		
		if (row_ptr->par->pagebreak_top){ /* draw a top pagebreak  */
			pain.line(0, offset + y_top + 2 * DefaultHeight(),
				  paperwidth, offset + y_top + 2 * DefaultHeight(),
				  LColor::pagebreak, Painter::line_onoffdash);
			y_top += 3 * DefaultHeight();
		}
		
		if (row_ptr->par->added_space_top.kind() == VSpace::VFILL) {
			/* draw a vfill top  */
			pain.line(0, offset + 2 + y_top,
				  LYX_PAPER_MARGIN, offset + 2 + y_top,
				  LColor::vfillline);
			
			pain.line(0, offset + y_top + 3 * DefaultHeight(),
				  LYX_PAPER_MARGIN,
				  offset + y_top + 3 * DefaultHeight(),
				  LColor::vfillline);
			
			pain.line(LYX_PAPER_MARGIN / 2, offset + 2 + y_top,
				  LYX_PAPER_MARGIN / 2,
				  offset + y_top + 3 * DefaultHeight(),
				  LColor::vfillline);
			
			y_top += 3 * DefaultHeight();
		}
		
		/* think about user added space */ 
		y_top += int(row_ptr->par->added_space_top.inPixels());
		
		/* think about the parskip */ 
		/* some parskips VERY EASY IMPLEMENTATION */ 
		if (parameters->paragraph_separation == BufferParams::PARSEP_SKIP) {
			if (layout.latextype == LATEX_PARAGRAPH
			    && firstpar->GetDepth() == 0
			    && firstpar->Previous())
				y_top += parameters->getDefSkip().inPixels();
			else if (firstpar->Previous()
				 && textclasslist.Style(parameters->textclass,
						   firstpar->Previous()->GetLayout()).latextype == LATEX_PARAGRAPH
				 && firstpar->Previous()->GetDepth() == 0)
				// is it right to use defskip here, too? (AS) 
				y_top += parameters->getDefSkip().inPixels();
		}
		
		if (row_ptr->par->line_top) {      /* draw a top line  */
			y_top +=  GetFont(row_ptr->par, 0).ascent('x');

			pain.line(0, offset + y_top,
				  paperwidth, offset + y_top,
				  LColor::topline,
				  Painter::line_solid,
				  Painter::line_thick);
			
			y_top +=  GetFont(row_ptr->par, 0).ascent('x');
		}
		
		/* should we print a label? */ 
		if (layout.labeltype >= LABEL_STATIC
		    && (layout.labeltype != LABEL_STATIC
			|| layout.latextype != LATEX_ENVIRONMENT
			|| row_ptr->par->IsFirstInSequence())) {
			font = GetFont(row_ptr->par, -2);
			if (!row_ptr->par->GetLabestring().empty()) {
				tmpx = x;
				string tmpstring = row_ptr->par->GetLabestring();
				
				if (layout.labeltype == LABEL_COUNTER_CHAPTER) {
					if (parameters->secnumdepth >= 0){
						/* this is special code for the chapter layout. This is printed in
						 * an extra row and has a pagebreak at the top. */
						maxdesc = int(font.maxDescent() * layout.spacing.getValue() * parameters->spacing.getValue())
							+ int(layout.parsep) * DefaultHeight();
						if (direction == LYX_DIR_RIGHT_TO_LEFT)
							tmpx = paperwidth - LeftMargin(row_ptr) - 
								font.stringWidth(tmpstring);
						pain.text(int(tmpx),
							  offset + row_ptr->baseline - row_ptr->ascent_of_text - maxdesc,
							  tmpstring, font);
					}
				} else {
					if (direction == LYX_DIR_LEFT_TO_RIGHT)
						tmpx = x - font.stringWidth(layout.labelsep)
							- font.stringWidth(tmpstring);
					else {
						tmpx = paperwidth - LeftMargin(row_ptr)
							+ font.stringWidth(layout.labelsep);
						if (row_ptr->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE)  {
							LyXFont font(LyXFont::ALL_SANE);
							font.setSize(LyXFont::SIZE_SMALL);
							tmpx += font.textWidth("Mwide-figM", 10);
						}
					}
					/* draw it! */
					pain.text(int(tmpx),
						  offset + row_ptr->baseline,
						  tmpstring, font);
				}
			}
			/* the labels at the top of an environment. More or less for bibliography */ 
		} else if (layout.labeltype == LABEL_TOP_ENVIRONMENT ||
			   layout.labeltype == LABEL_BIBLIO ||
			   layout.labeltype == LABEL_CENTERED_TOP_ENVIRONMENT) {
			if (row_ptr->par->IsFirstInSequence()) {
				font = GetFont(row_ptr->par, -2);
				if (!row_ptr->par->GetLabestring().empty()) {
					string tmpstring = row_ptr->par->GetLabestring();
					
					maxdesc = int(font.maxDescent() * layout.spacing.getValue() * parameters->spacing.getValue()
							 + (layout.labelbottomsep * DefaultHeight()));
					
					tmpx = x;
					if (layout.labeltype == LABEL_CENTERED_TOP_ENVIRONMENT){
						tmpx = ( ((direction == LYX_DIR_LEFT_TO_RIGHT)
							  ? x : LeftMargin(row_ptr) )
							 + paperwidth - RightMargin(row_ptr) ) / 2; 
						tmpx -= (font.stringWidth(tmpstring)/2);
					} else if (direction == LYX_DIR_RIGHT_TO_LEFT)
						tmpx = paperwidth - LeftMargin(row_ptr) - 
							font.stringWidth(tmpstring);
					pain.text(int(tmpx),
						  offset + row_ptr->baseline
						  - row_ptr->ascent_of_text
						  - maxdesc,
						  tmpstring, font);
				}
			}
		}
		if (layout.labeltype == LABEL_BIBLIO) { // ale970302
			if (row_ptr->par->bibkey) {
				font = GetFont(row_ptr->par, -1);
				if (direction == LYX_DIR_LEFT_TO_RIGHT)
					tmpx = x - font.stringWidth(layout.labelsep)
						- row_ptr->par->bibkey->width(owner_->painter(), font);
				else
					tmpx = paperwidth - LeftMargin(row_ptr)
						+ font.stringWidth(layout.labelsep);
				row_ptr->par->bibkey->draw(owner_->painter(),
							   font,
							   offset + row_ptr->baseline, 
							   tmpx);
			}
		} 
	}
	
	/* is it a last row? */
	par = row_ptr->par->LastPhysicalPar();
	if (row_ptr->par->ParFromPos(RowLast(row_ptr) + 1) == par
	    && (!row_ptr->next || row_ptr->next->par != row_ptr->par)) {     
		
		/* think about the margins */ 
		if (!row_ptr->next)
			y_bottom -= LYX_PAPER_MARGIN;
		
		/* draw a bottom pagebreak */ 
		if (firstpar->pagebreak_bottom) {
			pain.line(0, offset + y_bottom - 2 * DefaultHeight(),
				  paperwidth,
				  offset + y_bottom - 2 * DefaultHeight(),
				  LColor::pagebreak);
			y_bottom -= 3 * DefaultHeight();
		}
		
		if (firstpar->added_space_bottom.kind() == VSpace::VFILL) {
			/* draw a vfill bottom  */
			pain.line(0, offset + y_bottom - 3 * DefaultHeight(),
				  LYX_PAPER_MARGIN,
				  offset + y_bottom - 3 * DefaultHeight(),
				  LColor::vfillline);
			pain.line(0, offset + y_bottom - 2,
				  LYX_PAPER_MARGIN,
				  offset + y_bottom - 2,
				  LColor::vfillline);
			pain.line(LYX_PAPER_MARGIN / 2,
				  offset + y_bottom - 3 * DefaultHeight(),
				  LYX_PAPER_MARGIN / 2,
				  offset + y_bottom - 2,
				  LColor::vfillline);
			y_bottom -= 3* DefaultHeight();
		}
		
		/* think about user added space */ 
		y_bottom -= int(firstpar->added_space_bottom.inPixels());
		
		if (firstpar->line_bottom) {
			/* draw a bottom line */
			y_bottom -= GetFont(par, par->Last() - 1).ascent('x');
			pain.line(0, offset + y_bottom,
				  paperwidth, offset + y_bottom,
				  LColor::topline, Painter::line_solid,
				  Painter::line_thick);
			y_bottom -= GetFont(par, par->Last() - 1).ascent('x');
		}
	}
	
	/* draw the text in the pixmap */  
	pos_end = RowLast(row_ptr);
	
	vpos = row_ptr->pos;
	/* table stuff -- begin*/
	if (row_ptr->par->table) {
		bool on_off;
		int cell = NumberOfCell(row_ptr->par, row_ptr->pos);
		float x_old = x;
		x += row_ptr->par->table->GetBeginningOfTextInCell(cell);
		
		while (vpos <= pos_end)  {
			pos = vis2log(vpos);
			if (row_ptr->par->IsNewline(pos)) {
				
				x = x_old + row_ptr->par->table->WidthOfColumn(cell);
				/* draw the table lines, still very simple */
				on_off = !row_ptr->par->table->TopLine(cell);
				if ((!on_off ||
				     !row_ptr->par->table->TopAlreadyDrawed(cell)) &&
				    !row_ptr->par->table->IsContRow(cell))
					pain.line(int(x_old),
						  offset + row_ptr->baseline - row_ptr->ascent_of_text,
						  int(x - x_old),
						  offset + row_ptr->baseline - row_ptr->ascent_of_text,
						  LColor::tableline);
				
				on_off = !row_ptr->par->table->BottomLine(cell);
				if ((!on_off && !row_ptr->par->table->RowHasContRow(cell)) ||
				    row_ptr->par->table->VeryLastRow(cell))

					pain.line(int(x_old),
						  offset + y_bottom - 1,
						  int(x - x_old),
						  offset + y_bottom - 1,
						  LColor::tableline);

				on_off = !row_ptr->par->table->LeftLine(cell);

				pain.line(int(x_old),
					  offset + row_ptr->baseline - row_ptr->ascent_of_text,
					  int(x_old),
					  offset + y_bottom,
					  LColor::tableline,
					  Painter::line_onoffdash);
				
				on_off = !row_ptr->par->table->RightLine(cell);

				pain.line(int(x) - row_ptr->par->table->AdditionalWidth(cell),
					  offset + row_ptr->baseline - row_ptr->ascent_of_text,
					  int(x) - row_ptr->par->table->AdditionalWidth(cell),
					  offset + y_bottom - 1,
					  LColor::tableline,
					  Painter::line_onoffdash);
				
				x_old = x;
                /* take care about the alignment and other spaces */
				++cell;
				x += row_ptr->par->table->GetBeginningOfTextInCell(cell);
				if (row_ptr->par->table->IsFirstCell(cell))
					--cell; // little hack, sorry
				++vpos;
			} else if (row_ptr->par->IsHfill(pos)) {
				x += 1;

				pain.line(int(x),
					  offset + row_ptr->baseline - DefaultHeight() / 2,
					  int(x),
					  offset + row_ptr->baseline,
					  LColor::vfillline);
				
				x += 2;
				++vpos;
			} else {
				if (row_ptr->par->IsSeparator(pos)) {
					tmpx = x;
					x+= SingleWidth(row_ptr->par, pos);
					/* -------> Only draw protected spaces when not in
					 * free-spacing mode. */
					if (row_ptr->par->GetChar(pos) == LyXParagraph::META_PROTECTED_SEPARATOR && !layout.free_spacing) {
						pain.line(int(tmpx),
							  offset + row_ptr->baseline - 3,
							  int(tmpx),
							  offset + row_ptr->baseline - 1,
							  LColor::vfillline);
						
						pain.line(int(tmpx),
							  offset + row_ptr->baseline - 1,
							  int(x - tmpx - 2),
							  offset + row_ptr->baseline - 1,
							  LColor::vfillline);
						
						pain.line(int(x - 2),
							  offset + row_ptr->baseline - 3,
							  int(x - 2),
							  offset + row_ptr->baseline - 1,
							  LColor::vfillline);
						
						/* what about underbars? */
						font = GetFont(row_ptr->par, pos); 
						if (font.underbar() == LyXFont::ON
						    && font.latex() != LyXFont::ON) {
							pain.line(int(tmpx),
								  offset + row_ptr->baseline + 2,
								  int(x - tmpx),
								  offset + row_ptr->baseline + 2);
						}
					}
					++vpos;
				} else
					draw(row_ptr, vpos, offset, x);
			}
		}
		
		/* do not forget the very last cell. This has no NEWLINE so 
		 * ignored by the code above*/ 
		if (cell == row_ptr->par->table->GetNumberOfCells()-1){
			x = x_old + row_ptr->par->table->WidthOfColumn(cell);
			on_off = !row_ptr->par->table->TopLine(cell);
			if ((!on_off ||
			     !row_ptr->par->table->TopAlreadyDrawed(cell)) &&
			    !row_ptr->par->table->IsContRow(cell))

				pain.line(int(x_old),
					  offset + row_ptr->baseline - row_ptr->ascent_of_text,
					  int(x - x_old),
					  offset + row_ptr->baseline - row_ptr->ascent_of_text,
					  LColor::tableline,
					  Painter::line_onoffdash);
			on_off = !row_ptr->par->table->BottomLine(cell);
			if ((!on_off && !row_ptr->par->table->RowHasContRow(cell)) ||
			    row_ptr->par->table->VeryLastRow(cell))

				pain.line(int(x_old),
					  offset + y_bottom - 1,
					  int(x - x_old),
					  offset + y_bottom - 1,
					  LColor::tableline,
					  Painter::line_onoffdash);
			
			on_off = !row_ptr->par->table->LeftLine(cell);

			pain.line(int(x_old),
				  offset + row_ptr->baseline - row_ptr->ascent_of_text,
				  int(x_old),
				  offset + y_bottom - 1,
				  LColor::tableline,
				  Painter::line_onoffdash);
			
			on_off = !row_ptr->par->table->RightLine(cell);

			pain.line(int(x) - row_ptr->par->table->AdditionalWidth(cell),
				  offset + row_ptr->baseline - row_ptr->ascent_of_text,
				  int(x) - row_ptr->par->table->AdditionalWidth(cell),
				  offset + y_bottom - 1,
				  LColor::tableline,
				  Painter::line_onoffdash);
		}
	} else {
		/* table stuff -- end*/
		LyXParagraph::size_type main_body = 
			BeginningOfMainBody(row_ptr->par);
		if (main_body > 0 &&
		    (main_body-1 > pos_end || 
		     !row_ptr->par->IsLineSeparator(main_body-1)))
			main_body = 0;

		while (vpos <= pos_end)  {
			pos = vis2log(vpos);
			if (main_body > 0 && pos == main_body-1) {
				x += fill_label_hfill
					+ GetFont(row_ptr->par, -2).stringWidth(layout.labelsep)
					- SingleWidth(row_ptr->par, main_body-1);
			}

			if (row_ptr->par->IsHfill(pos)) {
				x += 1;
				pain.line(int(x),
					  offset + row_ptr->baseline - DefaultHeight() / 2,
					  int(x),
					  offset + row_ptr->baseline,
					  LColor::vfillline);
				
				if (HfillExpansion(row_ptr, pos)) {
					if (pos >= main_body) {
						pain.line(int(x),
							  offset + row_ptr->baseline - DefaultHeight() / 4,
							  int(fill_hfill),
							  offset + row_ptr->baseline - DefaultHeight() / 4,
							  LColor::vfillline,
							  Painter::line_onoffdash);
						x += fill_hfill;
                    } else {
			    pain.line(int(x),
				      offset + row_ptr->baseline - DefaultHeight() / 4,
				      int(fill_label_hfill),
				      offset + row_ptr->baseline - DefaultHeight() / 4,
				      LColor::vfillline,
				      Painter::line_onoffdash);
			    
			    x += fill_label_hfill;
                    }
					pain.line(int(x),
						  offset + row_ptr->baseline - DefaultHeight() / 2,
						  int(x),
						  offset + row_ptr->baseline,
						  LColor::vfillline);
				}
				x += 2;
				++vpos;
			} else {
				if (row_ptr->par->IsSeparator(pos)) {
					tmpx = x;
					x+= SingleWidth(row_ptr->par, pos);
					if (pos >= main_body)
						x+= fill_separator;
					/* -------> Only draw protected spaces when not in
					 * free-spacing mode. */
					if (row_ptr->par->GetChar(pos) == LyXParagraph::META_PROTECTED_SEPARATOR && !layout.free_spacing) {

						pain.line(int(tmpx),
							  offset + row_ptr->baseline - 3,
							  int(tmpx),
							  offset + row_ptr->baseline - 1,
							  LColor::vfillline);
						
						pain.line(int(tmpx),
							  offset + row_ptr->baseline - 1,
							  int(tmpx),
							  offset + row_ptr->baseline - 1,
							  LColor::vfillline);

						pain.line(int(x - 2),
							  offset + row_ptr->baseline - 3,
							  int(x - 2),
							  offset + row_ptr->baseline - 1,
							  LColor::vfillline);
						
						/* what about underbars? */
						font = GetFont(row_ptr->par, pos); 
						if (font.underbar() == LyXFont::ON
						    && font.latex() != LyXFont::ON) {
							pain.line(int(tmpx),
								  offset + row_ptr->baseline + 2,
								  int(x - tmpx),
								  offset + row_ptr->baseline + 2);
						}
					}
					++vpos;
				} else
					draw(row_ptr, vpos, offset, x);
			}
		}
	}
#if 0
	// check for FAST SELECTION
	if (fast_selection || mono_video) {
		if (selection) {
			
			/* selection code */ 
			if (sel_start_cursor.row == row_ptr && sel_end_cursor.row == row_ptr) {
				scr.fillRectangle(gc_select, sel_start_cursor.x, offset,
						  sel_end_cursor.x - sel_start_cursor.x,
						  row_ptr->height); 
			} else if (sel_start_cursor.row == row_ptr) {
				scr.fillRectangle(gc_select, sel_start_cursor.x, offset,
						  paperwidth - sel_start_cursor.x,
						  row_ptr->height);
			} else if (sel_end_cursor.row == row_ptr) {
				scr.fillRectangle(gc_select, 0, offset,
						  sel_end_cursor.x,
						  row_ptr->height);
			} else if (y > sel_start_cursor.y && y < sel_end_cursor.y) {
				scr.fillRectangle(gc_select, 0, offset,
						  paperwidth, row_ptr->height);
				
			}
		}
	}
// end of FAST SELECTION code
	/* thats it */
#endif
}
#else
void LyXText::GetVisibleRow(LyXScreen & scr, int offset, 
			    Row * row_ptr, long y)
{
	/* returns a printed row */
	LyXDirection direction = row_ptr->par->getParDirection();
	LyXParagraph::size_type vpos, pos, pos_end;
	float x, tmpx;
	int y_top, y_bottom;
	float fill_separator, fill_hfill, fill_label_hfill;
	LyXParagraph * par, * firstpar;
	LyXFont font;
	int maxdesc;
	if (row_ptr->height <= 0) {
		lyxerr << "LYX_ERROR: row.height: " << row_ptr->height << endl;
		return;
	}
	PrepareToPrint(row_ptr, x, fill_separator,
		       fill_hfill, fill_label_hfill);

	/* initialize the pixmap */
	
	scr.fillRectangle(gc_clear,
			  0, offset, paperwidth, row_ptr->height);
	// check for NOT FAST SELECTION
	if (!fast_selection && !mono_video && selection) {
		/* selection code */ 
		if (sel_start_cursor.row == row_ptr &&
		    sel_end_cursor.row == row_ptr) {
		        if (sel_start_cursor.x < sel_end_cursor.x)
			  scr.fillRectangle(gc_selection, sel_start_cursor.x,
					  offset,
					  sel_end_cursor.x - sel_start_cursor.x,
					  row_ptr->height);
			else
			  scr.fillRectangle(gc_selection, sel_end_cursor.x,
					  offset,
					  sel_start_cursor.x - sel_end_cursor.x,
					  row_ptr->height);
		}
		else if (sel_start_cursor.row == row_ptr) {
		     if (direction == LYX_DIR_LEFT_TO_RIGHT)
			scr.fillRectangle(gc_selection, sel_start_cursor.x,
					  offset,
					  paperwidth - sel_start_cursor.x,
					  row_ptr->height);
		     else
			scr.fillRectangle(gc_selection, 0, offset,
					  sel_start_cursor.x, row_ptr->height);
		} else if (sel_end_cursor.row == row_ptr) {
		     if (direction == LYX_DIR_LEFT_TO_RIGHT)
			scr.fillRectangle(gc_selection, 0, offset,
					  sel_end_cursor.x, row_ptr->height);
		     else
			scr.fillRectangle(gc_selection, sel_end_cursor.x,
					  offset,
					  paperwidth - sel_end_cursor.x,
					  row_ptr->height);
		} else if (y > sel_start_cursor.y && y < sel_end_cursor.y) {
			scr.fillRectangle(gc_selection, 0, offset,
					  paperwidth, row_ptr->height);
			
		}
	} // end of NOT FAST SELECTION code
	
	if (row_ptr->par->appendix){
	  scr.drawVerticalLine(gc_math, 1, offset, offset+row_ptr->height);
	  scr.drawVerticalLine(gc_math, paperwidth-2 , offset, offset+row_ptr->height);
	}

	if (row_ptr->par->pextra_type == LyXParagraph::PEXTRA_MINIPAGE) {
		/* draw a marker at the left margin! */ 
		LyXFont font = GetFont(row_ptr->par, 0);
		int asc = font.maxAscent();
		int x = (LYX_PAPER_MARGIN - font.width('|')) / 2;
		int y1 = (offset + row_ptr->baseline);
		int y2 = (offset + row_ptr->baseline) - asc;

		scr.drawVerticalLine(gc_minipage, x, y1, y2);
	}       
	if (row_ptr->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE) {
		LyXFont font(LyXFont::ALL_SANE);
		font.setSize(LyXFont::SIZE_FOOTNOTE);
#ifdef USE_PAINTER
		font.setColor(LColor::footnote);
#else
		font.setColor(LyXFont::RED);
#endif
		
		int box_x = LYX_PAPER_MARGIN;
		box_x += font.textWidth(" wide-tab ", 10);
		if (row_ptr->previous && 
		    row_ptr->previous->par->footnoteflag != LyXParagraph::OPEN_FOOTNOTE){
			string fs;
			switch (row_ptr->par->footnotekind) {
			case LyXParagraph::MARGIN:
				fs = " margin";
				break;
			case LyXParagraph::FIG:
				fs = " fig";
				break;
			case LyXParagraph::TAB:
				fs = " tab";
				break;
			case LyXParagraph::WIDE_FIG:
				fs = " wide-fig";
				break;
			case LyXParagraph::WIDE_TAB:
				fs = " wide-tab";
				break;
			case LyXParagraph::ALGORITHM:
				fs = " alg";
				break;
			case LyXParagraph::FOOTNOTE:
				fs = " foot";
				break;
			}
			
			// Determine background color.
			gc_type back = gc_lighted;
			if (mono_video) {
				back = gc_clear;
			}
			scr.fillRectangle(back, LYX_PAPER_MARGIN, offset+1,
					  box_x - LYX_PAPER_MARGIN, 
					  int(font.maxAscent())+
					  int(font.maxDescent()));
			
			scr.drawLine(gc_foot,
				     offset,
				     LYX_PAPER_MARGIN,
				     paperwidth - 2*LYX_PAPER_MARGIN);
			
			scr.drawString(font, fs,
				       offset + int(font.maxAscent())+1,
				       LYX_PAPER_MARGIN);
			scr.drawVerticalLine(gc_foot,
					     LYX_PAPER_MARGIN,
					     offset,
					     offset
					     + int(font.maxAscent())+
					     int(font.maxDescent()));
			
			scr.drawLine(gc_foot,
				     offset
				     + int(font.maxAscent())
				     + int(font.maxDescent()) + 1,
				     LYX_PAPER_MARGIN, box_x - LYX_PAPER_MARGIN); 
		}
		
		/* draw the open floats in a red box */
		scr.drawVerticalLine(gc_foot,
				     box_x,
				     offset,  offset + row_ptr->height);
		
		scr.drawVerticalLine(gc_foot,
				     paperwidth - LYX_PAPER_MARGIN,
				     offset,
				     offset + row_ptr->height);
					
	} else  {
		if (row_ptr->previous &&
		    row_ptr->previous->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE) {
			LyXFont font(LyXFont::ALL_SANE);
			font.setSize(LyXFont::SIZE_FOOTNOTE);

			int box_x = LYX_PAPER_MARGIN;
			box_x += font.textWidth(" wide-tab ", 10);
			
			scr.drawLine(gc_foot,
				     offset,
				     box_x,
				     paperwidth - LYX_PAPER_MARGIN - box_x);
		}
	}
	
	LyXLayout const & layout = textclasslist.Style(parameters->textclass,
					   row_ptr->par->GetLayout());
	firstpar = row_ptr->par->FirstPhysicalPar();
	
	y_top = 0;
	y_bottom = row_ptr->height;
	
	/* is it a first row? */ 
	if (row_ptr->pos == 0
	    && row_ptr->par == firstpar) {
		
		 /* start of appendix? */
		if (row_ptr->par->start_of_appendix){
		  scr.drawLine(gc_math, 
			       offset,
			       1, paperwidth-2);
		}

		/* think about the margins */ 
		if (!row_ptr->previous)
			y_top += LYX_PAPER_MARGIN;
		
		if (row_ptr->par->pagebreak_top){ /* draw a top pagebreak  */
			scr.drawOnOffLine(offset + y_top + 2 * DefaultHeight(),
					  0, paperwidth);
			y_top += 3 * DefaultHeight();
		}
		
		if (row_ptr->par->added_space_top.kind() == VSpace::VFILL) {
			/* draw a vfill top  */
			scr.drawLine(gc_fill, 
				     offset + 2 + y_top,
				     0, LYX_PAPER_MARGIN);
			scr.drawLine(gc_fill,
				     offset + y_top + 3 * DefaultHeight(),
				     0, LYX_PAPER_MARGIN);
			scr.drawVerticalOnOffLine(LYX_PAPER_MARGIN / 2, 
						  offset + 2 + y_top,
						  offset + y_top + 3 *
						  DefaultHeight());
			
			y_top += 3 * DefaultHeight();
		}
		
		/* think about user added space */ 
		y_top += int(row_ptr->par->added_space_top.inPixels());
		
		/* think about the parskip */ 
		/* some parskips VERY EASY IMPLEMENTATION */ 
		if (parameters->paragraph_separation == BufferParams::PARSEP_SKIP) {
			if (layout.latextype == LATEX_PARAGRAPH
			    && firstpar->GetDepth() == 0
			    && firstpar->Previous())
				y_top += parameters->getDefSkip().inPixels();
			else if (firstpar->Previous()
				 && textclasslist.Style(parameters->textclass,
						   firstpar->Previous()->GetLayout()).latextype == LATEX_PARAGRAPH
				 && firstpar->Previous()->GetDepth() == 0)
				// is it right to use defskip here, too? (AS) 
				y_top += parameters->getDefSkip().inPixels();
		}
		
		if (row_ptr->par->line_top) {      /* draw a top line  */
			y_top +=  GetFont(row_ptr->par, 0).ascent('x');

			scr.drawThickLine(offset + y_top,
					  0, paperwidth);
			y_top +=  GetFont(row_ptr->par, 0).ascent('x');
		}
		
		/* should we print a label? */ 
		if (layout.labeltype >= LABEL_STATIC
		    && (layout.labeltype != LABEL_STATIC
			|| layout.latextype != LATEX_ENVIRONMENT
			|| row_ptr->par->IsFirstInSequence())) {
			font = GetFont(row_ptr->par, -2);
			if (!row_ptr->par->GetLabestring().empty()) {
				tmpx = x;
				string tmpstring = row_ptr->par->GetLabestring();
				
				if (layout.labeltype == LABEL_COUNTER_CHAPTER) {
					if (parameters->secnumdepth >= 0){
						/* this is special code for the chapter layout. This is printed in
						 * an extra row and has a pagebreak at the top. */
						maxdesc = int(font.maxDescent() * layout.spacing.getValue() * parameters->spacing.getValue())
							+ int(layout.parsep) * DefaultHeight();
						if (direction == LYX_DIR_RIGHT_TO_LEFT)
							tmpx = paperwidth - LeftMargin(row_ptr) - 
								font.stringWidth(tmpstring);
						scr.drawString(font, tmpstring,
							       offset + row_ptr->baseline
							       - row_ptr->ascent_of_text - maxdesc,
							       int(tmpx));
					}
				} else {
					if (direction == LYX_DIR_LEFT_TO_RIGHT)
						tmpx = x - font.stringWidth(layout.labelsep)
							- font.stringWidth(tmpstring);
					else {
						tmpx = paperwidth - LeftMargin(row_ptr)
							+ font.stringWidth(layout.labelsep);
						if (row_ptr->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE)  {
							LyXFont font(LyXFont::ALL_SANE);
							font.setSize(LyXFont::SIZE_SMALL);
							tmpx += font.textWidth("Mwide-figM", 10);
						}
					}
					/* draw it! */
					scr.drawString(font, tmpstring,
						       offset + row_ptr->baseline, int(tmpx));
				}
			}
			/* the labels at the top of an environment. More or less for bibliography */ 
		} else if (layout.labeltype == LABEL_TOP_ENVIRONMENT ||
			   layout.labeltype == LABEL_BIBLIO ||
			   layout.labeltype == LABEL_CENTERED_TOP_ENVIRONMENT) {
			if (row_ptr->par->IsFirstInSequence()) {
				font = GetFont(row_ptr->par, -2);
				if (!row_ptr->par->GetLabestring().empty()) {
					string tmpstring = row_ptr->par->GetLabestring();
					
					maxdesc = int(font.maxDescent() * layout.spacing.getValue() * parameters->spacing.getValue()
							 + (layout.labelbottomsep * DefaultHeight()));
					
					tmpx = x;
					if (layout.labeltype == LABEL_CENTERED_TOP_ENVIRONMENT){
						tmpx = ( ((direction == LYX_DIR_LEFT_TO_RIGHT)
							  ? x : LeftMargin(row_ptr) )
							 + paperwidth - RightMargin(row_ptr) ) / 2; 
						tmpx -= (font.stringWidth(tmpstring)/2);
					} else if (direction == LYX_DIR_RIGHT_TO_LEFT)
						tmpx = paperwidth - LeftMargin(row_ptr) - 
							font.stringWidth(tmpstring);

					scr.drawString(font, tmpstring,
						       offset + row_ptr->baseline
						       - row_ptr->ascent_of_text - maxdesc,  
						       int(tmpx));		    
				}
			}
		}
		if (layout.labeltype == LABEL_BIBLIO) { // ale970302
			if (row_ptr->par->bibkey) {
				font = GetFont(row_ptr->par, -1);
				if (direction == LYX_DIR_LEFT_TO_RIGHT)
					tmpx = x - font.stringWidth(layout.labelsep)
						- row_ptr->par->bibkey->Width(font);
				else
					tmpx = paperwidth - LeftMargin(row_ptr)
						+ font.stringWidth(layout.labelsep);
				row_ptr->par->bibkey->Draw(font, scr,
							   offset + row_ptr->baseline, 
							   tmpx);
			}
		} 
	}
	
	/* is it a last row? */
	par = row_ptr->par->LastPhysicalPar();
	if (row_ptr->par->ParFromPos(RowLast(row_ptr) + 1) == par
	    && (!row_ptr->next || row_ptr->next->par != row_ptr->par)) {     
		
		/* think about the margins */ 
		if (!row_ptr->next)
			y_bottom -= LYX_PAPER_MARGIN;
		
		/* draw a bottom pagebreak */ 
		if (firstpar->pagebreak_bottom) {
			scr.drawOnOffLine(offset + y_bottom - 2 *
					  DefaultHeight(),
					  0, paperwidth);
			y_bottom -= 3 * DefaultHeight();
		}
		
		if (firstpar->added_space_bottom.kind() == VSpace::VFILL) {
			/* draw a vfill bottom  */
			scr.drawLine(gc_fill,
				     offset + y_bottom - 3 * DefaultHeight(),
				     0, LYX_PAPER_MARGIN);
			scr.drawLine(gc_fill, offset + y_bottom - 2,
				     0, LYX_PAPER_MARGIN);
			scr.drawVerticalOnOffLine(LYX_PAPER_MARGIN / 2, 
						  offset + y_bottom - 3 * DefaultHeight(),
						  offset + y_bottom - 2
				);	    
			y_bottom -= 3* DefaultHeight();
		}
		
		/* think about user added space */ 
		y_bottom -= int(firstpar->added_space_bottom.inPixels());
		
		if (firstpar->line_bottom) {
			/* draw a bottom line */
			y_bottom -= GetFont(par, par->Last() - 1).ascent('x');

			scr.drawThickLine(offset + y_bottom,
					  0, paperwidth);	    
			y_bottom -= GetFont(par, par->Last() - 1).ascent('x');
		}
	}
	
	/* draw the text in the pixmap */  
	pos_end = RowLast(row_ptr);
	
	vpos = row_ptr->pos;
	/* table stuff -- begin*/
	if (row_ptr->par->table) {
		bool on_off;
		int cell = NumberOfCell(row_ptr->par, row_ptr->pos);
		float x_old = x;
		x += row_ptr->par->table->GetBeginningOfTextInCell(cell);
		
		while (vpos <= pos_end)  {
			pos = vis2log(vpos);
			if (row_ptr->par->IsNewline(pos)) {
				
				x = x_old + row_ptr->par->table->WidthOfColumn(cell);
				/* draw the table lines, still very simple */
				on_off = !row_ptr->par->table->TopLine(cell);
				if ((!on_off ||
				     !row_ptr->par->table->TopAlreadyDrawed(cell)) &&
				    !row_ptr->par->table->IsContRow(cell))
					scr.drawTableLine(offset + row_ptr->baseline -
							  row_ptr->ascent_of_text,
							  int(x_old), int(x - x_old), on_off);
				on_off = !row_ptr->par->table->BottomLine(cell);
				if ((!on_off && !row_ptr->par->table->RowHasContRow(cell)) ||
				    row_ptr->par->table->VeryLastRow(cell))
					scr.drawTableLine(offset + y_bottom - 1,
							  int(x_old), int(x - x_old), on_off);
				on_off = !row_ptr->par->table->LeftLine(cell);
				
				scr.drawVerticalTableLine(int(x_old), 
							  offset + row_ptr->baseline -
							  row_ptr->ascent_of_text,
							  offset + y_bottom - 1,
							  on_off);
				on_off = !row_ptr->par->table->RightLine(cell);
				
				scr.drawVerticalTableLine(int(x) -
							  row_ptr->par->table->AdditionalWidth(cell),
							  offset + row_ptr->baseline -
							  row_ptr->ascent_of_text,
							  offset + y_bottom - 1,
							  on_off);
				x_old = x;
                /* take care about the alignment and other spaces */
				++cell;
				x += row_ptr->par->table->GetBeginningOfTextInCell(cell);
				if (row_ptr->par->table->IsFirstCell(cell))
					--cell; // little hack, sorry
				++vpos;
			} else if (row_ptr->par->IsHfill(pos)) {
				x += 1;
				
				scr.drawVerticalLine(gc_fill, int(x),  
						     offset + row_ptr->baseline - DefaultHeight()/2, 
						     offset + row_ptr->baseline);		
				x += 2;
				++vpos;
			} else {
				if (row_ptr->par->IsSeparator(pos)) {
					tmpx = x;
					x+= SingleWidth(row_ptr->par, pos);
					/* -------> Only draw protected spaces when not in
					 * free-spacing mode. */
					if (row_ptr->par->GetChar(pos) == LyXParagraph::META_PROTECTED_SEPARATOR && !layout.free_spacing) {
						scr.drawVerticalLine(gc_fill, int(tmpx),
								     offset + row_ptr->baseline - 3,
								     offset + row_ptr->baseline - 1);
						scr.drawLine(gc_fill,
							     offset + row_ptr->baseline - 1,
							     int(tmpx),
							     int(x-tmpx-2));
						scr.drawVerticalLine(gc_fill, int(x-2),
								     offset + row_ptr->baseline - 3,
								     offset + row_ptr->baseline - 1);			
						/* what about underbars? */
						font = GetFont(row_ptr->par, pos); 
						if (font.underbar() == LyXFont::ON
						    && font.latex() != LyXFont::ON) {
							scr.drawLine(gc_copy,
								     offset +
								     row_ptr->baseline + 2,
								     int(tmpx),
								     int(x - tmpx));			    
						}
					}
					++vpos;
				} else
					Draw(row_ptr, vpos, scr, offset, x);
			}
		}
		
		/* do not forget the very last cell. This has no NEWLINE so 
		 * ignored by the code above*/ 
		if (cell == row_ptr->par->table->GetNumberOfCells()-1){
			x = x_old + row_ptr->par->table->WidthOfColumn(cell);
			on_off = !row_ptr->par->table->TopLine(cell);
			if ((!on_off ||
			     !row_ptr->par->table->TopAlreadyDrawed(cell)) &&
			    !row_ptr->par->table->IsContRow(cell))
				
				scr.drawTableLine(offset + row_ptr->baseline -
						  row_ptr->ascent_of_text,
						  int(x_old), int(x - x_old), on_off);	    
			on_off = !row_ptr->par->table->BottomLine(cell);
			if ((!on_off && !row_ptr->par->table->RowHasContRow(cell)) ||
			    row_ptr->par->table->VeryLastRow(cell))
				
				scr.drawTableLine(offset + y_bottom - 1,
						  int(x_old), int(x - x_old), on_off);	    
			on_off = !row_ptr->par->table->LeftLine(cell);
			
			scr.drawVerticalTableLine(int(x_old), 
						  offset + row_ptr->baseline -
						  row_ptr->ascent_of_text,
						  offset + y_bottom - 1,
						  on_off);	    
			on_off = !row_ptr->par->table->RightLine(cell);
			
			scr.drawVerticalTableLine(int(x) -
						  row_ptr->par->table->AdditionalWidth(cell),
						  offset + row_ptr->baseline -
						  row_ptr->ascent_of_text,
						  offset + y_bottom - 1,
						  on_off);	    
		}
	} else {
		/* table stuff -- end*/
		LyXParagraph::size_type main_body = 
			BeginningOfMainBody(row_ptr->par);
		if (main_body > 0 &&
		    (main_body-1 > pos_end || 
		     !row_ptr->par->IsLineSeparator(main_body-1)))
			main_body = 0;

		while (vpos <= pos_end)  {
			pos = vis2log(vpos);
			if (main_body > 0 && pos == main_body-1) {
				x += fill_label_hfill
					+ GetFont(row_ptr->par, -2).stringWidth(layout.labelsep)
					- SingleWidth(row_ptr->par, main_body-1);
			}

			if (row_ptr->par->IsHfill(pos)) {
				x += 1;
				scr.drawVerticalLine(gc_fill, int(x),  
						     offset + row_ptr->baseline - DefaultHeight()/2, 
						     offset + row_ptr->baseline);		
				if (HfillExpansion(row_ptr, pos)) {
					if (pos >= main_body) {
						scr.drawOnOffLine(offset + row_ptr->baseline -
								  DefaultHeight()/4,
								  int(x),
								  int(fill_hfill));			
						x += fill_hfill;
                    } else {
			    scr.drawOnOffLine(offset + row_ptr->baseline -
					      DefaultHeight()/4,
					      int(x),
					      int(fill_label_hfill));
			    x += fill_label_hfill;
                    }
					scr.drawVerticalLine(gc_fill, int(x),
							     offset + row_ptr->baseline -
							     DefaultHeight()/2, 
							     offset + row_ptr->baseline);
				}
				x += 2;
				++vpos;
			} else {
				if (row_ptr->par->IsSeparator(pos)) {
					tmpx = x;
					x+= SingleWidth(row_ptr->par, pos);
					if (pos >= main_body)
						x+= fill_separator;
					/* -------> Only draw protected spaces when not in
					 * free-spacing mode. */
					if (row_ptr->par->GetChar(pos) == LyXParagraph::META_PROTECTED_SEPARATOR && !layout.free_spacing) {
						
						scr.drawVerticalLine(gc_fill, int(tmpx),
								     offset + row_ptr->baseline - 3,
								     offset + row_ptr->baseline - 1);
						scr.drawLine(gc_fill,
							     offset + row_ptr->baseline - 1,
							     int(tmpx),
							     int(x-tmpx-2));
						scr.drawVerticalLine(gc_fill, int(x-2),
								     offset + row_ptr->baseline - 3,
								     offset + row_ptr->baseline - 1);			
						/* what about underbars? */
						font = GetFont(row_ptr->par, pos); 
						if (font.underbar() == LyXFont::ON
						    && font.latex() != LyXFont::ON) {
							scr.drawLine(gc_copy,
					     offset + row_ptr->baseline + 2,
								     int(tmpx),
								     int(x - tmpx));
						}
					}
					++vpos;
				} else
					Draw(row_ptr, vpos, scr, offset, x);
			}
		}
	}
	// check for FAST SELECTION
	if (fast_selection || mono_video) {
		if (selection) {
			
			/* selection code */ 
			if (sel_start_cursor.row == row_ptr && sel_end_cursor.row == row_ptr) {
				scr.fillRectangle(gc_select, sel_start_cursor.x, offset,
						  sel_end_cursor.x - sel_start_cursor.x,
						  row_ptr->height); 
			} else if (sel_start_cursor.row == row_ptr) {
				scr.fillRectangle(gc_select, sel_start_cursor.x, offset,
						  paperwidth - sel_start_cursor.x,
						  row_ptr->height);
			} else if (sel_end_cursor.row == row_ptr) {
				scr.fillRectangle(gc_select, 0, offset,
						  sel_end_cursor.x,
						  row_ptr->height);
			} else if (y > sel_start_cursor.y && y < sel_end_cursor.y) {
				scr.fillRectangle(gc_select, 0, offset,
						  paperwidth, row_ptr->height);
				
			}
		}
	}
// end of FAST SELECTION code
	/* thats it */ 
}
#endif

int LyXText::DefaultHeight() const
{
	LyXFont font(LyXFont::ALL_SANE);
	return int(font.maxAscent() + font.maxDescent() * 1.5);
}

   
/* returns the column near the specified x-coordinate of the row 
* x is set to the real beginning of this column  */ 
int LyXText::GetColumnNearX(Row * row, int & x) const
{
	float tmpx = 0.0;
	float fill_separator, fill_hfill, fill_label_hfill;
   
	PrepareToPrint(row, tmpx, fill_separator,
		       fill_hfill, fill_label_hfill);

	LyXDirection direction = row->par->getParDirection();
	LyXParagraph::size_type vc = row->pos;
	LyXParagraph::size_type last = RowLast(row);
	LyXParagraph::size_type c;

	LyXLayout const & layout = textclasslist.Style(parameters->textclass,
					   row->par->GetLayout());
	/* table stuff -- begin */
	if (row->par->table) {
		if (row->next && row->next->par == row->par //the last row doesn't need a newline at the end
		    && row->par->IsNewline(last))
			last--;
		int cell = NumberOfCell(row->par, row->pos);
		float x_old = tmpx;
		bool ready = false;
		tmpx += row->par->table->GetBeginningOfTextInCell(cell);
		while (vc <= last
		       && (c = vis2log(vc)) >= 0
		       && tmpx + (SingleWidth(row->par, c)/2) <= x
		       && !ready){
			if (row->par->IsNewline(c)) {
				if (x_old + row->par->table->WidthOfColumn(cell) <= x){
					tmpx = x_old + row->par->table->WidthOfColumn(cell);
					x_old = tmpx;
					++cell;
					tmpx += row->par->table->GetBeginningOfTextInCell(cell);
					++vc;
				} else
					ready = true;
			} else {
				tmpx += SingleWidth(row->par, c);
				++vc;
			}
		}
	} else {
		/* table stuff -- end*/
		LyXParagraph::size_type main_body = BeginningOfMainBody(row->par);
		float last_tmpx = tmpx;

		if (main_body > 0 &&
		    (main_body-1 > last || 
		     !row->par->IsLineSeparator(main_body-1)))
			main_body = 0;

		while (vc <= last && tmpx <= x) {
			c = vis2log(vc);
			last_tmpx = tmpx;
			if (main_body > 0 && c == main_body-1) {
				tmpx += fill_label_hfill +
					GetFont(row->par, -2).stringWidth(layout.labelsep);
				if (row->par->IsLineSeparator(main_body-1))
					tmpx -= SingleWidth(row->par, main_body-1);
			}
	     
			tmpx += SingleWidth(row->par, c);
			if (HfillExpansion(row, c)) {
				if (c >= main_body)
					tmpx += fill_hfill;
				else
					tmpx += fill_label_hfill;
			}
			else if (c >= main_body
				 && row->par->IsSeparator(c)) {
				tmpx+= fill_separator;  
			}
			++vc;
		}

		if (vc > row->pos && (tmpx+last_tmpx)/2 > x) {
			vc--;
			tmpx = last_tmpx;
		}
	}
	/* make sure that a last space in a row doesnt count */
	if (row->pos <= last
	    && !(!row->next || row->next->par != row->par))
		if (direction == LYX_DIR_LEFT_TO_RIGHT && vc > last
		    && row->par->IsLineSeparator(vis2log(last)) ) {
			vc = last;
			tmpx -= fill_separator+SingleWidth(row->par, vis2log(last));
		} else if (direction == LYX_DIR_RIGHT_TO_LEFT 
			   && vc == row->pos
			   && row->par->IsLineSeparator(vis2log(row->pos)) ) {
			vc = row->pos+1;
			tmpx += fill_separator+SingleWidth(row->par, vis2log(row->pos));
		}

	if (row->pos > last)  // Row is empty?
		c = row->pos;
	else if (vc <= last) {
		c = vis2log(vc);
		LyXDirection direction = row->par->getLetterDirection(c);
		if (vc > row->pos && row->par->IsLineSeparator(c)
		    && row->par->getLetterDirection(vis2log(vc - 1)) != direction)
			c = vis2log(vc-1);
		if (direction == LYX_DIR_RIGHT_TO_LEFT)
			++c;
	} else {
		c = vis2log(last)+1;
		if (row->par->getLetterDirection(c - 1) == LYX_DIR_RIGHT_TO_LEFT)
			--c;		
	}

	if (!row->par->table && row->pos <= last && c > last
	    && row->par->IsNewline(last)) {
		if (row->par->getLetterDirection(last) == LYX_DIR_LEFT_TO_RIGHT)
			tmpx -= SingleWidth(row->par, last);
		else
			tmpx += SingleWidth(row->par, last);
		c = last;
	}

	c -= row->pos;
	x = int(tmpx);
	return c;
}

   
/* turn the selection into a new environment. If there is no selection,
* create an empty environment */ 
void LyXText::InsertFootnoteEnvironment(LyXParagraph::footnote_kind kind)
{
   /* no footnoteenvironment in a footnoteenvironment */ 
   if (cursor.par->footnoteflag != LyXParagraph::NO_FOOTNOTE) {
      WriteAlert(_("Impossible operation"), 
		 _("You can't insert a float in a float!"), 
		 _("Sorry."));
     return;
   }
   /* no marginpars in minipages */
   if (kind == LyXParagraph::MARGIN 
      && cursor.par->pextra_type == LyXParagraph::PEXTRA_MINIPAGE) {
      WriteAlert(_("Impossible operation"), 
		 _("You can't insert a marginpar in a minipage!"), 
		 _("Sorry."));
      return;
   }
   
   /* this doesnt make sense, if there is no selection */ 
   bool dummy_selection = false;
   if (!selection) {
      sel_start_cursor = cursor;       /* dummy selection  */
      sel_end_cursor = cursor;
      dummy_selection = true;
   }
   
   LyXParagraph *tmppar;

   if (sel_start_cursor.par->table || sel_end_cursor.par->table){
      WriteAlert(_("Impossible operation"), _("Cannot cut table."), _("Sorry."));
      return;
   }
     
   /* a test to make sure there is not already a footnote
    * in the selection. */
   
   tmppar = sel_start_cursor.par->ParFromPos(sel_start_cursor.pos);
   
   while (tmppar != sel_end_cursor.par->ParFromPos(sel_end_cursor.pos) && 
	  tmppar->footnoteflag == LyXParagraph::NO_FOOTNOTE)
     tmppar = tmppar->next;
   
   if (tmppar != sel_end_cursor.par->ParFromPos(sel_end_cursor.pos)
       || tmppar->footnoteflag != LyXParagraph::NO_FOOTNOTE) {
      WriteAlert(_("Impossible operation"), 
		 _("Float would include float!"), 
		 _("Sorry."));
      return;
   }
   
   /* ok we have a selection. This is always between sel_start_cursor
    * and sel_end cursor */

   SetUndo(Undo::FINISH, 
	   sel_start_cursor.par->ParFromPos(sel_start_cursor.pos)->previous, 
	   sel_end_cursor.par->ParFromPos(sel_end_cursor.pos)->next); 
   
   if (sel_end_cursor.pos > 0 
       && sel_end_cursor.par->IsLineSeparator(sel_end_cursor.pos - 1))
     sel_end_cursor.pos--;	       /* please break before a space at
					* the end */
   if (sel_start_cursor.par == sel_end_cursor.par
       && sel_start_cursor.pos > sel_end_cursor.pos)
     sel_start_cursor.pos--;

   sel_end_cursor.par->BreakParagraphConservative(sel_end_cursor.pos);
   
   sel_end_cursor.par = sel_end_cursor.par->Next();
   sel_end_cursor.pos = 0;
   
   // don't forget to insert a dummy layout paragraph if necessary
   if (sel_start_cursor.par->GetLayout() != sel_end_cursor.par->layout){
     sel_end_cursor.par->BreakParagraphConservative(0);
     sel_end_cursor.par->layout = LYX_DUMMY_LAYOUT;
     sel_end_cursor.par = sel_end_cursor.par->next;
   }
   else
     sel_end_cursor.par->layout = LYX_DUMMY_LAYOUT;

   cursor = sel_end_cursor;

   /* please break behind a space, if there is one. The space should
    * be erased too */ 
   if (sel_start_cursor.pos > 0 
       && sel_start_cursor.par->IsLineSeparator(sel_start_cursor.pos - 1))
     sel_start_cursor.pos--;
   if (sel_start_cursor.par->IsLineSeparator(sel_start_cursor.pos)) {
      sel_start_cursor.par->Erase(sel_start_cursor.pos);
   }
   
   sel_start_cursor.par->BreakParagraphConservative(sel_start_cursor.pos);
   tmppar = sel_start_cursor.par->Next();
   
   if (dummy_selection) {
	   tmppar->Clear();
	   if (kind == LyXParagraph::TAB
	       || kind == LyXParagraph::FIG 
               || kind == LyXParagraph::WIDE_TAB
	       || kind == LyXParagraph::WIDE_FIG 
               || kind == LyXParagraph::ALGORITHM) {
		   int lay = textclasslist.NumberOfLayout(parameters->textclass,
						     "Caption").second;
		   if (lay == -1) // layout not found
			   // use default layout "Standard" (0)
			   lay = 0;
		   tmppar->SetLayout(lay);
	   }
   }
   else {
     if (sel_start_cursor.pos > 0) {
       /* the footnote-environment should begin with a standard layout.
	* Imagine you insert a footnote within an enumeration, you 
	* certainly do not want an enumerated footnote! */ 
       tmppar->Clear();
     }
     else {
       /* this is a exception the user would sometimes expect, I hope */
       sel_start_cursor.par->Clear();
     }
   }
   
   while (tmppar != sel_end_cursor.par) {
      tmppar->footnoteflag = LyXParagraph::OPEN_FOOTNOTE;
      tmppar->footnotekind = kind;
      tmppar = tmppar->Next();
   } 

   RedoParagraphs(sel_start_cursor, sel_end_cursor.par->Next());
   
   SetCursor(sel_start_cursor.par->Next(), 0);

   ClearSelection();
}
   

// returns pointer to a specified row
Row * LyXText::GetRow(LyXParagraph * par,
		      LyXParagraph::size_type pos, long & y) const
{
	Row * tmprow;
	
	if (currentrow) {
		if (par == currentrow->par || par == currentrow->par->Previous()) {
			// do not dereference par, it may have been deleted
			// already! (Matthias) 
			while (currentrow->previous && currentrow->previous->par != par) {
				currentrow = currentrow->previous;
				currentrow_y -= currentrow->height;
			}
			while (currentrow->previous && currentrow->previous->par == par) {
				currentrow = currentrow->previous;
				currentrow_y -= currentrow->height;
			}
		}
		tmprow = currentrow;
		y = currentrow_y;
		// find the first row of the specified paragraph
		while (tmprow->next && (tmprow->par != par)) {
			y += tmprow->height;
			tmprow = tmprow->next;
		}
		
		if (tmprow->par == par){
			// now find the wanted row
			while (tmprow->pos < pos && tmprow->next && tmprow->next->par == par && 
			       tmprow->next->pos <= pos) {
				y += tmprow->height;
				tmprow = tmprow->next;
			}
			currentrow = tmprow;
			currentrow_y = y;
			return tmprow;
		}
	}
	tmprow = firstrow;
	y = 0;
	// find the first row of the specified paragraph
	while (tmprow->next && (tmprow->par != par)) {
		y += tmprow->height;
		tmprow = tmprow->next;
	}
	
	// now find the wanted row
	while (tmprow->pos < pos && tmprow->next && tmprow->next->par == par && 
	       tmprow->next->pos <= pos) {
		y += tmprow->height;
		tmprow = tmprow->next;
	}
	
	currentrow = tmprow;
	currentrow_y = y;
	
	return tmprow;
}
