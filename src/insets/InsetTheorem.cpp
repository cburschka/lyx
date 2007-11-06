#if 0
/**
 * \file InsetTheorem.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetTheorem.h"
#include "insets/InsetText.h"

#include "debug.h"
#include "gettext.h"
#include "FontInfo.h"
#include "Text.h"
#include "MetricsInfo.h"

#include <ostream>


namespace lyx {

using std::endl;
using std::ostream;


/*
  The intention is to be able to create arbitrary theorem like environments
   sing this class and some helper/container classes. It should be possible
   to create these theorems both from layout file and interactively by the
   user.
*/

Inset.heorem::InsetTheorem()
	: InsetCollapsable()
{
	setLabel(_("theorem"));
#if 0
	setAutoCollapse(false);
#endif
	setInsetName("Theorem");
}


void Inset.heorem::write(Buffer const * buf, ostream & os) const
{
	os << name() << "\n";
	InsetCollapsable::write(buf, os);
}


Inset * InsetTheorem::clone() const
{
	// FIXME: Is this inset used? If YES this is WRONG!!! (Jug)
	InsetTheorem * result = new InsetTheorem;
	result->setCollapsed(!isOpen());
	return result;
}


void Inset.heorem::metrics(MetricsInfo & mi, Dimension & dim) const
{
	InsetCollapsable::metrics(mi, dim);
	center_indent_ = (mi.base.textwidth - dim.wid) / 2;
	dim.wid = mi.base.textwidth;
}


void InsetTOC::draw(PainterInfo & pi, int x, int y) const
{
	InsetCollapsable::draw(pi, x + center_indent_, y);
}


string const Inset.heorem::editMessage() const
{
	return _("Opened Theorem Inset");
}


int Inset.heorem::latex(Buffer const * buf, odocstream & os,
			OutputParams const & runparams) const
{
	os << "\\begin{theorem}%\n";

	int i = inset.latex(buf, os, runparams);
	os << "\\end{theorem}%\n";

	return i + 2;
}


} // namespace lyx

#endif
