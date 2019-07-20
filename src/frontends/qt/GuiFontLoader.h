// -*- C++ -*-
/**
 * \file GuiFontLoader.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUI_FONTLOADER_H
#define GUI_FONTLOADER_H

class QFont;

namespace lyx {

class FontInfo;

namespace frontend {

class GuiFontMetrics;

/// Metrics on the font
GuiFontMetrics const & getFontMetrics(FontInfo const & f);
/// Get the QFont for this FontInfo
QFont const & getFont(FontInfo const & f);

} // namespace frontend
} // namespace lyx

#endif // GUI_FONTLOADER_H
