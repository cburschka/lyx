/**
 * \file insetspecialchar.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Jean-Marc Lasgouttes
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "insetspecialchar.h"

#include "debug.h"
#include "dimension.h"
#include "LaTeXFeatures.h"
#include "BufferView.h"
#include "frontends/Painter.h"
#include "frontends/font_metrics.h"
#include "lyxlex.h"
#include "lyxfont.h"
#include "metricsinfo.h"

using std::ostream;
using std::max;
using std::auto_ptr;


InsetSpecialChar::InsetSpecialChar(Kind k)
	: kind_(k)
{}


InsetSpecialChar::Kind InsetSpecialChar::kind() const
{
	return kind_;
}


void InsetSpecialChar::metrics(MetricsInfo & mi, Dimension & dim) const
{
	LyXFont & font = mi.base.font;
	dim.asc = font_metrics::maxAscent(font);
	dim.des = font_metrics::maxDescent(font);

	string s;
	switch (kind_) {
		case LIGATURE_BREAK:      s = "|";     break;
		case END_OF_SENTENCE:     s = ".";     break;
		case LDOTS:               s = ". . ."; break;
		case MENU_SEPARATOR:      s = " x ";   break;
		case HYPHENATION:      s = "-";   break;
	}
	dim.wid = font_metrics::width(s, font);
	if (kind_ == HYPHENATION && dim.wid > 5)
		dim.wid -= 2; // to make it look shorter
	dim_ = dim;
}


void InsetSpecialChar::draw(PainterInfo & pi, int x, int y) const
{
	LyXFont font = pi.base.font;

	switch (kind_) {
	case HYPHENATION:
	{
		font.setColor(LColor::special);
		pi.pain.text(x, y, '-', font);
		break;
	}
	case LIGATURE_BREAK:
	{
		font.setColor(LColor::special);
		pi.pain.text(x, y, '|', font);
		break;
	}
	case END_OF_SENTENCE:
	{
		font.setColor(LColor::special);
		pi.pain.text(x, y, '.', font);
		break;
	}
	case LDOTS:
	{
		font.setColor(LColor::special);
		pi.pain.text(x, y, ". . .", font);
		break;
	}
	case MENU_SEPARATOR:
	{
		// A triangle the width and height of an 'x'
		int w = font_metrics::width('x', font);
		int ox = font_metrics::width(' ', font) + x;
		int h = font_metrics::ascent('x', font);
		int xp[4], yp[4];

		xp[0] = ox;     yp[0] = y;
		xp[1] = ox;     yp[1] = y - h;
		xp[2] = ox + w; yp[2] = y - h/2;
		xp[3] = ox;     yp[3] = y;

		pi.pain.lines(xp, yp, 4, LColor::special);
		break;
	}
	}
}


// In lyxf3 this will be just LaTeX
void InsetSpecialChar::write(Buffer const *, ostream & os) const
{
	string command;
	switch (kind_) {
	case HYPHENATION:
		command = "\\-";
		break;
	case LIGATURE_BREAK:
		command = "\\textcompwordmark{}";
		break;
	case END_OF_SENTENCE:
		command = "\\@.";
		break;
	case LDOTS:
		command = "\\ldots{}";
		break;
	case MENU_SEPARATOR:
		command = "\\menuseparator";
		break;
	}
	os << "\\SpecialChar " << command << "\n";
}


// This function will not be necessary when lyx3
void InsetSpecialChar::read(Buffer const *, LyXLex & lex)
{
	lex.nextToken();
	string const command = lex.getString();

	if (command == "\\-")
		kind_ = HYPHENATION;
	else if (command == "\\textcompwordmark{}")
		kind_ = LIGATURE_BREAK;
	else if (command == "\\@.")
		kind_ = END_OF_SENTENCE;
	else if (command == "\\ldots{}")
		kind_ = LDOTS;
	else if (command == "\\menuseparator")
		kind_ = MENU_SEPARATOR;
	else
		lex.printError("InsetSpecialChar: Unknown kind: `$$Token'");
}


int InsetSpecialChar::latex(Buffer const *, ostream & os,
			    LatexRunParams const &) const
{
	switch (kind_) {
	case HYPHENATION:
		os << "\\-";
		break;
	case LIGATURE_BREAK:
		os << "\\textcompwordmark{}";
		break;
	case END_OF_SENTENCE:
		os << "\\@.";
		break;
	case LDOTS:
		os << "\\ldots{}";
		break;
	case MENU_SEPARATOR:
		os << "\\lyxarrow{}";
		break;
	}
	return 0;
}


int InsetSpecialChar::ascii(Buffer const *, ostream & os, int) const
{
	switch (kind_) {
	case HYPHENATION:
	case LIGATURE_BREAK:
		break;
	case END_OF_SENTENCE:
		os << '.';
		break;
	case LDOTS:
		os << "...";
		break;
	case MENU_SEPARATOR:
		os << "->";
		break;
	}
	return 0;
}


int InsetSpecialChar::linuxdoc(Buffer const *, ostream & os) const
{
	switch (kind_) {
	case HYPHENATION:
	case LIGATURE_BREAK:
		break;
	case END_OF_SENTENCE:
		os << '.';
		break;
	case LDOTS:
		os << "...";
		break;
	case MENU_SEPARATOR:
		os << "&lyxarrow;";
		break;
	}
	return 0;
}


int InsetSpecialChar::docbook(Buffer const *, ostream & os, bool) const
{
	switch (kind_) {
	case HYPHENATION:
	case LIGATURE_BREAK:
		break;
	case END_OF_SENTENCE:
		os << '.';
		break;
	case LDOTS:
		os << "...";
		break;
	case MENU_SEPARATOR:
		os << "&lyxarrow;";
		break;
	}
	return 0;
}


auto_ptr<InsetBase> InsetSpecialChar::clone() const
{
	return auto_ptr<InsetBase>(new InsetSpecialChar(kind_));
}


void InsetSpecialChar::validate(LaTeXFeatures & features) const
{
	if (kind_ == MENU_SEPARATOR) {
		features.require("lyxarrow");
	}
}


bool InsetSpecialChar::isChar() const
{
	return true;
}


bool InsetSpecialChar::isLetter() const
{
	return kind_ == HYPHENATION || kind_ == LIGATURE_BREAK;
}


bool InsetSpecialChar::isSpace() const
{
	return false;
}


bool InsetSpecialChar::isLineSeparator() const
{
#if 0
	// this would be nice, but it does not work, since
	// Paragraph::stripLeadingSpaces nukes the characters which
	// have this property. I leave the code here, since it should
	// eventually be made to work. (JMarc 20020327)
	return kind_ == HYPHENATION || kind_ == MENU_SEPARATOR;
#else
	return false;
#endif
}
