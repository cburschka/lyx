/**
 * \file inseterror.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "inseterror.h"

#include "BufferView.h"
#include "funcrequest.h"
#include "gettext.h"
#include "lyxfont.h"

#include "frontends/Dialogs.h"
#include "frontends/font_metrics.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"

#include "support/LAssert.h"

using std::ostream;

/* Error, used for the LaTeX-Error Messages */

InsetError::InsetError(string const & str, bool)
	: contents(str)
{}


InsetError::~InsetError()
{
	if (view())
		view()->owner()->getDialogs().hide("error");
}


dispatch_result InsetError::localDispatch(FuncRequest const & cmd)
{
	dispatch_result result = UNDISPATCHED;

	switch (cmd.action) {
	case LFUN_MOUSE_RELEASE:
		edit(cmd.view(), cmd.x, cmd.y, cmd.button());
		break;

	default:
		break;
	}

	return result;
}


int InsetError::ascent(BufferView *, LyXFont const & font) const
{
	LyXFont efont;
	efont.setSize(font.size()).decSize();
	return font_metrics::maxAscent(efont) + 1;
}


int InsetError::descent(BufferView *, LyXFont const & font) const
{
	LyXFont efont;
	efont.setSize(font.size()).decSize();
	return font_metrics::maxDescent(efont) + 1;
}


int InsetError::width(BufferView *, LyXFont const & font) const
{
	LyXFont efont;
	efont.setSize(font.size()).decSize();
	return 6 + font_metrics::width(_("Error"), efont);
}


void InsetError::draw(BufferView * bv, LyXFont const & font,
		      int baseline, float & x, bool) const
{
	lyx::Assert(bv);
	cache(bv);

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


void InsetError::edit(BufferView * bv, int, int, mouse_button::state)
{
	bv->owner()->getDialogs().show("error", getContents(), this);
}


void InsetError::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, mouse_button::none);
}
