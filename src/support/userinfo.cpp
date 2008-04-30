/**
 * \file userinfo.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/userinfo.h"
#include "support/environment.h"
#include "support/docstring.h"

#include "support/lassert.h"

#if defined (_WIN32)
# include "support/gettext.h"
# include <windows.h>
# include <lmcons.h>
#else
# include <pwd.h>
# ifdef HAVE_UNISTD_H
#  include <unistd.h>
# endif
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

using namespace std;

namespace lyx {
namespace support {

docstring const user_name()
{
#if defined (_WIN32)

	char name[UNLEN + 1];
	DWORD size = UNLEN + 1;
	if (!GetUserName(name, &size))
		return _("Unknown user");
	return from_local8bit(name);
#else
	struct passwd * pw = getpwuid(geteuid());
	LASSERT(pw, /**/);

	string name = pw->pw_gecos;
	if (name.empty())
		name = pw->pw_name;
	return from_local8bit(name);
#endif
}


docstring const user_email()
{
	//FIXME: quick fix wrt bug #3764; only Anonymous is detected now.
	//The code after should be used only after user approval.
	return docstring();

	
	string email = getEnv("EMAIL_ADDRESS");
	if (email.empty())
		email = getEnv("EMAIL");
	return from_local8bit(email);
}

} // namespace support
} // namespace lyx
