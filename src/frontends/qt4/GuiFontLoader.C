/**
 * \file GuiFontLoader.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiFontLoader.h"
#include "qt_helpers.h"

#include "debug.h"
#include "lyxrc.h"

#include "support/convert.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/systemcall.h"

#include <qfontinfo.h>

#include <boost/tuple/tuple.hpp>

#ifdef Q_WS_X11
#include <qwidget.h>
#include <X11/Xlib.h>
#include <algorithm>
#endif

using lyx::support::contains;

using std::endl;
using std::make_pair;

using std::pair;
using std::vector;
using std::string;


namespace lyx {
namespace frontend {

GuiFontLoader::~GuiFontLoader() {
}

namespace {

struct symbol_font {
	LyXFont::FONT_FAMILY lyx_family;
	string family;
	string xlfd;
};

symbol_font symbol_fonts[] = {
	{ LyXFont::SYMBOL_FAMILY,
		"symbol",
		"-*-symbol-*-*-*-*-*-*-*-*-*-*-adobe-fontspecific" },

	{ LyXFont::CMR_FAMILY,
		"cmr10",
		"-*-cmr10-medium-*-*-*-*-*-*-*-*-*-*-*" },

	{ LyXFont::CMSY_FAMILY,
		"cmsy10",
		"-*-cmsy10-*-*-*-*-*-*-*-*-*-*-*-*" },

	{ LyXFont::CMM_FAMILY,
		"cmmi10",
		"-*-cmmi10-medium-*-*-*-*-*-*-*-*-*-*-*" },

	{ LyXFont::CMEX_FAMILY,
		"cmex10",
		"-*-cmex10-*-*-*-*-*-*-*-*-*-*-*-*" },

	{ LyXFont::MSA_FAMILY,
		"msam10",
		"-*-msam10-*-*-*-*-*-*-*-*-*-*-*-*" },

	{ LyXFont::MSB_FAMILY,
		"msbm10",
		"-*-msbm10-*-*-*-*-*-*-*-*-*-*-*-*" },

	{ LyXFont::EUFRAK_FAMILY,
		"eufm10",
		"-*-eufm10-medium-*-*-*-*-*-*-*-*-*-*-*" },

	{ LyXFont::WASY_FAMILY,
		"wasy10",
		"-*-wasy10-medium-*-*-*-*-*-*-*-*-*-*-*" }
};

size_t const nr_symbol_fonts = sizeof(symbol_fonts) / sizeof(symbol_font);


string getRawName(string const & family)
{
	for (size_t i = 0; i < nr_symbol_fonts; ++i)
		if (family == symbol_fonts[i].family)
			return symbol_fonts[i].xlfd;

	lyxerr[Debug::FONT] << "BUG: family not found !" << endl;
	return string();
}


string const symbolFamily(LyXFont::FONT_FAMILY family)
{
	for (size_t i = 0; i < nr_symbol_fonts; ++i) {
		if (family == symbol_fonts[i].lyx_family)
			return symbol_fonts[i].family;
	}
	return string();
}


bool isSymbolFamily(LyXFont::FONT_FAMILY family)
{
	return family >= LyXFont::SYMBOL_FAMILY &&
		family <= LyXFont::WASY_FAMILY;
}


bool isChosenFont(QFont & font, string const & family)
{
	lyxerr[Debug::FONT] << "raw: " << fromqstr(font.rawName()) << endl;

	QFontInfo fi(font);

	// Note Qt lies about family quite often
	lyxerr[Debug::FONT] << "alleged fi family: "
		<< fromqstr(fi.family()) << endl;

	// So we check rawName first
	if (contains(fromqstr(font.rawName()), family)) {
		lyxerr[Debug::FONT] << " got it ";
		return true;
	}

	// Qt 3.2 beta1 returns "xft" for all xft fonts
	// Qt 4.1 returns "Multi" for all ? xft fonts
	if (font.rawName() == "xft" || font.rawName() == "Multi") {
		if (contains(fromqstr(fi.family()), family)) {
			lyxerr[Debug::FONT] << " got it (Xft) ";
			return true;
		}
	}

	return false;
}


pair<QFont, bool> const getSymbolFont(string const & family)
{
	lyxerr[Debug::FONT] << "Looking for font family "
		<< family << " ... ";
	string upper = family;
	upper[0] = toupper(family[0]);

	QFont font;
	font.setKerning(false);
	font.setFamily(toqstr(family));

	if (isChosenFont(font, family)) {
		lyxerr[Debug::FONT] << "normal!" << endl;
		return make_pair<QFont, bool>(font, true);
	}

	font.setFamily(toqstr(upper));

	if (isChosenFont(font, upper)) {
		lyxerr[Debug::FONT] << "upper!" << endl;
		return make_pair<QFont, bool>(font, true);
	}

	// A simple setFamily() fails on Qt 2

	font.setRawName(toqstr(getRawName(family)));

	if (isChosenFont(font, family)) {
		lyxerr[Debug::FONT] << "raw version!" << endl;
		return make_pair<QFont, bool>(font, true);
	}

	lyxerr[Debug::FONT] << " FAILED :-(" << endl;
	return make_pair<QFont, bool>(font, false);
}

} // namespace anon


GuiFontLoader::GuiFontLoader()
{
	for (int i1 = 0; i1 < LyXFont::NUM_FAMILIES; ++i1)
		for (int i2 = 0; i2 < 2; ++i2)
			for (int i3 = 0; i3 < 4; ++i3)
				for (int i4 = 0; i4 < 10; ++i4)
					fontinfo_[i1][i2][i3][i4] = 0;
}


void GuiFontLoader::update()
{
	for (int i1 = 0; i1 < LyXFont::NUM_FAMILIES; ++i1)
		for (int i2 = 0; i2 < 2; ++i2)
			for (int i3 = 0; i3 < 4; ++i3)
				for (int i4 = 0; i4 < 10; ++i4) {
					delete fontinfo_[i1][i2][i3][i4];
					fontinfo_[i1][i2][i3][i4] = 0;
				}
}


/////////////////////////////////////////////////


QLFontInfo::QLFontInfo(LyXFont const & f)
{
	font.setKerning(false);
	string const pat = symbolFamily(f.family());
	if (!pat.empty()) {
		bool tmp;
		boost::tie(font, tmp) = getSymbolFont(pat);
	} else {
		switch (f.family()) {
		case LyXFont::ROMAN_FAMILY:
			font.setFamily(toqstr(makeFontName(lyxrc.roman_font_name,
						    lyxrc.roman_font_foundry)));
			break;
		case LyXFont::SANS_FAMILY:
			font.setFamily(toqstr(makeFontName(lyxrc.sans_font_name,
						    lyxrc.sans_font_foundry)));
			break;
		case LyXFont::TYPEWRITER_FAMILY:
			font.setFamily(toqstr(makeFontName(lyxrc.typewriter_font_name,
						    lyxrc.typewriter_font_foundry)));
			break;
		default:
			break;
		}
	}

	switch (f.series()) {
		case LyXFont::MEDIUM_SERIES:
			font.setWeight(QFont::Normal);
			break;
		case LyXFont::BOLD_SERIES:
			font.setWeight(QFont::Bold);
			break;
		default:
			break;
	}

	switch (f.realShape()) {
		case LyXFont::ITALIC_SHAPE:
		case LyXFont::SLANTED_SHAPE:
			font.setItalic(true);
			break;
		default:
			break;
	}

	if (lyxerr.debugging(Debug::FONT)) {
		lyxerr[Debug::FONT] << "Font '" << f.stateText(0)
			<< "' matched by\n" << fromqstr(font.rawName()) << endl;
	}

	// Is this an exact match?
	if (font.exactMatch())
		lyxerr[Debug::FONT] << "This font is an exact match" << endl;
	else
		lyxerr[Debug::FONT] << "This font is NOT an exact match"
				    << endl;

	lyxerr[Debug::FONT] << "XFLD: " << fromqstr(font.rawName()) << endl;

	font.setPointSizeF(convert<double>(lyxrc.font_sizes[f.size()])
			       * lyxrc.zoom / 100.0);

	lyxerr[Debug::FONT] << "The font has size: "
			    << font.pointSizeF() << endl;

	if (f.realShape() != LyXFont::SMALLCAPS_SHAPE) {
		metrics.reset(new GuiFontMetrics(font));
	}
	else {	
		// handle small caps ourselves ...
		LyXFont smallfont = f;
		smallfont.decSize().decSize().setShape(LyXFont::UP_SHAPE);
		QFont font2(font);
		font2.setKerning(false);
		font2.setPointSizeF(convert<double>(lyxrc.font_sizes[smallfont.size()])
			       * lyxrc.zoom / 100.0);

		metrics.reset(new GuiFontMetrics(font, font2));
	}

}


bool GuiFontLoader::available(LyXFont const & f)
{
	static vector<int> cache_set(LyXFont::NUM_FAMILIES, false);
	static vector<int> cache(LyXFont::NUM_FAMILIES, false);

	LyXFont::FONT_FAMILY family = f.family();
	if (cache_set[family])
		return cache[family];
	cache_set[family] = true;

	string const pat = symbolFamily(family);
	if (pat.empty())
		// We don't care about non-symbol fonts
		return false;

	pair<QFont, bool> tmp = getSymbolFont(pat);
	if (!tmp.second)
		return false;

	cache[family] = true;
	return true;
}

} // namespace frontend
} // namespace lyx
