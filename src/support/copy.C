#include <config.h>

#include <stdio.h>

#include "support/lyxlib.h"
#include "LString.h"
#include "support/syscall.h"
#include "support/filetools.h"

bool lyx::copy(char const * from, char const * to)
{
	string command = "cp " + QuoteName(from) + " " + QuoteName(to);
	return Systemcalls().startscript(Systemcalls::System,
					 command) == 0;
}
