/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1997 Asger Alstrup
 *           and the LyX Team.
 *
 * ====================================================== */

#include <config.h>
#include <cmath>	// fabs()

#ifdef __GNUG__
#pragma implementation "FontLoader.h"
#endif

#include "FontLoader.h"
#include "FontInfo.h"
#include "gettext.h"
#include "debug.h"
#include "lyxrc.h"	// lyxrc.font_*
#include "BufferView.h"
#include "LyXView.h"
#include "frontends/GUIRunTime.h"

using std::endl;

extern BufferView * current_view;


// The global fontloader
FontLoader fontloader;


// Initialize font loader
FontLoader::FontLoader()
{
	reset();
}


// Destroy font loader
FontLoader::~FontLoader()
{
	unload();
}


// Update fonts after zoom, dpi, font names, or norm change
// For now, we just ditch all fonts we have. Later, we should
// reuse the ones that are already loaded.
void FontLoader::update()
{
	unload();
}


// Reset font loader
void FontLoader::reset()
{
	// Clear font infos, font structs and font metrics
	for (int i1 = 0; i1 < LyXFont::NUM_FAMILIES; ++i1)
		for (int i2 = 0; i2 < 2; ++i2)
			for (int i3 = 0; i3 < 4; ++i3) {
				fontinfo[i1][i2][i3] = 0;
				for (int i4 = 0; i4<10; ++i4) {
					fontstruct[i1][i2][i3][i4] = 0;
				}
			}
}


// Unload all fonts
void FontLoader::unload() 
{
	// Unload all fonts
	for (int i1 = 0; i1 < LyXFont::NUM_FAMILIES; ++i1)
		for (int i2 = 0; i2 < 2; ++i2)
			for (int i3 = 0; i3 < 4; ++i3) {
				if (fontinfo[i1][i2][i3]) {
					delete fontinfo[i1][i2][i3];
					fontinfo[i1][i2][i3] = 0;
				}
				for (int i4 = 0; i4 < 10; ++i4) {
					if (fontstruct[i1][i2][i3][i4]) {
						XFreeFont(GUIRunTime::x11Display(), fontstruct[i1][i2][i3][i4]);
						fontstruct[i1][i2][i3][i4] = 0;
					}
				}
			}
}


// Get font info
/* Takes care of finding which font that can match the given request. Tries
different alternatives. */
void FontLoader::getFontinfo(LyXFont::FONT_FAMILY family, 
			     LyXFont::FONT_SERIES series, 
			     LyXFont::FONT_SHAPE shape)
{
	// Do we have the font info already?
	if (fontinfo[family][series][shape] != 0)
		return;

	// Special fonts
	switch (family) 
	{
		case LyXFont::SYMBOL_FAMILY:
			fontinfo[family][series][shape] =
				new FontInfo("-*-symbol-*-*-*-*-*-*-*-*-*-*-adobe-fontspecific");
			return;

		case LyXFont::CMR_FAMILY:
			fontinfo[family][series][shape] =
				new FontInfo("-*-cmr10-medium-*-*-*-*-*-*-*-*-*-*-*");
			return;

		case LyXFont::CMSY_FAMILY:
			fontinfo[family][series][shape] =
				new FontInfo("-*-cmsy10-*-*-*-*-*-*-*-*-*-*-*-*");
			return;

		case LyXFont::CMM_FAMILY:
			fontinfo[family][series][shape] =
				new FontInfo("-*-cmmi10-medium-*-*-*-*-*-*-*-*-*-*-*");
			return;

		case LyXFont::CMEX_FAMILY:
			fontinfo[family][series][shape] =
				new FontInfo("-*-cmex10-*-*-*-*-*-*-*-*-*-*-*-*");
			return;

		case LyXFont::MSA_FAMILY:
			fontinfo[family][series][shape] =
				new FontInfo("-*-msam10-*-*-*-*-*-*-*-*-*-*-*-*");
			return;

		case LyXFont::MSB_FAMILY:
			fontinfo[family][series][shape] = 
				new FontInfo("-*-msbm10-*-*-*-*-*-*-*-*-*-*-*-*");
			return;

		case LyXFont::EUFRAK_FAMILY:
			fontinfo[family][series][shape] = 
				new FontInfo("-*-eufm10-medium-*-*-*-*-*-*-*-*-*-*-*");
			return;

		default:
			break;
	}
 

	// Normal font. Let's search for an existing name that matches.
	string ffamily;
	string fseries;
	string fshape;
	string norm = lyxrc.font_norm;
	string fontname;

	FontInfo * fi = new FontInfo();
	fontinfo[family][series][shape] = fi;

	for (int cfam = 0; cfam < 2; ++cfam) {
		// Determine family name
		switch (family) {
		case LyXFont::ROMAN_FAMILY:
			switch (cfam) {
			case 0: ffamily = lyxrc.roman_font_name; break;
			case 1: ffamily = "-*-times";
			default: cfam = 100;
			}
			break;
		case LyXFont::SANS_FAMILY:
			switch (cfam) {
			case 0: ffamily = lyxrc.sans_font_name; break;
			case 1: ffamily = "-*-helvetica";
			default: cfam = 100;
			}
			break;
		case LyXFont::TYPEWRITER_FAMILY:
			switch (cfam) {
			case 0: ffamily = lyxrc.typewriter_font_name; break;
			case 1: ffamily = "-*-courier";
			default: cfam = 100;
			}
			break;
		default: ;
		}

		for (int cser = 0; cser < 4; ++cser) {
			// Determine series name
			switch (series) {
			case LyXFont::MEDIUM_SERIES:
				switch (cser) {
				case 0: fseries = "-medium"; break;
				case 1: fseries = "-book"; break;
				case 2: fseries = "-light";
				default: cser = 100;
				}
				break;
			case LyXFont::BOLD_SERIES:
				switch (cser) {
				case 0: fseries = "-bold"; break;
				case 1: fseries = "-black"; break;
				case 2: fseries = "-demi"; break;
				case 3: fseries = "-demibold";
				default: cser = 100;
				}
				break;
			default: ;
			}

			for (int csha = 0; csha < 2; ++csha) {
				// Determine shape name
				switch (shape) {
				case LyXFont::UP_SHAPE:
				case LyXFont::SMALLCAPS_SHAPE:
					switch (csha) {
					case 0: fshape = "-r";
					default: csha = 100;
					}
					break;
				case LyXFont::ITALIC_SHAPE:
					switch (csha) {
					case 0: fshape = "-i"; break;
					case 1: fshape = "-o";
					default: csha = 100;
					}
					break;
				case LyXFont::SLANTED_SHAPE:
					switch (csha) {
					case 0: fshape = "-o"; break;
					case 1: fshape = "-i";
					default: csha = 100;
					}
					break;
				default: ;
				}
				//
				fontname = ffamily + fseries + fshape +
					   "-normal-*-*-*-*-*-*-*-" + norm;
				fi->setPattern(fontname);
				if (fi->exist()) {
					return;
				}
			}
		}
	}
}


// A dummy fontstruct used when there is no gui.
namespace {

XFontStruct dummyXFontStruct;
bool dummyXFontStructisGood = false;

} // namespace anon

/// Do load font
XFontStruct * FontLoader::doLoad(LyXFont::FONT_FAMILY family, 
				LyXFont::FONT_SERIES series, 
				LyXFont::FONT_SHAPE shape, 
				LyXFont::FONT_SIZE size)
{
	if (!lyxrc.use_gui) {
		if (!dummyXFontStructisGood) {
			// no character specific info
			dummyXFontStruct.per_char = 0; 
			// unit ascent on character displays
			dummyXFontStruct.ascent = 1; 
			// no descent on character displays
			dummyXFontStruct.descent = 0; 
			dummyXFontStructisGood = true;
		}

		return &dummyXFontStruct;
	}

	getFontinfo(family, series, shape);
	int fsize = int( (lyxrc.font_sizes[size] * lyxrc.dpi * 
			  (lyxrc.zoom/100.0) ) / 72.27 + 0.5 );

	string font = fontinfo[family][series][shape]->getFontname(fsize);

	if (font.empty()) {
		lyxerr << "No font matches request. Using 'fixed'." << endl;
		lyxerr << "Start LyX as 'lyx -dbg 515' to get more information." << endl;
		font = "fixed";
	}

	XFontStruct * fs = 0;

	current_view->owner()->messagePush(_("Loading font into X-Server..."));

	fs = XLoadQueryFont(GUIRunTime::x11Display(), font.c_str());
	
	if (fs == 0) {
		if (font == "fixed") {
			lyxerr << "We're doomed. Can't get 'fixed' font." << endl;
		} else {
			lyxerr << "Could not get font. Using 'fixed'." << endl;
			fs = XLoadQueryFont(GUIRunTime::x11Display(), "fixed");
		}
	} else if (lyxerr.debugging(Debug::FONT)) {
		// Tell user the font matching
		LyXFont f;
		f.setFamily(family);
		f.setSeries(series);
		f.setShape(shape);
		f.setSize(size);
		// The rest of the attributes are not interesting
		f.setEmph(LyXFont::INHERIT);
		f.setUnderbar(LyXFont::INHERIT);
		f.setNoun(LyXFont::INHERIT);
		f.setColor(LColor::inherit);
		lyxerr << "Font '" << f.stateText(0) 
		       << "' matched by\n" << font << endl;
	}

	current_view->owner()->messagePop();

	fontstruct[family][series][shape][size] = fs;
	return fs;
}


bool FontLoader::available(LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return false;

	if (!fontinfo[f.family()][f.series()][f.realShape()])
		getFontinfo(f.family(), f.series(), f.realShape());
	return fontinfo[f.family()][f.series()][f.realShape()]
		->getFontname(f.size()).size();
}
