/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1997 Asger Alstrup
 *
 * ======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetspecialchar.h"
#include "lyxdraw.h"
#include "debug.h"
#include "LaTeXFeatures.h"

InsetSpecialChar::InsetSpecialChar()
{
}


InsetSpecialChar::InsetSpecialChar(Kind k)
	: kind(k)
{
}


InsetSpecialChar::~InsetSpecialChar()
{
}


int InsetSpecialChar::Ascent(LyXFont const&font) const
{
	return font.maxAscent();
}


int InsetSpecialChar::Descent(LyXFont const&font) const
{
	return font.maxDescent();
}


int InsetSpecialChar::Width(LyXFont const&font) const
{
	LyXFont f = font;
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


void InsetSpecialChar::Draw(LyXFont font, LyXScreen &scr,
			    int baseline, float &x)
{
	switch (kind) {
	case HYPHENATION:
	{
		font.setColor(LyXFont::MAGENTA);
		scr.drawText(font, "-", 1, baseline, int(x));
		x += Width(font);
		break;
	}
	case END_OF_SENTENCE:
	{
		font.setColor(LyXFont::MAGENTA);
		scr.drawText(font, ".", 1, baseline, int(x));
		x += Width(font);
		break;
	}
	case LDOTS:
	{
		font.setColor(LyXFont::MAGENTA);
		scr.drawText(font, ". . .", 5, baseline, int(x));
		x += Width(font);
		break;
	}
	case MENU_SEPARATOR:
	{
		// A triangle the width and height of an 'x'
		int w = font.textWidth("x", 1);
		int ox = font.textWidth(" ", 1) + int(x);
		int h = font.ascent('x');
		XPoint p[4];
		p[0].x = ox;	p[0].y = baseline;
		p[1].x = ox;	p[1].y = baseline - h;
		p[2].x = ox + w;p[2].y = baseline - h/2;
		p[3].x = ox;	p[3].y = baseline;
		scr.drawLines(getGC(gc_copy), p, 4);
		x += Width(font);
	}
	}
}


// In lyxf3 this will be just LaTeX
void InsetSpecialChar::Write(FILE *file)
{
	string command;
	switch (kind) {
	case HYPHENATION:	command = "\\-";	break;
	case END_OF_SENTENCE:	command = "\\@.";	break;
	case LDOTS:		command = "\\ldots{}";	break;
	case MENU_SEPARATOR:    command = "\\menuseparator"; break;
	}
	fprintf(file, "\\SpecialChar %s\n", command.c_str());
}


// This function will not be necessary when lyx3
void InsetSpecialChar::Read(LyXLex &lex)
{    
	lex.nextToken();
	string command = lex.GetString();

	if (command=="\\-")
		kind = HYPHENATION;
	else if (command=="\\@.")
		kind = END_OF_SENTENCE;
	else if (command=="\\ldots{}")
		kind = LDOTS;
	else if (command=="\\menuseparator")
	        kind = MENU_SEPARATOR;
	else
		lex.printError("InsetSpecialChar: Unknown kind: `$$Token'");
}


int InsetSpecialChar::Latex(FILE *file, signed char /*fragile*/)
{
	string command;
	signed char dummy = 0;
	Latex(command, dummy);
	fprintf(file, "%s", command.c_str());
	return 0;
}


int InsetSpecialChar::Latex(string &file, signed char /*fragile*/)
{
	switch (kind) {
	case HYPHENATION:	file += "\\-";	break;
	case END_OF_SENTENCE:	file += "\\@.";	break;
	case LDOTS:		file += "\\ldots{}";	break;
	case MENU_SEPARATOR:    file += "\\lyxarrow{}"; break;
	}
	return 0;
}


int InsetSpecialChar::Linuxdoc(string &file)
{
	switch (kind) {
	case HYPHENATION:	file += "";	break;
	case END_OF_SENTENCE:	file += "";	break;
	case LDOTS:		file += "...";	break;
	case MENU_SEPARATOR:    file += "->";   break;
	}
	return 0;
}


int InsetSpecialChar::DocBook(string &file)
{
	switch (kind) {
	case HYPHENATION:	file += "";	break;
	case END_OF_SENTENCE:	file += "";	break;
	case LDOTS:		file += "...";	break;
	case MENU_SEPARATOR:    file += "->";   break;
	}
	return 0;
}


Inset* InsetSpecialChar::Clone()
{
	InsetSpecialChar *result = new InsetSpecialChar(kind);
	return result;
}


void InsetSpecialChar::Validate(LaTeXFeatures& features) const
{
	if (kind == MENU_SEPARATOR) {
		features.lyxarrow = true;
	}
}
