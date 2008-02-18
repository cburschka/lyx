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

#include "LyXRC.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/Systemcall.h"
#include "support/Package.h"
#include "support/os.h"

#include <QFontInfo>
#include <QFontDatabase>

using namespace std;
using namespace lyx::support;

QString const math_fonts[] = {"cmex10", "cmmi10", "cmr10", "cmsy10",
	"eufm10", "msam10", "msbm10", "wasy10", "esint10"};
int const num_math_fonts = sizeof(math_fonts) / sizeof(*math_fonts);


namespace lyx {

extern docstring const stateText(FontInfo const & f);

namespace frontend {

namespace {

struct SymbolFont {
	FontFamily lyx_family;
	QString family;
	QString xlfd;
};

SymbolFont symbol_fonts[] = {
	{ SYMBOL_FAMILY,
		"symbol",
		"-*-symbol-*-*-*-*-*-*-*-*-*-*-adobe-fontspecific" },

	{ CMR_FAMILY,
		"cmr10",
		"-*-cmr10-medium-*-*-*-*-*-*-*-*-*-*-*" },

	{ CMSY_FAMILY,
		"cmsy10",
		"-*-cmsy10-*-*-*-*-*-*-*-*-*-*-*-*" },

	{ CMM_FAMILY,
		"cmmi10",
		"-*-cmmi10-medium-*-*-*-*-*-*-*-*-*-*-*" },

	{ CMEX_FAMILY,
		"cmex10",
		"-*-cmex10-*-*-*-*-*-*-*-*-*-*-*-*" },

	{ MSA_FAMILY,
		"msam10",
		"-*-msam10-*-*-*-*-*-*-*-*-*-*-*-*" },

	{ MSB_FAMILY,
		"msbm10",
		"-*-msbm10-*-*-*-*-*-*-*-*-*-*-*-*" },

	{ EUFRAK_FAMILY,
		"eufm10",
		"-*-eufm10-medium-*-*-*-*-*-*-*-*-*-*-*" },

	{ WASY_FAMILY,
		"wasy10",
		"-*-wasy10-medium-*-*-*-*-*-*-*-*-*-*-*" },

	{ ESINT_FAMILY,
		"esint10",
		"-*-esint10-medium-*-*-*-*-*-*-*-*-*-*-*" }
};

size_t const nr_symbol_fonts = sizeof(symbol_fonts) / sizeof(SymbolFont);


QString getRawName(QString const & family)
{
	for (size_t i = 0; i < nr_symbol_fonts; ++i)
		if (family == symbol_fonts[i].family)
			return symbol_fonts[i].xlfd;

	LYXERR(Debug::FONT, "BUG: family not found !");
	return QString();
}


QString const symbolFamily(FontFamily family)
{
	for (size_t i = 0; i < nr_symbol_fonts; ++i) {
		if (family == symbol_fonts[i].lyx_family)
			return symbol_fonts[i].family;
	}
	return QString();
}


bool isSymbolFamily(FontFamily family)
{
	return family >= SYMBOL_FAMILY && family <= ESINT_FAMILY;
}


static bool isChosenFont(QFont & font, QString const & family)
{
	// QFontInfo won't find a font that has only a few glyphs at unusual
	// positions, e.g. the original esint10 font.
	// The workaround is to add dummy glyphs at least at all ASCII
	// positions.
	QFontInfo fi(font);

	LYXERR(Debug::FONT, "got: " << fromqstr(fi.family()));

	if (fi.family().contains(family)) {
		LYXERR(Debug::FONT, " got it ");
		return true;
	}

	return false;
}


QFont symbolFont(QString const & family, bool * ok)
{
	LYXERR(Debug::FONT, "Looking for font family "
		<< fromqstr(family) << " ... ");
	QString upper = family;
	upper[0] = family[0].toUpper();

	QFont font;
	font.setKerning(false);
	font.setFamily(family);

	if (isChosenFont(font, family)) {
		LYXERR(Debug::FONT, "normal!");
		*ok = true;
		return font;
	}

	LYXERR(Debug::FONT, "Trying " << fromqstr(upper) << " ... ");
	font.setFamily(upper);

	if (isChosenFont(font, upper)) {
		LYXERR(Debug::FONT, "upper!");
		*ok = true;
		return font;
	}

	// A simple setFamily() fails on Qt 2

	QString const rawName = getRawName(family);
	LYXERR(Debug::FONT, "Trying " << fromqstr(rawName) << " ... ");
	font.setRawName(rawName);

	if (isChosenFont(font, family)) {
		LYXERR(Debug::FONT, "raw version!");
		*ok = true;
		return font;
	}

	LYXERR(Debug::FONT, " FAILED :-(");
	*ok = false;
	return font;
}

} // namespace anon


GuiFontLoader::GuiFontLoader()
{
	QString const fonts_dir =
		toqstr(addPath(package().system_support().absFilename(), "fonts"));

	for (int i = 0 ; i < num_math_fonts; ++i) {
		QString const font_file = fonts_dir + '/' + math_fonts[i] + ".ttf";
		int fontID = QFontDatabase::addApplicationFont(font_file);

		LYXERR(Debug::FONT, "Adding font " << fromqstr(font_file)
				    << static_cast<const char *>
					(fontID < 0 ? " FAIL" : " OK"));
	}

	for (int i1 = 0; i1 < NUM_FAMILIES; ++i1)
		for (int i2 = 0; i2 < 2; ++i2)
			for (int i3 = 0; i3 < 4; ++i3)
				for (int i4 = 0; i4 < 10; ++i4)
					fontinfo_[i1][i2][i3][i4] = 0;
}


void GuiFontLoader::update()
{
	for (int i1 = 0; i1 < NUM_FAMILIES; ++i1) {
		for (int i2 = 0; i2 < 2; ++i2)
			for (int i3 = 0; i3 < 4; ++i3)
				for (int i4 = 0; i4 < 10; ++i4) {
					delete fontinfo_[i1][i2][i3][i4];
					fontinfo_[i1][i2][i3][i4] = 0;
				}
	}
}


GuiFontLoader::~GuiFontLoader()
{
	update();
}

/////////////////////////////////////////////////


static QString makeFontName(QString const & family, QString const & foundry)
{
	QString res = family;
	if (!foundry.isEmpty())
		res += " [" + foundry + ']';
	return res;
}


GuiFontInfo::GuiFontInfo(FontInfo const & f)
	: metrics(QFont())
{
	font.setKerning(false);
	QString const pat = symbolFamily(f.family());
	if (!pat.isEmpty()) {
		bool ok;
		font = symbolFont(pat, &ok);
	} else {
		switch (f.family()) {
		case ROMAN_FAMILY: {
			QString family = makeFontName(toqstr(lyxrc.roman_font_name),
																		toqstr(lyxrc.roman_font_foundry)); 
			font.setFamily(family);
#ifdef Q_WS_MACX
#if QT_VERSION >= 0x040300
			// Workaround for a Qt bug, see http://bugzilla.lyx.org/show_bug.cgi?id=3684
			// It is reported to Trolltech at 02/06/07 against 4.3 final.
			// FIXME: Add an upper version limit as soon as the bug is fixed in Qt.
			if (family == "Times" && !font.exactMatch())
				font.setFamily("Times New Roman");
#endif
#endif
			break;
		}
		case SANS_FAMILY:
			font.setFamily(makeFontName(toqstr(lyxrc.sans_font_name),
						    toqstr(lyxrc.sans_font_foundry)));
			break;
		case TYPEWRITER_FAMILY:
			font.setFamily(makeFontName(toqstr(lyxrc.typewriter_font_name),
						    toqstr(lyxrc.typewriter_font_foundry)));
			break;
		default:
			break;
		}
	}

	switch (f.series()) {
		case MEDIUM_SERIES:
			font.setWeight(QFont::Normal);
			break;
		case BOLD_SERIES:
			font.setWeight(QFont::Bold);
			break;
		default:
			break;
	}

	switch (f.realShape()) {
		case ITALIC_SHAPE:
		case SLANTED_SHAPE:
			font.setItalic(true);
			break;
		default:
			break;
	}

	LYXERR(Debug::FONT, "Font '" << to_utf8(stateText(f))
		<< "' matched by\n" << fromqstr(font.family()));

	// Is this an exact match?
	if (font.exactMatch())
		LYXERR(Debug::FONT, "This font is an exact match");
	else
		LYXERR(Debug::FONT, "This font is NOT an exact match");

	LYXERR(Debug::FONT, "XFLD: " << fromqstr(font.rawName()));

	font.setPointSizeF(convert<double>(lyxrc.font_sizes[f.size()])
			       * lyxrc.zoom / 100.0);

	LYXERR(Debug::FONT, "The font has size: " << font.pointSizeF());

	if (f.realShape() != SMALLCAPS_SHAPE) {
		metrics = GuiFontMetrics(font);
	} else {
		// handle small caps ourselves ...
		FontInfo smallfont = f;
		smallfont.decSize().decSize().setShape(UP_SHAPE);
		QFont font2(font);
		font2.setKerning(false);
		font2.setPointSizeF(convert<double>(lyxrc.font_sizes[smallfont.size()])
			       * lyxrc.zoom / 100.0);

		metrics = GuiFontMetrics(font, font2);
	}
}


bool GuiFontLoader::available(FontInfo const & f)
{
	static vector<int> cache_set(NUM_FAMILIES, false);
	static vector<int> cache(NUM_FAMILIES, false);

	FontFamily family = f.family();
	if (cache_set[family])
		return cache[family];
	cache_set[family] = true;

	QString const pat = symbolFamily(family);
	if (pat.isEmpty())
		// We don't care about non-symbol fonts
		return false;

	bool ok;
	symbolFont(pat, &ok);
	if (!ok)
		return false;

	cache[family] = true;
	return true;
}

} // namespace frontend
} // namespace lyx
