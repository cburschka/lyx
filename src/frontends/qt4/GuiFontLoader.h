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

#include "GuiFontMetrics.h"

#include <QFont>

namespace lyx {
namespace frontend {

/**
 * Qt font loader for LyX. Matches Fonts against
 * actual QFont instances, and also caches metrics.
 */
class GuiFontInfo
{
public:
	GuiFontInfo(FontInfo const & f);

	/// The font instance
	QFont font;
	/// Metrics on the font
	GuiFontMetrics metrics;
};


// Load font
GuiFontInfo const & getFontInfo(FontInfo const & f);
/// Get the QFont for this FontInfo
QFont const & getFont(FontInfo const & f);

} // namespace frontend
} // namespace lyx

#endif // GUI_FONTLOADER_H
