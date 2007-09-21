/**
 * \file InsetHFill.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetHFill.h"
#include "gettext.h"

#include "support/std_ostream.h"


namespace lyx {


InsetHFill::InsetHFill()
	: InsetCommand(InsetCommandParams("hfill"), std::string())
{}


Inset * InsetHFill::clone() const
{
	return new InsetHFill;
}


void InsetHFill::metrics(MetricsInfo &, Dimension & dim) const
{
	dim.wid = 3;
	dim.asc = 3;
	dim.des = 3;
}


docstring const InsetHFill::getScreenLabel(Buffer const &) const
{
	return _("Horizontal Fill");
}


int InsetHFill::plaintext(Buffer const &, odocstream & os,
			  OutputParams const &) const
{
	os << "     ";
	return 5;
}


int InsetHFill::docbook(Buffer const &, odocstream & os,
			OutputParams const &) const
{
	os << '\n';
	return 0;
}


void InsetHFill::write(Buffer const &, std::ostream & os) const
{
	os << "\n\\hfill\n";
}


bool InsetHFill::isSpace() const
{
	return true;
}


} // namespace lyx
