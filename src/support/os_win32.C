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
#include "support/filetools.h"
#include "support/lstrings.h"

#include "debug.h"

#include <windows.h>
#include <io.h>
#include <sys/cygwin.h>

using namespace lyx::support;
using std::endl;
using std::string;


namespace {

string binpath_;
string binname_;
string tmpdir_;

}


namespace lyx {
namespace support {
namespace os {

void init(int /* argc */, char * argv[])
{
	static bool initialized = false;
	if (initialized)
		return;
	initialized = true;

	string tmp = argv[0];
	binname_ = OnlyFilename(tmp);
	tmp = ExpandPath(tmp); // This expands ./ and ~/

	if (!is_absolute_path(tmp)) {
		string binsearchpath = GetEnvPath("PATH");
		// This will make "src/lyx" work always :-)
		binsearchpath += ";.";
		tmp = argv[0];
		tmp = FileOpenSearch(binsearchpath, tmp);
	}

	tmp = MakeAbsPath(OnlyPath(tmp));

	// In case we are running in place and compiled with shared libraries
	if (suffixIs(tmp, "/.libs/"))
		tmp.erase(tmp.length()-6, string::npos);
	binpath_ = tmp;
}


void warn(string const & mesg)
{
	MessageBox(0, mesg.c_str(), "LyX error",
	MB_OK|MB_ICONSTOP|MB_SYSTEMMODAL);
}


string current_root()
{
	return "/";
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


string slashify_path(string const & p)
{
	return subst(p, '\\', '/');
}


string external_path(string const & p)
{
	string dos_path = p;
	if (is_absolute_path(p)) {
		char dp[255];
		cygwin_conv_to_full_win32_path(p.c_str(), dp);
		dos_path = subst(dp,'\\','/');
	}
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
	char pp[256];
	cygwin_conv_to_posix_path(p.c_str(), pp);
	string const posix_path = MakeLatexName(pp);
	lyxerr[Debug::DEPEND]
		<< "<Win32 path correction> ["
		<< p << "]->>["
		<< posix_path << ']' << endl;
	return posix_path;
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


string binpath()
{
	return binpath_;
}


string binname()
{
	return binname_;
}


void setTmpDir(string const & p)
{
	tmpdir_ = p;
}


string getTmpDir()
{
	return tmpdir_;
}


shell_type shell()
{
	return UNIX;
}

} // namespace os
} // namespace support
} // namespace lyx
