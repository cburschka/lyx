/**
 * \file xfont_metrics.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GtkmmX.h"
#include "xftFontLoader.h"
#include "font_metrics.h"
#include "lyxrc.h"
#include "encoding.h"
#include "language.h"

#include "support/lstrings.h"
#include "debug.h"

#include <gtkmm.h>

#include <boost/scoped_array.hpp>

#include <algorithm>

using lyx::char_type;
using lyx::docstring;

using std::string;


namespace {


inline XftFont * getXftFont(LyXFont const & f)
{
	return fontLoader.load(f.family(), f.series(),
			       f.realShape(), f.size());
}


inline int XGlyphAscent(XGlyphInfo const & info)
{
	return info.y;
}


inline int XGlyphDescent(XGlyphInfo const & info)
{
	return info.height - info.y;
}


inline int XGlyphLbearing(XGlyphInfo const & info)
{
	return -info.x;
}


inline int XGlyphRbearing(XGlyphInfo const & info)
{
	return -info.x + info.width;
}


inline int XGlyphLogWidth(XGlyphInfo const & info)
{
	return info.xOff;
}

} // namespace anon


int font_metrics::maxAscent(LyXFont const & f)
{
	XftFont * font = getXftFont(f);
	return font->ascent;
}


int font_metrics::maxDescent(LyXFont const & f)
{
	XftFont * font = getXftFont(f);
	return font->descent;
}


int font_metrics::ascent(char_type c,LyXFont const & f)
{
	XftFont * font = getXftFont(f);
	XGlyphInfo glyph;
	XftTextExtents32(getDisplay(), font,
			 reinterpret_cast<FcChar32 *>(&c),
			 1,
			 &glyph);
	return XGlyphAscent(glyph);
}


int font_metrics::descent(char_type c,LyXFont const & f)
{
	XftFont * font = getXftFont(f);
	XGlyphInfo glyph;
	XftTextExtents32(getDisplay(), font,
			 reinterpret_cast<FcChar32 *>(&c),
			 1,
			 &glyph);
	return XGlyphDescent(glyph);
}


int font_metrics::lbearing(char_type c,LyXFont const & f)
{
	XftFont * font = getXftFont(f);
	XGlyphInfo glyph;
	XftTextExtents32(getDisplay(), font,
			 reinterpret_cast<FcChar32 *>(&c),
			 1,
			 &glyph);
	return XGlyphLbearing(glyph);
}


int font_metrics::rbearing(char_type c,LyXFont const & f)
{
	XftFont * font = getXftFont(f);
	XGlyphInfo glyph;
	XftTextExtents32(getDisplay(), font,
			 reinterpret_cast<FcChar32 *>(&c),
			 1,
			 &glyph);
	return XGlyphRbearing(glyph);
}


int font_metrics::width(char_type const * s, size_t n, LyXFont const & f)
{
	XftFont * font = getXftFont(f);
	XGlyphInfo glyph;
	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE){
		XftTextExtents32(getDisplay(),
                                font,
                                reinterpret_cast<FcChar32 const *>(s),
                                n,
                                &glyph);
		return XGlyphLogWidth(glyph);
	} else {
		int result = 0;
		LyXFont smallfont(f);
		smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);
		XftFont * fontS = getXftFont(smallfont);
		for (size_t i = 0; i < n; ++i) {
			char_type c = lyx::support::uppercase(s[i]);
			if (c != s[i]) {
				XftTextExtents32(getDisplay(),
                                                 fontS,
						 reinterpret_cast<FcChar32 *>(&c),
						 1,
						 &glyph);
				result += XGlyphLogWidth(glyph);
			} else {
				XftTextExtents32(getDisplay(),
                                                 font,
						 reinterpret_cast<FcChar32 *>(&c),
						 1,
						 &glyph);
				result += XGlyphLogWidth(glyph);
			}
		}
		return result;
	}
}


int font_metrics::signedWidth(docstring const & s, LyXFont const & f)
{
	if (s.empty())
		return 0;
	if (s[0] == '-')
                return width(s.substr(1, s.length() - 1), f);
	else
                return width(s, f);
}


void font_metrics::rectText(docstring const & str, LyXFont const & font,
	int & width,
	int & ascent,
	int & descent)
{
	static int const d = 2;
	width = font_metrics::width(str, font) + d * 2 + 2;
	ascent = font_metrics::maxAscent(font) + d;
	descent = font_metrics::maxDescent(font) + d;
}


void font_metrics::buttonText(docstring const & str, LyXFont const & font,
	int & width,
	int & ascent,
	int & descent)
{
	static int const d = 3;

	width = font_metrics::width(str, font) + d * 2 + 2;
	ascent = font_metrics::maxAscent(font) + d;
	descent = font_metrics::maxDescent(font) + d;
}
