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


InsetFoot::InsetFoot(InsetFoot const & in, bool same_id)
	: InsetFootlike(in, same_id)
{
	setLabel(_("foot"));
	setInsetName("Foot");
}


Inset * InsetFoot::clone(Buffer const &, bool same_id) const
{
	return new InsetFoot(*const_cast<InsetFoot *>(this), same_id);
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

void InsetFoot::validate(LaTeXFeatures & features) const
{
        features.NeedLyXFootnoteCode = true;
}
