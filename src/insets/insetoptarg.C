/**
 * \file insetoptarg.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetoptarg.h"

#include "debug.h"
#include "gettext.h"
#include "LColor.h"
#include "paragraph.h"


using std::string;
using std::auto_ptr;
using std::ostream;


InsetOptArg::InsetOptArg(BufferParams const & ins)
	: InsetCollapsable(ins, Collapsed)
{
	LyXFont font(LyXFont::ALL_SANE);
	font.setColor(LColor::collapsable);
	setLabelFont(font);
	setLabel(_("opt"));
}


InsetOptArg::InsetOptArg(InsetOptArg const & in)
	: InsetCollapsable(in)
{
	LyXFont font(LyXFont::ALL_SANE);
	font.setColor(LColor::collapsable);
	setLabelFont(font);
	setLabel(_("opt"));
}


auto_ptr<InsetBase> InsetOptArg::clone() const
{
	return auto_ptr<InsetBase>(new InsetOptArg(*this));
}


string const InsetOptArg::editMessage() const
{
	return _("Opened Optional Argument Inset");
}


void InsetOptArg::write(Buffer const & buf, ostream & os) const
{
	os << "OptArg" << "\n";
	InsetCollapsable::write(buf, os);
}


int InsetOptArg::latex(Buffer const &, ostream &,
		       OutputParams const &) const
{
	return 0;
}


int InsetOptArg::latexOptional(Buffer const & buf, ostream & os,
			       OutputParams const & runparams) const
{
	os << '[';
	int const i = InsetText::latex(buf, os, runparams);
	os << ']';
	return i + 2;
}
