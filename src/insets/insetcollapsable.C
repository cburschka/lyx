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
#include "support/LOstream.h"

using std::ostream;

InsetCollapsable::InsetCollapsable(Buffer * bf)
		: InsetText(bf)
{
    collapsed = true;
    label = "Label";
    autocollapse = true;
    autoBreakRows = true;
    framecolor = LColor::footnoteframe;
    widthOffset = 10;
    button_length = button_top_y = button_bottom_y = 0;
    setInsetName("Collapsable");
}


Inset * InsetCollapsable::Clone() const
{
    InsetCollapsable * result = new InsetCollapsable(buffer);
    result->init(buffer, this);

    result->collapsed = collapsed;
    return result;
}

void InsetCollapsable::Write(ostream & os) const
{
    os << getInsetName() << "\n\ncollapsed ";
    if (display())
	os << "false\n";
    else
	os << "true\n";
    WriteParagraphData(os);
}


void InsetCollapsable::Read(LyXLex & lex)
{
    if (lex.IsOK()) {
	lex.next();
        string token = lex.GetString();
	if (token == "collapsed") {
	    lex.next();
	    collapsed = lex.GetBool();
	}
    }
    InsetText::Read(lex);
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

    return getMaxWidth(pain, this) - widthOffset + 2;
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
    button_length = width_collapsed(pain, labelfont) + 2;
    button_top_y = -ascent_collapsed(pain, f);
    button_bottom_y = descent_collapsed(pain, f);
    if (collapsed) {
	draw_collapsed(pain, f, baseline, x);
	return;
    }

    int top_x = int(x);

    draw_collapsed(pain, f, baseline, x);
    x += 2;

    int w =  getMaxTextWidth(pain, this);
    int h = ascent(pain,f) + descent(pain, f);
    
    pain.rectangle(int(x), baseline - ascent(pain, f), w, h, framecolor);

    x += 4;
    drawTextXOffset = int(x) - top_x;
    InsetText::draw(pain, f, baseline, x);
}


void InsetCollapsable::Edit(BufferView * bv, int x, int y, unsigned int button)
{
    if (collapsed && autocollapse) {
	collapsed = false;
	UpdateLocal(bv, true);
	InsetText::Edit(bv, 0, 0, button);
    } else if (!collapsed) {
	InsetText::Edit(bv, x, y, button);
    }
}


Inset::EDITABLE InsetCollapsable::Editable() const
{
	if (collapsed)
		return IS_EDITABLE;
	return HIGHLY_EDITABLE;
}


void InsetCollapsable::InsetUnlock(BufferView * bv)
{
    if (autocollapse) {
	collapsed = true;
    }
    InsetText::InsetUnlock(bv);
    UpdateLocal(bv, false);
}


void InsetCollapsable::UpdateLocal(BufferView * bv, bool flag)
{
    InsetText::UpdateLocal(bv, flag);
}


void InsetCollapsable::InsetButtonPress(BufferView * bv,int x,int y,int button)
{
    if (!collapsed && (x >= button_length)) {
	InsetText::InsetButtonPress(bv, x, y, button);
    }
}


void InsetCollapsable::InsetButtonRelease(BufferView * bv,
					  int x, int y, int button)
{
    if ((x >= 0)  && (x < button_length) &&
	(y >= button_top_y) &&  (y < button_bottom_y)) {
	if (collapsed) {
	    collapsed = false;
	    InsetText::InsetButtonRelease(bv, 0, 0, button);
	    UpdateLocal(bv, true);
	} else {
	    collapsed = true;
	    UpdateLocal(bv, false);
	    bv->unlockInset(this);
	}
    } else if (!collapsed && (x >= button_length) && (y >= button_top_y)) {
	InsetText::InsetButtonRelease(bv, x, y, button);
    }
}


void InsetCollapsable::InsetMotionNotify(BufferView * bv,
					 int x, int y, int state)
{
    if (x >= button_length) {
	InsetText::InsetMotionNotify(bv, x, y, state);
    }
}


int InsetCollapsable::getMaxWidth(Painter & pain,
				  UpdatableInset const * inset) const
{
    if ((this == inset) && !owner())
	return pain.paperWidth();
    if (this == inset)
        return static_cast<UpdatableInset*>(owner())->getMaxWidth(pain,inset);
    if (owner())
        return static_cast<UpdatableInset*>(owner())->getMaxWidth(pain,inset)-
		width_collapsed(pain, labelfont) - 2 - widthOffset;

    return pain.paperWidth()-width_collapsed(pain, labelfont)-2-widthOffset;
}


int InsetCollapsable::getMaxTextWidth(Painter & pain,
				      UpdatableInset const * inset, int) const
{
    return getMaxWidth(pain, inset) -
	width_collapsed(pain, labelfont) - widthOffset - 2;
}
