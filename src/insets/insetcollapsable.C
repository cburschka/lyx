/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright (C) 1998 The LyX Team.
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetcollapsable.h"
#include "gettext.h"
#include "lyxfont.h"
#include "BufferView.h"
#include "Painter.h"


InsetCollapsable::InsetCollapsable(Buffer * bf)
		: InsetText(bf)
{
    collapsed = true;
    label = "Label";
    autocolapse = true;
    autoBreakRows = true;
    framecolor = LColor::footnoteframe;
    widthOffset = 10;
    button_x = button_top_y = button_bottom_y = top_x = -1;
}


Inset * InsetCollapsable::Clone() const
{
    InsetCollapsable * result = new InsetCollapsable(buffer);
    result->init(buffer, par);

    result->collapsed = collapsed;
    return result;
}

int InsetCollapsable::ascent_collapsed(Painter & pain, LyXFont const &) const
{
    int width = 0, ascent = 0, descent = 0;
    pain.buttonText(0, 0, label.c_str(), labelfont, false, 
		    width, ascent, descent);
    return ascent;
}


int InsetCollapsable::descent_collapsed(Painter & pain, LyXFont const &) const
{
    int width = 0, ascent = 0, descent = 0;
    pain.buttonText(0, 0, label.c_str(), labelfont, false, 
		    width, ascent, descent);
    return descent;
}


int InsetCollapsable::width_collapsed(Painter & pain, LyXFont const &) const
{
    int width, ascent, descent;
    pain.buttonText(TEXT_TO_INSET_OFFSET, 0, label.c_str(), labelfont, false,
		    width, ascent, descent);
    return width + (2*TEXT_TO_INSET_OFFSET);
}


int InsetCollapsable::ascent(Painter & pain, LyXFont const & font) const
{
    if (collapsed) 
	return ascent_collapsed(pain, font);
    else 
	return InsetText::ascent(pain, font) + TEXT_TO_TOP_OFFSET;
}


int InsetCollapsable::descent(Painter & pain, LyXFont const & font) const
{
    if (collapsed) 
	return descent_collapsed(pain, font);
    else 
	return InsetText::descent(pain, font) + TEXT_TO_BOTTOM_OFFSET;
}


int InsetCollapsable::width(Painter & pain, LyXFont const & font) const
{
    if (collapsed) 
	return width_collapsed(pain, font);

    return getMaxWidth(pain) - widthOffset + 2;
}


void InsetCollapsable::draw_collapsed(Painter & pain, LyXFont const &,
				      int baseline, float & x) const
{
    int width = 0;
    pain.buttonText(int(x) + TEXT_TO_INSET_OFFSET, baseline,
		    label.c_str(), labelfont, true, width);
    x += width + (2 * TEXT_TO_INSET_OFFSET);
}


void InsetCollapsable::draw(Painter & pain, LyXFont const & f, 
			    int baseline, float & x) const
{
    if (collapsed) {
	draw_collapsed(pain, f, baseline, x);
	return;
    }
    top_x = int(x);
    top_baseline = baseline;
    draw_collapsed(pain, f, baseline, x);
    button_x = int(x);
    button_top_y = -ascent_collapsed(pain, f);
    button_bottom_y = descent_collapsed(pain, f);
    
    maxWidth = getMaxWidth(pain) - button_x;
    x += 2;
    int
	w = maxWidth - widthOffset,
	h = ascent(pain,f) + descent(pain,f);
    
    pain.rectangle(int(x), baseline - ascent(pain, f), w, h, framecolor);

    x += 4;
    top_x = int(x - top_x);
    InsetText::draw(pain, f, baseline, x);
}


void InsetCollapsable::Edit(BufferView *bv, int x, int y, unsigned int button)
{
    if (collapsed) {
	collapsed = false;
	UpdateLocal(bv, true);
	InsetText::Edit(bv, 0, 0, button);
    } else if (button && (x < button_x)) {
	return;
    } else {
	InsetText::Edit(bv, x-top_x, y, button);
    }
}


Inset::EDITABLE InsetCollapsable::Editable() const
{
	if (collapsed)
		return IS_EDITABLE;
	return HIGHLY_EDITABLE;
}

void InsetCollapsable::InsetUnlock(BufferView *bv)
{
    if (autocolapse) {
	collapsed = true;
    }
    InsetText::InsetUnlock(bv);
    UpdateLocal(bv, true);
}


void InsetCollapsable::UpdateLocal(BufferView *bv, bool flag)
{
    maxWidth = getMaxWidth(bv->getPainter()) -
	width_collapsed(bv->getPainter(), labelfont);
    InsetText::UpdateLocal(bv, flag);
}


void InsetCollapsable::InsetButtonPress(BufferView *bv,int x,int y,int button)
{
    if ((x >= button_x) && (y >= button_top_y)) {
	InsetText::InsetButtonPress(bv, x-top_x, y, button);
    }
}


void InsetCollapsable::InsetButtonRelease(BufferView *bv, int x, int y, int button)
{
    if ((x < button_x)  && (y >= button_top_y) && (y <= button_bottom_y)) {
	collapsed = true;
	UpdateLocal(bv, false);
        bv->unlockInset(this);
    } else if ((x >= button_x) && (y >= button_top_y)) {
	InsetText::InsetButtonRelease(bv, x-top_x, y, button);
    }
}


void InsetCollapsable::InsetMotionNotify(BufferView *bv, int x, int y, int button)
{
    if ((x >= button_x) && (y >= button_top_y)) {
	InsetText::InsetMotionNotify(bv, x-top_x, y, button);
    }
}
