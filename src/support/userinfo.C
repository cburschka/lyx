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

#include "userinfo.h"
#include "LAssert.h"
#include "filetools.h"

#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

namespace lyx {
namespace support {

string const user_name()
{
	struct passwd * pw(getpwuid(geteuid()));
	Assert(pw);

	string name = pw->pw_gecos;
	if (name.empty())
		name = pw->pw_name;
	return name;
}


string const user_email()
{
	string email = GetEnv("EMAIL_ADDRESS");
	if (email.empty())
		email = GetEnv("EMAIL");
	return email;
}

} // namespace support
} // namespace lyx
