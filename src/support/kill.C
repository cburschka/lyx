#include <config.h>

#include <sys/types.h>
#include <signal.h>

#include "lyxlib.h"

int lyx::kill(pid_t pid, int sig)
{
	return ::kill(pid, sig);
}
