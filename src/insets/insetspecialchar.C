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

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetspecialchar.h"
#include "debug.h"
#include "LaTeXFeatures.h"
#include "BufferView.h"
#include "frontends/Painter.h"
#include "frontends/font_metrics.h"
#include "lyxlex.h"
#include "lyxfont.h"

using std::ostream;
using std::max;

InsetSpecialChar::InsetSpecialChar(Kind k)
	: kind_(k)
{}


InsetSpecialChar::Kind InsetSpecialChar::kind() const
{
	return kind_;
}

int InsetSpecialChar::ascent(BufferView *, LyXFont const & font) const
{
	return font_metrics::maxAscent(font);
}


int InsetSpecialChar::descent(BufferView *, LyXFont const & font) const
{
	return font_metrics::maxDescent(font);
}


int InsetSpecialChar::width(BufferView *, LyXFont const & font) const
{
	switch (kind_) {
	case HYPHENATION:
	{
		int w = font_metrics::width('-', font);
		if (w > 5)
			w -= 2; // to make it look shorter
		return w;
	}
	case LIGATURE_BREAK:
	{
		return font_metrics::width('|', font);
	}
	case END_OF_SENTENCE:
	{
		return font_metrics::width('.', font);
	}
	case LDOTS:
	{
		return font_metrics::width(". . .", font);
	}
	case MENU_SEPARATOR:
	{
		return font_metrics::width(" x ", font);
	}
	case PROTECTED_SEPARATOR:
	{
		return font_metrics::width('x', font);
	}

	}
	return 1; // To shut up gcc
}


void InsetSpecialChar::draw(BufferView * bv, LyXFont const & f,
			    int baseline, float & x, bool) const
{
	Painter & pain = bv->painter();
	LyXFont font(f);

	switch (kind_) {
	case HYPHENATION:
	{
		font.setColor(LColor::special);
		pain.text(int(x), baseline, "-", font);
		x += width(bv, font);
		break;
	}
	case LIGATURE_BREAK:
	{
		font.setColor(LColor::special);
		pain.text(int(x), baseline, "|", font);
		x += width(bv, font);
		break;
	}
	case END_OF_SENTENCE:
	{
		font.setColor(LColor::special);
		pain.text(int(x), baseline, ".", font);
		x += width(bv, font);
		break;
	}
	case LDOTS:
	{
		font.setColor(LColor::special);
		pain.text(int(x), baseline, ". . .", font);
		x += width(bv, font);
		break;
	}
	case MENU_SEPARATOR:
	{
		// A triangle the width and height of an 'x'
		int w = font_metrics::width('x', font);
		int ox = font_metrics::width(' ', font) + int(x);
		int h = font_metrics::ascent('x', font);
		int xp[4], yp[4];

		xp[0] = ox;	yp[0] = baseline;
		xp[1] = ox;	yp[1] = baseline - h;
		xp[2] = ox + w; yp[2] = baseline - h/2;
		xp[3] = ox;	yp[3] = baseline;

		pain.lines(xp, yp, 4, LColor::special);
		x += width(bv, font);
		break;
	}
	case PROTECTED_SEPARATOR:
	{
		float w = width(bv, font);
		int h = font_metrics::ascent('x', font);
		int xp[4], yp[4];

		xp[0] = int(x);
		yp[0] = baseline - max(h / 4, 1);

		xp[1] = int(x);
		yp[1] = baseline;

		xp[2] = int(x + w);
		yp[2] = baseline;

		xp[3] = int(x + w);
		yp[3] = baseline - max(h / 4, 1);

		pain.lines(xp, yp, 4, LColor::special);
		x += w;
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
	case PROTECTED_SEPARATOR:
		command = "~";
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
	else if (command == "~")
		kind_ = PROTECTED_SEPARATOR;
	else
		lex.printError("InsetSpecialChar: Unknown kind: `$$Token'");
}


int InsetSpecialChar::latex(Buffer const *, ostream & os, bool /*fragile*/,
			    bool free_space) const
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
	case PROTECTED_SEPARATOR:
		os << (free_space ? " " : "~");
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
		os << ".";
		break;
	case LDOTS:
		os << "...";
		break;
	case MENU_SEPARATOR:
		os << "->";
		break;
	case PROTECTED_SEPARATOR:
		os << " ";
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
		os << ".";
		break;
	case LDOTS:
		os << "...";
		break;
	case MENU_SEPARATOR:
		os << "&lyxarrow;";
		break;
	case PROTECTED_SEPARATOR:
		os << "&nbsp;";
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
		os << ".";
		break;
	case LDOTS:
		os << "...";
		break;
	case MENU_SEPARATOR:
		os << "&lyxarrow;";
		break;
	case PROTECTED_SEPARATOR:
		os << "&nbsp;";
		break;
	}
	return 0;
}


Inset * InsetSpecialChar::clone(Buffer const &, bool) const
{
	return new InsetSpecialChar(kind_);
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
	return kind_ == PROTECTED_SEPARATOR;
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
