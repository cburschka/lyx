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
 *  This is a wrapper program to start lyx under windows hiding its
 *  console window. It is adapted from program hidec at
 *  http://www.msfn.org/board/index.php?showtopic=49184&mode=threaded
 *
 *  This wrapper should be named lyx.exe and placed in the same directory
 *  as the real lyx executable which _must_ be renamed as lyxc.exe
 *
 *  Usage: 
 *      hidecmd [VAR=val ...] [<params>]
 *  where:
 *         VAR=val    set VAR=val (multiple settings may be specified)
 *         <params>   parameters for the real lyx executable
 *
 *  How to build this program:
 *    msvc:
 *	    cl.exe hidecmd.c /GA /O1 /Felyx.exe /link /subsystem:windows \
 *	           kernel32.lib advapi32.lib user32.lib libcmt.lib
 *    mingw/gcc:
 *   	gcc -mwindows hidecmd.c -o lyx.exe
 *
 */

#include <process.h>
#include <windows.h>

#ifdef _MSC_VER
//
// Using msvc, the following pragmas can reduce executable size from
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
	DWORD exitcode = 0;
	char delim = ' ';
	char * var;
	char * val;
	// two " are needed here: the first one quotes the entire
	// command, the second one the executable name
	char cmd[1024] = "cmd /c \"\"";
	//  i0 = strlen(cmd);
	int i0 = 9;
	int i;
	int err = 0;
	int inquote;
	char * cmdLine = GetCommandLine();
	// the name of the executable to be launched
	// (must be in the same directory)
	char * lyxc = "lyxc.exe";

	// When using GetCommandLine(), command name is included
	// but the full path may be missing, so skip it.
	if (*cmdLine == '\"') {
		delim = '\"';
		cmdLine++;
	}

	while (*cmdLine != delim && *cmdLine != 0)
		cmdLine++;

	if (*cmdLine == delim)
		cmdLine++;

	// skip over ' ' or '\t'
	while (*cmdLine != 0 && (*cmdLine == ' ' || *cmdLine == '\t'))
		cmdLine++; 

	// Use GetModuleFileName() to get the path to lyxc.exe
	GetModuleFileName(0, cmd + i0, sizeof(cmd) / 2);

	// substitute executable name
	for (i = i0; cmd[i] != 0; ++i);

	for (--i; i >= i0 && cmd[i] != '\\' && cmd[i] != '/'; --i);

	for (++i; *lyxc && i < sizeof(cmd); )
		cmd[i++] = *lyxc++;

	// check whether lyxc.exe is there
	cmd[i] = '\0';
	lyxc = cmd + i0;
	if (GetFileAttributesA(lyxc) == 0xFFFFFFFF) {
		exitcode = GetLastError();
		MessageBox(0, lyxc, "Error: cannot find the real LyX executable below", 0);
		ExitProcess(exitcode);
	}

	// it's there, so finish quoting filename
	cmd[i++] = '\"';

	// parse remainder of command line
	while (*cmdLine != 0 && !err) {
		if (i < sizeof(cmd))
			cmd[i++] = ' ';
		else
			err = 1;

		if ((*cmdLine >= 'A' && *cmdLine <= 'Z') ||
		    (*cmdLine >= 'a' && *cmdLine <= 'z'))
			var = cmd + i;
		else
			var = NULL;

		val = NULL;
		inquote = 0;
		while (!err && ((*cmdLine != 0 && *cmdLine != ' ' &&
				*cmdLine != '\t') || inquote))
		{
			if (*cmdLine == '\"')
				inquote = 1 - inquote;

			if (var && *cmdLine == '=' && !inquote)
				val = cmd + i;

			if (i < sizeof(cmd))
				cmd[i++] = *cmdLine;
			else
				err = 1;

			cmdLine++;
		}

		if (var && val && !err) {
			*val++ = '\0';		// mark end of var
			if (*val == '\"') {	// account for quoted val
				++val;
				--i;
			}
			cmd[i] = '\0';		// mark end of val
			i = var - cmd - 1;	// reset pointer
			SetEnvironmentVariable(var, val);
			// MessageBox(0, val, var, 0);
		}

		// skip spaces
		while (*cmdLine != 0 && (*cmdLine == ' ' || *cmdLine == '\t')) 
			cmdLine++;
	}

	if (i < sizeof(cmd) - 1) {
		// finish quoting the entire command
		cmd[i++] = '\"';
		cmd[i] = '\0';
	} else
		err = 1;
	
	if (err) {
		MessageBox(0, "Please, use a shorter command line.",
				"Error: command line is too long", 0);
		ExitProcess(0);
	}

	// create process with new console
	// memset(&si, 0, sizeof(si));
	val = (char *) &si;
	for (i = 0; i < sizeof(si); ++i)
		val[i] = 0x00;
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	if (CreateProcess(NULL, cmd,
		NULL, NULL, FALSE, CREATE_NEW_CONSOLE,
		NULL, NULL, &si, &pi))
	{
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
	}
	else
		exitcode = GetLastError();

	/* terminate this */
	ExitProcess(exitcode);
}
