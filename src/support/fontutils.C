/**
 * \file fontutils.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 *
 * General font utilities.
 */

#include <config.h>

#include "fontutils.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/systemcall.h"

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

#if defined(_WIN32) || (defined(__CYGWIN__) && defined(X_DISPLAY_MISSING))
#include "windows.h"
#ifdef __CYGWIN__
#undef max
#undef min
#endif
#include "support/os.h"
#include "support/package.h"
#include "support/path.h"
using lyx::support::addName;
using lyx::support::addPath;
using lyx::support::package;
namespace os = lyx::support::os;

using std::string;

string const win_fonts_truetype[] = {"cmex10", "cmmi10", "cmr10", "cmsy10",
	"eufm10", "msam10", "msbm10", "wasy10"};
const int num_fonts_truetype = sizeof(win_fonts_truetype) / sizeof(*win_fonts_truetype);
#endif


namespace lyx {
namespace support {

void addFontResources()
{
#ifdef __APPLE__
	CFBundleRef  myAppBundle = CFBundleGetMainBundle();
	CFURLRef  myAppResourcesURL, FontsURL;
	FSRef  fontDirRef;
	FSSpec  fontDirSpec;
	CFStringRef  filePath = CFStringCreateWithBytes(kCFAllocatorDefault,
					(UInt8 *) "Fonts", strlen("Fonts"),
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
#endif

#if defined(_WIN32) || (defined(__CYGWIN__) && defined(X_DISPLAY_MISSING))
	// Windows only: Add BaKoMa TrueType font resources
	string const fonts_dir = addPath(package().system_support(), "fonts");
	
	for (int i = 0 ; i < num_fonts_truetype ; ++i) {
		string const font_current = 
			addName(fonts_dir, win_fonts_truetype[i] + ".ttf");
		AddFontResource(os::external_path(font_current).c_str());
	}
#endif
}


void restoreFontResources()
{
#if defined(_WIN32) || (defined(__CYGWIN__) && defined(X_DISPLAY_MISSING))
	// Windows only: Remove BaKoMa TrueType font resources
	string const fonts_dir = addPath(package().system_support(), "fonts");
	
	for(int i = 0 ; i < num_fonts_truetype ; ++i) {
		string const font_current = 
			addName(fonts_dir, win_fonts_truetype[i] + ".ttf");
		RemoveFontResource(os::external_path(font_current).c_str());
	}
#endif
}

} //namespace support
} // namespace lyx
