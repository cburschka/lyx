/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <cctype>

#include "font.h"
#include "FontLoader.h"
#include "lyxrc.h"
#include "encoding.h"

// namespace {
static inline
XFontStruct * getXFontstruct(LyXFont const & f)
{
	return fontloader.load(f.family(), f.series(),
			       f.realShape(), f.size());
}


static inline
XID getFontID(LyXFont const & f)
{
	return getXFontstruct(f)->fid;
}
// } // end of anon namespace

int lyxfont::maxAscent(LyXFont const & f)
{
	return getXFontstruct(f)->ascent;
}


int lyxfont::maxDescent(LyXFont const & f)
{
	return getXFontstruct(f)->descent;
}


int lyxfont::ascent(char c, LyXFont const & f)
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


int lyxfont::descent(char c, LyXFont const & f)
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


int lyxfont::lbearing(char c, LyXFont const & f)
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


int lyxfont::rbearing(char c, LyXFont const & f)
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


int lyxfont::width(char const * s, size_t n, LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return n;

	if (lyxrc.font_norm_type == LyXRC::ISO_10646_1) {
		XChar2b * xs = new XChar2b[n];
		Encoding const * encoding = f.language()->encoding();
		//LyXFont const * font = &f;
		LyXFont font(f);
		if (f.family() == LyXFont::SYMBOL_FAMILY) {
#ifdef USE_UNICODE_FOR_SYMBOLS
			//LyXFont font2 = f;
			font.setFamily(LyXFont::ROMAN_FAMILY);
			font.setShape(LyXFont::UP_SHAPE);
			//font = &font2;
#endif
			encoding = encodings.symbol_encoding();
		}
		for (size_t i = 0; i < n; ++i) {
			Uchar c = encoding->ucs(s[i]);
			xs[i].byte1 = c >> 8;
			xs[i].byte2 = c & 0xff;
                }
		int result = width(xs, n, font);
		delete[] xs;
		return result;
	}

	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		return ::XTextWidth(getXFontstruct(f), s, n);
	} else {
		// emulate smallcaps since X doesn't support this
		unsigned int result = 0;
		char c;
		LyXFont smallfont(f);
		smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);
		for (size_t i = 0; i < n; ++i) {
			c = s[i];
			// when islower is a macro, the cast is needed (JMarc)
			if (islower(static_cast<unsigned char>(c))) {
				c = toupper(c);
				result += ::XTextWidth(getXFontstruct(smallfont), &c, 1);
			} else {
				result += ::XTextWidth(getXFontstruct(f), &c, 1);
			}
		}
		return result;
	}
}


int lyxfont::signedWidth(string const & s, LyXFont const & f)
{
	if (s.empty()) return 0;
	if (s[0] == '-')
		return -width(s.substr(1, s.length() - 1), f);
	else
		return width(s, f);
}


//int lyxfont::width(wstring const & s, int n, LyXFont const & f)
int lyxfont::width(XChar2b const * s, int n, LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return n;
	
	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		return ::XTextWidth16(getXFontstruct(f), s, n);
	} else {
		// emulate smallcaps since X doesn't support this
		unsigned int result = 0;
		static XChar2b c = {0, 0};
		LyXFont smallfont(f);
		smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);
		for (int i = 0; i < n; ++i) {
			if (s[i].byte1 == 0 && islower(s[i].byte2)) {
				c.byte2 = toupper(s[i].byte2);
				result += ::XTextWidth16(getXFontstruct(smallfont), &c, 1);
			} else {
				result += ::XTextWidth16(getXFontstruct(f), &s[i], 1);
			}
		}
		return result;
	}
}

int lyxfont::XTextWidth(LyXFont const & f, char const * str, int count)
{
	return ::XTextWidth(getXFontstruct(f), str, count);
}


int lyxfont::XTextWidth16(LyXFont const & f, XChar2b const * str, int count)
{
	return ::XTextWidth16(getXFontstruct(f), str, count);
}


void lyxfont::XSetFont(Display * display, GC gc, LyXFont const & f) 
{
	::XSetFont(display, gc, getFontID(f));
}

//} // end of namespace font
//} // end of namespace lyx
