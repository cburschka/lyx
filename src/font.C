#include <config.h>

#include <cctype>

#include "font.h"
#include "FontLoader.h"
#include "lyxrc.h"

extern LyXRC lyxrc;

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
	    && uc <= finfo->max_char_or_byte2) 
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
	    && uc <= finfo->max_char_or_byte2) 
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
	    && uc <= finfo->max_char_or_byte2) 
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
	    && uc <= finfo->max_char_or_byte2) 
		return finfo->per_char[uc - finfo->min_char_or_byte2].rbearing;
	else
		return width(c, f);
}


int lyxfont::width(char c, LyXFont const & f)
{
	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		return lyxrc.use_gui ? ::XTextWidth(getXFontstruct(f), &c, 1)
			: 1;
	} else {
		LyXFont smallfont(f);
		smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);
		if (islower(static_cast<unsigned char>(c))) {
			c = toupper(c);
			return ::XTextWidth(getXFontstruct(smallfont), &c, 1);
		} else {
			return ::XTextWidth(getXFontstruct(f), &c, 1);
		}
	}
}


int lyxfont::width(char const * s, int n, LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return n;
	
	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		return ::XTextWidth(getXFontstruct(f), s, n);
	} else {
		// emulate smallcaps since X doesn't support this
		unsigned int result = 0;
		char c;
		LyXFont smallfont(f);
		smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);
		for (int i = 0; i < n; ++i) {
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
	if (s.c_str()[0] == '-')
		return -width(s.c_str() + 1, s.length() - 1, f);
	else
		return width(s.c_str(), s.length(), f);
}


int lyxfont::XTextWidth(LyXFont const & f, char * str, int count)
{
	return ::XTextWidth(getXFontstruct(f), str, count);
}


void lyxfont::XSetFont(Display * display, GC gc, LyXFont const & f) 
{
	::XSetFont(display, gc, getFontID(f));
}

//} // end of namespace font
//} // end of namespace lyx
