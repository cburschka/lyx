#include <config.h>

#include "lyxlib.h"

#include <sys/types.h>
#include <csignal>

int lyx::kill(int pid, int sig)
{
	return ::kill(pid, sig);
}
