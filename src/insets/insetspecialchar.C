/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1997 Asger Alstrup
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetspecialchar.h"
#include "debug.h"
#include "LaTeXFeatures.h"
#include "Painter.h"


InsetSpecialChar::InsetSpecialChar(Kind k)
	: kind(k)
{}


int InsetSpecialChar::ascent(Painter &, LyXFont const & font) const
{
	return font.maxAscent();
}


int InsetSpecialChar::descent(Painter &, LyXFont const & font) const
{
	return font.maxDescent();
}


int InsetSpecialChar::width(Painter &, LyXFont const & font) const
{
	LyXFont f(font);
	switch (kind) {
	case HYPHENATION:
	{
		int w = f.textWidth("-", 1);
		if (w > 5) 
			w -= 2; // to make it look shorter
		return w;
	}
	case END_OF_SENTENCE:
	{
		return f.textWidth(".", 1);
	}
	case LDOTS:
	{
		return f.textWidth(". . .", 5);
	}
	case MENU_SEPARATOR: {
		return f.textWidth(" x ", 3);
	}
	}
	return 1; // To shut up gcc
}


void InsetSpecialChar::draw(Painter & pain, LyXFont const & f,
			    int baseline, float & x) const
{
	LyXFont font(f);
	switch (kind) {
	case HYPHENATION:
	{
		font.setColor(LColor::special);
		pain.text(int(x), baseline, "-", font);
		x += width(pain, font);
		break;
	}
	case END_OF_SENTENCE:
	{
		font.setColor(LColor::special);
		pain.text(int(x), baseline, ".", font);
		x += width(pain, font);
		break;
	}
	case LDOTS:
	{
		font.setColor(LColor::special);
		pain.text(int(x), baseline, ". . .", font);
		x += width(pain, font);
		break;
	}
	case MENU_SEPARATOR:
	{
		// A triangle the width and height of an 'x'
		int w = font.textWidth("x", 1);
		int ox = font.textWidth(" ", 1) + int(x);
		int h = font.ascent('x');
		int xp[4], yp[4];
		
		xp[0] = ox;	yp[0] = baseline;
		xp[1] = ox;	yp[1] = baseline - h;
		xp[2] = ox + w; yp[2] = baseline - h/2;
		xp[3] = ox;	yp[3] = baseline;
		
		pain.lines(xp, yp, 4, LColor::special);
		x += width(pain, font);
	}
	}
}


// In lyxf3 this will be just LaTeX
void InsetSpecialChar::Write(ostream & os)
{
	string command;
	switch (kind) {
	case HYPHENATION:	command = "\\-";	break;
	case END_OF_SENTENCE:	command = "\\@.";	break;
	case LDOTS:		command = "\\ldots{}";	break;
	case MENU_SEPARATOR:    command = "\\menuseparator"; break;
	}
	os << "\\SpecialChar " << command << "\n";
}


// This function will not be necessary when lyx3
void InsetSpecialChar::Read(LyXLex & lex)
{    
	lex.nextToken();
	string command = lex.GetString();

	if (command == "\\-")
		kind = HYPHENATION;
	else if (command == "\\@.")
		kind = END_OF_SENTENCE;
	else if (command == "\\ldots{}")
		kind = LDOTS;
	else if (command == "\\menuseparator")
	        kind = MENU_SEPARATOR;
	else
		lex.printError("InsetSpecialChar: Unknown kind: `$$Token'");
}


int InsetSpecialChar::Latex(ostream & os, signed char /*fragile*/)
{
	string command;
	signed char dummy = 0;
	Latex(command, dummy);
	os << command;
	return 0;
}


int InsetSpecialChar::Latex(string & file, signed char /*fragile*/)
{
	switch (kind) {
	case HYPHENATION:	file += "\\-";	break;
	case END_OF_SENTENCE:	file += "\\@.";	break;
	case LDOTS:		file += "\\ldots{}";	break;
	case MENU_SEPARATOR:    file += "\\lyxarrow{}"; break;
	}
	return 0;
}


int InsetSpecialChar::Linuxdoc(string & file)
{
	switch (kind) {
	case HYPHENATION:	file += "";	break;
	case END_OF_SENTENCE:	file += "";	break;
	case LDOTS:		file += "...";	break;
	case MENU_SEPARATOR:    file += "->";   break;
	}
	return 0;
}


int InsetSpecialChar::DocBook(string & file)
{
	switch (kind) {
	case HYPHENATION:	file += "";	break;
	case END_OF_SENTENCE:	file += "";	break;
	case LDOTS:		file += "...";	break;
	case MENU_SEPARATOR:    file += "->";   break;
	}
	return 0;
}


Inset * InsetSpecialChar::Clone() const
{
	return new InsetSpecialChar(kind);
}


void InsetSpecialChar::Validate(LaTeXFeatures & features) const
{
	if (kind == MENU_SEPARATOR) {
		features.lyxarrow = true;
	}
}
