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

#include "frontends/FontLoader.h"

#include "encoding.h"
#include "lyxfont.h"

#include <qfont.h>
#include <qfontmetrics.h>

#if QT_VERSION < 0x030100 || defined(Q_WS_MACX)
#define USE_LYX_FONTCACHE
#endif

#if defined(USE_LYX_FONTCACHE)
#include <map>
#endif

/**
 * Qt font loader for LyX. Matches LyXFonts against
 * actual QFont instances, and also caches metrics.
 */
class QLFontInfo {
public:
	QLFontInfo(LyXFont const & f);

	/// Return pixel width for the given unicode char
	int width(Uchar val);

	/// The font instance
	QFont font;
	/// Metrics on the font
	QFontMetrics metrics;

#if defined(USE_LYX_FONTCACHE)
	typedef std::map<Uchar, int> WidthCache;
	/// Cache of char widths
	WidthCache widthcache;
#endif
};


/// Hold info about a particular font
class GuiFontLoader: public lyx::frontend::FontLoader {
public:
	///
	GuiFontLoader();
	
	/// Destructor
	virtual ~GuiFontLoader();

	/// Update fonts after zoom, dpi, font names, or norm change
	virtual void update();

	/// Do we have anything matching?
	virtual bool available(LyXFont const & f);

	/// Get the QFont for this LyXFont
	QFont const & get(LyXFont const & f) {
		return fontinfo(f).font;
	}

	/// Get the QFont metrics for this LyXFont
	QFontMetrics const & metrics(LyXFont const & f) {
		return fontinfo(f).metrics;
	}

	/// Called the first time when available() can't load a symbol font
	static void addToFontPath();

	/// Get font info (font + metrics) for the given LyX font.
	QLFontInfo & fontinfo(LyXFont const & f) {
		// fi is a reference to the pointer type (QLFontInfo *) in the
		// fontinfo_ table.
		QLFontInfo * & fi =
			fontinfo_[f.family()][f.series()][f.realShape()][f.size()];
		if (!fi)
			fi = new QLFontInfo(f);
		return *fi;
	}

private:
	/// BUTT ugly !
	QLFontInfo * fontinfo_[LyXFont::NUM_FAMILIES][2][4][10];
};

extern GuiFontLoader fontloader;

#endif // QFONT_LOADER_H
