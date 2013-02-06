/**
 * \file Path.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/PathChanger.h"

// undef PathChanger macro when building PathChanger
#undef PathChanger


namespace lyx {
namespace support {

PathChanger::PathChanger(FileName const & path)
	: popped_(false)
{
	if (!path.empty()) {
		pushedDir_ = FileName::getcwd();

		if (pushedDir_.empty() || !path.chdir()) {
			/* FIXME: throw */
		}
	} else {
		popped_ = true;
	}
}


PathChanger::~PathChanger()
{
	if (!popped_)
		pop();
}


int PathChanger::pop()
{
	if (popped_) {
		// should throw an exception
		// throw logical_error();
		return 0;
	}

	if (!pushedDir_.chdir()) {
		// should throw an exception
		// throw DirChangeError();
	}
	popped_ = true;

	return 0;
}

} // namespace support
} // namespace lyx


#define PathChanger(x) unnamed_PathChanger;

#ifdef __APPLE__

#include <CoreServices/CoreServices.h>

int getPrivateFrameworkPathName(char * buf, unsigned len, char const * framework)
{
	// Get our application's main bundle from Core Foundation
	CFBundleRef myAppsBundle = CFBundleGetMainBundle();
	int result = 0 ;
	if (NULL != myAppsBundle) {
		CFURLRef baseURL = CFBundleCopyPrivateFrameworksURL( myAppsBundle );
		if (NULL != baseURL) {
			CFURLRef bundleURL = CFURLCreateCopyAppendingPathComponent( kCFAllocatorSystemDefault, baseURL,
				CFStringCreateWithCString( kCFAllocatorSystemDefault, framework, CFStringGetSystemEncoding() ),
				false );
			if (NULL != bundleURL) {
				result = CFURLGetFileSystemRepresentation( bundleURL, TRUE, (UInt8*)buf, len );
			}
		}
	}
	return result;
}

#endif

// in merged builds this is not the last line.
