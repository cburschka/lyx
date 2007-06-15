/**
 * \file InsetFoot.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetFoot.h"

#include "Buffer.h"
#include "gettext.h"
// the following is needed just to get the layout of the enclosing
// paragraph. This seems a bit too much to me (JMarc)
#include "OutputParams.h"

#include "support/std_ostream.h"


namespace lyx {

using std::string;
using std::auto_ptr;
using std::ostream;


InsetFoot::InsetFoot(BufferParams const & bp)
	: InsetFootlike(bp)
{
	setLabel(_("foot"));
}


InsetFoot::InsetFoot(InsetFoot const & in)
	: InsetFootlike(in)
{
	setLabel(_("foot"));
}


auto_ptr<Inset> InsetFoot::doClone() const
{
	return auto_ptr<Inset>(new InsetFoot(*this));
}


docstring const InsetFoot::editMessage() const
{
	return _("Opened Footnote Inset");
}


int InsetFoot::latex(Buffer const & buf, odocstream & os,
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
	runparams_in.encoding = runparams.encoding;

	return i + 2;
}


int InsetFoot::plaintext(Buffer const & buf, odocstream & os,
			 OutputParams const & runparams) const
{
	os << '[' << buf.B_("footnote") << ":\n";
	InsetText::plaintext(buf, os, runparams);
	os << "\n]";

	return PLAINTEXT_NEWLINE + 1; // one char on a separate line
}


int InsetFoot::docbook(Buffer const & buf, odocstream & os,
		       OutputParams const & runparams) const
{
	os << "<footnote>";
	int const i = InsetText::docbook(buf, os, runparams);
	os << "</footnote>";

	return i;
}


} // namespace lyx
