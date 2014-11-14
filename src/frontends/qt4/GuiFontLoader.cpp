/**
 * \file FontLoader.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FontLoader.h"

#include "FontInfo.h"
#include "GuiFontMetrics.h"
#include "qt_helpers.h"

#include "LyXRC.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/Systemcall.h"
#include "support/Package.h"
#include "support/os.h"

#include <QFontInfo>
#include <QFontDatabase>

#include "support/lassert.h"

using namespace std;
using namespace lyx::support;

QString const math_fonts[] = {"cmex10", "cmmi10", "cmr10", "cmsy10",
	"esint10", "eufm10", "msam10", "msbm10", "rsfs10", "stmary10",
	"wasy10"};
int const num_math_fonts = sizeof(math_fonts) / sizeof(*math_fonts);

namespace lyx {

extern docstring const stateText(FontInfo const & f);

namespace frontend {

/**
 * Matches Fonts against
 * actual QFont instances, and also caches metrics.
 */
class GuiFontInfo
{
public:
	GuiFontInfo(FontInfo const & f);

	/// The font instance
	QFont font;
	/// Metrics on the font
	GuiFontMetrics metrics;
};

namespace {

struct SymbolFont {
	FontFamily lyx_family;
	QString family;
	QString xlfd;
};

SymbolFont symbol_fonts[] = {
	{ SYMBOL_FAMILY,"symbol", "-*-symbol-*-*-*-*-*-*-*-*-*-*-adobe-fontspecific"},
	{ CMR_FAMILY,   "cmr10",  "-*-cmr10-medium-*-*-*-*-*-*-*-*-*-*-*" },
	{ CMSY_FAMILY,  "cmsy10", "-*-cmsy10-*-*-*-*-*-*-*-*-*-*-*-*" },
	{ CMM_FAMILY,   "cmmi10", "-*-cmmi10-medium-*-*-*-*-*-*-*-*-*-*-*" },
	{ CMEX_FAMILY,  "cmex10", "-*-cmex10-*-*-*-*-*-*-*-*-*-*-*-*" },
	{ MSA_FAMILY,   "msam10", "-*-msam10-*-*-*-*-*-*-*-*-*-*-*-*" },
	{ MSB_FAMILY,   "msbm10", "-*-msbm10-*-*-*-*-*-*-*-*-*-*-*-*" },
	{ EUFRAK_FAMILY,"eufm10", "-*-eufm10-medium-*-*-*-*-*-*-*-*-*-*-*" },
	{ RSFS_FAMILY,  "rsfs10", "-*-rsfs10-medium-*-*-*-*-*-*-*-*-*-*-*" },
	{ STMARY_FAMILY,"stmary10","-*-stmary10-medium-*-*-*-*-*-*-*-*-*-*-*" },
	{ WASY_FAMILY,  "wasy10", "-*-wasy10-medium-*-*-*-*-*-*-*-*-*-*-*" },
	{ ESINT_FAMILY, "esint10","-*-esint10-medium-*-*-*-*-*-*-*-*-*-*-*" }
};

size_t const nr_symbol_fonts = sizeof(symbol_fonts) / sizeof(symbol_fonts[0]);

/// BUTT ugly !
static GuiFontInfo * fontinfo_[NUM_FAMILIES][NUM_SERIES][NUM_SHAPE][NUM_SIZE];


// Get font info (font + metrics) for the given LyX font.
// if not cached, create it.
GuiFontInfo & fontinfo(FontInfo const & f)
{
	// LASSERT: Is there anything we might do here besides crash?
	LBUFERR(f.family() < NUM_FAMILIES);
	LBUFERR(f.series() < NUM_SERIES);
	LBUFERR(f.realShape() < NUM_SHAPE);
	LBUFERR(f.size() < NUM_SIZE);
	// fi is a reference to the pointer type (GuiFontInfo *) in the
	// fontinfo_ table.
	GuiFontInfo * & fi =
		fontinfo_[f.family()][f.series()][f.realShape()][f.size()];
	if (!fi)
		fi = new GuiFontInfo(f);
	return *fi;
}


QString rawName(QString const & family)
{
	for (size_t i = 0; i < nr_symbol_fonts; ++i)
		if (family == symbol_fonts[i].family)
			return symbol_fonts[i].xlfd;

	LYXERR(Debug::FONT, "BUG: family not found !");
	return QString();
}


QString symbolFamily(FontFamily family)
{
	for (size_t i = 0; i < nr_symbol_fonts; ++i) {
		if (family == symbol_fonts[i].lyx_family)
			return symbol_fonts[i].family;
	}
	return QString();
}


#if 0
bool isSymbolFamily(FontFamily family)
{
	return family >= SYMBOL_FAMILY && family <= ESINT_FAMILY;
}
#endif


static bool isChosenFont(QFont & font, QString const & family)
{
	// QFontInfo won't find a font that has only a few glyphs at unusual
	// positions, e.g. the original esint10 font.
	// The workaround is to add dummy glyphs at least at all ASCII
	// positions.
	QFontInfo fi(font);

	LYXERR(Debug::FONT, "got: " << fi.family());

	if (fi.family().contains(family)) {
		LYXERR(Debug::FONT, " got it ");
		return true;
	}

	return false;
}


QFont symbolFont(QString const & family, bool * ok)
{
	LYXERR(Debug::FONT, "Looking for font family " << family << " ... ");
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

	LYXERR(Debug::FONT, "Trying " << upper << " ... ");
	font.setFamily(upper);

	if (isChosenFont(font, upper)) {
		LYXERR(Debug::FONT, "upper!");
		*ok = true;
		return font;
	}

	// A simple setFamily() fails on Qt 2

	QString const raw = rawName(family);
	LYXERR(Debug::FONT, "Trying " << raw << " ... ");
	font.setRawName(raw);

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


FontLoader::FontLoader()
{
	QString const fonts_dir =
		toqstr(addPath(package().system_support().absFileName(), "fonts"));

	for (int i = 0 ; i < num_math_fonts; ++i) {
		QString const font_file = fonts_dir + '/' + math_fonts[i] + ".ttf";
		int fontID = QFontDatabase::addApplicationFont(font_file);

		LYXERR(Debug::FONT, "Adding font " << font_file
				    << (fontID < 0 ? " FAIL" : " OK"));
	}

	for (int i1 = 0; i1 < NUM_FAMILIES; ++i1)
		for (int i2 = 0; i2 < NUM_SERIES; ++i2)
			for (int i3 = 0; i3 < NUM_SHAPE; ++i3)
				for (int i4 = 0; i4 < NUM_SIZE; ++i4)
					fontinfo_[i1][i2][i3][i4] = 0;
}


void FontLoader::update()
{
	for (int i1 = 0; i1 < NUM_FAMILIES; ++i1)
		for (int i2 = 0; i2 < NUM_SERIES; ++i2)
			for (int i3 = 0; i3 < NUM_SHAPE; ++i3)
				for (int i4 = 0; i4 < NUM_SIZE; ++i4) {
					delete fontinfo_[i1][i2][i3][i4];
					fontinfo_[i1][i2][i3][i4] = 0;
				}
}


FontLoader::~FontLoader()
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
#if QT_VERSION >= 0x040300 //&& QT_VERSION < 0x040800
			// Workaround for a Qt bug, see http://www.lyx.org/trac/ticket/3684
			// and http://bugreports.qt.nokia.com/browse/QTBUG-11145.
			// FIXME: Check whether this is really fixed in Qt 4.8
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

	LYXERR(Debug::FONT, "Font '" << stateText(f)
		<< "' matched by\n" << font.family());

	// Is this an exact match?
	if (font.exactMatch())
		LYXERR(Debug::FONT, "This font is an exact match");
	else
		LYXERR(Debug::FONT, "This font is NOT an exact match");

	LYXERR(Debug::FONT, "XFLD: " << font.rawName());

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


bool FontLoader::available(FontInfo const & f)
{
	static vector<int> cache_set(NUM_FAMILIES, false);
	static vector<int> cache(NUM_FAMILIES, false);

	FontFamily family = f.family();
#ifdef Q_WS_MACX
	// Apple ships a font name "Symbol", which has more or less the same
	// glyphs as the original PostScript Symbol font, but it uses a different
	// encoding (see https://en.wikipedia.org/wiki/Symbol_(typeface)#cite_note-2).
	// Since we expect the font specific encoding of the original
	// PostScript Symbol font, we can't use the one provided on OS X.
	// See also the discussion in bug 7954.
	if (f.family() == SYMBOL_FAMILY)
		return false;
#endif
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


bool FontLoader::canBeDisplayed(char_type c)
{
	// bug 8493
	if (c == 0x0009)
		// FIXME check whether this is still needed for Qt5
		return false;
#if QT_VERSION < 0x050000 && defined(QT_MAC_USE_COCOA) && (QT_MAC_USE_COCOA > 0)
	// bug 7954, see also comment in GuiPainter::text()
	if (c == 0x00ad)
		return false;
#endif
	return true;
}


FontMetrics const & FontLoader::metrics(FontInfo const & f)
{
	return fontinfo(f).metrics;
}


GuiFontMetrics const & getFontMetrics(FontInfo const & f)
{
	return fontinfo(f).metrics;
}


QFont const & getFont(FontInfo const & f)
{
	return fontinfo(f).font;
}

} // namespace frontend
} // namespace lyx
