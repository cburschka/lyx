/**
 * \file os_cygwin.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Ruurd A. Reitsma
 * \author Claus Hentschel
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * Various OS specific functions
 */

#include <config.h>

#include "support/os.h"
#include "support/lstrings.h"

#include "debug.h"

#include <windows.h>
#include <io.h>

#include <sys/cygwin.h>

using std::endl;
using std::string;


namespace lyx {
namespace support {
namespace os {

void os::init(int, char *[])
{}


string current_root()
{
	return string("/");
}


string::size_type common_path(string const & p1, string const & p2)
{
	string::size_type i = 0;
	string::size_type	p1_len = p1.length();
	string::size_type	p2_len = p2.length();
	while (i < p1_len && i < p2_len && uppercase(p1[i]) == uppercase(p2[i]))
		++i;
	if ((i < p1_len && i < p2_len)
	    || (i < p1_len && p1[i] != '/' && i == p2_len)
	    || (i < p2_len && p2[i] != '/' && i == p1_len))
	{
		if (i)
			--i;     // here was the last match
		while (i && p1[i] != '/')
			--i;
	}
	return i;
}


namespace {

bool cygwin_path_fix_ = false;

} // namespace anon


string external_path(string const & p)
{
	string dos_path;

	// Translate from cygwin path syntax to dos path syntax
	if (cygwin_path_fix_ && is_absolute_path(p)) {
		char dp[PATH_MAX];
		cygwin_conv_to_full_win32_path(p.c_str(), dp);
		dos_path = !dp ? "" : dp;
	}

	else return p;

	//No backslashes in LaTeX files
	dos_path = subst(dos_path,'\\','/');

	lyxerr[Debug::LATEX]
		<< "<Cygwin path correction> ["
		<< p << "]->>["
		<< dos_path << ']' << endl;
	return dos_path;
}


string internal_path(string const & p)
{
	char posix_path[PATH_MAX];
	posix_path[0] = '\0';
	cygwin_conv_to_posix_path(p.c_str(), posix_path);
	return posix_path;
}


bool is_absolute_path(string const & p)
{
	if (p.empty())
		return false;

	bool isDosPath = (p.length() > 1 && p[1] == ':');
	bool isUnixPath = (p[0] == '/');

	return isDosPath || isUnixPath;
}


// returns a string suitable to be passed to popen when
// reading a pipe
char const * popen_read_mode()
{
	return "r";
}


string const & nulldev()
{
	static string const nulldev_ = "/dev/null";
	return nulldev_;
}


shell_type shell()
{
	return UNIX;
}


char path_separator()
{
	return ':';
}


void cygwin_path_fix(bool use_cygwin_paths)
{
	use_cygwin_paths_ = use_cygwin_paths;
}

} // namespace os
} // namespace support
} // namespace lyx
