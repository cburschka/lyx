#include <config.h>

#include "LString.h"
#include "gettext.h"

#ifdef ENABLE_NLS

char const * _(char const * str)
{
	// I'd rather have an Assert on str, we should not allow
	// null pointers here. Lgb
	// Assert(str);
	if (str && str[0])
		return gettext(str);
	else
		return "";
}


string const _(string const & str) 
{
	if (!str.empty()) {
		int const s = str.length();
		char * tmp = new char[s + 1];
		str.copy(tmp, s);
		tmp[s] = '\0';
		string ret(gettext(tmp));
		delete [] tmp;
		return ret;
	}
	else
		return string();
}

#endif
