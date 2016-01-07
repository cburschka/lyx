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
extern char const * const lyx_version;
//.lyx header. We don't want lyx_version because of VCS conflicts.
extern const int lyx_version_major;
extern const int lyx_version_minor;
///
extern char const * const lyx_git_commit_hash;
///
extern char const * const lyx_release_date;

/// Package identifier (lyx[-<version-suffix>])
extern char const * const lyx_package;

/// This is the version information shown by 'lyx -version'
extern char const * const lyx_version_info;

// Do not remove the comment below, so we get merge conflict in
// independent branches. Instead add your own.
#define LYX_FORMAT_LYX 506 // guillaume munch: convert "inset-modify tabular"
#define LYX_FORMAT_TEX2LYX 506

#if LYX_FORMAT_TEX2LYX != LYX_FORMAT_LYX
#ifndef _MSC_VER
#warning "tex2lyx produces an out of date file format."
#warning "Please update tex2lyx as soon as possible, since it depends implicitly"
#warning "on the current file format in some places (this causes bugs like #7780)."
#else
#pragma message("warning: tex2lyx produces an out of date file format. " \
	"Please update tex2lyx as soon as possible, since it depends implicitly " \
	"on the current file format in some places (this causes bugs like #7780).")
#endif
#endif

//} // namespace lyx

#endif
