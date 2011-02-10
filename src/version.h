// -*- C++ -*-
/**
 * \file version.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef VERSION_H
#define VERSION_H

//namespace lyx {

///About dialog, inset info, export headers
extern char const * lyx_version;
//.lyx header. We don't want lyx_version because of VCS conflicts.
extern int lyx_version_major;
extern int lyx_version_minor;
///
extern char const * lyx_release_date;

/// Package identifier (lyx[-<version-suffix>])
extern char const * lyx_package;

/// This is the version information shown by 'lyx -version'
extern char const * lyx_version_info;

//} // namespace lyx

#endif
