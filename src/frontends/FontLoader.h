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

#ifndef FONTLOADER_H
#define FONTLOADER_H

#include "support/strfwd.h"

namespace lyx {

class FontInfo;

namespace frontend {

class FontMetrics;

/// Hold info about a particular font
class FontLoader
{
public:
	///
	FontLoader();
	/// Clears cache
	~FontLoader();

	/// Update fonts after zoom, dpi, font names, or norm change
	// (basically by deleting all cached values)	
	void update();

	/// Is the given font available ?
	bool available(FontInfo const & f);

	/// Can the given symbol be displayed in general?
	bool canBeDisplayed(char_type c);

	/// Get the Font metrics for this FontInfo
	FontMetrics const & metrics(FontInfo const & f);
};


} // namespace frontend

/// Implementation is in Application.cpp
extern frontend::FontLoader & theFontLoader();

} // namespace lyx

#endif // FONTLOADER_H
