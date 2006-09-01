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
#include <windef.h>
#include <shellapi.h>	
#include <shlwapi.h>

#include <sys/cygwin.h>

using std::endl;
using std::string;

using lyx::support::contains;


namespace lyx {
namespace support {
namespace os {

namespace {

bool windows_style_tex_paths_ = false;

// In both is_posix_path() and is_windows_path() it is assumed that
// a valid posix or pseudo-windows path is passed. They simply tell
// whether the path looks posix/pseudo-windows or not.

bool is_posix_path(string const & p)
{
	return  p.empty() ||
		(!contains(p, '\\') && (p.length() <= 1 || p[1] != ':'));
}

// This is a test for a win32 style path with forward slashes (pseudo-windows).

bool is_windows_path(string const & p)
{
	return p.empty() || (!contains(p, '\\') && p[0] != '/');
}


enum PathStyle {
	posix,
	windows
};


string convert_path(string const & p, PathStyle const & target)
{
	char path_buf[PATH_MAX];

	if ((target == posix && is_posix_path(p)) ||
	    (target == windows && is_windows_path(p)))
		return p;

	path_buf[0] = '\0';

	if (target == posix)
		cygwin_conv_to_posix_path(p.c_str(), path_buf);
	else
		cygwin_conv_to_win32_path(p.c_str(), path_buf);

	return subst(path_buf[0] ? path_buf : p, '\\', '/');
}


string convert_path_list(string const & p, PathStyle const & target)
{
	if (p.empty())
		return p;

	char const * const pc = p.c_str();
	PathStyle const actual = cygwin_posix_path_list_p(pc) ? posix : windows;

	if (target != actual) {
		int const target_size = (target == posix) ?
				cygwin_win32_to_posix_path_list_buf_size(pc) :
				cygwin_posix_to_win32_path_list_buf_size(pc);

		char * ptr = new char[target_size];

		if (ptr) {
			if (target == posix)
				cygwin_win32_to_posix_path_list(pc, ptr);
			else
				cygwin_posix_to_win32_path_list(pc, ptr);

			string path_list = subst(ptr, '\\', '/');
			delete ptr;
			return path_list;
		}
	}

	return subst(p, '\\', '/');
}

} // namespace anon

void os::init(int, char *[])
{
	// Copy cygwin environment variables to the Windows environment
	// if they're not already there.

	char **envp = environ;
	char curval[2];
	string var;
	string val;
	bool temp_seen = false;

	while (envp && *envp) {
		val = split(*envp++, var, '=');

		if (var == "TEMP")
			temp_seen = true;
		
		if (GetEnvironmentVariable(var.c_str(), curval, 2) == 0
				&& GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
			/* Convert to Windows style where necessary */
			if (var == "PATH" || var == "LD_LIBRARY_PATH") {
				string const winpathlist =
				    convert_path_list(val, PathStyle(windows));
				if (!winpathlist.empty()) {
					SetEnvironmentVariable(var.c_str(),
						winpathlist.c_str());
				}
			} else if (var == "HOME" || var == "TMPDIR" ||
					var == "TMP" || var == "TEMP") {
				string const winpath =
					convert_path(val, PathStyle(windows));
				SetEnvironmentVariable(var.c_str(), winpath.c_str());
			} else {
				SetEnvironmentVariable(var.c_str(), val.c_str());
			}
		}
	}
	if (!temp_seen) {
		string const winpath = convert_path("/tmp", PathStyle(windows));
		SetEnvironmentVariable("TEMP", winpath.c_str());
	}
}


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


string external_path(string const & p)
{
#ifdef X_DISPLAY_MISSING
	return convert_path(p, PathStyle(windows));
#else
	return convert_path(p, PathStyle(posix));
#endif
}


string internal_path(string const & p)
{
	return convert_path(p, PathStyle(posix));
}


string external_path_list(string const & p)
{
	return convert_path_list(p, PathStyle(windows));
}


string internal_path_list(string const & p)
{
	return convert_path_list(p, PathStyle(posix));
}


string latex_path(string const & p)
{
	// We may need a posix style path or a windows style path (depending
	// on windows_style_tex_paths_), but we use always forward slashes,
	// since it gets written into a .tex file.

	if (windows_style_tex_paths_ && is_absolute_path(p)) {
		string dos_path = convert_path(p, PathStyle(windows));
		lyxerr[Debug::LATEX]
			<< "<Path correction for LaTeX> ["
			<< p << "]->>["
			<< dos_path << ']' << endl;
		return dos_path;
	}

	return convert_path(p, PathStyle(posix));
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


void windows_style_tex_paths(bool use_windows_paths)
{
	windows_style_tex_paths_ = use_windows_paths;
}


bool canAutoOpenFile(string const & ext, auto_open_mode const mode)
{
	if (ext.empty())
		return false;

	string const full_ext = "." + ext;

	DWORD bufSize = MAX_PATH + 100;
	TCHAR buf[MAX_PATH + 100];
	// reference: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc
	//                 /platform/shell/reference/shlwapi/registry/assocquerystring.asp
	char const * action = (mode == VIEW) ? "open" : "edit";
	return S_OK == AssocQueryString(0, ASSOCSTR_EXECUTABLE,
		full_ext.c_str(), action, buf, &bufSize);
}


bool autoOpenFile(string const & filename, auto_open_mode const mode)
{
	// reference: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc
	//                 /platform/shell/reference/functions/shellexecute.asp
	string const win_path = convert_path(filename, PathStyle(windows));
	char const * action = (mode == VIEW) ? "open" : "edit";
	return reinterpret_cast<int>(ShellExecute(NULL, action,
		win_path.c_str(), NULL, NULL, 1)) > 32;
}


} // namespace os
} // namespace support
} // namespace lyx
