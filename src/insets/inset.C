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


void Inset::Edit(BufferView *, int, int)
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
