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
 
using std::endl;
 
qfont_loader::qfont_loader()
{
}


qfont_loader::~qfont_loader()
{
}


void qfont_loader::update()
{
	int i1,i2,i3,i4;

	// fuck this !
 
	for (i1 = 0; i1 < LyXFont::NUM_FAMILIES; ++i1) {
		for (i2 = 0; i1 < 2; ++i2) {
			for (i3 = 0; i1 < 4; ++i3) {
				for (i4 = 0; i1 < 10; ++i4) {
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
			<< "' matched by\n" << ret.rawName().latin1() << endl;
	}
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
 
	font.setPointSize(int((lyxrc.font_sizes[f.size()] * lyxrc.dpi * 
		(lyxrc.zoom / 100.0)) / 72.27 + 0.5));
 
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

	metrics = QFontMetrics(font);
}


qfont_loader::font_info const * qfont_loader::getfontinfo(LyXFont const & f)
{
	if (!lyxrc.use_gui) {
		// FIXME
	}

	font_info * fi = fontinfo_[f.family()][f.series()][f.realShape()][f.size()].get();
	if (fi) {
		return fi;
	} else {
		fi = new font_info(f);
		fontinfo_[f.family()][f.series()][f.realShape()][f.size()].reset(fi);
		return fi;
	}
}

 
bool qfont_loader::available(LyXFont const & f)
{
	if (!lyxrc.use_gui)
		return false;

	return getfontinfo(f)->font.exactMatch();
}
