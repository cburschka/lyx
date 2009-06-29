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

///
extern char const * lyx_version;
///
extern char const * lyx_release_date;

/// Package identifier (lyx[-<version-suffix>])
extern char const * lyx_package;

/// This is the version information shown by 'lyx -version'
extern char const * lyx_version_info;

//} // namespace lyx

#endif
