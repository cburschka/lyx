/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 1998 The LyX Team.
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetert.h"
#include "gettext.h"
#include "lyxfont.h"
#include "Painter.h"
#include "lyx_gui_misc.h"


InsetERT::InsetERT(Buffer * bf)
	: InsetText(bf)
{
	closed = true;
	nomotion = false;
}


Inset * InsetERT::Clone() const
{
    InsetERT * result = new InsetERT(buffer);
    return result;
}


void InsetERT::Write(ostream & os) const
{
    os << "ERT\n";
    WriteParagraphData(os);
}


int InsetERT::ascent_closed(Painter & pain, LyXFont const & f) const
{
	int width, ascent, descent;
	LyXFont font(LyXFont::ALL_SANE);
	font.setSize(f.size());
	font.decSize();
	font.decSize();
	pain.buttonText(0, 0, _("ERT"), font, false, width, ascent, descent);
	return ascent;
}


int InsetERT::descent_closed(Painter & pain, LyXFont const & f) const
{
	int width, ascent, descent;
	LyXFont font(LyXFont::ALL_SANE);
	font.setSize(f.size());
	font.decSize();
	font.decSize();
	pain.buttonText(0, 0, _("ERT"), font, false, width, ascent, descent);
	return descent;
}


int InsetERT::width_closed(Painter & pain, LyXFont const & f) const
{
	int width, ascent, descent;
	LyXFont font(LyXFont::ALL_SANE);
	font.setSize(f.size());
	font.decSize();
	font.decSize();
	pain.buttonText(TEXT_TO_INSET_OFFSET, 0, _("ERT"), font, false,
			width, ascent, descent);
	return width + (2*TEXT_TO_INSET_OFFSET);
}


int InsetERT::ascent(Painter & pain, LyXFont const & font) const
{
    if (closed) 
	return ascent_closed(pain, font);
    else 
	return InsetText::ascent(pain, font);
}


int InsetERT::descent(Painter & pain, LyXFont const & font) const
{

    if (closed) 
	return descent_closed(pain, font);
    else 
	return InsetText::descent(pain, font);
}


int InsetERT::width(Painter & pain, LyXFont const & font) const
{
    if (closed) 
	return width_closed(pain, font);
    else 
	return InsetText::width(pain, font);
}


void InsetERT::draw_closed(Painter & pain, LyXFont const & f,
			   int baseline, float & x) const
{
	LyXFont font(LyXFont::ALL_SANE);
	font.setSize(f.size());
	font.decSize();
	font.decSize();
	font.setColor(LColor::ert);
	int width;
	pain.buttonText(int(x) + TEXT_TO_INSET_OFFSET, baseline,
			_("ERT"), font, true, width);
	x += width + (2 * TEXT_TO_INSET_OFFSET);
}


void InsetERT::draw(Painter & pain, LyXFont const & f,
		    int baseline, float & x) const
{
    if (closed) {
	top_x = int(x);
	top_baseline = baseline;
	draw_closed(pain, f, baseline, x);
    } else {
	InsetText::draw(pain, f, baseline, x);
    }
//    resetPos(bv);
}


void InsetERT::InsetButtonRelease(BufferView * bv, int x, int y, int button)
{
    nomotion = false;
    InsetText::InsetButtonRelease(bv, x, y, button);
}


void InsetERT::InsetButtonPress(BufferView * bv, int x, int y, int button)
{
    nomotion = false;
    InsetText::InsetButtonPress(bv, x, y, button);
}


void InsetERT::InsetMotionNotify(BufferView * bv, int x, int y, int button)
{
    if (nomotion)
	return;
    InsetText::InsetMotionNotify(bv, x, y, button);
}


void InsetERT::Edit(BufferView * bv, int x, int y, unsigned int button)
{
    closed = false;
    nomotion = true;
    LyXFont font(LyXFont::ALL_SANE);
    font.setLatex (LyXFont::ON);
    InsetText::Edit(bv, (x > (width_closed(bv->getPainter(),font)-5) ?
		     width(bv->getPainter(), font) : 0), y, button);
    real_current_font = current_font = font;
    bv->updateInset(this, false);
}


void InsetERT::InsetUnlock(BufferView * bv)
{
    closed = true;
    InsetText::InsetUnlock(bv);
    bv->updateInset(this, false);
}


bool InsetERT::InsertInset(Inset *)
{
    return false;
}


void InsetERT::SetFont(LyXFont const &, bool)
{
    WriteAlert(_("Impossible Operation!"),
	       _("Not permitted to change font-types inside ERT-insets!"),
	       _("Sorry."));
}
