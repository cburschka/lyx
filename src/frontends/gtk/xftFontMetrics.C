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
#include <gtkmm.h>

#include <algorithm>
#include "GtkmmX.h"
#include "support/lstrings.h"
#include "xftFontLoader.h"
#include "font_metrics.h"
#include "lyxrc.h"
#include "encoding.h"
#include "language.h"
#include "codeConvert.h"

#include <boost/scoped_array.hpp>


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


wchar_t C2WC(char ch)
{
	wchar_t wcs[2] = {0, 0};
	char mbs[2] = {0, 0};
	mbs[0] = ch;
	mbstowcs(wcs, mbs, 2);
	return wcs[0];
}


} // namespace anon


namespace font_metrics {


int maxAscent(LyXFont const & f)
{
	XftFont * font = getXftFont(f);
	return font->ascent;
}


int maxDescent(LyXFont const & f)
{
	XftFont * font = getXftFont(f);
	return font->descent;
}


int ascent(wchar_t c,LyXFont const & f)
{
	XftFont * font = getXftFont(f);
	XGlyphInfo glyph;
	XftTextExtents32(getDisplay(), font,
			 wcsToFcChar32StrFast(&c),
			 1,
			 &glyph);
	return XGlyphAscent(glyph);
}


int ascent(char c, LyXFont const & f)
{
	return ascent(C2WC(c), f);
}


int descent(wchar_t c,LyXFont const & f)
{
	XftFont * font = getXftFont(f);
	XGlyphInfo glyph;
	XftTextExtents32(getDisplay(), font,
			 wcsToFcChar32StrFast(&c),
			 1,
			 &glyph);
	return XGlyphDescent(glyph);
}


int descent(char c, LyXFont const & f)
{
	return descent(C2WC(c), f);
}


int lbearing(wchar_t c,LyXFont const & f)
 {
	XftFont * font = getXftFont(f);
	XGlyphInfo glyph;
	XftTextExtents32(getDisplay(), font,
			 wcsToFcChar32StrFast(&c),
			 1,
			 &glyph);
	return XGlyphLbearing(glyph);
 }


int rbearing(wchar_t c,LyXFont const & f)
{
	XftFont * font = getXftFont(f);
	XGlyphInfo glyph;
	XftTextExtents32(getDisplay(), font,
			 wcsToFcChar32StrFast(&c),
			 1,
			 &glyph);
	return XGlyphRbearing(glyph);
}


int lbearing(char c, LyXFont const & f)
{
	return lbearing(C2WC(c), f);
}


int rbearing(char c, LyXFont const & f)
{
	return rbearing(C2WC(c), f);
}


int width(wchar_t const * s, size_t n, LyXFont const & f)
{
	XftFont * font = getXftFont(f);
	XGlyphInfo glyph;
	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE){
		XftTextExtents32(getDisplay(), font,
				 wcsToFcChar32StrFast(s),
				 n,
				 &glyph);
		return XGlyphLogWidth(glyph);
	} else {
		int result = 0;
		LyXFont smallfont(f);
		smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);
		XftFont * fontS = getXftFont(smallfont);
		for (size_t i = 0; i < n; ++i) {
			wchar_t wc = lyx::support::uppercase(s[i]);
			if (wc != s[i]) {
				XftTextExtents32(getDisplay(), fontS,
						 wcsToFcChar32StrFast(&wc),
						 1,
						 &glyph);
				result += XGlyphLogWidth(glyph);
			} else {
				XftTextExtents32(getDisplay(), font,
						 wcsToFcChar32StrFast(&wc),
						 1,
						 &glyph);
				result += XGlyphLogWidth(glyph);
			}
		}
		return result;
	}
}


int width(wchar_t c,LyXFont const & f)
{
	return width(&c, 1, f);
}


int width(char const * s, size_t n,LyXFont const & f)
{
	boost::scoped_array<wchar_t> wcs(new wchar_t[n]);
	size_t len;
	if (fontLoader.isSpecial(f)) {
		unsigned char const * us =
			reinterpret_cast<unsigned char const *>(s);
		len = n;
		std::copy(us, us + n, wcs.get());
	} else
		len = mbstowcs(wcs.get(), s, n);
	return width(wcs.get(), len, f);
}


int signedWidth(string const & s, LyXFont const & f)
{
	if (s.empty())
		return 0;
	boost::scoped_array<wchar_t> wcs(new wchar_t[s.length() + 1]);
	int len = mbstowcs(wcs.get(), s.c_str(), s.length());
	if (wcs[0] == '-')
		return width(wcs.get() + 1, len - 1, f);
	else
		return width(wcs.get(), len, f);
}


void rectText(string const & str, LyXFont const & font,
	int & width,
	int & ascent,
	int & descent)
{
	static int const d = 2;
	width = font_metrics::width(str, font) + d * 2 + 2;
	ascent = font_metrics::maxAscent(font) + d;
	descent = font_metrics::maxDescent(font) + d;
}


void buttonText(string const & str, LyXFont const & font,
	int & width,
	int & ascent,
	int & descent)
{
	static int const d = 3;

	width = font_metrics::width(str, font) + d * 2 + 2;
	ascent = font_metrics::maxAscent(font) + d;
	descent = font_metrics::maxDescent(font) + d;
}


} // namespace font_metrics
