// -*- C++ -*-
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
#include <cstdlib>	// atoi()

#ifdef __GNUG__
#pragma implementation "FontLoader.h"
#endif

#include "gettext.h"
#include "FontLoader.h"
#include "FontInfo.h"
#include "debug.h"
#include "lyxrc.h"	// lyxrc.font_*
extern LyXRC * lyxrc;
#include "BufferView.h"
#include "LyXView.h"
#include "minibuffer.h"
extern BufferView * current_view;

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
	for (int i1 = 0; i1<4; i1++)
		for (int i2 = 0; i2<2; i2++)
			for (int i3 = 0; i3<4; i3++) {
				fontinfo[i1][i2][i3] = 0;
				for (int i4 = 0; i4<10; i4++) {
					fontstruct[i1][i2][i3][i4] = 0;
				}
			}
}

// Unload all fonts
void FontLoader::unload() 
{
	// Unload all fonts
	for (int i1 = 0; i1<4; i1++)
		for (int i2 = 0; i2<2; i2++)
			for (int i3 = 0; i3<4; i3++) {
				if (fontinfo[i1][i2][i3]) {
					delete fontinfo[i1][i2][i3];
					fontinfo[i1][i2][i3] = 0;
				}
				for (int i4 = 0; i4<10; i4++) {
					if (fontstruct[i1][i2][i3][i4]) {
						XFreeFont(fl_display, fontstruct[i1][i2][i3][i4]);
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

	// Special code for the symbol family
	if (family == LyXFont::SYMBOL_FAMILY){
		fontinfo[family][series][shape] = new FontInfo("-*-symbol-*");
		return;
	}

	// Normal font. Let's search for an existing name that matches.
	string ffamily;
	string fseries;
	string fshape;
	string norm = lyxrc->font_norm;
	string fontname;

	FontInfo * fi = new FontInfo();
	fontinfo[family][series][shape] = fi;

	for (int cfam = 0; cfam < 2; ++cfam) {
		// Determine family name
		switch (family) {
		case LyXFont::ROMAN_FAMILY:
			switch (cfam) {
			case 0: ffamily = lyxrc->roman_font_name; break;
			case 1: ffamily = "-*-times";
			default: cfam = 100;
			}
			break;
		case LyXFont::SANS_FAMILY:
			switch (cfam) {
			case 0: ffamily = lyxrc->sans_font_name; break;
			case 1: ffamily = "-*-helvetica";
			default: cfam = 100;
			}
			break;
		case LyXFont::TYPEWRITER_FAMILY:
			switch (cfam) {
			case 0: ffamily = lyxrc->typewriter_font_name; break;
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

/// Do load font
XFontStruct * FontLoader::doLoad(LyXFont::FONT_FAMILY family, 
				LyXFont::FONT_SERIES series, 
				LyXFont::FONT_SHAPE shape, 
				LyXFont::FONT_SIZE size)
{
	getFontinfo(family, series, shape);
	int fsize = int( (lyxrc->font_sizes[size] * lyxrc->dpi * 
			  (lyxrc->zoom/100.0) ) / 72.27 + 0.5 );

	string font = fontinfo[family][series][shape]->getFontname(fsize);

	if (font.empty()) {
		lyxerr << "No font matches request. Using 'fixed'." << endl;
		lyxerr << "Start LyX as 'lyx -dbg 515' to get more information." << endl;
		font = "fixed";
	}

	current_view->owner()->getMiniBuffer()->Store();
	current_view->owner()->getMiniBuffer()->Set(_("Loading font into X-Server..."));

	XFontStruct * fs = XLoadQueryFont(fl_display, font.c_str());

	if (fs == 0) {
		if (font == "fixed") {
			lyxerr << "We're doomed. Can't get 'fixed' font." << endl;
		} else {
			lyxerr << "Could not get font. Using 'fixed'." << endl;
			fs = XLoadQueryFont(fl_display, "fixed");
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
		f.setLatex(LyXFont::INHERIT);
		f.setColor(LColor::inherit);
		lyxerr << "Font '" << f.stateText() 
		       << "' matched by\n" << font << endl;
	}

	current_view->owner()->getMiniBuffer()->Reset();

	fontstruct[family][series][shape][size] = fs;
	return fs;
}
