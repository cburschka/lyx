/*
 * \file lyx_path_prefix.C
 * This file is part of LyX, the document processor.
 * http://www.lyx.org/
 * Licence details can be found in the file COPYING or copy at
 * http://www.lyx.org/about/license.php3

 * \author Angus Leeming
 * Full author contact details are available in file CREDITS or copy at
 * http://www.lyx.org/about/credits.php
 *
 * This little piece of code is used to insert some code into LyX's
 * Resources/lyx/configure script so that it will cause lyxrc.defaults to
 * contain
 *
 * \path_prefix "<path to sh.exe>;<path to python.exe>;..."
 *
 * Compile the code with
 *
 * g++ -I/c/Program\ Files/NSIS/Contrib -Wall -shared \
 *    lyx_path_prefix.c -o lyx_path_prefix.dll
 *
 * Move resulting .dll to /c/Program\ Files/NSIS/Plugins
 */

#include <windows.h>
#include "ExDLL/exdll.h"
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>

namespace {

std::string const subst(std::string const & a,
			std::string const & oldstr,
			std::string const & newstr)
{
	std::string lstr = a;
	std::string::size_type i = 0;
	std::string::size_type const olen = oldstr.length();
	while ((i = lstr.find(oldstr, i)) != std::string::npos) {
		lstr.replace(i, olen, newstr);
		i += newstr.length(); // We need to be sure that we dont
		// use the same i over and over again.
	}
	return lstr;
}

} // namespace anon


BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}


extern "C"
void __declspec(dllexport) set(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
	char configure_file[MAX_PATH];
	char path_prefix[MAX_PATH];

	EXDLL_INIT();

	popstring(configure_file);
	popstring(path_prefix);

	std::fstream fs(configure_file);
	if (!fs) {
		pushstring("-1");
		return;
	}

	std::istreambuf_iterator<char> const begin(fs);
	std::istreambuf_iterator<char> const end;

	std::string configure_data(begin, end);
	std::string::size_type const xfonts_pos = configure_data.find("X FONTS");
	if (xfonts_pos == std::string::npos) {
		pushstring("-1");
		return;
	}

	std::string::size_type const xfonts_start =
		configure_data.find_last_of('\n', xfonts_pos);
	if (xfonts_start == std::string::npos) {
		pushstring("-1");
		return;
	}

	fs.seekg(0);
	fs << configure_data.substr(0, xfonts_start)
	   << "\n"
	      "cat >>$outfile <<EOF\n"
	      "\n"
	      "\\\\path_prefix \"" << subst(path_prefix, "\\", "\\\\") << "\"\n"
	      "EOF\n"
	   << configure_data.substr(xfonts_start);

	// Now we've rebuilt configure, run it to generate things like
	// lyxrc.defaults.
	std::string configure_dir(configure_file);
	std::string::size_type const final_slash = configure_dir.find_last_of('\\');
	if (final_slash == std::string::npos) {
		pushstring("-1");
		return;
	}
	configure_dir = configure_dir.substr(0, final_slash);
	if (SetCurrentDirectory(configure_dir.c_str()) == 0) {
		pushstring("-1");
		return;
	}

	char path[MAX_PATH];
	if (GetEnvironmentVariable("PATH", path, MAX_PATH) == 0) {
		pushstring("-1");
		return;
	}

	std::string const path_str = std::string(path_prefix) + ';' + path;
	if (SetEnvironmentVariable("PATH", path_str.c_str()) == 0) {
		pushstring("-1");
		return;
	}

	if (system("start /min sh.exe configure") != 0) {
		pushstring("-1");
		return;
	}

	pushstring("0");
}
