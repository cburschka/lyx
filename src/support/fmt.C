#include <config.h>
#include <cstdio>
#include <cstdarg>

#include "LString.h"

/* This output manipulator gives the option to use Old style format
   specifications in ostreams. Note that this is done at the expense
   of typesafety, so if possible this manipulator should be avoided.
   When is it allowed to use this manipulator? I wrote it to be used
   i18n strings and gettext, and it should only(?) be used in that
   context.

   Ad. the implementation. I have only tested this on egcs-2.91.66 with
   glibc 2.1.2. So further testing is needed. The loop in fmt(...) will
   usually spin one or two times, but might spin more times with older
   glibc libraries, since the returned -1 when size is too small. Newer
   glibc returns the needed size.
   One problem can be that vsnprintf is not implemented on all archs,
   but AFAIK it is part of the new ANSI C standard.
   
   Lgb
*/

string fmt(char const * fmtstr ...)
{
	int size = 80;
	char * str = new char[size];
	va_list ap;
	while (true) {
		va_start(ap, fmtstr);
		int const r = ::vsnprintf(str, size, fmtstr, ap);
		va_end(ap);
		if (r == -1) { // size is too small
			delete [] str;
			size *= 2; // seems quite safe to double.
			str = new char[size];
		} else if (r >= size) { // r gives the needed size
			delete [] str;
			size += r;
			str = new char[size];
		} else {
			break;
		}
	}
	string res(str);
	delete [] str;
	return res;
}
