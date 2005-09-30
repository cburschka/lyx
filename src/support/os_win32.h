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

#include "LString.h"

#if !defined(_WIN32)
# error os_win32.h should be compiled only under Windows.
#endif

/* The GetLongPathNameA function declaration in
 * winbase.h under MinGW or Cygwin is protected
 * by the WINVER macro which is defined in windef.h
 *
 * We need to #include this file to make available the
 * DWORD, HMODULE et al. typedefs, so check WINVER now.
 *
 * Note: __CYGWIN__ can be defined here if building in _WIN32 mode.
 */
#if defined(__MINGW32__)  || defined(__CYGWIN__) || defined(__CYGWIN32__)
# if !defined(WINVER) || WINVER < 0x0500
#  error WINVER must be >= 0x0500
# endif
#endif

#include <windef.h>


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
	 *  the path asscociated with @c id.
	 */
	string const operator()(folder_id id) const;
private:
	typedef HRESULT (__stdcall * function_pointer)(HWND, int, HANDLE, DWORD, LPCSTR);

	HMODULE folder_module_;
	function_pointer folder_path_func_;
};

#endif // OS_WIN32_H
