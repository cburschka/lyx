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

#include "FontInfo.h"

#include "GuiFontMetrics.h"

#include "Encoding.h"

#include <QFont>

#include <boost/assert.hpp>
#include <boost/scoped_ptr.hpp>

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
	virtual bool available(FontInfo const & f);
	inline virtual FontMetrics const & metrics(FontInfo const & f) {
		return *fontinfo(f).metrics.get();
	}

	/// Get the QFont for this FontInfo
	QFont const & get(FontInfo const & f) {
		return fontinfo(f).font;
	}


	/// Get font info (font + metrics) for the given LyX font.
	GuiFontInfo & fontinfo(FontInfo const & f) {
		BOOST_ASSERT(f.family() < NUM_FAMILIES);
		BOOST_ASSERT(f.series() < 2);
		BOOST_ASSERT(f.realShape() < 4);
		BOOST_ASSERT(f.size() < 10);
		// fi is a reference to the pointer type (GuiFontInfo *) in the
		// fontinfo_ table.
		GuiFontInfo * & fi =
			fontinfo_[f.family()][f.series()][f.realShape()][f.size()];
		if (!fi)
			fi = new GuiFontInfo(f);
		return *fi;
	}

private:
	/// BUTT ugly !
	GuiFontInfo * fontinfo_[NUM_FAMILIES][2][4][10];
};


} // namespace frontend
} // namespace lyx

#endif // QT4_FONTLOADER_H
