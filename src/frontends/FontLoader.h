// -*- C++ -*-
/**
 * \file FontLoader.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_FONTLOADER_H
#define LYX_FONTLOADER_H

class LyXFont;

namespace lyx {
namespace frontend {

class FontMetrics;

/// Hold info about a particular font
class FontLoader
{
public:
	///
	FontLoader() {}
	///
	virtual ~FontLoader() {}

	/// Update fonts after zoom, dpi, font names, or norm change
	virtual void update() = 0;

	/// Is the given font available ?
	virtual bool available(LyXFont const & f) = 0;

	/// Get the Font metrics for this LyXFont
	virtual FontMetrics const & metrics(LyXFont const & f) = 0;
};

} // namespace frontend
} // namespace lyx

#endif // QFONT_LOADER_H
