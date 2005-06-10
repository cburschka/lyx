// -*- C++ -*-
/**
 * \file qfont_loader.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QFONTLOADER_H
#define QFONTLOADER_H

#if QT_VERSION < 0x030100
#define USE_LYX_FONTCACHE
#endif

#if defined(USE_LYX_FONTCACHE)
#include <map>
#endif

#include "encoding.h"
#include "lyxfont.h"

#include <qfont.h>
#include <qfontmetrics.h>

/**
 * Qt font loader for LyX. Matches LyXFonts against
 * actual QFont instances, and also caches metrics.
 */
class qfont_loader {
public:
	qfont_loader();

	~qfont_loader();

	/// update fonts after zoom, dpi, font names, or norm change
	void update();

	/// do we have anything matching?
	bool available(LyXFont const & f);

	/// get the QFont for this LyXFont
	QFont const & get(LyXFont const & f);

	/// get the QFont metrics for this LyXFont
	QFontMetrics const & metrics(LyXFont const & f) {
		return getfontinfo(f)->metrics;
	}

	/// return pixel width for the given unicode char
	int charwidth(LyXFont const & f, Uchar val);

	/// Called before QApplication is initialized
	static void initFontPath();

	/// Called the first time when available() can't load a symbol font
	static void addToFontPath();

private:
	/// hold info about a particular font
	class font_info {
	public:
		font_info(LyXFont const & f);

		/// the font instance
		QFont font;
		/// metrics on the font
		QFontMetrics metrics;

#if defined(USE_LYX_FONTCACHE)
		typedef std::map<Uchar, int> WidthCache;
		/// cache of char widths
		WidthCache widthcache;
#endif
	};

	/// get font info (font + metrics) for the given LyX font. Does not fail.
	font_info * getfontinfo(LyXFont const & f);

	/// BUTT ugly !
	font_info * fontinfo_[LyXFont::NUM_FAMILIES][2][4][10];
};

extern qfont_loader fontloader;

#endif // QFONT_LOADER_H
