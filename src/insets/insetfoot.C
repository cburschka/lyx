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
// the following are needed just to get the layout of the enclosing
// paragraph. This seems a bit too much to me (JMarc)
#include "lyxlayout.h"
#include "outputparams.h"
#include "paragraph.h"
#include "paragraph_funcs.h"

#include "support/std_ostream.h"


using std::string;
using std::auto_ptr;
using std::ostream;


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


auto_ptr<InsetBase> InsetFoot::doClone() const
{
	return auto_ptr<InsetBase>(new InsetFoot(*this));
}


string const InsetFoot::editMessage() const
{
	return _("Opened Footnote Inset");
}


int InsetFoot::latex(Buffer const & buf, ostream & os,
		     OutputParams const & runparams_in) const
{
	OutputParams runparams = runparams_in;
	// footnotes in titling commands like \title have moving arguments
	runparams.moving_arg |= runparams_in.intitle;

	// in titling commands, \thanks should be used instead of \footnote.
	// some classes (e.g. memoir) do not understand \footnote.
	if (runparams_in.intitle)
		os << "%\n\\thanks{";
	else
		os << "%\n\\footnote{";

	int const i = InsetText::latex(buf, os, runparams);
	os << "%\n}";

	return i + 2;
}


int InsetFoot::docbook(Buffer const & buf, ostream & os,
		       OutputParams const & runparams) const
{
	os << "<footnote>";
	int const i = InsetText::docbook(buf, os, runparams);
	os << "</footnote>";

	return i;
}
