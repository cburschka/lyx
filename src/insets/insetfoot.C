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


Inset * InsetFoot::Clone(Buffer const &) const
{
	InsetFoot * result = new InsetFoot;
	result->inset.init(&inset);

	result->collapsed = collapsed;
	return result;
}


string const InsetFoot::EditMessage() const
{
	return _("Opened Footnote Inset");
}


int InsetFoot::Latex(Buffer const * buf,
		     std::ostream & os, bool fragile, bool fp) const
{
	os << "\\footnote{%\n";
	
	int const i = inset.Latex(buf, os, fragile, fp);
	os << "}%\n";
	
	return i + 2;
}


bool InsetFoot::InsertInsetAllowed(Inset * in) const
{
	if ((in->LyxCode() == Inset::FOOT_CODE) ||
	    (in->LyxCode() == Inset::MARGIN_CODE)) {
		return false;
	}
	return true;
}
