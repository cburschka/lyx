#include <config.h>

#include <sys/types.h>
#include <csignal>

#include "lyxlib.h"

#ifndef CXX_GLOBAL_CSTD
using std::kill;
#endif

int lyx::kill(int pid, int sig)
{
	return ::kill(pid, sig);
}
