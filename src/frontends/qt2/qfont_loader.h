// -*- C++ -*-
/**
 * \file qfont_loader.h
 * Copyright 1997 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QFONTLOADER_H
#define QFONTLOADER_H 

#ifdef __GNUG__
#pragma interface
#endif

#include <config.h>
#include <boost/smart_ptr.hpp>
 
#include "lyxfont.h"
 
#include "LString.h"

#include <qfont.h>
#include <qfontmetrics.h>

/**
 * Qt font loader for LyX. Matches LyXFonts against
 * actual QFont instances, and also caches metrics.
 */
class qfont_loader {
public:
	qfont_loader();

	virtual ~qfont_loader();

	/// update fonts after zoom, dpi, font names, or norm change
	virtual void update();

	/// do we have anything matching?
	virtual bool available(LyXFont const & f);
 
	/// get the QFont for this LyXFont
	QFont const & get(LyXFont const & f) {
		return getfontinfo(f)->font;
	}

	/// get the QFont metrics for this LyXFont
	QFontMetrics const & metrics(LyXFont const & f) {
		return getfontinfo(f)->metrics;
	}

private:
	/// hold info about a particular font
	struct font_info {
		font_info(LyXFont const & f);

		/// the font instance
		QFont font;
		/// metrics on the font
		QFontMetrics metrics;
	};
 
	/// get font info (font + metrics) for the given LyX font. Does not fail.
	font_info const * getfontinfo(LyXFont const & f);
 
	/// BUTT ugly !
	boost::scoped_ptr<font_info> fontinfo_[LyXFont::NUM_FAMILIES][2][4][10]; 
};

extern qfont_loader fontloader;
 
#endif // QFONT_LOADER_H
