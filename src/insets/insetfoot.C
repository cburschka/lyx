/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 1998 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetfoot.h"
#include "gettext.h"
#include "lyxfont.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "insets/insettext.h"
#include "support/LOstream.h"
#include "debug.h"


InsetFoot::InsetFoot()
	: InsetFootlike()
{
	setLabel(_("foot"));
	setInsetName("Foot");
}


Inset * InsetFoot::clone(Buffer const &) const
{
	InsetFoot * result = new InsetFoot;
	result->inset.init(&inset);

	result->collapsed = collapsed;
	return result;
}


string const InsetFoot::editMessage() const
{
	return _("Opened Footnote Inset");
}


int InsetFoot::latex(Buffer const * buf,
		     std::ostream & os, bool fragile, bool fp) const
{
	os << "%\n\\footnote{";
	
	int const i = inset.latex(buf, os, fragile, fp);
	os << "%\n}";
	
	return i + 2;
}


bool InsetFoot::insertInsetAllowed(Inset * in) const
{
	if ((in->lyxCode() == Inset::FOOT_CODE) ||
	    (in->lyxCode() == Inset::MARGIN_CODE)) {
		return false;
	}
	return true;
}
