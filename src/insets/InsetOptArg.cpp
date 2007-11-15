/**
 * \file InsetOptArg.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetOptArg.h"

#include "FuncRequest.h"
#include "FuncStatus.h"

#include "debug.h"
#include "gettext.h"


namespace lyx {

using std::string;
using std::auto_ptr;
using std::ostream;


InsetOptArg::InsetOptArg(BufferParams const & ins)
	: InsetCollapsable(ins)
{
	Font font(Font::ALL_SANE);
	font.setColor(Color::collapsable);
	setLabelFont(font);
	setLabel(_("opt"));
}


InsetOptArg::InsetOptArg(InsetOptArg const & in)
	: InsetCollapsable(in)
{
	Font font(Font::ALL_SANE);
	font.setColor(Color::collapsable);
	setLabelFont(font);
	setLabel(_("opt"));
}


auto_ptr<Inset> InsetOptArg::doClone() const
{
	return auto_ptr<Inset>(new InsetOptArg(*this));
}


docstring const InsetOptArg::editMessage() const
{
	return _("Opened Optional Argument Inset");
}


bool InsetOptArg::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
		// paragraph breaks not allowed
		case LFUN_BREAK_PARAGRAPH:
		case LFUN_BREAK_PARAGRAPH_KEEP_LAYOUT:
		case LFUN_BREAK_PARAGRAPH_SKIP:
			status.enabled(false);
			return true;

		default:
			return InsetCollapsable::getStatus(cur, cmd, status);
		}
}


void InsetOptArg::write(Buffer const & buf, ostream & os) const
{
	os << "OptArg" << "\n";
	InsetCollapsable::write(buf, os);
}


int InsetOptArg::latex(Buffer const &, odocstream &,
		       OutputParams const &) const
{
	return 0;
}


int InsetOptArg::plaintext(Buffer const &, odocstream &,
			   OutputParams const &) const
{
	return 0; // do not output optional arguments
}


int InsetOptArg::docbook(Buffer const &, odocstream &,
			 OutputParams const &) const
{
	return 0;
}


int InsetOptArg::latexOptional(Buffer const & buf, odocstream & os,
			       OutputParams const & runparams) const
{
	odocstringstream ss;
	int ret = InsetText::latex(buf, ss, runparams);
	docstring str = ss.str();
	if (str.find(']') != docstring::npos)
		str = '{' + str + '}';
	os << '[' << str << ']';
	return ret;
}


} // namespace lyx
