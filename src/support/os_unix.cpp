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

#include "support/os.h"
#include "debug.h"

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>
#elif defined(HAVE_FONTCONFIG_FONTCONFIG_H)
#include "support/filetools.h"
#include "support/Package.h"
#include <fontconfig/fontconfig.h>
using lyx::support::addPath;
#endif

using std::endl;
using std::string;


namespace lyx {
namespace support {
namespace os {

void init(int, char *[])
{}


string current_root()
{
	return "/";
}


docstring::size_type common_path(docstring const & p1, docstring const & p2)
{
	docstring::size_type i = 0;
	docstring::size_type const p1_len = p1.length();
	docstring::size_type const p2_len = p2.length();
	while (i < p1_len && i < p2_len && p1[i] == p2[i])
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
	return p;
}


string internal_path(string const & p)
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


bool is_absolute_path(string const & p)
{
	return !p.empty() && p[0] == '/';
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


char path_separator()
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


bool autoOpenFile(string const & filename, auto_open_mode const mode)
{
#ifdef __APPLE__
// Reference: http://developer.apple.com/documentation/Carbon/Reference/LaunchServicesReference/
	FSRef fileref;
	OSStatus status =
		FSPathMakeRef((UInt8 *) filename.c_str(), &fileref, NULL);
	if (status != 0)
		return false;

	// this is what we would like to do but it seems that the
	// viewer for PDF is often quicktime...
	//LSRolesMask role = (mode == VIEW) ? kLSRolesViewer :  kLSRolesEditor;
	(void)mode;
	LSRolesMask role = kLSRolesAll;
	FSRef outAppRef;

	status = LSGetApplicationForItem(&fileref, role, &outAppRef, NULL);
	if (status == kLSApplicationNotFoundErr)
		return false;

	LSLaunchFSRefSpec inLaunchSpec;
	inLaunchSpec.appRef = &outAppRef;
	inLaunchSpec.numDocs = 1;
	inLaunchSpec.itemRefs = &fileref;
	inLaunchSpec.passThruParams = NULL;
	inLaunchSpec.launchFlags = kLSLaunchDefaults;
	inLaunchSpec.asyncRefCon = NULL;
	status = LSOpenFromRefSpec(&inLaunchSpec, NULL);

	return status != kLSApplicationNotFoundErr;
#else
	// silence compiler warnings
	(void)filename;
	(void)mode;

	// currently, no default viewer is tried for non-windows system
	// support for KDE/Gnome/Macintosh may be added later
	return false;
#endif
}


void addFontResources()
{
#ifdef __APPLE__
	CFBundleRef  myAppBundle = CFBundleGetMainBundle();
	CFURLRef  myAppResourcesURL, FontsURL;
	FSRef  fontDirRef;
	FSSpec  fontDirSpec;
	CFStringRef  filePath = CFStringCreateWithBytes(kCFAllocatorDefault,
					(UInt8 *) "fonts", strlen("fonts"),
					kCFStringEncodingISOLatin1, false);

	myAppResourcesURL = CFBundleCopyResourcesDirectoryURL(myAppBundle);
	FontsURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault,
			myAppResourcesURL, filePath, true);
	if (lyxerr.debugging(Debug::FONT)) {
		UInt8  buf[255];
		if (CFURLGetFileSystemRepresentation(FontsURL, true, buf, 255))
			lyxerr << "Adding Fonts directory: " << buf << endl;
	}
	CFURLGetFSRef (FontsURL, &fontDirRef);
	OSStatus err = FSGetCatalogInfo (&fontDirRef, kFSCatInfoNone,
					 NULL, NULL, &fontDirSpec, NULL);
	if (err)
		lyxerr << "FSGetCatalogInfo err = " << err << endl;
	err = FMActivateFonts (&fontDirSpec, NULL, NULL,
			       kFMLocalActivationContext);
	if (err)
		lyxerr << "FMActivateFonts err = " << err << endl;
#elif defined(HAVE_FONTCONFIG_FONTCONFIG_H)
	// Register BaKoMa truetype fonts with fontconfig
	string const fonts_dir =
		addPath(package().system_support().absFilename(), "fonts");
	if (!FcConfigAppFontAddDir(0, (FcChar8 const *)fonts_dir.c_str()))
		lyxerr << "Unable to register fonts with fontconfig." << endl;
#endif
}


void restoreFontResources()
{
#if defined(HAVE_FONTCONFIG_FONTCONFIG_H) && !defined(__APPLE__)
	FcConfigAppFontClear(0);
#endif
}

} // namespace os
} // namespace support
} // namespace lyx
