// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef ENABLE_NLS

#include "LString.h"

#  if HAVE_GETTEXT
#    include <libintl.h>      // use the header already in the system *EK*
#    ifdef HAVE_LOCALE_H
#      include <locale.h>        // for LC_MESSAGES
#    endif
#  else
#    include "../intl/libintl.h"
#  endif

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

void locale_init()
{
#  ifdef HAVE_LC_MESSAGES
	setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "");
	setlocale(LC_NUMERIC, "C");
#  endif
}

void gettext_init(string const & localedir)
{
	bindtextdomain(PACKAGE, localedir.c_str()); 
	textdomain(PACKAGE);
}


#else // ENABLE_NLS

void locale_init()
{
}

void gettext_init(string const &)
{
}
#endif
