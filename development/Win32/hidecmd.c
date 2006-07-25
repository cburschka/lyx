/* -*- C -*- */
/**
 * \file hidecmd.c
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author hasherfrog
 * \author Bo Peng
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

/**
 *  This utility function is used to start lyx under windows, but
 *  hide the console window. It is adapted from program hidec at
 *  http://www.msfn.org/board/index.php?showtopic=49184&mode=threaded
 *
 *  Usage: 
 *	hidecmd [/w] [/VAR=val] <filename> [<params>]
 *  where: /w		 wait for program termination
 *		/VAR=val	set VAR=val
 *		<filename>  executable program
 *		<params>	program parameters
 *
 *  How to built this program:
 *  msvc:
 *	cl.exe hidecmd.c /GA /O1 /link /subsystem:windows \
 *	   kernel32.lib advapi32.lib user32.lib libcmt.lib
 *  mingw/gcc:
 *	gcc -mno-cygwin -mwindows hidecmd.c -o hidecmd
 *
 */

#include <process.h>
#include <windows.h>


char * usage = "hidecmd [/w] [/VAR=val] <filename> [<params>]\n"
	"  where: /w		 wait for program termination\n"
	"		/VAR=val	set VAR=val\n"
	"		<filename>  executable program\n"
	"		<params>	program parameters\n";

#ifdef _MSC_VER
//
// Using msvc, the following progma can reduce executable size from
// 44k to 6k. I am not sure if mingw/gcc can take advantage of them
// though.
//
// do not link to default libraries
#pragma comment(linker,"/NODEFAULTLIB")
// unite code and data section (make the program smaller)
#pragma comment(linker,"/MERGE:.rdata=.text")
// resolve record in section of code
#pragma comment(linker,"/SECTION:.text,EWR")
// the new entry point (the WinMain entry point is big)
#pragma comment(linker,"/ENTRY:NewWinMain")

void NewWinMain(void)
#else  // mingw/gcc uses this entry point
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpszCmd, int nCmd)
#endif
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	int bWait = 0;
	DWORD exitcode = 0;
	char delim = ' ';
	char var[128];
	char val[128];
	int err = 0;
	char * cmdLine = GetCommandLine();
	int i;
	// start and end of variable/value
	char * s;
	char * e;

	// use GetCommandLine(), command name is included. Skip it
	if (*cmdLine == '\"') {
		delim = '\"';
		cmdLine++;
	}

	while (*cmdLine != delim && *cmdLine != 0)
		cmdLine++;

	if (*cmdLine == delim)
		cmdLine++;

	// skip over ' ' or '\t'
	while ((*cmdLine != 0) && ((*cmdLine == ' ') || (*cmdLine == '\t')))
		cmdLine++; 

	while (*cmdLine == '/') {
		// /w or /W option
		if (((cmdLine[1] == 'w') || (cmdLine[1] == 'W')) &&
			(cmdLine[2] == ' '))
		{
			bWait = 1;
			cmdLine += 3;
		// environment variable
		} else {
			cmdLine++;

			// get var
			s = var;
			e = s + sizeof(var) - 1;
			
			while (*cmdLine != 0 && *cmdLine != '=') {
				if (s < e) {
					*s++ = *cmdLine++;
				} else {
					cmdLine++;
					err = 1;
				}
			}

			// get value
			*s = 0;
			if (*cmdLine == '=')
				cmdLine++;

			delim = ' ';
			
			if (*cmdLine == '\"') {
				delim = '\"';
				cmdLine++;
			}

			s = val;
			e = s + sizeof(val) - 1;
			
			while (*cmdLine != delim && *cmdLine != 0) {
				if (s < e) {
					*s++ = *cmdLine++;
				} else {
					cmdLine++;
					err = 1;
				}
			}
			*s = 0;
			if (*cmdLine == delim)
				cmdLine++;

			SetEnvironmentVariable(var, val);
			// MessageBox(0, val, var, 0);
		}

		// skip spaces
		while ((*cmdLine != 0) && ((*cmdLine == ' ') || (*cmdLine == '\t'))) 
			cmdLine++;
	}

	// report error if there is no parameter 
	if (*cmdLine == 0)
	{
		MessageBox(0, usage, "Error: Incorrect usage", 0);
		ExitProcess(0);
	}
	
	if (err) {
		MessageBox(0, "One of the specified environment variables or its value is too long.", "Error: Variable name or value too long", 0);
		ExitProcess(0);
	}

	// create process with new console
	// memset(&si, 0, sizeof(si));
	s = (char *) &si;
	for (i = 0; i < sizeof(si); ++i)
		s[i] = 0x00;
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	if (CreateProcess(NULL, cmdLine,
		NULL, NULL, FALSE, CREATE_NEW_CONSOLE,
		NULL, NULL, &si, &pi))
	{
		if (bWait) 
			WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
	}
	else
		exitcode = GetLastError();

	/* terminate this */
	ExitProcess(exitcode);
}
