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
	font.setColor(LColor::footnote);
	setLabelFont(font);
	setAutoCollapse(false);
	setInsetName("Theorem");
}


void InsetTheorem::Write(Buffer const * buf, ostream & os) const 
{
	os << getInsetName() << "\n";
	InsetCollapsable::Write(buf, os);
}


Inset * InsetTheorem::Clone() const
{
	InsetTheorem * result = new InsetTheorem;
	
	result->collapsed = collapsed;
	return result;
}


char const * InsetTheorem::EditMessage() const
{
	return _("Opened Theorem Inset");
}


int InsetTheorem::Latex(Buffer const * buf,
			ostream & os, bool fragile, bool fp) const
{
	os << "\\begin{theorem}%\n";
	
	int i = inset->Latex(buf, os, fragile, fp);
	os << "\\end{theorem}%\n";
	
	return i + 2;
}


bool InsetTheorem::InsertInsetAllowed(Inset * inset) const
{
	lyxerr << "InsetTheorem::InsertInsetAllowed" << endl;
	
	if ((inset->LyxCode() == Inset::FOOT_CODE) ||
	    (inset->LyxCode() == Inset::MARGIN_CODE)) {
		return false;
	}
	return true;
}
