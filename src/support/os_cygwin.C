// os_win32.C copyright "Ruurd A. Reitsma" <rareitsma@yahoo.com>

// Various OS specific functions
#include <config.h>

#include "os.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "debug.h"

#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <sys/cygwin.h>
#include <cstdlib>


string const os::nulldev_ = "/dev/null";
os::shell_type os::shell_ = os::UNIX;


void os::init(int, char *[])
{}


string os::current_root()
{
	return string("/");
}


string::size_type os::common_path(string const &p1, string const &p2)
{
	string::size_type i = 0,
			p1_len = p1.length(),
			p2_len = p2.length();
	while (i < p1_len && i < p2_len && uppercase(p1[i]) == uppercase(p2[i])) ++i;
	if ((i < p1_len && i < p2_len)
	    || (i < p1_len && p1[i] != '/' && i == p2_len)
	    || (i < p2_len && p2[i] != '/' && i == p1_len)) {
		if (i) --i;     // here was the last match
		while (i && p1[i] != '/') --i;
	}
	return i;
}


namespace {

bool cygwin_path_fix_ = false;

} // namespace anon


string os::external_path(string const & p)
{
	string dos_path;

	// Translate from cygwin path syntax to dos path syntax
	if (cygwin_path_fix_ && is_absolute_path(p)) {
		char dp[PATH_MAX+1];
		cygwin_conv_to_full_win32_path(p.c_str(), dp);
		dos_path = !dp ? string() : dp;
	} else
		return p;

	// No backslashes in LaTeX files
	dos_path = subst(dos_path,'\\','/');

	lyxerr[Debug::LATEX]
		<< "<Cygwin path correction> ["
		<< p << "]->>["
		<< dos_path << ']' << std::endl;
	return dos_path;
}


string os::internal_path(string const & p)
{
	char posix_path[PATH_MAX+1];
	posix_path[0] = '\0';
	cygwin_conv_to_posix_path(p.c_str(), posix_path);
	return posix_path;
}


// (Claus H.) On Win32 both Unix and Win32/DOS pathnames are used.
// Therefore an absolute path could be either a pathname starting
// with a slash (Unix) or a pathname starting with a drive letter
// followed by a colon. Because a colon is not valid in pathes in Unix
// and at another location in Win32 testing just for the existance
// of the colon in the 2nd position seems to be enough!
bool os::is_absolute_path(string const & p)
{
	bool const isDosPath  = (p.size() > 1 && p[1] == ':');
	bool const isUnixPath = (p.size() > 0 && p[0] == '/');

	return isDosPath | isUnixPath;
}


// returns a string suitable to be passed to fopen when
// reading a file
char const * os::read_mode()
{
	return "rb";
}


// returns a string suitable to be passed to popen when
// reading a pipe
char const * os::popen_read_mode()
{
	return "r";
}


// The character used to separate paths returned by the
//  PATH environment variable.
char os::path_separator()
{
	return ':';
}


void cygwin_path_fix(bool use_cygwin_paths)
{
	cygwin_path_fix_ = use_cygwin_paths;
}
