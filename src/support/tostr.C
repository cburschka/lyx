#include <config.h>
 
#include "Lsstream.h"
#include "LString.h"


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
 
