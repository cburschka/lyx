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


InsetMarginal::InsetMarginal(BufferParams const & bp)
	: InsetFootlike(bp)
{
	setLabel(_("margin"));
	setInsetName("Marginal");
}


InsetMarginal::InsetMarginal(InsetMarginal const & in, bool same_id)
	: InsetFootlike(in, same_id)
{
	setLabel(_("margin"));
	setInsetName("Marginal");
}


Inset * InsetMarginal::clone(Buffer const &, bool same_id) const
{
	return new InsetMarginal(*const_cast<InsetMarginal *>(this), same_id);
}


string const InsetMarginal::editMessage() const
{
	return _("Opened Marginal Note Inset");
}


int InsetMarginal::latex(Buffer const * buf,
			 ostream & os, bool fragile, bool fp) const
{
	os << "%\n\\marginpar{";
	
	int const i = inset.latex(buf, os, fragile, fp);
	os << "%\n}";
	
	return i + 2;
}
