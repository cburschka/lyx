/**
 * \file os_win32.cpp
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
#include "support/os_win32.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/gettext.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/ExceptionMessage.h"

#include "support/lassert.h"

#include <cstdlib>
#include <vector>

/* The GetLongPathName macro may be defined on the compiling machine,
 * but we must use a bit of trickery if the resulting executable is
 * to run on a Win95 machine.
 * Fortunately, Microsoft provide the trickery. All we need is the
 * NewAPIs.h header file, available for download from Microsoft as
 * part of the Platform SDK.
 */
#if defined (HAVE_NEWAPIS_H)
// This should be defined already to keep Boost.Filesystem happy.
# if !defined (WANT_GETFILEATTRIBUTESEX_WRAPPER)
#   error Expected WANT_GETFILEATTRIBUTESEX_WRAPPER to be defined!
# endif
# define WANT_GETLONGPATHNAME_WRAPPER 1
# define COMPILE_NEWAPIS_STUBS
# include <NewAPIs.h>
# undef COMPILE_NEWAPIS_STUBS
# undef WANT_GETLONGPATHNAME_WRAPPER
#endif

#include <io.h>
#include <direct.h> // _getdrive
#include <shlobj.h>  // SHGetFolderPath
#include <windef.h>
#include <shellapi.h>
#include <shlwapi.h>

// Must define SHGFP_TYPE_CURRENT for older versions of MinGW.
#if defined(__MINGW32__)  || defined(__CYGWIN__) || defined(__CYGWIN32__)
# include <w32api.h>
# if __W32API_MAJOR_VERSION < 3 || \
     __W32API_MAJOR_VERSION == 3 && __W32API_MINOR_VERSION  < 2
#  define SHGFP_TYPE_CURRENT 0
# endif
#endif

#if !defined(ASSOCF_INIT_IGNOREUNKNOWN) && !defined(__MINGW32__)
#define ASSOCF_INIT_IGNOREUNKNOWN 0
#endif

using namespace std;

namespace lyx {
namespace support {
namespace os {

namespace {

bool windows_style_tex_paths_ = true;

string cygdrive = "/cygdrive";

} // namespace anon

void init(int /* argc */, char * argv[])
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
	 * # include <stdlib.h>  // for __argc, __argv
	 * # include <windows.h> // for WinMain
	 * #endif
	 *
	 * // This will require the "-mwindows" flag when linking with
	 * // gcc under MinGW.
	 * // For MSVC, use "/subsystem:windows".
	 * #if defined (_WIN32)
	 * int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
	 * {
	 *     return mymain(__argc, __argv);
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
	 * code as-is. A wrapper program hidecmd.c has been added to
	 * development/Win32 which hides the console window of lyx when
	 * lyx is invoked as a parameter of hidecmd.exe.
	 */

	// If cygwin is detected, query the cygdrive prefix
	HKEY regKey;
	char buf[MAX_PATH];
	DWORD bufSize = sizeof(buf);
	LONG retVal;

	retVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			"Software\\Cygnus Solutions\\Cygwin\\mounts v2",
			0, KEY_QUERY_VALUE, &regKey);
	if (retVal != ERROR_SUCCESS) {
		retVal = RegOpenKeyEx(HKEY_CURRENT_USER,
				"Software\\Cygnus Solutions\\Cygwin\\mounts v2",
				0, KEY_QUERY_VALUE, &regKey);
	}
	if (retVal == ERROR_SUCCESS) {
		retVal = RegQueryValueEx(regKey, "cygdrive prefix", NULL, NULL,
				(LPBYTE) buf, &bufSize);
		RegCloseKey(regKey);
		if ((retVal == ERROR_SUCCESS) && (bufSize <= MAX_PATH))
			cygdrive = rtrim(string(buf), "/");
	}
}


string current_root()
{
	// _getdrive returns the current drive (1=A, 2=B, and so on).
	char const drive = ::_getdrive() + 'A' - 1;
	return string(1, drive) + ":/";
}


bool isFilesystemCaseSensitive()
{
	return false;
}


docstring::size_type common_path(docstring const & p1, docstring const & p2)
{
	size_t i = 0;
	size_t const p1_len = p1.length();
	size_t const p2_len = p2.length();
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
	string const dos_path = subst(p, "/", "\\");

	LYXERR(Debug::LATEX, "<Win32 path correction> ["
		<< p << "]->>[" << dos_path << ']');
	return dos_path;
}


static string const get_long_path(string const & short_path)
{
	// GetLongPathName needs the path in file system encoding.
	// We can use to_local8bit, since file system encoding and the
	// local 8 bit encoding are identical on windows.
	vector<char> long_path(MAX_PATH);
	DWORD result = GetLongPathName(to_local8bit(from_utf8(short_path)).c_str(),
				       &long_path[0], long_path.size());

	if (result > long_path.size()) {
		long_path.resize(result);
		result = GetLongPathName(short_path.c_str(),
					 &long_path[0], long_path.size());
		LASSERT(result <= long_path.size(), /**/);
	}

	return (result == 0) ? short_path : to_utf8(from_filesystem8bit(&long_path[0]));
}


string internal_path(string const & p)
{
	return subst(get_long_path(p), "\\", "/");
}


string external_path_list(string const & p)
{
	return subst(p, '/', '\\');
}


string internal_path_list(string const & p)
{
	return subst(p, '\\', '/');
}


string latex_path(string const & p)
{
	// We may need a posix style path or a windows style path (depending
	// on windows_style_tex_paths_), but we use always forward slashes,
	// since it gets written into a .tex file.

	FileName path(p);
	if (!windows_style_tex_paths_ && path.isAbsolute()) {
		string const drive = p.substr(0, 2);
		string const cygprefix = cygdrive + "/" + drive.substr(0, 1);
		string const cygpath = subst(subst(p, '\\', '/'), drive, cygprefix);
		LYXERR(Debug::LATEX, "<Path correction for LaTeX> ["
			<< p << "]->>[" << cygpath << ']');
		return cygpath;
	}
	return subst(p, '\\', '/');
}


bool is_valid_strftime(string const & p)
{
	string::size_type pos = p.find_first_of('%');
	while (pos != string::npos) {
		if (pos + 1 == string::npos)
			break;
		if (!containsOnly(p.substr(pos + 1, 1),
			"aAbBcdfHIjmMpSUwWxXyYzZ%"))
			return false;
		if (pos + 2 == string::npos)
		      break;
		pos = p.find_first_of('%', pos + 2);
	}
	return true;
}


// returns a string suitable to be passed to popen when
// reading a pipe
char const * popen_read_mode()
{
	return "r";
}


string const & nulldev()
{
	static string const nulldev_ = "nul";
	return nulldev_;
}


shell_type shell()
{
	return CMD_EXE;
}


char path_separator()
{
	return ';';
}


void windows_style_tex_paths(bool use_windows_paths)
{
	windows_style_tex_paths_ = use_windows_paths;
}


GetFolderPath::GetFolderPath()
	: folder_module_(0),
	  folder_path_func_(0)
{
	folder_module_ = LoadLibrary("shfolder.dll");
	if (!folder_module_) {
		throw ExceptionMessage(ErrorException, _("System file not found"),
			_("Unable to load shfolder.dll\nPlease install."));
	}

	folder_path_func_ = reinterpret_cast<function_pointer>(::GetProcAddress(folder_module_, "SHGetFolderPathA"));
	if (folder_path_func_ == 0) {
		throw ExceptionMessage(ErrorException, _("System function not found"),
			_("Unable to find SHGetFolderPathA in shfolder.dll\n"
			  "Don't know how to proceed. Sorry."));
	}
}


GetFolderPath::~GetFolderPath()
{
	if (folder_module_)
		FreeLibrary(folder_module_);
}


// Given a folder ID, returns the folder name (in unix-style format).
// Eg CSIDL_PERSONAL -> "C:/Documents and Settings/USERNAME/My Documents"
string const GetFolderPath::operator()(folder_id _id) const
{
	char folder_path[MAX_PATH];

	int id = 0;
	switch (_id) {
	case PERSONAL:
		id = CSIDL_PERSONAL;
		break;
	case APPDATA:
		id = CSIDL_APPDATA;
		break;
	default:
		LASSERT(false, /**/);
	}
	HRESULT const result = (folder_path_func_)(0, id, 0,
						   SHGFP_TYPE_CURRENT,
						   folder_path);
	return (result == 0) ? os::internal_path(to_utf8(from_filesystem8bit(folder_path))) : string();
}


bool canAutoOpenFile(string const & ext, auto_open_mode const mode)
{
	if (ext.empty())
		return false;

	string const full_ext = "." + ext;

	DWORD bufSize = MAX_PATH + 100;
	TCHAR buf[MAX_PATH + 100];
	// reference: http://msdn.microsoft.com/en-us/library/bb773471.aspx
	char const * action = (mode == VIEW) ? "open" : "edit";
	return S_OK == AssocQueryString(ASSOCF_INIT_IGNOREUNKNOWN,
		ASSOCSTR_EXECUTABLE, full_ext.c_str(), action, buf, &bufSize);
}


bool autoOpenFile(string const & filename, auto_open_mode const mode)
{
	// reference: http://msdn.microsoft.com/en-us/library/bb762153.aspx
	char const * action = (mode == VIEW) ? "open" : "edit";
	return reinterpret_cast<int>(ShellExecute(NULL, action,
		to_local8bit(from_utf8(filename)).c_str(), NULL, NULL, 1)) > 32;
}

} // namespace os
} // namespace support
} // namespace lyx
