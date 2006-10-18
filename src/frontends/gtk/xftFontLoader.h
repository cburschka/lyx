// -*- C++ -*-
/**
 * \file xftFontLoader.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef XFT_FONT_LOADER_H
#define XFT_FONT_LOADER_H

#include "frontends/FontLoader.h"

#include "xftFontMetrics.h"

#include "lyxfont.h"

#include <gtkmm.h>
#include <X11/Xft/Xft.h>

class GWorkArea;


class xftFontLoader: public lyx::frontend::FontLoader {
public:
	///
	xftFontLoader();

	///
	virtual ~xftFontLoader();

	/// Update fonts after zoom, dpi, font names, or norm change
	virtual void update();

	virtual bool available(LyXFont const & f);
	virtual lyx::frontend::FontMetrics const & metrics(LyXFont const & f)
	{
		if (f.realShape() != LyXFont::SMALLCAPS_SHAPE)
			return font_metrics(load(f.family(), f.series(), f.realShape(), f.size()), 0);
		LyXFont scf(f);
		scf.decSize().decSize().setShape(LyXFont::UP_SHAPE);
		return font_metrics(load(f.family(), f.series(), f.realShape(), f.size()),
				load(scf.family(), scf.series(), scf.realShape(), scf.size()));
	}

	/// Load font
	XftFont * load(LyXFont::FONT_FAMILY family,
		      LyXFont::FONT_SERIES series,
		      LyXFont::FONT_SHAPE shape,
		      LyXFont::FONT_SIZE size)
	{
		if (fonts_[family][series][shape][size])
			return fonts_[family][series][shape][size];
		else
			return doLoad(family, series, shape, size);
	}

	bool isSpecial(LyXFont const & f)
	{
		switch (f.family()) {
		case LyXFont::CMR_FAMILY:
		case LyXFont::EUFRAK_FAMILY:
			return true;
		default:
			break;
		}
		return f.isSymbolFont();
	}
private:
	/// Array of fonts
	XftFont * fonts_[LyXFont::NUM_FAMILIES][2][4][10];
	XftPattern * getFontPattern(LyXFont::FONT_FAMILY family,
				    LyXFont::FONT_SERIES series,
				    LyXFont::FONT_SHAPE shape,
				    LyXFont::FONT_SIZE size);
	std::string familyString(LyXFont::FONT_FAMILY family);
	/// Reset font handler
	void reset();

	/// Unload all fonts
	void unload();

	/** Does the actual loading of a font. Updates fontstruct. */
	XftFont * doLoad(LyXFont::FONT_FAMILY family,
			 LyXFont::FONT_SERIES series,
			 LyXFont::FONT_SHAPE shape,
			 LyXFont::FONT_SIZE size);
};

///
extern xftFontLoader fontLoader;

#endif
