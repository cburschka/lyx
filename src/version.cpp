/* -*- C++ -*- */
/**
 * \file version.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#if ! defined(LYX_DATE)
	#include <lyx_date.h>
#endif

///
extern char const * const lyx_version = PACKAGE_VERSION;
///
extern const int lyx_version_major = LYX_MAJOR_VERSION;
extern const int lyx_version_minor = LYX_MINOR_VERSION;
///
extern char const * const lyx_git_commit_hash = LYX_GIT_COMMIT_HASH;
///
extern char const * const lyx_release_date = LYX_DATE;

/// Package identifier (lyx[-<version-suffix>])
extern char const * const lyx_package = PACKAGE;

/// This is the version information shown by 'lyx --version'
extern char const * const lyx_version_info = VERSION_INFO;
