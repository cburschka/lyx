/**
 * \file insettheorem.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insettheorem.h"
#include "insets/insettext.h"

#include "debug.h"
#include "BufferView.h"
#include "gettext.h"
#include "lyxfont.h"
#include "lyxtext.h"
#include "metricsinfo.h"

#include "support/std_ostream.h"

using std::endl;
using std::ostream;


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


auto_ptr<InsetBase> InsetTheorem::clone() const
{
#ifdef WITH_WARNINGS
#warning Is this inset used? If YES this is WRONG!!! (Jug)
#endif
	auto_ptr<InsetTheorem> result(new InsetTheorem);
	result->setCollapsed(!isOpen());

	return result;
}

void InsetTheorem::metrics(MetricsInfo & mi, Dimension & dim) const
{
	InsetCollapsable::metrics(mi, dim);
	center_indent_ = (mi.base.textwidth - dim.wid) / 2;
	dim.wid = mi.base.textwidth;
	dim_ = dim;
}


void InsetTOC::draw(PainterInfo & pi, int x, int y) const
{
	InsetCollapsable::draw(pi, x + center_indent_, y);
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
