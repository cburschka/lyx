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
#include "BufferView.h"
#include "Painter.h"
#include "font.h"

using std::ostream;
using std::max;

InsetSpecialChar::InsetSpecialChar(Kind k)
	: kind(k)
{}


int InsetSpecialChar::ascent(BufferView *, LyXFont const & font) const
{
	return lyxfont::maxAscent(font);
}


int InsetSpecialChar::descent(BufferView *, LyXFont const & font) const
{
	return lyxfont::maxDescent(font);
}


int InsetSpecialChar::width(BufferView *, LyXFont const & font) const
{
	switch (kind) {
	case HYPHENATION:
	{
		int w = lyxfont::width('-', font);
		if (w > 5) 
			w -= 2; // to make it look shorter
		return w;
	}
	case END_OF_SENTENCE:
	{
		return lyxfont::width('.', font);
	}
	case LDOTS:
	{
		return lyxfont::width(". . .", font);
	}
	case MENU_SEPARATOR:
	{
		return lyxfont::width(" x ", font);
	}
	case PROTECTED_SEPARATOR:
	{
		return lyxfont::width('x', font);
	}
	
	}
	return 1; // To shut up gcc
}


void InsetSpecialChar::draw(BufferView * bv, LyXFont const & f,
			    int baseline, float & x, bool) const
{
	Painter & pain = bv->painter();
	LyXFont font(f);

	switch (kind) {
	case HYPHENATION:
	{
		font.setColor(LColor::special);
		pain.text(int(x), baseline, "-", font);
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
		int w = lyxfont::width('x', font);
		int ox = lyxfont::width(' ', font) + int(x);
		int h = lyxfont::ascent('x', font);
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
		int h = lyxfont::ascent('x', font);
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
	switch (kind) {
	case HYPHENATION:	command = "\\-";	break;
	case END_OF_SENTENCE:	command = "\\@.";	break;
	case LDOTS:		command = "\\ldots{}";	break;
	case MENU_SEPARATOR:    command = "\\menuseparator"; break;
	case PROTECTED_SEPARATOR:
		//command = "\\protected_separator";
		command = "~";
		break;
	}
	os << "\\SpecialChar " << command << "\n";
}


// This function will not be necessary when lyx3
void InsetSpecialChar::read(Buffer const *, LyXLex & lex)
{    
	lex.nextToken();
	string const command = lex.GetString();

	if (command == "\\-")
		kind = HYPHENATION;
	else if (command == "\\@.")
		kind = END_OF_SENTENCE;
	else if (command == "\\ldots{}")
		kind = LDOTS;
	else if (command == "\\menuseparator")
	        kind = MENU_SEPARATOR;
	else if (command == "\\protected_separator"
		 || command == "~")
		kind = PROTECTED_SEPARATOR;
	else
		lex.printError("InsetSpecialChar: Unknown kind: `$$Token'");
}


int InsetSpecialChar::latex(Buffer const *, ostream & os, bool /*fragile*/,
			    bool free_space) const
{
	switch (kind) {
	case HYPHENATION:	  os << "\\-";	break;
	case END_OF_SENTENCE:	  os << "\\@.";	break;
	case LDOTS:		  os << "\\ldots{}";	break;
	case MENU_SEPARATOR:      os << "\\lyxarrow{}"; break;
	case PROTECTED_SEPARATOR: os << (free_space ? " " : "~"); break;
	}
	return 0;
}


int InsetSpecialChar::ascii(Buffer const *, ostream & os, int) const
{
	switch (kind) {
	case HYPHENATION:	                break;
	case END_OF_SENTENCE:	  os << ".";	break;
	case LDOTS:		  os << "...";	break;
	case MENU_SEPARATOR:      os << "->";   break;
	case PROTECTED_SEPARATOR: os << " ";   break;
	}
	return 0;
}


int InsetSpecialChar::linuxdoc(Buffer const * buf, ostream & os) const
{
	return ascii(buf, os, 0);
}


int InsetSpecialChar::docBook(Buffer const * buf, ostream & os) const
{
	return ascii(buf, os, 0);
}


Inset * InsetSpecialChar::clone(Buffer const &, bool) const
{
	return new InsetSpecialChar(kind);
}


void InsetSpecialChar::validate(LaTeXFeatures & features) const
{
	if (kind == MENU_SEPARATOR) {
		features.lyxarrow = true;
	}
}
