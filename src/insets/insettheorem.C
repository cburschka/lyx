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

#include "insettheorem.h"
#include "gettext.h"
#include "lyxfont.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "support/LOstream.h"
#include "debug.h"
#include "insets/insettext.h"

using std::ostream;
using std::endl;

/*
  The intention is to be able to create arbitrary theorem like environments
   sing this class and some helper/container classes. It should be possible
   to create these theorems both from layout file and interactively by the
   user.
*/

InsetTheorem::InsetTheorem()
	: InsetCollapsable()
{
	setLabel(_("theorem"));
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(LColor::collapsable);
	setLabelFont(font);
#if 0
	setAutoCollapse(false);
#endif
	setInsetName("Theorem");
}


void InsetTheorem::write(Buffer const * buf, ostream & os) const 
{
	os << getInsetName() << "\n";
	InsetCollapsable::write(buf, os);
}


Inset * InsetTheorem::clone(Buffer const &, bool) const
{
#warning Is this inset used? If YES this is WRONG!!! (Jug)
	InsetTheorem * result = new InsetTheorem;
	
	result->collapsed_ = collapsed_;
	return result;
}


string const InsetTheorem::editMessage() const
{
	return _("Opened Theorem Inset");
}


int InsetTheorem::latex(Buffer const * buf,
			ostream & os, bool fragile, bool fp) const
{
	os << "\\begin{theorem}%\n";
	
	int i = inset.latex(buf, os, fragile, fp);
	os << "\\end{theorem}%\n";
	
	return i + 2;
}
