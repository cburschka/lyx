/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *          Copyright (C) 1995-1999 The LyX Team.
 *
 * ======================================================*/

#include <config.h>
#include <clocale>

#ifdef __GNUG__
#pragma implementation "lyxfont.h"
#endif

#include <cctype>
#include "gettext.h"
#include "definitions.h"
#include "lyxfont.h"
#include "error.h"
#include "lyxrc.h"
#include "lyxlex.h"
#include "lyxdraw.h"
#include "FontLoader.h"
#include "support/lstrings.h"

extern LyXRC * lyxrc;

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

string const GUIColorNames[13] = 
{ N_("None"), N_("Black"), N_("White"), N_("Red"), N_("Green"), N_("Blue"),
    N_("Cyan"), N_("Magenta"), 
  N_("Yellow"), N_("Math"), N_("Inset"), N_("Inherit"), N_("Ignore") };
 
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

string const LyXColorNames[13] =
{ "none", "black", "white", "red", "green", "blue", "cyan", "magenta", 
  "yellow", "matherror", "inseterror", "default", "error" };

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
 
string const LaTeXColorNames[13] =
{ "none", "black", "white", "red", "green", "blue", "cyan", "magenta", 
  "yellow", "error12", "error13", "error14", "error15" };

/// Decreases font size by one
LyXFont& LyXFont::decSize() 
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
		lyxerr.print("Can't LyXFont::decSize on INCREASE_SIZE");
		break;
	case DECREASE_SIZE:
		lyxerr.print("Can't LyXFont::decSize on DECREASE_SIZE");
		break;
	case INHERIT_SIZE:
		lyxerr.print("Can't LyXFont::decSize on INHERIT_SIZE");
		break;
	case IGNORE_SIZE:
		lyxerr.print("Can't LyXFont::decSize on IGNORE_SIZE");
		break;
	}
	return (*this);
}


/// Increases font size by one
LyXFont& LyXFont::incSize() 
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
		lyxerr.print("Can't LyXFont::incSize on INCREASE_SIZE");
		break;
	case DECREASE_SIZE:
		lyxerr.print("Can't LyXFont::incSize on DECREASE_SIZE");
		break;
	case INHERIT_SIZE:
		lyxerr.print("Can't LyXFont::incSize on INHERIT_SIZE");
		break;
	case IGNORE_SIZE:
		lyxerr.print("Can't LyXFont::incSize on IGNORE_SIZE");
		break;
	}
	return (*this);
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
			lyxerr.print("LyXFont::setMisc: Need state"
				      " ON or OFF to toggle. Setting to ON");
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

	if(newfont.color() == color() && toggleall)
	  setColor(INHERIT_COLOR); // toggle 'back'
	else if (newfont.color() != IGNORE_COLOR)
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
		setColor(INHERIT_COLOR);
}


/// Realize font from a template
// This one is not pretty, but it's extremely fast (Asger)
LyXFont & LyXFont::realize(LyXFont const & tmplt)
{
	if (bits == inherit) {
		bits = tmplt.bits;
		return *this;
	}

	if ((bits & (Fam_Mask<<Fam_Pos)) == (ui32(INHERIT_FAMILY)<<Fam_Pos))
	{
		bits &= ~(Fam_Mask << Fam_Pos);
		bits |= (tmplt.bits & Fam_Mask << Fam_Pos);
	}
	if ((bits & (Ser_Mask<<Ser_Pos)) == (ui32(INHERIT_SERIES)<<Ser_Pos))
	{
		bits &= ~(Ser_Mask << Ser_Pos);
		bits |= (tmplt.bits & Ser_Mask << Ser_Pos);
	}
	if ((bits & (Sha_Mask << Sha_Pos)) == ui32(INHERIT_SHAPE) << Sha_Pos)
	{
		bits &= ~(Sha_Mask << Sha_Pos);
		bits |= (tmplt.bits & Sha_Mask << Sha_Pos);
	}
	if ((bits & (Siz_Mask << Siz_Pos)) == ui32(INHERIT_SIZE) << Siz_Pos)
	{
		bits &= ~(Siz_Mask << Siz_Pos);
		bits |= (tmplt.bits & Siz_Mask << Siz_Pos);
	}
	if ((bits & (Misc_Mask << Emp_Pos)) == ui32(INHERIT) << Emp_Pos)
	{
		bits &= ~(Misc_Mask << Emp_Pos);
		bits |= (tmplt.bits & Misc_Mask << Emp_Pos);
	}
	if ((bits & (Misc_Mask << Und_Pos)) == ui32(INHERIT) << Und_Pos)
	{
		bits &= ~(Misc_Mask << Und_Pos);
		bits |= (tmplt.bits & Misc_Mask << Und_Pos);
	}
	if ((bits & (Misc_Mask << Nou_Pos)) == ui32(INHERIT) << Nou_Pos)
	{
		bits &= ~(Misc_Mask << Nou_Pos);
		bits |= (tmplt.bits & Misc_Mask << Nou_Pos);
	}
	if ((bits & (Misc_Mask << Lat_Pos)) == ui32(INHERIT) << Lat_Pos)
	{
		bits &= ~(Misc_Mask << Lat_Pos);
		bits |= (tmplt.bits & Misc_Mask << Lat_Pos);
	}
	if ((bits & (Col_Mask << Col_Pos)) == ui32(INHERIT_COLOR) << Col_Pos)
	{
		bits &= ~(Col_Mask << Col_Pos);
		bits |= (tmplt.bits & Col_Mask << Col_Pos);
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
		color() != INHERIT_COLOR);
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
	if (color() != INHERIT_COLOR)
		buf += string(_(GUIColorNames[color()].c_str())) + ", ";
 
	if (emph() != INHERIT)
		buf += string(_("Emphasis ")) + _(GUIMiscNames[emph()].c_str()) + ", ";
	if (underbar() != INHERIT)
		buf += string(_("Underline ")) + _(GUIMiscNames[underbar()].c_str()) + ", ";
	if (noun() != INHERIT)
		buf += string(_("Noun ")) + _(GUIMiscNames[noun()].c_str()) + ", ";
	if (latex() != INHERIT)
		buf += string(_("Latex ")) + _(GUIMiscNames[latex()].c_str()) + ", ";
	if (buf.empty())
		buf = _("Default");
	buf = strip(buf, ' ');
	buf = strip(buf, ',');
	return buf;
}


// Set family according to lyx format string
LyXFont& LyXFont::setLyXFamily(string const & fam)
{
	string s = lowercase(fam);

	int i=0;
	while (s != LyXFamilyNames[i] && LyXFamilyNames[i] != "error") i++;
	if (s == LyXFamilyNames[i]) {
		setFamily(LyXFont::FONT_FAMILY(i));
	} else
		lyxerr.print("LyXFont::setLyXFamily: Unknown family `"+s+'\'');
	return (*this);
}


// Set series according to lyx format string
LyXFont& LyXFont::setLyXSeries(string const & ser)
{
	string s = lowercase(ser);

	int i=0;
	while (s != LyXSeriesNames[i] && LyXSeriesNames[i] != "error") i++;
	if (s == LyXSeriesNames[i]) {
		setSeries(LyXFont::FONT_SERIES(i));
	} else
		lyxerr.print("LyXFont::setLyXSeries: Unknown series `"+s+'\'');
	return (*this);
}


// Set shape according to lyx format string
LyXFont& LyXFont::setLyXShape(string const & sha)
{
	string s = lowercase(sha);

	int i=0;
	while (s != LyXShapeNames[i] && LyXShapeNames[i] != "error") i++;
	if (s == LyXShapeNames[i]) {
		setShape(LyXFont::FONT_SHAPE(i));
	} else
		lyxerr.print("LyXFont::setLyXShape: Unknown shape `"+s+'\'');
	return (*this);
}


// Set size according to lyx format string
LyXFont& LyXFont::setLyXSize(string const & siz)
{
	string s = lowercase(siz);
	int i=0;
	while (s != LyXSizeNames[i] && LyXSizeNames[i] != "error") i++;
	if (s == LyXSizeNames[i]) {
		setSize(LyXFont::FONT_SIZE(i));
	} else
		lyxerr.print("LyXFont::setLyXSize: Unknown size `"+s+'\'');
	return (*this);
}

// Set size according to lyx format string
LyXFont::FONT_MISC_STATE LyXFont::setLyXMisc(string const & siz)
{
	string s = lowercase(siz);
	int i=0;
	while (s != LyXMiscNames[i] && LyXMiscNames[i] != "error") i++;
	if (s == LyXMiscNames[i])
		return FONT_MISC_STATE(i);
	lyxerr.print("LyXFont::setLyXMisc: Unknown misc flag `"+s+'\'');
	return OFF;
}

/// Sets color after LyX text format
LyXFont& LyXFont::setLyXColor(string const & col)
{
	string s = lowercase(col);
	int i=0;
	while (s != LyXColorNames[i] && LyXColorNames[i] != "error") i++;
	if (s == LyXColorNames[i]) {
		setColor(LyXFont::FONT_COLOR(i));
	} else
		lyxerr.print("LyXFont::setLyXColor: Unknown Color `"+s+'\'');
	return (*this);
}


/// Sets size after GUI name
LyXFont& LyXFont::setGUISize(string const & siz)
{
	string s = lowercase(siz);
	int i=0;
	while (!lGUISizeNames[i].empty() &&
	       s != _(lGUISizeNames[i].c_str()))
	  i++;
	if (s == _(lGUISizeNames[i].c_str())) {
		setSize(LyXFont::FONT_SIZE(i));
	} else
		lyxerr.print("LyXFont::setGUISize: Unknown Size `"+s+'\'');
	return (*this);
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
		} else {
			lex.printError("Unknown tag `$$Token'");
			error = true;
		}
	}
	return *this;
}


/// Writes the changes from this font to orgfont in .lyx format in file
void LyXFont::lyxWriteChanges(LyXFont const & orgfont, FILE * file) const
{
	fprintf(file, "\n");
	if (orgfont.family() != family()) {
		fprintf(file, "\\family %s \n",
			LyXFamilyNames[family()].c_str());
	}
	if (orgfont.series() != series()) {
		fprintf(file, "\\series %s \n",
			LyXSeriesNames[series()].c_str());
	}
	if (orgfont.shape() != shape()) {
		fprintf(file, "\\shape %s \n", LyXShapeNames[shape()].c_str());
	}
	if (orgfont.size() != size()) {
		fprintf(file, "\\size %s \n", LyXSizeNames[size()].c_str());
	}
	if (orgfont.emph() != emph()) {
		fprintf(file, "\\emph %s \n", LyXMiscNames[emph()].c_str());
	}
	if (orgfont.underbar() != underbar()) {
		// This is only for backwards compatibility
		switch (underbar()) {
		case OFF:	fprintf(file, "\\bar no \n"); break;
		case ON:        fprintf(file, "\\bar under \n"); break;
		case TOGGLE:	lyxerr.print("LyXFont::lyxWriteFontChanges: "
					      "TOGGLE should not appear here!");
				break;
		case INHERIT:   fprintf(file, "\\bar default \n"); break;
		case IGNORE:    lyxerr.print("LyXFont::lyxWriteFontChanges: "
					      "IGNORE should not appear here!");
				break;
		}
	}
	if (orgfont.noun() != noun()) {
		fprintf(file, "\\noun %s \n", LyXMiscNames[noun()].c_str());
	}
	if (orgfont.latex() != latex()) {
		// This is only for backwards compatibility
		switch (latex()) {
		case OFF:	fprintf(file, "\\latex no_latex \n"); break;
		case ON:        fprintf(file, "\\latex latex \n"); break;
		case TOGGLE:	lyxerr.print("LyXFont::lyxWriteFontChanges: "
					      "TOGGLE should not appear here!");
				break;
		case INHERIT:   fprintf(file, "\\latex default \n"); break;
		case IGNORE:    lyxerr.print("LyXFont::lyxWriteFontChanges: "
					      "IGNORE should not appear here!");
				break;
		}
	}
	if (orgfont.color() != color()) {
		fprintf(file, "\\color %s\n", LyXColorNames[color()].c_str());
	}
}


/// Writes the head of the LaTeX needed to impose this font
// Returns number of chars written.
int LyXFont::latexWriteStartChanges(FILE * file, LyXFont const & base) const
{
	string font;
	int count = latexWriteStartChanges(font, base);
	fprintf(file, "%s", font.c_str());
	return count;
}


/// Writes the head of the LaTeX needed to impose this font
// Returns number of chars written.
int LyXFont::latexWriteStartChanges(string & file, LyXFont const & base) const
{
	LyXFont f = *this;
	f.reduce(base);
	
	if (f.bits == inherit)
		return 0;
	
	int count = 0;
	bool env = false;
	
	if (f.family() != INHERIT_FAMILY) {
		file += '\\';
		file += LaTeXFamilyNames[f.family()];
		file += '{';
		count += LaTeXFamilyNames[f.family()].length() + 2;
		env = true; //We have opened a new environment
	}
	if (f.series() != INHERIT_SERIES) {
		file += '\\';
		file += LaTeXSeriesNames[f.series()];
		file += '{';
		count += LaTeXSeriesNames[f.series()].length() + 2;
		env = true; //We have opened a new environment
	}
	if (f.shape() != INHERIT_SHAPE) {
		file += '\\';
		file += LaTeXShapeNames[f.shape()];
		file += '{';
		count += LaTeXShapeNames[f.shape()].length() + 2;
		env = true; //We have opened a new environment
	}
	if (f.color() != INHERIT_COLOR) {
		file += "\\textcolor{";
		file += LaTeXColorNames[f.color()];
		file += "}{";
		count += LaTeXColorNames[f.color()].length() + 13;
		env = true; //We have opened a new environment
	}
	if (f.emph() == ON) {
		file += "\\emph{";
		count += 6;
		env = true; //We have opened a new environment
	}
	if (f.underbar() == ON) {
		file += "\\underbar{";
		count += 10;
		env = true; //We have opened a new environment
	}
	// \noun{} is a LyX special macro
	if (f.noun() == ON) {
		file += "\\noun{";
		count += 8;
		env = true; //We have opened a new environment
	}
	if (f.size() != INHERIT_SIZE) {
		// If we didn't open an environment above, we open one here
		if (!env) {
			file += '{';
			count++;
		}
		file += '\\';
		file += LaTeXSizeNames[f.size()];
		file += ' ';
		count += LaTeXSizeNames[f.size()].length() + 2;
	}
	return count;
}


/// Writes ending block of LaTeX needed to close use of this font
// Returns number of chars written
// This one corresponds to latexWriteStartChanges(). (Asger)
int LyXFont::latexWriteEndChanges(FILE * file, LyXFont const & base) const
{
	string ending;
	int count = latexWriteEndChanges(ending, base);
	fprintf(file, "%s", ending.c_str());
	return count;
}


/// Writes ending block of LaTeX needed to close use of this font
// Returns number of chars written
// This one corresponds to latexWriteStartChanges(). (Asger)
int LyXFont::latexWriteEndChanges(string & file, LyXFont const & base) const
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
	if (f.family() != INHERIT_FAMILY) {
		file += '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}
	if (f.series() != INHERIT_SERIES) {
		file += '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}
	if (f.shape() != INHERIT_SHAPE) {
		file += '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}
	if (f.color() != INHERIT_COLOR) {
		file += '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}
	if (f.emph() == ON) {
		file += '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}
	if (f.underbar() == ON) {
		file += '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}
	if (f.noun() == ON) {
		file += '}';
		++count;
		env = true; // Size change need not bother about closing env.
	}
	if (f.size() != INHERIT_SIZE) {
		// We only have to close if only size changed
		if (!env) {
			file += '}';
			++count;
		}
	}
	
	return count;
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


GC LyXFont::getGC() const
{
	GC gc;
	if (latex() == ON)
		gc = ::getGC(gc_latex);
	else {
		if (color() == NONE)
			gc = ::getGC(gc_copy);
		else if (color() == MATH)
			gc = ::getGC(gc_math);
		else if (color() == INSET)
			gc = ::getGC(gc_foot);
		else
			gc = ::GetColorGC(color());
	}

	XSetFont(fl_display, gc, getXFontstruct()->fid);
	return gc;
}


XFontStruct* LyXFont::getXFontstruct() const
{
	return fontloader.load(family(),series(),realShape(),size());
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
	XFontStruct *finfo = getXFontstruct();
	if (finfo->per_char
	    && (unsigned int) c >= finfo->min_char_or_byte2
	    && (unsigned int) c <= finfo->max_char_or_byte2) {
		unsigned int index = (unsigned int) c - finfo->min_char_or_byte2;
		return finfo->per_char[index].ascent;
	} else
		return finfo->ascent;
}


int LyXFont::descent(char c) const
{
	XFontStruct *finfo = getXFontstruct();
	if (finfo->per_char
	    && (unsigned int) c >= finfo->min_char_or_byte2
	    && (unsigned int) c <= finfo->max_char_or_byte2) {
		unsigned int index = (unsigned int) c - finfo->min_char_or_byte2;
		return finfo->per_char[index].descent;
	} else
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


int LyXFont::textWidth(char const *s, int n) const
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
		for (int i=0; i < n; i++){
			c = s[i];
			if (islower((unsigned char) c)){
				c = toupper( (unsigned char) c );
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


int LyXFont::drawText(char const* s, int n, Pixmap pm, 
		       int baseline, int x) const
{
	if (realShape() != LyXFont::SMALLCAPS_SHAPE) {
		XDrawString(fl_display,
			    pm,
			    getGC(),
			    x, baseline,
			    s, n);
		XFlush(fl_display);
		return XTextWidth(getXFontstruct(), s, n);

	} else {
		// emulate smallcaps since X doesn't support this
		int i;
		char c;
		int sx = x;
		LyXFont smallfont = *this;
		smallfont.decSize();
		smallfont.decSize();
		smallfont.setShape(LyXFont::UP_SHAPE);
		for (i=0; i < n; i++){
			c = s[i];
			if (islower((unsigned char) c)){
				c = toupper((unsigned char) c);
				XDrawString(fl_display,
					    pm,
					    smallfont.getGC(),
					    x, baseline,
					    &c, 1);
				x += XTextWidth(smallfont.getXFontstruct(), &c, 1);
				XFlush(fl_display);
			} else {
				XDrawString(fl_display,
					    pm,
					    getGC(),
					    x, baseline,
					    &c, 1);
				x += XTextWidth(getXFontstruct(), &c, 1);
				XFlush(fl_display);
			}
		}
		return x - sx;
	}
}


int LyXFont::drawString(string const &s, Pixmap pm, int baseline, int x) const
{
	return drawText(s.c_str(), s.length(), pm, baseline, x);
}


bool LyXFont::equalExceptLatex(LyXFont const &f) const 
{
	LyXFont f1 = *this;
	f1.setLatex(f.latex());
	return f1 == f;
}
