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
#include <direct.h> // _getdrive


string const os::nulldev_ = "nul";
os::shell_type os::shell_ = os::CMD_EXE;


void os::init(int /* argc */, char * argv[])
{
	/* Note from Angus, 17 Jan 2005:
	 *
	 * The code below is taken verbatim from Ruurd's original patch
	 * porting LyX to Win32.
	 *
	 * Windows allows us to define LyX either as a console-based app
	 * or as a GUI-based app. Ruurd decided to define LyX as a
	 * console-based app with a "main" function rather than a "WinMain"
	 * function as the point of entry to the program, but to
	 * immediately close the console window that Windows helpfully
	 * opens for us. Doing so allows the user to see all of LyX's
	 * debug output simply by running LyX from a DOS or MSYS-shell
	 * prompt.
	 *
	 * The alternative approach is to define LyX as a genuine
	 * GUI-based app, with a "WinMain" function as the entry point to the
	 * executable rather than a "main" function, so:
	 *
	 * #if defined (_WIN32)
	 * # define WIN32_LEAN_AND_MEAN
	 * # include <stdlib.h>  // for __argc,__argv
	 * # include <windows.h> // for WinMain
	 * #endif
	 *
	 * // This will require the "-mwindows" flag when linking with
	 * // gcc under MinGW.
	 * // For MSVC, use "/subsystem:windows".
	 * #if defined (_WIN32)
	 * int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
	 * {
	 *     return mymain(__argc, __argv);
	 * }
	 * #endif
	 *
	 * where "mymain" is just a renamed "main".
	 *
	 * However, doing so means that the lyxerr messages would mysteriously
	 * disappear. They could be resurrected with something like:
	 *
	 * #ifdef WIN32
	 *  AllocConsole();
	 *  freopen("conin$","r",stdin);
	 *  freopen("conout$","w",stdout);
	 *  freopen("conout$","w",stderr);
	 * #endif
	 *
	 * This code could be invoked (say) the first time that lyxerr
	 * is called. However, Ruurd has tried this route and found that some
	 * shell scripts failed, for mysterious reasons...
	 *
	 * I've chosen for now, therefore, to simply add Ruurd's original
	 * code as-is.
	 */
	// Close the console when run (probably)
	// not run from command prompt
	char WindowTitle[1024];
	HWND hwndFound;
	GetConsoleTitle(WindowTitle,1024);
	if ((strcmp(WindowTitle, argv[0]) == 0) ||
		(strcmp(WindowTitle,"LyX") == 0)) {
		// format a "unique" newWindowTitle
		wsprintf(WindowTitle,"%d/%d",
			GetTickCount(),
			GetCurrentProcessId());
		// change current window title
		SetConsoleTitle(WindowTitle);
		// ensure window title has been updated
		Sleep(40);
		// look for newWindowTitle
		hwndFound=FindWindow(NULL, WindowTitle);
		// If found, hide it
		if ( hwndFound != NULL)
			ShowWindow( hwndFound, SW_HIDE);
	}
}


string os::current_root()
{
	// _getdrive returns the current drive (1=A, 2=B, and so on).
	char const drive = ::_getdrive() + 'A' - 1;
	return string(1, drive) + ":/";
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
	string dos_path = p;

	// No backslashes in LaTeX files
	dos_path = subst(dos_path,'\\','/');

	lyxerr[Debug::LATEX]
		<< "<Win32 path correction> ["
		<< p << "]->>["
		<< dos_path << ']' << std::endl;
	return dos_path;
}


string os::internal_path(string const & p)
{
	return subst(p,"\\","/");
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
	return ';';
}
