/**
 * \file src/FontInfo.cpp
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

#include "FontInfo.h"

#include "ColorSet.h"
#include "Lexer.h"
#include "LyXRC.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstring.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/RefChanger.h"

#include <algorithm>
#include <ostream>
#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

//
// Names for the GUI
//

char const * GUIFamilyNames[NUM_FAMILIES + 2 /* default & error */] =
{ N_("Roman"), N_("Sans Serif"), N_("Typewriter"), N_("Symbol"),
  "cmr", "cmsy", "cmm", "cmex", "msa", "msb", "ds", "eufrak", "rsfs", "stmry",
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
// Strings used to read and write .lyx format files
//
char const * LyXFamilyNames[NUM_FAMILIES + 2 /* default & error */] =
{ "roman", "sans", "typewriter", "symbol",
  "cmr", "cmsy", "cmm", "cmex", "msa", "msb", "ds", "eufrak", "rsfs", "stmry",
  "wasy", "esint", "default", "error" };

char const * LyXSeriesNames[NUM_SERIES + 2 /* default & error */] =
{ "medium", "bold", "default", "error" };

char const * LyXShapeNames[NUM_SHAPE + 2 /* default & error */] =
{ "up", "italic", "slanted", "smallcaps", "default", "error" };

char const * LyXSizeNames[NUM_SIZE + 4 /* increase, decrease, default & error */] =
{ "tiny", "scriptsize", "footnotesize", "small", "normal", "large",
  "larger", "largest", "huge", "giant",
  "increase", "decrease", "default", "error" };

char const * LyXMiscNames[5] =
{ "off", "on", "toggle", "default", "error" };


FontInfo const sane_font(
	ROMAN_FAMILY,
	MEDIUM_SERIES,
	UP_SHAPE,
	NORMAL_SIZE,
	TEXT_STYLE,
	Color_none,
	Color_background,
	FONT_OFF,
	FONT_OFF,
	FONT_OFF,
	FONT_OFF,
	FONT_OFF,
	FONT_OFF,
	FONT_OFF,
	FONT_OFF,
	FONT_OFF);

FontInfo const inherit_font(
	INHERIT_FAMILY,
	INHERIT_SERIES,
	INHERIT_SHAPE,
	INHERIT_SIZE,
	INHERIT_STYLE,
	Color_inherit,
	Color_inherit,
	FONT_INHERIT,
	FONT_INHERIT,
	FONT_INHERIT,
	FONT_INHERIT,
	FONT_INHERIT,
	FONT_INHERIT,
	FONT_INHERIT,
	FONT_OFF,
	FONT_INHERIT);

FontInfo const ignore_font(
	IGNORE_FAMILY,
	IGNORE_SERIES,
	IGNORE_SHAPE,
	IGNORE_SIZE,
	IGNORE_STYLE,
	Color_ignore,
	Color_ignore,
	FONT_IGNORE,
	FONT_IGNORE,
	FONT_IGNORE,
	FONT_IGNORE,
	FONT_IGNORE,
	FONT_IGNORE,
	FONT_IGNORE,
	FONT_IGNORE,
	FONT_IGNORE);


FontInfo::FontInfo()
{
	*this = sane_font;
}


/// Decreases font size_ by one
FontInfo & FontInfo::decSize()
{
	switch (size_) {
	case HUGER_SIZE:        size_ = HUGE_SIZE;     break;
	case HUGE_SIZE:         size_ = LARGEST_SIZE;  break;
	case LARGEST_SIZE:      size_ = LARGER_SIZE;   break;
	case LARGER_SIZE:       size_ = LARGE_SIZE;    break;
	case LARGE_SIZE:        size_ = NORMAL_SIZE;   break;
	case NORMAL_SIZE:       size_ = SMALL_SIZE;    break;
	case SMALL_SIZE:        size_ = FOOTNOTE_SIZE; break;
	case FOOTNOTE_SIZE:     size_ = SCRIPT_SIZE;   break;
	case SCRIPT_SIZE:       size_ = TINY_SIZE;     break;
	case TINY_SIZE:         break;
	case INCREASE_SIZE:
		LYXERR0("Can't FontInfo::decSize on INCREASE_SIZE");
		break;
	case DECREASE_SIZE:
		LYXERR0("Can't FontInfo::decSize on DECREASE_SIZE");
		break;
	case INHERIT_SIZE:
		LYXERR0("Can't FontInfo::decSize on INHERIT_SIZE");
		break;
	case IGNORE_SIZE:
		LYXERR0("Can't FontInfo::decSize on IGNORE_SIZE");
		break;
	}
	return *this;
}


/// Increases font size_ by one
FontInfo & FontInfo::incSize()
{
	switch (size_) {
	case HUGER_SIZE:	break;
	case HUGE_SIZE:         size_ = HUGER_SIZE;    break;
	case LARGEST_SIZE:      size_ = HUGE_SIZE;     break;
	case LARGER_SIZE:       size_ = LARGEST_SIZE;  break;
	case LARGE_SIZE:        size_ = LARGER_SIZE;   break;
	case NORMAL_SIZE:       size_ = LARGE_SIZE;    break;
	case SMALL_SIZE:        size_ = NORMAL_SIZE;   break;
	case FOOTNOTE_SIZE:     size_ = SMALL_SIZE;    break;
	case SCRIPT_SIZE:       size_ = FOOTNOTE_SIZE; break;
	case TINY_SIZE:         size_ = SCRIPT_SIZE;   break;
	case INCREASE_SIZE:
		LYXERR0("Can't FontInfo::incSize on INCREASE_SIZE");
		break;
	case DECREASE_SIZE:
		LYXERR0("Can't FontInfo::incSize on DECREASE_SIZE");
		break;
	case INHERIT_SIZE:
		LYXERR0("Can't FontInfo::incSize on INHERIT_SIZE");
		break;
	case IGNORE_SIZE:
		LYXERR0("Can't FontInfo::incSize on IGNORE_SIZE");
		break;
	}
	return *this;
}


double FontInfo::realSize() const
{
	double d = convert<double>(lyxrc.font_sizes[size()]);
	// The following is according to the average of the values in the
	// definitions of \defaultscriptratio and \defaultscriptscriptratio in LaTeX
	// font packages. No attempt is made to implement the actual values from
	// \DefineMathSizes.
	switch (style()) {
	case DISPLAY_STYLE:
	case TEXT_STYLE:
	case INHERIT_STYLE:
	case IGNORE_STYLE:
		break;
	case SCRIPT_STYLE:
		d *= .73;
		break;
	case SCRIPTSCRIPT_STYLE:
		d *= .55;
		break;
	}
	// Never go below the smallest size
	return max(d, convert<double>(lyxrc.font_sizes[TINY_SIZE]));
}


/// Reduce font to fall back to template where possible
void FontInfo::reduce(FontInfo const & tmplt)
{
	if (family_ == tmplt.family_)
		family_ = INHERIT_FAMILY;
	if (series_ == tmplt.series_)
		series_ = INHERIT_SERIES;
	if (shape_ == tmplt.shape_)
		shape_ = INHERIT_SHAPE;
	if (size_ == tmplt.size_)
		size_ = INHERIT_SIZE;
	if (style_ == tmplt.style_)
		style_ = INHERIT_STYLE;
	if (emph_ == tmplt.emph_)
		emph_ = FONT_INHERIT;
	if (underbar_ == tmplt.underbar_)
		underbar_ = FONT_INHERIT;
	if (strikeout_ == tmplt.strikeout_)
		strikeout_ = FONT_INHERIT;
	if (xout_ == tmplt.xout_)
		xout_ = FONT_INHERIT;
	if (uuline_ == tmplt.uuline_)
		uuline_ = FONT_INHERIT;
	if (uwave_ == tmplt.uwave_)
		uwave_ = FONT_INHERIT;
	if (noun_ == tmplt.noun_)
		noun_ = FONT_INHERIT;
	if (color_ == tmplt.color_)
		color_ = Color_inherit;
	if (background_ == tmplt.background_)
		background_ = Color_inherit;
	if (nospellcheck_ == tmplt.nospellcheck_)
		nospellcheck_ = FONT_INHERIT;
}


/// Realize font from a template
FontInfo & FontInfo::realize(FontInfo const & tmplt)
{
	if ((*this) == inherit_font) {
		operator=(tmplt);
		return *this;
	}

	if (family_ == INHERIT_FAMILY)
		family_ = tmplt.family_;

	if (series_ == INHERIT_SERIES)
		series_ = tmplt.series_;

	if (shape_ == INHERIT_SHAPE)
		shape_ = tmplt.shape_;

	if (size_ == INHERIT_SIZE)
		size_ = tmplt.size_;

	if (style_ == INHERIT_STYLE)
		style_ = tmplt.style_;

	if (emph_ == FONT_INHERIT)
		emph_ = tmplt.emph_;

	if (underbar_ == FONT_INHERIT)
		underbar_ = tmplt.underbar_;

	if (strikeout_ == FONT_INHERIT)
		strikeout_ = tmplt.strikeout_;

	if (xout_ == FONT_INHERIT)
		xout_ = tmplt.xout_;

	if (uuline_ == FONT_INHERIT)
		uuline_ = tmplt.uuline_;

	if (uwave_ == FONT_INHERIT)
		uwave_ = tmplt.uwave_;

	if (noun_ == FONT_INHERIT)
		noun_ = tmplt.noun_;

	if (color_ == Color_inherit)
		color_ = tmplt.color_;

	if (background_ == Color_inherit)
		background_ = tmplt.background_;

	if (nospellcheck_ == FONT_INHERIT)
		nospellcheck_ = tmplt.nospellcheck_;

	return *this;
}


Changer FontInfo::changeColor(ColorCode const color)
{
	return make_change(color_, color);
}


Changer FontInfo::changeShape(FontShape const shape)
{
	return make_change(shape_, shape);
}


Changer FontInfo::changeStyle(MathStyle const new_style)
{
	return make_change(style_, new_style);
}


Changer FontInfo::change(FontInfo font, bool realize)
{
	if (realize)
		font.realize(*this);
	return make_change(*this, font);
}


/// Updates a misc setting according to request
static FontState setMisc(FontState newfont,
	FontState org)
{
	if (newfont == FONT_TOGGLE) {
		if (org == FONT_ON)
			return FONT_OFF;
		else if (org == FONT_OFF)
			return FONT_ON;
		else {
			LYXERR0("Font::setMisc: Need state"
				" FONT_ON or FONT_OFF to toggle. Setting to FONT_ON");
			return FONT_ON;
		}
	} else if (newfont == FONT_IGNORE)
		return org;
	else
		return newfont;
}

/// Updates font settings according to request
void FontInfo::update(FontInfo const & newfont, bool toggleall)
{
	if (newfont.family_ == family_ && toggleall)
		setFamily(INHERIT_FAMILY); // toggle 'back'
	else if (newfont.family_ != IGNORE_FAMILY)
		setFamily(newfont.family_);
	// else it's IGNORE_SHAPE

	// "Old" behaviour: "Setting" bold will toggle bold on/off.
	switch (newfont.series_) {
	case BOLD_SERIES:
		// We toggle...
		if (series_ == BOLD_SERIES && toggleall)
			setSeries(MEDIUM_SERIES);
		else
			setSeries(BOLD_SERIES);
		break;
	case MEDIUM_SERIES:
	case INHERIT_SERIES:
		setSeries(newfont.series_);
		break;
	case IGNORE_SERIES:
		break;
	}

	if (newfont.shape_ == shape_ && toggleall)
		shape_ = INHERIT_SHAPE; // toggle 'back'
	else if (newfont.shape_ != IGNORE_SHAPE)
		shape_ = newfont.shape_;
	// else it's IGNORE_SHAPE

	if (newfont.size_ != IGNORE_SIZE) {
		if (newfont.size_ == INCREASE_SIZE)
			incSize();
		else if (newfont.size_ == DECREASE_SIZE)
			decSize();
		else
			size_ = newfont.size_;
	}

	if (newfont.style_ != IGNORE_STYLE) {
			style_ = newfont.style_;
	}

	setEmph(setMisc(newfont.emph_, emph_));
	setUnderbar(setMisc(newfont.underbar_, underbar_));
	setStrikeout(setMisc(newfont.strikeout_, strikeout_));
	setXout(setMisc(newfont.xout_, xout_));
	setUuline(setMisc(newfont.uuline_, uuline_));
	setUwave(setMisc(newfont.uwave_, uwave_));
	setNoun(setMisc(newfont.noun_, noun_));
	setNumber(setMisc(newfont.number_, number_));
	setNoSpellcheck(setMisc(newfont.nospellcheck_, nospellcheck_));

	if (newfont.color_ == color_ && toggleall)
		setColor(Color_inherit); // toggle 'back'
	else if (newfont.color_ != Color_ignore)
		setColor(newfont.color_);

	if (newfont.background_ == background_ && toggleall)
		setBackground(Color_inherit); // toggle 'back'
	else if (newfont.background_ != Color_ignore)
		setBackground(newfont.background_);
}

/// Is font resolved?
bool FontInfo::resolved() const
{
	return (family_ != INHERIT_FAMILY && series_ != INHERIT_SERIES
		&& shape_ != INHERIT_SHAPE && size_ != INHERIT_SIZE
		&& style_ != INHERIT_STYLE
		&& emph_ != FONT_INHERIT && underbar_ != FONT_INHERIT
		&& uuline_ != FONT_INHERIT && uwave_ != FONT_INHERIT
		&& strikeout_ != FONT_INHERIT && xout_ != FONT_INHERIT
		&& noun_ != FONT_INHERIT && color_ != Color_inherit
		&& background_ != Color_inherit && nospellcheck_ != FONT_INHERIT);
}


Color FontInfo::realColor() const
{
	if (paint_color_ != Color_none)
		return paint_color_;
	if (color_ == Color_none)
		return Color_foreground;
	return color_;
}


namespace {

void appendSep(string & s1, string const & s2)
{
	if (s2.empty())
		return;
	s1 += s1.empty() ? "" : "\n";
	s1 += s2;
}


string makeCSSTag(string const & key, string const & val)
{
	return key + ": " + val + ";";
}


string getFamilyCSS(FontFamily const & f)
{
	switch (f) {
	case ROMAN_FAMILY:
		return "serif";
	case SANS_FAMILY:
		return "sans-serif";
	case TYPEWRITER_FAMILY:
		return "monospace";
	case SYMBOL_FAMILY:
	case CMR_FAMILY:
	case CMSY_FAMILY:
	case CMM_FAMILY:
	case CMEX_FAMILY:
	case MSA_FAMILY:
	case MSB_FAMILY:
	case DS_FAMILY:
	case EUFRAK_FAMILY:
	case RSFS_FAMILY:
	case STMARY_FAMILY:
	case WASY_FAMILY:
	case ESINT_FAMILY:
	case INHERIT_FAMILY:
	case IGNORE_FAMILY:
		break;
	}
	return "";
}


string getSeriesCSS(FontSeries const & s)
{
	switch (s) {
	case MEDIUM_SERIES:
		return "normal";
	case BOLD_SERIES:
		return "bold";
	case INHERIT_SERIES:
	case IGNORE_SERIES:
		break;
	}
	return "";
}


string getShapeCSS(FontShape const & s)
{
	string fs = "normal";
	string fv = "normal";
	switch (s) {
	case UP_SHAPE: break;
	case ITALIC_SHAPE: fs = "italic"; break;
	case SLANTED_SHAPE: fs = "oblique"; break;
	case SMALLCAPS_SHAPE: fv = "small-caps"; break;
	case IGNORE_SHAPE:
	case INHERIT_SHAPE:
		fs = ""; fv = ""; break;
	}
	string retval;
	if (!fs.empty())
		appendSep(retval, makeCSSTag("font-style", fs));
	if (!fv.empty())
		appendSep(retval, makeCSSTag("font-variant", fv));
	return retval;
}


string getSizeCSS(FontSize const & s)
{
	switch (s) {
	case TINY_SIZE:
		return "xx-small";
	case SCRIPT_SIZE:
		return "x-small";
	case FOOTNOTE_SIZE:
	case SMALL_SIZE:
		return "small";
	case NORMAL_SIZE:
		return "medium";
	case LARGE_SIZE:
		return "large";
	case LARGER_SIZE:
	case LARGEST_SIZE:
		return "x-large";
	case HUGE_SIZE:
	case HUGER_SIZE:
		return "xx-large";
	case INCREASE_SIZE:
		return "larger";
	case DECREASE_SIZE:
		return "smaller";
	case IGNORE_SIZE:
	case INHERIT_SIZE:
		break;
	}
	return "";
}

} // namespace


// FIXME This does not yet handle color
docstring FontInfo::asCSS() const
{
	string retval;
	string tmp = getFamilyCSS(family_);
	if (!tmp.empty())
		appendSep(retval, makeCSSTag("font-family", tmp));
	tmp = getSeriesCSS(series_);
	if (!tmp.empty())
		appendSep(retval, makeCSSTag("font-weight", tmp));
	appendSep(retval, getShapeCSS(shape_));
	tmp = getSizeCSS(size_);
	if (!tmp.empty())
		appendSep(retval, makeCSSTag("font-size", tmp));
	return from_ascii(retval);
}


docstring const FontInfo::stateText(bool const terse) const
{
	odocstringstream os;
	if (family() != INHERIT_FAMILY && (!terse || family() != IGNORE_FAMILY))
		os << _(GUIFamilyNames[family()]) << ", ";
	if (series() != INHERIT_SERIES && (!terse || series() != IGNORE_SERIES))
		os << _(GUISeriesNames[series()]) << ", ";
	if (shape() != INHERIT_SHAPE && (!terse || shape() != IGNORE_SHAPE))
		os << _(GUIShapeNames[shape()]) << ", ";
	if (size() != INHERIT_SIZE && (!terse || size() != IGNORE_SIZE))
		os << _(GUISizeNames[size()]) << ", ";
	// FIXME: shall style be handled there? Probably not.
	if (color() != Color_inherit && (!terse || color() != Color_ignore))
		os << lcolor.getGUIName(color()) << ", ";
	// FIXME: uncomment this when we support background.
	//if (background() != Color_inherit)
	//	os << lcolor.getGUIName(background()) << ", ";
	if (emph() != FONT_INHERIT && (!terse || emph() != FONT_IGNORE))
		os << bformat(_("Emphasis %1$s, "),
			      _(GUIMiscNames[emph()]));
	if (underbar() != FONT_INHERIT && (!terse || underbar() == FONT_ON))
		os << bformat(_("Underline %1$s, "),
			      _(GUIMiscNames[underbar()]));
	if (uuline() != FONT_INHERIT && (!terse || uuline() == FONT_ON))
		os << bformat(_("Double underline %1$s, "),
			      _(GUIMiscNames[uuline()]));
	if (uwave() != FONT_INHERIT && (!terse || uwave() == FONT_ON))
		os << bformat(_("Wavy underline %1$s, "),
			      _(GUIMiscNames[uwave()]));
	if (strikeout() != FONT_INHERIT && (!terse || strikeout() == FONT_ON))
		os << bformat(_("Strike out %1$s, "),
			      _(GUIMiscNames[strikeout()]));
	if (xout() != FONT_INHERIT && (!terse || xout() == FONT_ON))
		os << bformat(_("Cross out %1$s, "),
			      _(GUIMiscNames[xout()]));
	if (noun() != FONT_INHERIT && (!terse || noun() != FONT_IGNORE))
		os << bformat(_("Noun %1$s, "),
			      _(GUIMiscNames[noun()]));
	if (*this == inherit_font)
		os << _("Default") << ", ";

	return os.str();
}


// Set family according to lyx format string
void setLyXFamily(string const & fam, FontInfo & f)
{
	string const s = ascii_lowercase(fam);

	int i = 0;
	while (LyXFamilyNames[i] != s &&
	       LyXFamilyNames[i] != string("error"))
		++i;
	if (s == LyXFamilyNames[i])
		f.setFamily(FontFamily(i));
	else
		LYXERR0("Unknown family `" << s << '\'');
}


// Set series according to lyx format string
void setLyXSeries(string const & ser, FontInfo & f)
{
	string const s = ascii_lowercase(ser);

	int i = 0;
	while (LyXSeriesNames[i] != s &&
	       LyXSeriesNames[i] != string("error")) ++i;
	if (s == LyXSeriesNames[i]) {
		f.setSeries(FontSeries(i));
	} else
		LYXERR0("Unknown series `" << s << '\'');
}


// Set shape according to lyx format string
void setLyXShape(string const & sha, FontInfo & f)
{
	string const s = ascii_lowercase(sha);

	int i = 0;
	while (LyXShapeNames[i] != s && LyXShapeNames[i] != string("error"))
			++i;
	if (s == LyXShapeNames[i])
		f.setShape(FontShape(i));
	else
		LYXERR0("Unknown shape `" << s << '\'');
}


// Set size according to lyx format string
void setLyXSize(string const & siz, FontInfo & f)
{
	string const s = ascii_lowercase(siz);
	int i = 0;
	while (LyXSizeNames[i] != s && LyXSizeNames[i] != string("error"))
		++i;
	if (s == LyXSizeNames[i]) {
		f.setSize(FontSize(i));
	} else
		LYXERR0("Unknown size `" << s << '\'');
}


// Set size according to lyx format string
FontState setLyXMisc(string const & siz)
{
	string const s = ascii_lowercase(siz);
	int i = 0;
	while (LyXMiscNames[i] != s &&
	       LyXMiscNames[i] != string("error")) ++i;
	if (s == LyXMiscNames[i])
		return FontState(i);
	LYXERR0("Unknown misc flag `" << s << '\'');
	return FONT_OFF;
}


/// Sets color after LyX text format
void setLyXColor(string const & col, FontInfo & f)
{
	f.setColor(lcolor.getFromLyXName(col));
}


// Read a font definition from given file in lyx format
// Used for layouts
FontInfo lyxRead(Lexer & lex, FontInfo const & fi)
{
	FontInfo f = fi;
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
			setLyXFamily(ttok, f);
		} else if (tok == "series") {
			lex.next();
			string const ttok = lex.getString();
			setLyXSeries(ttok, f);
		} else if (tok == "shape") {
			lex.next();
			string const ttok = lex.getString();
			setLyXShape(ttok, f);
		} else if (tok == "size") {
			lex.next();
			string const ttok = lex.getString();
			setLyXSize(ttok, f);
		} else if (tok == "misc") {
			lex.next();
			string const ttok = ascii_lowercase(lex.getString());

			if (ttok == "no_bar") {
				f.setUnderbar(FONT_OFF);
			} else if (ttok == "no_strikeout") {
				f.setStrikeout(FONT_OFF);
			} else if (ttok == "no_xout") {
				f.setXout(FONT_OFF);
			} else if (ttok == "no_uuline") {
				f.setUuline(FONT_OFF);
			} else if (ttok == "no_uwave") {
				f.setUwave(FONT_OFF);
			} else if (ttok == "no_emph") {
				f.setEmph(FONT_OFF);
			} else if (ttok == "no_noun") {
				f.setNoun(FONT_OFF);
			} else if (ttok == "emph") {
				f.setEmph(FONT_ON);
			} else if (ttok == "underbar") {
				f.setUnderbar(FONT_ON);
			} else if (ttok == "strikeout") {
				f.setStrikeout(FONT_ON);
			} else if (ttok == "xout") {
				f.setXout(FONT_ON);
			} else if (ttok == "uuline") {
				f.setUuline(FONT_ON);
			} else if (ttok == "uwave") {
				f.setUwave(FONT_ON);
			} else if (ttok == "noun") {
				f.setNoun(FONT_ON);
			} else if (ttok == "nospellcheck") {
				f.setNoSpellcheck(FONT_ON);
			} else if (ttok == "no_nospellcheck") {
				f.setNoSpellcheck(FONT_OFF);
			} else {
				lex.printError("Illegal misc type");
			}
		} else if (tok == "color") {
			lex.next();
			string const ttok = lex.getString();
			setLyXColor(ttok, f);
		} else {
			lex.printError("Unknown tag");
			error = true;
		}
	}
	return f;
}


void lyxWrite(ostream & os, FontInfo const & f, string const & start, int level)
{
	string indent;
	for (int i = 0; i < level; ++i)
		indent += '\t';
	ostringstream oss;
	if (f.family() != INHERIT_FAMILY)
		oss << indent << "\tFamily " << LyXFamilyNames[f.family()]
		    << '\n';
	if (f.series() != INHERIT_SERIES)
		oss << indent << "\tSeries " << LyXSeriesNames[f.series()]
		    << '\n';
	if (f.shape() != INHERIT_SHAPE)
		oss << indent << "\tShape " << LyXShapeNames[f.shape()]
		    << '\n';
	if (f.size() != INHERIT_SIZE)
		oss << indent << "\tSize " << LyXSizeNames[f.size()]
		    << '\n';
	//FIXME: shall style be handled here? Probably not.
	if (f.underbar() == FONT_ON)
		oss << indent << "\tMisc Underbar\n";
	else if (f.underbar() == FONT_OFF)
		oss << indent << "\tMisc No_Bar\n";
	if (f.strikeout() == FONT_ON)
		oss << indent << "\tMisc Strikeout\n";
	else if (f.strikeout() == FONT_OFF)
		oss << indent << "\tMisc No_Strikeout\n";
	if (f.xout() == FONT_ON)
		oss << indent << "\tMisc Xout\n";
	else if (f.xout() == FONT_OFF)
		oss << indent << "\tMisc No_Xout\n";
	if (f.uuline() == FONT_ON)
		oss << indent << "\tMisc Uuline\n";
	else if (f.uuline() == FONT_OFF)
		oss << indent << "\tMisc No_Uuline\n";
	if (f.uwave() == FONT_ON)
		oss << indent << "\tMisc Uwave\n";
	else if (f.uwave() == FONT_OFF)
		oss << indent << "\tMisc No_Uwave\n";
	if (f.emph() == FONT_ON)
		oss << indent << "\tMisc Emph\n";
	else if (f.emph() == FONT_OFF)
		oss << indent << "\tMisc No_Emph\n";
	if (f.noun() == FONT_ON)
		oss << indent << "\tMisc Noun\n";
	else if (f.noun() == FONT_OFF)
		oss << indent << "\tMisc No_Noun\n";
	if (f.nospellcheck() == FONT_ON)
		oss << indent << "\tMisc NoSpellcheck\n";
	else if (f.nospellcheck() == FONT_OFF)
		oss << indent << "\tMisc No_NoSpellcheck\n";
	if (f.color() != Color_inherit && f.color() != Color_none)
		oss << indent << "\tColor " << lcolor.getLyXName(f.color())
		    << '\n';
	if (!oss.str().empty()) {
		os << indent << start << '\n'
		   << oss.str()
		   << indent << "EndFont\n";
	}
}


} // namespace lyx
