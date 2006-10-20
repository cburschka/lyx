/**
 * \file insethfill.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insethfill.h"

#include "gettext.h"

#include "support/std_ostream.h"

using lyx::docstring;
using lyx::odocstream;

using std::ostream;


InsetHFill::InsetHFill()
	: InsetCommand(InsetCommandParams("hfill"), std::string())
{}


std::auto_ptr<InsetBase> InsetHFill::doClone() const
{
	return std::auto_ptr<InsetBase>(new InsetHFill);
}


void InsetHFill::metrics(MetricsInfo &, Dimension & dim) const
{
	dim.wid = 3;
	dim.asc = 3;
	dim.des = 3;
	dim_ = dim;
}


docstring const InsetHFill::getScreenLabel(Buffer const &) const
{
	return _("Horizontal Fill");
}


int InsetHFill::latex(Buffer const &, odocstream & os,
		      OutputParams const &) const
{
	os << getCommand() << "{}";
	return 0;
}


int InsetHFill::plaintext(Buffer const &, odocstream & os,
		      OutputParams const &) const
{
	os << '\t';
	return 0;
}


int InsetHFill::docbook(Buffer const &, odocstream & os,
			OutputParams const &) const
{
	os << '\n';
	return 0;
}


void InsetHFill::write(Buffer const &, ostream & os) const
{
	os << "\n\\hfill\n";
}


bool InsetHFill::isSpace() const
{
	return true;
}
