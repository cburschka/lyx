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

using std::ostream;
using std::endl;


InsetMarginal::InsetMarginal()
	: InsetFootlike()
{
	setLabel(_("margin"));
	setInsetName("Marginal");
}


Inset * InsetMarginal::Clone(Buffer const &) const
{
	InsetMarginal * result = new InsetMarginal;
	result->inset->init(inset);
	
	result->collapsed = collapsed;
	return result;
}


string const InsetMarginal::EditMessage() const
{
	return _("Opened Marginal Note Inset");
}


int InsetMarginal::Latex(Buffer const * buf,
			 ostream & os, bool fragile, bool fp) const
{
	os << "\\marginpar{%\n";
	
	int i = inset->Latex(buf, os, fragile, fp);
	os << "}%\n";
	
	return i + 2;
}


bool InsetMarginal::InsertInsetAllowed(Inset * in) const
{
	if ((in->LyxCode() == Inset::FOOT_CODE) ||
	    (in->LyxCode() == Inset::MARGIN_CODE)) {
		return false;
	}
	return true;
}
