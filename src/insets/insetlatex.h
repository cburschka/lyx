// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *
 *======================================================*/

#ifndef _INSET_LATEX_H
#define _INSET_LATEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxinset.h"
#include "LString.h"

/** Latex. Used to insert non-deletable LaTeX-Code automatically

  Behaves as LaTeXDels, but cannot be deleted with the normal
  delete/backspace operations. This is used when you insert a LaTeX
  figure (done as "\input "), but you still have to type the filename
  yourself after the inset. */
class InsetLatex: public Inset {
public:
	///
	InsetLatex();
	///
	InsetLatex(string const & string);
	///
	~InsetLatex();
	///
	int Ascent(LyXFont const &font) const;
	///
	int Descent(LyXFont const &font) const;
	///
	int Width(LyXFont const &font) const;
	///
	void Draw(LyXFont font, LyXScreen &scr, int baseline, float &x);
	///
	void Write(FILE *file);
	///
	void Read(LyXLex &lex);
	///
	int Latex(FILE *file, signed char fragile);
	///
	int Latex(string &file, signed char fragile);
	///
	int Linuxdoc(string &file);
	///
	int DocBook(string &file);
	///
	bool Deletable() const;
	///
	Inset* Clone();
	///
	Inset::Code LyxCode() const;
private:
	///
	string contents;
};

#endif
