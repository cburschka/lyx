#include <config.h>

#include "support/lyxlib.h"
#include "support/filetools.h"
#include "gettext.h"

string const lyx::getUserName()
{
	string userName(GetEnv("LOGNAME"));
	if (userName.empty())
		userName = GetEnv("USER");
	if (userName.empty())
		userName = _("unknown");
	return userName;
}
