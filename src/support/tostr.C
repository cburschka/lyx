/**
 * \file tostr.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/std_sstream.h"
#include "support/std_string.h"

using std::ostringstream;


string const tostr(bool b)
{
	return (b ? "true" : "false");
}


string const tostr(unsigned int i)
{
	ostringstream os;
	os << i;
	return STRCONV(os.str());
}


string const tostr(long int i)
{
	ostringstream os;
	os << i;
	return STRCONV(os.str());
}


string const tostr(double d)
{
	ostringstream os;
	os << d;
	return STRCONV(os.str());
}


string const tostr(int i)
{
	ostringstream os;
	os << i;
	return STRCONV(os.str());
}


string const tostr(string const & s)
{
	return s;
}

