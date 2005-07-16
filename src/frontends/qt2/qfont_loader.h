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

#include "encoding.h"
#include "lyxfont.h"

#include <qfont.h>
#include <qfontmetrics.h>

#if QT_VERSION < 0x030100
#define USE_LYX_FONTCACHE
#endif

#if defined(USE_LYX_FONTCACHE)
#include <map>
#endif

/**
 * Qt font loader for LyX. Matches LyXFonts against
 * actual QFont instances, and also caches metrics.
 */
class FontInfo {
public:
	FontInfo(LyXFont const & f);

	/// return pixel width for the given unicode char
	int width(Uchar val) const;

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


class FontLoader {
public:
	/// hold info about a particular font
	///
	FontLoader();

	/// update fonts after zoom, dpi, font names, or norm change
	void update();

	/// do we have anything matching?
	bool available(LyXFont const & f);

	/// get the QFont for this LyXFont
	QFont const & get(LyXFont const & f) {
		return fontinfo(f).font;
	}

	/// get the QFont metrics for this LyXFont
	QFontMetrics const & metrics(LyXFont const & f) {
		return fontinfo(f).metrics;
	}

	/// Called before QApplication is initialized
	static void initFontPath();

	/// Called the first time when available() can't load a symbol font
	static void addToFontPath();

	/// get font info (font + metrics) for the given LyX font. Does not fail.
	FontInfo & fontinfo(LyXFont const & f) {
		FontInfo * & fi =
			fontinfo_[f.family()][f.series()][f.realShape()][f.size()];
		if (!fi)
			fi = new FontInfo(f);
		return *fi;
	}

private:
	/// BUTT ugly !
	FontInfo * fontinfo_[LyXFont::NUM_FAMILIES][2][4][10];
};

extern FontLoader fontloader;

#endif // QFONT_LOADER_H
