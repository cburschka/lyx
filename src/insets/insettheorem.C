/**
 * \file insettheorem.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


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


InsetBase * InsetTheorem::clone() const
{
#ifdef WITH_WARNINGS
#warning Is this inset used? If YES this is WRONG!!! (Jug)
#endif
	InsetTheorem * result = new InsetTheorem;

	result->collapsed_ = collapsed_;
	return result;
}


string const InsetTheorem::editMessage() const
{
	return _("Opened Theorem Inset");
}


int InsetTheorem::latex(Buffer const * buf, ostream & os,
			LatexRunParams const & runparams) const
{
	os << "\\begin{theorem}%\n";

	int i = inset.latex(buf, os, runparams);
	os << "\\end{theorem}%\n";

	return i + 2;
}
