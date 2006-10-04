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

#ifndef QT4_FONTLOADER_H
#define QT4_FONTLOADER_H

#include "frontends/FontLoader.h"

#include "encoding.h"
#include "lyxfont.h"

#include <QFont>
#include <QFontMetrics>

// Starting with version 3.1.0, Qt/X11 does its own caching of
// character width, so it is not necessary to provide ours.
#if defined(Q_WS_MACX) || defined(Q_WS_WIN32)
#define USE_LYX_FONTCACHE
#include <map>
#endif

namespace lyx {
namespace frontend {

/**
 * Qt font loader for LyX. Matches LyXFonts against
 * actual QFont instances, and also caches metrics.
 */
class QLFontInfo {
public:
	QLFontInfo(LyXFont const & f);

	/// The font instance
	QFont font;
	/// Metrics on the font
	QFontMetrics metrics;

#ifndef USE_LYX_FONTCACHE
	/// Return pixel width for the given unicode char
	int width(Uchar val) { return metrics.width(QChar(val)); }

#else
	/// Return pixel width for the given unicode char
	int width(Uchar val);

private:
	typedef std::map<Uchar, int> WidthCache;
	/// Cache of char widths
	WidthCache widthcache;
#endif // USE_LYX_FONTCACHE
};


/// Hold info about a particular font
class GuiFontLoader: public FontLoader 
{
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


} // namespace frontend
} // namespace lyx

#endif // QT4_FONTLOADER_H
