/**
 * \file insetmarginal.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


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


InsetMarginal::InsetMarginal(InsetMarginal const & in)
	: InsetFootlike(in)
{
	setLabel(_("margin"));
	setInsetName("Marginal");
}


InsetBase * InsetMarginal::clone() const
{
	return new InsetMarginal(*this);
}


string const InsetMarginal::editMessage() const
{
	return _("Opened Marginal Note Inset");
}


int InsetMarginal::latex(Buffer const * buf, ostream & os,
			 LatexRunParams const & runparams) const
{
	os << "%\n\\marginpar{";

	int const i = inset.latex(buf, os, runparams);
	os << "%\n}";

	return i + 2;
}
