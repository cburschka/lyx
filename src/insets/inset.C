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

using std::endl;

/* Insets default methods */

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
{
}


bool Inset::AutoDelete() const
{
  return false;
}


void Inset::Edit(BufferView *, int, int, unsigned int)
{
}


LyXFont Inset::ConvertFont(LyXFont font)
{
  return font;
}


char const * Inset::EditMessage() const 
{
	return _("Opened inset");
}

 /* some stuff for inset locking */

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
{
}


void UpdatableInset::ShowInsetCursor(BufferView *)
{
}


void UpdatableInset::HideInsetCursor(BufferView *)
{
}


void UpdatableInset::Edit(BufferView * bv, int, int, unsigned int)
{
    LyXFont font;

    scx = 0;

    mx_scx = abs((width(bv, font) - bv->workWidth()) / 2);
}


void UpdatableInset::draw(BufferView *, LyXFont const &,
			  int /* baseline */, float & x, bool/*cleared*/) const
{
    if (scx) x += float(scx);
// ATTENTION: don't do the following here!!!
//    top_x = int(x);
//    top_baseline = baseline;
}


void UpdatableInset::SetFont(BufferView *, LyXFont const &, bool )
{
}


///  An updatable inset could handle lyx editing commands
#ifdef SCROLL_INSET
UpdatableInset::RESULT
UpdatableInset::LocalDispatch(BufferView *, 
			      int action, string const & arg) 
#else
UpdatableInset::RESULT
UpdatableInset::LocalDispatch(BufferView *, int, string const &)
#endif
{
#ifdef SCROLL_INSET
    if (action==LFUN_SCROLL_INSET)
	{
	    float xx;
	    sscanf(arg.c_str(), "%f", &xx);	
	    scroll(xx);

	    return DISPATCHED;
	}
#endif
    return UNDISPATCHED; 
}


int UpdatableInset::getMaxWidth(Painter & pain, UpdatableInset const *) const
{
    if (owner())
        return static_cast<UpdatableInset*>(owner())->getMaxWidth(pain, this);
    return pain.paperWidth();
}
