/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetlatex.h"
#include "lyxdraw.h"

/* Latex. Used to insert Latex-Code automatically */


InsetLatex::InsetLatex()
{
}


InsetLatex::InsetLatex(string const & string)
	: contents(string)
{
}


InsetLatex::~InsetLatex()
{
}


int InsetLatex::Ascent(LyXFont const & font) const
{
	return font.maxAscent() + 1;
}


int InsetLatex::Descent(LyXFont const & font) const
{
	return font.maxDescent() + 1;
}


int InsetLatex::Width(LyXFont const & font) const
{
	return 6 + font.stringWidth(contents);
}


void InsetLatex::Draw(LyXFont font, LyXScreen & scr,
		      int baseline, float & x)
{
	// Latex-insets are always LaTeX, so just correct the font */ 
	font.setLatex(LyXFont::ON);

	// Draw it as a box with the LaTeX text
	x += 1;

	scr.fillRectangle(gc_lighted, int(x), baseline - Ascent(font) +1 ,
			  Width(font) - 2, Ascent(font) + Descent(font) -2);
	
	scr.drawRectangle(gc_foot, int(x), baseline - Ascent(font) + 1,
			  Width(font)-2, Ascent(font)+Descent(font)-2);
	
	scr.drawString(font, contents, baseline, int(x+2));
	x +=  Width(font) - 1;
}


void InsetLatex::Write(FILE * file)
{
	fprintf(file, "Latex %s\n", contents.c_str());
}


void InsetLatex::Read(LyXLex & lex)
{
	FILE * file = lex.getFile();
	char c = 0;
	string tmp;
	while (!feof(file) && (c = fgetc(file)) != '\n') {
		tmp += char(c);
	}
	contents = tmp;
}


int InsetLatex::Latex(FILE * file, signed char /*fragile*/, bool /*fs*/)
{
	fprintf(file, "%s", contents.c_str());
	return 0;
}


int InsetLatex::Latex(string & file, signed char /*fragile*/, bool /*fs*/)
{
	file += contents;
	return 0;
}


int InsetLatex::Linuxdoc(string & file)
{
	file += contents;
	return 0;
}


int InsetLatex::DocBook(string & file)
{
	file += contents;
	return 0;
}


bool InsetLatex::Deletable() const
{
	return false;
}


InsetLatex * InsetLatex::Clone() const
{
	InsetLatex * result = new InsetLatex(contents);
	return result;
}


Inset::Code InsetLatex::LyxCode() const
{
	if (contents == "\\tableofcontents") return Inset::TOC_CODE;
	return Inset::NO_CODE;
}
