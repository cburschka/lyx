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
#include "support/LOstream.h"

using std::ostream;

InsetHFill::InsetHFill()
	: InsetCommand(InsetCommandParams("hfill"))
{}


int InsetHFill::latex(Buffer const &, ostream & os,
		      LatexRunParams const &) const
{
	os << getCommand();
	return 0;
}


int InsetHFill::ascii(Buffer const &, ostream & os, int) const
{
	os << '\t';
	return 0;
}


int InsetHFill::linuxdoc(Buffer const &, std::ostream & os) const
{
	os << '\n';
	return 0;
}


int InsetHFill::docbook(Buffer const &, std::ostream & os, bool) const
{
	os << '\n';
	return 0;
}

void InsetHFill::write(Buffer const &, ostream & os) const
{
	os << "\n\\hfill \n";
}
