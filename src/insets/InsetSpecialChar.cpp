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


namespace {

int logoWidth(FontInfo const & font, InsetSpecialChar::Kind kind) {
	frontend::FontMetrics const & fm = theFontMetrics(font);
	int const em = fm.width('M');
	int width = 0;
	// See drawlogo() below to understand what this does.
	switch (kind) {
	case InsetSpecialChar::PHRASE_LYX:
		width = fm.width(from_ascii("L")) - em / 6
			+ fm.width(from_ascii("Y")) - em / 8
			+ fm.width(from_ascii("X"));
		break;

	case InsetSpecialChar::PHRASE_TEX:
		width = fm.width(from_ascii("T")) - em / 6
			+ fm.width(from_ascii("E")) - em / 8
			+ fm.width(from_ascii("X"));
		break;

	case InsetSpecialChar::PHRASE_LATEX2E:
		width = logoWidth(font, InsetSpecialChar::PHRASE_LATEX)
			+ 3 * em / 20
			+ fm.width(from_ascii("2") + char_type(0x03b5));
		break;
	case InsetSpecialChar::PHRASE_LATEX: {
		FontInfo smaller = font;
		smaller.decSize().decSize();
		width = fm.width(from_ascii("L")) - 9 * em / 25
			+ theFontMetrics(smaller).width(from_ascii("A")) - 3 * em / 20
			+ logoWidth(font, InsetSpecialChar::PHRASE_TEX);
		break;
	}
	default:
		LYXERR0("No information for computing width of logo " << kind);
	}

	return width;
}

}


void InsetSpecialChar::metrics(MetricsInfo & mi, Dimension & dim) const
{
	frontend::FontMetrics const & fm =
		theFontMetrics(mi.base.font);
	dim.asc = fm.maxAscent();
	dim.des = fm.maxDescent();
	dim.wid = 0;

	docstring s;
	switch (kind_) {
		case LIGATURE_BREAK:
			s = from_ascii("|");
			break;
		case END_OF_SENTENCE:
			s = from_ascii(".");
			break;
		case LDOTS:
			s = from_ascii(". . .");
			break;
		case MENU_SEPARATOR:
			s = from_ascii(" x ");
			break;
		case HYPHENATION:
			dim.wid = fm.width(from_ascii("-"));
			if (dim.wid > 5)
				dim.wid -= 2; // to make it look shorter
			break;
		case SLASH:
			s = from_ascii("/");
			break;
		case NOBREAKDASH:
			s = from_ascii("-");
			break;
		case PHRASE_LYX:
		case PHRASE_TEX:
		case PHRASE_LATEX2E:
		case PHRASE_LATEX:
			dim.wid = logoWidth(mi.base.font, kind_);
			break;
	}
	if (dim.wid == 0)
		dim.wid = fm.width(s);

	setDimCache(mi, dim);
}


namespace {

void drawLogo(PainterInfo & pi, InsetSpecialChar::Kind kind, int & x, int & y) {
	FontInfo const & font = pi.base.font;
	// FIXME: this definition of em is bogus, but there is a need
	// for a big refactoring of the code around this issue anyway.
	int const em = theFontMetrics(font).width('M');
	switch (kind) {
	case InsetSpecialChar::PHRASE_LYX:
		/** Reference macro:
		 *  \providecommand{\LyX}{L\kern-.1667em\lower.25em\hbox{Y}\kern-.125emX\\@};
		 */
		x += pi.pain.text(x, y, from_ascii("L"), font);
		x -= em / 6;
		x += pi.pain.text(x, y + em / 4, from_ascii("Y"), font);
		x -= em / 8;
		x += pi.pain.text(x, y, from_ascii("X"), font);
		break;

	case InsetSpecialChar::PHRASE_TEX: {
		/** Reference macro:
		 *  \def\TeX{T\kern-.1667em\lower.5ex\hbox{E}\kern-.125emX\@}
		 */
		int const ex = theFontMetrics(font).ascent('x');
		x += pi.pain.text(x, y, from_ascii("T"), font);
		x -= em / 6;
		x += pi.pain.text(x, y + ex / 2, from_ascii("E"), font);
		x -= em / 8;
		x += pi.pain.text(x, y, from_ascii("X"), font);
		break;
	}
	case InsetSpecialChar::PHRASE_LATEX2E:
		/** Reference macro:
		 *  \DeclareRobustCommand{\LaTeXe}{\mbox{\m@th
		 *    \if b\expandafter\@car\f@series\@nil\boldmath\fi
		 *    \LaTeX\kern.15em2$_{\textstyle\varepsilon}$}}
		 */
		drawLogo(pi, InsetSpecialChar::PHRASE_LATEX, x, y);
		x += 3 * em / 20;
		x += pi.pain.text(x, y, from_ascii("2"), font);
		x += pi.pain.text(x, y + em / 4, char_type(0x03b5), font);
		break;

	case InsetSpecialChar::PHRASE_LATEX: {
		/** Reference macro:
		 * \DeclareRobustCommand{\LaTeX}{L\kern-.36em%
		 *        {\sbox\z@ T%
		 *         \vbox to\ht\z@{\hbox{\check@mathfonts
		 *                              \fontsize\sf@size\z@
		 *                              \math@fontsfalse\selectfont
		 *                              A}%
		 *                        \vss}%
		 *        }%
		 *        \kern-.15em%
		 *        \TeX}
		 */
		x += pi.pain.text(x, y, from_ascii("L"), font);
		x -= 9 * em / 25;
		FontInfo smaller = font;
		smaller.decSize().decSize();
		x += pi.pain.text(x, y - em / 5, from_ascii("A"), smaller);
		x -= 3 * em / 20;
		drawLogo(pi, InsetSpecialChar::PHRASE_TEX, x, y);
		break;
	}
	default:
		LYXERR0("No information for drawing logo " << kind);
	}
}

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
	case PHRASE_LYX:
	case PHRASE_TEX:
	case PHRASE_LATEX2E:
	case PHRASE_LATEX:
		drawLogo(pi, kind_, x, y);
		break;
	}
}


// In lyxf3 this will be just LaTeX
void InsetSpecialChar::write(ostream & os) const
{
	string command;
	switch (kind_) {
	case HYPHENATION:
		command = "softhyphen";
		break;
	case LIGATURE_BREAK:
		command = "ligaturebreak";
		break;
	case END_OF_SENTENCE:
		command = "endofsentence";
		break;
	case LDOTS:
		command = "ldots";
		break;
	case MENU_SEPARATOR:
		command = "menuseparator";
		break;
	case SLASH:
		command = "breakableslash";
		break;
	case NOBREAKDASH:
		command = "nobreakdash";
		break;
	case PHRASE_LYX:
		command = "LyX";
		break;
	case PHRASE_TEX:
		command = "TeX";
		break;
	case PHRASE_LATEX2E:
		command = "LaTeX2e";
		break;
	case PHRASE_LATEX:
		command = "LaTeX";
		break;
	}
	os << "\\SpecialChar " << command << "\n";
}


// This function will not be necessary when lyx3
void InsetSpecialChar::read(Lexer & lex)
{
	lex.next();
	string const command = lex.getString();

	if (command == "softhyphen")
		kind_ = HYPHENATION;
	else if (command == "ligaturebreak")
		kind_ = LIGATURE_BREAK;
	else if (command == "endofsentence")
		kind_ = END_OF_SENTENCE;
	else if (command == "ldots")
		kind_ = LDOTS;
	else if (command == "menuseparator")
		kind_ = MENU_SEPARATOR;
	else if (command == "breakableslash")
		kind_ = SLASH;
	else if (command == "nobreakdash")
		kind_ = NOBREAKDASH;
	else if (command == "LyX")
		kind_ = PHRASE_LYX;
	else if (command == "TeX")
		kind_ = PHRASE_TEX;
	else if (command == "LaTeX2e")
		kind_ = PHRASE_LATEX2E;
	else if (command == "LaTeX")
		kind_ = PHRASE_LATEX;
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
	case PHRASE_LYX:
		if (rp.moving_arg)
			os << "\\protect";
		os << "\\LyX{}";
		break;
	case PHRASE_TEX:
		if (rp.moving_arg)
			os << "\\protect";
		os << "\\TeX{}";
		break;
	case PHRASE_LATEX2E:
		if (rp.moving_arg)
			os << "\\protect";
		os << "\\LaTeXe{}";
		break;
	case PHRASE_LATEX:
		if (rp.moving_arg)
			os << "\\protect";
		os << "\\LaTeX{}";
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
	case PHRASE_LYX:
		os << "LyX";
		return 3;
	case PHRASE_TEX:
		os << "TeX";
		return 3;
	case PHRASE_LATEX2E:
		os << "LaTeX2";
		os.put(0x03b5);
		return 7;
	case PHRASE_LATEX:
		os << "LaTeX";
		return 5;
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
	case PHRASE_LYX:
		os << "LyX";
		break;
	case PHRASE_TEX:
		os << "TeX";
		break;
	case PHRASE_LATEX2E:
		os << "LaTeX2";
		os.put(0x03b5);
		break;
	case PHRASE_LATEX:
		os << "LaTeX";
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
	case PHRASE_LYX:
		xs << "LyX";
		break;
	case PHRASE_TEX:
		xs << "TeX";
		break;
	case PHRASE_LATEX2E:
		xs << "LaTeX2" << XHTMLStream::ESCAPE_NONE << "&#x3b5;";
		break;
	case PHRASE_LATEX:
		xs << "LaTeX";
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
	if (kind_ == PHRASE_LYX)
		features.require("LyX");
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
