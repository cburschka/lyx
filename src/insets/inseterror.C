/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "BufferView.h"
#include "font.h"
#include "lyxfont.h"
#include "gettext.h"
#include "inseterror.h"
#include "LyXView.h"
#include "Painter.h"
#include "frontends/Dialogs.h"

using std::ostream;

/* Error, used for the LaTeX-Error Messages */

InsetError::InsetError(string const & str, bool)
	: contents(str)
{}


int InsetError::ascent(BufferView *, LyXFont const & font) const
{
	LyXFont efont;
	efont.setSize(font.size()).decSize();
	return lyxfont::maxAscent(efont) + 1;
}


int InsetError::descent(BufferView *, LyXFont const & font) const
{
	LyXFont efont;
	efont.setSize(font.size()).decSize();
	return lyxfont::maxDescent(efont) + 1;
}


int InsetError::width(BufferView *, LyXFont const & font) const
{
	LyXFont efont;
	efont.setSize(font.size()).decSize();
	return 6 + lyxfont::width(_("Error"), efont);
}


void InsetError::draw(BufferView * bv, LyXFont const & font,
		      int baseline, float & x, bool) const
{
	Painter & pain = bv->painter();
	LyXFont efont;
	efont.setSize(font.size()).decSize();
	efont.setColor(LColor::error);
   
	// Draw as "Error" in a framed box
	x += 1;
	pain.fillRectangle(int(x), baseline - ascent(bv, font) + 1,
			  width(bv, font) - 2,
			  ascent(bv, font) + descent(bv, font) - 2,
			   LColor::insetbg);
	pain.rectangle(int(x), baseline - ascent(bv, font) + 1,
		       width(bv, font) - 2,
		       ascent(bv, font) + descent(bv, font) - 2,
		       LColor::error);
	pain.text(int(x + 2), baseline, _("Error"), efont);

	x +=  width(bv, font) - 1;
}


string const InsetError::editMessage() const 
{
	return _("Opened error");
}


void InsetError::edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getDialogs()->showError(this);
}


void InsetError::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, 0);
}
