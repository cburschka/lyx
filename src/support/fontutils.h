// -*- C++ -*-
/**
 * \file fontutils.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 *
 * General font utilities.
 * FIXME: only MAC and WIN32 for now but it would be nice to convince
 * fontconfig to do the same for linux.
 */

#ifndef LYX_FONTUTILS_H
#define LYX_FONTUTILS_H

namespace lyx {
namespace support {

/// Add fonts to the font subsystem, must be called before Application
/// is initialized.
void addFontResources();

/// Restore original font resources, must be called after Application
/// is destroyed.
void restoreFontResources();

} // namespace support
} // namespace lyx

#endif
