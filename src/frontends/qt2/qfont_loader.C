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
#if QT_VERSION < 0x030000
#include "support/lstrings.h"
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


qfont_loader::font_info::font_info(LyXFont const & f)
	: metrics(font)
{
	switch (f.family()) {
		case LyXFont::SYMBOL_FAMILY:
			font.setRawName("-*-symbol-*-*-*-*-*-*-*-*-*-*-adobe-fontspecific");
			break;
		case LyXFont::CMR_FAMILY:
			font.setRawName("-*-cmr10-medium-*-*-*-*-*-*-*-*-*-*-*");
			break;
		case LyXFont::CMSY_FAMILY:
			font.setRawName("-*-cmsy10-*-*-*-*-*-*-*-*-*-*-*-*");
			break;
		case LyXFont::CMM_FAMILY:
			font.setRawName("-*-cmmi10-medium-*-*-*-*-*-*-*-*-*-*-*");
			break;
		case LyXFont::CMEX_FAMILY:
			font.setRawName("-*-cmex10-*-*-*-*-*-*-*-*-*-*-*-*");
			break;
		case LyXFont::MSA_FAMILY:
			font.setRawName("-*-msam10-*-*-*-*-*-*-*-*-*-*-*-*");
			break;
		case LyXFont::MSB_FAMILY:
			font.setRawName("-*-msbm10-*-*-*-*-*-*-*-*-*-*-*-*");
			break;
		case LyXFont::EUFRAK_FAMILY:
			font.setRawName("-*-eufm10-medium-*-*-*-*-*-*-*-*-*-*-*");
			break;
		case LyXFont::ROMAN_FAMILY:
			font.setFamily("times");
			break;
		case LyXFont::SANS_FAMILY:
			font.setFamily("helvetica");
			break;
		case LyXFont::TYPEWRITER_FAMILY:
			font.setFamily("courier");
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

#if QT_VERSION >= 0x030000
	return getfontinfo(f)->font.exactMatch();
#else
	string tmp;
	switch (f.family()) {
	case LyXFont::SYMBOL_FAMILY:  tmp = "symbol"; break;
	case LyXFont::CMR_FAMILY:     tmp = "cmr10"; break;
	case LyXFont::CMSY_FAMILY:    tmp = "cmsy10"; break;
	case LyXFont::CMM_FAMILY:     tmp = "cmmi10"; break;
	case LyXFont::CMEX_FAMILY:    tmp = "cmex10"; break;
	case LyXFont::MSA_FAMILY:     tmp = "msam10"; break;
	case LyXFont::MSB_FAMILY:     tmp = "msbm10"; break;
	default: break;
	}
	if (tmp.empty())
		return false;
	else
		return token(getfontinfo(f)->font.rawName().latin1(), '-', 2)
			== tmp;
#endif
}
