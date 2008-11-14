/**
 * \file kill.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lyxlib.h"

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#include <csignal>

#ifdef _WIN32
#include "support/debug.h"
#include "support/os.h"

#include <windows.h>
#include <cerrno>
#endif //_WIN32

namespace lyx {

int support::kill(int pid, int sig)
{
#ifdef _WIN32
	if (pid == (int)GetCurrentProcessId())
		return -raise(sig);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);
	if (!hProcess) {
		LYXERR0("kill OpenProcess failed!");
		return -1;
	}
	if (!TerminateProcess(hProcess, sig)) {
		LYXERR0("kill process failed!");
		CloseHandle(hProcess);
		return -1;
	}
	CloseHandle(hProcess);
	return 0;
#else
	return ::kill(pid, sig);
#endif
}


} // namespace lyx
