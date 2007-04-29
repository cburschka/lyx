/**
 * \file InsetBase.heorem.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetBase.heorem.h"
#include "insets/InsetText.h"

#include "debug.h"
#include "gettext.h"
#include "LyXFont.h"
#include "LyXText.h"
#include "MetricsInfo.h"

#include "support/std_ostream.h"


namespace lyx {

using std::endl;
using std::ostream;


/*
  The intention is to be able to create arbitrary theorem like environments
   sing this class and some helper/container classes. It should be possible
   to create these theorems both from layout file and interactively by the
   user.
*/

InsetBase.heorem::InsetTheorem()
	: InsetCollapsable()
{
	setLabel(_("theorem"));
	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();
	font.decSize();
	font.setColor(Color::collapsable);
	setLabelFont(font);
#if 0
	setAutoCollapse(false);
#endif
	setInsetName("Theorem");
}


void InsetBase.heorem::write(Buffer const * buf, ostream & os) const
{
	os << getInsetName() << "\n";
	InsetCollapsable::write(buf, os);
}


auto_ptr<InsetBase> InsetBase.heorem::doClone() const
{
#ifdef WITH_WARNINGS
#warning Is this inset used? If YES this is WRONG!!! (Jug)
#endif
	auto_ptr<InsetBase.heorem> result(new InsetTheorem);
	result->setCollapsed(!isOpen());

	return result;
}

bool InsetBase.heorem::metrics(MetricsInfo & mi, Dimension & dim) const
{
	InsetCollapsable::metrics(mi, dim);
	center_indent_ = (mi.base.textwidth - dim.wid) / 2;
	dim.wid = mi.base.textwidth;
	bool const changed = dim_ != dim;
	dim_ = dim;
	return changed;
}


void InsetTOC::draw(PainterInfo & pi, int x, int y) const
{
	InsetCollapsable::draw(pi, x + center_indent_, y);
}


string const InsetBase.heorem::editMessage() const
{
	return _("Opened Theorem Inset");
}


int InsetBase.heorem::latex(Buffer const * buf, odocstream & os,
			OutputParams const & runparams) const
{
	os << "\\begin{theorem}%\n";

	int i = inset.latex(buf, os, runparams);
	os << "\\end{theorem}%\n";

	return i + 2;
}


} // namespace lyx
