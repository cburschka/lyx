/**
 * \file tostr.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include <boost/lexical_cast.hpp>


using boost::lexical_cast;

using std::string;


string const tostr(bool b)
{
	return (b ? "true" : "false");
}


string const tostr(unsigned int i)
{
	return lexical_cast<string>(i);
}


string const tostr(long int i)
{
	return lexical_cast<string>(i);
}


string const tostr(double d)
{
	return lexical_cast<string>(d);
}


string const tostr(int i)
{
	return lexical_cast<string>(i);
}


string const tostr(string const & s)
{
	return s;
}


string const tostr(long unsigned int i)
{
	return lexical_cast<string>(i);
}
