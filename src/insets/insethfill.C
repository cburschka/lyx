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

#include "support/std_ostream.h"

using std::ostream;


InsetHFill::InsetHFill()
	: InsetCommand(InsetCommandParams("hfill"), std::string())
{}


std::auto_ptr<InsetBase> InsetHFill::clone() const
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


std::string const InsetHFill::getScreenLabel(Buffer const &) const
{
	return getContents();
}


int InsetHFill::latex(Buffer const &, ostream & os,
		      OutputParams const &) const
{
	os << getCommand();
	return 0;
}


int InsetHFill::plaintext(Buffer const &, ostream & os,
		      OutputParams const &) const
{
	os << '\t';
	return 0;
}


int InsetHFill::linuxdoc(Buffer const &, std::ostream & os,
			 OutputParams const &) const
{
	os << '\n';
	return 0;
}


int InsetHFill::docbook(Buffer const &, std::ostream & os,
			OutputParams const &) const
{
	os << '\n';
	return 0;
}


void InsetHFill::write(Buffer const &, ostream & os) const
{
	os << "\n\\hfill \n";
}
