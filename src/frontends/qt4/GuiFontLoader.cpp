/**
 * \file GuiFontLoader.cpp
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
#include "LyXRC.h"

#include "support/convert.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/Systemcall.h"
#include "support/Package.h"
#include "support/os.h"

#include <boost/tuple/tuple.hpp>

#include <QFontInfo>
#include <QFontDatabase>

using lyx::support::contains;
using lyx::support::package;
using lyx::support::addPath;
using lyx::support::addName;

using std::endl;
using std::make_pair;

using std::pair;
using std::vector;
using std::string;

#if QT_VERSION >= 0x040200
string const math_fonts[] = {"cmex10", "cmmi10", "cmr10", "cmsy10",
	"eufm10", "msam10", "msbm10", "wasy10", "esint10"};
int const num_math_fonts = sizeof(math_fonts) / sizeof(*math_fonts);
#endif


namespace lyx {
namespace frontend {

namespace {

struct symbol_font {
	Font::FONT_FAMILY lyx_family;
	string family;
	string xlfd;
};

symbol_font symbol_fonts[] = {
	{ Font::SYMBOL_FAMILY,
		"symbol",
		"-*-symbol-*-*-*-*-*-*-*-*-*-*-adobe-fontspecific" },

	{ Font::CMR_FAMILY,
		"cmr10",
		"-*-cmr10-medium-*-*-*-*-*-*-*-*-*-*-*" },

	{ Font::CMSY_FAMILY,
		"cmsy10",
		"-*-cmsy10-*-*-*-*-*-*-*-*-*-*-*-*" },

	{ Font::CMM_FAMILY,
		"cmmi10",
		"-*-cmmi10-medium-*-*-*-*-*-*-*-*-*-*-*" },

	{ Font::CMEX_FAMILY,
		"cmex10",
		"-*-cmex10-*-*-*-*-*-*-*-*-*-*-*-*" },

	{ Font::MSA_FAMILY,
		"msam10",
		"-*-msam10-*-*-*-*-*-*-*-*-*-*-*-*" },

	{ Font::MSB_FAMILY,
		"msbm10",
		"-*-msbm10-*-*-*-*-*-*-*-*-*-*-*-*" },

	{ Font::EUFRAK_FAMILY,
		"eufm10",
		"-*-eufm10-medium-*-*-*-*-*-*-*-*-*-*-*" },

	{ Font::WASY_FAMILY,
		"wasy10",
		"-*-wasy10-medium-*-*-*-*-*-*-*-*-*-*-*" },

	{ Font::ESINT_FAMILY,
		"esint10",
		"-*-esint10-medium-*-*-*-*-*-*-*-*-*-*-*" }
};

size_t const nr_symbol_fonts = sizeof(symbol_fonts) / sizeof(symbol_font);


string getRawName(string const & family)
{
	for (size_t i = 0; i < nr_symbol_fonts; ++i)
		if (family == symbol_fonts[i].family)
			return symbol_fonts[i].xlfd;

	LYXERR(Debug::FONT) << "BUG: family not found !" << endl;
	return string();
}


string const symbolFamily(Font::FONT_FAMILY family)
{
	for (size_t i = 0; i < nr_symbol_fonts; ++i) {
		if (family == symbol_fonts[i].lyx_family)
			return symbol_fonts[i].family;
	}
	return string();
}


bool isSymbolFamily(Font::FONT_FAMILY family)
{
	return family >= Font::SYMBOL_FAMILY &&
	       family <= Font::ESINT_FAMILY;
}


bool isChosenFont(QFont & font, string const & family)
{
	// QFontInfo won't find a font that has only a few glyphs at unusual
	// positions, e.g. the original esint10 font.
	// The workaround is to add dummy glyphs at least at all ASCII
	// positions.
	QFontInfo fi(font);

	LYXERR(Debug::FONT) << "got: " << fromqstr(fi.family()) << endl;

	if (contains(fromqstr(fi.family()), family)) {
		LYXERR(Debug::FONT) << " got it ";
		return true;
	}

	return false;
}


pair<QFont, bool> const getSymbolFont(string const & family)
{
	LYXERR(Debug::FONT) << "Looking for font family "
		<< family << " ... ";
	string upper = family;
	upper[0] = toupper(family[0]);

	QFont font;
	font.setKerning(false);
	font.setFamily(toqstr(family));

	if (isChosenFont(font, family)) {
		LYXERR(Debug::FONT) << "normal!" << endl;
		return make_pair<QFont, bool>(font, true);
	}

	LYXERR(Debug::FONT) << "Trying " << upper << " ... ";
	font.setFamily(toqstr(upper));

	if (isChosenFont(font, upper)) {
		LYXERR(Debug::FONT) << "upper!" << endl;
		return make_pair<QFont, bool>(font, true);
	}

	// A simple setFamily() fails on Qt 2

	string const rawName = getRawName(family);
	LYXERR(Debug::FONT) << "Trying " << rawName << " ... ";
	font.setRawName(toqstr(rawName));

	if (isChosenFont(font, family)) {
		LYXERR(Debug::FONT) << "raw version!" << endl;
		return make_pair<QFont, bool>(font, true);
	}

	LYXERR(Debug::FONT) << " FAILED :-(" << endl;
	return make_pair<QFont, bool>(font, false);
}

} // namespace anon


GuiFontLoader::GuiFontLoader()
{
#if QT_VERSION >= 0x040200
	string const fonts_dir =
		addPath(package().system_support().absFilename(), "fonts");

	for (int i = 0 ; i < num_math_fonts; ++i) {
		string const font_file = lyx::support::os::external_path(
				addName(fonts_dir, math_fonts[i] + ".ttf"));
		int fontID = QFontDatabase::addApplicationFont(toqstr(font_file));

		LYXERR(Debug::FONT) << "Adding font " << font_file
				    << static_cast<const char *>
					(fontID < 0 ? " FAIL" : " OK")
				    << endl;
	}
#endif

	for (int i1 = 0; i1 < Font::NUM_FAMILIES; ++i1)
		for (int i2 = 0; i2 < 2; ++i2)
			for (int i3 = 0; i3 < 4; ++i3)
				for (int i4 = 0; i4 < 10; ++i4)
					fontinfo_[i1][i2][i3][i4] = 0;
}


void GuiFontLoader::update()
{
	for (int i1 = 0; i1 < Font::NUM_FAMILIES; ++i1) {
		for (int i2 = 0; i2 < 2; ++i2)
			for (int i3 = 0; i3 < 4; ++i3)
				for (int i4 = 0; i4 < 10; ++i4) {
					delete fontinfo_[i1][i2][i3][i4];
					fontinfo_[i1][i2][i3][i4] = 0;
				}
	}
}


/////////////////////////////////////////////////


QLFontInfo::QLFontInfo(Font const & f)
{
	font.setKerning(false);
	string const pat = symbolFamily(f.family());
	if (!pat.empty()) {
		bool tmp;
		boost::tie(font, tmp) = getSymbolFont(pat);
	} else {
		switch (f.family()) {
		case Font::ROMAN_FAMILY: {
			QString family = toqstr(makeFontName(lyxrc.roman_font_name,
																					 lyxrc.roman_font_foundry)); 
			font.setFamily(family);
#ifdef Q_WS_MACX
#if QT_VERSION >= 0x040300
			// Workaround for a Qt bug, see http://bugzilla.lyx.org/show_bug.cgi?id=3684
			// It is reported to Trolltech at 02/06/07 against 4.3 final.
			// FIXME: Add an upper version limit as soon as the bug is fixed in Qt.
			if (family == "Times" && !font.exactMatch())
				font.setFamily(QString::fromLatin1("Times New Roman"));
#endif
#endif
			break;
		}
		case Font::SANS_FAMILY:
			font.setFamily(toqstr(makeFontName(lyxrc.sans_font_name,
						    lyxrc.sans_font_foundry)));
			break;
		case Font::TYPEWRITER_FAMILY:
			font.setFamily(toqstr(makeFontName(lyxrc.typewriter_font_name,
						    lyxrc.typewriter_font_foundry)));
			break;
		default:
			break;
		}
	}

	switch (f.series()) {
		case Font::MEDIUM_SERIES:
			font.setWeight(QFont::Normal);
			break;
		case Font::BOLD_SERIES:
			font.setWeight(QFont::Bold);
			break;
		default:
			break;
	}

	switch (f.realShape()) {
		case Font::ITALIC_SHAPE:
		case Font::SLANTED_SHAPE:
			font.setItalic(true);
			break;
		default:
			break;
	}

	LYXERR(Debug::FONT) << "Font '" << to_utf8(f.stateText(0))
		<< "' matched by\n" << fromqstr(font.family()) << endl;

	// Is this an exact match?
	if (font.exactMatch())
		LYXERR(Debug::FONT) << "This font is an exact match" << endl;
	else
		LYXERR(Debug::FONT) << "This font is NOT an exact match"
				    << endl;

	LYXERR(Debug::FONT) << "XFLD: " << fromqstr(font.rawName()) << endl;

	font.setPointSizeF(convert<double>(lyxrc.font_sizes[f.size()])
			       * lyxrc.zoom / 100.0);

	LYXERR(Debug::FONT) << "The font has size: "
			    << font.pointSizeF() << endl;

	if (f.realShape() != Font::SMALLCAPS_SHAPE) {
		metrics.reset(new GuiFontMetrics(font));
	}
	else {
		// handle small caps ourselves ...
		Font smallfont = f;
		smallfont.decSize().decSize().setShape(Font::UP_SHAPE);
		QFont font2(font);
		font2.setKerning(false);
		font2.setPointSizeF(convert<double>(lyxrc.font_sizes[smallfont.size()])
			       * lyxrc.zoom / 100.0);

		metrics.reset(new GuiFontMetrics(font, font2));
	}

}


bool GuiFontLoader::available(Font const & f)
{
	static vector<int> cache_set(Font::NUM_FAMILIES, false);
	static vector<int> cache(Font::NUM_FAMILIES, false);

	Font::FONT_FAMILY family = f.family();
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
