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

#include "messages.h"
#include "LString.h"
#include "support/LAssert.h"

#include <boost/scoped_ptr.hpp>

#ifdef HAVE_LOCALE_H
#  include <locale.h>
#endif

namespace {

Messages & getLyXMessages()
{
	static Messages lyx_messages;

	return lyx_messages;
}

} // anon namespace


char const * _(char const * str)
{
	return getLyXMessages().get(str).c_str();
}


string const _(string const & str)
{
	return getLyXMessages().get(str);
}


#ifdef ENABLE_NLS

void locale_init()
{
#  ifdef HAVE_LC_MESSAGES
	setlocale(LC_MESSAGES, "");
#  endif
	setlocale(LC_CTYPE, "");
	setlocale(LC_NUMERIC, "C");
}

#else // ENABLE_NLS

void locale_init()
{
	setlocale(LC_NUMERIC, "C");
}

#endif
