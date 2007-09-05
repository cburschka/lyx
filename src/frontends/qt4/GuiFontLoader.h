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

#include "GuiFontMetrics.h"

#include "Encoding.h"
#include "Font.h"

#include <QFont>

namespace lyx {
namespace frontend {

/**
 * Qt font loader for LyX. Matches Fonts against
 * actual QFont instances, and also caches metrics.
 */
class QLFontInfo
{
public:
	QLFontInfo(Font const & f);

	/// The font instance
	QFont font;
	/// Metrics on the font
	boost::scoped_ptr<GuiFontMetrics> metrics;
};


/// Hold info about a particular font
class GuiFontLoader : public FontLoader
{
public:
	///
	GuiFontLoader();

	/// Destructor
	virtual ~GuiFontLoader() {}

	virtual void update();
	virtual bool available(Font const & f);
	inline virtual FontMetrics const & metrics(Font const & f) {
		return *fontinfo(f).metrics.get();
	}

	/// Get the QFont for this Font
	QFont const & get(Font const & f) {
		return fontinfo(f).font;
	}


	/// Get font info (font + metrics) for the given LyX font.
	QLFontInfo & fontinfo(Font const & f) {
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
	QLFontInfo * fontinfo_[Font::NUM_FAMILIES][2][4][10];
};


} // namespace frontend
} // namespace lyx

#endif // QT4_FONTLOADER_H
