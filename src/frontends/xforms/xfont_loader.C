/**
 * \file xfont_loader.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>
#include <cmath>	// fabs()

#ifdef __GNUG__
#pragma implementation
#endif

#include "xfont_loader.h"
#include "FontInfo.h"
#include "gettext.h"
#include "debug.h"
#include "lyxrc.h"	// lyxrc.font_*
#include "BufferView.h"
#include "frontends/LyXView.h"
#include "support/systemcall.h"
#include "support/filetools.h"

#include FORMS_H_LOCATION

using std::endl;

// The global fontloader
xfont_loader fontloader;


// Initialize font loader
xfont_loader::xfont_loader()
{
	reset();
}


// Destroy font loader
xfont_loader::~xfont_loader()
{
	unload();
}


// Update fonts after zoom, dpi, font names, or norm change
// For now, we just ditch all fonts we have. Later, we should
// reuse the ones that are already loaded.
void xfont_loader::update()
{
	unload();
}


// Reset font loader
void xfont_loader::reset()
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
void xfont_loader::unload()
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
						XFreeFont(fl_get_display(), fontstruct[i1][i2][i3][i4]);
						fontstruct[i1][i2][i3][i4] = 0;
					}
				}
			}
}

namespace {
string const symbolPattern(LyXFont::FONT_FAMILY family)
{
	switch (family) {
	case LyXFont::SYMBOL_FAMILY:
		return "-*-symbol-*-*-*-*-*-*-*-*-*-*-adobe-fontspecific";

	case LyXFont::CMR_FAMILY:
		return "-*-cmr10-medium-*-*-*-*-*-*-*-*-*-*-*";

	case LyXFont::CMSY_FAMILY:
		return "-*-cmsy10-*-*-*-*-*-*-*-*-*-*-*-*";

	case LyXFont::CMM_FAMILY:
		return "-*-cmmi10-medium-*-*-*-*-*-*-*-*-*-*-*";

	case LyXFont::CMEX_FAMILY:
		return "-*-cmex10-*-*-*-*-*-*-*-*-*-*-*-*";

	case LyXFont::MSA_FAMILY:
		return "-*-msam10-*-*-*-*-*-*-*-*-*-*-*-*";

	case LyXFont::MSB_FAMILY:
		return "-*-msbm10-*-*-*-*-*-*-*-*-*-*-*-*";

	case LyXFont::EUFRAK_FAMILY:
		return "-*-eufm10-medium-*-*-*-*-*-*-*-*-*-*-*";

	case LyXFont::WASY_FAMILY:
		return "-*-wasy10-medium-*-*-*-*-*-*-*-*-*-*-*";

	default:
		return string();
	}	
}

}

// Get font info
/* Takes care of finding which font that can match the given request. Tries
different alternatives. */
void xfont_loader::getFontinfo(LyXFont::FONT_FAMILY family,
			     LyXFont::FONT_SERIES series,
			     LyXFont::FONT_SHAPE shape)
{
	// Do we have the font info already?
	if (fontinfo[family][series][shape] != 0)
		return;

	// Special fonts
	string pat = symbolPattern(family);
	if (!pat.empty()) {
		static bool first_time = true;
		fontinfo[family][series][shape] = new FontInfo(pat);
		if (family != LyXFont::SYMBOL_FAMILY &&
		    !fontinfo[family][series][shape]->exist() &&
		    first_time) {
			first_time = false;
			string const dir = 
				OnlyPath(LibFileSearch("xfonts", "fonts.dir"));
			if (!dir.empty()) {
				int n;
				char ** p = XGetFontPath(fl_get_display(), &n);
				if (std::find(p, p+n, dir) != p+n)
					return;
				lyxerr << "Adding " << dir << " to the font path.\n";
				string const command = "xset fp+ " + dir;
				Systemcall s;
				if (!s.startscript(Systemcall::Wait, command)) {
					delete fontinfo[family][series][shape];
					fontinfo[family][series][shape] = new FontInfo(pat);	
				} else
					lyxerr << "Unable to add font path.\n";
			}
		}
		return;
	}


	// Normal font. Let's search for an existing name that matches.
	string ffamily;
	string fseries;
	string fshape;
	string norm = lyxrc.font_norm;
	string fontname;

	FontInfo * fi = new FontInfo;
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
XFontStruct * xfont_loader::doLoad(LyXFont::FONT_FAMILY family,
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
	// FIXME! CHECK! Should we use 72.0 or 72.27? (Lgb)
	int fsize = int((lyxrc.font_sizes[size] * lyxrc.dpi *
			  (lyxrc.zoom/100.0)) / 72.27 + 0.5);

	string font = fontinfo[family][series][shape]->getFontname(fsize);

	if (font.empty()) {
		lyxerr << "No font matches request. Using 'fixed'." << endl;
		lyxerr << "Start LyX as 'lyx -dbg 515' to get more information." << endl;
		font = "fixed";
	}

	XFontStruct * fs = 0;

	fs = XLoadQueryFont(fl_get_display(), font.c_str());

	if (fs == 0) {
		if (font == "fixed") {
			lyxerr << "We're doomed. Can't get 'fixed' font." << endl;
		} else {
			lyxerr << "Could not get font '" << font
				<< "'. Using 'fixed'." << endl;
			fs = XLoadQueryFont(fl_get_display(), "fixed");
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

	fontstruct[family][series][shape][size] = fs;
	return fs;
}


bool xfont_loader::available(LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return false;

	if (!fontinfo[f.family()][f.series()][f.realShape()])
		getFontinfo(f.family(), f.series(), f.realShape());
	return fontinfo[f.family()][f.series()][f.realShape()]->exist();
}
