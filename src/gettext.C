#include <config.h>

#include "LString.h"
#include "gettext.h"

#ifdef ENABLE_NLS

char const * _(char const * str)
{
	return gettext(str);
}


string const _(string const & str) 
{
	int const s = str.length();
	char * tmp = new char[s + 1];
	str.copy(tmp, s);
	tmp[s] = '\0';
	string ret(gettext(tmp));
	delete [] tmp;
	return ret;
}

#endif
