/**
 * \file kill.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lyxlib.h"

#include <sys/types.h>
#include <csignal>

#ifdef _WIN32
#include "debug.h"
#include "os.h"

#include <windows.h>
#include <errno.h>

using std::endl;
#endif //_WIN32

int lyx::support::kill(int pid, int sig)
{
#ifdef _WIN32
	if (pid == (int)GetCurrentProcessId()) {
		return -(raise(sig));
	} else {
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);
		if (!hProcess) {
			lyxerr << "kill OpenProcess failed!" << endl;
			return -1;
		} else {
			if (!TerminateProcess(hProcess, sig)){
				lyxerr << "kill process failed!" << endl;
				CloseHandle(hProcess);
				return -1;
			}
		CloseHandle(hProcess);
		}
	}
	return 0;

#else
	return ::kill(pid, sig);
#endif
}
