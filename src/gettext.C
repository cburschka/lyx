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

boost::scoped_ptr<Messages> lyx_messages;

} // anon namespace


char const * _(char const * str)
{
	// This breaks pretty much immediately
	// lyx::Assert(str && str[0]);

	if (!lyx_messages.get())
		return str;

	return lyx_messages->get(str).c_str();
}


string const _(string const & str)
{
	// This breaks pretty much immediately
	// lyx::Assert(!str.empty());

	if (!lyx_messages.get())
		return str;

	return lyx_messages->get(str);
}


void gettext_init(string const & localedir)
{
	lyx_messages.reset(new Messages("", localedir));
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
