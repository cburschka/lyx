/**
 * \file convert.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/convert.h"
#include "support/docstring.h"

#include <boost/lexical_cast.hpp>

#include <string>
#include <sstream>
//needed for Mac OSX 10.5.2 Leopard
#include <cstdlib>

using namespace std;

namespace lyx {

using boost::lexical_cast;

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
docstring convert<docstring>(int i)
{
	return from_ascii(lexical_cast<string>(i));
}


template<>
string convert<string>(unsigned int ui)
{
	return lexical_cast<string>(ui);
}


template<>
docstring convert<docstring>(unsigned int ui)
{
	return from_ascii(lexical_cast<string>(ui));
}


template<>
string convert<string>(unsigned long ul)
{
	return lexical_cast<string>(ul);
}


template<>
docstring convert<docstring>(unsigned long ul)
{
	return from_ascii(lexical_cast<string>(ul));
}


template<>
string convert<string>(long l)
{
	return lexical_cast<string>(l);
}


template<>
docstring convert<docstring>(long l)
{
	return from_ascii(lexical_cast<string>(l));
}


template<>
string convert<string>(float f)
{
	std::ostringstream val;
	val << f;
	return val.str();
}


template<>
string convert<string>(double d)
{
	std::ostringstream val;
	val << d;
	return val.str();
}


template<>
int convert<int>(string const s)
{
	return strtol(s.c_str(), 0, 10);
}


template<>
int convert<int>(docstring const s)
{
	return strtol(to_ascii(s).c_str(), 0, 10);
}


template<>
unsigned int convert<unsigned int>(string const s)
{
	return strtoul(s.c_str(), 0, 10);
}


template<>
unsigned long convert<unsigned long>(string const s)
{
	return strtoul(s.c_str(), 0, 10);
}


template<>
double convert<double>(string const s)
{
	return strtod(s.c_str(), 0);
}


template<>
int convert<int>(char const * cptr)
{
	return strtol(cptr, 0, 10);
}


template<>
double convert<double>(char const * cptr)
{
	return strtod(cptr, 0);
}


} // namespace lyx
