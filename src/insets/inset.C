/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team.
 *
 * ======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation "lyxinset.h"
#endif

#include "lyxinset.h"
#include "error.h"
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
	// nothing by default
}



bool Inset::AutoDelete() const
{
  return false;
}


void Inset::Edit(int, int)
{
}


LyXFont Inset::ConvertFont(LyXFont font)
{
  return font;
}


// Inset::Code Inset::LyxCode() const
// {
//   return Inset::NO_CODE;
// }

 /* some stuff for inset locking */


void UpdatableInset::InsetButtonPress(int x, int y, int button)
{
	lyxerr.debug(string("Inset Button Press x=")+ tostr(x) +
		      ", y=" + tostr(y) + ", button=" + tostr(button));
}


void UpdatableInset::InsetButtonRelease(int x, int y, int button)
{
	lyxerr.debug(string("Inset Button Release x=")+ tostr(x) +
		      ", y=" + tostr(y) + ", button=" + tostr(button));
}


void UpdatableInset::InsetKeyPress(XKeyEvent *)
{
	lyxerr.debug("Inset Keypress");
}


void UpdatableInset::InsetMotionNotify(int x, int y, int state)
{
	lyxerr.debug(string("Inset Motion Notify x=")+ tostr(x) +
		      ", y=" + tostr(y) + ", state=" + tostr(state));
}


void UpdatableInset::InsetUnlock()
{
	lyxerr.debug("Inset Unlock", Error::ANY);
}


// An updatable inset is highly editable by definition
unsigned char UpdatableInset::Editable() const
{
	return 2; // and what does "2" siginify? (Lgb)
}


void UpdatableInset::ToggleInsetCursor()
{
  // nothing
}
