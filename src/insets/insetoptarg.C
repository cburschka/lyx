/**
 * \file insetoptarg.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "debug.h"

#include "insetoptarg.h"
#include "support/LOstream.h"
#include "frontends/Alert.h"
#include "support/lstrings.h" // frontStrip, strip
#include "lyxtext.h"
#include "buffer.h"
#include "gettext.h"
#include "BufferView.h"
#include "support/lstrings.h"

using std::ostream;
using std::vector;
using std::pair;
using std::auto_ptr;


InsetOptArg::InsetOptArg(BufferParams const & ins)
	: InsetCollapsable(ins, true)
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


void InsetOptArg::write(Buffer const * buf, ostream & os) const
{
	os << "OptArg" << "\n";
	InsetCollapsable::write(buf, os);
}


int InsetOptArg::latex(Buffer const *, ostream &,
		       LatexRunParams const &) const
{
	return 0;
}


int InsetOptArg::latexOptional(Buffer const * buf, ostream & os,
			       LatexRunParams const & runparams) const
{
	os << '[';
	int const i = inset.latex(buf, os, runparams);
	os << ']';
	return i + 2;
}
