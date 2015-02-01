/**
 * \file InsetSpecialChar.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Jean-Marc Lasgouttes
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetSpecialChar.h"

#include "Dimension.h"
#include "Font.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "output_xhtml.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/docstream.h"

using namespace std;

namespace lyx {


InsetSpecialChar::InsetSpecialChar(Kind k)
	: Inset(0), kind_(k)
{}


InsetSpecialChar::Kind InsetSpecialChar::kind() const
{
	return kind_;
}


void InsetSpecialChar::metrics(MetricsInfo & mi, Dimension & dim) const
{
	frontend::FontMetrics const & fm =
		theFontMetrics(mi.base.font);
	dim.asc = fm.maxAscent();
	dim.des = fm.maxDescent();

	string s;
	switch (kind_) {
		case LIGATURE_BREAK:
			s = "|";
			break;
		case END_OF_SENTENCE:
			s = ".";
			break;
		case LDOTS:
			s = ". . .";
			break;
		case MENU_SEPARATOR:
			s = " x ";
			break;
		case HYPHENATION:
			s = "-";
			break;
		case SLASH:
			s = "/";
			break;
		case NOBREAKDASH:
			s = "-";
			break;
	}
	docstring ds(s.begin(), s.end());
	dim.wid = fm.width(ds);
	if (kind_ == HYPHENATION && dim.wid > 5)
		dim.wid -= 2; // to make it look shorter
	
	setDimCache(mi, dim);
}


void InsetSpecialChar::draw(PainterInfo & pi, int x, int y) const
{
	FontInfo font = pi.base.font;

	switch (kind_) {
	case HYPHENATION:
	{
		font.setColor(Color_special);
		pi.pain.text(x, y, char_type('-'), font);
		break;
	}
	case LIGATURE_BREAK:
	{
		font.setColor(Color_special);
		pi.pain.text(x, y, char_type('|'), font);
		break;
	}
	case END_OF_SENTENCE:
	{
		font.setColor(Color_special);
		pi.pain.text(x, y, char_type('.'), font);
		break;
	}
	case LDOTS:
	{
		font.setColor(Color_special);
		string ell = ". . . ";
		docstring dell(ell.begin(), ell.end());
		pi.pain.text(x, y, dell, font);
		break;
	}
	case MENU_SEPARATOR:
	{
		frontend::FontMetrics const & fm =
			theFontMetrics(font);

		// A triangle the width and height of an 'x'
		int w = fm.width(char_type('x'));
		int ox = fm.width(char_type(' ')) + x;
		int h = fm.ascent(char_type('x'));
		int xp[4], yp[4];

		xp[0] = ox;     yp[0] = y;
		xp[1] = ox;     yp[1] = y - h;
		xp[2] = ox + w; yp[2] = y - h/2;
		xp[3] = ox;     yp[3] = y;

		pi.pain.lines(xp, yp, 4, Color_special);
		break;
	}
	case SLASH:
	{
		font.setColor(Color_special);
		pi.pain.text(x, y, char_type('/'), font);
		break;
	}
	case NOBREAKDASH:
	{
		font.setColor(Color_latex);
		pi.pain.text(x, y, char_type('-'), font);
		break;
	}
	}
}


// In lyxf3 this will be just LaTeX
void InsetSpecialChar::write(ostream & os) const
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
	case SLASH:
		command = "\\slash{}";
		break;
	case NOBREAKDASH:
		command = "\\nobreakdash-";
		break;
	}
	os << "\\SpecialChar " << command << "\n";
}


// This function will not be necessary when lyx3
void InsetSpecialChar::read(Lexer & lex)
{
	lex.next();
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
	else if (command == "\\slash{}")
		kind_ = SLASH;
	else if (command == "\\nobreakdash-")
		kind_ = NOBREAKDASH;
	else
		lex.printError("InsetSpecialChar: Unknown kind: `$$Token'");
}


void InsetSpecialChar::latex(otexstream & os,
			     OutputParams const & rp) const
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
		if (rp.local_font->isRightToLeft())
			os << "\\lyxarrow*{}";
		else
			os << "\\lyxarrow{}";
		break;
	case SLASH:
		os << "\\slash{}";
		break;
	case NOBREAKDASH:
		if (rp.moving_arg)
			os << "\\protect";
		os << "\\nobreakdash-";
		break;
	}
}


int InsetSpecialChar::plaintext(odocstringstream & os,
        OutputParams const &, size_t) const
{
	switch (kind_) {
	case HYPHENATION:
		return 0;
	case LIGATURE_BREAK:
		os.put(0x200c);
		return 1;
	case END_OF_SENTENCE:
		os << '.';
		return 1;
	case LDOTS:
		os.put(0x2026);
		return 1;
	case MENU_SEPARATOR:
		os << "->";
		return 2;
	case SLASH:
		os << '/';
		return 1;
	case NOBREAKDASH:
		os.put(0x2011);
		return 1;
	}
	return 0;
}


int InsetSpecialChar::docbook(odocstream & os, OutputParams const &) const
{
	switch (kind_) {
	case HYPHENATION:
	case LIGATURE_BREAK:
		break;
	case END_OF_SENTENCE:
		os << '.';
		break;
	case LDOTS:
		os << "&hellip;";
		break;
	case MENU_SEPARATOR:
		os << "&lyxarrow;";
		break;
	case SLASH:
		os << '/';
		break;
	case NOBREAKDASH:
		os << '-';
		break;
	}
	return 0;
}


docstring InsetSpecialChar::xhtml(XHTMLStream & xs, OutputParams const &) const
{
	switch (kind_) {
	case HYPHENATION:
		break;
	case LIGATURE_BREAK:
		xs << XHTMLStream::ESCAPE_NONE << "&#8204;";
		break;
	case END_OF_SENTENCE:
		xs << '.';
		break;
	case LDOTS:
		xs << XHTMLStream::ESCAPE_NONE << "&hellip;";
		break;
	case MENU_SEPARATOR:
		xs << XHTMLStream::ESCAPE_NONE << "&rArr;";
		break;
	case SLASH:
		xs << XHTMLStream::ESCAPE_NONE << "&frasl;";
		break;
	case NOBREAKDASH:
		xs << XHTMLStream::ESCAPE_NONE << "&#8209;";
		break;
	}
	return docstring();
}


void InsetSpecialChar::toString(odocstream & os) const
{
	switch (kind_) {
	case LIGATURE_BREAK:
		// Do not output ZERO WIDTH NON JOINER here
		// Spell checker would choke on it.
		return;
	default:
		break;
	}
	odocstringstream ods;
	plaintext(ods, OutputParams(0));
	os << ods.str();
}


void InsetSpecialChar::forOutliner(docstring & os, size_t) const
{
	odocstringstream ods;
	plaintext(ods, OutputParams(0));
	os += ods.str();
}


void InsetSpecialChar::validate(LaTeXFeatures & features) const
{
	if (kind_ == MENU_SEPARATOR)
		features.require("lyxarrow");
	if (kind_ == NOBREAKDASH)
		features.require("amsmath");
}


bool InsetSpecialChar::isLetter() const
{
	return kind_ == HYPHENATION || kind_ == LIGATURE_BREAK
		|| kind_ == NOBREAKDASH;
}


bool InsetSpecialChar::isLineSeparator() const
{
#if 0
	// this would be nice, but it does not work, since
	// Paragraph::stripLeadingSpaces nukes the characters which
	// have this property. I leave the code here, since it should
	// eventually be made to work. (JMarc 20020327)
	return kind_ == HYPHENATION || kind_ == MENU_SEPARATOR
		|| kind_ == SLASH;
#else
	return false;
#endif
}


} // namespace lyx
