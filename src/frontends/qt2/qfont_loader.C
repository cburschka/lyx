/**
 * \file qfont_loader.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "qfont_loader.h"
#include "qt_helpers.h"
#include "debug.h"
#include "lyxrc.h"
#include "BufferView.h"
#include "qt_helpers.h"

#include <qglobal.h>
#include <qfontmetrics.h>
#include <qfontdatabase.h>
#include <qstringlist.h>
#include "support/lstrings.h"

#ifdef Q_WS_X11
#include <qwidget.h>
#include <X11/Xlib.h>
#include "support/systemcall.h"
#include "support/filetools.h"
#endif

using std::endl;


namespace {

void addFontPath()
{
#ifdef Q_WS_X11
	string const dir =  OnlyPath(LibFileSearch("xfonts", "fonts.dir"));
	if (!dir.empty()) {
		QWidget w;
		int n;
		char ** p = XGetFontPath(w.x11Display(), &n);
		if (std::find(p, p + n, dir) != p + n)
			return;
		lyxerr << "Adding " << dir << " to the font path.\n";
		string const command = "xset fp+ " + dir;
		Systemcall s;
		if (!s.startscript(Systemcall::Wait, command))
			return;
		lyxerr << "Unable to add font path.\n";
	}
#endif
}


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
	for (size_t i = 0; i < nr_symbol_fonts; ++i) {
		if (family == symbol_fonts[i].family)
			return symbol_fonts[i].xlfd;
	}
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


QFont const getSymbolFont(string const & family)
{
	lyxerr[Debug::FONT] << "Looking for font family "
		<< family << " ... ";
	string upper = family;
	upper[0] = toupper(family[0]);

	QFont font;
	font.setFamily(toqstr(family));

	// Note Qt lies about family, so we use rawName.
	if (contains(fromqstr(font.rawName()), family)) {
		lyxerr[Debug::FONT] << " got it !" << endl;
		return font;
	}

	font.setFamily(toqstr(upper));

	if (contains(fromqstr(font.rawName()), upper)) {
		lyxerr[Debug::FONT] << " got it (uppercase version) !" << endl;
		return font;
	}

	// A simple setFamily() fails on Qt 2

	font.setRawName(toqstr(getRawName(family)));

	if (contains(fromqstr(font.rawName()), family)) {
		lyxerr[Debug::FONT] << " got it (raw version) !" << endl;
		return font;
	}

	lyxerr[Debug::FONT] << " FAILED :(" << endl;
	return font;
}


bool isAvailable(LyXFont const & f)
{
	string const tmp = symbolFamily(f.family());

	if (tmp.empty())
		return false;

	QString const family(toqstr(tmp));

	lyxerr[Debug::FONT] << "Family " << tmp
	       << " isAvailable ?" << endl;

	QFontDatabase db;
	// pass false for match-locale: LaTeX fonts
	// do not have non-matching locale according
	// to Qt 2
	QStringList sl(db.families(false));

	for (QStringList::Iterator it = sl.begin(); it != sl.end(); ++it) {

		// Case-insensitive for Cmmi10 vs. cmmi10
		if ((*it).contains(family, false)) {
			lyxerr[Debug::FONT]
				<< "found family "
				<< fromqstr(*it) << endl;
			return true;
		}
	}

	return false;
}


} // namespace anon


qfont_loader::qfont_loader()
{
	for (int i1 = 0; i1 < LyXFont::NUM_FAMILIES; ++i1) {
		for (int i2 = 0; i2 < 2; ++i2) {
			for (int i3 = 0; i3 < 4; ++i3) {
				for (int i4 = 0; i4 < 10; ++i4) {
					fontinfo_[i1][i2][i3][i4] = 0;
				}
			}
		}
	}
}


qfont_loader::~qfont_loader()
{
}


void qfont_loader::update()
{
	for (int i1 = 0; i1 < LyXFont::NUM_FAMILIES; ++i1) {
		for (int i2 = 0; i2 < 2; ++i2) {
			for (int i3 = 0; i3 < 4; ++i3) {
				for (int i4 = 0; i4 < 10; ++i4) {
					delete fontinfo_[i1][i2][i3][i4];
					fontinfo_[i1][i2][i3][i4] = 0;
				}
			}
		}
	}
}


QFont const & qfont_loader::get(LyXFont const & f)
{
	static bool first_call = true;

	if (first_call) {
		first_call = false;
		addFontPath();
	}

	QFont const & ret(getfontinfo(f)->font);

	return ret;
}


qfont_loader::font_info::font_info(LyXFont const & f)
	: metrics(font)
{

	string const pat = symbolFamily(f.family());
	if (!pat.empty()) {
		font = getSymbolFont(pat);
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

	font.setPointSizeFloat(lyxrc.font_sizes[f.size()]
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
	if (font.exactMatch()) {
		lyxerr[Debug::FONT] << "This font is an exact match" << endl;
	} else {
		lyxerr[Debug::FONT] << "This font is NOT an exact match"
				    << endl;
	}

	lyxerr[Debug::FONT] << "XFLD: " << font.rawName() << endl;

	metrics = QFontMetrics(font);
}


qfont_loader::font_info * qfont_loader::getfontinfo(LyXFont const & f)
{
	if (!lyxrc.use_gui) {
		// FIXME
	}

	font_info * fi = fontinfo_[f.family()][f.series()][f.realShape()][f.size()];
	if (fi)
		return fi;
 
	font_info * fi2 = new font_info(f);
	fontinfo_[f.family()][f.series()][f.realShape()][f.size()] = fi2;
	return fi2;
}


int qfont_loader::charwidth(LyXFont const & f, Uchar val)
{
	font_info * fi = getfontinfo(f);

	font_info::WidthCache::const_iterator cit = fi->widthcache.find(val);
	if (cit != fi->widthcache.end())
		return cit->second;

	int const w = fi->metrics.width(QChar(val));
	fi->widthcache[val] = w;
	return w;
}

 
bool qfont_loader::available(LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return false;

	bool const is_available(isAvailable(f));
	lyxerr[Debug::FONT] << "font_loader::available returning "
		<< is_available << endl;
	return is_available;
}
