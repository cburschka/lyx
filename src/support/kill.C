#include <config.h>

#include <sys/types.h>
#include <signal.h>

#include "lyxlib.h"

int lyx::kill(int pid, int sig)
{
	return ::kill(pid, sig);
}
