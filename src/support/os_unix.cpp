/**
 * \file os_unix.cpp
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

#include "LyXRC.h"

#include "support/os.h"
#include "support/docstring.h"
#include "support/environment.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lassert.h"

#include <limits.h>
#include <locale.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFArray.h>
#endif

using namespace std;

namespace lyx {
namespace support {
namespace os {

namespace {

int argc_ = 0;
char ** argv_ = 0;

} // namespace anon

void init(int argc, char * argv[])
{
	argc_ = argc;
	argv_ = argv;

	// Set environment's default locale
	setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "C");
}


string utf8_argv(int i)
{
	LASSERT(i < argc_, return "");
	return to_utf8(from_local8bit(argv_[i]));
}


void remove_internal_args(int, int)
{}


string current_root()
{
	return "/";
}


bool isFilesystemCaseSensitive()
{
#ifdef __APPLE__
	return false;
#else
	return true;
#endif
}


docstring::size_type common_path(docstring const & p1, docstring const & p2)
{
	docstring::size_type i = 0;
	docstring::size_type const p1_len = p1.length();
	docstring::size_type const p2_len = p2.length();
#ifdef __APPLE__
	while (i < p1_len && i < p2_len && uppercase(p1[i]) == uppercase(p2[i]))
		++i;
#else
	while (i < p1_len && i < p2_len && p1[i] == p2[i])
		++i;
#endif
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
#ifdef __APPLE__
	return path_prefix_is(const_cast<string &>(path), pre, CASE_UNCHANGED);
#else
	return prefixIs(path, pre);
#endif
}


bool path_prefix_is(string & path, string const & pre, path_case how)
{
#ifdef __APPLE__
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
#else
	// silence compiler warnings
	(void)how;

	return prefixIs(path, pre);
#endif
}


string external_path(string const & p)
{
	return p;
}


string internal_path(string const & p)
{
	return p;
}


string safe_internal_path(string const & p, file_access)
{
	return p;
}


string external_path_list(string const & p)
{
	return p;
}


string internal_path_list(string const & p)
{
	return p;
}


string latex_path(string const & p)
{
	return p;
}


string latex_path_list(string const & p)
{
	return p;
}


bool is_valid_strftime(string const & p)
{
	string::size_type pos = p.find_first_of('%');
	while (pos != string::npos) {
		if (pos + 1 == string::npos)
			break;
		if (!containsOnly(p.substr(pos + 1, 1),
			"aAbBcCdDeEFgGhHIjklmMnOpPrRsStTuUVwWxXyYzZ%+"))
			return false;
		if (pos + 2 == string::npos)
		      break;
		pos = p.find_first_of('%', pos + 2);
	}
	return true;
}


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


char path_separator(path_type)
{
	return ':';
}


void windows_style_tex_paths(bool)
{}

bool canAutoOpenFile(string const & ext, auto_open_mode const mode)
{
#ifdef __APPLE__
// Reference: http://developer.apple.com/documentation/Carbon/Reference/LaunchServicesReference/
	CFStringRef cfs_ext = CFStringCreateWithBytes(kCFAllocatorDefault,
					(UInt8 *) ext.c_str(), ext.length(),
					kCFStringEncodingISOLatin1, false);
	// this is what we would like to do but it seems that the
	// viewer for PDF is often quicktime...
	//LSRolesMask role = (mode == VIEW) ? kLSRolesViewer :  kLSRolesEditor;
	(void)mode;
	LSRolesMask role = kLSRolesAll;
	FSRef outAppRef;
	OSStatus status =
		LSGetApplicationForInfo(kLSUnknownType, kLSUnknownCreator,
					cfs_ext, role, &outAppRef, NULL);
	CFRelease(cfs_ext);

	return status != kLSApplicationNotFoundErr;
#else
	// silence compiler warnings
	(void)ext;
	(void)mode;

	// currently, no default viewer is tried for non-windows system
	// support for KDE/Gnome/Macintosh may be added later
	return false;
#endif
}


bool autoOpenFile(string const & filename, auto_open_mode const mode,
		  string const & path)
{
#ifdef __APPLE__
// Reference: http://developer.apple.com/documentation/Carbon/Reference/LaunchServicesReference/
	// this is what we would like to do but it seems that the
	// viewer for PDF is often quicktime...
	//LSRolesMask role = (mode == VIEW) ? kLSRolesViewer :  kLSRolesEditor;
	(void)mode;
	LSRolesMask role = kLSRolesAll;

	CFURLRef docURL = CFURLCreateFromFileSystemRepresentation(
		NULL, (UInt8 *) filename.c_str(), filename.size(), false);
	CFURLRef appURL;
	OSStatus status = LSGetApplicationForURL(docURL, role, NULL, &appURL);
	if (status == kLSApplicationNotFoundErr)
		return false;

	CFURLRef docURLs[] = { docURL };
	CFArrayRef launchItems = CFArrayCreate(
		NULL,
		(const void**)docURLs, sizeof(docURLs) / sizeof(CFURLRef),
		NULL);
	LSLaunchURLSpec launchUrlSpec = {
		appURL, launchItems, NULL, kLSLaunchDefaults, NULL
	};

	string const texinputs = os::latex_path_list(
		replaceCurdirPath(path, lyxrc.texinputs_prefix));
	string const oldval = getEnv("TEXINPUTS");
	string const newval = ".:" + texinputs + ":" + oldval;
	if (!path.empty() && !lyxrc.texinputs_prefix.empty())
		setEnv("TEXINPUTS", newval);
	status = LSOpenFromURLSpec (&launchUrlSpec, NULL);
	CFRelease(launchItems);
	if (!path.empty() && !lyxrc.texinputs_prefix.empty())
		setEnv("TEXINPUTS", oldval);
	return status == 0;
#else
	// silence compiler warnings
	(void)filename;
	(void)mode;
	(void)path;

	// currently, no default viewer is tried for non-windows system
	// support for KDE/Gnome/Macintosh may be added later
	return false;
#endif
}


string real_path(string const & path)
{
#ifdef HAVE_DEF_PATH_MAX
	char rpath[PATH_MAX + 1];
	char * result = realpath(path.c_str(), rpath);
	return FileName::fromFilesystemEncoding(result ? rpath : path).absFileName();
#else
	char * result = realpath(path.c_str(), NULL);
	string ret = FileName::fromFilesystemEncoding(result ? result : path).absFileName();
	free(result);
	return ret;
#endif
}

} // namespace os
} // namespace support
} // namespace lyx
