/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright (C) 1998 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetcollapsable.h"
#include "gettext.h"
#include "lyxfont.h"
#include "BufferView.h"
#include "Painter.h"
#include "insets/insettext.h"
#include "support/LOstream.h"
#include "support/lstrings.h"
#include "debug.h"
#include "lyxtext.h"

class LyXText;

using std::ostream;
using std::endl;
using std::max;

InsetCollapsable::InsetCollapsable()
	: UpdatableInset()
{
    inset = new InsetText;
    inset->setOwner(this);
    collapsed = true;
    label = "Label";
    autocollapse = true;
    inset->SetAutoBreakRows(true);
    inset->SetDrawFrame(0, InsetText::ALWAYS);
    inset->SetFrameColor(0, LColor::footnoteframe);
    button_length = button_top_y = button_bottom_y = 0;
    setInsetName("Collapsable");
    widthCollapsed = oldWidth = 0;
    need_update = FULL;
}


Inset * InsetCollapsable::Clone(Buffer const &) const
{
    InsetCollapsable * result = new InsetCollapsable();
    result->inset->init(inset);
    result->inset->setOwner(result);

    result->collapsed = collapsed;
    return result;
}


bool InsetCollapsable::InsertInset(BufferView * bv, Inset * in)
{
    if (!InsertInsetAllowed(in)) {
	lyxerr << "InsetCollapsable::InsertInset: "
		"Unable to insert inset." << endl;
	return false;
    }
    
    return inset->InsertInset(bv, in);
}


void InsetCollapsable::Write(Buffer const * buf, ostream & os) const
{
    os << "collapsed " << tostr(collapsed) << "\n";
    inset->WriteParagraphData(buf, os);
}



void InsetCollapsable::Read(Buffer const * buf, LyXLex & lex)
{
    if (lex.IsOK()) {
	lex.next();
        string token = lex.GetString();
	if (token == "collapsed") {
	    lex.next();
	    collapsed = lex.GetBool();
	} else {
		lyxerr << "InsetCollapsable::Read: Missing collapsed!"
		       << endl;
	}
    }
    inset->Read(buf, lex);
}


int InsetCollapsable::ascent_collapsed(Painter & pain, LyXFont const &) const
{
    int width = 0, ascent = 0, descent = 0;
    pain.buttonText(0, 0, label, labelfont, false, 
		    width, ascent, descent);
    return ascent;
}


int InsetCollapsable::descent_collapsed(Painter & pain, LyXFont const &) const
{
    int width = 0, ascent = 0, descent = 0;
    pain.buttonText(0, 0, label, labelfont, false, 
		    width, ascent, descent);
    return descent;
}


int InsetCollapsable::width_collapsed(Painter & pain, LyXFont const &) const
{
    int width, ascent, descent;
    pain.buttonText(TEXT_TO_INSET_OFFSET, 0, label, labelfont, false,
		    width, ascent, descent);
    return width + (2*TEXT_TO_INSET_OFFSET);
}


int InsetCollapsable::ascent(BufferView * bv, LyXFont const & font) const
{
    if (collapsed) 
	return ascent_collapsed(bv->painter(), font);
    else 
	return inset->ascent(bv, font) + TEXT_TO_TOP_OFFSET;
}


int InsetCollapsable::descent(BufferView * bv, LyXFont const & font) const
{
    if (collapsed) 
	return descent_collapsed(bv->painter(), font);
    else 
	return inset->descent(bv, font) + TEXT_TO_BOTTOM_OFFSET;
}


int InsetCollapsable::width(BufferView * bv, LyXFont const & font) const
{
    if (collapsed) 
	return widthCollapsed;

    return inset->width(bv, font) + widthCollapsed;
}


void InsetCollapsable::draw_collapsed(Painter & pain, LyXFont const &,
				      int baseline, float & x) const
{
    int width = 0;
    pain.buttonText(int(x) + TEXT_TO_INSET_OFFSET,
		    baseline, label, labelfont, true, width);
    x += width + TEXT_TO_INSET_OFFSET;
}


void InsetCollapsable::draw(BufferView * bv, LyXFont const & f, 
			    int baseline, float & x, bool cleared) const
{
    Painter & pain = bv->painter();

    button_length = widthCollapsed;
    button_top_y = -ascent_collapsed(pain, f);
    button_bottom_y = descent_collapsed(pain, f);
    if (collapsed) {
	draw_collapsed(pain, f, baseline, x);
	x += TEXT_TO_INSET_OFFSET;
	return;
    }

    if (!cleared && ((inset->need_update == InsetText::FULL) ||
		     (inset->need_update == InsetText::INIT) ||
		     (top_x!=int(x)) || (top_baseline!=baseline))) {
	int w =  owner() ? width(bv, f) : pain.paperWidth();
	int h = ascent(bv, f) + descent(bv, f);
	int tx = (needFullRow() && !owner()) ? 0 : int(x);
	int ty = max(0, baseline - ascent(bv, f));
	
	if ((ty + h) > pain.paperHeight())
	    h = pain.paperHeight();
	if ((top_x + w) > pain.paperWidth())
	    w = pain.paperWidth();
	if (baseline < 0)
	    h += (baseline - ascent(bv, f));
	pain.fillRectangle(tx, ty - 1, w, h + 2);
	cleared = true;
    }

    // not needed if collapsed
    top_x = int(x);
    top_baseline = baseline;

    draw_collapsed(pain, f, baseline, x);
    inset->draw(bv, f, baseline, x, cleared);
    need_update = NONE;
}


void InsetCollapsable::Edit(BufferView * bv, int x, int y, unsigned int button)
{
    UpdatableInset::Edit(bv, x, y, button);

    if (collapsed && autocollapse) {
	collapsed = false;
	if (!bv->lockInset(this))
	    return;
	bv->updateInset(this, false);
	inset->Edit(bv, 0, 0, button);
    } else if (!collapsed) {
	if (!bv->lockInset(this))
	    return;
	inset->Edit(bv, x-widthCollapsed, y, button);
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
    inset->InsetUnlock(bv);
    bv->updateInset(this, false);
}


void InsetCollapsable::InsetButtonPress(BufferView * bv,int x,int y,int button)
{
    if (!collapsed && (x >= button_length)) {
	inset->InsetButtonPress(bv, x-widthCollapsed, y, button);
    }
}


void InsetCollapsable::InsetButtonRelease(BufferView * bv,
					  int x, int y, int button)
{
    if ((x >= 0)  && (x < button_length) &&
	(y >= button_top_y) &&  (y < button_bottom_y)) {
	if (collapsed) {
	    collapsed = false;
	    inset->InsetButtonRelease(bv, 0, 0, button);
	    bv->updateInset(this, false);
	} else {
	    collapsed = true;
	    bv->unlockInset(this);
	    bv->updateInset(this, false);
	}
    } else if (!collapsed && (x >= button_length) && (y >= button_top_y)) {
	inset->InsetButtonRelease(bv, x-widthCollapsed, y, button);
    }
}


void InsetCollapsable::InsetMotionNotify(BufferView * bv,
					 int x, int y, int state)
{
    if (x >= button_length) {
	inset->InsetMotionNotify(bv, x-widthCollapsed, y, state);
    }
}


void InsetCollapsable::InsetKeyPress(XKeyEvent * xke)
{
    inset->InsetKeyPress(xke);
}


int InsetCollapsable::Latex(Buffer const * buf, ostream & os,
			    bool fragile, bool free_spc) const
{
    return inset->Latex(buf, os, fragile, free_spc);
}


int InsetCollapsable::getMaxWidth(Painter & pain,
				  UpdatableInset const * inset) const
{
    int w = UpdatableInset::getMaxWidth(pain,inset);

    if (w < 0) {
	// What does a negative max width signify? (Lgb)
	return w;
    }
    // should be at least 30 pixels !!!
    return max(30, w - widthCollapsed);
}


#if 0
int InsetCollapsable::getMaxTextWidth(Painter & pain,
				      UpdatableInset const * inset) const
{
    return getMaxWidth(pain, inset) - widthCollapsed;
}
#endif


void InsetCollapsable::update(BufferView * bv, LyXFont const & font,
			      bool reinit)
{
    if (reinit) {
	need_update = FULL;
	if (owner())
	    owner()->update(bv, font, true);
	return;
    }
    if (!widthCollapsed) {
	widthCollapsed = width_collapsed(bv->painter(), font);
	inset->resizeLyXText(bv);
	need_update = FULL;
	if (owner()) {
		owner()->update(bv, font);
		return;
	}
    }
    if (oldWidth != width(bv, font)) {
	oldWidth = width(bv, font);
	inset->resizeLyXText(bv);
	need_update = FULL;
	if (owner()) {
		owner()->update(bv, font);
		return;
	}
    }
    inset->update(bv, font);
}


UpdatableInset::RESULT
InsetCollapsable::LocalDispatch(BufferView * bv, int action,
				string const & arg)
{
    UpdatableInset::RESULT result = inset->LocalDispatch(bv, action, arg);
    if (result == FINISHED)
        bv->unlockInset(this);
    return result;
}


bool InsetCollapsable::LockInsetInInset(BufferView * bv, UpdatableInset * in)
{
    if (inset == in)
	return true;
    return inset->LockInsetInInset(bv, in);
}


bool InsetCollapsable::UnlockInsetInInset(BufferView * bv, UpdatableInset * in,
					  bool lr)
{
    if (inset == in) {
	bv->unlockInset(this);
	return true;
    }
    return inset->UnlockInsetInInset(bv, in, lr);
}


bool InsetCollapsable::UpdateInsetInInset(BufferView * bv, Inset *in)
{
    if (in == inset)
	return true;
    return inset->UpdateInsetInInset(bv, in);
}


unsigned int InsetCollapsable::InsetInInsetY()
{
    return inset->InsetInInsetY();
}


void InsetCollapsable::Validate(LaTeXFeatures & features) const
{
    inset->Validate(features);
}


void InsetCollapsable::GetCursorPos(BufferView * bv, int & x, int & y) const
{
    inset->GetCursorPos(bv, x , y);
}


void InsetCollapsable::ToggleInsetCursor(BufferView * bv)
{
    inset->ToggleInsetCursor(bv);
}


UpdatableInset * InsetCollapsable::GetLockingInset()
{
    UpdatableInset * in = inset->GetLockingInset();
    if (inset == in)
	return this;
    return in;
}


UpdatableInset * InsetCollapsable::GetFirstLockingInsetOfType(Inset::Code c)
{
    if (c == LyxCode())
	return this;
    return inset->GetFirstLockingInsetOfType(c);
}


void InsetCollapsable::SetFont(BufferView * bv,
			       LyXFont const & font, bool toggleall)
{
    inset->SetFont(bv, font, toggleall);
}


bool InsetCollapsable::doClearArea() const
{
    return inset->doClearArea();
}


LyXText * InsetCollapsable::getLyXText(BufferView const * bv) const
{
    return inset->getLyXText(bv);
}


void InsetCollapsable::deleteLyXText(BufferView * bv, bool recursive) const
{
    inset->deleteLyXText(bv, recursive);
}


void InsetCollapsable::resizeLyXText(BufferView * bv) const
{
    inset->resizeLyXText(bv);
    LyXFont font(LyXFont::ALL_SANE);
    oldWidth = width(bv, font);
}
