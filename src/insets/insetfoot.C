/**
 * \file insetfoot.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetfoot.h"
#include "gettext.h"
#include "lyxfont.h"
#include "BufferView.h"
#include "latexrunparams.h"
#include "lyxtext.h"
#include "insets/insettext.h"
#include "support/LOstream.h"
#include "debug.h"
// the following are needed just to get the layout of the enclosing
// paragraph. This seems a bit too much to me (JMarc)
#include "lyxlayout.h"
#include "paragraph_funcs.h"


using std::ostream;
using std::auto_ptr;


InsetFoot::InsetFoot(BufferParams const & bp)
	: InsetFootlike(bp)
{
	setLabel(_("foot"));
	setInsetName("Foot");
}


InsetFoot::InsetFoot(InsetFoot const & in)
	: InsetFootlike(in)
{
	setLabel(_("foot"));
	setInsetName("Foot");
}


auto_ptr<InsetBase> InsetFoot::clone() const
{
	return auto_ptr<InsetBase>(new InsetFoot(*this));
}


string const InsetFoot::editMessage() const
{
	return _("Opened Footnote Inset");
}


int InsetFoot::latex(Buffer const * buf, ostream & os,
		     LatexRunParams const & runparams_in) const
{
	LatexRunParams runparams = runparams_in;
	if (buf) {
		runparams.moving_arg |= ownerPar(*buf, this).layout()->intitle;
	}

	os << "%\n\\footnote{";

	int const i = inset.latex(buf, os, runparams);
	os << "%\n}";

	return i + 2;
}


int InsetFoot::docbook(Buffer const * buf, ostream & os, bool mixcont) const
{
	os << "<footnote>";
	int const i = inset.docbook(buf, os, mixcont);
	os << "</footnote>";

	return i;
}
