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
#include "gettext.h"
#include "debug.h"
#include "lyxrc.h"
#include "BufferView.h"

#include <qglobal.h>
#if QT_VERSION < 300
#include "support/lstrings.h"
#endif

#ifdef Q_WS_X11
#include <qwidget.h>
#include <X11/Xlib.h>
#include "support/systemcall.h"
#include "support/filetools.h"
#endif

using std::endl;


qfont_loader::qfont_loader()
{
}


qfont_loader::~qfont_loader()
{
}


void qfont_loader::update()
{
	for (int i1 = 0; i1 < LyXFont::NUM_FAMILIES; ++i1) {
		for (int i2 = 0; i1 < 2; ++i2) {
			for (int i3 = 0; i1 < 4; ++i3) {
				for (int i4 = 0; i1 < 10; ++i4) {
					fontinfo_[i1][i2][i3][i4].reset(0);
				}
			}
		}
	}
}


QFont const & qfont_loader::get(LyXFont const & f)
{
	QFont const & ret(getfontinfo(f)->font);

	if (lyxerr.debugging(Debug::FONT)) {
		lyxerr[Debug::FONT] << "Font '" << f.stateText(0)
			<< "' matched by\n" << ret.rawName() << endl;
	}

	lyxerr[Debug::FONT] << "The font has size: "
			    << ret.pointSizeFloat() << endl;

	return ret;
}

namespace {

string const symbolPattern(LyXFont::FONT_FAMILY family)
{
	switch (family) {
	case LyXFont::SYMBOL_FAMILY:
		return "-*-symbol-*-*-*-*-*-*-*-*-*-*-adobe-fontspecific";

	case LyXFont::CMR_FAMILY:
		return "-*-cmr10-medium-*-*-*-*-*-*-*-*-*-*-*";

	case LyXFont::CMSY_FAMILY:
		return "-*-cmsy10-*-*-*-*-*-*-*-*-*-*-*-*";

	case LyXFont::CMM_FAMILY:
		return "-*-cmmi10-medium-*-*-*-*-*-*-*-*-*-*-*";

	case LyXFont::CMEX_FAMILY:
		return "-*-cmex10-*-*-*-*-*-*-*-*-*-*-*-*";

	case LyXFont::MSA_FAMILY:
		return "-*-msam10-*-*-*-*-*-*-*-*-*-*-*-*";

	case LyXFont::MSB_FAMILY:
		return "-*-msbm10-*-*-*-*-*-*-*-*-*-*-*-*";

	case LyXFont::EUFRAK_FAMILY:
		return "-*-eufm10-medium-*-*-*-*-*-*-*-*-*-*-*";

	case LyXFont::WASY_FAMILY:
		return "-*-wasy10-medium-*-*-*-*-*-*-*-*-*-*-*";

	default:
		return string();
	}	
}

bool addFontPath()
{
#ifdef Q_WS_X11
	string const dir =  OnlyPath(LibFileSearch("xfonts", "fonts.dir"));
	if (!dir.empty()) {
		QWidget w;
		int n;
		char ** p = XGetFontPath(w.x11Display(), &n);
		if (std::find(p, p+n, dir) != p+n)
			return false;
		lyxerr << "Adding " << dir << " to the font path.\n";
		string const command = "xset fp+ " + dir;
		Systemcall s;
		if (!s.startscript(Systemcall::Wait, command)) 
			return true;
		lyxerr << "Unable to add font path.\n";
	}
#endif
	return false;
}

bool isAvailable(QFont const & font, LyXFont const & f) {
#if QT_VERSION >= 300
	return font.exactMatch();
#else
	string tmp = symbolPattern(f.family());
	if (tmp.empty())
		return false;
	else
		return token(tmp, '-', 2) == 
			token(font.rawName().latin1(), '-', 2);
#endif
}

} // namespace anon

qfont_loader::font_info::font_info(LyXFont const & f)
	: metrics(font)
{

	string pat = symbolPattern(f.family());
	if (!pat.empty()) {
		static bool first_time = true;
		font.setRawName(pat.c_str());
		if (f.family() != LyXFont::SYMBOL_FAMILY &&
		    !isAvailable(font, f) && first_time) {
			first_time = false;
			if (addFontPath()) {
				font.setRawName(pat.c_str());
			}
		}
	} else 
		switch (f.family()) {
		case LyXFont::ROMAN_FAMILY:
			font.setFamily("times");
			break;
		case LyXFont::SANS_FAMILY:
			font.setFamily("helvetica");
			break;
		case LyXFont::TYPEWRITER_FAMILY:
			font.setFamily("courier");
			break;
		default:
			break;
	}

	font.setPointSizeFloat(lyxrc.font_sizes[f.size()]
			       * lyxrc.zoom / 100.0);

	// FIXME: lyxrc, check for failure etc.

	switch (f.series()) {
		case LyXFont::MEDIUM_SERIES:
			font.setWeight(QFont::Normal);
			break;
		case LyXFont::BOLD_SERIES:
			font.setWeight(QFont::Bold);
			break;
	}

	switch (f.realShape()) {
		case LyXFont::ITALIC_SHAPE:
		case LyXFont::SLANTED_SHAPE:
			font.setItalic(true);
			break;
	}

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


qfont_loader::font_info const * qfont_loader::getfontinfo(LyXFont const & f)
{
	if (!lyxrc.use_gui) {
		// FIXME
	}

	font_info * fi = fontinfo_[f.family()][f.series()][f.realShape()][f.size()].get();
	if (!fi) {
		fi = new font_info(f);
		fontinfo_[f.family()][f.series()][f.realShape()][f.size()].reset(fi);
	}

	return fi;
}


bool qfont_loader::available(LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return false;

	return isAvailable(getfontinfo(f)->font, f);
}
