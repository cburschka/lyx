/**
 * \file FontLoader.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "qfont_loader.h"
#include "qt_helpers.h"

#include "debug.h"
#include "lyxrc.h"

#include "frontends/lyx_gui.h"

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
using lyx::support::LibFileSearch;
using lyx::support::OnlyPath;
using lyx::support::QuoteName;
using lyx::support::Systemcall;

using std::endl;
using std::make_pair;

using std::pair;
using std::vector;
using std::string;

#ifdef Q_WS_MACX
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef Q_WS_WIN
// Require Windows API > Win98 (only needed for AddFontResourceEx)
#define _WIN32_WINNT 0x0500
#include <windows.h>
#include "support/os.h"
#include "support/package.h"
#include "support/path.h"
using lyx::support::AddName;
using lyx::support::AddPath;
using lyx::support::package;
namespace os = lyx::support::os;
string const win_fonts_truetype[] = {"cmex10", "cmmi10", "cmr10", "cmsy10",
	"eufm10", "msam10", "msbm10", "wasy10"};
const int num_fonts_truetype = sizeof(win_fonts_truetype) / sizeof(*win_fonts_truetype);
#endif

void FontLoader::initFontPath()
{
#ifdef Q_WS_MACX
	CFBundleRef  myAppBundle = CFBundleGetMainBundle();
	CFURLRef  myAppResourcesURL, FontsURL;
	FSRef  fontDirRef;
	FSSpec  fontDirSpec;
	CFStringRef  filePath = CFStringCreateWithBytes(kCFAllocatorDefault,
					(UInt8 *) "Fonts", strlen("Fonts"),
					kCFStringEncodingISOLatin1, false);

	myAppResourcesURL = CFBundleCopyResourcesDirectoryURL(myAppBundle);
	FontsURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault,
			myAppResourcesURL, filePath, true);
	if (lyxerr.debugging(Debug::FONT)) {
		UInt8  buf[255];
		if (CFURLGetFileSystemRepresentation(FontsURL, true, buf, 255))
			lyxerr << "Adding Fonts directory: " << buf << endl;
	}
	CFURLGetFSRef (FontsURL, &fontDirRef);
	OSStatus err = FSGetCatalogInfo (&fontDirRef, kFSCatInfoNone,
					 NULL, NULL, &fontDirSpec, NULL);
	if (err)
		lyxerr << "FSGetCatalogInfo err = " << err << endl;
	err = FMActivateFonts (&fontDirSpec, NULL, NULL,
			       kFMLocalActivationContext);
	if (err)
		lyxerr << "FMActivateFonts err = " << err << endl;
#endif

#ifdef Q_WS_WIN
	// Windows only: Add BaKoMa TrueType font resources
	string const fonts_dir = AddPath(package().system_support(), "fonts");
	
	for (int i = 0 ; i < num_fonts_truetype ; ++i) {
		string const font_current = os::external_path(
			AddName(fonts_dir, win_fonts_truetype[i] + ".ttf"));
		AddFontResourceEx(font_current.c_str(), FR_PRIVATE, 0);
	}
#endif
}

FontLoader::~FontLoader() {
#ifdef Q_WS_WIN
	// Windows only: Remove BaKoMa TrueType font resources
	string const fonts_dir = AddPath(package().system_support(), "fonts");
	
	for(int i = 0 ; i < num_fonts_truetype ; ++i) {
		string const font_current = os::external_path(
			AddName(fonts_dir, win_fonts_truetype[i] + ".ttf"));
		RemoveFontResourceEx(font_current.c_str(), FR_PRIVATE, 0);
	}
#endif
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
	if (font.rawName() == "xft") {
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


FontLoader::FontLoader()
{
	for (int i1 = 0; i1 < LyXFont::NUM_FAMILIES; ++i1)
		for (int i2 = 0; i2 < 2; ++i2)
			for (int i3 = 0; i3 < 4; ++i3)
				for (int i4 = 0; i4 < 10; ++i4)
					fontinfo_[i1][i2][i3][i4] = 0;
}


void FontLoader::update()
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
	: metrics(font)
{

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

	font.setPointSizeFloat(convert<double>(lyxrc.font_sizes[f.size()])
			       * lyxrc.zoom / 100.0);

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
			<< "' matched by\n" << font.rawName() << endl;
	}

	lyxerr[Debug::FONT] << "The font has size: "
			    << font.pointSizeFloat() << endl;

	// Is this an exact match?
	if (font.exactMatch())
		lyxerr[Debug::FONT] << "This font is an exact match" << endl;
	else
		lyxerr[Debug::FONT] << "This font is NOT an exact match"
				    << endl;

	lyxerr[Debug::FONT] << "XFLD: " << font.rawName() << endl;

	metrics = QFontMetrics(font);
}


int QLFontInfo::width(Uchar val)
{
// Starting with version 3.1.0, Qt/X11 does its own caching of
// character width, so it is not necessary to provide ours.
#if defined (USE_LYX_FONTCACHE)
	QLFontInfo::MetricsCache::const_iterator cit = widthcache.find(val);
	if (cit != widthcache.end())
		return cit->second;

	int const w = metrics.width(QChar(val));
	widthcache[val] = w;
	return w;
#else
	return metrics.width(QChar(val));
#endif
}


int QLFontInfo::ascent(char c)
{
#if defined(USE_LYX_FONTCACHE)
	Uchar const val = static_cast<Uchar>(c);
	QLFontInfo::MetricsCache::const_iterator cit = ascentcache.find(val);
	if (cit != ascentcache.end())
		return cit->second;

	QRect const & r = metrics.boundingRect(c);
	int const w = -r.top();
	ascentcache[val] = w;
	return w;
#else
	QRect const & r = metrics.boundingRect(c);
	return -r.top();
#endif
}


int QLFontInfo::descent(char c)
{
#if defined(USE_LYX_FONTCACHE)
	Uchar const val = static_cast<Uchar>(c);
	QLFontInfo::MetricsCache::const_iterator cit = descentcache.find(val);
	if (cit != descentcache.end())
		return cit->second;

	QRect const & r = metrics.boundingRect(c);
	int const w = r.bottom() + 1;
	descentcache[val] = w;
	return w;
#else
	QRect const & r = metrics.boundingRect(c);
	return r.bottom() + 1;
#endif
}


bool FontLoader::available(LyXFont const & f)
{
	if (!lyx_gui::use_gui)
		return false;

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
