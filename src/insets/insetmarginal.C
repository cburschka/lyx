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

#include "insetmarginal.h"
#include "gettext.h"
#include "lyxfont.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "insets/insettext.h"
#include "support/LOstream.h"
#include "debug.h"


InsetMarginal::InsetMarginal()
	: InsetFootlike()
{
	setLabel(_("margin"));
	setInsetName("Marginal");
}


Inset * InsetMarginal::clone(Buffer const &) const
{
	InsetMarginal * result = new InsetMarginal;
	result->inset.init(&inset);
	
	result->collapsed = collapsed;
	return result;
}


string const InsetMarginal::editMessage() const
{
	return _("Opened Marginal Note Inset");
}


int InsetMarginal::latex(Buffer const * buf,
			 std::ostream & os, bool fragile, bool fp) const
{
	os << "%\n\\marginpar{";
	
	int const i = inset.latex(buf, os, fragile, fp);
	os << "%\n}";
	
	return i + 2;
}


bool InsetMarginal::insertInsetAllowed(Inset * in) const
{
	if ((in->lyxCode() == Inset::FOOT_CODE) ||
	    (in->lyxCode() == Inset::MARGIN_CODE)) {
		return false;
	}
	return true;
}
