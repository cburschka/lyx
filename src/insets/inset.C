/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation "lyxinset.h"
#endif

#include "lyxinset.h"
#include "debug.h"
#include "BufferView.h"
#include "support/lstrings.h"
#include "Painter.h"
#include "commandtags.h"
#include "support/lstrings.h"

using std::endl;

// Insets default methods

bool Inset::Deletable() const
{
	return true;
}


bool Inset::DirectWrite() const
{
	return false;
}


Inset::EDITABLE Inset::Editable() const
{
	return NOT_EDITABLE;
}


void Inset::Validate(LaTeXFeatures &) const
{}


bool Inset::AutoDelete() const
{
	return false;
}


void Inset::Edit(BufferView *, int, int, unsigned int)
{}


LyXFont const Inset::ConvertFont(LyXFont const & font) const
{
	return LyXFont(font);
}


string const Inset::EditMessage() const 
{
	return _("Opened inset");
}


LyXText * Inset::getLyXText(BufferView const * bv, bool const) const
{
	if (owner())
		return owner()->getLyXText(bv, false);
	else
		return bv->text;
}


// some stuff for inset locking

void UpdatableInset::InsetButtonPress(BufferView *, int x, int y, int button)
{
	lyxerr.debug() << "Inset Button Press x=" << x
		       << ", y=" << y << ", button=" << button << endl;
}


void UpdatableInset::InsetButtonRelease(BufferView *, int x, int y, int button)
{
	lyxerr.debug() << "Inset Button Release x=" << x
		       << ", y=" << y << ", button=" << button << endl;
}


void UpdatableInset::InsetKeyPress(XKeyEvent *)
{
	lyxerr.debug() << "Inset Keypress" << endl;
}


void UpdatableInset::InsetMotionNotify(BufferView *, int x, int y, int state)
{
	lyxerr.debug() << "Inset Motion Notify x=" << x
		       << ", y=" << y << ", state=" << state << endl;
}


void UpdatableInset::InsetUnlock(BufferView *)
{
	lyxerr.debug() << "Inset Unlock" << endl;
}


// An updatable inset is highly editable by definition
Inset::EDITABLE UpdatableInset::Editable() const
{
	return HIGHLY_EDITABLE;
}


void UpdatableInset::ToggleInsetCursor(BufferView *)
{}


void UpdatableInset::ShowInsetCursor(BufferView *, bool)
{}


void UpdatableInset::HideInsetCursor(BufferView *)
{}


void UpdatableInset::Edit(BufferView *, int, int, unsigned int)
{}


void UpdatableInset::draw(BufferView *, LyXFont const &,
			  int /* baseline */, float & x,
			  bool/*cleared*/) const
{
	x += float(scx);
	// ATTENTION: don't do the following here!!!
	//    top_x = int(x);
	//    top_baseline = baseline;
}


void UpdatableInset::SetFont(BufferView *, LyXFont const &, bool )
{}


void UpdatableInset::scroll(BufferView * bv, float s) const
{
	LyXFont font;
	
	if (!s) {
		scx = 0;
		return;
	}

	int const workW = bv->workWidth();
	int const tmp_top_x = top_x - scx;
	
	if (tmp_top_x > 0 && 
	    (tmp_top_x + width(bv, font)) < workW)
		return;
	if (s > 0 && top_x > 0)
		return;

	// int mx_scx=abs((width(bv,font) - bv->workWidth())/2);
	//int const save_scx = scx;
    
	scx = int(s * workW / 2);
	// if (!display())
	// scx += 20;

	if ((tmp_top_x + scx + width(bv, font)) < (workW / 2)) {
		scx += (workW / 2) - (tmp_top_x + scx + width(bv, font));
	}
	// bv->updateInset(const_cast<UpdatableInset *>(this), false);
}

void UpdatableInset::scroll(BufferView * bv, int offset) const
{
	if (offset > 0) {
		if (!scx && top_x >= 20)
			return;
		if ((top_x + offset) > 20)
			scx = 0;
		// scx += offset - (top_x - scx + offset - 20);
		else
			scx += offset;
	} else {
		LyXFont const font;
		if (!scx && (top_x + width(bv, font)) < (bv->workWidth() - 20))
			return;
		if ((top_x - scx + offset + width(bv, font)) < (bv->workWidth() - 20)) {
			scx = bv->workWidth() - width(bv, font) - top_x + scx - 20; 
		} else {
			scx += offset;
		}
	}
	// bv->updateInset(const_cast<UpdatableInset *>(this), false);
}


///  An updatable inset could handle lyx editing commands
UpdatableInset::RESULT
UpdatableInset::LocalDispatch(BufferView * bv, 
			      kb_action action, string const & arg) 
{
	if (!arg.empty() && (action==LFUN_SCROLL_INSET)) {
		if (arg.find('.') != arg.npos) {
			float const xx = static_cast<float>(strToDbl(arg));
			scroll(bv, xx);
		} else {
			int const xx = strToInt(arg);
			scroll(bv, xx);
		}
		bv->updateInset(this, false);
		
		return DISPATCHED;
	}
	return UNDISPATCHED; 
}


int UpdatableInset::getMaxWidth(BufferView * bv, UpdatableInset const *) const
{
	if (owner())
		return static_cast<UpdatableInset*>
			(owner())->getMaxWidth(bv, this);
	return bv->workWidth();
}
