/**
 * \file src/Font.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author André Pönitz
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Font.h"

#include "BufferParams.h" // stateText
#include "debug.h"
#include "Encoding.h"
#include "gettext.h"
#include "Language.h"
#include "Color.h"
#include "Lexer.h"
#include "LyXRC.h"
#include "output_latex.h"
#include "OutputParams.h"

#include "support/lstrings.h"


namespace lyx {

using support::ascii_lowercase;
using support::bformat;
using support::rtrim;
using support::subst;

using std::endl;
using std::string;
using std::ostream;
using std::pair;

#ifndef CXX_GLOBAL_CSTD
using std::strlen;
#endif

//
// Names for the GUI
//

namespace {

char const * GUIFamilyNames[Font::NUM_FAMILIES + 2 /* default & error */] =
{ N_("Roman"), N_("Sans Serif"), N_("Typewriter"), N_("Symbol"),
  "cmr", "cmsy", "cmm", "cmex", "msa", "msb", "eufrak", "wasy", "esint",
  N_("Inherit"), N_("Ignore") };

char const * GUISeriesNames[4] =
{ N_("Medium"), N_("Bold"), N_("Inherit"), N_("Ignore") };

char const * GUIShapeNames[6] =
{ N_("Upright"), N_("Italic"), N_("Slanted"), N_("Smallcaps"), N_("Inherit"),
  N_("Ignore") };

char const * GUISizeNames[14] =
{ N_("Tiny"), N_("Smallest"), N_("Smaller"), N_("Small"), N_("Normal"), N_("Large"),
  N_("Larger"), N_("Largest"), N_("Huge"), N_("Huger"), N_("Increase"), N_("Decrease"),
  N_("Inherit"), N_("Ignore") };

char const * GUIMiscNames[5] =
{ N_("Off"), N_("On"), N_("Toggle"), N_("Inherit"), N_("Ignore") };


//
// Strings used to read and write .lyx format files
//
char const * LyXFamilyNames[Font::NUM_FAMILIES + 2 /* default & error */] =
{ "roman", "sans", "typewriter", "symbol",
  "cmr", "cmsy", "cmm", "cmex", "msa", "msb", "eufrak", "wasy", "esint",
  "default", "error" };

char const * LyXSeriesNames[4] =
{ "medium", "bold", "default", "error" };

char const * LyXShapeNames[6] =
{ "up", "italic", "slanted", "smallcaps", "default", "error" };

char const * LyXSizeNames[14] =
{ "tiny", "scriptsize", "footnotesize", "small", "normal", "large",
  "larger", "largest", "huge", "giant",
  "increase", "decrease", "default", "error" };

char const * LyXMiscNames[5] =
{ "off", "on", "toggle", "default", "error" };

//
// Strings used to write LaTeX files
//
char const * LaTeXFamilyNames[6] =
{ "textrm", "textsf", "texttt", "error1", "error2", "error3" };

char const * LaTeXSeriesNames[4] =
{ "textmd", "textbf", "error4", "error5" };

char const * LaTeXShapeNames[6] =
{ "textup", "textit", "textsl", "textsc", "error6", "error7" };

char const * LaTeXSizeNames[14] =
{ "tiny", "scriptsize", "footnotesize", "small", "normalsize", "large",
  "Large", "LARGE", "huge", "Huge", "error8", "error9", "error10", "error11" };

} // namespace anon


// Initialize static member
Font::FontBits Font::sane = {
	ROMAN_FAMILY,
	MEDIUM_SERIES,
	UP_SHAPE,
	SIZE_NORMAL,
	Color::none,
	OFF,
	OFF,
	OFF,
	OFF };

// Initialize static member
Font::FontBits Font::inherit = {
	INHERIT_FAMILY,
	INHERIT_SERIES,
	INHERIT_SHAPE,
	INHERIT_SIZE,
	Color::inherit,
	INHERIT,
	INHERIT,
	INHERIT,
	OFF };

// Initialize static member
Font::FontBits Font::ignore = {
	IGNORE_FAMILY,
	IGNORE_SERIES,
	IGNORE_SHAPE,
	IGNORE_SIZE,
	Color::ignore,
	IGNORE,
	IGNORE,
	IGNORE,
	IGNORE };


bool operator==(Font::FontBits const & lhs,
		Font::FontBits const & rhs)
{
	return lhs.family == rhs.family &&
		lhs.series == rhs.series &&
		lhs.shape == rhs.shape &&
		lhs.size == rhs.size &&
		lhs.color == rhs.color &&
		lhs.emph == rhs.emph &&
		lhs.underbar == rhs.underbar &&
		lhs.noun == rhs.noun &&
		lhs.number == rhs.number;
}


Font::Font()
	: bits(sane), lang(default_language), open_encoding_(false)
{}


Font::Font(Font::FONT_INIT1)
	: bits(inherit), lang(default_language), open_encoding_(false)
{}


Font::Font(Font::FONT_INIT2)
	: bits(ignore), lang(ignore_language), open_encoding_(false)
{}


Font::Font(Font::FONT_INIT3)
	: bits(sane), lang(default_language), open_encoding_(false)
{}


Font::Font(Font::FONT_INIT1, Language const * l)
	: bits(inherit), lang(l), open_encoding_(false)
{}


Font::Font(Font::FONT_INIT2, Language const * l)
	: bits(ignore), lang(l), open_encoding_(false)
{}


Font::Font(Font::FONT_INIT3, Language const * l)
	: bits(sane), lang(l), open_encoding_(false)
{}



Color_color Font::color() const
{
	return Color::color(bits.color);
}


bool Font::isRightToLeft() const
{
	return lang->rightToLeft();
}


bool Font::isVisibleRightToLeft() const
{
	return (lang->rightToLeft() &&
		number() != ON);
}


void Font::setFamily(Font::FONT_FAMILY f)
{
	bits.family = f;
}


void Font::setSeries(Font::FONT_SERIES s)
{
	bits.series = s;
}


void Font::setShape(Font::FONT_SHAPE s)
{
	bits.shape = s;
}


void Font::setSize(Font::FONT_SIZE s)
{
	bits.size = s;
}


void Font::setEmph(Font::FONT_MISC_STATE e)
{
	bits.emph = e;
}


void Font::setUnderbar(Font::FONT_MISC_STATE u)
{
	bits.underbar = u;
}


void Font::setNoun(Font::FONT_MISC_STATE n)
{
	bits.noun = n;
}


void Font::setColor(Color_color c)
{
	bits.color = int(c);
}


void Font::setLanguage(Language const * l)
{
	lang = l;
}


void Font::setNumber(Font::FONT_MISC_STATE n)
{
	bits.number = n;
}


/// Decreases font size by one
Font & Font::decSize()
{
	switch (size()) {
	case SIZE_HUGER:        setSize(SIZE_HUGE);     break;
	case SIZE_HUGE:         setSize(SIZE_LARGEST);  break;
	case SIZE_LARGEST:      setSize(SIZE_LARGER);   break;
	case SIZE_LARGER:       setSize(SIZE_LARGE);    break;
	case SIZE_LARGE:        setSize(SIZE_NORMAL);   break;
	case SIZE_NORMAL:       setSize(SIZE_SMALL);    break;
	case SIZE_SMALL:        setSize(SIZE_FOOTNOTE); break;
	case SIZE_FOOTNOTE:     setSize(SIZE_SCRIPT);   break;
	case SIZE_SCRIPT:       setSize(SIZE_TINY);     break;
	case SIZE_TINY:         break;
	case INCREASE_SIZE:
		lyxerr << "Can't Font::decSize on INCREASE_SIZE" << endl;
		break;
	case DECREASE_SIZE:
		lyxerr <<"Can't Font::decSize on DECREASE_SIZE" << endl;
		break;
	case INHERIT_SIZE:
		lyxerr <<"Can't Font::decSize on INHERIT_SIZE" << endl;
		break;
	case IGNORE_SIZE:
		lyxerr <<"Can't Font::decSize on IGNORE_SIZE" << endl;
		break;
	}
	return *this;
}


/// Increases font size by one
Font & Font::incSize()
{
	switch (size()) {
	case SIZE_HUGER:	break;
	case SIZE_HUGE:         setSize(SIZE_HUGER);    break;
	case SIZE_LARGEST:      setSize(SIZE_HUGE);     break;
	case SIZE_LARGER:       setSize(SIZE_LARGEST);  break;
	case SIZE_LARGE:        setSize(SIZE_LARGER);   break;
	case SIZE_NORMAL:       setSize(SIZE_LARGE);    break;
	case SIZE_SMALL:        setSize(SIZE_NORMAL);   break;
	case SIZE_FOOTNOTE:     setSize(SIZE_SMALL);    break;
	case SIZE_SCRIPT:       setSize(SIZE_FOOTNOTE); break;
	case SIZE_TINY:         setSize(SIZE_SCRIPT);   break;
	case INCREASE_SIZE:
		lyxerr <<"Can't Font::incSize on INCREASE_SIZE" << endl;
		break;
	case DECREASE_SIZE:
		lyxerr <<"Can't Font::incSize on DECREASE_SIZE" << endl;
		break;
	case INHERIT_SIZE:
		lyxerr <<"Can't Font::incSize on INHERIT_SIZE" << endl;
		break;
	case IGNORE_SIZE:
		lyxerr <<"Can't Font::incSize on IGNORE_SIZE" << endl;
		break;
	}
	return *this;
}


/// Updates a misc setting according to request
Font::FONT_MISC_STATE Font::setMisc(FONT_MISC_STATE newfont,
					  FONT_MISC_STATE org)
{
	if (newfont == TOGGLE) {
		if (org == ON)
			return OFF;
		else if (org == OFF)
			return ON;
		else {
			lyxerr <<"Font::setMisc: Need state"
				" ON or OFF to toggle. Setting to ON" << endl;
			return ON;
		}
	} else if (newfont == IGNORE)
		return org;
	else
		return newfont;
}


/// Updates font settings according to request
void Font::update(Font const & newfont,
		     Language const * document_language,
		     bool toggleall)
{
	if (newfont.family() == family() && toggleall)
		setFamily(INHERIT_FAMILY); // toggle 'back'
	else if (newfont.family() != IGNORE_FAMILY)
		setFamily(newfont.family());
	// else it's IGNORE_SHAPE

	// "Old" behaviour: "Setting" bold will toggle bold on/off.
	switch (newfont.series()) {
	case BOLD_SERIES:
		// We toggle...
		if (series() == BOLD_SERIES && toggleall)
			setSeries(MEDIUM_SERIES);
		else
			setSeries(BOLD_SERIES);
		break;
	case MEDIUM_SERIES:
	case INHERIT_SERIES:
		setSeries(newfont.series());
		break;
	case IGNORE_SERIES:
		break;
	}

	if (newfont.shape() == shape() && toggleall)
		setShape(INHERIT_SHAPE); // toggle 'back'
	else if (newfont.shape() != IGNORE_SHAPE)
		setShape(newfont.shape());
	// else it's IGNORE_SHAPE

	if (newfont.size() != IGNORE_SIZE) {
		if (newfont.size() == INCREASE_SIZE)
			incSize();
		else if (newfont.size() == DECREASE_SIZE)
			decSize();
		else
			setSize(newfont.size());
	}

	setEmph(setMisc(newfont.emph(), emph()));
	setUnderbar(setMisc(newfont.underbar(), underbar()));
	setNoun(setMisc(newfont.noun(), noun()));

	setNumber(setMisc(newfont.number(), number()));
	if (newfont.language() == language() && toggleall)
		if (language() == document_language)
			setLanguage(default_language);
		else
			setLanguage(document_language);
	else if (newfont.language() != ignore_language)
		setLanguage(newfont.language());

	if (newfont.color() == color() && toggleall)
		setColor(Color::inherit); // toggle 'back'
	else if (newfont.color() != Color::ignore)
		setColor(newfont.color());
}


/// Reduce font to fall back to template where possible
void Font::reduce(Font const & tmplt)
{
	if (family() == tmplt.family())
		setFamily(INHERIT_FAMILY);
	if (series() == tmplt.series())
		setSeries(INHERIT_SERIES);
	if (shape() == tmplt.shape())
		setShape(INHERIT_SHAPE);
	if (size() == tmplt.size())
		setSize(INHERIT_SIZE);
	if (emph() == tmplt.emph())
		setEmph(INHERIT);
	if (underbar() == tmplt.underbar())
		setUnderbar(INHERIT);
	if (noun() == tmplt.noun())
		setNoun(INHERIT);
	if (color() == tmplt.color())
		setColor(Color::inherit);
}


/// Realize font from a template
Font & Font::realize(Font const & tmplt)
{
	if (bits == inherit) {
		bits = tmplt.bits;
		return *this;
	}

	if (bits.family == INHERIT_FAMILY)
		bits.family = tmplt.bits.family;

	if (bits.series == INHERIT_SERIES)
		bits.series = tmplt.bits.series;

	if (bits.shape == INHERIT_SHAPE)
		bits.shape = tmplt.bits.shape;

	if (bits.size == INHERIT_SIZE)
		bits.size = tmplt.bits.size;

	if (bits.emph == INHERIT)
		bits.emph = tmplt.bits.emph;

	if (bits.underbar == INHERIT)
		bits.underbar = tmplt.bits.underbar;

	if (bits.noun == INHERIT)
		bits.noun = tmplt.bits.noun;

	if (bits.color == Color::inherit)
		bits.color = tmplt.bits.color;

	return *this;
}


/// Is font resolved?
bool Font::resolved() const
{
	return (family() != INHERIT_FAMILY && series() != INHERIT_SERIES &&
		shape() != INHERIT_SHAPE && size() != INHERIT_SIZE &&
		emph() != INHERIT && underbar() != INHERIT &&
		noun() != INHERIT &&
		color() != Color::inherit);
}


docstring const Font::stateText(BufferParams * params) const
{
	odocstringstream os;
	if (family() != INHERIT_FAMILY)
		os << _(GUIFamilyNames[family()]) << ", ";
	if (series() != INHERIT_SERIES)
		os << _(GUISeriesNames[series()]) << ", ";
	if (shape() != INHERIT_SHAPE)
		os << _(GUIShapeNames[shape()]) << ", ";
	if (size() != INHERIT_SIZE)
		os << _(GUISizeNames[size()]) << ", ";
	if (color() != Color::inherit)
		os << lcolor.getGUIName(color()) << ", ";
	if (emph() != INHERIT)
		os << bformat(_("Emphasis %1$s, "),
			      _(GUIMiscNames[emph()]));
	if (underbar() != INHERIT)
		os << bformat(_("Underline %1$s, "),
			      _(GUIMiscNames[underbar()]));
	if (noun() != INHERIT)
		os << bformat(_("Noun %1$s, "),
			      _(GUIMiscNames[noun()]));
	if (bits == inherit)
		os << _("Default") << ", ";
	if (!params || (language() != params->language))
		os << bformat(_("Language: %1$s, "),
			      _(language()->display()));
	if (number() != OFF)
		os << bformat(_("  Number %1$s"),
			      _(GUIMiscNames[number()]));
	return rtrim(os.str(), ", ");
}


// Set family according to lyx format string
Font & Font::setLyXFamily(string const & fam)
{
	string const s = ascii_lowercase(fam);

	int i = 0;
	while (LyXFamilyNames[i] != s &&
	       LyXFamilyNames[i] != string("error"))
		++i;
	if (s == LyXFamilyNames[i])
		setFamily(Font::FONT_FAMILY(i));
	else
		lyxerr << "Font::setLyXFamily: Unknown family `"
		       << s << '\'' << endl;
	return *this;
}


// Set series according to lyx format string
Font & Font::setLyXSeries(string const & ser)
{
	string const s = ascii_lowercase(ser);

	int i = 0;
	while (LyXSeriesNames[i] != s &&
	       LyXSeriesNames[i] != string("error")) ++i;
	if (s == LyXSeriesNames[i]) {
		setSeries(Font::FONT_SERIES(i));
	} else
		lyxerr << "Font::setLyXSeries: Unknown series `"
		       << s << '\'' << endl;
	return *this;
}


// Set shape according to lyx format string
Font & Font::setLyXShape(string const & sha)
{
	string const s = ascii_lowercase(sha);

	int i = 0;
	while (LyXShapeNames[i] != s && LyXShapeNames[i] != string("error"))
			++i;
	if (s == LyXShapeNames[i])
		setShape(Font::FONT_SHAPE(i));
	else
		lyxerr << "Font::setLyXShape: Unknown shape `"
		       << s << '\'' << endl;
	return *this;
}


// Set size according to lyx format string
Font & Font::setLyXSize(string const & siz)
{
	string const s = ascii_lowercase(siz);
	int i = 0;
	while (LyXSizeNames[i] != s && LyXSizeNames[i] != string("error"))
		++i;
	if (s == LyXSizeNames[i]) {
		setSize(Font::FONT_SIZE(i));
	} else
		lyxerr << "Font::setLyXSize: Unknown size `"
		       << s << '\'' << endl;
	return *this;
}


// Set size according to lyx format string
Font::FONT_MISC_STATE Font::setLyXMisc(string const & siz)
{
	string const s = ascii_lowercase(siz);
	int i = 0;
	while (LyXMiscNames[i] != s &&
	       LyXMiscNames[i] != string("error")) ++i;
	if (s == LyXMiscNames[i])
		return FONT_MISC_STATE(i);
	lyxerr << "Font::setLyXMisc: Unknown misc flag `"
	       << s << '\'' << endl;
	return OFF;
}


/// Sets color after LyX text format
Font & Font::setLyXColor(string const & col)
{
	setColor(lcolor.getFromLyXName(col));
	return *this;
}


// Returns size in latex format
string const Font::latexSize() const
{
	return LaTeXSizeNames[size()];
}


// Read a font definition from given file in lyx format
// Used for layouts
Font & Font::lyxRead(Lexer & lex)
{
	bool error = false;
	bool finished = false;
	while (!finished && lex.isOK() && !error) {
		lex.next();
		string const tok = ascii_lowercase(lex.getString());

		if (tok.empty()) {
			continue;
		} else if (tok == "endfont") {
			finished = true;
		} else if (tok == "family") {
			lex.next();
			string const ttok = lex.getString();
			setLyXFamily(ttok);
		} else if (tok == "series") {
			lex.next();
			string const ttok = lex.getString();
			setLyXSeries(ttok);
		} else if (tok == "shape") {
			lex.next();
			string const ttok = lex.getString();
			setLyXShape(ttok);
		} else if (tok == "size") {
			lex.next();
			string const ttok = lex.getString();
			setLyXSize(ttok);
		} else if (tok == "misc") {
			lex.next();
			string const ttok = ascii_lowercase(lex.getString());

			if (ttok == "no_bar") {
				setUnderbar(OFF);
			} else if (ttok == "no_emph") {
				setEmph(OFF);
			} else if (ttok == "no_noun") {
				setNoun(OFF);
			} else if (ttok == "emph") {
				setEmph(ON);
			} else if (ttok == "underbar") {
				setUnderbar(ON);
			} else if (ttok == "noun") {
				setNoun(ON);
			} else {
				lex.printError("Illegal misc type `$$Token'");
			}
		} else if (tok == "color") {
			lex.next();
			string const ttok = lex.getString();
			setLyXColor(ttok);
		} else {
			lex.printError("Unknown tag `$$Token'");
			error = true;
		}
	}
	return *this;
}


/// Writes the changes from this font to orgfont in .lyx format in file
void Font::lyxWriteChanges(Font const & orgfont,
			      ostream & os) const
{
	os << "\n";
	if (orgfont.family() != family())
		os << "\\family " << LyXFamilyNames[family()] << "\n";
	if (orgfont.series() != series())
		os << "\\series " << LyXSeriesNames[series()] << "\n";
	if (orgfont.shape() != shape())
		os << "\\shape " << LyXShapeNames[shape()] << "\n";
	if (orgfont.size() != size())
		os << "\\size " << LyXSizeNames[size()] << "\n";
	if (orgfont.emph() != emph())
		os << "\\emph " << LyXMiscNames[emph()] << "\n";
	if (orgfont.number() != number())
		os << "\\numeric " << LyXMiscNames[number()] << "\n";
	if (orgfont.underbar() != underbar()) {
		// This is only for backwards compatibility
		switch (underbar()) {
		case OFF:	os << "\\bar no\n"; break;
		case ON:        os << "\\bar under\n"; break;
		case TOGGLE:	lyxerr << "Font::lyxWriteFontChanges: "
					"TOGGLE should not appear here!"
				       << endl;
		break;
		case INHERIT:   os << "\\bar default\n"; break;
		case IGNORE:    lyxerr << "Font::lyxWriteFontChanges: "
					"IGNORE should not appear here!"
				       << endl;
		break;
		}
	}
	if (orgfont.noun() != noun()) {
		os << "\\noun " << LyXMiscNames[noun()] << "\n";
	}
	if (orgfont.color() != color())
		os << "\\color " << lcolor.getLyXName(color()) << '\n';
	if (orgfont.language() != language() &&
	    language() != latex_language) {
		if (language())
			os << "\\lang " << language()->lang() << "\n";
		else
			os << "\\lang unknown\n";
	}
}


/// Writes the head of the LaTeX needed to impose this font
// Returns number of chars written.
int Font::latexWriteStartChanges(odocstream & os, BufferParams const & bparams,
				    OutputParams const & runparams,
				    Font const & base,
				    Font const & prev) const
{
	bool env = false;

	int count = 0;
	if (language()->babel() != base.language()->babel() &&
	    language() != prev.language()) {
		if (language()->lang() == "farsi") {
			os << "\\textFR{";
			count += 8;
		} else if (!isRightToLeft() &&
			    base.language()->lang() == "farsi") {
			os << "\\textLR{";
			count += 8;
		} else if (language()->lang() == "arabic_arabi") {
			os << "\\textAR{";
			count += 8;
 		} else if (!isRightToLeft() &&
				base.language()->lang() == "arabic_arabi") {
			os << "\\textLR{";
			count += 8;
		// currently the remaining RTL languages are arabic_arabtex and hebrew
		} else if (isRightToLeft() != prev.isRightToLeft()) {
			if (isRightToLeft()) {
				os << "\\R{";
				count += 3;
			} else {
				os << "\\L{";
				count += 3;
			}
		} else if (!language()->babel().empty()) {
			string const tmp =
				subst(lyxrc.language_command_local,
				      "$$lang", language()->babel());
			os << from_ascii(tmp);
			count += tmp.length();
		} else {
			os << '{';
			count += 1;
		}
	}

	if (language()->encoding()->package() == Encoding::CJK) {
		pair<bool, int> const c = switchEncoding(os, bparams,
				runparams.moving_arg, *(runparams.encoding),
				*(language()->encoding()));
		if (c.first) {
			open_encoding_ = true;
			count += c.second;
			runparams.encoding = language()->encoding();
		}
	}

	// When the current language is Hebrew, Arabic, or Farsi
	// the numbers are written Left-to-Right. ArabTeX package
	// reorders the number automatically but the packages used
	// for Hebrew and Farsi (Arabi) do not.
	if (number() == ON && prev.number() != ON
		&& (language()->lang() == "hebrew"
			|| language()->lang() == "farsi")) {
		os << "{\\beginL ";
		count += 9;
	}

	Font f = *this;
	f.reduce(base);

	if (f.family() != INHERIT_FAMILY) {
		os << '\\'
		   << LaTeXFamilyNames[f.family()]
		   << '{';
		count += strlen(LaTeXFamilyNames[f.family()]) + 2;
		env = true; //We have opened a new environment
	}
	if (f.series() != INHERIT_SERIES) {
		os << '\\'
		   << LaTeXSeriesNames[f.series()]
		   << '{';
		count += strlen(LaTeXSeriesNames[f.series()]) + 2;
		env = true; //We have opened a new environment
	}
	if (f.shape() != INHERIT_SHAPE) {
		os << '\\'
		   << LaTeXShapeNames[f.shape()]
		   << '{';
		count += strlen(LaTeXShapeNames[f.shape()]) + 2;
		env = true; //We have opened a new environment
	}
	if (f.color() != Color::inherit && f.color() != Color::ignore) {
		os << "\\textcolor{"
		   << from_ascii(lcolor.getLaTeXName(f.color()))
		   << "}{";
		count += lcolor.getLaTeXName(f.color()).length() + 13;
		env = true; //We have opened a new environment
	}
	if (f.emph() == ON) {
		os << "\\emph{";
		count += 6;
		env = true; //We have opened a new environment
	}
	if (f.underbar() == ON) {
		os << "\\underbar{";
		count += 10;
		env = true; //We have opened a new environment
	}
	// \noun{} is a LyX special macro
	if (f.noun() == ON) {
		os << "\\noun{";
		count += 6;
		env = true; //We have opened a new environment
	}
	if (f.size() != INHERIT_SIZE) {
		// If we didn't open an environment above, we open one here
		if (!env) {
			os << '{';
			++count;
		}
		os << '\\'
		   << LaTeXSizeNames[f.size()]
		   << ' ';
		count += strlen(LaTeXSizeNames[f.size()]) + 2;
	}
	return count;
}


/// Writes ending block of LaTeX needed to close use of this font
// Returns number of chars written
// This one corresponds to latexWriteStartChanges(). (Asger)
int Font::latexWriteEndChanges(odocstream & os, BufferParams const & bparams,
				  OutputParams const & runparams,
				  Font const & base,
				  Font const & next) const
{
	int count = 0;
	bool env = false;

	// reduce the current font to changes against the base
	// font (of the layout). We use a temporary for this to
	// avoid changing this font instance, as that would break
	Font f = *this;
	f.reduce(base);

	if (f.family() != INHERIT_FAMILY) {
		os << '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}
	if (f.series() != INHERIT_SERIES) {
		os << '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}
	if (f.shape() != INHERIT_SHAPE) {
		os << '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}
	if (f.color() != Color::inherit && f.color() != Color::ignore) {
		os << '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}
	if (f.emph() == ON) {
		os << '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}
	if (f.underbar() == ON) {
		os << '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}
	if (f.noun() == ON) {
		os << '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}
	if (f.size() != INHERIT_SIZE) {
		// We only have to close if only size changed
		if (!env) {
			os << '}';
			++count;
		}
	}

	// When the current language is Hebrew, Arabic, or Farsi
	// the numbers are written Left-to-Right. ArabTeX package
	// reorders the number automatically but the packages used
	// for Hebrew and Farsi (Arabi) do not.
	if (number() == ON && next.number() != ON
		&& (language()->lang() == "hebrew"
			|| language()->lang() == "farsi")) {
		os << "\\endL}";
		count += 6;
	}

	if (open_encoding_) {
		// We need to close the encoding even if it does not change
		// to do correct environment nesting
		Encoding const * const ascii = encodings.getFromLyXName("ascii");
		pair<bool, int> const c = switchEncoding(os, bparams,
				runparams.moving_arg, *(runparams.encoding),
				*ascii);
		BOOST_ASSERT(c.first);
		count += c.second;
		runparams.encoding = ascii;
		open_encoding_ = false;
	}

	if (language() != base.language() && language() != next.language()) {
		os << '}';
		++count;
	}

	return count;
}


Color_color Font::realColor() const
{
	if (color() == Color::none)
		return Color::foreground;
	return color();
}


ostream & operator<<(ostream & os, Font::FONT_MISC_STATE fms)
{
	return os << int(fms);
}


std::ostream & operator<<(std::ostream & os, Font const & font)
{
	return os << "font:"
		<< " family " << font.bits.family
		<< " series " << font.bits.series
		<< " shape " << font.bits.shape
		<< " size " << font.bits.size
		<< " color " << font.bits.color
		<< " emph " << font.bits.emph
		<< " underbar " << font.bits.underbar
		<< " noun " << font.bits.noun
		<< " number " << font.bits.number
		<< " lang: " << (font.lang ? font.lang->lang() : 0);
}


} // namespace lyx
