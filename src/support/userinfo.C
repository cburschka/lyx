/**
 * \file userinfo.C
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

#include <boost/assert.hpp>

#if defined (_WIN32)
# include "gettext.h"
# include <lmcons.h>
# include <windows.h>
#else
# include <pwd.h>
# include <unistd.h>
#endif
#include <sys/types.h>

using std::string;

namespace lyx {
namespace support {

string const user_name()
{
#if defined (_WIN32)

	char name[UNLEN + 1];
	DWORD size = UNLEN + 1;
	if (!GetUserName(name, &size))
		return _("Unknown user");
	return name;
#else
	struct passwd * pw(getpwuid(geteuid()));
	BOOST_ASSERT(pw);

	string name = pw->pw_gecos;
	if (name.empty())
		name = pw->pw_name;
	return name;
#endif
}


string const user_email()
{
	string email = getEnv("EMAIL_ADDRESS");
	if (email.empty())
		email = getEnv("EMAIL");
	return email;
}

} // namespace support
} // namespace lyx
