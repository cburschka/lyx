#include <config.h>

#include <sys/types.h>
#include <csignal>

#include "lyxlib.h"

int lyx::kill(int pid, int sig)
{
	return ::kill(pid, sig);
}
