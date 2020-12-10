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

#include <string>
#include <sstream>
//needed for Mac OSX 10.5.2 Leopard
#include <cstdlib>

using namespace std;

namespace lyx {

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
	return to_string(sui);
}


template<>
string convert<string>(int i)
{
	return to_string(i);
}


template<>
docstring convert<docstring>(int i)
{
	return from_ascii(to_string(i));
}


template<>
string convert<string>(unsigned int ui)
{
	return to_string(ui);
}


template<>
docstring convert<docstring>(unsigned int ui)
{
	return from_ascii(to_string(ui));
}


template<>
string convert<string>(unsigned long ul)
{
	return to_string(ul);
}


template<>
docstring convert<docstring>(unsigned long ul)
{
	return from_ascii(to_string(ul));
}


#ifdef HAVE_LONG_LONG_INT
template<>
string convert<string>(unsigned long long ull)
{
	return to_string(ull);
}


template<>
docstring convert<docstring>(unsigned long long ull)
{
	return from_ascii(to_string(ull));
}


template<>
string convert<string>(long long ll)
{
	return to_string(ll);
}


template<>
docstring convert<docstring>(long long ll)
{
	return from_ascii(to_string(ll));
}


template<>
unsigned long long convert<unsigned long long>(string const s)
{
	return strtoull(s.c_str(), nullptr, 10);
}


/* not presently needed
template<>
long long convert<long long>(string const s)
{
	return strtoll(s.c_str(), nullptr, 10);
}
*/
#endif


template<>
string convert<string>(long l)
{
	return to_string(l);
}


template<>
docstring convert<docstring>(long l)
{
	return from_ascii(to_string(l));
}


template<>
string convert<string>(float f)
{
	ostringstream val;
	val << f;
	return val.str();
}


template<>
string convert<string>(double d)
{
	ostringstream val;
	val << d;
	return val.str();
}


template<>
docstring convert<docstring>(double d)
{
	return from_ascii(convert<string>(d));
}


template<>
int convert<int>(string const s)
{
	return int(strtol(s.c_str(), nullptr, 10));
}


int convert(string const & s, int base)
{
	return int(strtol(s.c_str(), nullptr, base));
}


template<>
int convert<int>(docstring const s)
{
	return int(strtol(to_ascii(s).c_str(), nullptr, 10));
}


template<>
unsigned int convert<unsigned int>(string const s)
{
	return static_cast<unsigned int>(strtoul(s.c_str(), nullptr, 10));
}


template<>
unsigned long convert<unsigned long>(string const s)
{
	return strtoul(s.c_str(), nullptr, 10);
}


template<>
double convert<double>(string const s)
{
	return strtod(s.c_str(), nullptr);
}


template<>
int convert<int>(char const * cptr)
{
	return int(strtol(cptr, nullptr, 10));
}


template<>
double convert<double>(char const * cptr)
{
	return strtod(cptr, nullptr);
}


} // namespace lyx
