/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
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
#include "insets/insetbib.h"
#include "lyx_gui_misc.h"
#include "gettext.h"
#include "bufferparams.h"
#include "buffer.h"
#include "minibuffer.h"
#include "debug.h"
#include "lyxrc.h"
#include "LyXView.h"
#include "lyxrow.h"
#include "Painter.h"
#include "tracer.h"
#include "font.h"

using std::max;
using std::min;
using std::endl;
using std::pair;

static const int LYX_PAPER_MARGIN = 20;


// ale070405
extern int bibitemMaxWidth(Painter &, LyXFont const &);

static int iso885968x[] = {
	0xbc,	// 0xa8 = fathatan
	0xbd,	// 0xa9 = dammatan
	0xbe,	// 0xaa = kasratan
	0xdb,	// 0xab = fatha
	0xdc,	// 0xac = damma
	0xdd,	// 0xad = kasra
	0xde,	// 0xae = shadda
	0xdf,	// 0xaf = sukun

	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0xb0-0xbf

	0,	// 0xc0	
	0xc1,	// 0xc1 = hamza
	0xc2,	// 0xc2 = ligature madda
	0xc3,	// 0xc3 = ligature hamza on alef
	0xc4,	// 0xc4 = ligature hamza on waw
	0xc5,	// 0xc5 = ligature hamza under alef
	0xc0,	// 0xc6 = ligature hamza on ya 
	0xc7,	// 0xc7 = alef
	0xeb,	// 0xc8 = baa 
	0xc9,	// 0xc9 = taa marbuta
	0xec,	// 0xca = taa
	0xed,	// 0xcb = thaa
	0xee,	// 0xcc = jeem
	0xef,	// 0xcd = haa
	0xf0,	// 0xce = khaa
	0xcf,	// 0xcf = dal

	0xd0,	// 0xd0 = thal
	0xd1,	// 0xd1 = ra
	0xd2,	// 0xd2 = zain
	0xf1,	// 0xd3 = seen
	0xf2,	// 0xd4 = sheen
	0xf3,	// 0xd5 = sad
	0xf4,	// 0xd6 = dad
	0xd7,	// 0xd7 = tah
	0xd8,	// 0xd8 = zah
	0xf5,	// 0xd9 = ain
	0xf6,	// 0xda = ghain
	0,0,0,0,0, // 0xdb- 0xdf

	0,	// 0xe0
	0xf7,	// 0xe1 = fa
	0xf8,	// 0xe2 = qaf
	0xf9,	// 0xe3 = kaf
	0xfa,	// 0xe4 = lam
	0xfb,	// 0xe5 = meem
	0xfc,	// 0xe6 = noon
	0xfd,	// 0xe7 = ha
	0xe8,	// 0xe8 = waw
	0xe9,	// 0xe9 = alef maksura
	0xfe	// 0xea = ya
};


inline
bool is_arabic(unsigned char c)
{
	return 0xa8 <= c && c <= 0xea && iso885968x[c-0xa8];
}


inline
bool is_nikud(unsigned char c)
{
        return 192 <= c && c <= 210;
}


unsigned char LyXText::TransformChar(unsigned char c, Letter_Form form) const
{
	if (is_arabic(c) && 
	    (form == FORM_INITIAL || form == FORM_MEDIAL) )
		return iso885968x[c-0xa8];
	else
		return c;
}


unsigned char LyXText::TransformChar(unsigned char c, LyXParagraph * par,
			LyXParagraph::size_type pos) const
{
	if (!is_arabic(c))
		if (isdigit(c))
			return c + (0xb0 - '0');
		else
			return c;

	bool not_first = (pos > 0 && is_arabic(par->GetChar(pos-1)));
	if (pos < par->Last()-1 && is_arabic(par->GetChar(pos+1)))
		if (not_first)
			return TransformChar(c,FORM_MEDIAL);
		else
			return TransformChar(c,FORM_INITIAL);
	else
		if (not_first)
			return TransformChar(c,FORM_FINAL);
		else
			return TransformChar(c,FORM_ISOLATED);
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

int LyXText::SingleWidth(LyXParagraph * par,
			 LyXParagraph::size_type pos) const
{
	char c = par->GetChar(pos);
	return SingleWidth(par, pos, c);
}


int LyXText::SingleWidth(LyXParagraph * par,
			 LyXParagraph::size_type pos, char c) const
{
	LyXFont font = GetFont(par, pos);

	// The most common case is handled first (Asger)
	if (IsPrintable(c)) {
		if (font.language()->RightToLeft) {
			if (font.language()->lang == "arabic" &&
			    lyxrc.font_norm == "iso8859-6.8x")
				c = TransformChar(c, par, pos);
			else if (font.language()->lang == "hebrew" &&
				 is_nikud(c))
				return 0;
		}
		return lyxfont::width(c, font);

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
		return lyxfont::width(fs, font);
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
	return lyxfont::width(c, font);
}


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


LyXParagraph::size_type LyXText::RowLastPrintable(Row const * row) const
{
	LyXParagraph::size_type last = RowLast(row);
	if (last >= row->pos && row->next && row->next->par == row->par &&
	    row->par->IsSeparator(last))
		return last - 1;
	else
		return last;
}


void LyXText::ComputeBidiTables(Row * row) const
{
	bidi_same_direction = true;
	if (!lyxrc.rtl_support) {
		bidi_start = -1;
		return;
	}
	bidi_start = row->pos;
	bidi_end = RowLastPrintable(row);

	if (bidi_start > bidi_end) {
		bidi_start = -1;
		return;
	}

	if (bidi_end + 2 - bidi_start >
	    static_cast<LyXParagraph::size_type>(log2vis_list.size())) {
		LyXParagraph::size_type new_size = 
			(bidi_end + 2 - bidi_start < 500) ?
			500 : 2 * (bidi_end + 2 - bidi_start);
		log2vis_list.resize(new_size);
		vis2log_list.resize(new_size);
		bidi_levels.resize(new_size);
	}

	vis2log_list[bidi_end + 1 - bidi_start] = -1;
	log2vis_list[bidi_end + 1 - bidi_start] = -1;

	LyXParagraph::size_type	stack[2];
	bool rtl_par = row->par->getParLanguage()->RightToLeft;
	int level = 0;
	bool rtl = false;
	bool rtl0 = false;
	LyXParagraph::size_type main_body = BeginningOfMainBody(row->par);

	for (LyXParagraph::size_type lpos = bidi_start; lpos <= bidi_end; ++lpos) {
		bool is_space = row->par->IsLineSeparator(lpos);
		LyXParagraph::size_type pos =
			(is_space && lpos+1 <= bidi_end &&
			 !row->par->IsLineSeparator(lpos+1) &&
			 (!row->par->table || !row->par->IsNewline(lpos+1)) )
			? lpos + 1 : lpos;
		LyXFont font = row->par->GetFontSettings(pos);
		bool new_rtl = font.isVisibleRightToLeft();
		bool new_rtl0 = font.isRightToLeft();
		int new_level;

		if (row->par->table && row->par->IsNewline(lpos)) {
			new_level = 0;
			new_rtl = new_rtl0 = false;
		} else if (lpos == main_body - 1 && row->pos < main_body - 1 &&
			   row->par->IsLineSeparator(lpos)) {
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
			LyXParagraph::size_type old_lpos =
				stack[--level];
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
		LyXParagraph::size_type old_lpos = stack[--level];
		int delta = bidi_end - old_lpos;
		if (level % 2)
			delta = -delta;
		log2vis_list[old_lpos - bidi_start] += delta;
	}

	LyXParagraph::size_type vpos = bidi_start - 1;
	for (LyXParagraph::size_type lpos = bidi_start; lpos <= bidi_end; ++lpos) {
		vpos += log2vis_list[lpos - bidi_start];
		vis2log_list[vpos - bidi_start] = lpos;
		log2vis_list[lpos - bidi_start] = vpos;
	}
}


// This method requires a previous call to ComputeBidiTables()
bool LyXText::IsBoundary(LyXParagraph * par, LyXParagraph::size_type pos) const
{
	if (!lyxrc.rtl_support)
		return false;    // This is just for speedup

	if (!bidi_InRange(pos - 1) ||
	    (par->table && par->IsNewline(pos-1)) )
		return false;

	bool rtl = bidi_level(pos - 1) % 2;
	bool rtl2 = rtl;
	if (pos == par->Last() ||
	    (par->table && par->IsNewline(pos)))
		rtl2 = par->isRightToLeftPar();
	else if (bidi_InRange(pos))
		rtl2 = bidi_level(pos) % 2;
	return rtl != rtl2;
}

bool LyXText::IsBoundary(LyXParagraph * par, LyXParagraph::size_type pos,
		 LyXFont const & font) const
{
	if (!lyxrc.rtl_support)
		return false;    // This is just for speedup

	bool rtl = font.isVisibleRightToLeft();
	bool rtl2 = rtl;
	if (pos == par->Last() ||
	    (par->table && par->IsNewline(pos)))
		rtl2 = par->isRightToLeftPar();
	else if (bidi_InRange(pos))
		rtl2 =  bidi_level(pos) % 2;
	return rtl != rtl2;
}


void LyXText::draw(Row const * row,
		   LyXParagraph::size_type & vpos,
		   int offset, float & x)
{
	Painter & pain = owner_->painter();
	
	LyXParagraph::size_type pos = vis2log(vpos);
	char c = row->par->GetChar(pos);
	float tmpx = x;

	if (IsNewlineChar(c)) {
		++vpos;
		// Draw end-of-line marker
		LyXFont font = GetFont(row->par, pos);
		int wid = lyxfont::width('n', font);
		int asc = lyxfont::maxAscent(font);
		int y = offset + row->baseline;
		int xp[3], yp[3];
		
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
		int y = (row->baseline - lyxfont::maxAscent(font2) 
			 + lyxfont::maxAscent(font));
	  
		font.setColor(LColor::footnote);

		// draw it and set new x position
		
		pain.text(int(x), offset + y, fs, font);
		x += lyxfont::width(fs, font);
		pain.line(int(tmpx), offset + row->baseline,
			  int(x), offset + row->baseline,
			  LColor::footnote);

		++vpos;
		return;
	} else if (c == LyXParagraph::META_INSET) {
		Inset * tmpinset = row->par->GetInset(pos);
		if (tmpinset) {
			tmpinset->draw(owner_->painter(), font,
				       offset + row->baseline, x);
		}
		++vpos;

		if (lyxrc.mark_foreign_language &&
		    font.language() != buffer->params.language_info) {
			int y = offset + row->baseline + 2;
			pain.line(int(tmpx), y, int(x), y,
				  LColor::language);
		}

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

	LyXParagraph::size_type last = RowLastPrintable(row);

	if (font.language()->lang == "hebrew") {
		if (is_nikud(c)) {
			LyXParagraph::size_type vpos2 = vpos;
			int width = lyxfont::width(c, font2);
			int dx = 0;
			while (vpos2 <= last &&
			       (pos = vis2log(vpos2)) >= 0
			       && static_cast<unsigned char>(c = row->par->GetChar(pos)) > ' '
			       && is_nikud(c))
				++vpos2;
			if (static_cast<unsigned char>(c = row->par->GetChar(pos)) > ' '
			    && !is_nikud(c)) {
				int width2 = SingleWidth(row->par, pos, c);
				dx = (c == 'ø' || c == 'ã') 
					? width2-width : (width2-width)/2;
			}
			// Draw nikud
			pain.text(int(x)+dx, offset + row->baseline, textstring, font);
		} else {
			while (vpos <= last &&
			       (pos = vis2log(vpos)) >= 0
			       && static_cast<unsigned char>(c = row->par->GetChar(pos)) > ' '
			       && !is_nikud(c)
			       && font2 == GetFont(row->par, pos)) {
				textstring += c;
				++vpos;
			}
			// Draw text and set the new x position
			pain.text(int(x), offset + row->baseline, textstring, font);
			x += lyxfont::width(textstring, font);
		}
	} else if (font.language()->lang == "arabic" &&
		   lyxrc.font_norm == "iso8859-6.8x") {
		textstring = TransformChar(c, row->par, pos);
		while (vpos <= last &&
		       (pos = vis2log(vpos)) >= 0
		       && static_cast<unsigned char>(c = row->par->GetChar(pos)) > ' '
		       && font2 == GetFont(row->par, pos)) {
			c = TransformChar(c, row->par, pos);
			textstring += c;
			++vpos;
		}
		// Draw text and set the new x position
		pain.text(int(x), offset + row->baseline, textstring, font);
		x += lyxfont::width(textstring, font);
	} else {
		while (vpos <= last &&
		       (pos = vis2log(vpos)) >= 0
		       && static_cast<unsigned char>(c = row->par->GetChar(pos)) > ' '
		       && font2 == GetFont(row->par, pos)) {
			textstring += c;
			++vpos;
		}
		// Draw text and set the new x position
		pain.text(int(x), offset + row->baseline, textstring, font);
		x += lyxfont::width(textstring, font);
	}
	
	// what about underbars?
	if (font.underbar() == LyXFont::ON && font.latex() != LyXFont::ON) {
		pain.line(int(tmpx), offset + row->baseline + 2,
			  int(x), offset + row->baseline + 2);
		
	} else if (lyxrc.mark_foreign_language &&
	    font.language() != buffer->params.language_info) {
		int y = offset + row->baseline + 2;
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
int LyXText::LeftMargin(Row const * row) const
{
	LyXLayout const & layout =
		textclasslist.Style(buffer->params.textclass,
				    row->par->GetLayout());
	
	string parindent = layout.parindent; 
	
	/* table stuff -- begin */ 
	if (row->par->table)
		parindent.erase();
	/* table stuff -- end */
	
	int x = LYX_PAPER_MARGIN;
	
	x += lyxfont::signedWidth(textclasslist
				  .TextClass(buffer->params.textclass)
				  .leftmargin(),
				  textclasslist
				  .TextClass(buffer->params.textclass)
				  .defaultfont());
	
	if (row->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE)  {
		LyXFont font(LyXFont::ALL_SANE);
		font.setSize(LyXFont::SIZE_SMALL);
		x += lyxfont::width("Mwide-figM", font)
			+ LYX_PAPER_MARGIN/2;
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
				    textclasslist.Style(buffer->params.textclass,
							newpar->GetLayout())
				    .nextnoindent)
					parindent.erase();
			}
		}
	} else {
		// find the next level paragraph
		
		LyXParagraph * newpar = row->par->DepthHook(row->par->GetDepth()-1);
		
		// make a corresponding row. Needed to call LeftMargin()
		
		// check wether it is a sufficent paragraph 
		if (newpar && newpar->footnoteflag == row->par->footnoteflag
		    && textclasslist
		        .Style(buffer->params.textclass, 
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
				parindent.erase();
			else
				parindent = textclasslist
					.Style(buffer->params.textclass, 
					       newpar->GetLayout()).parindent;
		}
		
	}
	
	LyXFont labelfont = GetFont(row->par, -2);
	switch (layout.margintype) {
	case MARGIN_DYNAMIC:
		if (!layout.leftmargin.empty()) {
			x += lyxfont::signedWidth(layout.leftmargin,
						  textclasslist
						  .TextClass(buffer->params.
							     textclass)
						  .defaultfont());
		}
		if (!row->par->GetLabelstring().empty()) {
			x += lyxfont::signedWidth(layout.labelindent,
						  labelfont);
			x += lyxfont::width(row->par->GetLabelstring(), labelfont);
			x += lyxfont::width(layout.labelsep, labelfont);
		}
		break;
	case MARGIN_MANUAL:
		x += lyxfont::signedWidth(layout.labelindent, labelfont);
		if (row->pos >= BeginningOfMainBody(row->par)) {
			if (!row->par->GetLabelWidthString().empty()) {
				x += lyxfont::width(row->par->GetLabelWidthString(),
					       labelfont);
				x += lyxfont::width(layout.labelsep, labelfont);
			}
		}
		break;
	case MARGIN_STATIC:
		x += lyxfont::signedWidth(layout.leftmargin, textclasslist.TextClass(buffer->params.textclass).defaultfont()) * 4
			/ (row->par->GetDepth() + 4);
		break;
	case MARGIN_FIRST_DYNAMIC:
		if (layout.labeltype == LABEL_MANUAL) {
			if (row->pos >= BeginningOfMainBody(row->par)) {
				x += lyxfont::signedWidth(layout.leftmargin, labelfont);
			} else {
				x += lyxfont::signedWidth(layout.labelindent, labelfont);
			}
		} else if (row->pos
			   // Special case to fix problems with
			   // theorems (JMarc)
			   || (layout.labeltype == LABEL_STATIC
			       && layout.latextype == LATEX_ENVIRONMENT
			       && ! row->par->IsFirstInSequence())) {
			x += lyxfont::signedWidth(layout.leftmargin, labelfont);
		} else if (layout.labeltype != LABEL_TOP_ENVIRONMENT
			   && layout.labeltype != LABEL_BIBLIO
			   && layout.labeltype !=
			   LABEL_CENTERED_TOP_ENVIRONMENT) {
			x += lyxfont::signedWidth(layout.labelindent,
						  labelfont);
			x += lyxfont::width(layout.labelsep, labelfont);
			x += lyxfont::width(row->par->GetLabelstring(), labelfont);
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
		
		x += lyxfont::signedWidth(layout.leftmargin,
					  textclasslist
					  .TextClass(buffer->params.textclass)
					  .defaultfont());
		x += minfill;
	}
	break;
	}
	if (row->par->pextra_type == LyXParagraph::PEXTRA_INDENT) {
		if (!row->par->pextra_widthp.empty()) {
			x += paperwidth *
				atoi(row->par->pextra_widthp.c_str()) / 100;
		} else if (!row->par->pextra_width.empty()) {
			int xx = VSpace(row->par->pextra_width).inPixels(owner_);
			if (xx > paperwidth)
				xx = paperwidth * 80 / 100;
			x += xx;
		} else { // should not happen
			LyXFont font(LyXFont::ALL_SANE);
			x += lyxfont::width("XXXXXX", font);
		}
	}
	
	int align; // wrong type
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
			buffer->params.paragraph_separation ==
			BufferParams::PARSEP_INDENT))
			x += lyxfont::signedWidth(parindent,
						  textclasslist
						  .TextClass(buffer->params
							     .textclass)
						  .defaultfont());
		else if (layout.labeltype == LABEL_BIBLIO) {
		       	// ale970405 Right width for bibitems
			x += bibitemMaxWidth(owner_->painter(),
					     textclasslist
					     .TextClass(buffer->params
							.textclass)
					     .defaultfont());
		}
	}
	return x;
}


int LyXText::RightMargin(Row const * row) const
{
	LyXLayout const & layout =
		textclasslist.Style(buffer->params.textclass,
				    row->par->GetLayout());
	
	int x = LYX_PAPER_MARGIN
		+ lyxfont::signedWidth(textclasslist
				       .TextClass(buffer->params.textclass)
				       .rightmargin(),
				       textclasslist
				       .TextClass(buffer->params.textclass)
				       .defaultfont());
	
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
		    && textclasslist.Style(buffer->params.textclass,
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
	x += lyxfont::signedWidth(layout.rightmargin, textclasslist.TextClass(buffer->params.textclass).defaultfont()) * 4
	      / (row->par->GetDepth() + 4);
	return x;
}


int LyXText::LabelEnd (Row const * row) const
{
	if (textclasslist.Style(buffer->params.textclass,
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
	width -= RightMargin(row);
	
	LyXParagraph::size_type main_body = BeginningOfMainBody(par);
	LyXLayout const & layout =
		textclasslist.Style(buffer->params.textclass, par->GetLayout());
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
				x += lyxfont::width(layout.labelsep,
						    GetFont(par, -2));
				if (par->IsLineSeparator(i - 1))
					x-= SingleWidth(par, i - 1);
				int left_margin = LabelEnd(row);
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
	LyXParagraph::size_type last = RowLastPrintable(row);
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

	// special handling of the right address boxes
	if (textclasslist.Style(buffer->params.textclass,
				row->par->GetLayout()).margintype
	    == MARGIN_RIGHT_ADDRESS_BOX) {
		int tmpfill = row->fill;
		row->fill = 0; // the minfill in MarginLeft()
		w = LeftMargin(row);
		row->fill = tmpfill;
	} else
		w = LeftMargin(row);
	
	LyXLayout const & layout = textclasslist.Style(buffer->params.textclass,
						       row->par->GetLayout());
	LyXParagraph::size_type main_body = 
		BeginningOfMainBody(row->par);
	LyXParagraph::size_type i = row->pos;

	while (i <= last) {
		if (main_body > 0 && i == main_body) {
			w += lyxfont::width(layout.labelsep, GetFont(row->par, -2));
			if (row->par->IsLineSeparator(i - 1))
				w -= SingleWidth(row->par, i - 1);
			int left_margin = LabelEnd(row);
			if (w < left_margin)
				w = left_margin;
		}
		w += SingleWidth(row->par, i);
		++i;
	}
	if (main_body > 0 && main_body > last) {
		w += lyxfont::width(layout.labelsep, GetFont(row->par, -2));
		if (last >= 0 && row->par->IsLineSeparator(last))
			w -= SingleWidth(row->par, last);
		int left_margin = LabelEnd(row);
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
		fill = max(lyxfont::width(row->par->labelwidthstring,
					  GetFont(row->par, -2)) - w,
			   0);
	}
	
	return fill;
}


// returns the number of separators in the specified row. The separator 
// on the very last column doesnt count
int LyXText::NumberOfSeparators(Row const * row) const
{
	int last = RowLast(row);
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
	if (textclasslist.Style(buffer->params.textclass,
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
   
   LyXLayout const & layout = textclasslist.Style(buffer->params.textclass,
						  firstpar->GetLayout());
   
   LyXFont font = GetFont(par, par->Last() - 1);
   LyXFont::FONT_SIZE size = font.size();
   font = GetFont(par, -1);
   font.setSize(size);

   LyXFont labelfont = GetFont(par, -2);

   float spacing_val = 1.0;
   if (!row_ptr->par->spacing.isDefault()) {
	   spacing_val = row_ptr->par->spacing.getValue();
   } else {
	   spacing_val = buffer->params.spacing.getValue();
   }
   //lyxerr << "spacing_val = " << spacing_val << endl;
   
   int maxasc = int(lyxfont::maxAscent(font) *
		   layout.spacing.getValue() *
		   spacing_val);
   int maxdesc = int(lyxfont::maxDescent(font) *
		    layout.spacing.getValue() *
		    spacing_val);

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

	asc = lyxfont::maxAscent(font);
	desc = lyxfont::maxDescent(font);
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
      if (buffer->params.paragraph_separation == BufferParams::PARSEP_SKIP) {
	 if (layout.isParagraph()
	     && firstpar->GetDepth() == 0
	     && firstpar->Previous())
	    maxasc += buffer->params.getDefSkip().inPixels(owner_);
	 else if (firstpar->Previous()
		  && textclasslist.Style(buffer->params.textclass,
			   firstpar->Previous()->GetLayout()).isParagraph()
		  && firstpar->Previous()->GetDepth() == 0)
	   // is it right to use defskip here too? (AS)
	   maxasc += buffer->params.getDefSkip().inPixels(owner_);
      }
      
      /* the paper margins */ 
      if (!row_ptr->par->previous)
      	 maxasc += LYX_PAPER_MARGIN;
      
      /* add the vertical spaces, that the user added */
      if (firstpar->added_space_top.kind() != VSpace::NONE)
      	 maxasc += int(firstpar->added_space_top.inPixels(owner_));
      
      /* do not forget the DTP-lines! 
       * there height depends on the font of the nearest character */
      if (firstpar->line_top)
      	 maxasc += 2 * lyxfont::ascent('x', GetFont(firstpar, 0));
      
      /* and now the pagebreaks */ 
      if (firstpar->pagebreak_top)
      	 maxasc += 3 * DefaultHeight();
      
      /*  this is special code for the chapter, since the label of this
       * layout is printed in an extra row */ 
      if (layout.labeltype == LABEL_COUNTER_CHAPTER
	  && buffer->params.secnumdepth >= 0) {
	      float spacing_val = 1.0;
	      if (!row_ptr->par->spacing.isDefault()) {
		      spacing_val = row_ptr->par->spacing.getValue();
	      } else {
		      spacing_val = buffer->params.spacing.getValue();
	      }
	      
	      labeladdon = int(lyxfont::maxDescent(labelfont) *
			       layout.spacing.getValue() *
			       spacing_val)
		      + int(lyxfont::maxAscent(labelfont) *
			    layout.spacing.getValue() *
			    spacing_val);
      }
      
      /* special code for the top label */ 
      if ((layout.labeltype == LABEL_TOP_ENVIRONMENT
	   || layout.labeltype == LABEL_BIBLIO
	   || layout.labeltype == LABEL_CENTERED_TOP_ENVIRONMENT)
	  && row_ptr->par->IsFirstInSequence()
	  && !row_ptr->par->GetLabelstring().empty()) {
	      float spacing_val = 1.0;
	      if (!row_ptr->par->spacing.isDefault()) {
		      spacing_val = row_ptr->par->spacing.getValue();
	      } else {
		      spacing_val = buffer->params.spacing.getValue();
	      }
	      
	      labeladdon = int(
		      (lyxfont::maxAscent(labelfont) *
		       layout.spacing.getValue() *
		       spacing_val)
		      +(lyxfont::maxDescent(labelfont) *
			layout.spacing.getValue() *
			spacing_val)
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
	       tmptop-= textclasslist.Style(buffer->params.textclass, row_ptr->previous->par->GetLayout()).bottomsep;
	    
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
	    maxasc += int(textclasslist.Style(buffer->params.textclass,
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
	    maxdesc += int(firstpar->added_space_bottom.inPixels(owner_));
	  
	  /* do not forget the DTP-lines! 
	   * there height depends on the font of the nearest character */
	  if (firstpar->line_bottom)
	    maxdesc += 2 * lyxfont::ascent('x', GetFont(par, par->Last() - 1));
	  
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
		usual = (textclasslist.Style(buffer->params.textclass, comparepar->GetLayout()).bottomsep * DefaultHeight());
		comparepar = comparepar->DepthHook(nextpar->GetDepth());
		if (comparepar->GetLayout()!= nextpar->GetLayout()
		    || nextpar->GetLabelWidthString() != 
		    	comparepar->GetLabelWidthString())
		  unusual = (textclasslist.Style(buffer->params.textclass, comparepar->GetLayout()).bottomsep * DefaultHeight());
		
		if (unusual > usual)
		  layoutdesc = unusual;
		else
		  layoutdesc = usual;
	     }
	     else if (comparepar->GetDepth() ==  nextpar->GetDepth()) {
		
		if (comparepar->GetLayout()!= nextpar->GetLayout()
		    || nextpar->GetLabelWidthString() != 
			comparepar->GetLabelWidthString())
		  layoutdesc = int(textclasslist.Style(buffer->params.textclass, comparepar->GetLayout()).bottomsep * DefaultHeight());
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
   LyXLayout const & layout = textclasslist.Style(buffer->params.textclass,
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
           SetCursor(cursor.par, cursor.par->size());
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
   if (layout.labeltype == LABEL_SENSITIVE) {
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
	     cursor.pos + cursor.par->ParFromPos(cursor.pos)->size());
   
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
   // CHECK See comment on top of text.C
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
	  Language const * lang = cursor.par->getParLanguage();
	  LyXFont font(LyXFont::ALL_INHERIT,lang);
          for (int i = 0; i < number; ++i) {
              cursor.par->InsertChar(pos, LyXParagraph::META_NEWLINE);
	      cursor.par->SetFont(pos, font);
	  }
		
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
	  Language const * lang = cursor.par->getParLanguage();
	  LyXFont font(LyXFont::ALL_INHERIT,lang);
          for (int i = 0; i < number; ++i) {
              cursor.par->InsertChar(pos, LyXParagraph::META_NEWLINE);
	      cursor.par->SetFont(pos, font);
	  }

          /* append the row into the table */
          cursor.par->table->AppendContRow(cell_org);
          RedoParagraph();
          return;
      }
      case LyXTable::APPEND_COLUMN: {
	      LyXParagraph::size_type pos = 0;
          int cell_org = actCell;
          int cell = 0;
	  Language const * lang = cursor.par->getParLanguage();
	  LyXFont font(LyXFont::ALL_INHERIT,lang);
          do{
              if (pos && (cursor.par->IsNewline(pos-1))){
                  if (cursor.par->table->AppendCellAfterCell(cell_org, cell)) {
                      cursor.par->InsertChar(pos, LyXParagraph::META_NEWLINE);
		      cursor.par->SetFont(pos, font);
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
          if (cursor.par->table->AppendCellAfterCell(cell_org, cell)) {
		  LyXParagraph::size_type last = cursor.par->Last();
		  cursor.par->InsertChar(last, LyXParagraph::META_NEWLINE);
		  cursor.par->SetFont(last, font);
	  }
		
          /* append the column into the table */ 
          cursor.par->table->AppendColumn(cell_org);
		
          RedoParagraph();
          return;
      }
      case LyXTable::DELETE_ROW:
          if (owner_->the_locking_inset)
              owner_->unlockInset(owner_->the_locking_inset);
          RemoveTableRow(&cursor);
          RedoParagraph();
          return;
	
      case LyXTable::DELETE_COLUMN: {
	      LyXParagraph::size_type pos = 0;
          int cell_org = actCell;
          int cell = 0;
          if (owner_->the_locking_inset)
              owner_->unlockInset(owner_->the_locking_inset);
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
		       cursor.par->size() - 1; i >= 0; --i)
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
		if ((cursor.pos > 0 && 
		     cursor.par->IsLineSeparator(cursor.pos - 1))
		    || (cursor.pos > 0 && cursor.par->IsNewline(cursor.pos - 1))
		    || (cursor.pos == 0 &&
			!(cursor.par->Previous()
			  && cursor.par->Previous()->footnoteflag
			  == LyXParagraph::OPEN_FOOTNOTE)))
			return;
	} else if (IsNewlineChar(c)) {
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

	++cursor.pos;

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
		SetCursorIntern(par, pos, false);
		/* make a complete redraw */
		RedoDrawingOfParagraph(cursor);
		cursor = tmpcursor;
	}
	else {
		/* redraw only the row */
		LyXCursor tmpcursor = cursor;
		SetCursorIntern(par, pos);
		//CHECK See comment on top of text.C
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
        SetCursorIntern(cursor.par, cursor.pos, false, cursor.boundary);
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
	} else {
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
		} else {
			cursor.par->Erase(cursor.pos);
			
			/* refresh the positions */
			tmprow = row;
			while (tmprow->next && tmprow->next->par == row->par) {
				tmprow = tmprow->next;
				tmprow->pos--;
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
	SetCursorIntern(cursor.par, cursor.pos, true, cursor.boundary);
	if (IsBoundary(cursor.par, cursor.pos) != cursor.boundary)
		SetCursor(cursor.par, cursor.pos, false, !cursor.boundary);
}

/* table stuff -- end*/


/* just a macro to make some thing easier. */ 
void LyXText::RedoParagraph() const
{
	ClearSelection();
	RedoParagraphs(cursor, cursor.par->Next());
	SetCursorIntern(cursor.par, cursor.pos);
}


/* insert a character, moves all the following breaks in the 
 * same Paragraph one to the right and make a rebreak */
void LyXText::InsertChar(char c)
{
	SetUndo(Undo::INSERT, 
		cursor.par->ParFromPos(cursor.pos)->previous, 
		cursor.par->ParFromPos(cursor.pos)->next);

	/* When the free-spacing option is set for the current layout,
	 * disable the double-space checking */

	bool freeSpacing = 
		textclasslist.Style(buffer->params.textclass,
			       cursor.row->par->GetLayout()).free_spacing;

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
	// CHECK There is a bug here! (Asger)
	
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
   
	if (!freeSpacing && IsLineSeparatorChar(c)) {
		if ((cursor.pos > 0 
		     && cursor.par->IsLineSeparator(cursor.pos - 1))
		    || (cursor.pos > 0
			&& cursor.par->IsNewline(cursor.pos - 1))
		    || (cursor.pos == 0
			&& !(cursor.par->Previous()
			     && cursor.par->Previous()->footnoteflag
			     == LyXParagraph::OPEN_FOOTNOTE))) {
	   		if (cursor.pos == 0 )
				owner_->owner()->getMiniBuffer()->Set(_("You cannot insert a space at the beginning of a paragraph.  Please read the Tutorial."));
			else
				owner_->owner()->getMiniBuffer()->Set(_("You cannot type two spaces this way.  Please read the Tutorial."));
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
			SetCursor(cursor.par, cursor.pos + 1, false, cursor.boundary);
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
		SetCursor(cursor.par, cursor.pos + 1, false, cursor.boundary);
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
		SetCursor(cursor.par, cursor.pos + 1, false, cursor.boundary);
	}

	/* check, wether the last characters font has changed. */ 
	if (cursor.pos && cursor.pos == cursor.par->Last()
	    && rawparfont != rawtmpfont) {
		RedoHeightOfParagraph(cursor);
	} else {
		/* now the special right address boxes */
		if (textclasslist.Style(buffer->params.textclass,
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

        bool is_rtl = row->par->isRightToLeftPar();

	if (is_rtl) {
		x = RightMargin(row);
		if (row->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE) {
			LyXFont font(LyXFont::ALL_SANE);
			font.setSize(LyXFont::SIZE_SMALL);
			x += lyxfont::width("Mwide-figM", font);
		}
	} else
		x = LeftMargin(row);
	
	/* is there a manual margin with a manual label */ 
	if (textclasslist.Style(buffer->params.textclass,
			   row->par->GetLayout()).margintype == MARGIN_MANUAL
	    && textclasslist.Style(buffer->params.textclass,
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
	  align = textclasslist.Style(buffer->params.textclass, row->par->GetLayout()).align;
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
	      else if (is_rtl)
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
	if (is_rtl) {
		LyXParagraph::size_type main_body = 
			BeginningOfMainBody(row->par);
		LyXParagraph::size_type last = RowLast(row);

		if (main_body > 0 &&
		    (main_body-1 > last || 
		     !row->par->IsLineSeparator(main_body-1))) {
			LyXLayout const & layout = textclasslist.Style(buffer->params.textclass,
								       row->par->GetLayout());
			x += lyxfont::width(layout.labelsep,
					    GetFont(row->par, -2));
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
	// CHECK See comment on top of text.C

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
	// Move cursor to the beginning, when not already there.
	if ( cursor.pos
	     && !cursor.par->IsSeparator(cursor.pos-1)
	     && !cursor.par->IsKomma(cursor.pos-1) )
		CursorLeftOneWord();

	// set the sel cursor
	sel_cursor = cursor;

	while ( cursor.pos < cursor.par->Last()
			&& !cursor.par->IsSeparator(cursor.pos)
			&& !cursor.par->IsKomma(cursor.pos) )
		cursor.pos++;
	SetCursor( cursor.par, cursor.pos );
	
	// finally set the selection
	SetSelection();
}


/* -------> Select the word currently under the cursor when:
			1: no selection is currently set,
			2: the cursor is not at the borders of the word. */

bool LyXText::SelectWordWhenUnderCursor() 
{
	if (!selection &&
	    cursor.pos > 0 && cursor.pos < cursor.par->Last()
	    && !cursor.par->IsSeparator(cursor.pos)
	    && !cursor.par->IsKomma(cursor.pos)
	    && !cursor.par->IsSeparator(cursor.pos -1)
	    && !cursor.par->IsKomma(cursor.pos -1) ) {
		SelectWord();
		return true;
	}
	return false;
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

#ifdef HAVE_SSTREAM
	std::ostringstream latex;
#else
	ostrstream latex;
#endif
	/* and find the end of the word 
	   (optional hyphens are part of a word) */
	while (cursor.pos < cursor.par->Last()
	       && (cursor.par->IsLetter(cursor.pos)) 
	           || (cursor.par->GetChar(cursor.pos) == LyXParagraph::META_INSET
		       && cursor.par->GetInset(cursor.pos) != 0
		       && cursor.par->GetInset(cursor.pos)->Latex(latex, false, false) == 0
#ifdef HAVE_SSTREAM
		       && latex.str() == "\\-"
#else
		&& string(latex.str(), 3) == "\\-" // this is not nice at all
#endif
			   ))
		cursor.pos++;

#ifndef HAVE_SSTREAM
	delete [] latex.str();
#endif
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

#ifdef HAVE_SSTREAM
	std::ostringstream latex;
#else
	ostrstream latex;
#endif
	
	/* now find the end of the word */
	while (cursor.pos < cursor.par->Last()
	       && (cursor.par->IsLetter(cursor.pos)
	           || (cursor.par->GetChar(cursor.pos) == LyXParagraph::META_INSET
		       && cursor.par->GetInset(cursor.pos) != 0
		       && cursor.par->GetInset(cursor.pos)->Latex(latex, false, false) == 0
#ifdef HAVE_SSTREAM
		       && latex.str() == "\\-"
#else
		       && string(latex.str(), 3) == "\\-"
#endif
			   )))
		cursor.pos++;
	
#ifndef HAVE_SSTREAM
	delete [] latex.str();
#endif
	SetCursor(cursor.par, cursor.pos);
	
	/* finally set the selection */ 
	SetSelection();
}


/* -------> Delete from cursor up to the end of the current or next word. */
void LyXText::DeleteWordForward()
{
	if (!cursor.par->Last())
		CursorRight();
	else {
		LyXCursor tmpcursor = cursor;
		selection = true; // to avoid deletion 
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
       if (!cursor.par->Last())
	       CursorLeft();
       else {
	       LyXCursor tmpcursor = cursor;
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
	if (!cursor.par->Last())
		// Paragraph is empty, so we just go to the right
		CursorRight();
	else {
		LyXCursor tmpcursor = cursor;
		selection = true; // to avoid deletion 
		CursorEnd();
		sel_cursor = cursor;
		cursor = tmpcursor;
		SetSelection();
		// What is this test for ??? (JMarc)
		if (!selection) {
			DeleteWordForward();
		} else {
			CutSelection();
		}
	}
}


// Change the case of a word at cursor position. 
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
		unsigned char c = tmppar->GetChar(tmppos);
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
		
		//tmppar->text[tmppos] = c;
		tmppar->SetChar(tmppos, c);
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

	// CHECK Look at the comment here.
	// This check is not very good...
	// The CursorRightIntern calls DeleteEmptyParagrapgMechanism
	// and that can very well delete the par or par->previous in
	// old_cursor. Will a solution where we compare paragraph id's
	//work better?
	if ((cursor.par->previous ? cursor.par->previous->id() : 0)
	    == old_cur_par_prev_id
	    && cursor.par->id() != old_cur_par_id)
		return; // delete-empty-paragraph-mechanism has done it

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


void LyXText::Backspace()
{
	/* table stuff -- begin */
	if (cursor.par->table) {
		BackspaceInTable();
		return;
	}
	/* table stuff -- end */

	// LyXFont rawtmpfont = current_font;
	// LyXFont realtmpfont = real_current_font;
	//    We don't need the above variables as calling to SetCursor() with third
	//    argument eqaul to false, will not change current_font & real_current_font
	
	// Get the font that is used to calculate the baselineskip
	LyXParagraph::size_type lastpos = cursor.par->Last();
	LyXFont rawparfont = cursor.par->GetFontSettings(lastpos - 1);

	if (cursor.pos == 0) {
		// The cursor is at the beginning of a paragraph, so the the backspace
		// will collapse two paragraphs into one.
		
		// we may paste some paragraphs
      
		// is it an empty paragraph?
      
		if ((lastpos == 0
		     || (lastpos == 1 && cursor.par->IsSeparator(0)))
		    && !(cursor.par->Next() 
			 && cursor.par->footnoteflag == LyXParagraph::NO_FOOTNOTE
			 && cursor.par->Next()->footnoteflag == LyXParagraph::OPEN_FOOTNOTE)) {
			// This is an empty paragraph and we delete it just by moving the cursor one step
			// left and let the DeleteEmptyParagraphMechanism handle the actual deletion
			// of the paragraph.
			
			if (cursor.par->previous) {
				LyXParagraph * tmppar = cursor.par->previous->FirstPhysicalPar();
				if (cursor.par->GetLayout() == tmppar->GetLayout()
				    && cursor.par->footnoteflag == tmppar->footnoteflag
				    && cursor.par->GetAlign() == tmppar->GetAlign()) {
					// Inherit botom DTD from the paragraph below.
					// (the one we are deleting)
					tmppar->line_bottom = cursor.par->line_bottom;
					tmppar->added_space_bottom = cursor.par->added_space_bottom;
					tmppar->pagebreak_bottom = cursor.par->pagebreak_bottom;
				}
				
				CursorLeftIntern();
		     
				// the layout things can change the height of a row !
				int tmpheight = cursor.row->height;
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
		
  		LyXParagraph * tmppar = cursor.par;
  		Row * tmprow = cursor.row;

		// We used to do CursorLeftIntern() here, but it is
		// not a good idea since it triggers the auto-delete
		// mechanism. So we do a CursorLeftIntern()-lite,
		// without the dreaded mechanism. (JMarc)
		if (cursor.par->Previous()) { 
			// steps into the above paragraph.
			SetCursorIntern(cursor.par->Previous(), 
					cursor.par->Previous()->Last(), false);
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
		if (cursor.par != tmppar
		    && (cursor.par->GetLayout() == tmppar->GetLayout()
			|| tmppar->GetLayout() == 0 /*standard*/)
		    && cursor.par->footnoteflag == tmppar->footnoteflag
		    /* table stuff -- begin*/
		    && !cursor.par->table /* no pasting of tables */ 
		    /* table stuff -- end*/
		    && cursor.par->GetAlign() == tmppar->GetAlign()) {

			RemoveParagraph(tmprow);
			RemoveRow(tmprow);
			cursor.par->PasteParagraph();
			
			if (!cursor.pos || !cursor.par->IsSeparator(cursor.pos - 1))
				; //cursor.par->InsertChar(cursor.pos, ' ');
			// strangely enough it seems that commenting out the line above removes
			// most or all of the segfaults. I will however also try to move the
			// two Remove... lines in front of the PasteParagraph too.
			else
				if (cursor.pos)
					cursor.pos--;
			
			status = LyXText::NEED_MORE_REFRESH;
			refresh_row = cursor.row;
			refresh_y = cursor.y - cursor.row->baseline;
			
			// remove the lost paragraph
			// This one is not safe, since the paragraph that the tmprow and the
			// following rows belong to has been deleted by the PasteParagraph
			// above. The question is... could this be moved in front of the
			// PasteParagraph?
			//RemoveParagraph(tmprow);
			//RemoveRow(tmprow);  
			
			AppendParagraph(cursor.row); // This rebuilds the rows.
			UpdateCounters(cursor.row);
			
			// the row may have changed, block, hfills etc.
			SetCursor(cursor.par, cursor.pos, false);
		}
	} else {
		/* this is the code for a normal backspace, not pasting
		 * any paragraphs */ 
		SetUndo(Undo::DELETE, 
			cursor.par->ParFromPos(cursor.pos)->previous, 
			cursor.par->ParFromPos(cursor.pos)->next); 
		// We used to do CursorLeftIntern() here, but it is
		// not a good idea since it triggers the auto-delete
		// mechanism. So we do a CursorLeftIntern()-lite,
		// without the dreaded mechanism. (JMarc)
		SetCursorIntern(cursor.par, cursor.pos - 1, false, cursor.boundary);
		
		// some insets are undeletable here
		if (cursor.par->GetChar(cursor.pos) == LyXParagraph::META_INSET) {
			if (!cursor.par->GetInset(cursor.pos)->Deletable())
				return; 
			// force complete redo when erasing display insets
			// this is a cruel method but safe..... Matthias 
			if (cursor.par->GetInset(cursor.pos)->display()){
				cursor.par->Erase(cursor.pos);
				RedoParagraph();
				return;
			}
		}
		
		Row * row = cursor.row;
		long y = cursor.y - row->baseline;
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
			Row * tmprow = row;
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
			Row * tmprow = row;
			while (tmprow->next && tmprow->next->par == row->par) {
				tmprow = tmprow->next;
				tmprow->pos--;
			}

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
				
				Row * tmprow = row->previous;
				
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
				SetCursor(cursor.par, cursor.pos, false, cursor.boundary);
				//current_font = rawtmpfont;
				//real_current_font = realtmpfont;
				// check, whether the last character's font has changed.
				if (rawparfont !=
				    cursor.par->GetFontSettings(cursor.par->Last() - 1))
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
			SetCursor(cursor.par, cursor.pos, false, cursor.boundary);
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
			SetCursor(cursor.par, cursor.pos, false, cursor.boundary);
		}
	}

	// current_font = rawtmpfont;
	// real_current_font = realtmpfont;

	lastpos = cursor.par->Last();
	if (cursor.pos == lastpos) {
		SetCurrentFont();
		if (IsBoundary(cursor.par, cursor.pos) != cursor.boundary)
			SetCursor(cursor.par, cursor.pos, false, !cursor.boundary);
	}
	
	// check, wether the last characters font has changed.
	if (rawparfont != 
	    cursor.par->GetFontSettings(lastpos - 1)) {
		RedoHeightOfParagraph(cursor);
	} else {
		// now the special right address boxes
		if (textclasslist.Style(buffer->params.textclass,
					cursor.par->GetLayout()).margintype == MARGIN_RIGHT_ADDRESS_BOX) {
			RedoDrawingOfParagraph(cursor); 
		}
	}
}


void LyXText::GetVisibleRow(int offset, Row * row_ptr, long y)
{
	/* returns a printed row */
	Painter & pain = owner_->painter();
	
	bool is_rtl = row_ptr->par->isRightToLeftPar();
	LyXParagraph::size_type last = RowLastPrintable(row_ptr);

	LyXParagraph::size_type vpos, pos;
	float x, tmpx;
	int y_top, y_bottom;
	float fill_separator, fill_hfill, fill_label_hfill;

	LyXFont font;
	int maxdesc;
	if (row_ptr->height <= 0) {
		lyxerr << "LYX_ERROR: row.height: " << row_ptr->height << endl;
		return;
	}
	PrepareToPrint(row_ptr, x, fill_separator,
		       fill_hfill, fill_label_hfill);
	
	// clear the area where we want to paint/print
	pain.fillRectangle(0, offset, paperwidth, row_ptr->height);
	
	if (selection) {
		/* selection code */
		if (bidi_same_direction) {
			if (sel_start_cursor.row == row_ptr &&
			    sel_end_cursor.row == row_ptr) {
				if (sel_start_cursor.x < sel_end_cursor.x)
					pain.fillRectangle(sel_start_cursor.x, offset,
							   sel_end_cursor.x - sel_start_cursor.x,
							   row_ptr->height,
							   LColor::selection);
				else
					pain.fillRectangle(sel_end_cursor.x, offset,
							   sel_start_cursor.x - sel_end_cursor.x,
							   row_ptr->height,
							   LColor::selection);
			} else if (sel_start_cursor.row == row_ptr) {
				if (is_rtl)
					pain.fillRectangle(0, offset,
							   sel_start_cursor.x,
							   row_ptr->height,
							   LColor::selection);
				else
					pain.fillRectangle(sel_start_cursor.x, offset,
							   paperwidth - sel_start_cursor.x,
							   row_ptr->height,
							   LColor::selection);
			} else if (sel_end_cursor.row == row_ptr) {
				if (is_rtl)
					pain.fillRectangle(sel_end_cursor.x, offset,
							   paperwidth - sel_end_cursor.x,
							   row_ptr->height,
							   LColor::selection);
				else
					pain.fillRectangle(0, offset,
							   sel_end_cursor.x,
							   row_ptr->height,
							   LColor::selection);
			} else if (y > long(sel_start_cursor.y)
				   && y < long(sel_end_cursor.y)) {
				pain.fillRectangle(0, offset,
						   paperwidth, row_ptr->height,
						   LColor::selection);
			}
		} else if ( sel_start_cursor.row != row_ptr &&
			    sel_end_cursor.row != row_ptr &&
			    y > long(sel_start_cursor.y)
			    && y < long(sel_end_cursor.y)) {
			pain.fillRectangle(0, offset,
					   paperwidth, row_ptr->height,
					   LColor::selection);
		} else if (sel_start_cursor.row == row_ptr ||
			   sel_end_cursor.row == row_ptr) {
			float tmpx = x;
			int cell = 0;
			if (row_ptr->par->table) {
				cell = NumberOfCell(row_ptr->par, row_ptr->pos);
				tmpx += row_ptr->par->table->GetBeginningOfTextInCell(cell);
			}
			if ( (sel_start_cursor.row != row_ptr && !is_rtl) ||
			     (sel_end_cursor.row != row_ptr && is_rtl))
				pain.fillRectangle(0, offset,
						   int(tmpx), row_ptr->height,
						   LColor::selection);
			if (row_ptr->par->table) {
				float x_old = x;
				for (vpos = row_ptr->pos; vpos <= last; ++vpos)  {
					pos = vis2log(vpos);
					float old_tmpx = tmpx;
					if (row_ptr->par->IsNewline(pos)) {
						tmpx = x_old + row_ptr->par->table->WidthOfColumn(cell);
						x_old = tmpx;
						++cell;
						tmpx += row_ptr->par->table->GetBeginningOfTextInCell(cell);
					} else {
						tmpx += SingleWidth(row_ptr->par, pos);
					}
					if ( (sel_start_cursor.row != row_ptr ||
					      sel_start_cursor.pos <= pos) &&
					     (sel_end_cursor.row != row_ptr ||
					      pos < sel_end_cursor.pos) )
						pain.fillRectangle(int(old_tmpx), offset,
								   int(tmpx - old_tmpx + 1),
								   row_ptr->height,
								   LColor::selection);
				}
			} else {
				LyXParagraph::size_type main_body =
					BeginningOfMainBody(row_ptr->par);

				for (vpos = row_ptr->pos; vpos <= last; ++vpos)  {
					pos = vis2log(vpos);
					float old_tmpx = tmpx;
					if (main_body > 0 && pos == main_body-1) {
						tmpx += fill_label_hfill +
							lyxfont::width(textclasslist.Style(buffer->params.textclass,
											   row_ptr->par->GetLayout()).labelsep,
								       GetFont(row_ptr->par, -2));
						if (row_ptr->par->IsLineSeparator(main_body-1))
							tmpx -= SingleWidth(row_ptr->par, main_body-1);
					}
					if (HfillExpansion(row_ptr, pos)) {
						tmpx += SingleWidth(row_ptr->par, pos);
						if (pos >= main_body)
							tmpx += fill_hfill;
						else 
							tmpx += fill_label_hfill;
					}
					else if (row_ptr->par->IsSeparator(pos)) {
						tmpx += SingleWidth(row_ptr->par, pos);
						if (pos >= main_body)
							tmpx += fill_separator;
					} else
						tmpx += SingleWidth(row_ptr->par, pos);

					if ( (sel_start_cursor.row != row_ptr ||
					      sel_start_cursor.pos <= pos) &&
					     (sel_end_cursor.row != row_ptr ||
					      pos < sel_end_cursor.pos) )
						pain.fillRectangle(int(old_tmpx), offset,
								   int(tmpx - old_tmpx + 1),
								   row_ptr->height,
							   LColor::selection);
				}
			}
			if ( (sel_start_cursor.row != row_ptr && is_rtl) ||
			     (sel_end_cursor.row != row_ptr && !is_rtl) )
				pain.fillRectangle(int(tmpx), offset,
						   int(paperwidth - tmpx),
						   row_ptr->height,
						   LColor::selection);
		}
	}

	int box_x = 0;
	if (row_ptr->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE) {
		LyXFont font(LyXFont::ALL_SANE);
		font.setSize(LyXFont::SIZE_FOOTNOTE);
		font.setColor(LColor::footnote);
		
		box_x = LYX_PAPER_MARGIN + lyxfont::width(" wide-tab ", font);
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
			
			pain.fillRectangle(LYX_PAPER_MARGIN,
					   offset + 1,
					   box_x - LYX_PAPER_MARGIN,
					   int(lyxfont::maxAscent(font)
					       + lyxfont::maxDescent(font)),
					   LColor::footnotebg);
			
			pain.line(LYX_PAPER_MARGIN, offset,
				  paperwidth - LYX_PAPER_MARGIN, offset,
				  LColor::footnoteframe);
			
			pain.text(LYX_PAPER_MARGIN,
				  offset + int(lyxfont::maxAscent(font)) + 1,
				  fs, font);
			
			pain.line(LYX_PAPER_MARGIN, offset,
				  LYX_PAPER_MARGIN,
				  offset + int(lyxfont::maxAscent(font)
					       + lyxfont::maxDescent(font)),
				  LColor::footnoteframe);
			
			pain.line(LYX_PAPER_MARGIN,
				  offset + int(lyxfont::maxAscent(font)
					       + lyxfont::maxDescent(font)) + 1,
				  box_x,
				  offset + int(lyxfont::maxAscent(font)
					       + lyxfont::maxDescent(font)) + 1,
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


		// Draw appendix lines
		LyXParagraph * p = row_ptr->par->PreviousBeforeFootnote()->FirstPhysicalPar();
		if (p->appendix){
			pain.line(1, offset,
				  1, offset + row_ptr->height,
				  LColor::appendixline);
			pain.line(paperwidth - 2, offset,
				  paperwidth - 2, offset + row_ptr->height,
				  LColor::appendixline);
		}

		// Draw minipage line
		bool minipage = p->pextra_type == LyXParagraph::PEXTRA_MINIPAGE;
		if (minipage)
			pain.line(LYX_PAPER_MARGIN/5, offset,
				  LYX_PAPER_MARGIN/5, 
				  offset + row_ptr->height - 1,
				  LColor::minipageline);

		// Draw depth lines
		int depth = p->GetDepth();
		for (int i = 1; i <= depth; ++i) {
			int line_x = (LYX_PAPER_MARGIN/5)*(i+minipage);
			pain.line(line_x, offset, line_x,
				  offset + row_ptr->height - 1,
				  LColor::depthbar);
		}
	} else if (row_ptr->previous &&
		   row_ptr->previous->par->footnoteflag
		   == LyXParagraph::OPEN_FOOTNOTE) {
		LyXFont font(LyXFont::ALL_SANE);
		font.setSize(LyXFont::SIZE_FOOTNOTE);
		
		int box_x = LYX_PAPER_MARGIN;
		box_x += lyxfont::width(" wide-tab ", font);
		
		pain.line(box_x, offset,
			  paperwidth - LYX_PAPER_MARGIN,
			  offset, LColor::footnote);
	}

	// Draw appendix lines
	LyXParagraph * firstpar = row_ptr->par->FirstPhysicalPar();
	if (firstpar->appendix){
		pain.line(1, offset,
			  1, offset + row_ptr->height,
			  LColor::appendixline);
		pain.line(paperwidth - 2, offset,
			  paperwidth - 2, offset + row_ptr->height,
			  LColor::appendixline);
	}

	// Draw minipage line
	bool minipage = firstpar->pextra_type == LyXParagraph::PEXTRA_MINIPAGE;
	if (minipage)
		pain.line(LYX_PAPER_MARGIN/5 + box_x, offset,
			  LYX_PAPER_MARGIN/5 + box_x, 
			  offset + row_ptr->height - 1,
			  LColor::minipageline);

	// Draw depth lines
	int depth = firstpar->GetDepth();
	if (depth > 0) {
		int next_depth = 0;
		int prev_depth = 0;
		if (row_ptr->next)
			if (row_ptr->par->footnoteflag ==
			    row_ptr->next->par->footnoteflag)
				next_depth = row_ptr->next->par->GetDepth();
			else if (row_ptr->par->footnoteflag != LyXParagraph::OPEN_FOOTNOTE)
				next_depth = depth;

		if (row_ptr->previous)
			if (row_ptr->par->footnoteflag ==
			    row_ptr->previous->par->footnoteflag)
				prev_depth = row_ptr->previous->par->GetDepth();
			else if (row_ptr->par->footnoteflag != LyXParagraph::OPEN_FOOTNOTE)
				prev_depth = depth;

		for (int i = 1; i <= depth; ++i) {
			int line_x = (LYX_PAPER_MARGIN/5)*(i+minipage)+box_x;
			pain.line(line_x, offset, line_x,
				  offset + row_ptr->height - 1 - (i-next_depth-1)*3,
				  LColor::depthbar);
		
			if (i > prev_depth)
				pain.fillRectangle(line_x, offset, LYX_PAPER_MARGIN/5, 2,
						   LColor::depthbar);
			if (i > next_depth)
				pain.fillRectangle(line_x,
						   offset + row_ptr->height - 2 - (i-next_depth-1)*3,
						   LYX_PAPER_MARGIN/5, 2,
						   LColor::depthbar);
		}
	}

	
	LyXLayout const & layout =
		textclasslist.Style(buffer->params.textclass,
				    row_ptr->par->GetLayout());

	y_top = 0;
	y_bottom = row_ptr->height;
	
	/* is it a first row? */ 
	if (row_ptr->pos == 0
	    && row_ptr->par == firstpar) {
		
		/* start of appendix? */
		if (row_ptr->par->start_of_appendix){
			pain.line(1, offset,
				  paperwidth - 2, offset,
				  LColor::appendixline);
		}
		
		/* think about the margins */ 
		if (!row_ptr->previous)
			y_top += LYX_PAPER_MARGIN;
		
		if (row_ptr->par->pagebreak_top){ /* draw a top pagebreak  */
			LyXFont pb_font;
			pb_font.setColor(LColor::pagebreak).decSize();
			int w = 0, a = 0, d = 0;
			pain.line(0, offset + y_top + 2*DefaultHeight(),
				  paperwidth, 
				  offset + y_top + 2*DefaultHeight(),
				  LColor::pagebreak, 
				  Painter::line_onoffdash)
				.rectText(0,
					  0,
					  _("Page Break (top)"),
					  pb_font,
					  LColor::background,
					  LColor::background, false, w, a, d);
			pain.rectText((paperwidth - w)/2,
				      offset +y_top + 2*DefaultHeight() +d,
				      _("Page Break (top)"),
				      pb_font,
				      LColor::background,
				      LColor::background);
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
		y_top += int(row_ptr->par->added_space_top.inPixels(owner_));
		
		/* think about the parskip */ 
		/* some parskips VERY EASY IMPLEMENTATION */ 
		if (buffer->params.paragraph_separation == BufferParams::PARSEP_SKIP) {
			if (layout.latextype == LATEX_PARAGRAPH
			    && firstpar->GetDepth() == 0
			    && firstpar->Previous())
				y_top += buffer->params.getDefSkip().inPixels(owner_);
			else if (firstpar->Previous()
				 && textclasslist.Style(buffer->params.textclass,
							firstpar->Previous()->GetLayout()).latextype == LATEX_PARAGRAPH
				 && firstpar->Previous()->GetDepth() == 0)
				// is it right to use defskip here, too? (AS) 
				y_top += buffer->params.getDefSkip().inPixels(owner_);
		}
		
		if (row_ptr->par->line_top) {      /* draw a top line  */
			y_top +=  lyxfont::ascent('x', GetFont(row_ptr->par, 0));
			
			pain.line(0, offset + y_top,
				  paperwidth, offset + y_top,
				  LColor::topline,
				  Painter::line_solid,
				  Painter::line_thick);
			
			y_top +=  lyxfont::ascent('x', GetFont(row_ptr->par, 0));
		}
		
		/* should we print a label? */ 
		if (layout.labeltype >= LABEL_STATIC
		    && (layout.labeltype != LABEL_STATIC
			|| layout.latextype != LATEX_ENVIRONMENT
			|| row_ptr->par->IsFirstInSequence())) {
			font = GetFont(row_ptr->par, -2);
			if (!row_ptr->par->GetLabelstring().empty()) {
				tmpx = x;
				string tmpstring = row_ptr->par->GetLabelstring();
				
				if (layout.labeltype == LABEL_COUNTER_CHAPTER) {
					if (buffer->params.secnumdepth >= 0) {
						/* this is special code for the chapter layout. This is printed in
						 * an extra row and has a pagebreak at the top. */
						float spacing_val = 1.0;
						if (!row_ptr->par->spacing.isDefault()) {
							spacing_val = row_ptr->par->spacing.getValue();
						} else {
							spacing_val = buffer->params.spacing.getValue();
						}
   
						maxdesc = int(lyxfont::maxDescent(font) * layout.spacing.getValue() * spacing_val)
							+ int(layout.parsep) * DefaultHeight();
						if (is_rtl)
							tmpx = paperwidth - LeftMargin(row_ptr) - 
								lyxfont::width(tmpstring, font);
						pain.text(int(tmpx),
							  offset + row_ptr->baseline - row_ptr->ascent_of_text - maxdesc,
							  tmpstring, font);
					}
				} else {
					if (is_rtl) {
						tmpx = paperwidth - LeftMargin(row_ptr)
							+ lyxfont::width(layout.labelsep, font);
						if (row_ptr->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE)  {
							LyXFont font(LyXFont::ALL_SANE);
							font.setSize(LyXFont::SIZE_SMALL);
							tmpx += lyxfont::width("Mwide-fixM", font);
						}
					} else
						tmpx = x - lyxfont::width(layout.labelsep, font)
							- lyxfont::width(tmpstring, font);

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
				if (!row_ptr->par->GetLabelstring().empty()) {
					string tmpstring = row_ptr->par->GetLabelstring();
					float spacing_val = 1.0;
					if (!row_ptr->par->spacing.isDefault()) {
						spacing_val = row_ptr->par->spacing.getValue();
					} else {
						spacing_val = buffer->params.spacing.getValue();
					}
   
					maxdesc = int(lyxfont::maxDescent(font) * layout.spacing.getValue() * spacing_val
						      + (layout.labelbottomsep * DefaultHeight()));
					
					tmpx = x;
					if (layout.labeltype == LABEL_CENTERED_TOP_ENVIRONMENT){
						tmpx = ( (is_rtl ? LeftMargin(row_ptr) : x)
							 + paperwidth - RightMargin(row_ptr) ) / 2; 
						tmpx -= lyxfont::width(tmpstring, font) / 2;
					} else if (is_rtl)
						tmpx = paperwidth - LeftMargin(row_ptr) - 
							lyxfont::width(tmpstring, font);
					pain.text(int(tmpx),
						  offset + row_ptr->baseline
						  - row_ptr->ascent_of_text
						  - maxdesc,
						  tmpstring, font);
				}
			}
		}
		if (layout.labeltype == LABEL_BIBLIO && row_ptr->par->bibkey) {
			font = GetFont(row_ptr->par, -1);
			if (is_rtl)
				tmpx = paperwidth - LeftMargin(row_ptr)
					+ lyxfont::width(layout.labelsep, font);
			else
				tmpx = x - lyxfont::width(layout.labelsep, font)
					- row_ptr->par->bibkey->width(owner_->painter(), font);
			row_ptr->par->bibkey->draw(pain,
						   font,
						   offset + row_ptr->baseline, 
						   tmpx);
		}
	}
	
	/* is it a last row? */
	LyXParagraph * par = row_ptr->par->LastPhysicalPar();
	if (row_ptr->par->ParFromPos(last + 1) == par
	    && (!row_ptr->next || row_ptr->next->par != row_ptr->par)) {     
		
		/* think about the margins */ 
		if (!row_ptr->next)
			y_bottom -= LYX_PAPER_MARGIN;
		
		/* draw a bottom pagebreak */ 
		if (firstpar->pagebreak_bottom) {
			LyXFont pb_font;
			pb_font.setColor(LColor::pagebreak).decSize();
			int w = 0, a = 0, d = 0;
			pain.line(0,
				  offset + y_bottom - 2 * DefaultHeight(), 
				  paperwidth, 
				  offset + y_bottom - 2 * DefaultHeight(),
				  LColor::pagebreak,
				  Painter::line_onoffdash)
				.rectText(0,
					  0,
					  _("Page Break (bottom)"),
					  pb_font,
					  LColor::background,
					  LColor::background, false, w, a, d);
			pain.rectText((paperwidth - w)/2,
				      offset +y_top + 2*DefaultHeight() +d,
				      _("Page Break (bottom)"),
				      pb_font,
				      LColor::background,
				      LColor::background);
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
		y_bottom -= int(firstpar->added_space_bottom.inPixels(owner_));
		
		if (firstpar->line_bottom) {
			/* draw a bottom line */
			y_bottom -= lyxfont::ascent('x', GetFont(par, par->Last() - 1));
			pain.line(0, offset + y_bottom,
				  paperwidth, offset + y_bottom,
				  LColor::topline, Painter::line_solid,
				  Painter::line_thick);
			y_bottom -= lyxfont::ascent('x', GetFont(par, par->Last() - 1));
		}

		// draw an endlabel
		int endlabel = row_ptr->par->GetEndLabel();
		switch (endlabel) {
		case END_LABEL_BOX:
		case END_LABEL_FILLED_BOX:
		{
			LyXFont font = GetFont(row_ptr->par, last);
			int size = int(0.75 * lyxfont::maxAscent(font));
			int y = (offset + row_ptr->baseline) - size;
			int x = is_rtl ? LYX_PAPER_MARGIN 
				: paperwidth - LYX_PAPER_MARGIN - size;
			if (row_ptr->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE)
				if (is_rtl) {
					LyXFont font(LyXFont::ALL_SANE);
					font.setSize(LyXFont::SIZE_SMALL);
					x += lyxfont::width("Mwide-figM", font);
				} else
					x -= LYX_PAPER_MARGIN/2;
			if (row_ptr->fill <= size)
				x += (size - row_ptr->fill + 1) * (is_rtl ? -1 : 1);
			if (endlabel == END_LABEL_BOX) {
				pain.line(x, y, x, y + size,
					  LColor::eolmarker);
				pain.line(x + size, y, x + size , y + size,
					  LColor::eolmarker);
				pain.line(x, y, x + size, y,
					  LColor::eolmarker);
				pain.line(x, y + size, x + size, y + size,
					  LColor::eolmarker);
			} else
				pain.fillRectangle(x, y, size, size,
						   LColor::eolmarker);
			break;
		}
		case END_LABEL_STATIC:
		{
			LyXTextClass::LayoutList::size_type layout = row_ptr->par->GetLayout();
			string tmpstring = textclasslist.Style(buffer->params.textclass,
							 layout).endlabelstring();
			font = GetFont(row_ptr->par, -2);
			int tmpx = is_rtl ? int(x) - lyxfont::width(tmpstring, font)
				: paperwidth - RightMargin(row_ptr) - row_ptr->fill;
			pain.text( tmpx, offset + row_ptr->baseline, tmpstring, font);
			break;
		}
		case END_LABEL_NO_LABEL:
			break;
		}
	}
	
	/* draw the text in the pixmap */  
	
	vpos = row_ptr->pos;
	/* table stuff -- begin*/
	if (row_ptr->par->table) {
		bool on_off;
		int cell = NumberOfCell(row_ptr->par, row_ptr->pos);
		float x_old = x;
		x += row_ptr->par->table->GetBeginningOfTextInCell(cell);
		
		while (vpos <= last)  {
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
						  int(x),
						  offset + row_ptr->baseline - row_ptr->ascent_of_text,
						  LColor::tableline,
						  on_off ? Painter::line_onoffdash : Painter::line_solid);
				
				on_off = !row_ptr->par->table->BottomLine(cell);
				if ((!on_off && !row_ptr->par->table->RowHasContRow(cell)) ||
				    row_ptr->par->table->VeryLastRow(cell))
					
					pain.line(int(x_old),
						  offset + y_bottom - 1,
						  int(x),
						  offset + y_bottom - 1,
						  LColor::tableline,
						  on_off ? Painter::line_onoffdash : Painter::line_solid);
				
				on_off = !row_ptr->par->table->LeftLine(cell);
				
				pain.line(int(x_old),
					  offset + row_ptr->baseline - row_ptr->ascent_of_text,
					  int(x_old),
					  offset + y_bottom - 1,
					  LColor::tableline,
					  on_off ? Painter::line_onoffdash : Painter::line_solid);
				
				on_off = !row_ptr->par->table->RightLine(cell);
				
				pain.line(int(x) - row_ptr->par->table->AdditionalWidth(cell),
					  offset + row_ptr->baseline - row_ptr->ascent_of_text,
					  int(x) - row_ptr->par->table->AdditionalWidth(cell),
					  offset + y_bottom - 1,
					  LColor::tableline,
					  on_off ? Painter::line_onoffdash : Painter::line_solid);
				
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
			} else if (row_ptr->par->IsSeparator(pos)) {
				tmpx = x;
				x+= SingleWidth(row_ptr->par, pos);
				++vpos;
			} else
				draw(row_ptr, vpos, offset, x);
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
					  int(x),
					  offset + row_ptr->baseline - row_ptr->ascent_of_text,
					  LColor::tableline,
					  on_off ? Painter::line_onoffdash : Painter::line_solid);
			on_off = !row_ptr->par->table->BottomLine(cell);
			if ((!on_off && !row_ptr->par->table->RowHasContRow(cell)) ||
			    row_ptr->par->table->VeryLastRow(cell))
				
				pain.line(int(x_old),
					  offset + y_bottom - 1,
					  int(x),
					  offset + y_bottom - 1,
					  LColor::tableline,
					  on_off ? Painter::line_onoffdash : Painter::line_solid);
			
			on_off = !row_ptr->par->table->LeftLine(cell);
			
			pain.line(int(x_old),
				  offset + row_ptr->baseline - row_ptr->ascent_of_text,
				  int(x_old),
				  offset + y_bottom - 1,
				  LColor::tableline,
				  on_off ? Painter::line_onoffdash : Painter::line_solid);
			
			on_off = !row_ptr->par->table->RightLine(cell);
			
			pain.line(int(x) - row_ptr->par->table->AdditionalWidth(cell),
				  offset + row_ptr->baseline - row_ptr->ascent_of_text,
				  int(x) - row_ptr->par->table->AdditionalWidth(cell),
				  offset + y_bottom - 1,
				  LColor::tableline,
				  on_off ? Painter::line_onoffdash : Painter::line_solid);
		}
	} else {
		/* table stuff -- end*/
		LyXParagraph::size_type main_body = 
			BeginningOfMainBody(row_ptr->par);
		if (main_body > 0 &&
		    (main_body-1 > last || 
		     !row_ptr->par->IsLineSeparator(main_body-1)))
			main_body = 0;
		
		while (vpos <= last)  {
			pos = vis2log(vpos);
			if (main_body > 0 && pos == main_body-1) {
				x += fill_label_hfill
					+ lyxfont::width(layout.labelsep, GetFont(row_ptr->par, -2))
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
							  int(x + fill_hfill),
							  offset + row_ptr->baseline - DefaultHeight() / 4,
							  LColor::vfillline,
							  Painter::line_onoffdash);
						x += fill_hfill;
					} else {
						pain.line(int(x),
							  offset + row_ptr->baseline - DefaultHeight() / 4,
							  int(x + fill_label_hfill),
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
			} else if (row_ptr->par->IsSeparator(pos)) {
				x += SingleWidth(row_ptr->par, pos);
				if (pos >= main_body)
					x += fill_separator;
				++vpos;
			} else
				draw(row_ptr, vpos, offset, x);
		}
	}
}


int LyXText::DefaultHeight() const
{
	LyXFont font(LyXFont::ALL_SANE);
	return int(lyxfont::maxAscent(font) + lyxfont::maxDescent(font) * 1.5);
}

   
/* returns the column near the specified x-coordinate of the row 
* x is set to the real beginning of this column  */ 
int LyXText::GetColumnNearX(Row * row, int & x, bool & boundary) const
{
	float tmpx = 0.0;
	float fill_separator, fill_hfill, fill_label_hfill;
   
	PrepareToPrint(row, tmpx, fill_separator,
		       fill_hfill, fill_label_hfill);

	LyXParagraph::size_type vc = row->pos;
	LyXParagraph::size_type last = RowLastPrintable(row);
	LyXParagraph::size_type c = 0;
	LyXLayout const & layout = textclasslist.Style(buffer->params.textclass,
						       row->par->GetLayout());
	bool left_side = false;

	/* table stuff -- begin */
	if (row->par->table) {
		//the last row doesn't need a newline at the end
		if (row->next && row->next->par == row->par
		    && row->par->IsNewline(last))
			last--;
		int cell = NumberOfCell(row->par, row->pos);
		float cell_x = tmpx + row->par->table->WidthOfColumn(cell);
		tmpx += row->par->table->GetBeginningOfTextInCell(cell);
		float last_tmpx = tmpx;
		while (vc <= last && tmpx <= x) {
		       c = vis2log(vc);
		       last_tmpx = tmpx;
		       if (row->par->IsNewline(c)) {
			       if (cell_x <= x){
				       ++cell;
				       tmpx = cell_x + row->par->table->GetBeginningOfTextInCell(cell);
				       cell_x += row->par->table->WidthOfColumn(cell);
				       ++vc;
			       } else
				       break;
		       } else {
			       tmpx += SingleWidth(row->par, c);
			       ++vc;
		       }
		}
		if (vc > row->pos && !row->par->IsNewline(c) &&
		    (tmpx+last_tmpx)/2 > x) {
			tmpx = last_tmpx;
			left_side = true;
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
					lyxfont::width(layout.labelsep,
					       GetFont(row->par, -2));
				if (row->par->IsLineSeparator(main_body-1))
					tmpx -= SingleWidth(row->par, main_body-1);
			}
	     
			if (HfillExpansion(row, c)) {
				x += SingleWidth(row->par, c);
				if (c >= main_body)
					tmpx += fill_hfill;
				else
					tmpx += fill_label_hfill;
			}
			else if (row->par->IsSeparator(c)) {
				tmpx += SingleWidth(row->par, c);
				if (c >= main_body)
					tmpx+= fill_separator;
			} else
				tmpx += SingleWidth(row->par, c);
			++vc;
		}

		if (vc > row->pos && (tmpx+last_tmpx)/2 > x) {
			tmpx = last_tmpx;
			left_side = true;
		}
	}


	if (vc > last + 1)  // This shouldn't happen.
		vc = last+1;

	boundary = false;

	if (row->pos > last)  // Row is empty?
		c = row->pos;
	else if (vc == row->pos ||
		 (row->par->table && vc <= last && row->par->IsNewline(vc-1)) ) {
		c = vis2log(vc);
		if (bidi_level(c) % 2 == 1)
			++c;
	} else {
		c = vis2log(vc - 1);
		bool rtl = (bidi_level(c) % 2 == 1);
		if (left_side == rtl) {
			++c;
			boundary = IsBoundary(row->par, c);
		}
	}

	if (!row->par->table && row->pos <= last && c > last
	    && row->par->IsNewline(last)) {
		if (bidi_level(last) % 2 == 0)
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
		   pair<bool, LyXTextClass::size_type> lres =
			   textclasslist.NumberOfLayout(buffer->params.textclass,
							"Caption");
		   LyXTextClass::size_type lay;
		   if (lres.first) {
			   // layout fount
			   lay = lres.second;
		   } else {
			   // layout not found
			   lay = 0; // use default layout "Standard" (0)
		   }
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
	Row * tmprow = firstrow;
	y = 0;
	
	// find the first row of the specified paragraph
	while (tmprow->next && tmprow->par != par) {
		y += tmprow->height;
		tmprow = tmprow->next;
	}
	
	// now find the wanted row
	while (tmprow->pos < pos
	       && tmprow->next
	       && tmprow->next->par == par
	       && tmprow->next->pos <= pos) {
		y += tmprow->height;
		tmprow = tmprow->next;
	}
	
	return tmprow;
}
