#include <config.h>

#include <cstdlib>

#include "lyxlib.h"

#if 0
using std::atoi;
#endif

int lyx::atoi(string const & nstr)
{
	return ::atoi(nstr.c_str());
}
