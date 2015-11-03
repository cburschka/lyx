/**
 * \file os_win32.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Ruurd A. Reitsma
 * \author Claus Hentschel
 * \author Angus Leeming
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 *
 * Various OS specific functions
 */

#include <config.h>

#include "LyXRC.h"

#include "support/os.h"
#include "support/os_win32.h"

#include "support/debug.h"
#include "support/environment.h"
#include "support/FileName.h"
#include "support/gettext.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/ExceptionMessage.h"
#include "support/qstring_helpers.h"

#include "support/lassert.h"

#include <cstdlib>
#include <vector>

#include <QString>

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

#if defined(__MINGW32__)
#include <stdio.h>
#endif


extern "C" {
extern void __wgetmainargs(int * argc, wchar_t *** argv, wchar_t *** envp,
			   int expand_wildcards, int * new_mode);
}

using namespace std;

namespace lyx {

void lyx_exit(int);

namespace support {
namespace os {

namespace {

int argc_ = 0;
wchar_t ** argv_ = 0;

bool windows_style_tex_paths_ = true;

string cygdrive = "/cygdrive";

BOOL terminate_handler(DWORD event)
{
	if (event == CTRL_CLOSE_EVENT
	    || event == CTRL_LOGOFF_EVENT
	    || event == CTRL_SHUTDOWN_EVENT) {
		lyx::lyx_exit(1);
		return TRUE;
	}
	return FALSE;
}

} // namespace anon

void init(int argc, char * argv[])
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


	// Get the wide program arguments array
	wchar_t ** envp = 0;
	int newmode = 0;
	__wgetmainargs(&argc_, &argv_, &envp, -1, &newmode);
	LATTEST(argc == argc_);

	// If Cygwin is detected, query the cygdrive prefix.
	// The cygdrive prefix is needed for translating windows style paths
	// to posix style paths in LaTeX files when the Cygwin teTeX is used.
	int i;
	HKEY hkey;
	char buf[MAX_PATH];
	DWORD bufsize = sizeof(buf);
	LONG retval = ERROR_FILE_NOT_FOUND;
	HKEY const mainkey[2] = { HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER };
	char const * const subkey[2] = {
		"Software\\Cygwin\\setup",                         // Cygwin 1.7
		"Software\\Cygnus Solutions\\Cygwin\\mounts v2\\/" // Prev. ver.
	};
	char const * const valuename[2] = {
		"rootdir", // Cygwin 1.7 or later
		"native"   // Previous versions
	};
	// Check for newer Cygwin versions first, then older ones
	for (i = 0; i < 2 && retval != ERROR_SUCCESS; ++i) {
		for (int k = 0; k < 2 && retval != ERROR_SUCCESS; ++k)
			retval = RegOpenKey(mainkey[k], subkey[i], &hkey);
	}
	if (retval == ERROR_SUCCESS) {
		// Query the Cygwin root directory
		retval = RegQueryValueEx(hkey, valuename[i - 1],
				NULL, NULL, (LPBYTE) buf, &bufsize);
		RegCloseKey(hkey);
		string const mount = string(buf) + "\\bin\\mount.exe";
		if (retval == ERROR_SUCCESS && FileName(mount).exists()) {
			cmd_ret const p =
				runCommand(mount + " --show-cygdrive-prefix");
			// The output of the mount command is as follows:
			// Prefix              Type         Flags
			// /cygdrive           system       binmode
			// So, we use the inner split to pass the second line
			// to the outer split which sets cygdrive with its
			// contents until the first blank, discarding the
			// unneeded return value.
			if (p.first != -1 && prefixIs(p.second, "Prefix"))
				split(split(p.second, '\n'), cygdrive, ' ');
		}
	}

	// Catch shutdown events.
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)terminate_handler, TRUE);
}


string utf8_argv(int i)
{
	LASSERT(i < argc_, return "");
	return fromqstr(QString::fromWCharArray(argv_[i]));
}


void remove_internal_args(int i, int num)
{
	argc_ -= num;
	for (int j = i; j < argc_; ++j)
		argv_[j] = argv_[j + num];
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


bool path_prefix_is(string const & path, string const & pre)
{
	return path_prefix_is(const_cast<string &>(path), pre, CASE_UNCHANGED);
}


bool path_prefix_is(string & path, string const & pre, path_case how)
{
	docstring const p1 = from_utf8(path);
	docstring const p2 = from_utf8(pre);
	docstring::size_type const p1_len = p1.length();
	docstring::size_type const p2_len = p2.length();
	docstring::size_type common_len = common_path(p1, p2);

	if (p2[p2_len - 1] == '/' && p1_len != p2_len)
		++common_len;

	if (common_len != p2_len)
		return false;

	if (how == CASE_ADJUSTED && !prefixIs(path, pre)) {
		if (p1_len < common_len)
			path = to_utf8(p2.substr(0, p1_len));
		else
			path = to_utf8(p2 + p1.substr(common_len,
							p1_len - common_len));
	}

	return true;
}


string external_path(string const & p)
{
	string const dos_path = subst(p, "/", "\\");

	LYXERR(Debug::LATEX, "<Win32 path correction> ["
		<< p << "]->>[" << dos_path << ']');
	return dos_path;
}


static QString const get_long_path(QString const & short_path)
{
	// GetLongPathNameW needs the path in utf16 encoding.
	vector<wchar_t> long_path(MAX_PATH);
	DWORD result = GetLongPathNameW((wchar_t *) short_path.utf16(),
				       &long_path[0], long_path.size());

	if (result > long_path.size()) {
		long_path.resize(result);
		result = GetLongPathNameW((wchar_t *) short_path.utf16(),
					 &long_path[0], long_path.size());
		LATTEST(result <= long_path.size());
	}

	return (result == 0) ? short_path : QString::fromWCharArray(&long_path[0]);
}


static QString const get_short_path(QString const & long_path, file_access how)
{
	// CreateFileW and GetShortPathNameW need the path in utf16 encoding.
	if (how == CREATE) {
		HANDLE h = CreateFileW((wchar_t *) long_path.utf16(),
				GENERIC_WRITE, 0, NULL, CREATE_NEW,
				FILE_ATTRIBUTE_NORMAL, NULL);
		if (h == INVALID_HANDLE_VALUE
		    && GetLastError() != ERROR_FILE_EXISTS)
			return long_path;
		CloseHandle(h);
	}
	vector<wchar_t> short_path(MAX_PATH);
	DWORD result = GetShortPathNameW((wchar_t *) long_path.utf16(),
				       &short_path[0], short_path.size());

	if (result > short_path.size()) {
		short_path.resize(result);
		result = GetShortPathNameW((wchar_t *) long_path.utf16(),
					 &short_path[0], short_path.size());
		LATTEST(result <= short_path.size());
	}

	return (result == 0) ? long_path : QString::fromWCharArray(&short_path[0]);
}


string internal_path(string const & p)
{
	return subst(fromqstr(get_long_path(toqstr(p))), "\\", "/");
}


string safe_internal_path(string const & p, file_access how)
{
	return subst(fromqstr(get_short_path(toqstr(p), how)), "\\", "/");
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

	if (!windows_style_tex_paths_ && FileName::isAbsolute(p)) {
		string const drive = p.substr(0, 2);
		string const cygprefix = cygdrive + "/" + drive.substr(0, 1);
		string const cygpath = subst(subst(p, '\\', '/'), drive, cygprefix);
		LYXERR(Debug::LATEX, "<Path correction for LaTeX> ["
			<< p << "]->>[" << cygpath << ']');
		return cygpath;
	}
	return subst(p, '\\', '/');
}


string latex_path_list(string const & p)
{
	if (p.empty())
		return p;

	// We may need a posix style path or a windows style path (depending
	// on windows_style_tex_paths_), but we use always forward slashes,
	// since this is standard for all tex engines.

	if (!windows_style_tex_paths_) {
		string pathlist;
		for (size_t i = 0, k = 0; i != string::npos; k = i) {
			i = p.find(';', i);
			string path = subst(p.substr(k, i - k), '\\', '/');
			if (FileName::isAbsolute(path)) {
				string const drive = path.substr(0, 2);
				string const cygprefix = cygdrive + "/"
							+ drive.substr(0, 1);
				path = subst(path, drive, cygprefix);
			}
			pathlist += path;
			if (i != string::npos) {
				pathlist += ':';
				++i;
			}
		}
		return pathlist;
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


char path_separator(path_type type)
{
	if (type == TEXENGINE)
		return windows_style_tex_paths_ ? ';' : ':';

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
		LASSERT(false, return string());
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


bool autoOpenFile(string const & filename, auto_open_mode const mode,
		  string const & path)
{
	string const texinputs = os::latex_path_list(
			replaceCurdirPath(path, lyxrc.texinputs_prefix));
	string const otherinputs = os::latex_path_list(path);
	string const sep = windows_style_tex_paths_ ? ";" : ":";
	string const oldtexinputs = getEnv("TEXINPUTS");
	string const newtexinputs = "." + sep + texinputs + sep + oldtexinputs;
	string const oldbibinputs = getEnv("BIBINPUTS");
	string const newbibinputs = "." + sep + otherinputs + sep + oldbibinputs;
	string const oldbstinputs = getEnv("BSTINPUTS");
	string const newbstinputs = "." + sep + otherinputs + sep + oldbstinputs;
	string const oldtexfonts = getEnv("TEXFONTS");
	string const newtexfonts = "." + sep + otherinputs + sep + oldtexfonts;
	if (!path.empty() && !lyxrc.texinputs_prefix.empty()) {
		setEnv("TEXINPUTS", newtexinputs);
		setEnv("BIBINPUTS", newbibinputs);
		setEnv("BSTINPUTS", newbstinputs);
		setEnv("TEXFONTS", newtexfonts);
	}

	// reference: http://msdn.microsoft.com/en-us/library/bb762153.aspx
	char const * action = (mode == VIEW) ? "open" : "edit";
	bool success = reinterpret_cast<int>(ShellExecute(NULL, action,
		to_local8bit(from_utf8(filename)).c_str(), NULL, NULL, 1)) > 32;

	if (!path.empty() && !lyxrc.texinputs_prefix.empty()) {
		setEnv("TEXINPUTS", oldtexinputs);
		setEnv("BIBINPUTS", oldbibinputs);
		setEnv("BSTINPUTS", oldbstinputs);
		setEnv("TEXFONTS", oldtexfonts);
	}
	return success;
}


string real_path(string const & path)
{
	// See http://msdn.microsoft.com/en-us/library/aa366789(VS.85).aspx
	QString const qpath = get_long_path(toqstr(path));
	HANDLE hpath = CreateFileW((wchar_t *) qpath.utf16(), GENERIC_READ,
				FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if (hpath == INVALID_HANDLE_VALUE) {
		// The file cannot be accessed.
		return path;
	}

	// Get the file size.
	DWORD size_hi = 0;
	DWORD size_lo = GetFileSize(hpath, &size_hi);

	if (size_lo == 0 && size_hi == 0) {
		// A zero-length file cannot be mapped.
		CloseHandle(hpath);
		return path;
	}

	// Create a file mapping object.
	HANDLE hmap = CreateFileMapping(hpath, NULL, PAGE_READONLY, 0, 1, NULL);

	if (!hmap) {
		CloseHandle(hpath);
		return path;
	}

	// Create a file mapping to get the file name.
	void * pmem = MapViewOfFile(hmap, FILE_MAP_READ, 0, 0, 1);

	if (!pmem) {
		CloseHandle(hmap);
		CloseHandle(hpath);
		return path;
	}

	TCHAR realpath[MAX_PATH + 1];

	if (!GetMappedFileName(GetCurrentProcess(), pmem, realpath, MAX_PATH)) {
		UnmapViewOfFile(pmem);
		CloseHandle(hmap);
		CloseHandle(hpath);
		return path;
	}

	// Translate device name to UNC prefix or drive letters.
	TCHAR tmpbuf[MAX_PATH] = TEXT("\\Device\\Mup\\");
	UINT namelen = _tcslen(tmpbuf);
	if (_tcsnicmp(realpath, tmpbuf, namelen) == 0) {
		// UNC path
		_snprintf(tmpbuf, MAX_PATH, "\\\\%s", realpath + namelen);
		strncpy(realpath, tmpbuf, MAX_PATH);
		realpath[MAX_PATH] = '\0';
	} else if (GetLogicalDriveStrings(MAX_PATH - 1, tmpbuf)) {
		// Check whether device name corresponds to some local drive.
		TCHAR name[MAX_PATH];
		TCHAR drive[3] = TEXT(" :");
		bool found = false;
		TCHAR * p = tmpbuf;
		do {
			// Copy the drive letter to the template string
			drive[0] = *p;
			// Look up each device name
			if (QueryDosDevice(drive, name, MAX_PATH)) {
				namelen = _tcslen(name);
				if (namelen < MAX_PATH) {
					found = _tcsnicmp(realpath, name, namelen) == 0;
					if (found) {
						// Repl. device spec with drive
						TCHAR tempfile[MAX_PATH];
						_snprintf(tempfile,
							MAX_PATH,
							"%s%s",
							drive,
							realpath + namelen);
						strncpy(realpath,
							tempfile,
							MAX_PATH);
						realpath[MAX_PATH] = '\0';
					}
				}
			}
			// Advance p to the next NULL character.
			while (*p++) ;
		} while (!found && *p);
	}
	UnmapViewOfFile(pmem);
	CloseHandle(hmap);
	CloseHandle(hpath);
	string const retpath = subst(string(realpath), '\\', '/');
	return FileName::fromFilesystemEncoding(retpath).absFileName();
}

} // namespace os
} // namespace support
} // namespace lyx
