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

#if defined(__CYGWIN__) || defined(__CYGWIN32__)
#include <sys/cygwin.h>
#include <cstdlib>

#elif defined(_WIN32)
# include <direct.h> // _getdrive
#endif
 

string os::binpath_ = string();
string os::binname_ = string();
string os::tmpdir_;
string os::homepath_;
string os::nulldev_;

#if defined(__CYGWIN__) || defined(__CYGWIN32__)
os::shell_type os::_shell = os::UNIX;
#else
os::shell_type os::_shell = os::CMD_EXE;
#endif

unsigned long os::cp_ = 0;

using std::endl;

void os::init(int /* argc */, char * argv[])
{
	static bool initialized = false;
	if (initialized) return;
	initialized = true;
	string tmp = internal_path(argv[0]);
	binname_ = OnlyFilename(tmp);
	tmp = ExpandPath(tmp); // This expands ./ and ~/

	if (!is_absolute_path(tmp)) {
		string binsearchpath = GetEnvPath("PATH");
		// This will make "src/lyx" work always :-)
		binsearchpath += ";.";
		tmp = internal_path(argv[0]);
		tmp = FileOpenSearch(binsearchpath, tmp);
	}

	tmp = MakeAbsPath(OnlyPath(tmp));

	// In case we are running in place and compiled with shared libraries
	if (suffixIs(tmp, "/.libs/"))
		tmp.erase(tmp.length()-6, string::npos);
	binpath_ = tmp;

#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	tmpdir_ = "/tmp";
	homepath_ = GetEnvPath("HOME");
	nulldev_ = "/dev/null";
#else
	// Use own tempdir
	tmp.erase(tmp.length()-4,4);
	tmpdir_ = tmp + "tmp";

	homepath_ = GetEnvPath("HOMEDRIVE") + GetEnvPath("HOMEPATH");
	nulldev_ = "nul";
#endif
}


void os::warn(string mesg)
{
	MessageBox(0, mesg.c_str(), "LyX error",
	MB_OK|MB_ICONSTOP|MB_SYSTEMMODAL);
}


string os::current_root()
{
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	return string("/");

#else
	// _getdrive returns the current drive (1=A, 2=B, and so on).
	char const drive = ::_getdrive() + 'A' - 1;
	return string(1, drive) + ":/";
#endif
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


string os::external_path(string const & p)
{	
	string dos_path;

#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	// Translate from cygwin path syntax to dos path syntax
	if (is_absolute_path(p)) {
		char dp[PATH_MAX+1];
		cygwin_conv_to_full_win32_path(p.c_str(), dp);
		dos_path = !dp ? string() : dp;
	}

	else return p;
#else // regular Win32
	dos_path = p;
#endif
	
	// No backslashes in LaTeX files
	dos_path = subst(dos_path,'\\','/');

	lyxerr[Debug::LATEX]
		<< "<Win32 path correction> ["
		<< p << "]->>["
		<< dos_path << ']' << endl;
	return dos_path;
}


string os::internal_path(string const & p)
{
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	char posix_path[PATH_MAX+1];
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
