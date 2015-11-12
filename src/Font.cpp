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
#include "ColorSet.h"
#include "Encoding.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Lexer.h"
#include "LyXRC.h"
#include "output_latex.h"
#include "OutputParams.h"

#include "support/lassert.h"
#include "support/convert.h"
#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <cstring>

using namespace std;
using namespace lyx::support;

namespace lyx {

//
// Strings used to read and write .lyx format files
//
// These are defined in FontInfo.cpp
extern char const * LyXFamilyNames[NUM_FAMILIES + 2];
extern char const * LyXSeriesNames[NUM_SERIES + 2];
extern char const * LyXShapeNames[NUM_SHAPE + 2];
extern char const * LyXSizeNames[NUM_SIZE + 4];
extern char const * LyXMiscNames[5];

//
// Names for the GUI
//

namespace {

char const * GUIFamilyNames[NUM_FAMILIES + 2 /* default & error */] =
{ N_("Roman"), N_("Sans Serif"), N_("Typewriter"), N_("Symbol"),
  "cmr", "cmsy", "cmm", "cmex", "msa", "msb", "eufrak", "rsfs", "stmry",
  "wasy", "esint", N_("Inherit"), N_("Ignore") };

char const * GUISeriesNames[NUM_SERIES + 2 /* default & error */] =
{ N_("Medium"), N_("Bold"), N_("Inherit"), N_("Ignore") };

char const * GUIShapeNames[NUM_SHAPE + 2 /* default & error */] =
{ N_("Upright"), N_("Italic"), N_("Slanted"), N_("Smallcaps"), N_("Inherit"),
  N_("Ignore") };

char const * GUISizeNames[NUM_SIZE + 4 /* increase, decrease, default & error */] =
{ N_("Tiny"), N_("Smallest"), N_("Smaller"), N_("Small"), N_("Normal"), N_("Large"),
  N_("Larger"), N_("Largest"), N_("Huge"), N_("Huger"), N_("Increase"), N_("Decrease"),
  N_("Inherit"), N_("Ignore") };

char const * GUIMiscNames[5] =
{ N_("Off"), N_("On"), N_("Toggle"), N_("Inherit"), N_("Ignore") };

//
// Strings used to write LaTeX files
//
char const * LaTeXFamilyNames[NUM_FAMILIES + 2] =
{ "textrm", "textsf", "texttt", "error1", "error2", "error3", "error4",
  "error5", "error6", "error7", "error8", "error9", "error10", "error11",
  "error12", "error13" };

char const * LaTeXSeriesNames[NUM_SERIES + 2] =
{ "textmd", "textbf", "error4", "error5" };

char const * LaTeXShapeNames[NUM_SHAPE + 2] =
{ "textup", "textit", "textsl", "textsc", "error6", "error7" };

char const * LaTeXSizeNames[NUM_SIZE + 4] =
{ "tiny", "scriptsize", "footnotesize", "small", "normalsize", "large",
  "Large", "LARGE", "huge", "Huge", "error8", "error9", "error10", "error11" };

} // namespace anon


Font::Font(FontInfo bits, Language const * l)
	: bits_(bits), lang_(l), open_encoding_(false)
{
	if (!lang_)
		lang_ = default_language;
}


bool Font::isRightToLeft() const
{
	return lang_->rightToLeft();
}


bool Font::isVisibleRightToLeft() const
{
	return (lang_->rightToLeft() &&
		bits_.number() != FONT_ON);
}


void Font::setLanguage(Language const * l)
{
	lang_ = l;
}


/// Updates font settings according to request
void Font::update(Font const & newfont,
		     Language const * document_language,
		     bool toggleall)
{
	bits_.update(newfont.fontInfo(), toggleall);

	if (newfont.language() == language() && toggleall)
		if (language() == document_language)
			setLanguage(default_language);
		else
			setLanguage(document_language);
	else if (newfont.language() == reset_language)
		setLanguage(document_language);
	else if (newfont.language() != ignore_language)
		setLanguage(newfont.language());
}


docstring const stateText(FontInfo const & f)
{
	odocstringstream os;
	if (f.family() != INHERIT_FAMILY)
		os << _(GUIFamilyNames[f.family()]) << ", ";
	if (f.series() != INHERIT_SERIES)
		os << _(GUISeriesNames[f.series()]) << ", ";
	if (f.shape() != INHERIT_SHAPE)
		os << _(GUIShapeNames[f.shape()]) << ", ";
	if (f.size() != FONT_SIZE_INHERIT)
		os << _(GUISizeNames[f.size()]) << ", ";
	if (f.color() != Color_inherit)
		os << lcolor.getGUIName(f.color()) << ", ";
	// FIXME: uncomment this when we support background.
	//if (f.background() != Color_inherit)
	//	os << lcolor.getGUIName(f.background()) << ", ";
	if (f.emph() != FONT_INHERIT)
		os << bformat(_("Emphasis %1$s, "),
			      _(GUIMiscNames[f.emph()]));
	if (f.underbar() != FONT_INHERIT)
		os << bformat(_("Underline %1$s, "),
			      _(GUIMiscNames[f.underbar()]));
	if (f.strikeout() != FONT_INHERIT)
		os << bformat(_("Strikeout %1$s, "),
			      _(GUIMiscNames[f.strikeout()]));
	if (f.uuline() != FONT_INHERIT)
		os << bformat(_("Double underline %1$s, "),
			      _(GUIMiscNames[f.uuline()]));
	if (f.uwave() != FONT_INHERIT)
		os << bformat(_("Wavy underline %1$s, "),
			      _(GUIMiscNames[f.uwave()]));
	if (f.noun() != FONT_INHERIT)
		os << bformat(_("Noun %1$s, "),
			      _(GUIMiscNames[f.noun()]));
	if (f == inherit_font)
		os << _("Default") << ", ";

	return os.str();
}


docstring const Font::stateText(BufferParams * params) const
{
	odocstringstream os;
	os << lyx::stateText(bits_);
	if (!params || (language() != params->language))
		os << bformat(_("Language: %1$s, "),
			      _(language()->display()));
	if (bits_.number() != FONT_OFF)
		os << "  " << bformat(_("Number %1$s"),
			      _(GUIMiscNames[bits_.number()]));
	return rtrim(os.str(), ", ");
}


// Returns size in latex format
string const Font::latexSize() const
{
	return LaTeXSizeNames[bits_.size()];
}


/// Writes the changes from this font to orgfont in .lyx format in file
void Font::lyxWriteChanges(Font const & orgfont,
			      ostream & os) const
{
	os << "\n";
	if (orgfont.fontInfo().family() != bits_.family())
		os << "\\family " << LyXFamilyNames[bits_.family()] << "\n";
	if (orgfont.fontInfo().series() != bits_.series())
		os << "\\series " << LyXSeriesNames[bits_.series()] << "\n";
	if (orgfont.fontInfo().shape() != bits_.shape())
		os << "\\shape " << LyXShapeNames[bits_.shape()] << "\n";
	if (orgfont.fontInfo().size() != bits_.size())
		os << "\\size " << LyXSizeNames[bits_.size()] << "\n";
	if (orgfont.fontInfo().emph() != bits_.emph())
		os << "\\emph " << LyXMiscNames[bits_.emph()] << "\n";
	if (orgfont.fontInfo().number() != bits_.number())
		os << "\\numeric " << LyXMiscNames[bits_.number()] << "\n";
	if (orgfont.fontInfo().underbar() != bits_.underbar()) {
		// This is only for backwards compatibility
		switch (bits_.underbar()) {
		case FONT_OFF:	os << "\\bar no\n"; break;
		case FONT_ON:        os << "\\bar under\n"; break;
		case FONT_TOGGLE:	lyxerr << "Font::lyxWriteFontChanges: "
					"FONT_TOGGLE should not appear here!"
				       << endl;
		break;
		case FONT_INHERIT:   os << "\\bar default\n"; break;
		case FONT_IGNORE:    lyxerr << "Font::lyxWriteFontChanges: "
					"IGNORE should not appear here!"
				       << endl;
		break;
		}
	}
	if (orgfont.fontInfo().strikeout() != bits_.strikeout()) {
		os << "\\strikeout " << LyXMiscNames[bits_.strikeout()] << "\n";
	}
	if (orgfont.fontInfo().uuline() != bits_.uuline()) {
		os << "\\uuline " << LyXMiscNames[bits_.uuline()] << "\n";
	}
	if (orgfont.fontInfo().uwave() != bits_.uwave()) {
		os << "\\uwave " << LyXMiscNames[bits_.uwave()] << "\n";
	}
	if (orgfont.fontInfo().noun() != bits_.noun()) {
		os << "\\noun " << LyXMiscNames[bits_.noun()] << "\n";
	}
	if (orgfont.fontInfo().color() != bits_.color())
		os << "\\color " << lcolor.getLyXName(bits_.color()) << '\n';
	// FIXME: uncomment this when we support background.
	//if (orgfont.fontInfo().background() != bits_.background())
	//	os << "\\color " << lcolor.getLyXName(bits_.background()) << '\n';
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

	// polyglossia or babel?
	if (runparams.use_polyglossia
	    && language()->lang() != base.language()->lang()
	    && language() != prev.language()) {
		if (!language()->polyglossia().empty()) {
			string tmp = "\\text" + language()->polyglossia();
			if (!language()->polyglossiaOpts().empty())
				tmp += "[" + language()->polyglossiaOpts() + "]";
			tmp += "{";
			os << from_ascii(tmp);
			count += tmp.length();
		} else if (language()->encoding()->package() != Encoding::CJK) {
			os << '{';
			count += 1;
		}
	} else if (language()->babel() != base.language()->babel() &&
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
		} else if (language()->encoding()->package() != Encoding::CJK) {
			os << '{';
			count += 1;
		}
	}

	// FIXME: skip this for XeTeX/LuaTeX? With nonTeXfonts or always?
	if (language()->encoding()->package() == Encoding::CJK) {
		pair<bool, int> const c = switchEncoding(os, bparams,
				runparams, *(language()->encoding()));
		if (c.first) {
			open_encoding_ = true;
			count += c.second;
			runparams.encoding = language()->encoding();
		}
	}

	// If the current language is Hebrew, Arabic, or Farsi
	// the numbers are written Left-to-Right. ArabTeX package
	// reorders the number automatically but the packages used
	// for Hebrew and Farsi (Arabi) do not.
	if (bits_.number() == FONT_ON && prev.fontInfo().number() != FONT_ON
		&& (language()->lang() == "hebrew"
			|| language()->lang() == "farsi" 
			|| language()->lang() == "arabic_arabi")) {
		os << "{\\beginL ";
		count += 9;
	}

	FontInfo f = bits_;
	f.reduce(base.bits_);

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
	if (f.color() != Color_inherit && f.color() != Color_ignore) {
		os << "\\textcolor{"
		   << from_ascii(lcolor.getLaTeXName(f.color()))
		   << "}{";
		count += lcolor.getLaTeXName(f.color()).length() + 13;
		env = true; //We have opened a new environment
	}
	// FIXME: uncomment this when we support background.
	/*
	if (f.background() != Color_inherit && f.background() != Color_ignore) {
		os << "\\textcolor{"
		   << from_ascii(lcolor.getLaTeXName(f.background()))
		   << "}{";
		count += lcolor.getLaTeXName(f.background()).length() + 13;
		env = true; //We have opened a new environment
	}
	*/
	if (f.emph() == FONT_ON) {
		os << "\\emph{";
		count += 6;
		env = true; //We have opened a new environment
	}
	// \noun{} is a LyX special macro
	if (f.noun() == FONT_ON) {
		os << "\\noun{";
		count += 6;
		env = true; //We have opened a new environment
	}
	if (f.size() != FONT_SIZE_INHERIT) {
		// If we didn't open an environment above, we open one here
		if (!env) {
			os << '{';
			++count;
		}
		os << '\\'
		   << LaTeXSizeNames[f.size()]
		   << "{}";
		count += strlen(LaTeXSizeNames[f.size()]) + 3;
	}
	// The ulem commands need to be on the deepest nesting level
	// because ulem puts every nested group or macro in a box,
	// which prevents linebreaks (#8424, #8733)
	if (f.underbar() == FONT_ON) {
		os << "\\uline{";
		count += 10;
		++runparams.inulemcmd;
	}
	if (f.strikeout() == FONT_ON) {
		os << "\\sout{";
		count += 9;
		++runparams.inulemcmd;
	}
	if (f.uuline() == FONT_ON) {
		os << "\\uuline{";
		count += 11;
		++runparams.inulemcmd;
	}
	if (f.uwave() == FONT_ON) {
		os << "\\uwave{";
		count += 10;
		++runparams.inulemcmd;
	}
	return count;
}


/// Writes ending block of LaTeX needed to close use of this font
// Returns number of chars written
// This one corresponds to latexWriteStartChanges(). (Asger)
int Font::latexWriteEndChanges(otexstream & os, BufferParams const & bparams,
				  OutputParams const & runparams,
				  Font const & base,
				  Font const & next,
				  bool const & closeLanguage) const
{
	int count = 0;
	bool env = false;

	// reduce the current font to changes against the base
	// font (of the layout). We use a temporary for this to
	// avoid changing this font instance, as that would break
	FontInfo f = bits_;
	f.reduce(base.bits_);

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
	if (f.color() != Color_inherit && f.color() != Color_ignore) {
		os << '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}
	if (f.emph() == FONT_ON) {
		os << '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}
	if (f.noun() == FONT_ON) {
		os << '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}
	if (f.size() != FONT_SIZE_INHERIT) {
		// We only have to close if only size changed
		if (!env) {
			os << '}';
			++count;
		}
	}
	if (f.underbar() == FONT_ON) {
		os << '}';
		++count;
		--runparams.inulemcmd;
	}
	if (f.strikeout() == FONT_ON) {
		os << '}';
		++count;
		--runparams.inulemcmd;
	}
	if (f.uuline() == FONT_ON) {
		os << '}';
		++count;
		--runparams.inulemcmd;
	}
	if (f.uwave() == FONT_ON) {
		os << '}';
		++count;
		--runparams.inulemcmd;
	}

	// If the current language is Hebrew, Arabic, or Farsi
	// the numbers are written Left-to-Right. ArabTeX package
	// reorders the number automatically but the packages used
	// for Hebrew and Farsi (Arabi) do not.
	if (bits_.number() == FONT_ON && next.fontInfo().number() != FONT_ON
		&& (language()->lang() == "hebrew"
			|| language()->lang() == "farsi"
			|| language()->lang() == "arabic_arabi")) {
		os << "\\endL}";
		count += 6;
	}

	if (open_encoding_) {
		// We need to close the encoding even if it does not change
		// to do correct environment nesting
		Encoding const * const ascii = encodings.fromLyXName("ascii");
		pair<bool, int> const c = switchEncoding(os.os(), bparams,
				runparams, *ascii);
		LATTEST(c.first);
		count += c.second;
		runparams.encoding = ascii;
		open_encoding_ = false;
	}

	if (closeLanguage
	    && language() != base.language() && language() != next.language()
	    && language()->encoding()->package() != Encoding::CJK) {
		os << '}';
		++count;
	}

	return count;
}


string Font::toString(bool const toggle) const
{
	string const lang = (language() == reset_language)
		? "reset" : language()->lang();

	ostringstream os;
	os << "family " << bits_.family() << '\n'
	   << "series " << bits_.series() << '\n'
	   << "shape " << bits_.shape() << '\n'
	   << "size " << bits_.size() << '\n'
	   << "emph " << bits_.emph() << '\n'
	   << "underbar " << bits_.underbar() << '\n'
	   << "strikeout " << bits_.strikeout() << '\n'
	   << "uuline " << bits_.uuline() << '\n'
	   << "uwave " << bits_.uwave() << '\n'
	   << "noun " << bits_.noun() << '\n'
	   << "number " << bits_.number() << '\n'
	   << "color " << bits_.color() << '\n'
	   << "language " << lang << '\n'
	   << "toggleall " << convert<string>(toggle);
	return os.str();
}


bool Font::fromString(string const & data, bool & toggle)
{
	istringstream is(data);
	Lexer lex;
	lex.setStream(is);

	int nset = 0;
	while (lex.isOK()) {
		string token;
		if (lex.next())
			token = lex.getString();

		if (token.empty() || !lex.next())
			break;

		if (token == "family") {
			int const next = lex.getInteger();
			bits_.setFamily(FontFamily(next));

		} else if (token == "series") {
			int const next = lex.getInteger();
			bits_.setSeries(FontSeries(next));

		} else if (token == "shape") {
			int const next = lex.getInteger();
			bits_.setShape(FontShape(next));

		} else if (token == "size") {
			int const next = lex.getInteger();
			bits_.setSize(FontSize(next));

		} else if (token == "emph" || token == "underbar" ||
			   token == "noun" || token == "number" ||
			   token == "uuline" || token == "uwave" ||
			   token == "strikeout") {

			int const next = lex.getInteger();
			FontState const misc = FontState(next);

			if (token == "emph")
				bits_.setEmph(misc);
			else if (token == "underbar")
				bits_.setUnderbar(misc);
			else if (token == "strikeout")
				bits_.setStrikeout(misc);
			else if (token == "uuline")
				bits_.setUuline(misc);
			else if (token == "uwave")
				bits_.setUwave(misc);
			else if (token == "noun")
				bits_.setNoun(misc);
			else if (token == "number")
				bits_.setNumber(misc);

		} else if (token == "color") {
			int const next = lex.getInteger();
			bits_.setColor(ColorCode(next));

		/**
		} else if (token == "background") {
			int const next = lex.getInteger();
			bits_.setBackground(ColorCode(next));
		*/

		} else if (token == "language") {
			string const next = lex.getString();
			setLanguage(languages.getLanguage(next));

		} else if (token == "toggleall") {
			toggle = lex.getBool();

		} else {
			// Unrecognised token
			break;
		}

		++nset;
	}
	return (nset > 0);
}


void Font::validate(LaTeXFeatures & features) const
{
	BufferParams const & bparams = features.bufferParams();
	Language const * doc_language = bparams.language;

	if (bits_.noun() == FONT_ON) {
		LYXERR(Debug::LATEX, "font.noun: " << bits_.noun());
		features.require("noun");
		LYXERR(Debug::LATEX, "Noun enabled. Font: " << to_utf8(stateText(0)));
	}
	if (bits_.underbar() == FONT_ON) {
		LYXERR(Debug::LATEX, "font.underline: " << bits_.underbar());
		features.require("ulem");
		LYXERR(Debug::LATEX, "Underline enabled. Font: " << to_utf8(stateText(0)));
	}
	if (bits_.strikeout() == FONT_ON) {
		LYXERR(Debug::LATEX, "font.strikeout: " << bits_.strikeout());
		features.require("ulem");
		LYXERR(Debug::LATEX, "Strikeout enabled. Font: " << to_utf8(stateText(0)));
	}
	if (bits_.uuline() == FONT_ON) {
		LYXERR(Debug::LATEX, "font.uuline: " << bits_.uuline());
		features.require("ulem");
		LYXERR(Debug::LATEX, "Double underline enabled. Font: " << to_utf8(stateText(0)));
	}
	if (bits_.uwave() == FONT_ON) {
		LYXERR(Debug::LATEX, "font.uwave: " << bits_.uwave());
		features.require("ulem");
		LYXERR(Debug::LATEX, "Wavy underline enabled. Font: " << to_utf8(stateText(0)));
	}
	switch (bits_.color()) {
		case Color_none:
		case Color_inherit:
		case Color_ignore:
			// probably we should put here all interface colors used for
			// font displaying! For now I just add this ones I know of (Jug)
		case Color_latex:
		case Color_notelabel:
			break;
		case Color_brown:
		case Color_darkgray:
		case Color_gray:
		case Color_lightgray:
		case Color_lime:
		case Color_olive:
		case Color_orange:
		case Color_pink:
		case Color_purple:
		case Color_teal:
		case Color_violet:
			features.require("xcolor");
			break;
		default:
			features.require("color");
			LYXERR(Debug::LATEX, "Color enabled. Font: " << to_utf8(stateText(0)));
	}

	// FIXME: Do something for background and soul package?

	if (((features.usePolyglossia() && lang_->polyglossia() != doc_language->polyglossia())
	     || (features.useBabel() && lang_->babel() != doc_language->babel())
	     || (doc_language->encoding()->package() == Encoding::CJK && lang_ != doc_language))
	    && lang_ != ignore_language
	    && lang_ != latex_language)
	{
		features.useLanguage(lang_);
		LYXERR(Debug::LATEX, "Found language " << lang_->lang());
	}
}


ostream & operator<<(ostream & os, FontState fms)
{
	return os << int(fms);
}


ostream & operator<<(ostream & os, FontInfo const & f)
{
	return os << "font:"
		<< " family " << f.family()
		<< " series " << f.series()
		<< " shape " << f.shape()
		<< " size " << f.size()
		<< " color " << f.color()
		// FIXME: uncomment this when we support background.
		//<< " background " << f.background()
		<< " emph " << f.emph()
		<< " underbar " << f.underbar()
		<< " strikeout " << f.strikeout()
		<< " uuline " << f.uuline()
		<< " uwave " << f.uwave()
		<< " noun " << f.noun()
		<< " number " << f.number();
}


ostream & operator<<(ostream & os, Font const & font)
{
	return os << font.bits_
		<< " lang: " << (font.lang_ ? font.lang_->lang() : 0);
}


} // namespace lyx
