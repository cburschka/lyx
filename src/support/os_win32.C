/**
 * \file os_win32.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Ruurd A. Reitsma
 *
 * Full author contact details are available in file CREDITS.
 *
 * Various OS specific functions
 */

#include <config.h>

#include "support/os.h"

#include "debug.h"

#include <windows.h>
#include <io.h>

#if defined(__CYGWIN__) || defined(__CYGWIN32__)
# include <sys/cygwin.h>

#elif defined(_WIN32)
# include <direct.h> // _getdrive
#endif

using std::endl;
using std::string;


namespace lyx {
namespace support {
namespace os {

void init(int, char *[])
{}


string current_root()
{
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	return string("/");

#else
	// _getdrive returns the current drive (1=A, 2=B, and so on).
	char const drive = ::_getdrive() + 'A' - 1;
	return string(1, drive) + ":/";
#endif
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


string external_path(string const & p)
{
	string dos_path;

#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	// Translate from cygwin path syntax to dos path syntax
	if (is_absolute_path(p)) {
		char dp[PATH_MAX];
		cygwin_conv_to_full_win32_path(p.c_str(), dp);
		dos_path = !dp ? "" : dp;
	}

	else return p;
#else // regular Win32
	dos_path = p;
#endif
	
	//No backslashes in LaTeX files
	dos_path = subst(dos_path,'\\','/');

	lyxerr[Debug::LATEX]
		<< "<Win32 path correction> ["
		<< p << "]->>["
		<< dos_path << ']' << endl;
	return dos_path;
}


// (Claus H.) Parsing the latex log file in an Win32 environment all
// files are mentioned in Win32/DOS syntax. Because LyX uses the dep file
// entries to check if any file has been changed we must retranslate
// the Win32/DOS pathnames into Cygwin pathnames.
string internal_path(string const & p)
{
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	char posix_path[PATH_MAX];
	posix_path[0] = '\0';
	cygwin_conv_to_posix_path(p.c_str(), posix_path);
	return posix_path;
#else
	return subst(p,"\\","/");
#endif
}


// (Claus H.) On Win32 both Unix and Win32/DOS pathnames are used.
// Therefore an absolute path could be either a pathname starting
// with a slash (Unix) or a pathname starting with a drive letter
// followed by a colon. Because a colon is not valid in pathes in Unix
// and at another location in Win32 testing just for the existance
// of the colon in the 2nd position seems to be enough!
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
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	static string const nulldev_ = "/dev/null";
#else
	static string const nulldev_ = "nul";
#endif
	return nulldev_;
}


shell_type shell()
{
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	return UNIX;
#else
	return CMD_EXE;
#endif
}

} // namespace os
} // namespace support
} // namespace lyx
