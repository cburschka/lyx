// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1998 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#include <fstream>
#include <algorithm>
using std::ifstream;
using std::min;
using std::max;

#include <cstdlib>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insettext.h"
#include "lyxlex.h"
#include "debug.h"
#include "lyxfont.h"
#include "lyxlex.h"
#include "commandtags.h"
#include "buffer.h"
#include "LyXView.h"
#include "BufferView.h"
#include "support/textutils.h"
#include "layout.h"
#include "insetlatexaccent.h"
#include "insetquotes.h"
#include "mathed/formulamacro.h"
#include "figinset.h"
#include "insetinfo.h"
#include "insetinclude.h"
#include "insetbib.h"
#include "insetcommand.h"
#include "insetindex.h"
#include "insetlabel.h"
#include "insetref.h"
//#include "insettabular.h"
#include "insetert.h"
#include "insetspecialchar.h"
#include "LaTeXFeatures.h"
#include "Painter.h"
#include "lyx_gui_misc.h"
#include "support/LAssert.h"

extern unsigned char getCurrentTextClass(Buffer *);

InsetText::InsetText(Buffer * buf)
{
    par = new LyXParagraph();
    the_locking_inset = 0;
    buffer = buf;
    cursor_visible = false;
    maxWidth = old_x = -1;
    actpos = selection_start = selection_end = 0;
    interline_space = 1;
    no_selection = false;
    init_inset = true;
    maxAscent = maxDescent = insetWidth = widthOffset = 0;
    autoBreakRows = false;
    xpos = 0.0;
}


InsetText::InsetText(InsetText const & ins, Buffer * buf)
{
    par = new LyXParagraph(ins.par);
    the_locking_inset = 0;
    buffer = buf;
    cursor_visible = false;
    maxWidth = old_x = -1;
    actpos = selection_start = selection_end = 0;
    interline_space = 1;
    no_selection = false;
    init_inset = true;
    maxAscent = maxDescent = insetWidth = widthOffset = 0;
    autoBreakRows = false;
    xpos = 0.0;
}


InsetText::~InsetText()
{
    delete par;
}


Inset * InsetText::Clone() const
{
    InsetText * t = new InsetText(*this, buffer);
    return t;
}


void InsetText::Write(ostream & os) const
{
    os << "Text\n";
    WriteParagraphData(os);
}


void InsetText::WriteParagraphData(ostream & os) const
{
    par->writeFile(os, buffer->params, 0, 0);
}


void InsetText::Read(LyXLex & lex)
{
    string token, tmptok;
    int pos = 0;
    LyXParagraph * return_par = 0;
    char depth = 0; // signed or unsigned?
    LyXParagraph::footnote_flag footnoteflag = LyXParagraph::NO_FOOTNOTE;
    LyXParagraph::footnote_kind footnotekind = LyXParagraph::FOOTNOTE;
    LyXFont font(LyXFont::ALL_INHERIT);

    delete par;
    par = new LyXParagraph;
    
    while (lex.IsOK()) {
        lex.nextToken();
        token = lex.GetString();
        if (token.empty())
            continue;
	if (token == "\\end_inset")
	    break;
	if (buffer->parseSingleLyXformat2Token(lex, par, return_par,
					       token, pos, depth,
					       font, footnoteflag,
					       footnotekind)) {
	    // the_end read this should NEVER happen
	    lex.printError("\\the_end read in inset! Error in document!");
	    return;
	}
    }
    if (token != "\\end_inset") {
        lex.printError("Missing \\end_inset at this point. "
                       "Read: `$$Token'");
    }
    init_inset = true;
}


int InsetText::ascent(Painter & pain, LyXFont const & font) const
{
    if (init_inset) {
	computeTextRows(pain, xpos);
	init_inset = false;
    }
    if (maxAscent)
	return maxAscent;
    return font.maxAscent();
}


int InsetText::descent(Painter & pain, LyXFont const & font) const
{
    if (init_inset) {
	computeTextRows(pain, xpos);
	init_inset = false;
    }
    if (maxDescent)
	return maxDescent;
    return font.maxDescent();
}


int InsetText::width(Painter & pain, LyXFont const &) const
{
    if (init_inset) {
	computeTextRows(pain, xpos);
	init_inset = false;
    }
    return insetWidth;
}


void InsetText::draw(Painter & pain, LyXFont const & f,
		     int baseline, float & x) const
{
    xpos = x;
    computeTextRows(pain, x);
    UpdatableInset::draw(pain, f, baseline, x);
    
    bool do_reset_pos = (x != top_x) || (baseline != top_baseline);
    top_x = int(x);
    top_baseline = baseline;
    computeBaselines(baseline);
    for(RowList::size_type r = 0; r < rows.size() - 1; ++r) {
        drawRowSelection(pain, rows[r].pos, rows[r + 1].pos, r, 
                         rows[r].baseline, x);
        drawRowText(pain, rows[r].pos, rows[r + 1].pos, rows[r].baseline, x);
    }
    x += insetWidth;
    if (!the_locking_inset && do_reset_pos) {
//        HideInsetCursor(bv);
//        resetPos(bv);
//        ShowInsetCursor(bv);
    }
}


void InsetText::drawRowSelection(Painter & pain, int startpos, int endpos,
                                 int row, int baseline, float x) const
{
    if (!hasSelection())
	return;

    int s_start, s_end;
    if (selection_start > selection_end) {
	s_start = selection_end;
	s_end = selection_start;
    } else {
	s_start = selection_start;
	s_end = selection_end;
    }
    if ((s_start > endpos) || (s_end < startpos))
	return;
    
    int esel_x;
    int ssel_x = esel_x = int(x);
    LyXFont font;
    int p = startpos;
    for(; p < endpos; ++p) {
	if (p == s_start)
	    ssel_x = int(x);
	if ((p >= s_start) && (p <= s_end))
	    esel_x = int(x);
	char ch = par->GetChar(p);
	font = GetFont(par,p);
	if (IsFloatChar(ch)) {
	    // skip for now
	} else if (ch == LyXParagraph::META_INSET) {
	    Inset const * tmpinset = par->GetInset(p);
	    x += tmpinset->width(pain, font);
	} else {
	    x += pain.width(ch,font);
	}
    }
    if (p == s_start)
	ssel_x = int(x);
    if ((p >= s_start) && (p <= s_end))
	esel_x = int(x);
    if (ssel_x < esel_x) {
	pain.fillRectangle(int(ssel_x), baseline-rows[row].asc,
			   int(esel_x - ssel_x),
			   rows[row].asc + rows[row].desc,
			   LColor::selection);
    }
}


void InsetText::drawRowText(Painter & pain, int startpos, int endpos,
			    int baseline, float x) const
{
    Assert(endpos <= par->Last());

    for(int p = startpos; p < endpos; ++p) {
	char ch = par->GetChar(p);
	LyXFont font = GetFont(par,p);
	if (IsFloatChar(ch)) {
	    // skip for now
	} else if (par->IsNewline(p)) {
		// Draw end-of-line marker
		int wid = font.width('n');
		int asc = font.maxAscent();
		int y = baseline;
		int xp[3], yp[3];
		
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
		x += wid;
	} else if (ch == LyXParagraph::META_INSET) {
	    Inset * tmpinset = par->GetInset(p);
	    if (tmpinset) 
		tmpinset->draw(pain, font, baseline, x);
	} else {
	    pain.text(int(x), baseline, ch, font);
	    x += pain.width(ch,font);
	}
    }
}


char const * InsetText::EditMessage() const
{
    return _("Opened Text Inset");
}


void InsetText::Edit(BufferView * bv, int x, int y, unsigned int button)
{
    UpdatableInset::Edit(bv, x, y, button);

    bv->lockInset(this);
    the_locking_inset = 0;
    inset_pos = inset_x = inset_y = 0;
    no_selection = true;
    setPos(bv, x,y);
    selection_start = selection_end = actpos;
    current_font = real_current_font = GetFont(par, actpos);
}


void InsetText::InsetUnlock(BufferView * bv)
{
    if (the_locking_inset)
	the_locking_inset->InsetUnlock(bv);
    HideInsetCursor(bv);
    if (hasSelection()) {
	selection_start = selection_end = actpos;
	UpdateLocal(bv, false);
    }
    the_locking_inset = 0;
    no_selection = false;
}


bool InsetText::UnlockInsetInInset(BufferView * bv, Inset * inset, bool lr)
{
    if (!the_locking_inset)
        return false;
    if (the_locking_inset == inset) {
        the_locking_inset->InsetUnlock(bv);
        the_locking_inset = 0;
        if (lr)
            moveRight(bv, false);
        return true;
    }
    return the_locking_inset->UnlockInsetInInset(bv, inset,lr);
}


bool InsetText::UpdateInsetInInset(BufferView * bv, Inset * inset)
{
    if (!the_locking_inset)
        return false;
    if (the_locking_inset != inset)
        return the_locking_inset->UpdateInsetInInset(bv, inset);
    float x = inset_x;
    inset->draw(bv->getPainter(), real_current_font, inset_y, x);
    UpdateLocal(bv, true);
    return true;
}


void InsetText::InsetButtonRelease(BufferView * bv, int x, int y, int button)
{
    if (the_locking_inset) {
        the_locking_inset->InsetButtonRelease(bv, x-inset_x, y-inset_y,button);
        return;
    }
    no_selection = false;
}


void InsetText::InsetButtonPress(BufferView * bv, int x, int y, int button)
{
    if (hasSelection()) {
	selection_start = selection_end = actpos;
	UpdateLocal(bv, false);
    }
    no_selection = false;
    if (the_locking_inset) {
	setPos(bv, x, y, false);
	UpdatableInset
	    *inset = 0;
	if (par->GetChar(actpos) == LyXParagraph::META_INSET)
	    inset = static_cast<UpdatableInset*>(par->GetInset(actpos));
	if (the_locking_inset == inset) {
	    the_locking_inset->InsetButtonPress(bv,x-inset_x,y-inset_y,button);
	    return;
	} else if (inset) {
	    // otherwise unlock the_locking_inset and lock the new inset
	    inset_x = cx-top_x;
	    inset_y = cy;
	    inset_pos = actpos;
	    the_locking_inset->InsetUnlock(bv);
	    the_locking_inset = inset;
	    the_locking_inset->Edit(bv, x - inset_x, y - inset_y, button);
	    return;
	}
	// otherwise only unlock the_locking_inset
	the_locking_inset->InsetUnlock(bv);
    }
    HideInsetCursor(bv);
    the_locking_inset = 0;
    setPos(bv, x, y);
    selection_start = selection_end = actpos;
    if (!the_locking_inset)
	ShowInsetCursor(bv);
}


void InsetText::InsetMotionNotify(BufferView * bv, int x, int y, int button)
{
    if (the_locking_inset) {
        the_locking_inset->InsetMotionNotify(bv, x - inset_x,
					     y - inset_y,button);
        return;
    }
    if (!no_selection) {
	int old = selection_end;
	setPos(bv, x, y, false);
	selection_end = actpos;
	if (old != selection_end)
	    UpdateLocal(bv, false);
    }
    no_selection = false;
}


void InsetText::InsetKeyPress(XKeyEvent * xke)
{
    if (the_locking_inset) {
        the_locking_inset->InsetKeyPress(xke);
        return;
    }
}


UpdatableInset::RESULT
InsetText::LocalDispatch(BufferView * bv,
			 int action, string const & arg)
{
    no_selection = false;
    if (UpdatableInset::LocalDispatch(bv, action, arg)) {
	resetPos(bv);
	return DISPATCHED;
    }

    UpdatableInset::RESULT
        result=DISPATCHED;

    if ((action < 0) && arg.empty())
        return FINISHED;

    if ((action != LFUN_DOWN) && (action != LFUN_UP) &&
        (action != LFUN_DOWNSEL) && (action != LFUN_UPSEL))
        old_x = -1;
    if (the_locking_inset) {
        result = the_locking_inset->LocalDispatch(bv, action, arg);
	if (result == DISPATCHED) {
	    the_locking_inset->ToggleInsetCursor(bv);
	    UpdateLocal(bv, false);
	    the_locking_inset->ToggleInsetCursor(bv);
            return result;
        } else if (result == FINISHED) {
	    if ((action == LFUN_RIGHT) || (action == -1)) {
		actpos = inset_pos + 1;
		resetPos(bv);
	    }
	    the_locking_inset = 0;
	    return DISPATCHED;
	}
    }
    HideInsetCursor(bv);
    switch (action) {
	// Normal chars
    case -1:
	par->InsertChar(actpos,arg[0]);
	par->SetFont(actpos,real_current_font);
	UpdateLocal(bv, true);
	++actpos;
	selection_start = selection_end = actpos;
	resetPos(bv);
	break;
        // --- Cursor Movements ---------------------------------------------
    case LFUN_RIGHTSEL:
	moveRight(bv, false);
	selection_end = actpos;
	UpdateLocal(bv, false);
	break;
    case LFUN_RIGHT:
	result= DISPATCH_RESULT(moveRight(bv));
	if (hasSelection()) {
	    selection_start = selection_end = actpos;
	    UpdateLocal(bv, false);
	} else {
	    selection_start = selection_end = actpos;
	}
	break;
    case LFUN_LEFTSEL:
	moveLeft(bv, false);
	selection_end = actpos;
	UpdateLocal(bv, false);
	break;
    case LFUN_LEFT:
          result= DISPATCH_RESULT(moveLeft(bv));
	  if (hasSelection()) {
	      selection_start = selection_end = actpos;
	      UpdateLocal(bv, false);
	  } else {
	      selection_start = selection_end = actpos;
	  }
          break;
    case LFUN_DOWNSEL:
	moveDown(bv, false);
	selection_end = actpos;
	UpdateLocal(bv, false);
	break;
    case LFUN_DOWN:
	result= DISPATCH_RESULT(moveDown(bv));
	if (hasSelection()) {
	    selection_start = selection_end = actpos;
	    UpdateLocal(bv, false);
	} else {
	    selection_start = selection_end = actpos;
	}
	break;
    case LFUN_UPSEL:
	moveUp(bv, false);
	selection_end = actpos;
	UpdateLocal(bv, false);
	break;
    case LFUN_UP:
	result= DISPATCH_RESULT(moveUp(bv));
	if (hasSelection()) {
	    selection_start = selection_end = actpos;
	    UpdateLocal(bv, false);
	} else {
	    selection_start = selection_end = actpos;
	}
	break;
    case LFUN_BACKSPACE:
	if (!actpos || par->IsNewline(actpos-1)) {
	    if (hasSelection()) {
		selection_start = selection_end = actpos;
		UpdateLocal(bv, false);
	    }
	    break;
	}
	moveLeft(bv);
    case LFUN_DELETE:
	if (Delete()) { // we need update
	    selection_start = selection_end = actpos;
	    UpdateLocal(bv, true);
	} else if (hasSelection()) {
	    selection_start = selection_end = actpos;
	    UpdateLocal(bv, false);
	}
	break;
    case LFUN_HOME:
	for(; actpos > rows[actrow].pos; --actpos)
	    cx -= SingleWidth(bv->getPainter(), par, actpos);
	cx -= SingleWidth(bv->getPainter(), par, actpos);
	if (hasSelection()) {
	    selection_start = selection_end = actpos;
	    UpdateLocal(bv, false);
	} else {
	    selection_start = selection_end = actpos;
	}
	break;
    case LFUN_END:
    {
	int checkpos = (int)rows[actrow + 1].pos;
	if ((actrow + 2) < (int)rows.size())
	    --checkpos;
	for(; actpos < checkpos; ++actpos)
	    cx += SingleWidth(bv->getPainter(), par, actpos);
	if (hasSelection()) {
	    selection_start = selection_end = actpos;
	    UpdateLocal(bv, false);
	} else {
	    selection_start = selection_end = actpos;
	}
    }
    break;
    case LFUN_MATH_MODE:   // Open or create a math inset
	InsertInset(bv, new InsetFormula);
	if (hasSelection()) {
	    selection_start = selection_end = actpos;
	    UpdateLocal(bv, false);
	} else {
	    selection_start = selection_end = actpos;
	}
	return DISPATCHED;
    case LFUN_BREAKLINE:
	par->InsertChar(actpos,LyXParagraph::META_NEWLINE);
	par->SetFont(actpos,real_current_font);
	UpdateLocal(bv, true);
	++actpos;
	selection_start = selection_end = actpos;
	resetPos(bv);
	break;
    default:
	result = UNDISPATCHED;
	break;
    }
    if (result != FINISHED) {
	if (!the_locking_inset)
	    ShowInsetCursor(bv);
    } else
        bv->unlockInset(this);
    return result;
}


int InsetText::Latex(ostream & os, signed char /*fragile*/,
#ifdef USE_OSTREAM_ONLY
		     bool) const
#else
		     bool free_spc) const
#endif
{
#ifdef USE_OSTREAM_ONLY
	TexRow texrow;
	int ret = par->SimpleTeXOnePar(os, texrow);
	return ret;
#else
    string fstr;

    int i = Latex(fstr, fragile, free_spc);
    os << fstr;
    return i;
#endif
}


#ifndef USE_OSTREAM_ONLY
int InsetText::Latex(string & file, signed char /* fragile */) const
{
    TexRow texrow;

    return par->SimpleTeXOnePar(file, texrow);
}
#endif


void InsetText::Validate(LaTeXFeatures & features) const
{
    par->validate(features);
}


// Returns the width of a character at a certain spot
int InsetText::SingleWidth(Painter & pain, LyXParagraph * par, int pos) const
{
    LyXFont font = GetFont(par, pos);
    char c = par->GetChar(pos);

    if (IsPrintable(c)) {
        return font.width(c);
    } else if (c == LyXParagraph::META_INSET) {
        Inset const * tmpinset = par->GetInset(pos);
        if (tmpinset)
            return tmpinset->width(pain, font);
        else
            return 0;
    } else if (IsSeparatorChar(c))
        c = ' ';
    else if (IsNewlineChar(c))
        c = 'n';
    return font.width(c);
}


// Returns the width of a character at a certain spot
void InsetText::SingleHeight(Painter & pain, LyXParagraph * par,int pos,
			     int & asc, int & desc) const
{
    LyXFont font = GetFont(par, pos);
    char c = par->GetChar(pos);

    asc = desc = 0;
    if (c == LyXParagraph::META_INSET) {
        Inset const * tmpinset=par->GetInset(pos);
        if (tmpinset) {
	    asc = tmpinset->ascent(pain, font);
	    desc = tmpinset->descent(pain, font);
        }
    } else {
        asc = font.maxAscent();
        desc = font.maxDescent();
    }
    return;
}


// Gets the fully instantiated font at a given position in a paragraph
// Basically the same routine as LyXParagraph::getFont() in paragraph.C.
// The difference is that this one is used for displaying, and thus we
// are allowed to make cosmetic improvements. For instance make footnotes
// smaller. (Asger)
// If position is -1, we get the layout font of the paragraph.
// If position is -2, we get the font of the manual label of the paragraph.
LyXFont InsetText::GetFont(LyXParagraph * par, int pos) const
{
    char par_depth = par->GetDepth();

    LyXLayout const & layout =
	    textclasslist.Style(buffer->params.textclass, par->GetLayout());

    // We specialize the 95% common case:
    if (par->footnoteflag == LyXParagraph::NO_FOOTNOTE && !par_depth) {
        if (pos >= 0) {
            // 95% goes here
            if (layout.labeltype == LABEL_MANUAL
                && pos < BeginningOfMainBody(par)) {
		// 1% goes here
                return par->GetFontSettings(pos).realize(layout.reslabelfont);
            } else
                return par->GetFontSettings(pos).realize(layout.resfont);
        } else {
            // 5% goes here.
            // process layoutfont for pos == -1 and labelfont for pos < -1
            if (pos == -1)
                return layout.resfont;
            else
                return layout.reslabelfont;
        }
    }
    // The uncommon case need not be optimized as much

    LyXFont layoutfont, tmpfont;

    if (pos >= 0){
        // 95% goes here
        if (pos < BeginningOfMainBody(par)) {
            // 1% goes here
            layoutfont = layout.labelfont;
        } else {
            // 99% goes here
            layoutfont = layout.font;
        }
        tmpfont = par->GetFontSettings(pos);
        tmpfont.realize(layoutfont);
    } else{
        // 5% goes here.
        // process layoutfont for pos == -1 and labelfont for pos < -1
        if (pos == -1)
            tmpfont = layout.font;
        else
            tmpfont = layout.labelfont;
    }
    
    // Resolve against environment font information
    //if (par->GetDepth()){ // already in while condition
    while (par && par_depth && !tmpfont.resolved()) {
        par = par->DepthHook(par_depth - 1);
        if (par) {
            tmpfont.realize(textclasslist.Style(buffer->params.textclass,
                                                par->GetLayout()).font);
            par_depth = par->GetDepth();
        }
    }
    tmpfont.realize((textclasslist.TextClass(buffer->params.textclass).
                    defaultfont()));
    return tmpfont;
}


int InsetText::BeginningOfMainBody(LyXParagraph * par) const
{
    if (textclasslist.Style(buffer->params.textclass,
                       par->GetLayout()).labeltype != LABEL_MANUAL)
        return 0;
    else
        return par->BeginningOfMainBody();
}


void InsetText::GetCursorPos(int & x, int & y) const
{
    x = cx;
    y = cy;
}


int InsetText::InsetInInsetY()
{
    if (!the_locking_inset)
	return 0;

    int y = inset_y;
    return (y + the_locking_inset->InsetInInsetY());
}


void InsetText::ToggleInsetCursor(BufferView * bv)
{
    if (the_locking_inset) {
        the_locking_inset->ToggleInsetCursor(bv);
        return;
    }

    LyXFont font = GetFont(par, actpos);

    int asc = font.maxAscent();
    int desc = font.maxDescent();
  
    if (cursor_visible)
        bv->hideLockedInsetCursor();
    else
        bv->showLockedInsetCursor(cx, cy, asc, desc);
    cursor_visible = !cursor_visible;
}


void InsetText::ShowInsetCursor(BufferView * bv)
{
    if (!cursor_visible) {
	LyXFont font = GetFont(par, actpos);
	
	int asc = font.maxAscent();
	int desc = font.maxDescent();
	bv->fitLockedInsetCursor(cx, cy, asc, desc);
	bv->showLockedInsetCursor(cx, cy, asc, desc);
	cursor_visible = true;
    }
}


void InsetText::HideInsetCursor(BufferView * bv)
{
    if (cursor_visible)
        ToggleInsetCursor(bv);
}


void InsetText::setPos(BufferView * bv, int x, int y, bool activate_inset)
{
    int ox = x;
    int oy = y;
	
    // search right X-pos x==0 -> top_x
    actpos = actrow = 0;
    cy = top_baseline;
    y += cy;
    for(unsigned int i = 1;
	((cy + rows[i - 1].desc) < y) && (i < rows.size() - 1); ++i) {
	cy = rows[i].baseline;
	actpos = rows[i].pos;
	actrow = i;
    }
    cy -= top_baseline;
    cx = top_x;
    x += top_x;

    int swh;
    int sw;
    int checkpos;

    sw = swh = SingleWidth(bv->getPainter(), par,actpos);
    if (par->GetChar(actpos)!=LyXParagraph::META_INSET)
	swh /= 2;
    checkpos = rows[actrow + 1].pos;
    if ((actrow+2) < (int)rows.size())
	--checkpos;
    while ((actpos < checkpos) && ((cx + swh) < x)) {
	cx += sw;
	++actpos;
	sw = swh = SingleWidth(bv->getPainter(), par,actpos);
	if (par->GetChar(actpos)!=LyXParagraph::META_INSET)
	    swh /= 2;
    }
    if (activate_inset && par->GetChar(actpos)==LyXParagraph::META_INSET) {
        the_locking_inset =
		static_cast<UpdatableInset*>(par->GetInset(actpos));
	inset_x = cx - top_x;
	inset_y = cy;
	inset_pos = actpos;
        the_locking_inset->Edit(bv, ox - inset_x, oy - inset_y, 0);
    }
}


bool InsetText::moveRight(BufferView * bv, bool activate_inset)
{
    if (actpos >= par->Last())
	return false;
    if (activate_inset && par->GetChar(actpos)==LyXParagraph::META_INSET) {
	the_locking_inset =
		static_cast<UpdatableInset*>(par->GetInset(actpos));
	inset_x = cx - top_x;
	inset_y = cy;
	inset_pos = actpos;
	the_locking_inset->Edit(bv, 0, 0, 0);
    } else {
	++actpos;
	resetPos(bv);
    }
    return true;
}


bool InsetText::moveLeft(BufferView * bv, bool activate_inset)
{
    if (actpos <= 0)
	return false;
    --actpos;
    if (activate_inset && par->GetChar(actpos)==LyXParagraph::META_INSET) {
	the_locking_inset =
		static_cast<UpdatableInset*>(par->GetInset(actpos));
	resetPos(bv);
	inset_x = cx - top_x;
	inset_y = cy;
	inset_pos = actpos;
	the_locking_inset->Edit(bv, the_locking_inset->
				width(bv->getPainter(), GetFont(par,actpos)),
				0, 0);
    } else {
	resetPos(bv);
    }
    return true;
}


bool InsetText::moveUp(BufferView * bv, bool activate_inset)
{
    if (!actrow)
	return false;
    cy = rows[actrow - 1].baseline - top_baseline;
    setPos(bv, cx - top_x, cy, activate_inset);
    return true;
}


bool InsetText::moveDown(BufferView * bv, bool activate_inset)
{
    if (actrow >= int(rows.size() - 2))
	return false;
    cy = rows[actrow + 1].baseline - top_baseline;
    setPos(bv, cx - top_x, cy, activate_inset);
    return true;
}


void InsetText::resetPos(BufferView * bv)
{
    int old_pos = actpos;

    cy = top_baseline;
    actrow = 0;
    for(unsigned int i = 0; (i < (rows.size()-1)) && (rows[i].pos <= actpos);
	++i) {
	cy = rows[i].baseline;
	actrow = i;
    }
    cy -= top_baseline;
    setPos(bv, 0, cy, false);
    cx = top_x;
    while(actpos < old_pos) {
	cx += SingleWidth(bv->getPainter(), par,actpos);
	++actpos;
    }
}


bool InsetText::Delete()
{
    /* some insets are undeletable here */
    if (par->GetChar(actpos)==LyXParagraph::META_INSET) {
        /* force complete redo when erasing display insets */ 
        /* this is a cruel mathod but save..... Matthias */ 
        if (par->GetInset(actpos)->Deletable() &&
            par->GetInset(actpos)->display()) {
            par->Erase(actpos);
            return true;
        }
        return false;
    }
    par->Erase(actpos);
    return true;
}


bool InsetText::InsertInset(BufferView * bv, Inset * inset)
{
    if (inset->Editable() == Inset::IS_EDITABLE) {
	UpdatableInset *i = (UpdatableInset *)inset;
	i->setOwner((UpdatableInset *)this);
    }
    par->InsertChar(actpos, LyXParagraph::META_INSET);
    par->InsertInset(actpos, inset);
    UpdateLocal(bv, true);
    the_locking_inset = static_cast<UpdatableInset*>(inset);
    inset_x = cx - top_x;
    inset_y = cy;
    inset_pos = actpos;
    inset->Edit(bv, 0, 0, 0);
    return true;
}


UpdatableInset * InsetText::GetLockingInset()
{
    return the_locking_inset ? the_locking_inset->GetLockingInset() : this;
}


void InsetText::SetFont(BufferView * bv, LyXFont const & font, bool toggleall)
{
    // if there is no selection just set the current_font
    if (!hasSelection()) {
	// Determine basis font
	LyXFont layoutfont;
	if (actpos < BeginningOfMainBody(par))
	    layoutfont = GetFont(par, -2);
	else
	    layoutfont = GetFont(par, -1);
	
	// Update current font
	real_current_font.update(font, toggleall);
	
	// Reduce to implicit settings
	current_font = real_current_font;
	current_font.reduce(layoutfont);
	// And resolve it completely
	real_current_font.realize(layoutfont);
	return;
    }
    
    int s_start, s_end;
    if (selection_start > selection_end) {
	s_start = selection_end;
	s_end = selection_start;
    } else {
	s_start = selection_start;
	s_end = selection_end;
    }
    LyXFont newfont;
    while(s_start < s_end) {
	newfont = GetFont(par,s_start);
	newfont.update(font, toggleall);
	SetCharFont(s_start, newfont);
	++s_start;
    }
    UpdateLocal(bv, true);
}


void InsetText::SetCharFont(int pos, LyXFont const & f)
{
    /* let the insets convert their font */
	LyXFont font(f);
	
    if (par->GetChar(pos) == LyXParagraph::META_INSET) {
	if (par->GetInset(pos))
	    font = par->GetInset(pos)->ConvertFont(font);
    }
    LyXLayout const & layout =
	    textclasslist.Style(buffer->params.textclass,par->GetLayout());

    // Get concrete layout font to reduce against
    LyXFont layoutfont;

    if (pos < BeginningOfMainBody(par))
	layoutfont = layout.labelfont;
    else
	layoutfont = layout.font;


    layoutfont.realize((textclasslist.TextClass(buffer->params.textclass).
		       defaultfont()));

    // Now, reduce font against full layout font
    font.reduce(layoutfont);

    par->SetFont(pos, font);
}


void InsetText::computeTextRows(Painter & pain, float x) const
{
    int p,
	nwp = 0,
	asc = 0,
	desc = 0,
	oasc = 0,
	odesc = 0,
	owidth = 0,
	wordAscent,
	wordDescent;
    row_struct row;

    if (rows.size())
	    rows.clear();
    int width = wordAscent = wordDescent = 0;
    insetWidth = maxAscent = maxDescent = 0;
    row.asc      = 0;
    row.desc     = 0;
    row.pos      = 0;
    row.baseline = 0;
    rows.push_back(row);
    if (!autoBreakRows) {
	for(p = 0; p < par->Last(); ++p) {
	    insetWidth += SingleWidth(pain, par, p);
	    SingleHeight(pain, par, p, asc, desc);
	    maxAscent = max(maxAscent, asc);
	    maxDescent = max(maxDescent, desc);
	}
	rows[0].asc = maxAscent;
	rows[0].desc = maxDescent;
	// alocate a dummy row for the endpos
	row.pos = par->Last();
	rows.push_back(row);
	return;
    }

    bool is_first_word_in_row = true;

    int cw, lastWordWidth = 0;

    maxWidth = getMaxWidth(pain) - widthOffset;
    for(p = 0; p < par->Last(); ++p) {
	cw = SingleWidth(pain, par, p);
	width += cw;
	lastWordWidth += cw;
	SingleHeight(pain, par, p, asc, desc);
	wordAscent = max(wordAscent, asc);
	wordDescent = max(wordDescent, desc);
	if (par->IsNewline(p)) {
	    rows.back().asc = wordAscent;
	    rows.back().desc = wordDescent;
	    row.pos = p+1;
	    rows.push_back(row);
	    SingleHeight(pain, par, p, oasc, odesc);
	    width = lastWordWidth = 0;
	    is_first_word_in_row = true;
	    wordAscent = wordDescent = 0;
	    continue;
	}
	Inset const * inset = 0;
	if (((p + 1) < par->Last()) &&
	    (par->GetChar(p + 1)==LyXParagraph::META_INSET))
	    inset = par->GetInset(p + 1);
	if (inset && inset->display()) {
	    if (!is_first_word_in_row && (width >= (maxWidth - x))) {
		// we have to split also the row above
		rows.back().asc = oasc;
		rows.back().desc = odesc;
		row.pos = nwp;
		rows.push_back(row);
		oasc = wordAscent;
		odesc = wordDescent;
		insetWidth = max(insetWidth, owidth);
		width = lastWordWidth;
		lastWordWidth = 0;
	    } else {
		    oasc = max(oasc, wordAscent);
		    odesc = max(odesc, wordDescent);
	    }
	    rows.back().asc = oasc;
	    rows.back().desc = odesc;
	    row.pos = ++p;
	    rows.push_back(row);
	    SingleHeight(pain, par, p, asc, desc);
	    rows.back().asc = asc;
	    rows.back().desc = desc;
	    row.pos = nwp = p + 1;
	    rows.push_back(row);
	    oasc = odesc = width = lastWordWidth = 0;
	    is_first_word_in_row = true;
	    wordAscent = wordDescent = 0;
	    continue;
	} else if (par->IsSeparator(p)) {
	    if (width >= maxWidth - x) {
		if (is_first_word_in_row) {
		    rows.back().asc = wordAscent;
		    rows.back().desc = wordDescent;
		    row.pos = p + 1;
		    rows.push_back(row);
		    oasc = odesc = width = 0;
		} else {
		    rows.back().asc = oasc;
		    rows.back().desc = odesc;
		    row.pos = nwp;
		    rows.push_back(row);
		    oasc = wordAscent;
		    odesc = wordDescent;
		    insetWidth = max(insetWidth, owidth);
		    width = lastWordWidth;
		}
		wordAscent = wordDescent = lastWordWidth = 0;
		nwp = p + 1;
		continue;
	    }
	    owidth = width;
	    oasc = max(oasc, wordAscent);
	    odesc = max(odesc, wordDescent);
	    wordAscent = wordDescent = lastWordWidth = 0;
	    nwp = p + 1;
	    is_first_word_in_row = false;
	}
    }
    // if we have some data in the paragraph we have ascent/descent
    if (p) {
	if (width >= (maxWidth - x)) {
	    // assign upper row
	    rows.back().asc = oasc;
	    rows.back().desc = odesc;
	    // assign and allocate lower row
	    row.pos = nwp;
	    rows.push_back(row);
	    rows.back().asc = wordAscent;
	    rows.back().desc = wordDescent;
	    width -= lastWordWidth;
	} else {
	    // assign last row data
//	    width = lastWordWidth;
//	    lastWordWidth = 0;
	    rows.back().asc = max(oasc, wordAscent);
	    rows.back().desc = max(odesc, wordDescent);
	}
    }
    insetWidth = max(insetWidth, width);
    // alocate a dummy row for the endpos
    row.pos = par->Last();
    rows.push_back(row);
    // calculate maxAscent/Descent
    maxAscent = rows[0].asc;
    maxDescent = rows[0].desc;
    for (RowList::size_type i = 1; i < rows.size() - 1; ++i) {
	maxDescent += rows[i].asc + rows[i].desc + interline_space;
    }
#if 0
    if (the_locking_inset) {
	computeBaselines(top_baseline);
	actpos = inset_pos;
	resetPos(bv);
	inset_x = cx - top_x;
	inset_y = cy;
    }
#endif
}


void InsetText::computeBaselines(int baseline) const
{
    rows[0].baseline = baseline;
    for (unsigned int i = 1; i < rows.size() - 1; i++) {
	rows[i].baseline = rows[i - 1].baseline + rows[i - 1].desc + 
	    rows[i].asc + interline_space;
    }
}

void InsetText::UpdateLocal(BufferView *bv, bool flag)
{
    init_inset = flag;
    bv->updateInset(this, flag);
}
