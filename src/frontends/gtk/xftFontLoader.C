/**
 * \file xftFontLoader.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include <gtkmm.h>
#include <cmath>	// fabs()

#include <X11/Xft/Xft.h>
#include "xftFontLoader.h"
#include "FontInfo.h"
#include "gettext.h"
#include "debug.h"
#include "lyxrc.h"	// lyxrc.font_*
#include "BufferView.h"
#include "frontends/LyXView.h"
#include "support/systemcall.h"
#include "support/filetools.h"
#include "GtkmmX.h"
#include <vector>
#include "frontends/lyx_gui.h"

using std::endl;

// The global fontLoader
xftFontLoader fontLoader;


// Initialize font loader
xftFontLoader::xftFontLoader()
{
}


// Destroy font loader
xftFontLoader::~xftFontLoader()
{
	unload();
}


// Update fonts after zoom, dpi, font names, or norm change
// For now, we just ditch all fonts we have. Later, we should
// reuse the ones that are already loaded.
void xftFontLoader::update()
{
	unload();
}


// Unload all fonts
void xftFontLoader::unload()
{
	// Unload all fonts
	for (int i1 = 0; i1 < LyXFont::NUM_FAMILIES; ++i1)
		for (int i2 = 0; i2 < 2; ++i2)
			for (int i3 = 0; i3 < 4; ++i3)
				for (int i4 = 0; i4 < 10; ++i4) {
					if (fonts_[i1][i2][i3][i4]){
						XftFontClose(getDisplay(), fonts_[i1][i2][i3][i4]);
						fonts_[i1][i2][i3][i4] = 0;
					}
				}
}


string xftFontLoader::familyString(LyXFont::FONT_FAMILY family)
{
	string ffamily;
	switch (family) {
	case LyXFont::ROMAN_FAMILY:
		ffamily = lyxrc.roman_font_name;
		break;
	case LyXFont::SANS_FAMILY:
		ffamily = lyxrc.sans_font_name;
		break;
	case LyXFont::TYPEWRITER_FAMILY:
		ffamily = lyxrc.typewriter_font_name;
		break;
	case LyXFont::CMR_FAMILY:
		ffamily = "cmr10";
		break;
	case LyXFont::CMSY_FAMILY:
		ffamily = "cmsy10";
		break;
	case LyXFont::CMM_FAMILY:
		ffamily = "cmmi10";
		break;
	case LyXFont::CMEX_FAMILY:
		ffamily = "cmex10";
		break;
	case LyXFont::MSA_FAMILY:
		ffamily = "msam10";
		break;
	case LyXFont::MSB_FAMILY:
		ffamily = "msbm10";
		break;
	default:
		ffamily = "Sans";
		break;
	}
	return ffamily;
}


// Get font pattern
/* Takes care of finding which font that can match the given request. Tries
different alternatives. */
XftPattern * xftFontLoader::getFontPattern(LyXFont::FONT_FAMILY family,
					  LyXFont::FONT_SERIES series,
					  LyXFont::FONT_SHAPE shape,
					  LyXFont::FONT_SIZE size)
{
	// Normal font. Let's search for an existing name that matches.
	string ffamily;
	int fweight;
	int fslant;
	double fsize = lyxrc.font_sizes[size] * lyxrc.zoom / 100.0;
	XftPattern *fpat = XftPatternCreate();

	ffamily = familyString(family);
	switch (series) {
	case LyXFont::MEDIUM_SERIES:
		fweight = XFT_WEIGHT_MEDIUM;
		break;
	case LyXFont::BOLD_SERIES:
		fweight = XFT_WEIGHT_BOLD;
		break;
	default:
		fweight = XFT_WEIGHT_MEDIUM;
		break;
	}

	switch (shape) {
	case LyXFont::UP_SHAPE:
	case LyXFont::SMALLCAPS_SHAPE:
		fslant = XFT_SLANT_ROMAN;
		break;
	case LyXFont::ITALIC_SHAPE:
		fslant = XFT_SLANT_ITALIC;
		break;
	case LyXFont::SLANTED_SHAPE:
		fslant = XFT_SLANT_OBLIQUE;
		break;
	default:
		fslant = XFT_SLANT_ROMAN;
		break;
	}
	XftPatternAddString(fpat, XFT_FAMILY, ffamily.c_str());
	XftPatternAddInteger(fpat, XFT_WEIGHT, fweight);
	XftPatternAddInteger(fpat, XFT_SLANT, fslant);
	XftPatternAddDouble(fpat, XFT_SIZE, fsize);
	return fpat;
}


/// Do load font
XftFont * xftFontLoader::doLoad(LyXFont::FONT_FAMILY family,
			       LyXFont::FONT_SERIES series,
			       LyXFont::FONT_SHAPE shape,
			       LyXFont::FONT_SIZE size)
{
	XftPattern *fpat = getFontPattern(family, series, shape, size);
	XftResult result;
	XftPattern *fpat2 = XftFontMatch(getDisplay(), getScreen(),
					 fpat, &result);
	XftFont * font = XftFontOpenPattern(getDisplay(), fpat2);
	fonts_[family][series][shape][size] = font;
	return font;
}


bool xftFontLoader::available(LyXFont const & f)
{
	if (!lyx_gui::use_gui)
		return false;

	static std::vector<bool> cache_set(LyXFont::NUM_FAMILIES, false);
	static std::vector<bool> cache(LyXFont::NUM_FAMILIES, false);

	LyXFont::FONT_FAMILY family = f.family();
	if (cache_set[family])
		return cache[family];
	cache_set[family] = true;

	string const ffamily = familyString(family);
	if (isSpecial(f)) {
		cache_set[family] = true;
		XftPattern *fpat = XftPatternCreate();
		XftPatternAddString(fpat, XFT_FAMILY, ffamily.c_str());
		XftResult result;
		XftPattern *fpat2 = XftFontMatch(getDisplay(), getScreen(),
						 fpat, &result);
		XftPatternDestroy(fpat);
		char * familyM;
		XftPatternGetString(fpat2, XFT_FAMILY, 0, &familyM);
		if (ffamily == familyM) {
			cache[family] = true;
			return true;
		}
		// We don't need to set cache[family] to false, as it
		// is initialized to false;
		return false;
	}
	// We don't care about non-symbol fonts
	return false;
}
