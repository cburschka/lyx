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

#include "tostr.h"

#include <boost/lexical_cast.hpp>

using boost::lexical_cast;

using std::string;


template<>
string convert<string>(bool b)
{
	return (b ? "true" : "false");
}


template<>
string convert<string>(char c)
{
	return string(1, c);
}


template<>
string convert<string>(short unsigned int sui)
{
	return lexical_cast<string>(sui);
}


template<>
string convert<string>(int i)
{
	return lexical_cast<string>(i);
}


template<>
string convert<string>(unsigned int ui)
{
	return lexical_cast<string>(ui);
}


template<>
string convert<string>(float f)
{
	return lexical_cast<string>(f);
}


template<>
string convert<string>(double d)
{
	return lexical_cast<string>(d);
}
