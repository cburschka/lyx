/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team.
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

/* Insets default methods */

bool Inset::Deletable() const
{
  return true;
}


bool Inset::DirectWrite() const
{
  return false;
}


unsigned char Inset::Editable() const
{
  return 0;
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
unsigned char UpdatableInset::Editable() const
{
	return 2; // and what does "2" siginify? (Lgb)
}


void UpdatableInset::ToggleInsetCursor(BufferView *)
{
}

void UpdatableInset::Edit(BufferView * bv, int, int, unsigned int)
{
    LyXFont
	font;
//    bview = bv;
    scx = 0;

    mx_scx=abs((width(bv->getPainter(), font) - bv->paperWidth())/2);
}


void UpdatableInset::draw(Painter &, LyXFont const &,
			  int baseline, float & x) const
{
    if (scx) x += float(scx);
    top_x = int(x);
    top_baseline = baseline;
}


void UpdatableInset::SetFont(LyXFont const &, bool )
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
