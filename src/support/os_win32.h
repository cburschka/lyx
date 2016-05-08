// -*- C++ -*-
/**
 * \file os_win32.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * These classes should be used only on Windows machines.
 */

#ifndef OS_WIN32_H
#define OS_WIN32_H

#include <string>

#if !defined(_WIN32)
# error os_win32.h should be compiled only under Windows.
#endif

/* The GetLongPathNameA function declaration in
 * <winbase.h> is protected by the WINVER macro which is
 * defined to a default value in <windef.h> under MinGW and Cygwin.
 *
 * SHGFP_TYPE_CURRENT is defined in <shlobj.h> for __W32API_VERSION >= 3.2
 * where it is protected by _WIN32_IE, also defined to a default value
 * in <windef.h> under MinGW and Cygwin.
 * It is missing in earlier versions of the MinGW w32api headers.
 *
 * We need to #include <windows.h> now to make available the
 * DWORD, HMODULE et al. typedefs, so first define WINVER, _WIN32_IE.
 *
 * Note: __CYGWIN__ can be defined here if building in _WIN32 mode.
 */
#if defined(__MINGW32__)  || defined(__CYGWIN__) || defined(__CYGWIN32__)
# if defined(WINVER)
#  if WINVER < 0x0500
#   error WINVER must be >= 0x0500
#  endif
# else
#  define WINVER 0x0500
# endif
# define _WIN32_IE 0x0500
#endif

#include <windows.h>
#include <tchar.h>
#include <psapi.h>


namespace lyx {
namespace support {
namespace os {

/** Win98 and earlier don't have SHGetFolderPath in shell32.dll.
 *  Microsoft recommend that we load shfolder.dll at run time and
 *  access the function through that.
 *
 *  shfolder.dll is loaded dynamically in the constructor. If loading
 *  fails or if the .dll is found not to contain SHGetFolderPathA then
 *  the program exits immediately. Otherwise, the .dll is unloaded in
 *  the destructor
 *
 *  The class makes SHGetFolderPath available through its function operator.
 *  It will work on all versions of Windows >= Win95.
 */
class GetFolderPath {
public:
	enum folder_id {
		/// CSIDL_PERSONAL
		PERSONAL,
		/// CSIDL_APPDATA
		APPDATA
	};

	GetFolderPath();
	~GetFolderPath();

	/** Wrapper for SHGetFolderPathA, returning
	 *  the path asscociated with @c id in utf8 encoding.
	 */
	std::string const operator()(folder_id id) const;
private:
	typedef HRESULT (__stdcall * function_pointer)(HWND, int, HANDLE, DWORD, LPCSTR);

	HMODULE folder_module_;
	function_pointer folder_path_func_;
};

} // namespace os
} // namespace support
} // namespace lyx

#endif // OS_WIN32_H
