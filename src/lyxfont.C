/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== */

#include <config.h>
#include <clocale>

#ifdef __GNUG__
#pragma implementation "lyxfont.h"
#endif

#include <cctype>

#include "gettext.h"
#include "lyxfont.h"
#include "debug.h"
#include "lyxrc.h"
#include "lyxlex.h"
#include "FontLoader.h"
#include "support/lstrings.h"


// The global fontloader
FontLoader fontloader;

//
// Names for the GUI
//

string const GUIFamilyNames[6] = 
{ N_("Roman"), N_("Sans serif"), N_("Typewriter"), N_("Symbol"), N_("Inherit"),
  N_("Ignore") };
  
string const GUISeriesNames[4] = 
{ N_("Medium"), N_("Bold"), N_("Inherit"), N_("Ignore") };

string const GUIShapeNames[6] = 
{ N_("Upright"), N_("Italic"), N_("Slanted"), N_("Smallcaps"), N_("Inherit"),
  N_("Ignore") };

string const GUISizeNames[14] = 
{ N_("Tiny"), N_("Smallest"), N_("Smaller"), N_("Small"), N_("Normal"), N_("Large"),
  N_("Larger"), N_("Largest"), N_("Huge"), N_("Huger"), N_("Increase"), N_("Decrease"), 
  N_("Inherit"), N_("Ignore") };
 
string const lGUISizeNames[15] = 
{ N_("tiny"), N_("smallest"), N_("smaller"), N_("small"), N_("normal"), N_("large"),
  N_("larger"), N_("largest"), N_("huge"), N_("huger"), N_("increase"), N_("decrease"),
  N_("inherit"), N_("ignore"), string() };
 
string const GUIMiscNames[5] = 
{ N_("Off"), N_("On"), N_("Toggle"), N_("Inherit"), N_("Ignore") };

string const GUIDirectionNames[5] = 
{ N_("LTR"), N_("RTL"), N_("Toggle"), N_("Inherit"), N_("Ignore") };


//
// Strings used to read and write .lyx format files
//
string const LyXFamilyNames[6] = 
{ "roman", "sans", "typewriter", "symbol", "default", "error" };
 
string const LyXSeriesNames[4] = 
{ "medium", "bold", "default", "error" };
 
string const LyXShapeNames[6] = 
{ "up", "italic", "slanted", "smallcaps", "default", "error" };
 
string const LyXSizeNames[14] = 
{ "tiny", "scriptsize", "footnotesize", "small", "normal", "large",
  "larger", "largest", "huge", "giant", 
  "increase-error", "decrease-error", "default", "error" };

string const LyXMiscNames[12] = 
{ "off", "on", "toggle", "default", "error" };

//
// Strings used to write LaTeX files
//

string const LaTeXFamilyNames[6] = 
{ "textrm", "textsf", "texttt", "error1", "error2", "error3" };
 
string const LaTeXSeriesNames[4] = 
{ "textmd", "textbf", "error4", "error5" };
 
string const LaTeXShapeNames[6] = 
{ "textup", "textit", "textsl", "textsc", "error6", "error7" };
 
string const LaTeXSizeNames[14] = 
{ "tiny", "scriptsize", "footnotesize", "small", "normalsize", "large",
  "Large", "LARGE", "huge", "Huge", "error8", "error9", "error10", "error11" };


LyXFont::FontBits LyXFont::sane = {
	ROMAN_FAMILY,
	MEDIUM_SERIES,
	UP_SHAPE,
	SIZE_NORMAL,
	LColor::none,
	OFF,
	OFF,
	OFF,
	OFF,
	LTR_DIR };

LyXFont::FontBits LyXFont::inherit = {
	INHERIT_FAMILY,
	INHERIT_SERIES,
	INHERIT_SHAPE,
	INHERIT_SIZE,
	LColor::inherit,
	INHERIT,
	INHERIT,
	INHERIT,
	INHERIT,
	INHERIT_DIR };

LyXFont::FontBits LyXFont::ignore = {
	IGNORE_FAMILY,
	IGNORE_SERIES,
	IGNORE_SHAPE,
	IGNORE_SIZE,
	LColor::ignore,
	IGNORE,
	IGNORE,
	IGNORE,
	IGNORE,
	IGNORE_DIR };


/// Decreases font size by one
LyXFont & LyXFont::decSize() 
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
		lyxerr << "Can't LyXFont::decSize on INCREASE_SIZE" << endl;
		break;
	case DECREASE_SIZE:
		lyxerr <<"Can't LyXFont::decSize on DECREASE_SIZE" << endl;
		break;
	case INHERIT_SIZE:
		lyxerr <<"Can't LyXFont::decSize on INHERIT_SIZE" << endl;
		break;
	case IGNORE_SIZE:
		lyxerr <<"Can't LyXFont::decSize on IGNORE_SIZE" << endl;
		break;
	}
	return *this;
}


/// Increases font size by one
LyXFont & LyXFont::incSize() 
{
	switch(size()) {
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
		lyxerr <<"Can't LyXFont::incSize on INCREASE_SIZE" << endl;
		break;
	case DECREASE_SIZE:
		lyxerr <<"Can't LyXFont::incSize on DECREASE_SIZE" << endl;
		break;
	case INHERIT_SIZE:
		lyxerr <<"Can't LyXFont::incSize on INHERIT_SIZE" << endl;
		break;
	case IGNORE_SIZE:
		lyxerr <<"Can't LyXFont::incSize on IGNORE_SIZE" << endl;
		break;
	}
	return *this;
}


/// Updates a misc setting according to request
LyXFont::FONT_MISC_STATE LyXFont::setMisc(FONT_MISC_STATE newfont, 
					  FONT_MISC_STATE org)
{
	if (newfont == TOGGLE) {
		if (org == ON)
			return OFF;
		else if (org == OFF)
			return ON;
		else {
			lyxerr <<"LyXFont::setMisc: Need state"
				" ON or OFF to toggle. Setting to ON" << endl;
			return ON;
		}
	} else if (newfont == IGNORE)
		return org;
	else 
		return newfont;
}


/// Updates font settings according to request
void LyXFont::update(LyXFont const & newfont, bool toggleall)
{
	if(newfont.family() == family() && toggleall)
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

	if(newfont.shape() == shape() && toggleall)
		setShape(INHERIT_SHAPE); // toggle 'back'
	else if (newfont.shape() != IGNORE_SHAPE)
		setShape(newfont.shape());
	// else it's IGNORE_SHAPE

	if (newfont.size() != IGNORE_SIZE) {
		if (newfont.size() == INCREASE_SIZE)
			incSize();
		else if (newfont.size() == DECREASE_SIZE)
			decSize();
		else if (newfont.size() == size() && toggleall)
			setSize(INHERIT_SIZE); // toggle 'back'
		else
			setSize(newfont.size());
	}

	setEmph(setMisc(newfont.emph(), emph()));
	setUnderbar(setMisc(newfont.underbar(), underbar()));
	setNoun(setMisc(newfont.noun(), noun()));
	setLatex(setMisc(newfont.latex(), latex()));

	switch(newfont.direction()) {
	case TOGGLE_DIR:
		if (direction() == LTR_DIR)
			setDirection(RTL_DIR);
		else
			setDirection(LTR_DIR);
		break;
	case IGNORE_DIR:
		break;
	default:
		setDirection(newfont.direction());
	}

	if(newfont.color() == color() && toggleall)
		setColor(LColor::inherit); // toggle 'back'
	else if (newfont.color() != LColor::ignore)
		setColor(newfont.color());
}


/// Reduce font to fall back to template where possible
void LyXFont::reduce(LyXFont const & tmplt)
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
	if (latex() == tmplt.latex())
		setLatex(INHERIT);
	if (color() == tmplt.color())
		setColor(LColor::inherit);
	if (direction() == tmplt.direction())
		setDirection(INHERIT_DIR);
}


/// Realize font from a template
LyXFont & LyXFont::realize(LyXFont const & tmplt)
{
	if (bits == inherit) {
		bits = tmplt.bits;
		return *this;
	}
	
	if (bits.family == INHERIT_FAMILY) {
		bits.family = tmplt.bits.family;
	}
	if (bits.series == INHERIT_SERIES) {
		bits.series = tmplt.bits.series;
	}
	if (bits.shape == INHERIT_SHAPE) {
		bits.shape = tmplt.bits.shape;
	}
	if (bits.size == INHERIT_SIZE) {
		bits.size = tmplt.bits.size;
	}
	if (bits.emph == INHERIT) {
		bits.emph = tmplt.bits.emph;
	}
	if (bits.underbar == INHERIT) {
		bits.underbar = tmplt.bits.underbar;
	}
	if (bits.noun == INHERIT) {
		bits.noun = tmplt.bits.noun;
	}
	if (bits.latex == INHERIT) {
		bits.latex = tmplt.bits.latex;
	}
	if (bits.color == LColor::inherit) {
		bits.color = tmplt.bits.color;
	}
	if (bits.direction == INHERIT_DIR) {
		bits.direction = tmplt.bits.direction;
	}

	return *this;
}


/// Is font resolved?
bool LyXFont::resolved() const
{
	return (family() != INHERIT_FAMILY && series() != INHERIT_SERIES &&
		shape() != INHERIT_SHAPE && size() != INHERIT_SIZE &&
		emph() != INHERIT && underbar() != INHERIT && 
		noun() != INHERIT && latex() != INHERIT && 
		color() != LColor::inherit &&
		direction() != INHERIT_DIR);
}


/// Build GUI description of font state
string LyXFont::stateText() const
{
	string buf;
	if (family() != INHERIT_FAMILY)
		buf += string(_(GUIFamilyNames[family()].c_str())) + ", ";
	if (series() != INHERIT_SERIES)
		buf += string(_(GUISeriesNames[series()].c_str())) + ", ";
	if (shape() != INHERIT_SHAPE)
		buf += string(_(GUIShapeNames[shape()].c_str())) + ", ";
	if (size() != INHERIT_SIZE)
		buf += string(_(GUISizeNames[size()].c_str())) + ", ";
	if (color() != LColor::inherit)
		buf += lcolor.getGUIName(color()) + ", ";
	if (emph() != INHERIT)
		buf += string(_("Emphasis ")) + _(GUIMiscNames[emph()].c_str()) + ", ";
	if (underbar() != INHERIT)
		buf += string(_("Underline ")) + _(GUIMiscNames[underbar()].c_str()) + ", ";
	if (noun() != INHERIT)
		buf += string(_("Noun ")) + _(GUIMiscNames[noun()].c_str()) + ", ";
	if (latex() != INHERIT)
		buf += string(_("Latex ")) + _(GUIMiscNames[latex()].c_str()) + ", ";
	if (direction() != INHERIT_DIR)
		buf += string(_("Direction ")) + _(GUIDirectionNames[direction()].c_str()) + ", ";
	if (buf.empty())
		buf = _("Default");
	buf = strip(buf, ' ');
	buf = strip(buf, ',');
	return buf;
}


// Set family according to lyx format string
LyXFont & LyXFont::setLyXFamily(string const & fam)
{
	string s = lowercase(fam);

	int i = 0;
	while (s != LyXFamilyNames[i] && LyXFamilyNames[i] != "error") ++i;
	if (s == LyXFamilyNames[i]) {
		setFamily(LyXFont::FONT_FAMILY(i));
	} else
		lyxerr << "LyXFont::setLyXFamily: Unknown family `"
		       << s << '\'' << endl;
	return *this;
}


// Set series according to lyx format string
LyXFont & LyXFont::setLyXSeries(string const & ser)
{
	string s = lowercase(ser);

	int i = 0;
	while (s != LyXSeriesNames[i] && LyXSeriesNames[i] != "error") ++i;
	if (s == LyXSeriesNames[i]) {
		setSeries(LyXFont::FONT_SERIES(i));
	} else
		lyxerr << "LyXFont::setLyXSeries: Unknown series `"
		       << s << '\'' << endl;
	return *this;
}


// Set shape according to lyx format string
LyXFont & LyXFont::setLyXShape(string const & sha)
{
	string s = lowercase(sha);

	int i = 0;
	while (s != LyXShapeNames[i] && LyXShapeNames[i] != "error") ++i;
	if (s == LyXShapeNames[i]) {
		setShape(LyXFont::FONT_SHAPE(i));
	} else
		lyxerr << "LyXFont::setLyXShape: Unknown shape `"
		       << s << '\'' << endl;
	return *this;
}


// Set size according to lyx format string
LyXFont & LyXFont::setLyXSize(string const & siz)
{
	string s = lowercase(siz);
	int i = 0;
	while (s != LyXSizeNames[i] && LyXSizeNames[i] != "error") ++i;
	if (s == LyXSizeNames[i]) {
		setSize(LyXFont::FONT_SIZE(i));
	} else
		lyxerr << "LyXFont::setLyXSize: Unknown size `"
		       << s << '\'' << endl;
	return *this;
}

// Set size according to lyx format string
LyXFont::FONT_MISC_STATE LyXFont::setLyXMisc(string const & siz)
{
	string s = lowercase(siz);
	int i= 0;
	while (s != LyXMiscNames[i] && LyXMiscNames[i] != "error") ++i;
	if (s == LyXMiscNames[i])
		return FONT_MISC_STATE(i);
	lyxerr << "LyXFont::setLyXMisc: Unknown misc flag `"
	       << s << '\'' << endl;
	return OFF;
}

/// Sets color after LyX text format
LyXFont & LyXFont::setLyXColor(string const & col)
{
	setColor(lcolor.getFromLyXName(col));
	return *this;
}


/// Sets size after GUI name
LyXFont & LyXFont::setGUISize(string const & siz)
{
#warning this might be wrong.
	/// ??????
	// this is how it was how it was done in the lyx repository...
	// but this does not make sense.
	setColor(lcolor.getFromGUIName(siz));
	return *this;
}


// Returns size in latex format
string LyXFont::latexSize() const
{
	return LaTeXSizeNames[size()];
}


// Read a font definition from given file in lyx format
// Used for layouts
LyXFont & LyXFont::lyxRead(LyXLex & lex)
{
	bool error = false;
	bool finished = false;
	while (!finished && lex.IsOK() && !error) {
		lex.next();
		string tok = lowercase(lex.GetString());

		if (tok.empty()) {
			continue;
		} else if (tok == "endfont") {
			finished = true;
		} else if (tok == "family") {
			lex.next();
			string tok = lex.GetString();
			setLyXFamily(tok);
		} else if (tok == "series") {
			lex.next();
			string tok = lex.GetString();
			setLyXSeries(tok);
		} else if (tok == "shape") {
			lex.next();
			string tok = lex.GetString();
			setLyXShape(tok);
		} else if (tok == "size") {
			lex.next();
			string tok = lex.GetString();
			setLyXSize(tok);
		} else if (tok == "latex") {
			lex.next();
			string tok = lowercase(lex.GetString());

			if (tok == "no_latex") {
				setLatex(OFF);
			} else if (tok == "latex") {
				setLatex(ON);
			} else {
				lex.printError("Illegal LaTeX type`$$Token'");
			}
		} else if (tok == "misc") {
			lex.next();
			string tok = lowercase(lex.GetString());

			if (tok == "no_bar") {
				setUnderbar(OFF);
			} else if (tok == "no_emph") {
				setEmph(OFF);
			} else if (tok == "no_noun") {
				setNoun(OFF);
			} else if (tok == "emph") {
				setEmph(ON);
			} else if (tok == "underbar") {
				setUnderbar(ON);
			} else if (tok == "noun") {
				setNoun(ON);
			} else {
				lex.printError("Illegal misc type `$$Token´");
			}
		} else if (tok == "color") {
			lex.next();
			string tok = lex.GetString();
			setLyXColor(tok);
		} else if (tok == "direction") {
			lex.next();
			string tok = lowercase(lex.GetString());

			if (tok == "ltr") {
				setDirection(LTR_DIR);
			} else if (tok == "rtl") {
				setDirection(RTL_DIR);
			} else {
				lex.printError("Illegal type`$$Token'");
			}
		} else {
			lex.printError("Unknown tag `$$Token'");
			error = true;
		}
	}
	return *this;
}


/// Writes the changes from this font to orgfont in .lyx format in file
void LyXFont::lyxWriteChanges(LyXFont const & orgfont, ostream & os) const
{
	os << "\n";
	if (orgfont.family() != family()) {
		os << "\\family " << LyXFamilyNames[family()] << " \n";
	}
	if (orgfont.series() != series()) {
		os << "\\series " << LyXSeriesNames[series()] << " \n";
	}
	if (orgfont.shape() != shape()) {
		os << "\\shape " << LyXShapeNames[shape()] << " \n";
	}
	if (orgfont.size() != size()) {
		os << "\\size " << LyXSizeNames[size()] << " \n";
	}
	if (orgfont.emph() != emph()) {
		os << "\\emph " << LyXMiscNames[emph()] << " \n";
	}
	if (orgfont.underbar() != underbar()) {
		// This is only for backwards compatibility
		switch (underbar()) {
		case OFF:	os << "\\bar no \n"; break;
		case ON:        os << "\\bar under \n"; break;
		case TOGGLE:	lyxerr << "LyXFont::lyxWriteFontChanges: "
					"TOGGLE should not appear here!"
				       << endl;
		break;
		case INHERIT:   os << "\\bar default \n"; break;
		case IGNORE:    lyxerr << "LyXFont::lyxWriteFontChanges: "
					"IGNORE should not appear here!"
				       << endl;
		break;
		}
	}
	if (orgfont.noun() != noun()) {
		os << "\\noun " << LyXMiscNames[noun()] << " \n";
	}
	if (orgfont.latex() != latex()) {
		// This is only for backwards compatibility
		switch (latex()) {
		case OFF:	os << "\\latex no_latex \n"; break;
		case ON:        os << "\\latex latex \n"; break;
		case TOGGLE:	lyxerr << "LyXFont::lyxWriteFontChanges: "
					"TOGGLE should not appear here!"
				       << endl;
		break;
		case INHERIT:   os << "\\latex default \n"; break;
		case IGNORE:    lyxerr << "LyXFont::lyxWriteFontChanges: "
					"IGNORE should not appear here!"
				       << endl;
		break;
		}
	}
	if (orgfont.color() != color()) {
		os << "\\color " << lcolor.getLyXName(color()) << "\n";
	}
	if (orgfont.direction() != direction()) {
		switch (direction()) {
		case RTL_DIR:	os << "\\direction rtl \n"; break;
		case LTR_DIR:	os << "\\direction ltr\n"; break;
		case TOGGLE_DIR:   lyxerr << "LyXFont::lyxWriteFontChanges: "
					"TOGGLE should not appear here!"
				       << endl;
		case INHERIT_DIR:   os << "\\direction default \n"; break;
		case IGNORE_DIR:    lyxerr << "LyXFont::lyxWriteFontChanges: "
					"IGNORE should not appear here!"
				       << endl;
		break;
		}
	}
}


/// Writes the head of the LaTeX needed to impose this font
// Returns number of chars written.
int LyXFont::latexWriteStartChanges(ostream & os, LyXFont const & base,
				    LyXFont const & prev) const
{
	LyXFont f = *this;
	f.reduce(base);
	
	if (f.bits == inherit)
		return 0;
	
	int count = 0;
	bool env = false;

	FONT_DIRECTION direction = f.direction();
	if (direction != prev.direction()) {
		if (direction == LTR_DIR) {
			os << "\\L{";
			count += 3;
			env = true; //We have opened a new environment
		}
		if (direction == RTL_DIR) {
			os << "\\R{";
			count += 3;
			env = true; //We have opened a new environment
		}
	}

	if (f.family() != INHERIT_FAMILY) {
		os << '\\'
		   << LaTeXFamilyNames[f.family()]
		   << '{';
		count += LaTeXFamilyNames[f.family()].length() + 2;
		env = true; //We have opened a new environment
	}
	if (f.series() != INHERIT_SERIES) {
		os << '\\'
		   << LaTeXSeriesNames[f.series()]
		   << '{';
		count += LaTeXSeriesNames[f.series()].length() + 2;
		env = true; //We have opened a new environment
	}
	if (f.shape() != INHERIT_SHAPE) {
		os << '\\'
		   << LaTeXShapeNames[f.shape()]
		   << '{';
		count += LaTeXShapeNames[f.shape()].length() + 2;
		env = true; //We have opened a new environment
	}
	if (f.color() != LColor::inherit) {
		os << "\\textcolor{"
		   << lcolor.getLaTeXName(f.color())
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
		count += 8;
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
		count += LaTeXSizeNames[f.size()].length() + 2;
	}
	return count;
}


/// Writes ending block of LaTeX needed to close use of this font
// Returns number of chars written
// This one corresponds to latexWriteStartChanges(). (Asger)
int LyXFont::latexWriteEndChanges(ostream & os, LyXFont const & base,
				  LyXFont const & next) const
{
	LyXFont f = *this; // why do you need this?
	f.reduce(base); // why isn't this just "reduce(base);" (Lgb)
	// Because this function is const. Everything breaks if this
	// method changes the font it represents. There is no speed penalty
	// by using the temporary. (Asger)

	if (f.bits == inherit)
		return 0;
	
	int count = 0;
	bool env = false;

	FONT_DIRECTION direction = f.direction();
	if ( direction != next.direction()
	    && (direction == RTL_DIR || direction == LTR_DIR) ) {
		os << '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}

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
	if (f.color() != LColor::inherit) {
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
	return count;
}


LColor::color LyXFont::realColor() const
{
	if (latex() == ON)
		return LColor::latex;
	if (color() == LColor::none)
		return LColor::foreground;
	return color();
}


// Convert logical attributes to concrete shape attribute
LyXFont::FONT_SHAPE LyXFont::realShape() const
{
	register FONT_SHAPE s = shape();

	if (emph() == ON) {
		if (s == UP_SHAPE)
			s = ITALIC_SHAPE;
		else
			s = UP_SHAPE;
	}
	if (noun() == ON)
		s = SMALLCAPS_SHAPE;
	return s;
}


XFontStruct * LyXFont::getXFontstruct() const
{
	return fontloader.load(family(), series(), realShape(), size());
}


int LyXFont::maxAscent() const
{
	return getXFontstruct()->ascent;
}


int LyXFont::maxDescent() const
{
	return getXFontstruct()->descent;
}


int LyXFont::ascent(char c) const
{
	XFontStruct * finfo = getXFontstruct();
	unsigned int uc = static_cast<unsigned char>(c);
	if (finfo->per_char
	    && uc >= finfo->min_char_or_byte2
	    && uc <= finfo->max_char_or_byte2) 
		return finfo->per_char[uc - finfo->min_char_or_byte2].ascent;
	else
		return finfo->ascent;
}


int LyXFont::descent(char c) const
{
	XFontStruct * finfo = getXFontstruct();
	unsigned int uc = static_cast<unsigned char>(c);
	if (finfo->per_char
	    && uc >= finfo->min_char_or_byte2
	    && uc <= finfo->max_char_or_byte2) 
		return finfo->per_char[uc - finfo->min_char_or_byte2].descent;
	else
		return finfo->descent;
}


// Specialized after profiling. (Asger)
int LyXFont::width(char c) const
{
	if (realShape() != LyXFont::SMALLCAPS_SHAPE){
		return XTextWidth(getXFontstruct(), &c, 1);
	} else {
		return textWidth(&c, 1);
	}
}


int LyXFont::lbearing(char c) const
{
	XFontStruct * finfo = getXFontstruct();
	unsigned int uc = static_cast<unsigned char>(c);
	if (finfo->per_char
	    && uc >= finfo->min_char_or_byte2
	    && uc <= finfo->max_char_or_byte2) 
		return finfo->per_char[uc - finfo->min_char_or_byte2].lbearing;
	else
		return 0;
}


int LyXFont::rbearing(char c) const
{
	XFontStruct * finfo = getXFontstruct();
	unsigned int uc = static_cast<unsigned char>(c);
	if (finfo->per_char
	    && uc >= finfo->min_char_or_byte2
	    && uc <= finfo->max_char_or_byte2) 
		return finfo->per_char[uc - finfo->min_char_or_byte2].rbearing;
	else
		return width(c);
}


int LyXFont::textWidth(char const * s, int n) const
{
	if (realShape() != LyXFont::SMALLCAPS_SHAPE){
		return XTextWidth(getXFontstruct(), s, n);
	} else {
		// emulate smallcaps since X doesn't support this
		unsigned int result = 0;
		char c;
		LyXFont smallfont = *this;
		smallfont.decSize();
		smallfont.decSize();
		smallfont.setShape(LyXFont::UP_SHAPE);
		for (int i = 0; i < n; ++i) {
			c = s[i];
			// when islower is a macro, the cast is needed (JMarc)
			if (islower(static_cast<unsigned char>(c))){
				c = toupper(c);
				result += XTextWidth(smallfont.getXFontstruct(), &c, 1);
			} else {
				result += XTextWidth(getXFontstruct(), &c, 1);
			}
		}
		return result;
	}
}


int LyXFont::stringWidth(string const & s) const
{
	if (s.empty()) return 0;
	return textWidth(s.c_str(), s.length());
}


int LyXFont::signedStringWidth(string const & s) const
{
	if (s.empty()) return 0;
	if (s.c_str()[0] == '-')
		return -textWidth(s.c_str()+1, s.length()-1);
	else
		return textWidth(s.c_str(), s.length());
}


int LyXFont::drawText(char const * s, int n, Pixmap, 
		      int, int x) const
{
	if (realShape() != LyXFont::SMALLCAPS_SHAPE) {
		/* XDrawString(fl_display,
		   pm,
		   getGC(),
		   x, baseline,
		   s, n);
		   XFlush(fl_display); */
		return XTextWidth(getXFontstruct(), s, n);

	} else {
		// emulate smallcaps since X doesn't support this
		char c;
		int sx = x;
		LyXFont smallfont = *this;
		smallfont.decSize();
		smallfont.decSize();
		smallfont.setShape(LyXFont::UP_SHAPE);
		for (int i = 0; i < n; ++i) {
			c = s[i];
			if (islower(static_cast<unsigned char>(c))){
				c = toupper(c);
				/* XDrawString(fl_display,
				   pm,
				   smallfont.getGC(),
				   x, baseline,
				   &c, 1); */
				x += XTextWidth(smallfont.getXFontstruct(),
						&c, 1);
				//XFlush(fl_display);
			} else {
				/* XDrawString(fl_display,
				   pm,
				   getGC(),
				   x, baseline,
				   &c, 1);*/
				x += XTextWidth(getXFontstruct(), &c, 1);
				//XFlush(fl_display);
			}
		}
		return x - sx;
	}
}


int LyXFont::drawString(string const & s, Pixmap pm, int baseline, int x) const
{
	return drawText(s.c_str(), s.length(), pm, baseline, x);
}


bool LyXFont::equalExceptLatex(LyXFont const & f) const 
{
	LyXFont f1 = *this;
	f1.setLatex(f.latex());
	return f1 == f;
}


LyXDirection LyXFont::getFontDirection() const
{
	if (lyxrc.rtl_support 
	    && direction() == LyXFont::RTL_DIR
	    && latex() != LyXFont::ON)
		return LYX_DIR_RIGHT_TO_LEFT;
	else
		return LYX_DIR_LEFT_TO_RIGHT;
}


ostream & operator<<(ostream & o, LyXFont::FONT_MISC_STATE fms)
{
	return o << int(fms);
}
