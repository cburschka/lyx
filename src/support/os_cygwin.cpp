/**
 * \file os_cygwin.cpp
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

#ifdef X_DISPLAY_MISSING
#include "support/filetools.h"
#include "support/Package.h"
#include "support/Path.h"
using lyx::support::addName;
using lyx::support::addPath;
using lyx::support::package;

// API definition for manually calling font functions on Windows 2000 and later
typedef int (WINAPI *FONTAPI)(LPCSTR, DWORD, PVOID);
#define FR_PRIVATE     0x10

// Names of TrueType fonts to load
string const win_fonts_truetype[] = {"cmex10", "cmmi10", "cmr10", "cmsy10",
	"eufm10", "msam10", "msbm10", "wasy10", "esint10"};
const int num_fonts_truetype = sizeof(win_fonts_truetype) / sizeof(*win_fonts_truetype);
#endif


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


/// Convert a path to or from posix style.
/// \p p is encoded in local 8bit encoding or utf8.
/// The result is returned in the same encoding as \p p.
string convert_path(string const & p, PathStyle const & target)
{
	char path_buf[PATH_MAX];

	if ((target == posix && is_posix_path(p)) ||
	    (target == windows && is_windows_path(p)))
		return p;

	path_buf[0] = '\0';

	// cygwin_conv_to_posix_path and cygwin_conv_to_win32_path do not
	// care about the encoding.
	if (target == posix)
		cygwin_conv_to_posix_path(p.c_str(), path_buf);
	else
		cygwin_conv_to_win32_path(p.c_str(), path_buf);

	return subst(path_buf[0] ? path_buf : p, '\\', '/');
}


/// Convert a path list to or from posix style.
/// \p p is encoded in local 8bit encoding or utf8.
/// The result is returned in the same encoding as \p p.
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
			// FIXME: See comment in convert_path() above
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
	// Make sure that the TEMP variable is set
	// and sync the Windows environment.

	char **envp = environ;
	string var;
	bool temp_seen = false;

	while (envp && *envp && !temp_seen) {
		split(*envp++, var, '=');
		if (var == "TEMP")
			temp_seen = true;
	}
	if (!temp_seen)
		::setenv("TEMP", "/tmp", true);

	cygwin_internal(CW_SYNC_WINENV);
}


string current_root()
{
	return string("/");
}


docstring::size_type common_path(docstring const & p1, docstring const & p2)
{
	docstring::size_type i = 0;
	docstring::size_type const p1_len = p1.length();
	docstring::size_type const p2_len = p2.length();
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
	return convert_path(p, PathStyle(posix));
}


string internal_path(string const & p)
{
	return convert_path(p, PathStyle(posix));
}


string external_path_list(string const & p)
{
	return convert_path_list(p, PathStyle(posix));
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
		LYXERR(Debug::LATEX)
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
	string const win_path = to_local8bit(from_utf8(convert_path(filename, PathStyle(windows))));
	char const * action = (mode == VIEW) ? "open" : "edit";
	return reinterpret_cast<int>(ShellExecute(NULL, action,
		win_path.c_str(), NULL, NULL, 1)) > 32;
}


void addFontResources()
{
#ifdef X_DISPLAY_MISSING
	// Windows only: Add BaKoMa TrueType font resources
	string const fonts_dir = addPath(package().system_support().absFilename(), "fonts");

	HMODULE hDLL = LoadLibrary("gdi32");
	FONTAPI pAddFontResourceEx =
		(FONTAPI) GetProcAddress(hDLL, "AddFontResourceExA");

	for (int i = 0 ; i < num_fonts_truetype ; ++i) {
		string const font_current = to_local8bit(from_utf8(convert_path(
			addName(fonts_dir, win_fonts_truetype[i] + ".ttf"),
			PathStyle(windows))));
		if (pAddFontResourceEx) {
			// Windows 2000 and later: Use AddFontResourceEx
			pAddFontResourceEx(font_current.c_str(), FR_PRIVATE, 0);
		} else {
			// Older Windows versions: Use AddFontResource
			AddFontResource(font_current.c_str());
		}
	}
	FreeLibrary(hDLL);
#endif
}


void restoreFontResources()
{
#ifdef X_DISPLAY_MISSING
	// Windows only: Remove BaKoMa TrueType font resources
	string const fonts_dir = addPath(package().system_support().absFilename(), "fonts");

	HMODULE hDLL = LoadLibrary("gdi32");
	FONTAPI pRemoveFontResourceEx = (FONTAPI) GetProcAddress(hDLL, "RemoveFontResourceExA");

	for(int i = 0 ; i < num_fonts_truetype ; ++i) {
		string const font_current = to_local8bit(from_utf8(convert_path(
			addName(fonts_dir, win_fonts_truetype[i] + ".ttf"),
			PathStyle(windows))));
		if (pRemoveFontResourceEx) {
			// Windows 2000 and later: Use RemoveFontResourceEx
			pRemoveFontResourceEx(font_current.c_str(), FR_PRIVATE, 0);
		} else {
			// Older Windows versions: Use RemoveFontResource
			RemoveFontResource(font_current.c_str());
		}
	}
	FreeLibrary(hDLL);
#endif
}

} // namespace os
} // namespace support
} // namespace lyx
