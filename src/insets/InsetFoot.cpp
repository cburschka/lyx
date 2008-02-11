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
#include "support/debug.h"

#include "InsetFoot.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Counters.h"
#include "support/gettext.h"
#include "Layout.h"
// FIXME: the following is needed just to get the layout of the enclosing
// paragraph. This seems a bit too much to me (JMarc)
#include "OutputParams.h"
#include "ParIterator.h"
#include "TextClass.h"
#include "TocBackend.h"

#include "support/lstrings.h"

#include <ostream>

using namespace std;

namespace lyx {


InsetFoot::InsetFoot(BufferParams const & bp)
	: InsetFootlike(bp)
{}


InsetFoot::InsetFoot(InsetFoot const & in)
	: InsetFootlike(in)
{}


Inset * InsetFoot::clone() const
{
	return new InsetFoot(*this);
}


docstring const InsetFoot::editMessage() const
{
	return _("Opened Footnote Inset");
}


void InsetFoot::updateLabels(Buffer const & buf, ParIterator const & it)
{
	TextClass const & tclass = buf.params().getTextClass();
	Counters & cnts = tclass.counters();
	docstring const & foot = from_ascii("footnote");
	Paragraph const & outer =  it.paragraph();
	if (!outer.layout()->intitle && cnts.hasCounter(foot)) {
		cnts.step(foot);
		//FIXME: the counter should format itself.
		setLabel(support::bformat(from_ascii("%1$s %2$s"), 
					  getLayout(buf.params()).labelstring, 
					  cnts.theCounter(foot)));
	
	}
	InsetCollapsable::updateLabels(buf, it);
}


void InsetFoot::addToToc(TocList & toclist, Buffer const & buf, ParConstIterator const &) const
{
	ParConstIterator pit = par_const_iterator_begin(*this);

	Toc & toc = toclist["footnote"];
	// FIXME: we probably want the footnote number too.
	docstring str;
	str = getNewLabel(str);
	toc.push_back(TocItem(pit, 0, str));
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
