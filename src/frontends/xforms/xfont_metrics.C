/**
 * \file xfont_metrics.C
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation "frontends/font_metrics.h"
#pragma implementation "frontends/xfont_metrics.h"
#endif

#include "support/lstrings.h"
#include "xfont_metrics.h"
#include "xfont_loader.h"
#include "lyxrc.h"
#include "encoding.h"
#include "language.h"

#include <boost/scoped_array.hpp>

namespace {

inline
XFontStruct * getXFontstruct(LyXFont const & f)
{
	return fontloader.load
		(f.family(), f.series(),
		f.realShape(), f.size());
}


inline
XID getFontID(LyXFont const & f)
{
	return getXFontstruct(f)->fid;
}

} // namespace anon

 
namespace font_metrics {
 
int maxAscent(LyXFont const & f)
{
	return getXFontstruct(f)->ascent;
}


int maxDescent(LyXFont const & f)
{
	return getXFontstruct(f)->descent;
}


int ascent(char c, LyXFont const & f)
{
	XFontStruct * finfo = getXFontstruct(f);
	unsigned int uc = static_cast<unsigned char>(c);
	if (finfo->per_char
	    && uc >= finfo->min_char_or_byte2
	    && uc <= finfo->max_char_or_byte2+256*finfo->max_byte1) 
		return finfo->per_char[uc - finfo->min_char_or_byte2].ascent;
	else
		return finfo->ascent;
}


int descent(char c, LyXFont const & f)
{
	XFontStruct * finfo = getXFontstruct(f);
	unsigned int uc = static_cast<unsigned char>(c);
	if (finfo->per_char
	    && uc >= finfo->min_char_or_byte2
	    && uc <= finfo->max_char_or_byte2+256*finfo->max_byte1) 
		return finfo->per_char[uc - finfo->min_char_or_byte2].descent;
	else
		return finfo->descent;
}


int lbearing(char c, LyXFont const & f)
{
	XFontStruct * finfo = getXFontstruct(f);
	unsigned int uc = static_cast<unsigned char>(c);
	if (finfo->per_char
	    && uc >= finfo->min_char_or_byte2
	    && uc <= finfo->max_char_or_byte2+256*finfo->max_byte1) 
		return finfo->per_char[uc - finfo->min_char_or_byte2].lbearing;
	else
		return 0;
}


int rbearing(char c, LyXFont const & f)
{
	XFontStruct * finfo = getXFontstruct(f);
	unsigned int uc = static_cast<unsigned char>(c);
	if (finfo->per_char
	    && uc >= finfo->min_char_or_byte2
	    && uc <= finfo->max_char_or_byte2+256*finfo->max_byte1) 
		return finfo->per_char[uc - finfo->min_char_or_byte2].rbearing;
	else
		return width(c, f);
}


int width(char const * s, size_t n, LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return n;

	if (lyxrc.font_norm_type == LyXRC::ISO_10646_1) {
		boost::scoped_array<XChar2b> xs(new XChar2b[n]);
		Encoding const * encoding = f.language()->encoding();
		LyXFont font(f);
		if (f.isSymbolFont()) {
#ifdef USE_UNICODE_FOR_SYMBOLS
			font.setFamily(LyXFont::ROMAN_FAMILY);
			font.setShape(LyXFont::UP_SHAPE);
#endif
			encoding = encodings.symbol_encoding();
		}
		for (size_t i = 0; i < n; ++i) {
			Uchar c = encoding->ucs(s[i]);
			xs[i].byte1 = c >> 8;
			xs[i].byte2 = c & 0xff;
                }
		int result = xfont_metrics::width(xs.get(), n, font);
		return result;
	}

	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		return ::XTextWidth(getXFontstruct(f), s, n);
	} else {
		// emulate smallcaps since X doesn't support this
		int result = 0;
		LyXFont smallfont(f);
		smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);
		for (size_t i = 0; i < n; ++i) {
			char const c = uppercase(s[i]);
			if (c != s[i]) {
				result += ::XTextWidth(getXFontstruct(smallfont), &c, 1);
			} else {
				result += ::XTextWidth(getXFontstruct(f), &c, 1);
			}
		}
		return result;
	}
}


int signedWidth(string const & s, LyXFont const & f)
{
	if (s.empty())
		return 0;
	if (s[0] == '-')
		return -width(s.substr(1, s.length() - 1), f);
	else
		return width(s, f);
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
 
namespace xfont_metrics {
 
int width(XChar2b const * s, int n, LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return n;
	
	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		return ::XTextWidth16(getXFontstruct(f), s, n);
	} else {
		// emulate smallcaps since X doesn't support this
		int result = 0;
		static XChar2b c;
		LyXFont smallfont(f);
		smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);
		for (int i = 0; i < n; ++i) {
			if (s[i].byte1)
				c = s[i];
			else {
				c.byte1 = s[i].byte1;
				c.byte2 = uppercase(s[i].byte2);
			}
			if (c.byte2 != s[i].byte2) {
				result += ::XTextWidth16(getXFontstruct(smallfont), &c, 1);
			} else {
				result += ::XTextWidth16(getXFontstruct(f), &s[i], 1);
		}
		}
		return result;
	}
}
 

int XTextWidth(LyXFont const & f, char const * str, int count)
{
	return ::XTextWidth(getXFontstruct(f), str, count);
}


int XTextWidth16(LyXFont const & f, XChar2b const * str, int count)
{
	return ::XTextWidth16(getXFontstruct(f), str, count);
}


/// hmm, not a metric !
void XSetFont(Display * display, GC gc, LyXFont const & f) 
{
	::XSetFont(display, gc, getFontID(f));
}
 
} // namespace xfont_metrics
