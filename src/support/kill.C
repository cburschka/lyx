#include <config.h>

#include <sys/types.h>
#include <signal.h>

#ifdef CXX_WORKING_NAMESPACES
namespace lyx {
	int kill(int pid, int sig)
	{
		return ::kill(pid, sig);
	}
}
#else
#include "lyxlib.h"

int lyx::kill(int pid, int sig)
{
	return ::kill(pid, sig);
}
#endif
