#include <config.h>

#include <sys/types.h>
#include <signal.h>

#ifdef CXX_WORKING_NAMESPACES
namespace lyx {
	int kill(pid_t pid, int sig)
	{
		return ::kill(pid, sig);
	}
}
#else
#include "lyxlib.h"

int lyx::kill(pid_t pid, int sig)
{
	return ::kill(pid, sig);
}
#endif
